/*
 * ct_treestore.cc
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

#include <assert.h>
#include <algorithm>
#include "ct_doc_rw.h"
#include "ct_app.h"
#include "ct_treestore.h"
#include "ct_misc_utils.h"

CtTreeIter::CtTreeIter(Gtk::TreeIter iter, const CtTreeModelColumns* columns)
 : Gtk::TreeIter(iter),
   _columns(columns)
{
}

CtTreeIter CtTreeIter::parent()
{
    return CtTreeIter((*this)->parent(), _columns);
}

bool CtTreeIter::get_node_read_only() const
{
    return (*this) && (*this)->get_value(_columns->colNodeRO);
}

void CtTreeIter::set_node_read_only(bool val)
{
    (*this)->set_value(_columns->colNodeRO, val);
}

gint64 CtTreeIter::get_node_id() const
{
    if (*this)
    {
        return (*this)->get_value(_columns->colNodeUniqueId);
    }
    return -1;
}

std::string CtTreeIter::get_node_name() const
{
    if (*this)
    {
        return (*this)->get_value(_columns->colNodeName);
    }
    return std::string();
}

void CtTreeIter::set_node_name(const Glib::ustring& node_name)
{
    (*this)->set_value(_columns->colNodeName, node_name);
}

std::string CtTreeIter::get_node_tags() const
{
    if (*this)
    {
        return (*this)->get_value(_columns->colNodeTags);
    }
    return std::string();
}

std::string CtTreeIter::get_node_foreground() const
{
    if (*this)
    {
        return (*this)->get_value(_columns->colForeground);
    }
    return std::string();
}

std::time_t CtTreeIter::get_node_creating_time() const
{
    if (*this)
    {
        return (*this)->get_value(_columns->colTsCreation);
    }
    return 0;
}

std::time_t CtTreeIter::get_node_modification_time() const
{
    if (*this)
    {
        return (*this)->get_value(_columns->colTsLastSave);
    }
    return 0;
}

void CtTreeIter::set_node_aux_icon(Glib::RefPtr<Gdk::Pixbuf> rPixbuf)
{
    (*this)->set_value(_columns->rColPixbufAux, rPixbuf);
}

Glib::RefPtr<Gsv::Buffer> CtTreeIter::get_node_text_buffer() const
{
    if (*this)
    {
        return (*this)->get_value(_columns->rColTextBuffer);
    }
    return Glib::RefPtr<Gsv::Buffer>();
}


CtTreeStore::CtTreeStore()
{
    _rTreeStore = Gtk::TreeStore::create(_columns);
}

CtTreeStore::~CtTreeStore()
{
    _iterDeleteAnchoredWidgets(getRootChildren());
    if (nullptr != _pCtSQLiteRead)
    {
        delete _pCtSQLiteRead;
    }
}

void CtTreeStore::_iterDeleteAnchoredWidgets(const Gtk::TreeModel::Children& children)
{
    for (Gtk::TreeIter treeIter = children.begin(); treeIter != children.end(); ++treeIter)
    {
        Gtk::TreeRow row = *treeIter;
        for (CtAnchoredWidget* pCtAnchoredWidget : row.get_value(_columns.colAnchoredWidgets))
        {
            delete pCtAnchoredWidget;
            //printf("~pCtAnchoredWidget\n");
        }
        row.get_value(_columns.colAnchoredWidgets).clear();

        _iterDeleteAnchoredWidgets(row.children());
    }
}

void CtTreeStore::expandToTreeRow(Gtk::TreeView* pTreeView, Gtk::TreeRow& row)
{
    Gtk::TreeIter iterParent = row.parent();
    if (static_cast<bool>(iterParent))
    {
        pTreeView->expand_to_path(_rTreeStore->get_path(iterParent));
    }
}

void CtTreeStore::treeviewSafeSetCursor(Gtk::TreeView* pTreeView, Gtk::TreeIter& treeIter)
{
    Gtk::TreeRow row = *treeIter;
    expandToTreeRow(pTreeView, row);
    pTreeView->set_cursor(_rTreeStore->get_path(treeIter));
}

void CtTreeStore::setTreePathTextCursorFromConfig(Gtk::TreeView* pTreeView, Gsv::View* pTextView)
{
    bool treeSelFromConfig{false};
    if (!CtApp::P_ctCfg->nodePath.empty())
    {
        Gtk::TreeIter treeIter = _rTreeStore->get_iter(CtApp::P_ctCfg->nodePath);
        if (static_cast<bool>(treeIter))
        {
            treeviewSafeSetCursor(pTreeView, treeIter);
            treeSelFromConfig = true;
            if (CtApp::P_ctCfg->treeClickExpand)
            {
                pTreeView->expand_row(_rTreeStore->get_path(treeIter), false/*open_all*/);
            }
            Glib::RefPtr<Gsv::Buffer> rTextBuffer = (*treeIter).get_value(_columns.rColTextBuffer);
            Gtk::TextIter textIter = rTextBuffer->get_iter_at_offset(CtApp::P_ctCfg->cursorPosition);
            if (static_cast<bool>(textIter))
            {
                rTextBuffer->place_cursor(textIter);
                pTextView->scroll_to(textIter, CtTextView::TEXT_SCROLL_MARGIN);
            }
        }
    }
    if (!treeSelFromConfig)
    {
        Gtk::TreeIter treeIter = _rTreeStore->get_iter("0");
        if (static_cast<bool>(treeIter))
        {
            pTreeView->set_cursor(_rTreeStore->get_path(treeIter));
        }
    }
}

