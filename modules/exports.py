# -*- coding: UTF-8 -*-
#
#       exports.py
#
#       Copyright 2009-2019 Giuseppe Penone <giuspen@gmail.com>
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

import os
import cgi
import base64
import shutil
import copy
import pango
import cons
import support


def rgb_str_from_int24bit(int24bit):
    r = (int24bit >> 16) & 0xff
    g = (int24bit >> 8) & 0xff
    b = int24bit & 0xff
    return "#%.2x%.2x%.2x" % (r, g, b)

def rgb_int24bit_from_str(rgb_str):
    rgb_24 = rgb_any_to_24(rgb_str)
    r = int(rgb_24[:2], 16)
    g = int(rgb_24[2:4], 16)
    b = int(rgb_24[4:], 16)
    return (r << 16 | g << 8 | b)

def rgb_any_to_24(rgb_in):
    """Convert any RGB to RRGGBB if needed"""
    if len(rgb_in) == 12:
        r = int(rgb_in[:4], 16)
        g = int(rgb_in[4:8], 16)
        b = int(rgb_in[8:], 16)
        r >>= 8
        g >>= 8
        b >>= 8
        return "%.2x%.2x%.2x" % (r, g, b)
    if len(rgb_in) == 6: return rgb_in
    if len(rgb_in) == 3: return 2*rgb_in[0]+2*rgb_in[1]+2*rgb_in[2]
    print "! rgb_any_to_24(%s)" % rgb_in
    return rgb_in

def rgb_get_is_blackish_or_whiteish(in_rgb):
    rgb_24 = rgb_any_to_24(in_rgb if not in_rgb.startswith("#") else in_rgb[1:])
    r = int(rgb_24[:2], 16)
    g = int(rgb_24[2:4], 16)
    b = int(rgb_24[4:], 16)
    # r+g+b black is 0
    # r+g+b white is 3*255 = 765
    sum_r_g_b = r+g+b
    if sum_r_g_b < 150:
        #print "black-ish", sum_r_g_b
        return True
    if sum_r_g_b > 700:
        #print "white-ish", sum_r_g_b
        return True
    return False

def rgb_24_get_is_dark(in_rgb_24):
    r = int(in_rgb_24[:2], 16)
    g = int(in_rgb_24[2:4], 16)
    b = int(in_rgb_24[4:], 16)
    # r+g+b black is 0
    # r+g+b white is 3*255 = 765
    max_24 = 255
    return (r+g+b < max_24)

def rgb_to_no_white(in_rgb):
    if len(in_rgb) == 12:
        r = int(in_rgb[:4], 16)
        g = int(in_rgb[4:8], 16)
        b = int(in_rgb[8:], 16)
        # r+g+b black is 0
        # r+g+b white is 3*65535
        max_48 = 65535
        if r+g+b > 2.2*max_48:
            r = max_48 - r
            g = max_48 - g
            b = max_48 - b
            out_rgb = "%.4x%.4x%.4x" % (r, g, b)
            #print "%s to %s" % (in_rgb, out_rgb)
        else:
            out_rgb = in_rgb
    else:
        if len(in_rgb) != 6:
            in_rgb = rgb_any_to_24(in_rgb)
        r = int(in_rgb[:2], 16)
        g = int(in_rgb[2:4], 16)
        b = int(in_rgb[4:], 16)
        # r+g+b black is 0
        # r+g+b white is 3*255
        max_24 = 255
        if r+g+b > 2.2*max_24:
            r = max_24 - r
            g = max_24 - g
            b = max_24 - b
            out_rgb = "%.2x%.2x%.2x" % (r, g, b)
            #print "%s to %s" % (in_rgb, out_rgb)
        else:
            out_rgb = in_rgb
    return out_rgb

def clean_text_to_utf8(in_text):
    """Clean from utf8 decode errors"""
    clean_text = in_text
    while clean_text:
        try:
            clean_text = unicode(clean_text, cons.STR_UTF8, cons.STR_IGNORE)
            break
        except: clean_text = clean_text[1:]
    return clean_text

def prepare_export_folder(dir_place, new_folder, overwrite_existing):
    if os.path.exists(os.path.join(dir_place, new_folder)):
        if overwrite_existing:
            shutil.rmtree(os.path.join(dir_place, new_folder))
        else:
            n = 2
            while os.path.exists(os.path.join(dir_place, new_folder + '%03d' % n)):
                n += 1
            new_folder += '%03d' % n
    return new_folder


class Export2CTD:
    """The Export to CTD Class"""

    def __init__(self, dad):
        """Export to Txt boot"""
        self.dad = dad

    def get_single_ct_filepath(self, filename_hint):
        """Prepare for the CTD file save"""
        filepath = support.dialog_file_save_as(filename_hint,
                                               filter_pattern="*.ct*",
                                               filter_name=_("CherryTree Document"),
                                               curr_folder=self.dad.file_dir,
                                               parent=self.dad.window)
        if filepath: filepath = self.dad.filepath_extension_fix(filepath)
        return filepath

    def nodes_all_export_to_ctd(self, filepath):
        """Export All Nodes To CTD"""
        if self.dad.filetype in ["d", "z"]:
            try: xml_string = self.dad.xml_handler.treestore_to_dom()
            except:
                support.dialog_error("%s write failed - all nodes to xml" % filepath, self.dad.window)
                raise
                return
        else: xml_string = ""
        try: self.dad.file_write_low_level(filepath, xml_string, first_write=True, exporting="a")
        except:
            support.dialog_error("%s write failed - writing to disk" % filepath, self.dad.window)
            raise

    def node_and_subnodes_export_to_ctd(self, tree_iter, filepath):
        """Export Node and Subnodes To CTD"""
        if self.dad.filetype in ["d", "z"]:
            try: xml_string = self.dad.xml_handler.treestore_sel_node_and_subnodes_to_dom(tree_iter)
            except:
                support.dialog_error("%s write failed - sel node and subnodes to xml" % filepath, self.dad.window)
                raise
                return
        else: xml_string = ""
        try: self.dad.file_write_low_level(filepath, xml_string, first_write=True, exporting="s")
        except:
            support.dialog_error("%s write failed - writing to disk" % filepath, self.dad.window)
            raise

    def node_export_to_ctd(self, tree_iter, filepath, sel_range=None):
        """Export the Selected Node To CTD"""
        if self.dad.filetype in ["d", "z"]:
            try: xml_string = self.dad.xml_handler.treestore_sel_node_only_to_dom(tree_iter, sel_range=sel_range)
            except:
                support.dialog_error("%s write failed - sel node to xml" % filepath, self.dad.window)
                raise
                return
        else: xml_string = ""
        try: self.dad.file_write_low_level(filepath, xml_string, first_write=True, exporting="n", sel_range=sel_range)
        except:
            support.dialog_error("%s write failed - writing to disk" % filepath, self.dad.window)
            raise


