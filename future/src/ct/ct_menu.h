/*
 * ct_menu.h
 *
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include <vector>
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

    void init_actions(CtApp *app);
    GtkWidget* build_menubar();

private:
    CtAction const * find_action(std::string id);

    GtkAccelGroup* default_accel_group();

    GtkWidget* build_menu_item(GtkMenu *menu, CtAction const *action);
    void build_menus(xmlpp::Node* node, GtkWidget *menu);
    const char* get_menu_markup();

private:
    std::vector<CtAction> _actions;
};
