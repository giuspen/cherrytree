/*
 * ct_widgets.cc
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

#include "ct_widgets.h"
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_main_win.h"
#include "ct_actions.h"
#include <glib/gstdio.h>

CtTmp::CtTmp()
{
}

CtTmp::~CtTmp()
{
    //std::cout << "~CtTmp()" << std::endl;
    for (const auto& currPair : _mapHiddenFiles)
    {
        if (g_file_test(currPair.second, G_FILE_TEST_IS_REGULAR) and (0 != g_remove(currPair.second)))
        {
            std::cerr << "!! g_remove" << std::endl;
        }
        g_free(currPair.second);
    }
    for (const auto& currPair : _mapHiddenDirs)
    {
        if (g_file_test(currPair.second, G_FILE_TEST_IS_DIR) and (0 != g_rmdir(currPair.second)))
        {
            std::cerr << "!! g_rmdir" << std::endl;
        }
        g_free(currPair.second);
    }
}

const gchar* CtTmp::getHiddenDirPath(const std::string& visiblePath)
{
    if (not _mapHiddenDirs.count(visiblePath))
    {
        _mapHiddenDirs[visiblePath] = g_dir_make_tmp(nullptr, nullptr);
    }
    return _mapHiddenDirs.at(visiblePath);
}

const gchar* CtTmp::getHiddenFilePath(const std::string& visiblePath)
{
    if (not _mapHiddenFiles.count(visiblePath))
    {
        const gchar* tempDir{getHiddenDirPath(visiblePath)};
        std::string basename{Glib::path_get_basename(visiblePath)};
        if (Glib::str_has_suffix(basename, ".ctx"))
        {
            basename.replace(basename.end()-1, basename.end(), "b");
        }
        else if (Glib::str_has_suffix(basename, ".ctz"))
        {
            basename.replace(basename.end()-1, basename.end(), "d");
        }
        _mapHiddenFiles[visiblePath] = g_build_filename(tempDir, basename.c_str(), NULL);
    }
    return _mapHiddenFiles.at(visiblePath);
}


const double CtTextView::TEXT_SCROLL_MARGIN{0.3};

CtAnchoredWidget::CtAnchoredWidget(CtMainWin* pCtMainWin, const int charOffset, const std::string& justification)
 : _pCtMainWin(pCtMainWin),
   _charOffset(charOffset),
   _justification(justification)
{
    _frame.set_shadow_type(Gtk::ShadowType::SHADOW_NONE);
    signal_button_press_event().connect([](GdkEventButton* /*pEvent*/){ return true; });
    add(_frame);


    // todo:
    //if table_justification:
    //    text_iter = text_buffer.get_iter_at_child_anchor(anchor)
    //    self.dad.state_machine.apply_object_justification(text_iter, table_justification, text_buffer)
    //elif self.dad.user_active:
    //    # if I apply a justification, the state is already updated
    //    self.dad.state_machine.update_state()
}

CtAnchoredWidget::~CtAnchoredWidget()
{
}

void CtAnchoredWidget::updateJustification(const Gtk::TextIter& textIter)
{
    updateJustification(CtTextIterUtil::get_text_iter_alignment(textIter, _pCtMainWin));
}

void CtAnchoredWidget::insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer)
{
    _rTextChildAnchor = rTextBuffer->create_child_anchor(rTextBuffer->get_iter_at_offset(_charOffset));
    if (not _justification.empty())
    {
        Gtk::TextIter textIterStart = rTextBuffer->get_iter_at_child_anchor(_rTextChildAnchor);
        Gtk::TextIter textIterEnd = textIterStart;
        textIterEnd.forward_char();
        Glib::ustring tagName = _pCtMainWin->get_text_tag_name_exist_or_create(CtConst::TAG_JUSTIFICATION, _justification);
        rTextBuffer->apply_tag_by_name(tagName, textIterStart, textIterEnd);
    }
}


CtTreeView::CtTreeView()
{
    set_headers_visible(false);
}

CtTreeView::~CtTreeView()
{
}

void CtTreeView::set_cursor_safe(const Gtk::TreeIter& iter)
{
    expand_to_path(get_model()->get_path(iter));
    set_cursor(get_model()->get_path(iter));
}


