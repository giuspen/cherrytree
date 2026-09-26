/*
 * ct_main_win_multi_node_events.cc
 *
 * Copyright 2009-2026
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

#include "ct_main_win.h"
#include "ct_actions.h"
#include <algorithm>
#include <cmath>
#include <unordered_set>

CtTreeIter CtMainWin::tree_cursor_iter()
{
    if (not _uCtTreeview or not _uCtTreestore) return CtTreeIter{};
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* pColumn{nullptr};
    _uCtTreeview->get_cursor(path, pColumn);
    if (path.empty()) return CtTreeIter{};
    if (auto iter = _uCtTreeview->get_model()->get_iter(path)) {
        return _uCtTreestore->to_ct_tree_iter(iter);
    }
    return CtTreeIter{};
}

std::vector<CtTreeIter> CtMainWin::selected_tree_iters(bool unique_data_holders)
{
    std::vector<CtTreeIter> selected;
    if (not _uCtTreeview or not _uCtTreestore) return selected;
    std::unordered_set<gint64> seen;
    for (auto path : _uCtTreeview->get_selection()->get_selected_rows()) {
        CtTreeIter iter = _uCtTreestore->get_iter(path);
        if (iter and (not unique_data_holders or seen.insert(iter.get_node_id_data_holder()).second)) {
            selected.push_back(iter);
        }
    }
    return selected;
}

std::vector<gint64> CtMainWin::selected_tree_node_ids()
{
    std::vector<gint64> ids;
    for (const CtTreeIter& iter : selected_tree_iters(false)) ids.push_back(iter.get_node_id());
    return ids;
}

void CtMainWin::restore_tree_selection(const std::vector<gint64>& node_ids, gint64 cursor_id)
{
    if (node_ids.empty()) return;
    if (std::find(node_ids.begin(), node_ids.end(), cursor_id) == node_ids.end()) cursor_id = node_ids.front();
    if (CtTreeIter cursor = _uCtTreestore->get_node_from_node_id(cursor_id)) {
        _uCtTreeview->expand_to_path(_uCtTreestore->get_path(cursor));
        _uCtTreeview->set_cursor(_uCtTreestore->get_path(cursor));
    }
    auto selection = _uCtTreeview->get_selection();
    selection->unselect_all();
    for (const gint64 id : node_ids) {
        if (CtTreeIter iter = _uCtTreestore->get_node_from_node_id(id)) {
            const auto path = _uCtTreestore->get_path(iter);
            _uCtTreeview->expand_to_path(path);
            selection->select(path);
        }
    }
}

std::vector<CtTreeIter> CtMainWin::selected_tree_root_iters()
{
    auto selected = selected_tree_iters(false);

    std::unordered_set<gint64> selected_ids;

    for (const CtTreeIter& iter : selected) {
        selected_ids.insert(iter.get_node_id());
    }

    std::vector<CtTreeIter> roots;
    roots.reserve(selected.size());

    for (const CtTreeIter& iter : selected) {
        bool has_selected_ancestor = false;

        for (CtTreeIter parent = iter.parent();
             parent;
             parent = parent.parent())
        {
            if (selected_ids.count(parent.get_node_id()) > 0) {
                has_selected_ancestor = true;
                break;
            }
        }

        if (not has_selected_ancestor) {
            roots.push_back(iter);
        }
    }

    return roots;
}

void CtMainWin::_store_previous_editor_state(CtTreeIter next_tree_iter)
{
    if (not _prevTreeIter or (_prevTreeIter.get_node_id() == next_tree_iter.get_node_id())) return;

    const gint64 prev_node_id_data_holder = _prevTreeIter.get_node_id_data_holder();
    auto pTextBuffer = _prevTreeIter.get_node_text_buffer();
    if (pTextBuffer->get_modified()) {
        _fileSaveNeeded = true;
        pTextBuffer->set_modified(false);
        _ctStateMachine.update_state(_prevTreeIter);
    }
    _nodesCursorPos[prev_node_id_data_holder] = pTextBuffer->property_cursor_position();
    if (not _multiNodeMode) {
        _nodesVScrollPos[prev_node_id_data_holder] = round(_scrolledwindowText.get_vadjustment()->get_value());
    }
}

void CtMainWin::_set_active_editor(CtTreeIter tree_iter, CtTextView* pTextView, const bool update_history)
{
    if (_multiNodeEditorRebuilding or not tree_iter or not pTextView) return;
    _store_previous_editor_state(tree_iter);
    _activeTreeIter = tree_iter;
    _pActiveTextview = pTextView;
    _prevTreeIter = tree_iter;

    if (user_active()) {
        const bool is_bookmarked = _uCtTreestore->is_node_bookmarked(tree_iter.get_node_id());
        menu_update_bookmark_menu_item(is_bookmarked);
        window_header_update();
        window_header_update_lock_icon(tree_iter.get_node_read_only());
        window_header_update_ghost_icon(tree_iter.get_node_is_excluded_from_search() or tree_iter.get_node_children_are_excluded_from_search());
        window_header_update_bookmark_icon(is_bookmarked);
        update_selected_node_statusbar_info();
    }
    if (update_history) {
        _ctStateMachine.node_selected_changed(tree_iter.get_node_id_data_holder());
    }
}

#if GTKMM_MAJOR_VERSION < 4

void CtMainWin::_reselect_tree_nodes(const std::vector<gint64>& node_ids)
{
    auto selection = _uCtTreeview->get_selection();
    for (const gint64 node_id : node_ids) {
        CtTreeIter iter = _uCtTreestore->get_node_from_node_id(node_id);
        if (iter) selection->select(_uCtTreestore->get_path(iter));
    }
}

bool CtMainWin::_multi_node_tree_button_press(GdkEventButton* event)
{
    _treeRightClickSelectionIds.clear();
    if (event->button == 1) {
        _treeDragSelectionIds.clear();
        _treeDragSourceNodeId = -1;
        _treeDragStarted = false;
    }
    if (event->button != 1 and event->button != 3) return false;
    Gtk::TreeModel::Path path;
    if (not _uCtTreeview->get_path_at_pos(static_cast<int>(event->x), static_cast<int>(event->y), path)) return false;
    CtTreeIter clicked = _uCtTreestore->get_iter(path);
    if (not clicked) return false;
    const auto selection = _uCtTreeview->get_selection();
    if (event->button == 1) {
        _treeDragSourceNodeId = clicked.get_node_id();
        // Modified clicks change the selection; snapshot those at drag-data-get.
        if (selection->is_selected(path) and not (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))) {
            _treeDragSelectionIds = selected_tree_node_ids();
        }
    }
    else if (selection->is_selected(path)) {
        _treeRightClickSelectionIds = selected_tree_node_ids();
    }
    return false;
}

void CtMainWin::_multi_node_tree_button_release(GdkEventButton* event)
{
    if (event->button == 1) {
        /*
         * A normal click on a member of a multi-selection temporarily
         * preserved the group on button press so that a possible drag
         * could start with the whole selection.
         *
         * If no drag actually started, restore normal GTK behaviour:
         * leave only the clicked row selected.
         */
        if (not _treeDragStarted and _treeDragSelectionIds.size() > 1) {
            Gtk::TreeModel::Path path;

            if (_uCtTreeview->get_path_at_pos(
                    static_cast<int>(event->x),
                    static_cast<int>(event->y),
                    path))
            {
                auto selection = _uCtTreeview->get_selection();

                selection->unselect_all();
                selection->select(path);
            }
        }

        if (not _treeDragStarted) {
            _treeDragSelectionIds.clear();
            _treeDragSourceNodeId = -1;
        }
    }
}

