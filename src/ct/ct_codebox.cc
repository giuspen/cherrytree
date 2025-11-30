/*
 * ct_codebox.cc
 *
 * Copyright 2009-2025
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
#include "ct_actions.h"
#include "ct_storage_sqlite.h"
#include "ct_logging.h"

const constexpr int MIN_SCROLL_HEIGHT = 47;

CtTextCell::CtTextCell(CtMainWin* pCtMainWin,
                       const Glib::ustring& textContent,
                       const std::string& syntaxHighlighting)
 : _syntaxHighlighting{syntaxHighlighting}
 , _ctTextview{pCtMainWin}
{
    _rTextBuffer = pCtMainWin->get_new_text_buffer(textContent);
    _ctTextview.set_buffer(_rTextBuffer);
    _ctTextview.setup_for_syntax(_syntaxHighlighting);

    _rTextBuffer->signal_insert().connect([pCtMainWin, this](const Gtk::TextIter& pos, const Glib::ustring& text, int /*bytes*/) {
        if (pCtMainWin->user_active() and not _ctTextview.column_edit_get_own_insert_delete_active()) {
            _ctTextview.column_edit_text_inserted(pos, text);
            pCtMainWin->get_state_machine().text_variation(pCtMainWin->curr_tree_iter().get_node_id_data_holder(), text);
            pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
        }
    }, false);
    _rTextBuffer->signal_erase().connect([pCtMainWin, this](const Gtk::TextIter& range_start, const Gtk::TextIter& range_end) {
        if (pCtMainWin->user_active() and not _ctTextview.column_edit_get_own_insert_delete_active()) {
            _ctTextview.column_edit_text_removed(range_start, range_end);
            pCtMainWin->get_state_machine().text_variation(pCtMainWin->curr_tree_iter().get_node_id_data_holder(), range_start.get_text(range_end));
            pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
        }
    }, false);
    _rTextBuffer->signal_mark_set().connect([pCtMainWin, this](const Gtk::TextIter&, const Glib::RefPtr<Gtk::TextMark>& rMark){
        if (pCtMainWin->user_active() and not pCtMainWin->force_exit()) {
            CtTreeIter currTreeIter = pCtMainWin->curr_tree_iter();
            if (currTreeIter) {
                _ctTextview.mm().set_editable(not currTreeIter.get_node_read_only());
                if (rMark->get_name() == "insert") {
                    _ctTextview.column_edit_selection_update();
                }
            }
        }
    }, false);
    // GTK3 legacy event signals
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _ctTextview.mm().signal_event_after().connect([pCtMainWin, this](GdkEvent* event){
        if (not pCtMainWin->user_active()) return;
        if (event->type == GDK_2BUTTON_PRESS and (1 == event->button.button or 2 == event->button.button))
            _ctTextview.for_event_after_double_click_button12(event);
        else if (event->type == GDK_BUTTON_PRESS)
            _ctTextview.for_event_after_button_press(event);
        else if (event->type == GDK_KEY_PRESS)
            _ctTextview.for_event_after_key_press(event, _syntaxHighlighting);
    });
    _ctTextview.mm().signal_motion_notify_event().connect([pCtMainWin, this](GdkEventMotion* event){
        if (not pCtMainWin->user_active()) return false;
        int x, y;
        _ctTextview.mm().window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, int(event->x), int(event->y), x, y);
        _ctTextview.cursor_and_tooltips_handler(x, y);
        return false;
    });
    // Scroll-event zoom handler disabled for cross-version build stability
#endif
}

Glib::ustring CtTextCell::get_text_content() const
{
    Gtk::TextIter start_iter = _rTextBuffer->begin();
    Gtk::TextIter end_iter = _rTextBuffer->end();
    return start_iter.get_text(end_iter);
}