CtTextView::CtTextView(CtMainWin* pCtMainWin)
 : _pCtMainWin(pCtMainWin)
{
    //set_sensitive(false);
    set_smart_home_end(Gsv::SMART_HOME_END_AFTER);
    set_left_margin(7);
    set_right_margin(7);
    set_insert_spaces_instead_of_tabs(_pCtMainWin->get_ct_config()->spacesInsteadTabs);
    set_tab_width((guint)_pCtMainWin->get_ct_config()->tabsWidth);
    if (_pCtMainWin->get_ct_config()->lineWrapping)
    {
        set_wrap_mode(Gtk::WrapMode::WRAP_WORD_CHAR);
    }
    else
    {
        set_wrap_mode(Gtk::WrapMode::WRAP_NONE);
    }
    for (const Gtk::TextWindowType& textWinType : std::list<Gtk::TextWindowType>{Gtk::TEXT_WINDOW_LEFT,
                                                                                 Gtk::TEXT_WINDOW_RIGHT,
                                                                                 Gtk::TEXT_WINDOW_TOP,
                                                                                 Gtk::TEXT_WINDOW_BOTTOM})
    {
        set_border_window_size(textWinType, 1);
    }
}

CtTextView::~CtTextView()
{
}

void CtTextView::setup_for_syntax(const std::string& syntax)
{
    std::string new_class;
    if (CtConst::RICH_TEXT_ID == syntax)         { new_class = "ct-view-rich-text"; }
    else if (CtConst::PLAIN_TEXT_ID == syntax)   { new_class = "ct-view-plain-text"; }
    else                                         { new_class = "ct-view-code"; }

    if (new_class != "ct-view-rich-text") get_style_context()->remove_class("ct-view-rich-text");
    if (new_class != "ct-view-plain-text") get_style_context()->remove_class("ct-view-plain-text");
    if (new_class != "ct-view-code") get_style_context()->remove_class("ct-view-code");
    get_style_context()->add_class(new_class);

    if (CtConst::RICH_TEXT_ID == syntax)
    {
        // todo: self.widget_set_colors(self.sourceview, self.rt_def_fg, self.rt_def_bg, False)
        set_highlight_current_line(_pCtMainWin->get_ct_config()->rtHighlCurrLine);
        if (_pCtMainWin->get_ct_config()->rtShowWhiteSpaces)
        {
            set_draw_spaces(Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE);
        }
    }
    else
    {
        // todo: self.widget_set_colors(self.sourceview, self.rt_def_fg, self.rt_def_bg, True)
        set_highlight_current_line(_pCtMainWin->get_ct_config()->ptHighlCurrLine);
        if (_pCtMainWin->get_ct_config()->ptShowWhiteSpaces)
        {
            set_draw_spaces(Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE);
        }
    }
}

void CtTextView::set_pixels_inside_wrap(int space_around_lines, int relative_wrapped_space)
{
    int pixels_around_wrap = (int)((double)space_around_lines * ((double)relative_wrapped_space / 100.0));
    Gtk::TextView::set_pixels_inside_wrap(pixels_around_wrap);
}

void CtTextView::set_selection_at_offset_n_delta(int offset, int delta, Glib::RefPtr<Gtk::TextBuffer> text_buffer /*=Glib::RefPtr<Gtk::TextBuffer>()*/)
{
    text_buffer = text_buffer ? text_buffer : get_buffer();
    Gtk::TextIter target = text_buffer->get_iter_at_offset(offset);
    if (target) {
        text_buffer->place_cursor(target);
        if (not target.forward_chars(delta)) {
            // #print "? bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
        }
        text_buffer->move_mark(text_buffer->get_selection_bound(), target);
    } else {
        // # print "! bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
    }
}

