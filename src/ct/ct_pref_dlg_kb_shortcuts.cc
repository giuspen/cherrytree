/*
 * ct_pref_dlg_kb_shortcuts.cc
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
#include <optional>

Gtk::Widget* CtPrefDlg::build_tab_kb_shortcuts()
{
    Glib::RefPtr<Gtk::TreeStore> treestore = Gtk::TreeStore::create(_shortcutModelColumns);
    fill_shortcut_model(treestore);
    auto treeview = Gtk::manage(new Gtk::TreeView{treestore});
    treeview->set_headers_visible(false);
    treeview->set_reorderable(true);
    treeview->set_size_request(300, 300);
    treeview->set_reorderable(false);

    // icon column
    Gtk::CellRendererPixbuf pixbuf_renderer;
    pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
    const int col_num_pixbuf = treeview->append_column("", pixbuf_renderer) - 1;
    treeview->get_column(col_num_pixbuf)->add_attribute(pixbuf_renderer, "icon-name", _shortcutModelColumns.icon);
    // shortcut column
    auto shortcut_cell_renderer = Gtk::manage(new Gtk::CellRendererText{});
    shortcut_cell_renderer->property_xalign() = 1;
    auto shortcut_column = Gtk::manage(new Gtk::TreeViewColumn{});
    shortcut_column->pack_start(*shortcut_cell_renderer, true);
    shortcut_column->set_cell_data_func(*shortcut_cell_renderer, [&](Gtk::CellRenderer* cell, const Gtk::TreeIter& iter){
        ((Gtk::CellRendererText*)cell)->property_markup() = "  " + str::xml_escape(CtStrUtil::get_accelerator_label(iter->get_value(_shortcutModelColumns.shortcut))) + "  ";
    });
    treeview->append_column(*shortcut_column);
    // desc
    treeview->append_column("", _shortcutModelColumns.desc);

    treeview->expand_all();
    auto scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow{});
    scrolledwindow->add(*treeview);

    auto vbox_buttons = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto button_edit = Gtk::manage(new Gtk::Button{});
    button_edit->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_edit",  Gtk::ICON_SIZE_BUTTON));
    button_edit->set_tooltip_text(_("Change Selected"));
    auto button_reset = Gtk::manage(new Gtk::Button{});
    button_reset->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo",  Gtk::ICON_SIZE_BUTTON));
    button_reset->set_tooltip_text(_("Reset to Default"));
    vbox_buttons->pack_start(*button_edit, false, false);
    vbox_buttons->pack_start(*Gtk::manage(new Gtk::Label{}), true, true);
    vbox_buttons->pack_start(*button_reset, false, false);
    auto hbox_main = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    hbox_main->pack_start(*scrolledwindow, true, true);
    hbox_main->pack_start(*vbox_buttons, false, false);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->pack_start(*hbox_main);

    auto f_after_treestore_changes = [this, scrolledwindow, treeview, treestore](){
        const auto vscroll = scrolledwindow->get_vadjustment()->get_value();
        auto tree_iter = treeview->get_selection()->get_selected();
        std::optional<Gtk::TreePath> tree_path;
        if (tree_iter) tree_path = treestore->get_path(tree_iter);
        fill_shortcut_model(treestore);
        treeview->expand_all();
        while (gtk_events_pending()) gtk_main_iteration();
        scrolledwindow->get_vadjustment()->set_value(vscroll);
        if (tree_path.has_value()) treeview->set_cursor(tree_path.value());
        need_restart(RESTART_REASON::SHORTCUT);
    };
    button_edit->signal_clicked().connect([this, treeview, f_after_treestore_changes](){
        if (edit_shortcut(treeview)) {
            f_after_treestore_changes();
        }
    });
    button_reset->signal_clicked().connect([this, f_after_treestore_changes](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            _pConfig->customKbShortcuts.clear();
            f_after_treestore_changes();
        }
    });
    treeview->signal_row_activated().connect([button_edit](const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*){
        button_edit->clicked();
    });
    auto button_edit_test_sensitive = [this, button_edit, treeview](){
        auto iter_sel = treeview->get_selection()->get_selected();
        button_edit->set_sensitive(iter_sel and not iter_sel->get_value(_shortcutModelColumns.key).empty());
    };
    treeview->signal_cursor_changed().connect(button_edit_test_sensitive);
    button_edit_test_sensitive();

    return pMainBox;
}

bool CtPrefDlg::edit_shortcut(Gtk::TreeView* treeview)
{
    auto tree_iter = treeview->get_selection()->get_selected();
    if (!tree_iter || tree_iter->get_value(_shortcutModelColumns.key).empty()) return false;
    std::string shortcut = tree_iter->get_value(_shortcutModelColumns.shortcut);
    std::string id = tree_iter->get_value(_shortcutModelColumns.key);
    if (edit_shortcut_dialog(shortcut)) {
        if (shortcut != "") {
            for (const CtMenuAction& action : _pCtMenu->get_actions())
                if (action.get_shortcut(_pCtMainWin->get_ct_config()) == shortcut && action.id != id) {
                    // todo: this is a shorter version from python code
                    std::string message = "<b>" + str::format(_("The Keyboard Shortcut '%s' is already in use"), CtStrUtil::get_accelerator_label(shortcut)) + "</b>\n\n";
                    message += str::format(_("The current associated action is '%s'"), str::replace(action.name, "_", "")) + "\n\n";
                    message += "<b>" + std::string(_("Do you want to steal the shortcut?")) + "</b>";
                    if (!CtDialogs::question_dialog(message, *this))
                        return false;
                    _pCtMainWin->get_ct_config()->customKbShortcuts[action.id] = "";
                }
        }
        _pCtMainWin->get_ct_config()->customKbShortcuts[id] = shortcut;
        return true;
    }
    return false;
}

bool CtPrefDlg::edit_shortcut_dialog(std::string& shortcut)
{
    std::string kb_shortcut_key = shortcut;
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_CONTROL.c_str(), "");
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_SHIFT.c_str(), "");
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_ALT.c_str(), "");
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_META.c_str(), "");

    Gtk::Dialog dialog{_("Edit Keyboard Shortcut"),
                       *this,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 100);
    auto radiobutton_kb_none = Gtk::manage(new Gtk::RadioButton{_("No Keyboard Shortcut")});
    auto radiobutton_kb_shortcut = Gtk::manage(new Gtk::RadioButton{});
    radiobutton_kb_shortcut->join_group(*radiobutton_kb_none);
    auto ctrl_toggle = Gtk::manage(new Gtk::ToggleButton{"Ctrl"});
    auto shift_toggle = Gtk::manage(new Gtk::ToggleButton{"Shift"});
    auto alt_toggle = Gtk::manage(new Gtk::ToggleButton{"Alt"});
    auto meta_toggle = Gtk::manage(new Gtk::ToggleButton{"Meta"});
    ctrl_toggle->set_size_request(70, 1);
    shift_toggle->set_size_request(70, 1);
    alt_toggle->set_size_request(70, 1);
    meta_toggle->set_size_request(70,1);
    auto key_entry = Gtk::manage(new Gtk::Entry{});
    auto vbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 5/*spacing*/});
    hbox->pack_start(*radiobutton_kb_shortcut);
    hbox->pack_start(*ctrl_toggle);
    hbox->pack_start(*shift_toggle);
    hbox->pack_start(*alt_toggle);
    hbox->pack_start(*meta_toggle);
    hbox->pack_start(*key_entry);
    vbox->pack_start(*radiobutton_kb_none);
    vbox->pack_start(*hbox);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(*vbox);

    key_entry->set_text(kb_shortcut_key);
    radiobutton_kb_none->set_active(kb_shortcut_key.empty());
    radiobutton_kb_shortcut->set_active(!kb_shortcut_key.empty());
    ctrl_toggle->set_active(shortcut.find(_pCtMenu->KB_CONTROL) != std::string::npos);
    shift_toggle->set_active(shortcut.find(_pCtMenu->KB_SHIFT) != std::string::npos);
    alt_toggle->set_active(shortcut.find(_pCtMenu->KB_ALT) != std::string::npos);
    meta_toggle->set_active(shortcut.find(_pCtMenu->KB_META) != std::string::npos);

    auto f_kb_shortcut_on_off = [ctrl_toggle, shift_toggle, alt_toggle, meta_toggle, key_entry, radiobutton_kb_shortcut](){
        const bool isSensitive = radiobutton_kb_shortcut->get_active();
        ctrl_toggle->set_sensitive(isSensitive);
        shift_toggle->set_sensitive(isSensitive);
        alt_toggle->set_sensitive(isSensitive);
        meta_toggle->set_sensitive(isSensitive);
        key_entry->set_sensitive(isSensitive);
    };
    f_kb_shortcut_on_off();

    radiobutton_kb_shortcut->signal_toggled().connect(f_kb_shortcut_on_off);
    radiobutton_kb_none->signal_toggled().connect(f_kb_shortcut_on_off);

    key_entry->signal_key_press_event().connect([key_entry](GdkEventKey* key)->bool{
        Glib::ustring keyname = gdk_keyval_name(key->keyval);
        if (keyname.size() == 1) keyname = keyname.uppercase();
        key_entry->set_text(keyname);
        key_entry->select_region(0, (int)keyname.size());
        return true;
    }, false);

    content_area->show_all();
    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return false;

    shortcut = "";
    if (radiobutton_kb_shortcut->get_active() && !key_entry->get_text().empty()) {
        if (ctrl_toggle->get_active()) shortcut += _pCtMenu->KB_CONTROL;
        if (shift_toggle->get_active()) shortcut += _pCtMenu->KB_SHIFT;
        if (alt_toggle->get_active()) shortcut += _pCtMenu->KB_ALT;
        if (meta_toggle->get_active()) shortcut += _pCtMenu->KB_META;
        shortcut += key_entry->get_text();
    }
    return true;
}

void CtPrefDlg::fill_shortcut_model(Glib::RefPtr<Gtk::TreeStore> model)
{
    model->clear();
    std::string category_name = "no name";
    Gtk::TreeModel::Row cat_row;
    for (const CtMenuAction& action : _pCtMenu->get_actions()) {
        if (action.category.empty()) continue;
        if (action.category != category_name) {
            category_name = action.category;
            cat_row = *model->append();
            cat_row[_shortcutModelColumns.desc] = action.category;
        }
        auto row = *model->append(cat_row.children());
        row[_shortcutModelColumns.icon] = action.image;
        row[_shortcutModelColumns.key] = action.id;
        row[_shortcutModelColumns.desc] = action.desc;
        row[_shortcutModelColumns.shortcut] = action.get_shortcut(_pCtMainWin->get_ct_config());
    }
}
