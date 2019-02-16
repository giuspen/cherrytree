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
#include "ct_dialogs.h"

// Toggle Show/Hide the Tree
void CtActions::toggle_show_hide_tree()
{
    CtApp::P_ctCfg->treeVisible = !CtApp::P_ctCfg->treeVisible;
    _ctMainWin->show_hide_tree_view(CtApp::P_ctCfg->treeVisible);
}

// Toggle Show/Hide the Toolbar
void CtActions::toggle_show_hide_toolbar()
{
    CtApp::P_ctCfg->toolbarVisible = !CtApp::P_ctCfg->toolbarVisible;
    _ctMainWin->show_hide_toolbar(CtApp::P_ctCfg->toolbarVisible);
}

void CtActions::toggle_show_hide_node_name_header()
{
    CtApp::P_ctCfg->showNodeNameHeader = !CtApp::P_ctCfg->showNodeNameHeader;
    _ctMainWin->show_hide_win_header(CtApp::P_ctCfg->showNodeNameHeader);
}

// Toggle Focus Between Tree and Text
void CtActions::toggle_tree_text()
{
    if (_ctMainWin->get_tree_view().has_focus())
        _ctMainWin->get_text_view().grab_focus();
    else
        _ctMainWin->get_tree_view().grab_focus();
}

// Expand all Tree Nodes
void CtActions::nodes_expand_all()
{
    _ctMainWin->get_tree_view().expand_all();
}

// Collapse all Tree Nodes
void CtActions::nodes_collapse_all()
{
    _ctMainWin->get_tree_view().collapse_all();
}

// Increase the Size of the Toolbar Icons
void CtActions::toolbar_icons_size_increase()
{
    if (CtApp::P_ctCfg->toolbarIconSize == 5) {
        ct_dialogs::info_dialog(_("The Size of the Toolbar Icons is already at the Maximum Value"), *_ctMainWin);
        return;
    }
    CtApp::P_ctCfg->toolbarIconSize += 1;
    _ctMainWin->set_toolbar_icon_size(CtApp::P_ctCfg->toolbarIconSize);
}

// Decrease the Size of the Toolbar Icons
void CtActions::toolbar_icons_size_decrease()
{
    if (CtApp::P_ctCfg->toolbarIconSize == 1) {
        ct_dialogs::info_dialog(_("The Size of the Toolbar Icons is already at the Minimum Value"), *_ctMainWin);
        return;
    }
    CtApp::P_ctCfg->toolbarIconSize -= 1;
    _ctMainWin->set_toolbar_icon_size(CtApp::P_ctCfg->toolbarIconSize);
}

// Toggle Fullscreen State
void CtActions::fullscreen_toggle()
{
    if (_ctMainWin->get_state_flags() & GDK_WINDOW_STATE_FULLSCREEN)
        _ctMainWin->unfullscreen();
    else
        _ctMainWin->fullscreen();
}