// Called at list indent/unindent time
void CtTextView::list_change_level(Gtk::TextIter iter_insert, const CtListInfo& list_info, bool level_increase)
{
    if (not _pCtMainWin->get_ct_actions()->_is_curr_node_not_read_only_or_error()) return;

    auto on_scope_exit = scope_guard([&](void*) { _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active() = true; });
    _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active() = false;

    int curr_offset = list_info.startoffs;
    int end_offset = CtList(_pCtMainWin, get_buffer()).get_multiline_list_element_end_offset(iter_insert, list_info);
    int curr_level = list_info.level;
    int next_level = level_increase ? curr_level+1 : curr_level-1;
    Gtk::TextIter iter_start = get_buffer()->get_iter_at_offset(curr_offset);
    CtListInfo prev_list_info = CtList(_pCtMainWin, get_buffer()).get_prev_list_info_on_level(iter_start, next_level);
    // print prev_list_info
    if (list_info.type != CtListType::Todo)
    {
        int bull_offset = curr_offset + 3*list_info.level;
        int bull_idx;
        if (list_info.type == CtListType::Bullet)
        {
            if (prev_list_info and prev_list_info.type == CtListType::Bullet)
                bull_idx = prev_list_info.num;
            else
            {
                int idx_old = list_info.num;
                int idx_offset = idx_old - curr_level % (int)_pCtMainWin->get_ct_config()->charsListbul.size();
                bull_idx = (next_level + idx_offset) % (int)_pCtMainWin->get_ct_config()->charsListbul.size();
            }
            replace_text(Glib::ustring(1, _pCtMainWin->get_ct_config()->charsListbul[(size_t)bull_idx]), bull_offset, bull_offset+1);
        }
        else if (list_info.type == CtListType::Number)
        {
            int this_num, index;
            if (prev_list_info and prev_list_info.type == CtListType::Number)
            {
                this_num = prev_list_info.num + 1;
                index = prev_list_info.aux;
            }
            else
            {
                this_num = 1;
                int idx_old = list_info.aux;
                int idx_offset = idx_old - curr_level % CtConst::NUM_CHARS_LISTNUM;
                index = (next_level + idx_offset) % CtConst::NUM_CHARS_LISTNUM;
            }
            Glib::ustring text_to = std::to_string(this_num) + Glib::ustring(1, CtConst::CHARS_LISTNUM[(size_t)index]) + CtConst::CHAR_SPACE;
            replace_text(text_to, bull_offset, bull_offset + CtList(_pCtMainWin, get_buffer()).get_leading_chars_num(list_info.type, list_info.num));
        }
    }
    iter_start = get_buffer()->get_iter_at_offset(curr_offset);
    // print "%s -> %s" % (curr_offset, end_offset)
    while (curr_offset < end_offset)
    {
        if (level_increase)
        {
            get_buffer()->insert(iter_start, Glib::ustring(3, CtConst::CHAR_SPACE[0]));
            end_offset += 3;
            iter_start = get_buffer()->get_iter_at_offset(curr_offset+3);
        }
        else
        {
            get_buffer()->erase(iter_start, get_buffer()->get_iter_at_offset(curr_offset+3));
            end_offset -= 3;
            iter_start = get_buffer()->get_iter_at_offset(curr_offset+1);
        }
        if (not CtList(_pCtMainWin, get_buffer()).char_iter_forward_to_newline(iter_start) or not iter_start.forward_char())
            break;
        curr_offset = iter_start.get_offset();
    }
    _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active() = true;
    _pCtMainWin->get_ct_actions()->getCtMainWin()->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtTextView::replace_text(const Glib::ustring& text, int start_offset, int end_offset)
{
    get_buffer()->erase(get_buffer()->get_iter_at_offset(start_offset), get_buffer()->get_iter_at_offset(end_offset));
    get_buffer()->insert(get_buffer()->get_iter_at_offset(start_offset), text);
}

// Called after every Double Click with button 1
void CtTextView::for_event_after_double_click_button1(GdkEvent* event)
{
    auto text_buffer = get_buffer();
    int x, y;
    window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, (int)event->button.x, (int)event->button.y, x, y);
    Gtk::TextIter iter_start;
    get_iter_at_location(iter_start, x, y);
    _pCtMainWin->apply_tag_try_automatic_bounds(text_buffer, iter_start);
}

// Called after every Triple Click with button 1
void CtTextView::for_event_after_triple_click_button1(GdkEvent* event)
{
     auto text_buffer = get_buffer();
     int x, y;
     window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, (int)event->button.x, (int)event->button.y, x, y);
     Gtk::TextIter iter_start;
     get_iter_at_location(iter_start, x, y);
     _pCtMainWin->apply_tag_try_automatic_bounds_triple_click(text_buffer, iter_start);
}

// Called after every gtk.gdk.BUTTON_PRESS on the SourceView
void CtTextView::for_event_after_button_press(GdkEvent* event)
{
    auto text_buffer = get_buffer();
    if (event->button.button == 1 or event->button.button == 2)
    {
        int x, y;
        window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, (int)event->button.x, (int)event->button.y, x, y);
        Gtk::TextIter text_iter;
        get_iter_at_location(text_iter, x, y);
        auto tags = text_iter.get_tags();
        // check whether we are hovering a link
        for (auto& tag: tags)
        {
            Glib::ustring tag_name = tag->property_name();
            if (str::startswith(tag_name, CtConst::TAG_LINK))
            {
                _pCtMainWin->get_ct_actions()->link_clicked(tag_name.substr(5), event->button.button == 2);
                return;
            }
        }
        if (CtList(_pCtMainWin, text_buffer).is_list_todo_beginning(text_iter))
            if (_pCtMainWin->get_ct_actions()->_is_curr_node_not_read_only_or_error())
                CtList(_pCtMainWin, text_buffer).todo_list_rotate_status(text_iter);
    }
    else if (event->button.button == 3 and not text_buffer->get_has_selection())
    {
        int x, y;
        window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, (int)event->button.x, (int)event->button.y, x, y);
        Gtk::TextIter text_iter;
        get_iter_at_location(text_iter, x, y);
        text_buffer->place_cursor(text_iter);
    }
}

