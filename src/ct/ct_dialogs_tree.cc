/*
 * ct_dialogs_tree.cc
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

#include "ct_dialogs.h"
#include "ct_main_win.h"

bool CtDialogs::node_prop_dialog(const Glib::ustring &title,
                                 CtMainWin* pCtMainWin,
                                 CtNodeData& nodeData,
                                 const std::set<Glib::ustring>& tags_set)
{
    CtConfig* pCtConfig = pCtMainWin->get_ct_config();
    Gtk::Dialog dialog = Gtk::Dialog{title,
                                     *pCtMainWin,
                                     Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(300, -1);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    Gtk::Entry name_entry;
    name_entry.set_text(nodeData.name);

    auto grid_icons = Gtk::manage(new Gtk::Grid{});
    grid_icons->set_row_spacing(0);
    grid_icons->set_column_spacing(0);
    grid_icons->set_row_homogeneous(true);

    Gtk::CheckButton is_bold_checkbutton{_("Bold")};
    is_bold_checkbutton.set_active(nodeData.isBold);
    is_bold_checkbutton.set_margin_top(4);

    Gtk::CheckButton fg_checkbutton{_("Use Selected Color")};
    fg_checkbutton.set_active(not nodeData.foregroundRgb24.empty());
    Glib::ustring real_fg = not nodeData.foregroundRgb24.empty() ? nodeData.foregroundRgb24 : (not pCtConfig->currColour_nn.empty() ? pCtConfig->currColour_nn.c_str() : "red");
    Gtk::ColorButton fg_colorbutton{Gdk::RGBA{real_fg}};
    fg_colorbutton.set_sensitive(not nodeData.foregroundRgb24.empty());

    Gtk::CheckButton c_icon_checkbutton{_("Use Selected Icon")};
    c_icon_checkbutton.set_active(nodeData.customIconId > 0 and nodeData.customIconId < CtStockIcon::size());
    Gtk::Button c_icon_button;
    int currCustomIconId = c_icon_checkbutton.get_active() ? nodeData.customIconId : pCtConfig->lastIconSel;
    c_icon_button.set_image(*pCtMainWin->new_managed_image_from_stock(CtStockIcon::at(currCustomIconId), Gtk::ICON_SIZE_BUTTON));
    c_icon_button.set_sensitive(c_icon_checkbutton.get_active());

    grid_icons->attach(fg_checkbutton, 0, 1, 1, 1);
    grid_icons->attach(fg_colorbutton, 1, 1, 1, 1);
    grid_icons->attach(c_icon_checkbutton, 0, 2, 1, 1);
    grid_icons->attach(c_icon_button, 1, 2, 1, 1);

    Gtk::Box name_vbox{Gtk::ORIENTATION_VERTICAL};
    name_vbox.pack_start(name_entry);
    name_vbox.pack_start(is_bold_checkbutton);
    name_vbox.pack_start(*grid_icons);
    Gtk::Frame name_frame{Glib::ustring{"<b>"}+_("Node Name")+"</b>"};
    dynamic_cast<Gtk::Label*>(name_frame.get_label_widget())->set_use_markup(true);
    name_frame.set_shadow_type(Gtk::SHADOW_NONE);
    name_frame.add(name_vbox);
    Gtk::RadioButton radiobutton_rich_text{_("Rich Text")};
    Gtk::RadioButton::Group rbGroup = radiobutton_rich_text.get_group();
    Gtk::RadioButton radiobutton_plain_text(rbGroup, _("Plain Text"));
    Gtk::RadioButton radiobutton_auto_syntax_highl(rbGroup, _("Automatic Syntax Highlighting"));
    Gtk::Button button_prog_lang;
    std::string syntax_hl_id = nodeData.syntax;
    if (nodeData.syntax == CtConst::RICH_TEXT_ID || nodeData.syntax == CtConst::PLAIN_TEXT_ID) {
        syntax_hl_id = pCtConfig->autoSynHighl;
    }
    std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
    button_prog_lang.set_label(syntax_hl_id);
    button_prog_lang.set_image(*pCtMainWin->new_managed_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
    if (nodeData.syntax == CtConst::RICH_TEXT_ID) {
        radiobutton_rich_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    }
    else if (nodeData.syntax == CtConst::PLAIN_TEXT_ID) {
        radiobutton_plain_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    }
    else {
        radiobutton_auto_syntax_highl.set_active(true);
    }
    Gtk::Box type_vbox{Gtk::ORIENTATION_VERTICAL};
    type_vbox.pack_start(radiobutton_rich_text);
    type_vbox.pack_start(radiobutton_plain_text);
    type_vbox.pack_start(radiobutton_auto_syntax_highl);
    type_vbox.pack_start(button_prog_lang);
    Gtk::Frame type_frame{Glib::ustring{"<b>"}+_("Node Type")+"</b>"};
    dynamic_cast<Gtk::Label*>(type_frame.get_label_widget())->set_use_markup(true);
    type_frame.set_shadow_type(Gtk::SHADOW_NONE);
    type_frame.add(type_vbox);
    type_frame.set_sensitive(!nodeData.isReadOnly);

    Gtk::Box tags_hbox{Gtk::ORIENTATION_HORIZONTAL, 2/*spacing*/};
    Gtk::Entry tags_entry;
    tags_entry.set_text(nodeData.tags);
    Gtk::Button button_browse_tags;
    button_browse_tags.set_image(*pCtMainWin->new_managed_image_from_stock("ct_find", Gtk::ICON_SIZE_BUTTON));
    button_browse_tags.set_sensitive(!tags_set.empty());
    tags_hbox.pack_start(tags_entry);
    tags_hbox.pack_start(button_browse_tags, false, false);
    Gtk::Frame tags_frame{Glib::ustring{"<b>"}+_("Tags for Searching")+"</b>"};
    dynamic_cast<Gtk::Label*>(tags_frame.get_label_widget())->set_use_markup(true);
    tags_frame.set_shadow_type(Gtk::SHADOW_NONE);
    tags_frame.add(tags_hbox);

    Gtk::Label excl_label{_("Exclude from Searches")};
    Gtk::Box excl_hbox{Gtk::ORIENTATION_HORIZONTAL, 2/*spacing*/};
    excl_hbox.set_margin_left(5);
    Gtk::CheckButton excl_me_checkbutton{_("This Node")};
    excl_me_checkbutton.set_active(nodeData.excludeMeFromSearch);
    Gtk::CheckButton excl_ch_checkbutton{_("The Subnodes")};
    excl_ch_checkbutton.set_active(nodeData.excludeChildrenFromSearch);
    excl_hbox.pack_start(excl_label);
    excl_hbox.pack_start(excl_me_checkbutton);
    excl_hbox.pack_start(excl_ch_checkbutton);

    Gtk::CheckButton ro_checkbutton{_("Read Only")};
    ro_checkbutton.set_active(nodeData.isReadOnly);

    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->set_spacing(5);
    pContentArea->pack_start(name_frame);
    pContentArea->pack_start(type_frame);
    pContentArea->pack_start(tags_frame);
    pContentArea->pack_start(excl_hbox);
    pContentArea->pack_start(ro_checkbutton);
    pContentArea->show_all();
    name_entry.grab_focus();

    button_prog_lang.signal_clicked().connect([&dialog, &pCtMainWin, &button_prog_lang](){
        auto itemStore = CtChooseDialogListStore::create();
        unsigned pathSelectIdx{0};
        unsigned pathCurrIdx{0};
        const auto currSyntaxHighl = button_prog_lang.get_label();
        for (const auto& lang : pCtMainWin->get_language_manager()->get_language_ids()) {
            itemStore->add_row(pCtMainWin->get_code_icon_name(lang), "", lang);
            if (lang == currSyntaxHighl) {
                pathSelectIdx = pathCurrIdx;
            }
            ++pathCurrIdx;
        }
        const Gtk::TreeIter treeIter = CtDialogs::choose_item_dialog(dialog,
                                                                     _("Automatic Syntax Highlighting"),
                                                                     itemStore,
                                                                     nullptr/*single_column_name*/,
                                                                     std::to_string(pathSelectIdx));
        if (treeIter) {
            const Glib::ustring syntax_hl_id = treeIter->get_value(itemStore->columns.desc);
            const std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
            button_prog_lang.set_label(syntax_hl_id);
            button_prog_lang.set_image(*pCtMainWin->new_managed_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
        }
    });
    radiobutton_auto_syntax_highl.signal_toggled().connect([&radiobutton_auto_syntax_highl, &button_prog_lang](){
       button_prog_lang.set_sensitive(radiobutton_auto_syntax_highl.get_active());
    });
    button_browse_tags.signal_clicked().connect([&dialog, &tags_entry, &tags_set](){
        auto itemStore = CtChooseDialogListStore::create();
        for (const auto& tag : tags_set) {
            itemStore->add_row("", "", tag);
        }
        const Gtk::TreeIter treeIter = CtDialogs::choose_item_dialog(dialog, _("Choose Existing Tag"), itemStore, _("Tag Name"));
        if (treeIter) {
            std::string cur_tag = tags_entry.get_text();
            if  (str::endswith(cur_tag, CtConst::CHAR_SPACE)) {
                tags_entry.set_text(cur_tag + treeIter->get_value(itemStore->columns.desc));
            }
            else {
                tags_entry.set_text(cur_tag + CtConst::CHAR_SPACE + treeIter->get_value(itemStore->columns.desc));
            }
        }
    });
    ro_checkbutton.signal_toggled().connect([&ro_checkbutton, &type_frame](){
        type_frame.set_sensitive(!ro_checkbutton.get_active());
    });
    fg_checkbutton.signal_toggled().connect([&](){
        fg_colorbutton.set_sensitive(fg_checkbutton.get_active());
    });
    fg_colorbutton.signal_pressed().connect([&pCtMainWin, &fg_colorbutton](){
        Glib::ustring ret_colour = fg_colorbutton.get_rgba().to_string();
        if (CtDialogs::colour_pick_dialog(pCtMainWin, _("Pick a Color"), ret_colour, false) == CtPickDlgState::SELECTED) {
            fg_colorbutton.set_rgba(Gdk::RGBA(ret_colour));
        }
    });
    c_icon_checkbutton.signal_toggled().connect([&c_icon_checkbutton, &c_icon_button, &nodeData, &currCustomIconId](){
        const bool custom_icon_active = c_icon_checkbutton.get_active();
        c_icon_button.set_sensitive(custom_icon_active);
        if (custom_icon_active) {
            nodeData.customIconId = currCustomIconId;
        }
        else {
            nodeData.customIconId = 0;
        }
    });
    c_icon_button.signal_clicked().connect([&dialog, &pCtMainWin, &c_icon_button, &nodeData, &currCustomIconId](){
        auto itemStore = CtChooseDialogListStore::create();
        int pathCurrIdx{0};
        int pathSelectIdx{0};
        for (const auto i : CtConst::NODE_CUSTOM_ICONS_ORDERED) {
            itemStore->add_row(CtStockIcon::at(i), std::to_string(i), "");
            if (static_cast<int>(nodeData.customIconId) == i) {
                pathSelectIdx = pathCurrIdx;
            }
            ++pathCurrIdx;
        }
        const Gtk::TreeIter treeIter = CtDialogs::choose_item_dialog(dialog,
                                                                     _("Select Node Icon"),
                                                                     itemStore,
                                                                     nullptr/*single_column_name*/,
                                                                     std::to_string(pathSelectIdx));
        if (treeIter) {
            nodeData.customIconId = static_cast<guint32>(std::stoi(treeIter->get_value(itemStore->columns.key)));
            currCustomIconId = nodeData.customIconId;
            c_icon_button.set_label("");
            c_icon_button.property_always_show_image() = true; // to fix not showing image on Win32
            c_icon_button.set_image(*pCtMainWin->new_managed_image_from_stock(treeIter->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
        }
    });
    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        if (GDK_KEY_Escape == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_REJECT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    if (dialog.run() != Gtk::RESPONSE_ACCEPT) {
        return false;
    }

    nodeData.name = str::trim(name_entry.get_text());
    nodeData.name = str::replace(nodeData.name, "\r", ""); // the given name can contain \r and \n as a result
    nodeData.name = str::replace(nodeData.name, "\n", ""); // of cope/paste from some source
    nodeData.name = str::replace(nodeData.name, "\t", " ");

    if (nodeData.name.empty()) {
        nodeData.name = CtConst::CHAR_QUESTION;
    }
    if (radiobutton_rich_text.get_active()) {
        nodeData.syntax = CtConst::RICH_TEXT_ID;
    }
    else if (radiobutton_plain_text.get_active()) {
        nodeData.syntax = CtConst::PLAIN_TEXT_ID;
    }
    else {
        nodeData.syntax = button_prog_lang.get_label();
        pCtConfig->autoSynHighl = nodeData.syntax;
    }
    nodeData.tags = tags_entry.get_text();
    nodeData.isReadOnly = ro_checkbutton.get_active();
    nodeData.excludeMeFromSearch = excl_me_checkbutton.get_active();
    nodeData.excludeChildrenFromSearch = excl_ch_checkbutton.get_active();
    if (c_icon_checkbutton.get_active()) {
        pCtConfig->lastIconSel = nodeData.customIconId;
    }
    nodeData.isBold = is_bold_checkbutton.get_active();
    if (fg_checkbutton.get_active()) {
        nodeData.foregroundRgb24 = CtRgbUtil::get_rgb24str_from_str_any(fg_colorbutton.get_color().to_string());
        pCtConfig->currColour_nn = nodeData.foregroundRgb24;
    }
    else {
        nodeData.foregroundRgb24.clear();
    }
    return true;
}

Gtk::TreeIter CtDialogs::choose_node_dialog(CtMainWin* pCtMainWin,
                                            Gtk::TreeView& parentTreeView,
                                            const Glib::ustring& title,
                                            CtTreeStore* pCtTreeStore,
                                            Gtk::TreeIter sel_tree_iter)
{
    Gtk::Dialog dialog{title,
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);
    Gtk::TreeView treeview_2{pCtTreeStore->get_store()};
    treeview_2.set_headers_visible(false);
    treeview_2.set_search_column(1);
    treeview_2.append_column("", pCtTreeStore->get_columns().rColPixbuf);
    treeview_2.append_column("", pCtTreeStore->get_columns().colNodeName);
    Gtk::ScrolledWindow scrolledwindow;
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview_2);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(scrolledwindow);

    auto expand_collapse_row = [&treeview_2](Gtk::TreePath path){
        if (treeview_2.row_expanded(path)) {
            treeview_2.collapse_row(path);
        }
        else {
            treeview_2.expand_row(path, false);
        }
    };

    treeview_2.signal_event().connect([&treeview_2, &expand_collapse_row](GdkEvent* event)->bool{
        bool retVal{false}; // propagate event
        if ( (event->type != GDK_BUTTON_PRESS) &&
             (event->type != GDK_2BUTTON_PRESS) &&
             (event->type != GDK_KEY_PRESS) )
        {
            // do nothing
        }
        else if ( (event->type == GDK_BUTTON_PRESS) &&
                  (event->button.button == 2) )
        {
            Gtk::TreePath path_at_click;
            if (treeview_2.get_path_at_pos((int)event->button.x, (int)event->button.y, path_at_click))
            {
                expand_collapse_row(path_at_click);
                retVal = true; // stop event
            }
        }
        else if ( (event->type == GDK_2BUTTON_PRESS) &&
                  (event->button.button == 1) )
        {
            if (treeview_2.get_selection()->get_selected()) {
                expand_collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
                retVal = true; // stop event
            }
        }
        else if ( (event->type == GDK_KEY_PRESS) &&
                  (treeview_2.get_selection()->get_selected()) )
        {
            if (event->key.keyval == GDK_KEY_Left) {
                treeview_2.collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
                retVal = true; // stop event
            }
            else if (event->key.keyval == GDK_KEY_Right) {
                treeview_2.expand_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()), false);
                retVal = true; // stop event
            }
        }
        return retVal;
    });

    pContentArea->show_all();
    std::string expanded_collapsed_string = pCtTreeStore->treeview_get_tree_expanded_collapsed_string(parentTreeView);
    pCtTreeStore->treeview_set_tree_expanded_collapsed_string(expanded_collapsed_string, treeview_2, pCtMainWin->get_ct_config()->nodesBookmExp);
    if (sel_tree_iter) {
        Gtk::TreePath sel_path = treeview_2.get_model()->get_path(sel_tree_iter);
        treeview_2.expand_to_path(sel_path);
        treeview_2.set_cursor(sel_path);
        treeview_2.scroll_to_row(sel_path);
    }

    return dialog.run() == Gtk::RESPONSE_ACCEPT ? treeview_2.get_selection()->get_selected() : Gtk::TreeIter{};
}

