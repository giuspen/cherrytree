/*
 * ct_actions_tree.cc
 *
 * Copyright 2009-2020
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
    if (!_pCtMainWin->get_tree_store().get_iter_first()) {
        CtDialogs::error_dialog(_("The Tree is Empty!"), *_pCtMainWin);
        return false;
    }
    return true;
}

bool CtActions::_is_curr_node_not_read_only_or_error()
{
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) {
        CtDialogs::error_dialog(_("The Selected Node is Read Only"), *_pCtMainWin);
        return false;
    }
    return true;
}

// Returns True if ok (no syntax highlighting) or False and prompts error dialog
bool CtActions::_is_curr_node_not_syntax_highlighting_or_error(bool plain_text_ok /*=false*/)
{
    if (_pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID
        || (plain_text_ok && _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() == CtConst::PLAIN_TEXT_ID))
        return true;
    if (!plain_text_ok)
        CtDialogs::warning_dialog(_("This Feature is Available Only in Rich Text Nodes"), *_pCtMainWin);
    else
        CtDialogs::warning_dialog(_("This Feature is Not Available in Automatic Syntax Highlighting Nodes"), *_pCtMainWin);
    return false;
}

// Returns True if ok (there's a selection) or False and prompts error dialog
bool CtActions::_is_there_text_selection_or_error()
{
    if (!_is_there_selected_node_or_error()) return false;
    if (!_curr_buffer()->get_has_selection())
    {
        CtDialogs::error_dialog(_("No Text is Selected"), *_pCtMainWin);
        return false;
    }
    return true;
}

