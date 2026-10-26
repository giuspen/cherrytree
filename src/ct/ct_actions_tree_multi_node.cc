/*
 * ct_actions_tree_multi_node.cc
 *
 * Copyright 2009-2025
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
#include "ct_actions.h"
#include "ct_dialogs.h"
#include "ct_treestore.h"
#include "ct_logging.h"
#include <algorithm>
#include <vector>
void CtActions::nodes_move_after(const std::vector<gint64>& node_ids,
                                 Gtk::TreeModel::iterator father_iter,
                                 Gtk::TreeModel::iterator brother_iter,
                                 bool set_first,
                                 bool keep_focus)
{
    if (node_ids.empty()) return;
    CtTreeStore& tree_store = _pCtMainWin->get_tree_store();
    const gint64 father_id =
        father_iter ? tree_store.to_ct_tree_iter(father_iter).get_node_id() : -1;
    const gint64 brother_id =
        brother_iter ? tree_store.to_ct_tree_iter(brother_iter).get_node_id() : -1;
    auto selection_ids = _pCtMainWin->selected_tree_node_ids();
    if (selection_ids.empty()) selection_ids = node_ids;
    const CtTreeIter cursor = _tree_cursor_iter();
    const gint64 cursor_id = cursor ? cursor.get_node_id() : node_ids.front();
    auto move_node = [&](gint64 node_id) {
        CtTreeIter iter = tree_store.get_node_from_node_id(node_id);
        if (not iter) return;
        Gtk::TreeModel::iterator father =
            father_id >= 0
                ? tree_store.get_node_from_node_id(father_id)
                : Gtk::TreeModel::iterator{};
        Gtk::TreeModel::iterator brother =
            brother_id >= 0
                ? tree_store.get_node_from_node_id(brother_id)
                : Gtk::TreeModel::iterator{};
        node_move_after(iter, father, brother, set_first, keep_focus);
    };
    if (brother_iter or set_first) {
        for (auto id = node_ids.rbegin(); id != node_ids.rend(); ++id) {
            move_node(*id);
        }
    }
    else {
        for (const gint64 id : node_ids) {
            move_node(id);
        }
    }
    if (not keep_focus) _pCtMainWin->restore_tree_selection(selection_ids, cursor_id);
    for (const gint64 id : node_ids) {
        if (CtTreeIter iter = tree_store.get_node_from_node_id(id)) tree_store.update_nodes_icon(iter, true);
    }
}
std::vector<gint64> CtActions::_selected_sibling_node_ids()
{
    if (not _is_there_selected_node_or_error()) return {};
    const auto selected = _pCtMainWin->selected_tree_root_iters();
    if (selected.empty()) return {};
    const CtTreeIter parent = selected.front().parent();
    const gint64 parent_id = parent ? parent.get_node_id() : -1;
    std::vector<gint64> ids;
    for (const CtTreeIter& iter : selected) {
        const CtTreeIter iter_parent = iter.parent();
        if ((iter_parent ? iter_parent.get_node_id() : -1) != parent_id) {
            CtDialogs::error_dialog(_("Multiple selected nodes must have the same parent."), *_pCtMainWin);
            return {};
        }
        ids.push_back(iter.get_node_id());
    }
    return ids;
}
void CtActions::_nodes_move_vertical(bool up)
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });
    auto ids = _selected_sibling_node_ids();
    if (ids.empty()) return;
    CtTreeStore& store = _pCtMainWin->get_tree_store();
    Gtk::TreeModel::iterator neighbor = store.get_node_from_node_id(up ? ids.front() : ids.back());
    if (up) --neighbor;
    else ++neighbor;
    if (not neighbor) return;
    const auto selection_ids = _pCtMainWin->selected_tree_node_ids();
    const CtTreeIter cursor = _tree_cursor_iter();
    const gint64 cursor_id = cursor ? cursor.get_node_id() : ids.front();
    if (not up) std::reverse(ids.begin(), ids.end());
    for (const gint64 id : ids) {
        CtTreeIter current = store.get_node_from_node_id(id);
        neighbor = current;
        if (up) --neighbor;
        else ++neighbor;
        CtTreeIter adjacent = store.to_ct_tree_iter(neighbor);
        store.get_store()->iter_swap(current, adjacent);
        const auto sequence = current.get_node_sequence();
        current.set_node_sequence(adjacent.get_node_sequence());
        adjacent.set_node_sequence(sequence);
        current.pending_edit_db_node_hier();
        adjacent.pending_edit_db_node_hier();
    }
    _pCtMainWin->restore_tree_selection(selection_ids, cursor_id);
    _pCtMainWin->update_window_save_needed();
}