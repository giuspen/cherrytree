/*
 * ct_actions_tree.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include "ct_image.h"
#include "ct_app.h"
#include "ct_dialogs.h"
#include "ct_doc_rw.h"
#include <ctime>

bool dialog_node_prop(std::string title, Gtk::Window& parent, CtNodeData& nodeData, const std::set<std::string>& tags_set);

bool CtActions::_is_there_selected_node_or_error()
{
    if (_pCtMainWin->curr_tree_iter()) return true;
    ct_dialogs::warning_dialog(_("No Node is Selected"), *_pCtMainWin);
    return false;
}

bool CtActions::_is_tree_not_empty_or_error()
{
    if (!_pCtTreestore->get_iter_first()) {
        ct_dialogs::error_dialog(_("The Tree is Empty!"), *_pCtMainWin);
        return false;
    }
    return true;
}

bool CtActions::_is_curr_node_not_read_only_or_error()
{
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) {
        ct_dialogs::error_dialog(_("The Selected Node is Read Only"), *_pCtMainWin);
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
        ct_dialogs::warning_dialog(_("This Feature is Available Only in Rich Text Nodes"), *_pCtMainWin);
    else
        ct_dialogs::warning_dialog(_("This Feature is Not Available in Automatic Syntax Highlighting Nodes"), *_pCtMainWin);
    return false;
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

void CtActions::_node_add(bool duplicate, bool add_child)
{
    CtNodeData nodeData;
    if (duplicate)
     {
        if (!_is_there_selected_node_or_error()) return;
        _pCtTreestore->getNodeData(_pCtMainWin->curr_tree_iter(), nodeData);

        if (nodeData.syntax != CtConst::RICH_TEXT_ID) {
            nodeData.rTextBuffer = CtMiscUtil::get_new_text_buffer(nodeData.syntax, nodeData.rTextBuffer->get_text());
            nodeData.anchoredWidgets.clear();
        } else {
            // todo:
            //state = self.state_machine.requested_state_previous(self.treestore[tree_iter_from][3])
            //self.load_buffer_from_state(state, given_tree_iter=new_node_iter)

            // todo: temporary solution
            nodeData.anchoredWidgets.clear();
            nodeData.rTextBuffer = CtMiscUtil::get_new_text_buffer(nodeData.syntax, nodeData.rTextBuffer->get_text());
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
        if (!dialog_node_prop(title, *_pCtMainWin, nodeData, _pCtTreestore->get_used_tags()))
            return;
    }
    _node_add_with_data(_pCtMainWin->curr_tree_iter(), nodeData, add_child);
}

void CtActions::_node_add_with_data(Gtk::TreeIter curr_iter, CtNodeData& nodeData, bool add_child)
{
    if (!nodeData.rTextBuffer)
        nodeData.rTextBuffer = CtMiscUtil::get_new_text_buffer(nodeData.syntax);
    nodeData.tsCreation = std::time(nullptr);
    nodeData.tsLastSave = nodeData.tsCreation;
    nodeData.nodeId = _pCtTreestore->node_id_get();

    _pCtMainWin->update_window_save_needed();
    CtApp::P_ctCfg->syntaxHighlighting = nodeData.syntax;

    Gtk::TreeIter nodeIter;
    if (add_child) {
        nodeIter = _pCtTreestore->appendNode(&nodeData, &curr_iter /* as parent */);
    } else if (curr_iter)
        nodeIter = _pCtTreestore->insertNode(&nodeData, curr_iter /* after */);
    else
        nodeIter = _pCtTreestore->appendNode(&nodeData);

    _pCtTreestore->to_ct_tree_iter(nodeIter).pending_new_db_node();
    _pCtTreestore->nodes_sequences_fix(curr_iter ? curr_iter->parent() : Gtk::TreeIter(), false);
    _pCtTreestore->updateNodeAuxIcon(nodeIter);
    _pCtMainWin->get_tree_view().set_cursor_safe(nodeIter);
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::_node_child_exist_or_create(Gtk::TreeIter parentIter, const std::string& nodeName)
{
    Gtk::TreeIter childIter = parentIter ? parentIter->children().begin() : _pCtTreestore->get_iter_first();
    for (; childIter; ++childIter)
        if (_pCtTreestore->to_ct_tree_iter(childIter).get_node_name() == nodeName) {
            _pCtMainWin->get_tree_view().set_cursor_safe(childIter);
            return;
        }
    CtNodeData nodeData;
    nodeData.name = nodeName;
    nodeData.isBold = false;
    nodeData.customIconId = 0;
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    nodeData.isRO = false;
    _node_add_with_data(parentIter, nodeData, true);
}

