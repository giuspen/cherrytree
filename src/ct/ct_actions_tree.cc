/*
 * ct_actions_tree.cc
 *
 * Copyright 2009-2024
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
#include "ct_image.h"
#include "ct_dialogs.h"
#include "ct_clipboard.h"
#include <ctime>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>

bool CtActions::_is_there_selected_node_or_error()
{
    if (_pCtMainWin->curr_tree_iter()) return true;
    CtDialogs::warning_dialog(_("No Node is Selected"), *_pCtMainWin);
    return false;
}

bool CtActions::_is_tree_not_empty_or_error()
{
    if (not _pCtMainWin->get_tree_store().get_iter_first()) {
        CtDialogs::error_dialog(_("The Tree is Empty!"), *_pCtMainWin);
        return false;
    }
    return true;
}

bool CtActions::_is_curr_node_not_read_only_or_error()
{
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) {
        CtDialogs::error_dialog(_("The Selected Node is Read Only."), *_pCtMainWin);
        return false;
    }
    return true;
}

// Returns True if ok (no syntax highlighting) or False and prompts error dialog
bool CtActions::_is_curr_node_not_syntax_highlighting_or_error(bool plain_text_ok /*=false*/)
{
    if (_pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID
        or (plain_text_ok and _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() == CtConst::PLAIN_TEXT_ID))
        return true;
    if (not plain_text_ok)
        CtDialogs::warning_dialog(_("This Feature is Available Only in Rich Text Nodes."), *_pCtMainWin);
    else
        CtDialogs::warning_dialog(_("This Feature is Not Available in Automatic Syntax Highlighting Nodes."), *_pCtMainWin);
    return false;
}

// Returns True if ok (there's a selection) or False and prompts error dialog
bool CtActions::_is_there_text_selection_or_error()
{
    if (not _is_there_selected_node_or_error()) return false;
    if (not _curr_buffer()->get_has_selection()) {
        CtDialogs::error_dialog(_("No Text is Selected."), *_pCtMainWin);
        return false;
    }
    return true;
}

bool CtActions::_is_there_anch_widg_selection_or_error(const char anch_widg_id)
{
    if (not _is_there_selected_node_or_error()) return false;
    if (not _is_curr_node_not_syntax_highlighting_or_error()) return false;
    bool already_failed{false};
    Gtk::TextIter iter_insert;
    if (_curr_buffer()->get_has_selection()) {
        Gtk::TextIter iter_sel_end;
        _pCtMainWin->get_text_view().get_buffer()->get_selection_bounds(iter_insert, iter_sel_end);
        const int num_chars = iter_sel_end.get_offset() - iter_insert.get_offset();
        if (num_chars != 1) {
            already_failed = true;
        }
    }
    else {
        iter_insert = _pCtMainWin->get_text_view().get_buffer()->get_insert()->get_iter();
    }
    if (not already_failed) {
        auto widgets = _pCtMainWin->curr_tree_iter().get_anchored_widgets(iter_insert.get_offset(), iter_insert.get_offset());
        if (not widgets.empty()) {
            if ('t' == anch_widg_id) {
                auto pTableCommon = dynamic_cast<CtTableCommon*>(widgets.front());
                if (pTableCommon) {
                    curr_table_anchor = pTableCommon;
                    return true;
                }
            }
            else if ('c' == anch_widg_id) {
                auto pCodeBox = dynamic_cast<CtCodebox*>(widgets.front());
                if (pCodeBox) {
                    curr_codebox_anchor = pCodeBox;
                    return true;
                }
            }
        }
    }
    if ('t' == anch_widg_id) CtDialogs::error_dialog(_("No Table is Selected."), *_pCtMainWin);
    else if ('c' == anch_widg_id) CtDialogs::error_dialog(_("No CodeBox is Selected."), *_pCtMainWin);
    return false;
}

// Put Selection Upon the anchored widget
void CtActions::object_set_selection(CtAnchoredWidget* widget)
{
    Gtk::TextIter iter_object = _curr_buffer()->get_iter_at_child_anchor(widget->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_object;
    iter_bound.forward_char();
    if (dynamic_cast<CtImage*>(widget)) {
        _pCtMainWin->get_text_view().mm().grab_focus();
    }
    _curr_buffer()->select_range(iter_object, iter_bound);
}

// Returns True if there's not a node selected or is not rich text
bool CtActions::_node_sel_and_rich_text()
{
    if (not _is_there_selected_node_or_error()) return false;
    if (not _is_curr_node_not_syntax_highlighting_or_error()) return false;
    return true;
}

void CtActions::node_subnodes_copy()
{
    if (not _is_there_selected_node_or_error()) return;
    _pCtMainWin->signal_app_tree_node_copy();
}

void CtActions::node_subnodes_paste()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    _pCtMainWin->signal_app_tree_node_paste();
}

