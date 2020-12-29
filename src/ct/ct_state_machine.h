/*
 * ct_state_machine.h
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

#pragma once

#include "ct_treestore.h"
#include "ct_image.h"
#include "ct_codebox.h"
#include "ct_table.h"
#include <vector>
#include <map>
#include <glibmm/regex.h>
#include <memory>

class CtMainWin;

class CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState(int charOffset, const std::string& justification) : charOffset(charOffset), justification(justification) {}
    virtual bool equal(std::shared_ptr<CtAnchoredWidgetState> state) = 0;
    virtual CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) = 0;

public:
    int charOffset;
    std::string justification;
};

class CtAnchoredWidgetState_ImagePng : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_ImagePng(CtImagePng* image);
    virtual ~CtAnchoredWidgetState_ImagePng() = default;
    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    Glib::ustring link;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
};

class CtAnchoredWidgetState_Anchor : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_Anchor(CtImageAnchor* anchor);
    virtual ~CtAnchoredWidgetState_Anchor() = default;
    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    Glib::ustring name;
};

class CtAnchoredWidgetState_EmbFile : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_EmbFile(CtImageEmbFile* embFile);
    virtual ~CtAnchoredWidgetState_EmbFile() = default;
    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    fs::path      fileName;
    std::string   rawBlob;      // raw data, not a string
    time_t        timeSeconds;
    const size_t  uniqueId;
};

class CtAnchoredWidgetState_Codebox : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_Codebox(CtCodebox* codebox);
    virtual ~CtAnchoredWidgetState_Codebox() = default;
    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    Glib::ustring content, syntax;
    int width, height;
    bool widthInPixels, brackets, showNum;
};

class CtAnchoredWidgetState_Table : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_Table(CtTable* table);
    virtual ~CtAnchoredWidgetState_Table() = default;
    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    int colWidthDefault;
    CtTableColWidths colWidths;
    std::vector<std::vector<Glib::ustring>> rows;
    size_t currRow;
    size_t currCol;
};

struct CtNodeState
{
    CtNodeState()  { buffer_xml.create_root_node("buffer"); }
    ~CtNodeState() { }

    std::list<std::shared_ptr<CtAnchoredWidgetState>> widgetStates;
    xmlpp::Document buffer_xml;
    Glib::ustring   buffer_xml_string;
    int             cursor_pos;
};

struct CtNodeStates
{
    std::vector<std::shared_ptr<CtNodeState>> states;
    int index;
    int indicator;

    std::shared_ptr<CtNodeState> get_state() { return states[index]; }
};

class CtStateMachine
{
public:
    CtStateMachine(CtMainWin* pCtMainWin);

    void reset();
    gint64 requested_visited_previous();
    gint64 requested_visited_next();
    void node_selected_changed(gint64 node_id);
    void text_variation(gint64 node_id, const Glib::ustring& varied_text);
    std::shared_ptr<CtNodeState> requested_state_previous(gint64 node_id);
    std::shared_ptr<CtNodeState> requested_state_current(gint64 node_id);
    std::shared_ptr<CtNodeState> requested_state_subsequent(gint64 node_id);
    void delete_states(gint64 node_id);
    bool curr_index_is_last_index(gint64 node_id);
    void not_undoable_timeslot_set(bool not_undoable_val);
    bool not_undoable_timeslot_get();
    void update_state();
    void update_state(CtTreeIter tree_iter);
    void update_curr_state_cursor_pos(gint64 node_id);

    void set_go_bk_fw_click(bool val) { _go_bk_fw_click = val; }

    const std::vector<gint64>& get_visited_nodes_list() { return _visited_nodes_list; }
    void  set_visited_nodes_list(const std::vector<gint64>& list) { _visited_nodes_list = list; _visited_nodes_idx = _visited_nodes_list.size() - 1;}

private:
    CtMainWin*                  _pCtMainWin;
    Glib::RefPtr<Glib::Regex>   _word_regex;
    bool                        _go_bk_fw_click;
    bool                        _not_undoable_timeslot;

    std::vector<gint64>         _visited_nodes_list;
    int                         _visited_nodes_idx;

    std::map<gint64, CtNodeStates> _node_states;
};

