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

void TheTreeStore::viewConnect(Gtk::TreeView *pTreeView)
{
    pTreeView->set_model(_rTreeStore);
}

void TheTreeStore::viewAppendColumns(Gtk::TreeView *pTreeView)
{
    Gtk::TreeView::Column* pColumns = Gtk::manage(new Gtk::TreeView::Column(""));
    pColumns->pack_start(_columns.rColPixbuf, /*expand=*/false);
    pColumns->pack_start(_columns.colNodeName);
    pColumns->pack_start(_columns.rColPixbufAux, /*expand=*/false);
    pTreeView->append_column(*pColumns);
}

bool TheTreeStore::readNodesFromFilepath(const char* filepath, const Gtk::TreeIter *pParentIter)
{
    bool retOk{false};
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
        p_ct_doc_read->m_signal_add_bookmark.connect(sigc::mem_fun(this, &TheTreeStore::onRequestAddBookmark));
        p_ct_doc_read->m_signal_append_node.connect(sigc::mem_fun(this, &TheTreeStore::onRequestAppendNode));
        p_ct_doc_read->treeWalk(pParentIter);
        delete p_ct_doc_read;
        retOk = true;
    }
    return retOk;
}

Glib::RefPtr<Gdk::Pixbuf> TheTreeStore::_getNodeIcon(int nodeDepth, Glib::ustring &syntax, guint32 customIconId)
{
    Glib::RefPtr<Gdk::Pixbuf> r_pixbuf;

    if (customIconId != 0)
    {
        // customIconId
        r_pixbuf = CTApplication::R_icontheme->load_icon(NODES_STOCKS.at(customIconId), NODE_ICON_SIZE);
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
            if (1 == NODES_ICONS.count(nodeDepth))
            {
                r_pixbuf = CTApplication::R_icontheme->load_icon(NODES_ICONS.at(nodeDepth), NODE_ICON_SIZE);
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
        r_pixbuf = CTApplication::R_icontheme->load_icon(getStockIdForCodeType(syntax), NODE_ICON_SIZE);
    }
    return r_pixbuf;
}

Gtk::TreeIter TheTreeStore::appendNode(CtNodeData *pNodeData, const Gtk::TreeIter *pParentIter)
{
    Gtk::TreeIter new_iter;
    //std::cout << pNodeData->name << std::endl;

    if (pParentIter == nullptr)
    {
        new_iter = _rTreeStore->append();
    }
    else
    {
        new_iter = _rTreeStore->append(static_cast<Gtk::TreeRow>(**pParentIter).children());
    }
    Gtk::TreeRow row = *new_iter;
    row[_columns.rColPixbuf] = _getNodeIcon(_rTreeStore->iter_depth(new_iter), pNodeData->syntax, pNodeData->customIconId);
    row[_columns.colNodeName] = pNodeData->name;
    //row[_columns.rColTextBuffer] = ;
    row[_columns.colNodeUniqueId] = pNodeData->nodeId;
    row[_columns.colSyntaxHighlighting] = pNodeData->syntax;
    //row[_columns.colNodeSequence] = ;
    row[_columns.colNodeTags] = pNodeData->tags;
    row[_columns.colNodeRO] = pNodeData->isRO;
    //row[_columns.rColPixbufAux] = ;
    row[_columns.colCustomIconId] = pNodeData->customIconId;
    row[_columns.colWeight] = _getPangoWeight(pNodeData->isBold);
    //row[_columns.colForeground] = ;
    row[_columns.colTsCreation] = pNodeData->tsCreation;
    row[_columns.colTsLastSave] = pNodeData->tsLastSave;
    return new_iter;
}

void TheTreeStore::onRequestAddBookmark(gint64 nodeId)
{
    _bookmarks.push_back(nodeId);
}

guint16 TheTreeStore::_getPangoWeight(bool isBold)
{
    return isBold ? PANGO_WEIGHT_HEAVY : PANGO_WEIGHT_NORMAL;
}

Gtk::TreeIter TheTreeStore::onRequestAppendNode(CtNodeData *pNodeData, const Gtk::TreeIter *pParentIter)
{
    return appendNode(pNodeData, pParentIter);
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
