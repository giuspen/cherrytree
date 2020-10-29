/*
 * ct_codebox.cc
 *
 * Copyright 2009-2020
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

#include "ct_codebox.h"
#include "ct_list.h"
#include "ct_clipboard.h"
#include "ct_misc_utils.h"
#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_storage_sqlite.h"
#include "ct_logging.h"

const constexpr int MIN_SCROLL_HEIGHT = 47;

CtTextCell::CtTextCell(CtMainWin* pCtMainWin,
                       const Glib::ustring& textContent,
                       const std::string& syntaxHighlighting)
 : _syntaxHighlighting(syntaxHighlighting),
   _ctTextview(pCtMainWin)
{
    _rTextBuffer = pCtMainWin->get_new_text_buffer(textContent);
    _ctTextview.set_buffer(_rTextBuffer);
    _ctTextview.setup_for_syntax(_syntaxHighlighting);

    _rTextBuffer->signal_insert().connect([&](const Gtk::TextIter& pos, const Glib::ustring& text, int /*bytes*/) {
        if (_ctTextview.getCtMainWin()->user_active() and not _ctTextview.own_insert_delete_active()) {
            _ctTextview.text_inserted(pos, text);
            _ctTextview.getCtMainWin()->get_state_machine().text_variation(_ctTextview.getCtMainWin()->curr_tree_iter().get_node_id(), text);
            _ctTextview.getCtMainWin()->update_window_save_needed(CtSaveNeededUpdType::nbuf);
        }
    }, false);
    _rTextBuffer->signal_erase().connect([&](const Gtk::TextIter& range_start, const Gtk::TextIter& range_end) {
        if (_ctTextview.getCtMainWin()->user_active() and not _ctTextview.own_insert_delete_active()) {
            _ctTextview.text_removed(range_start, range_end);
            _ctTextview.getCtMainWin()->get_state_machine().text_variation(_ctTextview.getCtMainWin()->curr_tree_iter().get_node_id(), range_start.get_text(range_end));
            _ctTextview.getCtMainWin()->update_window_save_needed(CtSaveNeededUpdType::nbuf);
        }
    }, false);
    _rTextBuffer->signal_mark_set().connect([&](const Gtk::TextIter& /*iter*/, const Glib::RefPtr<Gtk::TextMark>& rMark){
        if (_ctTextview.getCtMainWin()->user_active()) {
            if (rMark->get_name() == "insert") {
                _ctTextview.selection_update();
            }
        }
    }, false);
}

CtTextCell::~CtTextCell()
{
}

Glib::ustring CtTextCell::get_text_content() const
{
    Gtk::TextIter start_iter = _rTextBuffer->begin();
    Gtk::TextIter end_iter = _rTextBuffer->end();
    return start_iter.get_text(end_iter);
}

void CtTextCell::set_syntax_highlighting(const std::string& syntaxHighlighting, Gsv::LanguageManager* pGsvLanguageManager)
{
    _syntaxHighlighting = syntaxHighlighting;
    if (CtConst::RICH_TEXT_ID != syntaxHighlighting)
    {
        if (CtConst::PLAIN_TEXT_ID == syntaxHighlighting)
            _rTextBuffer->set_highlight_syntax(false);
        else
        {
            _rTextBuffer->set_language(pGsvLanguageManager->get_language(syntaxHighlighting));
            _rTextBuffer->set_highlight_syntax(true);
        }
    }
}


const Gsv::DrawSpacesFlags CtCodebox::DRAW_SPACES_FLAGS = Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE;

