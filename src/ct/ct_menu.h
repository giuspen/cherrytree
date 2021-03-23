/*
 * ct_menu.h
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

#include <string>
#include <libxml++/libxml++.h>
#include <gtkmm.h>
#include "ct_types.h"

class CtConfig;

struct CtMenuAction
{
    std::string category;
    std::string id;
    std::string image;
    std::string name;
    std::string built_in_shortcut;
    std::string desc;
    sigc::slot<void> run_action;

    sigc::signal<void, bool> signal_set_sensitive = sigc::signal<void, bool>();
    sigc::signal<void, bool> signal_set_visible = sigc::signal<void, bool>();

    const std::string& get_shortcut(CtConfig* pCtConfig) const;
};

class CtApp;
class CtActions;

class CtMenu
{
public:
    CtMenu(CtConfig* pCtConfig, CtActions* pActions);

public:
    const char*       None       = "";
    const std::string KB_CONTROL = "<control>";
    const std::string KB_SHIFT   = "<shift>";
    const std::string KB_ALT     = "<alt>";
    const std::string KB_META    = "<meta>";

    enum POPUP_MENU_TYPE {Node, Text, Code, Link, Codebox, Image, Anchor, EmbFile, PopupMenuNum };

public:
   static Gtk::MenuItem* create_menu_item(Gtk::Menu* pMenu, const char* name, const char* image, const char* desc);

public:
    void init_actions(CtActions* pActions);

    CtMenuAction*  find_action(const std::string& id);
    const std::list<CtMenuAction>& get_actions() { return _actions; }
    Glib::RefPtr<Gtk::AccelGroup> get_accel_group() { return _pAccelGroup; }

    static Gtk::MenuItem*   find_menu_item(Gtk::MenuBar* menuBar, std::string name);
    static Gtk::AccelLabel* get_accel_label(Gtk::MenuItem* item);
    static int              calculate_image_shift(Gtk::MenuItem* menuItem);

    std::vector<Gtk::Toolbar*> build_toolbars(Gtk::MenuToolButton*& pRecentDocsMenuToolButton);
    Gtk::MenuBar*              build_menubar();
    Gtk::Menu*                 build_bookmarks_menu(std::list<std::pair<gint64, std::string>>& bookmarks,
                                                    sigc::slot<void, gint64>& bookmark_action);
    Gtk::Menu*                 build_recent_docs_menu(const CtRecentDocsFilepaths& recentDocsFilepaths,
                                                      sigc::slot<void, const std::string&>& recent_doc_open_action,
                                                      sigc::slot<void, const std::string&>& recent_doc_rm_action);

    Gtk::Menu*                 get_popup_menu(POPUP_MENU_TYPE popupMenuType);
    void                       build_popup_menu(Gtk::Menu* pMenu, POPUP_MENU_TYPE popupMenuType);
    void                       build_popup_menu_table_cell(Gtk::Menu* pMenu, const bool first_row, const bool first_col, const bool last_row, const bool last_col);

private:
    void                    _walk_menu_xml(Gtk::MenuShell* pMenuShell, const char* document, const char* xpath);
    void                    _walk_menu_xml(Gtk::MenuShell* pMenuShell, xmlpp::Node* pNode);
    Gtk::Menu*              _add_menu_submenu(Gtk::MenuShell* pMenuShell, const char* id, const char* name, const char* image);
    Gtk::MenuItem*          _add_menu_item(Gtk::MenuShell* pMenuShell, CtMenuAction* pAction);
    static Gtk::MenuItem*   _add_menu_item(Gtk::MenuShell* pMenuShell,
                                           const char* name,
                                           const char* image,
                                           const char* shortcut,
                                           Glib::RefPtr<Gtk::AccelGroup> accelGroup,
                                           const char* desc,
                                           gpointer action_data,
                                           sigc::signal<void, bool>* signal_set_sensitive,
                                           sigc::signal<void, bool>* signal_set_visible,
                                           const bool use_underline = true);
    static void             _add_menu_item_image_or_label(Gtk::MenuItem* pMenuItem, const char* image, Gtk::AccelLabel* label);
    Gtk::SeparatorMenuItem* _add_menu_separator(Gtk::MenuShell* pMenuShell);

    std::vector<std::string> _get_ui_str_toolbars();
    const char*              _get_ui_str_menu();
    const char*              _get_popup_menu_ui_str_text();
    const char*              _get_popup_menu_ui_str_code();
    const char*              _get_popup_menu_ui_str_image();
    const char*              _get_popup_menu_ui_str_anchor();
    const char*              _get_popup_menu_ui_str_embfile();

private:
    CtConfig*                     _pCtConfig;
    std::list<CtMenuAction>       _actions;
    Glib::RefPtr<Gtk::Builder>    _rGtkBuilder;
    Glib::RefPtr<Gtk::AccelGroup> _pAccelGroup;
    Gtk::Menu*                    _popupMenus[POPUP_MENU_TYPE::PopupMenuNum] = {};
};
