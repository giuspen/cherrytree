/*
 * ct_dialogs.cc
 *
 * Copyright 2009-2020
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
#include "ct_storage_control.h"
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/colorchooserdialog.h>

CtDialogTextEntry::CtDialogTextEntry(const Glib::ustring& title,
                                     const bool forPassword,
                                     Gtk::Window* pParentWin)
{
    set_title(title);
    set_transient_for(*pParentWin);
    set_modal();

    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default_response(Gtk::RESPONSE_OK);

    _entry.set_icon_from_stock(Gtk::Stock::CLEAR, Gtk::ENTRY_ICON_SECONDARY);
    _entry.set_size_request(350, -1);
    if (forPassword)
    {
        _entry.set_visibility(false);
    }
    get_vbox()->pack_start(_entry, true, true, 0);

    // Special signals which correspond to the underlying X-Windows events are suffixed by _event.
    // By default, your signal handlers are called after any previously-connected signal handlers. However, this can be a problem with the X Event signals. For instance, the existing signal handlers, or the default signal handler, might return true to stop other signal handlers from being called. To specify that your signal handler should be called before the other signal handlers, so that it will always be called, you can specify false for the optional after parameter
    _entry.signal_key_press_event().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_on_entry_key_press_event), false/*call me before other*/);

    _entry.signal_icon_press().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_on_entry_icon_press));

    get_vbox()->show_all();
}

bool CtDialogTextEntry::_on_entry_key_press_event(GdkEventKey *pEventKey)
{
    if (GDK_KEY_Return == pEventKey->keyval)
    {
        Gtk::Button *pButton = static_cast<Gtk::Button*>(get_widget_for_response(Gtk::RESPONSE_OK));
        pButton->clicked();
        return true;
    }
    return false;
}

// pygtk: dialog_choose_element_in_list
Gtk::TreeIter CtDialogs::choose_item_dialog(Gtk::Window& parent,
                                            const Glib::ustring& title,
                                            Glib::RefPtr<CtChooseDialogListStore> rModel,
                                            const gchar* single_column_name /* = nullptr */)
{
    Gtk::Dialog dialog(title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 300);
    Gtk::ScrolledWindow* pScrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    pScrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Gtk::TreeView* pElementsTreeview = Gtk::manage(new Gtk::TreeView(rModel));
    pElementsTreeview->set_headers_visible(false);
    pElementsTreeview->get_selection()->select(Gtk::TreePath("0"));
    pElementsTreeview->signal_row_activated().connect([&](const Gtk::TreeModel::Path&, Gtk::TreeViewColumn* ) {
        if (Gtk::TreeIter iter = pElementsTreeview->get_selection()->get_selected()) {
            static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT))->clicked();
        }
    });

    Gtk::CellRendererPixbuf pixbuf_renderer;
    if (nullptr == single_column_name)
    {
        pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
        int col_num = pElementsTreeview->append_column("", pixbuf_renderer) - 1;
        pElementsTreeview->get_column(col_num)->add_attribute(pixbuf_renderer, "icon-name", rModel->columns.stock_id);
        pElementsTreeview->append_column("", rModel->columns.desc);
        pElementsTreeview->set_search_column(2);
    }
    else
    {
        pElementsTreeview->append_column(single_column_name, rModel->columns.desc);
    }
    pScrolledwindow->add(*pElementsTreeview);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(*pScrolledwindow);
    pContentArea->show_all();
    pElementsTreeview->grab_focus();

    return (Gtk::RESPONSE_ACCEPT == dialog.run() ? pElementsTreeview->get_selection()->get_selected() : Gtk::TreeIter());
}

// Dialog to select between the Selected Node/Selected Node + Subnodes/All Tree
CtExporting CtDialogs::selnode_selnodeandsub_alltree_dialog(Gtk::Window& parent,
                                                            bool also_selection,
                                                            bool* last_include_node_name,
                                                            bool* last_new_node_page,
                                                            bool* last_index_in_page,
                                                            bool* last_single_file)
{
    Gtk::Dialog dialog(_("Involved Nodes"),
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    //dialog.set_default_size(400, 300);

    auto radiobutton_selection = Gtk::RadioButton(_("Selected Text Only"));
    auto radiobutton_selnode = Gtk::RadioButton(_("Selected Node Only"));
    auto radiobutton_selnodeandsub = Gtk::RadioButton(_("Selected Node and Subnodes"));
    auto radiobutton_alltree = Gtk::RadioButton(_("All the Tree"));
    radiobutton_selnodeandsub.join_group(radiobutton_selnode);
    radiobutton_alltree.join_group(radiobutton_selnode);

    auto content_area = dialog.get_content_area();
    if (also_selection)
    {
        radiobutton_selection.join_group(radiobutton_selnode);
        content_area->pack_start(radiobutton_selection);
    }
    content_area->pack_start(radiobutton_selnode);
    content_area->pack_start(radiobutton_selnodeandsub);
    content_area->pack_start(radiobutton_alltree);

    auto separator_item_1 = Gtk::HSeparator();
    auto checkbutton_node_name = Gtk::CheckButton(_("Include Node Name"));
    if (last_include_node_name != nullptr)
    {
        checkbutton_node_name.set_active(*last_include_node_name);
        content_area->pack_start(separator_item_1);
        content_area->pack_start(checkbutton_node_name);
    }
    auto separator_item_2 = Gtk::HSeparator();
    auto checkbutton_index_in_page = Gtk::CheckButton(_("Links Tree in Every Page"));
    if (last_index_in_page != nullptr)
    {
        checkbutton_index_in_page.set_active(*last_index_in_page);
        content_area->pack_start(separator_item_2);
        content_area->pack_start(checkbutton_index_in_page);
    }
    auto checkbutton_new_node_page = Gtk::CheckButton(_("New Node in New Page"));
    if (last_new_node_page != nullptr)
    {
        checkbutton_new_node_page.set_active(*last_new_node_page);
        content_area->pack_start(checkbutton_new_node_page);
    }
    auto checkbutton_single_file = Gtk::CheckButton(_("Single File"));
    if (last_single_file != nullptr) {
        checkbutton_single_file.set_active(*last_single_file);
        content_area->pack_start(checkbutton_single_file);
    }
    content_area->show_all();
    int response = dialog.run();

    if (last_include_node_name != nullptr) *last_include_node_name = checkbutton_node_name.get_active();
    if (last_index_in_page != nullptr) *last_index_in_page = checkbutton_index_in_page.get_active();
    if (last_new_node_page != nullptr) *last_new_node_page = checkbutton_new_node_page.get_active();
    if (last_single_file != nullptr) *last_single_file = checkbutton_single_file.get_active();

    if (response != Gtk::RESPONSE_ACCEPT) return CtExporting::NONE;
    if (radiobutton_selnode.get_active()) return CtExporting::CURRENT_NODE;
    if (radiobutton_selnodeandsub.get_active()) return CtExporting::CURRENT_NODE_AND_SUBNODES;
    if (radiobutton_alltree.get_active()) return CtExporting::ALL_TREE;
    return CtExporting::SELECTED_TEXT;
}

// Dialog to select a color, featuring a palette
CtDialogs::CtPickDlgState CtDialogs::color_pick_dialog(CtMainWin* pCtMainWin,
                                                       const Glib::ustring& title,
                                                       Gdk::RGBA& ret_color,
                                                       bool allow_remove_color)
{
    Gtk::ColorChooserDialog dialog(title);
    dialog.set_transient_for(*pCtMainWin);
    dialog.set_modal(true);
    dialog.property_destroy_with_parent() = true;
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    if (allow_remove_color) {
        dialog.add_button(_("Remove Color"), Gtk::RESPONSE_NONE);
    }
    std::vector<std::string> colors = str::split(pCtMainWin->get_ct_config()->colorPalette, ":");
    std::vector<Gdk::RGBA> rgbas;
    for (const std::string& color : colors)
    {
        rgbas.push_back(Gdk::RGBA(color));
    }
    dialog.add_palette(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10, rgbas);
    dialog.set_rgba(ret_color);

    auto on_key_press_dialog = [&](GdkEventKey *pEventKey)->bool
    {
        if (GDK_KEY_Return == pEventKey->keyval) {
            Gtk::Button *pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_OK));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    const int response = dialog.run();

    if (Gtk::RESPONSE_NONE == response) {
        return CtPickDlgState::REMOVE_COLOR;
    }
    if (Gtk::RESPONSE_OK != response) {
        return CtPickDlgState::CANCEL;
    }
    ret_color = dialog.get_rgba();
    std::string ret_color_hex8 = CtRgbUtil::rgb_any_to_24(ret_color);
    size_t color_qty = colors.size();
    colors.erase(std::find(colors.begin(), colors.end(), ret_color_hex8));
    if (color_qty == colors.size())
    {
        colors.pop_back();
    }
    colors.insert(colors.begin(), ret_color_hex8);

    return CtPickDlgState::SELECTED;
}

// The Question dialog, returns True if the user presses OK
bool CtDialogs::question_dialog(const Glib::ustring& message,
                                Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent,
                              message,
                              true/* use_markup */,
                              Gtk::MESSAGE_QUESTION,
                              Gtk::BUTTONS_YES_NO,
                              true/* modal */);
    dialog.set_title(_("Question"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_YES))->grab_focus();
    return (Gtk::RESPONSE_YES == dialog.run());
}