// Called after every gtk.gdk.KEY_PRESS on the SourceView
void CtTextView::for_event_after_key_press(GdkEvent* event, const Glib::ustring& syntaxHighlighting)
{
    auto text_buffer = get_buffer();
    auto config = _pCtMainWin->get_ct_config();
    bool is_code = syntaxHighlighting != CtConst::RICH_TEXT_ID and syntaxHighlighting != CtConst::PLAIN_TEXT_ID;

    if (not is_code and config->autoSmartQuotes and (event->key.keyval == GDK_KEY_quotedbl or event->key.keyval == GDK_KEY_apostrophe))
    {
        Gtk::TextIter iter_insert = text_buffer->get_insert()->get_iter();
        int offset_1 = iter_insert.get_offset()-1;
        if (offset_1 > 0)
        {
            Glib::ustring  start_char, char_0, char_1;
            if (event->key.keyval == GDK_KEY_quotedbl)
            {
                start_char = CtConst::CHAR_DQUOTE;
                char_0 = config->chars_smart_dquote[0];
                char_1 = config->chars_smart_dquote[1];
            }
            else
            {
                start_char = CtConst::CHAR_SQUOTE;
                char_0 = config->chars_smart_squote[0];
                char_1 = config->chars_smart_squote[1];
            }
            Gtk::TextIter iter_start = text_buffer->get_iter_at_offset(offset_1-1);
            int offset_0 = -1;
            while (true)
            {
                auto curr_char = iter_start.get_char();
                if (curr_char == start_char[0])
                {
                    int candidate_offset = iter_start.get_offset();
                    if (not iter_start.backward_char()
                            or iter_start.get_char() == CtConst::CHAR_NEWLINE[0]
                            or iter_start.get_char() == CtConst::CHAR_SPACE[0]
                            or iter_start.get_char() == CtConst::CHAR_TAB[0])
                        offset_0 = candidate_offset;
                    break;
                }
                if (curr_char == CtConst::CHAR_NEWLINE[0]) break;
                if (not iter_start.backward_char()) break;
            }
            if (offset_0 >= 0)
            {
                replace_text(char_0, offset_0, offset_0+1);
                replace_text(char_1, offset_1, offset_1+1);
            }
        }
    }
    else if (event->key.state & Gdk::SHIFT_MASK)
    {
        if (event->key.keyval == GDK_KEY_Return)
        {
            Gtk::TextIter iter_insert = text_buffer->get_insert()->get_iter();
            Gtk::TextIter iter_start = iter_insert;
            iter_start.backward_char();
            CtListInfo list_info = CtList(_pCtMainWin, text_buffer).get_paragraph_list_info(iter_start);
            if (list_info)
                text_buffer->insert(iter_insert, Glib::ustring(3*(1+(size_t)list_info.level), CtConst::CHAR_SPACE[0]));
        }
    }
    else if (event->key.keyval == GDK_KEY_Return or event->key.keyval == GDK_KEY_space)
    {
        Gtk::TextIter iter_insert = text_buffer->get_insert()->get_iter();
        if (syntaxHighlighting == CtConst::RICH_TEXT_ID)
        {
            Gtk::TextIter iter_end_link = iter_insert;
            iter_end_link.backward_char();
            if (_apply_tag_try_link(iter_end_link, iter_insert.get_offset()))
                iter_insert = text_buffer->get_insert()->get_iter();
        }
        Gtk::TextIter iter_start = iter_insert;
        if (event->key.keyval == GDK_KEY_Return)
        {
            int cursor_key_press = iter_insert.get_offset();
            //print "cursor_key_press", cursor_key_press
            if (cursor_key_press == _pCtMainWin->get_ct_actions()->getCtMainWin()->cursor_key_press())
            {
                // problem of event-after called twice, once before really executing
                return;
            }
            if (not iter_start.backward_char()) return;
            if (iter_start.get_char() != CtConst::CHAR_NEWLINE[0]) return;
            if (iter_start.backward_char() and iter_start.get_char() == CtConst::CHAR_NEWLINE[0])
                return; // former was an empty row
            CtListInfo list_info = CtList(_pCtMainWin, text_buffer).get_paragraph_list_info(iter_start);
            if (not list_info)
            {
                if (config->autoIndent)
                {
                    iter_start = iter_insert;
                    Glib::ustring former_line_indent = _get_former_line_indentation(iter_start);
                    if (not former_line_indent.empty()) text_buffer->insert_at_cursor(former_line_indent);
                }
                return; // former was not a list
            }
            // possible enter on empty list element
            int insert_offset = iter_insert.get_offset();
            int chars_to_startoffs = 1 + CtList(_pCtMainWin, text_buffer).get_leading_chars_num(list_info.type, list_info.num) + 3*list_info.level;
            if ((insert_offset - list_info.startoffs) == chars_to_startoffs)
            {
                Gtk::TextIter iter_list_quit;
                // enter on empty list element
                if (list_info.level > 0)
                {
                    list_change_level(iter_insert, list_info, false);
                    iter_insert = text_buffer->get_insert()->get_iter();
                    iter_list_quit = text_buffer->get_iter_at_offset(iter_insert.get_offset()-1);
                }
                else
                    iter_list_quit = text_buffer->get_iter_at_offset(list_info.startoffs);
                text_buffer->erase(iter_list_quit, iter_insert);
                return;
            }
            // list new element
            int curr_level = list_info.level;
            Glib::ustring pre_spaces = curr_level ? Glib::ustring((size_t)(3*curr_level), CtConst::CHAR_SPACE[0]) : "";
            if (list_info.type == CtListType::Bullet)
            {
                text_buffer->insert(iter_insert, pre_spaces+config->charsListbul[(size_t)list_info.num]+CtConst::CHAR_SPACE);
            }
            else if (list_info.type == CtListType::Todo)
                text_buffer->insert(iter_insert, pre_spaces+config->charsTodo[0]+CtConst::CHAR_SPACE);
            else
            {
                int new_num = list_info.num + 1;
                int index = list_info.aux;
                text_buffer->insert(iter_insert, pre_spaces + std::to_string(new_num) + CtConst::CHARS_LISTNUM[(size_t)index] + CtConst::CHAR_SPACE);
                new_num += 1;
                iter_start = text_buffer->get_iter_at_offset(insert_offset);
                CtList(_pCtMainWin, text_buffer).char_iter_forward_to_newline(iter_start);
                list_info = CtList(_pCtMainWin, text_buffer).get_next_list_info_on_level(iter_start, curr_level);
                // print list_info
                while (list_info and list_info.type == CtListType::Number)
                {
                    iter_start = text_buffer->get_iter_at_offset(list_info.startoffs);
                    int end_offset = CtList(_pCtMainWin, text_buffer).get_multiline_list_element_end_offset(iter_start, list_info);
                    Gtk::TextIter iter_end = text_buffer->get_iter_at_offset(end_offset);
                    CtTextRange range = CtList(_pCtMainWin, text_buffer).list_check_n_remove_old_list_type_leading(iter_start, iter_end);
                    end_offset -= range.leading_chars_num;
                    text_buffer->insert(iter_start, std::to_string(new_num) + Glib::ustring(1, CtConst::CHARS_LISTNUM[(size_t)index]) + CtConst::CHAR_SPACE);
                    end_offset += CtList(_pCtMainWin, text_buffer).get_leading_chars_num(list_info.type, new_num);
                    iter_start = text_buffer->get_iter_at_offset(end_offset);
                    new_num += 1;
                    list_info = CtList(_pCtMainWin, text_buffer).get_next_list_info_on_level(iter_start, curr_level);
                }
            }
        }
        else // keyname == CtConst::STR_KEY_SPACE
        {
            if (not is_code and config->enableSymbolAutoreplace and iter_start.backward_chars(2))
            {
                if (iter_start.get_char() == CtConst::CHAR_GREATER[0] and iter_start.backward_char())
                {
                    if (iter_start.get_line_offset() == 0)
                    {
                        // at line start
                        if (iter_start.get_char() == CtConst::CHAR_LESSER[0])
                            // "<> " becoming "◇ "
                            _special_char_replace(CtConst::CHARS_LISTBUL_DEFAULT[1], iter_start, iter_insert);
                        else if (iter_start.get_char() == CtConst::CHAR_MINUS[0])
                            // "-> " becoming "→ "
                            _special_char_replace(CtConst::CHARS_LISTBUL_DEFAULT[4], iter_start, iter_insert);
                        else if (iter_start.get_char() == CtConst::CHAR_EQUAL[0])
                            // "=> " becoming "⇒ "
                            _special_char_replace(CtConst::CHARS_LISTBUL_DEFAULT[5], iter_start, iter_insert);
                    }
                    else if (iter_start.get_char() == CtConst::CHAR_MINUS[0] and iter_start.backward_char())
                    {
                        if (iter_start.get_char() == CtConst::CHAR_LESSER[0])
                            // "<-> " becoming "↔ "
                            _special_char_replace(CtConst::SPECIAL_CHAR_ARROW_DOUBLE[0], iter_start, iter_insert);
                        else if (iter_start.get_char() == CtConst::CHAR_MINUS[0])
                            // "--> " becoming "→ "
                            _special_char_replace(CtConst::SPECIAL_CHAR_ARROW_RIGHT[0], iter_start, iter_insert);
                    }
                    else if (iter_start.get_char() == CtConst::CHAR_EQUAL[0] and iter_start.backward_char())
                    {
                        if (iter_start.get_char() == CtConst::CHAR_LESSER[0])
                            // "<=> " becoming "⇔ "
                            _special_char_replace(CtConst::SPECIAL_CHAR_ARROW_DOUBLE2[0], iter_start, iter_insert);
                        else if (iter_start.get_char() == CtConst::CHAR_EQUAL[0])
                            // "==> " becoming "⇒ "
                            _special_char_replace(CtConst::SPECIAL_CHAR_ARROW_RIGHT2[0], iter_start, iter_insert);
                    }
                }
                else if (iter_start.get_char() == CtConst::CHAR_MINUS[0] and iter_start.backward_char()
                        and iter_start.get_char() == CtConst::CHAR_MINUS[0] and iter_start.backward_char()
                        and iter_start.get_char() == CtConst::CHAR_LESSER[0])
                        // "<-- " becoming "← "
                        _special_char_replace(CtConst::SPECIAL_CHAR_ARROW_LEFT[0], iter_start, iter_insert);
                else if (iter_start.get_char() == CtConst::CHAR_EQUAL[0] and iter_start.backward_char()
                        and iter_start.get_char() == CtConst::CHAR_EQUAL[0] and iter_start.backward_char()
                        and iter_start.get_char() == CtConst::CHAR_LESSER[0])
                        // "<== " becoming "⇐ "
                        _special_char_replace(CtConst::SPECIAL_CHAR_ARROW_LEFT2[0], iter_start, iter_insert);
                else if (iter_start.get_char() == CtConst::CHAR_PARENTH_CLOSE[0] and iter_start.backward_char())
                {
                    if (g_unichar_tolower(iter_start.get_char()) == 'c' and iter_start.backward_char()
                       and iter_start.get_char() == CtConst::CHAR_PARENTH_OPEN[0])
                        // "(c) " becoming "© "
                        _special_char_replace(CtConst::SPECIAL_CHAR_COPYRIGHT[0], iter_start, iter_insert);
                    else if (g_unichar_tolower(iter_start.get_char()) == 'r' and iter_start.backward_char()
                            and iter_start.get_char() == CtConst::CHAR_PARENTH_OPEN[0])
                            // "(r) " becoming "® "
                            _special_char_replace(CtConst::SPECIAL_CHAR_REGISTERED_TRADEMARK[0], iter_start, iter_insert);
                    else if (g_unichar_tolower(iter_start.get_char()) == 'm' and iter_start.backward_char()
                            and g_unichar_tolower(iter_start.get_char()) == 't' and iter_start.backward_char()
                            and iter_start.get_char() == CtConst::CHAR_PARENTH_OPEN[0])
                            // "(tm) " becoming "™ "
                            _special_char_replace(CtConst::SPECIAL_CHAR_UNREGISTERED_TRADEMARK[0], iter_start, iter_insert);
                }
                else if (iter_start.get_char() == CtConst::CHAR_STAR[0] and iter_start.get_line_offset() == 0)
                    // "* " becoming "• " at line start
                    _special_char_replace(CtConst::CHARS_LISTBUL_DEFAULT[0], iter_start, iter_insert);
                else if (iter_start.get_char() == CtConst::CHAR_SQ_BR_CLOSE[0] and iter_start.backward_char())
                {
                    if (iter_start.get_line_offset() == 0 and iter_start.get_char() == CtConst::CHAR_SQ_BR_OPEN[0])
                        // "[] " becoming "☐ " at line start
                        _special_char_replace(config->charsTodo[0], iter_start, iter_insert);
                }
                else if (iter_start.get_char() == CtConst::CHAR_COLON[0] and iter_start.backward_char())
                {
                    if (iter_start.get_line_offset() == 0 and iter_start.get_char() == CtConst::CHAR_COLON[0])
                        // ":: " becoming "▪ " at line start
                        _special_char_replace(CtConst::CHARS_LISTBUL_DEFAULT[2], iter_start, iter_insert);
                }
            }
        }
    }
}

