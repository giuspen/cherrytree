/*
 * ct_pref_dlg_toolbar.cc
 *
 * Copyright 2009-2026
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
    auto add_to_box = [](Gtk::Box* box, Gtk::Widget& child, bool expand = false, bool fill = false) {
#if GTKMM_MAJOR_VERSION >= 4
        box->append(child);
        child.set_hexpand(expand);
        child.set_vexpand(expand);
#else
        box->pack_start(child, expand, fill);
#endif
    };

    Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(_toolbarModelColumns);
    fill_toolbar_model(liststore);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(liststore));
    treeview->set_headers_visible(false);
    treeview->set_reorderable(true);
    treeview->set_size_request(300, 300);
    #if GTKMM_MAJOR_VERSION >= 4
    {
        auto const_iter = liststore->children().begin();
        if (const_iter)
            treeview->get_selection()->select(liststore->get_iter(liststore->get_path(const_iter)));
    }
    #else
    treeview->get_selection()->select(Gtk::TreePath("0"));
    #endif

    Gtk::CellRendererPixbuf pixbuf_renderer;
#if GTKMM_MAJOR_VERSION >= 4
    pixbuf_renderer.property_icon_size() = Gtk::IconSize::LARGE;
#else
    pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
#endif
    const int col_num_pixbuf = treeview->append_column("", pixbuf_renderer) - 1;
    treeview->get_column(col_num_pixbuf)->add_attribute(pixbuf_renderer, "icon-name", _toolbarModelColumns.icon);

    treeview->append_column("", _toolbarModelColumns.desc);
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
#if GTKMM_MAJOR_VERSION >= 4
    scrolledwindow->set_child(*treeview);
#else
    scrolledwindow->add(*treeview);
#endif

    Gtk::Button* button_add = Gtk::manage(new Gtk::Button());
#if GTKMM_MAJOR_VERSION >= 4
    button_add->set_icon_name("ct_add");
#else
    button_add->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_add",  Gtk::ICON_SIZE_BUTTON));
#endif
    button_add->set_tooltip_text(_("Add"));
    Gtk::Button* button_remove = Gtk::manage(new Gtk::Button());
#if GTKMM_MAJOR_VERSION >= 4
    button_remove->set_icon_name("ct_remove");
#else
    button_remove->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_remove", Gtk::ICON_SIZE_BUTTON));
#endif
    button_remove->set_tooltip_text(_("Remove Selected"));
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
#if GTKMM_MAJOR_VERSION >= 4
    button_reset->set_icon_name("ct_undo");
#else
    button_reset->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
    button_reset->set_tooltip_text(_("Reset to Default"));

    auto hbox = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
#else
        Gtk::ORIENTATION_HORIZONTAL
#endif
    });
    auto vbox = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
#else
        Gtk::ORIENTATION_VERTICAL
#endif
    });
    add_to_box(vbox, *button_add);
    add_to_box(vbox, *button_remove);
    add_to_box(vbox, *Gtk::manage(new Gtk::Label()), true, true);
    add_to_box(vbox, *button_reset);
    add_to_box(hbox, *scrolledwindow, true, true);
    add_to_box(hbox, *vbox);

    auto pMainBox = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
#else
        Gtk::ORIENTATION_VERTICAL
#endif
        , 3/*spacing*/});
    add_to_box(pMainBox, *hbox, true, true);

    auto model_update_suppressed = std::make_shared<bool>(false);
    button_add->signal_clicked().connect([this, treeview, liststore, model_update_suppressed](){
#if GTKMM_MAJOR_VERSION >= 4
        *model_update_suppressed = true;
#endif
        const bool added = add_new_item_in_toolbar_model(treeview, liststore);
#if GTKMM_MAJOR_VERSION >= 4
        *model_update_suppressed = false;
#endif
        if (added) {
            update_config_toolbar_from_model(liststore);
            apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
        }
    });
    auto remove_selected_item = [this, treeview, liststore, model_update_suppressed](){
        auto selected = treeview->get_selection()->get_selected();
        if (!selected) return;
#if GTKMM_MAJOR_VERSION >= 4
        *model_update_suppressed = true;
#endif
        liststore->erase(selected);
#if GTKMM_MAJOR_VERSION >= 4
        *model_update_suppressed = false;
#endif
        update_config_toolbar_from_model(liststore);
        apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
    };
    button_remove->signal_clicked().connect(remove_selected_item);
    button_reset->signal_clicked().connect([this, liststore, model_update_suppressed](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            _pConfig->toolbarUiList = CtConst::TOOLBAR_VEC_DEFAULT;
            *model_update_suppressed = true;
            fill_toolbar_model(liststore);
            *model_update_suppressed = false;
            update_config_toolbar_from_model(liststore);
            apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
        }
    });
