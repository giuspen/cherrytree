/*
 * ct_dialogs.cc
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
#include "ct_treestore.h"
#include "ct_main_win.h"

void CtDialogs::bookmarks_handle_dialog(CtMainWin* pCtMainWin)
{
    CtTreeStore& ctTreestore = pCtMainWin->get_tree_store();
    const std::list<gint64>& bookmarks = ctTreestore.bookmarks_get();

    Gtk::Dialog dialog(_("Handle the Bookmarks List"),
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(500, 400);

    Glib::RefPtr<CtChooseDialogTreeStore> rModel = CtChooseDialogTreeStore::create();
    for (const gint64& node_id : bookmarks)
    {
        rModel->add_row("ct_pin", "", ctTreestore.get_node_name_from_node_id(node_id), node_id);
    }

    Gtk::TreeView treeview(rModel);
    treeview.set_headers_visible(false);
    treeview.set_reorderable(true);
    Gtk::CellRendererPixbuf pixbuf_renderer;
    int col_num = treeview.append_column("", pixbuf_renderer) - 1;
    treeview.get_column(col_num)->add_attribute(pixbuf_renderer, "icon-name", rModel->columns.stock_id);
    treeview.append_column("", rModel->columns.desc);
    Gtk::ScrolledWindow scrolledwindow;
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview);
    Gtk::Box* pContentArea = dialog.get_content_area();

    Gtk::Button button_move_up;
    button_move_up.set_image_from_icon_name("ct_go-up", Gtk::ICON_SIZE_DND);
    button_move_up.set_tooltip_text(_("Move the Selected Bookmark Up"));
    Gtk::Button button_move_down;
    button_move_down.set_image_from_icon_name("ct_go-down", Gtk::ICON_SIZE_DND);
    button_move_down.set_tooltip_text(_("Move the Selected Bookmark Down"));
    Gtk::Button button_delete;
    button_delete.set_image_from_icon_name("ct_remove", Gtk::ICON_SIZE_DND);
    button_delete.set_tooltip_text(_("Remove the Selected Bookmark"));
    Gtk::Button button_sort_desc;
    button_sort_desc.set_image_from_icon_name("ct_sort-desc", Gtk::ICON_SIZE_DND);
    button_sort_desc.set_tooltip_text(_("Sort the Bookmarks Descending"));
    Gtk::Button button_sort_asc;
    button_sort_asc.set_image_from_icon_name("ct_sort-asc", Gtk::ICON_SIZE_DND);
    button_sort_asc.set_tooltip_text(_("Sort the Bookmarks Ascending"));
    Gtk::Label label1;
    Gtk::Label label2;
    Gtk::HBox hbox;
    Gtk::VBox vbox;
    vbox.set_spacing(1);
    vbox.pack_start(button_move_up, false, false);
    vbox.pack_start(button_move_down, false, false);
    vbox.pack_start(button_delete, false, false);
    vbox.pack_start(label1, true, true);
    vbox.pack_start(button_sort_desc, false, false);
    vbox.pack_start(button_sort_asc, false, false);
    vbox.pack_start(label2, true, false);
    hbox.pack_start(scrolledwindow, true, true);
    hbox.pack_start(vbox, false, false);
    pContentArea->pack_start(hbox);
    pContentArea->show_all();

    treeview.signal_key_press_event().connect([&rModel, &treeview](GdkEventKey* key)->bool
    {
        if (key->keyval == GDK_KEY_Delete)
        {
            Gtk::TreeIter tree_iter = treeview.get_selection()->get_selected();
            if (tree_iter)
            {
                rModel->erase(tree_iter);
            }
            return true; // stop event
        }
        return false; // propagate event
    });
    treeview.signal_button_press_event().connect([&treeview, &rModel, &pCtMainWin, &ctTreestore](GdkEventButton* event)->bool
    {
        if (event->button != 1 || event->type != GDK_2BUTTON_PRESS)
        {
            return false; // propagate event
        }
        Gtk::TreePath clicked_path;
        if (false == treeview.get_path_at_pos((int)event->x, (int)event->y, clicked_path))
        {
            return false; // propagate event
        }
        Gtk::TreeIter clicked_iter = rModel->get_iter(clicked_path);
        gint64 node_id = clicked_iter->get_value(rModel->columns.node_id);
        Gtk::TreeIter tree_iter = ctTreestore.get_node_from_node_id(node_id);
        pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
        return true; // stop event
    });
    button_move_up.signal_clicked().connect([&treeview, &rModel]()
    {
        Gtk::TreeIter curr_iter = treeview.get_selection()->get_selected();
        Gtk::TreeIter prev_iter = --treeview.get_selection()->get_selected();
        if (curr_iter && prev_iter)
        {
            rModel->iter_swap(curr_iter, prev_iter);
        }
    });
    button_move_down.signal_clicked().connect([&treeview, &rModel]()
    {
        Gtk::TreeIter curr_iter = treeview.get_selection()->get_selected();
        Gtk::TreeIter next_iter = ++treeview.get_selection()->get_selected();
        if (curr_iter && next_iter)
        {
            rModel->iter_swap(curr_iter, next_iter);
        }
    });
    button_delete.signal_clicked().connect([&treeview, &rModel]()
    {
        Gtk::TreeIter tree_iter = treeview.get_selection()->get_selected();
        if (tree_iter)
        {
            rModel->erase(tree_iter);
        }
    });
    button_sort_asc.signal_clicked().connect([&rModel]()
    {
        auto need_swap = [&rModel](Gtk::TreeIter& l, Gtk::TreeIter& r)
        {
            int cmp = l->get_value(rModel->columns.desc).compare(r->get_value(rModel->columns.desc));
            return (cmp > 0);
        };
        CtMiscUtil::node_siblings_sort_iteration(rModel, rModel->children(), need_swap);
    });
    button_sort_desc.signal_clicked().connect([&rModel]()
    {
        auto need_swap = [&rModel](Gtk::TreeIter& l, Gtk::TreeIter& r)
        {
            int cmp = l->get_value(rModel->columns.desc).compare(r->get_value(rModel->columns.desc));
            return (cmp < 0);
        };
        CtMiscUtil::node_siblings_sort_iteration(rModel, rModel->children(), need_swap);
    });

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
    {
        return;
    }

    std::set<gint64> temp_bookmarks;
    std::list<gint64> temp_bookmarks_order;
    rModel->foreach_iter([&temp_bookmarks, &temp_bookmarks_order, &rModel](const Gtk::TreeIter& iter)
    {
        gint64 node_id = iter->get_value(rModel->columns.node_id);
        temp_bookmarks.insert(node_id);
        temp_bookmarks_order.push_back(node_id);
        return false; /* to continue */
    });

    std::list<gint64> removed_bookmarks;
    for (const gint64& node_id : bookmarks)
    {
        if (0 == temp_bookmarks.count(node_id))
        {
            removed_bookmarks.push_back(node_id);
        }
    }

    ctTreestore.bookmarks_set(temp_bookmarks_order);
    gint64 curr_node_id = pCtMainWin->curr_tree_iter().get_node_id();
    for (gint64& node_id: removed_bookmarks)
    {
        Gtk::TreeIter tree_iter = ctTreestore.get_node_from_node_id(node_id);
        if (tree_iter)
        {
            ctTreestore.update_node_aux_icon(tree_iter);
            if (curr_node_id == node_id)
            {
                pCtMainWin->menu_update_bookmark_menu_item(false);
            }
        }
    }

    pCtMainWin->menu_set_bookmark_menu_items();
    ctTreestore.pending_edit_db_bookmarks();
    pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::book);
}

