/*
 * ct_list.cc
 *
 * Copyright 2009-2021
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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

#include "ct_main_win.h"
#include "ct_list.h"
#include "ct_const.h"
#include <gtkmm/textbuffer.h>

/*
std::string CtList::get_list_type(int list_num_id)
{
    if (list_num_id > 0)  return "Numbered";
    if (list_num_id < 0)  return "Bulleted";
    else                  return "Todo";
}
*/

// Unified Handler of Lists
void CtList::list_handler(CtListType target_list_num_id)
{
    struct LevelCount {
        int level;
        int count;
    };

    CtTextRange range;
    int end_offset;
    int new_par_offset = -1;
    std::list<LevelCount> leading_num_count;

    if (_curr_buffer->get_has_selection()) {
        Gtk::TextIter sel_end;
        _curr_buffer->get_selection_bounds(range.iter_start, sel_end);
        end_offset = sel_end.get_offset() - 2;
    }
    else {
        end_offset = 0;
        range.iter_start = _curr_buffer->get_insert()->get_iter();
    }
    while (new_par_offset < end_offset) {
        // std::cout << new_par_offset << " " << end_offset;
        range = get_paragraph_iters(&range.iter_start);
        if (!range.iter_start) {
            // empty line
            if (leading_num_count.empty()) {
                // this is the first iteration
                range.iter_start = _curr_buffer->get_insert()->get_iter();
                if (target_list_num_id == CtListType::Todo)        _curr_buffer->insert(range.iter_start, _pCtMainWin->get_ct_config()->charsTodo[0] + CtConst::CHAR_SPACE);
                else if (target_list_num_id == CtListType::Bullet) _curr_buffer->insert(range.iter_start, _pCtMainWin->get_ct_config()->charsListbul[0] + CtConst::CHAR_SPACE);
                else                                                          _curr_buffer->insert(range.iter_start, "1. ");
            }
            break;
        }
        CtListInfo list_info = get_paragraph_list_info(range.iter_start);
        // print list_info
        if (list_info and range.iter_start.get_offset() != list_info.startoffs)
            new_par_offset = range.iter_end.get_offset();
        else {
            range = list_check_n_remove_old_list_type_leading(range.iter_start, range.iter_end);
            end_offset -= range.leading_chars_num;
            if (!list_info or list_info.type != target_list_num_id) {
                // the target list type differs from this paragraph list type
                while (CtTextIterUtil::startswith(range.iter_start, Glib::ustring(3, CtConst::CHAR_SPACE[0]).c_str()))
                    range.iter_start.forward_chars(3);
                if (target_list_num_id == CtListType::Todo) {
                    new_par_offset = range.iter_end.get_offset() + 2;
                    end_offset += 2;
                    _curr_buffer->insert(range.iter_start, _pCtMainWin->get_ct_config()->charsTodo[0] + CtConst::CHAR_SPACE);
                }
                else if (target_list_num_id == CtListType::Bullet) {
                    new_par_offset = range.iter_end.get_offset() + 2;
                    end_offset += 2;
                    int bull_idx = (!list_info) ? 0 : (list_info.level % (int)_pCtMainWin->get_ct_config()->charsListbul.size());
                    _curr_buffer->insert(range.iter_start, _pCtMainWin->get_ct_config()->charsListbul[bull_idx] + CtConst::CHAR_SPACE);
                }
                else {
                    int index;
                    if (!list_info) {
                        index = 0;
                        if (leading_num_count.empty()) leading_num_count = {LevelCount{0, 1}};
                        else                           leading_num_count = {LevelCount{0, leading_num_count.front().count+1}};
                    }
                    else {
                        int level = list_info.level;
                        index = level % CtConst::CHARS_LISTNUM.size();
                        if (leading_num_count.empty())
                            leading_num_count = {LevelCount{level, 1}};
                        else {
                            while (true) {
                                if (level == leading_num_count.back().level) {
                                    leading_num_count.back().count += 1;
                                    break;
                                }
                                if (level > leading_num_count.back().level) {
                                    leading_num_count.push_back(LevelCount{level, 1});
                                    break;
                                }
                                if (leading_num_count.size() == 1) {
                                    leading_num_count = {LevelCount{level, 1}};
                                    break;
                                }
                                leading_num_count.pop_back();
                            }
                        }
                    }
                    Glib::ustring leading_str = std::to_string(leading_num_count.back().count) + Glib::ustring(1, CtConst::CHARS_LISTNUM[index]) + CtConst::CHAR_SPACE;
                    new_par_offset = range.iter_end.get_offset() + (int)leading_str.size();
                    end_offset += leading_str.size();
                    _curr_buffer->insert(range.iter_start, leading_str);
                }
            }
            else
                new_par_offset = range.iter_end.get_offset();
        }
        range.iter_start = _curr_buffer->get_iter_at_offset(new_par_offset+1);
        if (!range.iter_start) break;
    }
}

