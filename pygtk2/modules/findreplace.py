# -*- coding: UTF-8 -*-
#
#       findreplace.py
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

import gtk
import gobject
import re
import cgi
import time
import datetime
import cons
import menus
import support
import config


def dialog_date_select(parent_win, title, curr_time):
    """Dialog to select a Date"""
    dialog = gtk.Dialog(title=title,
        parent=parent_win,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                 gtk.STOCK_OK, gtk.RESPONSE_OK))
    dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
    content_area = dialog.get_content_area()
    calendar = gtk.Calendar()
    struct_time = time.localtime(curr_time)
    calendar.select_month(struct_time.tm_mon-1, struct_time.tm_year) # month 0-11
    calendar.select_day(struct_time.tm_mday) # day 1-31
    adj_h = gtk.Adjustment(value=struct_time.tm_hour, lower=0, upper=23, step_incr=1)
    spinbutton_h = gtk.SpinButton(adj_h)
    spinbutton_h.set_value(struct_time.tm_hour)
    adj_m = gtk.Adjustment(value=struct_time.tm_min, lower=0, upper=59, step_incr=1)
    spinbutton_m = gtk.SpinButton(adj_m)
    spinbutton_m.set_value(struct_time.tm_min)
    hbox = gtk.HBox()
    hbox.pack_start(spinbutton_h)
    hbox.pack_start(spinbutton_m)
    content_area.pack_start(calendar)
    content_area.pack_start(hbox)
    def on_key_press_dialog(widget, event):
        if gtk.gdk.keyval_name(event.keyval) == cons.STR_KEY_RETURN:
            try: dialog.get_widget_for_response(gtk.RESPONSE_OK).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
            return True
        return False
    dialog.connect("key_press_event", on_key_press_dialog)
    def on_mouse_button_clicked_dialog(widget, event):
        if event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
            try: dialog.get_widget_for_response(gtk.RESPONSE_OK).clicked()
            except: print cons.STR_PYGTK_222_REQUIRED
    dialog.connect('button-press-event', on_mouse_button_clicked_dialog)
    content_area.show_all()
    response = dialog.run()
    dialog.hide()
    if response != gtk.RESPONSE_OK: return None
    new_year, new_month, new_day = calendar.get_date()
    new_h = int(spinbutton_h.get_value())
    new_m = int(spinbutton_m.get_value())
    new_datetime = datetime.datetime(new_year, new_month+1, new_day, new_h, new_m)
    new_time = time.mktime(new_datetime.timetuple())
    return new_time


