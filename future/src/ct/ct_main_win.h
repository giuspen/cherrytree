/*
 * ct_main_win.h
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

#pragma once

#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include "ct_treestore.h"
#include "ct_misc_utils.h"
#include "ct_menu.h"

class CtTreeView : public Gtk::TreeView
{
public:
    CtTreeView();
    virtual ~CtTreeView();
    void set_cursor_safe(const Gtk::TreeIter& iter);

protected:
};

class CtTextView : public Gsv::View
{
public:
    CtTextView();
    virtual ~CtTextView();

    void setupForSyntax(const std::string& syntaxHighlighting);
    void set_pixels_inside_wrap(int space_around_lines, int relative_wrapped_space);
    void set_selection_at_offset_n_delta(int offset, int delta, Glib::RefPtr<Gtk::TextBuffer> text_buffer = Glib::RefPtr<Gtk::TextBuffer>());

    static const double TEXT_SCROLL_MARGIN;
protected:
    void _setFontForSyntax(const std::string& syntaxHighlighting);
};

class CtDialogTextEntry : public Gtk::Dialog
{
public:
    CtDialogTextEntry(const char* title, const bool forPassword, Gtk::Window* pParent);
    virtual ~CtDialogTextEntry();
    Glib::ustring getEntryText();

protected:
    bool _onEntryKeyPress(GdkEventKey* eventKey);
    void _onEntryIconPress(Gtk::EntryIconPosition iconPosition, const GdkEventButton* event);
    Gtk::Entry _entry;
};

struct CtWinHeader
{
    Gtk::HBox        headerBox;
    Gtk::HButtonBox  buttonBox;
    Gtk::Label       nameLabel;
    Gtk::Image       lockIcon;
    Gtk::Image       bookmarkIcon;
    Gtk::EventBox    eventBox;
};

class CtMenu;
class CtActions;
class CtMainWin : public Gtk::ApplicationWindow
{
public:
    CtMainWin(CtMenu* pCtMenu);
    virtual ~CtMainWin();

    bool readNodesFromGioFile(const Glib::RefPtr<Gio::File>& r_file, const bool isImport);
    void configApply();
    void update_window_save_needed(const std::string& update_type = "",
                                   bool new_machine_state = false, void* give_tree_iter = nullptr) { /* todo: */ }

    CtTreeIter    curr_tree_iter();
    CtTreeStore&  get_tree_store();
    CtTreeView&   get_tree_view();
    CtTextView&   get_text_view();

private:
    Gtk::EventBox& _initWindowHeader();

public:
    void window_header_update();
    void window_header_update_lock_icon(bool show);
    void window_header_update_bookmark_icon(bool show);
    void window_header_update_last_visited();
    void window_header_update_num_last_visited();

    void treeview_set_colors();
    void menu_tree_update_for_bookmarked_node(bool is_bookmarked);
    void bookmark_action_select_node(gint64 node_id);
    void set_bookmarks_menu_items();

protected:
    void                _onTheTreeviewSignalCursorChanged();
    bool                _onTheTreeviewSignalButtonPressEvent(GdkEventButton* event);
    bool                _onTheTreeviewSignalKeyPressEvent(GdkEventKey* event);
    bool                _onTheTreeviewSignalPopupMenu();
    void                _titleUpdate(bool saveNeeded);

    Gtk::VBox           _vboxMain;
    Gtk::VBox           _vboxText;
    Gtk::HPaned         _hPaned;
    Gtk::MenuBar*       _pMenu;
    CtMenu*             _ctMenu;
    Gtk::MenuItem*      _pBookmarksSubmenu;
    Gtk::Menu*          _pNodePopup;
    CtWinHeader         _windowHeader;
    Gtk::ScrolledWindow _scrolledwindowTree;
    Gtk::ScrolledWindow _scrolledwindowText;
    CtTreeStore         _ctTreestore;
    CtTreeView          _ctTreeview;
    CtTextView          _ctTextview;
    std::string         _currFileName;
    std::string         _currFileDir;
};
