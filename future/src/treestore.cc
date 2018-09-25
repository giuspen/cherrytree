/*
 * treestore.cc
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

#include <glibmm/i18n.h>
#include <iostream>
#include <set>
#include "treestore.h"
#include "ct_doc_rw.h"
#include "ct_app.h"


TheTreeStore::TheTreeStore()
{
    _rTreeStore = Gtk::TreeStore::create(_columns);
}

TheTreeStore::~TheTreeStore()
{
}

void TheTreeStore::view_connect(Gtk::TreeView *pTreeView)
{
    pTreeView->set_model(_rTreeStore);
}

void TheTreeStore::view_append_columns(Gtk::TreeView *pTreeView)
{
    Gtk::TreeView::Column* pColumns = Gtk::manage(new Gtk::TreeView::Column(""));
    pColumns->pack_start(_columns.rColPixbuf, /*expand=*/false);
    pColumns->pack_start(_columns.colNodeName);
    pColumns->pack_start(_columns.rColPixbufAux, /*expand=*/false);
    pTreeView->append_column(*pColumns);
}

bool TheTreeStore::readNodesFromFilepath(const char* filepath, const Gtk::TreeIter *pParentIter)
{
    bool retOk = false;
    CherryTreeDocRead* p_ct_doc_read{nullptr};
    if (g_str_has_suffix(filepath, ".ctd"))
    {
        p_ct_doc_read = new CherryTreeXMLRead(filepath);
    }
    else if (g_str_has_suffix(filepath, ".ctb"))
    {
        p_ct_doc_read = new CherryTreeSQLiteRead(filepath);
    }
    if (p_ct_doc_read != nullptr)
    {
        p_ct_doc_read->m_signal_add_bookmark.connect(sigc::mem_fun(this, &TheTreeStore::on_request_add_bookmark));
        p_ct_doc_read->m_signal_append_node.connect(sigc::mem_fun(this, &TheTreeStore::on_request_append_node));
        p_ct_doc_read->tree_walk(pParentIter);
        delete p_ct_doc_read;
        retOk = true;
    }
    return retOk;
}

Glib::RefPtr<Gdk::Pixbuf> TheTreeStore::_get_node_icon(int node_depth, Glib::ustring &syntax, guint32 custom_icon_id)
{
    Glib::RefPtr<Gdk::Pixbuf> r_pixbuf;

    if (custom_icon_id != 0)
    {
        // custom_icon_id
        r_pixbuf = CTApplication::R_icontheme->load_icon(NODES_STOCKS.at(custom_icon_id), NODE_ICON_SIZE);
    }
    else if (NODE_ICON_TYPE_NONE == CTApplication::P_ctCfg->nodesIcons)
    {
        // NODE_ICON_TYPE_NONE
        r_pixbuf = CTApplication::R_icontheme->load_icon(NODES_STOCKS.at(NODE_ICON_NO_ICON_ID), NODE_ICON_SIZE);
    }
    else if (1 == std::set<Glib::ustring>({RICH_TEXT_ID, PLAIN_TEXT_ID}).count(syntax))
    {
        // text node
        if (NODE_ICON_TYPE_CHERRY == CTApplication::P_ctCfg->nodesIcons)
        {
            if (1 == NODES_ICONS.count(node_depth))
            {
                r_pixbuf = CTApplication::R_icontheme->load_icon(NODES_ICONS.at(node_depth), NODE_ICON_SIZE);
            }
            else
            {
                r_pixbuf = CTApplication::R_icontheme->load_icon(NODES_ICONS.at(-1), NODE_ICON_SIZE);
            }
        }
        else
        {
            // NODE_ICON_TYPE_CUSTOM
            r_pixbuf = CTApplication::R_icontheme->load_icon(NODES_STOCKS.at(CTApplication::P_ctCfg->defaultIconText), NODE_ICON_SIZE);
        }
    }
    else
    {
        // code node
        r_pixbuf = CTApplication::R_icontheme->load_icon(get_stock_id_for_code_type(syntax), NODE_ICON_SIZE);
    }
    return r_pixbuf;
}

Gtk::TreeIter TheTreeStore::append_node(t_ct_node_data *p_node_data, const Gtk::TreeIter *p_parent_iter)
{
    Gtk::TreeIter new_iter;
    //std::cout << p_node_data->name << std::endl;

    if (p_parent_iter == nullptr)
    {
        new_iter = _rTreeStore->append();
    }
    else
    {
        new_iter = _rTreeStore->append(static_cast<Gtk::TreeRow>(**p_parent_iter).children());
    }
    Gtk::TreeRow row = *new_iter;
    row[_columns.rColPixbuf] = _get_node_icon(_rTreeStore->iter_depth(new_iter), p_node_data->syntax, p_node_data->custom_icon_id);
    row[_columns.colNodeName] = p_node_data->name;
    //row[_columns.rColTextBuffer] = ;
    row[_columns.colNodeUniqueId] = p_node_data->node_id;
    row[_columns.colSyntaxHighlighting] = p_node_data->syntax;
    //row[_columns.colNodeSequence] = ;
    row[_columns.colNodeTags] = p_node_data->tags;
    row[_columns.colNodeRO] = p_node_data->is_ro;
    //row[_columns.rColPixbufAux] = ;
    row[_columns.colCustomIconId] = p_node_data->custom_icon_id;
    row[_columns.colWeight] = _get_pango_weight(p_node_data->is_bold);
    //row[_columns.colForeground] = ;
    row[_columns.colTsCreation] = p_node_data->ts_creation;
    row[_columns.colTsLastSave] = p_node_data->ts_lastsave;
    return new_iter;
}

void TheTreeStore::on_request_add_bookmark(gint64 node_id)
{
    _bookmarks.push_back(node_id);
}

guint16 TheTreeStore::_get_pango_weight(bool is_bold)
{
    return is_bold ? PANGO_WEIGHT_HEAVY : PANGO_WEIGHT_NORMAL;
}

Gtk::TreeIter TheTreeStore::on_request_append_node(t_ct_node_data *p_node_data, const Gtk::TreeIter *p_parent_iter)
{
    return append_node(p_node_data, p_parent_iter);
}

Glib::ustring TheTreeStore::getNodeName(Gtk::TreeIter treeIter)
{
    Glib::ustring retNodeName;
    if (treeIter)
    {
        Gtk::TreeRow treeRow = *treeIter;
        retNodeName = treeRow[_columns.colNodeName];
    }
    return retNodeName;
}
