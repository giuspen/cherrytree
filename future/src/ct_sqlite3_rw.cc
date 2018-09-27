/*
 * ct_sqlite3_rw.cc
 *
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <iostream>
#include "ct_doc_rw.h"
#include "str_utils.h"


CherryTreeSQLiteRead::CherryTreeSQLiteRead(const char* filepath)
{
    int ret_code = sqlite3_open(filepath, &mp_db);
    if(ret_code != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_open: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
}


CherryTreeSQLiteRead::~CherryTreeSQLiteRead()
{
    sqlite3_close(mp_db);
}


void CherryTreeSQLiteRead::treeWalk(const Gtk::TreeIter *pParentIter)
{
    sqlite3_stmt *p_stmt;
    if(sqlite3_prepare_v2(mp_db, "SELECT nodeId FROM bookmark ORDER BY sequence ASC", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
    while(sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        gint64 nodeId = sqlite3_column_int64(p_stmt, 0);
        m_signal_add_bookmark.emit(nodeId);
    }
    sqlite3_finalize(p_stmt);

    std::list<gint64> top_nodes_ids = _sqlite3GetChildrenNodeIdFromFatherId(0);
    for(gint64 &top_node_id : top_nodes_ids)
    {
        _sqlite3TreeWalkIter(top_node_id, pParentIter);
    }
}


void CherryTreeSQLiteRead::_sqlite3TreeWalkIter(gint64 nodeId, const Gtk::TreeIter *pParentIter)
{
    Gtk::TreeIter new_iter = _sqlite3NodeProcess(nodeId, pParentIter);

    std::list<gint64> children_nodes_ids = _sqlite3GetChildrenNodeIdFromFatherId(nodeId);
    for(gint64 &child_node_id : children_nodes_ids)
    {
        _sqlite3TreeWalkIter(child_node_id, &new_iter);
    }
}


std::list<gint64> CherryTreeSQLiteRead::_sqlite3GetChildrenNodeIdFromFatherId(gint64 father_id)
{
    std::list<gint64> ret_children;
    sqlite3_stmt *p_stmt;
    if(sqlite3_prepare_v2(mp_db, "SELECT nodeId FROM children WHERE father_id=? ORDER BY sequence ASC", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
    sqlite3_bind_int(p_stmt, 1, father_id);
    while(sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        gint64 nodeId = sqlite3_column_int64(p_stmt, 0);
        ret_children.push_back(nodeId);
    }
    sqlite3_finalize(p_stmt);
    return ret_children;
}


CtNodeData CherryTreeSQLiteRead::_sqlite3GetNodeProperties(gint64 nodeId)
{
    CtNodeData nodeData;
    nodeData.nodeId = nodeId;
    sqlite3_stmt *p_stmt;
    if(sqlite3_prepare_v2(mp_db, "SELECT name, syntax, tags, isRO, is_richtxt, tsCreation, tsLastSave FROM node WHERE nodeId=?", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
    sqlite3_bind_int(p_stmt, 1, nodeId);
    if(sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        nodeData.name = (const char*)sqlite3_column_text(p_stmt, 0);
        nodeData.syntax = (const char*)sqlite3_column_text(p_stmt, 1);
        nodeData.tags = (const char*)sqlite3_column_text(p_stmt, 2);
        gint64 readonly_n_custom_icon_id = sqlite3_column_int64(p_stmt, 3);
        nodeData.isRO = bool(readonly_n_custom_icon_id & 0x01);
        nodeData.customIconId = readonly_n_custom_icon_id >> 1;
        gint64 richtxt_bold_foreground = sqlite3_column_int64(p_stmt, 4);
        nodeData.isBold = bool((richtxt_bold_foreground >> 1) & 0x01);
        nodeData.fgOverride = bool((richtxt_bold_foreground >> 2) & 0x01);
        if(nodeData.fgOverride)
        {
            set_rgb24_str_from_rgb24_int((richtxt_bold_foreground >> 3) & 0xffffff, nodeData.foregroundRgb24);
        }
        nodeData.tsCreation = sqlite3_column_int64(p_stmt, 5);
        nodeData.tsLastSave = sqlite3_column_int64(p_stmt, 6);
    }
    else
    {
        std::cerr << "!! missing node properties for id " << nodeId << std::endl;
    }
    sqlite3_finalize(p_stmt);
    return nodeData;
}


Gtk::TreeIter CherryTreeSQLiteRead::_sqlite3NodeProcess(gint64 nodeId, const Gtk::TreeIter *pParentIter)
{
    CtNodeData nodeData = _sqlite3GetNodeProperties(nodeId);
    Gtk::TreeIter new_iter = m_signal_append_node.emit(&nodeData, pParentIter);
    return new_iter;
}
