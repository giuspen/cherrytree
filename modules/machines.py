# -*- coding: UTF-8 -*-
#
#       machines.py
#       
#       Copyright 2009-2011 Giuseppe Penone <giuspen@gmail.com>
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

import gtk, xml.dom.minidom, re, base64, copy, StringIO
import cons, support, exports


def get_encoded_buffer_from_pixbuf(pixbuf):
   """Pixbuf To Encoded Buffer"""
   io = StringIO.StringIO()
   pixbuf.save_to_callback(io.write, "png")
   encoded_buffer = base64.b64encode(io.getvalue())
   return encoded_buffer

def get_pixbuf_from_encoded_buffer(buffer):
   """Encoded Buffer To Pixbuf"""
   pixbuf_loader = gtk.gdk.pixbuf_loader_new_with_mime_type("image/png")
   pixbuf_loader.write(base64.b64decode(buffer))
   pixbuf_loader.close()
   pixbuf = pixbuf_loader.get_pixbuf()
   return pixbuf

def get_pixbuf_from_png_encoded_string(png_encoded_string):
   """PNG Encoded String To Pixbuf"""
   fd = open(cons.IMG_PATH, "wb")
   fd.write(base64.b64decode(png_encoded_string))
   fd.close()
   pixbuf = gtk.gdk.pixbuf_new_from_file(cons.IMG_PATH)
   return pixbuf


