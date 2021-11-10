/*
 * ct_actions_view.cc
 *
 * Copyright 2009-2020
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

#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include "ct_dialogs.h"

// Toggle Show/Hide the Tree
void CtActions::toggle_show_hide_tree()
{
    auto pCtConfig = _pCtMainWin->get_ct_config();
    pCtConfig->treeVisible = not pCtConfig->treeVisible;
    _pCtMainWin->show_hide_tree_view(pCtConfig->treeVisible);
    if (pCtConfig->treeVisible) {
        _pCtMainWin->get_tree_view().grab_focus();
    }
    else {
        _pCtMainWin->get_text_view().grab_focus();
    }
}

void CtActions::toggle_show_hide_menubar()
{
    auto pCtConfig = _pCtMainWin->get_ct_config();
    pCtConfig->menubarVisible = not pCtConfig->menubarVisible;
    _pCtMainWin->show_hide_menubar(pCtConfig->menubarVisible);
}

void CtActions::toggle_show_hide_toolbars()
{
    auto pCtConfig = _pCtMainWin->get_ct_config();
    pCtConfig->toolbarVisible = not pCtConfig->toolbarVisible;
    _pCtMainWin->show_hide_toolbars(pCtConfig->toolbarVisible);
}

void CtActions::toggle_show_hide_statusbar()
{
    auto pCtConfig = _pCtMainWin->get_ct_config();
    pCtConfig->statusbarVisible = not pCtConfig->statusbarVisible;
    _pCtMainWin->show_hide_statusbar(pCtConfig->statusbarVisible);
}

void CtActions::toggle_show_hide_node_name_header()
{
    auto pCtConfig = _pCtMainWin->get_ct_config();
    pCtConfig->showNodeNameHeader = not pCtConfig->showNodeNameHeader;
    _pCtMainWin->show_hide_win_header(pCtConfig->showNodeNameHeader);
}

void CtActions::toggle_show_hide_tree_lines()
{
    auto pCtConfig = _pCtMainWin->get_ct_config();
    pCtConfig->treeLinesVisible = not pCtConfig->treeLinesVisible;
    _pCtMainWin->show_hide_tree_lines(pCtConfig->treeLinesVisible);
}

// Toggle Focus Between Tree and Text
void CtActions::toggle_tree_text()
{
    if (_pCtMainWin->get_tree_view().has_focus())
        _pCtMainWin->get_text_view().grab_focus();
    else
        _pCtMainWin->get_tree_view().grab_focus();
}

// Expand all Tree Nodes
void CtActions::nodes_expand_all()
{
    _pCtMainWin->get_tree_view().expand_all();
}

// Collapse all Tree Nodes
void CtActions::nodes_collapse_all()
{
    _pCtMainWin->get_tree_view().collapse_all();
}

// Increase the Size of the Toolbar Icons
void CtActions::toolbar_icons_size_increase()
{
    if (_pCtMainWin->get_ct_config()->toolbarIconSize == 5) {
        CtDialogs::info_dialog(_("The Size of the Toolbar Icons is already at the Maximum Value"), *_pCtMainWin);
        return;
    }
    _pCtMainWin->get_ct_config()->toolbarIconSize += 1;
    _pCtMainWin->set_toolbars_icon_size(_pCtMainWin->get_ct_config()->toolbarIconSize);
}

// Decrease the Size of the Toolbar Icons
void CtActions::toolbar_icons_size_decrease()
{
    if (_pCtMainWin->get_ct_config()->toolbarIconSize == 1) {
        CtDialogs::info_dialog(_("The Size of the Toolbar Icons is already at the Minimum Value"), *_pCtMainWin);
        return;
    }
    _pCtMainWin->get_ct_config()->toolbarIconSize -= 1;
    _pCtMainWin->set_toolbars_icon_size(_pCtMainWin->get_ct_config()->toolbarIconSize);
}

// Toggle Fullscreen State
void CtActions::fullscreen_toggle()
{
    if (_pCtMainWin->get_titlebar()) {
        // unfullscreen doesn't work with the custom titlebar, so we can only maximise
        if (_pCtMainWin->property_is_maximized()) {
            _pCtMainWin->unmaximize();
        }
        else {
            _pCtMainWin->maximize();
        }
    }
    else {
        if (_pCtMainWin->get_window()->get_state() & GDK_WINDOW_STATE_FULLSCREEN) {
            _pCtMainWin->unfullscreen();
        }
        else {
            _pCtMainWin->fullscreen();
        }
    }
}

void CtActions::_menubar_in_titlebar_set(const bool setOn)
{
    _pCtMainWin->get_ct_config()->menubarInTitlebar = setOn;
    CtDialogs::info_dialog(_("This Change will have Effect Only After Restarting CherryTree"), *_pCtMainWin);
}
