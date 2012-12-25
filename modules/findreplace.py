# -*- coding: UTF-8 -*-
#
#       findreplace.py
#
#       Copyright 2009-2013 Giuseppe Penone <giuspen@gmail.com>
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

import gtk, gobject
import re
import cons, support, config


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
    
    def iterated_find_dialog(self):
        """Iterated Find/Replace Dialog"""
        dialog = self.dad.glade.iteratedfinddialog
        def on_key_press_iterated_find_dialog(widget, event):
            if gtk.gdk.keyval_name(event.keyval) == "Return":
                dialog.get_widget_for_response(1).clicked()
                return True
        dialog.connect("key_press_event", on_key_press_iterated_find_dialog)
        response = dialog.run()
        dialog.hide()
        if response == 1:
            # find forward
            self.replace_active = False
            self.find_again()
        elif response == 2:
            # replace
            self.replace_active = True
            self.replace_subsequent = True
            self.find_again()
            self.replace_subsequent = False
        elif response == 3:
            # undo replace
            self.dad.requested_step_back()
            self.iterated_find_dialog()
        elif response == 4:
            # find backward
            self.find_back()
    
    def find_in_selected_node(self):
        """Search for a pattern in the selected Node"""
        entry_hint = ""
        if not self.from_find_iterated:
            iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
            iter_bound = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_selection_bound())
            entry_predefined_text = self.dad.curr_buffer.get_text(iter_insert, iter_bound)
            if not self.replace_active:
                pattern = self.dad.dialog_input(title=_("Search in Current Node..."),
                                            entry_hint=entry_predefined_text,
                                            search_opt=True)
            else:
                pattern = self.dad.dialog_input(title=_("Replace in Current Node..."),
                                            entry_hint=entry_predefined_text,
                                            search_opt=True, replace_opt=True)
            if entry_predefined_text != "":
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(), iter_insert)
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(), iter_bound)
            if pattern != None: self.curr_find = ["in_selected_node", pattern]
            else: return
        else: pattern = self.curr_find[1]
        forward = self.dad.glade.search_fw_radiobutton.get_active()
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.dad.glade.search_first_fromsel_radiobutton.get_active()
        all_matches = self.dad.glade.search_all_radiobutton.get_active()
        self.matches_num = 0
        # searching start
        if self.dad.user_active:
            self.dad.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        if all_matches:
            self.liststore_create_or_clean()
            self.all_matches_first_in_node = True
            while self.parse_current_node_content(pattern, forward, first_fromsel, all_matches, True):
                self.matches_num += 1
        elif self.parse_current_node_content(pattern, forward, first_fromsel, all_matches, True):
            self.matches_num = 1
        if self.matches_num == 0:
            support.dialog_info(_("The pattern '%s' was not found") % pattern, self.dad.window)
        elif all_matches:
            self.dad.glade.allmatchesdialog.set_title(str(self.matches_num) + cons.CHAR_SPACE + _("Matches"))
            self.dad.glade.allmatchesdialog.run()
            self.dad.glade.allmatchesdialog.hide()
        elif self.dad.glade.checkbutton_iterated_find_dialog.get_active():
            self.iterated_find_dialog()
        if user_active_restore: self.dad.user_active = True

    def find_in_all_nodes(self):
        """Search for a pattern in all the Tree Nodes"""
        if not self.from_find_iterated:
            iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
            iter_bound = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_selection_bound())
            entry_predefined_text = self.dad.curr_buffer.get_text(iter_insert, iter_bound)
            if not self.replace_active:
                pattern = self.dad.dialog_input(title=_("Search in All Nodes..."),
                                            entry_hint=entry_predefined_text,
                                            search_opt=True)
            else:
                pattern = self.dad.dialog_input(title=_("Replace in All Nodes..."),
                                            entry_hint=entry_predefined_text,
                                            search_opt=True, replace_opt=True)
            if entry_predefined_text != "":
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(), iter_insert)
                self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(), iter_bound)
            if pattern != None: self.curr_find = ["in_all_nodes", pattern]
            else: return
        else: pattern = self.curr_find[1]
        starting_tree_iter = self.dad.curr_tree_iter.copy()
        current_cursor_pos = self.dad.curr_buffer.get_property(cons.STR_CURSOR_POSITION)
        forward = self.dad.glade.search_fw_radiobutton.get_active()
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.dad.glade.search_first_fromsel_radiobutton.get_active()
        all_matches = self.dad.glade.search_all_radiobutton.get_active()
        if first_fromsel:
            self.first_useful_node = False # no one node content was parsed yet
            node_iter = self.dad.curr_tree_iter.copy()
        else:
            self.first_useful_node = True # all range will be parsed so no matter
            if forward: node_iter = self.dad.treestore.get_iter_first()
            else: node_iter = self.dad.get_tree_iter_last_sibling(None)
        self.matches_num = 0
        if all_matches: self.liststore_create_or_clean()
        config.get_tree_expanded_collapsed_string(self.dad)
        # searching start
        if self.dad.user_active:
            self.dad.user_active = False
            user_active_restore = True
        else: user_active_restore = False
        while node_iter:
            self.all_matches_first_in_node = True
            while self.parse_given_node_content(node_iter, pattern, forward, first_fromsel, all_matches):
                self.matches_num += 1
                if not all_matches: break
            if self.matches_num == 1 and not all_matches: break
            last_top_node_iter = node_iter.copy() # we need this if we start from a node that is not in top level
            if forward: node_iter = self.dad.treestore.iter_next(node_iter)
            else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
            # code that, in case we start from a node that is not top level, climbs towards the top
            while not node_iter:
                node_iter = self.dad.treestore.iter_parent(last_top_node_iter)
                if node_iter:
                    last_top_node_iter = node_iter.copy()
                    # we do not check the father on purpose, only the uncles in the proper direction
                    if forward: node_iter = self.dad.treestore.iter_next(node_iter)
                    else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
                else: break
        if self.matches_num == 0:
            config.set_tree_expanded_collapsed_string(self.dad)
            support.dialog_info(_("The pattern '%s' was not found") % pattern, self.dad.window)
            self.dad.treeview_safe_set_cursor(starting_tree_iter)
            self.dad.sourceview.grab_focus()
            self.dad.curr_buffer.place_cursor(self.dad.curr_buffer.get_iter_at_offset(current_cursor_pos))
            self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), 0.3)
        else:
            config.set_tree_expanded_collapsed_string(self.dad)
            if all_matches:
                self.dad.glade.allmatchesdialog.set_title(str(self.matches_num) + cons.CHAR_SPACE + _("Matches"))
                self.dad.glade.allmatchesdialog.run()
                self.dad.glade.allmatchesdialog.hide()
            else:
                self.dad.treeview_safe_set_cursor(self.dad.curr_tree_iter)
                self.dad.sourceview.grab_focus()
                self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), 0.3)
                if self.dad.glade.checkbutton_iterated_find_dialog.get_active():
                    self.iterated_find_dialog()
        if user_active_restore: self.dad.user_active = True

    def find_a_node(self, *args):
        """Search for a pattern between all the Node's Names"""
        if not self.from_find_iterated:
            if not self.replace_active:
                pattern_clean = self.dad.dialog_input(title=_("Search For a Node Name..."), search_opt=True)
            else:
                pattern_clean = self.dad.dialog_input(title=_("Replace in Node Names..."), search_opt=True, replace_opt=True)
            if pattern_clean != None: self.curr_find = ["a_node", pattern_clean]
            else: return
        else: pattern_clean = self.curr_find[1]
        if not self.dad.glade.checkbutton_re.get_active(): # NOT REGULAR EXPRESSION
            pattern_ready = re.escape(pattern_clean) # backslashes all non alphanum chars => to not spoil re
            if self.dad.glade.checkbutton_whole_word.get_active(): # WHOLE WORD
                pattern_ready = r'\b' + pattern_ready + r'\b'
            elif self.dad.glade.checkbutton_start_word.get_active(): # START WORD
                pattern_ready = r'\b' + pattern_ready
        else: pattern_ready = pattern_clean
        if self.dad.glade.checkbutton_match_case.get_active(): # CASE SENSITIVE
            pattern = re.compile(pattern_ready, re.UNICODE|re.MULTILINE)
        else: pattern = re.compile(pattern_ready, re.IGNORECASE|re.UNICODE|re.MULTILINE)
        forward = self.dad.glade.search_fw_radiobutton.get_active()
        if self.from_find_back:
            forward = not forward
            self.from_find_back = False
        first_fromsel = self.dad.glade.search_first_fromsel_radiobutton.get_active()
        all_matches = self.dad.glade.search_all_radiobutton.get_active()
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
        if all_matches: self.liststore_create_or_clean()
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
                    # we do not check the father on purpose, only the uncles in the proper direction
                    if forward: node_iter = self.dad.treestore.iter_next(node_iter)
                    else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
                else: break
        if self.matches_num == 0:
            support.dialog_info(_("The pattern '%s' was not found") % pattern_clean, self.dad.window)
        elif all_matches:
            self.dad.glade.allmatchesdialog.set_title(str(self.matches_num) + cons.CHAR_SPACE + _("Matches"))
            self.dad.glade.allmatchesdialog.run()
            self.dad.glade.allmatchesdialog.hide()
        elif self.dad.glade.checkbutton_iterated_find_dialog.get_active():
            self.iterated_find_dialog()
        if self.matches_num and self.replace_active: self.dad.update_window_save_needed()

    def find_pattern(self, pattern, start_iter, forward, all_matches):
        """Returns (start_iter, end_iter) or (None, None)"""
        text = self.dad.curr_buffer.get_text(*self.dad.curr_buffer.get_bounds()).decode("utf-8")
        if not self.dad.glade.checkbutton_re.get_active(): # NOT REGULAR EXPRESSION
            pattern = re.escape(pattern) # backslashes all non alphanum chars => to not spoil re
            if self.dad.glade.checkbutton_whole_word.get_active(): # WHOLE WORD
                pattern = r'\b' + pattern + r'\b'
            elif self.dad.glade.checkbutton_start_word.get_active(): # START WORD
                pattern = r'\b' + pattern
        if self.dad.glade.checkbutton_match_case.get_active(): # CASE SENSITIVE
            pattern = re.compile(pattern, re.UNICODE|re.MULTILINE)
        else: pattern = re.compile(pattern, re.IGNORECASE|re.UNICODE|re.MULTILINE)
        start_offset = start_iter.get_offset()
        start_offset -= self.get_num_objs_before_offset(start_offset)
        if forward:
            match = pattern.search(text, start_offset)
        else:
            match = None
            for temp_match in pattern.finditer(text, 0, start_offset): match = temp_match
        if match:
            if self.replace_active: obj_match_offsets = (None, None)
            else: obj_match_offsets = self.check_pattern_in_object_between(pattern,
                                                                           start_iter.get_offset(),
                                                                           match.start(),
                                                                           forward)
            if obj_match_offsets[0] != None: match_offsets = (obj_match_offsets[0], obj_match_offsets[1])
            else: match_offsets = (match.start(), match.end())
        else:
            if self.replace_active: obj_match_offsets = (None, None)
            else: obj_match_offsets = self.check_pattern_in_object_between(pattern,
                                                                           start_iter.get_offset(),
                                                                           -1,
                                                                           forward)
            if obj_match_offsets[0] != None: match_offsets = (obj_match_offsets[0], obj_match_offsets[1])
            else: match_offsets = (None, None)
        if match_offsets[0] != None:
            if obj_match_offsets[0] == None: num_objs = self.get_num_objs_before_offset(match_offsets[0])
            else: num_objs = 0
            final_start_offset = match_offsets[0] + num_objs
            final_delta_offset = match_offsets[1] - match_offsets[0]
            #print "IN", final_start_offset, final_delta_offset, self.dad.treestore[self.dad.curr_tree_iter][1]
            #for count in range(final_delta_offset):
            #    print count, self.dad.curr_buffer.get_iter_at_offset(final_start_offset+count).get_char()
            self.dad.set_selection_at_offset_n_delta(final_start_offset, final_delta_offset)
            #print "OUT"
            mark_insert = self.dad.curr_buffer.get_insert()
            iter_insert = self.dad.curr_buffer.get_iter_at_mark(mark_insert)
            self.dad.sourceview.scroll_to_mark(mark_insert, 0.25)
            if all_matches:
                if self.newline_trick: newline_trick_offset = 1
                else: newline_trick_offset = 0
                self.liststore.append([self.dad.treestore[self.dad.curr_tree_iter][3],
                                       match_offsets[0] + num_objs - newline_trick_offset,
                                       match_offsets[1] + num_objs - newline_trick_offset,
                                       self.dad.treestore[self.dad.curr_tree_iter][1],
                                       self.get_line_content(iter_insert) if obj_match_offsets[0] == None else obj_match_offsets[2]])
            if self.replace_active:
                replacer_text = self.dad.glade.replace_entry.get_text().decode("utf-8")
                self.dad.curr_buffer.delete_selection(interactive=False, default_editable=True)
                self.dad.curr_buffer.insert_at_cursor(replacer_text)
                if not all_matches:
                    self.dad.set_selection_at_offset_n_delta(match_offsets[0] + num_objs,
                                                             len(replacer_text))
                self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])
                self.dad.ctdb_handler.pending_edit_db_node_buff(self.dad.treestore[self.dad.curr_tree_iter][3], force_user_active=True)
            return True
        return False
    
    def check_pattern_in_object(self, pattern, obj):
        """Search for the pattern in the given object"""
        if obj[0] == "table":
            for row in obj[1][1]['matrix']:
                for col in row:
                    if pattern.search(col): return (True, "<table>")
        elif obj[0] == "codebox":
            if pattern.search(obj[1][1]['fill_text']): return (True, "<codebox>")
        return (False, "")

    def check_pattern_in_object_between(self, pattern, start_offset, end_offset, forward):
        """Search for the pattern in the given slice and direction"""
        if not forward: start_offset -= 1
        if end_offset < 0:
            end_offset = self.dad.curr_buffer.get_end_iter().get_offset() if forward else 0
        sel_range = (start_offset, end_offset) if forward else (end_offset, start_offset)
        obj_vec = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(self.dad.curr_buffer,
                                                                               sel_range=sel_range)
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

    def get_num_objs_before_offset(self, max_offset):
        """Returns the num of objects from buffer start to the given offset"""
        num_objs = 0
        local_limit_offset = max_offset
        curr_iter = self.dad.curr_buffer.get_start_iter()
        while curr_iter.get_offset() < local_limit_offset:
            anchor = curr_iter.get_child_anchor()
            if anchor:
                num_objs += 1
                local_limit_offset += 1
            if not curr_iter.forward_char(): break
        return num_objs

    def parse_current_node_content(self, pattern, forward, first_fromsel, all_matches, first_node):
        """Returns True if pattern was find, False otherwise"""
        buff_start_iter = self.dad.curr_buffer.get_start_iter()
        if buff_start_iter.get_char() != cons.CHAR_NEWLINE:
            self.newline_trick = True
            if not self.dad.curr_buffer.get_modified(): restore_modified = True
            else: restore_modified = False
            self.dad.curr_buffer.insert(buff_start_iter, cons.CHAR_NEWLINE)
        else:
            self.newline_trick = False
            restore_modified = False
        if (first_fromsel and first_node)\
        or (all_matches and not self.all_matches_first_in_node):
            iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
            iter_bound = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_selection_bound())
            if not self.replace_active or self.replace_subsequent:
                # it's a find or subsequent replace, so we want, given a selected word, to find for the subsequent one
                if forward:
                    if iter_bound != None and iter_insert.compare(iter_bound) < 0: start_iter = iter_bound
                    else: start_iter = iter_insert
                else:
                    if iter_bound != None and iter_insert.compare(iter_bound) > 0: start_iter = iter_bound
                    else: start_iter = iter_insert
            else:
                # it's a first replace, so we want, given a selected word, to replace starting from this one
                if forward:
                    if iter_bound != None and iter_insert.compare(iter_bound) > 0: start_iter = iter_bound
                    else: start_iter = iter_insert
                else:
                    if iter_bound != None and iter_insert.compare(iter_bound) < 0: start_iter = iter_bound
                    else: start_iter = iter_insert
        else:
            if forward: start_iter = self.dad.curr_buffer.get_start_iter()
            else: start_iter = self.dad.curr_buffer.get_end_iter()
            if all_matches: self.all_matches_first_in_node = False
        pattern_found = self.find_pattern(pattern, start_iter, forward, all_matches)
        if self.newline_trick:
            buff_start_iter = self.dad.curr_buffer.get_start_iter()
            buff_step_iter = buff_start_iter.copy()
            if buff_step_iter.forward_char(): self.dad.curr_buffer.delete(buff_start_iter, buff_step_iter)
            if restore_modified: self.dad.curr_buffer.set_modified(False)
        if self.replace_active and pattern_found:
            self.dad.update_window_save_needed("nbuf")
        return pattern_found

    def parse_given_node_content(self, node_iter, pattern, forward, first_fromsel, all_matches):
        """Returns True if pattern was found, False otherwise"""
        node_path = self.dad.treestore.get_path(node_iter)
        if not self.first_useful_node:
            # first_fromsel plus first_node not already parsed
            if self.dad.curr_tree_iter == None or node_path == self.dad.treestore.get_path(self.dad.curr_tree_iter):
                self.first_useful_node = True # a first_node was parsed
                self.dad.treeview_safe_set_cursor(node_iter)
                if self.parse_current_node_content(pattern, forward, first_fromsel, all_matches, True): return True # first_node node, first_fromsel
        else:
            # not first_fromsel or first_fromsel with first_node already parsed
            self.dad.treeview_safe_set_cursor(node_iter)
            if self.parse_current_node_content(pattern, forward, first_fromsel, all_matches, False): return True # not first_node node
        node_iter = self.dad.treestore.iter_children(node_iter) # check for children
        if node_iter != None and not forward: node_iter = self.dad.get_tree_iter_last_sibling(node_iter)
        while node_iter != None:
            self.all_matches_first_in_node = True
            while self.parse_given_node_content(node_iter, pattern, forward, first_fromsel, all_matches):
                self.matches_num += 1
                if not all_matches: break
            if self.matches_num == 1 and not all_matches: break
            if forward: node_iter = self.dad.treestore.iter_next(node_iter)
            else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
        return False

    def parse_node_name(self, node_iter, pattern, forward, all_matches):
        """Recursive function that searchs for the given pattern"""
        text_name = self.dad.treestore[node_iter][1].decode("utf-8")
        match = pattern.search(text_name)
        if not match:
            text_tags = self.dad.treestore[node_iter][6].decode("utf-8")
            match = pattern.search(text_tags)
        if match:
            if all_matches:
                self.liststore.append([self.dad.treestore[node_iter][3],
                                       0,
                                       0,
                                       self.dad.treestore[node_iter][1],
                                       self.get_first_line_content(self.dad.get_textbuffer_from_tree_iter(node_iter))])
            if self.replace_active:
                replacer_text = self.dad.glade.replace_entry.get_text().decode("utf-8")
                text_name = text_name.replace(self.curr_find[1], replacer_text)
                self.dad.treestore[node_iter][1] = text_name
                self.ctdb_handler.pending_edit_db_node_prop(self.dad.treestore[node_iter][3])
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

    def replace_in_all_nodes(self):
        """Replace the pattern in all the Tree Nodes"""
        self.replace_active = True
        self.find_in_all_nodes()
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
        elif self.curr_find[0] == "in_all_nodes": self.find_in_all_nodes()
        elif self.curr_find[0] == "a_node": self.find_a_node()
        self.from_find_iterated = False

    def replace_again(self):
        """Continue the previous replace (a_node/in_selected_node/in_all_nodes)"""
        self.replace_active = True
        self.replace_subsequent = True
        self.find_again()
        self.replace_active = False
        self.replace_subsequent = False

    def get_line_content(self, text_iter):
        """Returns the Line Content Given the Text Iter"""
        line_start = text_iter.copy()
        line_end = text_iter.copy()
        if not line_start.backward_char(): return ""
        while line_start.get_char() != cons.CHAR_NEWLINE:
            if not line_start.backward_char(): break
        else: line_start.forward_char()
        while line_end.get_char() != cons.CHAR_NEWLINE:
            if not line_end.forward_char(): break
        return self.dad.curr_buffer.get_text(line_start, line_end)

    def get_first_line_content(self, text_buffer):
        """Returns the First Not Empty Line Content Given the Text Buffer"""
        start_iter = text_buffer.get_start_iter()
        while start_iter.get_char() == cons.CHAR_NEWLINE:
            if not start_iter.forward_char(): return ""
        end_iter = start_iter.copy()
        while end_iter.get_char() != cons.CHAR_NEWLINE:
            if not end_iter.forward_char(): break
        return text_buffer.get_text(start_iter, end_iter)

    def liststore_create_or_clean(self):
        """Check Whether the Liststore was Already Created or Not"""
        if "liststore" in dir(self):
            self.liststore.clear()
            return
        # ROW: 0-node_id, 1-start_offset, 2-end_offset, 3-node_name, 4-line_content
        self.liststore = gtk.ListStore(long, long, long, str, str)
        self.treeview = gtk.TreeView(self.liststore)
        self.renderer_text_node = gtk.CellRendererText()
        self.renderer_text_line = gtk.CellRendererText()
        self.node_column = gtk.TreeViewColumn(_("Node Name"), self.renderer_text_node, text=3)
        self.treeview.append_column(self.node_column)
        self.line_column = gtk.TreeViewColumn(_("Line Content"), self.renderer_text_line, text=4)
        self.treeview.append_column(self.line_column)
        self.treeviewselection = self.treeview.get_selection()
        self.treeview.connect('event-after', self.on_treeview_event_after)
        self.dad.glade.scrolledwindow_allmatches.add(self.treeview)
        self.dad.glade.scrolledwindow_allmatches.show_all()

    def on_treeview_event_after(self, treeview, event):
        """Catches mouse buttons clicks"""
        if event.type != gtk.gdk.BUTTON_PRESS: return
        model, list_iter = self.treeviewselection.get_selected()
        tree_iter = self.dad.get_tree_iter_from_node_id(model[list_iter][0])
        if not tree_iter:
            support.dialog_error(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)") % model[list_iter][0], self.dad.window)
            self.liststore.remove(list_iter)
            return
        self.dad.treeview_safe_set_cursor(tree_iter)
        if model[list_iter][1] != 0 and model[list_iter][2] != 0:
            self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(),
                                           self.dad.curr_buffer.get_iter_at_offset(model[list_iter][1]))
            self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(),
                                           self.dad.curr_buffer.get_iter_at_offset(model[list_iter][2]))
            self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), 0.25)
        else: self.dad.sourceview.grab_focus()
