/*
 * ct_state_machine.cc
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_state_machine.h"
#include "ct_main_win.h"
#include "ct_storage_xml.h"

// ImagePng
CtAnchoredWidgetState_ImagePng::CtAnchoredWidgetState_ImagePng(CtImagePng* image)
    :CtAnchoredWidgetState(image->getOffset(), image->getJustification()),
      link(image->get_link()), pixbuf(image->get_pixbuf()->copy())
{

}

bool CtAnchoredWidgetState_ImagePng::equal(std::shared_ptr<CtAnchoredWidgetState> state)
{
    CtAnchoredWidgetState_ImagePng* other_state = dynamic_cast<CtAnchoredWidgetState_ImagePng*>(state.get());
    return other_state && charOffset == other_state->charOffset && justification == other_state->justification &&
            link == other_state->link &&
            pixbuf->get_byte_length() == other_state->pixbuf->get_byte_length() &&
            memcmp(pixbuf->get_pixels(), other_state->pixbuf->get_pixels(), pixbuf->get_byte_length() * sizeof(guint8));
}

CtAnchoredWidget* CtAnchoredWidgetState_ImagePng::to_widget(CtMainWin* pCtMainWin)
{
    return new CtImagePng(pCtMainWin, pixbuf->copy(), link, charOffset, justification);
}

// ImageAnchor
CtAnchoredWidgetState_Anchor::CtAnchoredWidgetState_Anchor(CtImageAnchor* anchor)
    :CtAnchoredWidgetState(anchor->getOffset(), anchor->getJustification()),
      name(anchor->get_anchor_name())
{

}

bool CtAnchoredWidgetState_Anchor::equal(std::shared_ptr<CtAnchoredWidgetState> state)
{
    CtAnchoredWidgetState_Anchor* other_state = dynamic_cast<CtAnchoredWidgetState_Anchor*>(state.get());
    return other_state && charOffset == other_state->charOffset && justification == other_state->justification &&
            name == other_state->name;
}

CtAnchoredWidget* CtAnchoredWidgetState_Anchor::to_widget(CtMainWin* pCtMainWin)
{
    return new CtImageAnchor(pCtMainWin, name, charOffset, justification);
}

// ImageEmbFile
CtAnchoredWidgetState_EmbFile::CtAnchoredWidgetState_EmbFile(CtImageEmbFile* embFile)
    :CtAnchoredWidgetState(embFile->getOffset(), embFile->getJustification()),
      fileName(embFile->get_file_name()), rawBlob(embFile->get_raw_blob()), timeSeconds(embFile->get_time())
{

}

bool CtAnchoredWidgetState_EmbFile::equal(std::shared_ptr<CtAnchoredWidgetState> state)
{
    CtAnchoredWidgetState_EmbFile* other_state = dynamic_cast<CtAnchoredWidgetState_EmbFile*>(state.get());
    return other_state && charOffset == other_state->charOffset && justification == other_state->justification &&
            fileName == other_state->fileName && rawBlob == other_state->rawBlob && timeSeconds == other_state->timeSeconds;
}

CtAnchoredWidget* CtAnchoredWidgetState_EmbFile::to_widget(CtMainWin* pCtMainWin)
{
    return new CtImageEmbFile(pCtMainWin, fileName, rawBlob, timeSeconds, charOffset, justification);
}

// Codebox
CtAnchoredWidgetState_Codebox::CtAnchoredWidgetState_Codebox(CtCodebox* codebox)
    :CtAnchoredWidgetState(codebox->getOffset(), codebox->getJustification()),
      content(codebox->get_text_content()), syntax(codebox->get_syntax_highlighting()), width(codebox->get_frame_width()), height(codebox->get_frame_height()),
      widthInPixels(codebox->get_width_in_pixels()), brackets(codebox->get_highlight_brackets()), showNum(codebox->get_show_line_numbers())
{
}

bool CtAnchoredWidgetState_Codebox::equal(std::shared_ptr<CtAnchoredWidgetState> state)
{
    CtAnchoredWidgetState_Codebox* other_state = dynamic_cast<CtAnchoredWidgetState_Codebox*>(state.get());
    return other_state && charOffset == other_state->charOffset && justification == other_state->justification &&
            syntax == other_state->syntax && width == other_state->width && height == other_state->height &&
            widthInPixels == other_state->widthInPixels && brackets == other_state->brackets && showNum == other_state->showNum &&
            content == other_state->content;
}

CtAnchoredWidget* CtAnchoredWidgetState_Codebox::to_widget(CtMainWin* pCtMainWin)
{
    return new CtCodebox(pCtMainWin, content, syntax, width, height,
                         charOffset, justification, widthInPixels, brackets, showNum);
}

// Table
CtAnchoredWidgetState_Table::CtAnchoredWidgetState_Table(CtTable* table)
    :CtAnchoredWidgetState(table->getOffset(), table->getJustification()), colMin(table->get_col_min()), colMax(table->get_col_max())
{
    for (auto& row: table->get_table_matrix())
    {
        rows.push_back(std::vector<Glib::ustring>());
        for (auto& cell: row)  rows.back().push_back(cell->get_text_content());
    }
}

bool CtAnchoredWidgetState_Table::equal(std::shared_ptr<CtAnchoredWidgetState> state)
{
    CtAnchoredWidgetState_Table* other_state = dynamic_cast<CtAnchoredWidgetState_Table*>(state.get());
    return other_state && charOffset == other_state->charOffset && justification == other_state->justification &&
            colMin == other_state->colMin && colMax == other_state->colMax && rows == other_state->rows;
}

CtAnchoredWidget* CtAnchoredWidgetState_Table::to_widget(CtMainWin* pCtMainWin)
{
    CtTableMatrix tableMatrix;
    for (auto& row: rows)
    {
        tableMatrix.push_back(CtTableRow());
        for (auto& cell: row) tableMatrix.back().push_back(new CtTableCell(pCtMainWin, cell, CtConst::TABLE_CELL_TEXT_ID));
    }
    return new CtTable(pCtMainWin, tableMatrix, colMin, colMax, charOffset, justification);
}



//
CtStateMachine::CtStateMachine(CtMainWin *pCtMainWin) : _pCtMainWin(pCtMainWin)
{
    _word_regex = Glib::Regex::create("\\w");
    _go_bk_fw_click = false;
    _not_undoable_timeslot = false;
    _visited_nodes_idx = -1;
}

// State Machine Reset
void CtStateMachine::reset()
{
    _visited_nodes_list.clear();
    _visited_nodes_idx = -1;
    _node_states.clear();
}

// Requested the Previous Visited Node
gint64 CtStateMachine::requested_visited_previous()
{
    if (_visited_nodes_idx <= 0)
        return -1;
     _visited_nodes_idx -= 1;
     return _visited_nodes_list[_visited_nodes_idx];
}

// Requested the Next Visited Node
gint64 CtStateMachine::requested_visited_next()
{
    if (_visited_nodes_idx != -1 && _visited_nodes_idx < (int)_visited_nodes_list.size() - 1)
    {
        _visited_nodes_idx += 1;
       return _visited_nodes_list[_visited_nodes_idx];
    }
    return -1;
}

// When a New Node is Selected
void CtStateMachine::node_selected_changed(gint64 node_id)
{
    if (node_id == -1)
        return;
    if (_pCtMainWin->user_active() && !_go_bk_fw_click)
    {
        int last_index = (int)_visited_nodes_list.size() - 1;
        if (_visited_nodes_idx != last_index)
            _visited_nodes_list.erase(_visited_nodes_list.begin() + (_visited_nodes_idx + 1), _visited_nodes_list.begin() + (last_index+1));
        vec::remove(_visited_nodes_list, node_id);
        _visited_nodes_list.push_back(node_id);
        _visited_nodes_idx = _visited_nodes_list.size() - 1;
    }
    if (!map::exists(_node_states, node_id))
    {
        auto node = _pCtMainWin->curr_tree_iter();
        auto state = std::shared_ptr<CtNodeState>(new CtNodeState());
        CtStorageXmlHelper(_pCtMainWin).save_buffer_no_widgets_to_xml(state->buffer_xml.get_root_node(), node.get_node_text_buffer(), 0, -1, 'n');
        state->buffer_xml_string = state->buffer_xml.write_to_string();
        for (auto widget: node.get_embedded_pixbufs_tables_codeboxes())
            state->widgetStates.push_back(widget->get_state());
        state->cursor_pos = 0;

        CtNodeStates states;
        states.states.push_back(state);
        states.index = 0;     // first state
        states.indicator = 0; // the current buffer state is saved
        _node_states.insert(std::make_pair(node_id, states));
    }
}

// Insertion or Removal of text in the given node_id
void CtStateMachine::text_variation(gint64 node_id, const Glib::ustring& varied_text)
{
    if (varied_text.find(CtConst::CHAR_NEWLINE) != Glib::ustring::npos)
    {
        update_state();
        return;
    }
    bool is_alphanum = _word_regex->match(varied_text); // we search for an alphanumeric character
    if (_node_states[node_id].indicator < 2)
    {
        if (is_alphanum) _node_states[node_id].indicator = 2; // alphanumeric transition
        else             _node_states[node_id].indicator = 1; // non alphanumeric transition
    }
    else if (!is_alphanum) // _nodes_indicators[node_id] == 2 and non alphanumeric transition
        update_state();
}

// A Previous State, if Existing, is Requested
std::shared_ptr<CtNodeState> CtStateMachine::requested_state_previous(gint64 node_id)
{
    if (curr_index_is_last_index(node_id))
        update_state();
    if (_node_states[node_id].index > 0)
        _node_states[node_id].index -= 1;
    return _node_states[node_id].get_state();
}

// The current state is requested
std::shared_ptr<CtNodeState> CtStateMachine::requested_state_current(gint64 node_id)
{
    return _node_states[node_id].get_state();
}

// A Subsequent State, if Existing, is Requested
std::shared_ptr<CtNodeState> CtStateMachine::requested_state_subsequent(gint64 node_id)
{
    if (_node_states[node_id].index < (int)_node_states[node_id].states.size()-1)
        _node_states[node_id].index += 1;
    return _node_states[node_id].get_state();
}

// Delete the states for the given node_id
void CtStateMachine::delete_states(gint64 node_id)
{
    _node_states.erase(node_id);
    if (vec::exists(_visited_nodes_list, node_id))
    {
        vec::remove(_visited_nodes_list, node_id);
        _visited_nodes_idx = _visited_nodes_list.size()-1;
    }
}

// Are we in the last state?
bool CtStateMachine::curr_index_is_last_index(gint64 node_id)
{
    int curr_index = _node_states[node_id].index;
    int last_index = _node_states[node_id].states.size() - 1;
    return curr_index == last_index;
}

void CtStateMachine::not_undoable_timeslot_set(bool not_undoable_val)
{
    _not_undoable_timeslot = not_undoable_val;
}

bool CtStateMachine::not_undoable_timeslot_get()
{
    return _not_undoable_timeslot;
}

// Update the state
void CtStateMachine::update_state()
{
    update_state(_pCtMainWin->curr_tree_iter());
}

// Update the state for the given node_id
void CtStateMachine::update_state(CtTreeIter tree_iter)
{
    if (not_undoable_timeslot_get()) return;
    if (!tree_iter) return;

    gint64 node_id = tree_iter.get_node_id();
    auto& node_states = _node_states[node_id];
    if (!node_states.states.empty() && !curr_index_is_last_index(node_id))
    {
        node_states.states.erase(node_states.states.begin() + node_states.index + 1, node_states.states.end());
    }

    auto new_state = std::shared_ptr<CtNodeState>(new CtNodeState());
    CtStorageXmlHelper(_pCtMainWin).save_buffer_no_widgets_to_xml(new_state->buffer_xml.get_root_node(), tree_iter.get_node_text_buffer(), 0, -1, 'n');
    new_state->buffer_xml_string = new_state->buffer_xml.write_to_string();
    for (auto widget: tree_iter.get_embedded_pixbufs_tables_codeboxes())
        new_state->widgetStates.push_back(widget->get_state());
    new_state->cursor_pos = tree_iter.get_node_text_buffer()->property_cursor_position();

    if (node_states.states.size() > 0)
    {
        auto compare_widgets = [](const std::list<std::shared_ptr<CtAnchoredWidgetState>> lhs, const std::list<std::shared_ptr<CtAnchoredWidgetState>> rhs) {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](std::shared_ptr<CtAnchoredWidgetState> lhs, std::shared_ptr<CtAnchoredWidgetState> rhs) {
                return lhs->equal(rhs);
            });
        };
        auto last_state = node_states.states.back();
        if (new_state->buffer_xml_string == last_state->buffer_xml_string && compare_widgets(new_state->widgetStates, last_state->widgetStates))
        {
            last_state->cursor_pos = new_state->cursor_pos;
            return; // #print "update_state not needed"
        }
    }

    node_states.states.push_back(new_state);
    while ((int)node_states.states.size() > _pCtMainWin->get_ct_config()->limitUndoableSteps)
        node_states.states.erase(node_states.states.begin());
    node_states.index = node_states.states.size() - 1;
    node_states.indicator = 0; // the current buffer state is saved
}

// If the buffer is still not modified update cursor pos
void CtStateMachine::update_curr_state_cursor_pos(gint64 node_id)
{
    if (!map::exists(_node_states, node_id)) return;
    int cursor_pos = _pCtMainWin->curr_buffer()->property_cursor_position();
    _node_states[node_id].get_state()->cursor_pos = cursor_pos;
}