// Choose the CherryTree data storage type (xml or db) and protection
bool CtDialogs::choose_data_storage_dialog(storage_select_args& args)
{
    Gtk::Dialog dialog(_("Choose Storage Type"),
                       *args.pParentWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(350, -1);
    dialog.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);

    Glib::ustring labelPrefixSQLite{"SQLite, "};
    Glib::ustring labelPrefixXML{"XML, "};
    Gtk::RadioButton radiobutton_sqlite_not_protected(labelPrefixSQLite + _("Not Protected") + " (.ctb)");
    Gtk::RadioButton::Group rbGroup = radiobutton_sqlite_not_protected.get_group();
    Gtk::RadioButton radiobutton_sqlite_pass_protected(rbGroup, labelPrefixSQLite + _("Password Protected") + " (.ctx)");
    Gtk::RadioButton radiobutton_xml_not_protected(rbGroup, labelPrefixXML + _("Not Protected") + " (.ctd)");
    Gtk::RadioButton radiobutton_xml_pass_protected(rbGroup, labelPrefixXML + _("Password Protected") + " (.ctz)");

    Gtk::VBox type_vbox;
    type_vbox.pack_start(radiobutton_sqlite_not_protected);
    type_vbox.pack_start(radiobutton_sqlite_pass_protected);
    type_vbox.pack_start(radiobutton_xml_not_protected);
    type_vbox.pack_start(radiobutton_xml_pass_protected);

    Gtk::Frame type_frame(Glib::ustring("<b>")+_("Storage Type")+"</b>");
    dynamic_cast<Gtk::Label*>(type_frame.get_label_widget())->set_use_markup(true);
    type_frame.set_shadow_type(Gtk::SHADOW_NONE);
    type_frame.add(type_vbox);

    Gtk::Entry entry_passw_1;
    entry_passw_1.set_visibility(false);
    Gtk::Entry entry_passw_2;
    entry_passw_2.set_visibility(false);
    Gtk::Label label_passwd(_("CT saves the document in an encrypted 7zip archive. When viewing or editing the document, CT extracts the encrypted archive to a temporary folder, and works on the unencrypted copy. When closing, the unencrypted copy is deleted from the temporary directory. Note that in the case of application or system crash, the unencrypted document will remain in the temporary folder."));
    label_passwd.set_width_chars(70);
    label_passwd.set_line_wrap(true);
    Gtk::VBox vbox_passw;
    vbox_passw.pack_start(entry_passw_1);
    vbox_passw.pack_start(entry_passw_2);
    vbox_passw.pack_start(label_passwd);

    Gtk::Frame passw_frame(Glib::ustring("<b>")+_("Enter the New Password Twice")+"</b>");
    dynamic_cast<Gtk::Label*>(passw_frame.get_label_widget())->set_use_markup(true);
    passw_frame.set_shadow_type(Gtk::SHADOW_NONE);
    passw_frame.add(vbox_passw);

    if (args.ctDocEncrypt == CtDocEncrypt::False)
    {
        passw_frame.set_sensitive(false);
        if (args.ctDocType == CtDocType::SQLite)
        {
            radiobutton_sqlite_not_protected.set_active(true);
        }
        else if (args.ctDocType == CtDocType::XML)
        {
            radiobutton_xml_not_protected.set_active(true);
        }
    }
    else if (args.ctDocEncrypt == CtDocEncrypt::True)
    {
        passw_frame.set_sensitive(true);
        if (args.ctDocType == CtDocType::SQLite)
        {
            radiobutton_sqlite_pass_protected.set_active(true);
        }
        else
        {
            radiobutton_xml_pass_protected.set_active(true);
        }
    }
    else {
        radiobutton_sqlite_not_protected.set_active(true);
        passw_frame.set_sensitive(false);
    }

    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->set_spacing(5);
    pContentArea->pack_start(type_frame);
    pContentArea->pack_start(passw_frame);
    pContentArea->show_all();

    auto on_radiobutton_savetype_toggled = [&]()
    {
        if ( radiobutton_sqlite_pass_protected.get_active() ||
             radiobutton_xml_pass_protected.get_active() )
        {
            passw_frame.set_sensitive(true);
            entry_passw_1.grab_focus();
        }
        else
        {
            passw_frame.set_sensitive(false);
        }
    };
    auto on_key_press_edit_data_storage_type_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->clicked();
            return true;
        }
        return false;
    };
    radiobutton_sqlite_not_protected.signal_toggled().connect(on_radiobutton_savetype_toggled);
    radiobutton_sqlite_pass_protected.signal_toggled().connect(on_radiobutton_savetype_toggled);
    radiobutton_xml_not_protected.signal_toggled().connect(on_radiobutton_savetype_toggled);
    dialog.signal_key_press_event().connect(on_key_press_edit_data_storage_type_dialog, false/*call me before other*/);

    const int response = dialog.run();
    dialog.hide();

    bool retVal{Gtk::RESPONSE_ACCEPT == response};
    if (retVal)
    {
        args.ctDocType = (radiobutton_xml_not_protected.get_active() || radiobutton_xml_pass_protected.get_active() ?
                         CtDocType::XML : CtDocType::SQLite);
        args.ctDocEncrypt = (radiobutton_sqlite_pass_protected.get_active() || radiobutton_xml_pass_protected.get_active() ?
                            CtDocEncrypt::True : CtDocEncrypt::False);
        if (CtDocEncrypt::True == args.ctDocEncrypt)
        {
            args.password = entry_passw_1.get_text();
            if (args.password.empty())
            {
                error_dialog(_("The Password Fields Must be Filled"), *args.pParentWin);
                retVal = false;
            }
            else if (args.password != entry_passw_2.get_text())
            {
                error_dialog(_("The Two Inserted Passwords Do Not Match"), *args.pParentWin);
                retVal = false;
            }
        }
    }
    return retVal;
}