void CtActions::node_subnodes_paste2(CtTreeIter& other_ct_tree_iter,
                                     CtMainWin* pWinToCopyFrom)
{
    // create duplicate of the top node
    _node_add(CtDuplicateShared::Duplicate, false/*add_as_child*/, &other_ct_tree_iter, pWinToCopyFrom);

    Gtk::TreeIter new_top_iter = _pCtMainWin->curr_tree_iter();

    // function to duplicate a node
    auto duplicate_subnode = [&](CtTreeIter old_iter, Gtk::TreeIter new_parent) {
        CtNodeData node_data{};
        std::shared_ptr<CtNodeState> node_state;
        pWinToCopyFrom->get_tree_store().get_node_data(old_iter, node_data, true/*loadTextBuffer*/);
        if (node_data.syntax != CtConst::RICH_TEXT_ID) {
            node_data.pTextBuffer = _pCtMainWin->get_new_text_buffer(node_data.pTextBuffer->get_text());
            node_data.anchoredWidgets.clear();
        }
        else {
            CtStateMachine& state_machine_from = pWinToCopyFrom->get_state_machine();
            state_machine_from.update_state(old_iter);
            node_state = state_machine_from.requested_state_current(old_iter.get_node_id_data_holder());
            node_data.anchoredWidgets.clear();                          // node_state will be used instead
            node_data.pTextBuffer = _pCtMainWin->get_new_text_buffer(); // node_state will be used instead
        }
        node_data.tsCreation = std::time(nullptr);
        node_data.tsLastSave = node_data.tsCreation;
        node_data.nodeId = _pCtMainWin->get_tree_store().node_id_get();
        auto new_iter = _pCtMainWin->get_tree_store().append_node(&node_data, &new_parent/*as parent*/);
        if (node_state) {
           _pCtMainWin->load_buffer_from_state(node_state, _pCtMainWin->get_tree_store().to_ct_tree_iter(new_iter));
        }
        _pCtMainWin->get_tree_store().to_ct_tree_iter(new_iter).pending_new_db_node();
        return new_iter;
    };

    // function to duplicate all sub nodes
    std::function<void(Gtk::TreeIter, Gtk::TreeIter)> duplicate_subnodes;
    duplicate_subnodes = [&](Gtk::TreeIter old_parent, Gtk::TreeIter new_parent) {
        for (auto child : old_parent->children()) {
            auto new_child = duplicate_subnode(pWinToCopyFrom->get_tree_store().to_ct_tree_iter(child), new_parent);
            duplicate_subnodes(child, new_child);
        }
    };
    duplicate_subnodes(other_ct_tree_iter, new_top_iter);

    _pCtMainWin->get_tree_store().nodes_sequences_fix(new_top_iter->parent(), true);
    pWinToCopyFrom->get_tree_view().set_cursor_safe(other_ct_tree_iter); // this line fixes glich with text_buffer with widgets caused by the next line
    _pCtMainWin->get_tree_view().set_cursor_safe(new_top_iter);
    _pCtMainWin->get_text_view().mm().grab_focus();
}

void CtActions::node_subnodes_duplicate()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    CtTreeIter top_iter = _pCtMainWin->curr_tree_iter();
    node_subnodes_paste2(top_iter, _pCtMainWin);
}

void CtActions::_node_add(const CtDuplicateShared duplicate_shared,
                          const bool add_as_child,
                          const CtTreeIter* pCtTreeIterFrom/*=nullptr*/,
                          CtMainWin* pWinToCopyFrom/*=nullptr*/)
{
    CtNodeData nodeData{};
    std::shared_ptr<CtNodeState> node_state;
    if (CtDuplicateShared::None == duplicate_shared) {
        std::string title = add_as_child ? _("New Child Node Properties") : _("New Node Properties");
        CtTreeIter currTreeIter = _pCtMainWin->curr_tree_iter();
        nodeData.syntax = currTreeIter ? currTreeIter.get_node_syntax_highlighting() : CtConst::RICH_TEXT_ID;
        if (not CtDialogs::node_prop_dialog(title, _pCtMainWin, nodeData, _pCtMainWin->get_tree_store().get_used_tags())) {
            return;
        }
    }
    else {
        pWinToCopyFrom->get_tree_store().get_node_data(*pCtTreeIterFrom, nodeData, true/*loadTextBuffer*/);
        if (CtDuplicateShared::Duplicate == duplicate_shared) {
            if (CtConst::RICH_TEXT_ID != nodeData.syntax) {
                nodeData.pTextBuffer = _pCtMainWin->get_new_text_buffer(nodeData.pTextBuffer->get_text());
                nodeData.anchoredWidgets.clear();
            }
            else {
                CtStateMachine& state_machine_from = pWinToCopyFrom->get_state_machine();
                state_machine_from.update_state(*pCtTreeIterFrom);
                node_state = state_machine_from.requested_state_current(pCtTreeIterFrom->get_node_id_data_holder());
                nodeData.anchoredWidgets.clear();                          // node_state will be used instead
                nodeData.pTextBuffer = _pCtMainWin->get_new_text_buffer(); // node_state will be used instead
            }
            nodeData.sharedNodesMasterId = 0;
        }
        else {
            // CtDuplicateShared::Shared
            if (nodeData.sharedNodesMasterId > 0) {
                // node from is also a shared node, we just point to the same master / leave it as is
            }
            else {
                // node from is not a shared node, let's set the shared node id to its node id (new node id will be assigned)
                nodeData.sharedNodesMasterId = nodeData.nodeId;
            }
        }
    }
    (void)_node_add_with_data(_pCtMainWin->curr_tree_iter(), nodeData, add_as_child, node_state);
}