class FindReplace:
    """Handler of Bulleted and Numbered Lists"""

    def __init__(self, dad):
        """Lists Handler boot"""
        self.dad = dad
        self.replace_active = False
        self.replace_subsequent = False
        self.curr_find = [None, ""] # [latest find type, latest find pattern]
        self.from_find_iterated = False
        self.from_find_back = False
        self.newline_trick = False
        # 0-node_id, 1-start_offset, 2-end_offset, 3-node_name, 4-line_content, 5-line_num 6-node_hier_name
        self.allmatches_liststore = gtk.ListStore(long, long, long, str, str, int, str)
        self.allmatches_title = ""
        self.allmatches_position = None
        self.allmatches_size = None
        self.allmatches_path = None
        self.iteratedfinddialog = None
        self.latest_node_offset = {}
        time_now = time.time()
        time_yesterday = time_now - 86400 #24*60*60
        self.search_replace_dict = {'find':"", 'replace':"",
'match_case':False, 'reg_exp':False, 'whole_word':False, 'start_word':False,
'fw':True, 'a_ff_fa':0,
'ts_cre_>': [False, time_yesterday],
'ts_cre_<': [False, time_now],
'ts_mod_>': [False, time_yesterday],
'ts_mod_<': [False, time_now],
'idialog':True}

    def is_node_within_time_filter(self, node_iter):
        """Returns True if the given node_iter is within the Time Filter"""
        ts_cre = self.dad.treestore[node_iter][12]
        if self.search_replace_dict['ts_cre_>'][0] and ts_cre < self.search_replace_dict['ts_cre_>'][1]:
            return False
        if self.search_replace_dict['ts_cre_<'][0] and ts_cre > self.search_replace_dict['ts_cre_<'][1]:
            return False
        ts_mod = self.dad.treestore[node_iter][13]
        if self.search_replace_dict['ts_mod_>'][0] and ts_mod < self.search_replace_dict['ts_mod_>'][1]:
            return False
        if self.search_replace_dict['ts_mod_<'][0] and ts_mod > self.search_replace_dict['ts_mod_<'][1]:
            return False
        return True

    def dialog_search(self, title, replace_on, multiple_nodes, pattern_required):
        """Opens the Search Dialog"""
        dialog = gtk.Dialog(title=title,
                            parent=self.dad.window,
                            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
                            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
        dialog.set_default_size(400, -1)
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        search_entry = gtk.Entry()
        search_entry.set_text(self.search_replace_dict['find'])
        try:
            button_ok = dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT)
            if pattern_required:
                button_ok.set_sensitive(bool(self.search_replace_dict['find']))
        except:
            print cons.STR_PYGTK_222_REQUIRED
            button_ok = None
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
        if multiple_nodes:
            ts_format = "%A, %d %B %Y, %H:%M"
            ts_node_created_after_checkbutton = gtk.CheckButton(label=_("Node Created After"))
            ts_label = support.get_timestamp_str(ts_format, self.search_replace_dict['ts_cre_>'][1])
            ts_node_created_after_button = gtk.Button(label=ts_label)
            ts_node_created_after_hbox = gtk.HBox()
            ts_node_created_after_hbox.set_homogeneous(True)
            ts_node_created_after_hbox.pack_start(ts_node_created_after_checkbutton)
            ts_node_created_after_hbox.pack_start(ts_node_created_after_button)
            ts_node_created_before_checkbutton = gtk.CheckButton(label=_("Node Created Before"))
            ts_label = support.get_timestamp_str(ts_format, self.search_replace_dict['ts_cre_<'][1])
            ts_node_created_before_button = gtk.Button(label=ts_label)
            ts_node_created_before_hbox = gtk.HBox()
            ts_node_created_before_hbox.set_homogeneous(True)
            ts_node_created_before_hbox.pack_start(ts_node_created_before_checkbutton)
            ts_node_created_before_hbox.pack_start(ts_node_created_before_button)
            ts_node_modified_after_checkbutton = gtk.CheckButton(label=_("Node Modified After"))
            ts_label = support.get_timestamp_str(ts_format, self.search_replace_dict['ts_mod_>'][1])
            ts_node_modified_after_button = gtk.Button(label=ts_label)
            ts_node_modified_after_hbox = gtk.HBox()
            ts_node_modified_after_hbox.set_homogeneous(True)
            ts_node_modified_after_hbox.pack_start(ts_node_modified_after_checkbutton)
            ts_node_modified_after_hbox.pack_start(ts_node_modified_after_button)
            ts_node_modified_before_checkbutton = gtk.CheckButton(label=_("Node Modified Before"))
            ts_label = support.get_timestamp_str(ts_format, self.search_replace_dict['ts_mod_<'][1])
            ts_node_modified_before_button = gtk.Button(label=ts_label)
            ts_node_modified_before_hbox = gtk.HBox()
            ts_node_modified_before_hbox.set_homogeneous(True)
            ts_node_modified_before_hbox.pack_start(ts_node_modified_before_checkbutton)
            ts_node_modified_before_hbox.pack_start(ts_node_modified_before_button)
            ts_node_created_after_checkbutton.set_active(self.search_replace_dict['ts_cre_>'][0])
            ts_node_created_before_checkbutton.set_active(self.search_replace_dict['ts_cre_<'][0])
            ts_node_modified_after_checkbutton.set_active(self.search_replace_dict['ts_mod_>'][0])
            ts_node_modified_before_checkbutton.set_active(self.search_replace_dict['ts_mod_<'][0])
            ts_node_vbox = gtk.VBox()
            ts_node_vbox.pack_start(ts_node_created_after_hbox)
            ts_node_vbox.pack_start(ts_node_created_before_hbox)
            ts_node_vbox.pack_start(gtk.HSeparator())
            ts_node_vbox.pack_start(ts_node_modified_after_hbox)
            ts_node_vbox.pack_start(ts_node_modified_before_hbox)
            ts_frame = gtk.Frame(label="<b>"+_("Time filter")+"</b>")
            ts_frame.get_label_widget().set_use_markup(True)
            ts_frame.set_shadow_type(gtk.SHADOW_NONE)
            ts_frame.add(ts_node_vbox)
            def on_ts_node_button_clicked(widget, ts_id):
                if ts_id == 'ts_cre_>':
                    title = _("Node Created After")
                elif ts_id == 'ts_cre_<':
                    title = _("Node Created Before")
                elif ts_id == 'ts_mod_>':
                    title = _("Node Modified After")
                else:
                    title = _("Node Modified Before")
                new_time = dialog_date_select(dialog, title, self.search_replace_dict[ts_id][1])
                if new_time:
                    self.search_replace_dict[ts_id][1] = new_time
                    widget.set_label(support.get_timestamp_str(ts_format, new_time))
            ts_node_created_after_button.connect('clicked', on_ts_node_button_clicked, 'ts_cre_>')
            ts_node_created_before_button.connect('clicked', on_ts_node_button_clicked, 'ts_cre_<')
            ts_node_modified_after_button.connect('clicked', on_ts_node_button_clicked, 'ts_mod_>')
            ts_node_modified_before_button.connect('clicked', on_ts_node_button_clicked, 'ts_mod_<')
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
        if multiple_nodes:
            opt_vbox.pack_start(ts_frame)
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
            if keyname == cons.STR_KEY_RETURN:
                if button_ok is not None and button_ok.get_sensitive():
                    button_ok.clicked()
                return True
            return False
        dialog.connect('key_press_event', on_key_press_searchdialog)
        def on_search_entry_changed(editable):
            if button_ok is not None:
                button_ok.set_sensitive(bool(search_entry.get_text()))
        if pattern_required:
            search_entry.connect("changed", on_search_entry_changed)
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            find_content = unicode(search_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            self.search_replace_dict['find'] = find_content
            if replace_on:
                self.search_replace_dict['replace'] = unicode(replace_entry.get_text(), cons.STR_UTF8, cons.STR_IGNORE)
            self.search_replace_dict['match_case'] = match_case_checkbutton.get_active()
            self.search_replace_dict['reg_exp'] = reg_exp_checkbutton.get_active()
            self.search_replace_dict['whole_word'] = whole_word_checkbutton.get_active()
            self.search_replace_dict['start_word'] = start_word_checkbutton.get_active()
            self.search_replace_dict['fw'] = fw_radiobutton.get_active()
            self.search_replace_dict['a_ff_fa'] = 0 if all_radiobutton.get_active() else 1 if first_from_radiobutton.get_active() else 2
            self.search_replace_dict['ts_cre_>'][0] = ts_node_created_after_checkbutton.get_active() if multiple_nodes else False
            self.search_replace_dict['ts_cre_<'][0] = ts_node_created_before_checkbutton.get_active() if multiple_nodes else False
            self.search_replace_dict['ts_mod_>'][0] = ts_node_modified_after_checkbutton.get_active() if multiple_nodes else False
            self.search_replace_dict['ts_mod_<'][0] = ts_node_modified_before_checkbutton.get_active() if multiple_nodes else False
            self.search_replace_dict['idialog'] = iter_dialog_checkbutton.get_active()
            return self.search_replace_dict['find']
        return None

    def iterated_find_dialog(self):
        """Iterated Find/Replace Dialog"""
        if not self.iteratedfinddialog:
            dialog = gtk.Dialog(title=_("Iterate Latest Find/Replace"),
                parent=self.dad.window, flags=gtk.DIALOG_DESTROY_WITH_PARENT)
            button_close = dialog.add_button(_("Close"), 0)
            button_find_bw = dialog.add_button(_("Find Previous"), 4)
            button_find_fw = dialog.add_button(_("Find Next"), 1)
            button_replace = dialog.add_button(_("Replace"), 2)
            button_undo = dialog.add_button(_("Undo"), 3)
            dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
            button_close.set_image(gtk.image_new_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_BUTTON))
            button_find_bw.set_image(gtk.image_new_from_stock("find_back", gtk.ICON_SIZE_BUTTON))
            button_find_fw.set_image(gtk.image_new_from_stock("find_again", gtk.ICON_SIZE_BUTTON))
            button_replace.set_image(gtk.image_new_from_stock("find_replace", gtk.ICON_SIZE_BUTTON))
            button_undo.set_image(gtk.image_new_from_stock(gtk.STOCK_UNDO, gtk.ICON_SIZE_BUTTON))
            def on_button_find_bw_clicked(widget):
                dialog.hide()
                self.replace_active = False
                self.find_back()
            def on_button_find_fw_clicked(widget):
                dialog.hide()
                self.replace_active = False
                self.find_again()
            def on_button_replace_clicked(widget):
                dialog.hide()
                self.replace_active = True
                self.replace_subsequent = True
                self.find_again()
                self.replace_subsequent = False
            def on_button_undo_clicked(widget):
                self.dad.requested_step_back()
            button_close.connect('clicked', lambda x : dialog.hide())
            button_find_bw.connect('clicked', on_button_find_bw_clicked)
            button_find_fw.connect('clicked', on_button_find_fw_clicked)
            button_replace.connect('clicked', on_button_replace_clicked)
            button_undo.connect('clicked', on_button_undo_clicked)
            def on_key_press_iterated_find_dialog(widget, event):
                if gtk.gdk.keyval_name(event.keyval) == cons.STR_KEY_RETURN:
                    try: dialog.get_widget_for_response(1).clicked()
                    except: print cons.STR_PYGTK_222_REQUIRED
                    return True
                return False
            dialog.connect("key_press_event", on_key_press_iterated_find_dialog)
            self.iteratedfinddialog = dialog
        self.iteratedfinddialog.show()

    def find_in_selected_node(self):
        """Search for a pattern in the selected Node"""
        entry_hint = ""
        if not self.from_find_iterated:
            self.latest_node_offset = {}
            iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
            iter_bound = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_selection_bound())
            entry_predefined_text = self.dad.curr_buffer.get_text(iter_insert, iter_bound)
            if entry_predefined_text:
                self.search_replace_dict['find'] = entry_predefined_text
            if self.replace_active: title = _("Replace in Current Node...")
            else: title = _("Search in Current Node...")
            pattern = self.dialog_search(title, self.replace_active, False, True)
            if entry_predefined_text != "":
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(), iter_insert)
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(), iter_bound)
            if pattern: self.curr_find = ["in_selected_node", pattern]
            else: return
        else: pattern = self.curr_find[1]
        forward = self.search_replace_dict['fw']
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.search_replace_dict['a_ff_fa'] == 1
        all_matches = self.search_replace_dict['a_ff_fa'] == 0
        self.matches_num = 0
        # searching start
        if self.dad.user_active:
            self.dad.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        if all_matches:
            self.allmatches_liststore.clear()
            self.all_matches_first_in_node = True
            while self.parse_node_content_iter(self.dad.curr_tree_iter, self.dad.curr_buffer, pattern, forward, first_fromsel, all_matches, True):
                self.matches_num += 1
        elif self.parse_node_content_iter(self.dad.curr_tree_iter, self.dad.curr_buffer, pattern, forward, first_fromsel, all_matches, True):
            self.matches_num = 1
        if self.matches_num == 0:
            support.dialog_info(_("The pattern '%s' was not found") % pattern, self.dad.window)
        elif all_matches:
            self.allmatches_title = str(self.matches_num) + cons.CHAR_SPACE + _("Matches")
            self.allmatchesdialog_show()
        elif self.search_replace_dict['idialog']:
            self.iterated_find_dialog()
        if user_active_restore: self.dad.user_active = True

    def update_all_matches_progress(self):
        self.dad.progressbar.set_fraction(float(self.processed_nodes)/self.dad.num_nodes)
        if self.matches_num != self.latest_matches:
            self.latest_matches = self.matches_num
            self.dad.progressbar.set_text(str(self.matches_num))

    def find_in_all_nodes(self, father_tree_iter):
        """Search for a pattern in all the Tree Nodes"""
        if not self.from_find_iterated:
            self.latest_node_offset = {}
            iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
            iter_bound = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_selection_bound())
            entry_predefined_text = self.dad.curr_buffer.get_text(iter_insert, iter_bound)
            if entry_predefined_text:
                self.search_replace_dict['find'] = entry_predefined_text
            if self.replace_active:
                if father_tree_iter: title = _("Replace in Selected Node and Subnodes")
                else: title = _("Replace in All Nodes")
            else:
                if father_tree_iter: title = _("Search in Selected Node and Subnodes")
                else: title = _("Search in All Nodes")
            pattern = self.dialog_search(title, self.replace_active, True, True)
            if entry_predefined_text != "":
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(), iter_insert)
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(), iter_bound)
            if pattern:
                if not father_tree_iter: self.curr_find = ["in_all_nodes", pattern]
                else: self.curr_find = ["in_sel_nod_n_sub", pattern]
            else: return
        else: pattern = self.curr_find[1]
        starting_tree_iter = self.dad.curr_tree_iter.copy()
        current_cursor_pos = self.dad.curr_buffer.get_property(cons.STR_CURSOR_POSITION)
        forward = self.search_replace_dict['fw']
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.search_replace_dict['a_ff_fa'] == 1
        all_matches = self.search_replace_dict['a_ff_fa'] == 0
        if first_fromsel or father_tree_iter:
            self.first_useful_node = False # no one node content was parsed yet
            node_iter = self.dad.curr_tree_iter.copy()
        else:
            self.first_useful_node = True # all range will be parsed so no matter
            if forward: node_iter = self.dad.treestore.get_iter_first()
            else: node_iter = self.dad.get_tree_iter_last_sibling(None)
        self.matches_num = 0
        if all_matches: self.allmatches_liststore.clear()
        config.get_tree_expanded_collapsed_string(self.dad)
        # searching start
        if self.dad.user_active:
            self.dad.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        self.processed_nodes = 0
        self.latest_matches = 0
        self.dad.update_num_nodes(father_tree_iter)
        if all_matches:
            self.dad.progressbar.set_text("0")
            self.dad.progresstop.show()
            self.dad.progressbar.show()
            while gtk.events_pending(): gtk.main_iteration()
        search_start_time = time.time()
        while node_iter:
            self.all_matches_first_in_node = True
            while self.parse_given_node_content(node_iter, pattern, forward, first_fromsel, all_matches):
                self.matches_num += 1
                if not all_matches or self.dad.progress_stop: break
            self.processed_nodes += 1
            if self.matches_num == 1 and not all_matches: break
            if father_tree_iter and not self.from_find_iterated: break
            last_top_node_iter = node_iter.copy() # we need this if we start from a node that is not in top level
            if forward: node_iter = self.dad.treestore.iter_next(node_iter)
            else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
            if not node_iter and father_tree_iter: break
            # code that, in case we start from a node that is not top level, climbs towards the top
            while not node_iter:
                node_iter = self.dad.treestore.iter_parent(last_top_node_iter)
                if node_iter:
                    last_top_node_iter = node_iter.copy()
                    # we do not check the parent on purpose, only the uncles in the proper direction
                    if forward: node_iter = self.dad.treestore.iter_next(node_iter)
                    else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
                else: break
            if self.dad.progress_stop: break
            if all_matches:
                self.update_all_matches_progress()
        search_end_time = time.time()
        print search_end_time - search_start_time, "sec"
        if user_active_restore: self.dad.user_active = True
        config.set_tree_expanded_collapsed_string(self.dad)
        if not self.matches_num or all_matches:
            self.dad.treeview_safe_set_cursor(starting_tree_iter)
            self.dad.objects_buffer_refresh()
            self.dad.sourceview.grab_focus()
            self.dad.curr_buffer.place_cursor(self.dad.curr_buffer.get_iter_at_offset(current_cursor_pos))
            self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
        if not self.matches_num:
            support.dialog_info(_("The pattern '%s' was not found") % pattern, self.dad.window)
        else:
            if all_matches:
                self.allmatches_title = str(self.matches_num) + cons.CHAR_SPACE + _("Matches")
                self.allmatchesdialog_show()
            else:
                self.dad.treeview_safe_set_cursor(self.dad.curr_tree_iter)
                if self.search_replace_dict['idialog']:
                    self.iterated_find_dialog()
        if all_matches:
            assert self.processed_nodes == self.dad.num_nodes or self.dad.progress_stop, "%s != %s" % (self.processed_nodes, self.dad.num_nodes)
            self.dad.progresstop.hide()
            self.dad.progressbar.hide()
            self.dad.progress_stop = False

    def find_a_node(self):
        """Search for a pattern between all the Node's Names"""
        if not self.from_find_iterated:
            if self.replace_active: title = _("Replace in Node Names...")
            else: title = _("Search For a Node Name...")
            pattern_clean = self.dialog_search(title, self.replace_active, True, False)
            if pattern_clean is not None: self.curr_find = ["a_node", pattern_clean]
            else: return
        else: pattern_clean = self.curr_find[1]
        if not self.search_replace_dict['reg_exp']: # NOT REGULAR EXPRESSION
            pattern_ready = re.escape(pattern_clean) # backslashes all non alphanum chars => to not spoil re
            if self.search_replace_dict['whole_word']: # WHOLE WORD
                pattern_ready = r'\b' + pattern_ready + r'\b'
            elif self.search_replace_dict['start_word']: # START WORD
                pattern_ready = r'\b' + pattern_ready
        else: pattern_ready = pattern_clean
        if self.search_replace_dict['match_case']: # CASE SENSITIVE
            pattern = re.compile(pattern_ready, re.UNICODE|re.MULTILINE)
        else: pattern = re.compile(pattern_ready, re.IGNORECASE|re.UNICODE|re.MULTILINE)
        forward = self.search_replace_dict['fw']
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.search_replace_dict['a_ff_fa'] == 1
        all_matches = self.search_replace_dict['a_ff_fa'] == 0
        if first_fromsel:
            if forward: node_iter = self.dad.treestore.iter_next(self.dad.curr_tree_iter)
            else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, self.dad.curr_tree_iter)
            top_node_iter = self.dad.curr_tree_iter.copy()
            while not node_iter:
                node_iter = top_node_iter.copy()
                if forward: node_iter = self.dad.treestore.iter_next(node_iter)
                else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
                top_node_iter = self.dad.treestore.iter_parent(top_node_iter)
                if not top_node_iter: break
        else:
            if forward: node_iter = self.dad.treestore.get_iter_first()
            else: node_iter = self.dad.get_tree_iter_last_sibling(None)
        self.matches_num = 0
        if all_matches: self.allmatches_liststore.clear()
        # searching start
        while node_iter != None:
            if self.parse_node_name(node_iter, pattern, forward, all_matches):
                self.matches_num += 1
                if not all_matches: break
            last_top_node_iter = node_iter.copy() # we need this if we start from a node that is not in top level
            if forward: node_iter = self.dad.treestore.iter_next(node_iter)
            else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
            # code that, in case we start from a node that is not top level, climbs towards the top
            while node_iter == None:
                node_iter = self.dad.treestore.iter_parent(last_top_node_iter)
                if node_iter != None:
                    last_top_node_iter = node_iter.copy()
                    # we do not check the parent on purpose, only the uncles in the proper direction
                    if forward: node_iter = self.dad.treestore.iter_next(node_iter)
                    else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
                else: break
        self.dad.objects_buffer_refresh()
        if self.matches_num == 0:
            support.dialog_info(_("The pattern '%s' was not found") % pattern_clean, self.dad.window)
        elif all_matches:
            self.allmatches_title = str(self.matches_num) + cons.CHAR_SPACE + _("Matches")
            self.allmatchesdialog_show()
        elif self.search_replace_dict['idialog']:
            self.iterated_find_dialog()
        if self.matches_num and self.replace_active: self.dad.update_window_save_needed()

    def find_pattern(self, tree_iter, text_buffer, pattern, start_iter, forward, all_matches):
        """Returns (start_iter, end_iter) or (None, None)"""
        text = unicode(text_buffer.get_text(*text_buffer.get_bounds()), cons.STR_UTF8, cons.STR_IGNORE)
        if not self.search_replace_dict['reg_exp']: # NOT REGULAR EXPRESSION
            pattern = re.escape(pattern) # backslashes all non alphanum chars => to not spoil re
            if self.search_replace_dict['whole_word']: # WHOLE WORD
                pattern = r'\b' + pattern + r'\b'
            elif self.search_replace_dict['start_word']: # START WORD
                pattern = r'\b' + pattern
        if self.search_replace_dict['match_case']: # CASE SENSITIVE
            pattern = re.compile(pattern, re.UNICODE|re.MULTILINE)
        else: pattern = re.compile(pattern, re.IGNORECASE|re.UNICODE|re.MULTILINE)
        start_offset = start_iter.get_offset()
        #start_offset -= self.get_num_objs_before_offset(text_buffer, start_offset)
        if forward:
            match = pattern.search(text, start_offset)
        else:
            match = None
            for temp_match in pattern.finditer(text, 0, start_offset): match = temp_match
        if self.replace_active: obj_match_offsets = (None, None)
        else: obj_match_offsets = self.check_pattern_in_object_between(text_buffer,
            pattern,
            start_iter.get_offset(),
            match.start() if match else -1,
            forward)
        if obj_match_offsets[0] != None: match_offsets = (obj_match_offsets[0], obj_match_offsets[1])
        else: match_offsets = (match.start(), match.end()) if match else (None, None)
        if match_offsets[0] == None: return False
        # match found!
        if obj_match_offsets[0] == None: num_objs = self.get_num_objs_before_offset(text_buffer, match_offsets[0])
        else: num_objs = 0
        final_start_offset = match_offsets[0] + num_objs
        final_delta_offset = match_offsets[1] - match_offsets[0]
        #print "IN", final_start_offset, final_delta_offset, self.dad.treestore[tree_iter][1]
        #for count in range(final_delta_offset):
        #    print count, text_buffer.get_iter_at_offset(final_start_offset+count).get_char()
        if not self.dad.curr_tree_iter\
        or self.dad.treestore[tree_iter][3] != self.dad.treestore[self.dad.curr_tree_iter][3]:
            self.dad.treeview_safe_set_cursor(tree_iter)
        self.dad.set_selection_at_offset_n_delta(final_start_offset, final_delta_offset)
        #print "OUT"
        mark_insert = text_buffer.get_insert()
        iter_insert = text_buffer.get_iter_at_mark(mark_insert)
        if all_matches:
            if self.newline_trick: newline_trick_offset = 1
            else: newline_trick_offset = 0
            node_id = self.dad.treestore[tree_iter][3]
            start_offset = match_offsets[0] + num_objs - newline_trick_offset
            end_offset = match_offsets[1] + num_objs - newline_trick_offset
            node_name = self.dad.treestore[tree_iter][1]
            node_hier_name = support.get_node_hierarchical_name(self.dad, tree_iter, separator=" << ", for_filename=False, root_to_leaf=False)
            line_content = self.get_line_content(text_buffer, iter_insert) if obj_match_offsets[0] == None else obj_match_offsets[2]
            line_num = text_buffer.get_iter_at_offset(start_offset).get_line()
            if not self.newline_trick: line_num += 1
            self.allmatches_liststore.append([node_id, start_offset, end_offset, node_name, line_content, line_num, cgi.escape(node_hier_name)])
            #print line_num, self.matches_num
        else: self.dad.sourceview.scroll_to_mark(mark_insert, cons.SCROLL_MARGIN)
        if self.replace_active:
            if self.dad.get_node_read_only(): return False
            replacer_text = self.search_replace_dict['replace']
            text_buffer.delete_selection(interactive=False, default_editable=True)
            text_buffer.insert_at_cursor(replacer_text)
            if not all_matches:
                self.dad.set_selection_at_offset_n_delta(match_offsets[0] + num_objs, len(replacer_text))
            self.dad.state_machine.update_state()
            self.dad.ctdb_handler.pending_edit_db_node_buff(self.dad.treestore[tree_iter][3], force_user_active=True)
        return True

    def check_pattern_in_object(self, pattern, obj):
        """Search for the pattern in the given object"""
        if obj[0] == "pixbuf":
            pixbuf_attrs = dir(obj[1][1])
            if "filename" in pixbuf_attrs:
                if pattern.search(obj[1][1].filename): return (True, obj[1][1].filename)
            elif "anchor" in pixbuf_attrs:
                if pattern.search(obj[1][1].anchor): return (True, obj[1][1].anchor)
        elif obj[0] == "table":
            for row in obj[1][1]['matrix']:
                for col in row:
                    if pattern.search(col): return (True, "<table>")
        elif obj[0] == "codebox":
            if pattern.search(obj[1][1]['fill_text']): return (True, "<codebox>")
        return (False, "")

    def check_pattern_in_object_between(self, text_buffer, pattern, start_offset, end_offset, forward):
        """Search for the pattern in the given slice and direction"""
        if not forward: start_offset -= 1
        if end_offset < 0:
            end_offset = text_buffer.get_end_iter().get_offset() if forward else 0
        sel_range = (start_offset, end_offset) if forward else (end_offset, start_offset)
        obj_vec = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(text_buffer, sel_range=sel_range)
        if not obj_vec: return (None, None)
        if forward:
            for element in obj_vec:
                patt_in_obj = self.check_pattern_in_object(pattern, element)
                if patt_in_obj[0]:
                    return (element[1][0], element[1][0]+1, patt_in_obj[1])
        else:
            for element in reversed(obj_vec):
                patt_in_obj = self.check_pattern_in_object(pattern, element)
                if patt_in_obj[0]:
                    return (element[1][0], element[1][0]+1, patt_in_obj[1])
        return (None, None)

    def get_num_objs_before_offset(self, text_buffer, max_offset):
        """Returns the num of objects from buffer start to the given offset"""
        num_objs = 0
        local_limit_offset = max_offset
        curr_iter = text_buffer.get_start_iter()
        curr_offset = curr_iter.get_offset()
        while curr_offset <= local_limit_offset:
            anchor = curr_iter.get_child_anchor()
            if anchor:
                num_objs += 1
                local_limit_offset += 1
            if not curr_iter.forward_char():
                break
            next_offset = curr_iter.get_offset()
            if next_offset == curr_offset:
                break
            curr_offset = next_offset
        return num_objs

    def get_inner_start_iter(self, text_buffer, forward, node_id):
        """Get start_iter when not at beginning or end"""
        if text_buffer.get_has_selection():
            iter_start, iter_end = text_buffer.get_selection_bounds()
            offsets = [iter_start.get_offset(), iter_end.get_offset()]
        else:
            iter_insert = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            offsets = [iter_insert.get_offset()]
        if not self.replace_active or self.replace_subsequent:
            # it's a find or subsequent replace, so we want, given a selected word, to find for the subsequent one
            if forward:
                start_iter = text_buffer.get_iter_at_offset(max(offsets))
            else:
                start_iter = text_buffer.get_iter_at_offset(min(offsets))
        else:
            # it's a first replace, so we want, given a selected word, to replace starting from this one
            if forward:
                start_iter = text_buffer.get_iter_at_offset(min(offsets))
            else:
                start_iter = text_buffer.get_iter_at_offset(max(offsets))
        if self.latest_node_offset\
        and self.latest_node_offset["n"] == node_id\
        and self.latest_node_offset["o"] == start_iter.get_offset():
            if forward: start_iter.forward_char()
            else: start_iter.backward_char()
        self.latest_node_offset["n"] = node_id
        self.latest_node_offset["o"] = start_iter.get_offset()
        #print self.latest_node_offset["n"], offsets, self.latest_node_offset["o"]
        return start_iter

    def parse_node_content_iter(self, tree_iter, text_buffer, pattern, forward, first_fromsel, all_matches, first_node):
        """Returns True if pattern was find, False otherwise"""
        try:
            buff_start_iter = text_buffer.get_start_iter()
            if buff_start_iter.get_char() != cons.CHAR_NEWLINE:
                self.newline_trick = True
                if not text_buffer.get_modified(): restore_modified = True
                else: restore_modified = False
                text_buffer.insert(buff_start_iter, cons.CHAR_NEWLINE)
            else:
                self.newline_trick = False
                restore_modified = False
            if (first_fromsel and first_node)\
            or (all_matches and not self.all_matches_first_in_node):
                node_id = self.dad.get_node_id_from_tree_iter(tree_iter)
                start_iter = self.get_inner_start_iter(text_buffer, forward, node_id)
            else:
                if forward: start_iter = text_buffer.get_start_iter()
                else: start_iter = text_buffer.get_end_iter()
                if all_matches: self.all_matches_first_in_node = False
            if self.is_node_within_time_filter(tree_iter):
                pattern_found = self.find_pattern(tree_iter, text_buffer, pattern, start_iter, forward, all_matches)
            else:
                pattern_found = False
            if self.newline_trick:
                buff_start_iter = text_buffer.get_start_iter()
                buff_step_iter = buff_start_iter.copy()
                if buff_step_iter.forward_char(): text_buffer.delete(buff_start_iter, buff_step_iter)
                if restore_modified: text_buffer.set_modified(False)
            if self.replace_active and pattern_found:
                self.dad.update_window_save_needed("nbuf", given_tree_iter=tree_iter)
            return pattern_found
        except: # caused by bad symbol, #664
            return False

    def parse_given_node_content(self, node_iter, pattern, forward, first_fromsel, all_matches):
        """Returns True if pattern was found, False otherwise"""
        text_buffer = self.dad.get_textbuffer_from_tree_iter(node_iter)
        if not self.first_useful_node:
            # first_fromsel plus first_node not already parsed
            if not self.dad.curr_tree_iter\
            or self.dad.treestore[node_iter][3] == self.dad.treestore[self.dad.curr_tree_iter][3]:
                self.first_useful_node = True # a first_node was parsed
                if self.parse_node_content_iter(node_iter, text_buffer, pattern, forward, first_fromsel, all_matches, True):
                    return True # first_node node, first_fromsel
        else:
            # not first_fromsel or first_fromsel with first_node already parsed
            if self.parse_node_content_iter(node_iter, text_buffer, pattern, forward, first_fromsel, all_matches, False):
                return True # not first_node node
        node_iter = self.dad.treestore.iter_children(node_iter) # check for children
        if node_iter and not forward: node_iter = self.dad.get_tree_iter_last_sibling(node_iter)
        while node_iter and not self.dad.progress_stop:
            self.all_matches_first_in_node = True
            while self.parse_given_node_content(node_iter, pattern, forward, first_fromsel, all_matches):
                self.matches_num += 1
                if not all_matches or self.dad.progress_stop: break
            if self.matches_num == 1 and not all_matches: break
            if forward: node_iter = self.dad.treestore.iter_next(node_iter)
            else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
            self.processed_nodes += 1
            if all_matches:
                self.update_all_matches_progress()
        return False

    def parse_node_name(self, node_iter, pattern, forward, all_matches):
        """Recursive function that searchs for the given pattern"""
        if self.is_node_within_time_filter(node_iter):
            text_name = self.dad.treestore[node_iter][1].decode(cons.STR_UTF8)
            match = pattern.search(text_name)
            if not match:
                text_tags = self.dad.treestore[node_iter][6].decode(cons.STR_UTF8)
                match = pattern.search(text_tags)
        else:
            match = None
        if match:
            if all_matches:
                node_id = self.dad.treestore[node_iter][3]
                node_name = self.dad.treestore[node_iter][1]
                node_hier_name = support.get_node_hierarchical_name(self.dad, node_iter, separator=" << ", for_filename=False, root_to_leaf=False)
                line_content = self.get_first_line_content(self.dad.get_textbuffer_from_tree_iter(node_iter))
                self.allmatches_liststore.append([node_id, 0, 0, node_name, line_content, 1, cgi.escape(node_hier_name)])
            if self.replace_active and not self.dad.treestore[node_iter][7]:
                replacer_text = self.search_replace_dict['replace']
                text_name = pattern.sub(replacer_text, text_name)
                self.dad.treestore[node_iter][1] = text_name
                self.dad.ctdb_handler.pending_edit_db_node_prop(self.dad.treestore[node_iter][3])
            if not all_matches:
                self.dad.treeview_safe_set_cursor(node_iter)
                self.dad.sourceview.grab_focus()
                return True
            else: self.matches_num += 1
        node_iter = self.dad.treestore.iter_children(node_iter) # check for children
        if node_iter != None and not forward: node_iter = self.dad.get_tree_iter_last_sibling(node_iter)
        while node_iter != None:
            if self.parse_node_name(node_iter, pattern, forward, all_matches)\
            and not all_matches: return True
            if forward: node_iter = self.dad.treestore.iter_next(node_iter)
            else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
        return False

    def find_back(self):
        """Continue the previous search (a_node/in_selected_node/in_all_nodes) but in Opposite Direction"""
        self.from_find_back = True
        self.replace_active = False
        self.find_again()

    def replace_in_selected_node(self):
        """Replace a pattern in the selected Node"""
        self.replace_active = True
        self.find_in_selected_node()
        self.replace_active = False

    def replace_in_all_nodes(self, father_tree_iter):
        """Replace the pattern in all the Tree Nodes"""
        self.replace_active = True
        self.find_in_all_nodes(father_tree_iter)
        self.replace_active = False

    def replace_in_nodes_names(self):
        """Replace the pattern between all the Node's Names"""
        self.replace_active = True
        self.find_a_node()
        self.replace_active = False

    def find_again(self):
        """Continue the previous search (a_node/in_selected_node/in_all_nodes)"""
        self.from_find_iterated = True
        if self.curr_find[0] == None: support.dialog_warning(_("No Previous Search Was Performed During This Session"), self.dad.window)
        elif self.curr_find[0] == "in_selected_node": self.find_in_selected_node()
        elif self.curr_find[0] == "in_all_nodes": self.find_in_all_nodes(None)
        elif self.curr_find[0] == "in_sel_nod_n_sub": self.find_in_all_nodes(self.dad.curr_tree_iter)
        elif self.curr_find[0] == "a_node": self.find_a_node()
        self.from_find_iterated = False

    def replace_again(self):
        """Continue the previous replace (a_node/in_selected_node/in_all_nodes)"""
        self.replace_active = True
        self.replace_subsequent = True
        self.find_again()
        self.replace_active = False
        self.replace_subsequent = False

    def get_line_content(self, text_buffer, text_iter):
        """Returns the Line Content Given the Text Iter"""
        try:
            line_start = text_iter.copy()
            line_end = text_iter.copy()
            if not line_start.backward_char(): return ""
            while line_start.get_char() != cons.CHAR_NEWLINE:
                if not line_start.backward_char(): break
            else: line_start.forward_char()
            while line_end.get_char() != cons.CHAR_NEWLINE:
                if not line_end.forward_char(): break
            return text_buffer.get_text(line_start, line_end)
        except: # caused by bad symbol, #664
            return u''

    def get_first_line_content(self, text_buffer):
        """Returns the First Not Empty Line Content Given the Text Buffer"""
        try:
            start_iter = text_buffer.get_start_iter()
            while start_iter.get_char() == cons.CHAR_NEWLINE:
                if not start_iter.forward_char(): return ""
            end_iter = start_iter.copy()
            while end_iter.get_char() != cons.CHAR_NEWLINE:
                if not end_iter.forward_char(): break
            return text_buffer.get_text(start_iter, end_iter)
        except: # caused by bad symbol, #664
            return u''

    def allmatchesdialog_show(self):
        """Create the All Matches Dialog"""
        allmatchesdialog = gtk.Dialog(title=self.allmatches_title, parent=self.dad.window, flags=gtk.DIALOG_DESTROY_WITH_PARENT)
        if self.allmatches_position:
            allmatchesdialog.set_default_size(self.allmatches_size[0], self.allmatches_size[1])
            allmatchesdialog.move(self.allmatches_position[0], self.allmatches_position[1])
        else:
            allmatchesdialog.set_default_size(700, 350)
            allmatchesdialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        kb_sh = menus.get_menu_item_kb_shortcut(self.dad, "toggle_show_allmatches_dlg")
        button_hide = allmatchesdialog.add_button(_("Hide (Restore with '%s')") % kb_sh, gtk.RESPONSE_CLOSE)
        button_hide.set_image(gtk.image_new_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_BUTTON))
        treeview = gtk.TreeView(self.allmatches_liststore)
        renderer_text_node = gtk.CellRendererText()
        renderer_text_linenum = gtk.CellRendererText()
        renderer_text_linecontent = gtk.CellRendererText()
        node_column = gtk.TreeViewColumn(_("Node Name"), renderer_text_node, text=3)
        treeview.append_column(node_column)
        linenum_column = gtk.TreeViewColumn(_("Line"), renderer_text_linenum, text=5)
        treeview.append_column(linenum_column)
        linecontent_column = gtk.TreeViewColumn(_("Line Content"), renderer_text_linecontent, text=4)
        treeview.append_column(linecontent_column)
        treeview.set_tooltip_column(6)
        treeview.connect('event-after', self.on_treeview_event_after)
        scrolledwindow_allmatches = gtk.ScrolledWindow()
        scrolledwindow_allmatches.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scrolledwindow_allmatches.add(treeview)
        content_area = allmatchesdialog.get_content_area()
        content_area.pack_start(scrolledwindow_allmatches)
        def on_allmatchesdialog_delete_event(widget, event):
            self.allmatches_position = allmatchesdialog.get_position()
            self.allmatches_size = (allmatchesdialog.get_allocation().width,
                                    allmatchesdialog.get_allocation().height)
            model, list_iter = treeview.get_selection().get_selected()
            if not list_iter:
                self.allmatches_path = None
            else:
                self.allmatches_path = self.allmatches_liststore.get_path(list_iter)
            return False
        allmatchesdialog.connect('delete-event', on_allmatchesdialog_delete_event)
        def on_button_hide_clicked(button):
            on_allmatchesdialog_delete_event(None, None)
            allmatchesdialog.destroy()
        button_hide.connect('clicked', on_button_hide_clicked)
        if self.allmatches_path is not None:
            treeview.set_cursor(self.allmatches_path)
            treeview.scroll_to_cell(self.allmatches_path)
        allmatchesdialog.show_all()

    def on_treeview_event_after(self, treeview, event):
        """Catches mouse buttons clicks"""
        if event.type not in [gtk.gdk.BUTTON_PRESS, gtk.gdk.KEY_PRESS]: return
        model, list_iter = treeview.get_selection().get_selected()
        if not list_iter: return
        tree_iter = self.dad.get_tree_iter_from_node_id(model[list_iter][0])
        if not tree_iter:
            support.dialog_error(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)") % model[list_iter][0], self.dad.window)
            self.allmatches_liststore.remove(list_iter)
            return
        self.dad.treeview_safe_set_cursor(tree_iter)
        self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(),
                                       self.dad.curr_buffer.get_iter_at_offset(model[list_iter][1]))
        self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(),
                                       self.dad.curr_buffer.get_iter_at_offset(model[list_iter][2]))
        self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