// Move a node to a parent and after a sibling
void CtActions::_node_move_after(Gtk::TreeIter iter_to_move, Gtk::TreeIter father_iter,
                                 Gtk::TreeIter brother_iter /*= Gtk::TreeIter()*/, bool set_first /*= false*/)
{
    Gtk::TreeIter new_node_iter;
    if (brother_iter)   new_node_iter = _pCtTreestore->get_store()->insert_after(brother_iter);
    else if (set_first) new_node_iter = _pCtTreestore->get_store()->prepend(father_iter->children());
    else                new_node_iter = _pCtTreestore->get_store()->append(father_iter->children());

    // we move also all the children
    std::function<void(Gtk::TreeIter&,Gtk::TreeIter&)> node_move_data_and_children;
    node_move_data_and_children = [this, &node_move_data_and_children](Gtk::TreeIter& old_iter,Gtk::TreeIter& new_iter) {
        CtNodeData node_data;
        _pCtTreestore->getNodeData(old_iter, node_data);
        _pCtTreestore->updateNodeData(new_iter, node_data);
        for (Gtk::TreeIter child: old_iter->children()) {
            Gtk::TreeIter new_child = _pCtTreestore->get_store()->append(new_iter->children());
            node_move_data_and_children(child, new_child);
        }
    };
    node_move_data_and_children(iter_to_move, new_node_iter);

    // now we can remove the old iter (and all children)
    _pCtTreestore->get_store()->erase(iter_to_move);
    //todo: self.ctdb_handler.pending_edit_db_node_hier(self.treestore[new_node_iter][3])
    _pCtTreestore->nodes_sequences_fix(Gtk::TreeIter(), true);
    if (father_iter)
        _pCtMainWin->get_tree_view().expand_row(_pCtTreestore->get_path(father_iter), false);
    else
        _pCtMainWin->get_tree_view().expand_row(_pCtTreestore->get_path(new_node_iter), false);
    Gtk::TreePath new_node_path = _pCtTreestore->get_path(new_node_iter);
    _pCtMainWin->get_tree_view().collapse_row(new_node_path);
    _pCtMainWin->get_tree_view().set_cursor(new_node_path);
    _pCtMainWin->update_window_save_needed();
}

bool CtActions::_need_node_swap(Gtk::TreeIter& leftIter, Gtk::TreeIter& rightIter, bool ascending)
{
    int cmp = _pCtTreestore->to_ct_tree_iter(leftIter).get_node_name().compare(_pCtTreestore->to_ct_tree_iter(rightIter).get_node_name());
    return ascending ? cmp > 0 : cmp < 0;
}

bool CtActions::_tree_sort_level_and_sublevels(const Gtk::TreeNodeChildren& children, bool ascending)
{
    auto need_swap = [this,&ascending](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, ascending); };
    bool swap_excecuted = CtMiscUtil::node_siblings_sort_iteration(_pCtTreestore->get_store(), children, need_swap);
    for (auto& child: children)
        if (_tree_sort_level_and_sublevels(child.children(), ascending))
            swap_excecuted = true;
    return swap_excecuted;
}

