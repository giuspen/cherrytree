#pragma once
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

class CtMainWin;
class CtTreeStore;
namespace ct_dialogs {

template<class GtkStoreBase>
class CtChooseDialogStore : public GtkStoreBase
{
public:
    struct CtChooseDialogModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<Glib::ustring> stock_id;
       Gtk::TreeModelColumn<Glib::ustring> key;
       Gtk::TreeModelColumn<Glib::ustring> desc;
       Gtk::TreeModelColumn<gint64>        node_id;

       CtChooseDialogModelColumns() { add(stock_id); add(key); add(desc); add(node_id); }
    } columns;

public:
    static Glib::RefPtr<CtChooseDialogStore<GtkStoreBase>> create();
    void add_row(const std::string& stock_id, const std::string& key, const std::string& desc, gint64 node_id = 0);
};
typedef CtChooseDialogStore<Gtk::ListStore> CtChooseDialogListStore;
typedef CtChooseDialogStore<Gtk::TreeStore> CtChooseDialogTreeStore;

Gtk::TreeModel::iterator choose_item_dialog(Gtk::Window& parent,const std::string& title,
                                            Glib::RefPtr<CtChooseDialogListStore> model,
                                            const gchar* one_column_name = nullptr);

// Dialog to select a color, featuring a palette
bool color_pick_dialog(Gtk::Window& parent, Gdk::RGBA& color);

// The Question dialog, returns True if the user presses OK
bool question_dialog(const std::string& message, Gtk::Window& parent);

// The Info dialog
void info_dialog(const std::string& message, Gtk::Window& parent);

// The Warning dialog
void warning_dialog(const std::string& message, Gtk::Window& parent);

// The Error dialog
void error_dialog(const std::string& message, Gtk::Window& parent);

// Dialog to Select a Node
Gtk::TreeIter choose_node_dialog(Gtk::Window& parent, Gtk::TreeView& parentTreeView, const std::string& title, CtTreeStore* treestore, Gtk::TreeIter sel_tree_iter);

// Handle the Bookmarks List
void bookmarks_handle_dialog(CtMainWin* ctMainWin);

} // namespace ct_dialogs
