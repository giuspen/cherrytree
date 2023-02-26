/*
 * ct_actions_tree.cc
 *
 * Copyright 2009-2023
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

bool CtActions::_is_there_table_selection_or_error()
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
            auto pTableCommon = dynamic_cast<CtTableCommon*>(widgets.front());
            if (pTableCommon) {
                curr_table_anchor = pTableCommon;
                return true;
            }
        }
    }
    CtDialogs::error_dialog(_("No Table is Selected."), *_pCtMainWin);
    return false;
}

// Put Selection Upon the achrored widget
void CtActions::object_set_selection(CtAnchoredWidget* widget)
{
    Gtk::TextIter iter_object = _curr_buffer()->get_iter_at_child_anchor(widget->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_object;
    iter_bound.forward_char();
    if (dynamic_cast<CtImage*>(widget)) {
        _pCtMainWin->get_text_view().grab_focus();
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
    _pCtMainWin->signal_app_tree_node_paste();
}

void CtActions::node_subnodes_paste2(CtTreeIter& other_ct_tree_iter,
                                     CtMainWin* pWinToCopyFrom)
{
    // create duplicate of the top node
    _node_add(true/*is_duplicate*/, false/*add_as_child*/, &other_ct_tree_iter, pWinToCopyFrom);

    Gtk::TreeIter new_top_iter = _pCtMainWin->curr_tree_iter();

    // function to duplicate a node
    auto duplicate_subnode = [&](CtTreeIter old_iter, Gtk::TreeIter new_parent) {
        CtNodeData node_data;
        std::shared_ptr<CtNodeState> node_state;
        pWinToCopyFrom->get_tree_store().get_node_data(old_iter, node_data);
        if (node_data.syntax != CtConst::RICH_TEXT_ID) {
            node_data.rTextBuffer = _pCtMainWin->get_new_text_buffer(node_data.rTextBuffer->get_text());
            node_data.anchoredWidgets.clear();
        }
        else {
            CtStateMachine& state_machine_from = pWinToCopyFrom->get_state_machine();
            state_machine_from.update_state(old_iter);
            node_state = state_machine_from.requested_state_current(old_iter.get_node_id());
            node_data.anchoredWidgets.clear();                          // node_state will be used instead
            node_data.rTextBuffer = _pCtMainWin->get_new_text_buffer(); // node_state will be used instead
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
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::node_subnodes_duplicate()
{
    if (not _is_there_selected_node_or_error()) return;
    CtTreeIter top_iter = _pCtMainWin->curr_tree_iter();
    node_subnodes_paste2(top_iter, _pCtMainWin);
}

void CtActions::_node_add(const bool is_duplicate,
                          const bool add_as_child,
                          const CtTreeIter* pCtTreeIterFrom/*=nullptr*/,
                          CtMainWin* pWinToCopyFrom/*=nullptr*/)
{
    CtNodeData nodeData;
    std::shared_ptr<CtNodeState> node_state;
    if (is_duplicate) {
        pWinToCopyFrom->get_tree_store().get_node_data(*pCtTreeIterFrom, nodeData);

        if (nodeData.syntax != CtConst::RICH_TEXT_ID) {
            nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer(nodeData.rTextBuffer->get_text());
            nodeData.anchoredWidgets.clear();
        }
        else {
            CtStateMachine& state_machine_from = pWinToCopyFrom->get_state_machine();
            state_machine_from.update_state(*pCtTreeIterFrom);
            node_state = state_machine_from.requested_state_current(pCtTreeIterFrom->get_node_id());
            nodeData.anchoredWidgets.clear();                          // node_state will be used instead
            nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer(); // node_state will be used instead
        }
    }
    else {
        std::string title = add_as_child ? _("New Child Node Properties") : _("New Node Properties");
        CtTreeIter currTreeIter = _pCtMainWin->curr_tree_iter();
        nodeData.syntax = currTreeIter ? currTreeIter.get_node_syntax_highlighting() : CtConst::RICH_TEXT_ID;
        if (not CtDialogs::node_prop_dialog(title, _pCtMainWin, nodeData, _pCtMainWin->get_tree_store().get_used_tags())) {
            return;
        }
    }
    (void)_node_add_with_data(_pCtMainWin->curr_tree_iter(), nodeData, add_as_child, node_state);
}

Gtk::TreeIter CtActions::_node_add_with_data(Gtk::TreeIter curr_iter, CtNodeData& nodeData, bool add_as_child, std::shared_ptr<CtNodeState> node_state)
{
    if (not nodeData.rTextBuffer) {
        nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer();
    }
    nodeData.tsCreation = std::time(nullptr);
    nodeData.tsLastSave = nodeData.tsCreation;
    nodeData.nodeId = _pCtMainWin->get_tree_store().node_id_get();

    _pCtMainWin->update_window_save_needed();
    _pCtConfig->syntaxHighlighting = nodeData.syntax;

    Gtk::TreeIter nodeIter;
    if (add_as_child) {
        nodeIter = _pCtMainWin->get_tree_store().append_node(&nodeData, &curr_iter/*as parent*/);
    }
    else if (curr_iter) {
        nodeIter = _pCtMainWin->get_tree_store().insert_node(&nodeData, curr_iter/*after*/);
    }
    else {
        nodeIter = _pCtMainWin->get_tree_store().append_node(&nodeData);
    }

    if (node_state) {
        _pCtMainWin->load_buffer_from_state(node_state, _pCtMainWin->get_tree_store().to_ct_tree_iter(nodeIter));
    }
    _pCtMainWin->get_tree_store().to_ct_tree_iter(nodeIter).pending_new_db_node();
    _pCtMainWin->get_tree_store().nodes_sequences_fix(nodeIter->parent(), false);
    _pCtMainWin->get_tree_store().update_node_aux_icon(nodeIter);
    _pCtMainWin->get_tree_view().set_cursor_safe(nodeIter);
    _pCtMainWin->get_text_view().grab_focus();
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
    CtNodeData nodeData;
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
    Gtk::TreeIter new_node_iter;
    if (brother_iter)   new_node_iter = _pCtMainWin->get_tree_store().get_store()->insert_after(brother_iter);
    else if (set_first) new_node_iter = _pCtMainWin->get_tree_store().get_store()->prepend(father_iter->children());
    else                new_node_iter = _pCtMainWin->get_tree_store().get_store()->append(father_iter->children());

    // we move also all the children
    std::function<void(Gtk::TreeIter&,Gtk::TreeIter&)> node_move_data_and_children;
    node_move_data_and_children = [this, &node_move_data_and_children](Gtk::TreeIter& old_iter,Gtk::TreeIter& new_iter) {
        CtNodeData node_data;
        _pCtMainWin->get_tree_store().get_node_data(old_iter, node_data);
        _pCtMainWin->get_tree_store().update_node_data(new_iter, node_data);
        for (Gtk::TreeIter child: old_iter->children()) {
            Gtk::TreeIter new_child = _pCtMainWin->get_tree_store().get_store()->append(new_iter->children());
            node_move_data_and_children(child, new_child);
        }
    };
    node_move_data_and_children(iter_to_move, new_node_iter);

    // now we can remove the old iter (and all children)
    _pCtMainWin->resetPrevTreeIter();
    _pCtMainWin->get_tree_store().get_store()->erase(iter_to_move);
    _pCtMainWin->get_tree_store().to_ct_tree_iter(new_node_iter).pending_edit_db_node_hier();

    _pCtMainWin->get_tree_store().nodes_sequences_fix(Gtk::TreeIter(), true);
    if (father_iter)
        _pCtMainWin->get_tree_view().expand_row(_pCtMainWin->get_tree_store().get_path(father_iter), false);
    else
        _pCtMainWin->get_tree_view().expand_row(_pCtMainWin->get_tree_store().get_path(new_node_iter), false);    
    Gtk::TreePath new_node_path = _pCtMainWin->get_tree_store().get_path(new_node_iter);
    _pCtMainWin->get_tree_view().collapse_row(new_node_path);
    _pCtMainWin->get_tree_view().set_cursor(new_node_path);
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
    if (not _is_there_selected_node_or_error()) return;
    CtNodeData nodeData;
    _pCtMainWin->get_tree_store().get_node_data(_pCtMainWin->curr_tree_iter(), nodeData);
    CtNodeData newData = nodeData;
    if (not CtDialogs::node_prop_dialog(_("Node Properties"), _pCtMainWin, newData, _pCtMainWin->get_tree_store().get_used_tags()))
        return;

    _pCtConfig->syntaxHighlighting = newData.syntax;

    // leaving rich text
    if (nodeData.syntax != newData.syntax)
        if (nodeData.syntax == CtConst::RICH_TEXT_ID)
            if (not CtDialogs::question_dialog(_("Leaving the Node Type Rich Text you will Lose all Formatting for This Node, Do you want to Continue?"), *_pCtMainWin))
                return;

    // update node info, because we might need to delete widgets later
    _pCtMainWin->get_tree_store().update_node_data(_pCtMainWin->curr_tree_iter(), newData);

    if (nodeData.syntax != newData.syntax)
    {
        // if from/to RICH , change buffer
        if (nodeData.syntax == CtConst::RICH_TEXT_ID || newData.syntax == CtConst::RICH_TEXT_ID)
            _pCtMainWin->switch_buffer_text_source(_pCtMainWin->curr_tree_iter().get_node_text_buffer(), _pCtMainWin->curr_tree_iter(), newData.syntax, nodeData.syntax);
        else {
            // todo: improve code by only changing syntax of buffer and text_view
            _pCtMainWin->switch_buffer_text_source(_pCtMainWin->curr_tree_iter().get_node_text_buffer(), _pCtMainWin->curr_tree_iter(), newData.syntax, nodeData.syntax);
        }

        // from RICH to text
        if (nodeData.syntax == CtConst::RICH_TEXT_ID) {
            _pCtMainWin->get_state_machine().delete_states(_pCtMainWin->curr_tree_iter().get_node_id());
            _pCtMainWin->get_state_machine().update_state(_pCtMainWin->curr_tree_iter());
        }
    }

    _pCtMainWin->get_text_view().set_editable(!newData.isReadOnly);
    _pCtMainWin->update_selected_node_statusbar_info();
    _pCtMainWin->get_tree_store().update_node_aux_icon(_pCtMainWin->curr_tree_iter());
    _pCtMainWin->window_header_update();
    _pCtMainWin->window_header_update_lock_icon(newData.isReadOnly);
    _pCtMainWin->window_header_update_ghost_icon(newData.excludeMeFromSearch or newData.excludeChildrenFromSearch);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::npro);
    _pCtMainWin->get_text_view().grab_focus();
}

// Change the Selected Node's Children Syntax Highlighting to the Parent's Syntax Highlighting
void CtActions::node_inherit_syntax()
{
    if (not _is_there_selected_node_or_error()) return;

    const std::string& new_syntax = _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting();
    std::function<void(Gtk::TreeIter)> iterate_childs;
    iterate_childs = [&](Gtk::TreeIter parent){
        for (auto child: parent->children())
        {
            CtTreeIter iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(child);
            std::string node_syntax = iter.get_node_syntax_highlighting();
            if (not iter.get_node_read_only() && node_syntax != new_syntax)
            {
                // if from/to RICH , change buffer
                if (node_syntax == CtConst::RICH_TEXT_ID || new_syntax == CtConst::RICH_TEXT_ID)
                    _pCtMainWin->switch_buffer_text_source(iter.get_node_text_buffer(), iter, new_syntax, node_syntax);
                else {
                    // todo: improve code by only changing syntax of buffer and text_view
                    _pCtMainWin->switch_buffer_text_source(iter.get_node_text_buffer(), iter, new_syntax, node_syntax);
                }

                // from RICH to text
                if (node_syntax == CtConst::RICH_TEXT_ID) {
                    _pCtMainWin->get_state_machine().delete_states(iter.get_node_id());
                    _pCtMainWin->get_state_machine().update_state(iter);
                }
                _pCtMainWin->get_tree_store().update_node_icon(iter);
                iter.pending_edit_db_node_prop();
            }
            iterate_childs(child);
        }
    };

    iterate_childs(_pCtMainWin->curr_tree_iter());

    // to recover text view
    _pCtMainWin->resetPrevTreeIter();
    _pCtMainWin->get_tree_view().set_cursor(_pCtMainWin->get_tree_store().get_path(_pCtMainWin->curr_tree_iter()));

    _pCtMainWin->update_window_save_needed();
}

// Delete the Selected Node
void CtActions::node_delete()
{
    if (not _is_there_selected_node_or_error()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    std::function<void(Gtk::TreeIter, int, std::list<gint64>&, std::list<std::string>&)> collect_children;
    collect_children = [this, &collect_children](Gtk::TreeIter iter,
                                                 int level,
                                                 std::list<gint64>& listIds,
                                                 std::list<std::string>& listWarns) {
        CtTreeIter ctTreeIter = _pCtMainWin->get_tree_store().to_ct_tree_iter(iter);
        listIds.push_back(ctTreeIter.get_node_id());
        if (listWarns.size() > 15) {
            if (listWarns.size() == 16) {
                listWarns.push_back(std::string(CtConst::CHAR_NEWLINE) + "...");
            }
        }
        else {
            listWarns.push_back(CtConst::CHAR_NEWLINE + str::repeat(CtConst::CHAR_SPACE, level*3) + _pCtConfig->charsListbul[0] + CtConst::CHAR_SPACE + ctTreeIter.get_node_name());
            for (auto child : iter->children()) {
                collect_children(child, level + 1, listIds, listWarns);
            }
        }
    };

    std::list<gint64> lstNodesIds;
    std::list<std::string> lstNodesWarn;
    collect_children(_pCtMainWin->curr_tree_iter(), 0, lstNodesIds, lstNodesWarn);
    Glib::ustring warning_label = str::format(_("Are you sure to <b>Delete the node '%s'?</b>"), str::xml_escape(_pCtMainWin->curr_tree_iter().get_node_name()));
    if (not _pCtMainWin->curr_tree_iter()->children().empty()) {
        warning_label += str::repeat(CtConst::CHAR_NEWLINE, 2) + _("The node <b>has Children, they will be Deleted too!</b>");
        warning_label += str::xml_escape(str::join(lstNodesWarn, ""));
    }
    if (not CtDialogs::question_dialog(warning_label, *_pCtMainWin)) {
        return;
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
        _pCtMainWin->get_text_view().grab_focus();
    }
    else {
        _curr_buffer()->set_text("");
        _pCtMainWin->window_header_update();
        _pCtMainWin->update_selected_node_statusbar_info();
        _pCtMainWin->get_text_view().set_sensitive(false);
    }

    _pCtMainWin->get_tree_store().get_store()->erase(erase_iter);

    bool anyRemovedBookmarked{false};
    for (gint64 nodeId : lstNodesIds) {
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
    if (not _is_there_selected_node_or_error()) return;
    bool node_is_ro = not _pCtMainWin->curr_tree_iter().get_node_read_only();
    _pCtMainWin->curr_tree_iter().set_node_read_only(node_is_ro);
    _pCtMainWin->get_text_view().set_editable(!node_is_ro);
    _pCtMainWin->window_header_update_lock_icon(node_is_ro);
    _pCtMainWin->update_selected_node_statusbar_info();
    _pCtMainWin->get_tree_store().update_node_aux_icon(_pCtMainWin->curr_tree_iter());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::npro);
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::_node_date(const bool from_sel_not_root)
{
    const time_t time = std::time(nullptr);
    const Glib::ustring year = str::time_format("%Y", time);
    const Glib::ustring month = str::time_format("%B", time);
    const Glib::ustring day = str::time_format("%d %a", time);

    _pCtMainWin->get_state_machine().set_go_bk_fw_click(true); // so nodes won't be in the list of visited
    Gtk::TreeIter nodeParent;
    if (from_sel_not_root) {
        if (not _is_there_selected_node_or_error()) return;
        nodeParent = _pCtMainWin->curr_tree_iter();
    }
    Gtk::TreeIter treeIterYear = node_child_exist_or_create(nodeParent, year, false/*focusIfExisting*/);
    Gtk::TreeIter treeIterMonth = node_child_exist_or_create(treeIterYear, month, false/*focusIfExisting*/);
    _pCtMainWin->get_state_machine().set_go_bk_fw_click(false);
    (void)node_child_exist_or_create(treeIterMonth, day, true/*focusIfExisting*/);
}

void CtActions::node_up()
{
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
    if (not _is_there_selected_node_or_error()) return;
    auto prev_iter = --_pCtMainWin->curr_tree_iter();
    if (not prev_iter) return;
    node_move_after(_pCtMainWin->curr_tree_iter(), prev_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

void CtActions::node_left()
{
    if (not _is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    if (not father_iter) return;
    node_move_after(_pCtMainWin->curr_tree_iter(), father_iter->parent(), father_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

void CtActions::node_change_father()
{
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

//"""Sorts the Tree Ascending"""
void CtActions::tree_sort_ascending()
{
    if (_tree_sort_level_and_sublevels(_pCtMainWin->get_tree_store().get_store()->children(), true)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(Gtk::TreeIter(), true);
        _pCtMainWin->update_window_save_needed();
    }
}

//"""Sorts the Tree Ascending"""
void CtActions::tree_sort_descending()
{
    if (_tree_sort_level_and_sublevels(_pCtMainWin->get_tree_store().get_store()->children(), false)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(Gtk::TreeIter(), true);
        _pCtMainWin->update_window_save_needed();
    }
}

//"""Sorts all the Siblings of the Selected Node Ascending"""
void CtActions::node_siblings_sort_ascending()
{
    if (not _is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _pCtMainWin->get_tree_store().get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, true); };
    if (CtMiscUtil::node_siblings_sort(_pCtMainWin->get_tree_store().get_store(), children, need_swap)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(father_iter, true);
        _pCtMainWin->update_window_save_needed();
    }
}

//"""Sorts all the Siblings of the Selected Node Descending"""
void CtActions::node_siblings_sort_descending()
{
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
    _pCtMainWin->get_state_machine().set_go_bk_fw_click(true);
    auto on_scope_exit = scope_guard([&](void*) { _pCtMainWin->get_state_machine().set_go_bk_fw_click(false); });

    auto new_node_id = _pCtMainWin->get_state_machine().requested_visited_previous();
    if (new_node_id > 0) {
        auto node_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(new_node_id);
        if (node_iter)
            _pCtMainWin->get_tree_view().set_cursor_safe(node_iter);
        else
            node_go_back();
    }
}

// Go to the Next Visited Node
void CtActions::node_go_forward()
{
    _pCtMainWin->get_state_machine().set_go_bk_fw_click(true);
    auto on_scope_exit = scope_guard([&](void*) { _pCtMainWin->get_state_machine().set_go_bk_fw_click(false); });

    auto new_node_id = _pCtMainWin->get_state_machine().requested_visited_next();
    if (new_node_id > 0) {
        auto node_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(new_node_id);
        if (node_iter)
            _pCtMainWin->get_tree_view().set_cursor_safe(node_iter);
        else
            node_go_forward();
    }
}

void CtActions::bookmark_curr_node()
{
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
    CtDialogs::bookmarks_handle_dialog(_pCtMainWin);
}

void CtActions::tree_info()
{
    if (not _is_tree_not_empty_or_error()) return;
    CtSummaryInfo summaryInfo{};
    _pCtMainWin->get_tree_store().populate_summary_info(summaryInfo);
    CtDialogs::summary_info_dialog(_pCtMainWin, summaryInfo);
}

void CtActions::tree_clear_property_exclude_from_search()
{
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
