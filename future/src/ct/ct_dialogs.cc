/*
 * ct_dialogs.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/colorchooserdialog.h>
#include "ct_app.h"
#include "ct_treestore.h"

using namespace ct_dialogs;

namespace ct_dialogs {
template class CtChooseDialogStore<Gtk::ListStore>;
template class CtChooseDialogStore<Gtk::TreeStore>;
}

template<class GtkStoreBase>
Glib::RefPtr<CtChooseDialogStore<GtkStoreBase>> CtChooseDialogStore<GtkStoreBase>::create()
{
    Glib::RefPtr<CtChooseDialogStore<GtkStoreBase>> model(new CtChooseDialogStore<GtkStoreBase>());
    model->set_column_types(model->columns);
    return model;
}

template<class GtkStoreBase>
void CtChooseDialogStore<GtkStoreBase>::add_row(const std::string& stock_id, const std::string& key, const std::string& desc, gint64 node_id /*=0*/)
{
    auto row = *GtkStoreBase::append();
    row[columns.stock_id] = stock_id;
    row[columns.key] = key;
    row[columns.desc] = desc;
    row[columns.node_id] = node_id;
}


Gtk::TreeModel::iterator ct_dialogs::choose_item_dialog(Gtk::Window& parent, const std::string& title,
                                                        Glib::RefPtr<CtChooseDialogListStore> model,
                                                        const gchar* one_columns_name /* = nullptr */)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 300);
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Gtk::TreeView* elements_treeview = Gtk::manage(new Gtk::TreeView(model));
    elements_treeview->set_headers_visible(false);
    Gtk::CellRendererPixbuf pix_buf_renderer;
    if (one_columns_name == nullptr) {
        int col_num = elements_treeview->append_column("", pix_buf_renderer) - 1;
        elements_treeview->get_column(col_num)->add_attribute(pix_buf_renderer, "icon-name", model->columns.stock_id);
    }
    elements_treeview->append_column(one_columns_name ? one_columns_name : "", model->columns.desc);
    scrolledwindow->add(*elements_treeview);
    //list_parms->sel_iter = elements_liststore->get_iter_first()
    //if list_parms->sel_iter:
    //    elements_treeview->set_cursor(elements_liststore->get_path(list_parms->sel_iter))
    auto content_area = dialog.get_content_area();
    content_area->pack_start(*scrolledwindow);
    content_area->show_all();
    elements_treeview->grab_focus();

    if (dialog.run() != Gtk::RESPONSE_ACCEPT) return Gtk::TreeModel::iterator();
    return elements_treeview->get_selection()->get_selected();
}

// Dialog to select a color, featuring a palette
bool ct_dialogs::color_pick_dialog(Gtk::Window& parent, Gdk::RGBA& color)
{
    Gtk::ColorChooserDialog dialog(_("Pick a Color"), parent);
    dialog.set_transient_for(parent);
    dialog.set_modal(true);
    dialog.set_property("destroy-with-parent", true);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    std::vector<std::string> colors = str::split(CtApp::P_ctCfg->colorPalette, ":");
    std::vector<Gdk::RGBA> rgbas;
    for (const auto& c: colors)
        rgbas.push_back(Gdk::RGBA(c));
    dialog.add_palette(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10, rgbas);
    dialog.set_rgba(color);

    if (Gtk::RESPONSE_OK != dialog.run())
        return false;

    std::string ret_color_hex8 = CtRgbUtil::rgb_any_to_24(dialog.get_rgba());
    size_t color_qty = colors.size();
    colors.erase(std::find(colors.begin(), colors.end(), ret_color_hex8));
    if (color_qty == colors.size())
        colors.pop_back();
    colors.insert(colors.begin(), ret_color_hex8);

    color = dialog.get_rgba();
    return true;
}

// The Question dialog, returns True if the user presses OK
bool ct_dialogs::question_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Question"),
              true /* use_markup */, Gtk::MESSAGE_QUESTION,
              Gtk::BUTTONS_OK_CANCEL, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    return dialog.run() == Gtk::RESPONSE_OK;
}

// The Info dialog
void ct_dialogs::info_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Info"),
              true /* use_markup */, Gtk::MESSAGE_INFO,
              Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// The Warning dialog
void ct_dialogs::warning_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Warning"),
              true /* use_markup */, Gtk::MESSAGE_WARNING,
              Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// The Error dialog