class XMLHandler:
   """The Handler of the procedures involving XML"""
   
   def __init__(self, dad):
      """Machine boot"""
      self.dad = dad
   
   def dom_to_buffer(self, textbuffer, tagged_text):
      """Given a TextBuffer and a string of rich text, fills the buffer properly"""
      textbuffer.delete(*textbuffer.get_bounds())
      dom = xml.dom.minidom.parseString(tagged_text)
      dom_iter = dom.firstChild
      if dom_iter.nodeName != "node": return False
      child_dom_iter = dom_iter.firstChild
      # loop for text
      while child_dom_iter!= None:
         if child_dom_iter.nodeName == "rich_text": self.rich_text_deserialize(textbuffer, child_dom_iter)
         child_dom_iter = child_dom_iter.nextSibling
      textbuffer.set_modified(False)
      return True
   
   def dom_to_treestore(self, ctd, discard_ids, tree_father=None, reset_nodes_names=True):
      """Parse an XML Cherry Tree Document file and build the Tree"""
      dom = xml.dom.minidom.parseString(ctd)
      cherrytree = dom.firstChild
      self.dad.bookmarks = []
      if reset_nodes_names:
         self.dad.nodes_names_dict = {}
         bookmarks_menu = self.dad.ui.get_widget("/MenuBar/BookmarksMenu").get_submenu()
         for menu_item in self.dad.bookmarks_menu_items:
            bookmarks_menu.remove(menu_item)
         self.dad.bookmarks_menu_items = []
      if cherrytree.nodeName != "cherrytree": return False
      else:
         dom_iter = cherrytree.firstChild
         while dom_iter!= None:
            if dom_iter.nodeName == "node": self.append_tree_node(dom_iter, tree_father, discard_ids)
            elif dom_iter.nodeName == "bookmarks":
               self.dad.bookmarks = dom_iter.attributes['list'].value.split(",")
            dom_iter = dom_iter.nextSibling
         return True
         
   def append_tree_node(self, dom_iter, tree_father, discard_ids):
      """Given the dom_iter node, adds it to the tree"""
      if not discard_ids and dom_iter.hasAttribute('unique_id'):
         unique_id = long(dom_iter.attributes['unique_id'].value)
         self.dad.node_id_add(unique_id)
      else: unique_id = self.dad.node_id_get()
      if dom_iter.hasAttribute('tags'): node_tags = dom_iter.attributes['tags'].value
      else: node_tags = ""
      if dom_iter.hasAttribute('readonly'): readonly = (dom_iter.attributes['readonly'].value == "True")
      else: readonly = False
      syntax_highlighting = dom_iter.attributes['prog_lang'].value
      if syntax_highlighting != cons.CUSTOM_COLORS_ID and syntax_highlighting not in self.dad.available_languages:
         syntax_highlighting = syntax_highlighting.lower().replace("C++", "cpp")
         if syntax_highlighting not in self.dad.available_languages:
            syntax_highlighting = cons.CUSTOM_COLORS_ID
      if tree_father == None: node_level = 0
      else: node_level = self.dad.treestore[tree_father][5] + 1
      cherry = self.dad.get_node_icon(node_level, syntax_highlighting)
      curr_buffer = self.dad.buffer_create(syntax_highlighting)
      if syntax_highlighting != cons.CUSTOM_COLORS_ID: curr_buffer.begin_not_undoable_action()
      # loop into rich text, write into the buffer
      child_dom_iter = dom_iter.firstChild
      while child_dom_iter != None:
         if child_dom_iter.nodeName == "rich_text":
            self.rich_text_deserialize(curr_buffer, child_dom_iter)
         elif child_dom_iter.nodeName == "encoded_png": self.image_deserialize(curr_buffer, child_dom_iter, 2)
         elif child_dom_iter.nodeName == "table": self.table_deserialize(curr_buffer, child_dom_iter)
         elif child_dom_iter.nodeName == "codebox": self.codebox_deserialize(curr_buffer, child_dom_iter)
         elif child_dom_iter.nodeName == "encoded_image": self.image_deserialize(curr_buffer, child_dom_iter, 1)
         elif child_dom_iter.nodeName == "node": break
         child_dom_iter = child_dom_iter.nextSibling
      if syntax_highlighting != cons.CUSTOM_COLORS_ID: curr_buffer.end_not_undoable_action()
      curr_buffer.set_modified(False)
      #print unique_id
      # insert the node containing the buffer into the tree
      tree_iter = self.dad.treestore.append(tree_father, [cherry,
                                                          dom_iter.attributes['name'].value,
                                                          curr_buffer,
                                                          unique_id,
                                                          syntax_highlighting,
                                                          node_level,
                                                          node_tags,
                                                          readonly])
      self.dad.nodes_names_dict[self.dad.treestore[tree_iter][3]] = self.dad.treestore[tree_iter][1]
      # loop for child nodes
      while child_dom_iter!= None:
         if child_dom_iter.nodeName == 'rich_text':
            support.dialog_error("Rich text instead of child node??!!", self.dad.window)
            break
         elif child_dom_iter.nodeName == "node": self.append_tree_node(child_dom_iter, tree_iter, discard_ids)
         child_dom_iter = child_dom_iter.nextSibling
      
   def codebox_deserialize(self, curr_buffer, dom_node):
      """From the XML codebox text to the SourceBuffer"""
      char_offset = int(dom_node.attributes["char_offset"].value)
      justification = dom_node.attributes["justification"].value if dom_node.hasAttribute("justification") else "left"
      self.dad.curr_buffer = curr_buffer # the table_insert method will need this
      codebox_dict = {
         'frame_width': int(dom_node.attributes['frame_width'].value),
         'frame_height': int(dom_node.attributes['frame_height'].value),
         'width_in_pixels': dom_node.hasAttribute("width_in_pixels") and dom_node.attributes['width_in_pixels'].value == "True",
         'syntax_highlighting': dom_node.attributes['syntax_highlighting'].value,
         'highlight_brackets': dom_node.hasAttribute("highlight_brackets") and dom_node.attributes['highlight_brackets'].value == "True",
         'show_line_numbers': dom_node.hasAttribute("show_line_numbers") and dom_node.attributes['show_line_numbers'].value == "True",
         'fill_text': dom_node.firstChild.data if dom_node.firstChild else ""
      }
      self.dad.codeboxes_handler.codebox_insert(curr_buffer.get_iter_at_offset(char_offset),
                                                codebox_dict,
                                                justification)
      
   def table_deserialize(self, curr_buffer, dom_node):
      """From the XML table text to the SourceBuffer"""
      char_offset = int(dom_node.attributes["char_offset"].value)
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
      self.dad.curr_buffer = curr_buffer # the table_insert method will need this
      self.dad.tables_handler.table_insert(curr_buffer.get_iter_at_offset(char_offset), table, justification)
      
   def image_deserialize(self, curr_buffer, dom_node, version):
      """From the XML embedded image text to the SourceBuffer"""
      char_offset = int(dom_node.attributes["char_offset"].value)
      if dom_node.hasAttribute("justification"): justification = dom_node.attributes["justification"].value
      else: justification = "left"
      if dom_node.hasAttribute("anchor"):
         pixbuf = gtk.gdk.pixbuf_new_from_file(cons.ANCHOR_CHAR)
         pixbuf.anchor = dom_node.attributes["anchor"].value
      else:
         if version == 2: pixbuf = get_pixbuf_from_encoded_buffer(dom_node.firstChild.data)
         else: pixbuf = get_pixbuf_from_png_encoded_string(dom_node.firstChild.data)
      self.dad.curr_buffer = curr_buffer # the apply_tag method will need this
      self.dad.image_insert(curr_buffer.get_iter_at_offset(char_offset), pixbuf, justification)
      
   def rich_text_deserialize(self, curr_buffer, dom_node):
      """From the XML rich text to the SourceBuffer"""
      if dom_node.firstChild: text = dom_node.firstChild.data
      else: text = ""
      tag_names = []
      for tag_property in cons.TAG_PROPERTIES:
         if dom_node.hasAttribute(tag_property):
            property_value = dom_node.attributes[tag_property].value
            if property_value: tag_names.append(self.dad.apply_tag_exist_or_create(tag_property, property_value))
      tags_num = len(tag_names)
      if tags_num == 0: curr_buffer.insert(curr_buffer.get_end_iter(), text)
      else: curr_buffer.insert_with_tags_by_name(curr_buffer.get_end_iter(), text, *tag_names)
      
   def treestore_node_to_dom(self, node_iter):
      """Given a treestore iter returns the CherryTree rich text"""
      if "dom" in dir(self): del self.dom
      self.dom = xml.dom.minidom.Document()
      self.append_dom_node(node_iter, self.dom, to_disk=False)
      return self.dom.toxml()
      
   def treestore_sel_node_and_subnodes_to_dom(self, tree_iter):
      """Parse the Given Node and Subnodes and Generate an XML Cherry Tree Document"""
      if "dom" in dir(self): del self.dom
      self.dom = xml.dom.minidom.Document()
      cherrytree = self.dom.createElement("cherrytree")
      self.dom.appendChild(cherrytree)
      # given node and subnodes parsing
      self.append_dom_node(tree_iter, cherrytree, to_disk=True)
      return self.dom.toxml()
      
   def treestore_to_dom(self):
      """Parse the Tree and Generate an XML Cherry Tree Document"""
      if "dom" in dir(self): del self.dom
      self.dom = xml.dom.minidom.Document()
      cherrytree = self.dom.createElement("cherrytree")
      self.dom.appendChild(cherrytree)
      # full tree parsing
      tree_iter = self.dad.treestore.get_iter_first()
      while tree_iter != None:
         self.append_dom_node(tree_iter, cherrytree, to_disk=True)
         tree_iter = self.dad.treestore.iter_next(tree_iter)
      self.append_bookmarks(cherrytree)
      return self.dom.toxml()
      
   def append_bookmarks(self, dom_father):
      """Adds the bookmarks data to the DOM"""
      if len(self.dad.bookmarks) == 0: return
      dom_iter = self.dom.createElement("bookmarks")
      dom_iter.setAttribute("list", ",".join(self.dad.bookmarks))
      dom_father.appendChild(dom_iter)
      
   def append_dom_node(self, tree_iter, dom_father, to_disk):
      """Given the tree_iter node, adds it to the DOM"""
      dom_iter = self.dom.createElement("node")
      dom_iter.setAttribute("name", self.dad.treestore[tree_iter][1])
      dom_iter.setAttribute("unique_id", str(self.dad.treestore[tree_iter][3]))
      programming_language = self.dad.treestore[tree_iter][4]
      dom_iter.setAttribute("prog_lang", programming_language)
      dom_iter.setAttribute("tags", self.dad.treestore[tree_iter][6])
      dom_iter.setAttribute("readonly", str(self.dad.treestore[tree_iter][7]))
      dom_father.appendChild(dom_iter)
      # allocate and init the rich text attributes
      self.curr_attributes = {}
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      curr_buffer = self.dad.treestore[tree_iter][2]
      start_iter = curr_buffer.get_start_iter()
      end_iter = curr_buffer.get_end_iter()
      if programming_language == cons.CUSTOM_COLORS_ID:
         # rich text insert
         curr_iter = start_iter.copy()
         self.rich_text_attributes_update(curr_iter, self.curr_attributes)
         tag_found = curr_iter.forward_to_tag_toggle(None)
         while tag_found:
            self.rich_text_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes)
            if curr_iter.compare(end_iter) == 0: break
            else:
               self.rich_text_attributes_update(curr_iter, self.curr_attributes)
               offset_old = curr_iter.get_offset()
               start_iter.set_offset(offset_old)
               tag_found = curr_iter.forward_to_tag_toggle(None)
               if curr_iter.get_offset() == offset_old: break
         else: self.rich_text_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes)
         # in case of writing to disk it's time to serialize the info about the images
         if to_disk == True:
            pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer)
            # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
            for element in pixbuf_table_codebox_vector:
               if element[0] == "pixbuf": self.pixbuf_element_to_xml(element[1], dom_iter, self.dom)
               elif element[0] == "table": self.table_element_to_xml(element[1], dom_iter)
               elif element[0] == "codebox": self.codebox_element_to_xml(element[1], dom_iter)
      else:
         # plain text insert
         self.rich_text_serialize(dom_iter, start_iter, end_iter, self.curr_attributes)
      tree_iter = self.dad.treestore.iter_children(tree_iter) # check for childrens
      while tree_iter != None:
         self.append_dom_node(tree_iter, dom_iter, to_disk)
         tree_iter = self.dad.treestore.iter_next(tree_iter)
         
   def codebox_element_to_xml(self, element, dom_node):
      """From element [char_offset, codebox, justification] to dom node"""
      dom_iter = self.dom.createElement("codebox")
      dom_iter.setAttribute("char_offset", str(element[0]))
      if element[2] != "left": dom_iter.setAttribute("justification", element[2])
      dom_iter.setAttribute("frame_width", str(element[1]['frame_width']))
      dom_iter.setAttribute("frame_height", str(element[1]['frame_height']))
      dom_iter.setAttribute("width_in_pixels", str(element[1]['width_in_pixels']))
      dom_iter.setAttribute("syntax_highlighting", str(element[1]['syntax_highlighting']))
      dom_iter.setAttribute("highlight_brackets", str(element[1]['highlight_brackets']))
      dom_iter.setAttribute("show_line_numbers", str(element[1]['show_line_numbers']))
      dom_node.appendChild(dom_iter)
      text_iter = self.dom.createTextNode(element[1]['fill_text'])
      dom_iter.appendChild(text_iter)
      
   def table_element_to_xml(self, element, dom_node):
      """From element [char_offset, table, justification] to dom node"""
      dom_iter = self.dom.createElement("table")
      dom_iter.setAttribute("char_offset", str(element[0]))
      if element[2] != "left": dom_iter.setAttribute("justification", element[2])
      dom_iter.setAttribute("col_min", str(element[1]['col_min']))
      dom_iter.setAttribute("col_max", str(element[1]['col_max']))
      dom_node.appendChild(dom_iter)
      for row in element[1]['matrix']:
         dom_row = self.dom.createElement("row")
         dom_iter.appendChild(dom_row)
         for cell in row:
            dom_cell = self.dom.createElement("cell")
            dom_row.appendChild(dom_cell)
            text_iter = self.dom.createTextNode(cell)
            dom_cell.appendChild(text_iter)
   
   def pixbuf_element_to_xml(self, element, dom_node, dom):
      """From element [char_offset, pixbuf, justification] to dom node"""
      dom_iter = dom.createElement("encoded_png")
      dom_iter.setAttribute("char_offset", str(element[0]))
      if element[2] != "left": dom_iter.setAttribute("justification", element[2])
      if "anchor" in dir(element[1]):
         dom_iter.setAttribute("anchor", element[1].anchor)
         is_anchor_image = True
      else: is_anchor_image = False
      dom_node.appendChild(dom_iter)
      if not is_anchor_image: text_iter = dom.createTextNode(get_encoded_buffer_from_pixbuf(element[1]))
      else: text_iter = dom.createTextNode("anchor")
      dom_iter.appendChild(text_iter)
   
   def rich_text_serialize(self, dom_node, start_iter, end_iter, curr_attributes, change_case="n"):
      """Appends a new SourceBuffer part to the XML rich text"""
      dom_iter = self.dom.createElement("rich_text")
      for tag_property in cons.TAG_PROPERTIES:
         if curr_attributes[tag_property] != "":
            dom_iter.setAttribute(tag_property, curr_attributes[tag_property])
      dom_node.appendChild(dom_iter)
      slot_text = start_iter.get_text(end_iter)
      if change_case != "n":
         if change_case == "l": slot_text = slot_text.lower()
         elif change_case == "u": slot_text = slot_text.upper()
         elif change_case == "t": slot_text = slot_text.swapcase()
      text_iter = self.dom.createTextNode(slot_text)
      dom_iter.appendChild(text_iter)
      
   def rich_text_attributes_update(self, curr_iter, curr_attributes):
      """Updates the list of Attributes for the Current Slice"""
      toggled_off = curr_iter.get_toggled_tags(toggled_on=False)
      for tag in toggled_off:
         tag_name = tag.get_property("name")
         if tag_name:
            if tag_name[0:7] == "weight_": curr_attributes["weight"] = ""
            elif tag_name[0:11] == "foreground_": curr_attributes["foreground"] = ""
            elif tag_name[0:11] == "background_": curr_attributes["background"] = ""
            elif tag_name[0:6] == "style_": curr_attributes["style"] = ""
            elif tag_name[0:10] == "underline_": curr_attributes["underline"] = ""
            elif tag_name[0:14] == "strikethrough_": curr_attributes["strikethrough"] = ""
            elif tag_name[0:6] == "scale_": curr_attributes["scale"] = ""
            elif tag_name[0:14] == "justification_": curr_attributes["justification"] = ""
            elif tag_name[0:5] == "link_": curr_attributes["link"] = ""
            else: support.dialog_error("Failure processing the toggling OFF tag %s" % tag_name, self.dad.window)
      toggled_on = curr_iter.get_toggled_tags(toggled_on=True)
      for tag in toggled_on:
         tag_name = tag.get_property("name")
         if tag_name:
            if tag_name[0:7] == "weight_": curr_attributes["weight"] = tag_name[7:]
            elif tag_name[0:11] == "foreground_":
               new_foreground = tag_name[11:]
               if new_foreground != cons.COLOR_BLACK: curr_attributes["foreground"] = new_foreground
            elif tag_name[0:11] == "background_":
               new_background = tag_name[11:]
               if new_background != cons.COLOR_WHITE: curr_attributes["background"] = new_background
            elif tag_name[0:6] == "scale_": curr_attributes["scale"] = tag_name[6:]
            elif tag_name[0:14] == "justification_": curr_attributes["justification"] = tag_name[14:]
            elif tag_name[0:6] == "style_": curr_attributes["style"] = tag_name[6:]
            elif tag_name[0:10] == "underline_": curr_attributes["underline"] = tag_name[10:]
            elif tag_name[0:14] == "strikethrough_": curr_attributes["strikethrough"] = tag_name[14:]
            elif tag_name[0:5] == "link_": curr_attributes["link"] = tag_name[5:]
            else: support.dialog_error("Failure processing the toggling ON tag %s" % tag_name, self.dad.window)
   
   def toc_insert(self, text_buffer, node_id):
      """Given the text_buffer, inserts the Table Of Contents"""
      self.curr_attributes = {}
      self.toc_counters = {"h1":0, "h2":0}
      self.toc_list = [] # 0: anchor name; 1: text in h1 or h2
      for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
      start_iter = text_buffer.get_start_iter()
      end_iter = text_buffer.get_end_iter()
      curr_iter = start_iter.copy()
      self.rich_text_attributes_update(curr_iter, self.curr_attributes)
      tag_found = curr_iter.forward_to_tag_toggle(None)
      while tag_found:
         offsets = self.toc_insert_parser(text_buffer, start_iter, curr_iter)
         if offsets:
            start_iter = text_buffer.get_iter_at_offset(offsets[0])
            curr_iter = text_buffer.get_iter_at_offset(offsets[1])
            end_iter = text_buffer.get_end_iter()
         if curr_iter.compare(end_iter) == 0: break
         else:
            self.rich_text_attributes_update(curr_iter, self.curr_attributes)
            offset_old = curr_iter.get_offset()
            start_iter.set_offset(offset_old)
            tag_found = curr_iter.forward_to_tag_toggle(None)
            if curr_iter.get_offset() == offset_old: break
      else: self.toc_insert_parser(text_buffer, start_iter, curr_iter)
      if len(self.toc_list) == 0: return False
      tag_property == "link"
      property_value = "node" + cons.CHAR_SPACE + str(node_id)
      curr_offset = 0
      text_buffer.insert(text_buffer.get_iter_at_offset(curr_offset), cons.CHAR_NEWLINE)
      curr_offset += 1
      for element in self.toc_list:
         tag_names = []
         tag_names.append(self.dad.apply_tag_exist_or_create(tag_property, property_value + cons.CHAR_SPACE + element[0]))
         if element[0][:2] == "h1":
            text_buffer.insert(text_buffer.get_iter_at_offset(curr_offset), cons.CHAR_LISTBUL + cons.CHAR_SPACE)
            curr_offset += 2
         else:
            text_buffer.insert(text_buffer.get_iter_at_offset(curr_offset), 3*cons.CHAR_SPACE + cons.CHAR_LISTBUL + cons.CHAR_SPACE)
            curr_offset += 5
         text_buffer.insert_with_tags_by_name(text_buffer.get_iter_at_offset(curr_offset), element[1], *tag_names)
         curr_offset += len(element[1])
         text_buffer.insert(text_buffer.get_iter_at_offset(curr_offset), cons.CHAR_NEWLINE)
         curr_offset += 1
      text_buffer.insert(text_buffer.get_iter_at_offset(curr_offset), cons.CHAR_NEWLINE)
      return True
   
   def toc_insert_parser(self, text_buffer, start_iter, end_iter):
      """Parses a Tagged String for the TOC insert"""
      if self.curr_attributes["scale"] not in ["h1", "h2"]: return None
      start_offset = start_iter.get_offset()
      end_offset = end_iter.get_offset()
      if self.curr_attributes["scale"] == "h1":
         self.toc_counters["h1"] += 1
         self.toc_list.append(["h1-%d" % self.toc_counters["h1"], text_buffer.get_text(start_iter, end_iter)])
      else:
         self.toc_counters["h2"] += 1
         self.toc_list.append(["h2-%d" % self.toc_counters["h2"], text_buffer.get_text(start_iter, end_iter)])
      anchor_start = start_iter.copy()
      if anchor_start.backward_char():
         anchor = anchor_start.get_child_anchor()
         if anchor and "pixbuf" in dir(anchor) and "anchor" in dir(anchor.pixbuf):
            text_buffer.delete(anchor_start, start_iter)
            start_offset -= 1
            end_offset -= 1
            start_iter = text_buffer.get_iter_at_offset(start_offset)
      pixbuf = gtk.gdk.pixbuf_new_from_file(cons.ANCHOR_CHAR)
      pixbuf.anchor = self.toc_list[-1][0]
      self.dad.image_insert(start_iter, pixbuf)
      return (start_offset+1, end_offset+1)