Gtk::TreeIter CtActions::_node_add_with_data(Gtk::TreeIter curr_iter,
                                             CtNodeData& nodeData,
                                             const bool add_as_child,
                                             std::shared_ptr<CtNodeState> node_state)
{
    if (nodeData.sharedNodesMasterId <= 0) {
        // not a shared node
        if (not nodeData.pTextBuffer) {
            nodeData.pTextBuffer = _pCtMainWin->get_new_text_buffer();
        }
        nodeData.tsCreation = std::time(nullptr);
        nodeData.tsLastSave = nodeData.tsCreation;
    }
    CtTreeStore& ct_treestore = _pCtMainWin->get_tree_store();
    nodeData.nodeId = ct_treestore.node_id_get();

    _pCtMainWin->update_window_save_needed();
    _pCtConfig->syntaxHighlighting = nodeData.syntax;

    Gtk::TreeIter nodeIter;
    if (add_as_child) {
        nodeIter = ct_treestore.append_node(&nodeData, &curr_iter/*as parent*/);
    }
    else if (curr_iter) {
        nodeIter = ct_treestore.insert_node(&nodeData, curr_iter/*after*/);
    }
    else {
        nodeIter = ct_treestore.append_node(&nodeData);
    }
    CtTreeIter nodeCtIter = ct_treestore.to_ct_tree_iter(nodeIter);
    if (node_state) {
        _pCtMainWin->load_buffer_from_state(node_state, nodeCtIter);
    }
    nodeCtIter.pending_new_db_node();
    ct_treestore.nodes_sequences_fix(nodeIter->parent(), false);
    ct_treestore.update_node_aux_icon(nodeCtIter);
    _pCtMainWin->get_tree_view().set_cursor_safe(nodeIter);
    _pCtMainWin->get_text_view().mm().grab_focus();
    return nodeIter;
}

Gtk::TreeIter CtActions::node_child_exist_or_create(Gtk::TreeIter parentIter, const std::string& nodeName, const bool focusIfExisting)
{
    Gtk::TreeIter childIter = parentIter ? parentIter->children().begin() : _pCtMainWin->get_tree_store().get_iter_first();
    for (; childIter; ++childIter) {
        if (_pCtMainWin->get_tree_store().to_ct_tree_iter(childIter).get_node_name() == nodeName) {
            if (focusIfExisting) {
                _pCtMainWin->get_tree_view().set_cursor_safe(childIter);
            }
            return childIter;
        }
    }
    CtNodeData nodeData{};
    nodeData.name = nodeName;
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    return _node_add_with_data(parentIter, nodeData, true/*add_as_child*/, nullptr/*node_state*/);
}

// Move a node to a parent and after a sibling
void CtActions::node_move_after(Gtk::TreeIter iter_to_move,
                                Gtk::TreeIter father_iter,
                                Gtk::TreeIter brother_iter/*= Gtk::TreeIter{}*/,
                                bool set_first/*= false*/)
{
    CtTreeStore& ctTreeStore = _pCtMainWin->get_tree_store();
    Glib::RefPtr<Gtk::TreeStore> pTreeStore = ctTreeStore.get_store();
    Gtk::TreeIter new_node_iter;
    if (brother_iter)   new_node_iter = pTreeStore->insert_after(brother_iter);
    else if (set_first) new_node_iter = pTreeStore->prepend(father_iter->children());
    else                new_node_iter = pTreeStore->append(father_iter->children());

    // we move also all the children
    std::function<void(Gtk::TreeIter&,Gtk::TreeIter&)> node_move_data_and_children;
    node_move_data_and_children = [&](Gtk::TreeIter& old_iter,Gtk::TreeIter& new_iter) {
        CtNodeData node_data{};
        ctTreeStore.get_node_data(old_iter, node_data, true/*loadTextBuffer*/);
        ctTreeStore.update_node_data(new_iter, node_data);
        for (Gtk::TreeIter child : old_iter->children()) {
            Gtk::TreeIter new_child = pTreeStore->append(new_iter->children());
            node_move_data_and_children(child, new_child);
        }
    };
    node_move_data_and_children(iter_to_move, new_node_iter);

    // now we can remove the old iter (and all children)
    _pCtMainWin->resetPrevTreeIter();
    pTreeStore->erase(iter_to_move);
    ctTreeStore.to_ct_tree_iter(new_node_iter).pending_edit_db_node_hier();

    ctTreeStore.nodes_sequences_fix(Gtk::TreeIter(), true);
    CtTreeView& ctTreeView = _pCtMainWin->get_tree_view();
    if (father_iter) {
        ctTreeView.expand_row(ctTreeStore.get_path(father_iter), false);
    }
    else {
        ctTreeView.expand_row(ctTreeStore.get_path(new_node_iter), false);
    }
    Gtk::TreePath new_node_path = _pCtMainWin->get_tree_store().get_path(new_node_iter);
    ctTreeView.collapse_row(new_node_path);
    ctTreeView.set_cursor(new_node_path);
    _pCtMainWin->update_window_save_needed();
}