void CtActions::node_edit()
{
    if (!_is_there_selected_node_or_error()) return;
    CtNodeData nodeData;
    _pCtTreestore->getNodeData(_pCtMainWin->curr_tree_iter(), nodeData);
    CtNodeData newData = nodeData;
    if (!dialog_node_prop(_("Node Properties"), *_pCtMainWin, newData, _pCtTreestore->get_used_tags()))
        return;

    CtApp::P_ctCfg->syntaxHighlighting = newData.syntax;
    if (nodeData.syntax !=  newData.syntax) {
        if (nodeData.syntax == CtConst::RICH_TEXT_ID) {
            // leaving rich text
            if (!ct_dialogs::question_dialog(_("Leaving the Node Type Rich Text you will Lose all Formatting for This Node, Do you want to Continue?"), *_pCtMainWin)) {
                return;
            }
            // todo:
            // SWITCH TextBuffer -> SourceBuffer
            //self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
            //self.curr_buffer = self.treestore[self.curr_tree_iter][2]
            //self.state_machine.delete_states(self.get_node_id_from_tree_iter(self.curr_tree_iter))
        } else if (newData.syntax == CtConst::RICH_TEXT_ID) {
            // going to rich text
            // SWITCH SourceBuffer -> TextBuffer
            //self.switch_buffer_text_source(self.curr_buffer, self.curr_tree_iter, self.syntax_highlighting, self.treestore[self.curr_tree_iter][4])
            //self.curr_buffer = self.treestore[self.curr_tree_iter][2]
        } else if (nodeData.syntax == CtConst::PLAIN_TEXT_ID) {
            // plain text to code
            //self.sourceview.modify_font(pango.FontDescription(self.code_font))
        } else if (newData.syntax == CtConst::PLAIN_TEXT_ID) {
            // code to plain text
            // self.sourceview.modify_font(pango.FontDescription(self.pt_font))
        }
    }
    _pCtTreestore->updateNodeData(_pCtMainWin->curr_tree_iter(), newData);
    //todo: if self.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
    //  self.set_sourcebuffer_syntax_highlight(self.curr_buffer, self.syntax_highlighting)
    _pCtMainWin->get_text_view().set_editable(!newData.isRO);
    //todo: self.update_selected_node_statusbar_info()
    _pCtTreestore->updateNodeAuxIcon(_pCtMainWin->curr_tree_iter());
    _pCtMainWin->window_header_update();
    _pCtMainWin->window_header_update_lock_icon(newData.isRO);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::npro);
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::node_toggle_read_only()
{
    if (!_is_there_selected_node_or_error()) return;
    bool node_is_ro = !_pCtMainWin->curr_tree_iter().get_node_read_only();
    _pCtMainWin->curr_tree_iter().set_node_read_only(node_is_ro);
    _pCtMainWin->get_text_view().set_editable(!node_is_ro);
    _pCtMainWin->window_header_update_lock_icon(node_is_ro);
    //todo: self.update_selected_node_statusbar_info()
    _pCtTreestore->updateNodeAuxIcon(_pCtMainWin->curr_tree_iter());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::npro);
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::node_date()
{
    time_t time = std::time(nullptr);

    std::string year = str::time_format("%Y", time);
    std::string month = str::time_format("%B", time);
    std::string day = str::time_format("%d %a", time);

    _node_child_exist_or_create(Gtk::TreeIter(), year);
    _node_child_exist_or_create(_pCtMainWin->curr_tree_iter(), month);
    _node_child_exist_or_create(_pCtMainWin->curr_tree_iter(), day);
}

void CtActions::node_up()
{
    if (!_is_there_selected_node_or_error()) return;
    auto prev_iter = --_pCtMainWin->curr_tree_iter();
    if (!prev_iter) return;
    _pCtTreestore->get_store()->iter_swap(_pCtMainWin->curr_tree_iter(), prev_iter);
    //todo: self.nodes_sequences_swap(self.curr_tree_iter, prev_iter)
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[self.curr_tree_iter][3])
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[prev_iter][3])
    _pCtMainWin->get_tree_view().set_cursor(_pCtTreestore->get_path(_pCtMainWin->curr_tree_iter()));
    _pCtMainWin->update_window_save_needed();
}