void CtTreeStore::viewConnect(Gtk::TreeView* pTreeView)
{
    pTreeView->set_model(_rTreeStore);
}

void CtTreeStore::viewAppendColumns(Gtk::TreeView* pTreeView)
{
    Gtk::TreeView::Column* pColumns = Gtk::manage(new Gtk::TreeView::Column(""));
    pColumns->pack_start(_columns.rColPixbuf, /*expand=*/false);
    pColumns->pack_start(_columns.colNodeName);
    pColumns->set_expand(true);
    pTreeView->append_column(*pColumns);
    pTreeView->append_column("", _columns.rColPixbufAux);
}

bool CtTreeStore::readNodesFromFilepath(const char* filepath, const bool isImport, const Gtk::TreeIter* pParentIter)
{
    bool retOk{false};
    CtDocType docType = CtMiscUtil::getDocType(filepath);
    CtDocRead* pCtDocRead{nullptr};
    if (CtDocType::XML == docType)
    {
        pCtDocRead = new CtXmlRead(filepath, nullptr);
    }
    else if (CtDocType::SQLite == docType)
    {
        pCtDocRead = new CtSQLiteRead(filepath);
    }
    if (pCtDocRead != nullptr)
    {
        pCtDocRead->signalAddBookmark.connect(sigc::mem_fun(this, &CtTreeStore::onRequestAddBookmark));
        pCtDocRead->signalAppendNode.connect(sigc::mem_fun(this, &CtTreeStore::onRequestAppendNode));
        pCtDocRead->treeWalk(pParentIter);
        if (!isImport && (CtDocType::SQLite == docType))
        {
            _pCtSQLiteRead = dynamic_cast<CtSQLiteRead*>(pCtDocRead);
        }
        else
        {
            delete pCtDocRead;
        }
        retOk = true;
    }
    return retOk;
}

Glib::RefPtr<Gdk::Pixbuf> CtTreeStore::_getNodeIcon(int nodeDepth, const std::string &syntax, guint32 customIconId)
{
    Glib::RefPtr<Gdk::Pixbuf> rPixbuf;

    if (0 != customIconId)
    {
        // customIconId
        rPixbuf = CtApp::R_icontheme->load_icon(CtConst::NODES_STOCKS.at(customIconId), CtConst::NODE_ICON_SIZE);
    }
    else if (CtConst::NODE_ICON_TYPE_NONE == CtApp::P_ctCfg->nodesIcons)
    {
        // NODE_ICON_TYPE_NONE
        rPixbuf = CtApp::R_icontheme->load_icon(CtConst::NODES_STOCKS.at(CtConst::NODE_ICON_NO_ICON_ID), CtConst::NODE_ICON_SIZE);
    }
    else if (CtStrUtil::isPgcharInPgcharSet(syntax.c_str(), CtConst::TEXT_SYNTAXES))
    {
        // text node
        if (CtConst::NODE_ICON_TYPE_CHERRY == CtApp::P_ctCfg->nodesIcons)
        {
            if (1 == CtConst::NODES_ICONS.count(nodeDepth))
            {
                rPixbuf = CtApp::R_icontheme->load_icon(CtConst::NODES_ICONS.at(nodeDepth), CtConst::NODE_ICON_SIZE);
            }
            else
            {
                rPixbuf = CtApp::R_icontheme->load_icon(CtConst::NODES_ICONS.at(-1), CtConst::NODE_ICON_SIZE);
            }
        }
        else
        {
            // NODE_ICON_TYPE_CUSTOM
            rPixbuf = CtApp::R_icontheme->load_icon(CtConst::NODES_STOCKS.at(CtApp::P_ctCfg->defaultIconText), CtConst::NODE_ICON_SIZE);
        }
    }
    else
    {
        // code node
        rPixbuf = CtApp::R_icontheme->load_icon(CtConst::getStockIdForCodeType(syntax), CtConst::NODE_ICON_SIZE);
    }
    return rPixbuf;
}