// Clean List Start Characters
CtTextRange CtList::list_check_n_remove_old_list_type_leading(Gtk::TextIter iter_start, Gtk::TextIter iter_end)
{
    int start_offset = iter_start.get_offset();
    int end_offset = iter_end.get_offset();
    int leading_chars_num = 0;
    CtListInfo list_info = get_paragraph_list_info(iter_start);
    if (list_info) {
        leading_chars_num = get_leading_chars_num(list_info.type, list_info.num);
        start_offset += 3 * list_info.level;
        iter_start = _curr_buffer->get_iter_at_offset(start_offset);
        iter_end = iter_start;
        iter_end.forward_chars(leading_chars_num);
        _curr_buffer->erase(iter_start, iter_end);
        end_offset -= leading_chars_num;
    }
    iter_start = _curr_buffer->get_iter_at_offset(start_offset);
    iter_end = _curr_buffer->get_iter_at_offset(end_offset);
    return CtTextRange{iter_start, iter_end, leading_chars_num};
}

// Get Number of Leading Chars from the List Num
int CtList::get_leading_chars_num(CtListType type, int list_info_num)
{
    if (type == CtListType::Number)
        return (int)std::to_string(list_info_num).size() + 2; // '1. '
    return 2;
}

// Number < 0 if bulleted list, > 0 if numbered list, 0 if TODO list, None if not a list
CtListInfo CtList::list_get_number_n_level(Gtk::TextIter iter_first_paragraph)
{
    Gtk::TextIter iter_start = iter_first_paragraph;
    int level = 0;
    while (iter_start) {
        auto ch = iter_start.get_char();
        auto& bulls_list = _pCtMainWin->get_ct_config()->charsListbul.item();
        if (str::indexOf(bulls_list, ch) != -1) {
            if (iter_start.forward_char() and iter_start.get_char() == ' ') {
                int num = str::indexOf(bulls_list, ch);
                return CtListInfo{CtListType::Bullet, num, level, -1, -1};
            }
            break;
        }
        if (str::indexOf(_pCtMainWin->get_ct_config()->charsTodo.item(), ch) != -1) {
            if (iter_start.forward_char() and iter_start.get_char() == ' ')
                return CtListInfo{CtListType::Todo, 0, level, -1, -1};
            break;
        }
        if (ch == ' ') {
            if (CtTextIterUtil::startswith(iter_start, Glib::ustring(3, CtConst::CHAR_SPACE[0]).c_str())) {
                iter_start.forward_chars(3);
                level += 1;
            }
            else {
                break;
            }
        }
        else {
            bool match = ch >= '1' and ch <= '9';
            if (!match) {
                break;
            }
            Glib::ustring number_str(1, ch);
            while (iter_start.forward_char() and iter_start.get_char() >= '0' and iter_start.get_char() <= '9')
                number_str += iter_start.get_char();
            ch = iter_start.get_char();
            if (str::indexOf(CtConst::CHARS_LISTNUM, ch) != -1 and iter_start.forward_char() and iter_start.get_char() == ' ') {
                int num = std::stoi(number_str);
                auto aux = str::indexOf(CtConst::CHARS_LISTNUM, ch);
                return CtListInfo{CtListType::Number, num, level, aux, -1};
            }
            break;
        }
    }

    return CtListInfo{CtListType::None, -1, level, -1, -1};
}

// Get the list end offset
int CtList::get_multiline_list_element_end_offset(Gtk::TextIter curr_iter, CtListInfo list_info)
{
    Gtk::TextIter iter_start = curr_iter;
    if (iter_start.get_char() == '\n') {
        if (!iter_start.forward_char()) {
            // the end of buffer is also the list end
            return iter_start.get_offset();
        }
    }
    else {
        if (!char_iter_forward_to_newline(iter_start) or !iter_start.forward_char()) {
            // the end of buffer is also the list end
            return iter_start.get_offset();
        }
    }
    CtListInfo number_n_level = list_get_number_n_level(iter_start);
    // print number_n_level
    if (number_n_level.type == CtListType::None and number_n_level.level == list_info.level+1) {
        // multiline indentation
        return get_multiline_list_element_end_offset(iter_start, list_info);
    }
    return iter_start.get_offset()-1;
}