// The Info dialog
void CtDialogs::info_dialog(const Glib::ustring& message,
                            Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent,
                              message,
                              true/* use_markup */,
                              Gtk::MESSAGE_INFO,
                              Gtk::BUTTONS_OK,
                              true/* modal */);
    dialog.set_title(_("Info"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// The Warning dialog
void CtDialogs::warning_dialog(const Glib::ustring& message,
                               Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent,
                              message,
                              true/* use_markup */,
                              Gtk::MESSAGE_WARNING,
                              Gtk::BUTTONS_OK,
                              true/* modal */);
    dialog.set_title(_("Warning"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// The Error dialog
void CtDialogs::error_dialog(const Glib::ustring& message,
                             Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent,
                              message,
                              true/* use_markup */,
                              Gtk::MESSAGE_ERROR,
                              Gtk::BUTTONS_OK,
                              true/* modal */);
    dialog.set_title(_("Error"));
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// Dialog to Select a Node
Gtk::TreeIter CtDialogs::choose_node_dialog(CtMainWin* pCtMainWin,
                                            Gtk::TreeView& parentTreeView,
                                            const Glib::ustring& title,
                                            CtTreeStore* pCtTreeStore,
                                            Gtk::TreeIter sel_tree_iter)
{
    Gtk::Dialog dialog(title,
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);
    Gtk::TreeView treeview_2(pCtTreeStore->get_store());
    treeview_2.set_headers_visible(false);
    treeview_2.set_search_column(1);
    treeview_2.append_column("", pCtTreeStore->get_columns().rColPixbuf);
    treeview_2.append_column("", pCtTreeStore->get_columns().colNodeName);
    Gtk::ScrolledWindow scrolledwindow;
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview_2);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(scrolledwindow);

    auto expand_collapse_row = [&treeview_2](Gtk::TreePath path)
    {
        if (treeview_2.row_expanded(path))
        {
            treeview_2.collapse_row(path);
        }
        else
        {
            treeview_2.expand_row(path, false);
        }
    };

    treeview_2.signal_event().connect([&treeview_2, &expand_collapse_row](GdkEvent* event)->bool
    {
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
            if (treeview_2.get_selection()->get_selected())
            {
                expand_collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
                retVal = true; // stop event
            }
        }
        else if ( (event->type == GDK_KEY_PRESS) &&
                  (treeview_2.get_selection()->get_selected()) )
        {
            if (event->key.keyval == GDK_KEY_Left)
            {
                treeview_2.collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
                retVal = true; // stop event
            }
            else if (event->key.keyval == GDK_KEY_Right)
            {
                treeview_2.expand_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()), false);
                retVal = true; // stop event
            }
        }
        return retVal;
    });

    pContentArea->show_all();
    std::string expanded_collapsed_string = pCtTreeStore->treeview_get_tree_expanded_collapsed_string(parentTreeView);
    pCtTreeStore->treeview_set_tree_expanded_collapsed_string(expanded_collapsed_string, treeview_2, pCtMainWin->get_ct_config()->nodesBookmExp);
    if (sel_tree_iter)
    {
        Gtk::TreePath sel_path = treeview_2.get_model()->get_path(sel_tree_iter);
        treeview_2.expand_to_path(sel_path);
        treeview_2.set_cursor(sel_path);
        treeview_2.scroll_to_row(sel_path);
    }

    return (dialog.run() == Gtk::RESPONSE_ACCEPT ? treeview_2.get_selection()->get_selected() : Gtk::TreeIter());
}

// Handle the Bookmarks List
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
    Gtk::Button button_move_down;
    button_move_down.set_image_from_icon_name("ct_go-down", Gtk::ICON_SIZE_DND);
    Gtk::Button button_delete;
    button_delete.set_image_from_icon_name("ct_clear", Gtk::ICON_SIZE_DND);
    Gtk::Button button_sort_desc;
    button_sort_desc.set_image_from_icon_name("ct_sort-desc", Gtk::ICON_SIZE_DND);
    Gtk::Button button_sort_asc;
    button_sort_asc.set_image_from_icon_name("ct_sort-asc", Gtk::ICON_SIZE_DND);
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

// Dialog to select a Date
std::time_t CtDialogs::date_select_dialog(Gtk::Window& parent,
                                          const Glib::ustring& title,
                                          const std::time_t& curr_time)
{
    Gtk::Dialog dialog(title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);

    std::tm struct_time = *std::localtime(&curr_time);

    Gtk::Box* pContentArea = dialog.get_content_area();
    Gtk::Calendar calendar;
    calendar.select_month((guint)(struct_time.tm_mon-1), (guint)struct_time.tm_year); // month 0-11
    calendar.select_day((guint)struct_time.tm_mday); // day 1-31
    Glib::RefPtr<Gtk::Adjustment> rAdj_h = Gtk::Adjustment::create(struct_time.tm_hour, 0, 23, 1);
    Gtk::SpinButton spinbutton_h(rAdj_h);
    spinbutton_h.set_value(struct_time.tm_hour);
    Glib::RefPtr<Gtk::Adjustment> rAdj_m = Gtk::Adjustment::create(struct_time.tm_min, 0, 59, 1);
    Gtk::SpinButton spinbutton_m(rAdj_m);
    spinbutton_m.set_value(struct_time.tm_min);
    Gtk::HBox hbox;
    hbox.pack_start(spinbutton_h);
    hbox.pack_start(spinbutton_m);
    pContentArea->pack_start(calendar);
    pContentArea->pack_start(hbox);

    dialog.signal_button_press_event().connect([&dialog](GdkEventButton* event)->bool
    {
        if ( (event->button == 1) &&
             (event->type == GDK_2BUTTON_PRESS) )
        {
            return false;
        }
        dialog.response(Gtk::RESPONSE_ACCEPT);
        return true;
    });
    pContentArea->show_all();

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
    {
        return 0;
    }

    guint new_year, new_month, new_day;
    calendar.get_date(new_year, new_month, new_day);

    std::tm tmtime = {};
    tmtime.tm_year = (int)new_year;
    tmtime.tm_mon = (int)(new_month) + 1;
    tmtime.tm_mday = (int)new_day;
    tmtime.tm_hour = spinbutton_h.get_value_as_int();
    tmtime.tm_min = spinbutton_m.get_value_as_int();

    std::time_t new_time = std::mktime(&tmtime);
    return new_time;
}