void CtTreeStore::getNodeData(Gtk::TreeIter treeIter, CtNodeData& nodeData)
{
    Gtk::TreeRow row = *treeIter;

    nodeData.name =  row[_columns.colNodeName];
    nodeData.rTextBuffer = row[_columns.rColTextBuffer];
    nodeData.nodeId = nodeData.nodeId = row[_columns.colNodeUniqueId];
    nodeData.syntax = row[_columns.colSyntaxHighlighting];
    //row[_columns.colNodeSequence] = ;
    nodeData.tags = row[_columns.colNodeTags];
    nodeData.isRO = row[_columns.colNodeRO];
    //row[_columns.rColPixbufAux] = ;
    nodeData.customIconId = row[_columns.colCustomIconId];
    nodeData.isBold = _getBold(row[_columns.colWeight]);
    nodeData.foregroundRgb24 = row[_columns.colForeground];
    nodeData.tsCreation = row[_columns.colTsCreation];
    nodeData.tsLastSave = row[_columns.colTsLastSave];
    nodeData.anchoredWidgets = row[_columns.colAnchoredWidgets];
}

void CtTreeStore::updateNodeData(Gtk::TreeIter treeIter, const CtNodeData& nodeData)
{
    Gtk::TreeRow row = *treeIter;
    row[_columns.rColPixbuf] = _getNodeIcon(_rTreeStore->iter_depth(treeIter), nodeData.syntax, nodeData.customIconId);
    row[_columns.colNodeName] = nodeData.name;
    row[_columns.rColTextBuffer] = nodeData.rTextBuffer;
    row[_columns.colNodeUniqueId] = nodeData.nodeId;
    row[_columns.colSyntaxHighlighting] = nodeData.syntax;
    //row[_columns.colNodeSequence] = ;
    row[_columns.colNodeTags] = nodeData.tags;
    row[_columns.colNodeRO] = nodeData.isRO;
    //row[_columns.rColPixbufAux] = ;
    row[_columns.colCustomIconId] = nodeData.customIconId;
    row[_columns.colWeight] = _getPangoWeight(nodeData.isBold);
    row[_columns.colForeground] = nodeData.foregroundRgb24;
    row[_columns.colTsCreation] = nodeData.tsCreation;
    row[_columns.colTsLastSave] = nodeData.tsLastSave;
    row[_columns.colAnchoredWidgets] = nodeData.anchoredWidgets;

    updateNodeAuxIcon(treeIter);
    add_used_tags(nodeData.tags);
    _nodes_names_dict[nodeData.nodeId] = nodeData.name;
}

void CtTreeStore::updateNodeAuxIcon(Gtk::TreeIter treeIter)
{
    bool is_ro = treeIter->get_value(_columns.colNodeRO);
    bool is_bookmark = set::exists(_bookmarks, treeIter->get_value(_columns.colNodeUniqueId));
    std::string stock_id;
    if (is_ro && is_bookmark) stock_id = "lockpin";
    else if (is_ro)           stock_id = "locked";
    else if (is_bookmark)     stock_id = "pin";

    if (stock_id.empty())
        treeIter->set_value(_columns.rColPixbufAux, Glib::RefPtr<Gdk::Pixbuf>());
    else
        treeIter->set_value(_columns.rColPixbufAux, CtApp::R_icontheme->load_icon(stock_id, CtConst::NODE_ICON_SIZE));
}

Gtk::TreeIter CtTreeStore::appendNode(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter)
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
    updateNodeData(newIter, *pNodeData);
    return newIter;
}