void CtTextCell::set_syntax_highlighting(const std::string& syntaxHighlighting, GtkSourceLanguageManager* pGtkSourceLanguageManager)
{
    _syntaxHighlighting = syntaxHighlighting;
    auto pGtkSourceBuffer = GTK_SOURCE_BUFFER(_rTextBuffer->gobj());
    if (CtConst::TABLE_CELL_TEXT_ID == syntaxHighlighting or
        CtConst::RICH_TEXT_ID == syntaxHighlighting or
        CtConst::PLAIN_TEXT_ID == syntaxHighlighting)
    {
        gtk_source_buffer_set_highlight_syntax(pGtkSourceBuffer, false);
    }
    else {
        GtkSourceLanguage* pGtkSourceLanguage = gtk_source_language_manager_get_language(pGtkSourceLanguageManager, syntaxHighlighting.c_str());
        if (pGtkSourceLanguage) {
            gtk_source_buffer_set_language(pGtkSourceBuffer, pGtkSourceLanguage);
            gtk_source_buffer_set_highlight_syntax(pGtkSourceBuffer, true);
        }
        else {
            spdlog::error("!! {} pGtkSourceLanguage '{}'", __FUNCTION__, syntaxHighlighting);
        }
    }
}

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
 : CtAnchoredWidget{pCtMainWin, charOffset, justification}
 , CtTextCell{pCtMainWin, textContent, syntaxHighlighting}
 , _frameWidth{frameWidth}
 , _frameHeight{frameHeight}
{
    _ctTextview.mm().get_style_context()->add_class("ct-codebox");
    // Border width API removed in GTK4; avoid using set_border_width
    _set_scrollbars_policies();
#if GTKMM_MAJOR_VERSION >= 4
    _scrolledwindow.set_child(_ctTextview.mm());
    _hbox.append(_scrolledwindow);
#else
    _scrolledwindow.add(_ctTextview.mm());
    _hbox.pack_start(_scrolledwindow, true/*expand*/, true/*fill*/);
#endif
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _toolbar.get_style_context()->add_class("ct-cboxtoolbar");
    _toolbar.set_property("orientation", Gtk::ORIENTATION_VERTICAL);
    _toolbar.set_toolbar_style(Gtk::ToolbarStyle::TOOLBAR_ICONS);
    _toolbar.set_icon_size(Gtk::ICON_SIZE_MENU);
    update_toolbar_buttons();
#if GTKMM_MAJOR_VERSION >= 4
    _hbox.append(_toolbar);
#else
    _hbox.pack_start(_toolbar, false/*expand*/, false/*fill*/);
#endif
#endif
#if GTKMM_MAJOR_VERSION >= 4
    _frame.set_child(_hbox);
#else
    _frame.add(_hbox);
    _frame.signal_size_allocate().connect(sigc::mem_fun(*this, &CtCodebox::_on_frame_size_allocate));
    show_all();
#endif

    set_width_in_pixels(widthInPixels);
    set_highlight_brackets(highlightBrackets);
    set_show_line_numbers(showLineNumbers);

    // signals
    // GTK3 legacy event signals
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _ctTextview.mm().signal_populate_popup().connect([this](Gtk::Menu* menu){
        if (not _pCtMainWin->user_active()) return;
        _pCtMainWin->get_ct_actions()->curr_codebox_anchor = this;
        _pCtMainWin->get_ct_menu().build_popup_menu(menu, CtMenu::POPUP_MENU_TYPE::Codebox);
    });
    _ctTextview.mm().signal_key_press_event().connect(sigc::mem_fun(*this, &CtCodebox::_on_key_press_event), false);
    _ctTextview.mm().signal_button_press_event().connect([this](GdkEventButton* event){
        if (not _pCtMainWin->user_active()) return false;
        _pCtMainWin->get_ct_actions()->curr_codebox_anchor = this;
        if ( event->button != 3 /* right button */ and
             event->type != GDK_3BUTTON_PRESS )
        {
            _pCtMainWin->get_ct_actions()->object_set_selection(this);
        }
        return false;
    });
#endif
    // Scroll adjustments for auto-resize
#if GTKMM_MAJOR_VERSION >= 4
    auto vAdj = _scrolledwindow.get_vadjustment();
    if (vAdj) {
        vAdj->signal_value_changed().connect([this, vAdj](){
            if (_pCtConfig->codeboxAutoResizeH) {
                vAdj->set_value(0);
            }
        });
    }
    auto hAdj = _scrolledwindow.get_hadjustment();
    if (hAdj) {
        hAdj->signal_value_changed().connect([this, hAdj](){
            if (_pCtConfig->codeboxAutoResizeW) {
                hAdj->set_value(0);
            }
        });
    }
#else
    _scrolledwindow.get_vscrollbar()->signal_value_changed().connect([this](){
        if (_pCtConfig->codeboxAutoResizeH) {
            _scrolledwindow.get_vscrollbar()->set_value(0);
        }
    });
    _scrolledwindow.get_hscrollbar()->signal_value_changed().connect([this](){
        if (_pCtConfig->codeboxAutoResizeW) {
            _scrolledwindow.get_hscrollbar()->set_value(0);
        }
    });
#endif
    _uCtPairCodeboxMainWin.reset(new CtPairCodeboxMainWin{this, _pCtMainWin});
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "cut-clipboard", G_CALLBACK(CtClipboard::on_cut_clipboard), _uCtPairCodeboxMainWin.get());
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "copy-clipboard", G_CALLBACK(CtClipboard::on_copy_clipboard), _uCtPairCodeboxMainWin.get());
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "paste-clipboard", G_CALLBACK(CtClipboard::on_paste_clipboard), _uCtPairCodeboxMainWin.get());
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _toolButtonPlay.signal_clicked().connect([this](){
        CtActions* pCtActions = _pCtMainWin->get_ct_actions();
        pCtActions->curr_codebox_anchor = this;
        pCtActions->object_set_selection(this);
        pCtActions->exec_code_all();
    });
    _toolButtonCopy.signal_clicked().connect([this](){
        CtActions* pCtActions = _pCtMainWin->get_ct_actions();
        pCtActions->curr_codebox_anchor = this;
        pCtActions->object_set_selection(this);
        pCtActions->codebox_copy_content();
    });
    _toolButtonProp.signal_clicked().connect([this](){
        CtActions* pCtActions = _pCtMainWin->get_ct_actions();
        pCtActions->curr_codebox_anchor = this;
        pCtActions->object_set_selection(this);
        pCtActions->codebox_change_properties();
    });
