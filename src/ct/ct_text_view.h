/*
 * ct_text_view.h
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

#pragma once

#include "ct_types.h"
#include "ct_column_edit.h"

#include <gtkmm/textview.h>
#include <gspell/gspell.h>
#include <memory>
#include <unordered_map>

class CtMainWin;
struct CtStatusBar;

class CtTextView
{
public:
    CtTextView(CtMainWin* pCtMainWin);
    virtual ~CtTextView();

    GtkSourceView* gobj() { return _pGtkSourceView; }
    Gtk::TextView& mm() { return *_pTextView; }
    const Gtk::TextView& mm() const { return *_pTextView; }
    Glib::RefPtr<Gtk::TextBuffer> get_buffer() { return _pTextView->get_buffer(); }
    Glib::RefPtr<const Gtk::TextBuffer> get_buffer() const { return _pTextView->get_buffer(); }

    void setup_for_syntax(const std::string& syntaxHighlighting); // pygtk: sourceview_set_properties
    void set_pixels_inside_wrap(int space_around_lines, int relative_wrapped_space);
    void set_selection_at_offset_n_delta(const int offset,
                                         const int delta,
                                         Glib::RefPtr<Gtk::TextBuffer> pTextBuffer = Glib::RefPtr<Gtk::TextBuffer>{});
    int expand_collapsed_anchors(const int offset,
                                 const int delta,
                                 Glib::RefPtr<Gtk::TextBuffer> pTextBuffer = Glib::RefPtr<Gtk::TextBuffer>{});
    void list_change_level(Gtk::TextIter iter_insert, const CtListInfo& list_info, bool level_increase);
    void replace_text(const Glib::ustring& text, int start_offset, int end_offset);

    void for_event_after_double_click_button12(GdkEvent* event);
    void for_event_after_triple_click_button12(GdkEvent* event);
    void for_event_after_button_press(GdkEvent* event);
    void for_event_after_key_press(GdkEvent* event, const Glib::ustring& syntaxHighlighting);

    void cursor_and_tooltips_handler(int x, int y);
    void cursor_and_tooltips_reset();
    void zoom_text(const std::optional<bool> is_increase, const std::string& syntaxHighlighting);
    void set_spell_check(bool allow_on);
    void synch_spell_check_change_from_gspell_right_click_menu();

    void set_buffer(const Glib::RefPtr<Gtk::TextBuffer>& buffer);
    CtColEditState column_edit_get_state() const {
        return _columnEdit.get_state();
    }
    Glib::ustring column_edit_copy() const {
        return _columnEdit.copy();
    }
    Glib::ustring column_edit_cut() {
        return _columnEdit.cut();
    }
    void column_edit_paste(const std::string& column_txt) {
        _columnEdit.paste(column_txt);
    }
    void column_edit_selection_update() {
        _columnEdit.selection_update();
    }
    void column_edit_text_inserted(const Gtk::TextIter& pos, const Glib::ustring& text) {
        _columnEdit.text_inserted(pos, text);
    }
    void column_edit_text_removed(const Gtk::TextIter& range_start, const Gtk::TextIter& range_end) {
        _columnEdit.text_removed(range_start, range_end);
    }
    bool column_edit_get_ctrl_down() {
        return _columnEdit.get_ctrl_down();
    }
    bool column_edit_get_alt_down() {
        return _columnEdit.get_alt_down();
    }
    bool column_edit_get_own_insert_delete_active() {
        return _columnEdit.get_own_insert_delete_active();
    }
    guint64 get_todo_rotate_time() { return _todoRotateTime; }
    std::string get_syntax_highlighting() { return _syntaxHighlighting; }

private:
    bool          _apply_tag_try_link(Gtk::TextIter iter_end, int offset_cursor);
    Glib::ustring _get_former_line_indentation(Gtk::TextIter iter_start);
    void          _special_char_replace(gunichar special_char, Gtk::TextIter iter_start, Gtk::TextIter iter_insert);
    /// Replace the char between iter_start and iter_end with another one
    void          _special_char_replace(Glib::ustring special_char, Gtk::TextIter iter_start, Gtk::TextIter iter_end);
    void          _set_highlight_current_line_enabled(const bool enabled);

    #if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    bool _on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int, int, guint time);
    void _on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context,
                                int x,
                                int y,
                                const Gtk::SelectionData& selection_data,
                                guint info,
                                guint time);
    void _on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& context);
    void _on_drag_end(const Glib::RefPtr<Gdk::DragContext>& context);
    #else
    // GTK4 drag & drop handled via event controllers (DragSource/DropTarget)
    void _setup_drag_and_drop_gtk4();
    void _on_drag_source_prepare_gtk4(Gdk::Drag& drag);
    void _on_drop_target_drop_gtk4(const Glib::ValueBase& value, double x, double y);
    // GTK4 controller refs and state for internal rich text drag
    Glib::RefPtr<Gtk::DragSource> _dragSource4;
    Glib::RefPtr<Gtk::DropTarget> _dropTarget4;
    Glib::ustring _drag_serialized_rich_text4;
    int _drag_start_offset4{0};
    int _drag_end_offset4{0};
    bool _is_internal_drag4{false};
    #endif

    bool _is_internal_drag{false};
    int _drag_start_offset;
    int _drag_end_offset;

#ifdef MD_AUTO_REPLACEMENT
    bool          _markdown_filter_active();
#endif // MD_AUTO_REPLACEMENT

public:
    static const double TEXT_SCROLL_MARGIN;

private:
    static std::unordered_map<std::string, GspellChecker*> _static_spell_checkers;
    static GspellChecker* _get_spell_checker(const std::string& lang);

private:
#ifdef MD_AUTO_REPLACEMENT
    std::unique_ptr<CtMarkdownFilter> _md_handler;
#endif // MD_AUTO_REPLACEMENT
    CtMainWin* const _pCtMainWin;
    CtConfig* const _pCtConfig;
    CtStatusBar* const _pCtStatusBar;
    GtkSourceView* const _pGtkSourceView;
    Gtk::TextView* _pTextView;
    CtColumnEdit _columnEdit;
    guint32      _todoRotateTime{0};
    std::string  _syntaxHighlighting;
};
