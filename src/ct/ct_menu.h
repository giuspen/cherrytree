/*
 * ct_menu.h
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

#pragma once

#include <string>
#include <libxml++/libxml++.h>
#include <sigc++/sigc++.h>
#include <sigc++/signal.h>
#include <gtkmm.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include "ct_types.h"

class CtConfig;
class CtActions;
class CtMainWin;

struct CtMenuAction
{
    std::string category;
    std::string id;
    std::string image;
    std::string name;
    std::string built_in_shortcut;
    std::string desc;
    std::function<void()> run_action;

    // store signals as shared_ptr so the struct remains copyable/movable and
    // the actual sigc::signal type may be forward-declared by other headers.
    std::shared_ptr<sigc::signal<void, bool>> signal_set_sensitive;
    std::shared_ptr<sigc::signal<void, bool>> signal_set_visible;

    CtMenuAction();

    // Template constructor accepting const char* or std::string for string parameters
    template<typename RunT>
    CtMenuAction(std::string_view category_, std::string_view id_, std::string_view image_, 
                 std::string_view name_, std::string_view built_in_shortcut_, std::string_view desc_, RunT run)
     : category(category_)
     , id(id_)
     , image(image_)
     , name(name_)
     , built_in_shortcut(built_in_shortcut_)
     , desc(desc_)
     , run_action([run]() mutable { run(); })
     , signal_set_sensitive(std::make_shared<sigc::signal<void, bool>>())
     , signal_set_visible(std::make_shared<sigc::signal<void, bool>>())
    {
    }

    const std::string& get_shortcut(CtConfig* pCtConfig) const;
    bool is_shortcut_overridden(CtConfig* pCtConfig) const;
};

class CtMenu
{
public:
    CtMenu(CtMainWin* pCtMainWin);

public:
    const char*       None       = "";
    const std::string KB_CONTROL = "<control>";
    const std::string KB_SHIFT   = "<shift>";
    const std::string KB_ALT     = "<alt>";
    const std::string KB_META    = "<meta>";

    enum POPUP_MENU_TYPE {Node, Text, Code, Link, Codebox, Image, Latex, Anchor, EmbFile, Terminal, PopupMenuNum};

public:
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
   static Gtk::MenuItem* create_menu_item(Gtk::Menu* pMenu, const char* name, const char* image, const char* desc);
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */

public:
    void init_actions(CtActions* pActions);

    CtMenuAction*  find_action(const std::string& id);
    const std::list<CtMenuAction>& get_actions() { return _actions; }
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    Glib::RefPtr<Gtk::AccelGroup> get_accel_group() { return _pAccelGroup; }

    static Gtk::MenuItem*   find_menu_item(Gtk::MenuShell* menuShell, std::string name);
    static Gtk::AccelLabel* get_accel_label(Gtk::MenuItem* item);

    std::vector<Gtk::Toolbar*> build_toolbars(Gtk::MenuToolButton*& pRecentDocsMenuToolButton, Gtk::ToolButton*& pToolButtonSave);
    Gtk::MenuBar*              build_menubar();
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
#if GTKMM_MAJOR_VERSION >= 4
    // GTK4 alternatives: Toolbar as boxes of buttons; Menu via Popover with buttons
    std::vector<Gtk::Box*>     build_toolbars4(Gtk::MenuButton*& pRecentDocsMenuButton, Gtk::Button*& pButtonSave);
    Gtk::MenuButton*           build_menubutton4();
    Gtk::MenuButton*           build_menubutton_model4(); // hierarchical Gio::MenuModel based
    void                       refresh_shortcuts_gtk4();
    void                       populate_recent_docs_menu4(Gtk::MenuButton* recentBtn, const CtRecentDocsFilepaths& recentDocsFilepaths);
    Gtk::MenuButton*           build_bookmarks_button4(std::list<std::tuple<gint64, Glib::ustring, const char*>>& bookmarks,
                                                       sigc::slot<void, gint64> bookmark_action,
                                                       const bool isTopMenu);
