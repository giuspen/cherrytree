/*
 * ct_menu.h
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

#include <string>
#include <libxml++/libxml++.h>
#include <gtkmm.h>
#include "ct_types.h"

class CtConfig;

struct CtAction
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
    CtMenu(CtConfig* pCtConfig);

public:
    const char*       None       = "";
    const std::string KB_CONTROL = "<control>";
    const std::string KB_SHIFT   = "<shift>";
    const std::string KB_ALT     = "<alt>";

    enum POPUP_MENU_TYPE {Node, Text, Code, Link, TableHeaderCell, TableCell, Codebox, Image, Anchor, EmbFile, PopupMenuNum };

public:
    void init_actions(CtApp* pApp, CtActions* pActions);
    CtAction* find_action(const std::string& id);
    const std::list<CtAction>& get_actions() { return _actions; }

    GtkAccelGroup* default_accel_group();

    static Gtk::MenuItem* find_menu_item(Gtk::MenuBar* menuBar, std::string name);

    Gtk::Toolbar* build_toolbar(Gtk::MenuToolButton*& pRecentDocsMenuToolButton);
    Gtk::MenuBar* build_menubar();
    Gtk::Menu*    build_bookmarks_menu(std::list<std::pair<gint64, std::string>>& bookmarks,
                                       sigc::slot<void, gint64>& bookmark_action);
    Gtk::Menu*    build_recent_docs_menu(const CtRecentDocsFilepaths& recentDocsFilepaths,
                                         sigc::slot<void, const std::string&>& recent_doc_open_action,
                                         sigc::slot<void, const std::string&>& recent_doc_rm_action);
    Gtk::Menu*    build_special_chars_menu(const Glib::ustring& specialChars,
                                           sigc::slot<void, gunichar>& spec_char_action);

    Gtk::Menu*    get_popup_menu(POPUP_MENU_TYPE popupMenuType);
    Gtk::Menu*    build_popup_menu(GtkWidget* pMenu, POPUP_MENU_TYPE popupMenuType);

private:
    GtkWidget*     _walk_menu_xml(GtkWidget* pMenu, const char* document, const char* xpath);
    void           _walk_menu_xml(GtkWidget* pMenu, xmlpp::Node* pNode);
    GtkWidget*     _add_submenu(GtkWidget* pMenu, const char* id, const char* name, const char* image);
    Gtk::MenuItem* _add_menu_item(GtkWidget* pMenu, CtAction* pAction);
    Gtk::MenuItem* _add_menu_item(GtkWidget* pMenu, const char* name, const char* image, const char*shortcut,
                                 const char* desc, gpointer action_data,
                                 sigc::signal<void, bool>* signal_set_sensitive = nullptr,
                                 sigc::signal<void, bool>* signal_set_visible = nullptr);
    GtkWidget*     _add_separator(GtkWidget* pMenu);
    void           _add_menu_item_image_or_label(Gtk::Widget* pMenuItem, const char* image, GtkWidget* pLabel);

    std::string _get_ui_str_toolbar();
    const char* _get_ui_str_menu();
    const char* _get_popup_menu_ui_str_text();
    const char* _get_popup_menu_ui_str_code();
    const char* _get_popup_menu_ui_str_image();
    const char* _get_popup_menu_ui_str_anchor();
    const char* _get_popup_menu_ui_str_embfile();

private:
    CtConfig*                  _pCtConfig;
    std::list<CtAction>        _actions;
    Glib::RefPtr<Gtk::Builder> _rGtkBuilder;
    GtkAccelGroup*             _pAccelGroup;
    Gtk::Menu*                 _popupMenus[POPUP_MENU_TYPE::PopupMenuNum] = {};
};