void ct_dialogs::error_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Error"),
              true /* use_markup */, Gtk::MESSAGE_ERROR,
              Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// Dialog to Select a Node
Gtk::TreeIter ct_dialogs::choose_node_dialog(Gtk::Window& parent, Gtk::TreeView& parentTreeView, const std::string& title, CtTreeStore* treestore, Gtk::TreeIter sel_tree_iter)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);
    auto treeview_2 = Gtk::TreeView(treestore->get_store());
    treeview_2.set_headers_visible(false);
    treeview_2.set_search_column(1);
    treeview_2.append_column("", treestore->get_columns().rColPixbuf);
    treeview_2.append_column("", treestore->get_columns().colNodeName);
    auto scrolledwindow = Gtk::ScrolledWindow();
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview_2);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(scrolledwindow);

    auto expand_collapse_row = [&treeview_2](Gtk::TreePath path) {
        if (treeview_2.row_expanded(path))
            treeview_2.collapse_row(path);
        else
            treeview_2.expand_row(path, false);
    };

    treeview_2.signal_event().connect([&treeview_2, &expand_collapse_row](GdkEvent* event)->bool{
        if (event->type != GDK_BUTTON_PRESS && event->type!= GDK_2BUTTON_PRESS && event->type != GDK_KEY_PRESS)
            return false;
        if (event->type == GDK_BUTTON_PRESS && event->button.button == 2) {
            Gtk::TreePath path_at_click;
            if (treeview_2.get_path_at_pos(event->button.x, event->button.y, path_at_click)) {
                expand_collapse_row(path_at_click);
                return true;
            }
        } else if (event->type == GDK_2BUTTON_PRESS && event->button.button == 1) {
            if (treeview_2.get_selection()->get_selected()) {
                expand_collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
                return true;
            }
        } else if (event->type == GDK_KEY_PRESS && treeview_2.get_selection()->get_selected()) {
            if (event->key.keyval == GDK_KEY_Left)
                treeview_2.collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
            else if (event->key.keyval == GDK_KEY_Right)
                treeview_2.expand_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()), false);
            else
                return false;
            return true;
        }
        return false;
    });
    
    content_area->show_all();
    std::string expanded_collapsed_string = treestore->get_tree_expanded_collapsed_string(parentTreeView);
    treestore->set_tree_expanded_collapsed_string(expanded_collapsed_string, treeview_2, CtApp::P_ctCfg->nodesBookmExp);
    if (sel_tree_iter) {
        Gtk::TreePath sel_path = treeview_2.get_model()->get_path(sel_tree_iter);
        treeview_2.expand_to_path(sel_path);
        treeview_2.set_cursor(sel_path);
        treeview_2.scroll_to_row(sel_path);
    }

    return dialog.run() == Gtk::RESPONSE_ACCEPT ? treeview_2.get_selection()->get_selected() : Gtk::TreeIter();
}