void CtMainWin::_multi_node_tree_event_after(GdkEvent* event)
{
    if (event->type == GDK_BUTTON_PRESS &&
        event->button.button == 3 && not _treeRightClickSelectionIds.empty())
    {
        _reselect_tree_nodes(_treeRightClickSelectionIds);
        _treeRightClickSelectionIds.clear();
    }
    if (event->type == GDK_BUTTON_PRESS && event->button.button == 1) {
        /* Preserve the captured group while the press may become a drag. */
        if (_treeDragSelectionIds.size() > 1) {
            _reselect_tree_nodes(_treeDragSelectionIds);
        }
    }
}

#endif /* GTKMM_MAJOR_VERSION < 4 */

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)

void CtMainWin::_multi_node_drag_data_received(
    const Glib::RefPtr<Gdk::DragContext>& context,
    int x,
    int y,
    const Gtk::SelectionData& selection_data,
    guint time)
{
    bool move_performed = false;
    const bool keep_focus = _drag_shift_held;

    auto on_scope_exit = scope_guard([&](void*) {
        const auto selection_ids = _treeDragSelectionIds;
        const gint64 cursor_id = _treeDragSourceNodeId;

        _treeDragSelectionIds.clear();
        _treeDragSourceNodeId = -1;
        _treeDragStarted = false;

        context->drag_finish(move_performed, false, time);

        // GTK finishes the drag after this handler. Shift must also bypass
        // the deferred restoration, which would expand the destination.
        if (not (move_performed and keep_focus)) {
            Glib::signal_idle().connect_once(sigc::track_obj(
                [this, selection_ids, cursor_id]() {
                    restore_tree_selection(selection_ids, cursor_id);
                }, *this));
        }
    });

    Gtk::TreePath destination;
    Gtk::TreeViewDropPosition position{Gtk::TREE_VIEW_DROP_BEFORE};
    if (not _uCtTreeview->get_dest_row_at_pos(x, y, destination, position)) return;
    const auto source = selection_data.get_data_as_string();
    if (source.empty()) return;
    Gtk::TreePath source_path{source};
    CtTreeIter drag = _uCtTreestore->get_iter(source_path);

    CtTreeIter drop = _uCtTreestore->get_iter(destination);
    if (not drag or not drop) return;

    const gint64 drag_id = drag.get_node_id();
    if (std::find(_treeDragSelectionIds.begin(), _treeDragSelectionIds.end(), drag_id) == _treeDragSelectionIds.end()) {
        _treeDragSelectionIds = {drag_id};
    }
    std::unordered_set<gint64> selected(_treeDragSelectionIds.begin(), _treeDragSelectionIds.end());
    std::vector<gint64> node_ids;
    for (const gint64 id : _treeDragSelectionIds) {
        CtTreeIter iter = _uCtTreestore->get_node_from_node_id(id);
        if (not iter) return;
        CtTreeIter ancestor = iter.parent();
        while (ancestor and not selected.count(ancestor.get_node_id())) ancestor = ancestor.parent();
        if (not ancestor) node_ids.push_back(id);
    }
    if (node_ids.empty()) return;

    CtTreeIter parent = _uCtTreestore->get_node_from_node_id(node_ids.front()).parent();
    const gint64 parent_id = parent ? parent.get_node_id() : -1;
    for (const gint64 id : node_ids) {
        CtTreeIter iter_parent = _uCtTreestore->get_node_from_node_id(id).parent();
        if ((iter_parent ? iter_parent.get_node_id() : -1) != parent_id) {
            CtDialogs::error_dialog(_("Multiple selected nodes must have the same parent."), *this);
            return;
        }
    }

    const std::unordered_set<gint64> moving(node_ids.begin(), node_ids.end());
    if (moving.count(drop.get_node_id())) return;
    CtTreeIter new_parent = (position == Gtk::TREE_VIEW_DROP_BEFORE or position == Gtk::TREE_VIEW_DROP_AFTER)
                         ? drop.parent() : drop;
    for (CtTreeIter ancestor = new_parent; ancestor; ancestor = ancestor.parent()) {
        if (moving.count(ancestor.get_node_id())) {
            CtDialogs::error_dialog(_("The new parent can't be one of his children!"), *this);
            return;
        }
    }

    Gtk::TreeModel::iterator sibling;
    bool set_first = false;
    if (position == Gtk::TREE_VIEW_DROP_BEFORE) {
        sibling = drop;
        while (--sibling) {
            if (not moving.count(_uCtTreestore->to_ct_tree_iter(sibling).get_node_id())) break;
        }
        set_first = not sibling;
    }
    else if (position == Gtk::TREE_VIEW_DROP_AFTER) sibling = drop;
    _uCtActions->nodes_move_after(node_ids, new_parent, sibling, set_first, keep_focus);
    move_performed = true;
}

void CtMainWin::_multi_node_drag_data_get(Gtk::SelectionData& selection_data)
{
    CtTreeIter iter = _treeDragSourceNodeId >= 0
                   ? _uCtTreestore->get_node_from_node_id(_treeDragSourceNodeId) : tree_cursor_iter();
    if (not iter) return;
    _treeDragStarted = true;
    if (_treeDragSelectionIds.empty()) {
        _treeDragSelectionIds = selected_tree_node_ids();
        if (std::find(_treeDragSelectionIds.begin(), _treeDragSelectionIds.end(), iter.get_node_id()) == _treeDragSelectionIds.end()) {
            _treeDragSelectionIds = {iter.get_node_id()};
        }
    }
    if (_treeDragSelectionIds.size() > 1) {
        _reselect_tree_nodes(_treeDragSelectionIds);
    }

    const auto path = _uCtTreestore->get_path(iter).to_string();
    selection_data.set("UTF8_STRING", 8, reinterpret_cast<const guint8*>(path.c_str()), static_cast<int>(path.size()));
}

#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
