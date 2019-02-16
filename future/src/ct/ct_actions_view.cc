/*
 * ct_actions_view.cc
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

#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>

// Toggle Show/Hide the Tree
void CtActions::toggle_show_hide_tree()
{

}

void CtActions::toggle_show_hide_toolbar()
{
    CtApp::P_ctCfg->toolbarVisible = !CtApp::P_ctCfg->toolbarVisible;
    _ctMainWin->show_hide_toolbar(CtApp::P_ctCfg->toolbarVisible);
}

void CtActions::toggle_show_hide_node_name_header()
{

}

void CtActions::toggle_tree_text()
{

}

void CtActions::nodes_expand_all()
{

}

void CtActions::nodes_collapse_all()
{

}

void CtActions::toolbar_icons_size_increase()
{

}

void CtActions::toolbar_icons_size_decrease()
{

}

void CtActions::fullscreen_toggle()
{

}