void CtActions::node_down()
{
    if (!_is_there_selected_node_or_error()) return;
    auto next_iter = ++_pCtMainWin->curr_tree_iter();
    if (!next_iter) return;
    _pCtTreestore->get_store()->iter_swap(_pCtMainWin->curr_tree_iter(), next_iter);
    //todo: self.nodes_sequences_swap(self.curr_tree_iter, subseq_iter)
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[self.curr_tree_iter][3])
    //self.ctdb_handler.pending_edit_db_node_hier(self.treestore[subseq_iter][3])
    _pCtMainWin->get_tree_view().set_cursor(_pCtTreestore->get_path(_pCtMainWin->curr_tree_iter()));
    _pCtMainWin->update_window_save_needed();
}

void CtActions::node_right()
{
    if (!_is_there_selected_node_or_error()) return;
    auto prev_iter = --_pCtMainWin->curr_tree_iter();
    if (!prev_iter) return;
    _node_move_after(_pCtMainWin->curr_tree_iter(), prev_iter);
    //todo: if (CtApp::P_ctCfg->nodesIcons == "c") self.treeview_refresh(change_icon=True)
}

void CtActions::node_left()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    if (!father_iter) return;
    _node_move_after(_pCtMainWin->curr_tree_iter(), father_iter->parent(), father_iter);
    //todo: if (CtApp::P_ctCfg->nodesIcons == "c") self.treeview_refresh(change_icon=True)
}

void CtActions::node_change_father()
{
    if (!_is_there_selected_node_or_error()) return;
    CtTreeIter old_father_iter = _pCtMainWin->curr_tree_iter().parent();
    CtTreeIter father_iter = _pCtTreestore->to_ct_tree_iter(ct_dialogs::choose_node_dialog(*_pCtMainWin,
                                   _pCtMainWin->get_tree_view(), _("Select the New Parent"), _pCtTreestore, _pCtMainWin->curr_tree_iter()));
    if (!father_iter) return;
    gint64 curr_node_id = _pCtMainWin->curr_tree_iter().get_node_id();
    gint64 old_father_node_id = old_father_iter.get_node_id();
    gint64 new_father_node_id = father_iter.get_node_id();
    if (curr_node_id == new_father_node_id) {
        ct_dialogs::error_dialog(_("The new parent can't be the very node to move!"), *_pCtMainWin);
        return;
    }
    if (old_father_node_id != -1 && new_father_node_id == old_father_node_id) {
        ct_dialogs::info_dialog(_("The new chosen parent is still the old parent!"), *_pCtMainWin);
        return;
    }
    for (CtTreeIter move_towards_top_iter = father_iter.parent(); move_towards_top_iter; move_towards_top_iter = move_towards_top_iter.parent())
        if (move_towards_top_iter.get_node_id() == curr_node_id) {
            ct_dialogs::error_dialog(_("The new parent can't be one of his children!"), *_pCtMainWin);
            return;
        }

    _node_move_after(_pCtMainWin->curr_tree_iter(), father_iter);
    //todo: if self.nodes_icons == "c": self.treeview_refresh(change_icon=True)
}

//"""Sorts the Tree Ascending"""
void CtActions::tree_sort_ascending()
{
    if (_tree_sort_level_and_sublevels(_pCtTreestore->get_store()->children(), true)) {
        _pCtTreestore->nodes_sequences_fix(Gtk::TreeIter(), true);
        _pCtMainWin->update_window_save_needed();
    }
}

//"""Sorts the Tree Ascending"""
void CtActions::tree_sort_descending()
{
    if (_tree_sort_level_and_sublevels(_pCtTreestore->get_store()->children(), false)) {
        _pCtTreestore->nodes_sequences_fix(Gtk::TreeIter(), true);
        _pCtMainWin->update_window_save_needed();
    }
}

//"""Sorts all the Siblings of the Selected Node Ascending"""
void CtActions::node_siblings_sort_ascending()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _pCtTreestore->get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, true); };
    if (CtMiscUtil::node_siblings_sort_iteration(_pCtTreestore->get_store(), children, need_swap)) {
        _pCtTreestore->nodes_sequences_fix(father_iter, true);
        _pCtMainWin->update_window_save_needed();
    }
}