// Handle the Bookmarks List
void ct_dialogs::bookmarks_handle_dialog(CtMainWin* ctMainWin)
{
    CtTreeStore& ctTreestore = ctMainWin->get_tree_store();
    const std::list<gint64>& bookmarks = ctTreestore.get_bookmarks();

    Gtk::Dialog dialog(_("Handle the Bookmarks List"), *ctMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(500, 400);

    auto model = ct_dialogs::CtChooseDialogTreeStore::create();
    for (const gint64& node_id: bookmarks)
        model->add_row("pin", "", ctTreestore.get_node_name_from_node_id(node_id), node_id);

    auto treeview = Gtk::TreeView(model);
    treeview.set_headers_visible(false);
    treeview.set_reorderable(true);
    Gtk::CellRendererPixbuf pix_buf_renderer;
    int col_num = treeview.append_column("", pix_buf_renderer) - 1;
    treeview.get_column(col_num)->add_attribute(pix_buf_renderer, "icon-name", model->columns.stock_id);
    treeview.append_column("", model->columns.desc);
    auto scrolledwindow = Gtk::ScrolledWindow();
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview);
    auto content_area = dialog.get_content_area();

    auto button_move_up = Gtk::Button();
    button_move_up.set_image_from_icon_name("gtk-go-up", Gtk::ICON_SIZE_DND);
    auto button_move_down = Gtk::Button();
    button_move_down.set_image_from_icon_name("gtk-go-down", Gtk::ICON_SIZE_DND);
    auto button_delete = Gtk::Button();
    button_delete.set_image_from_icon_name("gtk-clear", Gtk::ICON_SIZE_DND);
    auto button_sort_desc = Gtk::Button();
    button_sort_desc.set_image_from_icon_name("gtk-sort-descending", Gtk::ICON_SIZE_DND);
    auto button_sort_asc = Gtk::Button();
    button_sort_asc.set_image_from_icon_name("gtk-sort-ascending", Gtk::ICON_SIZE_DND);
    auto label1 = Gtk::Label();
    auto label2 = Gtk::Label();
    auto hbox = Gtk::HBox();
    auto vbox = Gtk::VBox();
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
    content_area->pack_start(hbox);
    content_area->show_all();

    treeview.signal_key_press_event().connect([&model, &treeview](GdkEventKey* key) -> bool {
        if (key->keyval == GDK_KEY_Delete) {
            Gtk::TreeIter tree_iter = treeview.get_selection()->get_selected();
            if (tree_iter) model->erase(tree_iter);
            return true;
        }
        return false;
    });
    treeview.signal_button_press_event().connect([&treeview, &model, &ctMainWin, &ctTreestore](GdkEventButton* event) -> bool {
        if (event->button != 1 || event->type != GDK_2BUTTON_PRESS) return false;
        Gtk::TreePath clicked_path;
        if (!treeview.get_path_at_pos(event->x, event->y, clicked_path)) return false;
        Gtk::TreeIter clicked_iter = model->get_iter(clicked_path);
        gint64 node_id = clicked_iter->get_value(model->columns.node_id);
        Gtk::TreeIter tree_iter = ctTreestore.get_tree_iter_from_node_id(node_id);
        ctMainWin->get_tree_view().set_cursor_safe(tree_iter);
        return true;
    });
    button_move_up.signal_clicked().connect([&treeview, &model](){
        Gtk::TreeIter curr_iter = treeview.get_selection()->get_selected();
        Gtk::TreeIter prev_iter = --treeview.get_selection()->get_selected();
        if (curr_iter && prev_iter) model->iter_swap(curr_iter, prev_iter);
    });
    button_move_down.signal_clicked().connect([&treeview, &model](){
        Gtk::TreeIter curr_iter = treeview.get_selection()->get_selected();
        Gtk::TreeIter next_iter = ++treeview.get_selection()->get_selected();
        if (curr_iter && next_iter) model->iter_swap(curr_iter, next_iter);
    });
    button_delete.signal_clicked().connect([&treeview, &model](){
        Gtk::TreeIter tree_iter = treeview.get_selection()->get_selected();
        if (tree_iter) model->erase(tree_iter);
    });
    button_sort_asc.signal_clicked().connect([&model](){
        auto need_swap = [&model](Gtk::TreeIter& l, Gtk::TreeIter& r) {
            int cmp = l->get_value(model->columns.desc).compare(r->get_value(model->columns.desc));
            return cmp > 0;
        };
        CtMiscUtil::node_siblings_sort_iteration(model, model->children(), need_swap);
    });
    button_sort_desc.signal_clicked().connect([&model](){
        auto need_swap = [&model](Gtk::TreeIter& l, Gtk::TreeIter& r) {
            int cmp = l->get_value(model->columns.desc).compare(r->get_value(model->columns.desc));
            return cmp < 0;
        };
        CtMiscUtil::node_siblings_sort_iteration(model, model->children(), need_swap);
    });

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return;


    std::set<gint64> temp_bookmarks;
    std::list<gint64> temp_bookmarks_order;
    model->foreach_iter([&temp_bookmarks, &temp_bookmarks_order, &model](const Gtk::TreeIter& iter) {
        gint64 node_id = iter->get_value(model->columns.node_id);
        temp_bookmarks.insert(node_id);
        temp_bookmarks_order.push_back(node_id);
        return false; /* to continue */
    });

    std::list<gint64> removed_bookmarks;
    for (const gint64& node_id: bookmarks)
        if (!set::exists(temp_bookmarks, node_id))
            removed_bookmarks.push_back(node_id);

    ctTreestore.set_bookmarks(temp_bookmarks_order);
    gint64 curr_node_id = ctMainWin->curr_tree_iter().get_node_id();
    for (gint64& node_id: removed_bookmarks) {
        Gtk::TreeIter tree_iter = ctTreestore.get_tree_iter_from_node_id(node_id);
        if (tree_iter) {
            ctTreestore.updateNodeAuxIcon(tree_iter);
            if (curr_node_id == node_id)
                ctMainWin->menu_tree_update_for_bookmarked_node(false);
        }
    }

    ctMainWin->set_bookmarks_menu_items();
    //dad.ctdb_handler.pending_edit_db_bookmarks()
    ctMainWin->update_window_save_needed("book");
}

