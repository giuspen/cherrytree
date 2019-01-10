/*
 * ct_menu.h
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

#include "ct_app.h"

#include <string>
#include <libxml++/libxml++.h>
#include <gtkmm.h>

struct CtAction
{
    std::string id;
    std::string stock;
    std::string name;
    std::string shortcut;
    std::string desc;
    sigc::slot<void> run_action;
};

class CtApp;
class CtMenu
{
public:
    CtMenu();

    void init_actions(CtApp* pApp);

    GtkAccelGroup* default_accel_group();

    Gtk::Toolbar* build_toolbar();
    Gtk::MenuBar* build_menubar();
    Gtk::Menu*    build_popup_menu_node();
    Gtk::Menu*    build_popup_menu_text();
    Gtk::Menu*    build_popup_menu_code();
    Gtk::Menu*    build_popup_menu_link();
    Gtk::Menu*    build_popup_menu_table();
    Gtk::Menu*    build_popup_menu_table_cell();
    Gtk::Menu*    build_popup_menu_table_codebox();

private:
    CtAction const* find_action(const std::string& id);

    GtkWidget* walk_menu_xml(GtkWidget* pMenu, const char* document, const char* xpath);
    void       walk_menu_xml(GtkWidget* pMenu, xmlpp::Node* pNode);
    GtkWidget* add_submenu(GtkWidget* pMenu, const char* name);
    GtkWidget* add_menu_item(GtkWidget* pMenu, const char *name, const char *stock = nullptr, const char *shortcut = nullptr, gpointer action_data = nullptr);

    std::string get_toolbar_ui_str();
    const char* get_menu_ui_str();
    const char* get_popup_menu_text_ui_str();
    const char* get_popup_menu_code_ui_str();

private:
    std::list<CtAction>        _actions;
    Glib::RefPtr<Gtk::Builder> _rGtkBuilder;
    GtkAccelGroup*             _pAccelGroup;

};
