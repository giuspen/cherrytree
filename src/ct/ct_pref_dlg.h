/*
 * ct_pref_dlg.h
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

#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <glibmm/value.h>
#include <glibmm/ustring.h>
#include "ct_menu.h"

class CtMainWin;

class CtPrefDlg : public Gtk::Dialog
{
public:
    CtPrefDlg(CtMainWin* pCtMainWin);
    virtual ~CtPrefDlg() override;

private:
    Gtk::Widget* build_tab_text_n_code();
    Gtk::Widget* build_tab_text();
    Gtk::Widget* build_tab_rich_text();
    Gtk::Widget* build_tab_plain_text_n_code();
    Gtk::Widget* build_tab_tree_1();
    Gtk::Widget* build_tab_tree_2();
    Gtk::Widget* build_tab_fonts();
    Gtk::Widget* build_tab_links();
    Gtk::Widget* build_tab_toolbar();
    Gtk::Widget* build_tab_kb_shortcuts();
    Gtk::Widget* build_tab_misc();

private:
    enum RESTART_REASON {MONOSPACE         = 1 << 0, EMBFILE_SIZE = 1 << 1,
                         SHOW_EMBFILE_NAME = 1 << 2, LINKS        = 1 << 3,
                         ANCHOR_SIZE       = 1 << 4, COLOR        = 1 << 5,
                         SCHEME            = 1 << 6, LANG         = 1 << 7,
                         SHORTCUT          = 1 << 9};

    const Glib::ustring reset_warning = Glib::ustring("<b>")+_("Are you sure to Reset to Default?")+"</b>";

private:
    void need_restart(RESTART_REASON reason, const gchar* msg = nullptr);


    void fill_custom_exec_commands_model(Glib::RefPtr<Gtk::ListStore> model);
    void add_new_command_in_model(Glib::RefPtr<Gtk::ListStore> model);

    void fill_toolbar_model(Glib::RefPtr<Gtk::ListStore> model);
    void add_new_item_in_toolbar_model(Gtk::TreeIter row, const Glib::ustring& key);
    bool add_new_item_in_toolbar_model(Gtk::TreeView* treeview, Glib::RefPtr<Gtk::ListStore> model);
    void update_config_toolbar_from_model(Glib::RefPtr<Gtk::ListStore> model);

    void fill_shortcut_model(Glib::RefPtr<Gtk::TreeStore> model);
    bool edit_shortcut(Gtk::TreeView* treeview);
    bool edit_shortcut_dialog(std::string& shortcut);

    void apply_for_each_window(std::function<void(CtMainWin*)> callback);

public:
    static std::string get_code_exec_term_run(CtMainWin* pCtMainWin);

private:
    struct UniversalModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<Glib::ustring>  icon;
       Gtk::TreeModelColumn<Glib::ustring>  key;
       Gtk::TreeModelColumn<Glib::ustring>  desc;
       Gtk::TreeModelColumn<Glib::ustring>  shortcut;
       UniversalModelColumns() { add(icon); add(key); add(desc); add(shortcut); }
       virtual ~UniversalModelColumns();
    };

private:
    UniversalModelColumns _commandModelColumns;
    UniversalModelColumns _toolbarModelColumns;
    UniversalModelColumns _shortcutModelColumns;
    CtMainWin*            _pCtMainWin;
    CtMenu*               _pCtMenu;
    int                   _restartReasons;
};
