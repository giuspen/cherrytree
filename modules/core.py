# -*- coding: UTF-8 -*-
#
#       core.py
#
#       Copyright 2009-2017 Giuseppe Penone <giuspen@gmail.com>
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

import gtk
import pango
import gtksourceview2
import gobject
import sys
import os
import re
import glob
import subprocess
import webbrowser
import base64
import cgi
import urllib2
import shutil
import time
import pgsc_spellcheck
import cons
import menus
import support
import config
import machines
import clipboard
import imports
import exports
import printing
import tablez
import lists
import findreplace
import codeboxes
import ctdb
if cons.HAS_APPINDICATOR: import appindicator

class CherryTree:
    """Application's GUI"""

    def __init__(self, lang_str, open_with_file, node_name, boss, is_startup, is_arg, export_mode):
        """GUI Startup"""
        self.boss = boss
        self.filetype = ""
        self.user_active = True
        self.ctrl_down = False
        self.window = gtk.Window()
        self.window.set_title("CherryTree")
        self.window.set_default_size(963, 630)
        self.window.set_icon_from_file(os.path.join(cons.GLADE_PATH, "cherrytree.png"))
        # instantiate external handlers
        self.clipboard_handler = clipboard.ClipboardHandler(self)
        self.lists_handler = lists.ListsHandler(self)
        self.tables_handler = tablez.TablesHandler(self)
        self.codeboxes_handler = codeboxes.CodeBoxesHandler(self)
        self.state_machine = machines.StateMachine(self)
        self.xml_handler = machines.XMLHandler(self)
        self.html_handler = exports.Export2Html(self)
        self.ctdb_handler = ctdb.CTDBHandler(self)
        self.print_handler = printing.PrintHandler(self)
        # icon factory
        factory = gtk.IconFactory()
        for stock_name in cons.STOCKS_N_FILES:
            filename = stock_name + ".png"
            filepath = os.path.join(cons.GLADE_PATH, filename)
            pixbuf = gtk.gdk.pixbuf_new_from_file(filepath)
            iconset = gtk.IconSet(pixbuf)
            factory.add(stock_name, iconset)
        factory.add_default()
        # system settings
        try:
            gtk_settings = gtk.settings_get_default()
            gtk_settings.set_property("gtk-button-images", True)
            gtk_settings.set_property("gtk-menu-images", True)
        except: pass # older gtk do not have the property "gtk-menu-images"
        os.environ['UBUNTU_MENUPROXY'] = '0' # cherrytree has custom stock icons not visible in appmenu
        vbox_main = gtk.VBox()
        self.window.add(vbox_main)
        self.country_lang = lang_str
        self.cursor_position = 0
        config.config_file_load(self)
        if not cons.HAS_APPINDICATOR: self.use_appind = False
        elif not cons.HAS_SYSTRAY: self.use_appind = True
        # ui manager
        actions = gtk.ActionGroup("Actions")
        actions.add_actions(menus.get_entries(self))
        self.ui = gtk.UIManager()
        self.ui.insert_action_group(actions, 0)
        self.window.add_accel_group(self.ui.get_accel_group())
        self.ui.add_ui_from_string(menus.UI_INFO)
        self.ui.add_ui_from_string(config.get_toolbar_ui_str(self))
        # menubar add
        vbox_main.pack_start(self.ui.get_widget("/MenuBar"), False, False)
        # toolbar add
        vbox_main.pack_start(self.ui.get_widget("/ToolBar"), False, False)
        # hpaned add
        self.hpaned = gtk.HPaned()
        self.scrolledwindow_tree = gtk.ScrolledWindow()
        self.scrolledwindow_tree.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.scrolledwindow_text = gtk.ScrolledWindow()
        self.scrolledwindow_text.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.scrolledwindow_text.get_hscrollbar().connect('value-changed', self.on_hscrollbar_text_value_changed)
        self.vbox_text = gtk.VBox()
        self.header_node_name_eventbox = self.instantiate_node_name_header()
        self.vbox_text.pack_start(self.header_node_name_eventbox, False, False)
        self.vbox_text.pack_start(self.scrolledwindow_text)
        if self.tree_right_side:
            self.hpaned.add1(self.vbox_text)
            self.hpaned.add2(self.scrolledwindow_tree)
        else:
            self.hpaned.add1(self.scrolledwindow_tree)
            self.hpaned.add2(self.vbox_text)
        vbox_main.pack_start(self.hpaned)
        # statusbar add
        self.statusbar = gtk.Statusbar()
        self.statusbar_context_id = self.statusbar.get_context_id('')
        self.latest_statusbar_update_time = {}
        self.progressbar = gtk.ProgressBar()
        progress_frame = gtk.Frame()
        progress_frame.set_shadow_type(gtk.SHADOW_NONE)
        progress_frame.set_border_width(1)
        progress_frame.add(self.progressbar)
        self.progresstop = gtk.Button()
        self.progresstop.set_image(gtk.image_new_from_stock("gtk-stop", gtk.ICON_SIZE_MENU))
        self.progresstop.connect('clicked', self.on_button_progresstop_clicked)
        hbox_statusbar = gtk.HBox()
        hbox_statusbar.pack_start(self.statusbar, True, True)
        hbox_statusbar.pack_start(progress_frame, False, True)
        hbox_statusbar.pack_start(self.progresstop, False, True)
        vbox_main.pack_start(hbox_statusbar, False, False)
        # ROW: 0-icon_stock_id, 1-name, 2-buffer, 3-unique_id, 4-syntax_highlighting, 5-node_sequence, 6-tags, 7-readonly, 8-aux_icon_stock_id, 9-custom_icon_id, 10-weight, 11-foreground, 12-ts_creation, 13-ts_lastsave
        self.treestore = gtk.TreeStore(str, str, gobject.TYPE_PYOBJECT, long, str, int, str, gobject.TYPE_BOOLEAN, str, int, int, str, float, float)
        self.treeview = gtk.TreeView(self.treestore)
        self.treeview.set_headers_visible(False)
        self.treeview.drag_source_set(gtk.gdk.BUTTON1_MASK,
                                      [('CT_DND', gtk.TARGET_SAME_WIDGET, 0)],
                                      gtk.gdk.ACTION_MOVE)
        self.treeview.drag_dest_set(gtk.DEST_DEFAULT_ALL,
                                    [('CT_DND', gtk.TARGET_SAME_WIDGET, 0)],
                                    gtk.gdk.ACTION_MOVE)
        self.renderer_pixbuf = gtk.CellRendererPixbuf()
        self.renderer_text = gtk.CellRendererText()
        self.renderer_text.set_property('wrap-mode', pango.WRAP_WORD_CHAR)
        self.aux_renderer_pixbuf = gtk.CellRendererPixbuf()
        self.column = gtk.TreeViewColumn()
        self.column.pack_start(self.renderer_pixbuf, False)
        self.column.pack_start(self.renderer_text, True)
        self.column.pack_start(self.aux_renderer_pixbuf, False)
        self.column.set_attributes(self.renderer_pixbuf, stock_id=0)
        self.column.set_attributes(self.renderer_text, text=1, weight=10, foreground=11)
        self.column.set_attributes(self.aux_renderer_pixbuf, stock_id=8)
        self.treeview.append_column(self.column)
        self.treeview.set_search_column(1)
        self.treeviewselection = self.treeview.get_selection()
        self.treeview.connect('cursor-changed', self.on_node_changed)
        self.treeview.connect('event-after', self.on_event_after_tree)
        self.tree_just_auto_expanded = False
        self.treeview.connect('test-collapse-row', self.on_test_collapse_row_tree)
        self.treeview.connect('button-press-event', self.on_mouse_button_clicked_tree)
        self.treeview.connect('key_press_event', self.on_key_press_cherrytree)
        self.treeview.connect('drag-motion', self.on_drag_motion_cherrytree)
        self.treeview.connect('drag-data-received', self.on_drag_data_recv_cherrytree)
        self.treeview.connect('drag-data-get', self.on_drag_data_get_cherrytree)
        self.scrolledwindow_tree.add(self.treeview)
        self.orphan_accel_group = gtk.AccelGroup()
        self.menu_tree_create()
        self.window.connect('delete-event', self.on_window_delete_event)
        self.window.connect("focus-out-event", self.on_window_focus_out_event)
        self.window.connect('window-state-event', self.on_window_state_event)
        self.window.connect("size-allocate", self.on_window_n_tree_size_allocate_event)
        self.window.connect('key_press_event', self.on_key_press_window)
        self.window.connect("destroy", self.boss.on_window_destroy_event)
        self.scrolledwindow_tree.connect("size-allocate", self.on_window_n_tree_size_allocate_event)
        self.sourcestyleschememanager = gtksourceview2.StyleSchemeManager()
        self.sourceview = gtksourceview2.View()
        self.sourceview.set_sensitive(False)
        self.sourceview.set_smart_home_end(gtksourceview2.SMART_HOME_END_AFTER)
        self.sourceview.connect('populate-popup', self.on_sourceview_populate_popup)
        self.sourceview.connect("event", self.on_sourceview_event)
        self.sourceview.connect("motion-notify-event", self.on_sourceview_motion_notify_event)
        self.sourceview.connect("visibility-notify-event", self.on_sourceview_visibility_notify_event)
        self.sourceview.connect("event-after", self.on_sourceview_event_after)
        self.sourceview.connect("copy-clipboard", self.clipboard_handler.copy, False)
        self.sourceview.connect("cut-clipboard", self.clipboard_handler.cut, False)
        self.sourceview.connect("paste-clipboard", self.clipboard_handler.paste)
        self.sourceview.set_left_margin(7)
        self.sourceview.set_right_margin(7)
        self.sourcebuffers = {}
        self.hovering_link_iter_offset = -1
        self.tag_table = gtk.TextTagTable()
        self.scrolledwindow_text.add(self.sourceview)
        self.progress_stop = False
        self.go_bk_fw_click = False
        self.highlighted_obj = None
        self.embfiles_opened = {}
        self.embfiles_sentinel_id = None
        self.codebox_sentinel_id = None
        self.bookmarks = []
        self.bookmarks_menu_items = []
        self.nodes_names_dict = {}
        self.password = None
        self.export_single = False
        self.last_include_node_name = True
        self.last_index_in_page = True
        self.last_new_node_page = False
        self.curr_tree_iter = None
        self.curr_window_n_tree_width = None
        self.curr_buffer = None
        self.node_add_is_duplication = False
        self.nodes_cursor_pos = {}
        self.links_entries = {'webs':'', 'file':'', 'fold':'', 'anch':'', 'node':None}
        self.tags_set = set()
        self.file_update = False
        self.anchor_size_mod = False
        self.embfile_size_mod = False
        self.embfile_show_filename_mod = False
        self.cursor_key_press = None
        self.autosave_timer_id = None
        self.spell_check_init = False
        self.writing_to_disk = False
        self.export_overwrite = False
        self.mod_time_sentinel_id = None
        self.mod_time_val = 0
        self.prefpage = 0
        support.set_menu_items_recent_documents(self)
        support.set_menu_items_special_chars(self)
        self.find_handler = findreplace.FindReplace(self)
        if not export_mode:
            self.window.show_all() # this before the config_file_apply that could hide something
            self.window.present()
        config.config_file_apply(self)
        self.combobox_prog_lang_init()
        if is_arg or not is_startup or self.reload_doc_last:
            self.file_startup_load(open_with_file, node_name)
        else: self.file_name = ""
        if self.tree_is_empty(): self.update_node_name_header()
        if self.systray:
            if not self.boss.systray_active:
                self.status_icon_enable()
            if self.start_on_systray: self.window.hide_all()
        else: self.ui.get_widget("/MenuBar/FileMenu/exit_app").set_property(cons.STR_VISIBLE, False)
        if self.check_version: self.check_for_newer_version()

    def on_button_progresstop_clicked(self, *args):
        """Progress Stop Button Clicked"""
        self.progress_stop = True
        self.progresstop.hide()

    def check_for_newer_version(self, *args):
        """Check for a Newer Version"""
        self.statusbar.pop(self.statusbar_context_id)
        self.statusbar.push(self.statusbar_context_id, _("Checking for Newer Version..."))
        while gtk.events_pending(): gtk.main_iteration()
        try:
            fd = urllib2.urlopen(cons.NEWER_VERSION_URL, timeout=3)
            latest_version = fd.read().replace(cons.CHAR_NEWLINE, "")
            if len(latest_version) > 10:
                # html error page
                raise
            splitted_latest_v = [int(i) for i in latest_version.split(".")]
            splitted_local_v = [int(i) for i in cons.VERSION.split(".")]
            weighted_latest_v = splitted_latest_v[0]*10000 + splitted_latest_v[1]*100 + splitted_latest_v[2]
            weighted_local_v = splitted_local_v[0]*10000 + splitted_local_v[1]*100 + splitted_local_v[2]
            if weighted_latest_v > weighted_local_v:
                support.dialog_info(_("A Newer Version Is Available!") + " (%s)" % latest_version, self.window)
                self.statusbar.pop(self.statusbar_context_id)
                self.update_selected_node_statusbar_info()
            else:
                self.statusbar.pop(self.statusbar_context_id)
                if weighted_latest_v == weighted_local_v:
                    msg_txt = _("You Are Using the Latest Version Available") + " (%s)" % latest_version
                else:
                    msg_txt = _("You Are Using a Development Version")
                self.statusbar.push(self.statusbar_context_id, msg_txt)
        except:
            self.statusbar.pop(self.statusbar_context_id)
            self.statusbar.push(self.statusbar_context_id, _("Failed to Retrieve Latest Version Information - Try Again Later"))

    def get_node_icon(self, node_level, node_code, custom_icon_id):
        """Returns the Stock Id given the Node Level"""
        if custom_icon_id:
            # overridden icon
            stock_id = cons.NODES_STOCKS[custom_icon_id]
        elif node_code in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            # text node
            if self.nodes_icons == "c":
                if node_level in cons.NODES_ICONS:
                    stock_id = cons.NODES_ICONS[node_level]
                else:
                    stock_id = cons.NODES_ICONS[-1]
            elif self.nodes_icons == "b":
                stock_id = cons.NODES_STOCKS[self.default_icon_text]
            else:
                stock_id = cons.NODES_STOCKS[cons.NODE_ICON_NO_ICON_ID]
        else:
            # code node
            if self.nodes_icons in ["c", "b"]:
                stock_id = config.get_stock_id_for_code_type(node_code)
            else:
                stock_id = cons.NODES_STOCKS[cons.NODE_ICON_NO_ICON_ID]
        return stock_id

    def text_selection_change_case(self, change_type):
        """Change the Case of the Selected Text/the Underlying Word"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if not self.is_curr_node_not_read_only_or_error(): return
        if not text_buffer.get_has_selection() and not support.apply_tag_try_automatic_bounds(self, text_buffer=text_buffer):
            support.dialog_warning(_("No Text is Selected"), self.window)
            return
        iter_start, iter_end = text_buffer.get_selection_bounds()
        if from_codebox or self.syntax_highlighting != cons.RICH_TEXT_ID:
            text_to_change_case = text_buffer.get_text(iter_start, iter_end)
            if change_type == "l": text_to_change_case = text_to_change_case.lower()
            elif change_type == "u": text_to_change_case = text_to_change_case.upper()
            elif change_type == "t": text_to_change_case = text_to_change_case.swapcase()
        else:
            rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(text_buffer,
                iter_start,
                iter_end,
                change_case=change_type)
        start_offset = iter_start.get_offset()
        end_offset = iter_end.get_offset()
        text_buffer.delete(iter_start, iter_end)
        iter_insert = text_buffer.get_iter_at_offset(start_offset)
        if from_codebox or self.syntax_highlighting != cons.RICH_TEXT_ID:
            text_buffer.insert(iter_insert, text_to_change_case)
        else:
            text_buffer.move_mark(text_buffer.get_insert(), iter_insert)
            self.clipboard_handler.from_xml_string_to_buffer(rich_text)
        text_buffer.select_range(text_buffer.get_iter_at_offset(start_offset),
                                 text_buffer.get_iter_at_offset(end_offset))

    def text_selection_toggle_case(self, *args):
        """Toggles the Case of the Selected Text/the Underlying Word"""
        self.text_selection_change_case("t")

    def text_selection_upper_case(self, *args):
        """Uppers the Case of the Selected Text/the Underlying Word"""
        self.text_selection_change_case("u")

    def text_selection_lower_case(self, *args):
        """Lowers the Case of the Selected Text/the Underlying Word"""
        self.text_selection_change_case("l")

    def text_row_selection_duplicate(self, *args):
        """Duplicates the Whole Row or a Selection"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if not self.is_curr_node_not_read_only_or_error(): return
        if text_buffer.get_has_selection():
            iter_start, iter_end = text_buffer.get_selection_bounds() # there's a selection
            sel_start_offset = iter_start.get_offset()
            sel_end_offset = iter_end.get_offset()
            if from_codebox or self.syntax_highlighting != cons.RICH_TEXT_ID:
                text_to_duplicate = text_buffer.get_text(iter_start, iter_end)
                if cons.CHAR_NEWLINE in text_to_duplicate:
                    text_to_duplicate = cons.CHAR_NEWLINE + text_to_duplicate
                text_buffer.insert(iter_end, text_to_duplicate)
            else:
                rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(text_buffer, iter_start, iter_end)
                if cons.CHAR_NEWLINE in rich_text:
                    text_buffer.insert(iter_end, cons.CHAR_NEWLINE)
                    iter_end = text_buffer.get_iter_at_offset(sel_end_offset+1)
                    text_buffer.move_mark(text_buffer.get_insert(), iter_end)
                self.clipboard_handler.from_xml_string_to_buffer(rich_text)
            text_buffer.select_range(text_buffer.get_iter_at_offset(sel_start_offset),
                                     text_buffer.get_iter_at_offset(sel_end_offset))
        else:
            cursor_offset = text_buffer.get_iter_at_mark(text_buffer.get_insert()).get_offset()
            iter_start, iter_end = self.lists_handler.get_paragraph_iters(text_buffer=text_buffer)
            if iter_start == None:
                iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
                text_buffer.insert(iter_start, cons.CHAR_NEWLINE)
            else:
                if from_codebox or self.syntax_highlighting != cons.RICH_TEXT_ID:
                    text_to_duplicate = text_buffer.get_text(iter_start, iter_end)
                    text_buffer.insert(iter_end, cons.CHAR_NEWLINE + text_to_duplicate)
                else:
                    rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(text_buffer, iter_start, iter_end)
                    sel_end_offset = iter_end.get_offset()
                    text_buffer.insert(iter_end, cons.CHAR_NEWLINE)
                    iter_end = text_buffer.get_iter_at_offset(sel_end_offset+1)
                    text_buffer.move_mark(text_buffer.get_insert(), iter_end)
                    self.clipboard_handler.from_xml_string_to_buffer(rich_text)
                    text_buffer.place_cursor(text_buffer.get_iter_at_offset(cursor_offset))
        self.state_machine.update_state()

    def text_row_up(self, *args):
        """Moves Up the Current Row/Selected Rows"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters(text_buffer=text_buffer)
        if iter_start == None:
            iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            iter_end = iter_start.copy()
        last_line_situation = False
        iter_end.forward_char()
        missing_leading_newline = False
        destination_iter = iter_start.copy()
        if not destination_iter.backward_char(): return
        if not destination_iter.backward_char(): missing_leading_newline = True
        else:
            while destination_iter.get_char() != cons.CHAR_NEWLINE:
                if not destination_iter.backward_char():
                    missing_leading_newline = True
                    break
        if not missing_leading_newline: destination_iter.forward_char()
        destination_offset = destination_iter.get_offset()
        start_offset = iter_start.get_offset()
        end_offset = iter_end.get_offset()
        #print "iter_start %s %s '%s'" % (start_offset, ord(iter_start.get_char()), iter_start.get_char())
        #print "iter_end %s %s '%s'" % (end_offset, ord(iter_end.get_char()), iter_end.get_char())
        #print "destination_iter %s %s '%s'" % (destination_offset, ord(destination_iter.get_char()), destination_iter.get_char())
        text_to_move = text_buffer.get_text(iter_start, iter_end)
        diff_offsets = end_offset - start_offset
        if from_codebox or self.syntax_highlighting != cons.RICH_TEXT_ID:
            text_buffer.delete(iter_start, iter_end)
            destination_iter = text_buffer.get_iter_at_offset(destination_offset)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE:
                diff_offsets += 1
                text_to_move += cons.CHAR_NEWLINE
            text_buffer.move_mark(text_buffer.get_insert(), destination_iter)
            text_buffer.insert(destination_iter, text_to_move)
            self.set_selection_at_offset_n_delta(destination_offset, diff_offsets-1, text_buffer=text_buffer)
        else:
            rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(text_buffer, iter_start,
iter_end, exclude_iter_sel_end=True)
            text_buffer.delete(iter_start, iter_end)
            destination_iter = text_buffer.get_iter_at_offset(destination_offset)
            if destination_offset > 0:
                # clear the newline from any tag
                clr_start_iter = text_buffer.get_iter_at_offset(destination_offset-1)
                text_buffer.remove_all_tags(clr_start_iter, destination_iter)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE:
                diff_offsets += 1
                append_newline = True
            else: append_newline = False
            text_buffer.move_mark(text_buffer.get_insert(), destination_iter)
            # trick of space to prevent subsequent text to take pasted text tag(s)
            text_buffer.insert_at_cursor(cons.CHAR_SPACE)
            destination_iter = text_buffer.get_iter_at_offset(destination_offset)
            text_buffer.move_mark(text_buffer.get_insert(), destination_iter)
            # write moved line
            self.clipboard_handler.from_xml_string_to_buffer(rich_text)
            if append_newline: text_buffer.insert_at_cursor(cons.CHAR_NEWLINE)
            # clear space trick
            cursor_iter = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            text_buffer.delete(cursor_iter, text_buffer.get_iter_at_offset(cursor_iter.get_offset()+1))
            # selection
            self.set_selection_at_offset_n_delta(destination_offset, diff_offsets-1, text_buffer=text_buffer)
        self.state_machine.update_state()

    def text_row_down(self, *args):
        """Moves Down the Current Row/Selected Rows"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters(text_buffer=text_buffer)
        if iter_start == None:
            iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            iter_end = iter_start.copy()
        if not iter_end.forward_char(): return
        missing_leading_newline = False
        destination_iter = iter_end.copy()
        while destination_iter.get_char() != cons.CHAR_NEWLINE:
            if not destination_iter.forward_char():
                missing_leading_newline = True
                break
        destination_iter.forward_char()
        destination_offset = destination_iter.get_offset()
        start_offset = iter_start.get_offset()
        end_offset = iter_end.get_offset()
        #print "iter_start %s %s '%s'" % (start_offset, ord(iter_start.get_char()), iter_start.get_char())
        #print "iter_end %s %s '%s'" % (end_offset, ord(iter_end.get_char()), iter_end.get_char())
        #print "destination_iter %s %s '%s'" % (destination_offset, ord(destination_iter.get_char()), destination_iter.get_char())
        text_to_move = text_buffer.get_text(iter_start, iter_end)
        diff_offsets = end_offset - start_offset
        if from_codebox or self.syntax_highlighting != cons.RICH_TEXT_ID:
            text_buffer.delete(iter_start, iter_end)
            destination_offset -= diff_offsets
            destination_iter = text_buffer.get_iter_at_offset(destination_offset)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE:
                diff_offsets += 1
                text_to_move += cons.CHAR_NEWLINE
            if missing_leading_newline:
                diff_offsets += 1
                text_to_move = cons.CHAR_NEWLINE + text_to_move
            text_buffer.insert(destination_iter, text_to_move)
            if not missing_leading_newline:
                self.set_selection_at_offset_n_delta(destination_offset, diff_offsets-1, text_buffer=text_buffer)
            else:
                self.set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2, text_buffer=text_buffer)
        else:
            rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(text_buffer,
 iter_start, iter_end, exclude_iter_sel_end=True)
            text_buffer.delete(iter_start, iter_end)
            destination_offset -= diff_offsets
            destination_iter = text_buffer.get_iter_at_offset(destination_offset)
            if destination_offset > 0:
                # clear the newline from any tag
                clr_start_iter = text_buffer.get_iter_at_offset(destination_offset-1)
                text_buffer.remove_all_tags(clr_start_iter, destination_iter)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE:
                diff_offsets += 1
                append_newline = True
            else: append_newline = False
            text_buffer.move_mark(text_buffer.get_insert(), destination_iter)
            if missing_leading_newline:
                diff_offsets += 1
                text_buffer.insert_at_cursor(cons.CHAR_NEWLINE)
            # trick of space to prevent subsequent text to take pasted text tag(s)
            text_buffer.insert_at_cursor(cons.CHAR_SPACE)
            destination_iter = text_buffer.get_iter_at_offset(destination_offset)
            text_buffer.move_mark(text_buffer.get_insert(), destination_iter)
            # write moved line
            self.clipboard_handler.from_xml_string_to_buffer(rich_text)
            if append_newline: text_buffer.insert_at_cursor(cons.CHAR_NEWLINE)
            # clear space trick
            cursor_iter = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            text_buffer.delete(cursor_iter, text_buffer.get_iter_at_offset(cursor_iter.get_offset()+1))
            # selection
            if not missing_leading_newline:
                self.set_selection_at_offset_n_delta(destination_offset, diff_offsets-1, text_buffer=text_buffer)
            else:
                self.set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2, text_buffer=text_buffer)
        self.state_machine.update_state()

    def get_text_view_n_buffer_codebox_proof(self):
        """Returns Tuple TextView, TextBuffer, Boolean checking if CodeBox in Use"""
        anchor = self.codeboxes_handler.codebox_in_use_get_anchor()
        if anchor:
            text_view = anchor.sourceview
            text_buffer = text_view.get_buffer()
            from_codebox = True
        else:
            text_buffer = self.curr_buffer
            text_view = self.sourceview
            from_codebox = False
        return text_view, text_buffer, from_codebox

    def text_row_delete(self, *args):
        """Deletes the Whole Row"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters(text_buffer=text_buffer)
        if iter_start == None:
            iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            iter_end = iter_start.copy()
        if not iter_end.forward_char() and not iter_start.backward_char(): return
        text_buffer.delete(iter_start, iter_end)
        self.state_machine.update_state()

    def text_row_cut(self, *args):
        """Cut a Whole Row"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters(text_buffer=text_buffer)
        if iter_start == None:
            iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            iter_end = iter_start.copy()
        if not iter_end.forward_char() and not iter_start.backward_char(): return
        text_buffer.select_range(iter_start, iter_end)
        text_view.emit("cut-clipboard")

    def text_row_copy(self, *args):
        """Copy a Whole Row"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters(text_buffer=text_buffer)
        if iter_start == None:
            iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            iter_end = iter_start.copy()
        if not iter_end.forward_char() and not iter_start.backward_char(): return
        text_buffer.select_range(iter_start, iter_end)
        text_view.emit("copy-clipboard")

    def on_hscrollbar_text_value_changed(self, hscrollbar):
        """Catches Text Horizontal Scrollbar Movements"""
        curr_val = hscrollbar.get_value()
        #print curr_val
        if curr_val == 7:
            hscrollbar.set_value(0)

    def on_key_press_window(self, widget, event):
        """Catches Window key presses"""
        if not self.curr_tree_iter: return
        keyname = gtk.gdk.keyval_name(event.keyval)
        if event.state & gtk.gdk.MOD1_MASK:
            if keyname == cons.STR_KEY_LEFT:
                self.go_back()
                return True
            elif keyname == cons.STR_KEY_RIGHT:
                self.go_forward()
                return True
        elif (event.state & gtk.gdk.CONTROL_MASK):
            if keyname == cons.STR_KEY_TAB:
                self.toggle_tree_text()
                return True
            elif keyname in ["plus", "KP_Add"]:
                if self.treeview.is_focus(): self.zoom_tree_p()
                else: self.zoom_text_p()
                return True
            elif keyname in ["minus", "KP_Subtract"]:
                if self.treeview.is_focus(): self.zoom_tree_m()
                else: self.zoom_text_m()
                return True
        return False

    def on_key_press_cherrytree(self, widget, event):
        """Catches Tree key presses"""
        if not self.curr_tree_iter: return False
        keyname = gtk.gdk.keyval_name(event.keyval)
        if event.state & gtk.gdk.SHIFT_MASK:
            if event.state & gtk.gdk.CONTROL_MASK:
                if keyname == cons.STR_KEY_RIGHT:
                    self.node_change_father()
                    return True
            elif keyname == cons.STR_KEY_UP:
                self.node_up()
                return True
            elif keyname == cons.STR_KEY_DOWN:
                self.node_down()
                return True
            elif keyname == cons.STR_KEY_LEFT:
                self.node_left()
                return True
            elif keyname == cons.STR_KEY_RIGHT:
                self.node_right()
                return True
        elif event.state & gtk.gdk.MOD1_MASK:
            pass
        elif event.state & gtk.gdk.CONTROL_MASK:
            if keyname == cons.STR_KEY_UP:
                prev_iter = self.get_tree_iter_prev_sibling(self.treestore, self.curr_tree_iter)
                if prev_iter:
                    while prev_iter:
                        move_iter = prev_iter
                        prev_iter = self.get_tree_iter_prev_sibling(self.treestore, prev_iter)
                    self.treeview_safe_set_cursor(move_iter)
                return True
            elif keyname == cons.STR_KEY_DOWN:
                next_iter = self.treestore.iter_next(self.curr_tree_iter)
                if next_iter:
                    while next_iter:
                        move_iter = next_iter
                        next_iter = self.treestore.iter_next(next_iter)
                    self.treeview_safe_set_cursor(move_iter)
                return True
            elif keyname == cons.STR_KEY_LEFT:
                father_iter = self.treestore.iter_parent(self.curr_tree_iter)
                if father_iter:
                    while father_iter:
                        move_iter = father_iter
                        father_iter = self.treestore.iter_parent(father_iter)
                    self.treeview_safe_set_cursor(move_iter)
                return True
            elif keyname == cons.STR_KEY_RIGHT:
                child_iter = self.treestore.iter_children(self.curr_tree_iter)
                if child_iter:
                    while child_iter:
                        move_iter = child_iter
                        child_iter = self.treestore.iter_children(child_iter)
                    self.treeview_safe_set_cursor(move_iter)
                return True
            elif keyname in ["plus", "KP_Add"]:
                self.zoom_tree_p()
                return True
            elif keyname in ["minus", "KP_Subtract"]:
                self.zoom_tree_m()
                return True
        else:
            if keyname == cons.STR_KEY_LEFT:
                self.treeview.collapse_row(self.treestore.get_path(self.curr_tree_iter))
                return True
            elif keyname == cons.STR_KEY_RIGHT:
                self.treeview.expand_row(self.treestore.get_path(self.curr_tree_iter), open_all=False)
                return True
            elif keyname == cons.STR_KEY_RETURN:
                self.toggle_tree_node_expanded_collapsed()
                return True
            elif keyname == cons.STR_KEY_DELETE:
                self.node_delete()
                return True
            elif keyname == cons.STR_KEY_MENU:
                self.node_menu_tree.popup(None, None, None, 0, event.time)
                return True
            elif keyname == cons.STR_KEY_TAB:
                self.toggle_tree_text()
                return True
        return False

    def zoom_tree_p(self):
        """Increase Tree Font"""
        font_vec = self.tree_font.split(cons.CHAR_SPACE)
        font_num = int(font_vec[-1])
        font_vec[-1] = str(font_num+1)
        self.tree_font = cons.CHAR_SPACE.join(font_vec)
        self.set_treeview_font()

    def zoom_tree_m(self):
        """Decrease Tree Font"""
        font_vec = self.tree_font.split(cons.CHAR_SPACE)
        font_num = int(font_vec[-1])
        if font_num > 6:
            font_vec[-1] = str(font_num-1)
            self.tree_font = cons.CHAR_SPACE.join(font_vec)
            self.set_treeview_font()

    def zoom_text_p(self):
        """Increase Text Font"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if from_codebox or self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            font_vec = self.code_font.split(cons.CHAR_SPACE)
            font_num = int(font_vec[-1])
            font_vec[-1] = str(font_num+1)
            self.code_font = cons.CHAR_SPACE.join(font_vec)
            if not from_codebox:
                self.sourceview.modify_font(pango.FontDescription(self.code_font))
            else:
                support.rich_text_node_modify_codeboxes_font(self.curr_buffer.get_start_iter(), self.code_font)
        else:
            font_vec = self.text_font.split(cons.CHAR_SPACE)
            font_num = int(font_vec[-1])
            font_vec[-1] = str(font_num+1)
            self.text_font = cons.CHAR_SPACE.join(font_vec)
            self.sourceview.modify_font(pango.FontDescription(self.text_font))

    def zoom_text_m(self):
        """Decrease Text Font"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if from_codebox or self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            font_vec = self.code_font.split(cons.CHAR_SPACE)
            font_num = int(font_vec[-1])
            if font_num > 6:
                font_vec[-1] = str(font_num-1)
                self.code_font = cons.CHAR_SPACE.join(font_vec)
                if not from_codebox:
                    self.sourceview.modify_font(pango.FontDescription(self.code_font))
                else:
                    support.rich_text_node_modify_codeboxes_font(self.curr_buffer.get_start_iter(), self.code_font)
        else:
            font_vec = self.text_font.split(cons.CHAR_SPACE)
            font_num = int(font_vec[-1])
            if font_num > 6:
                font_vec[-1] = str(font_num-1)
                self.text_font = cons.CHAR_SPACE.join(font_vec)
                self.sourceview.modify_font(pango.FontDescription(self.text_font))

    def fullscreen_toggle(self, *args):
        """Toggle Fullscreen State"""
        if (self.window.window.get_state() & gtk.gdk.WINDOW_STATE_FULLSCREEN):
            self.window.window.unfullscreen()
        else:
            self.window.window.fullscreen()

    def toggle_tree_node_expanded_collapsed(self, *args):
        """Toggle Selected Tree Node Expanded/Collapsed"""
        if not self.is_there_selected_node_or_error(): return
        if self.treeview.row_expanded(self.treestore.get_path(self.curr_tree_iter)):
            self.treeview.collapse_row(self.treestore.get_path(self.curr_tree_iter))
        else:
            self.treeview.expand_row(self.treestore.get_path(self.curr_tree_iter), open_all=False)

    def toggle_ena_dis_spellcheck(self, *args):
        """Toggle Enable/Disable Spell Check"""
        self.enable_spell_check = not self.enable_spell_check
        if self.enable_spell_check: self.spell_check_set_on()
        else: self.spell_check_set_off(True)

    def toggle_tree_text(self, *args):
        """Toggle Focus Between Tree and Text"""
        if self.treeview.is_focus():
            self.sourceview.grab_focus()
        else: self.treeview.grab_focus()

    def on_drag_motion_cherrytree(self, widget, drag_context, x, y, timestamp):
        """Cherry Tree drag motion"""
        if y < cons.TREE_DRAG_EDGE_PROX or y > (widget.get_allocation().height - cons.TREE_DRAG_EDGE_PROX):
            delta = -cons.TREE_DRAG_EDGE_SCROLL if y < cons.TREE_DRAG_EDGE_PROX else cons.TREE_DRAG_EDGE_SCROLL
            vscroll_obj = self.scrolledwindow_tree.get_vscrollbar()
            vscroll_obj.set_value(vscroll_obj.get_value() + delta)
        if x < cons.TREE_DRAG_EDGE_PROX or x > (widget.get_allocation().width - cons.TREE_DRAG_EDGE_PROX):
            delta = -cons.TREE_DRAG_EDGE_SCROLL if x < cons.TREE_DRAG_EDGE_PROX else cons.TREE_DRAG_EDGE_SCROLL
            hscroll_obj = self.scrolledwindow_tree.get_hscrollbar()
            hscroll_obj.set_value(hscroll_obj.get_value() + delta)
        drop_info = self.treeview.get_dest_row_at_pos(x, y)
        if drop_info:
            drop_path, drop_pos = drop_info
            widget.set_drag_dest_row(drop_path, drop_pos)
        return True

    def on_drag_data_recv_cherrytree(self, widget, drag_context, x, y, selection_data, info, timestamp):
        """Cherry Tree drag data received"""
        drop_info = self.treeview.get_dest_row_at_pos(x, y)
        if drop_info:
            drop_path, drop_pos = drop_info
            if not drop_pos: drop_pos = gtk.TREE_VIEW_DROP_BEFORE
            drop_iter = self.treestore.get_iter(drop_path)
            # check for bad drop
            if not self.drag_iter: return False
            drag_node_id = self.treestore[self.drag_iter][3]
            if self.treestore[drop_iter][3] == drag_node_id:
                print "drag node and drop node are the same"
                return False
            move_towards_top_iter = self.treestore.iter_parent(drop_iter)
            while move_towards_top_iter:
                if self.treestore[move_towards_top_iter][3] == drag_node_id:
                    support.dialog_error(_("The new parent can't be one of his children!"), self.window)
                    return False
                move_towards_top_iter = self.treestore.iter_parent(move_towards_top_iter)
            if drop_pos == gtk.TREE_VIEW_DROP_BEFORE:
                prev_iter = self.get_tree_iter_prev_sibling(self.treestore, drop_iter)
                # note: prev_iter could be None, use drop_iter to retrieve the parent
                self.node_move_after(self.drag_iter, self.treestore.iter_parent(drop_iter), prev_iter, True)
            elif drop_pos == gtk.TREE_VIEW_DROP_AFTER:
                self.node_move_after(self.drag_iter, self.treestore.iter_parent(drop_iter), drop_iter)
            else: # drop in
                self.node_move_after(self.drag_iter, drop_iter)
            if self.nodes_icons == "c": self.treeview_refresh(change_icon=True)
        return True

    def on_drag_data_get_cherrytree(self, widget, drag_context, selection_data, info, timestamp):
        """Cherry Tree drag data received"""
        tree_model, tree_iter = self.treeviewselection.get_selected()
        self.drag_iter = tree_iter
        selection_data.set("UTF8_STRING", 8, "fake") # without this, the drag_data_recv will not be called
        return True

    def nodes_add_from_cherrytree_file(self, action):
        """Appends Nodes at the Bottom of the Current Ones, Importing from a CherryTree File"""
        filepath = support.dialog_file_select(filter_pattern=["*.ct*"],
            filter_name=_("CherryTree Document"),
            curr_folder=self.pick_dir_import,
            parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        document_loaded_ok = False
        if filepath[-1] in ["d", "z"]:
            # xml
            cherrytree_string = self.file_get_cherrytree_data(filepath, False)
            if cherrytree_string: document_loaded_ok = True
            elif cherrytree_string == None: return # no error exit
        elif filepath[-1] in ["b", "x"]:
            # db
            source_db = self.file_get_cherrytree_data(filepath, False)
            if source_db: document_loaded_ok = True
            elif source_db == None: return # no error exit
        if document_loaded_ok:
            try:
                if filepath[-1] in ["d", "z"]:
                    self.nodes_add_from_cherrytree_data(cherrytree_string)
                else: self.nodes_add_from_cherrytree_data("", source_db)
                document_loaded_ok = True
            except: document_loaded_ok = False
        if not document_loaded_ok:
            support.dialog_error(_('"%s" is Not a CherryTree Document') % filepath, self.window)
            return

    def nodes_add_from_notecase_file(self, action):
        """Add Nodes Parsing a NoteCase File"""
        filepath = support.dialog_file_select(filter_pattern=["*.ncd"],
            filter_name=_("NoteCase Document"),
            curr_folder=self.pick_dir_import,
            parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        try:
            file_descriptor = open(filepath, 'r')
            notecase_string = file_descriptor.read()
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        notecase = imports.NotecaseHandler(self)
        cherrytree_string = notecase.get_cherrytree_xml(notecase_string)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_tuxcards_file(self, action):
        """Add Nodes Parsing a TuxCards File"""
        filepath = support.dialog_file_select(curr_folder=self.pick_dir_import, parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        try:
            file_descriptor = open(filepath, 'r')
            tuxcards_string = re.sub(cons.BAD_CHARS, "", file_descriptor.read())
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        tuxcards = imports.TuxCardsHandler(self)
        cherrytree_string = tuxcards.get_cherrytree_xml(tuxcards_string)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_keepnote_folder(self, action):
        """Add Nodes Parsing a KeepNote Folder"""
        folderpath = support.dialog_folder_select(curr_folder=self.pick_dir_import, parent=self.window)
        if not folderpath: return
        self.pick_dir_import = os.path.dirname(folderpath)
        keepnote = imports.KeepnoteHandler(self, folderpath)
        cherrytree_string = keepnote.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_zim_folder(self, action):
        """Add Nodes Parsing a Zim Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), "Notebooks", "Notes")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if not folderpath: return
        zim = imports.ZimHandler(self, folderpath)
        cherrytree_string = zim.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        zim.set_links_to_nodes(self)

    def nodes_add_from_gnote_folder(self, action):
        """Add Nodes Parsing a Gnote Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), ".local", "share", "gnote")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if not folderpath: return
        gnote = imports.TomboyHandler(self, folderpath)
        cherrytree_string = gnote.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        gnote.set_links_to_nodes(self)

    def nodes_add_from_rednotebook_folder(self, action):
        """Add Nodes Parsing a RedNotebook Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), ".rednotebook", "data")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if not folderpath: return
        rednotebook = imports.RedNotebookHandler(self, folderpath)
        cherrytree_string = rednotebook.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_tomboy_folder(self, action):
        """Add Nodes Parsing a Tomboy Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), ".local", "share", "tomboy")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if not folderpath: return
        tomboy = imports.TomboyHandler(self, folderpath)
        cherrytree_string = tomboy.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        tomboy.set_links_to_nodes(self)

    def nodes_add_from_basket_folder(self, action):
        """Add Nodes Parsing a Basket Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), ".kde", "share", "apps", "basket", "baskets")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if not folderpath: return
        basket = imports.BasketHandler(self, folderpath)
        if basket.check_basket_structure():
            cherrytree_string = basket.get_cherrytree_xml()
            self.nodes_add_from_cherrytree_data(cherrytree_string)
        else: support.dialog_error("%s is not a basket folder" % folderpath, self.window)

    def nodes_add_from_epim_html_file(self, action):
        """Add Nodes from Selected EPIM HTML File"""
        filepath = support.dialog_file_select(filter_pattern=["*.html", "*.HTML", "*.htm", "*.HTM"] if cons.IS_WIN_OS else [],
            filter_mime=["text/html"] if not cons.IS_WIN_OS else [],
            filter_name=_("EPIM HTML Document"),
            curr_folder=self.pick_dir_import, parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        html_folder = imports.epim_html_file_to_hier_files(filepath)
        if not html_folder: return
        html = imports.HTMLHandler(self)
        cherrytree_string = html.get_cherrytree_xml(folderpath=html_folder)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_html_file(self, action):
        """Add Nodes from Selected HTML File"""
        filepath = support.dialog_file_select(filter_pattern=["*.html", "*.HTML", "*.htm", "*.HTM"] if cons.IS_WIN_OS else [],
            filter_mime=["text/html"] if not cons.IS_WIN_OS else [],
            filter_name=_("HTML Document"),
            curr_folder=self.pick_dir_import, parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        html = imports.HTMLHandler(self)
        cherrytree_string = html.get_cherrytree_xml(filepath=filepath)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_html_folder(self, action):
        """Add Nodes from HTML File(s) in Selected Folder"""
        folderpath = support.dialog_folder_select(curr_folder=self.pick_dir_import, parent=self.window)
        if not folderpath: return
        self.pick_dir_import = os.path.dirname(folderpath)
        html = imports.HTMLHandler(self)
        cherrytree_string = html.get_cherrytree_xml(folderpath=folderpath)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_plain_text_file(self, action):
        """Add Nodes from Selected Plain Text File"""
        if not hasattr(self, "ext_plain_import"):
            self.ext_plain_import = "txt"
        if cons.IS_WIN_OS:
            ext_plain_import = support.dialog_img_n_entry(self.window, _("Plain Text Document"), self.ext_plain_import, gtk.STOCK_FILE)
            if not ext_plain_import: return
            self.ext_plain_import = ext_plain_import
            filter_pattern = ["*."+self.ext_plain_import.lower(), "*."+self.ext_plain_import.upper()]
        else:
            filter_pattern = []
        filepath = support.dialog_file_select(filter_pattern=filter_pattern,
            filter_mime=["text/*"] if not cons.IS_WIN_OS else [],
            filter_name=_("Plain Text Document"),
            curr_folder=self.pick_dir_import, parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        plain = imports.PlainTextHandler(self)
        cherrytree_string = plain.get_cherrytree_xml(filepath=filepath)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_plain_text_folder(self, action):
        """Add Nodes from Plain Text File(s) in Selected Folder"""
        if not hasattr(self, "ext_plain_import"):
            self.ext_plain_import = "txt"
        if cons.IS_WIN_OS:
            ext_plain_import = support.dialog_img_n_entry(self.window, _("Plain Text Document"), self.ext_plain_import, gtk.STOCK_FILE)
            if not ext_plain_import: return
            self.ext_plain_import = ext_plain_import
        folderpath = support.dialog_folder_select(curr_folder=self.pick_dir_import, parent=self.window)
        if not folderpath: return
        self.pick_dir_import = os.path.dirname(folderpath)
        plain = imports.PlainTextHandler(self)
        cherrytree_string = plain.get_cherrytree_xml(folderpath=folderpath)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_treepad_file(self, action):
        """Add Nodes Parsing a Treepad File"""
        filepath = support.dialog_file_select(filter_pattern=["*.hjt"],
            filter_name=_("Treepad Document"),
            curr_folder=self.pick_dir_import,
            parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        try:
            file_descriptor = open(filepath, 'rb')
            treepad_string = file_descriptor.read()
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        treepad = imports.TreepadHandler()
        cherrytree_string = treepad.get_cherrytree_xml(support.auto_decode_str(treepad_string))
        file_descriptor.close()
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_keynote_file(self, action):
        """Add Nodes Parsing a Keynote File"""
        filepath = support.dialog_file_select(filter_pattern=["*.knt"],
            filter_name=_("KeyNote Document"),
            curr_folder=self.pick_dir_import,
            parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        try:
            file_descriptor = open(filepath, 'r')
            keynote = imports.KeynoteHandler(self)
            cherrytree_string = keynote.get_cherrytree_xml(file_descriptor)
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        keynote.set_links_to_nodes(self)

    def nodes_add_from_mempad_file(self, action):
        """Add Nodes Parsing a Mempad File"""
        filepath = support.dialog_file_select(filter_pattern=["*.lst"],
            filter_name=_("Mempad Document"),
            curr_folder=self.pick_dir_import,
            parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        try:
            file_descriptor = open(filepath, 'r')
            mempad = imports.MempadHandler()
            cherrytree_string = mempad.get_cherrytree_xml(file_descriptor)
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_knowit_file(self, action):
        """Add Nodes Parsing a Knowit File"""
        filepath = support.dialog_file_select(filter_pattern=["*.kno"],
            filter_name=_("Knowit Document"),
            curr_folder=self.pick_dir_import,
            parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        knowit = imports.KnowitHandler(self)
        try:
            file_descriptor = open(filepath, 'r')
            cherrytree_string = knowit.get_cherrytree_xml(file_descriptor)
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        knowit.set_links_to_nodes(self)

    def nodes_add_from_leo_file(self, action):
        """Add Nodes Parsing a Leo File"""
        filepath = support.dialog_file_select(filter_pattern=["*.leo"],
            filter_name=_("Leo Document"),
            curr_folder=self.pick_dir_import,
            parent=self.window)
        if not filepath: return
        self.pick_dir_import = os.path.dirname(filepath)
        try:
            file_descriptor = open(filepath, 'r')
            leo_string = file_descriptor.read()
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        leo = imports.LeoHandler()
        cherrytree_string = leo.get_cherrytree_xml(leo_string)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_cherrytree_data(self, cherrytree_string, cherrytree_db=None):
        """Adds Nodes to the Tree Parsing a CherryTree XML String / CherryTree DB"""
        if self.user_active:
            self.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        file_loaded = False
        former_node = self.curr_tree_iter # we'll restore after the import
        tree_father = None # init the value of the imported nodes father
        if self.curr_tree_iter:
            self.nodes_cursor_pos[self.treestore[self.curr_tree_iter][3]] = self.curr_buffer.get_property(cons.STR_CURSOR_POSITION)
            if self.curr_buffer.get_modified() == True:
                self.file_update = True
                self.curr_buffer.set_modified(False)
                self.state_machine.update_state()
            dialog = gtk.Dialog(title=_("Who is the Parent?"),
                                parent=self.window,
                                flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                                buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                                         gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
            dialog.set_size_request(350, -1)
            dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
            radiobutton_root = gtk.RadioButton(label=_("The Tree Root"))
            radiobutton_curr_node = gtk.RadioButton(label=_("The Selected Node"))
            radiobutton_curr_node.set_group(radiobutton_root)
            content_area = dialog.get_content_area()
            content_area.pack_start(radiobutton_root)
            content_area.pack_start(radiobutton_curr_node)
            content_area.show_all()
            response = dialog.run()
            if radiobutton_curr_node.get_active(): tree_father = self.curr_tree_iter
            dialog.destroy()
            if response != gtk.RESPONSE_ACCEPT:
                if user_active_restore: self.user_active = True
                return
        try:
            if not cherrytree_db:
                cherrytree_string = re.sub(cons.BAD_CHARS, "", cherrytree_string)
                if self.xml_handler.dom_to_treestore(cherrytree_string, discard_ids=True,
                                                     tree_father=tree_father):
                    file_loaded = True
            else:
                self.ctdb_handler.read_db_full(cherrytree_db, discard_ids=True,
                                               tree_father=tree_father)
                cherrytree_db.close()
                file_loaded = True
        except: raise
        if file_loaded:
            self.update_window_save_needed()
            if not former_node: former_node = self.treestore.get_iter_first()
            if former_node:
                self.curr_tree_iter = former_node
                self.curr_buffer = self.treestore[self.curr_tree_iter][2]
                self.sourceview.set_buffer(None)
                self.sourceview.set_buffer(self.curr_buffer)
                self.syntax_highlighting = self.treestore[self.curr_tree_iter][4]
                self.curr_buffer.connect('modified-changed', self.on_modified_changed)
                if self.syntax_highlighting == cons.RICH_TEXT_ID:
                    self.curr_buffer.connect('insert-text', self.on_text_insertion)
                    self.curr_buffer.connect('delete-range', self.on_text_removal)
                    self.sourceview.modify_font(pango.FontDescription(self.text_font))
                elif self.syntax_highlighting == cons.PLAIN_TEXT_ID:
                    self.sourceview.modify_font(pango.FontDescription(self.text_font))
                else:
                    self.sourceview.modify_font(pango.FontDescription(self.code_font))
                self.sourceview.set_sensitive(True)
                self.update_node_name_header()
                self.state_machine.node_selected_changed(self.treestore[self.curr_tree_iter][3])
                self.objects_buffer_refresh()
                # try to restore cursor position if in memory
                if self.treestore[former_node][3] in self.nodes_cursor_pos:
                    self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(self.nodes_cursor_pos[self.treestore[former_node][3]]))
                    self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
        else: support.dialog_error('Error Parsing the CherryTree File', self.window)
        if user_active_restore: self.user_active = True

    def nodes_expand_all(self, action):
        """Expand all Tree Nodes"""
        self.treeview.expand_all()

    def nodes_collapse_all(self, action):
        """Collapse all Tree Nodes"""
        self.treeview.collapse_all()

    def on_sourceview_populate_popup(self, textview, menu):
        """Extend the Default Right-Click Menu"""
        if self.syntax_highlighting == cons.RICH_TEXT_ID:
            for menuitem in menu.get_children():
                try:
                    if menuitem.get_image().get_property("stock") == "gtk-paste":
                        menuitem.set_sensitive(True)
                except: pass
            if self.hovering_link_iter_offset >= 0:
                target_iter = self.curr_buffer.get_iter_at_offset(self.hovering_link_iter_offset)
                if target_iter:
                    do_set_cursor = True
                    if self.curr_buffer.get_has_selection():
                        iter_sel_start, iter_sel_end = self.curr_buffer.get_selection_bounds()
                        if self.hovering_link_iter_offset >= iter_sel_start.get_offset()\
                        and self.hovering_link_iter_offset <= iter_sel_end.get_offset():
                            do_set_cursor = False
                    if do_set_cursor: self.curr_buffer.place_cursor(target_iter)
                self.menu_populate_popup(menu, menus.get_popup_menu_entries_link(self))
            else: self.menu_populate_popup(menu, menus.get_popup_menu_entries_text(self))
        else: self.menu_populate_popup(menu, menus.get_popup_menu_entries_code(self))

    def menu_populate_popup(self, menu, entries, accel_group=None):
        """Populate the given menu with the given entries"""
        if not accel_group: accel_group = self.ui.get_accel_group()
        curr_submenu = None
        for attributes in entries:
            if attributes[0] == "separator":
                menu_item = gtk.SeparatorMenuItem()
                if curr_submenu: curr_submenu.append(menu_item)
                else: menu.append(menu_item)
            elif attributes[0] == "submenu-start":
                curr_submenu = gtk.Menu()
                menu_item = gtk.ImageMenuItem(attributes[1])
                menu_item.set_image(gtk.image_new_from_stock(attributes[2], gtk.ICON_SIZE_MENU))
                menu_item.set_submenu(curr_submenu)
                menu.append(menu_item)
            elif attributes[0] == "submenu-end":
                curr_submenu = None
                continue
            else:
                menu_item = gtk.ImageMenuItem(attributes[1])
                menu_item.connect('activate', attributes[4])
                menu_item.set_image(gtk.image_new_from_stock(attributes[0], gtk.ICON_SIZE_MENU))
                menu_item.set_tooltip_text(attributes[3])
                if attributes[2]:
                    key, mod = gtk.accelerator_parse(attributes[2])
                    for element in [cons.STR_KEY_UP, cons.STR_KEY_DOWN, cons.STR_KEY_LEFT, cons.STR_KEY_RIGHT, cons.STR_KEY_DELETE]:
                        if element in attributes[2]:
                            accel_group = self.orphan_accel_group
                            break
                    menu_item.add_accelerator("activate", accel_group, key, mod, gtk.ACCEL_VISIBLE)
                if curr_submenu:
                    curr_submenu.append(menu_item)
                    if attributes[0] == "horizontal_rule":
                        menu_item.show()
                        special_menu = gtk.Menu()
                        for special_char in self.special_chars:
                            menu_item = gtk.MenuItem(special_char)
                            menu_item.connect("activate", support.insert_special_char, special_char, self)
                            menu_item.show()
                            special_menu.append(menu_item)
                        special_menuitem = gtk.ImageMenuItem(_("Insert _Special Character"))
                        special_menuitem.set_image(gtk.image_new_from_stock("insert", gtk.ICON_SIZE_MENU))
                        special_menuitem.set_tooltip_text(_("Insert a Special Character"))
                        special_menuitem.set_submenu(special_menu)
                        curr_submenu.append(special_menuitem)
                        special_menuitem.show()
                    else:
                        #print attributes[0]
                        pass
                else: menu.append(menu_item)
            menu_item.show()

    def menu_tree_create(self):
        """Create the Tree Menus"""
        self.node_menu_tree = gtk.Menu()
        self.top_menu_tree = self.ui.get_widget("/MenuBar/TreeMenu").get_submenu()
        for menuitem in self.top_menu_tree:
            self.top_menu_tree.remove(menuitem)
        self.menu_populate_popup(self.top_menu_tree, menus.get_popup_menu_tree(self))
        self.menu_populate_popup(self.node_menu_tree, menus.get_popup_menu_tree(self))

    def menu_tree_update_for_bookmarked_node(self, is_bookmarked):
        """Update Tree Menu according to Node in Bookmarks or Not"""
        self.header_node_name_icon_pin.set_property(cons.STR_VISIBLE, is_bookmarked)
        for menu_tree in [self.top_menu_tree, self.node_menu_tree]:
            for menuitem in menu_tree:
                try:
                    if menuitem.get_image().get_property("stock") == "pin-add":
                        if is_bookmarked: menuitem.hide()
                        else: menuitem.show()
                    if menuitem.get_image().get_property("stock") == "pin-remove":
                        if not is_bookmarked: menuitem.hide()
                        else: menuitem.show()
                except: pass

    def on_window_state_event(self, window, event):
        """Catch Window's Events"""
        if event.changed_mask & gtk.gdk.WINDOW_STATE_MAXIMIZED:
            if event.new_window_state & gtk.gdk.WINDOW_STATE_MAXIMIZED:
                # the window was maximized
                self.win_is_maximized = True
            else:
                # the window was unmaximized
                self.win_is_maximized = False

    def dialog_preferences(self, *args):
        """Opens the Preferences Dialog"""
        self.combobox_country_lang_init()
        self.combobox_style_scheme_init()
        if self.spell_check_init: self.combobox_spell_check_lang_init()
        config.dialog_preferences(self)
        config.config_file_save(self)

    def autosave_timer_start(self):
        """Start Autosave Timer"""
        self.autosave_timer_id = gobject.timeout_add(self.autosave[1]*1000*60, self.autosave_timer_iter)

    def autosave_timer_stop(self):
        """Stop Autosave Timer"""
        gobject.source_remove(self.autosave_timer_id)
        self.autosave_timer_id = None

    def autosave_timer_iter(self):
        """Iteration of the Autosave"""
        if not self.tree_is_empty() and self.user_active:
            print "autosave iter"
            self.file_save()
        else: print "autosave skip"
        return True # this way we keep the timer alive

    def modification_time_sentinel_start(self):
        """Start Timer that checks for modification time"""
        self.mod_time_sentinel_id = gobject.timeout_add(5*1000, self.modification_time_sentinel_iter) # 5 sec

    def modification_time_sentinel_stop(self):
        """Stop Timer that checks for modification time"""
        gobject.source_remove(self.mod_time_sentinel_id)
        self.mod_time_sentinel_id = None

    def modification_time_sentinel_iter(self):
        """Iteration of the Modification Time Sentinel"""
        if self.file_name and self.mod_time_val:
            file_path = os.path.join(self.file_dir, self.file_name)
            if os.path.isfile(file_path) and not self.writing_to_disk:
                read_mod_time = os.path.getmtime(file_path)
                #print "former modified: %s (%s)" % (time.ctime(self.mod_time_val), self.mod_time_val)
                #print "last modified: %s (%s)" % (time.ctime(read_mod_time), read_mod_time)
                if read_mod_time > self.mod_time_val:
                    curr_sys_time = time.time()
                    print curr_sys_time, read_mod_time, curr_sys_time-read_mod_time
                    if curr_sys_time - read_mod_time > 2:
                        self.filepath_open(file_path, force_reset=True)
                        self.statusbar.pop(self.statusbar_context_id)
                        self.statusbar.push(self.statusbar_context_id, _("The Document was Reloaded After External Update to CT* File"))
        return True # this way we keep the timer alive

    def modification_time_update_value(self, mtime):
        """Update Value of Modification Time Sentinel"""
        file_path = os.path.join(self.file_dir, self.file_name)
        if os.path.isfile(file_path): self.mod_time_val = os.path.getmtime(file_path) if mtime else 0

    def status_icon_enable(self):
        """Creates the Stats Icon"""
        self.boss.systray_active = True
        if self.use_appind:
            self.boss.ind = appindicator.Indicator(cons.APP_NAME, "indicator-messages", appindicator.CATEGORY_APPLICATION_STATUS)
            self.boss.ind.set_icon_theme_path(cons.GLADE_PATH)
            self.boss.ind.set_status(appindicator.STATUS_ACTIVE)
            self.boss.ind.set_attention_icon("indicator-messages-new")
            for icp in ["/usr/share/icons/hicolor/scalable/apps/cherrytree.svg", "/usr/local/share/icons/hicolor/scalable/apps/cherrytree.svg", "glade/cherrytree.png"]:
                if os.path.isfile(icp):
                    icon_path = icp
                    break
            else: icon_path = cons.APP_NAME
            self.boss.ind.set_icon(icon_path)
            self.boss.ind.set_menu(self.ui.get_widget("/SysTrayMenu"))
        else:
            self.boss.status_icon = gtk.StatusIcon()
            self.boss.status_icon.set_from_stock(cons.APP_NAME)
            self.boss.status_icon.connect('button-press-event', self.on_mouse_button_clicked_systray)
            self.boss.status_icon.set_tooltip(_("CherryTree Hierarchical Note Taking"))

    def toggle_show_hide_main_window(self, *args):
        self.ui.get_widget("/SysTrayMenu").hide()
        while gtk.events_pending(): gtk.main_iteration()
        do_show = True
        for runn_win in self.boss.running_windows:
            if runn_win.window.has_toplevel_focus():
                do_show = False
                break
        for runn_win in self.boss.running_windows:
            if do_show:
                runn_win.window.show_all()
                runn_win.window.deiconify()
                runn_win.window.present()
                config.config_file_apply(runn_win)
                runn_win.window.move(runn_win.win_position[0], runn_win.win_position[1])
                #print "restored position", runn_win.win_position[0], runn_win.win_position[1]
            else:
                runn_win.win_position = runn_win.window.get_position()
                config.config_file_save(runn_win)
                runn_win.window.hide_all()

    def on_mouse_button_clicked_systray(self, widget, event):
        """Catches mouse buttons clicks upon the system tray icon"""
        if event.button == 1: self.toggle_show_hide_main_window()
        elif event.button == 3: self.ui.get_widget("/SysTrayMenu").popup(None, None, None, event.button, event.time)
        return False

    def node_id_get(self):
        """Returns the node_ids, all Different Each Other"""
        new_node_id = 1
        while self.get_tree_iter_from_node_id(new_node_id) or new_node_id in self.ctdb_handler.nodes_to_rm_set:
            new_node_id += 1
        return new_node_id

    def set_treeview_font(self):
        """Update the TreeView Font"""
        self.renderer_text.set_property('font-desc', pango.FontDescription(self.tree_font))
        self.treeview_refresh()

    def treeview_refresh(self, change_icon=False):
        """Refresh the Treeview"""
        if self.curr_buffer:
            cell_area = self.treeview.get_cell_area(self.treestore.get_path(self.curr_tree_iter),
                                                    self.treeview.get_columns()[0])
            config.get_tree_expanded_collapsed_string(self)
            self.treeview.set_model(None)
            if change_icon:
                tree_iter = self.treestore.get_iter_first()
                while tree_iter:
                    self.change_icon_iter(tree_iter)
                    tree_iter = self.treestore.iter_next(tree_iter)
            self.treeview.set_model(self.treestore)
            if self.user_active: config.set_tree_expanded_collapsed_string(self)
            self.treeview.set_cursor(self.treestore.get_path(self.curr_tree_iter))
            self.treeview.scroll_to_point(0, cell_area.height/2)

    def change_icon_iter(self, tree_iter):
        """Changing all icons type - iter"""
        self.treestore[tree_iter][0] = self.get_node_icon(self.treestore.iter_depth(tree_iter),
                                                          self.treestore[tree_iter][4],
                                                          self.treestore[tree_iter][9])
        child_tree_iter = self.treestore.iter_children(tree_iter)
        while child_tree_iter:
            self.change_icon_iter(child_tree_iter)
            child_tree_iter = self.treestore.iter_next(child_tree_iter)

    def file_startup_load(self, open_with_file, node_name):
        """Try to load a file if there are the conditions"""
        #print "file_startup_load '%s' ('%s', '%s')" % (open_with_file, self.file_name, self.file_dir)
        if open_with_file:
            open_with_file = open_with_file.decode(sys.getfilesystemencoding()).encode(cons.STR_UTF8, cons.STR_IGNORE)
            self.file_name = os.path.basename(open_with_file)
            self.file_dir = os.path.dirname(open_with_file)
            #print "open_with_file -> file_name '%s', file_dir '%s'" % (self.file_name, self.file_dir)
        elif self.boss.running_windows:
            self.file_name = ""
            return
        if self.file_name and os.path.isfile(os.path.join(self.file_dir, self.file_name)):
            self.file_load(os.path.join(self.file_dir, self.file_name))
            self.modification_time_update_value(True)
            if self.rest_exp_coll == 1:
                self.treeview.expand_all()
            else:
                if self.rest_exp_coll == 2:
                    self.expanded_collapsed_string = ""
                config.set_tree_expanded_collapsed_string(self)
            # is there a node name to focus?
            if node_name:
                node_name = unicode(node_name, cons.STR_UTF8, cons.STR_IGNORE)
                node_name_iter = self.get_tree_iter_from_node_name(node_name, use_content=False)
                if node_name_iter:
                    self.node_path = self.treestore.get_path(node_name_iter)
                    self.cursor_position = 0
                else:
                    node_name_iter = self.get_tree_iter_from_node_name(node_name, use_content=True)
                    if node_name_iter:
                        self.node_path = self.treestore.get_path(node_name_iter)
                        self.cursor_position = 0
            # we try to restore the focused node
            config.set_tree_path_and_cursor_pos(self)
        else:
            if self.file_name:
                print "? not is_file '%s'" % (os.path.join(self.file_dir, self.file_name))
                self.file_name = ""
            self.update_window_save_not_needed()
        self.file_update = False

    def on_modified_changed(self, sourcebuffer):
        """When the modification flag is changed"""
        if self.user_active and sourcebuffer.get_modified() == True:
            self.update_window_save_needed("nbuf")

    def file_save_as(self, *args):
        """Save the file providing a new name"""
        if not self.is_tree_not_empty_or_error(): return
        if not support.dialog_choose_data_storage(self): return
        filename_hint = self.file_name[:-1] + self.filetype if len(self.file_name) > 4 else ""
        filepath = support.dialog_file_save_as(filename_hint,
                                               filter_pattern="*.ct" + self.filetype,
                                               filter_name=_("CherryTree Document"),
                                               curr_folder=self.file_dir,
                                               parent=self.window)
        restore_filetype = False
        if not filepath: restore_filetype = True
        self.modification_time_update_value(False)
        if not restore_filetype:
            filepath = self.filepath_extension_fix(filepath)
            if not self.file_write(filepath, first_write=True): restore_filetype = True
        if restore_filetype:
            # restore filetype previous dialog_choose_data_storage
            if len(self.file_name) > 4: self.filetype = self.file_name[-1]
        else:
            self.file_dir = os.path.dirname(filepath)
            self.file_name = os.path.basename(filepath)
            support.add_recent_document(self, filepath)
            self.update_window_save_not_needed()
            self.state_machine.update_state()
            self.objects_buffer_refresh()
        self.modification_time_update_value(True)
        self.ui.get_widget("/MenuBar/FileMenu/ct_vacuum").set_property(cons.STR_VISIBLE, self.filetype in ["b", "x"])

    def filepath_extension_fix(self, filepath):
        """Check a filepath to have the proper extension"""
        extension = ".ct" + self.filetype
        if len(filepath) < 4 or filepath[-4:] != extension: return filepath + extension
        return filepath

    def file_vacuum(self, *args):
        """Vacuum the DB"""
        if self.file_name and self.filetype in ["b", "x"]:
            self.file_update = True
            self.ctdb_handler.is_vacuum = True
            self.file_save()
            self.ctdb_handler.is_vacuum = False
        else: print "no document or not SQLite"

    def file_save(self, *args):
        """Save the file"""
        if self.file_name:
            if self.file_update or (self.curr_tree_iter != None and self.curr_buffer.get_modified() == True):
                self.modification_time_update_value(False)
                if self.is_tree_not_empty_or_error() \
                and self.file_write(os.path.join(self.file_dir, self.file_name), first_write=False):
                    self.update_window_save_not_needed()
                    self.state_machine.update_state()
                self.modification_time_update_value(True)
            else: print "no changes"
        else: self.file_save_as()

    def file_write_low_level(self, filepath, xml_string, first_write, exporting="", sel_range=None):
        """File Write Low Level (ctd, ctb, ctz, ctx)"""
        if self.password:
            if cons.IS_WIN_OS:
                drive, tail = os.path.splitdrive(os.path.dirname(filepath))
                if tail.startswith(cons.CHAR_BSLASH): tail = tail[1:]
            else:
                tail = os.path.dirname(filepath)
                if tail.startswith(cons.CHAR_SLASH): tail = tail[1:]
            tree_tmp_folder = os.path.join(cons.TMP_FOLDER, tail)
            if not os.path.isdir(tree_tmp_folder): os.makedirs(tree_tmp_folder)
            last_letter = "d" if xml_string else "b"
            filepath_tmp = os.path.join(tree_tmp_folder, os.path.basename(filepath[:-1] + last_letter))
        else:
            filepath_tmp = filepath
        if xml_string: file_descriptor = open(filepath_tmp, 'w')
        else:
            if first_write:
                if not exporting:
                    if "db" in dir(self) and self.db: self.db_old = self.db
                    self.db = self.ctdb_handler.new_db(filepath_tmp)
                    if "db_old" in dir(self) and self.db_old:
                        self.db_old.close()
                        del self.db_old
                elif exporting in ["a", "s", "n"]:
                    print "exporting", exporting
                    exp_db = self.ctdb_handler.new_db(filepath_tmp, exporting, sel_range)
                    exp_db.close()
            else: self.ctdb_handler.pending_data_write(self.db)
        if xml_string:
            file_descriptor.write(xml_string)
            file_descriptor.close()
            if first_write and not exporting:
                if "db" in dir(self) and self.db:
                    self.db.close()
                    del self.db
        if self.password:
            if cons.IS_WIN_OS:
                esc_tmp_folder = support.windows_cmd_prepare_path(tree_tmp_folder)
                esc_filepath = support.windows_cmd_prepare_path(filepath)
                esc_filepath_tmp = support.windows_cmd_prepare_path(filepath_tmp)
            else:
                esc_tmp_folder = re.escape(tree_tmp_folder)
                esc_filepath = re.escape(filepath)
                esc_filepath_tmp = re.escape(filepath_tmp)
            dot_tmp_existing = False
            if os.path.isfile(filepath+".tmp"):
                try: os.remove(filepath+".tmp")
                except: subprocess.call("rm %s.tmp" % esc_filepath, shell=True)
            if os.path.isfile(filepath):
                # old archive
                try:
                    shutil.move(filepath, filepath+".tmp")
                    dot_tmp_existing = True
                except:
                    if not subprocess.call("mv %s %s.tmp" % (esc_filepath, esc_filepath), shell=True):
                        dot_tmp_existing = True
                    else: subprocess.call("%s d %s" % (cons.SZA_PATH, esc_filepath), shell=True)
            bash_str = '%s a -p%s -w%s -mx1 -bd -y %s %s' % (cons.SZA_PATH,
                self.password,
                esc_tmp_folder,
                esc_filepath,
                esc_filepath_tmp)
            #print bash_str
            if not xml_string and not exporting: self.db.close()
            dest_file_size = 0
            for attempt_num in range(3):
                ret_code = subprocess.call(bash_str, shell=True)
                if os.path.isfile(filepath):
                    dest_file_size = os.path.getsize(filepath)
                    if dest_file_size > cons.MIN_CT_DOC_SIZE: break
            print "dest_file_size %s bytes" % dest_file_size
            if xml_string: os.remove(filepath_tmp)
            elif not exporting:
                self.db = self.ctdb_handler.get_connected_db_from_dbpath(filepath_tmp)
                self.ctdb_handler.remove_at_quit_set.add(filepath_tmp)
            if not ret_code and dest_file_size:
                # everything OK
                if dot_tmp_existing and os.path.isfile(filepath+".tmp"):
                    # remove temporary safety file
                    try: os.remove(filepath+".tmp")
                    except: subprocess.call("rm %s.tmp" % esc_filepath, shell=True)
            else:
                print "7za FAIL!!!"
                # spoiled file is worse than no file, this way the backups will not be spoiled
                if os.path.isfile(filepath):
                    try: os.remove(filepath)
                    except: subprocess.call("rm %s" % esc_filepath, shell=True)
                if dot_tmp_existing and os.path.isfile(filepath+".tmp"):
                    # restore file version from temporary safety file
                    try: shutil.move(filepath+".tmp", filepath)
                    except: subprocess.call("mv %s.tmp %s" % (esc_filepath, esc_filepath), shell=True)
                raise

    def backups_handling(self, filepath_orig):
        """Handler of backup before save"""
        filepath = filepath_orig + (self.backup_num-1)*cons.CHAR_TILDE
        while True:
            if os.path.isfile(filepath):
                if filepath.endswith(cons.CHAR_TILDE):
                    try: shutil.move(filepath, filepath + cons.CHAR_TILDE)
                    except: subprocess.call("mv %s %s~" % (re.escape(filepath), re.escape(filepath)), shell=True)
                else:
                    try: shutil.copy(filepath, filepath + cons.CHAR_TILDE)
                    except: subprocess.call("cp %s %s~" % (re.escape(filepath), re.escape(filepath)), shell=True)
            if filepath.endswith(cons.CHAR_TILDE): filepath = filepath[:-1]
            else: break

    def file_write(self, filepath, first_write):
        """File Write"""
        if not cons.IS_WIN_OS and not os.access(os.path.dirname(filepath), os.W_OK):
            support.dialog_error(_("You Have No Write Access to %s") % os.path.dirname(filepath), self.window)
            return False
        if self.filetype in ["d", "z"]:
            try: xml_string = self.xml_handler.treestore_to_dom()
            except:
                support.dialog_error("%s write failed - tree to xml" % filepath, self.window)
                raise
                return False
        else: xml_string = ""
        # backup before save new version
        if self.backup_copy: self.backups_handling(filepath)
        # if the filename is protected, we use unprotected type before compress and protect
        try:
            self.writing_to_disk = True
            self.statusbar.push(self.statusbar_context_id, _("Writing to Disk..."))
            while gtk.events_pending(): gtk.main_iteration()
            self.file_write_low_level(filepath, xml_string, first_write)
            self.statusbar.pop(self.statusbar_context_id)
            self.writing_to_disk = False
            return True
        except:
            if not os.path.isfile(filepath) and os.path.isfile(filepath + cons.CHAR_TILDE):
                try: os.rename(filepath + cons.CHAR_TILDE, filepath)
                except:
                    print "os.rename failed"
                    subprocess.call("mv %s~ %s" % (re.escape(filepath), re.escape(filepath)), shell=True)
            support.dialog_error("%s write failed - writing to disk" % filepath, self.window)
            raise
            self.writing_to_disk = False
            return False

    def file_open(self, *args):
        """Opens a dialog to browse for a cherrytree filepath"""
        filepath = support.dialog_file_select(filter_pattern=["*.ct*"],
            filter_name=_("CherryTree Document"),
            curr_folder=self.file_dir,
            parent=self.window)
        if not filepath: return
        #self.filepath_boss_open(filepath, "")
        self.filepath_open(filepath)

    def filepath_boss_open(self, filepath, nodename):
        """Daddy, please, open a document for me"""
        msg_server_to_core['p'] = filepath
        msg_server_to_core['n'] = nodename
        msg_server_to_core['f'] = 1

    def filepath_open(self, filepath, force_reset=False):
        """Opens an existing filepath"""
        config.get_tree_expanded_collapsed_string(self)
        old_file_name = self.file_name
        old_exp_coll_str = self.expanded_collapsed_string
        if self.curr_tree_iter:
            old_node_path_str = config.get_node_path_str_from_path(self.treestore.get_path(self.curr_tree_iter))
            old_cursor_pos = self.curr_buffer.get_property(cons.STR_CURSOR_POSITION)
        else:
            old_node_path_str = ""
            old_cursor_pos = 0
        if not self.reset(force_reset): return
        new_file_name = os.path.basename(filepath)
        if new_file_name != old_file_name:
            if new_file_name == self.expcollnam1:
                self.expanded_collapsed_string = self.expcollstr1
                self.node_path = config.get_node_path_from_str(self.expcollsel1)
                self.cursor_position = self.expcollcur1
                self.expcollnam1 = old_file_name
                self.expcollstr1 = old_exp_coll_str
                self.expcollsel1 = old_node_path_str
                self.expcollcur1 = old_cursor_pos
            elif new_file_name == self.expcollnam2:
                self.expanded_collapsed_string = self.expcollstr2
                self.node_path = config.get_node_path_from_str(self.expcollsel2)
                self.cursor_position = self.expcollcur2
                self.expcollnam2 = old_file_name
                self.expcollstr2 = old_exp_coll_str
                self.expcollsel2 = old_node_path_str
                self.expcollcur2 = old_cursor_pos
            elif new_file_name == self.expcollnam3:
                self.expanded_collapsed_string = self.expcollstr3
                self.node_path = config.get_node_path_from_str(self.expcollsel3)
                self.cursor_position = self.expcollcur3
                self.expcollnam3 = old_file_name
                self.expcollstr3 = old_exp_coll_str
                self.expcollsel3 = old_node_path_str
                self.expcollcur3 = old_cursor_pos
            else:
                self.expanded_collapsed_string = ""
                self.node_path = None
                self.memory_save_old_file_props(old_file_name, old_exp_coll_str, old_node_path_str, old_cursor_pos)
        self.file_load(filepath)
        self.modification_time_update_value(True)
        if self.rest_exp_coll == 1:
            self.treeview.expand_all()
        else:
            if self.rest_exp_coll == 2:
                self.expanded_collapsed_string = ""
            config.set_tree_expanded_collapsed_string(self)
        config.set_tree_path_and_cursor_pos(self)

    def folder_cfg_open(self, *args):
        """Open the Directory with Preferences Files"""
        self.external_folderpath_open(cons.CONFIG_DIR)

    def memory_save_old_file_props(self, old_file_name, old_exp_coll_str, old_node_path_str, old_cursor_pos):
        """Store properties of file that was just closed"""
        if not self.expcollnam1 or self.expcollnam1 == old_file_name:
            self.expcollnam1 = old_file_name
            self.expcollstr1 = old_exp_coll_str
            self.expcollsel1 = old_node_path_str
            self.expcollcur1 = old_cursor_pos
        elif not self.expcollnam2 or self.expcollnam2 == old_file_name:
            self.expcollnam2 = old_file_name
            self.expcollstr2 = old_exp_coll_str
            self.expcollsel2 = old_node_path_str
            self.expcollcur2 = old_cursor_pos
        elif not self.expcollnam3 or self.expcollnam3 == old_file_name:
            self.expcollnam3 = old_file_name
            self.expcollstr3 = old_exp_coll_str
            self.expcollsel3 = old_node_path_str
            self.expcollcur3 = old_cursor_pos
        else:
            self.expcollnam3 = self.expcollnam2
            self.expcollstr3 = self.expcollstr2
            self.expcollsel3 = self.expcollsel2
            self.expcollcur3 = self.expcollcur2
            self.expcollnam2 = self.expcollnam1
            self.expcollstr2 = self.expcollstr1
            self.expcollsel2 = self.expcollsel1
            self.expcollcur2 = self.expcollcur1
            self.expcollnam1 = old_file_name
            self.expcollstr1 = old_exp_coll_str
            self.expcollsel1 = old_node_path_str
            self.expcollcur1 = old_cursor_pos

    def is_7za_available(self):
        """Check 7za binary executable to be available"""
        ret_code = subprocess.call("%s" % cons.SZA_PATH, shell=True)
        if ret_code:
            support.dialog_error(_("Binary Executable '7za' Not Found, Check The Package 'p7zip-full' to be Installed Properly"), self.window)
            return False
        return True

    def dialog_insert_password(self, file_name):
        """Prompts a Dialog Asking for the File Password"""
        dialog = gtk.Dialog(title=_("Enter Password for %s") % file_name,
                            parent=self.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
        dialog.set_default_size(350, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        entry = gtk.Entry()
        entry.set_visibility(False)
        content_area = dialog.get_content_area()
        content_area.pack_start(entry)
        def on_key_press_enter_password_dialog(widget, event):
            if gtk.gdk.keyval_name(event.keyval) == cons.STR_KEY_RETURN:
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
            return False
        dialog.connect("key_press_event", on_key_press_enter_password_dialog)
        dialog.show_all()
        if not cons.IS_WIN_OS:
            the_window = dialog.get_window()
            the_window.focus(gtk.gdk.x11_get_server_time(the_window))
        dialog.present()
        response = dialog.run()
        passw = unicode(entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
        dialog.destroy()
        while gtk.events_pending(): gtk.main_iteration()
        if response != gtk.RESPONSE_ACCEPT: return ""
        return passw

    def file_get_cherrytree_data(self, filepath, main_file):
        """returns the cherrytree xml string or db descriptor given the filepath"""
        password_protected = False
        if filepath[-1] in ["z", "x"]:
            password_protected = True
            while True:
                password_str = self.dialog_insert_password(os.path.basename(filepath))
                if not password_str:
                    if self.tree_is_empty():
                        self.memory_save_old_file_props(self.file_name, self.expanded_collapsed_string, self.node_path or "", self.cursor_position)
                        self.file_name = ""
                        self.password = None
                    return None
                if main_file: self.password = password_str
                if not self.is_7za_available(): return None
                if cons.IS_WIN_OS:
                    drive, tail = os.path.splitdrive(os.path.dirname(filepath))
                    if tail.startswith(cons.CHAR_BSLASH): tail = tail[1:]
                else:
                    tail = os.path.dirname(filepath)
                    if tail.startswith(cons.CHAR_SLASH): tail = tail[1:]
                tree_tmp_folder = os.path.join(cons.TMP_FOLDER, tail)
                tree_tmp_folder_tmp = os.path.join(tree_tmp_folder, "tmp")
                if not os.path.isdir(tree_tmp_folder_tmp): os.makedirs(tree_tmp_folder_tmp)
                last_letter = "d" if filepath[-1] == "z" else "b"
                extracted_file_name = os.path.basename(filepath[:-1] + last_letter)
                filepath_tmp = os.path.join(tree_tmp_folder, extracted_file_name)
                filepath_tmp_tmp = os.path.join(tree_tmp_folder_tmp, extracted_file_name)
                if cons.IS_WIN_OS:
                    esc_tmp_folder = support.windows_cmd_prepare_path(tree_tmp_folder_tmp)
                    esc_filepath = support.windows_cmd_prepare_path(filepath)
                else:
                    esc_tmp_folder = re.escape(tree_tmp_folder_tmp)
                    esc_filepath = re.escape(filepath)
                bash_str = '%s e -p%s -w%s -bd -y -o%s %s' % (cons.SZA_PATH,
                    password_str,
                    esc_tmp_folder,
                    esc_tmp_folder,
                    esc_filepath)
                #print bash_str
                ret_code = subprocess.call(bash_str, shell=True)
                if ret_code != 0:
                    support.dialog_error(_('Wrong Password'), self.window)
                    continue
                if not os.path.isfile(filepath_tmp_tmp):
                    print "? the compressed file was renamed"
                    files_list = glob.glob(os.path.join(tree_tmp_folder_tmp, "*"+filepath_tmp_tmp[-4:]))
                    #print files_list
                    old_filepath_tmp_tmp = files_list[0]
                    for file_path in files_list:
                        if os.path.getmtime(file_path) > os.path.getmtime(old_filepath_tmp_tmp):
                            old_filepath_tmp_tmp = file_path
                    os.rename(old_filepath_tmp_tmp, filepath_tmp_tmp)
                if not os.path.isfile(filepath_tmp_tmp):
                    print "! cannot find extracted file", filepath_tmp_tmp
                    raise
                dest_file_size = os.path.getsize(filepath_tmp_tmp)
                if dest_file_size < cons.MIN_CT_DOC_SIZE:
                    print "? extracted file zero size"
                    support.dialog_error(_('Wrong Password'), self.window)
                else:
                    break # extraction successful
            if os.path.isfile(filepath_tmp):
                os.remove(filepath_tmp)
            shutil.copy(filepath_tmp_tmp, filepath_tmp)
            os.remove(filepath_tmp_tmp)
        elif filepath[-1] not in ["d", "b"]:
            print "bad filepath[-1]", filepath[-1]
            return False
        elif main_file: self.password = None
        if filepath[-1] in ["d", "z"]:
            try:
                if password_protected: file_descriptor = open(filepath_tmp, 'r')
                else: file_descriptor = open(filepath, 'rb')
                cherrytree_string = file_descriptor.read()
                file_descriptor.close()
                if password_protected: os.remove(filepath_tmp)
            except:
                print "error reading from plain text xml"
                raise
                return False
            return re.sub(cons.BAD_CHARS, "", cherrytree_string)
        else:
            try:
                if password_protected: db = self.ctdb_handler.get_connected_db_from_dbpath(filepath_tmp)
                else: db = self.ctdb_handler.get_connected_db_from_dbpath(filepath)
                if password_protected:
                    if filepath[-1] == "z": os.remove(filepath_tmp)
                    else: self.ctdb_handler.remove_at_quit_set.add(filepath_tmp)
            except:
                print "error connecting to db"
                raise
                return False
            return db

    def file_load(self, filepath):
        """Loads a .CTD into a GTK TreeStore"""
        document_loaded_ok = False
        if filepath[-3:] in ["ctd", "ctz"]:
            # xml
            self.filetype = "d"
            cherrytree_string = self.file_get_cherrytree_data(filepath, True)
            if cherrytree_string: document_loaded_ok = True
            elif cherrytree_string == None: return # no error exit
        elif filepath[-3:] in ["ctb", "ctx"]:
            # db
            self.filetype = "b"
            self.db = self.file_get_cherrytree_data(filepath, True)
            if self.db: document_loaded_ok = True
            elif self.db == None: return # no error exit
        if document_loaded_ok:
            document_loaded_ok = False
            if self.user_active:
                self.user_active = False
                user_active_restore = True
            else: user_active_restore = False
            file_loaded = False
            if self.filetype in ["d", "z"]:
                # xml
                if self.xml_handler.dom_to_treestore(cherrytree_string, discard_ids=False):
                    document_loaded_ok = True
            else:
                # db
                self.ctdb_handler.read_db_full(self.db, discard_ids=False)
                document_loaded_ok = True
            if document_loaded_ok:
                self.file_dir = os.path.dirname(filepath)
                self.file_name = os.path.basename(filepath)
                self.filetype = self.file_name[-1]
                self.ui.get_widget("/MenuBar/FileMenu/ct_vacuum").set_property(cons.STR_VISIBLE, self.filetype in ["b", "x"])
                support.add_recent_document(self, filepath)
                support.set_bookmarks_menu_items(self)
                self.update_window_save_not_needed()
            if user_active_restore: self.user_active = True
        if not document_loaded_ok:
            support.dialog_error(_('"%s" is Not a CherryTree Document') % filepath, self.window)
            self.file_name = ""

    def file_new(self, *args):
        """Starts a new unsaved instance"""
        #if self.reset(): self.node_add()
        self.filepath_boss_open("", "")

    def tags_add_from_node(self, tags_str):
        """Populate Tags Set from String"""
        for single_tag in tags_str.split(cons.CHAR_SPACE):
            single_tag_stripped = single_tag.strip()
            if single_tag_stripped:
                self.tags_set.add(single_tag_stripped)
                #print "tags_set +=", single_tag_stripped

    def reset(self, force_reset=False):
        """Reset the Application"""
        if not force_reset and not self.tree_is_empty() and not self.check_unsaved(): return False
        self.modification_time_update_value(False)
        if self.user_active:
            self.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        if self.curr_tree_iter != None:
            self.curr_buffer.set_text("")
            self.curr_tree_iter = None
            self.update_node_name_header()
            self.update_selected_node_statusbar_info()
        self.treestore.clear()
        self.tags_set.clear()
        self.latest_statusbar_update_time = {}
        self.file_name = ""
        self.password = None
        self.xml_handler.reset_nodes_names()
        self.bookmarks = []
        self.update_window_save_not_needed()
        self.state_machine.reset()
        self.sourceview.set_sensitive(False)
        if "db" in dir(self) and self.db: self.db.close()
        for filepath_tmp in self.ctdb_handler.remove_at_quit_set:
            if os.path.isfile(filepath_tmp):
                os.remove(filepath_tmp)
        self.ctdb_handler.reset()
        if user_active_restore: self.user_active = True
        return True

    def export_to_ctd(self, action):
        """Export the Selected Node and its Subnodes"""
        if not self.is_there_selected_node_or_error(): return
        export_type = support.dialog_selnode_selnodeandsub_alltree(self, also_selection=True)
        if export_type == 0: return
        ctd_handler = exports.Export2CTD(self)
        restore_passw = self.password
        restore_filetype = self.filetype
        if not support.dialog_choose_data_storage(self): return
        if export_type == 1:
            # only selected node
            proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
            proposed_name = self.filepath_extension_fix(proposed_name)
            ctd_filepath = ctd_handler.get_single_ct_filepath(proposed_name)
            if ctd_filepath:
                ctd_handler.node_export_to_ctd(self.curr_tree_iter, ctd_filepath)
        elif export_type == 2:
            # selected node and subnodes
            proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
            proposed_name = self.filepath_extension_fix(proposed_name)
            ctd_filepath = ctd_handler.get_single_ct_filepath(proposed_name)
            if ctd_filepath:
                ctd_handler.node_and_subnodes_export_to_ctd(self.curr_tree_iter, ctd_filepath)
                if restore_filetype in ["b", "x"] and self.curr_tree_iter:
                    self.state_machine.update_state()
                    self.objects_buffer_refresh()
        elif export_type == 3:
            # all nodes
            proposed_name = self.file_name[:-1] + self.filetype
            ctd_filepath = ctd_handler.get_single_ct_filepath(proposed_name)
            if ctd_filepath:
                ctd_handler.nodes_all_export_to_ctd(ctd_filepath)
                if restore_filetype in ["b", "x"] and self.curr_tree_iter:
                    self.state_machine.update_state()
                    self.objects_buffer_refresh()
        else:
            # only selection
            if self.is_there_text_selection_or_error():
                iter_start, iter_end = self.curr_buffer.get_selection_bounds()
                proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                proposed_name = self.filepath_extension_fix(proposed_name)
                ctd_filepath = ctd_handler.get_single_ct_filepath(proposed_name)
                if ctd_filepath:
                    sel_range = [iter_start.get_offset(), iter_end.get_offset()]
                    ctd_handler.node_export_to_ctd(self.curr_tree_iter, ctd_filepath, sel_range)
        self.password = restore_passw
        self.filetype = restore_filetype

    def export_to_txt_single(self, *args):
        """Export To Plain Text Single File"""
        self.export_single = True
        self.export_to_txt_multiple()
        self.export_single = False

    def export_to_txt_multiple(self, *args):
        """Export To Plain Text Multiple Files"""
        if not self.is_there_selected_node_or_error(): return
        if args and args[0] == "Auto":
            export_type = 3
            self.export_single = False
            self.last_include_node_name = True
        else:
            export_type = support.dialog_selnode_selnodeandsub_alltree(self, also_selection=True, also_include_node_name=True)
        txt_handler = exports.Export2Txt(self)
        if export_type == 0: return
        if export_type == 1:
            # only selected node
            proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
            txt_filepath = txt_handler.get_single_txt_filepath(proposed_name)
            if txt_filepath:
                if os.path.isfile(txt_filepath): os.remove(txt_filepath)
                tree_iter_for_node_name = self.curr_tree_iter if self.last_include_node_name else None
                txt_handler.node_export_to_txt(self.curr_buffer, txt_filepath, tree_iter_for_node_name=tree_iter_for_node_name)
        elif export_type == 2:
            # selected node and subnodes
            if self.export_single:
                txt_filepath = txt_handler.get_single_txt_filepath(self.file_name)
                if txt_filepath:
                    if os.path.isfile(txt_filepath): os.remove(txt_filepath)
                    txt_handler.nodes_all_export_to_txt(top_tree_iter=self.curr_tree_iter, single_txt_filepath=txt_filepath, include_node_name=self.last_include_node_name)
            else:
                folder_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                if txt_handler.prepare_txt_folder(folder_name):
                    txt_handler.nodes_all_export_to_txt(self.curr_tree_iter, include_node_name=self.last_include_node_name)
        elif export_type == 3:
            # all nodes
            if self.export_single:
                txt_filepath = txt_handler.get_single_txt_filepath(self.file_name)
                if txt_filepath:
                    if os.path.isfile(txt_filepath): os.remove(txt_filepath)
                    txt_handler.nodes_all_export_to_txt(single_txt_filepath=txt_filepath, include_node_name=self.last_include_node_name)
            else:
                if args and args[0] == "Auto":
                    dir_string = args[1]
                    self.export_overwrite = args[2]
                else:
                    dir_string = ""
                if txt_handler.prepare_txt_folder(self.file_name, dir_place=dir_string):
                    txt_handler.nodes_all_export_to_txt(include_node_name=self.last_include_node_name)
        else:
            # only selection
            if self.is_there_text_selection_or_error():
                iter_start, iter_end = self.curr_buffer.get_selection_bounds()
                proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                txt_filepath = txt_handler.get_single_txt_filepath(proposed_name)
                if txt_filepath:
                    if os.path.isfile(txt_filepath): os.remove(txt_filepath)
                    sel_range = [iter_start.get_offset(), iter_end.get_offset()]
                    tree_iter_for_node_name = self.curr_tree_iter if self.last_include_node_name else None
                    txt_handler.node_export_to_txt(self.curr_buffer, txt_filepath, sel_range, tree_iter_for_node_name=tree_iter_for_node_name)

    def export_to_html(self, *args):
        """Export to HTML"""
        if not self.is_there_selected_node_or_error(): return
        if args and args[0] == "Auto":
            export_type = 3
        else:
            export_type = support.dialog_selnode_selnodeandsub_alltree(self, also_selection=True, also_include_node_name=True, also_index_in_page=True)
        if export_type == 0: return
        if export_type == 1:
            # only selected node
            folder_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
            if self.html_handler.prepare_html_folder(folder_name):
                self.html_handler.node_export_to_html(self.curr_tree_iter)
        elif export_type == 2:
            # selected node and subnodes
            folder_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
            if self.html_handler.prepare_html_folder(folder_name):
                self.html_handler.nodes_all_export_to_html(self.curr_tree_iter)
                if self.filetype in ["b", "x"] and self.curr_tree_iter:
                    self.state_machine.update_state()
                    self.objects_buffer_refresh()
        elif export_type == 3:
            # all nodes
            if args and args[0] == "Auto":
                dir_string = args[1]
                self.export_overwrite = args[2]
            else:
                dir_string = ""
            if self.html_handler.prepare_html_folder(self.file_name, dir_place=dir_string):
                self.html_handler.nodes_all_export_to_html()
                if self.filetype in ["b", "x"] and self.curr_tree_iter:
                    self.state_machine.update_state()
                    self.objects_buffer_refresh()
        else:
            # only selection
            if self.is_there_text_selection_or_error():
                folder_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                if self.html_handler.prepare_html_folder(folder_name):
                    self.html_handler.node_export_to_html(self.curr_tree_iter, only_selection=True)

    def export_print_page_setup(self, action):
        """Print Page Setup Operations"""
        if self.print_handler.settings is None:
            self.print_handler.settings = gtk.PrintSettings()
        self.print_handler.page_setup = gtk.print_run_page_setup_dialog(self.window,
                                            self.print_handler.page_setup,
                                            self.print_handler.settings)

    def export_to_pdf(self, action):
        """Start Export to PDF Operations"""
        self.print_handler.pdf_filepath = cons.CHAR_TILDE
        self.export_print(action)
        self.print_handler.pdf_filepath = ""

    def export_print(self, action):
        """Start Print Operations"""
        if not self.is_there_selected_node_or_error(): return
        export_type = support.dialog_selnode_selnodeandsub_alltree(self, also_selection=True, also_include_node_name=True, also_new_node_page=True)
        if export_type == 0: return
        pdf_handler = exports.ExportPrint(self)
        if export_type == 1:
            # only selected node
            if self.print_handler.pdf_filepath == cons.CHAR_TILDE:
                proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                pdf_filepath = pdf_handler.get_pdf_filepath(proposed_name)
                if not pdf_filepath: return
                self.print_handler.pdf_filepath = pdf_filepath
            pdf_handler.node_export_print(self.curr_tree_iter, self.last_include_node_name)
        elif export_type == 2:
            # selected node and subnodes
            if self.print_handler.pdf_filepath == cons.CHAR_TILDE:
                pdf_filepath = pdf_handler.get_pdf_filepath(self.file_name)
                if not pdf_filepath: return
                self.print_handler.pdf_filepath = pdf_filepath
            pdf_handler.nodes_all_export_print(self.curr_tree_iter, self.last_include_node_name, self.last_new_node_page)
        elif export_type == 3:
            # all nodes
            if self.print_handler.pdf_filepath == cons.CHAR_TILDE:
                pdf_filepath = pdf_handler.get_pdf_filepath(self.file_name)
                if not pdf_filepath: return
                self.print_handler.pdf_filepath = pdf_filepath
            pdf_handler.nodes_all_export_print(None, self.last_include_node_name, self.last_new_node_page)
        else:
            # only selection
            if self.is_there_text_selection_or_error():
                iter_start, iter_end = self.curr_buffer.get_selection_bounds()
                if self.print_handler.pdf_filepath == cons.CHAR_TILDE:
                    proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                    pdf_filepath = pdf_handler.get_pdf_filepath(proposed_name)
                    if not pdf_filepath: return
                    self.print_handler.pdf_filepath = pdf_filepath
                pdf_handler.node_export_print(self.curr_tree_iter,
                                              self.last_include_node_name,
                                              sel_range=[iter_start.get_offset(), iter_end.get_offset()])

    def tree_sort_level_and_sublevels(self, model, father_iter, ascending):
        """Sorts the Tree Level and All the Sublevels"""
        movements = False
        while self.node_siblings_sort_iteration(model, father_iter, ascending, 1):
            movements = True
        if father_iter: curr_sibling = model.iter_children(father_iter)
        else: curr_sibling = model.get_iter_first()
        while curr_sibling:
            if self.tree_sort_level_and_sublevels(model, curr_sibling, ascending): movements = True
            curr_sibling = model.iter_next(curr_sibling)
        return movements

    def tree_sort_ascending(self, *args):
        """Sorts the Tree Ascending"""
        if self.tree_sort_level_and_sublevels(self.treestore, None, True):
            self.nodes_sequences_fix(None, True)
            self.update_window_save_needed()

    def tree_sort_descending(self, *args):
        """Sorts the Tree Descending"""
        if self.tree_sort_level_and_sublevels(self.treestore, None, False):
            self.nodes_sequences_fix(None, True)
            self.update_window_save_needed()

    def node_siblings_sort_ascending(self, *args):
        """Sorts all the Siblings of the Selected Node Ascending"""
        if not self.is_there_selected_node_or_error(): return
        father_iter = self.treestore.iter_parent(self.curr_tree_iter)
        movements = False
        while self.node_siblings_sort_iteration(self.treestore, father_iter, True, 1):
            movements = True
        if movements:
            self.nodes_sequences_fix(father_iter, False)
            self.update_window_save_needed()

    def node_siblings_sort_descending(self, *args):
        """Sorts all the Siblings of the Selected Node Descending"""
        if not self.is_there_selected_node_or_error(): return
        father_iter = self.treestore.iter_parent(self.curr_tree_iter)
        movements = False
        while self.node_siblings_sort_iteration(self.treestore, father_iter, False, 1):
            movements = True
        if movements:
            self.nodes_sequences_fix(father_iter, False)
            self.update_window_save_needed()

    def node_siblings_sort_iteration(self, model, father_iter, ascending, column):
        """Runs One Sorting Iteration, Returns True if Any Swap was Necessary"""
        if father_iter: curr_sibling = model.iter_children(father_iter)
        else: curr_sibling = model.get_iter_first()
        if not curr_sibling: return False
        next_sibling = model.iter_next(curr_sibling)
        swap_executed = False
        while next_sibling != None:
            if (ascending and model[next_sibling][column].lower() < model[curr_sibling][column].lower())\
            or (not ascending and model[next_sibling][column].lower() > model[curr_sibling][column].lower()):
                model.swap(next_sibling, curr_sibling)
                swap_executed = True
            else: curr_sibling = next_sibling
            next_sibling = model.iter_next(curr_sibling)
        return swap_executed

    def node_inherit_syntax(self, *args):
        """Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting"""
        if not self.is_there_selected_node_or_error(): return
        self.sourceview.set_buffer(None)
        self.node_inherit_syntax_iter(self.curr_tree_iter)
        self.sourceview.set_buffer(self.treestore[self.curr_tree_iter][2])

    def node_inherit_syntax_iter(self, iter_father):
        """Iteration of the Node Inherit Syntax"""
        iter_child = self.treestore.iter_children(iter_father)
        while iter_child != None:
            if not self.get_node_read_only(iter_child) and self.treestore[iter_child][4] != self.treestore[iter_father][4]:
                self.get_textbuffer_from_tree_iter(iter_child)
                old_syntax_highl = self.treestore[iter_child][4]
                self.treestore[iter_child][4] = self.treestore[iter_father][4]
                self.treestore[iter_child][0] = self.get_node_icon(self.treestore.iter_depth(iter_child),
                                                                   self.treestore[iter_child][4],
                                                                   self.treestore[iter_child][9])
                self.switch_buffer_text_source(self.treestore[iter_child][2],
                                               iter_child,
                                               self.treestore[iter_child][4],
                                               old_syntax_highl)
                if self.treestore[iter_child][4] not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
                    self.set_sourcebuffer_syntax_highlight(self.treestore[iter_child][2],
                                                           self.treestore[iter_child][4])
                self.ctdb_handler.pending_edit_db_node_prop(self.treestore[iter_child][3])
                self.update_window_save_needed()
            self.node_inherit_syntax_iter(iter_child)
            iter_child = self.treestore.iter_next(iter_child)

    def node_up(self, *args):
        """Node up one position"""
        if not self.is_there_selected_node_or_error(): return
        prev_iter = self.get_tree_iter_prev_sibling(self.treestore, self.curr_tree_iter)
        if prev_iter != None:
            self.treestore.swap(self.curr_tree_iter, prev_iter)
            self.nodes_sequences_swap(self.curr_tree_iter, prev_iter)
            self.ctdb_handler.pending_edit_db_node_hier(self.treestore[self.curr_tree_iter][3])
            self.ctdb_handler.pending_edit_db_node_hier(self.treestore[prev_iter][3])
            self.treeview.set_cursor(self.treestore.get_path(self.curr_tree_iter))
            self.update_window_save_needed()

    def node_down(self, *args):
        """Node down one position"""
        if not self.is_there_selected_node_or_error(): return
        subseq_iter = self.treestore.iter_next(self.curr_tree_iter)
        if subseq_iter != None:
            self.treestore.swap(self.curr_tree_iter, subseq_iter)
            self.nodes_sequences_swap(self.curr_tree_iter, subseq_iter)
            self.ctdb_handler.pending_edit_db_node_hier(self.treestore[self.curr_tree_iter][3])
            self.ctdb_handler.pending_edit_db_node_hier(self.treestore[subseq_iter][3])
            self.treeview.set_cursor(self.treestore.get_path(self.curr_tree_iter))
            self.update_window_save_needed()

    def node_right(self, *args):
        """Node right one position"""
        if not self.is_there_selected_node_or_error(): return
        prev_iter = self.get_tree_iter_prev_sibling(self.treestore, self.curr_tree_iter)
        if prev_iter != None:
            self.node_move_after(self.curr_tree_iter,
                                 prev_iter)
            if self.nodes_icons == "c": self.treeview_refresh(change_icon=True)

    def node_left(self, *args):
        """Node left one position"""
        if not self.is_there_selected_node_or_error(): return
        father_iter = self.treestore.iter_parent(self.curr_tree_iter)
        if father_iter != None:
            self.node_move_after(self.curr_tree_iter,
                                 self.treestore.iter_parent(father_iter),
                                 father_iter)
            if self.nodes_icons == "c": self.treeview_refresh(change_icon=True)

    def node_move_after(self, iter_to_move, father_iter, brother_iter=None, set_first=False):
        """Move a node to a parent and after a sibling"""
        if brother_iter:
            new_node_iter = self.treestore.insert_after(father_iter, brother_iter, self.treestore[iter_to_move])
        elif set_first: new_node_iter = self.treestore.prepend(father_iter, self.treestore[iter_to_move])
        else: new_node_iter = self.treestore.append(father_iter, self.treestore[iter_to_move])
        # we move also all the children
        self.node_move_children(iter_to_move, new_node_iter)
        # now we can remove the old iter (and all children)
        self.treestore.remove(iter_to_move)
        self.ctdb_handler.pending_edit_db_node_hier(self.treestore[new_node_iter][3])
        self.nodes_sequences_fix(None, True)
        if father_iter: self.treeview.expand_row(self.treestore.get_path(father_iter), False)
        else: self.treeview.expand_row(self.treestore.get_path(new_node_iter), False)
        self.curr_tree_iter = new_node_iter
        new_node_path = self.treestore.get_path(new_node_iter)
        self.treeview.collapse_row(new_node_path)
        self.treeview.set_cursor(new_node_path)
        self.update_window_save_needed()

    def node_move_children(self, old_father, new_father):
        """Move the children from a parent to another"""
        children_iter_to_move = self.treestore.iter_children(old_father)
        while children_iter_to_move != None:
            new_children_iter = self.treestore.append(new_father, self.treestore[children_iter_to_move])
            self.node_move_children(children_iter_to_move, new_children_iter)
            children_iter_to_move = self.treestore.iter_next(children_iter_to_move)

    def node_change_father(self, *args):
        """Node browse for a new parent"""
        if not self.is_there_selected_node_or_error(): return
        curr_node_id = self.treestore[self.curr_tree_iter][3]
        old_father_iter = self.treestore.iter_parent(self.curr_tree_iter)
        if old_father_iter != None: old_father_node_id = self.treestore[old_father_iter][3]
        else: old_father_node_id = None
        father_iter = support.dialog_choose_node(self, _("Select the New Parent"), self.treestore, self.curr_tree_iter)
        if not father_iter: return
        new_father_node_id = self.treestore[father_iter][3]
        if curr_node_id == new_father_node_id:
            support.dialog_error(_("The new parent can't be the very node to move!"), self.window)
            return
        if old_father_node_id != None and new_father_node_id == old_father_node_id:
            support.dialog_info(_("The new chosen parent is still the old parent!"), self.window)
            return
        move_towards_top_iter = self.treestore.iter_parent(father_iter)
        while move_towards_top_iter:
            if self.treestore[move_towards_top_iter][3] == curr_node_id:
                support.dialog_error(_("The new parent can't be one of his children!"), self.window)
                return
            move_towards_top_iter = self.treestore.iter_parent(move_towards_top_iter)
        self.node_move_after(self.curr_tree_iter, father_iter)
        if self.nodes_icons == "c": self.treeview_refresh(change_icon=True)

    def get_node_id_from_tree_iter(self, tree_iter):
        """Given a TreeIter, Returns the Node Id"""
        return self.treestore[tree_iter][3]

    def get_tree_iter_from_node_id(self, node_id):
        """Given a Node Id, Returns the TreeIter or None"""
        tree_iter = self.treestore.get_iter_first()
        while tree_iter != None:
            if self.treestore[tree_iter][3] == node_id: return tree_iter
            child_tree_iter = self.get_tree_iter_from_node_id_children(tree_iter, node_id)
            if child_tree_iter != None: return child_tree_iter
            tree_iter = self.treestore.iter_next(tree_iter)
        return None

    def get_tree_iter_from_node_id_children(self, father_iter, node_id):
        """Iterative function searching for Node Id between Children"""
        tree_iter = self.treestore.iter_children(father_iter)
        while tree_iter != None:
            if self.treestore[tree_iter][3] == node_id: return tree_iter
            child_tree_iter = self.get_tree_iter_from_node_id_children(tree_iter, node_id)
            if child_tree_iter != None: return child_tree_iter
            tree_iter = self.treestore.iter_next(tree_iter)
        return None

    def update_num_nodes(self, father_tree_iter):
        """Count the Nodes"""
        self.num_nodes = 0
        if father_tree_iter:
            self.update_num_nodes_children(father_tree_iter)
        else:
            tree_iter = self.treestore.get_iter_first()
            while tree_iter:
                self.update_num_nodes_children(tree_iter)
                tree_iter = self.treestore.iter_next(tree_iter)

    def update_num_nodes_children(self, father_iter):
        """Count the Nodes Children"""
        self.num_nodes += 1
        tree_iter = self.treestore.iter_children(father_iter)
        while tree_iter:
            self.update_num_nodes_children(tree_iter)
            tree_iter = self.treestore.iter_next(tree_iter)

    def get_tree_iter_from_node_name(self, node_name, use_content=False):
        """Given a Node Id, Returns the TreeIter or None"""
        tree_iter = self.treestore.get_iter_first()
        while tree_iter != None:
            if not use_content:
                if self.treestore[tree_iter][1] == node_name: return tree_iter
            else:
                if node_name.lower() in self.treestore[tree_iter][1].lower(): return tree_iter
            child_tree_iter = self.get_tree_iter_from_node_name_children(tree_iter, node_name, use_content)
            if child_tree_iter != None: return child_tree_iter
            tree_iter = self.treestore.iter_next(tree_iter)
        return None

    def get_tree_iter_from_node_name_children(self, father_iter, node_name, use_content):
        """Iterative function searching for Node Id between Children"""
        tree_iter = self.treestore.iter_children(father_iter)
        while tree_iter != None:
            if not use_content:
                if self.treestore[tree_iter][1] == node_name: return tree_iter
            else:
                if node_name.lower() in self.treestore[tree_iter][1].lower(): return tree_iter
            child_tree_iter = self.get_tree_iter_from_node_name_children(tree_iter, node_name, use_content)
            if child_tree_iter != None: return child_tree_iter
            tree_iter = self.treestore.iter_next(tree_iter)
        return None

    def treeview_expand_to_tree_iter(self, tree_iter):
        """Expanded Node at Tree Iter"""
        father_iter = self.treestore.iter_parent(tree_iter)
        if father_iter:
            father_path = self.treestore.get_path(father_iter)
            self.treeview.expand_to_path(father_path)

    def treeview_safe_set_cursor(self, tree_iter):
        """Set Cursor being sure the Node is Expanded"""
        self.treeview_expand_to_tree_iter(tree_iter)
        tree_path = self.treestore.get_path(tree_iter)
        self.treeview.set_cursor(tree_path)

    def on_help_menu_item_activated(self, menuitem, data=None):
        """Show the Online Manual"""
        webbrowser.open("http://giuspen.com/cherrytreemanual/Introduction.html")

    def on_window_focus_out_event(self, widget, event, data=None):
        """When the main windows loses the focus (e.g. dialog)"""
        if self.ctrl_down: self.ctrl_down = False

    def on_event_after_tree(self, widget, event):
        """Catches events after"""
        if event.type == gtk.gdk.KEY_PRESS:
            if not self.ctrl_down:
                keyname = gtk.gdk.keyval_name(event.keyval)
                if keyname in cons.STR_KEYS_CONTROL:
                    self.ctrl_down = True
        elif event.type == gtk.gdk.KEY_RELEASE:
            if self.ctrl_down:
                keyname = gtk.gdk.keyval_name(event.keyval)
                if keyname in cons.STR_KEYS_CONTROL:
                    self.ctrl_down = False
        elif event.type == gtk.gdk.SCROLL:
            if self.ctrl_down:
                if event.direction == gtk.gdk.SCROLL_UP:
                    self.zoom_tree_p()
                elif event.direction == gtk.gdk.SCROLL_DOWN:
                    self.zoom_tree_m()
        elif event.type == gtk.gdk.BUTTON_PRESS:
            if event.button == 1:
                if self.tree_click_focus_text:
                    self.sourceview.grab_focus()
                if self.tree_click_expand:
                    path_at_click = self.treeview.get_path_at_pos(int(event.x), int(event.y))
                    if path_at_click and not self.treeview.row_expanded(path_at_click[0]):
                        self.treeview.expand_row(path_at_click[0], open_all=False)
                        self.tree_just_auto_expanded = True
                    else:
                        self.tree_just_auto_expanded = False
        elif event.type == gtk.gdk._2BUTTON_PRESS:
            if event.button == 1:
                self.toggle_tree_node_expanded_collapsed()

    def on_test_collapse_row_tree(self, treeview, tree_iter, tree_path):
        """Just before collapsing a node"""
        if self.tree_click_expand:
            if self.tree_just_auto_expanded:
                self.tree_just_auto_expanded = False
                return True
        return False

    def on_mouse_button_clicked_tree(self, widget, event):
        """Catches mouse buttons clicks"""
        if event.button == 3:
            self.node_menu_tree.popup(None, None, None, event.button, event.time)
        elif event.button == 2:
            path_at_click = self.treeview.get_path_at_pos(int(event.x), int(event.y))
            if path_at_click:
                if self.treeview.row_expanded(path_at_click[0]):
                    self.treeview.collapse_row(path_at_click[0])
                else: self.treeview.expand_row(path_at_click[0], False)
        return False

    def set_sourcebuffer_with_style_scheme(self):
        if exports.rgb_24_get_is_dark(self.rt_def_bg[1:]):
            style_scheme = cons.STYLE_SCHEME_DARK
        else:
            style_scheme = cons.STYLE_SCHEME_LIGHT
        if not style_scheme in self.sourcebuffers.keys():
            self.sourcebuffers[style_scheme] = gtksourceview2.Buffer()
            self.sourcebuffers[style_scheme].set_style_scheme(self.sourcestyleschememanager.get_scheme(style_scheme))
        self.sourceview.set_buffer(self.sourcebuffers[style_scheme])

    def buffer_create(self, syntax_highlighting):
        """Returns a New Instantiated SourceBuffer"""
        if syntax_highlighting != cons.RICH_TEXT_ID:
            sourcebuffer = gtksourceview2.Buffer()
            sourcebuffer.set_style_scheme(self.sourcestyleschememanager.get_scheme(self.style_scheme))
            if syntax_highlighting != cons.PLAIN_TEXT_ID:
                self.set_sourcebuffer_syntax_highlight(sourcebuffer, syntax_highlighting)
            else:
                sourcebuffer.set_highlight_syntax(False)
            sourcebuffer.set_highlight_matching_brackets(True)
            return sourcebuffer
        else: return gtk.TextBuffer(self.tag_table)

    def combobox_prog_lang_init(self):
        """Init The Programming Languages Syntax Highlighting ComboBox"""
        self.language_manager = gtksourceview2.LanguageManager()
        search_path = self.language_manager.get_search_path()
        search_path.append(cons.SPECS_PATH)
        self.language_manager.set_search_path(search_path)
        #print self.language_manager.get_search_path()
        self.available_languages = sorted(self.language_manager.get_language_ids())
        if "def" in self.available_languages: self.available_languages.remove("def")

    def combobox_country_lang_init(self):
        """Init The Country Language ComboBox"""
        if not "country_lang_liststore" in dir(self):
            self.country_lang_liststore = gtk.ListStore(str)
            for country_lang in cons.AVAILABLE_LANGS:
                self.country_lang_liststore.append([country_lang])

    def combobox_style_scheme_init(self):
        """Init The Style Scheme ComboBox"""
        if not "style_scheme_liststore" in dir(self):
            self.style_scheme_liststore = gtk.ListStore(str)
            style_schemes_list = []
            for style_scheme in sorted(self.sourcestyleschememanager.get_scheme_ids()):
                self.style_scheme_liststore.append([style_scheme])
                style_schemes_list.append(style_scheme)
            if not self.style_scheme in style_schemes_list: self.style_scheme = style_schemes_list[0]

    def combobox_spell_check_lang_init(self):
        """Init The Spell Check Language ComboBox"""
        if not "spell_check_lang_liststore" in dir(self):
            self.spell_check_lang_liststore = gtk.ListStore(str)
            code_lang_list = []
            for code_lang in sorted(self.spell_check_get_languages()):
                self.spell_check_lang_liststore.append([code_lang])
                code_lang_list.append(code_lang)
            if not self.spell_check_lang in code_lang_list: self.spell_check_lang = code_lang_list[0]

    def get_combobox_iter_from_value(self, liststore, column_num, value):
        """Returns the Liststore iter Given the First Column Value"""
        curr_iter = liststore.get_iter_first()
        while curr_iter != None:
            if liststore[curr_iter][column_num] == value: break
            else: curr_iter = liststore.iter_next(curr_iter)
        else: return liststore.get_iter_first()
        return curr_iter

    def set_sourcebuffer_syntax_highlight(self, sourcebuffer, syntax_highlighting):
        """Set the given syntax highlighting to the given sourcebuffer"""
        sourcebuffer.set_language(self.language_manager.get_language(syntax_highlighting))
        sourcebuffer.set_highlight_syntax(True)

    def nodes_sequences_swap(self, first_iter, second_iter):
        """Swap the sequences num of the two iters"""
        first_iter_seq = self.treestore[first_iter][5]
        self.treestore[first_iter][5] = self.treestore[second_iter][5]
        self.treestore[second_iter][5] = first_iter_seq

    def nodes_sequences_get_max_siblings(self, tree_father):
        """Get Maximum sibling sequence num"""
        tree_iter = self.treestore.iter_children(tree_father) if tree_father else self.treestore.get_iter_first()
        node_sequence = 0
        while tree_iter != None:
            if self.treestore[tree_iter][5] > node_sequence:
                node_sequence = self.treestore[tree_iter][5]
            tree_iter = self.treestore.iter_next(tree_iter)
        return node_sequence

    def nodes_sequences_fix(self, father_iter, process_children):
        """Parse Tree and Fix Node Sequences"""
        tree_iter = self.treestore.iter_children(father_iter) if father_iter else self.treestore.get_iter_first()
        node_sequence = 0
        while tree_iter != None:
            node_sequence += 1
            if self.treestore[tree_iter][5] != node_sequence:
                self.treestore[tree_iter][5] = node_sequence
                self.ctdb_handler.pending_edit_db_node_hier(self.treestore[tree_iter][3])
            if process_children:
                self.nodes_sequences_fix(tree_iter, process_children)
            tree_iter = self.treestore.iter_next(tree_iter)

    def node_duplicate(self, *args):
        """Duplicate the Selected Node"""
        if not self.is_there_selected_node_or_error(): return
        self.node_add_is_duplication = True
        self.node_add()
        self.node_add_is_duplication = False

    def node_add(self, *args):
        """Add a node having common parent with the selected node"""
        if not self.node_add_is_duplication:
            ret_name, ret_syntax, ret_tags, ret_ro, ret_c_icon_id, ret_is_bold, ret_fg = self.dialog_nodeprop(_("New Node Properties"), syntax_highl=self.syntax_highlighting)
            if not ret_name: return
        else:
            tree_iter_from = self.curr_tree_iter
            ret_name = self.treestore[tree_iter_from][1]
            ret_syntax = self.treestore[tree_iter_from][4]
            ret_tags = self.treestore[tree_iter_from][6]
            ret_ro = self.treestore[tree_iter_from][7]
            ret_c_icon_id = self.treestore[tree_iter_from][9]
            ret_is_bold = support.get_pango_is_bold(self.treestore[tree_iter_from][10])
            ret_fg = self.treestore[tree_iter_from][11]
        self.update_window_save_needed()
        self.syntax_highlighting = ret_syntax
        father_iter = self.treestore.iter_parent(self.curr_tree_iter) if self.curr_tree_iter else None
        node_level = self.treestore.iter_depth(father_iter)+1 if father_iter else 0
        cherry = self.get_node_icon(node_level, self.syntax_highlighting, ret_c_icon_id)
        new_node_id = self.node_id_get()
        ts_creation = time.time()
        ts_lastsave = ts_creation
        if self.curr_tree_iter != None:
            new_node_iter = self.treestore.insert_after(father_iter,
                self.curr_tree_iter,
                [cherry, ret_name, self.buffer_create(self.syntax_highlighting),
                 new_node_id, self.syntax_highlighting, 0, ret_tags, ret_ro, None, ret_c_icon_id,
                 support.get_pango_weight(ret_is_bold), ret_fg, ts_creation, ts_lastsave])
        else:
            new_node_iter = self.treestore.append(father_iter,
                [cherry, ret_name, self.buffer_create(self.syntax_highlighting),
                 new_node_id, self.syntax_highlighting, 0, ret_tags, ret_ro, None, ret_c_icon_id,
                 support.get_pango_weight(ret_is_bold), ret_fg, ts_creation, ts_lastsave])
        if ret_tags: self.tags_add_from_node(ret_tags)
        self.ctdb_handler.pending_new_db_node(new_node_id)
        self.nodes_sequences_fix(father_iter, False)
        self.update_node_aux_icon(new_node_iter)
        self.nodes_names_dict[new_node_id] = ret_name
        if self.node_add_is_duplication:
            if self.syntax_highlighting != cons.RICH_TEXT_ID:
                text_buffer_from = self.treestore[tree_iter_from][2]
                text_buffer_to = self.treestore[new_node_iter][2]
                content = text_buffer_from.get_text(*text_buffer_from.get_bounds())
                text_buffer_to.begin_not_undoable_action()
                text_buffer_to.set_text(content)
                text_buffer_to.end_not_undoable_action()
            else:
                state = self.state_machine.requested_state_previous(self.treestore[tree_iter_from][3])
                self.load_buffer_from_state(state, given_tree_iter=new_node_iter)
        new_node_path = self.treestore.get_path(new_node_iter)
        self.treeview.set_cursor(new_node_path)
        self.sourceview.grab_focus()

    def node_child_exist_or_create(self, father_iter, node_name):
        """Create Child Node or Just Select it if Already Existing"""
        curr_iter = self.treestore.iter_children(father_iter) if father_iter else self.treestore.get_iter_first()
        while curr_iter:
            if self.treestore[curr_iter][1] == node_name:
                self.treeview_safe_set_cursor(curr_iter)
                return
            curr_iter = self.treestore.iter_next(curr_iter)
        self.node_child_add_with_data(father_iter, node_name, cons.RICH_TEXT_ID, "", False, 0, False, None)

    def node_child_add(self, *args):
        """Add a node having as parent the selected node"""
        if not self.is_there_selected_node_or_error(): return
        ret_name, ret_syntax, ret_tags, ret_ro, ret_c_icon_id, ret_is_bold, ret_fg = self.dialog_nodeprop(_("New Child Node Properties"), syntax_highl=self.syntax_highlighting)
        if not ret_name: return
        self.node_child_add_with_data(self.curr_tree_iter, ret_name, ret_syntax, ret_tags, ret_ro, ret_c_icon_id, ret_is_bold, ret_fg)

    def node_child_add_with_data(self, father_iter, ret_name, ret_syntax, ret_tags, ret_ro, ret_c_icon_id, ret_is_bold, ret_fg):
        """Add a node having as parent the given node"""
        self.update_window_save_needed()
        self.syntax_highlighting = ret_syntax
        node_level = self.treestore.iter_depth(father_iter)+1 if father_iter else 0
        cherry = self.get_node_icon(node_level, self.syntax_highlighting, ret_c_icon_id)
        new_node_id = self.node_id_get()
        ts_creation = time.time()
        ts_lastsave = ts_creation
        new_node_iter = self.treestore.append(father_iter,
            [cherry, ret_name, self.buffer_create(self.syntax_highlighting),
             new_node_id, self.syntax_highlighting, 0, ret_tags, ret_ro, None, ret_c_icon_id,
             support.get_pango_weight(ret_is_bold), ret_fg, ts_creation, ts_lastsave])
        self.ctdb_handler.pending_new_db_node(new_node_id)
        self.nodes_sequences_fix(father_iter, False)
        self.update_node_aux_icon(new_node_iter)
        self.nodes_names_dict[new_node_id] = ret_name
        self.treeview_safe_set_cursor(new_node_iter)
        self.sourceview.grab_focus()

    def node_delete(self, *args):
        """Delete the Selected Node"""
        if not self.is_there_selected_node_or_error(): return
        warning_label = _("Are you sure to <b>Delete the node '%s'?</b>") % self.treestore[self.curr_tree_iter][1]
        if self.treestore.iter_children(self.curr_tree_iter) != None:
            warning_label += cons.CHAR_NEWLINE*2+_("The node <b>has Children, they will be Deleted too!</b>")
            self.nodes_rows_count = 0
            warning_label += self.get_node_children_list(self.curr_tree_iter, 0)
        response = support.dialog_question_warning(self.window, warning_label)
        if response != gtk.RESPONSE_ACCEPT: return # the user did not confirm
        # next selected node will be previous sibling or next sibling or parent or None
        new_iter = self.get_tree_iter_prev_sibling(self.treestore, self.curr_tree_iter)
        if new_iter == None:
            new_iter = self.treestore.iter_next(self.curr_tree_iter)
            if new_iter == None: new_iter = self.treestore.iter_parent(self.curr_tree_iter)
        self.update_window_save_needed("ndel")
        self.treestore.remove(self.curr_tree_iter)
        self.curr_tree_iter = None
        if new_iter != None:
            self.treeview.set_cursor(self.treestore.get_path(new_iter))
            self.sourceview.grab_focus()
        else:
            self.curr_buffer.set_text("")
            self.update_node_name_header()
            self.update_selected_node_statusbar_info()
            self.sourceview.set_sensitive(False)

    def node_toggle_read_only(self, *args):
        """Toggle the Read Only Property of the Selected Node"""
        if not self.is_there_selected_node_or_error(): return
        node_is_ro = not self.get_node_read_only()
        self.treestore[self.curr_tree_iter][7] = node_is_ro
        self.sourceview.set_editable(not node_is_ro)
        self.header_node_name_icon_lock.set_property(cons.STR_VISIBLE, node_is_ro)
        self.update_selected_node_statusbar_info()
        self.update_node_aux_icon(self.curr_tree_iter)
        self.update_window_save_needed("npro")
        self.sourceview.grab_focus()

    def node_edit(self, *args):
        """Edit the Properties of the Selected Node"""
        if not self.is_there_selected_node_or_error(): return
        ret_name, ret_syntax, ret_tags, ret_ro, ret_c_icon_id, ret_is_bold, ret_fg = self.dialog_nodeprop(_("Node Properties"),
            name=self.treestore[self.curr_tree_iter][1],
            syntax_highl=self.treestore[self.curr_tree_iter][4],
            tags=self.treestore[self.curr_tree_iter][6],
            ro=self.treestore[self.curr_tree_iter][7],
            c_icon_id=self.treestore[self.curr_tree_iter][9],
            is_bold=support.get_pango_is_bold(self.treestore[self.curr_tree_iter][10]),
            fg=self.treestore[self.curr_tree_iter][11])
        if not ret_name: return
        self.syntax_highlighting = ret_syntax
        if self.treestore[self.curr_tree_iter][4] != self.syntax_highlighting:
            if self.treestore[self.curr_tree_iter][4] == cons.RICH_TEXT_ID:
                # leaving rich text
                if not support.dialog_question(_("Leaving the Node Type Rich Text you will Lose all Formatting for This Node, Do you want to Continue?"), self.window):
                    self.syntax_highlighting = cons.RICH_TEXT_ID # STEP BACK (we stay in CUSTOM COLORS)
                    return
                # SWITCH TextBuffer -> SourceBuffer
                self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
                self.curr_buffer = self.treestore[self.curr_tree_iter][2]
                self.state_machine.delete_states(self.get_node_id_from_tree_iter(self.curr_tree_iter))
            elif self.syntax_highlighting == cons.RICH_TEXT_ID:
                # going to rich text
                # SWITCH SourceBuffer -> TextBuffer
                self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
                self.curr_buffer = self.treestore[self.curr_tree_iter][2]
            elif self.treestore[self.curr_tree_iter][4] == cons.PLAIN_TEXT_ID:
                # plain text to code
                self.sourceview.modify_font(pango.FontDescription(self.code_font))
            elif self.syntax_highlighting == cons.PLAIN_TEXT_ID:
                # code to plain text
                self.sourceview.modify_font(pango.FontDescription(self.text_font))
        self.treestore[self.curr_tree_iter][1] = ret_name
        self.treestore[self.curr_tree_iter][4] = self.syntax_highlighting
        self.treestore[self.curr_tree_iter][6] = ret_tags
        if ret_tags: self.tags_add_from_node(ret_tags)
        self.treestore[self.curr_tree_iter][7] = ret_ro
        self.treestore[self.curr_tree_iter][9] = ret_c_icon_id
        self.treestore[self.curr_tree_iter][10] = support.get_pango_weight(ret_is_bold)
        self.treestore[self.curr_tree_iter][11] = ret_fg
        self.treestore[self.curr_tree_iter][0] = self.get_node_icon(self.treestore.iter_depth(self.curr_tree_iter),
                                                                    self.syntax_highlighting,
                                                                    ret_c_icon_id)
        if self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            self.set_sourcebuffer_syntax_highlight(self.curr_buffer, self.syntax_highlighting)
        self.sourceview.set_editable(not ret_ro)
        self.update_selected_node_statusbar_info()
        self.update_node_aux_icon(self.curr_tree_iter)
        self.treeview_set_colors()
        self.update_node_name_header()
        self.update_window_save_needed("npro")
        self.sourceview.grab_focus()

    def node_date(self, *args):
        """Insert Date Node in Tree"""
        curr_time = time.time()
        now_year = support.get_timestamp_str("%Y", curr_time)
        now_month = support.get_timestamp_str("%B", curr_time)
        now_day = support.get_timestamp_str("%d %a", curr_time)
        #print now_year, now_month, now_day
        if self.curr_tree_iter:
            curr_depth = self.treestore.iter_depth(self.curr_tree_iter)
            if curr_depth == 0:
                if self.treestore[self.curr_tree_iter][1] == now_year:
                    self.node_child_exist_or_create(self.curr_tree_iter, now_month)
                    self.node_date()
                    return
            else:
                if self.treestore[self.curr_tree_iter][1] == now_month\
                and self.treestore[self.treestore.iter_parent(self.curr_tree_iter)][1] == now_year:
                    self.node_child_exist_or_create(self.curr_tree_iter, now_day)
                    return
                if self.treestore[self.curr_tree_iter][1] == now_year:
                    self.node_child_exist_or_create(self.curr_tree_iter, now_month)
                    self.node_date()
                    return
        self.node_child_exist_or_create(None, now_year)
        self.node_date()

    def get_node_children_list(self, father_tree_iter, level):
        """Return a string listing the node children"""
        node_children_list = ""
        self.nodes_rows_count += 1
        if self.nodes_rows_count > 15: return "..."
        node_children_list += cons.CHAR_NEWLINE + level*3*cons.CHAR_SPACE + self.chars_listbul[0] + \
                              cons.CHAR_SPACE +self.treestore[father_tree_iter][1]
        tree_iter = self.treestore.iter_children(father_tree_iter)
        while tree_iter:
            node_children_list += self.get_node_children_list(tree_iter, level+1)
            tree_iter = self.treestore.iter_next(tree_iter)
        return node_children_list

    def widget_set_colors(self, widget, fg, bg, syntax_highl, gdk_col_fg=None):
        """Set a Widget's foreground and background colors"""
        if not syntax_highl:
            widget.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(bg))
            widget.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(fg))
            style = widget.get_style()
            # gtk.STATE_NORMAL, gtk.STATE_ACTIVE, gtk.STATE_PRELIGHT, gtk.STATE_SELECTED, gtk.STATE_INSENSITIVE
            widget.modify_text(gtk.STATE_SELECTED, style.fg[3] if not gdk_col_fg else gdk_col_fg)
            widget.modify_text(gtk.STATE_ACTIVE, style.fg[3] if not gdk_col_fg else gdk_col_fg)
            widget.modify_base(gtk.STATE_ACTIVE, style.bg[3])

    def treeview_set_colors(self):
        """Set Treeview Colors"""
        col_fg = self.treestore[self.curr_tree_iter][11] if self.curr_tree_iter else None
        gdk_col_fg = gtk.gdk.color_parse(col_fg) if col_fg else None
        self.widget_set_colors(self.treeview, self.tt_def_fg, self.tt_def_bg, False, gdk_col_fg=gdk_col_fg)

    def sourceview_set_properties(self, tree_iter, syntax_highl):
        """Set sourceview properties according to current node"""
        if syntax_highl == cons.RICH_TEXT_ID:
            self.treestore[tree_iter][2].connect('insert-text', self.on_text_insertion)
            self.treestore[tree_iter][2].connect('delete-range', self.on_text_removal)
            self.treestore[tree_iter][2].connect('mark-set', self.on_textbuffer_mark_set)
            self.sourceview.modify_font(pango.FontDescription(self.text_font))
            self.sourceview.set_draw_spaces(codeboxes.DRAW_SPACES_FLAGS if self.rt_show_white_spaces else 0)
            self.sourceview.set_highlight_current_line(self.rt_highl_curr_line)
            self.widget_set_colors(self.sourceview, self.rt_def_fg, self.rt_def_bg, False)
        else:
            if syntax_highl == cons.PLAIN_TEXT_ID:
                self.sourceview.modify_font(pango.FontDescription(self.text_font))
            else:
                self.sourceview.modify_font(pango.FontDescription(self.code_font))
            self.sourceview.set_draw_spaces(codeboxes.DRAW_SPACES_FLAGS if self.pt_show_white_spaces else 0)
            self.sourceview.set_highlight_current_line(self.pt_highl_curr_line)
            self.widget_set_colors(self.sourceview, self.rt_def_fg, self.rt_def_bg, True)

    def switch_buffer_text_source(self, text_buffer, tree_iter, new_syntax_highl, old_syntax_highl):
        """Switch TextBuffer -> SourceBuffer or SourceBuffer -> TextBuffer"""
        if self.user_active:
            self.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        if old_syntax_highl == cons.RICH_TEXT_ID and new_syntax_highl != cons.RICH_TEXT_ID:
            rich_to_non_rich = True
            txt_handler = exports.Export2Txt(self)
            node_text = txt_handler.node_export_to_txt(text_buffer, "")
        else:
            rich_to_non_rich = False
            node_text = text_buffer.get_text(*text_buffer.get_bounds())
        self.treestore[tree_iter][2] = self.buffer_create(new_syntax_highl)
        if rich_to_non_rich: self.treestore[tree_iter][2].begin_not_undoable_action()
        self.treestore[tree_iter][2].set_text(node_text)
        if rich_to_non_rich: self.treestore[tree_iter][2].end_not_undoable_action()
        self.sourceview.set_buffer(self.treestore[tree_iter][2])
        self.treestore[tree_iter][2].connect('modified-changed', self.on_modified_changed)
        self.sourceview_set_properties(tree_iter, new_syntax_highl)
        if user_active_restore: self.user_active = True
        self.ctdb_handler.pending_edit_db_node_buff(self.treestore[tree_iter][3])

    def on_node_changed(self, *args):
        """Actions to be triggered from the changing of node"""
        model, new_iter = self.treeviewselection.get_selected()
        if new_iter == None: return # no node selected
        elif self.curr_tree_iter != None and model[new_iter][3] == model[self.curr_tree_iter][3]:
            return # if i click on an already selected node
        if self.enable_spell_check and self.user_active and self.syntax_highlighting == cons.RICH_TEXT_ID:
            self.spell_check_set_off()
        if self.curr_tree_iter and self.curr_buffer:
            if self.user_active:
                self.nodes_cursor_pos[model[self.curr_tree_iter][3]] = self.curr_buffer.get_property(cons.STR_CURSOR_POSITION)
                #print "cursor_pos %s save for node %s" % (self.nodes_cursor_pos[model[self.curr_tree_iter][3]], model[self.curr_tree_iter][3])
            if self.curr_buffer.get_modified():
                self.file_update = True
                self.curr_buffer.set_modified(False)
                self.state_machine.update_state()
        self.curr_tree_iter = new_iter
        self.curr_buffer = self.get_textbuffer_from_tree_iter(self.curr_tree_iter)
        if self.rt_highl_curr_line and self.user_active and self.treestore[new_iter][4] == cons.RICH_TEXT_ID:
            self.set_sourcebuffer_with_style_scheme()
        self.sourceview.set_buffer(self.curr_buffer)
        self.syntax_highlighting = self.treestore[self.curr_tree_iter][4]
        self.curr_buffer.connect('modified-changed', self.on_modified_changed)
        self.sourceview_set_properties(self.curr_tree_iter, self.syntax_highlighting)
        self.sourceview.set_sensitive(True)
        node_is_ro = self.get_node_read_only()
        self.sourceview.set_editable(not node_is_ro)
        self.treeview_set_colors()
        self.update_node_name_header()
        self.state_machine.node_selected_changed(self.treestore[self.curr_tree_iter][3])
        self.objects_buffer_refresh()
        self.update_selected_node_statusbar_info()
        if self.user_active:
            node_id = model[new_iter][3]
            if node_id in self.nodes_cursor_pos:
                already_visited = True
                cursor_pos = self.nodes_cursor_pos[node_id]
            else:
                already_visited = False
                cursor_pos = 0
            cursor_iter = self.curr_buffer.get_iter_at_offset(cursor_pos)
            if cursor_iter:
                #print "cursor_pos %s restore for node %s" % (cursor_pos, node_id)
                self.curr_buffer.place_cursor(cursor_iter)
                self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
            if self.syntax_highlighting == cons.RICH_TEXT_ID:
                #if not already_visited: self.lists_handler.todo_lists_old_to_new_conversion(self.curr_buffer)
                if self.enable_spell_check: self.spell_check_set_on()
            node_is_bookmarked = (str(node_id) in self.bookmarks)
            self.menu_tree_update_for_bookmarked_node(node_is_bookmarked)
            self.header_node_name_icon_lock.set_property(cons.STR_VISIBLE, node_is_ro)

    def on_button_node_name_header_clicked(self, button, idx):
        node_id = self.node_name_header_buttons[idx+1]
        tree_iter = self.get_tree_iter_from_node_id(node_id)
        if tree_iter:
            self.treeview_safe_set_cursor(tree_iter)
            self.sourceview.grab_focus()

    def instantiate_node_name_header(self):
        """Instantiate Node Name Header"""
        self.header_node_name_hbox = gtk.HBox()
        self.header_node_name_hbuttonbox = gtk.HButtonBox()
        self.header_node_name_label = gtk.Label()
        self.header_node_name_label.set_padding(10, 0)
        self.header_node_name_label.set_ellipsize(pango.ELLIPSIZE_MIDDLE)
        self.header_node_name_icon_lock = gtk.image_new_from_stock("locked", gtk.ICON_SIZE_MENU)
        self.header_node_name_icon_pin = gtk.image_new_from_stock("pin", gtk.ICON_SIZE_MENU)
        self.header_node_name_hbox.pack_start(self.header_node_name_hbuttonbox, expand=False)
        self.header_node_name_hbox.pack_start(self.header_node_name_label, expand=True)
        self.header_node_name_hbox.pack_start(self.header_node_name_icon_lock, expand=False)
        self.header_node_name_hbox.pack_start(self.header_node_name_icon_pin, expand=False)
        header_node_name_eventbox = gtk.EventBox()
        header_node_name_eventbox.add(self.header_node_name_hbox)
        return header_node_name_eventbox

    def update_node_name_header_labels_latest_visited(self):
        """Update on the Node Name Header the Labels of Latest Visited"""
        if self.user_active and self.nodes_on_node_name_header and self.state_machine.visited_nodes_idx != None:
            sel_node_id = self.get_node_id_from_tree_iter(self.curr_tree_iter) if self.curr_tree_iter else -1
            self.node_name_header_buttons = {}
            buttons = self.header_node_name_hbuttonbox.get_children()
            assert len(buttons) == self.nodes_on_node_name_header, "%s != %s" % (len(buttons), self.nodes_on_node_name_header)
            curr_button_num = self.nodes_on_node_name_header
            for i in reversed(range(len(self.state_machine.visited_nodes_list))):
                node_id = self.state_machine.visited_nodes_list[i]
                if node_id != sel_node_id:
                    tree_iter = self.get_tree_iter_from_node_id(node_id)
                    if tree_iter:
                        node_name = self.treestore[tree_iter][1]
                        self.node_name_header_buttons[curr_button_num] = self.state_machine.visited_nodes_list[i]
                        markup = "<small>"+cgi.escape(node_name)+"</small>"
                        curr_button_idx = curr_button_num-1
                        buttons[curr_button_idx].get_children()[0].set_markup(markup)
                        buttons[curr_button_idx].show_all()
                        buttons[curr_button_idx].set_tooltip_text(node_name)
                        curr_button_num -= 1
                        if not curr_button_num: break
            for i in range(curr_button_num):
                buttons[i].hide()

    def update_node_name_header_num_latest_visited(self):
        """Update on the Node Name Header the Number of Latest Visited"""
        for button in self.header_node_name_hbuttonbox.get_children():
            self.header_node_name_hbuttonbox.remove(button)
        for i in range(self.nodes_on_node_name_header):
            button = gtk.Button()
            button.connect('clicked', self.on_button_node_name_header_clicked, i)
            label = gtk.Label()
            label.set_ellipsize(pango.ELLIPSIZE_END)
            button.add(label)
            self.header_node_name_hbuttonbox.add(button)
        self.update_node_name_header_labels_latest_visited()

    def update_node_name_header(self):
        """Update Node Name Header"""
        node_hier_name = self.treestore[self.curr_tree_iter][1] if self.curr_tree_iter else ""
        self.header_node_name_eventbox.modify_bg(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.tt_def_bg))
        foreground = self.treestore[self.curr_tree_iter][11] if self.curr_tree_iter else None
        fg = self.tt_def_fg if not foreground else foreground
        self.header_node_name_label.set_text(
            "<b><span foreground=\"" + fg + "\" size=\"xx-large\">"+\
            cgi.escape(node_hier_name) + "</span></b>")
        self.header_node_name_label.set_use_markup(True)
        self.update_node_name_header_labels_latest_visited()

    def get_textbuffer_from_tree_iter(self, tree_iter):
        """Returns the text buffer given the tree iter"""
        if not self.treestore[tree_iter][2]:
            # we are using db storage and the buffer was not created yet
            self.ctdb_handler.read_db_node_content(tree_iter, self.db)
        return self.treestore[tree_iter][2]

    def on_textbuffer_mark_set(self, text_buffer, text_iter, text_mark):
        """Highlight Focused Objects"""
        if not text_buffer.get_has_selection():
            if self.highlighted_obj: support.set_object_highlight(self, None)
            return
        try:
            iter_sel_start, iter_sel_end = text_buffer.get_selection_bounds()
            if iter_sel_end.get_offset() - iter_sel_start.get_offset() == 1:
                anchor = iter_sel_start.get_child_anchor()
                if anchor != None:
                    anchor_dir = dir(anchor)
                    if "pixbuf" in anchor_dir: support.set_object_highlight(self, anchor.eventbox)
                    elif "liststore" in anchor_dir: support.set_object_highlight(self, anchor.frame)
                    elif "sourcebuffer" in anchor_dir: support.set_object_highlight(self, anchor.frame)
        except: pass

    def update_window_save_needed(self, update_type=None, new_state_machine=False, given_tree_iter=None):
        """Window title preceeded by an asterix"""
        tree_iter = self.curr_tree_iter if not given_tree_iter else given_tree_iter
        if tree_iter and self.treestore[tree_iter][4] == cons.RICH_TEXT_ID:
            self.treestore[tree_iter][2].set_modified(True)
        if not self.file_update:
            self.window_title_update(True)
            self.file_update = True
        if update_type:
            if update_type == "nbuf":
                if tree_iter:
                    node_id = self.get_node_id_from_tree_iter(tree_iter)
                    self.ctdb_handler.pending_edit_db_node_buff(node_id)
                    curr_time = time.time()
                    self.treestore[tree_iter][13] = curr_time
                    if (not node_id in self.latest_statusbar_update_time.keys())\
                    or (curr_time - self.latest_statusbar_update_time[node_id] > 60):
                        self.latest_statusbar_update_time[node_id] = curr_time
                        self.update_selected_node_statusbar_info()
            elif update_type == "npro":
                if tree_iter:
                    self.ctdb_handler.pending_edit_db_node_prop(self.treestore[tree_iter][3])
            elif update_type == "ndel":
                if tree_iter:
                    self.ctdb_handler.pending_rm_db_node(self.treestore[tree_iter][3])
                    self.state_machine.delete_states(self.treestore[tree_iter][3])
            elif update_type == "book": self.ctdb_handler.pending_edit_db_bookmarks()
        if new_state_machine and tree_iter:
            self.state_machine.update_state()

    def update_window_save_not_needed(self):
        """Window title not preceeded by an asterix"""
        self.window_title_update(False)
        self.file_update = False
        if self.curr_tree_iter != None:
            self.curr_buffer.set_modified(False)
            curr_iter = self.curr_buffer.get_start_iter()
            while 1:
                anchor = curr_iter.get_child_anchor()
                if anchor != None and "sourcebuffer" in dir(anchor): anchor.sourcebuffer.set_modified(False)
                if not curr_iter.forward_char(): break

    def window_title_str_get(self):
        """Get Title string"""
        if self.file_name: return self.file_name + " - " + self.file_dir + " - CherryTree %s" % cons.VERSION
        return "CherryTree %s" % cons.VERSION

    def window_title_update(self, save_needed):
        """Update window title"""
        if save_needed: self.window.set_title("*" + self.window_title_str_get())
        else: self.window.set_title(self.window_title_str_get())

    def replace_again(self, *args):
        """Continue the previous replace (a_node/in_selected_node/in_all_nodes)"""
        self.find_handler.replace_again()

    def find_again(self, *args):
        """Continue the previous search (a_node/in_selected_node/in_all_nodes)"""
        self.find_handler.search_replace_dict['idialog'] = False
        self.find_handler.find_again()

    def find_back(self, *args):
        """Continue the previous search (a_node/in_selected_node/in_all_nodes) but in Opposite Direction"""
        self.find_handler.search_replace_dict['idialog'] = False
        self.find_handler.find_back()

    def replace_in_selected_node(self, *args):
        """Replace a pattern in the selected Node"""
        if not self.is_there_selected_node_or_error(): return
        self.find_handler.replace_in_selected_node()

    def find_in_selected_node(self, *args):
        """Search for a pattern in the selected Node"""
        if not self.is_there_selected_node_or_error(): return
        self.find_handler.find_in_selected_node()

    def replace_in_all_nodes(self, *args):
        """Replace the pattern in all the Tree Nodes"""
        if not self.is_tree_not_empty_or_error(): return
        self.find_handler.replace_in_all_nodes(None)

    def find_in_all_nodes(self, *args):
        """Search for a pattern in all the Tree Nodes"""
        if not self.is_tree_not_empty_or_error(): return
        self.find_handler.find_in_all_nodes(None)

    def replace_in_sel_node_and_subnodes(self, *args):
        """Replace the pattern Selected Node and SubNodes"""
        if not self.is_there_selected_node_or_error(): return
        self.find_handler.replace_in_all_nodes(self.curr_tree_iter)

    def find_in_sel_node_and_subnodes(self, *args):
        """Search for a pattern in Selected Node and SubNodes"""
        if not self.is_there_selected_node_or_error(): return
        self.find_handler.find_in_all_nodes(self.curr_tree_iter)

    def replace_in_nodes_names(self, *args):
        """Replace the pattern between all the Node's Names"""
        if not self.is_tree_not_empty_or_error(): return
        self.find_handler.replace_in_nodes_names()

    def find_a_node(self, *args):
        """Search for a pattern between all the Node's Names"""
        if not self.is_tree_not_empty_or_error(): return
        self.find_handler.find_a_node()

    def find_allmatchesdialog_restore(self, *args):
        """Restore AllMatchesDialog"""
        self.find_handler.allmatchesdialog_show()

    def get_tree_iter_last_sibling(self, node_iter):
        """Returns the last top level iter or None if the tree is empty"""
        if node_iter == None:
            node_iter = self.treestore.get_iter_first()
            if node_iter == None: return None
        next_iter = self.treestore.iter_next(node_iter)
        while next_iter != None:
            node_iter = next_iter
            next_iter = self.treestore.iter_next(next_iter)
        return node_iter

    def get_tree_iter_prev_sibling(self, model, node_iter):
        """Returns the previous sibling iter or None if the given iter is the first"""
        node_path = model.get_path(node_iter)
        sibling_index = len(node_path)-1
        prev_iter = None
        while prev_iter == None and node_path[sibling_index] > 0:
            node_path_list = list(node_path)
            node_path_list[sibling_index] -= 1
            prev_path = tuple(node_path_list)
            prev_iter = model.get_iter(prev_path)
        return prev_iter

    def go_back(self, *args):
        """Go to the Previous Visited Node"""
        self.go_bk_fw_click = True
        new_node_id = self.state_machine.requested_visited_previous()
        if new_node_id:
            node_iter = self.get_tree_iter_from_node_id(new_node_id)
            if node_iter: self.treeview_safe_set_cursor(node_iter)
            else: self.go_back()
        self.go_bk_fw_click = False

    def go_forward(self, *args):
        """Go to the Next Visited Node"""
        self.go_bk_fw_click = True
        new_node_id = self.state_machine.requested_visited_next()
        if new_node_id:
            node_iter = self.get_tree_iter_from_node_id(new_node_id)
            if node_iter: self.treeview_safe_set_cursor(node_iter)
            else: self.go_forward()
        self.go_bk_fw_click = False

    def load_buffer_from_state(self, state, given_tree_iter=None):
        """Load Text Buffer from State Machine"""
        if not given_tree_iter:
            if self.enable_spell_check:
                spell_check_restore = True
                self.toggle_ena_dis_spellcheck()
            else: spell_check_restore = False
            tree_iter = self.curr_tree_iter
            text_buffer = self.curr_buffer
        else:
            tree_iter = given_tree_iter
            text_buffer = self.get_textbuffer_from_tree_iter(tree_iter)
        if self.user_active:
            self.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        self.xml_handler.dom_to_buffer(text_buffer, state[0])
        pixbuf_table_vector = state[1]
        # pixbuf_table_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
        for element in pixbuf_table_vector:
            if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(text_buffer, element[1])
            elif element[0] == "table": self.state_machine.load_embedded_table_element(text_buffer, element[1])
            elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(text_buffer, element[1])
        if not given_tree_iter:
            self.sourceview.set_buffer(None)
            self.sourceview.set_buffer(text_buffer)
            self.objects_buffer_refresh()
            text_buffer.place_cursor(text_buffer.get_iter_at_offset(state[2]))
            self.sourceview.scroll_to_mark(text_buffer.get_insert(), cons.SCROLL_MARGIN)
        if user_active_restore: self.user_active = True
        if not given_tree_iter:
            if spell_check_restore: self.toggle_ena_dis_spellcheck()
        self.update_window_save_needed("nbuf", given_tree_iter=tree_iter)

    def requested_step_back(self, *args):
        """Step Back for the Current Node, if Possible"""
        if not self.curr_tree_iter: return
        if not self.is_curr_node_not_read_only_or_error(): return
        if self.syntax_highlighting == cons.RICH_TEXT_ID:
            # TEXT BUFFER STATE MACHINE
            step_back = self.state_machine.requested_state_previous(self.treestore[self.curr_tree_iter][3])
            # step_back is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
            if step_back != None:
                self.load_buffer_from_state(step_back)
        elif self.curr_buffer.can_undo():
            self.curr_buffer.undo()
            self.update_window_save_needed("nbuf")

    def requested_step_ahead(self, *args):
        """Step Ahead for the Current Node, if Possible"""
        if not self.curr_tree_iter: return
        if not self.is_curr_node_not_read_only_or_error(): return
        if self.syntax_highlighting == cons.RICH_TEXT_ID:
            # TEXT BUFFER STATE MACHINE
            step_ahead = self.state_machine.requested_state_subsequent(self.treestore[self.curr_tree_iter][3])
            # step_ahead is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
            if step_ahead != None:
                self.load_buffer_from_state(step_ahead)
        elif self.curr_buffer.can_redo():
            self.curr_buffer.redo()
            self.update_window_save_needed("nbuf")

    def objects_buffer_refresh(self):
        """Buffer Refresh (Needed for Objects)"""
        if not self.curr_tree_iter: return
        if self.user_active and self.curr_buffer.get_modified():
            self.file_update = True
            self.curr_buffer.set_modified(False)
            self.state_machine.update_state()
        refresh = self.state_machine.requested_state_current(self.treestore[self.curr_tree_iter][3])
        # refresh is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
        pixbuf_table_vector = refresh[1]
        if len(pixbuf_table_vector) > 0:
            if self.user_active:
                self.user_active = False
                user_active_restore = True
            else: user_active_restore = False
            iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
            iter_bound = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_selection_bound())
            insert_offset = iter_insert.get_offset()
            bound_offset = iter_bound.get_offset()
            self.curr_buffer.set_text("")
            self.xml_handler.dom_to_buffer(self.curr_buffer, refresh[0])
            for element in pixbuf_table_vector:
                if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(self.curr_buffer, element[1])
                elif element[0] == "table": self.state_machine.load_embedded_table_element(self.curr_buffer, element[1])
                elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(self.curr_buffer, element[1])
            self.curr_buffer.set_modified(False)
            self.curr_buffer.move_mark(self.curr_buffer.get_insert(), self.curr_buffer.get_iter_at_offset(insert_offset))
            self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), self.curr_buffer.get_iter_at_offset(bound_offset))
            self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
            if user_active_restore: self.user_active = True

    def on_text_insertion(self, sourcebuffer, text_iter, text_inserted, length):
        """Text insertion callback"""
        if self.user_active:
            self.state_machine.text_variation(self.treestore[self.curr_tree_iter][3], text_inserted)

    def on_text_removal(self, sourcebuffer, start_iter, end_iter):
        """Text removal callback"""
        if self.user_active and self.curr_tree_iter:
            self.state_machine.text_variation(self.treestore[self.curr_tree_iter][3],
                                              sourcebuffer.get_text(start_iter, end_iter))

    def horizontal_rule_insert(self, action):
        """Insert a Horizontal Line"""
        if not self.is_curr_node_not_read_only_or_error(): return
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        text_buffer.insert_at_cursor(cons.CHAR_NEWLINE+self.h_rule+cons.CHAR_NEWLINE)

    def dialog_nodeprop(self, title, name="", syntax_highl=cons.RICH_TEXT_ID, tags="", ro=False, c_icon_id=0, is_bold=False, fg=None):
        """Opens the Node Properties Dialog"""
        dialog = gtk.Dialog(title=title,
                            parent=self.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
        dialog.set_default_size(300, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        name_entry = gtk.Entry()
        name_entry.set_text(name)
        is_bold_checkbutton = gtk.CheckButton(label=_("Bold"))
        is_bold_checkbutton.set_active(is_bold)
        fg_checkbutton = gtk.CheckButton(label=_("Use Selected Color"))
        fg_checkbutton.set_active(fg != None)
        if fg: curr_color = gtk.gdk.color_parse(fg)
        elif self.curr_colors['n']: curr_color = self.curr_colors['n']
        else: curr_color = gtk.gdk.color_parse("red")
        fg_colorbutton = gtk.ColorButton(color=curr_color)
        fg_colorbutton.set_sensitive(fg != None)
        fg_hbox = gtk.HBox()
        fg_hbox.set_spacing(2)
        fg_hbox.pack_start(fg_checkbutton, expand=False)
        fg_hbox.pack_start(fg_colorbutton, expand=False)
        dialog.ret_fg = fg
        c_icon_checkbutton = gtk.CheckButton(label=_("Use Selected Icon"))
        c_icon_checkbutton.set_active(c_icon_id in cons.NODES_STOCKS_KEYS)
        c_icon_button = gtk.Button()
        if c_icon_checkbutton.get_active():
            c_icon_button.set_image(gtk.image_new_from_stock(cons.NODES_STOCKS[c_icon_id], gtk.ICON_SIZE_BUTTON))
        else:
            c_icon_button.set_label(_("click me"))
            c_icon_button.set_sensitive(False)
        c_icon_hbox = gtk.HBox()
        c_icon_hbox.set_spacing(2)
        c_icon_hbox.pack_start(c_icon_checkbutton, expand=False)
        c_icon_hbox.pack_start(c_icon_button, expand=False)
        dialog.ret_c_icon_id = c_icon_id
        name_vbox = gtk.VBox()
        name_vbox.pack_start(name_entry)
        name_vbox.pack_start(is_bold_checkbutton)
        name_vbox.pack_start(fg_hbox)
        name_vbox.pack_start(c_icon_hbox)
        name_frame = gtk.Frame(label="<b>"+_("Node Name")+"</b>")
        name_frame.get_label_widget().set_use_markup(True)
        name_frame.set_shadow_type(gtk.SHADOW_NONE)
        name_frame.add(name_vbox)
        radiobutton_rich_text = gtk.RadioButton(label=_("Rich Text"))
        radiobutton_plain_text = gtk.RadioButton(label=_("Plain Text"))
        radiobutton_plain_text.set_group(radiobutton_rich_text)
        radiobutton_auto_syntax_highl = gtk.RadioButton(label=_("Automatic Syntax Highlighting"))
        radiobutton_auto_syntax_highl.set_group(radiobutton_rich_text)
        button_prog_lang = gtk.Button()
        button_label = syntax_highl if syntax_highl not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID] else self.auto_syn_highl
        button_stock_id = config.get_stock_id_for_code_type(button_label)
        button_prog_lang.set_label(button_label)
        button_prog_lang.set_image(gtk.image_new_from_stock(button_stock_id, gtk.ICON_SIZE_MENU))
        if syntax_highl == cons.RICH_TEXT_ID:
            radiobutton_rich_text.set_active(True)
            button_prog_lang.set_sensitive(False)
        elif syntax_highl == cons.PLAIN_TEXT_ID:
            radiobutton_plain_text.set_active(True)
            button_prog_lang.set_sensitive(False)
        else:
            radiobutton_auto_syntax_highl.set_active(True)
        type_vbox = gtk.VBox()
        type_vbox.pack_start(radiobutton_rich_text)
        type_vbox.pack_start(radiobutton_plain_text)
        type_vbox.pack_start(radiobutton_auto_syntax_highl)
        type_vbox.pack_start(button_prog_lang)
        type_frame = gtk.Frame(label="<b>"+_("Node Type")+"</b>")
        type_frame.get_label_widget().set_use_markup(True)
        type_frame.set_shadow_type(gtk.SHADOW_NONE)
        type_frame.add(type_vbox)
        if ro: type_frame.set_sensitive(False)
        tags_hbox = gtk.HBox()
        tags_hbox.set_spacing(2)
        tags_entry = gtk.Entry()
        tags_entry.set_text(tags)
        button_browse_tags = gtk.Button()
        button_browse_tags.set_image(gtk.image_new_from_stock("find", gtk.ICON_SIZE_BUTTON))
        if not self.tags_set: button_browse_tags.set_sensitive(False)
        tags_hbox.pack_start(tags_entry)
        tags_hbox.pack_start(button_browse_tags, expand=False)
        tags_frame = gtk.Frame(label="<b>"+_("Tags for Searching")+"</b>")
        tags_frame.get_label_widget().set_use_markup(True)
        tags_frame.set_shadow_type(gtk.SHADOW_NONE)
        tags_frame.add(tags_hbox)
        ro_checkbutton = gtk.CheckButton(label=_("Read Only"))
        ro_checkbutton.set_active(ro)
        content_area = dialog.get_content_area()
        content_area.set_spacing(5)
        content_area.pack_start(name_frame)
        content_area.pack_start(type_frame)
        content_area.pack_start(tags_frame)
        content_area.pack_start(ro_checkbutton)
        content_area.show_all()
        name_entry.grab_focus()
        def on_button_prog_lang_clicked(button):
            icon_n_key_list = []
            for key in self.available_languages:
                stock_id = config.get_stock_id_for_code_type(key)
                icon_n_key_list.append([key, stock_id, key])
            sel_key = support.dialog_choose_element_in_list(self.window, _("Automatic Syntax Highlighting"), [], "", icon_n_key_list)
            if sel_key:
                button.set_label(sel_key)
                button.set_image(gtk.image_new_from_stock(sel_key, gtk.ICON_SIZE_MENU))
        button_prog_lang.connect('clicked', on_button_prog_lang_clicked)
        def on_key_press_nodepropdialog(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if keyname == cons.STR_KEY_RETURN:
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
            return False
        dialog.connect('key_press_event', on_key_press_nodepropdialog)
        def on_radiobutton_auto_syntax_highl_toggled(radiobutton):
            button_prog_lang.set_sensitive(radiobutton.get_active())
        radiobutton_auto_syntax_highl.connect("toggled", on_radiobutton_auto_syntax_highl_toggled)
        def on_browse_tags_button_clicked(button):
            ret_tag_name = support.dialog_choose_element_in_list(dialog,
                _("Choose Existing Tag"), [[element] for element in sorted(self.tags_set)], _("Tag Name"))
            if ret_tag_name:
                curr_text = tags_entry.get_text()
                print curr_text, ret_tag_name
                if not curr_text: tags_entry.set_text(ret_tag_name)
                elif curr_text.endswith(cons.CHAR_SPACE): tags_entry.set_text(curr_text+ret_tag_name)
                else: tags_entry.set_text(curr_text+cons.CHAR_SPACE+ret_tag_name)
        button_browse_tags.connect('clicked', on_browse_tags_button_clicked)
        def on_checkbutton_ro_toggled(checkbutton):
            type_frame.set_sensitive(not checkbutton.get_active())
        ro_checkbutton.connect('toggled', on_checkbutton_ro_toggled)
        def on_fg_checkbutton_toggled(checkbutton):
            fg_colorbutton.set_sensitive(checkbutton.get_active())
        fg_checkbutton.connect('toggled', on_fg_checkbutton_toggled)
        def on_fg_colorbutton_press_event(colorbutton, event):
            ret_color = support.dialog_color_pick(self, colorbutton.get_color())
            if ret_color:
                colorbutton.set_color(ret_color)
            return True
        fg_colorbutton.connect('button-press-event', on_fg_colorbutton_press_event)
        def on_c_icon_checkbutton_toggled(checkbutton):
            c_icon_button.set_sensitive(checkbutton.get_active())
        c_icon_checkbutton.connect('toggled', on_c_icon_checkbutton_toggled)
        def on_c_icon_button_clicked(button):
            icon_n_label_list = []
            for key in cons.NODES_STOCKS_KEYS:
                icon_n_label_list.append([str(key), cons.NODES_STOCKS[key], ""])
            sel_key = support.dialog_choose_element_in_list(dialog, _("Select Node Icon"), [], "", icon_n_label_list)
            if sel_key:
                dialog.ret_c_icon_id = int(sel_key)
                c_icon_button.set_label("")
                c_icon_button.set_image(gtk.image_new_from_stock(cons.NODES_STOCKS[dialog.ret_c_icon_id], gtk.ICON_SIZE_BUTTON))
        c_icon_button.connect('clicked', on_c_icon_button_clicked)
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            ret_name = unicode(name_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            if not ret_name: ret_name = cons.CHAR_QUESTION
            if radiobutton_rich_text.get_active(): ret_syntax = cons.RICH_TEXT_ID
            elif radiobutton_plain_text.get_active(): ret_syntax = cons.PLAIN_TEXT_ID
            else:
                ret_syntax = button_prog_lang.get_label()
                self.auto_syn_highl = ret_syntax
            ret_tags = unicode(tags_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            ret_ro = ro_checkbutton.get_active()
            ret_c_icon_id = dialog.ret_c_icon_id if c_icon_checkbutton.get_active() else 0
            ret_is_bold = is_bold_checkbutton.get_active()
            if fg_checkbutton.get_active():
                ret_fg = "#" + exports.rgb_any_to_24(fg_colorbutton.get_color().to_string()[1:])
                self.curr_colors['n'] = fg_colorbutton.get_color()
            else:
                ret_fg = None
            return [ret_name, ret_syntax, ret_tags, ret_ro, ret_c_icon_id, ret_is_bold, ret_fg]
        return [None, None, None, None, None, None, None]

    def toolbar_icons_size_increase(self, *args):
        """Increase the Size of the Toolbar Icons"""
        if self.toolbar_icon_size == 5:
            support.dialog_info(_("The Size of the Toolbar Icons is already at the Maximum Value"), self.window)
            return
        self.toolbar_icon_size += 1
        self.ui.get_widget("/ToolBar").set_property("icon-size", config.ICONS_SIZE[self.toolbar_icon_size])

    def toolbar_icons_size_decrease(self, *args):
        """Decrease the Size of the Toolbar Icons"""
        if self.toolbar_icon_size == 1:
            support.dialog_info(_("The Size of the Toolbar Icons is already at the Minimum Value"), self.window)
            return
        self.toolbar_icon_size -= 1
        self.ui.get_widget("/ToolBar").set_property("icon-size", config.ICONS_SIZE[self.toolbar_icon_size])

    def toggle_show_hide_toolbar(self, *args):
        """Toggle Show/Hide the Toolbar"""
        old_toolbar_status = self.ui.get_widget("/ToolBar").get_property(cons.STR_VISIBLE)
        self.toolbar_visible = not old_toolbar_status
        self.ui.get_widget("/ToolBar").set_property(cons.STR_VISIBLE, self.toolbar_visible)

    def toggle_show_hide_tree(self, *args):
        """Toggle Show/Hide the Tree"""
        if self.scrolledwindow_tree.get_property(cons.STR_VISIBLE):
            self.scrolledwindow_tree.hide()
        else: self.scrolledwindow_tree.show()

    def toggle_show_hide_node_name_header(self, *args):
        """Toggle Show/Hide the Node Title Header"""
        old_show_node_name_label = self.header_node_name_hbox.get_property(cons.STR_VISIBLE)
        self.show_node_name_header = not old_show_node_name_label
        self.header_node_name_hbox.set_property(cons.STR_VISIBLE, self.show_node_name_header)

    def quit_application(self, *args):
        """Just Hide or Quit the gtk main loop"""
        if self.systray:
            self.win_position = self.window.get_position()
            self.window.hide_all()
        else: self.quit_application_totally()

    def quit_application_totally(self, *args):
        """The process is Shut Down"""
        if self.embfiles_opened and not support.dialog_exit_del_temp_files(self):
            self.really_quit = False # user pressed cancel
            return
        if not self.check_unsaved():
            self.really_quit = False # user pressed cancel
            return
        config.config_file_save(self)
        if "db" in dir(self) and self.db: self.db.close()
        for filepath_tmp in self.ctdb_handler.remove_at_quit_set:
            if os.path.isfile(filepath_tmp):
                os.remove(filepath_tmp)
        for filepath_tmp in self.embfiles_opened.keys(): os.remove(filepath_tmp)
        self.window.destroy()
        if not self.boss.running_windows:
            if not self.use_appind and "status_icon" in dir(self.boss): self.boss.status_icon.set_property(cons.STR_VISIBLE, False)

    def on_window_delete_event(self, widget, event, data=None):
        """Before close the application (from the window top right X)..."""
        self.really_quit = True
        self.quit_application()
        if not self.really_quit: return True # stop the delete event (user pressed cancel)
        else: return self.systray # True == stop the delete event, False == propogate the delete event

    def check_unsaved(self):
        """Before close the current document, check for possible Unsaved"""
        if self.curr_tree_iter and (self.curr_buffer.get_modified() or self.file_update):
            if self.autosave_on_quit: response = 2
            else: response = support.dialog_exit_save(self.window)
            if response == 2: self.file_save() # button YES pressed or autosave ON
            elif response < 0: response = 6
        else: response = 0 # no need to save
        if response == 6: return False # button CANCEL
        return True

    def dialog_about(self, *args):
        """Show the About Dialog and hide it when a button is pressed"""
        support.dialog_about(self)

    def anchor_handle(self, action):
        """Insert an Anchor"""
        if not self.node_sel_and_rich_text(): return
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(cons.ANCHOR_CHAR, self.anchor_size, self.anchor_size)
        self.anchor_edit_dialog(pixbuf, iter_insert)

    def anchor_edit(self, *args):
        """Edit an Anchor"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_insert = self.curr_buffer.get_iter_at_child_anchor(self.curr_anchor_anchor)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        self.anchor_edit_dialog(self.curr_anchor_anchor.pixbuf, iter_insert, iter_bound)

    def anchor_edit_dialog(self, pixbuf, iter_insert, iter_bound=None):
        """Anchor Edit Dialog"""
        if "anchor" in dir (pixbuf):
            anchor_curr_name = pixbuf.anchor
            dialog_title = _("Edit Anchor")
        else:
            anchor_curr_name = ""
            dialog_title = _("Insert Anchor")
        ret_anchor_name = support.dialog_img_n_entry(self.window, dialog_title, anchor_curr_name, "anchor")
        if not ret_anchor_name: return
        pixbuf.anchor = ret_anchor_name
        if iter_bound != None: # only in case of modify
            image_justification = self.state_machine.get_iter_alignment(iter_insert)
            image_offset = iter_insert.get_offset()
            self.curr_buffer.delete(iter_insert, iter_bound)
            iter_insert = self.curr_buffer.get_iter_at_offset(image_offset)
        else: image_justification = None
        self.image_insert(iter_insert, pixbuf, image_justification)

    def exec_code(self, *args):
        """Exec Code"""
        if not self.is_there_selected_node_or_error(): return
        if self.syntax_highlighting == cons.RICH_TEXT_ID:
            code_type = None
            anchor = self.codeboxes_handler.codebox_in_use_get_anchor()
            if anchor:
                code_type = anchor.syntax_highlighting
                code_val = unicode(anchor.sourcebuffer.get_text(*anchor.sourcebuffer.get_bounds()), cons.STR_UTF8, cons.STR_IGNORE)
            if not code_type:
                support.dialog_warning(_("No CodeBox is Selected"), self.window)
                return
        else:
            code_type = self.syntax_highlighting
            code_val = unicode(self.curr_buffer.get_text(*self.curr_buffer.get_bounds()), cons.STR_UTF8, cons.STR_IGNORE)
        #print code_type
        binary_cmd = config.get_code_exec_type_cmd(self, code_type)
        if not binary_cmd:
            support.dialog_warning(_("You must associate a command to '%s'.\nDo so in the Preferences Dialog") % code_type, self.window)
            return
        filepath_src_tmp = os.path.join(cons.TMP_FOLDER, "exec_code."+config.get_code_exec_ext(self, code_type))
        filepath_bin_tmp = os.path.join(cons.TMP_FOLDER, "exec_code.exe")
        binary_cmd = binary_cmd.replace(config.CODE_EXEC_TMP_SRC, filepath_src_tmp).replace(config.CODE_EXEC_TMP_BIN, filepath_bin_tmp)
        terminal_cmd = config.get_code_exec_term_run(self).replace(config.CODE_EXEC_COMMAND, binary_cmd)
        #print terminal_cmd
        warning_label = "<b>"+_("Do you want to Execute the Code?")+"</b>"
        response = support.dialog_question_warning(self.window, warning_label)
        if response != gtk.RESPONSE_ACCEPT:
            return # the user did not confirm
        if not os.path.isdir(cons.TMP_FOLDER): os.makedirs(cons.TMP_FOLDER)
        with open(filepath_src_tmp, 'w') as fd:
            fd.write(code_val)
        ret_code = subprocess.call(terminal_cmd, shell=True)
        if ret_code and terminal_cmd.startswith("xterm ") and subprocess.call("xterm -version", shell=True):
            support.dialog_error(_("Install the package 'xterm' or configure a different terminal in the Preferences Dialog"), self.window)
        self.ctdb_handler.remove_at_quit_set.add(filepath_src_tmp)
        self.ctdb_handler.remove_at_quit_set.add(filepath_bin_tmp)

    def embfile_insert(self, *args):
        """Embedded File Insert"""
        if not self.node_sel_and_rich_text(): return
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        filepath = support.dialog_file_select(curr_folder=self.pick_dir_file, parent=self.window)
        if not filepath: return
        self.pick_dir_file = os.path.dirname(filepath)
        if os.path.getsize(filepath) > self.embfile_max_size*1024*1024:
            support.dialog_error(_("The Maximum Size for Embedded Files is %s MB") % self.embfile_max_size, self.window)
            return
        pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(cons.FILE_CHAR, self.embfile_size, self.embfile_size)
        pixbuf.filename = os.path.basename(filepath)
        with open(filepath, 'rb') as fd:
            pixbuf.embfile = fd.read()
        pixbuf.time = time.time()
        self.image_insert(iter_insert, pixbuf, image_justification=None)

    def embfile_open(self, *args):
        """Embedded File Open"""
        if not hasattr(self.curr_file_anchor.pixbuf, "id"):
            self.boss.embfiles_id += 1
            self.curr_file_anchor.pixbuf.id = self.boss.embfiles_id
        filename = str(self.treestore[self.curr_tree_iter][3])+cons.CHAR_MINUS+str(self.curr_file_anchor.pixbuf.id)+cons.CHAR_MINUS+str(os.getpid())+cons.CHAR_MINUS+self.curr_file_anchor.pixbuf.filename
        filepath = os.path.join(cons.TMP_FOLDER, filename)
        if not os.path.isdir(cons.TMP_FOLDER): os.makedirs(cons.TMP_FOLDER)
        with open(filepath, 'wb') as fd:
            fd.write(self.curr_file_anchor.pixbuf.embfile)
        #if self.treestore[self.curr_tree_iter][7]:
        #    os.chmod(filepath, os.stat(filepath).st_mode & ~stat.S_IWUSR & ~stat.S_IWGRP & ~stat.S_IWOTH)
        print "embopen", filepath
        self.external_filepath_open(filepath, False)
        self.embfiles_opened[filepath] = os.path.getmtime(filepath)
        if not self.embfiles_sentinel_id: self.embfiles_sentinel_start()

    def embfile_save(self, *args):
        """Embedded File Save Dialog"""
        iter_insert = self.curr_buffer.get_iter_at_child_anchor(self.curr_file_anchor)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        filepath = support.dialog_file_save_as(filename=self.curr_file_anchor.pixbuf.filename,
            curr_folder=self.pick_dir_file,
            parent=self.window)
        if not filepath: return
        self.pick_dir_file = os.path.dirname(filepath)
        with open(filepath, 'wb') as fd:
            fd.write(self.curr_file_anchor.pixbuf.embfile)

    def embfiles_sentinel_start(self):
        """Start Timer that checks for modification time"""
        self.embfiles_sentinel_id = gobject.timeout_add(500, self.embfiles_sentinel_iter) # 1/2 sec

    def embfiles_sentinel_stop(self):
        """Stop Timer that checks for modification time"""
        gobject.source_remove(self.embfiles_sentinel_id)
        self.embfiles_sentinel_id = None

    def embfiles_sentinel_iter(self):
        """Iteration of the Modification Time Sentinel"""
        for filepath in self.embfiles_opened.keys():
            if not os.path.isfile(filepath):
                print "embdrop", filepath
                del self.embfiles_opened[filepath]
                break
            if self.embfiles_opened[filepath] != os.path.getmtime(filepath):
                self.embfiles_opened[filepath] = os.path.getmtime(filepath)
                data_vec = os.path.basename(filepath).split(cons.CHAR_MINUS)
                node_id = int(data_vec[0])
                embfile_id = int(data_vec[1])
                tree_iter = self.get_tree_iter_from_node_id(node_id)
                if not tree_iter: continue
                is_ro = self.get_node_read_only(tree_iter)
                if is_ro:
                    support.dialog_warning(_("Cannot Edit Embedded File in Read Only Node"), self.window)
                    continue
                self.treeview_safe_set_cursor(tree_iter)
                start_iter = self.curr_buffer.get_start_iter()
                keep_going = True
                while keep_going:
                    anchor = start_iter.get_child_anchor()
                    if anchor and "pixbuf" in dir(anchor) and "id" in dir(anchor.pixbuf) and anchor.pixbuf.id == embfile_id:
                        with open(filepath, 'rb') as fd:
                            anchor.pixbuf.embfile = fd.read()
                            anchor.pixbuf.time = time.time()
                        self.embfile_set_tooltip(anchor)
                        self.update_window_save_needed("nbuf")
                        self.statusbar.pop(self.statusbar_context_id)
                        self.statusbar.push(self.statusbar_context_id, _("Embedded File Automatically Updated:") + cons.CHAR_SPACE + anchor.pixbuf.filename)
                    keep_going = start_iter.forward_char()
        return True # this way we keep the timer alive

    def codebox_sentinel_start(self):
        """Start Timer that monitors CodeBox"""
        self.codebox_sentinel_id = gobject.timeout_add(250, self.codebox_sentinel_iter) # 1/4 sec

    def codebox_sentinel_stop(self):
        """Stop Timer that monitors CodeBox"""
        gobject.source_remove(self.codebox_sentinel_id)
        self.codebox_sentinel_id = None

    def codebox_sentinel_iter(self):
        """Iteration of the CodeBox Sentinel"""
        if not self.codeboxes_handler.key_down:
            if self.codeboxes_handler.curr_v > 0:
                #print "codebox_v", self.codeboxes_handler.curr_v
                self.codeboxes_handler.codebox_increase_height()
                self.codeboxes_handler.curr_v = 0
            if self.codeboxes_handler.curr_h > 0:
                #print "codebox_h", self.codeboxes_handler.curr_h
                self.codeboxes_handler.codebox_increase_width()
                self.codeboxes_handler.curr_h = 0
        return True # this way we keep the timer alive

    def toc_insert(self, *args):
        """Insert Table Of Contents"""
        if not self.is_there_selected_node_or_error(): return
        if not self.node_sel_and_rich_text(): return
        toc_type = support.dialog_selnode_selnodeandsub_alltree(self, also_selection=False)
        if toc_type == 0: return
        if self.user_active:
            self.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        if toc_type == 1:
            # only selected node
            ret_toc_list = self.xml_handler.toc_insert_one(self.curr_buffer, self.treestore[self.curr_tree_iter][3])
        elif toc_type == 2:
            # selected node and subnodes
            ret_toc_list = self.xml_handler.toc_insert_all(self.curr_buffer, self.curr_tree_iter)
        else:
            # all nodes
            ret_toc_list = self.xml_handler.toc_insert_all(self.curr_buffer, None)
        if user_active_restore: self.user_active = True
        if ret_toc_list:
            self.file_update = False
            self.update_window_save_needed("nbuf")
        else: support.dialog_warning(_("Not Any H1, H2 or H3 Formatting Found"), self.window)

    def table_handle(self, *args):
        """Insert Table"""
        if not self.node_sel_and_rich_text(): return
        if not self.is_curr_node_not_read_only_or_error(): return
        self.tables_handler.table_handle()

    def codebox_handle(self, *args):
        """Insert Code Box"""
        if not self.node_sel_and_rich_text(): return
        if not self.is_curr_node_not_read_only_or_error(): return
        self.codeboxes_handler.codebox_handle()

    def is_curr_node_not_syntax_highlighting_or_error(self, plain_text_ok=False):
        """Returns True if ok (no syntax highlighting) or False and prompts error dialog"""
        if self.syntax_highlighting == cons.RICH_TEXT_ID\
        or (plain_text_ok and self.syntax_highlighting == cons.PLAIN_TEXT_ID):
            return True
        if not plain_text_ok:
            support.dialog_warning(_("This Feature is Available Only in Rich Text Nodes"), self.window)
        else:
            support.dialog_warning(_("This Feature is Not Available in Automatic Syntax Highlighting Nodes"), self.window)
        return False

    def is_there_selected_node_or_error(self):
        """Returns True if ok (there's a selected node) or False and prompts error dialog"""
        if not self.curr_tree_iter:
            support.dialog_warning(_("No Node is Selected"), self.window)
            return False
        return True

    def is_tree_not_empty_or_error(self):
        """Returns True if the Tree is Not Empty or False and prompts error dialog"""
        if self.tree_is_empty():
            support.dialog_error(_("The Tree is Empty!"), self.window)
            return False
        return True

    def is_there_text_selection_or_error(self):
        """Returns True if ok (there's a selection) or False and prompts error dialog"""
        if not self.is_there_selected_node_or_error(): return False
        if not self.curr_buffer.get_has_selection():
            support.dialog_error(_("No Text is Selected"), self.window)
            return False
        return True

    def get_node_read_only(self, tree_iter=None):
        """Returns True if the Given Node is Read Only"""
        if tree_iter is None: tree_iter = self.curr_tree_iter
        return tree_iter and self.treestore[tree_iter][7]

    def is_curr_node_not_read_only_or_error(self):
        """Returns False if the Current Selected Node is Read Only and prompts error dialog"""
        if self.get_node_read_only():
            support.dialog_error(_("The Selected Node is Read Only"), self.window)
            return False
        return True

    def node_sel_and_rich_text(self):
        """Returns True if there's not a node selected or is not rich text"""
        if not self.is_there_selected_node_or_error(): return False
        if not self.is_curr_node_not_syntax_highlighting_or_error(): return False
        return True

    def tree_info(self, action):
        """Tree Summary Information"""
        if not self.is_tree_not_empty_or_error(): return
        dialog = gtk.Dialog(title=_("Tree Summary Information"),
                            parent=self.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
        dialog.set_default_size(400, 300)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        table = gtk.Table(8, 2)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Rich Text Nodes") + "</b>")
        table.attach(label, 0, 1, 0, 1)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Plain Text Nodes") + "</b>")
        table.attach(label, 0, 1, 1, 2)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Code Nodes") + "</b>")
        table.attach(label, 0, 1, 2, 3)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Images") + "</b>")
        table.attach(label, 0, 1, 3, 4)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Embedded Files") + "</b>")
        table.attach(label, 0, 1, 4, 5)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Tables") + "</b>")
        table.attach(label, 0, 1, 5, 6)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of CodeBoxes") + "</b>")
        table.attach(label, 0, 1, 6, 7)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Anchors") + "</b>")
        table.attach(label, 0, 1, 7, 8)
        self.summary_nodes_rich_text_num = 0
        self.summary_nodes_plain_text_num = 0
        self.summary_nodes_code_num = 0
        self.summary_images_num = 0
        self.summary_embfile_num = 0
        self.summary_tables_num = 0
        self.summary_codeboxes_num = 0
        self.summary_anchors_num = 0
        # full tree parsing
        tree_iter = self.treestore.get_iter_first()
        while tree_iter != None:
            self.tree_info_iter(tree_iter)
            tree_iter = self.treestore.iter_next(tree_iter)
        self.objects_buffer_refresh()
        label = gtk.Label("%s" % self.summary_nodes_rich_text_num)
        table.attach(label, 1, 2, 0, 1)
        label = gtk.Label("%s" % self.summary_nodes_plain_text_num)
        table.attach(label, 1, 2, 1, 2)
        label = gtk.Label("%s" % self.summary_nodes_code_num)
        table.attach(label, 1, 2, 2, 3)
        label = gtk.Label("%s" % self.summary_images_num)
        table.attach(label, 1, 2, 3, 4)
        label = gtk.Label("%s" % self.summary_embfile_num)
        table.attach(label, 1, 2, 4, 5)
        label = gtk.Label("%s" % self.summary_tables_num)
        table.attach(label, 1, 2, 5, 6)
        label = gtk.Label("%s" % self.summary_codeboxes_num)
        table.attach(label, 1, 2, 6, 7)
        label = gtk.Label("%s" % self.summary_anchors_num)
        table.attach(label, 1, 2, 7, 8)
        content_area = dialog.get_content_area()
        content_area.pack_start(table)
        content_area.show_all()
        dialog.get_action_area().set_layout(gtk.BUTTONBOX_SPREAD)
        dialog.run()
        dialog.destroy()
        if self.enable_spell_check and self.syntax_highlighting == cons.RICH_TEXT_ID:
            self.spell_check_set_on()

    def tree_info_iter(self, tree_iter):
        """Tree Summary Information Iteration"""
        curr_buffer = self.get_textbuffer_from_tree_iter(tree_iter)
        pixbuf_table_vector = self.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer)
        # pixbuf_table_vector is [ [ "pixbuf"/"table", [offset, pixbuf, alignment] ],... ]
        if self.treestore[tree_iter][4] == cons.RICH_TEXT_ID: self.summary_nodes_rich_text_num += 1
        elif self.treestore[tree_iter][4] == cons.PLAIN_TEXT_ID: self.summary_nodes_plain_text_num += 1
        else: self.summary_nodes_code_num += 1
        curr_node_images = 0
        curr_node_embfiles = 0
        curr_node_tables = 0
        curr_node_codeboxes = 0
        curr_node_anchors = 0
        for element in pixbuf_table_vector:
            if element[0] == "pixbuf":
                pixbuf_attrs = dir(element[1][1])
                if "anchor" in pixbuf_attrs: curr_node_anchors += 1
                elif "embfile" in pixbuf_attrs: curr_node_embfiles += 1
                else: curr_node_images += 1
            elif element[0] == "table": curr_node_tables += 1
            elif element[0] == "codebox": curr_node_codeboxes += 1
        if curr_node_images or curr_node_embfiles or curr_node_tables or curr_node_codeboxes or curr_node_anchors:
            #print "node with object(s):", self.treestore[tree_iter][1]
            self.summary_images_num += curr_node_images
            self.summary_embfile_num += curr_node_embfiles
            self.summary_tables_num += curr_node_tables
            self.summary_codeboxes_num += curr_node_codeboxes
            self.summary_anchors_num += curr_node_anchors
        # iterate children
        tree_iter = self.treestore.iter_children(tree_iter)
        while tree_iter != None:
            self.tree_info_iter(tree_iter)
            tree_iter = self.treestore.iter_next(tree_iter)

    def image_handle(self, *args):
        """Insert/Edit Image"""
        if not self.node_sel_and_rich_text(): return
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        filename = support.dialog_file_select(curr_folder=self.pick_dir_img, parent=self.window)
        if not filename: return
        self.pick_dir_img = os.path.dirname(filename)
        try:
            pixbuf = gtk.gdk.pixbuf_new_from_file(filename)
            self.image_edit_dialog(pixbuf, self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert()))
        except:
            support.dialog_error(_("Image Format Not Recognized"), self.window)

    def image_edit_dialog(self, pixbuf, insert_iter, iter_bound=None):
        """Insert/Edit Image Dialog"""
        ret_pixbuf = support.dialog_image_handle(self.window, _("Image Properties"), pixbuf)
        if not ret_pixbuf: return
        ret_pixbuf.link = ""
        if iter_bound != None: # only in case of modify
            image_justification = self.state_machine.get_iter_alignment(insert_iter)
            image_offset = insert_iter.get_offset()
            self.curr_buffer.delete(insert_iter, iter_bound)
            insert_iter = self.curr_buffer.get_iter_at_offset(image_offset)
        else: image_justification = None
        self.image_insert(insert_iter, ret_pixbuf, image_justification)

    def image_frame_get_link_color(self, link):
        """From Image Link to Color"""
        if link.startswith("webs"): return self.col_link_webs
        if link.startswith("node"): return self.col_link_node
        if link.startswith("file"): return self.col_link_file
        return self.col_link_fold

    def image_insert(self, iter_insert, pixbuf, image_justification=None, text_buffer=None):
        if not pixbuf: return
        if not text_buffer: text_buffer = self.curr_buffer
        image_offset = iter_insert.get_offset()
        anchor = text_buffer.create_child_anchor(iter_insert)
        anchor.pixbuf = pixbuf
        anchor.eventbox = gtk.EventBox()
        anchor.eventbox.set_visible_window(False)
        anchor.frame = gtk.Frame()
        anchor.frame.set_shadow_type(gtk.SHADOW_NONE)
        pixbuf_attrs = dir(pixbuf)
        if not hasattr(anchor.pixbuf, "link"): anchor.pixbuf.link = ""
        if "anchor" in pixbuf_attrs:
            anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_anchor, anchor)
            anchor.eventbox.set_tooltip_text(pixbuf.anchor)
        elif "filename" in pixbuf_attrs:
            anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_file, anchor)
            self.embfile_set_tooltip(anchor)
            if self.embfile_show_filename:
                anchor_label = gtk.Label()
                anchor_label.set_markup("<b><small>"+pixbuf.filename+"</small></b>")
                anchor_label.modify_fg(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.rt_def_fg))
                anchor.frame.set_label_widget(anchor_label)
        else:
            anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_image, anchor)
            anchor.eventbox.connect("visibility-notify-event", self.on_image_visibility_notify_event)
            if anchor.pixbuf.link:
                self.image_link_apply_frame_label(anchor)
        anchor.image = gtk.Image()
        anchor.frame.add(anchor.image)
        anchor.eventbox.add(anchor.frame)
        anchor.image.set_from_pixbuf(anchor.pixbuf)
        self.sourceview.add_child_at_anchor(anchor.eventbox, anchor)
        anchor.eventbox.show_all()
        if image_justification:
            text_iter = text_buffer.get_iter_at_offset(image_offset)
            self.state_machine.apply_object_justification(text_iter, image_justification, text_buffer)
        elif self.user_active:
            # if I apply a justification, the state is already updated
            self.state_machine.update_state()

    def embfile_set_tooltip(self, anchor):
        """Set Embedded File Tooltip"""
        embfile_bytes = len(anchor.pixbuf.embfile)
        embfile_Kbytes = float(embfile_bytes)/1024
        embfile_Mbytes = embfile_Kbytes/1024
        if embfile_Mbytes > 1: human_readable_size = "%.1f MB" % embfile_Mbytes
        else: human_readable_size = "%.1f KB" % embfile_Kbytes
        timestamp = support.get_timestamp_str(self.timestamp_format, anchor.pixbuf.time)
        anchor.eventbox.set_tooltip_text("%s\n%s (%d Bytes)\n%s" % (anchor.pixbuf.filename, human_readable_size, embfile_bytes, timestamp))

    def image_edit(self, *args):
        """Edit the selected Image"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_insert = self.curr_buffer.get_iter_at_child_anchor(self.curr_image_anchor)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        self.image_edit_dialog(self.curr_image_anchor.pixbuf, iter_insert, iter_bound)

    def image_save(self, *args):
        """Save to Disk the selected Image"""
        filename = support.dialog_file_save_as(curr_folder=self.pick_dir_img,
                                               filter_pattern="*.png",
                                               filter_name=_("PNG Image"),
                                               parent=self.window)
        if not filename: return
        self.pick_dir_img = os.path.dirname(filename)
        if len(filename) < 4 or filename[-4:] != ".png": filename += ".png"
        try: self.curr_image_anchor.pixbuf.save(filename, "png")
        except: support.dialog_error(_("Write to %s Failed") % filename, self.window)

    def object_set_selection(self, anchor):
        """Put Selection Upon the Image"""
        iter_object = self.curr_buffer.get_iter_at_child_anchor(anchor)
        iter_bound = iter_object.copy()
        iter_bound.forward_char()
        if "pixbuf" in dir(anchor): self.sourceview.grab_focus()
        self.curr_buffer.select_range(iter_object, iter_bound)

    def copy_as_plain_text(self, *args):
        """Copy as Plain Text"""
        self.clipboard_handler.force_plain_text = True
        self.sourceview.emit("copy-clipboard")

    def cut_as_plain_text(self, *args):
        """Copy as Plain Text"""
        self.clipboard_handler.force_plain_text = True
        self.sourceview.emit("cut-clipboard")

    def paste_as_plain_text(self, *args):
        """Paste as Plain Text"""
        self.clipboard_handler.force_plain_text = True
        self.sourceview.emit("paste-clipboard")

    def image_cut(self, *args):
        """Cut Image"""
        self.object_set_selection(self.curr_image_anchor)
        self.sourceview.emit("cut-clipboard")

    def image_copy(self, *args):
        """Copy Image"""
        self.object_set_selection(self.curr_image_anchor)
        self.sourceview.emit("copy-clipboard")

    def image_delete(self, *args):
        """Delete Image"""
        self.object_set_selection(self.curr_image_anchor)
        self.curr_buffer.delete_selection(True, self.sourceview.get_editable())
        self.sourceview.grab_focus()

    def embfile_cut(self, *args):
        """Cut Embedded File"""
        self.object_set_selection(self.curr_file_anchor)
        self.sourceview.emit("cut-clipboard")

    def embfile_copy(self, *args):
        """Copy Embedded File"""
        self.object_set_selection(self.curr_file_anchor)
        self.sourceview.emit("copy-clipboard")

    def embfile_delete(self, *args):
        """Delete Embedded File"""
        self.object_set_selection(self.curr_file_anchor)
        self.curr_buffer.delete_selection(True, self.sourceview.get_editable())
        self.sourceview.grab_focus()

    def anchor_cut(self, *args):
        """Cut Anchor"""
        self.object_set_selection(self.curr_anchor_anchor)
        self.sourceview.emit("cut-clipboard")

    def anchor_copy(self, *args):
        """Copy Anchor"""
        self.object_set_selection(self.curr_anchor_anchor)
        self.sourceview.emit("copy-clipboard")

    def anchor_delete(self, *args):
        """Delete Anchor"""
        self.object_set_selection(self.curr_anchor_anchor)
        self.curr_buffer.delete_selection(True, self.sourceview.get_editable())
        self.sourceview.grab_focus()

    def on_mouse_button_clicked_image(self, widget, event, anchor):
        """Catches mouse buttons clicks upon images"""
        self.curr_image_anchor = anchor
        self.object_set_selection(self.curr_image_anchor)
        if event.button in [1, 2]:
            if event.type == gtk.gdk._2BUTTON_PRESS:
                self.image_edit()
            elif self.curr_image_anchor.pixbuf.link:
                self.link_clicked(self.curr_image_anchor.pixbuf.link, event.button == 2)
        elif event.button == 3:
            if self.curr_image_anchor.pixbuf.link: self.ui.get_widget("/ImageMenu/img_link_dismiss").show()
            else: self.ui.get_widget("/ImageMenu/img_link_dismiss").hide()
            self.ui.get_widget("/ImageMenu").popup(None, None, None, event.button, event.time)
        return True # do not propagate the event

    def on_mouse_button_clicked_file(self, widget, event, anchor):
        """Catches mouse buttons clicks upon file images"""
        self.curr_file_anchor = anchor
        self.object_set_selection(self.curr_file_anchor)
        if event.button == 3:
            self.ui.get_widget("/EmbFileMenu").popup(None, None, None, event.button, event.time)
        elif event.type == gtk.gdk._2BUTTON_PRESS: self.embfile_open()
        return True # do not propagate the event

    def on_mouse_button_clicked_anchor(self, widget, event, anchor):
        """Catches mouse buttons clicks upon anchor images"""
        self.curr_anchor_anchor = anchor
        self.object_set_selection(self.curr_anchor_anchor)
        if event.button == 3:
            self.ui.get_widget("/AnchorMenu").popup(None, None, None, event.button, event.time)
        elif event.type == gtk.gdk._2BUTTON_PRESS: self.anchor_edit()
        return True # do not propagate the event

    def strip_trailing_spaces(self, *args):
        """Remove trailing spaces/tabs"""
        cleaned_lines = support.strip_trailing_spaces(self.curr_buffer)
        support.dialog_info("%s " % cleaned_lines + _("Lines Stripped"), self.window)

    def apply_tag_foreground(self, *args):
        """The Foreground Color Chooser Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_FOREGROUND)

    def apply_tag_background(self, *args):
        """The Background Color Chooser Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_BACKGROUND)

    def apply_tag_link(self, *args):
        """The Link Insert Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_LINK)

    def apply_tag_bold(self, *args):
        """The Bold Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_WEIGHT, cons.TAG_PROP_HEAVY)

    def apply_tag_italic(self, *args):
        """The Italic Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_STYLE, cons.TAG_PROP_ITALIC)

    def apply_tag_underline(self, *args):
        """The Underline Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_UNDERLINE, cons.TAG_PROP_SINGLE)

    def apply_tag_strikethrough(self, *args):
        """The Strikethrough Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_STRIKETHROUGH, cons.TAG_PROP_TRUE)

    def apply_tag_small(self, *args):
        """The Small Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_SMALL)

    def apply_tag_subscript(self, *args):
        """The Subscript Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_SUB)

    def apply_tag_superscript(self, *args):
        """The Superscript Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_SUP)

    def apply_tag_monospace(self, *args):
        """The Monospace Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.apply_tag(cons.TAG_FAMILY, cons.TAG_PROP_MONOSPACE)

    def apply_tag_h1(self, *args):
        """The H1 Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if not iter_start: return
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_H1, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_h2(self, *args):
        """The H2 Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if not iter_start: return
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_H2, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_h3(self, *args):
        """The H3 Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if not iter_start: return
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_H3, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_justify_right(self, *args):
        """The Justify Right Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if not iter_start: return
        self.apply_tag(cons.TAG_JUSTIFICATION, cons.TAG_PROP_RIGHT, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_justify_left(self, *args):
        """The Justify Left Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if not iter_start: return
        self.apply_tag(cons.TAG_JUSTIFICATION, cons.TAG_PROP_LEFT, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_justify_center(self, *args):
        """The Justify Center Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if not iter_start: return
        self.apply_tag(cons.TAG_JUSTIFICATION, cons.TAG_PROP_CENTER, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_justify_fill(self, *args):
        """The Justify Fill Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if not iter_start: return
        self.apply_tag(cons.TAG_JUSTIFICATION, cons.TAG_PROP_FILL, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def list_bulleted_handler(self, *args):
        """Handler of the Bulleted List"""
        if not self.is_curr_node_not_read_only_or_error(): return
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if from_codebox or self.is_curr_node_not_syntax_highlighting_or_error(plain_text_ok=True):
            self.lists_handler.list_handler(-1, text_buffer=text_buffer)

    def list_numbered_handler(self, *args):
        """Handler of the Numbered List"""
        if not self.is_curr_node_not_read_only_or_error(): return
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if from_codebox or self.is_curr_node_not_syntax_highlighting_or_error(plain_text_ok=True):
            self.lists_handler.list_handler(1, text_buffer=text_buffer)

    def list_todo_handler(self, *args):
        """Handler of the ToDo List"""
        if not self.is_curr_node_not_read_only_or_error(): return
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if from_codebox or self.is_curr_node_not_syntax_highlighting_or_error(plain_text_ok=True):
            self.lists_handler.list_handler(0, text_buffer=text_buffer)

    def apply_tag_latest(self, *args):
        """The Iterate Tagging Button was Pressed"""
        if not self.is_curr_node_not_read_only_or_error(): return
        if not self.latest_tag[0]:
            support.dialog_warning(_("No Previous Text Format Was Performed During This Session"), self.window)
        else:
            self.apply_tag(*self.latest_tag)

    def link_cut(self, *args):
        """Cut Link"""
        if self.link_check_around_cursor():
            self.sourceview.emit("cut-clipboard")

    def link_copy(self, *args):
        """Copy Link"""
        if self.link_check_around_cursor():
            self.sourceview.emit("copy-clipboard")

    def link_dismiss(self, *args):
        """Dismiss Link"""
        if not self.is_curr_node_not_read_only_or_error(): return
        if self.link_check_around_cursor():
            self.remove_text_formatting()

    def link_delete(self, *args):
        """Delete Link"""
        if self.link_check_around_cursor():
            self.curr_buffer.delete_selection(True, self.sourceview.get_editable())
            self.sourceview.grab_focus()

    def links_entries_reset(self):
        """Reset Global Links Variables"""
        self.link_node_id = None
        self.links_entries['webs'] = ""
        self.links_entries['file'] = ""
        self.links_entries['fold'] = ""
        self.links_entries['anch'] = ""

    def links_entries_pre_dialog(self, curr_link):
        """Prepare Global Links Variables for Dialog"""
        vector = curr_link.split()
        self.link_type = vector[0]
        if self.link_type == cons.LINK_TYPE_WEBS:
            self.links_entries['webs'] = vector[1]
        elif self.link_type == cons.LINK_TYPE_FILE:
            self.links_entries['file'] = unicode(base64.b64decode(vector[1]), cons.STR_UTF8, cons.STR_IGNORE)
        elif self.link_type == cons.LINK_TYPE_FOLD:
            self.links_entries['fold'] = unicode(base64.b64decode(vector[1]), cons.STR_UTF8, cons.STR_IGNORE)
        elif self.link_type == cons.LINK_TYPE_NODE:
            self.link_node_id = long(vector[1])
            if len(vector) >= 3:
                if len(vector) == 3: anchor_name = vector[2]
                else: anchor_name = curr_link[len(vector[0]) + len(vector[1]) + 2:]
                self.links_entries['anch'] = anchor_name
        else:
            support.dialog_error("Tag Name Not Recognized! (%s)" % self.link_type, self.window)
            self.link_type = cons.LINK_TYPE_WEBS
            return False
        return True

    def links_entries_post_dialog(self):
        """Read Global Links Variables from Dialog"""
        property_value = ""
        if self.link_type == cons.LINK_TYPE_WEBS:
            link_url = self.links_entries['webs']
            if link_url:
                if len(link_url) < 8\
                or (link_url[0:7] != "http://" and link_url[0:8] != "https://"):
                    link_url = "http://" + link_url
                property_value = cons.LINK_TYPE_WEBS + cons.CHAR_SPACE + link_url
        elif self.link_type in [cons.LINK_TYPE_FILE, cons.LINK_TYPE_FOLD]:
            link_uri = self.links_entries['file'] if self.link_type == cons.LINK_TYPE_FILE else self.links_entries['fold']
            if link_uri:
                link_uri = base64.b64encode(link_uri)
                property_value = self.link_type + cons.CHAR_SPACE + link_uri
        elif self.link_type == cons.LINK_TYPE_NODE:
            tree_iter = self.links_entries['node']
            if tree_iter:
                link_anchor = self.links_entries['anch']
                property_value = cons.LINK_TYPE_NODE + cons.CHAR_SPACE + str(self.treestore[tree_iter][3])
                if link_anchor: property_value += cons.CHAR_SPACE + link_anchor
        return property_value

    def image_link_edit(self, *args):
        """Edit the Link Associated to the Image"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.links_entries_reset()
        if not self.curr_image_anchor.pixbuf.link:
            self.link_type = cons.LINK_TYPE_WEBS # default value
        elif not self.links_entries_pre_dialog(self.curr_image_anchor.pixbuf.link):
            return
        sel_tree_iter = self.get_tree_iter_from_node_id(self.link_node_id) if self.link_node_id else None
        if not support.dialog_link_handle(self, _("Insert/Edit Link"), sel_tree_iter):
            return
        property_value = self.links_entries_post_dialog()
        if property_value:
            self.curr_image_anchor.pixbuf.link = property_value
            self.image_link_apply_frame_label(self.curr_image_anchor)
            self.objects_buffer_refresh()
            self.update_window_save_needed("nbuf", True)

    def image_link_apply_frame_label(self, anchor):
        """Image Link Apply Frame Label"""
        old_widget = anchor.frame.get_label_widget()
        if old_widget: old_widget.destroy()
        if anchor.pixbuf.link:
            anchor_label = gtk.Label()
            anchor_label.set_markup("<b><small>▲</small></b>")
            anchor_label.modify_fg(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.image_frame_get_link_color(anchor.pixbuf.link)))
            anchor.frame.set_label_widget(anchor_label)
            anchor_label.show()

    def image_link_dismiss(self, *args):
        """Dismiss the Link Associated to the Image"""
        if not self.is_curr_node_not_read_only_or_error(): return
        self.curr_image_anchor.pixbuf.link = ""
        self.image_link_apply_frame_label(self.curr_image_anchor)
        self.update_window_save_needed("nbuf", True)

    def apply_tag(self, tag_property, property_value=None, iter_sel_start=None, iter_sel_end=None, text_buffer=None):
        """Apply a tag"""
        if self.user_active and not self.is_curr_node_not_syntax_highlighting_or_error(): return
        if not text_buffer: text_buffer = self.curr_buffer
        if iter_sel_start == None and iter_sel_end == None:
            if tag_property != cons.TAG_JUSTIFICATION:
                if not self.is_there_selected_node_or_error(): return
                if tag_property == cons.TAG_LINK:
                    self.links_entries_reset()
                if not text_buffer.get_has_selection():
                    if tag_property != cons.TAG_LINK:
                        if not support.apply_tag_try_automatic_bounds(self):
                            support.dialog_warning(_("No Text is Selected"), self.window)
                            return
                    else:
                        tag_property_value = self.link_check_around_cursor()
                        if tag_property_value == "":
                            if not support.apply_tag_try_automatic_bounds(self):
                                link_name = support.dialog_img_n_entry(self.window, _("Link Name"), "", "link_handle")
                                if not link_name: return
                                start_offset = text_buffer.get_iter_at_mark(text_buffer.get_insert()).get_offset()
                                text_buffer.insert_at_cursor(link_name)
                                end_offset = text_buffer.get_iter_at_mark(text_buffer.get_insert()).get_offset()
                                text_buffer.select_range(text_buffer.get_iter_at_offset(start_offset), text_buffer.get_iter_at_offset(end_offset))
                            self.link_type = cons.LINK_TYPE_WEBS # default value
                        else:
                            if not self.links_entries_pre_dialog(tag_property_value):
                                return
                iter_sel_start, iter_sel_end = text_buffer.get_selection_bounds()
            else:
                support.dialog_warning(_("The Cursor is Not into a Paragraph"), self.window)
                return
        if property_value == None:
            if tag_property == cons.TAG_LINK:
                if support.get_next_chars_from_iter_are(iter_sel_start, cons.WEB_LINK_STARTERS):
                    self.link_type = cons.LINK_TYPE_WEBS
                    self.links_entries['webs'] = text_buffer.get_text(iter_sel_start, iter_sel_end)
                insert_offset = iter_sel_start.get_offset()
                bound_offset = iter_sel_end.get_offset()
                sel_tree_iter = self.get_tree_iter_from_node_id(self.link_node_id) if self.link_node_id else None
                if not support.dialog_link_handle(self, _("Insert/Edit Link"), sel_tree_iter): return
                iter_sel_start = text_buffer.get_iter_at_offset(insert_offset)
                iter_sel_end = text_buffer.get_iter_at_offset(bound_offset)
                property_value = self.links_entries_post_dialog()
            else:
                assert tag_property[0] in ['f', 'b'], "!! bad tag_property '%s'" % tag_property
                ret_color = support.dialog_color_pick(self, self.curr_colors[tag_property[0]])
                if not ret_color: return
                self.curr_colors[tag_property[0]] = ret_color
                property_value = ret_color.to_string()
        if self.user_active and tag_property != cons.TAG_LINK:
            self.latest_tag = [tag_property, property_value]
        sel_start_offset = iter_sel_start.get_offset()
        sel_end_offset = iter_sel_end.get_offset()
        # if there's already a tag about this property, we remove it before apply the new one
        for offset in range(sel_start_offset, sel_end_offset):
            iter_sel_start = text_buffer.get_iter_at_offset(offset)
            curr_tags = iter_sel_start.get_tags()
            for curr_tag in curr_tags:
                tag_name = curr_tag.get_property("name")
                #print tag_name
                if not tag_name: continue
                iter_sel_end = text_buffer.get_iter_at_offset(offset+1)
                if (tag_property == cons.TAG_WEIGHT and tag_name.startswith("weight_"))\
                or (tag_property == cons.TAG_STYLE and tag_name.startswith("style_"))\
                or (tag_property == cons.TAG_UNDERLINE and tag_name.startswith("underline_"))\
                or (tag_property == cons.TAG_STRIKETHROUGH and tag_name.startswith("strikethrough_"))\
                or (tag_property == cons.TAG_FAMILY and tag_name.startswith("family_")):
                    text_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
                    property_value = "" # just tag removal
                elif tag_property == cons.TAG_SCALE and tag_name.startswith("scale_"):
                    text_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
                    #print property_value, tag_name[6:]
                    if property_value == tag_name[6:]:
                        property_value = "" # just tag removal
                elif tag_property == cons.TAG_JUSTIFICATION and tag_name[0:14] == "justification_":
                    text_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
                elif (tag_property == cons.TAG_FOREGROUND and tag_name[0:11] == "foreground_")\
                or (tag_property == cons.TAG_BACKGROUND and tag_name[0:11] == "background_")\
                or (tag_property == cons.TAG_LINK and tag_name[0:5] == "link_"):
                    text_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
        if property_value:
            text_buffer.apply_tag_by_name(self.apply_tag_exist_or_create(tag_property, property_value),
                                          text_buffer.get_iter_at_offset(sel_start_offset),
                                          text_buffer.get_iter_at_offset(sel_end_offset))
        if self.user_active:
            self.update_window_save_needed("nbuf", True)

    def apply_tag_exist_or_create(self, tag_property, property_value):
        """Check into the Tags Table whether the Tag Exists, if Not Creates it"""
        if property_value == "large": property_value = cons.TAG_PROP_H1
        elif property_value == "largo": property_value = cons.TAG_PROP_H2
        tag_name = tag_property + "_" + property_value
        tag = self.tag_table.lookup(str(tag_name))
        if tag == None:
            tag = gtk.TextTag(str(tag_name))
            if property_value == cons.TAG_PROP_HEAVY: tag.set_property(tag_property, pango.WEIGHT_HEAVY)
            elif property_value == cons.TAG_PROP_SMALL: tag.set_property(tag_property, pango.SCALE_SMALL)
            elif property_value == cons.TAG_PROP_H1: tag.set_property(tag_property, pango.SCALE_XX_LARGE)
            elif property_value == cons.TAG_PROP_H2: tag.set_property(tag_property, pango.SCALE_X_LARGE)
            elif property_value == cons.TAG_PROP_H3: tag.set_property(tag_property, pango.SCALE_LARGE)
            elif property_value == cons.TAG_PROP_ITALIC: tag.set_property(tag_property, pango.STYLE_ITALIC)
            elif property_value == cons.TAG_PROP_SINGLE: tag.set_property(tag_property, pango.UNDERLINE_SINGLE)
            elif property_value == cons.TAG_PROP_TRUE: tag.set_property(tag_property, True)
            elif property_value == cons.TAG_PROP_LEFT: tag.set_property(tag_property, gtk.JUSTIFY_LEFT)
            elif property_value == cons.TAG_PROP_RIGHT: tag.set_property(tag_property, gtk.JUSTIFY_RIGHT)
            elif property_value == cons.TAG_PROP_CENTER: tag.set_property(tag_property, gtk.JUSTIFY_CENTER)
            elif property_value == cons.TAG_PROP_FILL: tag.set_property(tag_property, gtk.JUSTIFY_FILL)
            elif property_value == cons.TAG_PROP_MONOSPACE:
                tag.set_property(tag_property, property_value)
                if self.monospace_bg:
                    tag.set_property(cons.TAG_BACKGROUND, self.monospace_bg)
            elif property_value == cons.TAG_PROP_SUB:
                tag.set_property(cons.TAG_SCALE, pango.SCALE_X_SMALL)
                rise = pango.FontDescription(self.text_font).get_size() / -4
                tag.set_property("rise", rise)
            elif property_value == cons.TAG_PROP_SUP:
                tag.set_property(cons.TAG_SCALE, pango.SCALE_X_SMALL)
                rise = pango.FontDescription(self.text_font).get_size() / 2
                tag.set_property("rise", rise)
            elif property_value[0:4] == cons.LINK_TYPE_WEBS:
                if self.links_underline: tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, self.col_link_webs)
            elif property_value[0:4] == cons.LINK_TYPE_NODE:
                if self.links_underline: tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, self.col_link_node)
            elif property_value[0:4] == cons.LINK_TYPE_FILE:
                if self.links_underline: tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, self.col_link_file)
            elif property_value[0:4] == cons.LINK_TYPE_FOLD:
                if self.links_underline: tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, self.col_link_fold)
            else: tag.set_property(tag_property, property_value)
            self.tag_table.add(tag)
        return str(tag_name)

    def spell_check_set_new_lang(self, new_lang):
        """Set a New Language to Spell Checker"""
        self.spellchecker._language = new_lang
        self.spellchecker._dictionary = self.spellchecker._broker.request_dict(new_lang)
        self.spellchecker.recheck()
        self.spell_check_reload_on_buffer()
        self.spell_check_lang = new_lang
        self.update_selected_node_statusbar_info()

    def spell_check_notify_new_lang(self, new_lang):
        """Receive New Lang from PyGtkSpellCheck"""
        self.spell_check_lang = new_lang
        self.update_selected_node_statusbar_info()

    def spell_check_set_on(self):
        """Enable Spell Check"""
        if not self.spell_check_init:
            self.spell_check_init = True
            self.spellchecker = pgsc_spellcheck.SpellChecker(self.sourceview, self, self.syntax_highlighting == cons.RICH_TEXT_ID, self.spell_check_lang)
            self.combobox_spell_check_lang_init()
        else:
            self.spellchecker.enable()
            if self.syntax_highlighting == cons.RICH_TEXT_ID:
                self.spell_check_reload_on_buffer()
        self.update_selected_node_statusbar_info()

    def spell_check_set_off(self, update_statusbar=False):
        """Disable Spell Check"""
        if self.spell_check_init: self.spellchecker.disable()
        if update_statusbar: self.update_selected_node_statusbar_info()

    def spell_check_reload_on_buffer(self):
        """Reload Spell Checker on curr Buffer"""
        self.spellchecker.buffer_initialize()

    def spell_check_get_languages(self):
        """Get Installed Dictionaries for Spell Check"""
        return [code for code, name in self.spellchecker.languages]

    def link_check_around_cursor_iter(self, text_iter):
        """Check if the text iter is on a link"""
        tags = text_iter.get_tags()
        for tag in tags:
            tag_name = tag.get_property("name")
            if tag_name and tag_name[0:4] == cons.TAG_LINK: return tag_name
        return ""

    def link_check_around_cursor(self):
        """Check if the cursor is on a link, in this case select the link and return the tag_property_value"""
        text_iter = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        tag_name = self.link_check_around_cursor_iter(text_iter)
        if not tag_name:
            if text_iter.get_char() == cons.CHAR_SPACE\
            and text_iter.backward_char():
                tag_name = self.link_check_around_cursor_iter(text_iter)
                if not tag_name: return ""
            else: return ""
        iter_end = text_iter.copy()
        while iter_end.forward_char():
            ret_tag_name = self.link_check_around_cursor_iter(iter_end)
            if ret_tag_name != tag_name:
                break
        while text_iter.backward_char():
            ret_tag_name = self.link_check_around_cursor_iter(text_iter)
            if ret_tag_name != tag_name:
                text_iter.forward_char()
                break
        if text_iter.equal(iter_end): return ""
        self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_end)
        self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), text_iter)
        return tag_name[5:]

    def external_folderpath_open(self, filepath):
        """Open Folderpath with External App"""
        if self.folderlink_custom_action[0]:
            if cons.IS_WIN_OS: filepath = cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE
            else: filepath = re.escape(filepath)
            subprocess.call(self.folderlink_custom_action[1] % filepath, shell=True)
        else:
            if cons.IS_WIN_OS: os.startfile(filepath)
            else: subprocess.call(config.LINK_CUSTOM_ACTION_DEFAULT_FILE % re.escape(filepath), shell=True)

    def external_filepath_open(self, filepath, open_fold_if_no_app_error):
        """Open Filepath with External App"""
        if self.filelink_custom_action[0]:
            if cons.IS_WIN_OS: filepath = cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE
            else: filepath = re.escape(filepath)
            subprocess.call(self.filelink_custom_action[1] % filepath, shell=True)
        else:
            if cons.IS_WIN_OS:
                try: os.startfile(filepath)
                except:
                    if open_fold_if_no_app_error: os.startfile(os.path.dirname(filepath))
            else: subprocess.call(config.LINK_CUSTOM_ACTION_DEFAULT_FILE % re.escape(filepath), shell=True)

    def link_process_filepath(self, filepath_raw):
        filepath_orig = unicode(base64.b64decode(filepath_raw), cons.STR_UTF8, cons.STR_IGNORE)
        filepath = support.get_proper_platform_filepath(filepath_orig, True)
        if not os.path.isabs(filepath) and os.path.isfile(os.path.join(self.file_dir, filepath)):
            filepath = os.path.join(self.file_dir, filepath)
        return filepath

    def link_process_folderpath(self, folderpath_raw):
        folderpath_orig = unicode(base64.b64decode(folderpath_raw), cons.STR_UTF8, cons.STR_IGNORE)
        folderpath = support.get_proper_platform_filepath(folderpath_orig, False)
        if not os.path.isabs(folderpath) and os.path.isdir(os.path.join(self.file_dir, folderpath)):
            folderpath = os.path.join(self.file_dir, folderpath)
        return folderpath

    def link_clicked(self, tag_property_value, from_wheel):
        """Function Called at Every Link Click"""
        vector = tag_property_value.split()
        if vector[0] == cons.LINK_TYPE_WEBS:
            # link to webpage
            clean_weblink = vector[1].replace("amp;", "")
            if self.weblink_custom_action[0]:
                subprocess.call(self.weblink_custom_action[1] % clean_weblink, shell=True)
            else: webbrowser.open(clean_weblink)
        elif vector[0] == cons.LINK_TYPE_FILE:
            # link to file
            filepath = self.link_process_filepath(vector[1])
            if not os.path.isfile(filepath):
                support.dialog_error(_("The File Link '%s' is Not Valid") % filepath, self.window)
                return
            if from_wheel:
                filepath = os.path.dirname(os.path.abspath(filepath))
            self.external_filepath_open(filepath, True)
        elif vector[0] == cons.LINK_TYPE_FOLD:
            # link to folder
            folderpath = self.link_process_folderpath(vector[1])
            if not os.path.isdir(folderpath):
                support.dialog_error(_("The Folder Link '%s' is Not Valid") % folderpath, self.window)
                return
            if from_wheel:
                folderpath = os.path.dirname(os.path.abspath(folderpath))
            self.external_folderpath_open(folderpath)
        elif vector[0] == cons.LINK_TYPE_NODE:
            # link to a tree node
            tree_iter = self.get_tree_iter_from_node_id(long(vector[1]))
            if tree_iter == None:
                support.dialog_error(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)") % vector[1], self.window)
                return
            self.treeview_safe_set_cursor(tree_iter)
            self.sourceview.grab_focus()
            self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(gtk.gdk.Cursor(gtk.gdk.XTERM))
            self.sourceview.set_tooltip_text(None)
            if len(vector) >= 3:
                if len(vector) == 3: anchor_name = vector[2]
                else: anchor_name = tag_property_value[len(vector[0]) + len(vector[1]) + 2:]
                iter_anchor = self.link_seek_for_anchor(anchor_name)
                if iter_anchor == None:
                    if len(anchor_name) > cons.MAX_TOOLTIP_LINK_CHARS:
                        anchor_name = anchor_name[:cons.MAX_TOOLTIP_LINK_CHARS] + "..."
                    support.dialog_warning(_("No anchor named '%s' found") % anchor_name, self.window)
                else:
                    self.curr_buffer.place_cursor(iter_anchor)
                    self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
        else: support.dialog_error("Tag Name Not Recognized! (%s)" % vector[0], self.window)

    def link_seek_for_anchor(self, anchor_name):
        """Given an Anchor Name, Seeks for it in the Current Node and Returns the Iter or None"""
        curr_iter = self.curr_buffer.get_start_iter()
        while 1:
            anchor = curr_iter.get_child_anchor()
            if anchor != None:
                if "pixbuf" in dir(anchor) and "anchor" in dir(anchor.pixbuf) and anchor.pixbuf.anchor == anchor_name:
                    return curr_iter
            if not curr_iter.forward_char(): break
        return None

    def sourceview_hovering_link_get_tooltip(self, link):
        """Get the tooltip for the underlying link"""
        tooltip = ""
        vector = link.split()
        if vector[0] in [cons.LINK_TYPE_FILE, cons.LINK_TYPE_FOLD]:
            tooltip = unicode(base64.b64decode(vector[1]), cons.STR_UTF8, cons.STR_IGNORE)
        else:
            if vector[0] == cons.LINK_TYPE_NODE and long(vector[1]) in self.nodes_names_dict:
                tooltip = self.nodes_names_dict[long(vector[1])]
            else: tooltip = vector[1].replace("amp;", "")
            if len(vector) >= 3:
                if len(vector) == 3: anchor_name = vector[2]
                else: anchor_name = link[5 + len(vector[0]) + len(vector[1]) + 2:]
                tooltip += "#" + anchor_name
        return tooltip

    def on_sourceview_event(self, text_view, event):
        """Called at every event on the SourceView"""
        if event.type == gtk.gdk.KEY_PRESS:
            keyname = gtk.gdk.keyval_name(event.keyval)
            if (event.state & gtk.gdk.SHIFT_MASK):
                if keyname == cons.STR_KEY_SHIFT_TAB:
                    if not self.curr_buffer.get_has_selection():
                        iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
                        list_info = self.lists_handler.get_paragraph_list_info(iter_insert)
                        if list_info and list_info["level"]:
                            support.on_sourceview_list_change_level(self, iter_insert, list_info, self.curr_buffer, False)
                            return True
            elif (event.state & gtk.gdk.CONTROL_MASK) and keyname == "space":
                iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
                anchor = iter_insert.get_child_anchor()
                if anchor and hasattr(anchor, "sourcebuffer"):
                    anchor.sourceview.grab_focus()
                    return True
            elif keyname == cons.STR_KEY_RETURN:
                iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
                if iter_insert: self.cursor_key_press = iter_insert.get_offset()
                else: self.cursor_key_press = None
                #print "self.cursor_key_press", self.cursor_key_press
            elif keyname == cons.STR_KEY_MENU:
                if self.syntax_highlighting == cons.RICH_TEXT_ID:
                    if not self.curr_buffer.get_has_selection(): return False
                    iter_sel_start, iter_sel_end = self.curr_buffer.get_selection_bounds()
                    num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset()
                    if num_chars != 1: return False
                    anchor = iter_sel_start.get_child_anchor()
                    if not anchor: return False
                    if not "pixbuf" in dir(anchor): return False
                    if "anchor" in dir(anchor.pixbuf):
                        self.curr_anchor_anchor = anchor
                        self.object_set_selection(self.curr_anchor_anchor)
                        self.ui.get_widget("/AnchorMenu").popup(None, None, None, 3, event.time)
                    else:
                        self.curr_image_anchor = anchor
                        self.object_set_selection(self.curr_image_anchor)
                        if self.curr_image_anchor.pixbuf.link: self.ui.get_widget("/ImageMenu/img_link_dismiss").show()
                        else: self.ui.get_widget("/ImageMenu/img_link_dismiss").hide()
                        self.ui.get_widget("/ImageMenu").popup(None, None, None, 3, event.time)
                    return True
            elif keyname == cons.STR_KEY_TAB:
                if not self.curr_buffer.get_has_selection():
                    iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
                    list_info = self.lists_handler.get_paragraph_list_info(iter_insert)
                    if list_info:
                        support.on_sourceview_list_change_level(self, iter_insert, list_info, self.curr_buffer, True)
                        return True
                elif self.syntax_highlighting == cons.RICH_TEXT_ID:
                    iter_sel_start, iter_sel_end = self.curr_buffer.get_selection_bounds()
                    num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset()
                    if num_chars != 1: return False
                    anchor = iter_sel_start.get_child_anchor()
                    if not anchor: return False
                    if not "liststore" in dir(anchor): return False
                    self.curr_buffer.place_cursor(iter_sel_end)
                    self.sourceview.grab_focus()
                    return True
        return False

    def on_sourceview_event_after(self, text_view, event):
        """Called after every event on the SourceView"""
        if event.type == gtk.gdk._2BUTTON_PRESS and event.button == 1:
            support.on_sourceview_event_after_double_click_button1(self, text_view, event)
        elif event.type in [gtk.gdk.BUTTON_PRESS, gtk.gdk.KEY_PRESS]:
            if self.syntax_highlighting == cons.RICH_TEXT_ID\
            and self.curr_tree_iter and not self.curr_buffer.get_modified():
                self.state_machine.update_curr_state_cursor_pos(self.treestore[self.curr_tree_iter][3])
            if event.type == gtk.gdk.BUTTON_PRESS:
                return support.on_sourceview_event_after_button_press(self, text_view, event)
            if event.type == gtk.gdk.KEY_PRESS:
                return support.on_sourceview_event_after_key_press(self, text_view, event, self.syntax_highlighting)
        elif event.type == gtk.gdk.KEY_RELEASE:
            return support.on_sourceview_event_after_key_release(self, text_view, event)
        elif event.type == gtk.gdk.SCROLL:
            return support.on_sourceview_event_after_scroll(self, text_view, event)
        return False

    def special_char_replace(self, special_char, iter_start, iter_insert, text_buffer):
        """A special char replacement is triggered"""
        text_buffer.delete(iter_start, iter_insert)
        iter_insert = text_buffer.get_iter_at_mark(text_buffer.get_insert())
        text_buffer.insert(iter_insert, special_char + cons.CHAR_SPACE)

    def replace_text_at_offset(self, text_to, offset_from, offset_to, text_buffer):
        text_buffer.delete(text_buffer.get_iter_at_offset(offset_from),
            text_buffer.get_iter_at_offset(offset_to))
        text_buffer.insert(text_buffer.get_iter_at_offset(offset_from), text_to)

    def on_sourceview_motion_notify_event(self, text_view, event):
        """Update the cursor image if the pointer moved"""
        if not self.sourceview.get_cursor_visible():
            self.sourceview.set_cursor_visible(True)
        if self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(gtk.gdk.Cursor(gtk.gdk.XTERM))
            return False
        x, y = self.sourceview.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, int(event.x), int(event.y))
        support.sourceview_cursor_and_tooltips_handler(self, text_view, x, y)
        return False

    def update_selected_node_statusbar_info(self):
        """Update the statusbar with node info"""
        if not self.curr_tree_iter:
            statusbar_text = _("No Node is Selected")
        else:
            separator_text = "  -  "
            statusbar_text = _("Node Type") + _(": ")
            if self.syntax_highlighting == cons.RICH_TEXT_ID: statusbar_text += _("Rich Text")
            elif self.syntax_highlighting == cons.PLAIN_TEXT_ID: statusbar_text += _("Plain Text")
            else: statusbar_text += self.syntax_highlighting
            if self.treestore[self.curr_tree_iter][6]: statusbar_text += separator_text + _("Tags") + _(": ") + self.treestore[self.curr_tree_iter][6]
            if self.enable_spell_check and self.syntax_highlighting == cons.RICH_TEXT_ID:
                statusbar_text += separator_text + _("Spell Check") + _(": ") + self.spell_check_lang
            if self.word_count:
                statusbar_text += separator_text + _("Word Count") + _(": ") + str(support.get_word_count(self))
            ts_creation = self.treestore[self.curr_tree_iter][12]
            if ts_creation:
                timestamp_creation = support.get_timestamp_str(self.timestamp_format, ts_creation)
                statusbar_text += separator_text + _("Date Created") + _(": ") + timestamp_creation
            ts_lastsave = self.treestore[self.curr_tree_iter][13]
            if ts_lastsave:
                timestamp_lastsave = support.get_timestamp_str(self.timestamp_format, ts_lastsave)
                statusbar_text += separator_text + _("Date Modified") + _(": ") + timestamp_lastsave
            print "sel node id=%s, seq=%s" % (self.treestore[self.curr_tree_iter][3], self.treestore[self.curr_tree_iter][5])
        self.statusbar.pop(self.statusbar_context_id)
        self.statusbar.push(self.statusbar_context_id, statusbar_text)

    def on_image_visibility_notify_event(self, widget, event):
        """Problem of image colored frame disappearing"""
        widget.queue_draw()

    def on_sourceview_visibility_notify_event(self, text_view, event):
        """Update the cursor image if the window becomes visible (e.g. when a window covering it got iconified)"""
        if self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(gtk.gdk.Cursor(gtk.gdk.XTERM))
            return False
        wx, wy, mod = text_view.window.get_pointer()
        bx, by = text_view.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, wx, wy)
        support.sourceview_cursor_and_tooltips_handler(self, text_view, bx, by)
        return False

    def on_window_n_tree_size_allocate_event(self, widget, allocation):
        """New Size Allocated"""
        if not self.curr_window_n_tree_width:
            self.curr_window_n_tree_width = {'window_width': self.window.get_allocation().width,
                                             'tree_width': self.scrolledwindow_tree.get_allocation().width}
        else:
            if not self.curr_buffer: return
            if self.curr_window_n_tree_width['window_width'] != self.window.get_allocation().width\
            or self.curr_window_n_tree_width['tree_width'] != self.scrolledwindow_tree.get_allocation().width:
                self.curr_window_n_tree_width['window_width'] = self.window.get_allocation().width
                self.curr_window_n_tree_width['tree_width'] = self.scrolledwindow_tree.get_allocation().width
                curr_iter = self.curr_buffer.get_start_iter()
                while 1:
                    anchor = curr_iter.get_child_anchor()
                    if anchor != None and "width_in_pixels" in dir(anchor) and not anchor.width_in_pixels:
                        self.codeboxes_handler.codebox_apply_width_height(anchor)
                    if not curr_iter.forward_char(): break

    def get_text_window_width(self):
        """Get the Text Window Width"""
        return (self.window.get_allocation().width -\
                self.scrolledwindow_tree.get_allocation().width-\
                cons.MAIN_WIN_TO_TEXT_WIN_NORMALIZER)

    def remove_text_formatting(self, *args):
        """Cleans the Selected Text from All Formatting Tags"""
        if not self.node_sel_and_rich_text(): return
        if not self.curr_buffer.get_has_selection() and not support.apply_tag_try_automatic_bounds(self):
            support.dialog_warning(_("No Text is Selected"), self.window)
            return
        iter_sel_start, iter_sel_end = self.curr_buffer.get_selection_bounds()
        self.curr_buffer.remove_all_tags(iter_sel_start, iter_sel_end)
        if self.enable_spell_check: self.spell_check_set_on()
        self.update_window_save_needed("nbuf", True)

    def update_node_aux_icon(self, tree_iter):
        """Set Aux Icon to node"""
        node_id = self.get_node_id_from_tree_iter(tree_iter)
        is_bookmarked = str(node_id) in self.bookmarks
        is_ro = self.get_node_read_only(tree_iter)
        if is_bookmarked and is_ro:
            stock_id = "lockpin"
        elif is_bookmarked:
            stock_id = "pin"
        elif is_ro:
            stock_id = "locked"
        else:
            stock_id = None
        self.treestore[tree_iter][8] = stock_id

    def bookmark_curr_node_remove(self, *args):
        """Remove the Current Node from the Bookmarks List"""
        if not self.is_there_selected_node_or_error(): return
        curr_node_id_str = str(self.get_node_id_from_tree_iter(self.curr_tree_iter))
        if curr_node_id_str in self.bookmarks:
            self.bookmarks.remove(curr_node_id_str)
            support.set_bookmarks_menu_items(self)
            self.update_node_aux_icon(self.curr_tree_iter)
            self.update_window_save_needed("book")
            self.menu_tree_update_for_bookmarked_node(False)

    def bookmark_curr_node(self, *args):
        """Add the Current Node to the Bookmarks List"""
        if not self.is_there_selected_node_or_error(): return
        curr_node_id_str = str(self.get_node_id_from_tree_iter(self.curr_tree_iter))
        if not curr_node_id_str in self.bookmarks:
            self.bookmarks.append(curr_node_id_str)
            support.set_bookmarks_menu_items(self)
            self.update_node_aux_icon(self.curr_tree_iter)
            self.update_window_save_needed("book")
            self.menu_tree_update_for_bookmarked_node(True)

    def bookmarks_handle(self, *args):
        """Handle the Bookmarks List"""
        if support.bookmarks_handle(self):
            self.update_window_save_needed("book")

    def timestamp_insert(self, *args):
        """Insert Timestamp"""
        text_view, text_buffer, from_codebox = self.get_text_view_n_buffer_codebox_proof()
        if not text_buffer: return
        if not self.is_curr_node_not_read_only_or_error(): return
        timestamp = support.get_timestamp_str(self.timestamp_format, time.time())
        text_buffer.insert_at_cursor(timestamp)

    def set_selection_at_offset_n_delta(self, offset, delta, text_buffer=None):
        """Set the Selection from given offset to offset+delta"""
        if not text_buffer: text_buffer = self.curr_buffer
        target = text_buffer.get_iter_at_offset(offset)
        if target:
            text_buffer.place_cursor(target)
            if not target.forward_chars(delta):
                #print "? bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
                pass
            text_buffer.move_mark(text_buffer.get_selection_bound(), target)
            return
        print "! bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])

    def tree_is_empty(self):
        """Return True if the treestore is empty"""
        return (self.treestore.get_iter_first() == None)
