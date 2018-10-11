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
    CtDocType docType = CtMiscUtil::getDocType(filepath);
    CherryTreeDocRead* pCtDocRead{nullptr};
    if (CtDocType::XML == docType)
    {
        pCtDocRead = new CherryTreeXMLRead(filepath);
    }
    else if (CtDocType::SQLite == docType)
    {
        pCtDocRead = new CherryTreeSQLiteRead(filepath);
    }
    if (pCtDocRead != nullptr)
    {
        pCtDocRead->signalAddBookmark.connect(sigc::mem_fun(this, &TheTreeStore::onRequestAddBookmark));
        pCtDocRead->signalAppendNode.connect(sigc::mem_fun(this, &TheTreeStore::onRequestAppendNode));
        pCtDocRead->treeWalk(pParentIter);
        delete pCtDocRead;
        retOk = true;
    }
    return retOk;
}

Glib::RefPtr<Gdk::Pixbuf> TheTreeStore::_getNodeIcon(int nodeDepth, std::string &syntax, guint32 customIconId)
{
    Glib::RefPtr<Gdk::Pixbuf> rPixbuf;

    if (0 != customIconId)
    {
        // customIconId
        rPixbuf = CTApplication::R_icontheme->load_icon(CtConst::NODES_STOCKS.at(customIconId), CtConst::NODE_ICON_SIZE);
    }
    else if (CtConst::NODE_ICON_TYPE_NONE == CTApplication::P_ctCfg->nodesIcons)
    {
        // NODE_ICON_TYPE_NONE
        rPixbuf = CTApplication::R_icontheme->load_icon(CtConst::NODES_STOCKS.at(CtConst::NODE_ICON_NO_ICON_ID), CtConst::NODE_ICON_SIZE);
    }
    else if (1 == std::set<Glib::ustring>({CtConst::RICH_TEXT_ID, CtConst::PLAIN_TEXT_ID}).count(syntax))
    {
        // text node
        if (CtConst::NODE_ICON_TYPE_CHERRY == CTApplication::P_ctCfg->nodesIcons)
        {
            if (1 == CtConst::NODES_ICONS.count(nodeDepth))
            {
                rPixbuf = CTApplication::R_icontheme->load_icon(CtConst::NODES_ICONS.at(nodeDepth),CtConst:: NODE_ICON_SIZE);
            }
            else
            {
                rPixbuf = CTApplication::R_icontheme->load_icon(CtConst::NODES_ICONS.at(-1), CtConst::NODE_ICON_SIZE);
            }
        }
        else
        {
            // NODE_ICON_TYPE_CUSTOM
            rPixbuf = CTApplication::R_icontheme->load_icon(CtConst::NODES_STOCKS.at(CTApplication::P_ctCfg->defaultIconText), CtConst::NODE_ICON_SIZE);
        }
    }
    else
    {
        // code node
        rPixbuf = CTApplication::R_icontheme->load_icon(CtConst::getStockIdForCodeType(syntax), CtConst::NODE_ICON_SIZE);
    }
    return rPixbuf;
}

Gtk::TreeIter TheTreeStore::appendNode(CtNodeData *pNodeData, const Gtk::TreeIter *pParentIter)
{
    Gtk::TreeIter newIter;
    //std::cout << pNodeData->name << std::endl;

    if (nullptr == pParentIter)
    {
        newIter = _rTreeStore->append();
    }
    else
    {
        newIter = _rTreeStore->append(static_cast<Gtk::TreeRow>(**pParentIter).children());
    }
    Gtk::TreeRow row = *newIter;
    row[_columns.rColPixbuf] = _getNodeIcon(_rTreeStore->iter_depth(newIter), pNodeData->syntax, pNodeData->customIconId);
    row[_columns.colNodeName] = pNodeData->name;
    row[_columns.rColTextBuffer] = pNodeData->rTextBuffer;
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
    return newIter;
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