// Dialog to select between the Selected Node/Selected Node + Subnodes/All Tree
CtExporting CtDialogs::selnode_selnodeandsub_alltree_dialog(Gtk::Window& parent,
                                                            bool also_selection,
                                                            bool* last_include_node_name,
                                                            bool* last_new_node_page,
                                                            bool* last_index_in_page,
                                                            bool* last_single_file)
{
    Gtk::Dialog dialog{_("Involved Nodes"),
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);

    auto radiobutton_selection = Gtk::RadioButton{_("Selected Text Only")};
    auto radiobutton_selnode = Gtk::RadioButton{_("Selected Node Only")};
    auto radiobutton_selnodeandsub = Gtk::RadioButton{_("Selected Node and Subnodes")};
    auto radiobutton_alltree = Gtk::RadioButton{_("All the Tree")};
    radiobutton_selnodeandsub.join_group(radiobutton_selnode);
    radiobutton_alltree.join_group(radiobutton_selnode);

    auto content_area = dialog.get_content_area();
    if (also_selection) {
        radiobutton_selection.join_group(radiobutton_selnode);
        content_area->pack_start(radiobutton_selection);
    }
    content_area->pack_start(radiobutton_selnode);
    content_area->pack_start(radiobutton_selnodeandsub);
    content_area->pack_start(radiobutton_alltree);

    auto separator_item_1 = Gtk::HSeparator();
    auto checkbutton_node_name = Gtk::CheckButton(_("Include Node Name"));
    if (last_include_node_name != nullptr) {
        checkbutton_node_name.set_active(*last_include_node_name);
        content_area->pack_start(separator_item_1);
        content_area->pack_start(checkbutton_node_name);
    }
    auto separator_item_2 = Gtk::HSeparator();
    auto checkbutton_index_in_page = Gtk::CheckButton(_("Links Tree in Every Page"));
    if (last_index_in_page != nullptr) {
        checkbutton_index_in_page.set_active(*last_index_in_page);
        content_area->pack_start(separator_item_2);
        content_area->pack_start(checkbutton_index_in_page);
    }
    auto checkbutton_new_node_page = Gtk::CheckButton(_("New Node in New Page"));
    if (last_new_node_page != nullptr) {
        checkbutton_new_node_page.set_active(*last_new_node_page);
        content_area->pack_start(checkbutton_new_node_page);
    }
    auto checkbutton_single_file = Gtk::CheckButton(_("Single File"));
    if (last_single_file != nullptr) {
        checkbutton_single_file.set_active(*last_single_file);
        content_area->pack_start(checkbutton_single_file);
    }
    content_area->show_all();
    const int response = dialog.run();

    if (last_include_node_name != nullptr) *last_include_node_name = checkbutton_node_name.get_active();
    if (last_index_in_page != nullptr) *last_index_in_page = checkbutton_index_in_page.get_active();
    if (last_new_node_page != nullptr) *last_new_node_page = checkbutton_new_node_page.get_active();
    if (last_single_file != nullptr) *last_single_file = checkbutton_single_file.get_active();

    if (response != Gtk::RESPONSE_ACCEPT) return CtExporting::NONESAVE;
    if (radiobutton_selnode.get_active()) return CtExporting::CURRENT_NODE;
    if (radiobutton_selnodeandsub.get_active()) return CtExporting::CURRENT_NODE_AND_SUBNODES;
    if (radiobutton_alltree.get_active()) return CtExporting::ALL_TREE;
    return CtExporting::SELECTED_TEXT;
}