#endif
}

void CtCodebox::_set_scrollbars_policies()
{
    // Scroll policies and wrap mode differ between GTK3 and GTK4
#if GTKMM_MAJOR_VERSION >= 4
    const Gtk::PolicyType hscrollbar_policy = _pCtConfig->codeboxAutoResizeW ? Gtk::PolicyType::NEVER : Gtk::PolicyType::AUTOMATIC;
    const Gtk::PolicyType vscrollbar_policy = _pCtConfig->codeboxAutoResizeH ? Gtk::PolicyType::NEVER : Gtk::PolicyType::AUTOMATIC;
    _scrolledwindow.set_policy(hscrollbar_policy, vscrollbar_policy);
    const Gtk::WrapMode wrap_mode = (_pCtConfig->codeboxAutoResizeW || ! _pCtConfig->lineWrapping) ? Gtk::WrapMode::NONE : Gtk::WrapMode::WORD_CHAR;
#else
    const Gtk::PolicyType hscrollbar_policy = _pCtConfig->codeboxAutoResizeW ? Gtk::POLICY_NEVER : Gtk::POLICY_AUTOMATIC;
    const Gtk::PolicyType vscrollbar_policy = _pCtConfig->codeboxAutoResizeH ? Gtk::POLICY_NEVER : (_frameHeight < MIN_SCROLL_HEIGHT ? Gtk::POLICY_EXTERNAL : Gtk::POLICY_AUTOMATIC);
    _scrolledwindow.set_policy(hscrollbar_policy, vscrollbar_policy);
    const Gtk::WrapMode wrap_mode = _pCtConfig->codeboxAutoResizeW or not _pCtConfig->lineWrapping ? Gtk::WrapMode::WRAP_NONE : Gtk::WrapMode::WRAP_WORD_CHAR;
#endif
    _ctTextview.mm().set_wrap_mode(wrap_mode);
}

void CtCodebox::update_toolbar_buttons()
{
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _toolbar.foreach([this](Gtk::Widget& widget){ _toolbar.remove(widget); });
    if (_pCtConfig->codeboxWithToolbar) {
        _toolbar.set_tooltip_text(_syntaxHighlighting);
        if (CtConst::PLAIN_TEXT_ID != _syntaxHighlighting) {
            const std::string label_n_tooltip = fmt::format("[{}] - {}", _syntaxHighlighting, _("Execute Code"));
            _toolButtonPlay.set_icon_name("ct_play");
            _toolButtonPlay.set_label(label_n_tooltip);
            _toolButtonPlay.set_tooltip_text(label_n_tooltip);
            _toolbar.insert(_toolButtonPlay, -1);
        }
        _toolButtonCopy.set_icon_name("ct_edit_copy");
        _toolButtonCopy.set_label(_("Copy Code"));
        _toolButtonCopy.set_tooltip_text(_("Copy Code"));
        _toolbar.insert(_toolButtonCopy, -1);
        _toolButtonProp.set_icon_name("ct_codebox_edit");
        _toolButtonProp.set_label(_("Change CodeBox Properties"));
        _toolButtonProp.set_tooltip_text(_("Change CodeBox Properties"));
        _toolbar.insert(_toolButtonProp, -1);
    }
#else
    // No-op for GTK4 build where toolbar API differs
#endif
}

void CtCodebox::apply_width_height(const int parentTextWidth)
{
    int frameWidth = _widthInPixels ? _frameWidth : (parentTextWidth*_frameWidth)/100;
    _scrolledwindow.set_size_request(frameWidth, _frameHeight);
}

