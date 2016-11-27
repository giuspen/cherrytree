# -*- coding: UTF-8 -*-
#
#       findreplace.py
#
#       Copyright 2009-2016 Giuseppe Penone <giuspen@gmail.com>
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

import gtk, gobject
import re, cgi, time
import cons, menus, support, config


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
        self.allmatchesdialog_init()
        self.iteratedfinddialog = None
        self.latest_node_offset = {}

    def iterated_find_dialog(self):
        """Iterated Find/Replace Dialog"""
        if not self.iteratedfinddialog:
            dialog = gtk.Dialog(title=_("Iterate Latest Find/Replace"),
                parent=self.dad.window, flags=gtk.DIALOG_DESTROY_WITH_PARENT)
            button_close = dialog.add_button(_("Close"), 0)
            button_find_bw = dialog.add_button(_("Find"), 4)
            button_find_fw = dialog.add_button(_("Find"), 1)
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
                self.dad.search_replace_dict['find'] = entry_predefined_text
            if self.replace_active: title = _("Replace in Current Node...")
            else: title = _("Search in Current Node...")
            pattern = self.dad.dialog_search(title, self.replace_active)
            if entry_predefined_text != "":
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(), iter_insert)
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(), iter_bound)
            if pattern != None: self.curr_find = ["in_selected_node", pattern]
            else: return
        else: pattern = self.curr_find[1]
        forward = self.dad.search_replace_dict['fw']
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.dad.search_replace_dict['a_ff_fa'] == 1
        all_matches = self.dad.search_replace_dict['a_ff_fa'] == 0
        self.matches_num = 0
        # searching start
        if self.dad.user_active:
            self.dad.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        if all_matches:
            self.liststore.clear()
            self.all_matches_first_in_node = True
            while self.parse_node_content_iter(self.dad.curr_tree_iter, self.dad.curr_buffer, pattern, forward, first_fromsel, all_matches, True):
                self.matches_num += 1
        elif self.parse_node_content_iter(self.dad.curr_tree_iter, self.dad.curr_buffer, pattern, forward, first_fromsel, all_matches, True):
            self.matches_num = 1
        if self.matches_num == 0:
            support.dialog_info(_("The pattern '%s' was not found") % pattern, self.dad.window)
        elif all_matches:
            self.allmatchesdialog.set_title(str(self.matches_num) + cons.CHAR_SPACE + _("Matches"))
            self.allmatchesdialog.show()
        elif self.dad.search_replace_dict['idialog']:
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
                self.dad.search_replace_dict['find'] = entry_predefined_text
            if self.replace_active:
                if father_tree_iter: title = _("Replace in Selected Node and Subnodes")
                else: title = _("Replace in All Nodes")
            else:
                if father_tree_iter: title = _("Search in Selected Node and Subnodes")
                else: title = _("Search in All Nodes")
            pattern = self.dad.dialog_search(title, self.replace_active)
            if entry_predefined_text != "":
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(), iter_insert)
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(), iter_bound)
            if pattern != None:
                if not father_tree_iter: self.curr_find = ["in_all_nodes", pattern]
                else: self.curr_find = ["in_sel_nod_n_sub", pattern]
            else: return
        else: pattern = self.curr_find[1]
        starting_tree_iter = self.dad.curr_tree_iter.copy()
        current_cursor_pos = self.dad.curr_buffer.get_property(cons.STR_CURSOR_POSITION)
        forward = self.dad.search_replace_dict['fw']
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.dad.search_replace_dict['a_ff_fa'] == 1
        all_matches = self.dad.search_replace_dict['a_ff_fa'] == 0
        if first_fromsel or father_tree_iter:
            self.first_useful_node = False # no one node content was parsed yet
            node_iter = self.dad.curr_tree_iter.copy()
        else:
            self.first_useful_node = True # all range will be parsed so no matter
            if forward: node_iter = self.dad.treestore.get_iter_first()
            else: node_iter = self.dad.get_tree_iter_last_sibling(None)
        self.matches_num = 0
        if all_matches: self.liststore.clear()
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
                self.allmatchesdialog.set_title(str(self.matches_num) + cons.CHAR_SPACE + _("Matches"))
                self.allmatchesdialog.show()
            else:
                self.dad.treeview_safe_set_cursor(self.dad.curr_tree_iter)
                if self.dad.search_replace_dict['idialog']:
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
            pattern_clean = self.dad.dialog_search(title, self.replace_active)
            if pattern_clean != None: self.curr_find = ["a_node", pattern_clean]
            else: return
        else: pattern_clean = self.curr_find[1]
        if not self.dad.search_replace_dict['reg_exp']: # NOT REGULAR EXPRESSION
            pattern_ready = re.escape(pattern_clean) # backslashes all non alphanum chars => to not spoil re
            if self.dad.search_replace_dict['whole_word']: # WHOLE WORD
                pattern_ready = r'\b' + pattern_ready + r'\b'
            elif self.dad.search_replace_dict['start_word']: # START WORD
                pattern_ready = r'\b' + pattern_ready
        else: pattern_ready = pattern_clean
        if self.dad.search_replace_dict['match_case']: # CASE SENSITIVE
            pattern = re.compile(pattern_ready, re.UNICODE|re.MULTILINE)
        else: pattern = re.compile(pattern_ready, re.IGNORECASE|re.UNICODE|re.MULTILINE)
        forward = self.dad.search_replace_dict['fw']
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.dad.search_replace_dict['a_ff_fa'] == 1
        all_matches = self.dad.search_replace_dict['a_ff_fa'] == 0
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
        if all_matches: self.liststore.clear()
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
            self.allmatchesdialog.set_title(str(self.matches_num) + cons.CHAR_SPACE + _("Matches"))
            self.allmatchesdialog.show()
        elif self.dad.search_replace_dict['idialog']:
            self.iterated_find_dialog()
        if self.matches_num and self.replace_active: self.dad.update_window_save_needed()

    def find_pattern(self, tree_iter, text_buffer, pattern, start_iter, forward, all_matches):
        """Returns (start_iter, end_iter) or (None, None)"""
        text = unicode(text_buffer.get_text(*text_buffer.get_bounds()), cons.STR_UTF8, cons.STR_IGNORE)
        if not self.dad.search_replace_dict['reg_exp']: # NOT REGULAR EXPRESSION
            pattern = re.escape(pattern) # backslashes all non alphanum chars => to not spoil re
            if self.dad.search_replace_dict['whole_word']: # WHOLE WORD
                pattern = r'\b' + pattern + r'\b'
            elif self.dad.search_replace_dict['start_word']: # START WORD
                pattern = r'\b' + pattern
        if self.dad.search_replace_dict['match_case']: # CASE SENSITIVE
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
            self.liststore.append([node_id, start_offset, end_offset, node_name, line_content, line_num, cgi.escape(node_hier_name)])
            #print line_num, self.matches_num
        else: self.dad.sourceview.scroll_to_mark(mark_insert, cons.SCROLL_MARGIN)
        if self.replace_active:
            if self.dad.is_curr_node_read_only(): return False
            replacer_text = self.dad.search_replace_dict['replace']
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
        pattern_found = self.find_pattern(tree_iter, text_buffer, pattern, start_iter, forward, all_matches)
        if self.newline_trick:
            buff_start_iter = text_buffer.get_start_iter()
            buff_step_iter = buff_start_iter.copy()
            if buff_step_iter.forward_char(): text_buffer.delete(buff_start_iter, buff_step_iter)
            if restore_modified: text_buffer.set_modified(False)
        if self.replace_active and pattern_found:
            self.dad.update_window_save_needed("nbuf", given_tree_iter=tree_iter)
        return pattern_found

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
        text_name = self.dad.treestore[node_iter][1].decode(cons.STR_UTF8)
        match = pattern.search(text_name)
        if not match:
            text_tags = self.dad.treestore[node_iter][6].decode(cons.STR_UTF8)
            match = pattern.search(text_tags)
        if match:
            if all_matches:
                node_id = self.dad.treestore[node_iter][3]
                node_name = self.dad.treestore[node_iter][1]
                node_hier_name = support.get_node_hierarchical_name(self.dad, node_iter, separator=" << ", for_filename=False, root_to_leaf=False)
                line_content = self.get_first_line_content(self.dad.get_textbuffer_from_tree_iter(node_iter))
                self.liststore.append([node_id, 0, 0, node_name, line_content, 1, cgi.escape(node_hier_name)])
            if self.replace_active and not self.dad.treestore[node_iter][7]:
                replacer_text = self.dad.search_replace_dict['replace']
                text_name = text_name.replace(self.curr_find[1], replacer_text)
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
        line_start = text_iter.copy()
        line_end = text_iter.copy()
        if not line_start.backward_char(): return ""
        while line_start.get_char() != cons.CHAR_NEWLINE:
            if not line_start.backward_char(): break
        else: line_start.forward_char()
        while line_end.get_char() != cons.CHAR_NEWLINE:
            if not line_end.forward_char(): break
        return text_buffer.get_text(line_start, line_end)

    def get_first_line_content(self, text_buffer):
        """Returns the First Not Empty Line Content Given the Text Buffer"""
        start_iter = text_buffer.get_start_iter()
        while start_iter.get_char() == cons.CHAR_NEWLINE:
            if not start_iter.forward_char(): return ""
        end_iter = start_iter.copy()
        while end_iter.get_char() != cons.CHAR_NEWLINE:
            if not end_iter.forward_char(): break
        return text_buffer.get_text(start_iter, end_iter)

    def allmatchesdialog_init(self):
        """Create the All Matches Dialog"""
        self.allmatchesdialog = gtk.Dialog(parent=self.dad.window, flags=gtk.DIALOG_DESTROY_WITH_PARENT)
        self.allmatchesdialog.set_default_size(700, 350)
        self.allmatchesdialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        kb_sh = menus.get_menu_item_kb_shortcut(self.dad, "toggle_show_allmatches_dlg")
        button_hide = self.allmatchesdialog.add_button(_("Hide (Restore with '%s')") % kb_sh, gtk.RESPONSE_CLOSE)
        button_hide.set_image(gtk.image_new_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_BUTTON))
        button_hide.connect('clicked', lambda x : self.allmatchesdialog.hide())
        # ROW: 0-node_id, 1-start_offset, 2-end_offset, 3-node_name, 4-line_content, 5-line_num 6-node_hier_name
        self.liststore = gtk.ListStore(long, long, long, str, str, int, str)
        self.treeview = gtk.TreeView(self.liststore)
        self.renderer_text_node = gtk.CellRendererText()
        self.renderer_text_linenum = gtk.CellRendererText()
        self.renderer_text_linecontent = gtk.CellRendererText()
        self.node_column = gtk.TreeViewColumn(_("Node Name"), self.renderer_text_node, text=3)
        self.treeview.append_column(self.node_column)
        self.linenum_column = gtk.TreeViewColumn(_("Line"), self.renderer_text_linenum, text=5)
        self.treeview.append_column(self.linenum_column)
        self.linecontent_column = gtk.TreeViewColumn(_("Line Content"), self.renderer_text_linecontent, text=4)
        self.treeview.append_column(self.linecontent_column)
        self.treeview.set_tooltip_column(6)
        self.treeviewselection = self.treeview.get_selection()
        self.treeview.connect('event-after', self.on_treeview_event_after)
        scrolledwindow_allmatches = gtk.ScrolledWindow()
        scrolledwindow_allmatches.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scrolledwindow_allmatches.add(self.treeview)
        content_area = self.allmatchesdialog.get_content_area()
        content_area.pack_start(scrolledwindow_allmatches)
        content_area.show_all()

    def on_treeview_event_after(self, treeview, event):
        """Catches mouse buttons clicks"""
        if event.type not in [gtk.gdk.BUTTON_PRESS, gtk.gdk.KEY_PRESS]: return
        model, list_iter = self.treeviewselection.get_selected()
        if not list_iter: return
        tree_iter = self.dad.get_tree_iter_from_node_id(model[list_iter][0])
        if not tree_iter:
            support.dialog_error(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)") % model[list_iter][0], self.dad.window)
            self.liststore.remove(list_iter)
            return
        self.dad.treeview_safe_set_cursor(tree_iter)
        self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(),
                                       self.dad.curr_buffer.get_iter_at_offset(model[list_iter][1]))
        self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(),
                                       self.dad.curr_buffer.get_iter_at_offset(model[list_iter][2]))
        self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