// Given a level check for previous list number on the level or None
CtListInfo CtList::get_prev_list_info_on_level(Gtk::TextIter iter_start, int level)
{
    CtListInfo ret_val;
    while (iter_start) {
        if (!char_iter_backward_to_newline(iter_start))
            break;
        CtListInfo list_info = get_paragraph_list_info(iter_start);
        if (!list_info)
            break;
        if (list_info.level < level)
            break;
        if (list_info.level == level) {
            ret_val = list_info;
            break;
        }
    }
    return ret_val;
}

// Given a level check for next list number on the level or None
CtListInfo CtList::get_next_list_info_on_level(Gtk::TextIter iter_start, int level)
{
    CtListInfo ret_val;
    while (iter_start) {
        (void)char_iter_forward_to_newline(iter_start);
        CtListInfo list_info = get_paragraph_list_info(iter_start);
        if (!list_info)
            break;
        if (list_info.level == level) {
            ret_val = list_info;
            break;
        }
    }
    return ret_val;
}

// Returns a dictionary indicating List Element Number, List Level and List Element Start Offset
CtListInfo CtList::get_paragraph_list_info(Gtk::TextIter iter_start_orig)
{
    bool buffer_start = false;
    Gtk::TextIter iter_start = iter_start_orig;
    // let's search for the paragraph start
    if (iter_start.get_char() == '\n')
        if (!iter_start.backward_char())
            buffer_start = true; // if we are exactly on the paragraph end
    if (!buffer_start) {
        while (true)
            if (iter_start.get_char() == '\n')
                break; // we got the previous paragraph start
            else if (!iter_start.backward_char()) {
                buffer_start = true;
                break; // we reached the buffer start
            }
    }
    if (!buffer_start)
        iter_start.forward_char();
    // get the number of the paragraph starting with iter_start
    CtListInfo number_n_level = list_get_number_n_level(iter_start);
    int curr_level = number_n_level.level;
    if (number_n_level)
        return CtListInfo{number_n_level.type, number_n_level.num, curr_level, number_n_level.aux, iter_start.get_offset()};
    // print number_n_level
    if (!buffer_start and curr_level > 0) {
        // may be a list paragraph but after a shift+return
        iter_start.backward_char();
        CtListInfo list_info = get_paragraph_list_info(iter_start);
        // print list_info
        if (list_info)
            if ((list_info and list_info.level == (curr_level-1)) or
                (!list_info and list_info.level == curr_level) /* it's used for shift+return */)
            {
                return list_info;
            }
    }
    return CtListInfo{}; // this paragraph is not a list
}

// Generates and Returns two iters indicating the paragraph bounds
CtTextRange CtList::get_paragraph_iters(Gtk::TextIter* force_iter /*= nullptr*/)
{
    Gtk::TextIter iter_start, iter_end;
    if (!force_iter and _curr_buffer->get_has_selection()) {
        _curr_buffer->get_selection_bounds(iter_start, iter_end); // there's a selection
    }
    else {
        // there's not a selection/iter forced
        if (!force_iter) iter_start = _curr_buffer->get_insert()->get_iter();
        else             iter_start = *force_iter;
        iter_end = iter_start;
        if (iter_start.get_char() == '\n') {
            // we're upon a row end
            if ( not iter_start.backward_char() or  // empty very first line
                 iter_start.get_char() == '\n' ) // empty line
            {
                return CtTextRange{iter_end, iter_end};
            }
        }
    }
    while (iter_end) {
        auto ch = iter_end.get_char();
        if (ch == '\n') break; // we got it
        else if (!iter_end.forward_char())  break; // we reached the buffer end
    }
    while (iter_start or iter_start == _curr_buffer->end()) {
        auto ch = iter_start.get_char();
        if (ch == '\n') { // we got it
            iter_start.forward_char();        // step forward to the beginning of the new line
            break;
        }
        else if (!iter_start.backward_char())
            break; // we reached the buffer start
    }
    return CtTextRange{iter_start, iter_end};
}

// Check if ☐ or ☑ or ☒
bool CtList::is_list_todo_beginning(Gtk::TextIter square_bracket_open_iter)
{
    if (_pCtMainWin->get_ct_config()->charsTodo.contains(Glib::ustring(1, square_bracket_open_iter.get_char()))) {
        CtListInfo list_info = get_paragraph_list_info(square_bracket_open_iter);
        if (list_info.type == CtListType::Todo)
            return true;
    }
    return false;
}

