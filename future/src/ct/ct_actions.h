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
    bool is_there_selected_node_or_error();
    void _node_add(bool duplicate, bool add_child);

public:
    // tree actions
    void node_add()       { _node_add(false, false); }
    void node_dublicate() { _node_add(true, false);  }
    void node_child_add() { _node_add(false, true); }
    void node_edit();
    void node_toggle_read_only();
};