// Looks at all tags covering the position (x, y) in the text view
// and if one of them is a link, change the cursor to the HAND2 cursor
void CtTextView::cursor_and_tooltips_handler(int x, int y)
{
    int hovering_link_iter_offset = -1;
    Glib::ustring tooltip;
    Gtk::TextIter text_iter;
    get_iter_at_location(text_iter, x, y);

    if (CtList(_pCtMainWin, get_buffer()).is_list_todo_beginning(text_iter))
    {
        get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::X_CURSOR));
        set_tooltip_text("");
        return;
    }
    auto tags = text_iter.get_tags();
    bool find_link = false;
    for (auto tag: tags)
    {
        Glib::ustring tag_name = tag->property_name();
        if (str::startswith(tag_name, CtConst::TAG_LINK))
        {
            find_link = true;
            hovering_link_iter_offset = text_iter.get_offset();
            tooltip = _pCtMainWin->sourceview_hovering_link_get_tooltip(tag_name.substr(5));
            break;
        }
    }
    if (not find_link)
    {
        Gtk::TextIter iter_anchor = text_iter;
        for (int i: {0, 1})
        {
            if (i == 1) iter_anchor.backward_char();
            auto widgets = _pCtMainWin->get_ct_actions()->getCtMainWin()->curr_tree_iter().get_embedded_pixbufs_tables_codeboxes({iter_anchor.get_offset(), iter_anchor.get_offset()});
            if (not widgets.empty())
                if (CtImagePng* image = dynamic_cast<CtImagePng*>(widgets.front()))
                    if (not image->get_link().empty())
                    {
                        hovering_link_iter_offset = text_iter.get_offset();
                        tooltip = _pCtMainWin->sourceview_hovering_link_get_tooltip(image->get_link());
                        break;
                    }
        }
    }
    if (_pCtMainWin->get_ct_actions()->getCtMainWin()->hovering_link_iter_offset() != hovering_link_iter_offset)
    {
        _pCtMainWin->get_ct_actions()->getCtMainWin()->hovering_link_iter_offset() = hovering_link_iter_offset;
        // print "link", dad.hovering_link_iter_offset
    }
    if (_pCtMainWin->get_ct_actions()->getCtMainWin()->hovering_link_iter_offset() >= 0)
    {
        get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::HAND2));
        if (tooltip.size() > (size_t)CtConst::MAX_TOOLTIP_LINK_CHARS)
            tooltip = tooltip.substr(0, (size_t)CtConst::MAX_TOOLTIP_LINK_CHARS) + "...";
        set_tooltip_text(tooltip);
    }
    else
    {
        get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::XTERM));
        set_tooltip_text("");
    }
}

