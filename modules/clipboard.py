# -*- coding: UTF-8 -*-
#
#       clipboard.py
#       
#       Copyright 2010 Giuseppe Penone <giuspen@gmail.com>
#       
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#       
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#       
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.

import gtk, xml.dom.minidom
import cons, machines, exports


TARGET_CTD_PLAIN_TEXT = 'UTF8_STRING'
TARGET_CTD_RICH_TEXT = 'CTD_RICH'
TARGET_CTD_IMAGE = 'image/png'
TARGET_CTD_TABLE = 'CTD_TABLE'
TARGET_CTD_CODEBOX = 'CTD_CODEBOX'
TARGET_HTML = 'text/html'
TARGETS_PLAIN_TEXT = ("UTF8_STRING", "COMPOUND_TEXT", "STRING", "TEXT")
TARGETS_IMAGES = ('image/png', 'image/jpeg', 'image/bmp', 'image/tiff', 'image/x-MS-bmp', 'image/x-bmp')


class ClipboardHandler:
   """Handler of Clipboard"""
   
   def __init__(self, dad):
      """Clipboard Handler boot"""
      self.dad = dad
      self.clipboard = gtk.clipboard_get()
      
   def copy(self, sourceview):
      """Copy to Clipboard"""
      sourceview.stop_emission("copy-clipboard")
      if not self.dad.curr_buffer.get_has_selection(): return
      self.selection_to_clipboard(self.dad.curr_buffer, sourceview)
      
   def cut(self, sourceview):
      """Cut to Clipboard"""
      sourceview.stop_emission("cut-clipboard")
      if not self.dad.curr_buffer.get_has_selection(): return
      self.selection_to_clipboard(self.dad.curr_buffer, sourceview)
      self.dad.curr_buffer.delete_selection(True, sourceview.get_editable())
      self.dad.sourceview.grab_focus()
      
   def selection_to_clipboard(self, text_buffer, sourceview):
      """Write the Selected Content to the Clipboard"""
      iter_sel_start, iter_sel_end = text_buffer.get_selection_bounds()
      num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset()
      if num_chars == 1:
         anchor = iter_sel_start.get_child_anchor()
         if anchor:
            anchor_dir = dir(anchor)
            if "pixbuf" in anchor_dir:
               self.clipboard.set_with_data([(TARGET_CTD_IMAGE, 0, 0)],
                                            self.get_func,
                                            self.clear_func,
                                            anchor.pixbuf)
               return
            elif "liststore" in anchor_dir:
               table_dict = self.dad.state_machine.table_to_dict(anchor)
               self.clipboard.set_with_data([(TARGET_CTD_TABLE, 0, 0)],
                                            self.get_func,
                                            self.clear_func,
                                            table_dict)
               return
            elif "sourcebuffer" in anchor_dir:
               codebox_dict = self.dad.state_machine.codebox_to_dict(anchor, for_print=False)
               self.clipboard.set_with_data([(TARGET_CTD_CODEBOX, 0, 0)],
                                            self.get_func,
                                            self.clear_func,
                                            codebox_dict)
               return
      html_handler = exports.Export2Html(self.dad)
      html_text = html_handler.selection_export_to_html(text_buffer, iter_sel_start, iter_sel_end, self.dad.syntax_highlighting)
      if self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         plain_text = text_buffer.get_text(iter_sel_start, iter_sel_end)
         rich_text = self.rich_text_get_from_text_buffer_selection(text_buffer, iter_sel_start, iter_sel_end)
         self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_PLAIN_TEXT, TARGET_CTD_RICH_TEXT, TARGET_HTML)],
                                      self.get_func,
                                      self.clear_func,
                                      (plain_text, rich_text, html_text))
      else:
         plain_text = text_buffer.get_text(iter_sel_start, iter_sel_end)
         self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_PLAIN_TEXT, TARGET_HTML)],
                                      self.get_func,
                                      self.clear_func,
                                      (plain_text, None, html_text))
      
   def get_func(self, clipboard, selectiondata, info, data):
      """Connected with clipboard.set_with_data"""
      target = selectiondata.get_target()
      if target == TARGET_CTD_PLAIN_TEXT: selectiondata.set('UTF8_STRING', 8, data[0])
      elif target == TARGET_CTD_RICH_TEXT: selectiondata.set('UTF8_STRING', 8, data[1])
      elif target == TARGET_HTML: selectiondata.set('UTF8_STRING', 8, data[2])
      elif target == TARGET_CTD_IMAGE: selectiondata.set_pixbuf(data)
      elif target == TARGET_CTD_CODEBOX:
         dom = xml.dom.minidom.Document()
         self.dad.xml_handler.codebox_element_to_xml([0, data, "left"], dom)
         selectiondata.set('UTF8_STRING', 8, dom.toxml())
      elif target == TARGET_CTD_TABLE:
         dom = xml.dom.minidom.Document()
         self.dad.xml_handler.table_element_to_xml([0, data, "left"], dom)
         selectiondata.set('UTF8_STRING', 8, dom.toxml())
      
   def clear_func(self, clipboard, data):
      """Connected with clipboard.set_with_data"""
      # this is to eventually free memory allocated when filling the clipboard
      pass
      
   def paste(self, sourceview):
      """Paste from Clipboard"""
      sourceview.stop_emission("paste-clipboard")
      targets = self.clipboard.wait_for_targets()
      if not targets: return
      self.dad.curr_buffer.delete_selection(True, sourceview.get_editable())
      self.clipboard.request_contents(TARGET_HTML, self.to_html)
      if TARGET_CTD_RICH_TEXT in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         self.clipboard.request_contents(TARGET_CTD_RICH_TEXT, self.to_rich_text)
         return
      if TARGET_CTD_CODEBOX in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         self.clipboard.request_contents(TARGET_CTD_CODEBOX, self.to_codebox)
         return
      if TARGET_CTD_TABLE in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
         self.clipboard.request_contents(TARGET_CTD_TABLE, self.to_table)
         return
      for target in TARGETS_IMAGES:
         if target in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.clipboard.request_contents(target, self.to_image)
            return
      for target in TARGETS_PLAIN_TEXT:
         if target in targets:
            self.clipboard.request_contents(target, self.to_plain_text)
            break
   
   def to_html(self, clipboard, selectiondata, data):
      """From Clipboard to HTML Text"""
      print ord(selectiondata.data[0]), ord(selectiondata.data[1]), selectiondata.data[2:]
   
   def to_plain_text(self, clipboard, selectiondata, data):
      """From Clipboard to Plain Text"""
      plain_text = selectiondata.get_text()
      self.dad.curr_buffer.insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                  plain_text)
   
   def to_rich_text(self, clipboard, selectiondata, data):
      """From Clipboard to Rich Text"""
      dom = xml.dom.minidom.parseString(selectiondata.get_text())
      dom_node = dom.firstChild
      if dom_node.nodeName != "root":
         print "rich text from clipboard error"
         return
      child_dom_iter = dom_node.firstChild
      while child_dom_iter != None:
         if child_dom_iter.nodeName == "slot":
            nephew_dom_iter = child_dom_iter.firstChild
            while nephew_dom_iter != None:
               if nephew_dom_iter.nodeName == "rich_text":
                  self.dom_node_to_rich_text(nephew_dom_iter)
               elif nephew_dom_iter.nodeName == "encoded_png":
                  self.dom_node_to_image(nephew_dom_iter)
               elif nephew_dom_iter.nodeName == "table":
                  self.dom_node_to_table(nephew_dom_iter)
               elif nephew_dom_iter.nodeName == "codebox":
                  self.dom_node_to_codebox(nephew_dom_iter)
               nephew_dom_iter = nephew_dom_iter.nextSibling
         child_dom_iter = child_dom_iter.nextSibling
   
   def dom_node_to_rich_text(self, dom_node):
      """From dom_node to Rich Text"""
      if dom_node.firstChild: text = dom_node.firstChild.data
      else: text = ""
      tag_names = []
      for tag_property in cons.TAG_PROPERTIES:
         if dom_node.hasAttribute(tag_property):
            property_value = dom_node.attributes[tag_property].value
            if property_value: tag_names.append(self.dad.apply_tag_exist_or_create(tag_property, property_value))
      tags_num = len(tag_names)
      if tags_num == 0:
         self.dad.curr_buffer.insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                     text)
      else:
         self.dad.curr_buffer.insert_with_tags_by_name(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                                       text,
                                                       *tag_names)
   
   def to_image(self, clipboard, selectiondata, data):
      """From Clipboard to Image"""
      pixbuf = selectiondata.get_pixbuf()
      self.dad.image_insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()), pixbuf)
   
   def dom_node_to_image(self, dom_node):
      """From dom_node to Image"""
      if dom_node.hasAttribute("justification"): justification = dom_node.attributes["justification"].value
      else: justification = "left"
      if dom_node.hasAttribute("anchor"):
         pixbuf = gtk.gdk.pixbuf_new_from_file(cons.ANCHOR_CHAR)
         pixbuf.anchor = dom_node.attributes["anchor"].value
      else: pixbuf = machines.get_pixbuf_from_encoded_buffer(dom_node.firstChild.data)
      self.dad.image_insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                            pixbuf,
                            justification)
   
   def to_codebox(self, clipboard, selectiondata, data):
      """From Clipboard to CodeBox"""
      dom = xml.dom.minidom.parseString(selectiondata.get_text())
      dom_node = dom.firstChild
      if dom_node.nodeName != "codebox":
         print "codebox from clipboard error"
         return
      self.dom_node_to_codebox(dom_node)
   
   def dom_node_to_codebox(self, dom_node):
      """From dom_node to CodeBox"""
      if dom_node.hasAttribute("justification"): justification = dom_node.attributes["justification"].value
      else: justification = "left"
      if dom_node.hasAttribute("width_in_pixels") and dom_node.attributes['width_in_pixels'].value != "True":
         width_in_pixels = False
      else: width_in_pixels = True
      if dom_node.firstChild: fill_text = dom_node.firstChild.data
      else: fill_text = ""
      codebox_dict = {
      'frame_width': int(dom_node.attributes['frame_width'].value),
      'frame_height': int(dom_node.attributes['frame_height'].value),
      'width_in_pixels': width_in_pixels,
      'syntax_highlighting': dom_node.attributes['syntax_highlighting'].value,
      'fill_text': fill_text
      }
      self.dad.codeboxes_handler.codebox_insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                                codebox_dict,
                                                justification)
   
   def to_table(self, clipboard, selectiondata, data):
      """From Clipboard to Table"""
      dom = xml.dom.minidom.parseString(selectiondata.get_text())
      dom_node = dom.firstChild
      if dom_node.nodeName != "table":
         print "table from clipboard error"
         return
      self.dom_node_to_table(dom_node)
   
   def dom_node_to_table(self, dom_node):
      """From dom_node to Table"""
      if dom_node.hasAttribute("justification"): justification = dom_node.attributes["justification"].value
      else: justification = "left"
      table = {'matrix': [], 
               'col_min': int(dom_node.attributes['col_min'].value),
               'col_max': int(dom_node.attributes["col_max"].value)}
      child_dom_iter = dom_node.firstChild
      while child_dom_iter != None:
         if child_dom_iter.nodeName == "row":
            table['matrix'].append([])
            nephew_dom_iter = child_dom_iter.firstChild
            while nephew_dom_iter != None:
               if nephew_dom_iter.nodeName == "cell":
                  if nephew_dom_iter.firstChild != None: table['matrix'][-1].append(nephew_dom_iter.firstChild.data)
                  else: table['matrix'][-1].append("")
               nephew_dom_iter = nephew_dom_iter.nextSibling
         child_dom_iter = child_dom_iter.nextSibling
      self.dad.tables_handler.table_insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                           table,
                                           justification)
   
   def rich_text_get_from_text_buffer_selection(self, text_buffer, iter_sel_start, iter_sel_end):
      """Given text_buffer and selection, returns the rich text xml"""
      pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(text_buffer,
                                                                                                 for_print=1,
                                                                                                 sel_range=(iter_sel_start.get_offset(), iter_sel_end.get_offset()))
      # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
      dom = xml.dom.minidom.Document()
      root = dom.createElement("root")
      dom.appendChild(root)
      obj_pos = 0
      start_offset = iter_sel_start.get_offset()
      for end_offset_element in pixbuf_table_codebox_vector:
         end_offset = end_offset_element[1][0]
         if obj_pos < len(pixbuf_table_codebox_vector): obj_element = pixbuf_table_codebox_vector[obj_pos]
         else: obj_element = None
         self.rich_text_process_slot(dom, root, start_offset, end_offset, text_buffer, obj_element)
         obj_pos += 1
         start_offset = end_offset
      self.rich_text_process_slot(dom, root, start_offset, iter_sel_end.get_offset(), text_buffer, None)
      return dom.toxml()
      
   def rich_text_process_slot(self, dom, root, start_offset, end_offset, text_buffer, obj_element):
      """Process a Single Pango Slot"""
      dom_iter = dom.createElement("slot")
      root.appendChild(dom_iter)
      start_iter = text_buffer.get_iter_at_offset(start_offset)
      #print "process slot (%s->%s)" % (start_offset, end_offset)
      # begin operations
      self.curr_attributes = {}
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      curr_iter = start_iter.copy()
      self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
      tag_found = curr_iter.forward_to_tag_toggle(None)
      while tag_found:
         if curr_iter.get_offset() > end_offset:
            curr_iter = text_buffer.get_iter_at_offset(end_offset)
         self.dad.xml_handler.rich_text_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes)
         offset_old = curr_iter.get_offset()
         if offset_old >= end_offset: break
         else:
            self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
            start_iter.set_offset(offset_old)
            tag_found = curr_iter.forward_to_tag_toggle(None)
            if curr_iter.get_offset() == offset_old: break
      else: self.dad.xml_handler.rich_text_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes)
      if obj_element:
         if obj_element[0] == "pixbuf": self.dad.xml_handler.pixbuf_element_to_xml(obj_element[1], dom_iter, dom)
         elif obj_element[0] == "table": self.dad.xml_handler.table_element_to_xml(obj_element[1], dom_iter)
         elif obj_element[0] == "codebox": self.dad.xml_handler.codebox_element_to_xml(obj_element[1], dom_iter)
