/*
 * ct_menu.cc
 *
 * Copyright 2009-2024
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
#include "ct_menu.h"
#include "ct_storage_xml.h"

static xmlpp::Attribute* get_attribute(xmlpp::Node* pNode, char const* name)
{
    xmlpp::Element* pElement = static_cast<xmlpp::Element*>(pNode);
    return pElement->get_attribute(name);
}

static void on_menu_activate(void* /*pObject*/, CtMenuAction* pAction)
{
    if (pAction) {
        // this allows the menu to close before the action is executed
        Glib::signal_idle().connect_once([pAction](){
            pAction->run_action();
        });
    }
}

const std::string& CtMenuAction::get_shortcut(CtConfig* pCtConfig) const
{
    const auto it = pCtConfig->customKbShortcuts.find(id);
    return it != pCtConfig->customKbShortcuts.end() ? it->second : built_in_shortcut;
}

bool CtMenuAction::is_shortcut_overridden(CtConfig* pCtConfig) const
{
    const auto it = pCtConfig->customKbShortcuts.find(id);
    if (pCtConfig->customKbShortcuts.end() == it) return false;
    if (it->second != built_in_shortcut) return true;
    // we have a value in the map but it's just like the default => cleanup
    pCtConfig->customKbShortcuts.erase(id);
    return false;
}

CtMenu::CtMenu(CtMainWin* pCtMainWin)
 : _pCtMainWin{pCtMainWin}
 , _pCtConfig{pCtMainWin->get_ct_config()}
{
    _pAccelGroup = Gtk::AccelGroup::create();
    _rGtkBuilder = Gtk::Builder::create();
    init_actions(pCtMainWin->get_ct_actions());
}

/*static*/Gtk::MenuItem* CtMenu::create_menu_item(Gtk::Menu* pMenu, const char* name, const char* image, const char* desc)
{
    return _add_menu_item_full(pMenu,
                               name,
                               image,
                               nullptr,
                               Glib::RefPtr<Gtk::AccelGroup>{},
                               desc,
                               nullptr,
                               nullptr,
                               nullptr);
}

/*static*/ Gtk::MenuItem* CtMenu::find_menu_item(Gtk::MenuShell* menuShell, std::string name)
{
    for (Gtk::Widget* child : menuShell->get_children())
        if (auto menuItem = dynamic_cast<Gtk::MenuItem*>(child))
            if (menuItem->get_name() == name)
                return menuItem;

    // check first level menu items, these menu items have complicated structure
    for (Gtk::Widget* child : menuShell->get_children())
        if (auto menuItem = dynamic_cast<Gtk::MenuItem*>(child))
            if (Gtk::Menu* subMenu = menuItem->get_submenu())
                for (Gtk::Widget* subChild : subMenu->get_children())
                    if (auto subItem = dynamic_cast<Gtk::MenuItem*>(subChild))
                        if (Gtk::Widget* subItemChild = subItem->get_child())
                            if (subItemChild->get_name() == name)
                                return subItem; // it's right, not a subItemChild

    return nullptr;
}

/*static*/ Gtk::AccelLabel* CtMenu::get_accel_label(Gtk::MenuItem* item)
{
    if (auto box = dynamic_cast<Gtk::Box*>(item->get_child()))
        if (auto label = dynamic_cast<Gtk::AccelLabel*>(box->get_children().back()))
            return label;
    return nullptr;
}

std::vector<Gtk::Toolbar*> CtMenu::build_toolbars(Gtk::MenuToolButton*& pRecentDocsMenuToolButton, Gtk::ToolButton*& pToolButtonSave)
{
    pRecentDocsMenuToolButton = nullptr;
    std::vector<Gtk::Toolbar*> toolbars;
    for (const auto& toolbar_str : _get_ui_str_toolbars()) {
        Gtk::Toolbar* pToolbar = nullptr;
        _rGtkBuilder->add_from_string(toolbar_str);
        _rGtkBuilder->get_widget("ToolBar" + std::to_string(toolbars.size()), pToolbar);
        toolbars.push_back(pToolbar);
        if (not pRecentDocsMenuToolButton) {
            _rGtkBuilder->get_widget("RecentDocs", pRecentDocsMenuToolButton);
        }
        if (not pToolButtonSave) {
            _rGtkBuilder->get_widget("ct_save", pToolButtonSave);
        }
    }
    return toolbars;
}