// Put Selection Upon the achrored widget
void CtActions::object_set_selection(CtAnchoredWidget* widget)
{
    Gtk::TextIter iter_object = _curr_buffer()->get_iter_at_child_anchor(widget->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_object;
    iter_bound.forward_char();
    if (dynamic_cast<CtImage*>(widget))
        _pCtMainWin->get_text_view().grab_focus();
    _curr_buffer()->select_range(iter_object, iter_bound);
}

// Returns True if there's not a node selected or is not rich text
bool CtActions::_node_sel_and_rich_text()
{
    if (!_is_there_selected_node_or_error()) return false;
    if (!_is_curr_node_not_syntax_highlighting_or_error()) return false;
    return true;
}

void CtActions::node_subnodes_duplicate()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter top_iter = _pCtMainWin->curr_tree_iter();
    // create duplicate of the selected node
    _node_add(true, false);
    Gtk::TreeIter new_top_iter = _pCtMainWin->curr_tree_iter();

    // function to duplicate a node
    auto duplicate_subnode = [&](CtTreeIter old_iter, Gtk::TreeIter new_parent) {
        CtNodeData node_data;
        std::shared_ptr<CtNodeState> node_state;
        _pCtMainWin->get_tree_store().get_node_data(old_iter, node_data);
        if (node_data.syntax != CtConst::RICH_TEXT_ID) {
            node_data.rTextBuffer = _pCtMainWin->get_new_text_buffer(node_data.rTextBuffer->get_text());
            node_data.anchoredWidgets.clear();
        } else {
            _pCtMainWin->get_state_machine().update_state(old_iter);
            node_state = _pCtMainWin->get_state_machine().requested_state_current(old_iter.get_node_id());
            node_data.anchoredWidgets.clear();
            node_data.rTextBuffer = _pCtMainWin->get_new_text_buffer();
        }
        node_data.tsCreation = std::time(nullptr);
        node_data.tsLastSave = node_data.tsCreation;
        node_data.nodeId = _pCtMainWin->get_tree_store().node_id_get();
        auto new_iter = _pCtMainWin->get_tree_store().append_node(&node_data, &new_parent /* as parent */);
        _pCtMainWin->get_tree_store().to_ct_tree_iter(new_iter).pending_new_db_node();
        return new_iter;
    };

    // function to duplicate all sub nodes
    std::function<void(Gtk::TreeIter, Gtk::TreeIter)> duplicate_subnodes;
    duplicate_subnodes = [&](Gtk::TreeIter old_parent, Gtk::TreeIter new_parent) {
        for (auto child: old_parent->children()) {
            auto new_child = duplicate_subnode(_pCtMainWin->get_tree_store().to_ct_tree_iter(child), new_parent);
            duplicate_subnodes(child, new_child);
        }
    };
    duplicate_subnodes(top_iter, new_top_iter);

    _pCtMainWin->get_tree_store().nodes_sequences_fix(new_top_iter->parent(), true);
    _pCtMainWin->get_tree_view().set_cursor_safe(new_top_iter);
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::_node_add(bool duplicate, bool add_child)
{
    CtNodeData nodeData;
    std::shared_ptr<CtNodeState> node_state;
    if (duplicate)
    {
        if (!_is_there_selected_node_or_error()) return;
        _pCtMainWin->get_tree_store().get_node_data(_pCtMainWin->curr_tree_iter(), nodeData);

        if (nodeData.syntax != CtConst::RICH_TEXT_ID) {
            nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer(nodeData.rTextBuffer->get_text());
            nodeData.anchoredWidgets.clear();
        } else {
            _pCtMainWin->get_state_machine().update_state(_pCtMainWin->curr_tree_iter());
            node_state = _pCtMainWin->get_state_machine().requested_state_current(_pCtMainWin->curr_tree_iter().get_node_id());
            nodeData.anchoredWidgets.clear();
            nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer();
        }
    }
    else
    {
        if (add_child && !_is_there_selected_node_or_error()) return;
        std::string title = add_child ? _("New Child Node Properties") : _("New Node Properties");
        nodeData.isBold = false;
        nodeData.customIconId = 0;
        nodeData.syntax = CtConst::RICH_TEXT_ID;
        nodeData.isRO = false;
        if (not CtDialogs::node_prop_dialog(title, _pCtMainWin, nodeData, _pCtMainWin->get_tree_store().get_used_tags()))
            return;
    }
    _node_add_with_data(_pCtMainWin->curr_tree_iter(), nodeData, add_child, node_state);
}

void CtActions::_node_add_with_data(Gtk::TreeIter curr_iter, CtNodeData& nodeData, bool add_child, std::shared_ptr<CtNodeState> node_state)
{
    if (!nodeData.rTextBuffer)
        nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer();
    nodeData.tsCreation = std::time(nullptr);
    nodeData.tsLastSave = nodeData.tsCreation;
    nodeData.nodeId = _pCtMainWin->get_tree_store().node_id_get();

    _pCtMainWin->update_window_save_needed();
    _pCtMainWin->get_ct_config()->syntaxHighlighting = nodeData.syntax;

    Gtk::TreeIter nodeIter;
    if (add_child) {
        nodeIter = _pCtMainWin->get_tree_store().append_node(&nodeData, &curr_iter /* as parent */);
    } else if (curr_iter)
        nodeIter = _pCtMainWin->get_tree_store().insert_node(&nodeData, curr_iter /* after */);
    else
        nodeIter = _pCtMainWin->get_tree_store().append_node(&nodeData);

    if (node_state)
        _pCtMainWin->load_buffer_from_state(node_state, _pCtMainWin->get_tree_store().to_ct_tree_iter(nodeIter));
    _pCtMainWin->get_tree_store().to_ct_tree_iter(nodeIter).pending_new_db_node();
    _pCtMainWin->get_tree_store().nodes_sequences_fix(curr_iter ? curr_iter->parent() : Gtk::TreeIter(), false);
    _pCtMainWin->get_tree_store().update_node_aux_icon(nodeIter);
    _pCtMainWin->get_tree_view().set_cursor_safe(nodeIter);
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::_node_child_exist_or_create(Gtk::TreeIter parentIter, const std::string& nodeName)
{
    Gtk::TreeIter childIter = parentIter ? parentIter->children().begin() : _pCtMainWin->get_tree_store().get_iter_first();
    for (; childIter; ++childIter)
        if (_pCtMainWin->get_tree_store().to_ct_tree_iter(childIter).get_node_name() == nodeName) {
            _pCtMainWin->get_tree_view().set_cursor_safe(childIter);
            return;
        }
    CtNodeData nodeData;
    nodeData.name = nodeName;
    nodeData.isBold = false;
    nodeData.customIconId = 0;
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    nodeData.isRO = false;
    _node_add_with_data(parentIter, nodeData, true, nullptr);
}

// Move a node to a parent and after a sibling
void CtActions::_node_move_after(Gtk::TreeIter iter_to_move, Gtk::TreeIter father_iter,
                                 Gtk::TreeIter brother_iter /*= Gtk::TreeIter()*/, bool set_first /*= false*/)
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
    bool swap_excecuted = CtMiscUtil::node_siblings_sort_iteration(_pCtMainWin->get_tree_store().get_store(), children, need_swap);
    for (auto& child: children)
        if (_tree_sort_level_and_sublevels(child.children(), ascending))
            swap_excecuted = true;
    return swap_excecuted;
}

void CtActions::node_edit()
{
    if (!_is_there_selected_node_or_error()) return;
    CtNodeData nodeData;
    _pCtMainWin->get_tree_store().get_node_data(_pCtMainWin->curr_tree_iter(), nodeData);
    CtNodeData newData = nodeData;
    if (not CtDialogs::node_prop_dialog(_("Node Properties"), _pCtMainWin, newData, _pCtMainWin->get_tree_store().get_used_tags()))
        return;

    _pCtMainWin->get_ct_config()->syntaxHighlighting = newData.syntax;

    // leaving rich text
    if (nodeData.syntax != newData.syntax)
        if (nodeData.syntax == CtConst::RICH_TEXT_ID)
            if (!CtDialogs::question_dialog(_("Leaving the Node Type Rich Text you will Lose all Formatting for This Node, Do you want to Continue?"), *_pCtMainWin))
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

    _pCtMainWin->get_text_view().set_editable(!newData.isRO);
    _pCtMainWin->update_selected_node_statusbar_info();
    _pCtMainWin->get_tree_store().update_node_aux_icon(_pCtMainWin->curr_tree_iter());
    _pCtMainWin->window_header_update();
    _pCtMainWin->window_header_update_lock_icon(newData.isRO);
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
            if (!iter.get_node_read_only() && node_syntax != new_syntax)
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
    if (!_is_there_selected_node_or_error()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;

    std::function<void(Gtk::TreeIter, int, std::vector<std::string>&)> collect_children;
    collect_children = [this, &collect_children](Gtk::TreeIter iter, int level, std::vector<std::string>& list) {
      if (list.size() > 15) {
          if (list.size() == 16)
            list.push_back(std::string(CtConst::CHAR_NEWLINE) + "...");
      } else {
          list.push_back(CtConst::CHAR_NEWLINE + str::repeat(CtConst::CHAR_SPACE, level*3) + _pCtMainWin->get_ct_config()->charsListbul[0] +
                  CtConst::CHAR_SPACE + _pCtMainWin->get_tree_store().to_ct_tree_iter(iter).get_node_name());
          for (auto child: iter->children())
              collect_children(child, level + 1, list);
      }
    };


    Glib::ustring warning_label = str::format(_("Are you sure to <b>Delete the node '%s'?</b>"), _pCtMainWin->curr_tree_iter().get_node_name());
    if (!_pCtMainWin->curr_tree_iter()->children().empty())
    {
        std::vector<std::string> lst;
        collect_children(_pCtMainWin->curr_tree_iter(), 0, lst);
        warning_label += str::repeat(CtConst::CHAR_NEWLINE, 2) + _("The node <b>has Children, they will be Deleted too!</b>");
        warning_label += str::join(lst, "");
    }
    if (!CtDialogs::question_dialog(warning_label, *_pCtMainWin))
        return;
    // next selected node will be previous sibling or next sibling or parent or None
    Gtk::TreeIter new_iter = --_pCtMainWin->curr_tree_iter();
    if (!new_iter) new_iter = ++_pCtMainWin->curr_tree_iter();
    if (!new_iter) new_iter = _pCtMainWin->curr_tree_iter().parent();

    _pCtMainWin->resetPrevTreeIter();
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::ndel);
    _pCtMainWin->get_tree_store().get_store()->erase(_pCtMainWin->curr_tree_iter());

    if (new_iter)
    {
        _pCtMainWin->get_tree_view().set_cursor_safe(new_iter);
        _pCtMainWin->get_text_view().grab_focus();
    }
    else
    {
        _curr_buffer()->set_text("");
        _pCtMainWin->window_header_update();
        _pCtMainWin->update_selected_node_statusbar_info();
        _pCtMainWin->get_text_view().set_sensitive(false);
    }
}

void CtActions::node_toggle_read_only()
{
    if (!_is_there_selected_node_or_error()) return;
    bool node_is_ro = !_pCtMainWin->curr_tree_iter().get_node_read_only();
    _pCtMainWin->curr_tree_iter().set_node_read_only(node_is_ro);
    _pCtMainWin->get_text_view().set_editable(!node_is_ro);
    _pCtMainWin->window_header_update_lock_icon(node_is_ro);
    _pCtMainWin->update_selected_node_statusbar_info();
    _pCtMainWin->get_tree_store().update_node_aux_icon(_pCtMainWin->curr_tree_iter());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::npro);
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::node_date()
{
    time_t time = std::time(nullptr);

    std::string year = str::time_format("%Y", time);
    std::string month = str::time_format("%B", time);
    std::string day = str::time_format("%d %a", time);

    _pCtMainWin->get_state_machine().set_go_bk_fw_click(true); // so nodes don't be in the list of visited
    _node_child_exist_or_create(Gtk::TreeIter(), year);
    _node_child_exist_or_create(_pCtMainWin->curr_tree_iter(), month);
    _pCtMainWin->get_state_machine().set_go_bk_fw_click(false);
    _node_child_exist_or_create(_pCtMainWin->curr_tree_iter(), day);
}

void CtActions::node_up()
{
    if (!_is_there_selected_node_or_error()) return;
    auto prev_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(--_pCtMainWin->curr_tree_iter());
    if (!prev_iter) return;
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
    if (!_is_there_selected_node_or_error()) return;
    auto next_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(++_pCtMainWin->curr_tree_iter());
    if (!next_iter) return;
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
    if (!_is_there_selected_node_or_error()) return;
    auto prev_iter = --_pCtMainWin->curr_tree_iter();
    if (!prev_iter) return;
    _node_move_after(_pCtMainWin->curr_tree_iter(), prev_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

void CtActions::node_left()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    if (!father_iter) return;
    _node_move_after(_pCtMainWin->curr_tree_iter(), father_iter->parent(), father_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

void CtActions::node_change_father()
{
    if (!_is_there_selected_node_or_error()) return;
    CtTreeIter old_father_iter = _pCtMainWin->curr_tree_iter().parent();
    CtTreeIter father_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(CtDialogs::choose_node_dialog(_pCtMainWin,
                                   _pCtMainWin->get_tree_view(), _("Select the New Parent"), &_pCtMainWin->get_tree_store(), _pCtMainWin->curr_tree_iter()));
    if (!father_iter) return;
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

    _node_move_after(_pCtMainWin->curr_tree_iter(), father_iter);
    _pCtMainWin->get_tree_store().update_nodes_icon(_pCtMainWin->curr_tree_iter(), true);
}

bool CtActions::node_move(Gtk::TreeModel::Path src_path, Gtk::TreeModel::Path dest_path)
{
    if (src_path == dest_path) {
        CtDialogs::error_dialog(_("The new parent can't be the very node to move!"), *_pCtMainWin);
        return false;
    }
    if (dest_path.is_descendant(src_path)) {
        CtDialogs::error_dialog(_("The new parent can't be one of his children!"), *_pCtMainWin);
        return false;
    }
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
            _node_move_after(src_iter, father_dest_iter, dest_iter, false);
        } else {
            _node_move_after(src_iter, father_dest_iter, CtTreeIter(), true); // put it as first
        }
    } else { // case 2, 3
        if (dest_path.prev()) {
            CtTreeIter dest_iter = _pCtMainWin->get_tree_store().get_iter(dest_path); // put after siblings
            _node_move_after(src_iter, father_dest_iter, dest_iter, false);
        } else {
            _node_move_after(src_iter, father_dest_iter, CtTreeIter(), true); // put it as first child
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
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _pCtMainWin->get_tree_store().get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, true); };
    if (CtMiscUtil::node_siblings_sort_iteration(_pCtMainWin->get_tree_store().get_store(), children, need_swap)) {
        _pCtMainWin->get_tree_store().nodes_sequences_fix(father_iter, true);
        _pCtMainWin->update_window_save_needed();
    }
}

//"""Sorts all the Siblings of the Selected Node Descending"""
void CtActions::node_siblings_sort_descending()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _pCtMainWin->get_tree_store().get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, false); };
    if (CtMiscUtil::node_siblings_sort_iteration(_pCtMainWin->get_tree_store().get_store(), children, need_swap)) {
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
    if (!_is_there_selected_node_or_error()) return;
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
    if (!_is_there_selected_node_or_error()) return;
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
    auto& ctTreestore = _pCtMainWin->get_tree_store();
    _pCtMainWin->get_tree_view().get_model()->foreach(
        [&](const Gtk::TreePath& /*treePath*/, const Gtk::TreeIter& treeIter)->bool
        {
            auto ctTreeIter = ctTreestore.to_ct_tree_iter(treeIter);
            const auto nodeSyntax = ctTreeIter.get_node_syntax_highlighting();
            if (nodeSyntax == CtConst::RICH_TEXT_ID) {
                ++summaryInfo.nodes_rich_text_num;
            }
            else if (nodeSyntax == CtConst::PLAIN_TEXT_ID) {
                ++summaryInfo.nodes_plain_text_num;
            }
            else {
                ++summaryInfo.nodes_code_num;
            }
            (void)ctTreeIter.get_node_text_buffer(); // ensure the node content is populated
            for (CtAnchoredWidget* pAnchoredWidget : ctTreeIter.get_embedded_pixbufs_tables_codeboxes_fast()) {
                switch (pAnchoredWidget->get_type()) {
                    case CtAnchWidgType::CodeBox: ++summaryInfo.codeboxes_num; break;
                    case CtAnchWidgType::ImageAnchor: ++summaryInfo.anchors_num; break;
                    case CtAnchWidgType::ImageEmbFile: ++summaryInfo.embfile_num; break;
                    case CtAnchWidgType::ImagePng: ++summaryInfo.images_num; break;
                    case CtAnchWidgType::Table: ++summaryInfo.tables_num; break;
                }
            }
            return false; /* false for continue */
        }
    );
    CtDialogs::summary_info_dialog(_pCtMainWin, summaryInfo);
}

void CtActions::node_link_to_clipboard()
{
    if (!_is_there_selected_node_or_error()) return;
    CtClipboard(_pCtMainWin).node_link_to_clipboard(_pCtMainWin->curr_tree_iter());
}
