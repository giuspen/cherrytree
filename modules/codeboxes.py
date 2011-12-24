# -*- coding: UTF-8 -*-
#
#       codeboxes.py
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

import gtk, gtksourceview2, pango
import os
import cons, support

DRAW_SPACES_FLAGS = gtksourceview2.DRAW_SPACES_ALL & ~gtksourceview2.DRAW_SPACES_NEWLINE


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

    def on_key_press_codeboxhandledialog(self, widget, event):
        """Catches CodeBoxHandle Dialog key presses"""
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == "Return": self.dad.glade.codeboxhandledialog_button_ok.clicked()

    def codebox_handle(self):
        """Insert Code Box"""
        if self.dad.curr_buffer.get_has_selection():
            iter_sel_start, iter_sel_end = self.dad.curr_buffer.get_selection_bounds()
            fill_text = self.dad.curr_buffer.get_text(iter_sel_start, iter_sel_end)
        else: fill_text = None
        self.dad.glade.codeboxhandledialog.set_title(_("Insert a CodeBox"))
        response = self.dad.glade.codeboxhandledialog.run()
        self.dad.glade.codeboxhandledialog.hide()
        if response != 1: return # the user aborted the operation
        codebox_dict = {
           'frame_width': int(self.dad.glade.spinbutton_codebox_width.get_value()),
           'frame_height': int(self.dad.glade.spinbutton_codebox_height.get_value()),
           'width_in_pixels': self.dad.glade.radiobutton_codebox_pixels.get_active(),
           'syntax_highlighting': self.dad.prog_lang_liststore[self.dad.glade.combobox_prog_lang_codebox.get_active_iter()][1],
           'highlight_brackets': self.dad.glade.checkbutton_codebox_matchbrackets.get_active(),
           'show_line_numbers': self.dad.glade.checkbutton_codebox_linenumbers.get_active(),
           'fill_text': fill_text
        }
        if fill_text: self.dad.curr_buffer.delete(iter_sel_start, iter_sel_end)
        iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
        self.codebox_insert(iter_insert, codebox_dict)

    def codebox_insert(self, iter_insert, codebox_dict, codebox_justification=None):
        """Insert Code Box"""
        anchor = self.dad.curr_buffer.create_child_anchor(iter_insert)
        anchor.frame_width = codebox_dict['frame_width']
        anchor.frame_height = codebox_dict['frame_height']
        anchor.width_in_pixels = codebox_dict['width_in_pixels']
        anchor.syntax_highlighting = codebox_dict['syntax_highlighting']
        anchor.highlight_brackets = codebox_dict['highlight_brackets']
        anchor.show_line_numbers = codebox_dict['show_line_numbers']
        anchor.sourcebuffer = gtksourceview2.Buffer()
        anchor.sourcebuffer.set_style_scheme(self.dad.sourcestylescheme)
        self.dad.set_sourcebuffer_syntax_highlight(anchor.sourcebuffer, anchor.syntax_highlighting)
        anchor.sourcebuffer.set_highlight_matching_brackets(anchor.highlight_brackets)
        anchor.sourcebuffer.connect('insert-text', self.dad.on_text_insertion)
        anchor.sourcebuffer.connect('delete-range', self.dad.on_text_removal)
        anchor.sourcebuffer.connect('modified-changed', self.dad.on_modified_changed)
        if codebox_dict['fill_text']:
            anchor.sourcebuffer.set_text(codebox_dict['fill_text'])
            anchor.sourcebuffer.set_modified(False)
        anchor.sourceview = gtksourceview2.View(anchor.sourcebuffer)
        anchor.sourceview.set_highlight_current_line(True)
        if self.dad.show_white_spaces:
            anchor.sourceview.set_draw_spaces(DRAW_SPACES_FLAGS)
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
            self.dad.state_machine.apply_image_justification(self.dad.curr_buffer.get_iter_at_child_anchor(anchor), codebox_justification)
        else:
            # if I apply a justification, the state is already updated
            self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])

    def codebox_increase_width(self, *args):
        """Increase CodeBox Width"""
        if self.curr_codebox_anchor.width_in_pixels: self.curr_codebox_anchor.frame_width += 10
        else: self.curr_codebox_anchor.frame_width += 3
        self.codebox_apply_width_height(self.curr_codebox_anchor, True)

    def codebox_decrease_width(self, *args):
        """Increase CodeBox Width"""
        if self.curr_codebox_anchor.width_in_pixels:
            if self.curr_codebox_anchor.frame_width - 10 >= 10:
                self.curr_codebox_anchor.frame_width -= 10
                self.codebox_apply_width_height(self.curr_codebox_anchor, True)
        else:
            if self.curr_codebox_anchor.frame_width - 3 >= 10:
                self.curr_codebox_anchor.frame_width -= 3
                self.codebox_apply_width_height(self.curr_codebox_anchor, True)

    def codebox_increase_height(self, *args):
        """Increase CodeBox Width"""
        self.curr_codebox_anchor.frame_height += 10
        self.codebox_apply_width_height(self.curr_codebox_anchor, True)

    def codebox_decrease_height(self, *args):
        """Increase CodeBox Width"""
        if self.curr_codebox_anchor.frame_height - 10 >= 10:
            self.curr_codebox_anchor.frame_height -= 10
            self.codebox_apply_width_height(self.curr_codebox_anchor, True)

    def codebox_apply_width_height(self, anchor, from_shortcut=False):
        """Apply Width and Height Changes to CodeBox"""
        if anchor.width_in_pixels: frame_width = anchor.frame_width
        else: frame_width = self.dad.get_text_window_width()*anchor.frame_width/100
        anchor.frame.set_size_request(frame_width, anchor.frame_height)
        if from_shortcut:
            self.dad.update_window_save_needed()
            self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])

    def codebox_change_properties(self, action):
        """Change CodeBox Properties"""
        self.dad.user_active = False
        self.dad.glade.spinbutton_codebox_width.set_value(self.curr_codebox_anchor.frame_width)
        self.dad.glade.spinbutton_codebox_height.set_value(self.curr_codebox_anchor.frame_height)
        self.dad.glade.radiobutton_codebox_pixels.set_active(self.curr_codebox_anchor.width_in_pixels)
        self.dad.glade.radiobutton_codebox_percent.set_active(not self.curr_codebox_anchor.width_in_pixels)
        self.dad.glade.checkbutton_codebox_matchbrackets.set_active(self.curr_codebox_anchor.highlight_brackets)
        self.dad.glade.checkbutton_codebox_linenumbers.set_active(self.curr_codebox_anchor.show_line_numbers)
        self.dad.glade.combobox_prog_lang_codebox.set_active_iter(self.dad.get_combobox_prog_lang_iter(self.curr_codebox_anchor.syntax_highlighting))
        self.dad.glade.codeboxhandledialog.set_title(_("Edit CodeBox"))
        self.dad.user_active = True
        response = self.dad.glade.codeboxhandledialog.run()
        self.dad.glade.codeboxhandledialog.hide()
        if response != 1: return # the user aborted the operation
        self.curr_codebox_anchor.syntax_highlighting = self.dad.prog_lang_liststore[self.dad.glade.combobox_prog_lang_codebox.get_active_iter()][1]
        self.dad.set_sourcebuffer_syntax_highlight(self.curr_codebox_anchor.sourcebuffer, self.curr_codebox_anchor.syntax_highlighting)
        self.curr_codebox_anchor.frame_width = int(self.dad.glade.spinbutton_codebox_width.get_value())
        self.curr_codebox_anchor.frame_height = int(self.dad.glade.spinbutton_codebox_height.get_value())
        self.curr_codebox_anchor.width_in_pixels = self.dad.glade.radiobutton_codebox_pixels.get_active()
        self.curr_codebox_anchor.highlight_brackets = self.dad.glade.checkbutton_codebox_matchbrackets.get_active()
        self.curr_codebox_anchor.sourcebuffer.set_highlight_matching_brackets(self.curr_codebox_anchor.highlight_brackets)
        self.curr_codebox_anchor.show_line_numbers = self.dad.glade.checkbutton_codebox_linenumbers.get_active()
        self.curr_codebox_anchor.sourceview.set_show_line_numbers(self.curr_codebox_anchor.show_line_numbers)
        self.codebox_apply_width_height(self.curr_codebox_anchor)
        self.dad.update_window_save_needed()
        self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])

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
        self.dad.object_set_selection(self.curr_codebox_anchor)

    def on_sourceview_populate_popup_codebox(self, textview, menu, anchor):
        """Extend the Default Right-Click Menu of the CodeBox"""
        self.dad.menu_populate_popup(menu, cons.get_popup_menu_entries_codebox(self), self.dad.orphan_accel_group)