// Increase or Decrease Text Font
void CtTextView::zoom_text(bool is_increase)
{
    Glib::RefPtr<Gtk::StyleContext> context = get_style_context();
    Pango::FontDescription description = context->get_font(context->get_state());
    auto size = description.get_size() / Pango::SCALE + (is_increase ? 1 : -1);
    if (size < 6) size = 6;
    description.set_size(size * Pango::SCALE);
    override_font(description);
}

// Try and apply link to previous word (after space or newline)
bool CtTextView::_apply_tag_try_link(Gtk::TextIter iter_end, int offset_cursor)
{
    // Apply Link to Node Tag if the text is a node name
    auto apply_tag_try_node_name = [this](Gtk::TextIter iter_start, Gtk::TextIter iter_end)
    {
        Glib::ustring node_name = get_buffer()->get_text(iter_start, iter_end);
        CtTreeIter node_dest = _pCtMainWin->get_ct_actions()->getCtMainWin()->curr_tree_store().get_node_from_node_name(node_name);
        if (node_dest)
        {
            get_buffer()->select_range(iter_start, iter_end);
            Glib::ustring property_value = CtConst::LINK_TYPE_NODE + CtConst::CHAR_SPACE + std::to_string(node_dest.get_node_id());
            _pCtMainWin->get_ct_actions()->_apply_tag(CtConst::TAG_LINK, property_value);
            return true;
        }
        return false;
    };

    bool tag_applied = false;
    Gtk::TextIter iter_start = iter_end;
    if (iter_start.backward_char() and iter_start.get_char() == CtConst::CHAR_SQ_BR_CLOSE[0]
        and iter_start.backward_char() and iter_start.get_char() == CtConst::CHAR_SQ_BR_CLOSE[0])
    {
        int curr_state = 0;
        while (iter_start.backward_char())
        {
            auto curr_char = iter_start.get_char();
            if (curr_char == CtConst::CHAR_NEWLINE[0])
                break;
            if (curr_char == CtConst::CHAR_SQ_BR_OPEN[0])
            {
                if (curr_state == 0)
                    curr_state = 1;
                else
                {
                    curr_state = 2;
                    break;
                }
            }
        }
        if (curr_state == 2)
        {
            int start_offset = iter_start.get_offset()+2;
            int end_offset = iter_end.get_offset()-2;
            if (apply_tag_try_node_name(get_buffer()->get_iter_at_offset(start_offset), get_buffer()->get_iter_at_offset(end_offset)))
            {
                tag_applied = true;
                get_buffer()->erase(get_buffer()->get_iter_at_offset(end_offset), get_buffer()->get_iter_at_offset(end_offset+2));
                get_buffer()->erase(get_buffer()->get_iter_at_offset(start_offset-2), get_buffer()->get_iter_at_offset(start_offset));
                if (offset_cursor != -1)
                    offset_cursor -= 4;
            }
        }
    }
    else
    {
        iter_start = iter_end;
        while (iter_start.backward_char())
        {
            auto curr_char = iter_start.get_char();
            if (curr_char == CtConst::CHAR_SPACE[0] or curr_char == CtConst::CHAR_NEWLINE[0] or curr_char == CtConst::CHAR_CR[0] or curr_char == CtConst::CHAR_TAB[0])
            {
                iter_start.forward_char();
                break;
            }
        }
        int num_chars = iter_end.get_offset() - iter_start.get_offset();
        if (num_chars > 4 and CtTextIterUtil::get_next_chars_from_iter_are(iter_start, CtConst::WEB_LINK_STARTERS))
         {
            get_buffer()->select_range(iter_start, iter_end);
            Glib::ustring link_url = get_buffer()->get_text(iter_start, iter_end);
            if (not str::startswith(link_url, "htt") and !str::startswith(link_url, "ftp")) link_url = "http://" + link_url;
            Glib::ustring property_value = CtConst::LINK_TYPE_WEBS + CtConst::CHAR_SPACE + link_url;
            _pCtMainWin->get_ct_actions()->_apply_tag(CtConst::TAG_LINK, property_value);
            tag_applied = true;
        }
        else if (num_chars > 2 and CtTextIterUtil::get_is_camel_case(iter_start, num_chars))
            if (apply_tag_try_node_name(iter_start, iter_end))
                tag_applied = true;
    }
    if (tag_applied and offset_cursor != -1)
        get_buffer()->place_cursor(get_buffer()->get_iter_at_offset(offset_cursor));
    return tag_applied;
}

