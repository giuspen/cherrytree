# -*- coding: UTF-8 -*-
#
#       codeboxes.py
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

import gtk, gtksourceview2, pango
import os
import cons, support

DRAW_SPACES_FLAGS = gtksourceview2.DRAW_SPACES_ALL & ~gtksourceview2.DRAW_SPACES_NEWLINE
CB_WIDTH_HEIGHT_STEP_PIX = 30
CB_WIDTH_HEIGHT_STEP_PERC = 9
CB_WIDTH_LIMIT_MIN = 40
CB_HEIGHT_LIMIT_MIN = 30


class CodeBoxesHandler:
    """Handler of the CodeBoxes"""

    def __init__(self, dad):
        """Lists Handler boot"""
        self.dad = dad

    def codebox_cut(self, *args):
        """Cut CodeBox"""
        self.dad.object_set_selection(self.curr_codebox_anchor)
        self.dad.sourceview.emit("cut-clipboard")

    def codebox_copy(self, *args):
        """Copy CodeBox"""
        self.dad.object_set_selection(self.curr_codebox_anchor)
        self.dad.sourceview.emit("copy-clipboard")

    def codebox_delete(self, *args):
        """Delete CodeBox"""
        self.dad.object_set_selection(self.curr_codebox_anchor)
        self.dad.curr_buffer.delete_selection(True, self.dad.sourceview.get_editable())
        self.dad.sourceview.grab_focus()

    def dialog_codeboxhandle(self, title, syntax_highl, line_num, match_bra):
        """Opens the CodeBox Handle Dialog"""
        dialog = gtk.Dialog(title=title,
                            parent=self.dad.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
        dialog.set_default_size(300, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        
        combobox_prog_lang = gtk.ComboBox(model=self.dad.prog_lang_liststore)
        cell = gtk.CellRendererText()
        combobox_prog_lang.pack_start(cell, True)
        combobox_prog_lang.add_attribute(cell, 'text', 0)
        combobox_prog_lang.set_active_iter(self.dad.get_combobox_iter_from_value(self.dad.prog_lang_liststore, 1, syntax_highl))
        type_vbox = gtk.VBox()
        type_vbox.pack_start(combobox_prog_lang)
        type_vbox.set_border_width(6)
        
        type_frame = gtk.Frame(label="<b>"+_("Automatic Syntax Highlighting")+"</b>")
        type_frame.get_label_widget().set_use_markup(True)
        type_frame.set_shadow_type(gtk.SHADOW_NONE)
        type_frame.add(type_vbox)
        
        label_width = gtk.Label(_("Width"))
        adj_width = gtk.Adjustment(value=self.dad.codebox_width, lower=1, upper=10000, step_incr=1)
        spinbutton_width = gtk.SpinButton(adj_width)
        spinbutton_width.set_value(self.dad.codebox_width)
        label_height = gtk.Label(_("Height"))
        adj_height = gtk.Adjustment(value=self.dad.codebox_height, lower=1, upper=10000, step_incr=1)
        spinbutton_height = gtk.SpinButton(adj_height)
        spinbutton_height.set_value(self.dad.codebox_height)
        
        radiobutton_codebox_pixels = gtk.RadioButton(label=_("pixels"))
        radiobutton_codebox_percent = gtk.RadioButton(label="%")
        radiobutton_codebox_percent.set_group(radiobutton_codebox_pixels)
        radiobutton_codebox_pixels.set_active(self.dad.codebox_width_pixels)
        radiobutton_codebox_percent.set_active(not self.dad.codebox_width_pixels)
        
        vbox_pix_perc = gtk.VBox()
        vbox_pix_perc.pack_start(radiobutton_codebox_pixels)
        vbox_pix_perc.pack_start(radiobutton_codebox_percent)
        hbox_width = gtk.HBox()
        hbox_width.pack_start(label_width, expand=False)
        hbox_width.pack_start(spinbutton_width, expand=False)
        hbox_width.pack_start(vbox_pix_perc)
        hbox_width.set_spacing(5)
        hbox_height = gtk.HBox()
        hbox_height.pack_start(label_height, expand=False)
        hbox_height.pack_start(spinbutton_height, expand=False)
        hbox_height.set_spacing(5)
        vbox_size = gtk.VBox()
        vbox_size.pack_start(hbox_width)
        vbox_size.pack_start(hbox_height)
        size_align = gtk.Alignment()
        size_align.set_padding(0, 6, 6, 6)
        size_align.add(vbox_size)
        
        size_frame = gtk.Frame(label="<b>"+_("Size")+"</b>")
        size_frame.get_label_widget().set_use_markup(True)
        size_frame.set_shadow_type(gtk.SHADOW_NONE)
        size_frame.add(size_align)
        
        checkbutton_codebox_linenumbers = gtk.CheckButton(label=_("Show Line Numbers"))
        checkbutton_codebox_linenumbers.set_active(line_num)
        checkbutton_codebox_matchbrackets = gtk.CheckButton(label=_("Highlight Matching Brackets"))
        checkbutton_codebox_matchbrackets.set_active(match_bra)
        vbox_options = gtk.VBox()
        vbox_options.pack_start(checkbutton_codebox_linenumbers)
        vbox_options.pack_start(checkbutton_codebox_matchbrackets)
        opt_align = gtk.Alignment()
        opt_align.set_padding(6, 6, 6, 6)
        opt_align.add(vbox_options)
        
        options_frame = gtk.Frame(label="<b>"+_("Options")+"</b>")
        options_frame.get_label_widget().set_use_markup(True)
        options_frame.set_shadow_type(gtk.SHADOW_NONE)
        options_frame.add(opt_align)
        
        content_area = dialog.get_content_area()
        content_area.set_spacing(5)
        content_area.pack_start(type_frame)
        content_area.pack_start(size_frame)
        content_area.pack_start(options_frame)
        content_area.show_all()
        def on_key_press_codeboxhandle(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if keyname == cons.STR_RETURN:
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
            return False
        def on_radiobutton_codebox_pixels_toggled(radiobutton):
            if radiobutton.get_active():
                spinbutton_width.set_value(700)
            else:
                if spinbutton_width.get_value() > 100:
                    spinbutton_width.set_value(90)
        dialog.connect('key_press_event', on_key_press_codeboxhandle)
        radiobutton_codebox_pixels.connect('toggled', on_radiobutton_codebox_pixels_toggled)
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            ret_syntax = self.dad.prog_lang_liststore[combobox_prog_lang.get_active_iter()][1]
            self.dad.codebox_width = spinbutton_width.get_value()
            self.dad.codebox_width_pixels = radiobutton_codebox_pixels.get_active()
            self.dad.codebox_height = spinbutton_height.get_value()
            ret_line_num = checkbutton_codebox_linenumbers.get_active()
            ret_match_bra = checkbutton_codebox_matchbrackets.get_active()
            return [ret_syntax, ret_line_num, ret_match_bra]
        return [None, None, None]

    def codebox_handle(self):
        """Insert Code Box"""
        if self.dad.curr_buffer.get_has_selection():
            iter_sel_start, iter_sel_end = self.dad.curr_buffer.get_selection_bounds()
            fill_text = unicode(self.dad.curr_buffer.get_text(iter_sel_start, iter_sel_end), cons.STR_UTF8, cons.STR_IGNORE)
        else: fill_text = None
        ret_syntax, ret_line_num, ret_match_bra = self.dialog_codeboxhandle(_("Insert a CodeBox"), "sh", False, True)
        if not ret_syntax: return
        codebox_dict = {
           'frame_width': int(self.dad.codebox_width),
           'frame_height': int(self.dad.codebox_height),
           'width_in_pixels': self.dad.codebox_width_pixels,
           'syntax_highlighting': ret_syntax,
           'highlight_brackets': ret_match_bra,
           'show_line_numbers': ret_line_num,
           'fill_text': fill_text
        }
        if fill_text: self.dad.curr_buffer.delete(iter_sel_start, iter_sel_end)
        iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
        self.codebox_insert(iter_insert, codebox_dict)

    def codebox_insert(self, iter_insert, codebox_dict, codebox_justification=None, text_buffer=None):
        """Insert Code Box"""
        if not text_buffer: text_buffer = self.dad.curr_buffer
        anchor = text_buffer.create_child_anchor(iter_insert)
        self.curr_codebox_anchor = anchor
        anchor.frame_width = codebox_dict['frame_width']
        anchor.frame_height = codebox_dict['frame_height']
        anchor.width_in_pixels = codebox_dict['width_in_pixels']
        anchor.syntax_highlighting = codebox_dict['syntax_highlighting']
        anchor.highlight_brackets = codebox_dict['highlight_brackets']
        anchor.show_line_numbers = codebox_dict['show_line_numbers']
        anchor.sourcebuffer = gtksourceview2.Buffer()
        anchor.sourcebuffer.set_style_scheme(self.dad.sourcestyleschememanager.get_scheme(self.dad.style_scheme))
        self.dad.set_sourcebuffer_syntax_highlight(anchor.sourcebuffer, anchor.syntax_highlighting)
        anchor.sourcebuffer.set_highlight_matching_brackets(anchor.highlight_brackets)
        anchor.sourcebuffer.connect('insert-text', self.dad.on_text_insertion)
        anchor.sourcebuffer.connect('delete-range', self.dad.on_text_removal)
        anchor.sourcebuffer.connect('modified-changed', self.dad.on_modified_changed)
        if codebox_dict['fill_text']:
            anchor.sourcebuffer.set_text(codebox_dict['fill_text'])
            anchor.sourcebuffer.set_modified(False)
        anchor.sourceview = gtksourceview2.View(anchor.sourcebuffer)
        anchor.sourceview.set_smart_home_end(gtksourceview2.SMART_HOME_END_BEFORE)
        if self.dad.highl_curr_line: anchor.sourceview.set_highlight_current_line(True)
        if self.dad.show_white_spaces: anchor.sourceview.set_draw_spaces(DRAW_SPACES_FLAGS)
        anchor.sourceview.modify_font(pango.FontDescription(self.dad.code_font))
        anchor.sourceview.set_show_line_numbers(anchor.show_line_numbers)
        anchor.sourceview.set_insert_spaces_instead_of_tabs(self.dad.spaces_instead_tabs)
        anchor.sourceview.set_tab_width(self.dad.tabs_width)
        anchor.sourceview.set_auto_indent(self.dad.auto_indent)
        anchor.sourceview.connect('populate-popup', self.on_sourceview_populate_popup_codebox, anchor)
        anchor.sourceview.connect('key_press_event', self.on_key_press_sourceview_codebox, anchor)
        anchor.sourceview.connect('button-press-event', self.on_mouse_button_clicked_codebox, anchor)
        if self.dad.line_wrapping: anchor.sourceview.set_wrap_mode(gtk.WRAP_WORD)
        else: anchor.sourceview.set_wrap_mode(gtk.WRAP_NONE)
        scrolledwindow = gtk.ScrolledWindow()
        scrolledwindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scrolledwindow.add(anchor.sourceview)
        anchor.frame = gtk.Frame()
        self.codebox_apply_width_height(anchor)
        anchor.frame.add(scrolledwindow)
        self.dad.sourceview.add_child_at_anchor(anchor.frame, anchor)
        for win in [gtk.TEXT_WINDOW_LEFT, gtk.TEXT_WINDOW_RIGHT, gtk.TEXT_WINDOW_TOP, gtk.TEXT_WINDOW_BOTTOM]:
            anchor.sourceview.set_border_window_size(win, 1)
        anchor.frame.set_shadow_type(gtk.SHADOW_NONE)
        anchor.frame.show_all()
        if codebox_justification:
            text_iter = text_buffer.get_iter_at_child_anchor(anchor)
            self.dad.state_machine.apply_object_justification(text_iter, codebox_justification, text_buffer)
        elif self.dad.user_active:
            # if I apply a justification, the state is already updated
            self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])

    def codebox_increase_width(self, *args):
        """Increase CodeBox Width"""
        if self.curr_codebox_anchor.width_in_pixels:
            self.codebox_change_width_height(self.curr_codebox_anchor.frame_width + CB_WIDTH_HEIGHT_STEP_PIX, 0)
        else:
            self.codebox_change_width_height(self.curr_codebox_anchor.frame_width + CB_WIDTH_HEIGHT_STEP_PERC, 0)

    def codebox_decrease_width(self, *args):
        """Decrease CodeBox Width"""
        if self.curr_codebox_anchor.width_in_pixels:
            if self.curr_codebox_anchor.frame_width - CB_WIDTH_HEIGHT_STEP_PIX >= CB_WIDTH_LIMIT_MIN:
                self.codebox_change_width_height(self.curr_codebox_anchor.frame_width - CB_WIDTH_HEIGHT_STEP_PIX, 0)
        else:
            if self.curr_codebox_anchor.frame_width - CB_WIDTH_HEIGHT_STEP_PERC >= CB_WIDTH_LIMIT_MIN:
                self.codebox_change_width_height(self.curr_codebox_anchor.frame_width - CB_WIDTH_HEIGHT_STEP_PERC, 0)

    def codebox_increase_height(self, *args):
        """Increase CodeBox Height"""
        self.codebox_change_width_height(0, self.curr_codebox_anchor.frame_height + CB_WIDTH_HEIGHT_STEP_PIX)

    def codebox_decrease_height(self, *args):
        """Decrease CodeBox Height"""
        if self.curr_codebox_anchor.frame_height - CB_WIDTH_HEIGHT_STEP_PIX >= CB_HEIGHT_LIMIT_MIN:
            self.codebox_change_width_height(0, self.curr_codebox_anchor.frame_height - CB_WIDTH_HEIGHT_STEP_PIX)

    def codebox_apply_width_height(self, anchor, from_shortcut=False):
        """Apply Width and Height Changes to CodeBox"""
        if anchor.width_in_pixels: frame_width = anchor.frame_width
        else: frame_width = self.dad.get_text_window_width()*anchor.frame_width/100
        anchor.frame.set_size_request(frame_width, anchor.frame_height)
        if from_shortcut:
            self.dad.update_window_save_needed("nbuf", True)

    def codebox_change_width_height(self, new_width, new_height):
        """Replace CodeBox changing Width and Height"""
        codebox_iter = self.dad.curr_buffer.get_iter_at_child_anchor(self.curr_codebox_anchor)
        codebox_element = [codebox_iter.get_offset(),
                           self.dad.state_machine.codebox_to_dict(self.curr_codebox_anchor,
                                                                  for_print=0),
                           self.dad.state_machine.get_iter_alignment(codebox_iter)]
        if new_width: codebox_element[1]['frame_width'] = new_width
        if new_height: codebox_element[1]['frame_height'] = new_height
        #cursor_pos_restore = self.curr_codebox_anchor.sourcebuffer.get_property(cons.STR_CURSOR_POSITION)
        self.codebox_delete()
        iter_insert = self.dad.curr_buffer.get_iter_at_offset(codebox_element[0])
        self.codebox_insert(iter_insert, codebox_element[1], codebox_element[2])
        self.curr_codebox_anchor.sourceview.grab_focus()
        #self.curr_codebox_anchor.sourcebuffer.place_cursor(self.curr_codebox_anchor.sourcebuffer.get_iter_at_offset(cursor_pos_restore))
        #self.curr_codebox_anchor.sourceview.scroll_to_mark(self.curr_codebox_anchor.sourcebuffer.get_insert(), 0.3)

    def codebox_change_properties(self, action):
        """Change CodeBox Properties"""
        self.dad.codebox_width = self.curr_codebox_anchor.frame_width
        self.dad.codebox_width_pixels = self.curr_codebox_anchor.width_in_pixels
        self.dad.codebox_height = self.curr_codebox_anchor.frame_height
        ret_syntax, ret_line_num, ret_match_bra = self.dialog_codeboxhandle(_("Edit CodeBox"), self.curr_codebox_anchor.syntax_highlighting, self.curr_codebox_anchor.show_line_numbers, self.curr_codebox_anchor.highlight_brackets)
        if not ret_syntax: return
        self.curr_codebox_anchor.syntax_highlighting = ret_syntax
        self.dad.set_sourcebuffer_syntax_highlight(self.curr_codebox_anchor.sourcebuffer, self.curr_codebox_anchor.syntax_highlighting)
        self.curr_codebox_anchor.frame_width = int(self.dad.codebox_width)
        self.curr_codebox_anchor.frame_height = int(self.dad.codebox_height)
        self.curr_codebox_anchor.width_in_pixels = self.dad.codebox_width_pixels
        self.curr_codebox_anchor.highlight_brackets = ret_match_bra
        self.curr_codebox_anchor.sourcebuffer.set_highlight_matching_brackets(self.curr_codebox_anchor.highlight_brackets)
        self.curr_codebox_anchor.show_line_numbers = ret_line_num
        self.curr_codebox_anchor.sourceview.set_show_line_numbers(self.curr_codebox_anchor.show_line_numbers)
        self.codebox_apply_width_height(self.curr_codebox_anchor)
        self.dad.update_window_save_needed("nbuf", True)

    def on_key_press_sourceview_codebox(self, widget, event, anchor):
        """Extend the Default Right-Click Menu of the CodeBox"""
        if event.state & gtk.gdk.CONTROL_MASK:
            keyname = gtk.gdk.keyval_name(event.keyval)
            self.curr_codebox_anchor = anchor
            if keyname == "period":
                if event.state & gtk.gdk.MOD1_MASK:
                    self.codebox_decrease_width()
                else: self.codebox_increase_width()
            elif keyname == "comma":
                if event.state & gtk.gdk.MOD1_MASK:
                    self.codebox_decrease_height()
                else: self.codebox_increase_height()

    def on_mouse_button_clicked_codebox(self, widget, event, anchor):
        """Catches mouse buttons clicks"""
        self.curr_codebox_anchor = anchor
        if event.button != 3:
            self.dad.object_set_selection(self.curr_codebox_anchor)

    def on_sourceview_populate_popup_codebox(self, textview, menu, anchor):
        """Extend the Default Right-Click Menu of the CodeBox"""
        self.dad.menu_populate_popup(menu, cons.get_popup_menu_entries_codebox(self), self.dad.orphan_accel_group)
