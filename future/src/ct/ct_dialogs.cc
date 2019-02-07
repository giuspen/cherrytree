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

    //set_bookmarks_menu_items(dad)
    //dad.ctdb_handler.pending_edit_db_bookmarks()
    ctMainWin->update_window_save_needed("book");
}
