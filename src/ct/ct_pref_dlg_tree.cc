/*
 * ct_pref_dlg_tree.cc
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

Gtk::Widget* CtPrefDlg::build_tab_tree()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_nodes_icons = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_node_icon_cherry = Gtk::manage(new Gtk::RadioButton(_("Use Different Cherries per Level")));
    Gtk::RadioButton* radiobutton_node_icon_custom = Gtk::manage(new Gtk::RadioButton(_("Use Selected Icon")));
    radiobutton_node_icon_custom->join_group(*radiobutton_node_icon_cherry);
    Gtk::RadioButton* radiobutton_node_icon_none = Gtk::manage(new Gtk::RadioButton(_("No Icon")));
    radiobutton_node_icon_none->join_group(*radiobutton_node_icon_cherry);
    Gtk::CheckButton* checkbutton_aux_icon_hide = Gtk::manage(new Gtk::CheckButton(_("Hide Right Side Auxiliary Icon")));
    checkbutton_aux_icon_hide->set_active(pConfig->auxIconHide);

    Gtk::Button* c_icon_button = Gtk::manage(new Gtk::Button());
    c_icon_button->set_image(*_pCtMainWin->new_image_from_stock(CtConst::NODE_CUSTOM_ICONS.at(pConfig->defaultIconText), Gtk::ICON_SIZE_BUTTON));
    Gtk::HBox* c_icon_hbox = Gtk::manage(new Gtk::HBox());
    c_icon_hbox->set_spacing(2);
    c_icon_hbox->pack_start(*radiobutton_node_icon_custom, false, false);
    c_icon_hbox->pack_start(*c_icon_button, false, false);

    vbox_nodes_icons->pack_start(*radiobutton_node_icon_cherry, false, false);
    vbox_nodes_icons->pack_start(*c_icon_hbox, false, false);
    vbox_nodes_icons->pack_start(*radiobutton_node_icon_none, false, false);
    vbox_nodes_icons->pack_start(*checkbutton_aux_icon_hide, false, false);
    Gtk::Frame* frame_nodes_icons = new_managed_frame_with_align(_("Default Text Nodes Icons"), vbox_nodes_icons);

    radiobutton_node_icon_cherry->set_active(pConfig->nodesIcons == "c");
    radiobutton_node_icon_custom->set_active(pConfig->nodesIcons == "b");
    radiobutton_node_icon_none->set_active(pConfig->nodesIcons == "n");

    Gtk::VBox* vbox_nodes_startup = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_nodes_startup_restore = Gtk::manage(new Gtk::RadioButton(_("Restore Expanded/Collapsed Status")));
    Gtk::RadioButton* radiobutton_nodes_startup_expand = Gtk::manage(new Gtk::RadioButton(_("Expand all Nodes")));
    radiobutton_nodes_startup_expand->join_group(*radiobutton_nodes_startup_restore);
    Gtk::RadioButton* radiobutton_nodes_startup_collapse = Gtk::manage(new Gtk::RadioButton(_("Collapse all Nodes")));
    radiobutton_nodes_startup_collapse->join_group(*radiobutton_nodes_startup_restore);
    Gtk::CheckButton* checkbutton_nodes_bookm_exp = Gtk::manage(new Gtk::CheckButton(_("Nodes in Bookmarks Always Visible")));
    checkbutton_nodes_bookm_exp->set_active(pConfig->nodesBookmExp);
    checkbutton_nodes_bookm_exp->set_sensitive(pConfig->restoreExpColl != CtRestoreExpColl::ALL_EXP);

    vbox_nodes_startup->pack_start(*radiobutton_nodes_startup_restore, false, false);
    vbox_nodes_startup->pack_start(*radiobutton_nodes_startup_expand, false, false);
    vbox_nodes_startup->pack_start(*radiobutton_nodes_startup_collapse, false, false);
    vbox_nodes_startup->pack_start(*checkbutton_nodes_bookm_exp, false, false);
    Gtk::Frame* frame_nodes_startup = new_managed_frame_with_align(_("Nodes Status at Startup"), vbox_nodes_startup);

    radiobutton_nodes_startup_restore->set_active(pConfig->restoreExpColl == CtRestoreExpColl::FROM_STR);
    radiobutton_nodes_startup_expand->set_active(pConfig->restoreExpColl == CtRestoreExpColl::ALL_EXP);
    radiobutton_nodes_startup_collapse->set_active(pConfig->restoreExpColl == CtRestoreExpColl::ALL_COLL);

    Gtk::VBox* vbox_misc_tree = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_tree_nodes_names_width = Gtk::manage(new Gtk::HBox());
    hbox_tree_nodes_names_width->set_spacing(4);
    Gtk::CheckButton* checkbutton_tree_nodes_names_wrap_ena = Gtk::manage(new Gtk::CheckButton(_("Tree Nodes Names Wrapping Width")));
    checkbutton_tree_nodes_names_wrap_ena->set_active(pConfig->cherryWrapEnabled);
    Glib::RefPtr<Gtk::Adjustment> adj_tree_nodes_names_width = Gtk::Adjustment::create(pConfig->cherryWrapWidth, 10, 10000, 1);
    Gtk::SpinButton* spinbutton_tree_nodes_names_width = Gtk::manage(new Gtk::SpinButton(adj_tree_nodes_names_width));
    spinbutton_tree_nodes_names_width->set_value(pConfig->cherryWrapWidth);
    spinbutton_tree_nodes_names_width->set_sensitive(pConfig->cherryWrapEnabled);
    hbox_tree_nodes_names_width->pack_start(*checkbutton_tree_nodes_names_wrap_ena, false, false);
    hbox_tree_nodes_names_width->pack_start(*spinbutton_tree_nodes_names_width, false, false);
    Gtk::CheckButton* checkbutton_tree_right_side = Gtk::manage(new Gtk::CheckButton(_("Display Tree on the Right Side")));
    checkbutton_tree_right_side->set_active(pConfig->treeRightSide);
    Gtk::CheckButton* checkbutton_tree_click_focus_text = Gtk::manage(new Gtk::CheckButton(_("Move Focus to Text at Mouse Click")));
    checkbutton_tree_click_focus_text->set_active(pConfig->treeClickFocusText);
    Gtk::CheckButton* checkbutton_tree_click_expand = Gtk::manage(new Gtk::CheckButton(_("Expand Node at Mouse Click")));
    checkbutton_tree_click_expand->set_active(pConfig->treeClickExpand);
    Gtk::HBox* hbox_nodes_on_node_name_header = Gtk::manage(new Gtk::HBox());
    hbox_nodes_on_node_name_header->set_spacing(4);
    Gtk::Label* label_nodes_on_node_name_header = Gtk::manage(new Gtk::Label(_("Last Visited Nodes on Node Name Header")));
    Glib::RefPtr<Gtk::Adjustment> adj_nodes_on_node_name_header = Gtk::Adjustment::create(pConfig->nodesOnNodeNameHeader, 0, 100, 1);
    Gtk::SpinButton* spinbutton_nodes_on_node_name_header = Gtk::manage(new Gtk::SpinButton(adj_nodes_on_node_name_header));
    spinbutton_nodes_on_node_name_header->set_value(pConfig->nodesOnNodeNameHeader);
    hbox_nodes_on_node_name_header->pack_start(*label_nodes_on_node_name_header, false, false);
    hbox_nodes_on_node_name_header->pack_start(*spinbutton_nodes_on_node_name_header, false, false);

    vbox_misc_tree->pack_start(*hbox_tree_nodes_names_width, false, false);
    vbox_misc_tree->pack_start(*checkbutton_tree_right_side, false, false);
    vbox_misc_tree->pack_start(*checkbutton_tree_click_focus_text, false, false);
    vbox_misc_tree->pack_start(*checkbutton_tree_click_expand, false, false);
    vbox_misc_tree->pack_start(*hbox_nodes_on_node_name_header, false, false);
    Gtk::Frame* frame_misc_tree = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_tree);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_nodes_icons, false, false);
    pMainBox->pack_start(*frame_nodes_startup, false, false);
    pMainBox->pack_start(*frame_misc_tree, false, false);

    checkbutton_tree_nodes_names_wrap_ena->signal_toggled().connect([this,
                                                                     pConfig,
                                                                     checkbutton_tree_nodes_names_wrap_ena,
                                                                     spinbutton_tree_nodes_names_width](){
        pConfig->cherryWrapEnabled = checkbutton_tree_nodes_names_wrap_ena->get_active();
        spinbutton_tree_nodes_names_width->set_sensitive(pConfig->cherryWrapEnabled);
        need_restart(RESTART_REASON::TREE_NODE_WRAP);
    });
    spinbutton_tree_nodes_names_width->signal_value_changed().connect([this, pConfig, spinbutton_tree_nodes_names_width](){
        pConfig->cherryWrapWidth = spinbutton_tree_nodes_names_width->get_value_as_int();
        need_restart(RESTART_REASON::TREE_NODE_WRAP);
    });
    checkbutton_tree_right_side->signal_toggled().connect([this, pConfig, checkbutton_tree_right_side](){
        pConfig->treeRightSide = checkbutton_tree_right_side->get_active();
        apply_for_each_window([](CtMainWin* win) { win->config_switch_tree_side(); });
    });
    checkbutton_tree_click_focus_text->signal_toggled().connect([pConfig, checkbutton_tree_click_focus_text](){
        pConfig->treeClickFocusText = checkbutton_tree_click_focus_text->get_active();
    });
    checkbutton_tree_click_expand->signal_toggled().connect([pConfig, checkbutton_tree_click_expand](){
        pConfig->treeClickExpand = checkbutton_tree_click_expand->get_active();
    });
    spinbutton_nodes_on_node_name_header->signal_value_changed().connect([this, pConfig, spinbutton_nodes_on_node_name_header](){
        pConfig->nodesOnNodeNameHeader = spinbutton_nodes_on_node_name_header->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) { win->window_header_update(); });
    });

    radiobutton_node_icon_cherry->signal_toggled().connect([this, pConfig, radiobutton_node_icon_cherry](){
        if (!radiobutton_node_icon_cherry->get_active()) return;
        pConfig->nodesIcons = "c";
        apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false); });
    });
    radiobutton_node_icon_custom->signal_toggled().connect([this, pConfig, radiobutton_node_icon_custom](){
        if (!radiobutton_node_icon_custom->get_active()) return;
        pConfig->nodesIcons = "b";
        apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false); });
    });
    radiobutton_node_icon_none->signal_toggled().connect([this, pConfig, radiobutton_node_icon_none](){
        if (!radiobutton_node_icon_none->get_active()) return;
        pConfig->nodesIcons = "n";
        apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false); });
    });
    c_icon_button->signal_clicked().connect([this, pConfig, c_icon_button](){
        auto itemStore = CtChooseDialogListStore::create();
        for (int i = 1 /* skip 0 */; i < (int)CtConst::NODE_CUSTOM_ICONS.size(); ++i)
            itemStore->add_row(CtConst::NODE_CUSTOM_ICONS[i], std::to_string(i), "");
        auto res = CtDialogs::choose_item_dialog(*this, _("Select Node Icon"), itemStore);
        if (res) {
            pConfig->defaultIconText = std::stoi(res->get_value(itemStore->columns.key));
            c_icon_button->set_image(*_pCtMainWin->new_image_from_stock(res->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
            apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false);});
        }
    });
    radiobutton_nodes_startup_restore->signal_toggled().connect([pConfig, radiobutton_nodes_startup_restore, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_restore->get_active()) return;
        pConfig->restoreExpColl = CtRestoreExpColl::FROM_STR;
        checkbutton_nodes_bookm_exp->set_sensitive(true);
    });
    radiobutton_nodes_startup_expand->signal_toggled().connect([pConfig, radiobutton_nodes_startup_expand, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_expand->get_active()) return;
        pConfig->restoreExpColl = CtRestoreExpColl::ALL_EXP;
        checkbutton_nodes_bookm_exp->set_sensitive(false);
    });
    radiobutton_nodes_startup_collapse->signal_toggled().connect([pConfig, radiobutton_nodes_startup_collapse, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_collapse->get_active()) return;
        pConfig->restoreExpColl = CtRestoreExpColl::ALL_COLL;
        checkbutton_nodes_bookm_exp->set_sensitive(true);
    });
    checkbutton_nodes_bookm_exp->signal_toggled().connect([pConfig, checkbutton_nodes_bookm_exp](){
        pConfig->nodesBookmExp = checkbutton_nodes_bookm_exp->get_active();
    });
    checkbutton_aux_icon_hide->signal_toggled().connect([this, pConfig, checkbutton_aux_icon_hide](){
        pConfig->auxIconHide = checkbutton_aux_icon_hide->get_active();
        apply_for_each_window([pConfig](CtMainWin* win) { win->get_tree_view().get_column(CtTreeView::AUX_ICON_COL_NUM)->set_visible(!pConfig->auxIconHide); });
    });

    return pMainBox;
}