private:
    Gtk::Popover*              _build_actions_popover();
    // Map action id -> widget used in GTK4 (button) for sensitivity/visibility updates
    std::unordered_map<std::string, Gtk::Widget*> _gtk4ActionWidgets;
    static std::string         _shortcut_display(const std::string& accel);
#endif /* GTKMM_MAJOR_VERSION >= 4 */
    Gtk::Menu*                 build_bookmarks_menu(std::list<std::tuple<gint64, Glib::ustring, const char*>>& bookmarks,
                                                    sigc::slot<void, gint64>& bookmark_action,
                                                    const bool isTopMenu);
    Gtk::Menu*                 build_recent_docs_menu(const CtRecentDocsFilepaths& recentDocsFilepaths,
                                                      sigc::slot<void, const std::string&>& recent_doc_open_action,
                                                      sigc::slot<void, const std::string&>& recent_doc_rm_action);

    Gtk::Menu*                 get_popup_menu(POPUP_MENU_TYPE popupMenuType);
    void                       build_popup_menu(Gtk::Menu* pMenu, POPUP_MENU_TYPE popupMenuType);
    void                       build_popup_menu_table_cell(Gtk::Menu* pMenu, const bool first_row, const bool first_col, const bool last_row, const bool last_col);

private:
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    void                    _walk_menu_xml(Gtk::MenuShell* pMenuShell, const char* document, const char* xpath);
    void                    _walk_menu_xml(Gtk::MenuShell* pMenuShell, xmlpp::Node* pNode);
    Gtk::Menu*              _add_menu_submenu(Gtk::MenuShell* pMenuShell, const char* id, const char* name, const char* image);
    Gtk::MenuItem*          _add_menu_item(Gtk::MenuShell* pMenuShell,
                                           CtMenuAction* pAction,
                                           std::list<sigc::connection>* pListConnections = nullptr);
    static Gtk::MenuItem*   _add_menu_item_full(Gtk::MenuShell* pMenuShell,
                                                const char* name,
                                                const char* image,
                                                const char* shortcut,
                                                Glib::RefPtr<Gtk::AccelGroup> accelGroup,
                                                const char* desc,
                                                gpointer action_data,
                                                sigc::signal<void, bool>* signal_set_sensitive,
                                                sigc::signal<void, bool>* signal_set_visible,
                                                std::list<sigc::connection>* pListConnections = nullptr,
                                                const bool use_underline = true);
    static void             _add_menu_item_image_or_label(Gtk::MenuItem* pMenuItem, const char* image, Gtk::AccelLabel* label);
    Gtk::SeparatorMenuItem* _add_menu_separator(Gtk::MenuShell* pMenuShell);
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
#if GTKMM_MAJOR_VERSION >= 4
    // Future: Gio::Menu integration with app actions
#endif /* GTKMM_MAJOR_VERSION >= 4 */

    std::vector<std::string> _get_ui_str_toolbars();
    const char*              _get_ui_str_menu();
    const char*              _get_popup_menu_ui_str_text();
    const char*              _get_popup_menu_ui_str_code();
    const char*              _get_popup_menu_ui_str_image();
    const char*              _get_popup_menu_ui_str_latex();
    const char*              _get_popup_menu_ui_str_anchor();
    const char*              _get_popup_menu_ui_str_embfile();
    const char*              _get_popup_menu_ui_str_terminal();

private:
    CtMainWin*                    const _pCtMainWin;
    CtConfig*                     const _pCtConfig;
    std::list<CtMenuAction>       _actions;
    Glib::RefPtr<Gtk::Builder>    _rGtkBuilder;
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    Glib::RefPtr<Gtk::AccelGroup> _pAccelGroup;
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
    Gtk::Menu*                    _popupMenus[POPUP_MENU_TYPE::PopupMenuNum] = {};
    std::list<sigc::connection>   _curr_bookm_submenu_sigc_conn;
};
