# -*- coding: UTF-8 -*-
#
#       findreplace.py
#       
#       Copyright 2010 Giuseppe Penone <giuspen@gmail.com>
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
      self.curr_find = [None, ""] # [latest find type, latest find pattern]
      self.from_find_iterated = False
   
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
      forward = self.dad.glade.search_fw_radiobutton.get_property("active")
      first_fromsel = self.dad.glade.search_first_fromsel_radiobutton.get_property("active")
      all_matches = self.dad.glade.search_all_radiobutton.get_property("active")
      self.matches_num = 0
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
      current_cursor_pos = self.dad.curr_buffer.get_property('cursor-position')
      forward = self.dad.glade.search_fw_radiobutton.get_property("active")
      first_fromsel = self.dad.glade.search_first_fromsel_radiobutton.get_property("active")
      all_matches = self.dad.glade.search_all_radiobutton.get_property("active")
      if first_fromsel:
         self.first_useful_node = False # no one node content was parsed yet
         node_iter = self.dad.curr_tree_iter.copy()
      else:
         self.first_useful_node = True # all range will be parsed so no matter
         if forward: node_iter = self.dad.treestore.get_iter_first()
         else: node_iter = self.dad.get_tree_iter_last_sibling(None)
      self.matches_num = 0
      if all_matches: self.liststore_create_or_clean()
      self.dad.expanded_collapsed_string = config.get_tree_expanded_collapsed_string(self.dad)
      # searching start
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
      elif all_matches:
         config.set_tree_expanded_collapsed_string(self.dad)
         self.dad.glade.allmatchesdialog.set_title(str(self.matches_num) + cons.CHAR_SPACE + _("Matches"))
         self.dad.glade.allmatchesdialog.run()
         self.dad.glade.allmatchesdialog.hide()
   
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
      forward = self.dad.glade.search_fw_radiobutton.get_property("active")
      first_fromsel = self.dad.glade.search_first_fromsel_radiobutton.get_property("active")
      all_matches = self.dad.glade.search_all_radiobutton.get_property("active")
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
      if forward:
         match = pattern.search(text, start_iter.get_offset())
      else:
         match = None
         for temp_match in pattern.finditer(text, 0, start_iter.get_offset()): match = temp_match
      if match:
         num_objs = self.get_num_objs_before_offset(match.start())
         target = self.dad.curr_buffer.get_iter_at_offset(match.start() + num_objs)
         self.dad.curr_buffer.place_cursor(target)
         target.forward_chars(match.end() - match.start())
         self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(), target)
         self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), 0.25)
         if all_matches:
            self.liststore.append([self.dad.curr_tree_iter,
                                   match.start() + num_objs,
                                   match.end() + num_objs,
                                   self.dad.treestore[self.dad.curr_tree_iter][1],
                                   self.get_line_content(target)])
         if self.replace_active:
            replacer_text = self.dad.glade.replace_entry.get_text().decode("utf-8")
            self.dad.curr_buffer.delete_selection(interactive=False, default_editable=True)
            self.dad.curr_buffer.insert_at_cursor(replacer_text)
            self.dad.state_machine.update_state(self.dad.treestore[self.dad.curr_tree_iter][3])
         return True
      else: return False
   
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
      if (first_fromsel and first_node)\
      or (all_matches and not self.all_matches_first_in_node):
         iter_insert = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert())
         iter_bound = self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_selection_bound())
         if not self.replace_active:
            # it's a find, so we want, given a selected word, to find for the subsequent one
            if forward:
               if iter_bound != None and iter_insert.compare(iter_bound) < 0: start_iter = iter_bound
               else: start_iter = iter_insert
            else:
               if iter_bound != None and iter_insert.compare(iter_bound) > 0: start_iter = iter_bound
               else: start_iter = iter_insert
         else:
            # it's a replace, so we want, given a selected word, to replace starting from this one
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
      return self.find_pattern(pattern, start_iter, forward, all_matches)
   
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
      node_iter = self.dad.treestore.iter_children(node_iter) # check for childrens
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
      text = self.dad.treestore[node_iter][1].decode("utf-8")
      match = pattern.search(text)
      if match != None:
         if all_matches:
            self.liststore.append([node_iter,
                                   0,
                                   0,
                                   self.dad.treestore[node_iter][1],
                                   self.get_first_line_content(self.dad.treestore[node_iter][2])])
         if self.replace_active:
            replacer_text = self.dad.glade.replace_entry.get_text().decode("utf-8")
            text = text.replace(self.curr_find[1], replacer_text)
            self.dad.treestore[node_iter][1] = text
         if not all_matches:
            self.dad.treeview_safe_set_cursor(node_iter)
            self.dad.sourceview.grab_focus()
         return True
      node_iter = self.dad.treestore.iter_children(node_iter) # check for childrens
      if node_iter != None and not forward: node_iter = self.dad.get_tree_iter_last_sibling(node_iter)
      while node_iter != None:
         if self.parse_node_name(node_iter, pattern, forward, all_matches):
            self.matches_num += 1
            if not all_matches: return True
         if forward: node_iter = self.dad.treestore.iter_next(node_iter)
         else: node_iter = self.dad.get_tree_iter_prev_sibling(self.dad.treestore, node_iter)
      return False
   
   def find_back(self):
      """Continue the previous search (a_node/in_selected_node/in_all_nodes) but in Opposite Direction"""
      if self.dad.glade.search_fw_radiobutton.get_property("active") == True:
         self.dad.glade.search_bw_radiobutton.set_property("active", True)
         self.find_again()
         self.dad.glade.search_fw_radiobutton.set_property("active", True)
      else:
         self.dad.glade.search_fw_radiobutton.set_property("active", True)
         self.find_again()
         self.dad.glade.search_bw_radiobutton.set_property("active", True)
      
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
      self.find_again()
      self.replace_active = False
      
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
      # ROW: 0-node_iter, 1-start_offset, 2-end_offset, 3-node_name, 4-line_content
      self.liststore = gtk.ListStore(gobject.TYPE_PYOBJECT, long, long, str, str)
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
      self.dad.treeview_safe_set_cursor(model[list_iter][0])
      if model[list_iter][1] != 0 and model[list_iter][2] != 0:
         self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_insert(),
                                        self.dad.curr_buffer.get_iter_at_offset(model[list_iter][1]))
         self.dad.curr_buffer.move_mark(self.dad.curr_buffer.get_selection_bound(),
                                        self.dad.curr_buffer.get_iter_at_offset(model[list_iter][2]))
         self.dad.sourceview.scroll_to_mark(self.dad.curr_buffer.get_insert(), 0.25)
      else: self.dad.sourceview.grab_focus()
