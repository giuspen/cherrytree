#pragma once
#include "ct_main_win.h"


class CtMainWin;
class CtActions
{
public:
    void init(CtMainWin* mainWin, CtTreeStore* treeStore) { _ctMainWin = mainWin; _ctTreestore = treeStore; _find_init(); }

private:
    CtMainWin*   _ctMainWin;
    CtTreeStore* _ctTreestore;

private:
    bool          _is_there_selected_node_or_error();

private:
    // helpers for tree actions
    void          _node_add(bool duplicate, bool add_child);
    void          _node_add_with_data(Gtk::TreeIter curr_iter, CtNodeData& nodeData, bool add_child);
    void          _node_child_exist_or_create(Gtk::TreeIter parentIter, const std::string& nodeName);
    void          _node_move_after(Gtk::TreeIter iter_to_move, Gtk::TreeIter father_iter,
                                   Gtk::TreeIter brother_iter = Gtk::TreeIter(), bool set_first = false);
    bool          _need_node_swap(Gtk::TreeIter& leftIter, Gtk::TreeIter& rightIter, bool ascendings);
    bool          _tree_sort_level_and_sublevels(const Gtk::TreeNodeChildren& children, bool ascending);

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
    void node_change_father();
    void tree_sort_ascending();
    void tree_sort_descending();
    void node_siblings_sort_ascending();
    void node_siblings_sort_descending();

    void bookmark_curr_node();
    void bookmark_curr_node_remove();
    void bookmarks_handle();

private:
    // helpers for find actions
    void                _find_init();
    bool                _parse_node_content_iter(const CtTreeIter& tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, const std::string& pattern,
                                                bool forward, bool first_fromsel, bool all_matches, bool first_node);
    Gtk::TextIter       _get_inner_start_iter(Glib::RefPtr<Gtk::TextBuffer> text_buffer, bool forward, const gint64& node_id);
    bool                _is_node_within_time_filter(const CtTreeIter& node_iter);
    bool                _find_pattern(CtTreeIter tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, std::string pattern,
                                      Gtk::TextIter start_iter, bool forward, bool all_matches);
    std::string         _get_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter text_iter);
    std::array<int, 2>  _check_pattern_in_object_between(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Glib::RefPtr<Glib::Regex> pattern,
                                                         int start_offset, int end_offset, bool forward, std::string& obj_content);
    int                 _get_num_objs_before_offset(Glib::RefPtr<Gtk::TextBuffer> text_buffer, int max_offset);

public:
    // find actions
    void find_in_selected_node();
    void find_in_all_nodes();
    void find_in_sel_node_and_subnodes();
    void find_a_node();
    void find_again();
    void find_back();
    void replace_in_selected_node();
    void replace_in_all_nodes();
    void replace_in_sel_node_and_subnodes();
    void replace_in_nodes_names();
    void replace_again();
    void find_allmatchesdialog_restore();
};