bool CtActions::_need_node_swap(Gtk::TreeIter& leftIter, Gtk::TreeIter& rightIter, bool ascending)
{
    Glib::ustring left_node_name = _pCtMainWin->get_tree_store().to_ct_tree_iter(leftIter).get_node_name().lowercase();
    Glib::ustring right_node_name = _pCtMainWin->get_tree_store().to_ct_tree_iter(rightIter).get_node_name().lowercase();
    //int cmp = left_node_name.compare(right_node_name);
    int cmp = CtStrUtil::natural_compare(left_node_name, right_node_name);

    return ascending ? cmp > 0 : cmp < 0;
}

bool CtActions::_tree_sort_level_and_sublevels(const Gtk::TreeNodeChildren& children, bool ascending)
{
    auto need_swap = [this,&ascending](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, ascending); };
    bool swap_excecuted = CtMiscUtil::node_siblings_sort(_pCtMainWin->get_tree_store().get_store(), children, need_swap);
    for (auto& child: children)
        if (_tree_sort_level_and_sublevels(child.children(), ascending))
            swap_excecuted = true;
    return swap_excecuted;
}

void CtActions::node_edit()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    CtNodeData nodeData{};
    CtTreeIter ct_tree_iter = _pCtMainWin->curr_tree_iter();
    CtTreeStore& ct_treestore = _pCtMainWin->get_tree_store();
    ct_treestore.get_node_data(ct_tree_iter, nodeData, true/*loadTextBuffer*/);
    CtNodeData newData = nodeData;
    if (not CtDialogs::node_prop_dialog(_("Node Properties"), _pCtMainWin, newData, ct_treestore.get_used_tags())) {
        return;
    }

    // leaving rich text ?
    if (nodeData.syntax != newData.syntax and
        CtConst::RICH_TEXT_ID == nodeData.syntax and
        not CtDialogs::question_dialog(_("Leaving the Node Type Rich Text you will Lose all Formatting for This Node, Do you want to Continue?"), *_pCtMainWin))
    {
        return;
    }

    _pCtConfig->syntaxHighlighting = newData.syntax;

    // update node info, because we might need to delete widgets later
    ct_treestore.update_node_data(ct_tree_iter, newData);

    if (ct_treestore.is_node_bookmarked(ct_tree_iter.get_node_id())) {
        _pCtMainWin->menu_set_bookmark_menu_items();
    }

    if (nodeData.syntax != newData.syntax) {
        // if from/to RICH , change buffer
        if (CtConst::RICH_TEXT_ID == nodeData.syntax or CtConst::RICH_TEXT_ID == newData.syntax) {
            _pCtMainWin->switch_buffer_text_source(ct_tree_iter.get_node_text_buffer(), ct_tree_iter, newData.syntax, nodeData.syntax);
        }
        else {
            // todo: improve code by only changing syntax of buffer and text_view
            _pCtMainWin->switch_buffer_text_source(ct_tree_iter.get_node_text_buffer(), ct_tree_iter, newData.syntax, nodeData.syntax);
        }

        // from RICH to text
        if (CtConst::RICH_TEXT_ID == nodeData.syntax) {
            _pCtMainWin->get_state_machine().delete_states(ct_tree_iter.get_node_id_data_holder());
            _pCtMainWin->get_state_machine().update_state(ct_tree_iter);
        }
    }

    _pCtMainWin->get_text_view().mm().set_editable(not newData.isReadOnly);
    _pCtMainWin->update_selected_node_statusbar_info();
    ct_treestore.update_node_aux_icon(ct_tree_iter);
    _pCtMainWin->window_header_update();
    _pCtMainWin->window_header_update_lock_icon(newData.isReadOnly);
    _pCtMainWin->window_header_update_ghost_icon(newData.excludeMeFromSearch or newData.excludeChildrenFromSearch);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::npro);
    _pCtMainWin->get_text_view().mm().grab_focus();

    // if this node belongs to a shared group, we need to update the other nodes of the group
    CtSharedNodesMap shared_nodes_map;
    if (ct_treestore.populate_shared_nodes_map(shared_nodes_map) > 0u) {
        for (auto& currPair : shared_nodes_map) {
            if (newData.nodeId == currPair.first or
                newData.sharedNodesMasterId == currPair.first)
            {
                // add the master id to the set of non master ids of the group
                currPair.second.insert(currPair.first);
                // loop all the ids of the group
                for (const gint64 nodeId : currPair.second) {
                    if (nodeId != newData.nodeId) {
                        // skip ourselves
                        CtTreeIter other_ct_tree_iter = ct_treestore.get_node_from_node_id(nodeId);
                        if (other_ct_tree_iter) {
                            newData.nodeId = nodeId;
                            newData.sharedNodesMasterId = other_ct_tree_iter.get_node_shared_master_id();
                            newData.sequence = other_ct_tree_iter.get_node_sequence();
                            ct_treestore.update_node_data(other_ct_tree_iter, newData);
                        }
                    }
                }
                break;
            }
        }
    }
}

// Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting
void CtActions::node_inherit_syntax()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;

    const std::string& new_syntax = _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting();
    std::function<void(Gtk::TreeIter)> f_iterate_childs;
    f_iterate_childs = [&](Gtk::TreeIter parent){
        for (auto child : parent->children()) {
            CtTreeIter iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(child);
            std::string node_syntax = iter.get_node_syntax_highlighting();
            if (not iter.get_node_read_only() and node_syntax != new_syntax) {
                // if from/to RICH , change buffer
                if (node_syntax == CtConst::RICH_TEXT_ID || new_syntax == CtConst::RICH_TEXT_ID)
                    _pCtMainWin->switch_buffer_text_source(iter.get_node_text_buffer(), iter, new_syntax, node_syntax);
                else {
                    // todo: improve code by only changing syntax of buffer and text_view
                    _pCtMainWin->switch_buffer_text_source(iter.get_node_text_buffer(), iter, new_syntax, node_syntax);
                }

                // from RICH to text
                if (node_syntax == CtConst::RICH_TEXT_ID) {
                    _pCtMainWin->get_state_machine().delete_states(iter.get_node_id_data_holder());
                    _pCtMainWin->get_state_machine().update_state(iter);
                }
                _pCtMainWin->get_tree_store().update_node_icon(iter);
                iter.pending_edit_db_node_prop();
            }
            f_iterate_childs(child);
        }
    };

    f_iterate_childs(_pCtMainWin->curr_tree_iter());

    // to recover text view
    _pCtMainWin->resetPrevTreeIter();
    _pCtMainWin->get_tree_view().set_cursor(_pCtMainWin->get_tree_store().get_path(_pCtMainWin->curr_tree_iter()));

    _pCtMainWin->update_window_save_needed();
}

void CtActions::node_delete()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    CtTreeStore& ctTreeStore = _pCtMainWin->get_tree_store();
    std::function<void(Gtk::TreeIter, int)> f_collect_ids_to_rm;
    std::list<gint64> nodeIdsToRemove;
    std::list<std::string> lstNodesWarn;
    f_collect_ids_to_rm = [this, &ctTreeStore, &nodeIdsToRemove, &lstNodesWarn, &f_collect_ids_to_rm](Gtk::TreeIter iter,
                                                                                                      const int level) {
        CtTreeIter ctTreeIter = ctTreeStore.to_ct_tree_iter(iter);
        nodeIdsToRemove.push_back(ctTreeIter.get_node_id());
        if (lstNodesWarn.size() > 15) {
            if (lstNodesWarn.size() == 16) {
                lstNodesWarn.push_back(CtConst::CHAR_NEWLINE + "...");
            }
        }
        else {
            lstNodesWarn.push_back(CtConst::CHAR_NEWLINE + str::repeat(CtConst::CHAR_SPACE, level*3) + _pCtConfig->charsListbul[0] + CtConst::CHAR_SPACE + ctTreeIter.get_node_name());
        }
        for (Gtk::TreeIter child : iter->children()) {
            f_collect_ids_to_rm(child, level + 1);
        }
    };
    f_collect_ids_to_rm(_pCtMainWin->curr_tree_iter(), 0);

    Glib::ustring warning_label = str::format(_("Are you sure to <b>Delete the node '%s'?</b>"), str::xml_escape(_pCtMainWin->curr_tree_iter().get_node_name()));
    if (nodeIdsToRemove.size() > 1u) {
        warning_label += str::repeat(CtConst::CHAR_NEWLINE, 2) + _("The node <b>has Children, they will be Deleted too!</b>");
        warning_label += str::xml_escape(str::join(lstNodesWarn, ""));
    }
    if (not CtDialogs::question_dialog(warning_label, *_pCtMainWin)) {
        return;
    }

    // if we delete a shared node master and not all the other members of the group,
    // then we need to move the data to a remaining member of the group
    CtSharedNodesMap shared_nodes_map;
    if (ctTreeStore.populate_shared_nodes_map(shared_nodes_map) > 0u) {
        for (const auto& currPair : shared_nodes_map) {
            if (vec::exists(nodeIdsToRemove, currPair.first)) {
                // we are removing a master, look for a non master in the group not being deleted
                CtTreeIter oldMasterTreeIter = ctTreeStore.get_node_from_node_id(currPair.first);
                if (oldMasterTreeIter) {
                    for (const gint64 new_master_id : currPair.second) {
                        if (not vec::exists(nodeIdsToRemove, new_master_id)) {
                            // found a non master in the group that is not being deleted
                            CtTreeIter newMasterTreeIter = ctTreeStore.get_node_from_node_id(new_master_id);
                            if (newMasterTreeIter) {
                                // copy over the data from the old master
                                CtNodeData nodeData{};
                                ctTreeStore.get_node_data(oldMasterTreeIter, nodeData, true/*loadTextBuffer*/);
                                nodeData.nodeId = new_master_id;
                                nodeData.sequence = newMasterTreeIter.get_node_sequence();
                                ctTreeStore.update_node_data(newMasterTreeIter, nodeData);
                                newMasterTreeIter.pending_edit_db_node_prop();
                                newMasterTreeIter.pending_edit_db_node_buff();
                                newMasterTreeIter.pending_edit_db_node_hier(); // master id is in the hierarchy table
                                // update other non masters to point to the new master
                                for (const gint64 nonMasterId : currPair.second) {
                                    if (nonMasterId != new_master_id and not vec::exists(nodeIdsToRemove, nonMasterId)) {
                                        CtTreeIter nonMasterTreeIter = ctTreeStore.get_node_from_node_id(nonMasterId);
                                        if (nonMasterTreeIter) {
                                            nonMasterTreeIter.set_node_shared_master_id(new_master_id);
                                            nonMasterTreeIter.pending_edit_db_node_hier(); // master id is in the hierarchy table
                                        }
                                        else {
                                            spdlog::error("!! unexp nonMasterId {} not in tree", nonMasterId);
                                        }
                                    }
                                }
                                break;
                            }
                            spdlog::error("!! unexp new_master_id {} not in tree", new_master_id);
                        }
                    }
                }
                else {
                    spdlog::error("!! unexp old_master_id {} not in tree", currPair.first);
                }
            }
        }
    }

    // next selected node will be previous sibling or next sibling or parent or None
    Gtk::TreeIter new_iter = --_pCtMainWin->curr_tree_iter();
    if (not new_iter) new_iter = ++_pCtMainWin->curr_tree_iter();
    if (not new_iter) new_iter = _pCtMainWin->curr_tree_iter().parent();

    _pCtMainWin->resetPrevTreeIter();
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::ndel);

    Gtk::TreeIter erase_iter = _pCtMainWin->curr_tree_iter();

    if (new_iter) {
        _pCtMainWin->get_tree_view().set_cursor_safe(new_iter);
        _pCtMainWin->get_text_view().mm().grab_focus();
    }
    else {
        _curr_buffer()->set_text("");
        _pCtMainWin->window_header_update();
        _pCtMainWin->update_selected_node_statusbar_info();
        _pCtMainWin->get_text_view().mm().set_sensitive(false);
    }

    ctTreeStore.get_store()->erase(erase_iter);

    bool anyRemovedBookmarked{false};
    for (gint64 nodeId : nodeIdsToRemove) {
        if (_pCtMainWin->get_tree_store().bookmarks_remove(nodeId)) {
            anyRemovedBookmarked = true;
        }
    }
    if (anyRemovedBookmarked) {
        _pCtMainWin->menu_set_bookmark_menu_items();
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::book);
    }
}

