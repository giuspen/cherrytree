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
    void _node_add(bool duplicate);

public:
    // tree actions
    void node_add()       { _node_add(false); }
    void node_dublicate() { _node_add(true);  }
    void node_child_add();
    void node_edit();
};