// Dialog to select a Date
std::time_t ct_dialogs::date_select_dialog(Gtk::Window& parent, const std::string& title, const std::time_t& curr_time)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);

    std::tm struct_time = *std::localtime(&curr_time);

    auto content_area = dialog.get_content_area();
    auto calendar = Gtk::Calendar();
    calendar.select_month(struct_time.tm_mon-1, struct_time.tm_year); // month 0-11
    calendar.select_day(struct_time.tm_mday); // day 1-31
    auto adj_h = Gtk::Adjustment::create(struct_time.tm_hour, 0, 23, 1);
    auto spinbutton_h = Gtk::SpinButton(adj_h);
    spinbutton_h.set_value(struct_time.tm_hour);
    auto adj_m = Gtk::Adjustment::create(struct_time.tm_min, 0, 59, 1);
    auto spinbutton_m = Gtk::SpinButton(adj_m);
    spinbutton_m.set_value(struct_time.tm_min);
    auto hbox = Gtk::HBox();
    hbox.pack_start(spinbutton_h);
    hbox.pack_start(spinbutton_m);
    content_area->pack_start(calendar);
    content_area->pack_start(hbox);

    dialog.signal_button_press_event().connect([&dialog](GdkEventButton* event)->bool {
        if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
            return false;
        dialog.response(Gtk::RESPONSE_ACCEPT);
        return true;
    });
    content_area->show_all();

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return 0;

    guint new_year, new_month, new_day;
    calendar.get_date(new_year, new_month, new_day);

    std::tm tmtime = {0};
    tmtime.tm_year = new_year;
    tmtime.tm_mon = new_month + 1;
    tmtime.tm_mday = new_day;
    tmtime.tm_hour = spinbutton_h.get_value_as_int();
    tmtime.tm_min = spinbutton_m.get_value_as_int();

    std::time_t new_time = std::mktime(&tmtime);
    return new_time;
}