class StateMachine:
   """The State Machine for the TextBuffer Ctrl+Z / Ctrl+Y Utility"""
   
   def __init__(self, dad):
      """Machine boot"""
      self.nodes_vectors = {}
      self.nodes_indexes = {}
      self.nodes_indicators = {}
      self.visited_nodes_list = []
      self.visited_nodes_idx = 0
      # indicator 0 -> current text in node_vector matches the textbuffer
      # indicator 1 -> textbuffer is ahead, but only of non alphanumeric chars
      # indicator 2 -> textbuffer is ahead with alphanumeric chars
      self.dad = dad
      
   def get_embedded_pixbufs_tables_codeboxes(self, text_buffer, for_print=0, sel_range=None):
      """Retrieve the list of Images Embedded into the Buffer"""
      pixbuf_table_codebox_vector = []
      if sel_range: curr_iter = text_buffer.get_iter_at_offset(sel_range[0])
      else: curr_iter = text_buffer.get_start_iter()
      while 1:
         anchor = curr_iter.get_child_anchor()
         if anchor != None:
            anchor_dir = dir(anchor)
            if "pixbuf" in anchor_dir:
               pixbuf_table_codebox_vector.append(["pixbuf", [curr_iter.get_offset(),
                                                   anchor.pixbuf,
                                                   self.get_iter_alignment(curr_iter)] ])
            elif "liststore" in anchor_dir:
               pixbuf_table_codebox_vector.append(["table", [curr_iter.get_offset(),
                                                   self.table_to_dict(anchor),
                                                   self.get_iter_alignment(curr_iter)] ])
            elif "sourcebuffer" in anchor_dir:
               pixbuf_table_codebox_vector.append(["codebox", [curr_iter.get_offset(),
                                                   self.codebox_to_dict(anchor, for_print),
                                                   self.get_iter_alignment(curr_iter)] ])
         if not curr_iter.forward_char(): break
         if sel_range and curr_iter.get_offset() > sel_range[1]: break
      return pixbuf_table_codebox_vector
      
   def table_to_dict(self, anchor):
      """Given an Anchor, Returns the Embedded Table as a dictionary"""
      columns_num = len(anchor.headers)
      table = {'matrix':[], 'col_min': anchor.table_col_min, 'col_max': anchor.table_col_max}
      tree_iter = anchor.liststore.get_iter_first()
      while tree_iter != None:
         row = []
         for column in range(columns_num): row.append(anchor.liststore[tree_iter][column])
         table['matrix'].append(row)
         tree_iter = anchor.liststore.iter_next(tree_iter)
      table['matrix'].append(copy.deepcopy(anchor.headers))
      return table
      
   def codebox_to_dict(self, anchor, for_print):
      """Given an Anchor, Returns the Embedded Codebox as a dictionary"""
      codebox_dict = {
      'frame_width': anchor.frame_width,
      'frame_height': anchor.frame_height,
      'width_in_pixels': anchor.width_in_pixels,
      'syntax_highlighting': anchor.syntax_highlighting,
      'highlight_brackets': anchor.highlight_brackets,
      'show_line_numbers': anchor.show_line_numbers,
      'fill_text': ""
      }
      if for_print == 1:
         pango_handler = exports.Export2Pango(self)
         codebox_dict['fill_text'] = pango_handler.pango_get_from_code_buffer(anchor.sourcebuffer)
      elif for_print == 2:
         codebox_dict['fill_text'] = self.dad.html_handler.html_get_from_code_buffer(anchor.sourcebuffer)
      else: codebox_dict['fill_text'] = anchor.sourcebuffer.get_text(*anchor.sourcebuffer.get_bounds())
      return codebox_dict
      
   def get_iter_alignment(self, iter_text):
      """Get the Alignment Value of the given Iter"""
      align_center = self.dad.apply_tag_exist_or_create("justification", "center")
      align_right = self.dad.apply_tag_exist_or_create("justification", "right")
      if iter_text.has_tag(self.dad.tag_table.lookup(align_center)): return "center"
      elif iter_text.has_tag(self.dad.tag_table.lookup(align_right)): return "right"
      else: return "left"
      
   def load_embedded_image_element(self, text_buffer, element):
      """Load an Image from the Embedded Vector into the Buffer"""
      iter_insert = text_buffer.get_iter_at_offset(element[0])
      self.dad.image_insert(iter_insert, element[1], element[2])
      
   def load_embedded_table_element(self, text_buffer, element):
      """Load a Table from the Embedded Vector into the Buffer"""
      iter_insert = text_buffer.get_iter_at_offset(element[0])
      self.dad.tables_handler.table_insert(iter_insert, element[1], element[2])
      
   def load_embedded_codebox_element(self, text_buffer, element):
      """Load a CodeBox from the Embedded Vector into the Buffer"""
      iter_insert = text_buffer.get_iter_at_offset(element[0])
      codebox_dict = {
      'frame_width': element[1]['frame_width'],
      'frame_height': element[1]['frame_height'],
      'width_in_pixels': element[1]['width_in_pixels'],
      'syntax_highlighting': element[1]['syntax_highlighting'],
      'highlight_brackets': element[1]['highlight_brackets'],
      'show_line_numbers': element[1]['show_line_numbers'],
      'fill_text': element[1]['fill_text']
      }
      self.dad.codeboxes_handler.codebox_insert(iter_insert,
                                                codebox_dict,
                                                element[2])
      
   def apply_image_justification(self, iter_start, justification):
      """Apply the Proper Justification to an Image"""
      if justification == None: return
      iter_end = iter_start.copy()
      iter_end.forward_char()
      self.dad.apply_tag("justification", justification, iter_sel_start=iter_start, iter_sel_end=iter_end)
      
   def reset(self):
      """State Machine Reset"""
      del self.nodes_vectors
      del self.nodes_indexes
      del self.nodes_indicators
      del self.visited_nodes_list
      del self.visited_nodes_idx
      self.nodes_vectors = {}
      self.nodes_indexes = {}
      self.nodes_indicators = {}
      self.visited_nodes_list = []
      self.visited_nodes_idx = 0
      
   def requested_previous_visited(self):
      """Requested the Previous Visited Node"""
      if self.visited_nodes_idx != None and self.visited_nodes_idx > 0:
         self.visited_nodes_idx -= 1
         return self.visited_nodes_list[self.visited_nodes_idx]
      else:
         #print "self.visited_nodes_idx", self.visited_nodes_idx
         return None
      
   def requested_next_visited(self):
      """Requested the Next Visited Node"""
      if self.visited_nodes_idx != None and self.visited_nodes_idx < len(self.visited_nodes_list) - 1:
         self.visited_nodes_idx += 1
         return self.visited_nodes_list[self.visited_nodes_idx]
      else:
         #print "self.visited_nodes_idx", self.visited_nodes_idx
         #print "last_index",  len(self.visited_nodes_list) - 1
         return None
      
   def forget_last_visited(self):
      """Remove the latest element from the list of the visited nodes"""
      self.visited_nodes_idx -= 1
      self.visited_nodes_list.pop()
      
   def node_selected_changed(self, node_id):
      """When a New Node is Selected"""
      if not self.dad.go_bk_fw_click:
         last_index = len(self.visited_nodes_list) - 1
         if self.visited_nodes_idx != last_index: del self.visited_nodes_list[self.visited_nodes_idx+1:last_index+1]
         self.visited_nodes_list.append(node_id)
         self.visited_nodes_idx = len(self.visited_nodes_list) - 1
      if node_id not in self.nodes_vectors:
         self.nodes_vectors[node_id] = []
         xml_content = self.dad.xml_handler.treestore_node_to_dom(self.dad.curr_tree_iter)
         pixbuf_table_codebox_vector = self.get_embedded_pixbufs_tables_codeboxes(self.dad.curr_buffer)
         self.nodes_vectors[node_id].append([xml_content, pixbuf_table_codebox_vector, 0])
         self.nodes_indexes[node_id] = 0 # first state
         self.nodes_indicators[node_id] = 0 # the current buffer state is saved
      
   def text_variation(self, node_id, varied_text):
      """Insertion or Removal of text in the given node_id"""
      alphanum = re.search("\w", varied_text, re.UNICODE) # we search for an alphanumeric character
      if self.nodes_indicators[node_id] < 2:
         if alphanum != None: self.nodes_indicators[node_id] = 2 # alphanumeric transition
         else: self.nodes_indicators[node_id] = 1 # non alphanumeric transition
      elif alphanum == None: # self.nodes_indicators[node_id] == 2 and non alphanumeric transition
         self.update_state(node_id)
      
   def requested_previous_state(self, node_id):
      """A Previous State, if Existing, is Requested"""
      if self.nodes_indicators[node_id] == 0:
         if self.nodes_indexes[node_id] == 0: return None
         else:
            self.nodes_indexes[node_id] -= 1
            return self.nodes_vectors[node_id][self.nodes_indexes[node_id]]
      else:
         self.update_state(node_id)
         self.nodes_indexes[node_id] -= 1
         return self.nodes_vectors[node_id][self.nodes_indexes[node_id]]
      
   def requested_current_state(self, node_id):
      """The current state is requested"""
      return self.nodes_vectors[node_id][self.nodes_indexes[node_id]]
      
   def requested_subsequent_state(self, node_id):
      """A Subsequent State, if Existing, is Requested"""
      if self.nodes_indicators[node_id] == 0:
         if self.nodes_indexes[node_id] == len(self.nodes_vectors[node_id]) - 1: return None
         else:
            self.nodes_indexes[node_id] += 1
            return self.nodes_vectors[node_id][self.nodes_indexes[node_id]]
      else: return None
      
   def update_state(self, node_id):
      """Update the state for the given node_id"""
      curr_index = self.nodes_indexes[node_id]
      last_index = len(self.nodes_vectors[node_id]) - 1
      if curr_index != last_index: del self.nodes_vectors[node_id][curr_index+1:last_index+1]
      xml_content = self.dad.xml_handler.treestore_node_to_dom(self.dad.curr_tree_iter)
      pixbuf_table_codebox_vector = self.get_embedded_pixbufs_tables_codeboxes(self.dad.curr_buffer)
      cursor_pos = self.dad.curr_buffer.get_property('cursor-position')
      self.nodes_vectors[node_id].append([xml_content, pixbuf_table_codebox_vector, cursor_pos])
      num_saved_states = len(self.nodes_vectors[node_id])
      if num_saved_states == 2: self.nodes_vectors[node_id][0][2] = cursor_pos
      while num_saved_states > cons.MAX_NODES_STATES_NUM:
         self.nodes_vectors[node_id].pop(0)
         num_saved_states -= 1
      self.nodes_indexes[node_id] = num_saved_states - 1
      self.nodes_indicators[node_id] = 0 # the current buffer state is saved