void CtDialogs::match_dialog(const Glib::ustring& title,
                             CtMainWin* ctMainWin,
                             Glib::RefPtr<CtMatchDialogStore>& rModel)
{
    // cannot use static because dialog can be used in the several windows
    Gtk::Dialog* pMatchesDialog = new Gtk::Dialog{title, *ctMainWin, Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    pMatchesDialog->set_transient_for(*ctMainWin);
    if (rModel->dlg_size[0] > 0)
    {
        pMatchesDialog->set_default_size(rModel->dlg_size[0], rModel->dlg_size[1]);
        pMatchesDialog->move(rModel->dlg_pos[0], rModel->dlg_pos[1]);
    }
    else
    {
        pMatchesDialog->set_default_size(700, 350);
        pMatchesDialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }
    CtMenuAction* pAction = ctMainWin->get_ct_menu().find_action("toggle_show_allmatches_dlg");
    Glib::ustring label = CtStrUtil::get_accelerator_label(pAction->get_shortcut(ctMainWin->get_ct_config()));
    Gtk::Button* pButtonHide = pMatchesDialog->add_button(str::format(_("Hide (Restore with '%s')"), label), Gtk::RESPONSE_CLOSE);
    pButtonHide->set_image_from_icon_name("ct_close", Gtk::ICON_SIZE_BUTTON);

    Gtk::TreeView* pTreeview = Gtk::manage(new Gtk::TreeView(rModel));
    pTreeview->append_column(_("Node Name"), rModel->columns.node_name);
    pTreeview->append_column(_("Line"), rModel->columns.line_num);
    pTreeview->append_column(_("Line Content"), rModel->columns.line_content);
    pTreeview->append_column("", rModel->columns.node_hier_name);
    pTreeview->get_column(3)->property_visible() = false;
    pTreeview->set_tooltip_column(3);
    Gtk::ScrolledWindow* pScrolledwindowAllmatches = Gtk::manage(new Gtk::ScrolledWindow());
    pScrolledwindowAllmatches->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pScrolledwindowAllmatches->add(*pTreeview);
    Gtk::Box* pContentArea = pMatchesDialog->get_content_area();
    pContentArea->pack_start(*pScrolledwindowAllmatches);

    auto select_found_line = [pTreeview, rModel, ctMainWin]()
    {
        Gtk::TreeIter list_iter = pTreeview->get_selection()->get_selected();
        if (!list_iter)
        {
            return;
        }
        gint64 node_id = list_iter->get_value(rModel->columns.node_id);
        CtTreeIter tree_iter = ctMainWin->get_tree_store().get_node_from_node_id(node_id);
        if (!tree_iter)
        {
            CtDialogs::error_dialog(str::format(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)"), node_id), *ctMainWin);
            rModel->erase(list_iter);
            return;
        }
        // remove previous selection because it can cause freezing in specific cases, see more in issue
        auto fake_iter = ctMainWin->get_text_view().get_buffer()->get_iter_at_offset(-1);
        ctMainWin->get_text_view().get_buffer()->place_cursor(fake_iter);

        ctMainWin->get_tree_view().set_cursor_safe(tree_iter);
        auto rCurrBuffer = ctMainWin->get_text_view().get_buffer();
        rCurrBuffer->select_range(rCurrBuffer->get_iter_at_offset(list_iter->get_value(rModel->columns.start_offset)),
                                  rCurrBuffer->get_iter_at_offset(list_iter->get_value(rModel->columns.end_offset)));
        ctMainWin->get_text_view().scroll_to(rCurrBuffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);

        // pump events so UI's not going to freeze (#835)
        while (gdk_events_pending())
            gtk_main_iteration();
    };

    if (!rModel->saved_path.empty())
    {
        auto iter = rModel->get_iter(rModel->saved_path);
        if (iter) {
            pTreeview->set_cursor(rModel->get_path(iter));
            pTreeview->scroll_to_row(rModel->get_path(iter), 0.5);
            select_found_line();
        }
    }

    pTreeview->signal_cursor_changed().connect(select_found_line);

    auto on_allmatchesdialog_delete_event = [pMatchesDialog, rModel, pTreeview](GdkEventAny* /*any_event*/)->bool{
        pMatchesDialog->get_position(rModel->dlg_pos[0], rModel->dlg_pos[1]);
        pMatchesDialog->get_size(rModel->dlg_size[0], rModel->dlg_size[1]);
        Gtk::TreeIter list_iter = pTreeview->get_selection()->get_selected();
        rModel->saved_path = list_iter ? pTreeview->get_model()->get_path(list_iter).to_string() : "";

        delete pMatchesDialog; // should delete ourselves
        return false;
    };
    pMatchesDialog->signal_delete_event().connect(on_allmatchesdialog_delete_event);
    pButtonHide->signal_clicked().connect([pMatchesDialog]()
    {
        pMatchesDialog->close();
    });

    pMatchesDialog->show_all();
}

// Insert/Edit Anchor Name
Glib::ustring CtDialogs::img_n_entry_dialog(Gtk::Window& parent,
                                            const Glib::ustring& title,
                                            const Glib::ustring& entry_content,
                                            const char* img_stock)
{
    Gtk::Dialog dialog(title,
                       parent,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);
    Gtk::Image image;
    image.set_from_icon_name(img_stock, Gtk::ICON_SIZE_BUTTON);
    Gtk::Entry entry;
    entry.set_text(entry_content);
    Gtk::HBox hbox;
    hbox.pack_start(image, false, false);
    hbox.pack_start(entry);
    hbox.set_spacing(5);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(hbox);
    pContentArea->show_all();
    entry.grab_focus();
    entry.signal_activate().connect([&](){
        if (!entry.get_text().empty())
            dialog.response(Gtk::RESPONSE_ACCEPT);
    });

    return (Gtk::RESPONSE_ACCEPT == dialog.run() ? str::trim(entry.get_text()) : "");
}

// Dialog to Insert/Edit Links
bool CtDialogs::link_handle_dialog(CtMainWin& ctMainWin,
                                   const Glib::ustring& title,
                                   Gtk::TreeIter sel_tree_iter,
                                   CtLinkEntry& link_entries)
{
    if (link_entries.type == "")
        link_entries.type = CtConst::LINK_TYPE_WEBS;

    CtTreeStore& ctTreestore = ctMainWin.get_tree_store();
    Gtk::Dialog dialog(title,
                       ctMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);

    Gtk::HBox hbox_webs;
    Gtk::Image image_webs;
    image_webs.set_from_icon_name("ct_link_website", Gtk::ICON_SIZE_BUTTON);
    Gtk::RadioButton radiobutton_webs(_("To WebSite"));
    Gtk::Entry entry_webs;
    entry_webs.set_text(link_entries.webs);
    hbox_webs.pack_start(image_webs, false, false);
    hbox_webs.pack_start(radiobutton_webs, false, false);
    hbox_webs.pack_start(entry_webs);
    hbox_webs.set_spacing(5);

    Gtk::HBox hbox_file;
    Gtk::Image image_file;
    image_file.set_from_icon_name("ct_file", Gtk::ICON_SIZE_BUTTON);
    Gtk::RadioButton radiobutton_file(_("To File"));
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
    Gtk::RadioButton radiobutton_folder(_("To Folder"));
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
    Gtk::RadioButton radiobutton_node(_("To Node"));
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
    hbox_anchor.pack_start(entry_anchor);
    hbox_anchor.pack_start(button_browse_anchor, false, false);

    Gtk::Frame frame_anchor(Glib::ustring("<b>")+_("Anchor Name (optional)")+"</b>");
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

    auto link_type_changed_on_dialog = [&]()
    {
        entry_webs.set_sensitive(link_entries.type == CtConst::LINK_TYPE_WEBS);
        hbox_detail.set_sensitive(link_entries.type == CtConst::LINK_TYPE_NODE);
        entry_file.set_sensitive(link_entries.type == CtConst::LINK_TYPE_FILE);
        entry_folder.set_sensitive(link_entries.type == CtConst::LINK_TYPE_FOLD);
        if (link_entries.type == CtConst::LINK_TYPE_WEBS)
        {
            entry_webs.grab_focus();
        }
        else if (link_entries.type == CtConst::LINK_TYPE_NODE)
        {
            treeview_2.grab_focus();
            if (first_in)
            {
                first_in = false;
                std::string exp_colpsd_str = ctTreestore.treeview_get_tree_expanded_collapsed_string(ctMainWin.get_tree_view());
                ctTreestore.treeview_set_tree_expanded_collapsed_string(exp_colpsd_str, treeview_2, ctMainWin.get_ct_config()->nodesBookmExp);
            }
            if (!sel_tree_iter)
            {
                sel_tree_iter = ctTreestore.get_iter_first();
            }
            Gtk::TreePath sel_path = ctTreestore.get_path(sel_tree_iter);
            treeview_2.expand_to_path(sel_path);
            treeview_2.set_cursor(sel_path);
            treeview_2.scroll_to_row(sel_path);
        }
        else if (link_entries.type == CtConst::LINK_TYPE_FILE)
        {
            entry_file.grab_focus();
        }
        else
        {
            entry_folder.grab_focus();
        }
    };

    radiobutton_webs.signal_toggled().connect([&]()
    {
        if (radiobutton_webs.get_active())
        {
            link_entries.type = CtConst::LINK_TYPE_WEBS;
        }
        link_type_changed_on_dialog();
    });
    entry_webs.signal_activate().connect([&](){
        if (!str::trim(entry_webs.get_text()).empty())
            dialog.response(Gtk::RESPONSE_ACCEPT);
    });
    radiobutton_node.signal_toggled().connect([&]()
    {
        if (radiobutton_node.get_active())
        {
            link_entries.type = CtConst::LINK_TYPE_NODE;
        }
        link_type_changed_on_dialog();
    });
    radiobutton_file.signal_toggled().connect([&]()
    {
        if (radiobutton_file.get_active())
        {
            link_entries.type = CtConst::LINK_TYPE_FILE;
        }
        link_type_changed_on_dialog();
    });
    entry_file.signal_activate().connect([&](){
        if (!str::trim(entry_file.get_text()).empty())
            dialog.response(Gtk::RESPONSE_ACCEPT);
    });
    radiobutton_folder.signal_toggled().connect([&]()
    {
        if (radiobutton_folder.get_active())
        {
            link_entries.type = CtConst::LINK_TYPE_FOLD;
        }
        link_type_changed_on_dialog();
    });
    entry_folder.signal_activate().connect([&](){
        if (!str::trim(entry_folder.get_text()).empty())
            dialog.response(Gtk::RESPONSE_ACCEPT);
    });
    button_browse_file.signal_clicked().connect([&]()
    {
        file_select_args args(&dialog);
        args.curr_folder=ctMainWin.get_ct_config()->pickDirFile;
        std::string filepath = file_select_dialog(args);
        if (filepath.empty())
        {
            return;
        }
        ctMainWin.get_ct_config()->pickDirFile = Glib::path_get_dirname(filepath);
        if (ctMainWin.get_ct_config()->linksRelative)
        {
            filepath = fs::relative(filepath, ctMainWin.get_ct_storage()->get_file_dir()).string();
        }
        entry_file.set_text(filepath);
    });
    button_browse_folder.signal_clicked().connect([&]()
    {
        std::string filepath = CtDialogs::folder_select_dialog(ctMainWin.get_ct_config()->pickDirFile, &dialog);
        if (filepath.empty())
        {
            return;
        }
        ctMainWin.get_ct_config()->pickDirFile = filepath;
        if (ctMainWin.get_ct_config()->linksRelative)
        {
            filepath = fs::relative(filepath, ctMainWin.get_ct_storage()->get_file_dir()).string();
        }
        entry_folder.set_text(filepath);
    });
    button_browse_anchor.signal_clicked().connect([&]()
    {
        if (!sel_tree_iter)
        {
            CtDialogs::warning_dialog(_("No Node is Selected"), dialog);
            return;
        }
        CtTreeIter ctTreeIter = ctTreestore.to_ct_tree_iter(sel_tree_iter);
        std::list<Glib::ustring> anchors_list;
        for (CtAnchoredWidget* pAnchoredWidget : ctTreeIter.get_embedded_pixbufs_tables_codeboxes_fast())
        {
            if (CtAnchWidgType::ImageAnchor == pAnchoredWidget->get_type())
            {
                anchors_list.push_back(dynamic_cast<CtImageAnchor*>(pAnchoredWidget)->get_anchor_name());
            }
        }
        if (anchors_list.empty())
        {
            info_dialog(_("There are No Anchors in the Selected Node"), dialog);
        }
        else
        {
            Glib::RefPtr<CtChooseDialogListStore> rItemStore = CtChooseDialogListStore::create();
            for (const Glib::ustring& anchName : anchors_list)
            {
                rItemStore->add_row("", "", anchName);
            }
            Gtk::TreeIter res = CtDialogs::choose_item_dialog(dialog, _("Choose Existing Anchor"), rItemStore, _("Anchor Name"));
            if (res)
            {
                Glib::ustring anchName = res->get_value(rItemStore->columns.desc);
                entry_anchor.set_text(anchName);
            }
        }
    });
    treeview_2.signal_event_after().connect([&](GdkEvent* event)
    {
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
            if (treeview_2.get_path_at_pos((int)event->button.x, (int)event->button.y, path_at_click))
            {
                if (treeview_2.row_expanded(path_at_click))
                    treeview_2.collapse_row(path_at_click);
                else
                    treeview_2.expand_row(path_at_click, true);
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
        else if (event->type == GDK_KEY_PRESS && sel_tree_iter)
        {
            Gtk::TreePath path = ctTreestore.get_path(sel_tree_iter);
            if (event->key.keyval == GDK_KEY_Left)
                treeview_2.collapse_row(path);
            else if (event->key.keyval == GDK_KEY_Right)
                treeview_2.expand_row(path, false);
        }
    });
    dialog.signal_key_press_event().connect([&](GdkEventKey* event)
    {
        if (event->keyval == GDK_KEY_Tab)
        {
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

    if (dialog.run() != GTK_RESPONSE_ACCEPT)
    {
        return false;
    }

    link_entries.webs = str::trim(entry_webs.get_text());
    link_entries.file = str::trim(entry_file.get_text());
    link_entries.fold = str::trim(entry_folder.get_text());
    link_entries.anch = str::trim(entry_anchor.get_text());
    link_entries.node_id = ctTreestore.to_ct_tree_iter(sel_tree_iter).get_node_id();
    return true;
}

// The Select file dialog, Returns the retrieved filepath or None
std::string CtDialogs::file_select_dialog(const file_select_args& args)
{
    auto chooser = Gtk::FileChooserNative::create(_("Select File"), *args.pParentWin, Gtk::FILE_CHOOSER_ACTION_OPEN);
    if (args.curr_folder.empty() || !fs::is_directory(args.curr_folder))
    {
        chooser->set_current_folder(g_get_home_dir());
    }
    else
    {
        chooser->set_current_folder(args.curr_folder.string());
    }
    if (!args.filter_pattern.empty())
    {
        Glib::RefPtr<Gtk::FileFilter> rFileFilter = Gtk::FileFilter::create();
        rFileFilter->set_name(args.filter_name);
        for (const std::string& element : args.filter_pattern)
        {
            rFileFilter->add_pattern(element);
        }
        chooser->add_filter(rFileFilter);
    }
    return (chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "");
}

// The Select folder dialog, returns the retrieved folderpath or None
std::string CtDialogs::folder_select_dialog(const std::string& curr_folder, Gtk::Window* pParentWin)
{
    auto chooser = Gtk::FileChooserNative::create(_("Select Folder"), *pParentWin, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    if (curr_folder.empty() || !Glib::file_test(curr_folder, Glib::FILE_TEST_IS_DIR))
    {
        chooser->set_current_folder(g_get_home_dir());
    }
    else
    {
        chooser->set_current_folder(curr_folder);
    }
    return (chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "");
}

// The Save file as dialog, Returns the retrieved filepath or None
std::string CtDialogs::file_save_as_dialog(const file_select_args& args)
{
    auto chooser = Gtk::FileChooserNative::create(_("Save File as"), *args.pParentWin, Gtk::FILE_CHOOSER_ACTION_SAVE);
    chooser->set_do_overwrite_confirmation(true);
    if (args.curr_folder.empty() || !fs::is_directory(args.curr_folder))
    {
        chooser->set_current_folder(g_get_home_dir());
    }
    else
    {
        chooser->set_current_folder(args.curr_folder.string());
    }
    if (!args.curr_file_name.empty())
    {
        chooser->set_current_name(args.curr_file_name.string());
    }
    if (!args.filter_pattern.empty())
    {
        Glib::RefPtr<Gtk::FileFilter> rFileFilter = Gtk::FileFilter::create();
        rFileFilter->set_name(args.filter_name);
        for (const std::string& element : args.filter_pattern)
        {
            rFileFilter->add_pattern(element);
        }
        chooser->add_filter(rFileFilter);
    }
    return (chooser->run() == Gtk::RESPONSE_ACCEPT ? chooser->get_filename() : "");
}

// Insert/Edit Image
Glib::RefPtr<Gdk::Pixbuf> CtDialogs::image_handle_dialog(Gtk::Window& parent_win,
                                                         const Glib::ustring& title,
                                                         Glib::RefPtr<Gdk::Pixbuf> rOriginalPixbuf)
{
    int width = rOriginalPixbuf->get_width();
    int height = rOriginalPixbuf->get_height();
    double image_w_h_ration = double(width)/height;

    Gtk::Dialog dialog(title,
                       parent_win,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    Gtk::Button* pOK_button = dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);
    Gtk::Button button_rotate_90_ccw;
    button_rotate_90_ccw.set_image_from_icon_name("ct_rotate-left", Gtk::ICON_SIZE_DND);
    Gtk::Button button_rotate_90_cw;
    button_rotate_90_cw.set_image_from_icon_name("ct_rotate-right", Gtk::ICON_SIZE_DND);
    Gtk::ScrolledWindow scrolledwindow;
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Glib::RefPtr<Gtk::Adjustment> rHAdj = Gtk::Adjustment::create(width, 1, height, 1);
    Glib::RefPtr<Gtk::Adjustment> rVAdj = Gtk::Adjustment::create(width, 1, width, 1);
    Gtk::Viewport viewport(rHAdj, rVAdj);
    Gtk::Image image(rOriginalPixbuf);
    scrolledwindow.add(viewport);
    viewport.add(image);
    Gtk::HBox hbox_1;
    hbox_1.pack_start(button_rotate_90_ccw, false, false);
    hbox_1.pack_start(scrolledwindow);
    hbox_1.pack_start(button_rotate_90_cw, false, false);
    hbox_1.set_spacing(2);
    Gtk::Label label_width(_("Width"));
    Glib::RefPtr<Gtk::Adjustment> rAdj_width = Gtk::Adjustment::create(width, 1, 10000, 1);
    Gtk::SpinButton spinbutton_width(rAdj_width);
    Gtk::Label label_height(_("Height"));
    Glib::RefPtr<Gtk::Adjustment> rAdj_height = Gtk::Adjustment::create(height, 1, 10000, 1);
    Gtk::SpinButton spinbutton_height(rAdj_height);
    Gtk::HBox hbox_2;
    hbox_2.pack_start(label_width);
    hbox_2.pack_start(spinbutton_width);
    hbox_2.pack_start(label_height);
    hbox_2.pack_start(spinbutton_height);
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(hbox_1);
    pContentArea->pack_start(hbox_2, false, false);
    pContentArea->set_spacing(6);

    bool stop_update = false;
    auto image_load_into_dialog = [&]()
    {
        stop_update = true;
        spinbutton_width.set_value(width);
        spinbutton_height.set_value(height);
        Glib::RefPtr<Gdk::Pixbuf> rPixbuf;
        if (width <= 900 && height <= 600)
        {
            // original size into the dialog
            rPixbuf = rOriginalPixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR);
        }
        else
        {
            // reduced size visible into the dialog
            if (width > 900)
            {
                int img_parms_width = 900;
                int img_parms_height = (int)(img_parms_width / image_w_h_ration);
                rPixbuf = rOriginalPixbuf->scale_simple(img_parms_width, img_parms_height, Gdk::INTERP_BILINEAR);
            }
            else
            {
                int img_parms_height = 600;
                int img_parms_width = (int)(img_parms_height * image_w_h_ration);
                rPixbuf = rOriginalPixbuf->scale_simple(img_parms_width, img_parms_height, Gdk::INTERP_BILINEAR);
            }
        }
        image.set(rPixbuf);
        stop_update = false;
    };
    button_rotate_90_cw.signal_clicked().connect([&]()
    {
        rOriginalPixbuf = rOriginalPixbuf->rotate_simple(Gdk::PixbufRotation::PIXBUF_ROTATE_CLOCKWISE);
        image_w_h_ration = 1./image_w_h_ration;
        std::swap(width, height); // new width is the former height and vice versa
        image_load_into_dialog();
    });
    button_rotate_90_ccw.signal_clicked().connect([&]()
    {
        rOriginalPixbuf = rOriginalPixbuf->rotate_simple(Gdk::PixbufRotation::PIXBUF_ROTATE_COUNTERCLOCKWISE);
        image_w_h_ration = 1./image_w_h_ration;
        std::swap(width, height); // new width is the former height and vice versa
        image_load_into_dialog();
    });
    spinbutton_width.signal_value_changed().connect([&]()
    {
        if (stop_update) return;
        width = spinbutton_width.get_value_as_int();
        height = (int)(width/image_w_h_ration);
        image_load_into_dialog();
    });
    spinbutton_height.signal_value_changed().connect([&]()
    {
        if (stop_update) return;
        height = spinbutton_height.get_value_as_int();
        width = (int)(height*image_w_h_ration);
        image_load_into_dialog();
    });
    image_load_into_dialog();
    pContentArea->show_all();
    pOK_button->grab_focus();

    return (Gtk::RESPONSE_ACCEPT == dialog.run() ? rOriginalPixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR) : Glib::RefPtr<Gdk::Pixbuf>());
}

// Opens the CodeBox Handle Dialog
bool CtDialogs::codeboxhandle_dialog(CtMainWin* pCtMainWin,
                                     const Glib::ustring& title)
{
    Gtk::Dialog dialog(title,
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(300, -1);
    dialog.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);

    CtConfig* pConfig = pCtMainWin->get_ct_config();

    Gtk::Button button_prog_lang;
    const Glib::ustring syntax_hl_id = (pConfig->codeboxSynHighl != CtConst::PLAIN_TEXT_ID ? pConfig->codeboxSynHighl : pConfig->autoSynHighl);
    const std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
    button_prog_lang.set_label(syntax_hl_id);
    button_prog_lang.set_image(*pCtMainWin->new_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
    Gtk::RadioButton radiobutton_plain_text(_("Plain Text"));
    Gtk::RadioButton radiobutton_auto_syntax_highl(_("Automatic Syntax Highlighting"));
    radiobutton_auto_syntax_highl.join_group(radiobutton_plain_text);
    if (pConfig->codeboxSynHighl == CtConst::PLAIN_TEXT_ID)
    {
        radiobutton_plain_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    }
    else
    {
        radiobutton_auto_syntax_highl.set_active(true);
    }
    Gtk::VBox type_vbox;
    type_vbox.pack_start(radiobutton_plain_text);
    type_vbox.pack_start(radiobutton_auto_syntax_highl);
    type_vbox.pack_start(button_prog_lang);
    Gtk::Frame type_frame(Glib::ustring("<b>")+_("Type")+"</b>");
    dynamic_cast<Gtk::Label*>(type_frame.get_label_widget())->set_use_markup(true);
    type_frame.set_shadow_type(Gtk::SHADOW_NONE);
    type_frame.add(type_vbox);

    Gtk::Label label_width(_("Width"));
    Glib::RefPtr<Gtk::Adjustment> rAdj_width = Gtk::Adjustment::create(pConfig->codeboxWidth, 1, 10000);
    Gtk::SpinButton spinbutton_width(rAdj_width);
    spinbutton_width.set_value(pConfig->codeboxWidth);
    Gtk::Label label_height(_("Height"));
    Glib::RefPtr<Gtk::Adjustment> rAdj_height = Gtk::Adjustment::create(pConfig->codeboxHeight, 1, 10000);
    Gtk::SpinButton spinbutton_height(rAdj_height);
    spinbutton_height.set_value(pConfig->codeboxHeight);

    Gtk::RadioButton radiobutton_codebox_pixels(_("pixels"));
    Gtk::RadioButton radiobutton_codebox_percent("%");
    radiobutton_codebox_percent.join_group(radiobutton_codebox_pixels);
    radiobutton_codebox_pixels.set_active(pConfig->codeboxWidthPixels);
    radiobutton_codebox_percent.set_active(!pConfig->codeboxWidthPixels);

    Gtk::VBox vbox_pix_perc;
    vbox_pix_perc.pack_start(radiobutton_codebox_pixels);
    vbox_pix_perc.pack_start(radiobutton_codebox_percent);
    Gtk::HBox hbox_width;
    hbox_width.pack_start(label_width, false, false);
    hbox_width.pack_start(spinbutton_width, false, false);
    hbox_width.pack_start(vbox_pix_perc);
    hbox_width.set_spacing(5);
    Gtk::HBox hbox_height;
    hbox_height.pack_start(label_height, false, false);
    hbox_height.pack_start(spinbutton_height, false, false);
    hbox_height.set_spacing(5);
    Gtk::VBox vbox_size;
    vbox_size.pack_start(hbox_width);
    vbox_size.pack_start(hbox_height);
    Gtk::Alignment size_align;
    size_align.set_padding(0, 6, 6, 6);
    size_align.add(vbox_size);

    Gtk::Frame size_frame(Glib::ustring("<b>")+_("Size")+"</b>");
    dynamic_cast<Gtk::Label*>(size_frame.get_label_widget())->set_use_markup(true);
    size_frame.set_shadow_type(Gtk::SHADOW_NONE);
    size_frame.add(size_align);

    Gtk::CheckButton checkbutton_codebox_linenumbers(_("Show Line Numbers"));
    checkbutton_codebox_linenumbers.set_active(pConfig->codeboxLineNum);
    Gtk::CheckButton checkbutton_codebox_matchbrackets(_("Highlight Matching Brackets"));
    checkbutton_codebox_matchbrackets.set_active(pConfig->codeboxMatchBra);
    Gtk::VBox vbox_options;
    vbox_options.pack_start(checkbutton_codebox_linenumbers);
    vbox_options.pack_start(checkbutton_codebox_matchbrackets);
    Gtk::Alignment opt_align;
    opt_align.set_padding(6, 6, 6, 6);
    opt_align.add(vbox_options);

    Gtk::Frame options_frame(Glib::ustring("<b>")+_("Options")+"</b>");
    dynamic_cast<Gtk::Label*>(options_frame.get_label_widget())->set_use_markup(true);
    options_frame.set_shadow_type(Gtk::SHADOW_NONE);
    options_frame.add(opt_align);

    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->set_spacing(5);
    pContentArea->pack_start(type_frame);
    pContentArea->pack_start(size_frame);
    pContentArea->pack_start(options_frame);
    pContentArea->show_all();

    button_prog_lang.signal_clicked().connect([&button_prog_lang, &dialog, pCtMainWin]()
    {
        Glib::RefPtr<CtChooseDialogListStore> rItemStore = CtChooseDialogListStore::create();
        for (const std::string& lang : pCtMainWin->get_language_manager()->get_language_ids())
        {
            rItemStore->add_row(pCtMainWin->get_code_icon_name(lang), "", lang);
        }
        Gtk::TreeIter res = CtDialogs::choose_item_dialog(dialog, _("Automatic Syntax Highlighting"), rItemStore);
        if (res)
        {
            const Glib::ustring syntax_hl_id = res->get_value(rItemStore->columns.desc);
            const std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
            button_prog_lang.set_label(syntax_hl_id);
            button_prog_lang.set_image(*pCtMainWin->new_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
        }
    });
    radiobutton_auto_syntax_highl.signal_toggled().connect([&radiobutton_auto_syntax_highl, &button_prog_lang]()
    {
        button_prog_lang.set_sensitive(radiobutton_auto_syntax_highl.get_active());
    });
    dialog.signal_key_press_event().connect([&](GdkEventKey* key)
    {
        if (key->keyval == GDK_KEY_Return)
        {
            spinbutton_width.update();
            spinbutton_height.update();
            dialog.response(Gtk::RESPONSE_ACCEPT);
            return true;
        }
        return false;
    });
    radiobutton_codebox_pixels.signal_toggled().connect([&radiobutton_codebox_pixels, &spinbutton_width]()
    {
        if (radiobutton_codebox_pixels.get_active())
        {
            spinbutton_width.set_value(700);
        }
        else if (spinbutton_width.get_value() > 100)
        {
            spinbutton_width.set_value(90);
        }
    });
    auto on_key_press_dialog = [&](GdkEventKey *pEventKey)->bool
    {
        if (GDK_KEY_Return == pEventKey->keyval) {
            Gtk::Button *pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    const int response = dialog.run();
    dialog.hide();

    if (response == Gtk::RESPONSE_ACCEPT)
    {
        pConfig->codeboxWidth = spinbutton_width.get_value_as_int();
        pConfig->codeboxWidthPixels = radiobutton_codebox_pixels.get_active();
        pConfig->codeboxHeight = spinbutton_height.get_value();
        pConfig->codeboxLineNum = checkbutton_codebox_linenumbers.get_active();
        pConfig->codeboxMatchBra = checkbutton_codebox_matchbrackets.get_active();
        if (radiobutton_plain_text.get_active())
        {
            pConfig->codeboxSynHighl = CtConst::PLAIN_TEXT_ID;
        }
        else
        {
            pConfig->codeboxSynHighl = button_prog_lang.get_label();
        }
        return true;
    }
    return false;
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
    auto on_key_press_edit_data_storage_type_dialog = [&](GdkEventKey *pEventKey)->bool
    {
        if (GDK_KEY_Return == pEventKey->keyval)
        {
            Gtk::Button *pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
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

bool CtDialogs::node_prop_dialog(const Glib::ustring &title,
                                 CtMainWin* pCtMainWin,
                                 CtNodeData& nodeData,
                                 const std::set<Glib::ustring>& tags_set)
{
    Gtk::Dialog dialog = Gtk::Dialog(title,
                                     *pCtMainWin,
                                     Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(300, -1);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    Gtk::Entry name_entry;
    name_entry.set_text(nodeData.name);

    auto* grid_icons = Gtk::manage(new Gtk::Grid());
    grid_icons->set_row_spacing(0);
    grid_icons->set_column_spacing(0);
    grid_icons->set_row_homogeneous(true);

    Gtk::CheckButton is_bold_checkbutton(_("Bold"));
    is_bold_checkbutton.set_active(nodeData.isBold);
    is_bold_checkbutton.set_margin_top(4);

    Gtk::CheckButton fg_checkbutton(_("Use Selected Color"));
    fg_checkbutton.set_active(not nodeData.foregroundRgb24.empty());
    Glib::ustring real_fg = not nodeData.foregroundRgb24.empty() ? nodeData.foregroundRgb24 : (not pCtMainWin->get_ct_config()->currColors.at('n').empty() ? pCtMainWin->get_ct_config()->currColors.at('n').c_str() : "red");
    Gtk::ColorButton fg_colorbutton{Gdk::RGBA(real_fg)};
    fg_colorbutton.set_sensitive(not nodeData.foregroundRgb24.empty());

    Gtk::CheckButton c_icon_checkbutton(_("Use Selected Icon"));
    c_icon_checkbutton.set_active(nodeData.customIconId > 0 && nodeData.customIconId < CtConst::NODE_CUSTOM_ICONS.size());
    Gtk::Button c_icon_button;
    if (c_icon_checkbutton.get_active())
    {
        c_icon_button.set_image(*pCtMainWin->new_image_from_stock(CtConst::NODE_CUSTOM_ICONS.at((int)nodeData.customIconId), Gtk::ICON_SIZE_BUTTON));
    }
    else
    {
        c_icon_button.set_label(_("click me"));
        c_icon_button.set_sensitive(false);
    }

    grid_icons->attach(fg_checkbutton, 0, 1, 1, 1);
    grid_icons->attach(fg_colorbutton, 1, 1, 1, 1);
    grid_icons->attach(c_icon_checkbutton, 0, 2, 1, 1);
    grid_icons->attach(c_icon_button, 1, 2, 1, 1);

    Gtk::VBox name_vbox;
    name_vbox.pack_start(name_entry);
    name_vbox.pack_start(is_bold_checkbutton);
    name_vbox.pack_start(*grid_icons);
    Gtk::Frame name_frame(Glib::ustring("<b>")+_("Node Name")+"</b>");
    ((Gtk::Label*)name_frame.get_label_widget())->set_use_markup(true);
    name_frame.set_shadow_type(Gtk::SHADOW_NONE);
    name_frame.add(name_vbox);
    Gtk::RadioButton radiobutton_rich_text(_("Rich Text"));
    Gtk::RadioButton::Group rbGroup = radiobutton_rich_text.get_group();
    Gtk::RadioButton radiobutton_plain_text(rbGroup, _("Plain Text"));
    Gtk::RadioButton radiobutton_auto_syntax_highl(rbGroup, _("Automatic Syntax Highlighting"));
    Gtk::Button button_prog_lang;
    std::string syntax_hl_id = nodeData.syntax;
    if (nodeData.syntax == CtConst::RICH_TEXT_ID || nodeData.syntax == CtConst::PLAIN_TEXT_ID)
    {
        syntax_hl_id = pCtMainWin->get_ct_config()->autoSynHighl;
    }
    std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
    button_prog_lang.set_label(syntax_hl_id);
    button_prog_lang.set_image(*pCtMainWin->new_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
    if (nodeData.syntax == CtConst::RICH_TEXT_ID)
    {
        radiobutton_rich_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    }
    else if (nodeData.syntax == CtConst::PLAIN_TEXT_ID)
    {
        radiobutton_plain_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    }
    else
    {
        radiobutton_auto_syntax_highl.set_active(true);
    }
    Gtk::VBox type_vbox;
    type_vbox.pack_start(radiobutton_rich_text);
    type_vbox.pack_start(radiobutton_plain_text);
    type_vbox.pack_start(radiobutton_auto_syntax_highl);
    type_vbox.pack_start(button_prog_lang);
    Gtk::Frame type_frame(Glib::ustring("<b>")+_("Node Type")+"</b>");
    static_cast<Gtk::Label*>(type_frame.get_label_widget())->set_use_markup(true);
    type_frame.set_shadow_type(Gtk::SHADOW_NONE);
    type_frame.add(type_vbox);
    type_frame.set_sensitive(!nodeData.isRO);
    Gtk::HBox tags_hbox;
    tags_hbox.set_spacing(2);
    Gtk::Entry tags_entry;
    tags_entry.set_text(nodeData.tags);
    Gtk::Button button_browse_tags;
    button_browse_tags.set_image(*pCtMainWin->new_image_from_stock("ct_find", Gtk::ICON_SIZE_BUTTON));
    button_browse_tags.set_sensitive(!tags_set.empty());
    tags_hbox.pack_start(tags_entry);
    tags_hbox.pack_start(button_browse_tags, false, false);
    Gtk::Frame tags_frame(Glib::ustring("<b>")+_("Tags for Searching")+"</b>");
    ((Gtk::Label*)tags_frame.get_label_widget())->set_use_markup(true);
    tags_frame.set_shadow_type(Gtk::SHADOW_NONE);
    tags_frame.add(tags_hbox);
    Gtk::CheckButton ro_checkbutton(_("Read Only"));
    ro_checkbutton.set_active(nodeData.isRO);

    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->set_spacing(5);
    pContentArea->pack_start(name_frame);
    pContentArea->pack_start(type_frame);
    pContentArea->pack_start(tags_frame);
    pContentArea->pack_start(ro_checkbutton);
    pContentArea->show_all();
    name_entry.grab_focus();

    button_prog_lang.signal_clicked().connect([&dialog, &pCtMainWin, &button_prog_lang]()
    {
        auto itemStore = CtChooseDialogListStore::create();
        for (auto lang : pCtMainWin->get_language_manager()->get_language_ids())
        {
            itemStore->add_row(pCtMainWin->get_code_icon_name(lang), "", lang);
        }
        const Gtk::TreeIter treeIter = CtDialogs::choose_item_dialog(dialog, _("Automatic Syntax Highlighting"), itemStore);
        if (treeIter)
        {
            const Glib::ustring syntax_hl_id = treeIter->get_value(itemStore->columns.desc);
            const std::string stock_id = pCtMainWin->get_code_icon_name(syntax_hl_id);
            button_prog_lang.set_label(syntax_hl_id);
            button_prog_lang.set_image(*pCtMainWin->new_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
        }
    });
    radiobutton_auto_syntax_highl.signal_toggled().connect([&radiobutton_auto_syntax_highl, &button_prog_lang]()
    {
       button_prog_lang.set_sensitive(radiobutton_auto_syntax_highl.get_active());
    });
    button_browse_tags.signal_clicked().connect([&dialog, &tags_entry, &tags_set]()
    {
        auto itemStore = CtChooseDialogListStore::create();
        for (const auto& tag : tags_set)
        {
            itemStore->add_row("", "", tag);
        }
        const Gtk::TreeIter treeIter = CtDialogs::choose_item_dialog(dialog, _("Choose Existing Tag"), itemStore, _("Tag Name"));
        if (treeIter)
        {
            std::string cur_tag = tags_entry.get_text();
            if  (str::endswith(cur_tag, CtConst::CHAR_SPACE))
            {
                tags_entry.set_text(cur_tag + treeIter->get_value(itemStore->columns.desc));
            }
            else
            {
                tags_entry.set_text(cur_tag + CtConst::CHAR_SPACE + treeIter->get_value(itemStore->columns.desc));
            }
        }
    });
    ro_checkbutton.signal_toggled().connect([&ro_checkbutton, &type_frame]()
    {
        type_frame.set_sensitive(!ro_checkbutton.get_active());
    });
    fg_checkbutton.signal_toggled().connect([&]()
    {
        fg_colorbutton.set_sensitive(fg_checkbutton.get_active());
    });
    fg_colorbutton.signal_pressed().connect([&pCtMainWin, &fg_colorbutton]()
    {
        Gdk::RGBA ret_color = fg_colorbutton.get_rgba();
        if (CtDialogs::color_pick_dialog(pCtMainWin, _("Pick a Color"), ret_color, false) == CtPickDlgState::SELECTED)
        {
            fg_colorbutton.set_rgba(ret_color);
        }
    });
    c_icon_checkbutton.signal_toggled().connect([&c_icon_checkbutton, &c_icon_button]()
    {
        c_icon_button.set_sensitive(c_icon_checkbutton.get_active());
    });
    c_icon_button.signal_clicked().connect([&dialog, &pCtMainWin, &c_icon_button, &nodeData]()
    {
        auto itemStore = CtChooseDialogListStore::create();
        for (size_t i = 1 /*skip 0*/; i < CtConst::NODE_CUSTOM_ICONS.size(); ++i)
        {
            itemStore->add_row(CtConst::NODE_CUSTOM_ICONS[i], std::to_string(i), "");
        }
        const Gtk::TreeIter treeIter = CtDialogs::choose_item_dialog(dialog, _("Select Node Icon"), itemStore);
        if (treeIter)
        {
            nodeData.customIconId = static_cast<guint32>(std::stoi(treeIter->get_value(itemStore->columns.key)));
            c_icon_button.set_label("");
            c_icon_button.set_image(*pCtMainWin->new_image_from_stock(treeIter->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
        }
    });
    auto on_key_press_edit_data_storage_type_dialog = [&](GdkEventKey *pEventKey)->bool
    {
        if (GDK_KEY_Return == pEventKey->keyval)
        {
            Gtk::Button *pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_edit_data_storage_type_dialog, false/*call me before other*/);

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
    {
        return false;
    }

    nodeData.name = str::trim(name_entry.get_text());
    nodeData.name = str::replace(nodeData.name, "\r", ""); // the given name can contain \r and \n as a result
    nodeData.name = str::replace(nodeData.name, "\n", ""); // of cope/paste from some source
    nodeData.name = str::replace(nodeData.name, "\t", " ");

    if (nodeData.name.empty())
    {
        nodeData.name = CtConst::CHAR_QUESTION;
    }
    if (radiobutton_rich_text.get_active())
    {
        nodeData.syntax = CtConst::RICH_TEXT_ID;
    }
    else if (radiobutton_plain_text.get_active())
    {
        nodeData.syntax = CtConst::PLAIN_TEXT_ID;
    }
    else
    {
        nodeData.syntax = button_prog_lang.get_label();
        pCtMainWin->get_ct_config()->autoSynHighl = nodeData.syntax;
    }
    nodeData.tags = tags_entry.get_text();
    nodeData.isRO = ro_checkbutton.get_active();
    nodeData.customIconId = c_icon_checkbutton.get_active() ? nodeData.customIconId : 0;
    nodeData.isBold = is_bold_checkbutton.get_active();
    if (fg_checkbutton.get_active())
    {
        nodeData.foregroundRgb24 = CtRgbUtil::get_rgb24str_from_str_any(fg_colorbutton.get_color().to_string());
        pCtMainWin->get_ct_config()->currColors['n'] = nodeData.foregroundRgb24;
    }
    else
    {
        nodeData.foregroundRgb24.clear();
    }
    return true;
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
    auto on_key_press_exit_dialog = [&](GdkEventKey *pEventKey)->bool
    {
        if (GDK_KEY_Return == pEventKey->keyval)
        {
            Gtk::Button *pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_YES));
            pButton->clicked();
            return true;
        }
        if (GDK_KEY_Escape == pEventKey->keyval)
        {
            Gtk::Button *pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_CANCEL));
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_exit_dialog, false/*call me before other*/);
    pContentArea->show_all();
    const int response = dialog.run();
    dialog.hide();
    if (Gtk::RESPONSE_YES == response)
    {
        return CtYesNoCancel::Yes;
    }
    if (Gtk::RESPONSE_NO == response)
    {
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
    dialog.set_copyright("Copyright © 2009-2020\n"
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
    dialog.set_artists({"OCAL <http://www.openclipart.org/>", "Zeltak <zeltak@gmail.com>", "Angelo Penone <angelo.penone@gmail.com>"});
    dialog.set_translator_credits(Glib::ustring{} +
 _("Armenian")+" (hy) Seda Stamboltsyan <sedastam@yandex.com>"+CtConst::CHAR_NEWLINE+
 _("Chinese Simplified")+" (zh_CN) Channing Wong <channing.wong@qq.com>"+CtConst::CHAR_NEWLINE+
 _("Czech")+" (cs) Pavel Fric <fripohled@blogspot.com>"+CtConst::CHAR_NEWLINE+
 _("Dutch")+" (nl) Luuk Geurts, Patrick Vijgeboom <pj.vijgeboom@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Finnish")+" (fi) Henri Kaustinen <hendrix.ks81@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("French")+" (fr) Klaus Becker <colonius@free.fr>"+CtConst::CHAR_NEWLINE+
 _("German")+" (de) Matthias Hoffmann <MHoffi@web.de>"+CtConst::CHAR_NEWLINE+
 _("Greek")+" (el) Delphina <delphina.2009@yahoo.gr>"+CtConst::CHAR_NEWLINE+
 _("Italian")+" (it) Vincenzo Reale <smart2128@baslug.org>"+CtConst::CHAR_NEWLINE+
 _("Japanese")+" (ja) Piyo <py2@live.jp>"+CtConst::CHAR_NEWLINE+
 _("Lithuanian")+" (lt) Zygis <zygimantus@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Polish")+" (pl) Marcin Swierczynski <orneo1212@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Portuguese Brazil")+" (pt_BR) Vinicius Schmidt <viniciussm@rocketmail.com>"+CtConst::CHAR_NEWLINE+
 _("Russian")+" (ru) Andriy Kovtun <kovtunos@yandex.ru>"+CtConst::CHAR_NEWLINE+
 _("Slovenian")+" (sl) Erik Lovrič <erik.lovric@gmail.com>"+CtConst::CHAR_NEWLINE+
 _("Spanish")+" (es) Daniel MC <i.e.betel@gmail.com>"+CtConst::CHAR_NEWLINE+
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

std::string CtDialogs::dialog_palette(CtMainWin* pCtMainWin)
{
    // based on plotinus
    struct CtPaletteColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<int>           order;
        Gtk::TreeModelColumn<Glib::ustring> id;
        Gtk::TreeModelColumn<Glib::ustring> path;
        Gtk::TreeModelColumn<Glib::ustring> label;
        Gtk::TreeModelColumn<Glib::ustring> accelerator;
        CtPaletteColumns() { add(order); add(id); add(path); add(label); add(accelerator); }
    } columns;


    Glib::ustring filter;
    std::vector<Glib::ustring> filter_words;

    auto get_command_score = [&](const Gtk::TreeIter& iter) -> int {
        auto label = iter->get_value(columns.label).lowercase();
        auto path = iter->get_value(columns.path).lowercase();
        int score = 0;
        if (str::startswith(label, filter)) return score;
        score++;
        if (label.find(filter) != Glib::ustring::npos) return score;
        score++;
        if (CtStrUtil::contains_words(label, filter_words)) return score;
        score++;
        if (CtStrUtil::contains_words(label, filter_words, false)) return score;
        score++;
        if (CtStrUtil::contains_words(path, filter_words)) return score;
        score++;
        if (CtStrUtil::contains_words(path, filter_words, false)) return score;
         return -1;
    };

    auto list_store = Gtk::ListStore::create(columns);
    int order_cnt = 0;
    for (auto& action: pCtMainWin->get_ct_menu().get_actions())
    {
        if (action.category.empty()) continue;
        auto& iter = *list_store->append();
        iter[columns.order] = ++order_cnt;
        iter[columns.id] = action.id;
        iter[columns.path] = action.category;
        iter[columns.label] = str::replace(action.name, "_", "");
        iter[columns.accelerator] = action.get_shortcut(pCtMainWin->get_ct_config());
    }

    auto tree_model_filter = Gtk::TreeModelFilter::create(list_store);
    tree_model_filter->set_visible_func([&](const Gtk::TreeIter& iter) -> bool {
        if (filter.empty()) return true;
        return get_command_score(iter) >= 0;
    });
    auto tree_model_sort = Gtk::TreeModelSort::create(tree_model_filter);
    auto tree_view = Gtk::TreeView();
    tree_view.set_model(tree_model_sort);
    tree_view.set_headers_visible(false);

    // The theme's style context is reliably available only after the widget has been realized
    tree_view.signal_realize().connect([&](){
        auto style_context = tree_view.get_style_context();
        auto text_color = style_context->get_color(Gtk::StateFlags::STATE_FLAG_NORMAL);
        auto selection_color = style_context->get_background_color(Gtk::StateFlags::STATE_FLAG_SELECTED | Gtk::StateFlags::STATE_FLAG_FOCUSED);
        text_color.set_alpha(0.4);

        auto append_column = [&](std::function<Glib::ustring(const Gtk::TreeIter& iter)> markup_function, bool align_right, Gdk::RGBA* text_color, double font_scale = 1) {
            auto cell_renderer = Gtk::manage(new Gtk::CellRendererText());
            if (align_right) cell_renderer->property_xalign() = 1;
            if (text_color != nullptr) cell_renderer->property_foreground_rgba() = *text_color;
            cell_renderer->property_scale() = font_scale;
            auto column = Gtk::manage(new Gtk::TreeViewColumn());
            column->pack_start(*cell_renderer, true);
            column->set_cell_data_func(*cell_renderer, [markup_function](Gtk::CellRenderer* cell, const Gtk::TreeIter& iter){
                ((Gtk::CellRendererText*)cell)->property_markup() = markup_function(iter);
            });
            tree_view.append_column(*column);
        };
        append_column([&](const Gtk::TreeIter& iter) -> Glib::ustring {
            return "  " + CtStrUtil::highlight_words(iter->get_value(columns.path), filter_words) + "  ";
        }, true, &text_color);
        append_column([&](const Gtk::TreeIter& iter) -> Glib::ustring {
            return CtStrUtil::highlight_words(iter->get_value(columns.label), filter_words);
        }, false, nullptr, 1.4);
        append_column([&](const Gtk::TreeIter& iter) -> Glib::ustring {
            return "  " + str::xml_escape(CtStrUtil::get_accelerator_label(iter->get_value(columns.accelerator))) + "  ";
        }, true, &selection_color);
    });

    auto set_filter = [&] (const Glib::ustring& raw_filter) {
        filter = Glib::Regex::create("/\\s{2,}/")->replace(raw_filter.c_str(), -1, 0, " ");
        filter = str::trim(filter).lowercase();
        filter_words = str::split(filter, " ");

        tree_model_filter->refilter();

        // TreeModelSort has no "resort" method, but reassigning the comparison function forces a resort
        tree_model_sort->set_default_sort_func([&](const Gtk::TreeIter& iter_a, const Gtk::TreeIter& iter_b) {
          // "The sort function used by TreeModelSort is not guaranteed to be stable" (GTK+ documentation),
          // so the original order of commands is needed as a tie-breaker
          int id_difference = iter_a->get_value(columns.order) - iter_b->get_value(columns.order);
          if (filter.empty()) return id_difference;

          int score_difference = get_command_score(iter_a) - get_command_score(iter_b);
          return (score_difference != 0) ? score_difference : id_difference;
        });
    };
    auto scroll_to_selected_item = [&]() {
        if (Gtk::TreeIter selected_iter = tree_view.get_selection()->get_selected()) {
            auto selected_path = tree_view.get_model()->get_path(selected_iter);
            tree_view.scroll_to_row(selected_path);
        }
    };
    auto select_first_item = [&]() {
        if (Gtk::TreeIter iter = tree_view.get_model()->get_iter("0")) {
            tree_view.get_selection()->select(iter);
            scroll_to_selected_item();
        }
    };
    auto select_previous_item = [&]() {
        if (Gtk::TreeIter selected_iter = tree_view.get_selection()->get_selected())
            if (--selected_iter) {
                tree_view.get_selection()->select(selected_iter);
                scroll_to_selected_item();
            }
    };
    auto select_next_item = [&]() {
        if (Gtk::TreeIter selected_iter = tree_view.get_selection()->get_selected())
            if (++selected_iter) {
                tree_view.get_selection()->select(selected_iter);
                scroll_to_selected_item();
            }
    };

    Gtk::Dialog popup_dialog("", *pCtMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    popup_dialog.set_transient_for(*pCtMainWin);
    popup_dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    popup_dialog.set_skip_taskbar_hint(true);
    popup_dialog.set_default_size(-1, 200);
    popup_dialog.set_size_request(-1, 350);

    // Width is determined by the width of the search entry/command list
    auto scrolled_window = Gtk::ScrolledWindow();
    scrolled_window.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_AUTOMATIC);
    popup_dialog.get_content_area()->pack_start(scrolled_window);

    select_first_item();
    tree_view.set_can_focus(false);
    scrolled_window.add(tree_view);

    auto header_bar = Gtk::HeaderBar();
    header_bar.property_spacing() = 0;
    popup_dialog.set_titlebar(header_bar);

    auto search_entry = Gtk::SearchEntry();
    search_entry.property_hexpand() = true;
    //if (Gtk.get_major_version() == 3 && Gtk.get_minor_version() < 22) {
      // GTK+ < 3.22 does not support expanding packed widgets
      // (see https://bugzilla.gnome.org/show_bug.cgi?id=724332)
    //  search_entry.set_size_request(600, -1);
    //}
    search_entry.property_margin() = 4;
    header_bar.set_custom_title(search_entry);
    search_entry.signal_changed().connect([&]() {
        set_filter(search_entry.get_text());
        select_first_item();
    });

    Gtk::TreeIter resulted_iter;
    auto run_command = [&] () {
        if (Gtk::TreeIter iter = tree_view.get_selection()->get_selected()) {
            resulted_iter = iter;
            popup_dialog.close();
        }
    };
    auto entry_activated = search_entry.signal_activate().connect([&]() {
        run_command();
    });
    tree_view.signal_row_activated().connect([&](const Gtk::TreeModel::Path&, Gtk::TreeViewColumn* ) {
        run_command();
    });
    popup_dialog.signal_show().connect([&]() {
        search_entry.grab_focus();
    });
    popup_dialog.signal_key_press_event().connect([&](GdkEventKey* key)->bool {
        if (key->keyval == GDK_KEY_Escape) {
            popup_dialog.close();
            return true;
        } else if (key->keyval == GDK_KEY_Tab || key->keyval == GDK_KEY_ISO_Left_Tab) {
            // Disable Tab and Shift+Tab to prevent navigating focus away from the search entry
            return true;
        } else if (key->keyval == GDK_KEY_Up) {
            select_previous_item();
            return true;
        } else if (key->keyval == GDK_KEY_Down) {
            select_next_item();
            return true;
        }
        return false;
    }, false);
    popup_dialog.show_all();

    // WIN_POS_CENTER_ON_PARENT works badly, so
    int root_x, root_y, height_1, width_1, height_2, width_2;
    pCtMainWin->get_position(root_x, root_y);
    pCtMainWin->get_size(width_1, height_1);
    popup_dialog.get_size(width_2, height_2);
    popup_dialog.move(root_x + (width_1 - width_2) / 2 - 150, root_y + (height_1 - height_2)/2 - 50);

    popup_dialog.run();

    if (resulted_iter)
        return resulted_iter->get_value(columns.id);
    return "";
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

// Opens the Table Handle Dialog, pygtk: dialog_tablehandle
CtDialogs::TableHandleResp CtDialogs::table_handle_dialog(CtMainWin* pCtMainWin, const Glib::ustring& title, const bool is_insert)
{
    Gtk::Dialog dialog(title, *pCtMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(*pCtMainWin);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);

    auto label_rows = Gtk::Label(_("Rows"));
    label_rows.set_halign(Gtk::Align::ALIGN_START);
    label_rows.set_margin_left(10);
    auto adj_rows = Gtk::Adjustment::create(pCtMainWin->get_ct_config()->tableRows, 1, 10000, 1);
    auto spinbutton_rows = Gtk::SpinButton(adj_rows);
    spinbutton_rows.set_value(pCtMainWin->get_ct_config()->tableRows);
    auto label_columns = Gtk::Label(_("Columns"));
    label_columns.set_halign(Gtk::Align::ALIGN_START);
    auto adj_columns = Gtk::Adjustment::create(pCtMainWin->get_ct_config()->tableColumns, 1, 10000, 1);
    auto spinbutton_columns = Gtk::SpinButton(adj_columns);
    spinbutton_columns.set_value(pCtMainWin->get_ct_config()->tableColumns);

    auto label_col_width = Gtk::Label(_("Default Width"));
    label_col_width.set_halign(Gtk::Align::ALIGN_START);
    auto adj_col_width = Gtk::Adjustment::create(pCtMainWin->get_ct_config()->tableColWidthDefault, 1, 10000, 1);
    auto spinbutton_col_width = Gtk::SpinButton(adj_col_width);
    spinbutton_col_width.set_value(pCtMainWin->get_ct_config()->tableColWidthDefault);

    auto label_size = Gtk::Label(std::string("<b>")+_("Table Size")+"</b>");
    label_size.set_use_markup();
    label_size.set_halign(Gtk::Align::ALIGN_START);
    auto label_col = Gtk::Label(std::string("<b>")+_("Column Properties")+"</b>");
    label_col.set_use_markup();
    label_col.set_halign(Gtk::Align::ALIGN_START);

    auto grid = Gtk::Grid();
    grid.property_margin() = 6;
    grid.set_row_spacing(4);
    grid.set_column_spacing(8);
    grid.set_row_homogeneous(true);

    if (is_insert)
    {
        grid.attach(label_size,         0, 0, 2, 1);
        grid.attach(label_rows,         0, 1, 1, 1);
        grid.attach(spinbutton_rows,    1, 1, 1, 1);
        grid.attach(label_columns,      2, 1, 1, 1);
        grid.attach(spinbutton_columns, 3, 1, 1, 1);
    }
    grid.attach(label_col,             0, 2, 2, 1);
    grid.attach(label_col_width,       0, 3, 1, 1);
    grid.attach(spinbutton_col_width,  1, 3, 1, 1);

    auto checkbutton_table_ins_from_file = Gtk::CheckButton(_("Import from CSV File"));

    auto content_area = dialog.get_content_area();
    content_area->set_spacing(5);
    content_area->pack_start(grid);
    if (is_insert) content_area->pack_start(checkbutton_table_ins_from_file);
    content_area->show_all();

    checkbutton_table_ins_from_file.signal_toggled().connect([&](){
        grid.set_sensitive(!checkbutton_table_ins_from_file.get_active());
    });

    auto on_key_press_dialog = [&](GdkEventKey *pEventKey)->bool
    {
        if (GDK_KEY_Return == pEventKey->keyval) {
            Gtk::Button *pButton = static_cast<Gtk::Button*>(dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT));
            pButton->grab_focus();
            pButton->clicked();
            return true;
        }
        return false;
    };
    dialog.signal_key_press_event().connect(on_key_press_dialog, false/*call me before other*/);

    const auto resp = dialog.run();
    if (Gtk::RESPONSE_ACCEPT == resp)
    {
        pCtMainWin->get_ct_config()->tableRows = spinbutton_rows.get_value_as_int();
        pCtMainWin->get_ct_config()->tableColumns = spinbutton_columns.get_value_as_int();
        pCtMainWin->get_ct_config()->tableColWidthDefault = spinbutton_col_width.get_value_as_int();
        if (checkbutton_table_ins_from_file.get_active())
            return TableHandleResp::OkFromFile;
        return TableHandleResp::Ok;
    }
    return TableHandleResp::Cancel;
}