void ct_dialogs::match_dialog(const std::string& title, CtMainWin& ctMainWin, Glib::RefPtr<CtMatchDialogStore> model)
{
    /* will be delete on hide event */
    Gtk::Dialog* allmatchesdialog = new Gtk::Dialog(title, ctMainWin, Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    allmatchesdialog->set_transient_for(ctMainWin);
    if (model->dlg_size[0] > 0) {
        allmatchesdialog->set_default_size(model->dlg_size[0], model->dlg_size[1]);
        allmatchesdialog->move(model->dlg_pos[0], model->dlg_pos[1]);
    } else {
        allmatchesdialog->set_default_size(700, 350);
        allmatchesdialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }
    CtAction* action = ctMainWin.get_ct_menu().find_action("toggle_show_allmatches_dlg");
    auto button_hide = allmatchesdialog->add_button(str::format(_("Hide (Restore with '%s')"), action->get_shortcut()), Gtk::RESPONSE_CLOSE);
    button_hide->set_image_from_icon_name(Gtk::Stock::CLOSE.id, Gtk::ICON_SIZE_BUTTON);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(model));
    treeview->append_column(_("Node Name"), model->columns.node_name);
    treeview->append_column(_("Line"), model->columns.line_num);
    treeview->append_column(_("Line Content"), model->columns.line_content);
    treeview->append_column("", model->columns.node_hier_name);
    treeview->get_column(3)->property_visible() = false;
    treeview->set_tooltip_column(3);
    auto scrolledwindow_allmatches = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow_allmatches->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow_allmatches->add(*treeview);
    auto content_area = allmatchesdialog->get_content_area();
    content_area->pack_start(*scrolledwindow_allmatches);

    if (model->saved_path) {
        treeview->set_cursor(model->saved_path);
        treeview->scroll_to_row(model->saved_path, 0.5);
    }

    treeview->signal_event_after().connect([treeview, model, &ctMainWin](GdkEvent* event) {
        if (event->type != GDK_BUTTON_PRESS && event->type != GDK_KEY_PRESS) return;
        Gtk::TreeIter list_iter = treeview->get_selection()->get_selected();
        if (!list_iter) return;
        gint64 node_id = list_iter->get_value(model->columns.node_id);
        CtTreeIter tree_iter = ctMainWin.get_tree_store().get_tree_iter_from_node_id(node_id);
        if (!tree_iter) {
            ct_dialogs::error_dialog(str::format(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)"), node_id), ctMainWin);
            model->erase(list_iter);
            return;
        }
        ctMainWin.get_tree_view().set_cursor_safe(tree_iter);
        auto curr_buffer = ctMainWin.get_text_view().get_buffer();
        curr_buffer->move_mark(curr_buffer->get_insert(),
                               curr_buffer->get_iter_at_offset(list_iter->get_value(model->columns.start_offset)));
        curr_buffer->move_mark(curr_buffer->get_selection_bound(),
                               curr_buffer->get_iter_at_offset(list_iter->get_value(model->columns.end_offset)));
        ctMainWin.get_text_view().scroll_to(curr_buffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
    });
    button_hide->signal_clicked().connect([allmatchesdialog](){
        allmatchesdialog->hide();
    });
    allmatchesdialog->signal_hide().connect([allmatchesdialog, treeview, model]() {
        allmatchesdialog->get_position(model->dlg_pos[0], model->dlg_pos[1]);
        allmatchesdialog->get_size(model->dlg_size[0], model->dlg_size[1]);
        auto list_iter = treeview->get_selection()->get_selected();
        model->saved_path = treeview->get_model()->get_path(list_iter);

        delete allmatchesdialog;
    });

    allmatchesdialog->show_all();
}

// Insert/Edit Anchor Name
Glib::ustring ct_dialogs::img_n_entry_dialog(Gtk::Window& parent, const char* title,
                                             const Glib::ustring& entry_content, const char* img_stock)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);
    auto image = Gtk::Image();
    image.set_from_icon_name(img_stock, Gtk::ICON_SIZE_BUTTON);
    auto entry = Gtk::Entry();
    entry.set_text(entry_content);
    auto hbox = Gtk::HBox();
    hbox.pack_start(image, false, false);
    hbox.pack_start(entry);
    hbox.set_spacing(5);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(hbox);
    content_area->show_all();
    entry.grab_focus();
    if (Gtk::RESPONSE_ACCEPT != dialog.run())
        return "";
    return str::trim(entry.get_text());
}