class ExportPrint:
    """The Export Print Class"""

    def __init__(self, dad):
        """Export Print boot"""
        self.dad = dad
        self.pango_handler = Export2Pango(dad)

    def get_pdf_filepath(self, proposed_name):
        """Dialog to select dest PDF"""
        ret_filepath = support.dialog_file_save_as(proposed_name + ".pdf",
            filter_pattern="*.pdf",
            filter_name=_("PDF File"),
            curr_folder=self.dad.pick_dir_export,
            parent=self.dad.window)
        if ret_filepath:
            if not ret_filepath.endswith(".pdf"): ret_filepath += ".pdf"
            self.dad.pick_dir_export = os.path.dirname(ret_filepath)
        return ret_filepath

    def nodes_all_export_print(self, top_tree_iter, include_node_name, new_node_in_new_page):
        """Export Print All Nodes"""
        self.pango_text = []
        self.pixbuf_table_codebox_vector = []
        self.text_font = self.dad.code_font
        if not top_tree_iter: tree_iter = self.dad.treestore.get_iter_first()
        else: tree_iter = top_tree_iter.copy()
        while tree_iter:
            self.nodes_all_export_print_iter(tree_iter, include_node_name, new_node_in_new_page)
            if top_tree_iter: break
            tree_iter = self.dad.treestore.iter_next(tree_iter)
        self.dad.objects_buffer_refresh()
        self.run_print()

    def nodes_all_export_print_iter(self, tree_iter, include_node_name, new_node_in_new_page):
        """Export Print All Nodes - Iter"""
        self.dad.get_textbuffer_from_tree_iter(tree_iter)
        if self.dad.treestore[tree_iter][4] == cons.RICH_TEXT_ID:
            pango_text, pixbuf_table_codebox_vector = self.pango_handler.pango_get_from_treestore_node(tree_iter)
            self.text_font = self.dad.rt_font # text font for all (also eventual code nodes)
        else:
            pango_text = [self.pango_handler.pango_get_from_code_buffer(self.dad.treestore[tree_iter][2])]
            pixbuf_table_codebox_vector = []
        if include_node_name: self.pango_text_add_node_name(tree_iter, pango_text)
        if not self.pango_text: self.pango_text = pango_text
        else:
            if new_node_in_new_page:
                pango_text[0] = 2*cons.CHAR_NEWPAGE + pango_text[0]
                self.pango_text += pango_text
                pixbuf_table_codebox_vector.insert(0, ["", [0, None, ""]])
            else:
                self.pango_text[-1] += cons.CHAR_NEWLINE*3 + pango_text[0]
                if len(pango_text) > 1: self.pango_text += pango_text[1:]
        self.pixbuf_table_codebox_vector += pixbuf_table_codebox_vector
        child_tree_iter = self.dad.treestore.iter_children(tree_iter)
        while child_tree_iter:
            self.nodes_all_export_print_iter(child_tree_iter, include_node_name, new_node_in_new_page)
            child_tree_iter = self.dad.treestore.iter_next(child_tree_iter)

    def pango_text_add_node_name(self, tree_iter, pango_text):
        """Add Node Name to Pango Text Vector"""
        pango_text[0] = "<b><i><span size=\"xx-large\">" \
                      + cgi.escape(self.dad.treestore[tree_iter][1]) \
                      + "</span></i></b>" + 2*cons.CHAR_NEWLINE + pango_text[0]

    def node_export_print(self, tree_iter, include_node_name, sel_range=None):
        """Export Print the Selected Node"""
        if self.dad.treestore[tree_iter][4] == cons.RICH_TEXT_ID:
            self.pango_text, self.pixbuf_table_codebox_vector = self.pango_handler.pango_get_from_treestore_node(tree_iter, sel_range)
            self.text_font = self.dad.rt_font
        else:
            self.pango_text = [self.pango_handler.pango_get_from_code_buffer(self.dad.treestore[tree_iter][2], sel_range)]
            self.pixbuf_table_codebox_vector = []
            self.text_font = self.dad.code_font if self.dad.treestore[tree_iter][4] != cons.PLAIN_TEXT_ID else self.dad.pt_font
        if include_node_name: self.pango_text_add_node_name(tree_iter, self.pango_text)
        self.run_print()

    def run_print(self):
        """Finally Run the Print"""
        self.dad.print_handler.print_text(self.dad.window,
            self.pango_text,
            self.text_font,
            self.dad.code_font,
            self.pixbuf_table_codebox_vector,
            self.dad.get_text_window_width())