// Returns the indentation of the former paragraph or empty string
Glib::ustring CtTextView::_get_former_line_indentation(Gtk::TextIter iter_start)
{
    if (not iter_start.backward_chars(2) or iter_start.get_char() == CtConst::CHAR_NEWLINE[0]) return "";
    bool buffer_start = false;
    while (true)
    {
        if (iter_start.get_char() == CtConst::CHAR_NEWLINE[0]) break; // we got the previous paragraph start
        else if (not iter_start.backward_char())
        {
            buffer_start = true;
            break; // we reached the buffer start
        }
    }
    if (not buffer_start) iter_start.forward_char();
    if (iter_start.get_char() == CtConst::CHAR_SPACE[0])
    {
        size_t num_spaces = 1;
        while (iter_start.forward_char() and iter_start.get_char() == CtConst::CHAR_SPACE[0])
            num_spaces += 1;
        return Glib::ustring(num_spaces, CtConst::CHAR_SPACE[0]);
    }
    if (iter_start.get_char() == CtConst::CHAR_TAB[0])
    {
        size_t num_tabs = 1;
        while (iter_start.forward_char() and iter_start.get_char() == CtConst::CHAR_TAB[0])
            num_tabs += 1;
        return Glib::ustring(num_tabs, CtConst::CHAR_TAB[0]);
    }
    return "";
}

// A special char replacement is triggered
void CtTextView::_special_char_replace(gunichar special_char, Gtk::TextIter iter_start, Gtk::TextIter iter_insert)
{
    get_buffer()->erase(iter_start, iter_insert);
    get_buffer()->insert_at_cursor(Glib::ustring(1, special_char) + CtConst::CHAR_SPACE);
}
