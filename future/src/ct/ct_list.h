/*
 * ct_list.h
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#pragma once

#include <gtkmm/textbuffer.h>
#include <glibmm/refptr.h>
#include "ct_types.h"

struct CtTextRange
{
    Gtk::TextIter iter_start;
    Gtk::TextIter iter_end;
    int leading_chars_num = 0;
};

class CtMainWin;

class CtList
{
public:
    CtList(CtMainWin* pCtMainWin,
           Glib::RefPtr<Gtk::TextBuffer> curr_buffer)
     : _pCtMainWin(pCtMainWin),
       _curr_buffer(curr_buffer) {}

    void        list_handler(CtListType target_list_num_id);
    CtTextRange list_check_n_remove_old_list_type_leading(Gtk::TextIter iter_start, Gtk::TextIter iter_end);
    int         get_leading_chars_num(CtListType type, int list_info_num);
    CtListInfo  list_get_number_n_level(Gtk::TextIter iter_first_paragraph);
    int         get_multiline_list_element_end_offset(Gtk::TextIter curr_iter, CtListInfo list_info);
    CtListInfo  get_prev_list_info_on_level(Gtk::TextIter iter_start, int level);
    CtListInfo  get_next_list_info_on_level(Gtk::TextIter iter_start, int level);
    CtListInfo  get_paragraph_list_info(Gtk::TextIter iter_start_orig);
    CtTextRange get_paragraph_iters(Gtk::TextIter* force_iter = nullptr);
    bool        is_list_todo_beginning(Gtk::TextIter square_bracket_open_iter);
    void        todo_list_rotate_status(Gtk::TextIter todo_char_iter);
    bool        char_iter_forward_to_newline(Gtk::TextIter& char_iter);
    bool        char_iter_backward_to_newline(Gtk::TextIter& char_iter);
    void        todo_lists_old_to_new_conversion();

private:
    CtMainWin* _pCtMainWin;
    Glib::RefPtr<Gtk::TextBuffer> _curr_buffer;
};