// Rotate status between ☐ and ☑ and ☒
void CtList::todo_list_rotate_status(Gtk::TextIter todo_char_iter)
{
    int iter_offset = todo_char_iter.get_offset();
    Glib::ustring todo_str(1, todo_char_iter.get_char());
    if (todo_str == _pCtMainWin->get_ct_config()->charsTodo[0]) {
        _curr_buffer->erase(todo_char_iter, _curr_buffer->get_iter_at_offset(iter_offset+1));
        _curr_buffer->insert(_curr_buffer->get_iter_at_offset(iter_offset), _pCtMainWin->get_ct_config()->charsTodo[1]);
    }
    else if (todo_str == _pCtMainWin->get_ct_config()->charsTodo[1]) {
        _curr_buffer->erase(todo_char_iter, _curr_buffer->get_iter_at_offset(iter_offset+1));
        _curr_buffer->insert(_curr_buffer->get_iter_at_offset(iter_offset), _pCtMainWin->get_ct_config()->charsTodo[2]);
    }
    else if (todo_str == _pCtMainWin->get_ct_config()->charsTodo[2]) {
        _curr_buffer->erase(todo_char_iter, _curr_buffer->get_iter_at_offset(iter_offset+1));
        _curr_buffer->insert(_curr_buffer->get_iter_at_offset(iter_offset), _pCtMainWin->get_ct_config()->charsTodo[0]);
    }
}

// Forwards char iter to line end
bool CtList::char_iter_forward_to_newline(Gtk::TextIter& char_iter)
{
    if (!char_iter.forward_char()) return false;
    while (char_iter.get_char() != '\n')
        if (!char_iter.forward_char())
            return false;
    return true;
}

// Backwards char iter to line start
bool CtList::char_iter_backward_to_newline(Gtk::TextIter& char_iter)
{
    if (!char_iter.backward_char()) return false;
    while (char_iter.get_char() != '\n')
        if (!char_iter.backward_char()) return false;
    return true;
}

// Conversion of todo lists from old to new type for a node
void CtList::todo_lists_old_to_new_conversion()
{
    Gtk::TextIter curr_iter = _curr_buffer->begin();
    bool keep_cleaning = false;
    bool first_line = true;
    while (curr_iter) {
        bool fw_needed = true;
        if ((first_line or curr_iter.get_char() == '\n') and curr_iter.forward_char()) {
            first_line = false;
            if (keep_cleaning) {
                Gtk::TextIter iter_bis = curr_iter;
                if (iter_bis.get_char() == ' ' and iter_bis.forward_char() and
                    iter_bis.get_char() == ' ' and iter_bis.forward_char() and
                    iter_bis.get_char() == ' ')
                {
                    bool no_stop = char_iter_forward_to_newline(curr_iter);
                    _curr_buffer->remove_all_tags(iter_bis, curr_iter);
                    if (no_stop) continue;
                    else         break;
                }
                else {
                    keep_cleaning = false;
                }
            }
            if (curr_iter.get_char() == '[' and curr_iter.forward_char() and
                (curr_iter.get_char() == ' ' or curr_iter.get_char() == 'X'))
            {
                auto middle_char = curr_iter.get_char();
                if (curr_iter.forward_char() and curr_iter.get_char() == ']' and curr_iter.forward_char()) {
                    Gtk::TextIter first_iter = curr_iter;
                    first_iter.backward_chars(3);
                    int iter_offset = first_iter.get_offset();
                    _curr_buffer->erase(first_iter, curr_iter);
                    auto todo_char = middle_char == ' ' ? _pCtMainWin->get_ct_config()->charsTodo[0] : _pCtMainWin->get_ct_config()->charsTodo[1];
                    _curr_buffer->insert(_curr_buffer->get_iter_at_offset(iter_offset), todo_char);
                    Gtk::TextIter curr_iter = _curr_buffer->get_iter_at_offset(iter_offset);
                    if (middle_char != ' ') {
                        first_iter = curr_iter;
                        bool no_stop = char_iter_forward_to_newline(curr_iter);
                        // print "%s(%s),%s(%s)" % (first_iter.get_char(), first_iter.get_offset(), curr_iter.get_char(), curr_iter.get_offset())
                        _curr_buffer->remove_all_tags(first_iter, curr_iter);
                        keep_cleaning = true;
                        if (no_stop) continue;
                        else         break;
                    }
                }
            }
            else {
                fw_needed = false;
            }
        }
        if (fw_needed and !char_iter_forward_to_newline(curr_iter)) {
            break;
        }
    }
}
