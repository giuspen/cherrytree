/*
 * ct_pref_dlg.h
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

#pragma once

#include <glibmm/i18n.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <glibmm/value.h>
#include <glibmm/ustring.h>
#include "ct_menu.h"

class CtMainWin;
class CtConfig;

class CtPrefDlg : public Gtk::Dialog
{
public:
    CtPrefDlg(CtMainWin* pCtMainWin);

private:
    Gtk::Widget* build_tab_text_n_code();
    Gtk::Widget* build_tab_rich_text();
    Gtk::Widget* build_tab_format();
    Gtk::Widget* build_tab_plain_text_n_code();
    Gtk::Widget* build_tab_special_characters();
    Gtk::Widget* build_tab_tree();
    Gtk::Widget* build_tab_theme();
    Gtk::Widget* build_tab_fonts();
    Gtk::Widget* build_tab_links();
    Gtk::Widget* build_tab_toolbar();
    Gtk::Widget* build_tab_kb_shortcuts();
    Gtk::Widget* build_tab_misc();

private:
    enum RESTART_REASON {MONOSPACE         = 1 << 0,  EMBFILE_SIZE       = 1 << 1,
                         SHOW_EMBFILE_NAME = 1 << 2,  LINKS              = 1 << 3,
                         ANCHOR_SIZE       = 1 << 4,  COLOR              = 1 << 5,
                         SCALABLE_TAGS     = 1 << 6,  LANG               = 1 << 7,
                         SHORTCUT          = 1 << 8,  CODEBOX_AUTORESIZE = 1 << 9,
                         TREE_NODE_WRAP    = 1 << 10};

    const Glib::ustring reset_warning = Glib::ustring{"<b>"}+_("Are you sure to Reset to Default?")+"</b>";

private:
    void need_restart(RESTART_REASON reason, const gchar* msg = nullptr);

    void _fill_custom_exec_commands_model(Glib::RefPtr<Gtk::ListStore> rModel);
    void _add_new_command_in_model(Gtk::TreeView* pTreeview, Glib::RefPtr<Gtk::ListStore> rModel);
    void _remove_command_from_model(Gtk::TreeView* pTreeview, Glib::RefPtr<Gtk::ListStore> rModel);
    std::set<std::string> _get_code_exec_type_keys();

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
    static std::string get_code_exec_type_cmd(CtMainWin* pCtMainWin, const std::string code_type);
    static std::string get_code_exec_ext(CtMainWin* pCtMainWin, const std::string code_type);
    static Gtk::Frame* new_managed_frame_with_align(const Glib::ustring& frameLabel, Gtk::Widget* pFrameChild);

private:
    struct UniversalModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<Glib::ustring>  icon;
       Gtk::TreeModelColumn<Glib::ustring>  key;
       Gtk::TreeModelColumn<Glib::ustring>  ext;
       Gtk::TreeModelColumn<Glib::ustring>  desc;
       Gtk::TreeModelColumn<Glib::ustring>  shortcut;
       UniversalModelColumns() { add(icon); add(key); add(ext); add(desc); add(shortcut); }
    };

private:
    UniversalModelColumns _commandModelColumns;
    UniversalModelColumns _toolbarModelColumns;
    UniversalModelColumns _shortcutModelColumns;
    CtMainWin* const      _pCtMainWin;
    CtMenu* const         _pCtMenu;
    CtConfig* const       _pConfig;
    int                   _restartReasons{0};
    const std::map<std::string, Glib::ustring> _mapCountryLanguages;
};