CtCodebox::CtCodebox(CtMainWin* pCtMainWin,
                     const Glib::ustring& textContent,
                     const std::string& syntaxHighlighting,
                     const int frameWidth,
                     const int frameHeight,
                     const int charOffset,
                     const std::string& justification,
                     const bool widthInPixels,
                     const bool highlightBrackets,
                     const bool showLineNumbers)
 : CtAnchoredWidget(pCtMainWin, charOffset, justification),
   CtTextCell(pCtMainWin, textContent, syntaxHighlighting),
   _frameWidth(frameWidth),
   _frameHeight(frameHeight),
   _key_down(false)
{
    _ctTextview.get_style_context()->add_class("ct-codebox");
    _ctTextview.set_border_width(1);

    if (!_pCtMainWin->get_ct_config()->codeboxAutoResize) {
        if (_frameHeight < MIN_SCROLL_HEIGHT) /* overwise not possible to have 20 px height*/
            _scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_EXTERNAL /* overwise not possible to have 20 px height*/);
        else
            _scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    } else {
        _scrolledwindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_NEVER);
        _ctTextview.set_wrap_mode(Gtk::WrapMode::WRAP_NONE);
    }

    _scrolledwindow.add(_ctTextview);
    _frame.add(_scrolledwindow);
    show_all();

    _ctTextview.set_monospace(true); // todo: remove than styles are implemented

    set_width_in_pixels(widthInPixels);
    set_highlight_brackets(highlightBrackets);
    set_show_line_numbers(showLineNumbers);

    // signals
    _ctTextview.signal_populate_popup().connect([this](Gtk::Menu* menu){
        if (not _pCtMainWin->user_active()) return;
        for (auto iter : menu->get_children()) menu->remove(*iter);
        _pCtMainWin->get_ct_actions()->curr_codebox_anchor = this;
        _pCtMainWin->get_ct_menu().build_popup_menu(menu, CtMenu::POPUP_MENU_TYPE::Codebox);
    });
    _ctTextview.signal_key_press_event().connect(sigc::mem_fun(*this, &CtCodebox::_on_key_press_event), false);
    _ctTextview.signal_key_release_event().connect([this](GdkEventKey*) { _key_down = false; return false; }, false);
    _ctTextview.signal_button_press_event().connect([this](GdkEventButton* event){
        if (not _pCtMainWin->user_active()) return false;
        _pCtMainWin->get_ct_actions()->curr_codebox_anchor = this;
        if ( event->button != 3 /* right button */ and
             event->type != GDK_3BUTTON_PRESS )
        {
            _pCtMainWin->get_ct_actions()->object_set_selection(this);
        }
        return false;
    });
    _ctTextview.signal_event_after().connect([this](GdkEvent* event){
        if (not _pCtMainWin->user_active()) return;
        if (event->type == GDK_2BUTTON_PRESS and event->button.button == 1)
            _ctTextview.for_event_after_double_click_button1(event);
        else if (event->type == GDK_BUTTON_PRESS)
            _ctTextview.for_event_after_button_press(event);
        else if (event->type == GDK_KEY_PRESS)
            _ctTextview.for_event_after_key_press(event, _syntaxHighlighting);
    });
    _ctTextview.signal_motion_notify_event().connect([this](GdkEventMotion* event){
        if (not _pCtMainWin->user_active()) return false;
        _ctTextview.set_editable(!_pCtMainWin->curr_tree_iter().get_node_read_only());
        int x, y;
        _ctTextview.window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, int(event->x), int(event->y), x, y);
        _ctTextview.cursor_and_tooltips_handler(x, y);
        return false;
    });
    _ctTextview.signal_scroll_event().connect([this](GdkEventScroll* event){
        if (!(event->state & GDK_CONTROL_MASK))
            return false;
        if  (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
            _ctTextview.zoom_text(event->direction == GDK_SCROLL_DOWN, get_syntax_highlighting());
        if  (event->direction == GDK_SCROLL_SMOOTH && event->delta_y != 0)
            _ctTextview.zoom_text(event->delta_y < 0, get_syntax_highlighting());
        return true;
    });
    _uCtPairCodeboxMainWin.reset(new CtPairCodeboxMainWin{this, _pCtMainWin});
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "cut-clipboard", G_CALLBACK(CtClipboard::on_cut_clipboard), _uCtPairCodeboxMainWin.get());
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "copy-clipboard", G_CALLBACK(CtClipboard::on_copy_clipboard), _uCtPairCodeboxMainWin.get());
}

CtCodebox::~CtCodebox()
{
}

void CtCodebox::apply_width_height(const int parentTextWidth)
{
    int frameWidth = _widthInPixels ? _frameWidth : (parentTextWidth*_frameWidth)/100;
    _scrolledwindow.set_size_request(frameWidth, _frameHeight);
}

void CtCodebox::apply_syntax_highlighting(const bool forceReApply)
{
    _pCtMainWin->apply_syntax_highlighting(get_buffer(), _syntaxHighlighting, forceReApply);
    _rTextBuffer->set_highlight_matching_brackets(_highlightBrackets);
}

void CtCodebox::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache*)
{
    // todo: fix code duplicates in void CtHtml2Xml::_insert_codebox()
    xmlpp::Element* p_codebox_node = p_node_parent->add_child("codebox");
    p_codebox_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_codebox_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_codebox_node->set_attribute("frame_width", std::to_string(_frameWidth));
    p_codebox_node->set_attribute("frame_height", std::to_string(_frameHeight));
    p_codebox_node->set_attribute("width_in_pixels", std::to_string(_widthInPixels));
    p_codebox_node->set_attribute("syntax_highlighting", _syntaxHighlighting);
    p_codebox_node->set_attribute("highlight_brackets", std::to_string(_highlightBrackets));
    p_codebox_node->set_attribute("show_line_numbers", std::to_string(_showLineNumbers));
    p_codebox_node->add_child_text(get_text_content());
}