Gtk::TreeIter CtTreeStore::insertNode(CtNodeData* pNodeData, const Gtk::TreeIter& afterIter)
{
    Gtk::TreeIter newIter = _rTreeStore->insert_after(afterIter);
    updateNodeData(newIter, *pNodeData);
    return newIter;
}

bool CtTreeStore::onRequestAddBookmark(gint64 nodeId)
{
    if (set::exists(_bookmarks, nodeId))
    {
        return false;
    }
    _bookmarks.insert(nodeId);
    _bookmarks_order.push_back(nodeId);
    return true;
}

bool CtTreeStore::onRequestRemoveBookmark(gint64 nodeId)
{
    if (!set::exists(_bookmarks, nodeId))
    {
        return false;
    }
    set::remove(_bookmarks, nodeId);
    vec::remove(_bookmarks_order, nodeId);
    return true;
}

guint16 CtTreeStore::_getPangoWeight(bool isBold)
{
    return isBold ? PANGO_WEIGHT_HEAVY : PANGO_WEIGHT_NORMAL;
}

bool CtTreeStore::_getBold(guint16 pangoWeight)
{
    return pangoWeight == PANGO_WEIGHT_HEAVY;
}

Gtk::TreeIter CtTreeStore::onRequestAppendNode(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter)
{
    return appendNode(pNodeData, pParentIter);
}

Glib::RefPtr<Gsv::Buffer> CtTreeStore::_getNodeTextBuffer(const Gtk::TreeIter& treeIter)
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};
    if (treeIter)
    {
        Gtk::TreeRow treeRow = *treeIter;
        rRetTextBuffer = treeRow.get_value(_columns.rColTextBuffer);
        if (!rRetTextBuffer)
        {
            // SQLite text buffer not yet populated
            assert(nullptr != _pCtSQLiteRead);
            std::list<CtAnchoredWidget*> anchoredWidgetList = treeRow.get_value(_columns.colAnchoredWidgets);
            rRetTextBuffer = _pCtSQLiteRead->getTextBuffer(treeRow.get_value(_columns.colSyntaxHighlighting),
                                                           anchoredWidgetList,
                                                           treeRow.get_value(_columns.colNodeUniqueId));
            treeRow.set_value(_columns.colAnchoredWidgets, anchoredWidgetList);
            treeRow.set_value(_columns.rColTextBuffer, rRetTextBuffer);
        }
    }
    return rRetTextBuffer;
}

void CtTreeStore::applyTextBufferToCtTextView(const Gtk::TreeIter& treeIter, CtTextView* pTextView)
{
    if (!treeIter)
    {
        std::cerr << "!! treeIter" << std::endl;
        return;
    }
    Gtk::TreeRow treeRow = *treeIter;
    std::cout << treeRow.get_value(_columns.colNodeName) << std::endl;
    Glib::RefPtr<Gsv::Buffer> rTextBuffer = _getNodeTextBuffer(treeIter);
    pTextView->setupForSyntax(treeRow.get_value(_columns.colSyntaxHighlighting));
    pTextView->set_buffer(rTextBuffer);
    for (CtAnchoredWidget* pCtAnchoredWidget : treeRow.get_value(_columns.colAnchoredWidgets))
    {
        Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = pCtAnchoredWidget->getTextChildAnchor();
        if (rChildAnchor)
        {
            if (0 == rChildAnchor->get_widgets().size())
            {
                Gtk::TextIter textIter = rTextBuffer->get_iter_at_child_anchor(rChildAnchor);
                pTextView->add_child_at_anchor(*pCtAnchoredWidget, rChildAnchor);
                pCtAnchoredWidget->applyWidthHeight(pTextView->get_allocation().get_width());
            }
            else
            {
                // this happens if we click on a node that is already selected, not an issue
                // we simply must not add the same widget to the anchor again
            }
        }
        else
        {
            std::cerr << "!! rChildAnchor" << std::endl;
        }
    }
    pTextView->show_all();
    pTextView->grab_focus();
}

gint64 CtTreeStore::node_id_get()
{
    // todo: this function works differently from python code
    // it's easer to find max than check every id is not used through all tree
    gint64 max_id = 0;
    _rTreeStore->foreach([&max_id, this](const Gtk::TreeModel::Path&, const Gtk::TreeIter& iter) -> bool{
        max_id = std::max(max_id, iter->get_value(_columns.colNodeUniqueId));
        return false;
    });
    return max_id+1;
}