#if GTKMM_MAJOR_VERSION >= 4
    auto controller_key = Gtk::EventControllerKey::create();
    controller_key->signal_key_pressed().connect([remove_selected_item](guint keyval, guint, Gdk::ModifierType) {
        if (keyval == GDK_KEY_Delete) {
            remove_selected_item();
            return true;
        }
        return false;
    }, false);
    treeview->add_controller(controller_key);
#else
    treeview->signal_key_press_event().connect([remove_selected_item](GdkEventKey* key) -> bool {
        if (key->keyval == GDK_KEY_Delete) {
            remove_selected_item();
            return true;
        }
        return false;
    });
#endif
#if GTKMM_MAJOR_VERSION >= 4
    auto update_after_model_change = [this, liststore, model_update_suppressed](){
        if (*model_update_suppressed) return;
        update_config_toolbar_from_model(liststore);
        apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
    };
    liststore->signal_row_inserted().connect([update_after_model_change](const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator&){
        update_after_model_change();
    });
    liststore->signal_row_deleted().connect([update_after_model_change](const Gtk::TreeModel::Path&){
        update_after_model_change();
    });
    liststore->signal_rows_reordered().connect([update_after_model_change](const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator&, int*){
        update_after_model_change();
    });
#else
    treeview->signal_drag_end().connect([this, liststore](const Glib::RefPtr<Gdk::DragContext>&){
        update_config_toolbar_from_model(liststore);
        apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
    });
#endif
    auto button_remove_test_sensitive = [button_remove, treeview](){
        button_remove->set_sensitive(treeview->get_selection()->count_selected_rows() > 0);
    };
    treeview->signal_cursor_changed().connect(button_remove_test_sensitive);
    button_remove_test_sensitive();

    return pMainBox;
}

void CtPrefDlg::fill_toolbar_model(Glib::RefPtr<Gtk::ListStore> model)
{
    std::vector<std::string> vecToolbarElements = str::split(_pConfig->toolbarUiList, ",");
    model->clear();
    for (const std::string& key : vecToolbarElements) {
        populate_row_in_toolbar_model(model->append(), key);
    }
}

void CtPrefDlg::populate_row_in_toolbar_model(Gtk::TreeModel::iterator row, const Glib::ustring& key)
{
    Glib::ustring icon, desc;
    if (key == Glib::ustring{CtConst::TAG_SEPARATOR}) {
        desc = CtConst::TAG_SEPARATOR_ANSI_REPR;
    }
    else if (key == Glib::ustring{CtConst::TOOLBAR_SPLIT}) {
        desc = _("Split Toolbar");
    }
    else if (key == Glib::ustring{CtConst::CHAR_STAR}) {
        icon = "ct_open";
        desc = _("Open a CherryTree File");
    }
    else if (CtMenuAction const* action = _pCtMenu->find_action(key)) {
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
    std::vector<std::string> vecToolbarElements = str::split(_pConfig->toolbarUiList, ",");
    for (const CtMenuAction& action : _pCtMenu->get_actions()) {
        if (action.desc.empty()) continue; // skip stub menu entries
        if (action.category.empty()) continue; // skip popup menu entries
        if (action.id == "ct_open_file" and
            vecToolbarElements.end() != std::find(vecToolbarElements.begin(), vecToolbarElements.end(), CtConst::CHAR_STAR))
        {
            continue;
        }
        if (vecToolbarElements.end() != std::find(vecToolbarElements.begin(), vecToolbarElements.end(), action.id)) {
            // do not allow to add a button that already exist
            continue;
        }
        Glib::ustring id = action.id == "ct_open_file" ? CtConst::CHAR_STAR : action.id;
        itemStore->add_row(action.image, id, action.desc);
    }

    auto chosen_row = CtDialogs::choose_item_dialog(*this,
                                                    _("Select Element to Add"),
                                                    itemStore,
                                                    nullptr/*single_column_name*/,
                                                    "0",
                                                    std::make_pair(500, _pConfig->winRect[3]));
    if (chosen_row) {
        auto selected_row = treeview->get_selection()->get_selected();
        auto new_row = selected_row ? model->insert_after(*selected_row) : model->append();
        populate_row_in_toolbar_model(new_row, chosen_row->get_value(itemStore->columns.key));
        return true;
    }
    return false;
}

void CtPrefDlg::update_config_toolbar_from_model(Glib::RefPtr<Gtk::ListStore> model)
{
    std::vector<std::string> items;
    for (auto it : model->children()) {
        items.push_back(it.get_value(_toolbarModelColumns.key));
    }
    _pConfig->toolbarUiList = str::join(items, ",");
}