CtYesNoCancel CtDialogs::exit_save_dialog(Gtk::Window& parent)
{
    Gtk::Dialog dialog = Gtk::Dialog(_("Warning"),
                                     parent,
                                     Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::DISCARD, Gtk::RESPONSE_NO);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_YES);
    dialog.set_default_response(Gtk::RESPONSE_YES);
    dialog.set_default_size(350, 150);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    Gtk::Image image;
    image.set_from_icon_name("ct_warning", Gtk::ICON_SIZE_DIALOG);
    Gtk::Label label(Glib::ustring("<b>")+_("The Current Document was Updated.")+"</b>\n\n<b>"+_("Do you want to Save the Changes?")+"</b>");
    label.set_use_markup(true);
    Gtk::HBox hbox;
    hbox.pack_start(image);
    hbox.pack_start(label);
    hbox.set_spacing(5);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(hbox);
    auto on_key_press_dialog = [&](GdkEventKey* pEventKey)->bool{
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_YES));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        if (GDK_KEY_Escape == pEventKey->keyval) {
            Gtk::Button* pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_CANCEL));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);
    pContentArea->show_all();
    const int response = dialog.run();
    dialog.hide();
    if (Gtk::RESPONSE_YES == response) {
        return CtYesNoCancel::Yes;
    }
    if (Gtk::RESPONSE_NO == response) {
        return CtYesNoCancel::No;
    }
    return CtYesNoCancel::Cancel;
}

