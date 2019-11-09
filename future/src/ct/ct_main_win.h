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
#include <gtkmm/statusbar.h>
#include "ct_treestore.h"
#include "ct_misc_utils.h"
#include "ct_menu.h"
#include "ct_widgets.h"

struct CtStatusBar
{
    Gtk::Statusbar   statusBar;
    guint            statusId;
    Gtk::ProgressBar progressBar;
    Gtk::Button      stopButton;
    Gtk::Frame       frame;
    Gtk::HBox        hbox;

    void set_progress_stop(bool stop) { _progress_stop = stop; }
    bool is_progress_stop()           { return _progress_stop; }
    void update_status(const Glib::ustring& text) { statusBar.pop(statusId); statusBar.push(text, statusId); }

private:
    bool _progress_stop;
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

enum class CtSaveNeededUpdType { None, nbuf, npro, ndel, book };

class CtMenu;
class CtActions;
class CtMainWin : public Gtk::ApplicationWindow
{
public:
    CtMainWin(CtMenu* pCtMenu);
    virtual ~CtMainWin();

    bool read_nodes_from_gio_file(const Glib::RefPtr<Gio::File>& r_file, const bool isImport);
    void config_apply_before_show_all();
    void config_apply_after_show_all();
    void configure_theme();
    void update_window_save_needed(const CtSaveNeededUpdType update_type = CtSaveNeededUpdType::None,
                                   const bool new_machine_state = false,
                                   const CtTreeIter* give_tree_iter = nullptr);
    void update_window_save_not_needed();
    bool get_file_save_needed();
    std::string get_curr_doc_file_path() { return (_ctCurrFile.rFile ? _ctCurrFile.rFile->get_path():""); }
    std::string get_curr_doc_file_name() { return (_ctCurrFile.rFile ? Glib::path_get_basename(_ctCurrFile.rFile->get_path()):""); } // pygtk: file_name
    std::string get_curr_doc_file_dir() { return (_ctCurrFile.rFile ? Glib::path_get_dirname(_ctCurrFile.rFile->get_path()):""); } // pygtk: file_dir
    void curr_file_mod_time_update_value(const bool doEnable); // pygtk: modification_time_update_value

    CtTreeIter    curr_tree_iter()  { return _ctTreestore.to_ct_tree_iter(_ctTreeview.get_selection()->get_selected()); }
    CtTreeStore&  get_tree_store()  { return _ctTreestore; }
    CtTreeView&   get_tree_view()   { return _ctTreeview; }
    CtTextView&   get_text_view()   { return _ctTextview; }
    CtMenu&       get_ct_menu()     { return *_pCtMenu; }
    CtStatusBar&  get_status_bar()  { return _ctStatusBar; }

    bool&         user_active()     { return _userActive; } // use as a function, because it's easier to put breakpoint
    int&          cursor_key_press() { return _cursorKeyPress; }
    int&          hovering_link_iter_offset() { return _hovering_link_iter_offset; }

    Glib::RefPtr<Gtk::TextBuffer> curr_buffer() { return _ctTextview.get_buffer(); }

private:
    Gtk::HBox&     _init_status_bar();
    Gtk::EventBox& _init_window_header();

public:
    void window_header_update();
    void window_header_update_lock_icon(bool show);
    void window_header_update_bookmark_icon(bool show);
    void window_header_update_last_visited();
    void window_header_update_num_last_visited();

    void menu_tree_update_for_bookmarked_node(bool is_bookmarked);
    void bookmark_action_select_node(gint64 node_id);
    void set_bookmarks_menu_items();

    void set_menu_items_special_chars();

    void show_hide_toolbar(bool visible)    { _pToolbar->property_visible() = visible; }
    void show_hide_tree_view(bool visible)  { _scrolledwindowTree.property_visible() = visible; }
    void show_hide_win_header(bool visible) { _ctWinHeader.headerBox.property_visible() = visible; }
    void set_toolbar_icon_size(int size)    { _pToolbar->property_icon_size() = CtMiscUtil::getIconSize(size); }

private:
    bool                _on_window_key_press_event(GdkEventKey* event);

    void                _on_treeview_cursor_changed(); // pygtk: on_node_changed
    bool                _on_treeview_button_release_event(GdkEventButton* event);
    bool                _on_treeview_key_press_event(GdkEventKey* event);
    bool                _on_treeview_popup_menu();

    void                _on_textview_populate_popup(Gtk::Menu* menu);
    bool                _on_textview_motion_notify_event(GdkEventMotion* event);
    bool                _on_textview_visibility_notify_event(GdkEventVisibility* event);
    void                _on_textview_size_allocate(Gtk::Allocation& allocation);
    bool                _on_textview_event(GdkEvent* event); // pygtk: on_sourceview_event
    void                _on_textview_event_after(GdkEvent* event); // pygtk: on_sourceview_event_after

    void                _title_update(const bool saveNeeded); // pygtk: window_title_update

private:
    Gtk::VBox           _vboxMain;
    Gtk::VBox           _vboxText;
    Gtk::HPaned         _hPaned;
    Gtk::MenuBar*       _pMenuBar;
    Gtk::Toolbar*       _pToolbar;
    CtMenu*             _pCtMenu;
    CtStatusBar         _ctStatusBar;
    CtWinHeader         _ctWinHeader;
    Gtk::MenuItem*      _pBookmarksSubmenu;
    Gtk::MenuItem*      _pSpecialCharsSubmenu;
    Gtk::ScrolledWindow _scrolledwindowTree;
    Gtk::ScrolledWindow _scrolledwindowText;
    CtTreeStore         _ctTreestore;
    CtTreeView          _ctTreeview;
    CtTextView          _ctTextview; /* todo: rename? because _ctTextview and _ctTreeview look the same */

    struct CtCurrFile
    {
        Glib::RefPtr<Gio::File> rFile;
        double modTime{0};
    };
    CtCurrFile _ctCurrFile;

    Glib::RefPtr<Gtk::CssProvider> _css_provider_theme;
    Glib::RefPtr<Gtk::CssProvider> _css_provider_theme_font;

private:
    bool                _userActive{true}; // pygtk: user_active
    int                 _cursorKeyPress{-1};
    int                 _hovering_link_iter_offset{-1};
    int                 _prevTextviewWidth{0};
    bool                _fileSaveNeeded{false}; // pygtk: file_update
    std::unordered_map<gint64, gint64> _latestStatusbarUpdateTime; // pygtk: latest_statusbar_update_time
    CtTreeIter          _prevTreeIter;
};
