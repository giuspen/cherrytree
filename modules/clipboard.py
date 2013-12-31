# -*- coding: UTF-8 -*-
#
#       clipboard.py
#
#       Copyright 2009-2014 Giuseppe Penone <giuspen@gmail.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
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

import gtk, os, xml.dom.minidom, re, base64, mimetypes
import cons, machines, exports, imports


TARGET_CTD_PLAIN_TEXT = 'UTF8_STRING'
TARGET_CTD_RICH_TEXT = 'CTD_RICH'
TARGET_CTD_TABLE = 'CTD_TABLE'
TARGET_CTD_CODEBOX = 'CTD_CODEBOX'
TARGETS_HTML = ('text/html', 'HTML Format')
TARGET_URI_LIST = 'text/uri-list'
TARGETS_PLAIN_TEXT = ("UTF8_STRING", "COMPOUND_TEXT", "STRING", "TEXT")
TARGETS_IMAGES = ('image/png', 'image/jpeg', 'image/bmp', 'image/tiff', 'image/x-MS-bmp', 'image/x-bmp')
TARGET_WINDOWS_FILE_NAME = 'FileName'


class ClipboardHandler:
    """Handler of Clipboard"""

    def __init__(self, dad):
        """Clipboard Handler boot"""
        self.dad = dad
        self.clipboard = gtk.clipboard_get()
        self.force_plain_text = False

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
        if not self.dad.treestore[self.dad.curr_tree_iter][7]:
            self.dad.curr_buffer.delete_selection(True, sourceview.get_editable())
            self.dad.sourceview.grab_focus()

    def table_row_to_clipboard(self, table_dict):
        """Put the Selected Table Row to the Clipboard"""
        html_text = self.dad.html_handler.table_export_to_html(table_dict)
        self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_TABLE, TARGETS_HTML[0])],
                                     self.get_func,
                                     self.clear_func,
                                     (table_dict, None, html_text))

    def table_row_paste(self, model_n_iter):
        """Paste Table Row from the Clipboard"""
        targets = self.clipboard.wait_for_targets()
        if not targets: return False
        if TARGET_CTD_TABLE in targets:
            self.clipboard.request_contents(TARGET_CTD_TABLE, self.to_table, model_n_iter)
            return True
        return False

    def selection_to_clipboard(self, text_buffer, sourceview):
        """Write the Selected Content to the Clipboard"""
        iter_sel_start, iter_sel_end = text_buffer.get_selection_bounds()
        num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset()
        if num_chars == 1:
            anchor = iter_sel_start.get_child_anchor()
            if anchor:
                anchor_dir = dir(anchor)
                if "pixbuf" in anchor_dir:
                    self.clipboard.set_with_data([(t, 0, 0) for t in TARGETS_IMAGES],
                                                 self.get_func,
                                                 self.clear_func,
                                                 anchor.pixbuf)
                    return
                elif "liststore" in anchor_dir:
                    table_dict = self.dad.state_machine.table_to_dict(anchor)
                    html_text = self.dad.html_handler.table_export_to_html(table_dict)
                    self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_TABLE, TARGETS_HTML[0])],
                                                 self.get_func,
                                                 self.clear_func,
                                                 (table_dict, None, html_text))
                    return
                elif "sourcebuffer" in anchor_dir:
                    codebox_dict = self.dad.state_machine.codebox_to_dict(anchor, for_print=0)
                    codebox_dict_html = self.dad.state_machine.codebox_to_dict(anchor, for_print=2)
                    html_text = self.dad.html_handler.codebox_export_to_html(codebox_dict_html)
                    self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_CODEBOX, TARGETS_HTML[0])],
                                                 self.get_func,
                                                 self.clear_func,
                                                 (codebox_dict, None, html_text))
                    return
        if not os.path.isdir(cons.TMP_FOLDER): os.mkdir(cons.TMP_FOLDER)
        html_text = self.dad.html_handler.selection_export_to_html(text_buffer, iter_sel_start, iter_sel_end, self.dad.syntax_highlighting)
        if self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            plain_text = text_buffer.get_text(iter_sel_start, iter_sel_end)
            rich_text = self.rich_text_get_from_text_buffer_selection(text_buffer, iter_sel_start, iter_sel_end)
            self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_PLAIN_TEXT, TARGET_CTD_RICH_TEXT, TARGETS_HTML[0])],
                                         self.get_func,
                                         self.clear_func,
                                         (plain_text, rich_text, html_text))
        else:
            plain_text = text_buffer.get_text(iter_sel_start, iter_sel_end)
            self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_PLAIN_TEXT, TARGETS_HTML[0])],
                                         self.get_func,
                                         self.clear_func,
                                         (plain_text, None, html_text))

    def get_func(self, clipboard, selectiondata, info, data):
        """Connected with clipboard.set_with_data"""
        target = selectiondata.get_target()
        if target == TARGET_CTD_PLAIN_TEXT: selectiondata.set('UTF8_STRING', 8, data[0])
        elif target == TARGET_CTD_RICH_TEXT: selectiondata.set('UTF8_STRING', 8, data[1])
        elif target == TARGETS_HTML[0]: selectiondata.set('UTF8_STRING', 8, data[2])
        elif target == TARGET_CTD_CODEBOX:
            dom = xml.dom.minidom.Document()
            self.dad.xml_handler.codebox_element_to_xml([0, data[0], cons.TAG_PROP_LEFT], dom)
            selectiondata.set('UTF8_STRING', 8, dom.toxml())
        elif target == TARGET_CTD_TABLE:
            dom = xml.dom.minidom.Document()
            self.dad.xml_handler.table_element_to_xml([0, data[0], cons.TAG_PROP_LEFT], dom)
            selectiondata.set('UTF8_STRING', 8, dom.toxml())
        elif target in TARGETS_IMAGES: selectiondata.set_pixbuf(data)

    def clear_func(self, clipboard, data):
        """Connected with clipboard.set_with_data"""
        # this is to free memory allocated when filling the clipboard
        del data

    def paste(self, sourceview):
        """Paste from Clipboard"""
        sourceview.stop_emission("paste-clipboard")
        if self.dad.treestore[self.dad.curr_tree_iter][7]: return
        targets = self.clipboard.wait_for_targets()
        if not targets: return
        self.dad.curr_buffer.delete_selection(True, sourceview.get_editable())
        if self.force_plain_text:
            self.force_plain_text = False
            for target in TARGETS_PLAIN_TEXT:
                if target in targets:
                    self.clipboard.request_contents(target, self.to_plain_text)
                    return
        #print targets
        if TARGET_CTD_RICH_TEXT in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.clipboard.request_contents(TARGET_CTD_RICH_TEXT, self.to_rich_text)
            return
        if TARGET_CTD_CODEBOX in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.clipboard.request_contents(TARGET_CTD_CODEBOX, self.to_codebox)
            return
        if TARGET_CTD_TABLE in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.clipboard.request_contents(TARGET_CTD_TABLE, self.to_table, None)
            return
        for target in TARGETS_HTML:
            if target in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
                self.clipboard.request_contents(target, self.to_html)
                return
        if TARGET_URI_LIST in targets:
            self.clipboard.request_contents(TARGET_URI_LIST, self.to_uri_list)
            return
        for target in TARGETS_IMAGES:
            if target in targets and self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
                self.clipboard.request_contents(target, self.to_image)
                return
        for target in TARGETS_PLAIN_TEXT:
            if target in targets:
                self.clipboard.request_contents(target, self.to_plain_text)
                return
        if TARGET_WINDOWS_FILE_NAME in targets:
            self.clipboard.request_contents(TARGET_WINDOWS_FILE_NAME, self.to_uri_list)
            return
        print "WARNING: targets not handled", targets

    def to_uri_list(self, clipboard, selectiondata, data):
        """From Clipboard to URI list"""
        selection_data = re.sub(cons.BAD_CHARS, "", selectiondata.data)
        if self.dad.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
            self.dad.curr_buffer.insert(iter_insert, selection_data)
            return
        uri_list = selection_data.split(cons.CHAR_NEWLINE)
        for element in uri_list:
            if len(element) > 7:
                iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
                if (element[0:4] == "http" or element[0:3] == "ftp"): property_value = "webs " + element
                elif element[0:7] == "file://":
                    file_path = element[7:].replace("%20", cons.CHAR_SPACE)
                    mimetype = mimetypes.guess_type(file_path)[0]
                    if mimetype and len(mimetype) > 5 and mimetype[0:6] == "image/" and os.path.isfile(file_path):
                        self.dad.image_insert(iter_insert, gtk.gdk.pixbuf_new_from_file(file_path))
                        iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
                        self.dad.curr_buffer.insert(iter_insert, 3*cons.CHAR_SPACE)
                        continue
                    else:
                        if os.path.isdir(file_path):
                            property_value = "fold %s" % base64.b64encode(file_path)
                        elif os.path.isfile(file_path):
                            property_value = "file %s" % base64.b64encode(file_path)
                        else:
                            property_value = None
                            print "ERROR: discarded file uri '%s'" % file_path
                else:
                    if os.path.isdir(element):
                        property_value = "fold %s" % base64.b64encode(element)
                    elif os.path.isfile(element):
                        property_value = "file %s" % base64.b64encode(element)
                    else:
                        property_value = None
                        print "ERROR: discarded ? uri '%s'" % element
                start_offset = iter_insert.get_offset()
                self.dad.curr_buffer.insert(iter_insert, element + cons.CHAR_NEWLINE)
                if property_value:
                    iter_sel_start = self.dad.curr_buffer.get_iter_at_offset(start_offset)
                    iter_sel_end = self.dad.curr_buffer.get_iter_at_offset(start_offset + len(element))
                    self.dad.curr_buffer.apply_tag_by_name(self.dad.apply_tag_exist_or_create("link", property_value),
                                                           iter_sel_start, iter_sel_end)

    def to_html(self, clipboard, selectiondata, data):
        """From Clipboard to HTML Text"""
        if not cons.IS_WIN_OS and ord(selectiondata.data[0]) == 0xff and ord(selectiondata.data[1]) in [0xfe, 0xff]:
            selection_data = selectiondata.data.decode(cons.STR_UTF16, cons.STR_IGNORE)
        else:
            match = re.match('.*\x00\w\x00\w\x00\w.*', selectiondata.data, re.UNICODE) # \w is alphanumeric char
            if match: selection_data = selectiondata.data.decode(cons.STR_UTF16, cons.STR_IGNORE)
            elif "HTML" in selectiondata.data: selection_data = selectiondata.data.decode(cons.STR_UTF8, cons.STR_IGNORE)
            else:
                utf8_OK = False
                utf16_OK = False
                selection_data = ""
                try:
                    selection_data = selectiondata.data.decode(cons.STR_UTF16)
                    utf16_OK = True
                except: pass
                try:
                    selection_data = selectiondata.data.decode(cons.STR_UTF8)
                    utf8_OK = True
                except: pass
                if utf16_OK == utf8_OK:
                    if cons.IS_WIN_OS: selection_data = selectiondata.data.decode(cons.STR_UTF16, cons.STR_IGNORE)
                    else: selection_data = selectiondata.data
        #print "###########################"
        #print selectiondata.data
        #print "###########################"
        #print selectiondata.data.decode(cons.STR_UTF16, cons.STR_IGNORE)
        #print "###########################"
        #for char in selection_data: print ord(char)
        selection_data = re.sub(cons.BAD_CHARS, "", selection_data)
        html_import = imports.HTMLFromClipboardHandler(self.dad)
        xml_string = html_import.get_clipboard_selection_xml(selection_data)
        self.from_xml_string_to_buffer(xml_string)

    def to_plain_text(self, clipboard, selectiondata, data):
        """From Clipboard to Plain Text"""
        plain_text = selectiondata.get_text()
        if not plain_text:
            print "? no clipboard plain text"
            return
        iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
        start_offset = iter_insert.get_offset()
        self.dad.curr_buffer.insert(iter_insert, plain_text)
        if self.dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            web_links_offsets = imports.get_web_links_offsets_from_plain_text(plain_text)
            if web_links_offsets:
                for offsets in web_links_offsets:
                    iter_sel_start = self.dad.curr_buffer.get_iter_at_offset(start_offset + offsets[0])
                    iter_sel_end = self.dad.curr_buffer.get_iter_at_offset(start_offset + offsets[1])
                    link_url = plain_text[offsets[0]:offsets[1]]
                    if link_url[0:3] not in ["htt", "ftp"]: link_url = "http://" + link_url
                    property_value = "webs " + link_url
                    self.dad.curr_buffer.apply_tag_by_name(self.dad.apply_tag_exist_or_create("link", property_value),
                                                           iter_sel_start, iter_sel_end)
            else:
                # check for file or folder path
                if not cons.CHAR_NEWLINE in plain_text:
                    property_value = None
                    if os.path.isdir(plain_text):
                        property_value = "fold %s" % base64.b64encode(plain_text)
                    elif os.path.isfile(plain_text):
                        property_value = "file %s" % base64.b64encode(plain_text)
                    if property_value:
                        iter_sel_end = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
                        iter_sel_start = iter_sel_end.copy()
                        iter_sel_start.backward_chars(len(plain_text))
                        self.dad.curr_buffer.apply_tag_by_name(self.dad.apply_tag_exist_or_create("link", property_value),
                                                               iter_sel_start, iter_sel_end)

    def to_rich_text(self, clipboard, selectiondata, data):
        """From Clipboard to Rich Text"""
        rich_text = selectiondata.get_text()
        if not rich_text:
            print "? no clipboard rich text"
            return
        self.from_xml_string_to_buffer(rich_text)

    def from_xml_string_to_buffer(self, xml_string):
        """From XML String to Text Buffer"""
        dom = xml.dom.minidom.parseString(xml_string)
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
                        self.dom_node_to_table(nephew_dom_iter, None)
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
        else: justification = cons.TAG_PROP_LEFT
        if dom_node.hasAttribute("anchor"):
            pixbuf = gtk.gdk.pixbuf_new_from_file(cons.ANCHOR_CHAR)
            pixbuf = pixbuf.scale_simple(self.dad.anchor_size, self.dad.anchor_size, gtk.gdk.INTERP_BILINEAR)
            pixbuf.anchor = dom_node.attributes["anchor"].value
        else: pixbuf = machines.get_pixbuf_from_encoded_buffer(dom_node.firstChild.data)
        if pixbuf:
            self.dad.image_insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                  pixbuf,
                                  image_justification=justification,
                                  text_buffer=self.dad.curr_buffer)

    def to_codebox(self, clipboard, selectiondata, data):
        """From Clipboard to CodeBox"""
        xml_text = selectiondata.get_text()
        if not xml_text:
            print "? no clipboard xml text"
            return
        dom = xml.dom.minidom.parseString(xml_text)
        dom_node = dom.firstChild
        if dom_node.nodeName != "codebox":
            print "codebox from clipboard error"
            return
        self.dom_node_to_codebox(dom_node)

    def dom_node_to_codebox(self, dom_node):
        """From dom_node to CodeBox"""
        justification = dom_node.attributes["justification"].value if dom_node.hasAttribute("justification") else cons.TAG_PROP_LEFT
        codebox_dict = {
           'frame_width': int(dom_node.attributes['frame_width'].value),
           'frame_height': int(dom_node.attributes['frame_height'].value),
           'width_in_pixels': dom_node.hasAttribute("width_in_pixels") and dom_node.attributes['width_in_pixels'].value == "True",
           'syntax_highlighting': dom_node.attributes['syntax_highlighting'].value,
           'highlight_brackets': dom_node.hasAttribute("highlight_brackets") and dom_node.attributes['highlight_brackets'].value == "True",
           'show_line_numbers': dom_node.hasAttribute("show_line_numbers") and dom_node.attributes['show_line_numbers'].value == "True",
           'fill_text': dom_node.firstChild.data if dom_node.firstChild else ""
        }
        self.dad.codeboxes_handler.codebox_insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                                  codebox_dict,
                                                  codebox_justification=justification,
                                                  text_buffer=self.dad.curr_buffer)

    def to_table(self, clipboard, selectiondata, table_model_n_iter):
        """From Clipboard to Table"""
        xml_text = selectiondata.get_text()
        if not xml_text:
            print "? no clipboard xml text"
            return
        dom = xml.dom.minidom.parseString(xml_text)
        dom_node = dom.firstChild
        if dom_node.nodeName != "table":
            print "table from clipboard error"
            return
        self.dom_node_to_table(dom_node, table_model_n_iter)

    def dom_node_to_table(self, dom_node, table_model_n_iter):
        """From dom_node to Table"""
        if dom_node.hasAttribute("justification"): justification = dom_node.attributes["justification"].value
        else: justification = cons.TAG_PROP_LEFT
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
        if not table_model_n_iter:
            # insert new table
            self.dad.tables_handler.table_insert(self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()),
                                                 table,
                                                 table_justification=justification,
                                                 text_buffer=self.dad.curr_buffer)
        else:
            # paste into existing table
            (model, iter) = table_model_n_iter
            num_columns = model.get_n_columns()
            for i, table_row in enumerate(table['matrix']):
                if i < len(table['matrix']) - 1:
                    if len(table_row) > num_columns: table_row = table_row[:num_columns]
                    elif len(table_row) < num_columns: table_row = table_row + [""]*(num_columns - len(table_row))
                    iter = model.insert_after(iter, table_row)

    def rich_text_get_from_text_buffer_selection(self, text_buffer, iter_sel_start, iter_sel_end,
                                                 change_case="n", exclude_iter_sel_end=False):
        """Given text_buffer and selection, returns the rich text xml"""
        iter_sel_start_offset = iter_sel_start.get_offset()
        iter_sel_end_offset = iter_sel_end.get_offset()
        if exclude_iter_sel_end: iter_sel_end_offset -= 1
        iter_sel_range = (iter_sel_start_offset, iter_sel_end_offset)
        pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(text_buffer,
                                                                                                   for_print=0,
                                                                                                   sel_range=iter_sel_range)
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
            self.rich_text_process_slot(dom, root, start_offset, end_offset, text_buffer, obj_element, change_case)
            obj_pos += 1
            start_offset = end_offset
        self.rich_text_process_slot(dom, root, start_offset, iter_sel_end.get_offset(), text_buffer, None, change_case)
        return dom.toxml()

    def rich_text_process_slot(self, dom, root, start_offset, end_offset, text_buffer, obj_element, change_case="n"):
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
            if curr_iter.get_offset() > end_offset: curr_iter = text_buffer.get_iter_at_offset(end_offset)
            self.dad.xml_handler.rich_txt_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes,
                                                    change_case=change_case, dom=dom)
            offset_old = curr_iter.get_offset()
            if offset_old >= end_offset: break
            else:
                self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
                start_iter.set_offset(offset_old)
                tag_found = curr_iter.forward_to_tag_toggle(None)
                if curr_iter.get_offset() == offset_old: break
        else:
            if curr_iter.get_offset() > end_offset: curr_iter = text_buffer.get_iter_at_offset(end_offset)
            self.dad.xml_handler.rich_txt_serialize(dom_iter, start_iter, curr_iter, self.curr_attributes,
                                                    change_case=change_case, dom=dom)
        if obj_element:
            if obj_element[0] == "pixbuf": self.dad.xml_handler.pixbuf_element_to_xml(obj_element[1], dom_iter, dom)
            elif obj_element[0] == "table": self.dad.xml_handler.table_element_to_xml(obj_element[1], dom_iter)
            elif obj_element[0] == "codebox": self.dad.xml_handler.codebox_element_to_xml(obj_element[1], dom_iter)