// Dialog to Insert/Edit Links
bool ct_dialogs::link_handle_dialog(CtMainWin& ctMainWin, const Glib::ustring& title, Gtk::TreeIter sel_tree_iter,
                                    CtLinkEntry& link_entries)
{
    CtTreeStore& ctTreestore = ctMainWin.get_tree_store();
    Gtk::Dialog dialog(title, ctMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);

    auto hbox_webs = Gtk::HBox();
    auto image_webs = Gtk::Image();
    image_webs.set_from_icon_name("link_website", Gtk::ICON_SIZE_BUTTON);
    auto radiobutton_webs = Gtk::RadioButton(_("To WebSite"));
    auto entry_webs = Gtk::Entry();
    entry_webs.set_text(link_entries.webs);
    hbox_webs.pack_start(image_webs, false, false);
    hbox_webs.pack_start(radiobutton_webs, false, false);
    hbox_webs.pack_start(entry_webs);
    hbox_webs.set_spacing(5);

    auto hbox_file = Gtk::HBox();
    auto image_file = Gtk::Image();
    image_file.set_from_icon_name(Gtk::Stock::FILE.id, Gtk::ICON_SIZE_BUTTON);
    auto radiobutton_file = Gtk::RadioButton(_("To File"));
    radiobutton_file.join_group(radiobutton_webs);
    auto entry_file = Gtk::Entry();
    entry_file.set_text(link_entries.file);
    auto button_browse_file = Gtk::Button();
    button_browse_file.set_image_from_icon_name("find", Gtk::ICON_SIZE_BUTTON);
    hbox_file.pack_start(image_file, false, false);
    hbox_file.pack_start(radiobutton_file, false, false);
    hbox_file.pack_start(entry_file);
    hbox_file.pack_start(button_browse_file, false, false);
    hbox_file.set_spacing(5);

    auto hbox_folder = Gtk::HBox();
    auto image_folder = Gtk::Image();
    image_folder.set_from_icon_name(Gtk::Stock::DIRECTORY.id, Gtk::ICON_SIZE_BUTTON);
    auto radiobutton_folder = Gtk::RadioButton(_("To Folder"));
    radiobutton_folder.join_group(radiobutton_webs);
    auto entry_folder = Gtk::Entry();
    entry_folder.set_text(link_entries.fold);
    auto button_browse_folder = Gtk::Button();
    button_browse_folder.set_image_from_icon_name("find", Gtk::ICON_SIZE_BUTTON);
    hbox_folder.pack_start(image_folder, false, false);
    hbox_folder.pack_start(radiobutton_folder, false, false);
    hbox_folder.pack_start(entry_folder);
    hbox_folder.pack_start(button_browse_folder, false, false);
    hbox_folder.set_spacing(5);

    auto hbox_node = Gtk::HBox();
    auto image_node = Gtk::Image();
    image_node.set_from_icon_name("cherrytree", Gtk::ICON_SIZE_BUTTON);
    auto radiobutton_node = Gtk::RadioButton(_("To Node"));
    radiobutton_node.join_group(radiobutton_webs);
    hbox_node.pack_start(image_node, false, false);
    hbox_node.pack_start(radiobutton_node);
    hbox_node.set_spacing(5);

    auto hbox_detail = Gtk::HBox();

    auto treeview_2 = Gtk::TreeView(ctMainWin.get_tree_store().get_store());
    treeview_2.set_headers_visible(false);
    treeview_2.set_search_column(1);
    auto renderer_pixbuf_2 = Gtk::CellRendererPixbuf();
    auto renderer_text_2 = Gtk::CellRendererText();
    auto column_2 = Gtk::TreeViewColumn();
    treeview_2.append_column("", ctMainWin.get_tree_store().get_columns().rColPixbuf);
    treeview_2.append_column("", ctMainWin.get_tree_store().get_columns().colNodeName);
    auto scrolledwindow = Gtk::ScrolledWindow();
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview_2);

    auto vbox_anchor = Gtk::VBox();
    auto label_over = Gtk::Label();
    auto label_below = Gtk::Label();

    auto hbox_anchor = Gtk::HBox();
    auto entry_anchor = Gtk::Entry();
    entry_anchor.set_text(link_entries.anch);
    auto button_browse_anchor = Gtk::Button();
    button_browse_anchor.set_image_from_icon_name("anchor", Gtk::ICON_SIZE_BUTTON);
    hbox_anchor.pack_start(entry_anchor);
    hbox_anchor.pack_start(button_browse_anchor, false, false);

    auto frame_anchor = Gtk::Frame(std::string("<b>")+_("Anchor Name (optional)")+"</b>");
    dynamic_cast<Gtk::Label*>(frame_anchor.get_label_widget())->set_use_markup(true);
    frame_anchor.set_shadow_type(Gtk::SHADOW_NONE);
    frame_anchor.add(hbox_anchor);

    vbox_anchor.pack_start(label_over);
    vbox_anchor.pack_start(frame_anchor, false, false);
    vbox_anchor.pack_start(label_below);

    hbox_detail.pack_start(scrolledwindow);
    hbox_detail.pack_start(vbox_anchor, false, false);

    auto content_area = dialog.get_content_area();
    content_area->pack_start(hbox_webs, false, false);
    content_area->pack_start(hbox_file, false, false);
    content_area->pack_start(hbox_folder, false, false);
    content_area->pack_start(hbox_node, false, false);
    content_area->pack_start(hbox_detail);
    content_area->set_spacing(5);

    radiobutton_webs.set_active(link_entries.type == CtConst::LINK_TYPE_WEBS);
    radiobutton_node.set_active(link_entries.type == CtConst::LINK_TYPE_NODE);
    radiobutton_file.set_active(link_entries.type == CtConst::LINK_TYPE_FILE);
    radiobutton_folder.set_active(link_entries.type == CtConst::LINK_TYPE_FOLD);

    bool first_in = true;
    sel_tree_iter = sel_tree_iter ? sel_tree_iter : link_entries.prev_node;

    auto link_type_changed_on_dialog = [&]() {
        entry_webs.set_sensitive(link_entries.type == CtConst::LINK_TYPE_WEBS);
        hbox_detail.set_sensitive(link_entries.type == CtConst::LINK_TYPE_NODE);
        entry_file.set_sensitive(link_entries.type == CtConst::LINK_TYPE_FILE);
        entry_folder.set_sensitive(link_entries.type == CtConst::LINK_TYPE_FOLD);
        if (link_entries.type == CtConst::LINK_TYPE_WEBS)
            entry_webs.grab_focus();
        else if (link_entries.type == CtConst::LINK_TYPE_NODE) {
            treeview_2.grab_focus();
            if (first_in) {
                first_in = false;
                auto exp_colpsd_str = ctTreestore.get_tree_expanded_collapsed_string(ctMainWin.get_tree_view());
                ctTreestore.set_tree_expanded_collapsed_string(exp_colpsd_str, treeview_2, CtApp::P_ctCfg->nodesBookmExp);
            }
            if (!sel_tree_iter) sel_tree_iter = ctTreestore.get_iter_first();
            auto sel_path = ctTreestore.get_path(sel_tree_iter);
            treeview_2.expand_to_path(sel_path);
            treeview_2.set_cursor(sel_path);
            treeview_2.scroll_to_row(sel_path);
        }
        else if (link_entries.type == CtConst::LINK_TYPE_FILE)
            entry_file.grab_focus();
        else
            entry_folder.grab_focus();
    };

    radiobutton_webs.signal_toggled().connect([&](){
        if (radiobutton_webs.get_active()) link_entries.type = CtConst::LINK_TYPE_WEBS;
        link_type_changed_on_dialog();
    });
    radiobutton_node.signal_toggled().connect([&](){
        if (radiobutton_node.get_active()) link_entries.type = CtConst::LINK_TYPE_NODE;
        link_type_changed_on_dialog();
    });
    radiobutton_file.signal_toggled().connect([&](){
        if (radiobutton_file.get_active()) link_entries.type = CtConst::LINK_TYPE_FILE;
        link_type_changed_on_dialog();
    });
    radiobutton_folder.signal_toggled().connect([&](){
        if (radiobutton_folder.get_active()) link_entries.type = CtConst::LINK_TYPE_FOLD;
        link_type_changed_on_dialog();
    });
    button_browse_file.signal_clicked().connect([&](){
        auto filepath = ct_dialogs::file_select_dialog({.parent=&dialog, .curr_folder=CtApp::P_ctCfg->pickDirFile});
        if (filepath.empty()) return;
        CtApp::P_ctCfg->pickDirFile = filepath; /* todo: os.path.dirname(filepath)*/;
        if (CtApp::P_ctCfg->linksRelative) {
            /* todo: filepath = os.path.relpath(filepath, dad.file_dir) */
        }
        entry_file.set_text(filepath);
    });
    button_browse_folder.signal_clicked().connect([&](){
        auto filepath = ct_dialogs::folder_select_dialog(CtApp::P_ctCfg->pickDirFile, &dialog);
        if (filepath.empty()) return;
        CtApp::P_ctCfg->pickDirFile = filepath;
        if (CtApp::P_ctCfg->linksRelative) {
            /* todo: try: filepath = os.path.relpath(filepath, dad.file_dir)
            except: print "cannot set relative path for different drives"
            */
        }
        entry_folder.set_text(filepath);
    });
    button_browse_anchor.signal_clicked().connect([&](){
        if (!sel_tree_iter) {
            ct_dialogs::warning_dialog(_("No Node is Selected"), dialog);
            return;
        }
        //anchors_list = [];
        auto text_buffer = ctTreestore.to_ct_tree_iter(sel_tree_iter).get_node_text_buffer();
        auto curr_iter = text_buffer->begin();
        while (true) {
            auto anchor = curr_iter.get_child_anchor();
            if (anchor) {
                /* todo:
                if "pixbuf" in dir(anchor) and "anchor" in dir(anchor.pixbuf):
                    anchors_list.append([anchor.pixbuf.anchor])
                    */
            }
            if (!curr_iter.forward_char()) break;
        }
        /* todo:
        dad.objects_buffer_refresh()
        if not anchors_list:
            dialog_info(_("There are No Anchors in the Selected Node"), dialog)
            return
        ret_anchor_name = dialog_choose_element_in_list(dialog, _("Choose Existing Anchor"), anchors_list, _("Anchor Name"))
        if ret_anchor_name: entry_anchor.set_text(ret_anchor_name)
        */
    });
    treeview_2.signal_event_after().connect([&](GdkEvent* event){
        if (event->type != GDK_BUTTON_PRESS && event->type != GDK_2BUTTON_PRESS && event->type != GDK_KEY_PRESS)
            return;
        sel_tree_iter = treeview_2.get_selection()->get_selected();
        if (event->type == GDK_BUTTON_PRESS && event->button.button == 2) {
            Gtk::TreePath path_at_click;
            if (treeview_2.get_path_at_pos(event->button.x, event->button.y, path_at_click)) {
                if (treeview_2.row_expanded(path_at_click))
                    treeview_2.collapse_row(path_at_click);
                else
                    treeview_2.expand_row(path_at_click, true);
            }
        }
        else if (event->type == GDK_2BUTTON_PRESS && event->button.button == 1 && sel_tree_iter) {
            auto path = ctTreestore.get_path(sel_tree_iter);
            if (treeview_2.row_expanded(path))
                treeview_2.collapse_row(path);
            else
                treeview_2.expand_row(path, true);
        }
        else if (event->type == GDK_KEY_PRESS && sel_tree_iter) {
            auto path = ctTreestore.get_path(sel_tree_iter);
            if (event->key.keyval == GDK_KEY_Left)
                treeview_2.collapse_row(path);
            else if (event->key.keyval == GDK_KEY_Right)
                treeview_2.expand_row(path, false);
        }
    });
    dialog.signal_key_press_event().connect([&](GdkEventKey* event){
        if (event->keyval == GDK_KEY_Tab) {
            if (link_entries.type == CtConst::LINK_TYPE_WEBS) radiobutton_file.set_active(true);
            else if (link_entries.type == CtConst::LINK_TYPE_FILE) radiobutton_folder.set_active(true);
            else if (link_entries.type == CtConst::LINK_TYPE_FOLD) radiobutton_node.set_active(true);
            else radiobutton_webs.set_active(true);
            return true;
        }
        return false;
    });

    content_area->show_all();
    link_type_changed_on_dialog();

    if (dialog.run() != GTK_RESPONSE_ACCEPT)
        return false;

    link_entries.webs = str::trim(entry_webs.get_text());
    link_entries.file = str::trim(entry_file.get_text());
    link_entries.fold = str::trim(entry_folder.get_text());
    link_entries.anch = str::trim(entry_anchor.get_text());
    link_entries.prev_node = sel_tree_iter;
    return true;
}

