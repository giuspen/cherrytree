/*
 * ct_state_machine.h
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
    CtAnchoredWidgetState(int charOffset, const std::string& justification)
     : charOffset(charOffset)
     , justification(justification)
    {}
    virtual ~CtAnchoredWidgetState() {}

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

    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    Glib::ustring name;
};

class CtAnchoredWidgetState_Latex : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_Latex(CtImageLatex* latex);

    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    Glib::ustring text;
    const size_t  uniqueId;
};

class CtAnchoredWidgetState_EmbFile : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_EmbFile(CtImageEmbFile* embFile);

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

    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override;

public:
    Glib::ustring content, syntax;
    int width, height;
    bool widthInPixels, brackets, showNum;
};

class CtAnchoredWidgetState_TableCommon : public CtAnchoredWidgetState
{
public:
    CtAnchoredWidgetState_TableCommon(const CtTableCommon* table);

    bool equal(std::shared_ptr<CtAnchoredWidgetState> state) override;

    CtAnchoredWidget* to_widget(CtMainWin* /*pCtMainWin*/) override { return nullptr; }
    CtTableLight* to_widget_light(CtMainWin* pCtMainWin) const;
    CtTableHeavy* to_widget_heavy(CtMainWin* pCtMainWin) const;

    int colWidthDefault;
    CtTableColWidths colWidths;
    std::vector<std::vector<Glib::ustring>> rows;
    size_t currRow;
    size_t currCol;
};
class CtAnchoredWidgetState_TableLight : public CtAnchoredWidgetState_TableCommon
{
public:
    CtAnchoredWidgetState_TableLight(const CtTableLight* table)
     : CtAnchoredWidgetState_TableCommon{table}
    {}
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override {
        return to_widget_light(pCtMainWin);
    }
};
class CtAnchoredWidgetState_TableHeavy : public CtAnchoredWidgetState_TableCommon
{
public:
    CtAnchoredWidgetState_TableHeavy(const CtTableHeavy* table)
     : CtAnchoredWidgetState_TableCommon{table}
    {}
    CtAnchoredWidget* to_widget(CtMainWin* pCtMainWin) override {
        return to_widget_heavy(pCtMainWin);
    }
};

struct CtNodeState
{
    CtNodeState()  { buffer_xml.create_root_node("buffer"); }

    std::list<std::shared_ptr<CtAnchoredWidgetState>> widgetStates;
    xmlpp::Document buffer_xml;
    Glib::ustring   buffer_xml_string;
    int             cursor_pos{0};
    int             v_adj_val{0};
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
    void node_selected_changed(const gint64 node_id_data_holder);
    void text_variation(const gint64 node_id_data_holder, const Glib::ustring& varied_text);
    std::shared_ptr<CtNodeState> requested_state_previous(const gint64 node_id_data_holder);
    std::shared_ptr<CtNodeState> requested_state_current(const gint64 node_id_data_holder);
    std::shared_ptr<CtNodeState> requested_state_subsequent(const gint64 node_id_data_holder);
    void delete_states(const gint64 node_id_data_holder);
    bool curr_index_is_last_index(const gint64 node_id_data_holder);
    void not_undoable_timeslot_set(bool not_undoable_val);
    bool not_undoable_timeslot_get();
    void update_state();
    void update_state(CtTreeIter tree_iter);
    void update_curr_state_cursor_pos(const gint64 node_id_data_holder);
    void update_curr_state_v_adj_val(const gint64 node_id_data_holder);

    void set_go_bk_fw_active(bool val) { _go_bk_fw_active = val; }

    const std::vector<gint64>& get_visited_nodes_list() { return _visited_nodes_list; }
    void set_visited_nodes_list(const std::vector<gint64>& list) {
        _visited_nodes_list = list;
        _visited_nodes_idx = _visited_nodes_list.size() - 1;
    }

private:
    CtMainWin*                  _pCtMainWin;
    Glib::RefPtr<Glib::Regex>   _word_regex;
    bool                        _go_bk_fw_active;
    bool                        _not_undoable_timeslot;

    std::vector<gint64>         _visited_nodes_list;
    int                         _visited_nodes_idx;

    std::map<gint64, CtNodeStates> _node_states;
};
