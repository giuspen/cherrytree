#pragma once
#include "ct_main_win.h"


class CtMainWin;
class CtActions
{
public:
    void init(CtMainWin* mainWin, CtTreeStore* treeStore) { _ctMainWin = mainWin; _ctTreestore = treeStore; }

private:
    CtMainWin*   _ctMainWin;
    CtTreeStore* _ctTreestore;

private:
    bool          _is_there_selected_node_or_error();
    void          _node_add(bool duplicate, bool add_child);
    void          _node_add_with_data(Gtk::TreeIter curr_iter, CtNodeData& nodeData, bool add_child);
    void          _node_child_exist_or_create(Gtk::TreeIter parentIter, const std::string& nodeName);
    void          _node_move_after(Gtk::TreeIter iter_to_move, Gtk::TreeIter father_iter,
                                   Gtk::TreeIter brother_iter = Gtk::TreeIter(), bool set_first = false);
public:
    // tree actions
    void node_add()       { _node_add(false, false); }
    void node_dublicate() { _node_add(true, false);  }
    void node_child_add() { _node_add(false, true); }
    void node_edit();
    void node_toggle_read_only();
    void node_date();
    void node_up();
    void node_down();
    void node_right();
    void node_left();
};