// The Select file dialog, Returns the retrieved filepath or None
Glib::ustring ct_dialogs::file_select_dialog(ct_dialogs::file_select_args args)
{
    auto chooser = Gtk::FileChooserDialog(_("Select File"), Gtk::FILE_CHOOSER_ACTION_OPEN);
    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    if (args.parent) {
        chooser.set_transient_for(*args.parent);
        chooser.set_property("modal", true);
        chooser.set_property("destroy-with-parent", true);
        chooser.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    } else
        chooser.set_position(Gtk::WIN_POS_CENTER);
    if (args.curr_folder.empty() /* todo: || os.path.isdir(curr_folder) == False */)
        chooser.set_current_folder("~" /* todo: os.path.expanduser('~')*/);
    else
        chooser.set_current_folder(args.curr_folder);
    if (args.filter_pattern.size() || args.filter_mime.size()) {
        auto filefilter = Gtk::FileFilter::create();
        filefilter->set_name(args.filter_name);
        for (auto& element: args.filter_pattern)
            filefilter->add_pattern(element);
        for (auto& element: args.filter_mime)
            filefilter->add_mime_type(element);
        chooser.add_filter(filefilter);
    }
    if (chooser.run() == Gtk::RESPONSE_ACCEPT)
        return chooser.get_filename();
    return "";
}

// The Select folder dialog, returns the retrieved folderpath or None
Glib::ustring ct_dialogs::folder_select_dialog(Glib::ustring curr_folder, Gtk::Window* parent /*= nullptr*/)
{
    auto chooser = Gtk::FileChooserDialog(_("Select Folder"), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    if (parent) {
        chooser.set_transient_for(*parent);
        chooser.set_property("modal", true);
        chooser.set_property("destroy-with-parent", true);
        chooser.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }
    else
        chooser.set_position(Gtk::WIN_POS_CENTER);
    if (curr_folder.empty() /* todo: ||  os.path.isdir(curr_folder) == False */)
        chooser.set_current_folder("~" /* todo: os.path.expanduser('~')*/);
    else
        chooser.set_current_folder(curr_folder);
    if (chooser.run() == Gtk::RESPONSE_ACCEPT)
        return chooser.get_filename();
    return "";
}