void CtActions::node_toggle_read_only()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    CtTreeIter currTreeIter = _pCtMainWin->curr_tree_iter();
    const bool node_is_ro = not currTreeIter.get_node_read_only();
    currTreeIter.set_node_read_only(node_is_ro);
    _pCtMainWin->get_text_view().mm().set_editable(not node_is_ro);
    _pCtMainWin->window_header_update_lock_icon(node_is_ro);
    _pCtMainWin->update_selected_node_statusbar_info();
    CtTreeStore& ct_treestore = _pCtMainWin->get_tree_store();
    ct_treestore.update_node_aux_icon(currTreeIter);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::npro);
    _pCtMainWin->get_text_view().mm().grab_focus();

    // if this node belongs to a shared group, we need to update all the nodes of the group
    CtSharedNodesMap shared_nodes_map;
    if (ct_treestore.populate_shared_nodes_map(shared_nodes_map) > 0u) {
        const gint64 currNodeId = currTreeIter.get_node_id();
        const gint64 currNodeMasterId = currTreeIter.get_node_shared_master_id();
        for (auto& currPair : shared_nodes_map) {
            if (currNodeId == currPair.first or
                currNodeMasterId == currPair.first)
            {
                // add the master id to the set of non master ids of the group
                currPair.second.insert(currPair.first);
                // loop all the ids of the group
                for (const gint64 nodeId : currPair.second) {
                    CtTreeIter other_ct_tree_iter = ct_treestore.get_node_from_node_id(nodeId);
                    if (other_ct_tree_iter) {
                        other_ct_tree_iter->set_value(ct_treestore.get_columns().colNodeIsReadOnly, node_is_ro);
                        ct_treestore.update_node_aux_icon(other_ct_tree_iter);
                    }
                }
                break;
            }
        }
    }
}

void CtActions::_node_date(const bool from_sel_not_root)
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    const time_t time = std::time(nullptr);
    const Glib::ustring year = str::time_format("%Y", time);
    const Glib::ustring month = str::time_format("%B", time);
    const Glib::ustring day = str::time_format("%d %a", time);

    _pCtMainWin->get_state_machine().set_go_bk_fw_active(true); // so nodes won't be in the list of visited
    Gtk::TreeIter nodeParent;
    if (from_sel_not_root) {
        if (not _is_there_selected_node_or_error()) return;
        nodeParent = _pCtMainWin->curr_tree_iter();
    }
    Gtk::TreeIter treeIterYear = node_child_exist_or_create(nodeParent, year, false/*focusIfExisting*/);
    Gtk::TreeIter treeIterMonth = node_child_exist_or_create(treeIterYear, month, false/*focusIfExisting*/);
    _pCtMainWin->get_state_machine().set_go_bk_fw_active(false);
    (void)node_child_exist_or_create(treeIterMonth, day, true/*focusIfExisting*/);
}

