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

// Dialog to select a Date
std::time_t date_select_dialog(Gtk::Window& parent, const std::string& title, const std::time_t& curr_time);

class CtMatchDialogStore : public Gtk::TreeStore
{
public:
    struct CtMatchModelColumns : public Gtk::TreeModel::ColumnRecord
    {
       Gtk::TreeModelColumn<gint64>         node_id;
       Gtk::TreeModelColumn<Glib::ustring>  node_name;
       Gtk::TreeModelColumn<Glib::ustring>  node_hier_name;
       Gtk::TreeModelColumn<int>            start_offset;
       Gtk::TreeModelColumn<int>            end_offset;
       Gtk::TreeModelColumn<int>            line_num;
       Gtk::TreeModelColumn<Glib::ustring>  line_content;
       CtMatchModelColumns() { add(node_id); add(node_name); add(node_hier_name);
                               add(start_offset); add(end_offset); add(line_num); add(line_content); }
    } columns;

    std::array<int, 2> dlg_size;
    std::array<int, 2> dlg_pos;
    Gtk::TreePath      saved_path;

public:
    static Glib::RefPtr<CtMatchDialogStore> create()
    {
        Glib::RefPtr<CtMatchDialogStore> model(new CtMatchDialogStore());
        model->set_column_types(model->columns);
        return model;
    }
    void add_row(gint64 node_id, const std::string& node_name, const std::string& node_hier_name,
                 int start_offset, int end_offset, int line_num, const std::string& line_content)
    {
        auto row = *append();
        row[columns.node_hier_name] = node_hier_name;
        row[columns.node_id] = node_id;             row[columns.node_name] = node_name;
        row[columns.start_offset] = start_offset;   row[columns.end_offset] = end_offset;
        row[columns.line_num] = line_num;           row[columns.line_content] = line_content;

    }
};

// the All Matches Dialog
void match_dialog(const std::string& title, CtMainWin& ctMainWin, Glib::RefPtr<CtMatchDialogStore> model);

} // namespace ct_dialogs