bool CtCodebox::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache*)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_CODEBOX_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else
    {
        const std::string codebox_txt = get_text_content();
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, codebox_txt.c_str(), codebox_txt.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 5, _syntaxHighlighting.c_str(), _syntaxHighlighting.size(), SQLITE_STATIC);
        sqlite3_bind_int64(p_stmt, 6, _frameWidth);
        sqlite3_bind_int64(p_stmt, 7, _frameHeight);
        sqlite3_bind_int64(p_stmt, 8, _widthInPixels);
        sqlite3_bind_int64(p_stmt, 9, _highlightBrackets);
        sqlite3_bind_int64(p_stmt, 10, _showLineNumbers);
        if (sqlite3_step(p_stmt) != SQLITE_DONE)
        {
            spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

std::shared_ptr<CtAnchoredWidgetState> CtCodebox::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_Codebox(this));
}

void CtCodebox::set_show_line_numbers(const bool showLineNumbers)
{
    _showLineNumbers = showLineNumbers;
    _ctTextview.set_show_line_numbers(_showLineNumbers);
}

void CtCodebox::set_width_height(int newWidth, int newHeight)
{
    if (newWidth) _frameWidth = newWidth;
    if (newHeight) {
        _frameHeight = newHeight;
        if (!_pCtMainWin->get_ct_config()->codeboxAutoResize) {
            if (_frameHeight < MIN_SCROLL_HEIGHT) /* overwise not possible to have 20 px height*/
                _scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_EXTERNAL /* overwise not possible to have 20 px height*/);
            else
                _scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        }
    }
    apply_width_height(_ctTextview.get_allocation().get_width());
}

void CtCodebox::set_highlight_brackets(const bool highlightBrackets)
{
    _highlightBrackets = highlightBrackets;
    _rTextBuffer->set_highlight_matching_brackets(_highlightBrackets);
}

void CtCodebox::apply_cursor_pos(const int cursorPos)
{
    if (cursorPos > 0)
    {
        _rTextBuffer->place_cursor(_rTextBuffer->get_iter_at_offset(cursorPos));
    }
}

// Catches CodeBox Key Presses
bool CtCodebox::_on_key_press_event(GdkEventKey* event)
{
    _key_down = true;
    if (not _pCtMainWin->user_active())
        return false;
    if (event->state & Gdk::CONTROL_MASK)
    {
        _pCtMainWin->get_ct_actions()->curr_codebox_anchor = this;
        if (event->keyval == GDK_KEY_period)
        {
            if (event->state & Gdk::MOD1_MASK)
                _pCtMainWin->get_ct_actions()->codebox_decrease_width();
            else
                _pCtMainWin->get_ct_actions()->codebox_increase_width();
            return true;
        }
        else if (event->keyval == GDK_KEY_comma)
        {
            if (event->state & Gdk::MOD1_MASK)
                _pCtMainWin->get_ct_actions()->codebox_decrease_height();
            else
                _pCtMainWin->get_ct_actions()->codebox_increase_height();
            return true;
        }
        else if (event->keyval == GDK_KEY_space)
        {
            Gtk::TextIter text_iter = _pCtMainWin->get_text_view().get_buffer()->get_iter_at_child_anchor(getTextChildAnchor());
            text_iter.forward_char();
            _pCtMainWin->get_text_view().get_buffer()->place_cursor(text_iter);
            _pCtMainWin->get_text_view().grab_focus();
            return true;
        }
        if (event->keyval == GDK_KEY_plus || event->keyval == GDK_KEY_KP_Add || event->keyval == GDK_KEY_equal) {
            _ctTextview.zoom_text(true, get_syntax_highlighting());
            return true;
        }
        else if (event->keyval == GDK_KEY_minus|| event->keyval == GDK_KEY_KP_Subtract) {
            _ctTextview.zoom_text(false, get_syntax_highlighting());
            return true;
        }
    }
    //std::cout << "keyval " << event->keyval << std::endl;
    if (event->keyval == GDK_KEY_Tab or event->keyval == GDK_KEY_ISO_Left_Tab)
    {
        auto text_buffer = _ctTextview.get_buffer();
        if (not text_buffer->get_has_selection())
        {
            Gtk::TextIter iter_insert = text_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(_pCtMainWin, text_buffer).get_paragraph_list_info(iter_insert);
            bool backward = event->state & Gdk::SHIFT_MASK;
            if (list_info)
            {
                if (backward and list_info.level)
                {
                    _ctTextview.list_change_level(iter_insert, list_info, false);
                    return true;
                }
                else if (not backward)
                {
                    _ctTextview.list_change_level(iter_insert, list_info, true);
                    return true;
                }
            }
        }
    }
    return false;
}