void CtActions::node_up()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    auto prev_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(--_pCtMainWin->curr_tree_iter());
    if (not prev_iter) return;
    _pCtMainWin->get_tree_store().get_store()->iter_swap(_pCtMainWin->curr_tree_iter(), prev_iter);
    auto cur_seq_num = _pCtMainWin->curr_tree_iter().get_node_sequence();
    auto prev_seq_num = prev_iter.get_node_sequence();
    _pCtMainWin->curr_tree_iter().set_node_sequence(prev_seq_num);
    prev_iter.set_node_sequence(cur_seq_num);
    _pCtMainWin->curr_tree_iter().pending_edit_db_node_hier();
    prev_iter.pending_edit_db_node_hier();
    _pCtMainWin->get_tree_view().set_cursor(_pCtMainWin->get_tree_store().get_path(_pCtMainWin->curr_tree_iter()));
    _pCtMainWin->update_window_save_needed();
}

void CtActions::node_down()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    auto next_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(++_pCtMainWin->curr_tree_iter());
    if (not next_iter) return;
    _pCtMainWin->get_tree_store().get_store()->iter_swap(_pCtMainWin->curr_tree_iter(), next_iter);
    auto cur_seq_num = _pCtMainWin->curr_tree_iter().get_node_sequence();
    auto next_seq_num = next_iter.get_node_sequence();
    _pCtMainWin->curr_tree_iter().set_node_sequence(next_seq_num);
    next_iter.set_node_sequence(cur_seq_num);
    _pCtMainWin->curr_tree_iter().pending_edit_db_node_hier();
    next_iter.pending_edit_db_node_hier();
    _pCtMainWin->get_tree_view().set_cursor(_pCtMainWin->get_tree_store().get_path(_pCtMainWin->curr_tree_iter()));
    _pCtMainWin->update_window_save_needed();
}

void CtActions::node_right()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    auto prev_iter = --_pCtMainWin->curr_tree_iter();
    if (not prev_iter) return;
    node_move_after(_pCtMainWin->curr_tree_iter(), prev_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

void CtActions::node_left()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    if (not father_iter) return;
    node_move_after(_pCtMainWin->curr_tree_iter(), father_iter->parent(), father_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

void CtActions::node_change_father()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    CtTreeIter old_father_iter = _pCtMainWin->curr_tree_iter().parent();
    CtTreeIter father_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(CtDialogs::choose_node_dialog(_pCtMainWin,
                                   _pCtMainWin->get_tree_view(), _("Select the New Parent"), &_pCtMainWin->get_tree_store(), _pCtMainWin->curr_tree_iter()));
    if (not father_iter) return;
    gint64 curr_node_id = _pCtMainWin->curr_tree_iter().get_node_id();
    gint64 old_father_node_id = old_father_iter.get_node_id();
    gint64 new_father_node_id = father_iter.get_node_id();
    if (curr_node_id == new_father_node_id) {
        CtDialogs::error_dialog(_("The new parent can't be the very node to move!"), *_pCtMainWin);
        return;
    }
    if (old_father_node_id != -1 && new_father_node_id == old_father_node_id) {
        CtDialogs::info_dialog(_("The new chosen parent is still the old parent!"), *_pCtMainWin);
        return;
    }
    for (CtTreeIter move_towards_top_iter = father_iter.parent(); move_towards_top_iter; move_towards_top_iter = move_towards_top_iter.parent())
        if (move_towards_top_iter.get_node_id() == curr_node_id) {
            CtDialogs::error_dialog(_("The new parent can't be one of his children!"), *_pCtMainWin);
            return;
        }

    node_move_after(_pCtMainWin->curr_tree_iter(), father_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

bool CtActions::node_move(Gtk::TreeModel::Path src_path, Gtk::TreeModel::Path dest_path, bool only_test_dest)
{
    if (src_path == dest_path) {
        if (not only_test_dest)
            CtDialogs::error_dialog(_("The new parent can't be the very node to move!"), *_pCtMainWin);
        return false;
    }
    if (dest_path.is_descendant(src_path)) {
        if (not only_test_dest)
            CtDialogs::error_dialog(_("The new parent can't be one of his children!"), *_pCtMainWin);
        return false;
    }
    if (only_test_dest)
        return true;

    Gtk::TreeModel::Path father_path{dest_path};
    father_path.up();
    CtTreeIter father_dest_iter = _pCtMainWin->get_tree_store().get_iter(father_path);
    CtTreeIter src_iter = _pCtMainWin->get_tree_store().get_iter(src_path);

    // 3 cases:
    // 1 - dest iter exists - insert before it, or at very first position
    // 2 - dest iter doesn't exist and there're siblings - insert after siblings
    // 3 - dest iter doesn't exist and no siblings - insert as a first child of father

    // case 1
    if (_pCtMainWin->get_tree_store().get_iter(dest_path)) {
        if (dest_path.prev()) {
            CtTreeIter dest_iter = _pCtMainWin->get_tree_store().get_iter(dest_path); // move iter to insert `before` it
            node_move_after(src_iter, father_dest_iter, dest_iter, false);
        } else {
            node_move_after(src_iter, father_dest_iter, CtTreeIter(), true); // put it as first
        }
    } else { // case 2, 3
        if (dest_path.prev()) {
            CtTreeIter dest_iter = _pCtMainWin->get_tree_store().get_iter(dest_path); // put after siblings
            node_move_after(src_iter, father_dest_iter, dest_iter, false);
        } else {
            node_move_after(src_iter, father_dest_iter, CtTreeIter(), true); // put it as first child
        }
    }
    return true;
}

void CtActions::tree_sort_ascending()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (_tree_sort_level_and_sublevels(_pCtMainWin->get_tree_store().get_store()->children(), true)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(Gtk::TreeIter(), true);
        _pCtMainWin->update_window_save_needed();
    }
}

void CtActions::tree_sort_descending()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (_tree_sort_level_and_sublevels(_pCtMainWin->get_tree_store().get_store()->children(), false)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(Gtk::TreeIter(), true);
        _pCtMainWin->update_window_save_needed();
    }
}

void CtActions::node_siblings_sort_ascending()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _pCtMainWin->get_tree_store().get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, true); };
    if (CtMiscUtil::node_siblings_sort(_pCtMainWin->get_tree_store().get_store(), children, need_swap)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(father_iter, true);
        _pCtMainWin->update_window_save_needed();
    }
}