class Export2Txt:
    """The Export to Txt Class"""

    def __init__(self, dad):
        """Export to Txt boot"""
        self.dad = dad

    def get_single_txt_filepath(self, proposed_name):
        """Prepare for the txt file save"""
        ret_filepath = support.dialog_file_save_as(proposed_name + ".txt",
                                                   filter_pattern="*.txt",
                                                   filter_name=_("Plain Text Document"),
                                                   curr_folder=self.dad.pick_dir_export,
                                                   parent=self.dad.window)
        if ret_filepath:
            if not ret_filepath.endswith(".txt"): ret_filepath += ".txt"
            self.dad.pick_dir_export = os.path.dirname(ret_filepath)
        return ret_filepath

    def prepare_txt_folder(self, new_folder, dir_place=""):
        """Prepare the website folder"""
        if not dir_place:
            dir_place = support.dialog_folder_select(curr_folder=self.dad.pick_dir_export, parent=self.dad.window)
            if dir_place == None: return False
        new_folder = support.clean_from_chars_not_for_filename(new_folder) + "_TXT"
        new_folder = prepare_export_folder(dir_place, new_folder, self.dad.export_overwrite)
        self.new_path = os.path.join(dir_place, new_folder)
        os.mkdir(self.new_path)
        return True

    def nodes_all_export_to_txt(self, top_tree_iter=None, single_txt_filepath="", include_node_name=True):
        """Export All Nodes To Txt"""
        if not top_tree_iter: tree_iter = self.dad.treestore.get_iter_first()
        else: tree_iter = top_tree_iter.copy()
        while tree_iter:
            self.nodes_all_export_to_txt_iter(tree_iter, single_txt_filepath, include_node_name)
            if top_tree_iter: break
            tree_iter = self.dad.treestore.iter_next(tree_iter)
        self.dad.objects_buffer_refresh()

    def nodes_all_export_to_txt_iter(self, tree_iter, single_txt_filepath, include_node_name):
        """Export All Nodes To Txt - iter"""
        text_buffer = self.dad.get_textbuffer_from_tree_iter(tree_iter)
        tree_iter_for_node_name = tree_iter if include_node_name else None
        if not single_txt_filepath:
            filepath = os.path.join(self.new_path,
                                    support.get_node_hierarchical_name(self.dad, tree_iter, trailer=".txt"))
            self.node_export_to_txt(text_buffer, filepath, tree_iter_for_node_name=tree_iter_for_node_name)
        else:
            self.node_export_to_txt(text_buffer, single_txt_filepath, tree_iter_for_node_name=tree_iter_for_node_name)
        child_tree_iter = self.dad.treestore.iter_children(tree_iter)
        while child_tree_iter:
            self.nodes_all_export_to_txt_iter(child_tree_iter, single_txt_filepath, include_node_name)
            child_tree_iter = self.dad.treestore.iter_next(child_tree_iter)

    def tag_link_in_given_iter(self, given_iter):
        """Check for tag link in given_iter"""
        for tag in given_iter.get_tags():
            tag_name = tag.get_property("name")
            if tag_name.startswith("link_"): return tag_name[5:]
        return ""

    def plain_process_slot(self, start_offset, end_offset, curr_buffer, check_link_target):
        """Process a Single plain Slot"""
        if end_offset == -1:
            end_offset = curr_buffer.get_end_iter().get_offset()
        #print "process slot (%s->%s)" % (start_offset, end_offset)
        # begin operations
        start_iter = curr_buffer.get_iter_at_offset(start_offset)
        end_iter = curr_buffer.get_iter_at_offset(end_offset)
        if not check_link_target:
            self.curr_plain_slots.append(curr_buffer.get_text(start_iter, end_iter))
            return
        start_link = self.tag_link_in_given_iter(start_iter)
        middle_link = self.tag_link_in_given_iter(curr_buffer.get_iter_at_offset((start_offset+end_offset)/2-1))
        end_link = self.tag_link_in_given_iter(curr_buffer.get_iter_at_offset(end_offset-1))
        if start_link and start_link == middle_link and middle_link == end_link and not start_link.startswith("node"):
            #print start_link
            plain_slot = self.dad.sourceview_hovering_link_get_tooltip(start_link)
        else: plain_slot = curr_buffer.get_text(start_iter, end_iter)
        self.curr_plain_slots.append(plain_slot)

    def get_codebox_plain(self, codebox):
        """Returns the plain CodeBox"""
        # codebox is: [offset, dict, justification]
        codebox_plain = cons.CHAR_NEWLINE + self.dad.h_rule + cons.CHAR_NEWLINE
        codebox_plain += codebox[1]['fill_text']
        codebox_plain += cons.CHAR_NEWLINE + self.dad.h_rule + cons.CHAR_NEWLINE
        return codebox_plain

    def get_table_plain(self, table_orig):
        """Returns the plain Table"""
        # table is: [offset, dict, justification]
        table_plain = cons.CHAR_NEWLINE
        table = copy.deepcopy(table_orig)
        table[1]['matrix'].insert(0, table[1]['matrix'].pop())
        for j, row in enumerate(table[1]['matrix']):
            table_plain += cons.CHAR_PIPE
            for cell in row:
                table_plain += cons.CHAR_SPACE + cell + cons.CHAR_SPACE + cons.CHAR_PIPE
            table_plain += cons.CHAR_NEWLINE
        return table_plain

    def plain_get_from_treestore_node(self, curr_buffer, sel_range, check_link_target):
        """Given a treestore iter returns the plain text"""
        pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer, sel_range=sel_range)
        # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
        self.curr_plain_slots = []
        start_offset = 0 if not sel_range else sel_range[0]
        for end_offset_element in pixbuf_table_codebox_vector:
            end_offset = end_offset_element[1][0]
            self.plain_process_slot(start_offset, end_offset, curr_buffer, False)
            start_offset = end_offset
        if not sel_range: self.plain_process_slot(start_offset, -1, curr_buffer, check_link_target and not pixbuf_table_codebox_vector)
        else: self.plain_process_slot(start_offset, sel_range[1], curr_buffer, check_link_target and not pixbuf_table_codebox_vector)
        #print "curr_plain_slots", curr_plain_slots
        #print "pixbuf_table_codebox_vector", pixbuf_table_codebox_vector
        return [self.curr_plain_slots, pixbuf_table_codebox_vector]

    def node_export_to_txt(self, text_buffer, filepath, sel_range=None, tree_iter_for_node_name=None, check_link_target=False):
        """Export the Selected Node To Txt"""
        plain_text = ""
        text_n_objects = self.plain_get_from_treestore_node(text_buffer, sel_range, check_link_target)
        self.images_count = 0
        for i, plain_slot in enumerate(text_n_objects[0]):
            plain_text += plain_slot
            if i < len(text_n_objects[1]):
                curr_object = text_n_objects[1][i]
                if curr_object[0] == "table": plain_text += self.get_table_plain(curr_object[1])
                elif curr_object[0] == "codebox": plain_text += self.get_codebox_plain(curr_object[1])
        if tree_iter_for_node_name:
            node_name = clean_text_to_utf8(self.dad.treestore[tree_iter_for_node_name][1])
            plain_text = node_name.upper() + cons.CHAR_NEWLINE + plain_text
        if filepath:
            file_descriptor = open(filepath, 'a')
            file_descriptor.write(plain_text + 2*cons.CHAR_NEWLINE)
            file_descriptor.close()
        return plain_text