Gtk::MenuBar* CtMenu::build_menubar()
{
    Gtk::MenuBar* pMenuBar = Gtk::manage(new Gtk::MenuBar());
    _walk_menu_xml(pMenuBar, _get_ui_str_menu(), nullptr);
    return pMenuBar;
}

Gtk::Menu* CtMenu::build_bookmarks_menu(std::list<std::tuple<gint64, Glib::ustring, const char*>>& bookmarks,
                                        sigc::slot<void, gint64>& bookmark_action,
                                        const bool isTopMenu)
{
    Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu{});
    if (isTopMenu) {
        // disconnect signals connected to prev submenu
        for (sigc::connection& sigc_conn : _curr_bookm_submenu_sigc_conn) {
            sigc_conn.disconnect();
        }
        _curr_bookm_submenu_sigc_conn.clear();
        _add_menu_item(pMenu, find_action("node_bookmark"), &_curr_bookm_submenu_sigc_conn);
        _add_menu_item(pMenu, find_action("node_unbookmark"), &_curr_bookm_submenu_sigc_conn);
        _add_menu_separator(pMenu);
    }
    _add_menu_item(pMenu, find_action("handle_bookmarks"));
    _add_menu_separator(pMenu);
    for (const auto& bookmark : bookmarks) {
        const gint64& node_id = std::get<0>(bookmark);
        const Glib::ustring& node_name = std::get<1>(bookmark);
        const char* node_icon = std::get<2>(bookmark);
        Gtk::MenuItem* pMenuItem = _add_menu_item_full(pMenu,
                                                       node_name.c_str(),
                                                       node_icon,
                                                       nullptr,
                                                       _pAccelGroup,
                                                       _pCtConfig->menusTooltips ? node_name.c_str() : nullptr,
                                                       nullptr,
                                                       nullptr,
                                                       nullptr);
        pMenuItem->signal_activate().connect(sigc::bind(bookmark_action, node_id));
    }
    return pMenu;
}

Gtk::Menu* CtMenu::build_recent_docs_menu(const CtRecentDocsFilepaths& recentDocsFilepaths,
                                          sigc::slot<void, const std::string&>& recent_doc_open_action,
                                          sigc::slot<void, const std::string&>& recent_doc_rm_action)
{
    Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu());
    for (const fs::path& filepath : recentDocsFilepaths) {
        bool file_exists = fs::exists(filepath);
        Gtk::MenuItem* pMenuItem = _add_menu_item_full(pMenu,
                                                       filepath.c_str(),
                                                       file_exists ? "ct_open" : "ct_urgent",
                                                       nullptr,
                                                       _pAccelGroup,
                                                       _pCtConfig->menusTooltips ? filepath.c_str() : nullptr,
                                                       nullptr,
                                                       nullptr,
                                                       nullptr,
                                                       nullptr,
                                                       false/*use_underline*/);
        pMenuItem->signal_activate().connect(sigc::bind(recent_doc_open_action, filepath.string()));
    }
    Gtk::MenuItem* pMenuItemRm = _add_menu_item_full(pMenu,
                                                     _("Remove from list"),
                                                     "ct_edit_delete",
                                                     nullptr,
                                                     _pAccelGroup,
                                                     _pCtConfig->menusTooltips ? _("Remove from list") : nullptr,
                                                     nullptr,
                                                     nullptr,
                                                     nullptr);
    Gtk::Menu* pMenuRm = Gtk::manage(new Gtk::Menu());
    pMenuItemRm->set_submenu(*pMenuRm);
    for (const fs::path& filepath : recentDocsFilepaths) {
        bool file_exists = fs::exists(filepath);
        Gtk::MenuItem* pMenuItem = _add_menu_item_full(pMenuRm,
                                                       filepath.c_str(),
                                                       file_exists ? "ct_edit_delete" : "ct_urgent",
                                                       nullptr,
                                                       _pAccelGroup,
                                                       _pCtConfig->menusTooltips ? filepath.c_str() : nullptr,
                                                       nullptr,
                                                       nullptr,
                                                       nullptr);
        pMenuItem->signal_activate().connect(sigc::bind(recent_doc_rm_action, filepath.string()));
    }
    return pMenu;
}