void CtTreeStore::add_used_tags(const std::string& tags)
{
    std::vector<std::string> tagVec = str::split(tags, CtConst::CHAR_SPACE);
    for (auto& tag: tagVec)
    {
        tag = str::trim(tag);
        if (!tag.empty())
            _usedTags.insert(tag);
    }
}

bool CtTreeStore::is_node_bookmarked(const gint64& node_id)
{
    return set::exists(_bookmarks, node_id);
}

std::string CtTreeStore::get_node_name_from_node_id(const gint64& node_id)
{
    return _nodes_names_dict.at(node_id);
}

CtTreeIter CtTreeStore::get_tree_iter_from_node_id(const gint64& node_id)
{
    Gtk::TreeIter find_iter;
    _rTreeStore->foreach_iter([&node_id, &find_iter, this](const Gtk::TreeIter& iter) {
        if (iter->get_value(_columns.colNodeUniqueId) != node_id) return false; /* continue */
        find_iter = iter;
        return true;
    });
    return to_ct_tree_iter(find_iter);
}

const std::list<gint64>& CtTreeStore::get_bookmarks()
{
    return _bookmarks_order;
}

void CtTreeStore::set_bookmarks(const std::list<gint64>& bookmarks_order)
{
    _bookmarks_order = bookmarks_order;
    _bookmarks.clear();
    std::copy(_bookmarks_order.begin(), _bookmarks_order.end(), std::inserter(_bookmarks, _bookmarks.begin()));
}


std::string CtTreeStore::get_tree_expanded_collapsed_string(Gtk::TreeView& treeView)
{
    std::vector<std::string> expanded_collapsed_vec;
    _rTreeStore->foreach([this, &treeView, &expanded_collapsed_vec](const Gtk::TreePath& path, const Gtk::TreeIter& iter)->bool{
        expanded_collapsed_vec.push_back(std::to_string(iter->get_value(_columns.colNodeUniqueId))
                                         + ","
                                         + (treeView.row_expanded(path) ? "True" : "False"));
        return false; /* false for continue */
    });
    return str::join(expanded_collapsed_vec, "_");
}

void CtTreeStore::set_tree_expanded_collapsed_string(const std::string& expanded_collapsed_string, Gtk::TreeView& treeView, bool nodes_bookm_exp)
{
    std::map<gint64, bool> expanded_collapsed_dict;
    auto expand_collapsed_vec = str::split(expanded_collapsed_string, "_");
    for (const std::string& element: expand_collapsed_vec) {
        auto couple = str::split(element, ",");
        if (couple.size() == 2)
            expanded_collapsed_dict[std::stoll(couple[0])] = CtStrUtil::isStrTrue(couple[1]);
    }

    treeView.collapse_all();
    _rTreeStore->foreach([this, &treeView, &expanded_collapsed_dict, &nodes_bookm_exp](const Gtk::TreePath& path, const Gtk::TreeIter& iter)->bool{
        gint64 node_id = iter->get_value(_columns.colNodeUniqueId);
        if (map::exists(expanded_collapsed_dict, node_id) && expanded_collapsed_dict.at(node_id))
            treeView.expand_row(path, false);
        else if (nodes_bookm_exp && set::exists(_bookmarks, node_id) && iter->parent())
            treeView.expand_to_path(_rTreeStore->get_path(iter->parent()));
        return false; /* false for continue */
    });
}


Glib::RefPtr<Gtk::TreeStore> CtTreeStore::get_store()
{
    return _rTreeStore;
}

Gtk::TreeIter CtTreeStore::get_iter_first()
{
    return _rTreeStore->get_iter("0");
}

Gtk::TreeIter CtTreeStore::get_tree_iter_last_sibling(const Gtk::TreeNodeChildren& children)
{
    if (children.empty()) return Gtk::TreeIter();
    return --children.end();
}

Gtk::TreeIter CtTreeStore::get_tree_iter_prev_sibling(Gtk::TreeIter tree_iter)
{
    return --tree_iter;
}

Gtk::TreePath CtTreeStore::get_path(Gtk::TreeIter tree_iter)
{
    return _rTreeStore->get_path(tree_iter);
}

CtTreeIter CtTreeStore::to_ct_tree_iter(Gtk::TreeIter tree_iter)
{
    return CtTreeIter(tree_iter, &get_columns());
}
