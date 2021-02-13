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

Gtk::Widget* CtPrefDlg::build_tab_kb_shortcuts()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Glib::RefPtr<Gtk::TreeStore> treestore = Gtk::TreeStore::create(_shortcutModelColumns);
    fill_shortcut_model(treestore);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(treestore));
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
    auto shortcut_cell_renderer = Gtk::manage(new Gtk::CellRendererText());
    shortcut_cell_renderer->property_xalign() = 1;
    auto shortcut_column = Gtk::manage(new Gtk::TreeViewColumn());
    shortcut_column->pack_start(*shortcut_cell_renderer, true);
    shortcut_column->set_cell_data_func(*shortcut_cell_renderer, [&](Gtk::CellRenderer* cell, const Gtk::TreeIter& iter){
        ((Gtk::CellRendererText*)cell)->property_markup() = "  " + str::xml_escape(CtStrUtil::get_accelerator_label(iter->get_value(_shortcutModelColumns.shortcut))) + "  ";
    });
    treeview->append_column(*shortcut_column);
    // desc
    treeview->append_column("", _shortcutModelColumns.desc);

    treeview->expand_all();
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->add(*treeview);

    Gtk::VBox* vbox_buttons = Gtk::manage(new Gtk::VBox());
    Gtk::Button* button_edit = Gtk::manage(new Gtk::Button());
    button_edit->set_image(*_pCtMainWin->new_image_from_stock("ct_edit",  Gtk::ICON_SIZE_BUTTON));
    button_edit->set_tooltip_text(_("Change Selected"));
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock("ct_undo",  Gtk::ICON_SIZE_BUTTON));
    button_reset->set_tooltip_text(_("Reset to Default"));
    vbox_buttons->pack_start(*button_edit, false, false);
    vbox_buttons->pack_start(*Gtk::manage(new Gtk::Label()), true, true);
    vbox_buttons->pack_start(*button_reset, false, false);
    Gtk::HBox* hbox_main = Gtk::manage(new Gtk::HBox());
    hbox_main->pack_start(*scrolledwindow, true, true);
    hbox_main->pack_start(*vbox_buttons, false, false);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->add(*hbox_main);

    button_edit->signal_clicked().connect([this, treeview](){
        if (edit_shortcut(treeview))
            need_restart(RESTART_REASON::SHORTCUT);
    });
    button_reset->signal_clicked().connect([this, pConfig, treestore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->customKbShortcuts.clear();
            fill_shortcut_model(treestore);
            need_restart(RESTART_REASON::SHORTCUT);
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
            for(const CtMenuAction& action : _pCtMenu->get_actions())
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

    Gtk::Dialog dialog(_("Edit Keyboard Shortcut"), *this, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 100);
    Gtk::RadioButton* radiobutton_kb_none = Gtk::manage(new Gtk::RadioButton(_("No Keyboard Shortcut")));
    Gtk::RadioButton* radiobutton_kb_shortcut = Gtk::manage(new Gtk::RadioButton());
    radiobutton_kb_shortcut->join_group(*radiobutton_kb_none);
    Gtk::ToggleButton* ctrl_toggle = Gtk::manage(new Gtk::ToggleButton("control"));
    Gtk::ToggleButton* shift_toggle = Gtk::manage(new Gtk::ToggleButton("shift"));
    Gtk::ToggleButton* alt_toggle = Gtk::manage(new Gtk::ToggleButton("alt"));
    Gtk::ToggleButton* meta_toggle = Gtk::manage(new Gtk::ToggleButton("meta"));
    ctrl_toggle->set_size_request(70, 1);
    shift_toggle->set_size_request(70, 1);
    alt_toggle->set_size_request(70, 1);
    meta_toggle->set_size_request(70,1);
    Gtk::Entry* key_entry = Gtk::manage(new Gtk::Entry());
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    hbox->pack_start(*radiobutton_kb_shortcut);
    hbox->pack_start(*ctrl_toggle);
    hbox->pack_start(*shift_toggle);
    hbox->pack_start(*alt_toggle);
    hbox->pack_start(*meta_toggle);
    hbox->pack_start(*key_entry);
    hbox->set_spacing(5);
    vbox->pack_start(*radiobutton_kb_none);
    vbox->pack_start(*hbox);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(*vbox);

    auto b1 = Glib::Binding::bind_property(key_entry->property_sensitive(), ctrl_toggle->property_sensitive());
    auto b2 = Glib::Binding::bind_property(key_entry->property_sensitive(), shift_toggle->property_sensitive());
    auto b3 = Glib::Binding::bind_property(key_entry->property_sensitive(), alt_toggle->property_sensitive());
    auto b5 = Glib::Binding::bind_property(key_entry->property_sensitive(), meta_toggle->property_sensitive());
    auto b4 = Glib::Binding::bind_property(radiobutton_kb_shortcut->property_active(), key_entry->property_sensitive());

    key_entry->set_sensitive(!kb_shortcut_key.empty());
    key_entry->set_text(kb_shortcut_key);
    radiobutton_kb_none->set_active(kb_shortcut_key.empty());
    radiobutton_kb_shortcut->set_active(!kb_shortcut_key.empty());
    ctrl_toggle->set_active(shortcut.find(_pCtMenu->KB_CONTROL) != std::string::npos);
    shift_toggle->set_active(shortcut.find(_pCtMenu->KB_SHIFT) != std::string::npos);
    alt_toggle->set_active(shortcut.find(_pCtMenu->KB_ALT) != std::string::npos);
    meta_toggle->set_active(shortcut.find(_pCtMenu->KB_META) != std::string::npos);

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
    for(const CtMenuAction& action: _pCtMenu->get_actions())
    {
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
