# -*- coding: UTF-8 -*-
#
#       lists.py
#
#       Copyright 2009-2015 Giuseppe Penone <giuspen@gmail.com>
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
import re
import cons, support


class ListsHandler:
    """Handler of Bulleted and Numbered Lists"""

    def __init__(self, dad):
        """Lists Handler boot"""
        self.dad = dad

    def list_handler(self, target_list_num_id, text_buffer=None):
        """Unified Handler of Lists"""
        if not text_buffer: text_buffer = self.dad.curr_buffer
        if text_buffer.get_has_selection():
            iter_start, sel_end = text_buffer.get_selection_bounds()
            end_offset = sel_end.get_offset() - 1
        else:
            end_offset = 0
            iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
        # get info about the paragraph starting with iter_start ([Number, Whether multiple line, List Start Iter Offset])
        list_info = self.get_paragraph_list_info(iter_start)
        leading_num_count = 0
        while leading_num_count == 0 or new_par_offset < end_offset:
            leading_num_count += 1
            iter_start, iter_end = self.get_paragraph_iters(text_buffer=text_buffer, force_iter=iter_start)
            if not iter_start:
                if leading_num_count == 1:
                    # this is the first iteration
                    if list_info[0] != target_list_num_id:
                        # the target list type differs from this paragraph list type
                        iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
                        if target_list_num_id == -1: text_buffer.insert(iter_start, cons.CHAR_LISTTODO + cons.CHAR_SPACE)
                        elif target_list_num_id == 0: text_buffer.insert(iter_start, cons.CHAR_LISTBUL + cons.CHAR_SPACE)
                        else: text_buffer.insert(iter_start, "1. ")
                break
            if self.is_list_indented_continuation(iter_start):
                new_par_offset = iter_end.get_offset()
                leading_num_count -= 1
            else:
                iter_start, iter_end, chars_rm = self.list_check_n_remove_old_list_type_leading(iter_start, iter_end, text_buffer)
                end_offset -= chars_rm
                if list_info[0] != target_list_num_id:
                    # the target list type differs from this paragraph list type
                    if target_list_num_id == -1:
                        new_par_offset = iter_end.get_offset() + 2
                        end_offset += 2
                        text_buffer.insert(iter_start, cons.CHAR_LISTTODO + cons.CHAR_SPACE)
                    elif target_list_num_id == 0:
                        new_par_offset = iter_end.get_offset() + 2
                        end_offset += 2
                        text_buffer.insert(iter_start, cons.CHAR_LISTBUL + cons.CHAR_SPACE)
                    else:
                        leading_str = "%s. " % leading_num_count
                        new_par_offset = iter_end.get_offset() + len(leading_str)
                        end_offset += len(leading_str)
                        text_buffer.insert(iter_start, leading_str)
                else: new_par_offset = iter_end.get_offset()
            iter_start = text_buffer.get_iter_at_offset(new_par_offset+1)
            if not iter_start: break

    def list_check_n_remove_old_list_type_leading(self, iter_start, iter_end, text_buffer):
        """Clean List Start Characters"""
        start_offset = iter_start.get_offset()
        end_offset = iter_end.get_offset()
        list_info = self.get_paragraph_list_info(iter_start)
        if list_info[0] != None:
            leading_chars_num = self.get_leading_chars_num_from_list_info_num(list_info[0])
            iter_end = iter_start.copy()
            iter_end.forward_chars(leading_chars_num)
            text_buffer.delete(iter_start, iter_end)
            end_offset -= leading_chars_num
        else: leading_chars_num = 0
        return text_buffer.get_iter_at_offset(start_offset), text_buffer.get_iter_at_offset(end_offset), leading_chars_num

    def get_leading_chars_num_from_list_info_num(self, list_info_num):
        """From List Info to List Prefix"""
        if list_info_num > 0: return len("%s. " % list_info_num)
        return 2

    def list_get_number(self, iter_first_paragraph):
        """Returns a Number or None (0 fot the bulleted list)"""
        iter_start = iter_first_paragraph.copy()
        char = iter_start.get_char()
        if char == cons.CHAR_LISTBUL:
            return 0 if iter_start.forward_char() and iter_start.get_char() == cons.CHAR_SPACE else None
        if char in [cons.CHAR_LISTTODO, cons.CHAR_LISTDONEOK, cons.CHAR_LISTDONEFAIL]:
            return -1 if iter_start.forward_char() and iter_start.get_char() == cons.CHAR_SPACE else None
        match = re.match('[1-9]', char)
        if not match: return None
        number_str = char
        while iter_start.forward_char() and re.match('[0-9]', iter_start.get_char()):
            number_str += iter_start.get_char()
        return int(number_str) if iter_start.get_char() == "." and iter_start.forward_char() and iter_start.get_char() == cons.CHAR_SPACE else None

    def get_paragraph_list_info(self, iter_start):
        """Returns [Number, Whether multiple line, List Start Iter Offset]
           Number == 0 if bulleted list, >=1 if numbered list or None if not a list"""
        buffer_start = False
        # let's search for the paragraph start
        if iter_start.get_char() == cons.CHAR_NEWLINE:
            if not iter_start.backward_char(): buffer_start = True # if we are exactly on the paragraph end
        if not buffer_start:
            while iter_start:
                if iter_start.get_char() == cons.CHAR_NEWLINE: break # we got the previous paragraph start
                elif not iter_start.backward_char():
                    buffer_start = True
                    break # we reached the buffer start
        if not buffer_start: iter_start.forward_char()
        # get the number of the paragraph starting with iter_start
        number = self.list_get_number(iter_start)
        if number != None: return [number, False, iter_start.get_offset()] # multiple line = False
        elif not buffer_start and support.get_next_chars_from_iter_are(iter_start, [3*cons.CHAR_SPACE]):
            # we are inside of a list paragraph but after a shift+return
            iter_start.backward_char()
            list_info = self.get_paragraph_list_info(iter_start)
            return [list_info[0], True, list_info[2]] # multiple line = True
        return [None, None, None] # this paragraph is not a list

    def get_paragraph_iters(self, text_buffer=None, force_iter=None):
        """Generates and Returns two iters indicating the paragraph bounds"""
        if not text_buffer: text_buffer = self.dad.curr_buffer
        if not force_iter and text_buffer.get_has_selection():
            iter_start, iter_end = text_buffer.get_selection_bounds() # there's a selection
        else:
            # There's not a selection/iter forced
            if not force_iter: iter_start = text_buffer.get_iter_at_mark(text_buffer.get_insert())
            else: iter_start = force_iter.copy()
            iter_end = iter_start.copy()
            if iter_start.get_char() == cons.CHAR_NEWLINE:
                # we're upon a row end
                if not iter_start.backward_char(): return (None, None)
                if iter_start.get_char() == cons.CHAR_NEWLINE: return (None, None)
        while iter_end != None:
            char = iter_end.get_char()
            if char == cons.CHAR_NEWLINE: break # we got it
            elif not iter_end.forward_char(): break # we reached the buffer end
        while iter_start != None:
            char = iter_start.get_char()
            if char == cons.CHAR_NEWLINE: # we got it
                iter_start.forward_char() # step forward to the beginning of the new line
                break
            elif not iter_start.backward_char(): break # we reached the buffer start
        return (iter_start, iter_end)

    def is_list_indented_continuation(self, iter_in):
        """The given iter is the beginning of an indented list item"""
        if not iter_in: return False
        iter_start = iter_in.copy()
        if iter_start.get_char() != cons.CHAR_SPACE: return False
        if not iter_start.forward_char() or iter_start.get_char() != cons.CHAR_SPACE: return False
        if not iter_start.forward_char() or iter_start.get_char() != cons.CHAR_SPACE: return False
        return True

    def is_list_todo_beginning(self, square_bracket_open_iter):
        """Check if ☐ or ☑ or ☒"""
        text_iter = square_bracket_open_iter.copy()
        if text_iter.backward_char():
            if text_iter.get_char() == cons.CHAR_NEWLINE:
                text_iter.forward_char()
            else: return False
        if text_iter.get_char() in [cons.CHAR_LISTTODO, cons.CHAR_LISTDONEOK, cons.CHAR_LISTDONEFAIL]:
            return True
        return False

    def todo_list_rotate_status(self, todo_char_iter, text_buffer):
        """Rotate status between ☐ and ☑ and ☒"""
        iter_offset = todo_char_iter.get_offset()
        if todo_char_iter.get_char() == cons.CHAR_LISTTODO:
            text_buffer.delete(todo_char_iter, text_buffer.get_iter_at_offset(iter_offset+1))
            text_buffer.insert(text_buffer.get_iter_at_offset(iter_offset), cons.CHAR_LISTDONEOK)
        elif todo_char_iter.get_char() == cons.CHAR_LISTDONEOK:
            text_buffer.delete(todo_char_iter, text_buffer.get_iter_at_offset(iter_offset+1))
            text_buffer.insert(text_buffer.get_iter_at_offset(iter_offset), cons.CHAR_LISTDONEFAIL)
        elif todo_char_iter.get_char() == cons.CHAR_LISTDONEFAIL:
            text_buffer.delete(todo_char_iter, text_buffer.get_iter_at_offset(iter_offset+1))
            text_buffer.insert(text_buffer.get_iter_at_offset(iter_offset), cons.CHAR_LISTTODO)

    def char_iter_forward_to_newline(self, char_iter):
        """Forwards char iter to line end"""
        if not char_iter.forward_char(): return False
        while char_iter.get_char() != cons.CHAR_NEWLINE:
            if not char_iter.forward_char(): return False
        return True

    def todo_lists_old_to_new_conversion(self, text_buffer):
        """Conversion of todo lists from old to new type for a node"""
        curr_iter = text_buffer.get_start_iter()
        keep_cleaning = False
        first_line = True
        while curr_iter:
            fw_needed = True
            if first_line or curr_iter.get_char() == cons.CHAR_NEWLINE and curr_iter.forward_char():
                first_line = False
                if keep_cleaning:
                    iter_bis = curr_iter.copy()
                    if iter_bis.get_char() == cons.CHAR_SPACE and iter_bis.forward_char()\
                    and iter_bis.get_char() == cons.CHAR_SPACE and iter_bis.forward_char()\
                    and iter_bis.get_char() == cons.CHAR_SPACE:
                        no_stop = self.char_iter_forward_to_newline(curr_iter)
                        text_buffer.remove_all_tags(iter_bis, curr_iter)
                        if no_stop: continue
                        else: break
                    else: keep_cleaning = False
                if curr_iter.get_char() == cons.CHAR_SQ_BR_OPEN and curr_iter.forward_char()\
                and curr_iter.get_char() in [cons.CHAR_SPACE, "X"]:
                    middle_char = curr_iter.get_char()
                    if curr_iter.forward_char() and curr_iter.get_char() == cons.CHAR_SQ_BR_CLOSE\
                    and curr_iter.forward_char():
                        first_iter = curr_iter.copy()
                        first_iter.backward_chars(3)
                        iter_offset = first_iter.get_offset()
                        text_buffer.delete(first_iter, curr_iter)
                        todo_char = cons.CHAR_LISTTODO if middle_char == cons.CHAR_SPACE else cons.CHAR_LISTDONEOK
                        text_buffer.insert(text_buffer.get_iter_at_offset(iter_offset), todo_char)
                        curr_iter = text_buffer.get_iter_at_offset(iter_offset)
                        if middle_char != cons.CHAR_SPACE:
                            first_iter = curr_iter.copy()
                            no_stop = self.char_iter_forward_to_newline(curr_iter)
                            #print "%s(%s),%s(%s)" % (first_iter.get_char(), first_iter.get_offset(), curr_iter.get_char(), curr_iter.get_offset())
                            text_buffer.remove_all_tags(first_iter, curr_iter)
                            keep_cleaning = True
                            if no_stop: continue
                            else: break
                else: fw_needed = False
            if fw_needed and not self.char_iter_forward_to_newline(curr_iter): break