// Application About Dialog
void CtDialogs::dialog_about(Gtk::Window& parent, Glib::RefPtr<Gdk::Pixbuf> icon)
{
    auto dialog = Gtk::AboutDialog();
    dialog.set_program_name("CherryTree");
    dialog.set_version(CtConst::CT_VERSION);
    dialog.set_copyright("Copyright © 2009-2021\n"
                         "Giuseppe Penone <giuspen@gmail.com>\n"
                         "Evgenii Gurianov <https://github.com/txe>");
    dialog.set_comments(_("A Hierarchical Note Taking Application, featuring Rich Text and Syntax Highlighting"));
    dialog.set_license(_(R"STR(
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301, USA.
)STR"));
    dialog.set_website("https://www.giuspen.com/cherrytree/");
    dialog.set_authors({"Giuseppe Penone <giuspen@gmail.com>", "Evgenii Gurianov <https://github.com/txe>"});
    dialog.set_artists({"Ugo Yak <https://www.instagram.com/ugoyak.art/>", "OCAL <http://www.openclipart.org/>", "Zeltak <zeltak@gmail.com>", "Angelo Penone <angelo.penone@gmail.com>"});
    dialog.set_translator_credits(Glib::ustring{} +
 _("Armenian")+" (hy) Seda Stamboltsyan <sedastam@yandex.com>"+CtConst::CHAR_NEWLINE+
 _("Bulgarian")+" (bg) Iliya Nikolaev <iliya.nikolaev@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Chinese Simplified")+" (zh_CN) Channing Wong <channing.wong@qq.com>"+CtConst::CHAR_NEWLINE+
 _("Czech")+" (cs) Pavel Fric <fripohled@blogspot.com>"+CtConst::CHAR_NEWLINE+
 _("Dutch")+" (nl) Luuk Geurts, Patrick Vijgeboom <pj.vijgeboom@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Finnish")+" (fi) Henri Kaustinen <hendrix.ks81@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("French")+" (fr) Klaus Becker <colonius@free.fr>"+CtConst::CHAR_NEWLINE+
 _("German")+" (de) Matthias Hoffmann <MHoffi@web.de>"+CtConst::CHAR_NEWLINE+
 _("Greek")+" (el) Delphina <delphina.2009@yahoo.gr>"+CtConst::CHAR_NEWLINE+
 _("Italian")+" (it) Vincenzo Reale <smart2128@baslug.org>"+CtConst::CHAR_NEWLINE+
 _("Japanese")+" (ja) Piyo <py2@live.jp>"+CtConst::CHAR_NEWLINE+
 _("Kazakh")+" (kk_KZ) Viktor Polyanskiy <camilot55@yandex.ru>"+CtConst::CHAR_NEWLINE+
 _("Korean")+" (ko) Sean Lee <icarusean@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Lithuanian")+" (lt) Zygis <zygimantus@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Polish")+" (pl) Marcin Swierczynski <orneo1212@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Portuguese Brazil")+" (pt_BR) Vinicius Schmidt <viniciussm@rocketmail.com>"+CtConst::CHAR_NEWLINE+
 _("Russian")+" (ru) Viktor Polyanskiy <camilot55@yandex.ru>"+CtConst::CHAR_NEWLINE+
 _("Slovenian")+" (sl) Erik Lovrič <erik.lovric@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Spanish")+" (es) UserFav <userfav.post@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Swedish")+" (sv) Åke Engelbrektson <eson@svenskasprakfiler.se>"+CtConst::CHAR_NEWLINE+
 _("Turkish")+" (tr) Ferhat Aydin <ferhataydin44@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Ukrainian")+" (uk) Andriy Kovtun <kovtunos@yandex.ru>");
    dialog.set_logo(icon);
    dialog.set_title(_("About CherryTree"));

    dialog.signal_activate_link().connect([](const Glib::ustring& link){
       fs::open_weblink(link);
       return true;
    }, false);

    dialog.set_transient_for(parent);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_modal(true);
    dialog.run();
}