Gtk::Menu* CtMenu::get_popup_menu(POPUP_MENU_TYPE popupMenuType)
{
    if (_popupMenus[popupMenuType] == nullptr) {
        Gtk::Menu* pMenu = Gtk::manage(new Gtk::Menu{});
        build_popup_menu(pMenu, popupMenuType);
        pMenu->attach_to_widget(*_pCtMainWin);
        _popupMenus[popupMenuType] = pMenu;
    }
    return _popupMenus[popupMenuType];
}

void CtMenu::build_popup_menu(Gtk::Menu* pMenu, POPUP_MENU_TYPE popupMenuType)
{
    switch (popupMenuType) {
        case CtMenu::POPUP_MENU_TYPE::Node: _walk_menu_xml(pMenu, _get_ui_str_menu(), "/menubar/menu[@action='TreeMenu']/*"); break;
        case CtMenu::POPUP_MENU_TYPE::Text: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_text(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Code: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_code(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Image: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_image(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Latex: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_latex(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Anchor: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_anchor(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::EmbFile: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_embfile(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Terminal: _walk_menu_xml(pMenu, _get_popup_menu_ui_str_terminal(), nullptr); break;
        case CtMenu::POPUP_MENU_TYPE::Link: {
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("apply_tag_link"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("link_cut"));
            _add_menu_item(pMenu, find_action("link_copy"));
            _add_menu_item(pMenu, find_action("link_dismiss"));
            _add_menu_item(pMenu, find_action("link_delete"));
        } break;
        case CtMenu::POPUP_MENU_TYPE::Codebox: {
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("cut_plain"));
            _add_menu_item(pMenu, find_action("copy_plain"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("codebox_load_from_file"));
            _add_menu_item(pMenu, find_action("codebox_save_to_file"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("codebox_cut"));
            _add_menu_item(pMenu, find_action("codebox_copy"));
            _add_menu_item(pMenu, find_action("codebox_copy_content"));
            _add_menu_item(pMenu, find_action("codebox_delete"));
            _add_menu_item(pMenu, find_action("codebox_delete_keeping_text"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("codebox_increase_width"));
            _add_menu_item(pMenu, find_action("codebox_decrease_width"));
            _add_menu_item(pMenu, find_action("codebox_increase_height"));
            _add_menu_item(pMenu, find_action("codebox_decrease_height"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("exec_code_all"));
            _add_menu_item(pMenu, find_action("exec_code_los"));
            _add_menu_item(pMenu, find_action("strip_trail_spaces"));
            _add_menu_item(pMenu, find_action("repl_tabs_spaces"));
            _add_menu_separator(pMenu);
            _add_menu_item(pMenu, find_action("codebox_change_properties"));
        } break;
        case CtMenu::POPUP_MENU_TYPE::PopupMenuNum: {
        } break;
    }
}

void CtMenu::build_popup_menu_table_cell(Gtk::Menu* pMenu,
                                         const bool first_row,
                                         const bool first_col,
                                         const bool last_row,
                                         const bool last_col)
{
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_cut"));
    _add_menu_item(pMenu, find_action("table_copy"));
    _add_menu_item(pMenu, find_action("table_delete"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_column_add"));
    _add_menu_item(pMenu, find_action("table_column_cut"));
    _add_menu_item(pMenu, find_action("table_column_copy"));
    _add_menu_item(pMenu, find_action("table_column_paste"));
    _add_menu_item(pMenu, find_action("table_column_delete"));
    _add_menu_separator(pMenu);
    if (not first_col) _add_menu_item(pMenu, find_action("table_column_left"));
    if (not last_col) _add_menu_item(pMenu, find_action("table_column_right"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_column_increase_width"));
    _add_menu_item(pMenu, find_action("table_column_decrease_width"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_row_add"));
    _add_menu_item(pMenu, find_action("table_row_cut"));
    _add_menu_item(pMenu, find_action("table_row_copy"));
    _add_menu_item(pMenu, find_action("table_row_paste"));
    _add_menu_item(pMenu, find_action("table_row_delete"));
    _add_menu_separator(pMenu);
    if (not first_row) _add_menu_item(pMenu, find_action("table_row_up"));
    if (not last_row) _add_menu_item(pMenu, find_action("table_row_down"));
    _add_menu_item(pMenu, find_action("table_rows_sort_ascending"));
    _add_menu_item(pMenu, find_action("table_rows_sort_descending"));
    _add_menu_separator(pMenu);
    _add_menu_item(pMenu, find_action("table_edit_properties"));
    _add_menu_item(pMenu, find_action("table_export"));
}

void CtMenu::_walk_menu_xml(Gtk::MenuShell* pMenuShell, const char* document, const char* xpath)
{
    xmlpp::DomParser parser;
    if (not CtXmlHelper::safe_parse_memory(parser, document)) {
        return;
    }
    if (xpath) {
        _walk_menu_xml(pMenuShell, parser.get_document()->get_root_node()->find(xpath)[0]);
    }
    else {
        _walk_menu_xml(pMenuShell, parser.get_document()->get_root_node());
    }
}

void CtMenu::_walk_menu_xml(Gtk::MenuShell* pMenuShell, xmlpp::Node* pNode)
{
    for (xmlpp::Node* pNodeIter = pNode; pNodeIter; pNodeIter = pNodeIter->get_next_sibling()) {
        if (pNodeIter->get_name() == "menubar" || pNodeIter->get_name() == "popup") {
            _walk_menu_xml(pMenuShell, pNodeIter->get_first_child());
        }
        else if (pNodeIter->get_name() == "menu") {
            if (xmlpp::Attribute* pAttrName = get_attribute(pNodeIter, "_name")) // menu name which need to be translated
            {
                xmlpp::Attribute* pAttrImage = get_attribute(pNodeIter, "image");
                Gtk::Menu* pSubmenu = _add_menu_submenu(pMenuShell, pAttrName->get_value().c_str(), _(pAttrName->get_value().c_str()), pAttrImage->get_value().c_str());
                _walk_menu_xml(pSubmenu, pNodeIter->get_first_child());
            }
            else { // otherwise it is an action id
                CtMenuAction const* pAction = find_action(get_attribute(pNodeIter, "action")->get_value());
                Gtk::Menu* pSubmenu = _add_menu_submenu(pMenuShell, pAction->id.c_str(), pAction->name.c_str(), pAction->image.c_str());
                _walk_menu_xml(pSubmenu, pNodeIter->get_first_child());
            }
        }
        else if (pNodeIter->get_name() == "menuitem") {
            CtMenuAction* pAction = find_action(get_attribute(pNodeIter, "action")->get_value());
            if (pAction) _add_menu_item(pMenuShell, pAction);
        }
        else if (pNodeIter->get_name() == "separator") {
            _add_menu_separator(pMenuShell);
        }
    }
}

Gtk::Menu* CtMenu::_add_menu_submenu(Gtk::MenuShell* pMenuShell, const char* id, const char* name, const char* image)
{
    Gtk::MenuItem* pMenuItem = Gtk::manage(new Gtk::MenuItem{});
    pMenuItem->set_name(id);
    Gtk::AccelLabel* pLabel = Gtk::manage(new Gtk::AccelLabel{name, true});
    pLabel->set_xalign(0.0);
    pLabel->set_accel_widget(*pMenuItem);

    _add_menu_item_image_or_label(pMenuItem, image, pLabel);
    pMenuItem->get_child()->set_name(id); // for find_menu_item()
    pMenuItem->show_all();

    Gtk::Menu* pSubMenu = Gtk::manage(new Gtk::Menu{});
    pMenuItem->set_submenu(*pSubMenu);
    pMenuShell->append(*pMenuItem);
    return pSubMenu;
}

Gtk::MenuItem* CtMenu::_add_menu_item(Gtk::MenuShell* pMenuShell,
                                      CtMenuAction* pAction,
                                      std::list<sigc::connection>* pListConnections/*= nullptr*/)
{
    std::string shortcut = pAction->get_shortcut(_pCtConfig);
    Gtk::MenuItem* pMenuItem = _add_menu_item_full(pMenuShell,
                                                   pAction->name.c_str(),
                                                   pAction->image.c_str(),
                                                   shortcut.c_str(),
                                                   _pAccelGroup,
                                                   _pCtConfig->menusTooltips ? pAction->desc.c_str() : nullptr,
                                                   (gpointer)pAction,
                                                   &pAction->signal_set_sensitive,
                                                   &pAction->signal_set_visible,
                                                   pListConnections);
    pMenuItem->get_child()->set_name(pAction->id); // for find_menu_item();
    return pMenuItem;
}

// based on inkscape/src/ui/desktop/menubar.cpp
/*static*/Gtk::MenuItem* CtMenu::_add_menu_item_full(Gtk::MenuShell* pMenuShell,
                                                     const char* name,
                                                     const char* image,
                                                     const char* shortcut,
                                                     Glib::RefPtr<Gtk::AccelGroup> accelGroup,
                                                     const char* desc,
                                                     gpointer action_data,
                                                     sigc::signal<void, bool>* signal_set_sensitive,
                                                     sigc::signal<void, bool>* signal_set_visible,
                                                     std::list<sigc::connection>* pListConnections/*= nullptr*/,
                                                     const bool use_underline/*= true*/)
{
    Gtk::MenuItem* pMenuItem = Gtk::manage(new Gtk::MenuItem{});

    if (desc && strlen(desc)) {
        pMenuItem->set_tooltip_text(desc);
    }
    // Now create the label and add it to the menu item
    Gtk::AccelLabel* pLabel = Gtk::manage(new Gtk::AccelLabel{name, use_underline});
    pLabel->set_xalign(0.0);
    pLabel->set_accel_widget(*pMenuItem);
    if (shortcut && strlen(shortcut)) {
        Gtk::AccelKey accel_key(shortcut);
        pMenuItem->add_accelerator("activate", accelGroup, accel_key.get_key(), accel_key.get_mod(), Gtk::ACCEL_VISIBLE);
    }

    _add_menu_item_image_or_label(pMenuItem, image, pLabel);

    if (signal_set_sensitive) {
        sigc::connection conn = signal_set_sensitive->connect(
            sigc::bind<0>(
                sigc::ptr_fun(&gtk_widget_set_sensitive),
                GTK_WIDGET(pMenuItem->gobj())));
        if (pListConnections) {
            pListConnections->push_back(std::move(conn));
        }
    }
    if (signal_set_visible) {
        sigc::connection conn = signal_set_visible->connect(
            sigc::bind<0>(
                sigc::ptr_fun(&gtk_widget_set_visible),
                GTK_WIDGET(pMenuItem->gobj())));
        if (pListConnections) {
            pListConnections->push_back(std::move(conn));
        }
    }
    if (action_data) {
        gtk_widget_set_events(GTK_WIDGET(pMenuItem->gobj()), GDK_KEY_PRESS_MASK);
        g_signal_connect(G_OBJECT(pMenuItem->gobj()), "activate", G_CALLBACK(on_menu_activate), action_data);
    }

    pMenuItem->show_all();
    pMenuShell->append(*pMenuItem);

    return pMenuItem;
}

/*static*/ void CtMenu::_add_menu_item_image_or_label(Gtk::MenuItem* pMenuItem, const char* image, Gtk::AccelLabel* pLabel)
{
    if (image && strlen(image)) {
        pMenuItem->set_name("ImageMenuItem");  // custom name to identify our "ImageMenuItems"
        Gtk::Image* pIcon = Gtk::manage(new Gtk::Image{});
        pIcon->set_from_icon_name(image, Gtk::ICON_SIZE_MENU);

        // create a box to hold icon and label as GtkMenuItem derives from GtkBin and can only hold one child
        static const int cImageMenuItemSpacing{8};
        auto pBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, cImageMenuItemSpacing});
        if (pMenuItem->get_direction() == Gtk::TEXT_DIR_RTL) {
            pBox->pack_end(*pIcon, false, false);
            pBox->pack_end(*pLabel, true, true);
        }
        else {
            pBox->pack_start(*pIcon, false, false);
            pBox->pack_start(*pLabel, true, true);
        }
        pMenuItem->add(*pBox);

        // to fix image placement in MenuBar /context menu
        // based on inkscape: src/ui/desktop/menu-icon-shift.cpp
        // we don't know which MenuItem will be first mapped, so connect all of them
        auto f_calculate_image_shift = [](Gtk::MenuItem* menuItem)->int{
            if (menuItem) {
                if (auto box = dynamic_cast<Gtk::Box*>(menuItem->get_child())) {
                    if (auto image = dynamic_cast<Gtk::Image*>(box->get_children()[0])) {
                        auto allocation_menuitem = menuItem->get_allocation();
                        auto allocation_image = image->get_allocation();
                        if (menuItem->get_direction() == Gtk::TEXT_DIR_RTL) {
                            return allocation_menuitem.get_width() - allocation_image.get_x() - allocation_image.get_width();
                        }
                        return -allocation_image.get_x();
                    }
                }
            }
            return 0;
        };
        static std::list<sigc::connection>* static_map_connectons = new std::list<sigc::connection>();
        if (static_map_connectons != nullptr) { // if null then the fix was applied
            static_map_connectons->push_back(pMenuItem->signal_map().connect([f_calculate_image_shift, pMenuItem]{
                spdlog::debug("shift images in MenuBar/context menu");
                const int shift = f_calculate_image_shift(pMenuItem);
                if (shift != 0) {
                    auto provider = Gtk::CssProvider::create();
                    auto const screen = Gdk::Screen::get_default();
                    Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                    if (pMenuItem->get_direction() == Gtk::TEXT_DIR_RTL) {
                        provider->load_from_data("menuitem box { margin-right:" + std::to_string(shift - cImageMenuItemSpacing) + "px; }");
                    }
                    else {
                        provider->load_from_data("menuitem box { margin-left:" + std::to_string(shift + cImageMenuItemSpacing) + "px; }");
                    }
                }
                // we don't need to call this again, so kill all map connections
                for (auto& connections : *static_map_connectons)
                    connections.disconnect();
                delete static_map_connectons;
                static_map_connectons = nullptr;
            }));
        }
    }
    else {
        pMenuItem->add(*pLabel);
    }
}

Gtk::SeparatorMenuItem* CtMenu::_add_menu_separator(Gtk::MenuShell* pMenuShell)
{
    Gtk::SeparatorMenuItem* pSeparatorItem = Gtk::manage(new Gtk::SeparatorMenuItem());
    pSeparatorItem->show_all();
    pMenuShell->append(*pSeparatorItem);
    return pSeparatorItem;
}
