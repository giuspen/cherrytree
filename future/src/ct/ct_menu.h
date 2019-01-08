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
    sigc::signal<void> run_action;
};

class CtApp;
class CtMenu
{
public:
    CtMenu();

    void init_actions(CtApp* pApp);
    GtkWidget* build_menubar();
    GtkAccelGroup* default_accel_group();
    std::string get_toolbar_ui_str();
private:
    CtAction const* find_action(const std::string& id);

    GtkWidget* build_menu_item(GtkMenu* pMenu, CtAction const* pAction);
    void build_menus(xmlpp::Node* pNode, GtkWidget* pMenu);
    const char* get_menu_ui_str();

private:
    std::list<CtAction> _actions;
};
