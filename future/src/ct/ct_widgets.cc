/*
 * ct_widgets.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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
#include "ct_app.h"

const double CtTextView::TEXT_SCROLL_MARGIN{0.3};


CtAnchoredWidget::CtAnchoredWidget(const int charOffset, const std::string& justification)
{
    _charOffset = charOffset;
    _justification = justification;
    _frame.set_shadow_type(Gtk::ShadowType::SHADOW_NONE);
    signal_button_press_event().connect([](GdkEventButton* /*pEvent*/){ return true; });
    add(_frame);
}

CtAnchoredWidget::~CtAnchoredWidget()
{

}



void CtAnchoredWidget::insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer)
{
    _rTextChildAnchor = rTextBuffer->create_child_anchor(rTextBuffer->get_iter_at_offset(_charOffset));
    if (!_justification.empty())
    {
        Gtk::TextIter textIterStart = rTextBuffer->get_iter_at_child_anchor(_rTextChildAnchor);
        Gtk::TextIter textIterEnd = textIterStart;
        textIterEnd.forward_char();
        Glib::ustring tagName = CtMiscUtil::getTextTagNameExistOrCreate(CtConst::TAG_JUSTIFICATION, _justification);
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

CtTextView::CtTextView()
{
    //set_sensitive(false);
    set_smart_home_end(Gsv::SMART_HOME_END_AFTER);
    set_left_margin(7);
    set_right_margin(7);
    set_insert_spaces_instead_of_tabs(CtApp::P_ctCfg->spacesInsteadTabs);
    set_tab_width((guint)CtApp::P_ctCfg->tabsWidth);
    if (CtApp::P_ctCfg->lineWrapping)
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

void CtTextView::setupForSyntax(const std::string& syntax)
{
    get_buffer()->signal_modified_changed().connect([](){
        // todo: elf.on_modified_changed

    });
    get_buffer()->signal_insert().connect([](const Gtk::TextIter&,const Glib::ustring&, int){
        // todo: self.on_text_insertion
        // if self.user_active:
        //            self.state_machine.text_variation(self.treestore[self.curr_tree_iter][3], text_inserted)
    });
    get_buffer()->signal_erase().connect([](const Gtk::TextIter, const Gtk::TextIter){
        // todo:  self.on_text_removal
        // if self.user_active and self.curr_tree_iter:
        //            self.state_machine.text_variation(self.treestore[self.curr_tree_iter][3], sourcebuffer.get_text(start_iter, end_iter))
    });
    // todo: exclude for codebox?
    get_buffer()->signal_mark_set().connect([](const Gtk::TextIter&,const Glib::RefPtr<Gtk::TextMark>){
        // todo: self.on_textbuffer_mark_set
    });

    if (CtConst::RICH_TEXT_ID == syntax)
    {
        // todo: self.widget_set_colors(self.sourceview, self.rt_def_fg, self.rt_def_bg, False)
        set_highlight_current_line(CtApp::P_ctCfg->rtHighlCurrLine);
        if (CtApp::P_ctCfg->rtShowWhiteSpaces)
        {
            set_draw_spaces(Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE);
        }
    }
    else
    {
        // todo: self.widget_set_colors(self.sourceview, self.rt_def_fg, self.rt_def_bg, True)
        set_highlight_current_line(CtApp::P_ctCfg->ptHighlCurrLine);
        if (CtApp::P_ctCfg->ptShowWhiteSpaces)
        {
            set_draw_spaces(Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE);
        }
    }
    _setFontForSyntax(syntax);
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
        if (!target.forward_chars(delta)) {
            // #print "? bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
        }
        text_buffer->move_mark(text_buffer->get_selection_bound(), target);
    } else {
        // # print "! bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
    }
}

void CtTextView::_setFontForSyntax(const std::string& syntaxHighlighting)
{
    Glib::RefPtr<Gtk::StyleContext> rStyleContext = get_style_context();
    std::string fontCss = CtFontUtil::getFontCssForSyntaxHighlighting(syntaxHighlighting);
    CtApp::R_cssProvider->load_from_data(fontCss);
    rStyleContext->add_provider(CtApp::R_cssProvider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

// Called at list indent/unindent time
void CtTextView::list_change_level(Gtk::TextIter iter_insert, const CtListInfo& list_info, bool level_increase)
{
    if (!CtApp::P_ctActions->_is_curr_node_not_read_only_or_error()) return;

    auto on_scope_exit = scope_guard([&](void*) { CtApp::P_ctActions->getCtMainWin()->user_active() = true; });
    CtApp::P_ctActions->getCtMainWin()->user_active() = false;

    int curr_offset = list_info.startoffs;
    int end_offset = CtList(get_buffer()).get_multiline_list_element_end_offset(iter_insert, list_info);
    int curr_level = list_info.level;
    int next_level = level_increase ? curr_level+1 : curr_level-1;
    Gtk::TextIter iter_start = get_buffer()->get_iter_at_offset(curr_offset);
    CtListInfo prev_list_info = CtList(get_buffer()).get_prev_list_info_on_level(iter_start, next_level);
    // print prev_list_info
    if (list_info.type != CtListInfo::TODO)
    {
        int bull_offset = curr_offset + 3*list_info.level;
        int bull_idx;
        if (list_info.type == CtListInfo::BULLET)
        {
            if (prev_list_info && prev_list_info.type == CtListInfo::BULLET)
                bull_idx = prev_list_info.num;
            else
            {
                int idx_old = list_info.num;
                int idx_offset = idx_old - curr_level % (int)CtApp::P_ctCfg->charsListbul.size();
                bull_idx = (next_level + idx_offset) % (int)CtApp::P_ctCfg->charsListbul.size();
            }
            replace_text(Glib::ustring(1, CtApp::P_ctCfg->charsListbul[(size_t)bull_idx]), bull_offset, bull_offset+1);
        }
        else if (list_info.type == CtListInfo::NUMBER)
        {
            int this_num, index;
            if (prev_list_info && prev_list_info.type == CtListInfo::NUMBER)
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
            replace_text(text_to, bull_offset, bull_offset+ CtList(get_buffer()).get_leading_chars_num(list_info.type, list_info.num));
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
        if (!CtList(get_buffer()).char_iter_forward_to_newline(iter_start) || !iter_start.forward_char())
            break;
        curr_offset = iter_start.get_offset();
    }
    CtApp::P_ctActions->getCtMainWin()->user_active() = true;
    CtApp::P_ctActions->getCtMainWin()->update_window_save_needed("nbuf", true);
}

void CtTextView::replace_text(const Glib::ustring& text, int start_offset, int end_offset)
{
    get_buffer()->erase(get_buffer()->get_iter_at_offset(start_offset), get_buffer()->get_iter_at_offset(end_offset));
    get_buffer()->insert(get_buffer()->get_iter_at_offset(start_offset), text);
}