class Export2Pango:
    """The Export to Pango Class"""

    def __init__(self, dad):
        """Export to Pango boot"""
        self.dad = dad

    def pango_get_from_code_buffer(self, code_buffer, sel_range=None):
        """Get rich text from syntax highlighted code node"""
        if not sel_range:
            curr_iter = code_buffer.get_start_iter()
            end_iter = code_buffer.get_end_iter()
        else:
            curr_iter = code_buffer.get_iter_at_offset(sel_range[0])
            end_iter = code_buffer.get_iter_at_offset(sel_range[1])
        code_buffer.ensure_highlight(curr_iter, end_iter)
        pango_text = ""
        former_tag_str = cons.COLOR_48_BLACK
        span_opened = False
        while 1:
            curr_tags = curr_iter.get_tags()
            if len(curr_tags) > 0:
                curr_tag_str = curr_tags[0].get_property("foreground-gdk").to_string()
                font_weight = curr_tags[0].get_property(cons.TAG_WEIGHT)
                if curr_tag_str == cons.COLOR_48_BLACK:
                    if former_tag_str != curr_tag_str:
                        former_tag_str = curr_tag_str
                        # end of tag
                        pango_text += "</span>"
                        span_opened = False
                else:
                    if former_tag_str != curr_tag_str:
                        former_tag_str = curr_tag_str
                        if span_opened: pango_text += "</span>"
                        # start of tag
                        curr_tag_str = "#" + rgb_to_no_white(curr_tag_str[1:])
                        pango_text += '<span foreground="%s" font_weight="%s">' % (curr_tag_str, font_weight)
                        span_opened = True
            elif span_opened:
                span_opened = False
                former_tag_str = cons.COLOR_48_BLACK
                pango_text += "</span>"
            try: pango_text += cgi.escape(curr_iter.get_char())
            except: pass
            if not curr_iter.forward_char() or (sel_range and curr_iter.get_offset() > sel_range[1]):
                if span_opened: pango_text += "</span>"
                break
        if len(pango_text) == 0 or pango_text[-1] != cons.CHAR_NEWLINE:
            pango_text += cons.CHAR_NEWLINE
        return pango_text

    def pango_get_from_treestore_node(self, node_iter, sel_range=None):
        """Given a treestore iter returns the Pango rich text"""
        curr_buffer = self.dad.treestore[node_iter][2]
        pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer,
            for_print=1,
            sel_range=sel_range)
        # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
        self.curr_pango_slots = []
        start_offset = 0 if not sel_range else sel_range[0]
        for end_offset_element in pixbuf_table_codebox_vector:
            end_offset = end_offset_element[1][0]
            self.pango_process_slot(start_offset, end_offset, curr_buffer)
            start_offset = end_offset
        if not sel_range: self.pango_process_slot(start_offset, -1, curr_buffer)
        else: self.pango_process_slot(start_offset, sel_range[1], curr_buffer)
        #print "curr_pango_slots", self.curr_pango_slots
        #print "pixbuf_table_codebox_vector", pixbuf_table_codebox_vector
        # fix the problem of the latest char not being a new line char
        if len(self.curr_pango_slots) > 0 and\
         ( len(self.curr_pango_slots[-1]) == 0 or self.curr_pango_slots[-1][-1] != cons.CHAR_NEWLINE ):
            self.curr_pango_slots[-1] += cons.CHAR_NEWLINE
        return [self.curr_pango_slots, pixbuf_table_codebox_vector]

    def pango_process_slot(self, start_offset, end_offset, curr_buffer):
        """Process a Single Pango Slot"""
        self.curr_pango_text = ""
        start_iter = curr_buffer.get_iter_at_offset(start_offset)
        if end_offset == -1:
            end_offset = curr_buffer.get_end_iter().get_offset()
        #print "process slot (%s->%s)" % (start_offset, end_offset)
        # begin operations
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        curr_iter = start_iter.copy()
        self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
        tag_found = curr_iter.forward_to_tag_toggle(None)
        while tag_found:
            if curr_iter.get_offset() > end_offset:
                curr_iter = curr_buffer.get_iter_at_offset(end_offset)
            self.pango_text_serialize(start_iter, curr_iter)
            offset_old = curr_iter.get_offset()
            if offset_old >= end_offset: break
            else:
                self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
                start_iter.set_offset(offset_old)
                tag_found = curr_iter.forward_to_tag_toggle(None)
                if curr_iter.get_offset() == offset_old: break
        else:
            if curr_iter.get_offset() > end_offset:
                curr_iter = curr_buffer.get_iter_at_offset(end_offset)
            self.pango_text_serialize(start_iter, curr_iter)
        self.curr_pango_slots.append(self.curr_pango_text)

    def pango_text_serialize(self, start_iter, end_iter):
        """Adds a slice to the Pango Text"""
        pango_attrs = ''
        superscript_active = False
        subscript_active = False
        monospace_active = False
        for tag_property in cons.TAG_PROPERTIES:
            if tag_property not in [cons.TAG_JUSTIFICATION, cons.TAG_LINK] and self.curr_attributes[tag_property] != '':
                property_value = self.curr_attributes[tag_property]
                # tag names fix
                if tag_property == cons.TAG_SCALE:
                    if property_value == cons.TAG_PROP_SUP:
                        superscript_active = True
                        continue
                    elif property_value == cons.TAG_PROP_SUB:
                        subscript_active = True
                        continue
                    else: tag_property = "size"
                    # tag properties fix
                    if property_value == cons.TAG_PROP_SMALL: property_value = 'x-small'
                    elif property_value == cons.TAG_PROP_H1: property_value = 'xx-large'
                    elif property_value == cons.TAG_PROP_H2: property_value = 'x-large'
                    elif property_value == cons.TAG_PROP_H3: property_value = 'large'
                elif tag_property == cons.TAG_FAMILY:
                    monospace_active = True
                    continue
                elif tag_property == cons.TAG_FOREGROUND:
                    property_value = "#" + rgb_to_no_white(property_value[1:])
                pango_attrs += ' %s="%s"' % (tag_property, property_value)
        if pango_attrs == '': tagged_text = cgi.escape(start_iter.get_text(end_iter))
        else: tagged_text = '<span' + pango_attrs + '>' + cgi.escape(start_iter.get_text(end_iter)) + '</span>'
        if superscript_active: tagged_text = "<sup>" + tagged_text + "</sup>"
        if subscript_active: tagged_text = "<sub>" + tagged_text + "</sub>"
        if monospace_active: tagged_text = "<tt>" + tagged_text + "</tt>"
        self.curr_pango_text += tagged_text


