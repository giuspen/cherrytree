/*
 * ct_widgets.h
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

#pragma once

#include "ct_types.h"
#include "ct_filesystem.h"
#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include <libxml++/libxml++.h>
#include <sqlite3.h>
#include <unordered_map>
#include <memory>
#include <gspell/gspell.h>
#include <map>
#include "ct_types.h"
#include "ct_filesystem.h"
#include "ct_column_edit.h"

class CtMDParser;
class CtClipboard;
class CtTmp
{
public:
    CtTmp() {}
    virtual ~CtTmp();
    fs::path getHiddenDirPath(const fs::path& visiblePath);
    fs::path getHiddenFilePath(const fs::path& visiblePath);

protected:
    std::unordered_map<std::string, gchar*> _mapHiddenDirs;
    std::unordered_map<std::string, gchar*> _mapHiddenFiles;
};

class CtMainWin;
class CtAnchoredWidgetState;
class CtStorageCache;
class CtMarkdownFilter;

class CtAnchoredWidget : public Gtk::EventBox
{
public:
    CtAnchoredWidget(CtMainWin* pCtMainWin, const int charOffset, const std::string& justification);
    virtual ~CtAnchoredWidget();

    void insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer);
    Glib::RefPtr<Gtk::TextChildAnchor> getTextChildAnchor() { return _rTextChildAnchor; }

    virtual void apply_width_height(const int parentTextWidth) = 0;
    virtual void apply_syntax_highlighting(const bool forceReApply) = 0;
    virtual void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache) = 0;
    virtual bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) = 0;
    virtual void set_modified_false() = 0;
    virtual CtAnchWidgType get_type() = 0;
    virtual std::shared_ptr<CtAnchoredWidgetState> get_state() = 0;

    void updateOffset(int charOffset) { _charOffset = charOffset; }
    void updateJustification(const std::string& justification) { _justification = justification; }
    void updateJustification(const Gtk::TextIter& textIter);

    int getOffset() const { return _charOffset; }
    const std::string& getJustification() const { return _justification; }

    bool operator<(const CtAnchoredWidget &other) { return getOffset() < other.getOffset(); }
    bool operator>(const CtAnchoredWidget &other) { return getOffset() > other.getOffset(); }

protected:
    CtMainWin* _pCtMainWin;
    int _charOffset;
    std::string _justification;
    Gtk::Frame _frame;
    Gtk::Label _labelWidget;
    Glib::RefPtr<Gtk::TextChildAnchor> _rTextChildAnchor;
};

class CtTreeView : public Gtk::TreeView
{
public:
    const inline static int TITLE_COL_NUM = 0;
    const inline static int AUX_ICON_COL_NUM = 1;

public:
    CtTreeView();
    virtual ~CtTreeView();

    void set_cursor_safe(const Gtk::TreeIter& iter);
    void set_tree_node_name_wrap_width(const bool wrap_enabled, const int wrap_width);
};

class CtTextView : public Gsv::View
{
public:
    CtTextView(CtMainWin* pCtMainWin);
    virtual ~CtTextView();

    CtMainWin*   getCtMainWin() { return _pCtMainWin; }

    void setup_for_syntax(const std::string& syntaxHighlighting); // pygtk: sourceview_set_properties
    void set_pixels_inside_wrap(int space_around_lines, int relative_wrapped_space);
    void set_selection_at_offset_n_delta(int offset, int delta, Glib::RefPtr<Gtk::TextBuffer> text_buffer = Glib::RefPtr<Gtk::TextBuffer>());
    void list_change_level(Gtk::TextIter iter_insert, const CtListInfo& list_info, bool level_increase);
    void replace_text(const Glib::ustring& text, int start_offset, int end_offset);

    void for_event_after_double_click_button1(GdkEvent* event);
    void for_event_after_triple_click_button1(GdkEvent* event);
    void for_event_after_button_press(GdkEvent* event);
    void for_event_after_key_press(GdkEvent* event, const Glib::ustring& syntaxHighlighting);

    void cursor_and_tooltips_handler(int x, int y);
    void zoom_text(const bool is_increase, const std::string& syntaxHighlighting);
    void set_spell_check(bool allow_on);
    void synch_spell_check_change_from_gspell_right_click_menu();

    void set_buffer(const Glib::RefPtr<Gtk::TextBuffer>& buffer);
    void selection_update() {
        _columnEdit.selection_update();
    }
    void text_inserted(const Gtk::TextIter& pos, const Glib::ustring& text) {
        _columnEdit.text_inserted(pos, text);
    }
    void text_removed(const Gtk::TextIter& range_start, const Gtk::TextIter& range_end) {
        _columnEdit.text_removed(range_start, range_end);
    }
    bool own_insert_delete_active() {
        return _columnEdit.own_insert_delete_active();
    }
    guint64 get_todo_rotate_time() { return _todoRotateTime; }

private:
    bool          _apply_tag_try_link(Gtk::TextIter iter_end, int offset_cursor);
    Glib::ustring _get_former_line_indentation(Gtk::TextIter iter_start);
    void          _special_char_replace(gunichar special_char, Gtk::TextIter iter_start, Gtk::TextIter iter_insert);
    /// Replace the char between iter_start and iter_end with another one
    void          _special_char_replace(Glib::ustring special_char, Gtk::TextIter iter_start, Gtk::TextIter iter_end);

#ifdef MD_AUTO_REPLACEMENT
    bool          _markdown_filter_active();
#endif // MD_AUTO_REPLACEMENT

public:
    static const double TEXT_SCROLL_MARGIN;

private:
    static std::map<std::string, GspellChecker*> _static_spell_checkers;
    static GspellChecker* _get_spell_checker(const std::string& lang);

private:
#ifdef MD_AUTO_REPLACEMENT
    std::unique_ptr<CtMarkdownFilter> _md_handler;
#endif // MD_AUTO_REPLACEMENT
    CtMainWin*   _pCtMainWin;
    CtColumnEdit _columnEdit;
    guint32      _todoRotateTime{0};
};