//"""Sorts all the Siblings of the Selected Node Descending"""
void CtActions::node_siblings_sort_descending()
{
    if (!_is_there_selected_node_or_error()) return;
    Gtk::TreeIter father_iter = _pCtMainWin->curr_tree_iter()->parent();
    const Gtk::TreeNodeChildren& children = father_iter ? father_iter->children() : _pCtTreestore->get_store()->children();
    auto need_swap = [this](Gtk::TreeIter& l, Gtk::TreeIter& r) { return _need_node_swap(l, r, false); };
    if (CtMiscUtil::node_siblings_sort_iteration(_pCtTreestore->get_store(), children, need_swap)) {
        _pCtTreestore->nodes_sequences_fix(father_iter, true);
        _pCtMainWin->update_window_save_needed();
    }
}

void CtActions::bookmark_curr_node()
{
    if (!_is_there_selected_node_or_error()) return;
    gint64 node_id = _pCtMainWin->curr_tree_iter().get_node_id();

    if (_pCtTreestore->onRequestAddBookmark(node_id)) {
        _pCtMainWin->set_bookmarks_menu_items();
        _pCtTreestore->updateNodeAuxIcon(_pCtMainWin->curr_tree_iter());
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::book);
        _pCtMainWin->menu_tree_update_for_bookmarked_node(true);
    }
}

void CtActions::bookmark_curr_node_remove()
{
    if (!_is_there_selected_node_or_error()) return;
    gint64 node_id = _pCtMainWin->curr_tree_iter().get_node_id();

    if (_pCtTreestore->onRequestRemoveBookmark(node_id)) {
        _pCtMainWin->set_bookmarks_menu_items();
        _pCtTreestore->updateNodeAuxIcon(_pCtMainWin->curr_tree_iter());
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::book);
        _pCtMainWin->menu_tree_update_for_bookmarked_node(false);
    }

}

void CtActions::bookmarks_handle()
{
    ct_dialogs::bookmarks_handle_dialog(_pCtMainWin);
}

