# -*- coding: UTF-8 -*-
#
#       core.py
#
#       Copyright 2009-2013 Giuseppe Penone <giuspen@gmail.com>
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

import gtk, pango, gtksourceview2, gobject
import sys, os, re, glob, subprocess, webbrowser, base64, cgi, urllib2, shutil, time, locale, pgsc_spellcheck
try:
    import appindicator
    HAS_APPINDICATOR = True
except: HAS_APPINDICATOR = False
XDG_CURRENT_DESKTOP = 'XDG_CURRENT_DESKTOP'
HAS_SYSTRAY = not (XDG_CURRENT_DESKTOP in os.environ and os.environ[XDG_CURRENT_DESKTOP] == "Unity")
import cons, support, config, machines, clipboard, imports, exports, printing, tablez, lists, findreplace, codeboxes, ctdb


class GladeWidgetsWrapper:
    """Handles the retrieval of glade widgets"""

    def __init__(self, glade_file_path, gui_instance):
        try:
            self.glade_widgets = gtk.Builder()
            self.glade_widgets.set_translation_domain(cons.APP_NAME)
            self.glade_widgets.add_from_file(glade_file_path)
            self.glade_widgets.connect_signals(gui_instance)
        except: print "Failed to load the glade file"

    def __getitem__(self, key):
        """Gives us the ability to do: wrapper['widget_name'].action()"""
        return self.glade_widgets.get_object(key)

    def __getattr__(self, attr):
        """Gives us the ability to do: wrapper.widget_name.action()"""
        new_widget = self.glade_widgets.get_object(attr)
        if new_widget is None: raise AttributeError, 'Widget %r not found' % attr
        setattr(self, attr, new_widget)
        return new_widget


