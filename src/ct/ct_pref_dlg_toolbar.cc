/*
 * ct_pref_dlg_toolbar.cc
 *
 * Copyright 2009-2021
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

#include "ct_pref_dlg.h"
#include "ct_main_win.h"

Gtk::Widget* CtPrefDlg::build_tab_toolbar()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(_toolbarModelColumns);
    fill_toolbar_model(liststore);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(liststore));
    treeview->set_headers_visible(false);
    treeview->set_reorderable(true);
    treeview->set_size_request(300, 300);
    treeview->get_selection()->select(Gtk::TreePath("0"));

    Gtk::CellRendererPixbuf pixbuf_renderer;
    pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
    const int col_num_pixbuf = treeview->append_column("", pixbuf_renderer) - 1;
    treeview->get_column(col_num_pixbuf)->add_attribute(pixbuf_renderer, "icon-name", _shortcutModelColumns.icon);

    treeview->append_column("", _toolbarModelColumns.desc);
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->add(*treeview);

    Gtk::Button* button_add = Gtk::manage(new Gtk::Button());
    button_add->set_image(*_pCtMainWin->new_image_from_stock("ct_add",  Gtk::ICON_SIZE_BUTTON));
    button_add->set_tooltip_text(_("Add"));
    Gtk::Button* button_remove = Gtk::manage(new Gtk::Button());
    button_remove->set_image(*_pCtMainWin->new_image_from_stock("ct_remove", Gtk::ICON_SIZE_BUTTON));
    button_remove->set_tooltip_text(_("Remove Selected"));
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset->set_tooltip_text(_("Reset to Default"));

    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());
    vbox->pack_start(*button_add, false, false);
    vbox->pack_start(*button_remove, false, false);
    vbox->pack_start(*Gtk::manage(new Gtk::Label()), true, true);
    vbox->pack_start(*button_reset, false, false);
    hbox->pack_start(*scrolledwindow, true, true);
    hbox->pack_start(*vbox, false, false);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*hbox);

    button_add->signal_clicked().connect([this, treeview, liststore](){
        if (add_new_item_in_toolbar_model(treeview, liststore)) {
            update_config_toolbar_from_model(liststore);
            apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
        }
    });
    button_remove->signal_clicked().connect([this, treeview, liststore](){
        liststore->erase(treeview->get_selection()->get_selected());
        update_config_toolbar_from_model(liststore);
        apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
    });
    button_reset->signal_clicked().connect([this, pConfig, liststore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->toolbarUiList = CtConst::TOOLBAR_VEC_DEFAULT;
            fill_toolbar_model(liststore);
            apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
        }
    });
    treeview->signal_key_press_event().connect([button_remove](GdkEventKey* key) -> bool {
        if (key->keyval == GDK_KEY_Delete) {
            button_remove->clicked();
            return true;
        }
        return false;
    });
    treeview->signal_drag_end().connect([this, liststore](const Glib::RefPtr<Gdk::DragContext>&){
        update_config_toolbar_from_model(liststore);
        apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
    });
    auto button_remove_test_sensitive = [button_remove, treeview](){
        button_remove->set_sensitive(treeview->get_selection()->get_selected());
    };
    treeview->signal_cursor_changed().connect(button_remove_test_sensitive);
    button_remove_test_sensitive();

    return pMainBox;
}

void CtPrefDlg::fill_toolbar_model(Glib::RefPtr<Gtk::ListStore> model)
{
    std::vector<std::string> vecToolbarElements = str::split(_pCtMainWin->get_ct_config()->toolbarUiList, ",");
    model->clear();
    for(const std::string& key: vecToolbarElements)
        add_new_item_in_toolbar_model(model->append(), key);
}

void CtPrefDlg::add_new_item_in_toolbar_model(Gtk::TreeIter row, const Glib::ustring& key)
{
    Glib::ustring icon, desc;
    if (key == CtConst::TAG_SEPARATOR)
    {
        desc = CtConst::TAG_SEPARATOR_ANSI_REPR;
    }
    else if (key == CtConst::TOOLBAR_SPLIT)
    {
        desc = _("Split Toolbar");
    }
    else if (key == CtConst::CHAR_STAR)
    {
        icon = "ct_open";
        desc = _("Open a CherryTree Document");

    }
    else if (CtMenuAction const* action = _pCtMenu->find_action(key))
    {
        icon = action->image;
        desc = action->desc;
    }

    row->set_value(_toolbarModelColumns.icon, icon);
    row->set_value(_toolbarModelColumns.key, key);
    row->set_value(_toolbarModelColumns.desc, desc);
}

bool CtPrefDlg::add_new_item_in_toolbar_model(Gtk::TreeView* treeview, Glib::RefPtr<Gtk::ListStore> model)
{
    auto itemStore = CtChooseDialogListStore::create();
    itemStore->add_row("", CtConst::TAG_SEPARATOR, CtConst::TAG_SEPARATOR_ANSI_REPR);
    itemStore->add_row("", CtConst::TOOLBAR_SPLIT, _("Split Toolbar"));
    for (const CtMenuAction& action: _pCtMenu->get_actions())
    {
        if (action.desc.empty()) continue; // skip stub menu entries
        if (action.id == "ct_open_file" && _pCtMainWin->get_ct_config()->toolbarUiList.find(CtConst::CHAR_STAR) != std::string::npos) continue;
        if (vec::exists(CtConst::TOOLBAR_VEC_BLACKLIST, action.id)) continue;
        Glib::ustring id = action.id == "ct_open_file" ? CtConst::CHAR_STAR : action.id;
        itemStore->add_row(action.image, id, action.desc);
    }

    auto chosen_row = CtDialogs::choose_item_dialog(*this, _("Select Element to Add"), itemStore);
    if (chosen_row) {
        auto selected_row = treeview->get_selection()->get_selected();
        auto new_row = selected_row ? model->insert_after(*selected_row) : model->append();
        add_new_item_in_toolbar_model(new_row, chosen_row->get_value(itemStore->columns.key));
        return true;
    }
    return false;
}

void CtPrefDlg::update_config_toolbar_from_model(Glib::RefPtr<Gtk::ListStore> model)
{
    std::vector<std::string> items;
    for (auto it: model->children())
        items.push_back(it.get_value(_toolbarModelColumns.key));
    _pCtMainWin->get_ct_config()->toolbarUiList = str::join(items, ",");
}