bool dialog_node_prop(std::string title, Gtk::Window& parent, CtNodeData& nodeData, const std::set<std::string>& tags_set)
{
    auto dialog = Gtk::Dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_default_size(300, -1);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    auto name_entry = Gtk::Entry();
    name_entry.set_text(nodeData.name);
    auto is_bold_checkbutton = Gtk::CheckButton(_("Bold"));
    is_bold_checkbutton.set_active(nodeData.isBold);
    auto fg_checkbutton = Gtk::CheckButton(_("Use Selected Color"));
    fg_checkbutton.set_active(nodeData.foregroundRgb24 != "");
    Glib::ustring real_fg = nodeData.foregroundRgb24 != "" ? nodeData.foregroundRgb24 : (CtApp::P_ctCfg->currColors.at('n') != "" ? CtApp::P_ctCfg->currColors.at('n').c_str() : "red");
    auto fg_colorbutton = Gtk::ColorButton(Gdk::RGBA(real_fg));
    fg_colorbutton.set_sensitive(nodeData.foregroundRgb24 != "");
    auto fg_hbox = Gtk::HBox();
    fg_hbox.set_spacing(2);
    fg_hbox.pack_start(fg_checkbutton, false, false);
    fg_hbox.pack_start(fg_colorbutton, false, false);
    auto c_icon_checkbutton = Gtk::CheckButton(_("Use Selected Icon"));
    c_icon_checkbutton.set_active(map::exists(CtConst::NODES_STOCKS, nodeData.customIconId));
    auto c_icon_button = Gtk::Button();
    if (c_icon_checkbutton.get_active())
        c_icon_button.set_image(*CtImage::new_image_from_stock(CtConst::NODES_STOCKS.at((int)nodeData.customIconId), Gtk::ICON_SIZE_BUTTON));
    else {
        c_icon_button.set_label(_("click me"));
        c_icon_button.set_sensitive(false);
    }
    auto c_icon_hbox = Gtk::HBox();
    c_icon_hbox.set_spacing(2);
    c_icon_hbox.pack_start(c_icon_checkbutton, false, false);
    c_icon_hbox.pack_start(c_icon_button, false, false);
    auto name_vbox = Gtk::VBox();
    name_vbox.pack_start(name_entry);
    name_vbox.pack_start(is_bold_checkbutton);
    name_vbox.pack_start(fg_hbox);
    name_vbox.pack_start(c_icon_hbox);
    auto name_frame = Gtk::Frame(std::string("<b>")+_("Node Name")+"</b>");
    ((Gtk::Label*)name_frame.get_label_widget())->set_use_markup(true);
    name_frame.set_shadow_type(Gtk::SHADOW_NONE);
    name_frame.add(name_vbox);
    auto radiobutton_rich_text = Gtk::RadioButton(_("Rich Text"));
    auto radiobutton_plain_text = Gtk::RadioButton(_("Plain Text"));
    radiobutton_plain_text.join_group(radiobutton_rich_text);
    auto radiobutton_auto_syntax_highl = Gtk::RadioButton(_("Automatic Syntax Highlighting"));
    radiobutton_auto_syntax_highl.join_group(radiobutton_rich_text);
    auto button_prog_lang = Gtk::Button();
    std::string syntax_hl_id = nodeData.syntax;
    if (nodeData.syntax == CtConst::RICH_TEXT_ID || nodeData.syntax == CtConst::PLAIN_TEXT_ID)
        syntax_hl_id = CtApp::P_ctCfg->autoSynHighl;
    std::string button_stock_id = CtConst::getStockIdForCodeType(syntax_hl_id);
    button_prog_lang.set_label(syntax_hl_id);
    button_prog_lang.set_image(*CtImage::new_image_from_stock(button_stock_id, Gtk::ICON_SIZE_MENU));
    if (nodeData.syntax == CtConst::RICH_TEXT_ID) {
        radiobutton_rich_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    } else if (nodeData.syntax == CtConst::PLAIN_TEXT_ID) {
        radiobutton_plain_text.set_active(true);
        button_prog_lang.set_sensitive(false);
    } else {
        radiobutton_auto_syntax_highl.set_active(true);
    }
    auto type_vbox = Gtk::VBox();
    type_vbox.pack_start(radiobutton_rich_text);
    type_vbox.pack_start(radiobutton_plain_text);
    type_vbox.pack_start(radiobutton_auto_syntax_highl);
    type_vbox.pack_start(button_prog_lang);
    auto type_frame = Gtk::Frame(std::string("<b>")+_("Node Type")+"</b>");
    ((Gtk::Label*)type_frame.get_label_widget())->set_use_markup(true);
    type_frame.set_shadow_type(Gtk::SHADOW_NONE);
    type_frame.add(type_vbox);
    type_frame.set_sensitive(!nodeData.isRO);
    auto tags_hbox = Gtk::HBox();
    tags_hbox.set_spacing(2);
    auto tags_entry = Gtk::Entry();
    tags_entry.set_text(nodeData.tags);
    auto button_browse_tags = Gtk::Button();
    button_browse_tags.set_image(*CtImage::new_image_from_stock("find", Gtk::ICON_SIZE_BUTTON));
    button_browse_tags.set_sensitive(!tags_set.empty());
    tags_hbox.pack_start(tags_entry);
    tags_hbox.pack_start(button_browse_tags, false, false);
    auto tags_frame = Gtk::Frame(std::string("<b>")+_("Tags for Searching")+"</b>");
    ((Gtk::Label*)tags_frame.get_label_widget())->set_use_markup(true);
    tags_frame.set_shadow_type(Gtk::SHADOW_NONE);
    tags_frame.add(tags_hbox);
    auto ro_checkbutton = Gtk::CheckButton(_("Read Only"));
    ro_checkbutton.set_active(nodeData.isRO);
    auto content_area = dialog.get_content_area();
    content_area->set_spacing(5);
    content_area->pack_start(name_frame);
    content_area->pack_start(type_frame);
    content_area->pack_start(tags_frame);
    content_area->pack_start(ro_checkbutton);
    content_area->show_all();
    name_entry.grab_focus();

    button_prog_lang.signal_clicked().connect([&parent, &button_prog_lang](){
        auto itemStore = ct_dialogs::CtChooseDialogListStore::create();
        for (auto lang: CtApp::R_languageManager->get_language_ids())
            itemStore->add_row(CtConst::getStockIdForCodeType(lang), "", lang);
        auto res = ct_dialogs::choose_item_dialog(parent, _("Automatic Syntax Highlighting"), itemStore);
        if (res) {
            std::string stock_id = res->get_value(itemStore->columns.desc);
            button_prog_lang.set_label(stock_id);
            button_prog_lang.set_image(*CtImage::new_image_from_stock(stock_id, Gtk::ICON_SIZE_MENU));
        }
    });
    radiobutton_auto_syntax_highl.signal_toggled().connect([&radiobutton_auto_syntax_highl, &button_prog_lang](){
       button_prog_lang.set_sensitive(radiobutton_auto_syntax_highl.get_active());
    });
    button_browse_tags.signal_clicked().connect([&parent, &tags_entry, &tags_set](){
        auto itemStore = ct_dialogs::CtChooseDialogListStore::create();
        for (const auto& tag: tags_set)
            itemStore->add_row("", "", tag);
        auto res = ct_dialogs::choose_item_dialog(parent, _("Choose Existing Tag"), itemStore, _("Tag Name"));
        if (res) {
            std::string cur_tag = tags_entry.get_text();
            if  (str::endswith(cur_tag, CtConst::CHAR_SPACE))
                tags_entry.set_text(cur_tag + res->get_value(itemStore->columns.desc));
            else
                tags_entry.set_text(cur_tag + CtConst::CHAR_SPACE + res->get_value(itemStore->columns.desc));
        }
    });
    ro_checkbutton.signal_toggled().connect([&ro_checkbutton, &type_frame](){
        type_frame.set_sensitive(ro_checkbutton.get_active());
    });
    fg_checkbutton.signal_toggled().connect([&fg_checkbutton, &fg_colorbutton](){
        fg_colorbutton.set_sensitive(fg_checkbutton.get_active());
    });
    fg_colorbutton.signal_pressed().connect([&parent, &fg_colorbutton](){
        Gdk::RGBA ret_color = fg_colorbutton.get_rgba();
        if (ct_dialogs::color_pick_dialog(parent, ret_color))
            fg_colorbutton.set_rgba(ret_color);
    });
    c_icon_checkbutton.signal_toggled().connect([&c_icon_checkbutton, &c_icon_button](){
        c_icon_button.set_sensitive(c_icon_checkbutton.get_active());
    });
    c_icon_button.signal_clicked().connect([&parent, &c_icon_button, &nodeData](){
        auto itemStore = ct_dialogs::CtChooseDialogListStore::create();
        for (auto& pair: CtConst::NODES_ICONS)
            itemStore->add_row(pair.second, std::to_string(pair.first), "");
        auto res = ct_dialogs::choose_item_dialog(parent, _("Select Node Icon"), itemStore);
        if (res) {
            nodeData.customIconId = static_cast<guint32>(std::stoi(res->get_value(itemStore->columns.key)));
            c_icon_button.set_label("");
            c_icon_button.set_image(*CtImage::new_image_from_stock(res->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
        }
    });

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return false;

    nodeData.name = name_entry.get_text();
    if (nodeData.name.empty())
        nodeData.name = CtConst::CHAR_QUESTION;
    if (radiobutton_rich_text.get_active())
        nodeData.syntax = CtConst::RICH_TEXT_ID;
    else if (radiobutton_plain_text.get_active())
        nodeData.syntax = CtConst::PLAIN_TEXT_ID;
    else {
        nodeData.syntax = button_prog_lang.get_label();
        CtApp::P_ctCfg->autoSynHighl = nodeData.syntax;
    }
    nodeData.tags = tags_entry.get_text();
    nodeData.isRO = ro_checkbutton.get_active();
    nodeData.customIconId = c_icon_checkbutton.get_active() ? nodeData.customIconId : 0;
    nodeData.isBold = is_bold_checkbutton.get_active();
    if (fg_checkbutton.get_active()) {
        nodeData.foregroundRgb24 = CtRgbUtil::getRgb24StrFromStrAny(fg_colorbutton.get_color().to_string());
        CtApp::P_ctCfg->currColors['n'] = nodeData.foregroundRgb24;
    }
    return true;
}