void CtActions::node_siblings_sort_descending()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _pCtMainWin->get_tree_store().get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, false); };
    if (CtMiscUtil::node_siblings_sort(_pCtMainWin->get_tree_store().get_store(), children, need_swap)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(father_iter, true);
        _pCtMainWin->update_window_save_needed();
    }
}

// Go to the Previous Visited Node
void CtActions::node_go_back()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    CtStateMachine& ct_state_machine = _pCtMainWin->get_state_machine();
    ct_state_machine.set_go_bk_fw_active(true);
    auto on_scope_exit = scope_guard([&](void*) {
        ct_state_machine.set_go_bk_fw_active(false);
        _in_action = false;
    });

    gint64 new_node_id = ct_state_machine.requested_visited_previous();
    while (new_node_id > 0) {
        auto node_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(new_node_id);
        if (node_iter) {
            _pCtMainWin->get_tree_view().set_cursor_safe(node_iter);
            break;
        }
        new_node_id = ct_state_machine.requested_visited_previous();
    }
}

// Go to the Next Visited Node
void CtActions::node_go_forward()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    CtStateMachine& ct_state_machine = _pCtMainWin->get_state_machine();
    ct_state_machine.set_go_bk_fw_active(true);
    auto on_scope_exit = scope_guard([&](void*) {
        ct_state_machine.set_go_bk_fw_active(false);
        _in_action = false;
    });

    gint64 new_node_id = ct_state_machine.requested_visited_next();
    while (new_node_id > 0) {
        auto node_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(new_node_id);
        if (node_iter) {
            _pCtMainWin->get_tree_view().set_cursor_safe(node_iter);
            break;
        }
        new_node_id = ct_state_machine.requested_visited_next();
    }
}

void CtActions::bookmark_curr_node()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    gint64 node_id = _pCtMainWin->curr_tree_iter().get_node_id();

    if (_pCtMainWin->get_tree_store().bookmarks_add(node_id)) {
        _pCtMainWin->menu_set_bookmark_menu_items();
        _pCtMainWin->get_tree_store().update_node_aux_icon(_pCtMainWin->curr_tree_iter());
        _pCtMainWin->window_header_update_bookmark_icon(true);
        _pCtMainWin->menu_update_bookmark_menu_item(true);
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::book);
    }
}

void CtActions::bookmark_curr_node_remove()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_there_selected_node_or_error()) return;
    gint64 node_id = _pCtMainWin->curr_tree_iter().get_node_id();

    if (_pCtMainWin->get_tree_store().bookmarks_remove(node_id)) {
        _pCtMainWin->menu_set_bookmark_menu_items();
        _pCtMainWin->get_tree_store().update_node_aux_icon(_pCtMainWin->curr_tree_iter());
        _pCtMainWin->window_header_update_bookmark_icon(false);
        _pCtMainWin->menu_update_bookmark_menu_item(false);
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::book);
    }
}

void CtActions::bookmarks_handle()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    CtDialogs::bookmarks_handle_dialog(_pCtMainWin);
}

void CtActions::tree_info()
{
    if (not _is_tree_not_empty_or_error()) return;
    CtSummaryInfo summaryInfo{};
    if (_pCtMainWin->get_tree_store().populate_summary_info(summaryInfo)) {
        CtDialogs::summary_info_dialog(_pCtMainWin, summaryInfo);
    }
}

void CtActions::tree_clear_property_exclude_from_search()
{
    if (_in_action) { spdlog::debug("?? 2*{}", __FUNCTION__); return; }
    _in_action = true;
    auto on_scope_exit = scope_guard([this](void*) { _in_action = false; });

    if (not _is_tree_not_empty_or_error()) return;
    const unsigned nodes_properties_changed = _pCtMainWin->get_tree_store().tree_clear_property_exclude_from_search();
    if (nodes_properties_changed > 0u) {
        _pCtMainWin->window_header_update_ghost_icon(false);
    }
    CtDialogs::info_dialog(str::format(_("%s Nodes Properties Changed"), std::to_string(nodes_properties_changed)), *_pCtMainWin);
}

void CtActions::node_link_to_clipboard()
{
    if (not _is_there_selected_node_or_error()) return;
    CtClipboard(_pCtMainWin).node_link_to_clipboard(_pCtMainWin->curr_tree_iter());
}