void CtDialogs::summary_info_dialog(CtMainWin* pCtMainWin, const CtSummaryInfo& summaryInfo)
{
    Gtk::Dialog dialog = Gtk::Dialog{_("Tree Summary Information"),
                                     *pCtMainWin,
                                     Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(400, 300);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    Gtk::Grid grid;
    grid.property_margin() = 6;
    grid.set_row_spacing(4);
    grid.set_column_spacing(8);
    grid.set_row_homogeneous(true);
    Gtk::Label label_rt_key;
    label_rt_key.set_markup(Glib::ustring{"<b>"} + _("Number of Rich Text Nodes") + "</b>");
    grid.attach(label_rt_key, 0, 0, 1, 1);
    Gtk::Label label_rt_val{std::to_string(summaryInfo.nodes_rich_text_num)};
    grid.attach(label_rt_val, 1, 0, 1, 1);
    Gtk::Label label_pt_key;
    label_pt_key.set_markup(Glib::ustring{"<b>"} + _("Number of Plain Text Nodes") + "</b>");
    grid.attach(label_pt_key, 0, 1, 1, 1);
    Gtk::Label label_pt_val{std::to_string(summaryInfo.nodes_plain_text_num)};
    grid.attach(label_pt_val, 1, 1, 1, 1);
    Gtk::Label label_co_key;
    label_co_key.set_markup(Glib::ustring{"<b>"} + _("Number of Code Nodes") + "</b>");
    grid.attach(label_co_key, 0, 2, 1, 1);
    Gtk::Label label_co_val{std::to_string(summaryInfo.nodes_code_num)};
    grid.attach(label_co_val, 1, 2, 1, 1);
    Gtk::Label label_im_key;
    label_im_key.set_markup(Glib::ustring{"<b>"} + _("Number of Images") + "</b>");
    grid.attach(label_im_key, 0, 3, 1, 1);
    Gtk::Label label_im_val{std::to_string(summaryInfo.images_num)};
    grid.attach(label_im_val, 1, 3, 1, 1);
    Gtk::Label label_ef_key;
    label_ef_key.set_markup(Glib::ustring{"<b>"} + _("Number of Embedded Files") + "</b>");
    grid.attach(label_ef_key, 0, 4, 1, 1);
    Gtk::Label label_ef_val{std::to_string(summaryInfo.embfile_num)};
    grid.attach(label_ef_val, 1, 4, 1, 1);
    Gtk::Label label_ta_key;
    label_ta_key.set_markup(Glib::ustring{"<b>"} + _("Number of Tables") + "</b>");
    grid.attach(label_ta_key, 0, 5, 1, 1);
    Gtk::Label label_ta_val{std::to_string(summaryInfo.tables_num)};
    grid.attach(label_ta_val, 1, 5, 1, 1);
    Gtk::Label label_cb_key;
    label_cb_key.set_markup(Glib::ustring{"<b>"} + _("Number of CodeBoxes") + "</b>");
    grid.attach(label_cb_key, 0, 6, 1, 1);
    Gtk::Label label_cb_val{std::to_string(summaryInfo.codeboxes_num)};
    grid.attach(label_cb_val, 1, 6, 1, 1);
    Gtk::Label label_an_key;
    label_an_key.set_markup(Glib::ustring{"<b>"} + _("Number of Anchors") + "</b>");
    grid.attach(label_an_key, 0, 7, 1, 1);
    Gtk::Label label_an_val{std::to_string(summaryInfo.anchors_num)};
    grid.attach(label_an_val, 1, 7, 1, 1);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(grid);
    pContentArea->show_all();
    dialog.run();
    dialog.hide();
}
