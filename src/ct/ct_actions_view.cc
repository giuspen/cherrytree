/*
 * ct_actions_view.cc
 *
 * Copyright 2009-2025
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

#include <sigc++/sigc++.h>
#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include "ct_dialogs.h"

void CtActions::toggle_show_hide_vte()
{
    _pCtConfig->vteVisible = not _pCtMainWin->is_vte_visible();
    (void)_pCtMainWin->show_hide_vte_cmd_passed_as_first_in_session(_pCtConfig->vteVisible, nullptr/*first_cmd_passed*/);
    if (_pCtConfig->vteVisible) {
        Gtk::Widget* pVte = _pCtMainWin->get_vte();
        if (pVte) pVte->grab_focus();
    }
    else {
        _pCtMainWin->get_text_view().mm().grab_focus();
    }
}

void CtActions::toggle_show_hide_tree()
{
    _pCtConfig->treeVisible = not _pCtConfig->treeVisible;
    _pCtMainWin->show_hide_tree_view(_pCtConfig->treeVisible);
    if (_pCtConfig->treeVisible) {
        _pCtMainWin->get_tree_view().grab_focus();
    }
    else {
        _pCtMainWin->get_text_view().mm().grab_focus();
    }
}

void CtActions::toggle_show_hide_menubar()
{
    _pCtConfig->menubarVisible = not _pCtConfig->menubarVisible;
    _pCtMainWin->show_hide_menubar(_pCtConfig->menubarVisible);
    _pCtMainWin->window_title_update();
    if (not _pCtConfig->menubarVisible and std::string::npos == _pCtConfig->toolbarUiList.find("toggle_show_menubar")) {
        spdlog::debug("toolbar + toggle_show_menubar");
        _pCtConfig->toolbarUiList += ",toggle_show_menubar";
#if GTKMM_MAJOR_VERSION >= 4
        _pCtMainWin->signal_app_apply_for_each_window->emit([](CtMainWin* win) { win->menu_rebuild_toolbars(true/*new_toolbar*/); });
#else
        _pCtMainWin->signal_app_apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true/*new_toolbar*/); });
#endif
    }
}

void CtActions::toggle_show_hide_toolbars()
{
    _pCtConfig->toolbarVisible = not _pCtConfig->toolbarVisible;
    _pCtMainWin->show_hide_toolbars(_pCtConfig->toolbarVisible);
}

void CtActions::toggle_show_hide_statusbar()
{
    _pCtConfig->statusbarVisible = not _pCtConfig->statusbarVisible;
    _pCtMainWin->show_hide_statusbar(_pCtConfig->statusbarVisible);
}

void CtActions::toggle_show_hide_node_name_header()
{
    _pCtConfig->showNodeNameHeader = not _pCtConfig->showNodeNameHeader;
    _pCtMainWin->show_hide_win_header(_pCtConfig->showNodeNameHeader);
}

void CtActions::toggle_show_hide_tree_lines()
{
    _pCtConfig->treeLinesVisible = not _pCtConfig->treeLinesVisible;
    _pCtMainWin->show_hide_tree_lines(_pCtConfig->treeLinesVisible);
}

void CtActions::toggle_focus_tree_text()
{
    if (_pCtMainWin->get_tree_view().has_focus()) {
        _pCtMainWin->get_text_view().mm().grab_focus();
    }
    else {
        _pCtMainWin->get_tree_view().grab_focus();
    }
}

void CtActions::toggle_focus_vte_text()
{
    if (_pCtMainWin->is_vte_visible() and _pCtMainWin->get_text_view().mm().has_focus()) {
        Gtk::Widget* pVte = _pCtMainWin->get_vte();
        if (pVte) pVte->grab_focus();
    }
    else {
        _pCtMainWin->get_text_view().mm().grab_focus();
    }
}

void CtActions::nodes_expand_all()
{
    _pCtMainWin->get_tree_view().expand_all();
}

void CtActions::nodes_collapse_all()
{
    _pCtMainWin->get_tree_view().collapse_all();
}

void CtActions::more_nodes_on_node_name_header()
{
    if (_pCtConfig->toolbarIconSize >= 100) {
        CtDialogs::info_dialog(_("The Number of Last Visited Nodes on Node Name Header is already at the Maximum Value."), *_pCtMainWin);
        return;
    }
    ++_pCtConfig->nodesOnNodeNameHeader;
    _pCtMainWin->window_header_update();
}

void CtActions::less_nodes_on_node_name_header()
{
    if (_pCtConfig->nodesOnNodeNameHeader <= 0) {
        CtDialogs::info_dialog(_("The Number of Last Visited Nodes on Node Name Header is already at the Minimum Value."), *_pCtMainWin);
        return;
    }
    --_pCtConfig->nodesOnNodeNameHeader;
    _pCtMainWin->window_header_update();
}

void CtActions::toolbar_icons_size_increase()
{
    if (_pCtConfig->toolbarIconSize >= 5) {
        CtDialogs::info_dialog(_("The Size of the Toolbar Icons is already at the Maximum Value."), *_pCtMainWin);
        return;
    }
    _pCtConfig->toolbarIconSize += 1;
    _pCtMainWin->set_toolbars_icon_size(_pCtConfig->toolbarIconSize);
}

void CtActions::toolbar_icons_size_decrease()
{
    if (_pCtConfig->toolbarIconSize <= 2) {
        CtDialogs::info_dialog(_("The Size of the Toolbar Icons is already at the Minimum Value."), *_pCtMainWin);
        return;
    }
    _pCtConfig->toolbarIconSize -= 1;
    _pCtMainWin->set_toolbars_icon_size(_pCtConfig->toolbarIconSize);
}

#if GTKMM_MAJOR_VERSION < 4
void CtActions::toggle_always_on_top()
{
    _pCtMainWin->toggle_always_on_top();
}
#endif /* GTKMM_MAJOR_VERSION < 4 */

void CtActions::toggle_fullscreen()
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
    _pCtConfig->menubarInTitlebar = setOn;
    CtDialogs::info_dialog(_("This Change will have Effect Only After Restarting CherryTree."), *_pCtMainWin);
}
