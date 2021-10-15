/*
 * ct_dialogs_link.cc
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

#include "ct_dialogs.h"
#include "ct_main_win.h"
#include "ct_storage_control.h"

bool CtDialogs::link_handle_dialog(CtMainWin& ctMainWin,
                                   const Glib::ustring& title,
                                   Gtk::TreeIter sel_tree_iter,
                                   CtLinkEntry& link_entries)
{
    if (link_entries.type == "") {
        link_entries.type = CtConst::LINK_TYPE_WEBS;
    }
    CtTreeStore& ctTreestore = ctMainWin.get_tree_store();
    Gtk::Dialog dialog{title,
                       ctMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(700, 500);

    Gtk::HBox hbox_webs;
    Gtk::Image image_webs;
    image_webs.set_from_icon_name("ct_link_website", Gtk::ICON_SIZE_BUTTON);
    Gtk::RadioButton radiobutton_webs{_("To WebSite")};
    Gtk::Entry entry_webs;
    entry_webs.set_text(link_entries.webs);
    hbox_webs.pack_start(image_webs, false, false);
    hbox_webs.pack_start(radiobutton_webs, false, false);
    hbox_webs.pack_start(entry_webs);
    hbox_webs.set_spacing(5);

    Gtk::HBox hbox_file;
    Gtk::Image image_file;
    image_file.set_from_icon_name("ct_file", Gtk::ICON_SIZE_BUTTON);
    Gtk::RadioButton radiobutton_file{_("To File")};
    radiobutton_file.join_group(radiobutton_webs);
    Gtk::Entry entry_file;
    entry_file.set_text(link_entries.file);
    Gtk::Button button_browse_file;
    button_browse_file.set_image_from_icon_name("ct_find", Gtk::ICON_SIZE_BUTTON);
    hbox_file.pack_start(image_file, false, false);
    hbox_file.pack_start(radiobutton_file, false, false);
    hbox_file.pack_start(entry_file);
    hbox_file.pack_start(button_browse_file, false, false);
    hbox_file.set_spacing(5);

    Gtk::HBox hbox_folder;
    Gtk::Image image_folder;
    image_folder.set_from_icon_name("ct_directory", Gtk::ICON_SIZE_BUTTON);
    Gtk::RadioButton radiobutton_folder{_("To Folder")};
    radiobutton_folder.join_group(radiobutton_webs);
    Gtk::Entry entry_folder;
    entry_folder.set_text(link_entries.fold);
    Gtk::Button button_browse_folder;
    button_browse_folder.set_image_from_icon_name("ct_find", Gtk::ICON_SIZE_BUTTON);
    hbox_folder.pack_start(image_folder, false, false);
    hbox_folder.pack_start(radiobutton_folder, false, false);
    hbox_folder.pack_start(entry_folder);
    hbox_folder.pack_start(button_browse_folder, false, false);
    hbox_folder.set_spacing(5);

    Gtk::HBox hbox_node;
    Gtk::Image image_node;
    image_node.set_from_icon_name("cherrytree", Gtk::ICON_SIZE_BUTTON);
    Gtk::RadioButton radiobutton_node{_("To Node")};
    radiobutton_node.join_group(radiobutton_webs);
    hbox_node.pack_start(image_node, false, false);
    hbox_node.pack_start(radiobutton_node);
    hbox_node.set_spacing(5);

    Gtk::HBox hbox_detail;

    Gtk::TreeView treeview_2(ctMainWin.get_tree_store().get_store());
    treeview_2.set_headers_visible(false);
    treeview_2.set_search_column(1);
    Gtk::CellRendererPixbuf renderer_pixbuf_2;
    Gtk::CellRendererText renderer_text_2;
    Gtk::TreeViewColumn column_2;
    treeview_2.append_column("", ctMainWin.get_tree_store().get_columns().rColPixbuf);
    treeview_2.append_column("", ctMainWin.get_tree_store().get_columns().colNodeName);
    Gtk::ScrolledWindow scrolledwindow;
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview_2);

    Gtk::VBox vbox_anchor;
    Gtk::Label label_over;
    Gtk::Label label_below;

    Gtk::HBox hbox_anchor;
    Gtk::Entry entry_anchor;
    entry_anchor.set_text(link_entries.anch);
    Gtk::Button button_browse_anchor;
    button_browse_anchor.set_image_from_icon_name("ct_anchor", Gtk::ICON_SIZE_BUTTON);
    Gtk::Button button_search_anchor;
    button_search_anchor.set_sensitive(not link_entries.anch.empty());
    button_search_anchor.set_image_from_icon_name("ct_find", Gtk::ICON_SIZE_BUTTON);
    hbox_anchor.pack_start(entry_anchor);
    hbox_anchor.pack_start(button_browse_anchor, false, false);
    hbox_anchor.pack_start(button_search_anchor, false, false);

    Gtk::Frame frame_anchor{Glib::ustring("<b>")+_("Anchor Name (optional)")+"</b>"};
    dynamic_cast<Gtk::Label*>(frame_anchor.get_label_widget())->set_use_markup(true);
    frame_anchor.set_shadow_type(Gtk::SHADOW_NONE);
    frame_anchor.add(hbox_anchor);

    vbox_anchor.pack_start(label_over);
    vbox_anchor.pack_start(frame_anchor, false, false);
    vbox_anchor.pack_start(label_below);

    hbox_detail.pack_start(scrolledwindow);
    hbox_detail.pack_start(vbox_anchor, false, false);

    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(hbox_webs, false, false);
    pContentArea->pack_start(hbox_file, false, false);
    pContentArea->pack_start(hbox_folder, false, false);
    pContentArea->pack_start(hbox_node, false, false);
    pContentArea->pack_start(hbox_detail);
    pContentArea->set_spacing(5);

    radiobutton_webs.set_active(link_entries.type == CtConst::LINK_TYPE_WEBS);
    radiobutton_node.set_active(link_entries.type == CtConst::LINK_TYPE_NODE);
    radiobutton_file.set_active(link_entries.type == CtConst::LINK_TYPE_FILE);
    radiobutton_folder.set_active(link_entries.type == CtConst::LINK_TYPE_FOLD);

    bool first_in{true};

    auto link_type_changed_on_dialog = [&](){
        entry_webs.set_sensitive(link_entries.type == CtConst::LINK_TYPE_WEBS);
        hbox_detail.set_sensitive(link_entries.type == CtConst::LINK_TYPE_NODE);
        entry_file.set_sensitive(link_entries.type == CtConst::LINK_TYPE_FILE);
        entry_folder.set_sensitive(link_entries.type == CtConst::LINK_TYPE_FOLD);
        if (link_entries.type == CtConst::LINK_TYPE_WEBS) {
            entry_webs.grab_focus();
        }
        else if (link_entries.type == CtConst::LINK_TYPE_NODE) {
            treeview_2.grab_focus();
            if (first_in) {
                first_in = false;
                std::string exp_colpsd_str = ctTreestore.treeview_get_tree_expanded_collapsed_string(ctMainWin.get_tree_view());
                ctTreestore.treeview_set_tree_expanded_collapsed_string(exp_colpsd_str, treeview_2, ctMainWin.get_ct_config()->nodesBookmExp);
            }
            if (!sel_tree_iter) {
                sel_tree_iter = ctTreestore.get_iter_first();
            }
            Gtk::TreePath sel_path = ctTreestore.get_path(sel_tree_iter);
            treeview_2.expand_to_path(sel_path);
            treeview_2.set_cursor(sel_path);
            treeview_2.scroll_to_row(sel_path);
        }
        else if (link_entries.type == CtConst::LINK_TYPE_FILE) {
            entry_file.grab_focus();
        }
        else {
            entry_folder.grab_focus();
        }
    };

    radiobutton_webs.signal_toggled().connect([&](){
        if (radiobutton_webs.get_active()){
            link_entries.type = CtConst::LINK_TYPE_WEBS;
        }
        link_type_changed_on_dialog();
    });
    entry_webs.signal_activate().connect([&](){
        if (!str::trim(entry_webs.get_text()).empty())
            dialog.response(Gtk::RESPONSE_ACCEPT);
    });
    radiobutton_node.signal_toggled().connect([&](){
        if (radiobutton_node.get_active()) {
            link_entries.type = CtConst::LINK_TYPE_NODE;
        }
        link_type_changed_on_dialog();
    });
    radiobutton_file.signal_toggled().connect([&](){
        if (radiobutton_file.get_active()) {
            link_entries.type = CtConst::LINK_TYPE_FILE;
        }
        link_type_changed_on_dialog();
    });
    entry_file.signal_activate().connect([&](){
        if (!str::trim(entry_file.get_text()).empty())
            dialog.response(Gtk::RESPONSE_ACCEPT);
    });
    radiobutton_folder.signal_toggled().connect([&](){
        if (radiobutton_folder.get_active()) {
            link_entries.type = CtConst::LINK_TYPE_FOLD;
        }
        link_type_changed_on_dialog();
    });
    entry_folder.signal_activate().connect([&](){
        if (!str::trim(entry_folder.get_text()).empty())
            dialog.response(Gtk::RESPONSE_ACCEPT);
    });
    button_browse_file.signal_clicked().connect([&](){
        CtDialogs::FileSelectArgs args{&dialog};
        args.curr_folder=ctMainWin.get_ct_config()->pickDirFile;
        std::string filepath = file_select_dialog(args);
        if (filepath.empty()) {
            return;
        }
        ctMainWin.get_ct_config()->pickDirFile = Glib::path_get_dirname(filepath);
        if (ctMainWin.get_ct_config()->linksRelative) {
            filepath = fs::relative(filepath, ctMainWin.get_ct_storage()->get_file_dir()).string();
        }
        entry_file.set_text(filepath);
    });
    button_browse_folder.signal_clicked().connect([&](){
        std::string filepath = CtDialogs::folder_select_dialog(ctMainWin.get_ct_config()->pickDirFile, &dialog);
        if (filepath.empty()) {
            return;
        }
        ctMainWin.get_ct_config()->pickDirFile = filepath;
        if (ctMainWin.get_ct_config()->linksRelative) {
            filepath = fs::relative(filepath, ctMainWin.get_ct_storage()->get_file_dir()).string();
        }
        entry_folder.set_text(filepath);
    });
    button_browse_anchor.signal_clicked().connect([&](){
        if (!sel_tree_iter) {
            CtDialogs::warning_dialog(_("No Node is Selected"), dialog);
            return;
        }
        CtTreeIter ctTreeIter = ctTreestore.to_ct_tree_iter(sel_tree_iter);
        std::list<Glib::ustring> anchors_list;
        for (CtAnchoredWidget* pAnchoredWidget : ctTreeIter.get_anchored_widgets_fast()) {
            if (CtAnchWidgType::ImageAnchor == pAnchoredWidget->get_type()) {
                anchors_list.push_back(dynamic_cast<CtImageAnchor*>(pAnchoredWidget)->get_anchor_name());
            }
        }
        if (anchors_list.empty()) {
            info_dialog(_("There are No Anchors in the Selected Node"), dialog);
        }
        else {
            Glib::RefPtr<CtChooseDialogListStore> rItemStore = CtChooseDialogListStore::create();
            for (const Glib::ustring& anchName : anchors_list) {
                rItemStore->add_row("", "", anchName);
            }
            Gtk::TreeIter res = CtDialogs::choose_item_dialog(dialog, _("Choose Existing Anchor"), rItemStore, _("Anchor Name"));
            if (res) {
                Glib::ustring anchName = res->get_value(rItemStore->columns.desc);
                entry_anchor.set_text(anchName);
            }
        }
    });
    entry_anchor.signal_changed().connect([&](){
        button_search_anchor.set_sensitive(not entry_anchor.get_text().empty());
    });
    entry_anchor.signal_key_press_event().connect([&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            button_search_anchor.clicked();
            return true;
        }
        return false;
    }, false);
    Gtk::TreeIter lastAnchorSearch;
    button_search_anchor.signal_clicked().connect([&](){
        bool pastPrevSearch{not static_cast<bool>(lastAnchorSearch)};
        const auto anchorName = entry_anchor.get_text();
        bool foundIt{false};
        for (unsigned i = 0; i < 2; ++i) {
            ctTreestore.get_store()->foreach([&](const Gtk::TreePath& /*treePath*/, const Gtk::TreeIter& treeIter)->bool{
                if (not pastPrevSearch) {
                    if (treeIter == lastAnchorSearch) {
                        pastPrevSearch = true;
                    }
                }
                else {
                    CtTreeIter ctTreeIter = ctTreestore.to_ct_tree_iter(treeIter);
                    for (CtAnchoredWidget* pAnchoredWidget : ctTreeIter.get_anchored_widgets_fast()) {
                        if (CtAnchWidgType::ImageAnchor == pAnchoredWidget->get_type()) {
                            if (anchorName == dynamic_cast<CtImageAnchor*>(pAnchoredWidget)->get_anchor_name()) {
                                Gtk::TreePath sel_path = ctTreestore.get_path(treeIter);
                                treeview_2.expand_to_path(sel_path);
                                treeview_2.set_cursor(sel_path);
                                treeview_2.scroll_to_row(sel_path);
                                lastAnchorSearch = treeIter;
                                foundIt = true;
                                return true; /* we're done */
                            }
                        }
                    }
                }
                return false; /* false for continue */
            });
            if (foundIt or not static_cast<bool>(lastAnchorSearch)) {
                break;
            }
            lastAnchorSearch = Gtk::TreeIter{};
        }
        if (not foundIt) {
            CtDialogs::info_dialog(str::format(_("The pattern '%s' was not found"), anchorName), dialog);
        }
    });
    treeview_2.signal_event_after().connect([&](GdkEvent* event){
        if ( (event->type != GDK_BUTTON_PRESS) &&
             (event->type != GDK_2BUTTON_PRESS) &&
             (event->type != GDK_KEY_PRESS) )
        {
            return;
        }
        sel_tree_iter = treeview_2.get_selection()->get_selected();
        if ( (event->type == GDK_BUTTON_PRESS) &&
             (event->button.button == 2) )
        {
            Gtk::TreePath path_at_click;
            if (treeview_2.get_path_at_pos((int)event->button.x, (int)event->button.y, path_at_click)) {
                if (treeview_2.row_expanded(path_at_click)) {
                    treeview_2.collapse_row(path_at_click);
                }
                else {
                    treeview_2.expand_row(path_at_click, true);
                }
            }
        }
        else if ( (event->type == GDK_2BUTTON_PRESS) &&
                  (event->button.button == 1) &&
                  sel_tree_iter )
        {
            Gtk::TreePath path = ctTreestore.get_path(sel_tree_iter);
            if (treeview_2.row_expanded(path))
                treeview_2.collapse_row(path);
            else
                treeview_2.expand_row(path, true);
        }
        else if (event->type == GDK_KEY_PRESS && sel_tree_iter) {
            Gtk::TreePath path = ctTreestore.get_path(sel_tree_iter);
            if (event->key.keyval == GDK_KEY_Left)
                treeview_2.collapse_row(path);
            else if (event->key.keyval == GDK_KEY_Right)
                treeview_2.expand_row(path, false);
        }
    });
    dialog.signal_key_press_event().connect([&](GdkEventKey* event) {
        if (event->keyval == GDK_KEY_Tab) {
            if (link_entries.type == CtConst::LINK_TYPE_WEBS) radiobutton_file.set_active(true);
            else if (link_entries.type == CtConst::LINK_TYPE_FILE) radiobutton_folder.set_active(true);
            else if (link_entries.type == CtConst::LINK_TYPE_FOLD) radiobutton_node.set_active(true);
            else radiobutton_webs.set_active(true);
            return true;
        }
        return false;
    }, false);

    pContentArea->show_all();
    link_type_changed_on_dialog();

    if (dialog.run() != GTK_RESPONSE_ACCEPT) {
        return false;
    }

    link_entries.webs = str::trim(entry_webs.get_text());
    link_entries.file = str::trim(entry_file.get_text());
    link_entries.fold = str::trim(entry_folder.get_text());
    link_entries.anch = str::trim(entry_anchor.get_text());
    link_entries.node_id = ctTreestore.to_ct_tree_iter(sel_tree_iter).get_node_id();
    return true;
}