class CherryTree:
    """Application's GUI"""

    def __init__(self, lang_str, open_with_file, node_name, boss):
        """GUI Startup"""
        self.boss = boss
        self.filetype = ""
        self.user_active = True
        # glade
        self.glade = GladeWidgetsWrapper(cons.GLADE_PATH + 'cherrytree.glade', self) # glade widgets access
        self.window = self.glade.window
        # instantiate external handlers
        self.clipboard_handler = clipboard.ClipboardHandler(self)
        self.lists_handler = lists.ListsHandler(self)
        self.tables_handler = tablez.TablesHandler(self)
        self.codeboxes_handler = codeboxes.CodeBoxesHandler(self)
        self.state_machine = machines.StateMachine(self)
        self.xml_handler = machines.XMLHandler(self)
        self.html_handler = exports.Export2Html(self)
        self.find_handler = findreplace.FindReplace(self)
        self.ctdb_handler = ctdb.CTDBHandler(self)
        self.print_handler = printing.PrintHandler()
        # icon factory
        factory = gtk.IconFactory()
        for filename in cons.STOCKS_N_FILES:
            stock_name = filename[:-4]
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
        config.config_file_load(self)
        if not HAS_APPINDICATOR: self.use_appind = False
        elif not HAS_SYSTRAY: self.use_appind = True
        if not HAS_APPINDICATOR or not HAS_SYSTRAY: self.glade.checkbutton_use_appind.set_sensitive(False)
        # ui manager
        actions = gtk.ActionGroup("Actions")
        actions.add_actions(cons.get_entries(self))
        self.ui = gtk.UIManager()
        self.ui.insert_action_group(actions, 0)
        self.window.add_accel_group(self.ui.get_accel_group())
        self.ui.add_ui_from_string(cons.UI_INFO)
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
        self.vbox_text = gtk.VBox()
        self.header_node_name_label = gtk.Label()
        self.header_node_name_label.set_padding(10, 0)
        self.header_node_name_label.set_ellipsize(pango.ELLIPSIZE_MIDDLE)
        self.header_node_name_eventbox = gtk.EventBox()
        self.header_node_name_eventbox.add(self.header_node_name_label)
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
        vbox_main.pack_start(self.statusbar, False, False)
        # ROW: 0-icon_stock_id, 1-name, 2-buffer, 3-unique_id, 4-syntax_highlighting, 5-node_sequence, 6-tags, 7-readonly
        self.treestore = gtk.TreeStore(str, str, gobject.TYPE_PYOBJECT, long, str, int, str, gobject.TYPE_BOOLEAN)
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
        self.column = gtk.TreeViewColumn()
        self.column.pack_start(self.renderer_pixbuf, False)
        self.column.pack_start(self.renderer_text, True)
        self.column.set_attributes(self.renderer_pixbuf, stock_id=0)
        self.column.set_attributes(self.renderer_text, text=1)
        self.treeview.append_column(self.column)
        self.treeviewselection = self.treeview.get_selection()
        self.treeview.connect('cursor-changed', self.on_node_changed)
        self.treeview.connect('button-press-event', self.on_mouse_button_clicked_tree)
        self.treeview.connect('key_press_event', self.on_key_press_cherrytree)
        self.treeview.connect('drag-motion', self.on_drag_motion_cherrytree)
        self.treeview.connect('drag-data-received', self.on_drag_data_recv_cherrytree)
        self.treeview.connect('drag-data-get', self.on_drag_data_get_cherrytree)
        self.scrolledwindow_tree.add(self.treeview)
        self.orphan_accel_group = gtk.AccelGroup()
        self.menu_tree_create()
        self.window.connect('window-state-event', self.on_window_state_event)
        self.window.connect("size-allocate", self.on_window_n_tree_size_allocate_event)
        self.window.connect('key_press_event', self.on_key_press_window)
        self.window.connect("destroy", self.boss.on_window_destroy_event)
        self.scrolledwindow_tree.connect("size-allocate", self.on_window_n_tree_size_allocate_event)
        self.glade.anchorhandledialog.connect('key_press_event', self.on_key_press_anchorhandledialog)
        self.glade.choosenodedialog.connect('key_press_event', self.on_key_press_choosenodedialog)
        self.glade.tablehandledialog.connect('key_press_event', self.tables_handler.on_key_press_tablehandledialog)
        self.glade.codeboxhandledialog.connect('key_press_event', self.codeboxes_handler.on_key_press_codeboxhandledialog)
        self.glade.exitdialog.connect('key_press_event', self.on_key_press_exitdialog)
        self.sourcestyleschememanager = gtksourceview2.StyleSchemeManager()
        self.sourceview = gtksourceview2.View()
        self.sourceview.set_sensitive(False)
        self.sourceview.set_smart_home_end(gtksourceview2.SMART_HOME_END_BEFORE)
        self.sourceview.connect('populate-popup', self.on_sourceview_populate_popup)
        self.sourceview.connect("motion-notify-event", self.on_sourceview_motion_notify_event)
        self.sourceview.connect("event-after", self.on_sourceview_event_after)
        self.sourceview.connect("visibility-notify-event", self.on_sourceview_visibility_notify_event)
        self.sourceview.connect("copy-clipboard", self.clipboard_handler.copy)
        self.sourceview.connect("cut-clipboard", self.clipboard_handler.cut)
        self.sourceview.connect("paste-clipboard", self.clipboard_handler.paste)
        self.sourceview.set_left_margin(7)
        self.sourceview.set_right_margin(7)
        self.hovering_over_link = False
        self.tag_table = gtk.TextTagTable()
        self.scrolledwindow_text.add(self.sourceview)
        self.go_bk_fw_click = False
        self.highlighted_obj = None
        self.bookmarks = []
        self.bookmarks_menu_items = []
        self.nodes_names_dict = {}
        self.password = None
        self.export_single = False
        self.curr_tree_iter = None
        self.curr_window_n_tree_width = None
        self.curr_buffer = None
        self.nodes_cursor_pos = {}
        self.search_replace_dict = {'find':"", 'replace':"", 'match_case':False, 'reg_exp':False, 'whole_word':False, 'start_word':False, 'fw':True, 'a_ff_fa':0, 'idialog':True}
        self.latest_tag = ["", ""] # [latest tag property, latest tag value]
        self.file_update = False
        self.autosave_timer_id = None
        self.mod_time_sentinel_id = None
        self.mod_time_val = 0
        support.set_menu_items_recent_documents(self)
        support.set_menu_items_special_chars(self)
        self.window.show_all() # this before the config_file_apply that could hide something
        self.window.present()
        config.config_file_apply(self)
        self.combobox_country_lang_init()
        self.combobox_style_scheme_init()
        self.combobox_prog_lang_init()
        if self.systray:
            if not self.boss.systray_active:
                self.status_icon_enable()
            if self.start_on_systray: self.window.hide()
        else: self.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property(cons.STR_VISIBLE, False)
        if self.reload_doc_last: self.file_startup_load(open_with_file, node_name)
        if self.check_version: self.check_for_newer_version()

    def check_for_newer_version(self, *args):
        """Check for a Newer Version"""
        self.statusbar.pop(self.statusbar_context_id)
        self.statusbar.push(self.statusbar_context_id, _("Checking for Newer Version..."))
        while gtk.events_pending(): gtk.main_iteration()
        try:
            fd = urllib2.urlopen(cons.NEWER_VERSION_URL, timeout=3)
            latest_version = fd.read().replace(cons.CHAR_NEWLINE, "")
            if latest_version != cons.VERSION:
                support.dialog_info(_("A Newer Version Is Available!") + " (%s)" % latest_version, self.window)
                self.statusbar.pop(self.statusbar_context_id)
                self.update_selected_node_statusbar_info()
            else:
                self.statusbar.pop(self.statusbar_context_id)
                self.statusbar.push(self.statusbar_context_id, _("This is the Latest Version Available") + " (%s)" % latest_version)
        except:
            self.statusbar.pop(self.statusbar_context_id)
            self.statusbar.push(self.statusbar_context_id, _("Failed to Retrieve Latest Version Information - Try Again Later"))

    def get_node_icon(self, node_level, node_code):
        """Returns the Stock Id given the Node Level"""
        if self.nodes_icons == "c":
            if node_code == cons.CUSTOM_COLORS_ID:
                if node_level in cons.NODES_ICONS: return cons.NODES_ICONS[node_level]
                else: return cons.NODES_ICONS[6]
            else:
                if node_code in cons.CODE_ICONS: return cons.CODE_ICONS[node_code]
                else: return "cherry_gray"
        elif self.nodes_icons == "b": return "node_bullet"
        else: return "node_no_icon"

    def text_selection_change_case(self, change_type):
        """Change the Case of the Selected Text/the Underlying Word"""
        if not self.curr_buffer.get_has_selection() and not self.apply_tag_try_automatic_bounds():
            support.dialog_warning(_("No Text is Selected"), self.window)
            return
        iter_start, iter_end = self.curr_buffer.get_selection_bounds()
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            text_to_change_case = self.curr_buffer.get_text(iter_start, iter_end)
            if change_type == "l": text_to_change_case = text_to_change_case.lower()
            elif change_type == "u": text_to_change_case = text_to_change_case.upper()
            elif change_type == "t": text_to_change_case = text_to_change_case.swapcase()
        else:
            rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(self.curr_buffer,
                                                                                        iter_start,
                                                                                        iter_end,
                                                                                        change_case=change_type)
        start_offset = iter_start.get_offset()
        end_offset = iter_end.get_offset()
        self.curr_buffer.delete(iter_start, iter_end)
        iter_insert = self.curr_buffer.get_iter_at_offset(start_offset)
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            self.curr_buffer.insert(iter_insert, text_to_change_case)
        else:
            self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_insert)
            self.clipboard_handler.from_xml_string_to_buffer(rich_text)
        self.curr_buffer.select_range(self.curr_buffer.get_iter_at_offset(start_offset),
                                      self.curr_buffer.get_iter_at_offset(end_offset))

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
        if self.curr_buffer.get_has_selection():
            iter_start, iter_end = self.curr_buffer.get_selection_bounds() # there's a selection
            sel_start_offset = iter_start.get_offset()
            sel_end_offset = iter_end.get_offset()
            if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
                text_to_duplicate = self.curr_buffer.get_text(iter_start, iter_end)
                if cons.CHAR_NEWLINE in text_to_duplicate:
                    text_to_duplicate = cons.CHAR_NEWLINE + text_to_duplicate
                self.curr_buffer.insert(iter_end, text_to_duplicate)
            else:
                rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(self.curr_buffer,
                                                                                            iter_start,
                                                                                            iter_end)
                if cons.CHAR_NEWLINE in rich_text:
                    self.curr_buffer.insert(iter_end, cons.CHAR_NEWLINE)
                    iter_end = self.curr_buffer.get_iter_at_offset(sel_end_offset+1)
                    self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_end)
                self.clipboard_handler.from_xml_string_to_buffer(rich_text)
            self.curr_buffer.select_range(self.curr_buffer.get_iter_at_offset(sel_start_offset),
                                          self.curr_buffer.get_iter_at_offset(sel_end_offset))
        else:
            iter_start, iter_end = self.lists_handler.get_paragraph_iters()
            if iter_start == None:
                iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
                self.curr_buffer.insert(iter_start, cons.CHAR_NEWLINE)
            else:
                if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
                    text_to_duplicate = self.curr_buffer.get_text(iter_start, iter_end)
                    self.curr_buffer.insert(iter_end, cons.CHAR_NEWLINE + text_to_duplicate)
                else:
                    rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(self.curr_buffer,
                                                                                                iter_start,
                                                                                                iter_end)
                    sel_end_offset = iter_end.get_offset()
                    self.curr_buffer.insert(iter_end, cons.CHAR_NEWLINE)
                    iter_end = self.curr_buffer.get_iter_at_offset(sel_end_offset+1)
                    self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_end)
                    self.clipboard_handler.from_xml_string_to_buffer(rich_text)
        self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])

    def text_row_up(self, *args):
        """Moves Up the Current Row/Selected Rows"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if iter_start == None:
            iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
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
        #print "***"
        #print "iter_start", iter_start.get_offset(), ord(iter_start.get_char()), iter_start.get_char()
        #print "iter_end", iter_end.get_offset(), ord(iter_end.get_char()), iter_end.get_char()
        #print "destination_iter", destination_iter.get_offset(), ord(destination_iter.get_char()), destination_iter.get_char()
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            text_to_move = self.curr_buffer.get_text(iter_start, iter_end)
            self.curr_buffer.delete(iter_start, iter_end)
            destination_iter = self.curr_buffer.get_iter_at_offset(destination_offset)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE: text_to_move += cons.CHAR_NEWLINE
            self.curr_buffer.insert(destination_iter, text_to_move)
            self.set_selection_at_offset_n_delta(destination_offset, len(text_to_move)-1)
        else:
            text_to_move = self.curr_buffer.get_text(iter_start, iter_end)
            diff_offsets = iter_end.get_offset() - iter_start.get_offset()
            rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(self.curr_buffer,
                                                                                        iter_start,
                                                                                        iter_end,
                                                                                        exclude_iter_sel_end=True)
            self.curr_buffer.delete(iter_start, iter_end)
            destination_iter = self.curr_buffer.get_iter_at_offset(destination_offset)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE:
                diff_offsets += 1
                append_newline = True
            else: append_newline = False
            self.curr_buffer.move_mark(self.curr_buffer.get_insert(), destination_iter)
            self.clipboard_handler.from_xml_string_to_buffer(rich_text)
            if append_newline: self.curr_buffer.insert_at_cursor(cons.CHAR_NEWLINE)
            self.set_selection_at_offset_n_delta(destination_offset, diff_offsets-1)
        self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])

    def text_row_down(self, *args):
        """Moves Down the Current Row/Selected Rows"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if iter_start == None:
            iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
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
        #print "***"
        #print "iter_start", iter_start.get_offset(), ord(iter_start.get_char()), iter_start.get_char()
        #print "iter_end", iter_end.get_offset(), ord(iter_end.get_char()), iter_end.get_char()
        #print "destination_iter", destination_iter.get_offset(), ord(destination_iter.get_char()), destination_iter.get_char()
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            text_to_move = self.curr_buffer.get_text(iter_start, iter_end)
            self.curr_buffer.delete(iter_start, iter_end)
            destination_offset -= len(text_to_move)
            destination_iter = self.curr_buffer.get_iter_at_offset(destination_offset)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE: text_to_move += cons.CHAR_NEWLINE
            if missing_leading_newline: text_to_move = cons.CHAR_NEWLINE + text_to_move
            self.curr_buffer.insert(destination_iter, text_to_move)
            if not missing_leading_newline:
                self.set_selection_at_offset_n_delta(destination_offset, len(text_to_move)-1)
            else:
                self.set_selection_at_offset_n_delta(destination_offset+1, len(text_to_move)-2)
        else:
            text_to_move = self.curr_buffer.get_text(iter_start, iter_end)
            diff_offsets = iter_end.get_offset() - iter_start.get_offset()
            rich_text = self.clipboard_handler.rich_text_get_from_text_buffer_selection(self.curr_buffer,
                                                                                        iter_start,
                                                                                        iter_end,
                                                                                        exclude_iter_sel_end=True)
            self.curr_buffer.delete(iter_start, iter_end)
            destination_offset -= diff_offsets
            destination_iter = self.curr_buffer.get_iter_at_offset(destination_offset)
            if not text_to_move or text_to_move[-1] != cons.CHAR_NEWLINE:
                diff_offsets += 1
                append_newline = True
            else: append_newline = False
            self.curr_buffer.move_mark(self.curr_buffer.get_insert(), destination_iter)
            if missing_leading_newline:
                diff_offsets += 1
                self.curr_buffer.insert_at_cursor(cons.CHAR_NEWLINE)
            self.clipboard_handler.from_xml_string_to_buffer(rich_text)
            if append_newline: self.curr_buffer.insert_at_cursor(cons.CHAR_NEWLINE)
            if not missing_leading_newline:
                self.set_selection_at_offset_n_delta(destination_offset, diff_offsets-1)
            else:
                self.set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2)
        self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])

    def text_row_delete(self, *args):
        """Deletes the Whole Row"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if iter_start == None:
            iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
            iter_end = iter_start.copy()
        if not iter_end.forward_char() and not iter_start.backward_char(): return
        self.curr_buffer.delete(iter_start, iter_end)
        self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])

    def text_row_cut(self, *args):
        """Cut a Whole Row"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if iter_start == None:
            iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
            iter_end = iter_start.copy()
        if not iter_end.forward_char() and not iter_start.backward_char(): return
        self.curr_buffer.select_range(iter_start, iter_end)
        self.sourceview.emit("cut-clipboard")

    def text_row_copy(self, *args):
        """Copy a Whole Row"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        if iter_start == None:
            iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
            iter_end = iter_start.copy()
        if not iter_end.forward_char() and not iter_start.backward_char(): return
        self.curr_buffer.select_range(iter_start, iter_end)
        self.sourceview.emit("copy-clipboard")

    def on_key_press_window(self, widget, event):
        """Catches Window key presses"""
        if not self.curr_tree_iter: return
        keyname = gtk.gdk.keyval_name(event.keyval)
        if event.state & gtk.gdk.MOD1_MASK:
            if keyname == "Left": self.go_back()
            elif keyname == "Right": self.go_forward()

    def on_key_press_cherrytree(self, widget, event):
        """Catches Tree key presses"""
        if not self.curr_tree_iter: return
        keyname = gtk.gdk.keyval_name(event.keyval)
        if event.state & gtk.gdk.SHIFT_MASK:
            if keyname == "Up": self.node_up()
            elif keyname == "Down": self.node_down()
            elif keyname == "Left": self.node_left()
            elif keyname == "Right": self.node_change_father()
        elif event.state & gtk.gdk.MOD1_MASK:
            pass
        elif event.state & gtk.gdk.CONTROL_MASK:
            pass
        else:
            if keyname == "Up":
                prev_iter = self.get_tree_iter_prev_sibling(self.treestore, self.curr_tree_iter)
                if prev_iter: self.treeview_safe_set_cursor(prev_iter)
            elif keyname == "Down":
                next_iter = self.treestore.iter_next(self.curr_tree_iter)
                if next_iter: self.treeview_safe_set_cursor(next_iter)
            elif keyname == "Left":
                father_iter = self.treestore.iter_parent(self.curr_tree_iter)
                if father_iter: self.treeview_safe_set_cursor(father_iter)
            elif keyname == "Right":
                child_iter = self.treestore.iter_children(self.curr_tree_iter)
                if child_iter: self.treeview_safe_set_cursor(child_iter)
            elif keyname == "Return": self.toggle_tree_node_expanded_collapsed()
            elif keyname == "Delete": self.node_delete()
            elif keyname == "Menu":
                self.menu_tree.popup(None, None, None, 0, event.time)
        widget.stop_emission("key_press_event")

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

    def toggle_tree_text(self, *args):
        """Toggle Focus Between Tree and Text"""
        if self.treeview.is_focus():
            self.sourceview.grab_focus()
        else: self.treeview.grab_focus()

    def on_drag_motion_cherrytree(self, widget, drag_context, x, y, timestamp):
        """Cherry Tree drag motion"""
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
                    support.dialog_error(_("The new father can't be one of his sons!"), self.window)
                    return False
                move_towards_top_iter = self.treestore.iter_parent(move_towards_top_iter)
            if drop_pos == gtk.TREE_VIEW_DROP_BEFORE:
                prev_iter = self.get_tree_iter_prev_sibling(self.treestore, drop_iter)
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

    def on_key_press_choosenodedialog(self, widget, event):
        """Catches ChooseNode Dialog key presses"""
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == "Return":
            try: self.glade.choosenodedialog.get_widget_for_response(1).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED

    def on_key_press_anchorhandledialog(self, widget, event):
        """Catches AnchorHandle Dialog key presses"""
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == "Return":
            try: self.glade.anchorhandledialog.get_widget_for_response(1).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED

    def nodes_add_from_cherrytree_file(self, action):
        """Appends Nodes at the Bottom of the Current Ones, Importing from a CherryTree File"""
        filepath = support.dialog_file_select(filter_pattern=["*.ct*"],
                                              filter_name=_("CherryTree Document"),
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if not filepath: return
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
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
        try:
            file_descriptor = open(filepath, 'r')
            notecase_string = file_descriptor.read()
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        notecase = imports.NotecaseHandler()
        cherrytree_string = notecase.get_cherrytree_xml(notecase_string)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_tuxcards_file(self, action):
        """Add Nodes Parsing a TuxCards File"""
        filepath = support.dialog_file_select(curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
        try:
            file_descriptor = open(filepath, 'r')
            tuxcards_string = file_descriptor.read()
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        tuxcards = imports.TuxCardsHandler()
        cherrytree_string = tuxcards.get_cherrytree_xml(tuxcards_string)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_keepnote_folder(self, action):
        """Add Nodes Parsing a KeepNote Folder"""
        folderpath = support.dialog_folder_select(curr_folder=self.file_dir, parent=self.window)
        if folderpath == None: return
        keepnote = imports.KeepnoteHandler(folderpath)
        cherrytree_string = keepnote.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_zim_folder(self, action):
        """Add Nodes Parsing a Zim Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), "Notebooks/Notes")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if folderpath == None: return
        zim = imports.ZimHandler(folderpath)
        cherrytree_string = zim.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        zim.set_links_to_nodes(self)

    def nodes_add_from_gnote_folder(self, action):
        """Add Nodes Parsing a Gnote Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), ".local/share/gnote")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if folderpath == None: return
        gnote = imports.TomboyHandler(folderpath)
        cherrytree_string = gnote.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        gnote.set_links_to_nodes(self)

    def nodes_add_from_tomboy_folder(self, action):
        """Add Nodes Parsing a Tomboy Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), ".local/share/tomboy")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if folderpath == None: return
        tomboy = imports.TomboyHandler(folderpath)
        cherrytree_string = tomboy.get_cherrytree_xml()
        self.nodes_add_from_cherrytree_data(cherrytree_string)
        tomboy.set_links_to_nodes(self)

    def nodes_add_from_basket_folder(self, action):
        """Add Nodes Parsing a Basket Folder"""
        start_folder = os.path.join(os.path.expanduser('~'), ".kde/share/apps/basket/baskets")
        folderpath = support.dialog_folder_select(curr_folder=start_folder, parent=self.window)
        if folderpath == None: return
        basket = imports.BasketHandler(self, folderpath)
        if basket.check_basket_structure():
            cherrytree_string = basket.get_cherrytree_xml()
            self.nodes_add_from_cherrytree_data(cherrytree_string)
        else: support.dialog_error("%s is not a basket folder" % folderpath, self.window)

    def nodes_add_from_plain_text_file(self, action):
        """Add Nodes from Selected Plain Text File(s)"""
        filepath = support.dialog_file_select(filter_pattern=["*.txt", "*.TXT"] if cons.IS_WIN_OS else [],
                                              filter_mime=["text/*"] if not cons.IS_WIN_OS else [],
                                              filter_name=_("Plain Text Document"),
                                              curr_folder=self.file_dir, parent=self.window)
        if filepath == None: return
        plain = imports.PlainTextHandler()
        cherrytree_string = plain.get_cherrytree_xml(filepath=filepath)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_plain_text_folder(self, action):
        """Add Nodes from Plain Text File(s) in Selected Folder"""
        folderpath = support.dialog_folder_select(curr_folder=self.file_dir, parent=self.window)
        if folderpath == None: return
        plain = imports.PlainTextHandler()
        cherrytree_string = plain.get_cherrytree_xml(folderpath=folderpath)
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_treepad_file(self, action):
        """Add Nodes Parsing a Treepad File"""
        filepath = support.dialog_file_select(filter_pattern=["*.hjt"],
                                              filter_name=_("Treepad Document"),
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
        try:
            file_descriptor = open(filepath, 'r')
            treepad = imports.TreepadHandler()
            cherrytree_string = treepad.get_cherrytree_xml(file_descriptor)
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_keynote_file(self, action):
        """Add Nodes Parsing a Keynote File"""
        filepath = support.dialog_file_select(filter_pattern=["*.knt"],
                                              filter_name=_("KeyNote Document"),
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
        try:
            file_descriptor = open(filepath, 'r')
            keynote = imports.KeynoteHandler()
            cherrytree_string = keynote.get_cherrytree_xml(file_descriptor)
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_mempad_file(self, action):
        """Add Nodes Parsing a Mempad File"""
        filepath = support.dialog_file_select(filter_pattern=["*.lst"],
                                              filter_name=_("Mempad Document"),
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
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
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
        try:
            file_descriptor = open(filepath, 'r')
            knowit = imports.KnowitHandler()
            cherrytree_string = knowit.get_cherrytree_xml(file_descriptor)
            file_descriptor.close()
        except:
            support.dialog_error("Error importing the file %s" % filepath, self.window)
            raise
            return
        self.nodes_add_from_cherrytree_data(cherrytree_string)

    def nodes_add_from_leo_file(self, action):
        """Add Nodes Parsing a Leo File"""
        filepath = support.dialog_file_select(filter_pattern=["*.leo"],
                                              filter_name=_("Leo Document"),
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
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
                self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
            dialog = gtk.Dialog(title=_("Who is the Father?"),
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
                if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
                    self.curr_buffer.connect('insert-text', self.on_text_insertion)
                    self.curr_buffer.connect('delete-range', self.on_text_removal)
                    self.sourceview.modify_font(pango.FontDescription(self.text_font))
                else: self.sourceview.modify_font(pango.FontDescription(self.code_font))
                self.sourceview.set_sensitive(True)
                self.update_node_name_header()
                self.state_machine.node_selected_changed(self.treestore[self.curr_tree_iter][3])
                self.objects_buffer_refresh()
                # try to restore cursor position if in memory
                if self.treestore[former_node][3] in self.nodes_cursor_pos:
                    self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(self.nodes_cursor_pos[self.treestore[former_node][3]]))
                    self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
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
        if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            for menuitem in menu.get_children():
                try:
                    if menuitem.get_image().get_property("stock") == "gtk-paste":
                        menuitem.set_sensitive(True)
                except: pass
            self.menu_populate_popup(menu, cons.get_popup_menu_entries_text(self))
        else: self.menu_populate_popup(menu, cons.get_popup_menu_entries_code(self))

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
                    for element in ["Up", "Down", "Left", "Right", "Delete"]:
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
        self.menu_tree = gtk.Menu()
        top_menu_tree = self.ui.get_widget("/MenuBar/TreeMenu").get_submenu()
        for menuitem in top_menu_tree:
            top_menu_tree.remove(menuitem)
        self.menu_populate_popup(top_menu_tree, cons.get_popup_menu_tree(self))
        self.menu_populate_popup(self.menu_tree, cons.get_popup_menu_tree(self))

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
        self.glade.prefdialog.run()
        self.glade.prefdialog.hide()
        # special characters
        if self.glade.textbuffer_special_chars.get_modified():
            self.special_chars = unicode(self.glade.textbuffer_special_chars.get_text(*self.glade.textbuffer_special_chars.get_bounds()).replace(cons.CHAR_NEWLINE, ""), cons.STR_UTF8, cons.STR_IGNORE)
            self.glade.textbuffer_special_chars.set_modified(False)
            support.set_menu_items_special_chars(self)
        # timer activate/modify handling
        new_autosave_value = int(self.glade.spinbutton_autosave.get_value())
        if self.autosave[1] != new_autosave_value:
            self.autosave[1] = new_autosave_value
            if self.autosave_timer_id != None: self.autosave_timer_stop()
        if self.autosave[0] and self.autosave_timer_id == None: self.autosave_timer_start()
        # update config file (for people that do not close the app but just logout/shutdown)
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
        self.mod_time_sentinel_id = gobject.timeout_add(20*1000, self.modification_time_sentinel_iter) # 20 sec

    def modification_time_sentinel_stop(self):
        """Stop Timer that checks for modification time"""
        gobject.source_remove(self.mod_time_sentinel_id)
        self.mod_time_sentinel_id = None

    def modification_time_sentinel_iter(self):
        """Iteration of the Modification Time Sentinel"""
        if self.file_dir and self.file_name and self.mod_time_val:
            file_path = os.path.join(self.file_dir, self.file_name)
            if os.path.isfile(file_path):
                read_mod_time = os.path.getmtime(file_path)
                #print "former modified: %s (%s)" % (time.ctime(self.mod_time_val), self.mod_time_val)
                #print "last modified: %s (%s)" % (time.ctime(read_mod_time), read_mod_time)
                if read_mod_time != self.mod_time_val:
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
        do_show = True
        for runn_win in self.boss.running_windows:
            if runn_win.window.has_toplevel_focus():
                do_show = False
                break
        for runn_win in self.boss.running_windows:
            if do_show:
                runn_win.window.show()
                runn_win.window.deiconify()
                runn_win.window.present()
                runn_win.window.move(runn_win.win_position[0], runn_win.win_position[1])
                #print "restored position", runn_win.win_position[0], runn_win.win_position[1]
            else:
                runn_win.win_position = runn_win.window.get_position()
                runn_win.window.hide()

    def on_mouse_button_clicked_systray(self, widget, event):
        """Catches mouse buttons clicks upon the system tray icon"""
        if event.button == 1: self.toggle_show_hide_main_window()
        elif event.button == 3: self.ui.get_widget("/SysTrayMenu").popup(None, None, None, event.button, event.time)

    def node_id_get(self):
        """Returns the node_ids, all Different Each Other"""
        new_node_id = 1
        while self.get_tree_iter_from_node_id(new_node_id) or new_node_id in self.ctdb_handler.nodes_to_rm_set:
            new_node_id += 1
        return new_node_id

    def on_fontbutton_text_font_set(self, picker):
        """A New Font For the Text was Chosen"""
        self.text_font = picker.get_font_name()
        if self.curr_tree_iter and self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.sourceview.modify_font(pango.FontDescription(self.text_font))

    def on_fontbutton_code_font_set(self, picker):
        """A New Font For the Text was Chosen"""
        self.code_font = picker.get_font_name()
        if self.curr_tree_iter and self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            self.sourceview.modify_font(pango.FontDescription(self.code_font))

    def on_fontbutton_tree_font_set(self, picker):
        """A New Font For the Tree was Chosen"""
        self.tree_font = picker.get_font_name()
        self.set_treeview_font()

    def set_treeview_font(self):
        """Update the TreeView Font"""
        self.renderer_text.set_property('font-desc', pango.FontDescription(self.tree_font))
        self.treeview_refresh()

    def treeview_refresh(self, change_icon=False):
        """Refresh the Treeview"""
        if self.curr_buffer:
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

    def change_icon_iter(self, tree_iter):
        """Changing all icons type - iter"""
        self.treestore[tree_iter][0] = self.get_node_icon(self.treestore.iter_depth(tree_iter),
                                                          self.treestore[tree_iter][4])
        child_tree_iter = self.treestore.iter_children(tree_iter)
        while child_tree_iter:
            self.change_icon_iter(child_tree_iter)
            child_tree_iter = self.treestore.iter_next(child_tree_iter)

    def file_startup_load(self, open_with_file, node_name):
        """Try to load a file if there are the conditions"""
        if open_with_file:
            self.file_name = os.path.basename(open_with_file)
            self.file_dir = os.path.dirname(open_with_file)
        elif self.boss.running_windows:
            self.file_name = ""
            return
        if self.file_dir and self.file_name and os.path.isfile(os.path.join(self.file_dir, self.file_name)):
            self.file_load(os.path.join(self.file_dir, self.file_name))
            self.modification_time_update_value(True)
            if self.rest_exp_coll == 1: self.treeview.expand_all()
            elif self.rest_exp_coll == 0: config.set_tree_expanded_collapsed_string(self)
            # is there a node name to focus?
            if node_name:
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
            if self.node_path != None:
                try: node_iter_to_focus = self.treestore.get_iter(self.node_path)
                except: node_iter_to_focus = None
                if node_iter_to_focus:
                    self.treeview_safe_set_cursor(node_iter_to_focus)
                    self.sourceview.grab_focus()
                    self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(self.cursor_position))
                    self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
            else: node_iter_to_focus = None
            if not node_iter_to_focus:
                node_iter_to_focus = self.treestore.get_iter_first()
                if node_iter_to_focus:
                    self.treeview.set_cursor(self.treestore.get_path(node_iter_to_focus))
                    self.sourceview.grab_focus()
        else:
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
        if not self.dialog_choose_data_storage(): return
        filename_hint = self.file_name[:-1] + self.filetype if len(self.file_name) > 4 else ""
        filepath = support.dialog_file_save_as(filename_hint,
                                               filter_pattern="*.ct" + self.filetype,
                                               filter_name=_("CherryTree Document"),
                                               curr_folder=self.file_dir,
                                               parent=self.window)
        restore_filetype = False
        if filepath == None: restore_filetype = True
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
            self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
            self.objects_buffer_refresh()
        self.modification_time_update_value(True)

    def filepath_extension_fix(self, filepath):
        """Check a filepath to have the proper extension"""
        extension = ".ct" + self.filetype
        if len(filepath) < 4 or filepath[-4:] != extension: return filepath + extension
        return filepath

    def file_save(self, *args):
        """Save the file"""
        if self.file_dir != "" and self.file_name != "":
            if self.file_update or (self.curr_tree_iter != None and self.curr_buffer.get_modified() == True):
                self.modification_time_update_value(False)
                if self.is_tree_not_empty_or_error() \
                and self.file_write(os.path.join(self.file_dir, self.file_name), first_write=False):
                    self.update_window_save_not_needed()
                    self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
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
            if xml_string: file_descriptor = open(filepath_tmp, 'w')
            else:
                if first_write:
                    if not exporting:
                        if "db" in dir(self): self.db_old = self.db
                        self.db = self.ctdb_handler.new_db(filepath_tmp)
                        if "db_old" in dir(self):
                            self.db_old.close()
                            del self.db_old
                    elif exporting in ["a", "s", "n"]:
                        print "exporting", exporting
                        exp_db = self.ctdb_handler.new_db(filepath_tmp, exporting, sel_range)
                        exp_db.close()
                else: self.ctdb_handler.pending_data_write(self.db)
        else:
            if xml_string: file_descriptor = open(filepath, 'w')
            else:
                if first_write:
                    if not exporting:
                        if "db" in dir(self): self.db_old = self.db
                        self.db = self.ctdb_handler.new_db(filepath)
                        if "db_old" in dir(self):
                            self.db_old.close()
                            del self.db_old
                    elif exporting in ["a", "s", "n"]:
                        print "exporting", exporting
                        exp_db = self.ctdb_handler.new_db(filepath, exporting, sel_range)
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
            if os.path.isfile(filepath):
                # clean older archive content
                subprocess.call("7za d %s" % esc_filepath, shell=True)
            bash_str = '7za a -p%s -w%s -bd -y %s %s' % (self.password,
                                                         esc_tmp_folder,
                                                         esc_filepath,
                                                         esc_filepath_tmp)
            #print bash_str
            if not xml_string and not exporting: self.db.close()
            ret_code = subprocess.call(bash_str, shell=True)
            #print ret_code
            if xml_string: os.remove(filepath_tmp)
            elif not exporting:
                self.db = self.ctdb_handler.get_connected_db_from_dbpath(filepath_tmp)
                self.ctdb_handler.remove_at_quit_set.add(filepath_tmp)

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
        if self.backup_copy and os.path.isfile(filepath):
            try: shutil.copy(filepath, filepath + cons.CHAR_TILDE)
            except: subprocess.call("cp %s %s~" % (re.escape(filepath), re.escape(filepath)), shell=True)
        # if the filename is protected, we use unprotected type before compress and protect
        try:
            self.statusbar.push(self.statusbar_context_id, _("Writing to Disk..."))
            while gtk.events_pending(): gtk.main_iteration()
            self.file_write_low_level(filepath, xml_string, first_write)
            self.statusbar.pop(self.statusbar_context_id)
            return True
        except:
            if not os.path.isfile(filepath) and os.path.isfile(filepath + cons.CHAR_TILDE):
                try: os.rename(filepath + cons.CHAR_TILDE, filepath)
                except:
                    print "os.rename failed"
                    subprocess.call("mv %s~ %s" % (re.escape(filepath), re.escape(filepath)), shell=True)
            support.dialog_error("%s write failed - writing to disk" % filepath, self.window)
            raise
            return False

    def file_open(self, *args):
        """Opens a dialog to browse for a cherrytree filepath"""
        filepath = support.dialog_file_select(filter_pattern=["*.ct*"],
                                              filter_name=_("CherryTree Document"),
                                              curr_folder=self.file_dir,
                                              parent=self.window)
        if filepath == None: return
        self.filepath_open(filepath)

    def filepath_open(self, filepath, force_reset=False):
        """Opens an existing filepath"""
        if not self.reset(force_reset): return
        self.file_load(filepath)
        self.modification_time_update_value(True)
        if self.rest_exp_coll == 1: self.treeview.expand_all()
        first_node_iter = self.treestore.get_iter_first()
        if first_node_iter != None:
            self.treeview.set_cursor(self.treestore.get_path(first_node_iter))
            self.sourceview.grab_focus()

    def dialog_choose_data_storage(self, *args):
        """Choose the CherryTree data storage type (xml or db) and protection"""
        dialog = gtk.Dialog(title=_("Choose Storage Type"),
                            parent=self.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        dialog.set_default_size(350, -1)
        radiobutton_sqlite_not_protected = gtk.RadioButton(label="SQLite, " + _("Not Protected") + " (.ctb)")
        radiobutton_sqlite_pass_protected = gtk.RadioButton(label="SQLite, " + _("Password Protected") + " (.ctx)")
        radiobutton_sqlite_pass_protected.set_group(radiobutton_sqlite_not_protected)
        radiobutton_xml_not_protected = gtk.RadioButton(label="XML, " + _("Not Protected") + " (.ctd)")
        radiobutton_xml_not_protected.set_group(radiobutton_sqlite_not_protected)
        radiobutton_xml_pass_protected = gtk.RadioButton(label="XML, " + _("Password Protected") + " (.ctz)")
        radiobutton_xml_pass_protected.set_group(radiobutton_sqlite_not_protected)
        type_vbox = gtk.VBox()
        type_vbox.pack_start(radiobutton_sqlite_not_protected)
        type_vbox.pack_start(radiobutton_sqlite_pass_protected)
        type_vbox.pack_start(radiobutton_xml_not_protected)
        type_vbox.pack_start(radiobutton_xml_pass_protected)
        type_frame = gtk.Frame(label="<b>"+_("Storage Type")+"</b>")
        type_frame.get_label_widget().set_use_markup(True)
        type_frame.set_shadow_type(gtk.SHADOW_NONE)
        type_frame.add(type_vbox)
        entry_passw_1 = gtk.Entry()
        entry_passw_1.set_visibility(False)
        entry_passw_2 = gtk.Entry()
        entry_passw_2.set_visibility(False)
        vbox_passw = gtk.VBox()
        vbox_passw.pack_start(entry_passw_1)
        vbox_passw.pack_start(entry_passw_2)
        passw_frame = gtk.Frame(label="<b>"+_("Enter the New Password Twice")+"</b>")
        passw_frame.get_label_widget().set_use_markup(True)
        passw_frame.set_shadow_type(gtk.SHADOW_NONE)
        passw_frame.add(vbox_passw)
        if len(self.file_name) > 4:
            if self.file_name[-1] == "b": radiobutton_sqlite_not_protected.set_active(True)
            elif self.file_name[-1] == "x": radiobutton_sqlite_pass_protected.set_active(True)
            elif self.file_name[-1] == "d": radiobutton_xml_not_protected.set_active(True)
            else: radiobutton_xml_pass_protected.set_active(True)
        if self.password: passw_frame.set_sensitive(True)
        else: passw_frame.set_sensitive(False)
        content_area = dialog.get_content_area()
        content_area.set_spacing(5)
        content_area.pack_start(type_frame)
        content_area.pack_start(passw_frame)
        content_area.show_all()
        def on_radiobutton_savetype_toggled(widget):
            if radiobutton_sqlite_pass_protected.get_active()\
            or radiobutton_xml_pass_protected.get_active():
                passw_frame.set_sensitive(True)
                entry_passw_1.grab_focus()
            else: passw_frame.set_sensitive(False)
        def on_key_press_edit_data_storage_type_dialog(widget, event):
            if gtk.gdk.keyval_name(event.keyval) == "Return":
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
        radiobutton_sqlite_not_protected.connect("toggled", on_radiobutton_savetype_toggled)
        radiobutton_sqlite_pass_protected.connect("toggled", on_radiobutton_savetype_toggled)
        radiobutton_xml_not_protected.connect("toggled", on_radiobutton_savetype_toggled)
        dialog.connect("key_press_event", on_key_press_edit_data_storage_type_dialog)
        response = dialog.run()
        storage_type_is_xml = (radiobutton_xml_not_protected.get_active()\
                               or radiobutton_xml_pass_protected.get_active())
        new_protection = {'on': (radiobutton_sqlite_pass_protected.get_active()\
                                 or radiobutton_xml_pass_protected.get_active()),
                          'p1': unicode(entry_passw_1.get_text(), cons.STR_UTF8, cons.STR_IGNORE),
                          'p2': unicode(entry_passw_2.get_text(), cons.STR_UTF8, cons.STR_IGNORE)}
        dialog.destroy()
        if response != gtk.RESPONSE_ACCEPT: return False
        if new_protection['on']:
            if new_protection['p1'] == "":
                support.dialog_error(_("The Password Fields Must be Filled"), self.window)
                return False
            if new_protection['p1'] != new_protection['p2']:
                support.dialog_error(_("The Two Inserted Passwords Do Not Match"), self.window)
                return False
            bad_chars_list = [cons.CHAR_SQUOTE, cons.CHAR_DQUOTE, cons.CHAR_BSLASH, cons.CHAR_SPACE]
            for bad_char in bad_chars_list:
                if bad_char in new_protection['p1']:
                    support.dialog_error(_("The Characters  %s  are Not Allowed") % cons.CHAR_SPACE.join(bad_chars_list), self.window)
                    return False
            if not new_protection['p1'] or not self.is_7za_available(): return False
            self.password = new_protection['p1']
        else: self.password = None
        if storage_type_is_xml:
            if self.password: self.filetype = "z"
            else: self.filetype = "d"
        else:
            if self.password: self.filetype = "x"
            else: self.filetype = "b"
        #print "self.filetype = '%s'" % self.filetype
        return True

    def is_7za_available(self):
        """Check 7za binary executable to be available"""
        ret_code = subprocess.call("7za", shell=True)
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
            if gtk.gdk.keyval_name(event.keyval) == "Return":
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
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
            password_str = self.dialog_insert_password(os.path.basename(filepath))
            if not password_str: return None
            if main_file: self.password = password_str
            if not self.is_7za_available(): return None
            if cons.IS_WIN_OS:
                drive, tail = os.path.splitdrive(os.path.dirname(filepath))
                if tail.startswith(cons.CHAR_BSLASH): tail = tail[1:]
            else:
                tail = os.path.dirname(filepath)
                if tail.startswith(cons.CHAR_SLASH): tail = tail[1:]
            tree_tmp_folder = os.path.join(cons.TMP_FOLDER, tail)
            if not os.path.isdir(tree_tmp_folder): os.makedirs(tree_tmp_folder)
            last_letter = "d" if filepath[-1] == "z" else "b"
            filepath_tmp = os.path.join(tree_tmp_folder, os.path.basename(filepath[:-1] + last_letter))
            if cons.IS_WIN_OS:
                esc_tmp_folder = support.windows_cmd_prepare_path(tree_tmp_folder)
                esc_filepath = support.windows_cmd_prepare_path(filepath)
            else:
                esc_tmp_folder = re.escape(tree_tmp_folder)
                esc_filepath = re.escape(filepath)
            bash_str = '7za e -p%s -w%s -bd -y -o%s %s' % (password_str,
                                                           esc_tmp_folder,
                                                           esc_tmp_folder,
                                                           esc_filepath)
            #print bash_str
            ret_code = subprocess.call(bash_str, shell=True)
            if ret_code != 0:
                support.dialog_error(_('Wrong Password'), self.window)
                return None
            if not os.path.isfile(filepath_tmp):
                print "? the compressed file was renamed"
                files_list = glob.glob(os.path.join(tree_tmp_folder, "*"+filepath_tmp[-4:]))
                if len(files_list) == 1:
                    old_filepath_tmp = files_list[0]
                    os.rename(old_filepath_tmp, filepath_tmp)
        elif filepath[-1] not in ["d", "b"]:
            print "bad filepath[-1]", filepath[-1]
            return False
        elif main_file: self.password = None
        if filepath[-1] in ["d", "z"]:
            try:
                if password_protected: file_descriptor = open(filepath_tmp, 'r')
                else: file_descriptor = open(filepath, 'r')
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
                support.add_recent_document(self, filepath)
                support.set_bookmarks_menu_items(self)
                self.update_window_save_not_needed()
            if user_active_restore: self.user_active = True
        if not document_loaded_ok:
            support.dialog_error(_('"%s" is Not a CherryTree Document') % filepath, self.window)
            self.file_name = ""

    def file_new(self, *args):
        """Starts a new unsaved instance"""
        if self.reset(): self.node_add()

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
        self.file_name = ""
        self.password = None
        self.update_window_save_not_needed()
        self.state_machine.reset()
        self.sourceview.set_sensitive(False)
        if "db" in dir(self) and self.db: self.db.close()
        for filepath_tmp in self.ctdb_handler.remove_at_quit_set: os.remove(filepath_tmp)
        self.ctdb_handler.reset()
        if user_active_restore: self.user_active = True
        return True

    def export_to_ctd(self, action):
        """Export the Selected Node and its Subnodes"""
        if not self.is_there_selected_node_or_error(): return
        export_type = support.dialog_selnode_selnodeandsub_alltree(self.window,
                                                                   also_selection=True)[0]
        if export_type == 0: return
        ctd_handler = exports.Export2CTD(self)
        restore_passw = self.password
        restore_filetype = self.filetype
        if not self.dialog_choose_data_storage(): return
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
                    self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
                    self.objects_buffer_refresh()
        elif export_type == 3:
            # all nodes
            proposed_name = self.file_name[:-1] + self.filetype
            ctd_filepath = ctd_handler.get_single_ct_filepath(proposed_name)
            if ctd_filepath:
                ctd_handler.nodes_all_export_to_ctd(ctd_filepath)
                if restore_filetype in ["b", "x"] and self.curr_tree_iter:
                    self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
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
        export_type, include_node_name = support.dialog_selnode_selnodeandsub_alltree(self.window,
                                                            also_selection=True, also_node_name=True)
        if export_type == 0: return
        txt_handler = exports.Export2Txt(self)
        if export_type == 1:
            # only selected node
            proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
            txt_filepath = txt_handler.get_single_txt_filepath(proposed_name)
            if txt_filepath:
                tree_iter_for_node_name = self.curr_tree_iter if include_node_name else None
                txt_handler.node_export_to_txt(self.curr_buffer, txt_filepath, tree_iter_for_node_name=tree_iter_for_node_name)
        elif export_type == 2:
            # selected node and subnodes
            if self.export_single:
                txt_filepath = txt_handler.get_single_txt_filepath(self.file_name)
                if txt_filepath:
                    txt_handler.nodes_all_export_to_txt(top_tree_iter=self.curr_tree_iter, single_txt_filepath=txt_filepath, include_node_name=include_node_name)
            else:
                folder_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                if txt_handler.prepare_txt_folder(folder_name):
                    txt_handler.nodes_all_export_to_txt(self.curr_tree_iter, include_node_name=include_node_name)
        elif export_type == 3:
            # all nodes
            if self.export_single:
                txt_filepath = txt_handler.get_single_txt_filepath(self.file_name)
                if txt_filepath:
                    txt_handler.nodes_all_export_to_txt(single_txt_filepath=txt_filepath, include_node_name=include_node_name)
            else:
                if txt_handler.prepare_txt_folder(self.file_name):
                    txt_handler.nodes_all_export_to_txt(include_node_name=include_node_name)
        else:
            # only selection
            if self.is_there_text_selection_or_error():
                iter_start, iter_end = self.curr_buffer.get_selection_bounds()
                proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                txt_filepath = txt_handler.get_single_txt_filepath(proposed_name)
                if txt_filepath:
                    sel_range = [iter_start.get_offset(), iter_end.get_offset()]
                    tree_iter_for_node_name = self.curr_tree_iter if include_node_name else None
                    txt_handler.node_export_to_txt(self.curr_buffer, txt_filepath, sel_range, tree_iter_for_node_name=tree_iter_for_node_name)

    def export_to_html(self, *args):
        """Export to HTML"""
        if not self.is_there_selected_node_or_error(): return
        export_type = support.dialog_selnode_selnodeandsub_alltree(self.window,
                                                                   also_selection=True)[0]
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
                    self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
                    self.objects_buffer_refresh()
        elif export_type == 3:
            # all nodes
            if self.html_handler.prepare_html_folder(self.file_name):
                self.html_handler.nodes_all_export_to_html()
                if self.filetype in ["b", "x"] and self.curr_tree_iter:
                    self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
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
        self.print_handler.page_setup = gtk.print_run_page_setup_dialog(self.glade.window,
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
        export_type, include_node_name = support.dialog_selnode_selnodeandsub_alltree(self.window,
                                                          also_selection=True, also_node_name=True)
        if export_type == 0: return
        pdf_handler = exports.ExportPrint(self)
        if export_type == 1:
            # only selected node
            if self.print_handler.pdf_filepath == cons.CHAR_TILDE:
                proposed_name = support.get_node_hierarchical_name(self, self.curr_tree_iter)
                pdf_filepath = pdf_handler.get_pdf_filepath(proposed_name)
                if not pdf_filepath: return
                self.print_handler.pdf_filepath = pdf_filepath
            pdf_handler.node_export_print(self.curr_tree_iter, include_node_name)
        elif export_type == 2:
            # selected node and subnodes
            if self.print_handler.pdf_filepath == cons.CHAR_TILDE:
                pdf_filepath = pdf_handler.get_pdf_filepath(self.file_name)
                if not pdf_filepath: return
                self.print_handler.pdf_filepath = pdf_filepath
            pdf_handler.nodes_all_export_print(self.curr_tree_iter, include_node_name)
        elif export_type == 3:
            # all nodes
            if self.print_handler.pdf_filepath == cons.CHAR_TILDE:
                pdf_filepath = pdf_handler.get_pdf_filepath(self.file_name)
                if not pdf_filepath: return
                self.print_handler.pdf_filepath = pdf_filepath
            pdf_handler.nodes_all_export_print(None, include_node_name)
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
                                              include_node_name,
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
        if self.tree_sort_level_and_sublevels(self.treestore, None, True): self.update_window_save_needed()

    def tree_sort_descending(self, *args):
        """Sorts the Tree Descending"""
        if self.tree_sort_level_and_sublevels(self.treestore, None, False): self.update_window_save_needed()

    def node_siblings_sort_ascending(self, *args):
        """Sorts all the Siblings of the Selected Node Ascending"""
        if not self.is_there_selected_node_or_error(): return
        father_iter = self.treestore.iter_parent(self.curr_tree_iter)
        movements = False
        while self.node_siblings_sort_iteration(self.treestore, father_iter, True, 1):
            movements = True
        if movements: self.update_window_save_needed()

    def node_siblings_sort_descending(self, *args):
        """Sorts all the Siblings of the Selected Node Descending"""
        if not self.is_there_selected_node_or_error(): return
        father_iter = self.treestore.iter_parent(self.curr_tree_iter)
        movements = False
        while self.node_siblings_sort_iteration(self.treestore, father_iter, False, 1):
            movements = True
        if movements: self.update_window_save_needed()

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
        """Change the Selected Node's Children Syntax Highlighting to the Father's Syntax Highlighting"""
        if not self.is_there_selected_node_or_error(): return
        self.node_inherit_syntax_iter(self.curr_tree_iter)
        self.sourceview.set_buffer(self.treestore[self.curr_tree_iter][2])

    def node_inherit_syntax_iter(self, iter_father):
        """Iteration of the Node Inherit Syntax"""
        iter_child = self.treestore.iter_children(iter_father)
        while iter_child != None:
            if self.treestore[iter_child][4] != self.treestore[iter_father][4]:
                self.get_textbuffer_from_tree_iter(iter_child)
                self.treestore[iter_child][4] = self.treestore[iter_father][4]
                self.treestore[iter_child][0] = self.get_node_icon(self.treestore.iter_depth(iter_child),
                                                                   self.treestore[iter_child][4])
                self.switch_buffer_text_source(self.treestore[iter_child][2],
                                               iter_child,
                                               self.treestore[iter_child][4])
                if self.treestore[iter_child][4] != cons.CUSTOM_COLORS_ID:
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
        """Move a node to a father and after a brother"""
        if brother_iter != None:
            new_node_iter = self.treestore.insert_after(father_iter, brother_iter, self.treestore[iter_to_move])
        elif set_first: new_node_iter = self.treestore.prepend(father_iter, self.treestore[iter_to_move])
        else: new_node_iter = self.treestore.append(father_iter, self.treestore[iter_to_move])
        # we move also all the children
        self.node_move_children(iter_to_move, new_node_iter)
        # now we can remove the old iter (and all children)
        self.treestore.remove(iter_to_move)
        self.nodes_sequences_fix(None, True)
        if father_iter != None: self.treeview.expand_row(self.treestore.get_path(father_iter), True)
        else: self.treeview.expand_row(self.treestore.get_path(new_node_iter), True)
        self.curr_tree_iter = new_node_iter
        new_node_path = self.treestore.get_path(new_node_iter)
        self.treeview.collapse_row(new_node_path)
        self.treeview.set_cursor(new_node_path)
        self.update_window_save_needed()

    def node_move_children(self, old_father, new_father):
        """Move the children from a father to another"""
        children_iter_to_move = self.treestore.iter_children(old_father)
        while children_iter_to_move != None:
            new_children_iter = self.treestore.append(new_father, self.treestore[children_iter_to_move])
            self.node_move_children(children_iter_to_move, new_children_iter)
            children_iter_to_move = self.treestore.iter_next(children_iter_to_move)

    def node_change_father(self, *args):
        """Node browse for a new father"""
        if not self.is_there_selected_node_or_error(): return
        curr_node_id = self.treestore[self.curr_tree_iter][3]
        old_father_iter = self.treestore.iter_parent(self.curr_tree_iter)
        if old_father_iter != None: old_father_node_id = self.treestore[old_father_iter][3]
        else: old_father_node_id = None
        self.node_choose_view_exist_or_create()
        self.glade.link_dialog_top_vbox.hide()
        self.glade.frame_link_anchor.hide()
        self.glade.choosenodedialog.set_title(_("Select the New Father"))
        self.glade.hbox_link_node_anchor.set_sensitive(True)
        response = self.glade.choosenodedialog.run()
        self.glade.choosenodedialog.hide()
        if response != 1: return # the user aborted the operation
        model, father_iter = self.treeviewselection_2.get_selected()
        new_father_node_id = self.treestore[father_iter][3]
        if curr_node_id == new_father_node_id:
            support.dialog_error(_("The new father can't be the very node to move!"), self.window)
            return
        if old_father_node_id != None and new_father_node_id == old_father_node_id:
            support.dialog_info(_("The new choosed father is again the old father!"), self.window)
            return
        move_towards_top_iter = self.treestore.iter_parent(father_iter)
        while move_towards_top_iter:
            if self.treestore[move_towards_top_iter][3] == curr_node_id:
                support.dialog_error(_("The new father can't be one of his sons!"), self.window)
                return
            move_towards_top_iter = self.treestore.iter_parent(move_towards_top_iter)
        self.node_move_after(self.curr_tree_iter, father_iter)
        if self.nodes_icons == "c": self.treeview_refresh(change_icon=True)

    def node_choose_view_exist_or_create(self, node_sel_id=None):
        """If The View Was Never Used, this will Create It"""
        if "treeview_2" not in dir(self):
            self.treeview_2 = gtk.TreeView(self.treestore)
            self.treeview_2.set_headers_visible(False)
            self.renderer_pixbuf_2 = gtk.CellRendererPixbuf()
            self.renderer_text_2 = gtk.CellRendererText()
            self.column_2 = gtk.TreeViewColumn()
            self.column_2.pack_start(self.renderer_pixbuf_2, False)
            self.column_2.pack_start(self.renderer_text_2, True)
            self.column_2.set_attributes(self.renderer_pixbuf_2, stock_id=0)
            self.column_2.set_attributes(self.renderer_text_2, text=1)
            self.treeview_2.append_column(self.column_2)
            self.treeviewselection_2 = self.treeview_2.get_selection()
            self.treeview_2.connect('button-press-event', self.on_mouse_button_clicked_treeview_2)
            self.glade.scrolledwindow_choosenode.add(self.treeview_2)
            self.glade.scrolledwindow_choosenode.show_all()
        if node_sel_id == None:
            self.treeview_2.set_cursor(self.treestore.get_path(self.curr_tree_iter))
        else:
            tree_iter_sel = self.get_tree_iter_from_node_id(node_sel_id)
            if tree_iter_sel != None: self.treeview_2.set_cursor(self.treestore.get_path(tree_iter_sel))

    def on_mouse_button_clicked_treeview_2(self, widget, event):
        """Catches mouse buttons clicks"""
        if event.button != 1: return
        if event.type == gtk.gdk._2BUTTON_PRESS: self.glade.choosenodedialog_button_ok.clicked()

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

    def treeview_safe_set_cursor(self, tree_iter):
        """Set Cursor being sure the Node is Expanded"""
        father_iter = self.treestore.iter_parent(tree_iter)
        if father_iter:
            father_path = self.treestore.get_path(father_iter)
            self.treeview.expand_to_path(father_path)
        tree_path = self.treestore.get_path(tree_iter)
        self.treeview.set_cursor(tree_path)

    def on_table_column_rename_radiobutton_toggled(self, radiobutton):
        """Table Column Rename Toggled"""
        if radiobutton.get_active():
            self.glade.table_column_rename_entry.set_sensitive(True)
            self.table_column_mode = "rename"
        else: self.glade.table_column_rename_entry.set_sensitive(False)

    def on_table_column_delete_radiobutton_toggled(self, radiobutton):
        """Table Column Delete Toggled"""
        if radiobutton.get_active(): self.table_column_mode = "delete"

    def on_table_column_add_radiobutton_toggled(self, radiobutton):
        """Table Column Delete Toggled"""
        if radiobutton.get_active():
            self.glade.table_column_new_entry.set_sensitive(True)
            self.table_column_mode = "add"
        else: self.glade.table_column_new_entry.set_sensitive(False)

    def on_table_column_left_radiobutton_toggled(self, radiobutton):
        """Table Column Left Toggled"""
        if radiobutton.get_active(): self.table_column_mode = cons.TAG_PROP_LEFT

    def on_table_column_right_radiobutton_toggled(self, radiobutton):
        """Table Column Right Toggled"""
        if radiobutton.get_active(): self.table_column_mode = cons.TAG_PROP_RIGHT

    def on_help_menu_item_activated(self, menuitem, data=None):
        """Show the Online Manual"""
        webbrowser.open("http://giuspen.com/cherrytreemanual/Introduction.html")

    def on_button_strftime_help_clicked(self, menuitem, data=None):
        """Show the strftime Manual Page"""
        lang_code = locale.getdefaultlocale()[0]
        if lang_code:
            page_lang = lang_code[0:2] if lang_code[0:2] in ["de", "es", "fr"] else ""
        else: page_lang = ""
        webbrowser.open("http://man.cx/strftime%283%29/" + page_lang)

    def on_spinbutton_tab_width_value_changed(self, spinbutton):
        """Tabs Width (in chars) Change Handling"""
        self.tabs_width = int(spinbutton.get_value())
        self.sourceview.set_tab_width(self.tabs_width)

    def on_spinbutton_limit_undoable_steps_value_changed(self, spinbutton):
        """Limit Undoable Steps Change Handling"""
        self.limit_undoable_steps = int(spinbutton.get_value())

    def on_spinbutton_tree_nodes_names_width_value_changed(self, spinbutton):
        """Cherry Wrap Width Change Handling"""
        self.cherry_wrap_width = int(spinbutton.get_value())
        self.renderer_text.set_property('wrap-width', self.cherry_wrap_width)
        self.treeview_refresh()

    def on_checkbutton_custom_weblink_cmd_toggled(self, checkbutton):
        """Custom Web Link Clicked Action Toggled Handling"""
        if checkbutton.get_active(): self.weblink_custom_action[0] = True
        else: self.weblink_custom_action[0] = False
        self.glade.entry_custom_weblink_cmd.set_sensitive(self.weblink_custom_action[0])

    def on_entry_custom_weblink_cmd_changed(self, entry):
        """Custom Web Link Clicked Action Edited"""
        self.weblink_custom_action[1] = entry.get_text()

    def on_checkbutton_custom_filelink_cmd_toggled(self, checkbutton):
        """Custom File Link Clicked Action Toggled Handling"""
        if checkbutton.get_active(): self.filelink_custom_action[0] = True
        else: self.filelink_custom_action[0] = False
        self.glade.entry_custom_filelink_cmd.set_sensitive(self.filelink_custom_action[0])

    def on_entry_custom_filelink_cmd_changed(self, entry):
        """Custom File Link Clicked Action Edited"""
        self.filelink_custom_action[1] = entry.get_text()

    def on_checkbutton_custom_folderlink_cmd_toggled(self, checkbutton):
        """Custom Folder Link Clicked Action Toggled Handling"""
        if checkbutton.get_active(): self.folderlink_custom_action[0] = True
        else: self.folderlink_custom_action[0] = False
        self.glade.entry_custom_folderlink_cmd.set_sensitive(self.folderlink_custom_action[0])

    def on_entry_custom_folderlink_cmd_changed(self, entry):
        """Custom Folder Link Clicked Action Edited"""
        self.folderlink_custom_action[1] = entry.get_text()

    def on_entry_timestamp_format_changed(self, entry):
        """Timestamp Format Edited"""
        self.timestamp_format = entry.get_text()

    def on_entry_horizontal_rule_changed(self, entry):
        """Horizontal Rule Edited"""
        if not self.user_active: return
        self.h_rule = entry.get_text()

    def on_checkbutton_systray_toggled(self, checkbutton):
        """SysTray Toggled Handling"""
        self.systray = checkbutton.get_active()
        if self.systray:
            self.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property(cons.STR_VISIBLE, True)
            self.glade.checkbutton_start_on_systray.set_sensitive(True)
        else:
            self.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property(cons.STR_VISIBLE, False)
            self.glade.checkbutton_start_on_systray.set_sensitive(False)
        if not self.user_active: return
        if self.systray:
            if not self.use_appind:
                if "status_icon" in dir(self.boss): self.boss.status_icon.set_property(cons.STR_VISIBLE, True)
                else: self.status_icon_enable()
            else:
                if "ind" in dir(self.boss): self.boss.ind.set_status(appindicator.STATUS_ACTIVE)
                else: self.status_icon_enable()
        else:
            if not self.use_appind: self.boss.status_icon.set_property(cons.STR_VISIBLE, False)
            else: self.boss.ind.set_status(appindicator.STATUS_PASSIVE)
        self.boss.systray_active = self.systray
        if len(self.boss.running_windows) > 1:
            for runn_win in self.boss.running_windows:
                if runn_win.window == self.window: continue
                runn_win.user_active = False
                runn_win.systray = self.boss.systray_active
                runn_win.glade.checkbutton_systray.set_active(self.boss.systray_active)
                runn_win.user_active = True

    def on_checkbutton_start_on_systray_toggled(self, checkbutton):
        """Start Minimized on SysTray Toggled Handling"""
        if checkbutton.get_active(): self.start_on_systray = True
        else: self.start_on_systray = False

    def on_checkbutton_use_appind_toggled(self, checkbutton):
        """Use AppIndicator Toggled Handling"""
        if not self.user_active: return
        if self.glade.checkbutton_systray.get_active():
            former_active = True
            self.glade.checkbutton_systray.set_active(False)
        else: former_active = False
        if checkbutton.get_active(): self.use_appind = True
        else: self.use_appind = False
        if former_active:
            self.glade.checkbutton_systray.set_active(True)
        if len(self.boss.running_windows) > 1:
            for runn_win in self.boss.running_windows:
                if runn_win.window == self.window: continue
                runn_win.user_active = False
                runn_win.use_appind = self.use_appind
                runn_win.glade.checkbutton_use_appind.set_active(self.use_appind)
                runn_win.user_active = True

    def on_checkbutton_autosave_toggled(self, checkbutton):
        """Autosave Toggled Handling"""
        self.autosave[0] = checkbutton.get_active()
        if not self.autosave[0] and self.autosave_timer_id != None: self.autosave_timer_stop()
        self.glade.spinbutton_autosave.set_sensitive(self.autosave[0])

    def on_checkbutton_reload_doc_last_toggled(self, checkbutton):
        """Reload Doc Last Toggled Handling"""
        self.reload_doc_last = checkbutton.get_active()

    def on_checkbutton_mod_time_sentinel_toggled(self, checkbutton):
        """Modification Time Sentinel Toggled Handling"""
        self.enable_mod_time_sentinel = checkbutton.get_active()
        if self.enable_mod_time_sentinel:
            if self.mod_time_sentinel_id == None:
                self.modification_time_sentinel_start()
        else:
            if self.mod_time_sentinel_id != None:
                self.modification_time_sentinel_stop()

    def on_checkbutton_line_wrap_toggled(self, checkbutton):
        """Lines Wrapping Toggled Handling"""
        self.line_wrapping = checkbutton.get_active()
        if self.line_wrapping: self.sourceview.set_wrap_mode(gtk.WRAP_WORD)
        else: self.sourceview.set_wrap_mode(gtk.WRAP_NONE)

    def on_checkbutton_spaces_tabs_toggled(self, checkbutton):
        """Insert Spaces Instead of Tabs Toggled Handling"""
        self.spaces_instead_tabs = checkbutton.get_active()
        self.sourceview.set_insert_spaces_instead_of_tabs(self.spaces_instead_tabs)

    def on_checkbutton_auto_indent_toggled(self, checkbutton):
        """Automatic Indentation Toggled Handling"""
        self.auto_indent = self.glade.checkbutton_auto_indent.get_active()

    def on_checkbutton_line_nums_toggled(self, checkbutton):
        """Show Line Num Toggled Handling"""
        self.show_line_numbers = checkbutton.get_active()
        self.sourceview.set_show_line_numbers(self.show_line_numbers)

    def on_radiobutton_nodes_startup_restore_toggled(self, checkbutton):
        """Restore Tree When Loaded Toggled"""
        if checkbutton.get_active(): self.rest_exp_coll = 0

    def on_radiobutton_nodes_startup_expand_toggled(self, checkbutton):
        """Expand Tree When Loaded Toggled"""
        if checkbutton.get_active(): self.rest_exp_coll = 1

    def on_radiobutton_nodes_startup_collapse_toggled(self, checkbutton):
        """Collapse Tree When Loaded Toggled"""
        if checkbutton.get_active(): self.rest_exp_coll = 2

    def on_checkbutton_newer_version_toggled(self, checkbutton):
        """Automatically Check for Newer Version Toggled"""
        self.check_version = checkbutton.get_active()

    def on_checkbutton_backup_before_saving_toggled(self, checkbutton):
        """Backup Before Save Toggled"""
        self.backup_copy = checkbutton.get_active()

    def on_checkbutton_autosave_on_quit_toggled(self, checkbutton):
        """Autosave on Quit Toggled"""
        self.autosave_on_quit = checkbutton.get_active()

    def on_checkbutton_spell_check_toggled(self, checkbutton):
        """Enable Spell Check Toggled"""
        if not self.user_active: return
        self.enable_spell_check = checkbutton.get_active()
        if self.enable_spell_check: self.spell_check_set_on()
        else: self.spell_check_set_off()
        self.glade.combobox_spell_check_lang.set_sensitive(self.enable_spell_check)

    def on_checkbutton_highlight_current_line_toggled(self, checkbutton):
        """Show White Spaces Toggled"""
        self.highl_curr_line = checkbutton.get_active()
        if self.user_active: support.dialog_info_after_restart(self.window)

    def on_checkbutton_show_white_spaces_toggled(self, checkbutton):
        """Show White Spaces Toggled"""
        self.show_white_spaces = checkbutton.get_active()
        if self.user_active: support.dialog_info_after_restart(self.window)

    def on_checkbutton_tree_right_side_toggled(self, checkbutton):
        """Display Tree on the Right Side Toggled"""
        if not self.user_active: return
        self.tree_right_side = checkbutton.get_active()
        tree_width = self.scrolledwindow_tree.get_allocation().width
        text_width = self.vbox_text.get_allocation().width
        self.hpaned.remove(self.scrolledwindow_tree)
        self.hpaned.remove(self.vbox_text)
        if self.tree_right_side:
            self.hpaned.add1(self.vbox_text)
            self.hpaned.add2(self.scrolledwindow_tree)
            self.hpaned.set_property('position', text_width)
        else:
            self.hpaned.add1(self.scrolledwindow_tree)
            self.hpaned.add2(self.vbox_text)
            self.hpaned.set_property('position', tree_width)

    def on_checkbutton_table_ins_from_file_toggled(self, checkbutton):
        """Import Table from CSV File Toggled"""
        self.glade.tablehandle_frame_table.set_sensitive(not checkbutton.get_active())
        self.glade.tablehandle_frame_col.set_sensitive(not checkbutton.get_active())

    def on_radiobutton_link_website_toggled(self, radiobutton):
        """Show/Hide Relative Frames"""
        if radiobutton.get_active(): self.link_type = cons.LINK_TYPE_WEBS
        self.link_type_changed_on_dialog(True)

    def on_radiobutton_link_node_anchor_toggled(self, checkbutton):
        """Show/Hide Relative Frames"""
        if checkbutton.get_active(): self.link_type = cons.LINK_TYPE_NODE
        self.link_type_changed_on_dialog(True)

    def on_radiobutton_link_file_toggled(self, radiobutton):
        """Show/Hide Relative Frames"""
        if radiobutton.get_active(): self.link_type = cons.LINK_TYPE_FILE
        self.link_type_changed_on_dialog(True)

    def on_radiobutton_link_folder_toggled(self, radiobutton):
        """Show/Hide Relative Frames"""
        if radiobutton.get_active(): self.link_type = cons.LINK_TYPE_FOLD
        self.link_type_changed_on_dialog(True)

    def link_type_changed_on_dialog(self, clear_when_user_active):
        """Change the Graphic of the Dialog according to the New Link Type"""
        self.glade.frame_link_website_url.set_sensitive(self.link_type == cons.LINK_TYPE_WEBS)
        self.glade.hbox_link_node_anchor.set_sensitive(self.link_type == cons.LINK_TYPE_NODE)
        self.glade.frame_link_filepath.set_sensitive(self.link_type in [cons.LINK_TYPE_FILE, cons.LINK_TYPE_FOLD])
        if clear_when_user_active and self.user_active:
            self.glade.link_website_entry.set_text("")
            self.glade.entry_file_to_link_to.set_text("")
            self.glade.link_anchor_entry.set_text("")

    def on_radiobutton_node_icon_cherry_toggled(self, radiobutton):
        """Change Variable Value Accordingly"""
        if radiobutton.get_active():
            self.nodes_icons = "c"
            if self.user_active: self.treeview_refresh(change_icon=True)

    def on_radiobutton_node_icon_bullet_toggled(self, radiobutton):
        """Change Variable Value Accordingly"""
        if radiobutton.get_active():
            self.nodes_icons = "b"
            if self.user_active: self.treeview_refresh(change_icon=True)

    def on_radiobutton_node_icon_none_toggled(self, radiobutton):
        """Change Variable Value Accordingly"""
        if radiobutton.get_active():
            self.nodes_icons = "n"
            if self.user_active: self.treeview_refresh(change_icon=True)

    def on_mouse_button_clicked_tree(self, widget, event):
        """Catches mouse buttons clicks"""
        if event.button == 3:
            self.menu_tree.popup(None, None, None, event.button, event.time)
        elif event.button == 2:
            path_at_click = self.treeview.get_path_at_pos(int(event.x), int(event.y))
            if path_at_click:
                if self.treeview.row_expanded(path_at_click[0]):
                    self.treeview.collapse_row(path_at_click[0])
                else: self.treeview.expand_row(path_at_click[0], False)
        elif event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
            self.toggle_tree_node_expanded_collapsed()

    def buffer_create(self, syntax_highlighting):
        """Returns a New Instantiated SourceBuffer"""
        if syntax_highlighting != cons.CUSTOM_COLORS_ID:
            sourcebuffer = gtksourceview2.Buffer()
            sourcebuffer.set_style_scheme(self.sourcestyleschememanager.get_scheme(self.style_scheme))
            self.set_sourcebuffer_syntax_highlight(sourcebuffer, syntax_highlighting)
            sourcebuffer.set_highlight_matching_brackets(True)
            return sourcebuffer
        else: return gtk.TextBuffer(self.tag_table)

    def combobox_prog_lang_init(self):
        """Init The Programming Languages Syntax Highlighting ComboBox"""
        self.prog_lang_liststore = gtk.ListStore(str, str)
        self.language_manager = gtksourceview2.LanguageManager()
        self.available_languages = self.language_manager.get_language_ids()
        if "def" in self.available_languages: self.available_languages.remove("def")
        for language_id in sorted(self.available_languages):
            self.prog_lang_liststore.append([self.language_manager.get_language(language_id).get_name(), language_id])
        combobox = self.glade.combobox_prog_lang_codebox
        combobox.set_model(self.prog_lang_liststore)
        cell = gtk.CellRendererText()
        combobox.pack_start(cell, True)
        combobox.add_attribute(cell, 'text', 0)
        if self.syntax_highlighting:
            combobox.set_active_iter(self.get_combobox_iter_from_value(self.prog_lang_liststore, 1, self.syntax_highlighting))
        else: combobox.set_active(0)

    def on_combobox_country_language_changed(self, combobox):
        """New Country Language Choosed"""
        new_iter = self.glade.combobox_country_language.get_active_iter()
        new_lang = self.country_lang_liststore[new_iter][0]
        if new_lang != self.country_lang:
            self.country_lang = new_lang
            support.dialog_info(_("The New Language will be Available Only After Restarting CherryTree"), self.window)
            lang_file_descriptor = file(cons.LANG_PATH, 'w')
            lang_file_descriptor.write(new_lang)
            lang_file_descriptor.close()

    def combobox_country_lang_init(self):
        """Init The Country Language ComboBox"""
        combobox = self.glade.combobox_country_language
        self.country_lang_liststore = gtk.ListStore(str)
        combobox.set_model(self.country_lang_liststore)
        cell = gtk.CellRendererText()
        combobox.pack_start(cell, True)
        combobox.add_attribute(cell, 'text', 0)
        for country_lang in cons.AVAILABLE_LANGS:
            self.country_lang_liststore.append([country_lang])
        combobox.set_active_iter(self.get_combobox_iter_from_value(self.country_lang_liststore, 0, self.country_lang))
        combobox.connect('changed', self.on_combobox_country_language_changed)
    
    def on_combobox_style_scheme_changed(self, combobox):
        """New Style Scheme Choosed"""
        new_iter = self.glade.combobox_style_scheme.get_active_iter()
        new_style = self.style_scheme_liststore[new_iter][0]
        if new_style != self.style_scheme:
            self.style_scheme = new_style
            support.dialog_info_after_restart(self.window)
    
    def combobox_style_scheme_init(self):
        """Init The Style Scheme ComboBox"""
        combobox = self.glade.combobox_style_scheme
        self.style_scheme_liststore = gtk.ListStore(str)
        combobox.set_model(self.style_scheme_liststore)
        cell = gtk.CellRendererText()
        combobox.pack_start(cell, True)
        combobox.add_attribute(cell, 'text', 0)
        style_schemes_list = []
        for style_scheme in sorted(self.sourcestyleschememanager.get_scheme_ids()):
            self.style_scheme_liststore.append([style_scheme])
            style_schemes_list.append(style_scheme)
        if not self.style_scheme in style_schemes_list: self.style_scheme = style_schemes_list[0]
        combobox.set_active_iter(self.get_combobox_iter_from_value(self.style_scheme_liststore, 0, self.style_scheme))
        combobox.connect('changed', self.on_combobox_style_scheme_changed)
    
    def on_combobox_spell_check_lang_changed(self, combobox):
        """New Spell Check Language Choosed"""
        if not self.user_active: return
        new_iter = self.glade.combobox_spell_check_lang.get_active_iter()
        new_lang_code = self.spell_check_lang_liststore[new_iter][0]
        if new_lang_code != self.spell_check_lang: self.spell_check_set_new_lang(new_lang_code)
    
    def combobox_spell_check_lang_init(self):
        """Init The Spell Check Language ComboBox"""
        combobox = self.glade.combobox_spell_check_lang
        self.spell_check_lang_liststore = gtk.ListStore(str)
        combobox.set_model(self.spell_check_lang_liststore)
        cell = gtk.CellRendererText()
        combobox.pack_start(cell, True)
        combobox.add_attribute(cell, 'text', 0)
        code_lang_list = []
        for code_lang in sorted(self.spell_check_get_languages()):
            self.spell_check_lang_liststore.append([code_lang])
            code_lang_list.append(code_lang)
        if not self.spell_check_lang in code_lang_list: self.spell_check_lang = code_lang_list[0]
        combobox.set_active_iter(self.get_combobox_iter_from_value(self.spell_check_lang_liststore, 0, self.spell_check_lang))
        combobox.connect('changed', self.on_combobox_spell_check_lang_changed)
    
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
        language_id = self.prog_lang_liststore[self.get_combobox_iter_from_value(self.prog_lang_liststore, 1, syntax_highlighting)][1]
        sourcebuffer.set_language(self.language_manager.get_language(language_id))
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

    def node_add(self, *args):
        """Add a node having common father with the selected node's"""
        ret_name, ret_syntax, ret_tags, ret_ro = self.dialog_nodeprop(_("New Node Properties"), syntax_highl=self.syntax_highlighting)
        if not ret_name: return
        self.update_window_save_needed()
        self.syntax_highlighting = ret_syntax
        father_iter = self.treestore.iter_parent(self.curr_tree_iter) if self.curr_tree_iter else None
        node_level = self.treestore.iter_depth(father_iter)+1 if father_iter else 0
        cherry = self.get_node_icon(node_level, self.syntax_highlighting)
        new_node_id = self.node_id_get()
        if self.curr_tree_iter != None:
            new_node_iter = self.treestore.insert_after(father_iter,
                self.curr_tree_iter,
                [cherry, ret_name, self.buffer_create(self.syntax_highlighting),
                 new_node_id, self.syntax_highlighting, 0, ret_tags, ret_ro])
        else:
            new_node_iter = self.treestore.append(father_iter,
                [cherry, ret_name, self.buffer_create(self.syntax_highlighting),
                 new_node_id, self.syntax_highlighting, 0, ret_tags, ret_ro])
        self.ctdb_handler.pending_new_db_node(new_node_id)
        self.nodes_sequences_fix(father_iter, False)
        self.nodes_names_dict[new_node_id] = ret_name
        new_node_path = self.treestore.get_path(new_node_iter)
        self.treeview.set_cursor(new_node_path)
        self.sourceview.grab_focus()

    def node_child_add(self, *args):
        """Add a node having as father the selected node"""
        if not self.is_there_selected_node_or_error(): return
        ret_name, ret_syntax, ret_tags, ret_ro = self.dialog_nodeprop(_("New Child Node Properties"), syntax_highl=self.syntax_highlighting)
        if not ret_name: return
        self.update_window_save_needed()
        self.syntax_highlighting = ret_syntax
        node_level = self.treestore.iter_depth(self.curr_tree_iter)+1 if self.curr_tree_iter else 0
        cherry = self.get_node_icon(node_level, self.syntax_highlighting)
        new_node_id = self.node_id_get()
        new_node_iter = self.treestore.append(self.curr_tree_iter,
            [cherry, ret_name, self.buffer_create(self.syntax_highlighting),
             new_node_id, self.syntax_highlighting, 0, ret_tags, ret_ro])
        self.ctdb_handler.pending_new_db_node(new_node_id)
        self.nodes_sequences_fix(self.curr_tree_iter, False)
        self.nodes_names_dict[new_node_id] = ret_name
        self.treeview_safe_set_cursor(new_node_iter)
        self.sourceview.grab_focus()

    def node_delete(self, *args):
        """Delete the Selected Node"""
        if not self.is_there_selected_node_or_error(): return
        warning_label = _("Are you sure to <b>Delete the node '%s'?</b>") % self.treestore[self.curr_tree_iter][1]
        if self.treestore.iter_children(self.curr_tree_iter) != None:
            warning_label += cons.CHAR_NEWLINE*2+_("The node <b>has Children, they will be Deleted too!</b>")
            warning_label += self.get_node_children_list(self.curr_tree_iter, 0)
        self.glade.label_node_delete.set_text(warning_label)
        self.glade.label_node_delete.set_use_markup(True)
        response = self.glade.nodedeletedialog.run()
        self.glade.nodedeletedialog.hide()
        if response != 1: return # the user did not confirm
        # next selected node will be previous sibling or next sibling or father or None
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

    def node_edit(self, *args):
        """Edit the Properties of the Selected Node"""
        if not self.is_there_selected_node_or_error(): return
        ret_name, ret_syntax, ret_tags, ret_ro = self.dialog_nodeprop(_("Node Properties"),
            name=self.treestore[self.curr_tree_iter][1],
            syntax_highl=self.treestore[self.curr_tree_iter][4],
            tags=self.treestore[self.curr_tree_iter][6],
            ro=self.treestore[self.curr_tree_iter][7])
        if not ret_name: return
        self.syntax_highlighting = ret_syntax
        if self.treestore[self.curr_tree_iter][4] == cons.CUSTOM_COLORS_ID and self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            if not support.dialog_question(_("Entering the Automatic Syntax Highlighting you will Lose all Custom Colors for This Node, Do you want to Continue?"), self.window):
                self.syntax_highlighting = cons.CUSTOM_COLORS_ID # STEP BACK (we stay in CUSTOM COLORS)
                return
            # SWITCH TextBuffer -> SourceBuffer
            self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting)
            self.curr_buffer = self.treestore[self.curr_tree_iter][2]
        elif self.treestore[self.curr_tree_iter][4] != cons.CUSTOM_COLORS_ID and self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            # SWITCH SourceBuffer -> TextBuffer
            self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting)
            self.curr_buffer = self.treestore[self.curr_tree_iter][2]
        self.treestore[self.curr_tree_iter][1] = ret_name
        self.treestore[self.curr_tree_iter][4] = self.syntax_highlighting
        self.treestore[self.curr_tree_iter][6] = ret_tags
        self.treestore[self.curr_tree_iter][7] = ret_ro
        self.treestore[self.curr_tree_iter][0] = self.get_node_icon(self.treestore.iter_depth(self.curr_tree_iter),
                                                                    self.syntax_highlighting)
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            self.set_sourcebuffer_syntax_highlight(self.curr_buffer, self.syntax_highlighting)
        self.sourceview.set_editable(not self.treestore[self.curr_tree_iter][7])
        self.update_selected_node_statusbar_info()
        self.update_node_name_header()
        self.update_window_save_needed("npro")
        self.sourceview.grab_focus()

    def get_node_children_list(self, father_tree_iter, level):
        """Return a string listing the node children"""
        node_children_list = ""
        node_children_list += cons.CHAR_NEWLINE + level*3*cons.CHAR_SPACE + cons.CHAR_LISTBUL + \
                              cons.CHAR_SPACE +self.treestore[father_tree_iter][1]
        tree_iter = self.treestore.iter_children(father_tree_iter)
        while tree_iter:
            node_children_list += self.get_node_children_list(tree_iter, level+1)
            tree_iter = self.treestore.iter_next(tree_iter)
        return node_children_list

    def sourceview_set_properties(self, tree_iter, syntax_highl):
        """Set sourceview properties according to current node"""
        if syntax_highl == cons.CUSTOM_COLORS_ID:
            self.treestore[tree_iter][2].connect('insert-text', self.on_text_insertion)
            self.treestore[tree_iter][2].connect('delete-range', self.on_text_removal)
            self.treestore[tree_iter][2].connect('mark-set', self.on_textbuffer_mark_set)
            self.sourceview.modify_font(pango.FontDescription(self.text_font))
            self.sourceview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.rt_def_bg))
            self.sourceview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.rt_def_fg))
            self.sourceview.set_draw_spaces(0)
            self.sourceview.set_highlight_current_line(False)
            if self.enable_spell_check: self.spell_check_reload_on_buffer()
        else:
            self.sourceview.modify_font(pango.FontDescription(self.code_font))
            if self.show_white_spaces: self.sourceview.set_draw_spaces(codeboxes.DRAW_SPACES_FLAGS)
            if self.highl_curr_line: self.sourceview.set_highlight_current_line(True)

    def switch_buffer_text_source(self, text_buffer, tree_iter, new_syntax_highl):
        """Switch TextBuffer -> SourceBuffer or SourceBuffer -> TextBuffer"""
        if self.user_active:
            self.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        node_text = text_buffer.get_text(*text_buffer.get_bounds())
        self.treestore[tree_iter][2] = self.buffer_create(new_syntax_highl)
        self.treestore[tree_iter][2].set_text(node_text)
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
        if self.curr_tree_iter:
            if self.user_active:
                self.nodes_cursor_pos[model[self.curr_tree_iter][3]] = self.curr_buffer.get_property(cons.STR_CURSOR_POSITION)
                #print "cursor_pos %s save for node %s" % (self.nodes_cursor_pos[model[self.curr_tree_iter][3]], model[self.curr_tree_iter][3])
            if self.curr_buffer.get_modified():
                self.file_update = True
                self.curr_buffer.set_modified(False)
                self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
        self.curr_tree_iter = new_iter
        self.curr_buffer = self.get_textbuffer_from_tree_iter(self.curr_tree_iter)
        self.sourceview.set_buffer(self.curr_buffer)
        self.syntax_highlighting = self.treestore[self.curr_tree_iter][4]
        self.curr_buffer.connect('modified-changed', self.on_modified_changed)
        self.sourceview_set_properties(self.curr_tree_iter, self.syntax_highlighting)
        self.sourceview.set_sensitive(True)
        self.sourceview.set_editable(not self.treestore[self.curr_tree_iter][7])
        self.update_node_name_header()
        self.state_machine.node_selected_changed(self.treestore[self.curr_tree_iter][3])
        self.objects_buffer_refresh()
        self.update_selected_node_statusbar_info()
        if self.user_active:
            if model[new_iter][3] in self.nodes_cursor_pos:
                already_visited = True
                cursor_pos = self.nodes_cursor_pos[model[new_iter][3]]
            else:
                already_visited = False
                cursor_pos = 0
            cursor_iter = self.curr_buffer.get_iter_at_offset(cursor_pos)
            if cursor_iter:
                #print "cursor_pos %s restore for node %s" % (cursor_pos, model[new_iter][3])
                self.curr_buffer.place_cursor(cursor_iter)
                self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
            if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
                if self.enable_spell_check: self.spell_check_set_on()
                if not already_visited: self.lists_handler.todo_lists_old_to_new_conversion(self.curr_buffer)

    def update_node_name_header(self):
        """Update Node Name Header"""
        node_hier_name = self.treestore[self.curr_tree_iter][1] if self.curr_tree_iter else ""
        self.header_node_name_eventbox.modify_bg(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.tt_def_bg))
        self.header_node_name_label.set_text(
            "<b><i><span foreground=\"" + self.tt_def_fg + "\" size=\"xx-large\">"+\
            cgi.escape(node_hier_name) + "</span></i></b>")
        self.header_node_name_label.set_use_markup(True)

    def get_textbuffer_from_tree_iter(self, tree_iter):
        """Returns the text buffer given the tree iter"""
        if not self.treestore[tree_iter][2]:
            # we are using db storage and the buffer was not created yet
            self.ctdb_handler.read_db_node_content(tree_iter, self.db)
        return self.treestore[tree_iter][2]

    def on_textbuffer_mark_set(self, text_buffer, text_iter, text_mark):
        """"""
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

    def update_window_save_needed(self, update_type=None, new_state_machine=False):
        """Window title preceeded by an asterix"""
        if not self.file_update:
            self.window_title_update(True)
            self.file_update = True
        if update_type:
            if update_type == "nbuf":
                if self.curr_tree_iter:
                    self.ctdb_handler.pending_edit_db_node_buff(self.treestore[self.curr_tree_iter][3])
            elif update_type == "npro":
                if self.curr_tree_iter:
                    self.ctdb_handler.pending_edit_db_node_prop(self.treestore[self.curr_tree_iter][3])
            elif update_type == "ndel":
                if self.curr_tree_iter:
                    self.ctdb_handler.pending_rm_db_node(self.treestore[self.curr_tree_iter][3])
            elif update_type == "book": self.ctdb_handler.pending_edit_db_bookmarks()
        if new_state_machine and self.curr_tree_iter:
            self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])

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

    def window_title_update(self, save_needed):
        """Update window title"""
        if save_needed:
            if self.file_name != "": self.window.set_title("*" + self.file_name + " - CherryTree %s" % cons.VERSION)
            else: self.window.set_title("*CherryTree")
        else:
            if self.file_name != "": self.window.set_title(self.file_name + " - CherryTree %s" % cons.VERSION)
            else: self.window.set_title("CherryTree %s" % cons.VERSION)

    def replace_again(self, *args):
        """Continue the previous replace (a_node/in_selected_node/in_all_nodes)"""
        self.find_handler.replace_again()

    def find_again(self, *args):
        """Continue the previous search (a_node/in_selected_node/in_all_nodes)"""
        self.search_replace_dict['idialog'] = False
        self.find_handler.find_again()

    def find_back(self, *args):
        """Continue the previous search (a_node/in_selected_node/in_all_nodes) but in Opposite Direction"""
        self.search_replace_dict['idialog'] = False
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
        self.find_handler.replace_in_all_nodes()

    def find_in_all_nodes(self, *args):
        """Search for a pattern in all the Tree Nodes"""
        if not self.is_tree_not_empty_or_error(): return
        self.find_handler.find_in_all_nodes()

    def replace_in_nodes_names(self, *args):
        """Replace the pattern between all the Node's Names"""
        if not self.is_tree_not_empty_or_error(): return
        self.find_handler.replace_in_nodes_names()

    def find_a_node(self, *args):
        """Search for a pattern between all the Node's Names"""
        if not self.is_tree_not_empty_or_error(): return
        self.find_handler.find_a_node()

    def find_allmatchesdialog_restore(self, *args):
        """Display the AllMatchesDialog Again"""
        if not self.find_handler.allmatchesdialog.get_property(cons.STR_VISIBLE):
            self.find_handler.allmatchesdialog.run()
            self.find_handler.allmatchesdialog.hide()

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
        new_node_id = self.state_machine.requested_previous_visited()
        if new_node_id:
            node_iter = self.get_tree_iter_from_node_id(new_node_id)
            if node_iter: self.treeview_safe_set_cursor(node_iter)
            else: self.go_back()
        self.go_bk_fw_click = False

    def go_forward(self, *args):
        """Go to the Next Visited Node"""
        self.go_bk_fw_click = True
        new_node_id = self.state_machine.requested_next_visited()
        if new_node_id:
            node_iter = self.get_tree_iter_from_node_id(new_node_id)
            if node_iter: self.treeview_safe_set_cursor(node_iter)
            else: self.go_forward()
        self.go_bk_fw_click = False

    def requested_step_back(self, *args):
        """Step Back for the Current Node, if Possible"""
        if self.curr_tree_iter == None: return
        if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            # TEXT BUFFER STATE MACHINE
            step_back = self.state_machine.requested_previous_state(self.treestore[self.curr_tree_iter][3])
            # step_back is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
            if step_back != None:
                if self.user_active:
                    self.user_active = False
                    user_active_restore = True
                else: user_active_restore = False
                self.xml_handler.dom_to_buffer(self.curr_buffer, step_back[0])
                pixbuf_table_vector = step_back[1]
                # pixbuf_table_vector is [ [ "pixbuf"/"table"/"codebox", [offset, pixbuf, alignment] ],... ]
                for element in pixbuf_table_vector:
                    if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(self.curr_buffer, element[1])
                    elif element[0] == "table": self.state_machine.load_embedded_table_element(self.curr_buffer, element[1])
                    elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(self.curr_buffer, element[1])
                self.sourceview.set_buffer(None)
                self.sourceview.set_buffer(self.curr_buffer)
                self.objects_buffer_refresh()
                self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(step_back[2]))
                self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
                if user_active_restore: self.user_active = True
                self.update_window_save_needed("nbuf")
        elif self.curr_buffer.can_undo():
            self.curr_buffer.undo()
            self.update_window_save_needed("nbuf")

    def requested_step_ahead(self, *args):
        """Step Ahead for the Current Node, if Possible"""
        if self.curr_tree_iter == None: return
        if self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            # TEXT BUFFER STATE MACHINE
            step_ahead = self.state_machine.requested_subsequent_state(self.treestore[self.curr_tree_iter][3])
            # step_ahead is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
            if step_ahead != None:
                if self.user_active:
                    self.user_active = False
                    user_active_restore = True
                else: user_active_restore = False
                self.xml_handler.dom_to_buffer(self.curr_buffer, step_ahead[0])
                pixbuf_table_vector = step_ahead[1]
                # pixbuf_table_vector is [ [ "pixbuf"/"table", [offset, pixbuf, alignment] ],... ]
                for element in pixbuf_table_vector:
                    if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(self.curr_buffer, element[1])
                    elif element[0] == "table": self.state_machine.load_embedded_table_element(self.curr_buffer, element[1])
                    elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(self.curr_buffer, element[1])
                self.sourceview.set_buffer(None)
                self.sourceview.set_buffer(self.curr_buffer)
                self.objects_buffer_refresh()
                self.curr_buffer.place_cursor(self.curr_buffer.get_iter_at_offset(step_ahead[2]))
                self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
                if user_active_restore: self.user_active = True
                self.update_window_save_needed("nbuf")
        elif self.curr_buffer.can_redo():
            self.curr_buffer.redo()
            self.update_window_save_needed("nbuf")

    def objects_buffer_refresh(self):
        """Buffer Refresh (Needed for Objects)"""
        if not self.curr_tree_iter: return
        refresh = self.state_machine.requested_current_state(self.treestore[self.curr_tree_iter][3])
        # refresh is [ [rich_text, pixbuf_table_vector, cursor_position],... ]
        pixbuf_table_vector = refresh[1]
        if len(pixbuf_table_vector) > 0:
            if self.user_active:
                self.user_active = False
                user_active_restore = True
            else: user_active_restore = False
            self.curr_buffer.set_text("")
            self.xml_handler.dom_to_buffer(self.curr_buffer, refresh[0])
            for element in pixbuf_table_vector:
                if element[0] == "pixbuf": self.state_machine.load_embedded_image_element(self.curr_buffer, element[1])
                elif element[0] == "table": self.state_machine.load_embedded_table_element(self.curr_buffer, element[1])
                elif element[0] == "codebox": self.state_machine.load_embedded_codebox_element(self.curr_buffer, element[1])
            self.curr_buffer.set_modified(False)
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
        if not self.is_there_selected_node_or_error(): return
        self.curr_buffer.insert_at_cursor(cons.CHAR_NEWLINE+self.h_rule+cons.CHAR_NEWLINE)

    def dialog_search(self, title, replace_on):
        """Opens the Search Dialog"""
        dialog = gtk.Dialog(title=title,
                            parent=self.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
        dialog.set_default_size(300, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        search_entry = gtk.Entry()
        search_entry.set_text(self.search_replace_dict['find'])
        search_frame = gtk.Frame(label="<b>"+_("Search for")+"</b>")
        search_frame.get_label_widget().set_use_markup(True)
        search_frame.set_shadow_type(gtk.SHADOW_NONE)
        search_frame.add(search_entry)
        if replace_on:
            replace_entry = gtk.Entry()
            replace_entry.set_text(self.search_replace_dict['replace'])
            replace_frame = gtk.Frame(label="<b>"+_("Replace with")+"</b>")
            replace_frame.get_label_widget().set_use_markup(True)
            replace_frame.set_shadow_type(gtk.SHADOW_NONE)
            replace_frame.add(replace_entry)
        opt_vbox = gtk.VBox()
        opt_vbox.set_spacing(1)
        four_1_hbox = gtk.HBox()
        four_1_hbox.set_homogeneous(True)
        four_2_hbox = gtk.HBox()
        four_2_hbox.set_homogeneous(True)
        bw_fw_hbox = gtk.HBox()
        bw_fw_hbox.set_homogeneous(True)
        three_hbox = gtk.HBox()
        three_hbox.set_homogeneous(True)
        three_vbox = gtk.VBox()
        match_case_checkbutton = gtk.CheckButton(label=_("Match Case"))
        match_case_checkbutton.set_active(self.search_replace_dict['match_case'])
        reg_exp_checkbutton = gtk.CheckButton(label=_("Regular Expression"))
        reg_exp_checkbutton.set_active(self.search_replace_dict['reg_exp'])
        whole_word_checkbutton = gtk.CheckButton(label=_("Whole Word"))
        whole_word_checkbutton.set_active(self.search_replace_dict['whole_word'])
        start_word_checkbutton = gtk.CheckButton(label=_("Start Word"))
        start_word_checkbutton.set_active(self.search_replace_dict['start_word'])
        fw_radiobutton = gtk.RadioButton(label=_("Forward"))
        fw_radiobutton.set_active(self.search_replace_dict['fw'])
        bw_radiobutton = gtk.RadioButton(label=_("Backward"))
        bw_radiobutton.set_group(fw_radiobutton)
        bw_radiobutton.set_active(not self.search_replace_dict['fw'])
        all_radiobutton = gtk.RadioButton(label=_("All, List Matches"))
        all_radiobutton.set_active(self.search_replace_dict['a_ff_fa'] == 0)
        first_from_radiobutton = gtk.RadioButton(label=_("First From Selection"))
        first_from_radiobutton.set_group(all_radiobutton)
        first_from_radiobutton.set_active(self.search_replace_dict['a_ff_fa'] == 1)
        first_all_radiobutton = gtk.RadioButton(label=_("First in All Range"))
        first_all_radiobutton.set_group(all_radiobutton)
        first_all_radiobutton.set_active(self.search_replace_dict['a_ff_fa'] == 2)
        iter_dialog_checkbutton = gtk.CheckButton(label=_("Show Iterated Find/Replace Dialog"))
        iter_dialog_checkbutton.set_active(self.search_replace_dict['idialog'])
        four_1_hbox.pack_start(match_case_checkbutton)
        four_1_hbox.pack_start(reg_exp_checkbutton)
        four_2_hbox.pack_start(whole_word_checkbutton)
        four_2_hbox.pack_start(start_word_checkbutton)
        bw_fw_hbox.pack_start(fw_radiobutton)
        bw_fw_hbox.pack_start(bw_radiobutton)
        three_hbox.pack_start(all_radiobutton)
        three_vbox.pack_start(first_from_radiobutton)
        three_vbox.pack_start(first_all_radiobutton)
        three_hbox.pack_start(three_vbox)
        opt_vbox.pack_start(four_1_hbox)
        opt_vbox.pack_start(four_2_hbox)
        opt_vbox.pack_start(gtk.HSeparator())
        opt_vbox.pack_start(bw_fw_hbox)
        opt_vbox.pack_start(gtk.HSeparator())
        opt_vbox.pack_start(three_hbox)
        opt_vbox.pack_start(gtk.HSeparator())
        opt_vbox.pack_start(iter_dialog_checkbutton)
        opt_frame = gtk.Frame(label="<b>"+_("Search options")+"</b>")
        opt_frame.get_label_widget().set_use_markup(True)
        opt_frame.set_shadow_type(gtk.SHADOW_NONE)
        opt_frame.add(opt_vbox)
        content_area = dialog.get_content_area()
        content_area.set_spacing(5)
        content_area.pack_start(search_frame)
        if replace_on: content_area.pack_start(replace_frame)
        content_area.pack_start(opt_frame)
        content_area.show_all()
        search_entry.grab_focus()
        def on_key_press_searchdialog(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if keyname == "Return":
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
        dialog.connect('key_press_event', on_key_press_searchdialog)
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            self.search_replace_dict['find'] = unicode(search_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            if replace_on:
                self.search_replace_dict['replace'] = unicode(replace_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            self.search_replace_dict['match_case'] = match_case_checkbutton.get_active()
            self.search_replace_dict['reg_exp'] = reg_exp_checkbutton.get_active()
            self.search_replace_dict['whole_word'] = whole_word_checkbutton.get_active()
            self.search_replace_dict['start_word'] = start_word_checkbutton.get_active()
            self.search_replace_dict['fw'] = fw_radiobutton.get_active()
            self.search_replace_dict['a_ff_fa'] = 0 if all_radiobutton.get_active() else 1 if first_from_radiobutton.get_active() else 2
            self.search_replace_dict['idialog'] = iter_dialog_checkbutton.get_active()
            return self.search_replace_dict['find']
        return None

    def dialog_nodeprop(self, title, name="", syntax_highl=cons.CUSTOM_COLORS_ID, tags="", ro=False):
        """Opens the Node Properties Dialog"""
        dialog = gtk.Dialog(title=title,
                            parent=self.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
        dialog.set_default_size(300, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        name_entry = gtk.Entry()
        name_entry.set_text(name)
        name_frame = gtk.Frame(label="<b>"+_("Node Name")+"</b>")
        name_frame.get_label_widget().set_use_markup(True)
        name_frame.set_shadow_type(gtk.SHADOW_NONE)
        name_frame.add(name_entry)
        radiobutton_rich_text = gtk.RadioButton(label=_("Rich Text"))
        radiobutton_auto_syntax_highl = gtk.RadioButton(label=_("Automatic Syntax Highlighting"))
        radiobutton_auto_syntax_highl.set_group(radiobutton_rich_text)
        combobox_prog_lang = gtk.ComboBox(model=self.prog_lang_liststore)
        cell = gtk.CellRendererText()
        combobox_prog_lang.pack_start(cell, True)
        combobox_prog_lang.add_attribute(cell, 'text', 0)
        combobox_prog_lang.set_active_iter(self.get_combobox_iter_from_value(self.prog_lang_liststore, 1, syntax_highl))
        if syntax_highl == cons.CUSTOM_COLORS_ID:
            radiobutton_rich_text.set_active(True)
            combobox_prog_lang.set_sensitive(False)
        else:
            radiobutton_auto_syntax_highl.set_active(True)
            combobox_prog_lang.set_active_iter(self.get_combobox_iter_from_value(self.prog_lang_liststore, 1, syntax_highl))
        type_vbox = gtk.VBox()
        type_vbox.pack_start(radiobutton_rich_text)
        type_vbox.pack_start(radiobutton_auto_syntax_highl)
        type_vbox.pack_start(combobox_prog_lang)
        type_frame = gtk.Frame(label="<b>"+_("Node Type")+"</b>")
        type_frame.get_label_widget().set_use_markup(True)
        type_frame.set_shadow_type(gtk.SHADOW_NONE)
        type_frame.add(type_vbox)
        tags_entry = gtk.Entry()
        tags_entry.set_text(tags)
        tags_frame = gtk.Frame(label="<b>"+_("Tags for Searching")+"</b>")
        tags_frame.get_label_widget().set_use_markup(True)
        tags_frame.set_shadow_type(gtk.SHADOW_NONE)
        tags_frame.add(tags_entry)
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
        def on_key_press_nodepropdialog(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if keyname == "Return":
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
        def on_radiobutton_rich_text_toggled(radiobutton):
            combobox_prog_lang.set_sensitive(not radiobutton.get_active())
        radiobutton_rich_text.connect("toggled", on_radiobutton_rich_text_toggled)
        dialog.connect('key_press_event', on_key_press_nodepropdialog)
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            ret_name = unicode(name_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            ret_syntax = cons.CUSTOM_COLORS_ID if radiobutton_rich_text.get_active() else self.prog_lang_liststore[combobox_prog_lang.get_active_iter()][1]
            ret_tags = unicode(tags_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            ret_ro = ro_checkbutton.get_active()
            return [ret_name, ret_syntax, ret_tags, ret_ro]
        return [None, None, None, None]

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
        if self.ui.get_widget("/ToolBar").get_property(cons.STR_VISIBLE): self.ui.get_widget("/ToolBar").hide()
        else: self.ui.get_widget("/ToolBar").show()

    def toggle_show_hide_tree(self, *args):
        """Toggle Show/Hide the Tree"""
        if self.scrolledwindow_tree.get_property(cons.STR_VISIBLE):
            self.scrolledwindow_tree.hide()
        else: self.scrolledwindow_tree.show()

    def toggle_show_hide_node_name_header(self, *args):
        """Toggle Show/Hide the Node Title Header"""
        if self.header_node_name_label.get_property(cons.STR_VISIBLE):
            self.header_node_name_label.hide()
        else: self.header_node_name_label.show()

    def quit_application(self, *args):
        """Just Hide or Quit the gtk main loop"""
        if self.systray:
            self.win_position = self.window.get_position()
            self.window.hide()
        else: self.quit_application_totally()

    def quit_application_totally(self, *args):
        """The process is Shut Down"""
        if not self.check_unsaved():
            self.really_quit = False # user pressed cancel
            return
        config.config_file_save(self)
        if "db" in dir(self) and self.db: self.db.close()
        for filepath_tmp in self.ctdb_handler.remove_at_quit_set: os.remove(filepath_tmp)
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
        if self.curr_tree_iter != None and (self.curr_buffer.get_modified() == True or self.file_update == True):
            if self.autosave_on_quit: response = 2
            else:
                response = self.glade.exitdialog.run()
                self.glade.exitdialog.hide()
            if response == 2: self.file_save() # button YES pressed or autosave ON
        else: response = 0 # no need to save
        if response == 6: return False # button CANCEL
        else: return True

    def on_key_press_exitdialog(self, widget, event):
        """Catches Exit Dialog key presses"""
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == "Return": self.glade.button_exitdialog_yes.clicked()
        elif keyname == "Escape": self.glade.button_exitdialog_canc.clicked()
        else: print keyname
        return True

    def dialog_about(self, *args):
        """Show the About Dialog and hide it when a button is pressed"""
        support.dialog_about(self)

    def anchor_handle(self, action):
        """Insert an Anchor"""
        if not self.node_sel_and_rich_text(): return
        iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        pixbuf = gtk.gdk.pixbuf_new_from_file(cons.ANCHOR_CHAR)
        pixbuf = pixbuf.scale_simple(self.anchor_size, self.anchor_size, gtk.gdk.INTERP_BILINEAR)
        self.anchor_edit_dialog(pixbuf, iter_insert)

    def anchor_edit(self, *args):
        """Edit an Anchor"""
        iter_insert = self.curr_buffer.get_iter_at_child_anchor(self.curr_anchor_anchor)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        self.anchor_edit_dialog(self.curr_anchor_anchor.pixbuf, iter_insert, iter_bound)

    def anchor_edit_dialog(self, pixbuf, iter_insert, iter_bound=None):
        """Anchor Edit Dialog"""
        if "anchor" in dir (pixbuf):
            self.glade.anchor_insert_edit_entry.set_text(pixbuf.anchor)
            self.glade.anchorhandledialog.set_title(_("Edit Anchor"))
        else:
            self.glade.anchor_insert_edit_entry.set_text("")
            self.glade.anchorhandledialog.set_title(_("Insert Anchor"))
        self.glade.anchor_insert_edit_entry.grab_focus()
        self.glade.anchor_enter_name_hbox.show()
        self.glade.scrolledwindow_anchors_list.hide()
        response = self.glade.anchorhandledialog.run()
        self.glade.anchorhandledialog.hide()
        if response != 1: return # the user aborted the operation
        anchor = self.glade.anchor_insert_edit_entry.get_text().strip()
        if anchor == "":
            support.dialog_error(_("The Anchor Name is Mandatory!"), self.window)
            return
        pixbuf.anchor = anchor
        if iter_bound != None: # only in case of modify
            image_justification = self.state_machine.get_iter_alignment(iter_insert)
            image_offset = iter_insert.get_offset()
            self.curr_buffer.delete(iter_insert, iter_bound)
            iter_insert = self.curr_buffer.get_iter_at_offset(image_offset)
        else: image_justification = None
        self.image_insert(iter_insert, pixbuf, image_justification)

    def toc_insert(self, *args):
        """Insert Table Of Contents"""
        if not self.is_there_selected_node_or_error(): return
        if not self.node_sel_and_rich_text(): return
        toc_type = support.dialog_selnode_selnodeandsub_alltree(self.window,
                                                                also_selection=False)[0]
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
        self.tables_handler.table_handle()

    def codebox_handle(self, *args):
        """Insert Code Box"""
        if not self.node_sel_and_rich_text(): return
        self.codeboxes_handler.codebox_handle()

    def on_radiobutton_rt_col_light_toggled(self, radiobutton):
        """Radiobutton Rich Text Color Light Toggled"""
        if not self.user_active or not radiobutton.get_active(): return
        self.glade.colorbutton_text_fg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_LIGHT_FG))
        self.glade.colorbutton_text_bg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_LIGHT_BG))
        self.glade.colorbutton_text_fg.set_sensitive(False)
        self.glade.colorbutton_text_bg.set_sensitive(False)
        self.on_colorbutton_text_fg_color_set(self.glade.colorbutton_text_fg)
        self.on_colorbutton_text_bg_color_set(self.glade.colorbutton_text_bg)

    def on_radiobutton_rt_col_dark_toggled(self, radiobutton):
        """Radiobutton Rich Text Color Dark Toggled"""
        if not self.user_active or not radiobutton.get_active(): return
        self.glade.colorbutton_text_fg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_DARK_FG))
        self.glade.colorbutton_text_bg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_DARK_BG))
        self.glade.colorbutton_text_fg.set_sensitive(False)
        self.glade.colorbutton_text_bg.set_sensitive(False)
        self.on_colorbutton_text_fg_color_set(self.glade.colorbutton_text_fg)
        self.on_colorbutton_text_bg_color_set(self.glade.colorbutton_text_bg)

    def on_radiobutton_rt_col_custom_toggled(self, radiobutton):
        """Radiobutton Rich Text Color Custom Toggled"""
        if not self.user_active or not radiobutton.get_active(): return
        self.glade.colorbutton_text_fg.set_sensitive(True)
        self.glade.colorbutton_text_bg.set_sensitive(True)

    def on_radiobutton_tt_col_light_toggled(self, radiobutton):
        """Radiobutton Tree Text Color Light Toggled"""
        if not self.user_active or not radiobutton.get_active(): return
        self.glade.colorbutton_tree_fg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_LIGHT_FG))
        self.glade.colorbutton_tree_bg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_LIGHT_BG))
        self.glade.colorbutton_tree_fg.set_sensitive(False)
        self.glade.colorbutton_tree_bg.set_sensitive(False)
        self.on_colorbutton_tree_fg_color_set(self.glade.colorbutton_tree_fg)
        self.on_colorbutton_tree_bg_color_set(self.glade.colorbutton_tree_bg)

    def on_radiobutton_tt_col_dark_toggled(self, radiobutton):
        """Radiobutton Tree Text Color Dark Toggled"""
        if not self.user_active or not radiobutton.get_active(): return
        self.glade.colorbutton_tree_fg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_DARK_FG))
        self.glade.colorbutton_tree_bg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_DARK_BG))
        self.glade.colorbutton_tree_fg.set_sensitive(False)
        self.glade.colorbutton_tree_bg.set_sensitive(False)
        self.on_colorbutton_tree_fg_color_set(self.glade.colorbutton_tree_fg)
        self.on_colorbutton_tree_bg_color_set(self.glade.colorbutton_tree_bg)

    def on_radiobutton_tt_col_custom_toggled(self, radiobutton):
        """Radiobutton Tree Text Color Custom Toggled"""
        if not self.user_active or not radiobutton.get_active(): return
        self.glade.colorbutton_tree_fg.set_sensitive(True)
        self.glade.colorbutton_tree_bg.set_sensitive(True)

    def on_radiobutton_codebox_pixels_toggled(self, radiobutton):
        """Radiobutton CodeBox Pixels/Percent Toggled"""
        if not self.user_active: return
        if radiobutton.get_active():
            self.glade.spinbutton_codebox_width.set_value(700)
        else: self.glade.spinbutton_codebox_width.set_value(100)

    def is_curr_node_not_syntax_highlighting_or_error(self):
        """Returns True if ok (no syntax highlighting) or False and prompts error dialog"""
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            support.dialog_warning(_("Automatic Syntax Highlighting Must be Disabled in order to Use This Feature"), self.window)
            return False
        return True

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
        table = gtk.Table(5, 2)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Text Nodes") + "</b>")
        table.attach(label, 0, 1, 0, 1)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Code Nodes") + "</b>")
        table.attach(label, 0, 1, 1, 2)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Images") + "</b>")
        table.attach(label, 0, 1, 2, 3)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of Tables") + "</b>")
        table.attach(label, 0, 1, 3, 4)
        label = gtk.Label()
        label.set_markup("<b>" + _("Number of CodeBoxes") + "</b>")
        table.attach(label, 0, 1, 4, 5)
        self.summary_nodes_text_num = 0
        self.summary_nodes_code_num = 0
        self.summary_images_num = 0
        self.summary_tables_num = 0
        self.summary_codeboxes_num = 0
        # full tree parsing
        tree_iter = self.treestore.get_iter_first()
        while tree_iter != None:
            self.tree_info_iter(tree_iter)
            tree_iter = self.treestore.iter_next(tree_iter)
        self.objects_buffer_refresh()
        label = gtk.Label("%s" % self.summary_nodes_text_num)
        table.attach(label, 1, 2, 0, 1)
        label = gtk.Label("%s" % self.summary_nodes_code_num)
        table.attach(label, 1, 2, 1, 2)
        label = gtk.Label("%s" % self.summary_images_num)
        table.attach(label, 1, 2, 2, 3)
        label = gtk.Label("%s" % self.summary_tables_num)
        table.attach(label, 1, 2, 3, 4)
        label = gtk.Label("%s" % self.summary_codeboxes_num)
        table.attach(label, 1, 2, 4, 5)
        content_area = dialog.get_content_area()
        content_area.pack_start(table)
        content_area.show_all()
        dialog.get_action_area().set_layout(gtk.BUTTONBOX_SPREAD)
        dialog.run()
        dialog.destroy()
        if self.enable_spell_check and self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.spell_check_set_on()

    def tree_info_iter(self, tree_iter):
        """Tree Summary Information Iteration"""
        curr_buffer = self.get_textbuffer_from_tree_iter(tree_iter)
        pixbuf_table_vector = self.state_machine.get_embedded_pixbufs_tables_codeboxes(curr_buffer)
        # pixbuf_table_vector is [ [ "pixbuf"/"table", [offset, pixbuf, alignment] ],... ]
        if self.treestore[tree_iter][4] == cons.CUSTOM_COLORS_ID: self.summary_nodes_text_num += 1
        else: self.summary_nodes_code_num += 1
        curr_node_images = 0
        curr_node_tables = 0
        curr_node_codeboxes = 0
        for element in pixbuf_table_vector:
            if element[0] == "pixbuf" and not "anchor" in dir(element[1][1]): curr_node_images += 1
            elif element[0] == "table": curr_node_tables += 1
            elif element[0] == "codebox": curr_node_codeboxes += 1
        if curr_node_images or curr_node_tables or curr_node_codeboxes:
            print "node with object(s):", self.treestore[tree_iter][1]
            self.summary_images_num += curr_node_images
            self.summary_tables_num += curr_node_tables
            self.summary_codeboxes_num += curr_node_codeboxes
        # iterate children
        tree_iter = self.treestore.iter_children(tree_iter)
        while tree_iter != None:
            self.tree_info_iter(tree_iter)
            tree_iter = self.treestore.iter_next(tree_iter)

    def image_handle(self, *args):
        """Insert/Edit Image"""
        if not self.node_sel_and_rich_text(): return
        iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        filename = support.dialog_file_select(curr_folder=self.pick_dir, parent=self.window)
        if filename == None: return
        self.pick_dir = os.path.dirname(filename)
        pixbuf = gtk.gdk.pixbuf_new_from_file(filename)
        self.image_edit_dialog(pixbuf, self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert()))

    def image_edit_dialog(self, pixbuf, insert_iter, iter_bound=None):
        """Insert/Edit Image Dialog"""
        self.original_pixbuf = pixbuf
        self.temp_image_width = pixbuf.get_width()
        self.temp_image_height = pixbuf.get_height()
        self.image_w_h_ration = float(self.temp_image_width)/self.temp_image_height
        self.image_load_into_dialog()
        response = self.glade.imageeditdialog.run()
        self.glade.imageeditdialog.hide()
        if response != 1: return # cancel was pressed
        if iter_bound != None: # only in case of modify
            image_justification = self.state_machine.get_iter_alignment(insert_iter)
            image_offset = insert_iter.get_offset()
            self.curr_buffer.delete(insert_iter, iter_bound)
            insert_iter = self.curr_buffer.get_iter_at_offset(image_offset)
        else: image_justification = None
        self.image_insert(insert_iter,
                          self.original_pixbuf.scale_simple(int(self.temp_image_width),
                                                            int(self.temp_image_height),
                                                            gtk.gdk.INTERP_BILINEAR),
                          image_justification)

    def image_insert(self, iter_insert, pixbuf, image_justification=None, text_buffer=None):
        if not pixbuf: return
        if not text_buffer: text_buffer = self.curr_buffer
        image_offset = iter_insert.get_offset()
        anchor = text_buffer.create_child_anchor(iter_insert)
        anchor.pixbuf = pixbuf
        anchor.eventbox = gtk.EventBox()
        anchor.eventbox.set_visible_window(False)
        if "anchor" in dir(pixbuf):
            anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_anchor, anchor)
            anchor.eventbox.set_tooltip_text(pixbuf.anchor)
        else:
            anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_image, anchor)
        anchor.image = gtk.Image()
        anchor.eventbox.add(anchor.image)
        anchor.image.set_from_pixbuf(anchor.pixbuf)
        self.sourceview.add_child_at_anchor(anchor.eventbox, anchor)
        anchor.eventbox.show_all()
        if image_justification:
            text_iter = text_buffer.get_iter_at_offset(image_offset)
            self.state_machine.apply_object_justification(text_iter, image_justification, text_buffer)
        elif self.user_active:
            # if I apply a justification, the state is already updated
            self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])

    def image_edit(self, *args):
        """Edit the selected Image"""
        iter_insert = self.curr_buffer.get_iter_at_child_anchor(self.curr_image_anchor)
        iter_bound = iter_insert.copy()
        iter_bound.forward_char()
        self.image_edit_dialog(self.curr_image_anchor.pixbuf, iter_insert, iter_bound)

    def image_save(self, *args):
        """Save to Disk the selected Image"""
        filename = support.dialog_file_save_as(curr_folder=self.pick_dir,
                                               filter_pattern="*.png",
                                               filter_name=_("PNG Image"),
                                               parent=self.window)
        if filename == None: return
        if len(filename) < 4 or filename[-4:] != ".png": filename += ".png"
        self.pick_dir = os.path.dirname(filename)
        try: self.curr_image_anchor.pixbuf.save(filename, "png")
        except: support.dialog_error(_("Write to %s Failed") % filename, self.window)

    def object_set_selection(self, anchor):
        """Put Selection Upon the Image"""
        iter_image = self.curr_buffer.get_iter_at_child_anchor(anchor)
        iter_bound = iter_image.copy()
        iter_bound.forward_char()
        self.curr_buffer.select_range(iter_image, iter_bound)

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

    def on_mouse_button_clicked_image(self, widget, event, anchor):
        """Catches mouse buttons clicks upon images"""
        self.curr_image_anchor = anchor
        self.object_set_selection(self.curr_image_anchor)
        if event.button == 3:
            self.ui.get_widget("/ImageMenu").popup(None, None, None, event.button, event.time)
        elif event.type == gtk.gdk._2BUTTON_PRESS: self.image_edit()
        return True # do not propagate the event

    def on_mouse_button_clicked_anchor(self, widget, event, anchor):
        """Catches mouse buttons clicks upon images"""
        self.curr_anchor_anchor = anchor
        self.object_set_selection(self.curr_anchor_anchor)
        if event.button == 3:
            self.ui.get_widget("/AnchorMenu").popup(None, None, None, event.button, event.time)
        elif event.type == gtk.gdk._2BUTTON_PRESS: self.anchor_edit()
        return True # do not propagate the event

    def image_load_into_dialog(self):
        """Load/Reload the Image Under Editing"""
        if self.user_active:
            self.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        self.glade.spinbutton_image_width.set_value(self.temp_image_width)
        self.glade.spinbutton_image_height.set_value(self.temp_image_height)
        if self.temp_image_width <= 900 and self.temp_image_height <= 600:
            # original size into the dialog
            pixbuf = self.original_pixbuf.scale_simple(int(self.temp_image_width),
                                                       int(self.temp_image_height),
                                                       gtk.gdk.INTERP_BILINEAR)
        else:
            # reduced size visible into the dialog
            if self.temp_image_width > 900:
                temp_image_width = 900
                temp_image_height = temp_image_width / self.image_w_h_ration
            else:
                temp_image_height = 600
                temp_image_width = temp_image_height * self.image_w_h_ration
            pixbuf = self.original_pixbuf.scale_simple(int(temp_image_width),
                                                       int(temp_image_height),
                                                       gtk.gdk.INTERP_BILINEAR)
        self.glade.image_under_editing.set_from_pixbuf(pixbuf)
        if user_active_restore: self.user_active = True

    def on_button_rotate_90_cw_clicked(self, *args):
        """Image Edit - Rotate 90 ClockWise"""
        self.original_pixbuf = self.original_pixbuf.rotate_simple(270)
        self.image_w_h_ration = 1/self.image_w_h_ration
        new_width = self.temp_image_height # new width is the former height and vice versa
        self.temp_image_height = self.temp_image_width
        self.temp_image_width = new_width
        self.image_load_into_dialog()

    def on_button_rotate_90_ccw_clicked(self, *args):
        """Image Edit - Rotate 90 CounterClockWise"""
        self.original_pixbuf = self.original_pixbuf.rotate_simple(90)
        self.image_w_h_ration = 1/self.image_w_h_ration
        new_width = self.temp_image_height # new width is the former height and vice versa
        self.temp_image_height = self.temp_image_width
        self.temp_image_width = new_width
        self.image_load_into_dialog()

    def on_spinbutton_image_width_value_changed(self, spinbutton):
        """Image Edit - Width Change Handling"""
        if self.user_active:
            self.temp_image_width = self.glade.spinbutton_image_width.get_value()
            self.temp_image_height = self.temp_image_width / self.image_w_h_ration
            self.image_load_into_dialog()

    def on_spinbutton_image_height_value_changed(self, spinbutton):
        """Image Edit - Height Change Handling"""
        if self.user_active:
            self.temp_image_height = self.glade.spinbutton_image_height.get_value()
            self.temp_image_width = self.temp_image_height * self.image_w_h_ration
            self.image_load_into_dialog()

    def on_spinbutton_anchor_size_value_changed(self, spinbutton):
        """Anchor Size Handling"""
        if not self.user_active: return
        self.anchor_size = int(self.glade.spinbutton_anchor_size.get_value())

    def strip_trailing_spaces(self, *args):
        """Remove trailing spaces/tabs"""
        support.strip_trailing_spaces(self.curr_buffer)

    def apply_tag_foreground(self, *args):
        """The Foreground Color Chooser Button was Pressed"""
        self.apply_tag(cons.TAG_FOREGROUND)

    def apply_tag_background(self, *args):
        """The Background Color Chooser Button was Pressed"""
        self.apply_tag(cons.TAG_BACKGROUND)

    def apply_tag_link(self, *args):
        """The Link Insert Button was Pressed"""
        self.apply_tag(cons.TAG_LINK)

    def apply_tag_bold(self, *args):
        """The Bold Button was Pressed"""
        self.apply_tag(cons.TAG_WEIGHT, cons.TAG_PROP_HEAVY)

    def apply_tag_italic(self, *args):
        """The Italic Button was Pressed"""
        self.apply_tag(cons.TAG_STYLE, cons.TAG_PROP_ITALIC)

    def apply_tag_underline(self, *args):
        """The Underline Button was Pressed"""
        self.apply_tag(cons.TAG_UNDERLINE, cons.TAG_PROP_SINGLE)

    def apply_tag_strikethrough(self, *args):
        """The Strikethrough Button was Pressed"""
        self.apply_tag(cons.TAG_STRIKETHROUGH, cons.TAG_PROP_TRUE)

    def apply_tag_small(self, *args):
        """The Small Button was Pressed"""
        self.apply_tag(cons.TAG_SCALE, "small")

    def apply_tag_subscript(self, *args):
        """The Subscript Button was Pressed"""
        self.apply_tag(cons.TAG_SCALE, "sub")

    def apply_tag_superscript(self, *args):
        """The Superscript Button was Pressed"""
        self.apply_tag(cons.TAG_SCALE, "sup")

    def apply_tag_monospace(self, *args):
        """The Monospace Button was Pressed"""
        self.apply_tag(cons.TAG_FAMILY, "monospace")

    def apply_tag_h1(self, *args):
        """The H1 Button was Pressed"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_H1, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_h2(self, *args):
        """The H2 Button was Pressed"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_H2, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_h3(self, *args):
        """The H3 Button was Pressed"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        self.apply_tag(cons.TAG_SCALE, cons.TAG_PROP_H3, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_justify_right(self, *args):
        """The Justify Right Button was Pressed"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        self.apply_tag(cons.TAG_JUSTIFICATION, cons.TAG_PROP_RIGHT, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_justify_left(self, *args):
        """The Justify Left Button was Pressed"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        self.apply_tag(cons.TAG_JUSTIFICATION, cons.TAG_PROP_LEFT, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_justify_center(self, *args):
        """The Justify Center Button was Pressed"""
        iter_start, iter_end = self.lists_handler.get_paragraph_iters()
        self.apply_tag(cons.TAG_JUSTIFICATION, cons.TAG_PROP_CENTER, iter_sel_start=iter_start, iter_sel_end=iter_end)

    def apply_tag_try_automatic_bounds(self):
        """Try to Select a Word Forward/Backward the Cursor"""
        iter_start = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        iter_end = iter_start.copy()
        end_moved = False
        while iter_end != None:
            char = iter_end.get_char()
            match = re.match('[^\s^$]', char, re.UNICODE)
            if not match: break # we got it
            elif not iter_end.forward_char(): break # we reached the buffer end
            end_moved = True
        if not end_moved: iter_start.backward_char() # we could be at the end of a word
        while iter_start != None:
            char = iter_start.get_char()
            match = re.match('[^\s^$]', char, re.UNICODE)
            if not match: # we got it
                iter_start.forward_char() # step forward to the beginning of the word
                break
            elif not iter_start.backward_char(): break # we reached the buffer start
        if iter_start.equal(iter_end): return False
        else:
            self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_end)
            self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), iter_start)
            return True

    def list_bulleted_handler(self, *args):
        """Handler of the Bulleted List"""
        if self.is_curr_node_not_syntax_highlighting_or_error():
            self.lists_handler.list_bulleted_handler()

    def list_numbered_handler(self, *args):
        """Handler of the Numbered List"""
        if self.is_curr_node_not_syntax_highlighting_or_error():
            self.lists_handler.list_numbered_handler()

    def list_todo_handler(self, *args):
        """Handler of the ToDo List"""
        if self.is_curr_node_not_syntax_highlighting_or_error():
            self.lists_handler.list_todo_handler()

    def apply_tag_latest(self, *args):
        """The Iterate Tagging Button was Pressed"""
        if self.latest_tag[0] == "": support.dialog_warning(_("No Previous Text Format Was Performed During This Session"), self.window)
        else: self.apply_tag(*self.latest_tag)

    def apply_tag(self, tag_property, property_value=None, iter_sel_start=None, iter_sel_end=None, text_buffer=None):
        """Apply a tag"""
        if self.user_active and not self.is_curr_node_not_syntax_highlighting_or_error(): return
        if not text_buffer: text_buffer = self.curr_buffer
        if iter_sel_start == None and iter_sel_end == None:
            if tag_property != cons.TAG_JUSTIFICATION:
                if not self.is_there_selected_node_or_error(): return
                if tag_property == cons.TAG_LINK: link_node_id = None
                if not text_buffer.get_has_selection():
                    if tag_property != cons.TAG_LINK:
                        if not self.apply_tag_try_automatic_bounds():
                            support.dialog_warning(_("No Text is Selected"), self.window)
                            return
                    else:
                        tag_property_value = self.link_check_around_cursor()
                        if tag_property_value == "":
                            if not self.apply_tag_try_automatic_bounds():
                                support.dialog_warning(_("No Text is Selected"), self.window)
                                return
                            self.glade.link_website_entry.set_text("")
                            self.glade.entry_file_to_link_to.set_text("")
                            self.glade.link_anchor_entry.set_text("")
                            self.link_type = cons.LINK_TYPE_WEBS # default value
                        else:
                            vector = tag_property_value.split()
                            self.link_type = vector[0]
                            if self.link_type == cons.LINK_TYPE_WEBS:
                                self.glade.link_website_entry.set_text(vector[1])
                            elif self.link_type in [cons.LINK_TYPE_FILE, cons.LINK_TYPE_FOLD]:
                                self.glade.entry_file_to_link_to.set_text(base64.b64decode(vector[1]))
                            elif self.link_type == cons.LINK_TYPE_NODE:
                                link_node_id = long(vector[1])
                                if len(vector) >= 3:
                                    if len(vector) == 3: anchor_name = vector[2]
                                    else: anchor_name = tag_property_value[len(vector[0]) + len(vector[1]) + 2:]
                                    self.glade.link_anchor_entry.set_text(anchor_name)
                            else:
                                support.dialog_error("Tag Name Not Recognized! (%s)" % self.link_type, self.window)
                                self.link_type = cons.LINK_TYPE_WEBS
                                return
                            if self.user_active:
                                self.user_active = False
                                user_active_restore = True
                            else: user_active_restore = False
                            self.glade.radiobutton_link_website.set_active(self.link_type == cons.LINK_TYPE_WEBS)
                            self.glade.radiobutton_link_node_anchor.set_active(self.link_type == cons.LINK_TYPE_NODE)
                            self.glade.radiobutton_link_file.set_active(self.link_type == cons.LINK_TYPE_FILE)
                            self.glade.radiobutton_link_folder.set_active(self.link_type == cons.LINK_TYPE_FOLD)
                            if user_active_restore: self.user_active = True
                iter_sel_start, iter_sel_end = text_buffer.get_selection_bounds()
            else:
                support.dialog_warning(_("The Cursor is Not into a Paragraph"), self.window)
                return
        if property_value == None:
            if tag_property == cons.TAG_LINK:
                if support.get_next_chars_from_iter_are(iter_sel_start, 7, "http://")\
                or support.get_next_chars_from_iter_are(iter_sel_start, 8, "https://")\
                or support.get_next_chars_from_iter_are(iter_sel_start, 4, "www."):
                    self.link_type = cons.LINK_TYPE_WEBS
                    self.glade.link_website_entry.set_text(text_buffer.get_text(iter_sel_start, iter_sel_end))
                self.node_choose_view_exist_or_create(link_node_id)
                self.glade.choosenodedialog.set_title(_("Insert/Edit a Link"))
                self.glade.link_dialog_top_vbox.show()
                self.glade.frame_link_anchor.show()
                self.link_type_changed_on_dialog(False)
                response = self.glade.choosenodedialog.run()
                self.glade.choosenodedialog.hide()
                if response != 1: return # the user aborted the operation
                if self.link_type == cons.LINK_TYPE_WEBS:
                    link_url = self.glade.link_website_entry.get_text().strip()
                    if link_url:
                        if len(link_url) < 8\
                        or (link_url[0:7] != "http://" and link_url[0:8] != "https://"):
                            link_url = "http://" + link_url
                        property_value = cons.LINK_TYPE_WEBS + cons.CHAR_SPACE + link_url
                    else: property_value = ""
                elif self.link_type in [cons.LINK_TYPE_FILE, cons.LINK_TYPE_FOLD]:
                    link_uri = self.glade.entry_file_to_link_to.get_text().strip()
                    if link_uri:
                        link_uri = base64.b64encode(link_uri)
                        property_value = self.link_type + cons.CHAR_SPACE + link_uri
                    else: property_value = ""
                elif self.link_type == cons.LINK_TYPE_NODE:
                    model, tree_iter = self.treeviewselection_2.get_selected()
                    if tree_iter:
                        link_anchor = self.glade.link_anchor_entry.get_text().strip()
                        property_value = cons.LINK_TYPE_NODE + cons.CHAR_SPACE + str(self.treestore[tree_iter][3])
                        if link_anchor: property_value += cons.CHAR_SPACE + link_anchor
                    else: property_value = ""
            else:
                dialog = gtk.ColorSelectionDialog(_("Pick a Color"))
                dialog.set_transient_for(self.window)
                dialog.set_property("modal", True)
                dialog.set_property("destroy-with-parent", True)
                dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
                gtk_settings = gtk.settings_get_default()
                gtk_settings.set_property("gtk-color-palette", ":".join(self.palette_list))
                colorselection = dialog.get_color_selection()
                colorselection.set_has_palette(True)
                if tag_property[0] == 'f':
                    if self.curr_colors['f']: colorselection.set_current_color(self.curr_colors['f'])
                elif tag_property[0] == 'b':
                    if self.curr_colors['b']: colorselection.set_current_color(self.curr_colors['b'])
                else: print "ERROR bad tag_property"
                response = dialog.run()
                dialog.hide()
                if response != gtk.RESPONSE_OK: return # cancel was clicked
                self.curr_colors[tag_property[0]] = colorselection.get_current_color()
                property_value = self.curr_colors[tag_property[0]].to_string()
                color_str_hex8 = "#" + self.html_handler.rgb_to_24(property_value[1:])
                if color_str_hex8 in self.palette_list:
                    self.palette_list.remove(color_str_hex8)
                else:
                    self.palette_list.pop()
                self.palette_list.insert(0, color_str_hex8)
        if tag_property != cons.TAG_LINK:
            self.latest_tag = [tag_property, property_value]
        curr_tags = iter_sel_start.get_tags()
        # if there's already a tag about this property, we remove it before apply the new one
        for curr_tag in curr_tags:
            tag_name = curr_tag.get_property("name")
            if not tag_name: continue
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
                if property_value == tag_name[6:]: return # just tag removal
            elif tag_property == cons.TAG_JUSTIFICATION and tag_name[0:14] == "justification_":
                text_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
            elif (tag_property == cons.TAG_FOREGROUND and tag_name[0:11] == "foreground_")\
            or (tag_property == cons.TAG_BACKGROUND and tag_name[0:11] == "background_")\
            or (tag_property == cons.TAG_LINK and tag_name[0:5] == "link_"):
                text_buffer.remove_tag(curr_tag, iter_sel_start, iter_sel_end)
        if property_value:
            text_buffer.apply_tag_by_name(self.apply_tag_exist_or_create(tag_property, property_value),
                                          iter_sel_start, iter_sel_end)
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
            elif property_value == "small": tag.set_property(tag_property, pango.SCALE_X_SMALL)
            elif property_value == cons.TAG_PROP_H1: tag.set_property(tag_property, pango.SCALE_XX_LARGE)
            elif property_value == cons.TAG_PROP_H2: tag.set_property(tag_property, pango.SCALE_X_LARGE)
            elif property_value == cons.TAG_PROP_H3: tag.set_property(tag_property, pango.SCALE_LARGE)
            elif property_value == cons.TAG_PROP_ITALIC: tag.set_property(tag_property, pango.STYLE_ITALIC)
            elif property_value == cons.TAG_PROP_SINGLE: tag.set_property(tag_property, pango.UNDERLINE_SINGLE)
            elif property_value == cons.TAG_PROP_TRUE: tag.set_property(tag_property, True)
            elif property_value == cons.TAG_PROP_LEFT: tag.set_property(tag_property, gtk.JUSTIFY_LEFT)
            elif property_value == cons.TAG_PROP_RIGHT: tag.set_property(tag_property, gtk.JUSTIFY_RIGHT)
            elif property_value == cons.TAG_PROP_CENTER: tag.set_property(tag_property, gtk.JUSTIFY_CENTER)
            elif property_value == "sub":
                tag.set_property(cons.TAG_SCALE, pango.SCALE_X_SMALL)
                rise = pango.FontDescription(self.text_font).get_size() / -4
                tag.set_property("rise", rise)
            elif property_value == "sup":
                tag.set_property(cons.TAG_SCALE, pango.SCALE_X_SMALL)
                rise = pango.FontDescription(self.text_font).get_size() / 2
                tag.set_property("rise", rise)
            elif property_value[0:4] == cons.LINK_TYPE_WEBS:
                tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, "#00004444ffff")
            elif property_value[0:4] == cons.LINK_TYPE_NODE:
                tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, "#071c838e071c")
            elif property_value[0:4] == cons.LINK_TYPE_FILE:
                tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, "#8b8b69691414")
            elif property_value[0:4] == cons.LINK_TYPE_FOLD:
                tag.set_property(cons.TAG_UNDERLINE, pango.UNDERLINE_SINGLE)
                tag.set_property(cons.TAG_FOREGROUND, "#7f7f7f7f7f7f")
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

    def spell_check_notify_new_lang(self, new_lang):
        """Receive New Lang from PyGtkSpellCheck"""
        self.spell_check_lang = new_lang
        self.user_active = False
        self.glade.combobox_spell_check_lang.set_active_iter(self.get_combobox_iter_from_value(self.spell_check_lang_liststore, 0, self.spell_check_lang))
        self.user_active = True

    def spell_check_set_on(self):
        """Enable Spell Check"""
        if not "spellchecker" in dir(self):
            self.spellchecker = pgsc_spellcheck.SpellChecker(self.sourceview, self, self.spell_check_lang)
            self.combobox_spell_check_lang_init()
        else:
            self.spellchecker.enable()
            self.spell_check_reload_on_buffer()

    def spell_check_set_off(self):
        """Disable Spell Check"""
        self.spellchecker.disable()

    def spell_check_reload_on_buffer(self):
        """Reload Spell Checker on curr Buffer"""
        self.spellchecker.buffer_initialize()

    def spell_check_get_languages(self):
        """Get Installed Dictionaries for Spell Check"""
        return [code for code, name in self.spellchecker.languages]

    def link_check_around_cursor(self):
        """Check if the cursor is on a link, in this case select the link and return the tag_property_value"""
        iter = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        tags = iter.get_tags()
        for tag in tags:
            tag_name = tag.get_property("name")
            if tag_name and tag_name[0:4] == cons.TAG_LINK: break
        else: return ""
        iter_end = iter.copy()
        if not iter_end.forward_to_tag_toggle(tag): return ""
        if not iter.backward_to_tag_toggle(tag): return ""
        self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_end)
        self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), iter)
        return tag_name[5:]

    def link_clicked(self, tag_property_value):
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
            filepath = base64.b64decode(vector[1])
            if not os.path.isfile(filepath):
                support.dialog_error(_("The File Link '%s' is Not Valid") % filepath, self.window)
                return
            if self.filelink_custom_action[0]:
                if cons.IS_WIN_OS: filepath = cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE
                else: filepath = re.escape(filepath)
                subprocess.call(self.filelink_custom_action[1] % filepath, shell=True)
            else:
                if cons.IS_WIN_OS:
                    try: os.startfile(filepath)
                    except: os.startfile(os.path.dirname(filepath))
                else: subprocess.call(config.LINK_CUSTOM_ACTION_DEFAULT_FILE % re.escape(filepath), shell=True)
        elif vector[0] == cons.LINK_TYPE_FOLD:
            # link to folder
            filepath = base64.b64decode(vector[1])
            if not os.path.isdir(filepath):
                support.dialog_error(_("The Folder Link '%s' is Not Valid") % filepath, self.window)
                return
            if self.folderlink_custom_action[0]:
                if cons.IS_WIN_OS: filepath = cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE
                else: filepath = re.escape(filepath)
                subprocess.call(self.folderlink_custom_action[1] % filepath, shell=True)
            else:
                if cons.IS_WIN_OS: os.startfile(filepath)
                else: subprocess.call(config.LINK_CUSTOM_ACTION_DEFAULT_FILE % re.escape(filepath), shell=True)
        elif vector[0] == cons.LINK_TYPE_NODE:
            # link to a tree node
            tree_iter = self.get_tree_iter_from_node_id(long(vector[1]))
            if tree_iter == None:
                support.dialog_error(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)") % vector[1], self.window)
                return
            self.treeview_safe_set_cursor(tree_iter)
            self.sourceview.grab_focus()
            self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(None)
            self.sourceview.set_tooltip_text(None)
            if len(vector) >= 3:
                if len(vector) == 3: anchor_name = vector[2]
                else: anchor_name = tag_property_value[len(vector[0]) + len(vector[1]) + 2:]
                iter_anchor = self.link_seek_for_anchor(anchor_name)
                if iter_anchor == None: support.dialog_warning(_("No anchor named '%s' found") % anchor_name, self.window)
                else:
                    self.curr_buffer.place_cursor(iter_anchor)
                    self.sourceview.scroll_to_mark(self.curr_buffer.get_insert(), 0.3)
        else: support.dialog_error("Tag Name Not Recognized! (%s)" % vector[0], self.window)

    def on_button_browse_for_file_to_link_to_clicked(self, *args):
        """The Button to browse for a file path on the links dialog was pressed"""
        if self.link_type == cons.LINK_TYPE_FILE:
            filepath = support.dialog_file_select(curr_folder=self.pick_dir, parent=self.window)
        else: filepath = support.dialog_folder_select(curr_folder=self.pick_dir, parent=self.window)
        if filepath == None: return
        if self.link_type == cons.LINK_TYPE_FILE: self.pick_dir = os.path.dirname(filepath)
        self.glade.entry_file_to_link_to.set_text(filepath)

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

    def on_colorbutton_text_fg_color_set(self, colorbutton):
        """ColorButton Rich Text FG Set"""
        if not self.user_active: return
        self.rt_def_fg = "#" + self.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        if self.curr_tree_iter and self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.sourceview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.rt_def_fg))

    def on_colorbutton_text_bg_color_set(self, colorbutton):
        """ColorButton Rich Text BG Set"""
        if not self.user_active: return
        self.rt_def_bg = "#" + self.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        if self.curr_tree_iter and self.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            self.sourceview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.rt_def_bg))

    def on_colorbutton_tree_fg_color_set(self, colorbutton):
        """ColorButton Rich Text FG Set"""
        if not self.user_active: return
        self.tt_def_fg = "#" + self.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        self.treeview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.tt_def_fg))
        if self.curr_tree_iter:
            self.update_node_name_header()

    def on_colorbutton_tree_bg_color_set(self, colorbutton):
        """ColorButton Rich Text FG Set"""
        if not self.user_active: return
        self.tt_def_bg = "#" + self.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        self.treeview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.tt_def_bg))
        if self.curr_tree_iter:
            self.update_node_name_header()

    def on_browse_anchors_button_clicked(self, *args):
        """Browse for Existing Anchors on the Selected Node"""
        self.anchors_liststore_exist_or_create()
        self.anchors_liststore.clear()
        model, tree_iter = self.treeviewselection_2.get_selected()
        curr_iter = self.treestore[tree_iter][2].get_start_iter()
        while 1:
            anchor = curr_iter.get_child_anchor()
            if anchor != None:
                if "pixbuf" in dir(anchor) and "anchor" in dir(anchor.pixbuf):
                    self.anchors_liststore.append([anchor.pixbuf.anchor])
            if not curr_iter.forward_char(): break
        anchor_first_iter = self.anchors_liststore.get_iter_first()
        if anchor_first_iter == None:
            support.dialog_info(_("There are No Anchors in the Selected Node"), self.window)
            return
        else: self.anchors_treeview.set_cursor(self.anchors_liststore.get_path(anchor_first_iter))
        self.glade.anchor_enter_name_hbox.hide()
        self.glade.scrolledwindow_anchors_list.show()
        self.glade.anchorhandledialog.set_title(_("Choose Existing Anchor"))
        response = self.glade.anchorhandledialog.run()
        self.glade.anchorhandledialog.hide()
        if response != 1: return # the user aborted the operation
        listmodel, listiter = self.anchors_treeviewselection.get_selected()
        self.glade.link_anchor_entry.set_text(self.anchors_liststore[listiter][0])

    def anchors_liststore_exist_or_create(self):
        """If Does Not Exist => Create Anchors Browser Liststore"""
        if not "anchors_liststore" in dir(self):
            self.anchors_liststore = gtk.ListStore(str)
            self.anchors_treeview = gtk.TreeView(self.anchors_liststore)
            self.anchors_renderer_text = gtk.CellRendererText()
            self.anchors_column = gtk.TreeViewColumn(_("Anchor Name"), self.anchors_renderer_text, text=0)
            self.anchors_treeview.append_column(self.anchors_column)
            self.anchors_treeviewselection = self.anchors_treeview.get_selection()
            self.anchors_treeview.connect('button-press-event', self.on_mouse_button_clicked_anchors_list)
            self.glade.scrolledwindow_anchors_list.add(self.anchors_treeview)
            self.glade.scrolledwindow_anchors_list.show_all()

    def on_mouse_button_clicked_anchors_list(self, widget, event):
        """Catches mouse buttons clicks"""
        if event.button != 1: return
        if event.type == gtk.gdk._2BUTTON_PRESS: self.glade.anchorhandledialog_button_ok.clicked()

    def sourceview_cursor_and_tooltips_handler(self, x, y):
        """Looks at all tags covering the position (x, y) in the text view,
           and if one of them is a link, change the cursor to the HAND2 cursor"""
        hovering_link = False
        hovering_anchor = False
        text_iter = self.sourceview.get_iter_at_location(x, y)
        if self.lists_handler.is_list_todo_beginning(text_iter):
            self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(gtk.gdk.Cursor(gtk.gdk.X_CURSOR))
            self.sourceview.set_tooltip_text(None)
            return
        tags = text_iter.get_tags()
        for tag in tags:
            tag_name = tag.get_property("name")
            if tag_name and tag_name[0:4] == cons.TAG_LINK:
                hovering_link = True
                vector = tag_name[5:].split()
                if vector[0] in [cons.LINK_TYPE_FILE, cons.LINK_TYPE_FOLD]:
                    tooltip = base64.b64decode(vector[1])
                else:
                    if vector[0] == cons.LINK_TYPE_NODE and long(vector[1]) in self.nodes_names_dict: tooltip = self.nodes_names_dict[long(vector[1])]
                    else: tooltip = vector[1].replace("amp;", "")
                    if len(vector) >= 3:
                        if len(vector) == 3: anchor_name = vector[2]
                        else: anchor_name = tag_name[5 + len(vector[0]) + len(vector[1]) + 2:]
                        tooltip += "#" + anchor_name
                break
        else:
            iter_anchor = text_iter.copy()
            pixbuf = iter_anchor.get_pixbuf()
            if pixbuf != None and "anchor" in dir(pixbuf): hovering_anchor = True
            else:
                iter_anchor.backward_char()
                pixbuf = iter_anchor.get_pixbuf()
                if pixbuf != None and "anchor" in dir(pixbuf): hovering_anchor = True
        if hovering_link != self.hovering_over_link: self.hovering_over_link = hovering_link
        if hovering_anchor:
            self.sourceview.set_tooltip_text(pixbuf.anchor)
            return
        if self.hovering_over_link:
            self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(gtk.gdk.Cursor(gtk.gdk.HAND2))
            self.sourceview.set_tooltip_text(tooltip)
        else:
            self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(None)
            self.sourceview.set_tooltip_text(None)

    def on_sourceview_event_after(self, text_view, event):
        """Called after every event on the SourceView"""
        if event.type == gtk.gdk._2BUTTON_PRESS and event.button == 1:
            x, y = text_view.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, int(event.x), int(event.y))
            iter_end = text_view.get_iter_at_location(x, y)
            iter_start = iter_end.copy()
            match = re.match('\w', iter_end.get_char(), re.UNICODE) # alphanumeric char
            if not match: return # double-click was not upon alphanumeric
            while match:
                if not iter_end.forward_char(): break # end of buffer
                match = re.match('\w', iter_end.get_char(), re.UNICODE) # alphanumeric char
            iter_start.backward_char()
            match = re.match('\w', iter_start.get_char(), re.UNICODE) # alphanumeric char
            while match:
                if not iter_start.backward_char(): break # start of buffer
                match = re.match('\w', iter_start.get_char(), re.UNICODE) # alphanumeric char
            if not match: iter_start.forward_char()
            self.curr_buffer.move_mark(self.curr_buffer.get_insert(), iter_start)
            self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), iter_end)
        elif self.syntax_highlighting != cons.CUSTOM_COLORS_ID: return
        if event.type == gtk.gdk.BUTTON_PRESS:
            if event.button == 1:
                x, y = text_view.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, int(event.x), int(event.y))
                text_iter = self.sourceview.get_iter_at_location(x, y)
                tags = text_iter.get_tags()
                # check whether we are hovering a link
                if tags:
                    for tag in tags:
                        tag_name = tag.get_property("name")
                        if tag_name and tag_name[0:4] == cons.TAG_LINK:
                            self.link_clicked(tag_name[5:])
                            return False
                if self.lists_handler.is_list_todo_beginning(text_iter):
                    self.lists_handler.todo_list_rotate_status(text_iter)
            elif event.button == 3 and not self.curr_buffer.get_has_selection():
                x, y = text_view.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, int(event.x), int(event.y))
                text_iter = self.sourceview.get_iter_at_location(x, y)
                self.curr_buffer.place_cursor(text_iter)
        elif event.type == gtk.gdk.KEY_PRESS:
            keyname = gtk.gdk.keyval_name(event.keyval)
            if (event.state & gtk.gdk.SHIFT_MASK): # Shift held down
                if keyname == "Return":
                    self.curr_buffer.insert(self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert()), 3*cons.CHAR_SPACE)
            elif keyname == "Return":
                iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
                if iter_insert == None:
                    return False
                iter_start = iter_insert.copy()
                if iter_start.backward_chars(2) and iter_start.get_char() == cons.CHAR_NEWLINE:
                    return False # former was an empty row
                list_info = self.lists_handler.get_paragraph_list_info(iter_start)
                if list_info[0] == None:
                    if self.auto_indent:
                        iter_start = iter_insert.copy()
                        former_line_indent = support.get_former_line_indentation(iter_start)
                        if former_line_indent: self.curr_buffer.insert_at_cursor(former_line_indent)
                    return False # former was not a list
                # possible list quit
                iter_list_quit = iter_insert.copy()
                if (list_info[0] == 0 and iter_list_quit.backward_chars(3) and iter_list_quit.get_char() == cons.CHAR_LISTBUL):
                    self.curr_buffer.delete(iter_list_quit, iter_insert)
                    return False # former was an empty paragraph => list quit
                elif (list_info[0] == -1 and iter_list_quit.backward_chars(3) and iter_list_quit.get_char() in [cons.CHAR_LISTTODO, cons.CHAR_LISTDONEOK, cons.CHAR_LISTDONEFAIL]):
                    self.curr_buffer.delete(iter_list_quit, iter_insert)
                    return False # former was an empty paragraph => list quit
                elif (list_info[0] > 0 and iter_list_quit.backward_chars(2) and iter_list_quit.get_char() == cons.CHAR_SPACE\
                and iter_list_quit.backward_char() and iter_list_quit.get_char() == '.'):
                    iter_list_quit.backward_chars(len(str(list_info[0])))
                    self.curr_buffer.delete(iter_list_quit, iter_insert)
                    return False # former was an empty paragraph => list quit
                if list_info[0] == 0: self.curr_buffer.insert(iter_insert, cons.CHAR_LISTBUL + cons.CHAR_SPACE)
                elif list_info[0] == -1: self.curr_buffer.insert(iter_insert, cons.CHAR_LISTTODO + cons.CHAR_SPACE)
                else:
                    curr_num = list_info[0] + 1
                    self.curr_buffer.insert(iter_insert, '%s. ' % curr_num)
                    self.lists_handler.list_adjust_ahead(curr_num, iter_insert.get_offset(), "num2num")
            elif keyname == "space":
                iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
                if iter_insert == None: return False
                iter_start = iter_insert.copy()
                if iter_start.backward_chars(2):
                    if iter_start.get_char() == cons.CHAR_GREATER and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_MINUS and iter_start.backward_char():
                        if iter_start.get_char() == cons.CHAR_LESSER:
                            self.special_char_replace(cons.SPECIAL_CHAR_ARROW_DOUBLE, iter_start, iter_insert)
                        elif iter_start.get_char() == cons.CHAR_MINUS:
                            self.special_char_replace(cons.SPECIAL_CHAR_ARROW_RIGHT, iter_start, iter_insert)
                    elif iter_start.get_char() == cons.CHAR_MINUS and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_MINUS and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_LESSER:
                        self.special_char_replace(cons.SPECIAL_CHAR_ARROW_LEFT, iter_start, iter_insert)
                    elif iter_start.get_char() == cons.CHAR_PARENTH_CLOSE and iter_start.backward_char():
                        if iter_start.get_char().lower() == "c" and iter_start.backward_char()\
                        and iter_start.get_char() == cons.CHAR_PARENTH_OPEN:
                            self.special_char_replace(cons.SPECIAL_CHAR_COPYRIGHT, iter_start, iter_insert)
                        elif iter_start.get_char().lower() == "r" and iter_start.backward_char()\
                        and iter_start.get_char() == cons.CHAR_PARENTH_OPEN:
                            self.special_char_replace(cons.SPECIAL_CHAR_REGISTERED_TRADEMARK, iter_start, iter_insert)
                        elif iter_start.get_char().lower() == "m" and iter_start.backward_char()\
                        and iter_start.get_char() == "t" and iter_start.backward_char()\
                        and iter_start.get_char() == cons.CHAR_PARENTH_OPEN:
                            self.special_char_replace(cons.SPECIAL_CHAR_UNREGISTERED_TRADEMARK, iter_start, iter_insert)
                    # Start bulleted list on "* " at line start
                    elif iter_start.get_char() == cons.CHAR_STAR and iter_start.get_line_offset() == 0:
                        self.curr_buffer.delete(iter_start, iter_insert)
                        self.lists_handler.list_bulleted_handler()
                    # Start todo list on "[ ]" at line start
                    elif iter_start.get_char() == cons.CHAR_SQ_BR_CLOSE and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_SPACE and iter_start.backward_char()\
                    and iter_start.get_char() == cons.CHAR_SQ_BR_OPEN\
                    and iter_start.get_line_offset() == 0:
                        self.curr_buffer.delete(iter_start, iter_insert)
                        self.lists_handler.list_todo_handler()
        return False

    def special_char_replace(self, special_char, iter_start, iter_insert):
        """A special char replacement is triggered"""
        self.state_machine.update_state(self.treestore[self.curr_tree_iter][3])
        self.curr_buffer.delete(iter_start, iter_insert)
        iter_insert = self.curr_buffer.get_iter_at_mark(self.curr_buffer.get_insert())
        self.curr_buffer.insert(iter_insert, special_char + cons.CHAR_SPACE)

    def on_sourceview_motion_notify_event(self, text_view, event):
        """Update the cursor image if the pointer moved"""
        if not self.sourceview.get_cursor_visible(): self.sourceview.set_cursor_visible(True)
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID: return
        x, y = self.sourceview.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, int(event.x), int(event.y))
        self.sourceview_cursor_and_tooltips_handler(x, y)
        return False

    def update_selected_node_statusbar_info(self):
        """Update the statusbar with node info"""
        if not self.curr_tree_iter:
            tooltip_text = _("No Node is Selected")
        else:
            if self.treestore[self.curr_tree_iter][4] == cons.CUSTOM_COLORS_ID: tooltip_text = _("Rich Text")
            else: tooltip_text = self.treestore[self.curr_tree_iter][4]
            if self.treestore[self.curr_tree_iter][7]: tooltip_text += "  -  " + _("Read Only")
            if self.treestore[self.curr_tree_iter][6]: tooltip_text += "  -  " + self.treestore[self.curr_tree_iter][6]
            print "sel node id=%s, seq=%s" % (self.treestore[self.curr_tree_iter][3], self.treestore[self.curr_tree_iter][5])
        self.statusbar.pop(self.statusbar_context_id)
        self.statusbar.push(self.statusbar_context_id, tooltip_text)

    def on_sourceview_visibility_notify_event(self, text_view, event):
        """Update the cursor image if the window becomes visible (e.g. when a window covering it got iconified)"""
        if self.syntax_highlighting != cons.CUSTOM_COLORS_ID: return
        wx, wy, mod = self.sourceview.window.get_pointer()
        bx, by = self.sourceview.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, wx, wy)
        self.sourceview_cursor_and_tooltips_handler(bx, by)
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
        if not self.curr_buffer.get_has_selection() and not self.apply_tag_try_automatic_bounds():
            support.dialog_warning(_("No Text is Selected"), self.window)
            return
        iter_sel_start, iter_sel_end = self.curr_buffer.get_selection_bounds()
        self.curr_buffer.remove_all_tags(iter_sel_start, iter_sel_end)
        if self.enable_spell_check: self.spell_check_set_on()
        self.update_window_save_needed("nbuf", True)

    def bookmark_curr_node(self, *args):
        """Add the Current Node to the Bookmarks List"""
        if not self.is_there_selected_node_or_error(): return
        curr_node_id_str = str(self.treestore[self.curr_tree_iter][3])
        if not curr_node_id_str in self.bookmarks:
            self.bookmarks.append(curr_node_id_str)
            support.set_bookmarks_menu_items(self)
            self.update_window_save_needed("book")

    def bookmarks_handle(self, *args):
        """Handle the Bookmarks List"""
        if support.bookmarks_handle(self):
            self.update_window_save_needed("book")
    
    def timestamp_insert(self, *args):
        """Insert Timestamp"""
        if not self.is_there_selected_node_or_error(): return
        self.curr_buffer.insert_at_cursor(time.strftime(self.timestamp_format))
    
    def set_selection_at_offset_n_delta(self, offset, delta):
        """Set the Selection from given offset to offset+delta"""
        target = self.curr_buffer.get_iter_at_offset(offset)
        if target:
            self.curr_buffer.place_cursor(target)
            if not target.forward_chars(delta):
                #print "? bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
                pass
            self.curr_buffer.move_mark(self.curr_buffer.get_selection_bound(), target)
            return
        print "! bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
    
    def tree_is_empty(self):
        """Return True if the treestore is empty"""
        return (self.treestore.get_iter_first() == None)