class Export2Html:
    """The Export to HTML Class"""

    def __init__(self, dad):
        """Export to HTML boot"""
        self.dad = dad
        self.tree_links_text = ""

    def prepare_html_folder(self, new_folder, dir_place=""):
        """Prepare the website folder"""
        if not dir_place:
            dir_place = support.dialog_folder_select(curr_folder=self.dad.pick_dir_export, parent=self.dad.window)
            if dir_place == None: return False
        new_folder = support.clean_from_chars_not_for_filename(new_folder) + "_HTML"
        new_folder = prepare_export_folder(dir_place, new_folder, self.dad.export_overwrite)
        self.new_path = os.path.join(dir_place, new_folder)
        self.images_dir = os.path.join(self.new_path, "images")
        self.embed_dir = os.path.join(self.new_path, "EmbeddedFiles")
        os.mkdir(self.new_path)
        os.mkdir(self.images_dir)
        os.mkdir(self.embed_dir)
        styles_css_filepath = os.path.join(cons.CONFIG_DIR, "styles.css")
        if not os.path.isfile(styles_css_filepath):
            shutil.copy(os.path.join(cons.GLADE_PATH, "styles.css"), cons.CONFIG_DIR)
        shutil.copy(styles_css_filepath, self.new_path)
        return True

    def nodes_all_export_to_html(self, top_tree_iter=None):
        """Export All Nodes To HTML"""
        # create tree links text
        self.tree_links_nums = ["1"]
        shutil.copy(os.path.join(cons.GLADE_PATH, "home.png"), self.images_dir)
        self.tree_links_text = '<div class="tree">\n'
        self.tree_links_text += '<p><strong>Index</strong></p>\n'
        if not top_tree_iter: tree_iter = self.dad.treestore.get_iter_first()
        else: tree_iter = top_tree_iter.copy()
        self.tree_count_level = 1
        while tree_iter:
            self.tree_links_text_iter(tree_iter)
            self.tree_links_nums[-1] = str( int(self.tree_links_nums[-1]) + 1 )
            if top_tree_iter: break
            tree_iter = self.dad.treestore.iter_next(tree_iter)
        if self.tree_count_level > 1:
            print self.tree_count_level-1
            self.tree_links_text += '</ol>'*(self.tree_count_level-1)
        self.tree_links_text += '</div>\n'
        # create index html page
        self.create_tree_index_page()
        # create html pages
        if not top_tree_iter: tree_iter = self.dad.treestore.get_iter_first()
        else: tree_iter = top_tree_iter.copy()
        while tree_iter:
            self.nodes_all_export_to_html_iter(tree_iter)
            if top_tree_iter: break
            tree_iter = self.dad.treestore.iter_next(tree_iter)
        self.tree_links_text = ""
        self.dad.objects_buffer_refresh()

    def nodes_all_export_to_html_iter(self, tree_iter):
        """Export All Nodes To HTML - iter"""
        self.node_export_to_html(tree_iter)
        child_tree_iter = self.dad.treestore.iter_children(tree_iter)
        while child_tree_iter:
            self.nodes_all_export_to_html_iter(child_tree_iter)
            child_tree_iter = self.dad.treestore.iter_next(child_tree_iter)

    def tree_links_text_iter(self, tree_iter):
        """Creating the Tree Links Text - iter"""
        href = self.get_html_filename(tree_iter)
        node_name = clean_text_to_utf8(self.dad.treestore[tree_iter][1])
        if self.tree_count_level < len(self.tree_links_nums):
            self.tree_count_level += 1
            self.tree_links_text += '<ol>\n'
        elif self.tree_count_level > len(self.tree_links_nums):
            i = self.tree_count_level - len(self.tree_links_nums)
            self.tree_links_text += '</ol>\n' * i
            self.tree_count_level -= i
        if self.tree_count_level == 1:
            self.tree_links_text += '<p><a href="' + href + '">' + node_name + '</a></p>\n'
        else:
            self.tree_links_text += '<li><a href="' + href + '">' + node_name + '</a></li>'
        self.tree_links_text += '\n'
        child_tree_iter = self.dad.treestore.iter_children(tree_iter)
        self.tree_links_nums.append("1")
        while child_tree_iter:
            self.tree_links_text_iter(child_tree_iter)
            self.tree_links_nums[-1] = str( int(self.tree_links_nums[-1]) + 1 )
            child_tree_iter = self.dad.treestore.iter_next(child_tree_iter)
        self.tree_links_nums.pop()

    def create_tree_index_page(self):
        """Write the index html file for the tree"""
        html_text = cons.HTML_HEADER % self.dad.file_name
        html_text += self.tree_links_text
        html_text += cons.HTML_FOOTER
        file_descriptor = open(os.path.join(self.new_path, "index.html"), 'w')
        file_descriptor.write(html_text)
        file_descriptor.close()

    def node_export_to_html(self, tree_iter, only_selection=False):
        """Export a Node To HTML"""
        if only_selection:
            iter_start, iter_end = self.dad.curr_buffer.get_selection_bounds()
            sel_range = [iter_start.get_offset(), iter_end.get_offset()]
        else: sel_range = None
        html_text = cons.HTML_HEADER % clean_text_to_utf8(self.dad.treestore[tree_iter][1])
        if self.tree_links_text and self.dad.last_index_in_page:
            td_tree = r'<div class="main">'
            td_page = r'<div class="page">'
            html_text += td_tree + self.tree_links_text + td_page
        if self.dad.last_include_node_name:
            html_text += '<h1><b><u>%s</u></b></h1>' % clean_text_to_utf8(self.dad.treestore[tree_iter][1])
        self.dad.get_textbuffer_from_tree_iter(tree_iter)
        if self.dad.treestore[tree_iter][4] == cons.RICH_TEXT_ID:
            text_n_objects = self.html_get_from_treestore_node(tree_iter, sel_range)
            self.images_count = 0
            for i, html_slot in enumerate(text_n_objects[0]):
                html_text += html_slot
                if i < len(text_n_objects[1]):
                    curr_object = text_n_objects[1][i]
                    if curr_object[0] == "pixbuf":
                        pix_dir = dir(curr_object[1][1])
                        if "embfile" in pix_dir:
                            html_text += self.get_embfile_html(curr_object[1], tree_iter)
                        else:
                            html_text += self.get_image_html(curr_object[1], tree_iter)
                    elif curr_object[0] == "table": html_text += self.get_table_html(curr_object[1])
                    elif curr_object[0] == "codebox": html_text += self.get_codebox_html(curr_object[1])
        else: html_text += self.html_get_from_code_buffer(self.dad.treestore[tree_iter][2], sel_range)
        if self.tree_links_text and not self.dad.last_index_in_page:
            html_text += '<p align="center">' + '<img src="%s" height="22" width="22">' % os.path.join("images", "home.png") + 2*cons.CHAR_SPACE + '<a href="index.html">' + _("Index") + '</a></p>'
        if self.tree_links_text and self.dad.last_index_in_page:
            html_text += '</div></div>\n'
        html_text += cons.HTML_FOOTER
        node_html_filepath = os.path.join(self.new_path, self.get_html_filename(tree_iter))
        #print "full=%s(prefix=%s)" % (len(node_html_filepath), len(self.new_path))
        file_descriptor = open(node_html_filepath, 'w')
        file_descriptor.write(html_text)
        file_descriptor.close()

    def selection_export_to_html(self, text_buffer, start_iter, end_iter, syntax_highlighting):
        """Returns the HTML given the text buffer and iter bounds"""
        html_text = cons.HTML_HEADER % ""
        if syntax_highlighting == cons.RICH_TEXT_ID:
            text_n_objects = self.html_get_from_rich_text_selection(text_buffer, start_iter, end_iter)
            self.images_count = 0
            for i, html_slot in enumerate(text_n_objects[0]):
                html_text += html_slot
                if i < len(text_n_objects[1]):
                    curr_object = text_n_objects[1][i]
                    if curr_object[0] == "pixbuf":
                        self.images_dir = cons.TMP_FOLDER
                        html_text += self.get_image_html(curr_object[1], None)
                    elif curr_object[0] == "table": html_text += self.get_table_html(curr_object[1])
                    elif curr_object[0] == "codebox": html_text += self.get_codebox_html(curr_object[1])
        else: html_text += self.html_get_from_code_buffer(text_buffer, sel_range=(start_iter.get_offset(), end_iter.get_offset()))
        html_text += cons.HTML_FOOTER
        return html_text

    def table_export_to_html(self, table_dict):
        """Returns the HTML given the table dict"""
        html_text = cons.HTML_HEADER % ""
        html_text += self.get_table_html([0, table_dict, cons.TAG_PROP_LEFT])
        html_text += cons.HTML_FOOTER
        return html_text

    def codebox_export_to_html(self, codebox_dict):
        """Returns the HTML given the table dict"""
        html_text = cons.HTML_HEADER % ""
        html_text += self.get_codebox_html([0, codebox_dict, cons.TAG_PROP_LEFT])
        html_text += cons.HTML_FOOTER
        return html_text

    def get_embfile_html(self, embeded, tree_iter):
        """Returns the HTML embedded file"""
        # embeded is: [offset, pixbuf, justification]
        embfile_align_text = self.get_object_alignment_string(embeded[2])
        if tree_iter:
            embfile_name = "%s-%s" % (self.dad.treestore[tree_iter][3], embeded[1].filename)
            embfile_rel_path = os.path.join("EmbeddedFiles", embfile_name)
        else:
            embfile_name = "%s.png" % embeded[1].filename
            embfile_rel_path = "file://" + os.path.join(self.embed_dir, embfile_name)
        embfile_html = '<table style="%s"><tr><td><a href="%s">Linked file: %s</a></td></tr></table>' % (embfile_align_text, embfile_rel_path, embeded[1].filename)
        with open(os.path.join(self.embed_dir,embfile_name), 'wb') as fd:
            fd.write(embeded[1].embfile)
        return embfile_html

    def get_image_html(self, image, tree_iter):
        """Returns the HTML Image"""
        # image is: [offset, pixbuf, justification]
        if "anchor" in dir(image[1]):
            return '<a name="%s"></a>' % image[1].anchor
        image_align_text = self.get_object_alignment_string(image[2])
        self.images_count += 1
        if tree_iter:
            image_name = "%s-%s.png" % (self.dad.treestore[tree_iter][3], self.images_count)
            image_rel_path = os.path.join("images", image_name)
        else:
            image_name = "%s.png" % self.images_count
            image_rel_path = "file://" + os.path.join(self.images_dir, image_name)
        image_html = '<img src="%s" alt="%s" />' % (image_rel_path, image_rel_path)
        if image[1].link:
            href = self.get_href_from_link_prop_val(image[1].link)
            image_html = '<a href="%s">' % href + image_html + "</a>"
        image[1].save(os.path.join(self.images_dir, image_name), "png")
        return image_html

    def get_codebox_html(self, codebox):
        """Returns the HTML CodeBox"""
        # codebox is: [offset, dict, justification]
        codebox_align_text = self.get_object_alignment_string(codebox[2])
        codebox_html = '<div class="codebox">'
        codebox_html += codebox[1]['fill_text']
        codebox_html += "</div>"
        return codebox_html

    def get_table_html(self, table_orig):
        """Returns the HTML Table"""
        # table is: [offset, dict, justification]
        table = copy.deepcopy(table_orig)
        table_align_text = self.get_object_alignment_string(table[2])
        table_html = '<table class="table">'
        table[1]['matrix'].insert(0, table[1]['matrix'].pop())
        for col in table[1]['matrix'][0]:
            table_html += '<col/>'
        for j, row in enumerate(table[1]['matrix']):
            table_html += "<tr>"
            for cell in row:
                cell = cgi.escape(cell)
                if j == 0: table_html += "<th>" + cell + "</th>"
                else: table_html += "<td>" + cell + "</td>"
            table_html += "</tr>"
        table_html += "</table>"
        return table_html

    def get_object_alignment_string(self, alignment):
        """Returns the style attribute(s) according to the alignment"""
        if alignment == cons.TAG_PROP_CENTER: return "margin-left:auto;margin-right:auto"
        elif alignment == cons.TAG_PROP_RIGHT: return "margin-left:auto"
        else: return "display:inline-table"

    def get_html_filename(self, tree_iter):
        """Get the HTML page filename given the tree iter"""
        return support.get_node_hierarchical_name(self.dad, tree_iter, trailer=".html").replace("#","~")

    def html_get_from_code_buffer(self, code_buffer, sel_range=None):
        """Get rich text from syntax highlighted code node"""
        if sel_range: curr_iter = code_buffer.get_iter_at_offset(sel_range[0])
        else: curr_iter = code_buffer.get_start_iter()
        code_buffer.ensure_highlight(curr_iter, code_buffer.get_end_iter())
        html_text = ""
        former_tag_str = cons.COLOR_48_BLACK
        span_opened = False
        while 1:
            curr_tags = curr_iter.get_tags()
            if len(curr_tags) > 0:
                curr_tag_str = curr_tags[0].get_property("foreground-gdk").to_string()
                font_weight = curr_tags[0].get_property(cons.TAG_WEIGHT)
                if curr_tag_str == cons.COLOR_48_BLACK:
                    if former_tag_str != curr_tag_str:
                        former_tag_str = curr_tag_str
                        # end of tag
                        html_text += "</span>"
                        span_opened = False
                else:
                    if former_tag_str != curr_tag_str:
                        former_tag_str = curr_tag_str
                        if span_opened: html_text += "</span>"
                        # start of tag
                        curr_tag_str = "#" + rgb_to_no_white(curr_tag_str[1:])
                        html_text += '<span style="color:#%s;font-weight:%s">' % (rgb_any_to_24(curr_tag_str[1:]), font_weight)
                        span_opened = True
            elif span_opened:
                span_opened = False
                former_tag_str = cons.COLOR_48_BLACK
                html_text += "</span>"
            html_text += cgi.escape(curr_iter.get_char()).replace(cons.CHAR_SPACE, "&nbsp;")
            if not curr_iter.forward_char() or (sel_range and curr_iter.get_offset() > sel_range[1]):
                if span_opened: html_text += "</span>"
                break
        html_text = html_text.replace(cons.CHAR_NEWLINE, "<br />")
        return '<div class="codebox">'+html_text+'</div>'

    def html_get_from_treestore_node(self, node_iter, sel_range=None):
        """Given a treestore iter returns the HTML rich text"""
        #print "to html node", self.dad.treestore[node_iter][1]
        curr_buffer = self.dad.treestore[node_iter][2]
        pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer,
                                                                                                   for_print=2,
                                                                                                   sel_range=sel_range)
        # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
        self.curr_html_slots = []
        start_offset = 0 if not sel_range else sel_range[0]
        for end_offset_element in pixbuf_table_codebox_vector:
            end_offset = end_offset_element[1][0]
            self.html_process_slot(start_offset, end_offset, curr_buffer)
            start_offset = end_offset
        if not sel_range: self.html_process_slot(start_offset, -1, curr_buffer)
        else: self.html_process_slot(start_offset, sel_range[1], curr_buffer)
        #print "curr_html_slots", self.curr_html_slots
        #print "pixbuf_table_codebox_vector", pixbuf_table_codebox_vector
        return [self.curr_html_slots, pixbuf_table_codebox_vector]

    def html_get_from_rich_text_selection(self, text_buffer, start_iter, end_iter):
        """Given a text buffer and iter bounds returns the HTML rich text"""
        pixbuf_table_codebox_vector = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(text_buffer,
                                                                                                   for_print=2,
                                                                                                   sel_range=(start_iter.get_offset(), end_iter.get_offset()))
        # pixbuf_table_codebox_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
        self.curr_html_slots = []
        start_offset = start_iter.get_offset()
        for end_offset_element in pixbuf_table_codebox_vector:
            end_offset = end_offset_element[1][0]
            self.html_process_slot(start_offset, end_offset, text_buffer)
            start_offset = end_offset
        self.html_process_slot(start_offset, end_iter.get_offset(), text_buffer)
        #print "curr_html_slots", self.curr_html_slots
        #print "pixbuf_table_codebox_vector", pixbuf_table_codebox_vector
        return [self.curr_html_slots, pixbuf_table_codebox_vector]

    def html_process_slot(self, start_offset, end_offset, curr_buffer):
        """Process a Single HTML Slot"""
        self.curr_html_text = ""
        start_iter = curr_buffer.get_iter_at_offset(start_offset)
        if end_offset == -1:
            end_offset = curr_buffer.get_end_iter().get_offset()
        #print "process slot (%s->%s)" % (start_offset, end_offset)
        # begin operations
        self.curr_attributes = {}
        for tag_property in cons.TAG_PROPERTIES: self.curr_attributes[tag_property] = ""
        curr_iter = start_iter.copy()
        self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
        tag_found = curr_iter.forward_to_tag_toggle(None)
        while tag_found:
            if curr_iter.get_offset() > end_offset:
                curr_iter = curr_buffer.get_iter_at_offset(end_offset)
            self.html_text_serialize(start_iter, curr_iter)
            offset_old = curr_iter.get_offset()
            if offset_old >= end_offset: break
            else:
                self.dad.xml_handler.rich_text_attributes_update(curr_iter, self.curr_attributes)
                start_iter.set_offset(offset_old)
                tag_found = curr_iter.forward_to_tag_toggle(None)
                if curr_iter.get_offset() == offset_old: break
        else:
            if curr_iter.get_offset() > end_offset:
                curr_iter = curr_buffer.get_iter_at_offset(end_offset)
            self.html_text_serialize(start_iter, curr_iter)
        self.curr_html_text = self.curr_html_text.replace("<br/><p ", "<p ")
        self.curr_html_text = self.curr_html_text.replace("</p><br/>", "</p>")
        for header in [cons.TAG_PROP_H1, cons.TAG_PROP_H2, cons.TAG_PROP_H3]:
            self.curr_html_text = self.curr_html_text.replace("</%s><%s>" % (header, header), "")
        self.curr_html_slots.append(self.curr_html_text)

    def html_text_serialize(self, start_iter, end_iter):
        """Adds a slice to the HTML Text"""
        inner_text = cgi.escape(start_iter.get_text(end_iter))
        if inner_text == "": return
        inner_text = inner_text.replace(cons.CHAR_NEWLINE, "<br />")
        html_attrs = ""
        superscript_active = False
        subscript_active = False
        monospace_active = False
        bold_active = False
        italic_active = False
        for tag_property in cons.TAG_PROPERTIES:
            if self.curr_attributes[tag_property] != '':
                property_value = self.curr_attributes[tag_property]
                #print property_value
                if tag_property == cons.TAG_WEIGHT:
                    # font-weight:bolder
                    #tag_property = "font-weight"
                    #property_value = "bolder"
                    bold_active = True
                    continue
                elif tag_property == cons.TAG_FOREGROUND:
                    # color:#FFFF00
                    tag_property = "color"
                    color_no_white = rgb_to_no_white(property_value[1:])
                    property_value = "#" + rgb_any_to_24(color_no_white)
                elif tag_property == cons.TAG_BACKGROUND:
                    # background-color:#FFFF00
                    tag_property = "background-color"
                    property_value = "#" + rgb_any_to_24(property_value[1:])
                elif tag_property == cons.TAG_STYLE:
                    # font-style:italic
                    #tag_property = "font-style"
                    #property_value = cons.TAG_PROP_ITALIC
                    italic_active = True
                    continue
                elif tag_property == cons.TAG_UNDERLINE:
                    # text-decoration:underline
                    tag_property = "text-decoration"
                    property_value = cons.TAG_UNDERLINE
                elif tag_property == cons.TAG_STRIKETHROUGH:
                    # text-decoration:line-through
                    tag_property = "text-decoration"
                    property_value = "line-through"
                elif tag_property == cons.TAG_SCALE:
                    if property_value == cons.TAG_PROP_SUP:
                        superscript_active = True
                        continue
                    elif property_value == cons.TAG_PROP_SUB:
                        subscript_active = True
                        continue
                    else:
                        # font-size:xx-large/x-large/x-small
                        tag_property = "font-size"
                        if property_value == cons.TAG_PROP_SMALL: property_value = "x-small"
                        elif property_value == cons.TAG_PROP_H1: property_value = "xx-large"
                        elif property_value == cons.TAG_PROP_H2: property_value = "x-large"
                        elif property_value == cons.TAG_PROP_H3: property_value = "large"
                elif tag_property == cons.TAG_FAMILY:
                    monospace_active = True
                    continue
                elif tag_property == cons.TAG_JUSTIFICATION:
                    # text-align:center/left/right
                    #tag_property = "text-align"
                    continue
                elif tag_property == cons.TAG_LINK:
                    # <a href="http://www.example.com/">link-text goes here</a>
                    href = self.get_href_from_link_prop_val(property_value)
                    if not href:
                        continue
                    self.curr_html_text += '<a href="' + href + '">' + inner_text + "</a>"
                    return
                html_attrs += "%s:%s;" % (tag_property, property_value)
        if html_attrs == "" or inner_text == "<br />":
            tagged_text = inner_text
        else:
            if 'xx-large' in html_attrs:
                tagged_text = '<h1>' + inner_text + "</h1>"
            elif 'x-large' in html_attrs:
                tagged_text = '<h2>' + inner_text + "</h2>"
            elif 'large' in html_attrs:
                tagged_text = '<h3>' + inner_text + '</h3>'
            elif 'x-small' in html_attrs:
                tagged_text = '<small>' + inner_text + "</small>"
            else:
                tagged_text = '<span style="' + html_attrs + '">' + inner_text + "</span>"
        if superscript_active: tagged_text = "<sup>" + tagged_text + "</sup>"
        if subscript_active: tagged_text = "<sub>" + tagged_text + "</sub>"
        if monospace_active: tagged_text = "<code>" + tagged_text + "</code>"
        if bold_active: tagged_text = "<strong>" + tagged_text + "</strong>"
        if italic_active: tagged_text = "<em>" + tagged_text + "</em>"
        self.curr_html_text += tagged_text
        #print "###############"
        #print self.curr_html_text
        #print "###############"

    def get_href_from_link_prop_val(self, link_prop_val):
        href = ""
        vector = link_prop_val.split()
        if vector[0] == cons.LINK_TYPE_WEBS:
            href = vector[1]
        elif vector[0] == cons.LINK_TYPE_FILE:
            filepath = self.dad.link_process_filepath(vector[1])
            href = "file://" + filepath
        elif vector[0] == cons.LINK_TYPE_FOLD:
            folderpath = self.dad.link_process_folderpath(vector[1])
            href = "file://" + folderpath
        elif vector[0] == cons.LINK_TYPE_NODE:
            dest_tree_iter = self.dad.get_tree_iter_from_node_id(long(vector[1]))
            if dest_tree_iter:
                href = self.get_html_filename(dest_tree_iter)
                if len(vector) >= 3:
                    if len(vector) == 3: anchor_name = vector[2]
                    else: anchor_name = link_prop_val[len(vector[0]) + len(vector[1]) + 2:]
                    href += "#" + anchor_name
        return href