void CtCodebox::apply_syntax_highlighting(const bool forceReApply)
{
    Glib::RefPtr<Gtk::TextBuffer> pTextBuffer = get_buffer();
    _pCtMainWin->apply_syntax_highlighting(pTextBuffer, _syntaxHighlighting, forceReApply);
    gtk_source_buffer_set_highlight_matching_brackets(GTK_SOURCE_BUFFER(pTextBuffer->gobj()), _highlightBrackets);
}

void CtCodebox::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache*, const std::string&/*multifile_dir*/)
{
    // todo: fix code duplicates in void CtHtml2Xml::_insert_codebox()
    xmlpp::Element* p_codebox_node = p_node_parent->add_child("codebox");
    p_codebox_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_codebox_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_codebox_node->set_attribute("frame_width", std::to_string(get_frame_width()));
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
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_CODEBOX_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK) {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else {
        const std::string codebox_txt = get_text_content();
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, codebox_txt.c_str(), codebox_txt.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 5, _syntaxHighlighting.c_str(), _syntaxHighlighting.size(), SQLITE_STATIC);
        sqlite3_bind_int64(p_stmt, 6, get_frame_width());
        sqlite3_bind_int64(p_stmt, 7, _frameHeight);
        sqlite3_bind_int64(p_stmt, 8, _widthInPixels);
        sqlite3_bind_int64(p_stmt, 9, _highlightBrackets);
        sqlite3_bind_int64(p_stmt, 10, _showLineNumbers);
        if (sqlite3_step(p_stmt) != SQLITE_DONE) {
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
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(_ctTextview.gobj()), _showLineNumbers);
}

void CtCodebox::set_width_height(int newWidth, int newHeight)
{
    if (newWidth) {
        _frameWidth = newWidth;
    }
    if (newHeight) {
        _frameHeight = newHeight;
        if (not _pCtConfig->codeboxAutoResizeH) {
            _set_scrollbars_policies();
        }
    }
    apply_width_height(_pCtMainWin->get_text_view().mm().get_allocation().get_width());
}

void CtCodebox::set_highlight_brackets(const bool highlightBrackets)
{
    _highlightBrackets = highlightBrackets;
    gtk_source_buffer_set_highlight_matching_brackets(GTK_SOURCE_BUFFER(_rTextBuffer->gobj()), _highlightBrackets);
}

void CtCodebox::apply_cursor_pos(const int cursorPos)
{
    if (cursorPos > 0) {
        _rTextBuffer->place_cursor(_rTextBuffer->get_iter_at_offset(cursorPos));
    }
}

// Catches CodeBox Key Presses
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
bool CtCodebox::_on_key_press_event(GdkEventKey* event)
{
    if (not _pCtMainWin->user_active())
        return false;
    if (event->state & Gdk::CONTROL_MASK) {
        _pCtMainWin->get_ct_actions()->curr_codebox_anchor = this;
        if (not (event->state & Gdk::MOD1_MASK)) {
            if (GDK_KEY_space == event->keyval) {
                Gtk::TextIter text_iter = _pCtMainWin->get_text_view().get_buffer()->get_iter_at_child_anchor(getTextChildAnchor());
                text_iter.forward_char();
                _pCtMainWin->get_text_view().get_buffer()->place_cursor(text_iter);
                _pCtMainWin->get_text_view().mm().grab_focus();
                return true;
            }
            if (GDK_KEY_plus == event->keyval or GDK_KEY_KP_Add == event->keyval or GDK_KEY_equal == event->keyval) {
                _ctTextview.zoom_text(true, get_syntax_highlighting());
                return true;
            }
            if (GDK_KEY_minus == event->keyval or GDK_KEY_KP_Subtract == event->keyval) {
                _ctTextview.zoom_text(false, get_syntax_highlighting());
                return true;
            }
            if (GDK_KEY_0 == event->keyval or GDK_KEY_KP_0 == event->keyval) {
                _ctTextview.zoom_text(std::nullopt, get_syntax_highlighting());
                return true;
            }
        }
    }
    if (GDK_KEY_Tab == event->keyval or GDK_KEY_ISO_Left_Tab == event->keyval) {
        auto text_buffer = _ctTextview.get_buffer();
        if (not text_buffer->get_has_selection()) {
            Gtk::TextIter iter_insert = text_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList{_pCtConfig, text_buffer}.get_paragraph_list_info(iter_insert);
            bool backward = event->state & Gdk::SHIFT_MASK;
            if (list_info) {
                if (backward and list_info.level) {
                    _ctTextview.list_change_level(iter_insert, list_info, false);
                    return true;
                }
                if (not backward) {
                    _ctTextview.list_change_level(iter_insert, list_info, true);
                    return true;
                }
            }
        }
    }
    return false;
}
#endif
