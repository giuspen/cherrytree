/*
 * ct_treestore.cc
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

#include <algorithm>
#include "ct_treestore.h"
#include "ct_doc_rw.h"
#include "ct_misc_utils.h"
#include "ct_main_win.h"

CtTreeModelColumns::~CtTreeModelColumns()
{
}

CtTreeIter::CtTreeIter(Gtk::TreeIter iter, const CtTreeModelColumns* pColumns, CtSQLite* pCtSQLite)
 : Gtk::TreeIter(iter),
   _pColumns(pColumns),
   _pCtSQLite(pCtSQLite)
{
}

CtTreeIter CtTreeIter::parent() const
{
    return CtTreeIter((*this)->parent(), _pColumns, _pCtSQLite);
}

CtTreeIter CtTreeIter::first_child() const
{
    return CtTreeIter((*this)->children().begin(), _pColumns, _pCtSQLite);
}

bool CtTreeIter::get_node_read_only() const
{
    return (*this) and (*this)->get_value(_pColumns->colNodeRO);
}

void CtTreeIter::set_node_read_only(bool val)
{
    (*this)->set_value(_pColumns->colNodeRO, val);
}

gint64 CtTreeIter::get_node_id() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeUniqueId) : -1;
}

std::vector<gint64> CtTreeIter::get_children_node_ids() const
{
    std::vector<gint64> retVec;
    CtTreeIter iterChild = first_child();
    while (iterChild)
    {
        retVec.push_back(iterChild.get_node_id());
        std::vector<gint64> children_node_ids = iterChild.get_children_node_ids();
        if (not children_node_ids.empty())
        {
            vec::vector_extend(retVec, children_node_ids);
        }
        iterChild++;
    }
    return retVec;
}

gint64 CtTreeIter::get_node_sequence() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeSequence) : -1;
}

bool CtTreeIter::get_node_is_bold() const
{
    return (*this) and get_is_bold_from_pango_weight((*this)->get_value(_pColumns->colWeight));
}

guint16 CtTreeIter::get_node_custom_icon_id() const
{
    return (*this) ? (*this)->get_value(_pColumns->colCustomIconId) : 0;
}

Glib::ustring CtTreeIter::get_node_name() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeName) : "";
}

void CtTreeIter::set_node_name(const Glib::ustring& node_name)
{
    (*this)->set_value(_pColumns->colNodeName, node_name);
}

Glib::ustring CtTreeIter::get_node_tags() const
{
    return (*this) ? (*this)->get_value(_pColumns->colNodeTags) : "";
}

std::string CtTreeIter::get_node_foreground() const
{
    return (*this) ? (*this)->get_value(_pColumns->colForeground) : "";
}

std::string CtTreeIter::get_node_syntax_highlighting() const
{
    return (*this) ? (*this)->get_value(_pColumns->colSyntaxHighlighting) : "";
}

bool CtTreeIter::get_node_is_rich_text() const
{
    return get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID;
}

gint64 CtTreeIter::get_node_creating_time() const
{
    return (*this) ? (*this)->get_value(_pColumns->colTsCreation) : 0;
}

gint64 CtTreeIter::get_node_modification_time() const
{
    return (*this) ? (*this)->get_value(_pColumns->colTsLastSave) : 0;
}

void CtTreeIter::set_node_modification_time(const gint64 modification_time)
{
    if (*this) (*this)->set_value(_pColumns->colTsLastSave, modification_time);
}

void CtTreeIter::set_node_aux_icon(Glib::RefPtr<Gdk::Pixbuf> rPixbuf)
{
    (*this)->set_value(_pColumns->rColPixbufAux, rPixbuf);
}

void CtTreeIter::set_node_sequence(gint64 num)
{
    (*this)->set_value(_pColumns->colNodeSequence, num);
}

Glib::RefPtr<Gsv::Buffer> CtTreeIter::get_node_text_buffer() const
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};
    if (*this)
    {
        rRetTextBuffer = (*this)->get_value(_pColumns->rColTextBuffer);
        if (not rRetTextBuffer and nullptr != _pCtSQLite)
        {
            // SQLite text buffer not yet populated
            std::list<CtAnchoredWidget*> anchoredWidgetList;
            rRetTextBuffer = _pCtSQLite->get_text_buffer((*this)->get_value(_pColumns->colSyntaxHighlighting),
                                                         anchoredWidgetList,
                                                         (*this)->get_value(_pColumns->colNodeUniqueId));
            (*this)->set_value(_pColumns->colAnchoredWidgets, anchoredWidgetList);
            (*this)->set_value(_pColumns->rColTextBuffer, rRetTextBuffer);
        }
    }
    return rRetTextBuffer;
}

int CtTreeIter::get_pango_weight_from_is_bold(bool isBold)
{
    return isBold ? PANGO_WEIGHT_HEAVY : PANGO_WEIGHT_NORMAL;
}

bool CtTreeIter::get_is_bold_from_pango_weight(int pangoWeight)
{
    return pangoWeight == PANGO_WEIGHT_HEAVY;
}

std::list<CtAnchoredWidget*> CtTreeIter::get_all_embedded_widgets()
{
    return (*this) ? (*this)->get_value(_pColumns->colAnchoredWidgets) : std::list<CtAnchoredWidget*>();
}
void CtTreeIter::remove_all_embedded_widgets()
{
    if (*this)
    {
        for (auto widget: (*this)->get_value(_pColumns->colAnchoredWidgets))
            delete widget;
        (*this)->set_value(_pColumns->colAnchoredWidgets, std::list<CtAnchoredWidget*>());
    }
}

std::list<CtAnchoredWidget*> CtTreeIter::get_embedded_pixbufs_tables_codeboxes(const std::pair<int,int>& offset_range)
{
    std::list<CtAnchoredWidget*> retAnchoredWidgetsList;
    if ((*this) and (*this)->get_value(_pColumns->colAnchoredWidgets).size() > 0)
    {
        Glib::RefPtr<Gsv::Buffer> rTextBuffer = get_node_text_buffer();
        Gtk::TextIter curr_iter = offset_range.first >= 0 ? rTextBuffer->get_iter_at_offset(offset_range.first) : rTextBuffer->begin();
        do
        {
            if ((offset_range.second >= 0) and (curr_iter.get_offset() > offset_range.second))
            {
                break;
            }
            Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = curr_iter.get_child_anchor();
            if (rChildAnchor)
            {
                for (CtAnchoredWidget* pCtAnchoredWidget : (*this)->get_value(_pColumns->colAnchoredWidgets))
                {
                    if (rChildAnchor == pCtAnchoredWidget->getTextChildAnchor())
                    {
                        pCtAnchoredWidget->updateOffset(curr_iter.get_offset());
                        pCtAnchoredWidget->updateJustification(curr_iter);
                        retAnchoredWidgetsList.push_back(pCtAnchoredWidget);
                        break;
                    }
                }
            }
        }
        while (curr_iter.forward_char());
    }
    return retAnchoredWidgetsList;
}

void CtTreeIter::pending_edit_db_node_prop()
{
    if (nullptr != _pCtSQLite)
    {
        const gint64 node_id = get_node_id();
        _pCtSQLite->pending_edit_db_node_prop(node_id);
    }
}

void CtTreeIter::pending_edit_db_node_buff()
{
    if (nullptr != _pCtSQLite)
    {
        const gint64 node_id = get_node_id();
        _pCtSQLite->pending_edit_db_node_buff(node_id);
    }
}

void CtTreeIter::pending_edit_db_node_hier()
{
    if (nullptr != _pCtSQLite)
    {
        const gint64 node_id = get_node_id();
        _pCtSQLite->pending_edit_db_node_hier(node_id);
    }
}

void CtTreeIter::pending_new_db_node()
{
    if (nullptr != _pCtSQLite)
    {
        const gint64 node_id = get_node_id();
        _pCtSQLite->pending_new_db_node(node_id);
    }
}


CtTreeStore::CtTreeStore(CtMainWin* pCtMainWin)
 : _pCtMainWin(pCtMainWin)
{
    _rTreeStore = Gtk::TreeStore::create(_columns);
}

CtTreeStore::~CtTreeStore()
{
    _iter_delete_anchored_widgets(get_root_children());
    if (nullptr != _pCtSQLite)
    {
        delete _pCtSQLite;
    }
}

void CtTreeStore::pending_rm_db_nodes(const std::vector<gint64>& node_ids)
{
    if (nullptr != _pCtSQLite)
    {
        _pCtSQLite->pending_rm_db_nodes(node_ids);
    }
}

void CtTreeStore::pending_edit_db_bookmarks()
{
    if (nullptr != _pCtSQLite)
    {
        _pCtSQLite->pending_edit_db_bookmarks();
    }
}

bool CtTreeStore::pending_data_write(const bool run_vacuum)
{
    if (nullptr != _pCtSQLite)
    {
        return _pCtSQLite->pending_data_write(this, _bookmarks, run_vacuum);
    }
    return false;
}

void CtTreeStore::_iter_delete_anchored_widgets(const Gtk::TreeModel::Children& children)
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

        _iter_delete_anchored_widgets(row.children());
    }
}

void CtTreeStore::expand_to_tree_row(Gtk::TreeView* pTreeView, Gtk::TreeRow& row)
{
    Gtk::TreeIter iterParent = row.parent();
    if (static_cast<bool>(iterParent))
    {
        pTreeView->expand_to_path(_rTreeStore->get_path(iterParent));
    }
}

void CtTreeStore::treeview_safe_set_cursor(Gtk::TreeView* pTreeView, Gtk::TreeIter& treeIter)
{
    Gtk::TreeRow row = *treeIter;
    expand_to_tree_row(pTreeView, row);
    pTreeView->set_cursor(_rTreeStore->get_path(treeIter));
}

void CtTreeStore::set_tree_path_n_text_cursor(Gtk::TreeView* pTreeView,
                                              Gsv::View* pTextView,
                                              const std::string& node_path,
                                              const int cursor_pos)
{
    bool treeSelFromConfig{false};
    if (not node_path.empty())
    {
        Gtk::TreeIter treeIter = _rTreeStore->get_iter(node_path);
        if (static_cast<bool>(treeIter))
        {
            treeview_safe_set_cursor(pTreeView, treeIter);
            treeSelFromConfig = true;
            if (_pCtMainWin->get_ct_config()->treeClickExpand)
            {
                pTreeView->expand_row(_rTreeStore->get_path(treeIter), false/*open_all*/);
            }
            Glib::RefPtr<Gsv::Buffer> rTextBuffer = (*treeIter).get_value(_columns.rColTextBuffer);
            Gtk::TextIter textIter = rTextBuffer->get_iter_at_offset(cursor_pos);
            if (static_cast<bool>(textIter))
            {
                rTextBuffer->place_cursor(textIter);
                pTextView->scroll_to(textIter, CtTextView::TEXT_SCROLL_MARGIN);
            }
        }
    }
    if (not treeSelFromConfig)
    {
        const Gtk::TreeIter treeIter = get_iter_first();
        if (static_cast<bool>(treeIter))
        {
            pTreeView->set_cursor(_rTreeStore->get_path(treeIter));
        }
    }
}

void CtTreeStore::view_connect(Gtk::TreeView* pTreeView)
{
    pTreeView->set_model(_rTreeStore);
}

void CtTreeStore::view_append_columns(Gtk::TreeView* pTreeView)
{
    Gtk::TreeView::Column* pColumns = Gtk::manage(new Gtk::TreeView::Column(""));
    pColumns->pack_start(_columns.rColPixbuf, /*expand=*/false);
    pColumns->pack_start(_columns.colNodeName);
    pColumns->set_expand(true);
    pTreeView->append_column(*pColumns);
    pTreeView->append_column("", _columns.rColPixbufAux);
    Gtk::TreeViewColumn* pTVCol0 = pTreeView->get_column(0);
    std::vector<Gtk::CellRenderer*> cellRenderers0 = pTVCol0->get_cells();
    if (cellRenderers0.size() > 1)
    {
        Gtk::CellRendererText *pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(cellRenderers0[1]);
        if (nullptr != pCellRendererText)
        {
            pTVCol0->add_attribute(pCellRendererText->property_weight(), _columns.colWeight);
            pTVCol0->set_cell_data_func(*pCellRendererText,
                [this](Gtk::CellRenderer* pCell, const Gtk::TreeIter& treeIter)
                {
                    Gtk::TreeRow row = *treeIter;
                    if (row.get_value(_columns.colForeground).empty())
                    {
                        dynamic_cast<Gtk::CellRendererText*>(pCell)->property_foreground() = _pCtMainWin->get_ct_config()->ttDefFg;
                    }
                    else
                    {
                        dynamic_cast<Gtk::CellRendererText*>(pCell)->property_foreground() = row.get_value(_columns.colForeground);
                    }
                });
        }
    }
}

bool CtTreeStore::read_nodes_from_filepath(const char* filepath, const bool isImport, const Gtk::TreeIter* pParentIter)
{
    bool retOk{false};
    CtDocType docType = CtMiscUtil::get_doc_type(filepath);
    CtDocRead* pCtDocRead{nullptr};
    if (CtDocType::XML == docType)
    {
        CtXmlRead* pCtXmlRead = new CtXmlRead(_pCtMainWin, filepath, nullptr);
        if (pCtXmlRead and (nullptr != pCtXmlRead->get_document()))
        {
            pCtDocRead = pCtXmlRead;
        }
    }
    else if (CtDocType::SQLite == docType)
    {
        CtSQLite* pCtSQLite = new CtSQLite(_pCtMainWin, filepath);
        if (pCtSQLite and pCtSQLite->get_db_open_ok())
        {
            pCtDocRead = pCtSQLite;
        }
    }
    if (pCtDocRead != nullptr)
    {
        pCtDocRead->signalAddBookmark.connect(sigc::mem_fun(this, &CtTreeStore::onRequestAddBookmark));
        pCtDocRead->signalAppendNode.connect(sigc::mem_fun(this, &CtTreeStore::onRequestAppendNode));
        retOk = pCtDocRead->read_populate_tree(pParentIter);
        if (not isImport and (CtDocType::SQLite == docType))
        {
            _pCtSQLite = dynamic_cast<CtSQLite*>(pCtDocRead);
        }
        else
        {
            delete pCtDocRead;
        }
    }
    return retOk;
}

void CtTreeStore::set_new_curr_sqlite_doc(CtSQLite* const pCtSQLite)
{
    if (pCtSQLite != _pCtSQLite)
    {
        if (nullptr != _pCtSQLite)
        {
            delete _pCtSQLite;
        }
        _pCtSQLite = pCtSQLite;
    }
}

Glib::RefPtr<Gdk::Pixbuf> CtTreeStore::_get_node_icon(int nodeDepth, const std::string &syntax, guint32 customIconId)
{
    Glib::RefPtr<Gdk::Pixbuf> rPixbuf;

    if (0 != customIconId)
    {
        // customIconId
        rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODES_STOCKS.at((int)customIconId), CtConst::NODE_ICON_SIZE);
    }
    else if (CtConst::NODE_ICON_TYPE_NONE == _pCtMainWin->get_ct_config()->nodesIcons)
    {
        // NODE_ICON_TYPE_NONE
        rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODES_STOCKS.at(CtConst::NODE_ICON_NO_ICON_ID), CtConst::NODE_ICON_SIZE);
    }
    else if (CtStrUtil::is_pgchar_in_pgchar_iterable(syntax.c_str(), CtConst::TEXT_SYNTAXES))
    {
        // text node
        if (CtConst::NODE_ICON_TYPE_CHERRY == _pCtMainWin->get_ct_config()->nodesIcons)
        {
            if (1 == CtConst::NODES_ICONS.count(nodeDepth))
            {
                rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODES_ICONS.at(nodeDepth), CtConst::NODE_ICON_SIZE);
            }
            else
            {
                rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODES_ICONS.at(-1), CtConst::NODE_ICON_SIZE);
            }
        }
        else
        {
            // NODE_ICON_TYPE_CUSTOM
            rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::NODES_STOCKS.at(_pCtMainWin->get_ct_config()->defaultIconText), CtConst::NODE_ICON_SIZE);
        }
    }
    else
    {
        // code node
        rPixbuf = _pCtMainWin->get_icon_theme()->load_icon(CtConst::getStockIdForCodeType(syntax), CtConst::NODE_ICON_SIZE);
    }
    return rPixbuf;
}

void CtTreeStore::get_node_data(const Gtk::TreeIter& treeIter, CtNodeData& nodeData)
{
    Gtk::TreeRow row = *treeIter;

    nodeData.name =  row[_columns.colNodeName];
    nodeData.rTextBuffer = row[_columns.rColTextBuffer];
    nodeData.nodeId = row[_columns.colNodeUniqueId];
    nodeData.syntax = row[_columns.colSyntaxHighlighting];
    //row[_columns.colNodeSequence] = ;
    nodeData.tags = row[_columns.colNodeTags];
    nodeData.isRO = row[_columns.colNodeRO];
    //row[_columns.rColPixbufAux] = ;
    nodeData.customIconId = row[_columns.colCustomIconId];
    nodeData.isBold = CtTreeIter::get_is_bold_from_pango_weight(row[_columns.colWeight]);
    nodeData.foregroundRgb24 = row[_columns.colForeground];
    nodeData.tsCreation = row[_columns.colTsCreation];
    nodeData.tsLastSave = row[_columns.colTsLastSave];
    nodeData.anchoredWidgets = row[_columns.colAnchoredWidgets];
}

void CtTreeStore::update_node_data(const Gtk::TreeIter& treeIter, const CtNodeData& nodeData)
{
    Gtk::TreeRow row = *treeIter;
    row[_columns.rColPixbuf] = _get_node_icon(_rTreeStore->iter_depth(treeIter), nodeData.syntax, nodeData.customIconId);
    row[_columns.colNodeName] = nodeData.name;
    row[_columns.rColTextBuffer] = nodeData.rTextBuffer;
    row[_columns.colNodeUniqueId] = nodeData.nodeId;
    row[_columns.colSyntaxHighlighting] = nodeData.syntax;
    //row[_columns.colNodeSequence] = ;
    row[_columns.colNodeTags] = nodeData.tags;
    row[_columns.colNodeRO] = nodeData.isRO;
    //row[_columns.rColPixbufAux] = ;
    row[_columns.colCustomIconId] = (guint16)nodeData.customIconId;
    row[_columns.colWeight] = CtTreeIter::get_pango_weight_from_is_bold(nodeData.isBold);
    row[_columns.colForeground] = nodeData.foregroundRgb24;
    row[_columns.colTsCreation] = nodeData.tsCreation;
    row[_columns.colTsLastSave] = nodeData.tsLastSave;
    // todo: should widgets be deleted?
    row[_columns.colAnchoredWidgets] = nodeData.anchoredWidgets;

    update_node_aux_icon(treeIter);
    add_used_tags(nodeData.tags);
    _nodes_names_dict[nodeData.nodeId] = nodeData.name;
}

void CtTreeStore::update_node_icon(const Gtk::TreeIter& treeIter)
{
    auto icon = _get_node_icon(_rTreeStore->iter_depth(treeIter),
                               treeIter->get_value(_columns.colSyntaxHighlighting),
                               treeIter->get_value(_columns.colCustomIconId));
    treeIter->set_value(_columns.rColPixbuf, icon);
}


void CtTreeStore::update_node_aux_icon(const Gtk::TreeIter& treeIter)
{
    bool is_ro = treeIter->get_value(_columns.colNodeRO);
    bool is_bookmark = vec::exists(_bookmarks, treeIter->get_value(_columns.colNodeUniqueId));
    std::string stock_id;
    if (is_ro and is_bookmark) stock_id = "lockpin";
    else if (is_ro)           stock_id = "locked";
    else if (is_bookmark)     stock_id = "pin";

    if (stock_id.empty())
        treeIter->set_value(_columns.rColPixbufAux, Glib::RefPtr<Gdk::Pixbuf>());
    else
        treeIter->set_value(_columns.rColPixbufAux, _pCtMainWin->get_icon_theme()->load_icon(stock_id, CtConst::NODE_ICON_SIZE));
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
    update_node_data(newIter, *pNodeData);
    return newIter;
}

Gtk::TreeIter CtTreeStore::insertNode(CtNodeData* pNodeData, const Gtk::TreeIter& afterIter)
{
    Gtk::TreeIter newIter = _rTreeStore->insert_after(afterIter);
    update_node_data(newIter, *pNodeData);
    return newIter;
}

bool CtTreeStore::onRequestAddBookmark(gint64 nodeId)
{
    if (vec::exists(_bookmarks, nodeId))
    {
        return false;
    }
    _bookmarks.push_back(nodeId);
    return true;
}

bool CtTreeStore::onRequestRemoveBookmark(gint64 nodeId)
{
    if (not vec::exists(_bookmarks, nodeId))
    {
        return false;
    }
    vec::remove(_bookmarks, nodeId);
    return true;
}

Gtk::TreeIter CtTreeStore::onRequestAppendNode(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter)
{
    return appendNode(pNodeData, pParentIter);
}

void CtTreeStore::_on_textbuffer_modified_changed(Glib::RefPtr<Gtk::TextBuffer> rTextBuffer)
{
    if (_pCtMainWin->user_active() and rTextBuffer->get_modified())
    {
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
    }
}

void CtTreeStore::_on_textbuffer_insert(const Gtk::TextBuffer::iterator& pos, const Glib::ustring& text, int bytes)
{
    if (_pCtMainWin->user_active())
        _pCtMainWin->get_state_machine().text_variation(_pCtMainWin->curr_tree_iter().get_node_id(), text);
}

void CtTreeStore::_on_textbuffer_erase(const Gtk::TextBuffer::iterator& range_start, const Gtk::TextBuffer::iterator& range_end)
{
     if (_pCtMainWin->user_active())
       _pCtMainWin->get_state_machine().text_variation(_pCtMainWin->curr_tree_iter().get_node_id(), range_start.get_text(range_end));
}

void CtTreeStore::apply_textbuffer_to_textview(const CtTreeIter& treeIter, CtTextView* pTextView)
{
    if (not static_cast<bool>(treeIter))
    {
        pTextView->set_buffer(Glib::RefPtr<Gsv::Buffer>{});
        pTextView->set_sensitive(false);
        return;
    }

    // disconnect signals connected to prev node
    for (sigc::connection& sigc_conn : _curr_node_sigc_conn)
    {
        sigc_conn.disconnect();
    }
    _curr_node_sigc_conn.clear();

    CtTreeIter node = to_ct_tree_iter(treeIter);
    std::cout << node.get_node_name() << std::endl;

    Glib::RefPtr<Gsv::Buffer> rTextBuffer = node.get_node_text_buffer();
    pTextView->setup_for_syntax(node.get_node_syntax_highlighting());
    pTextView->set_buffer(rTextBuffer);
    pTextView->set_sensitive(true);
    pTextView->set_editable(not node.get_node_read_only());

    for (CtAnchoredWidget* pCtAnchoredWidget : node.get_all_embedded_widgets())
    {
        Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = pCtAnchoredWidget->getTextChildAnchor();
        if (rChildAnchor)
        {
            if (0 == rChildAnchor->get_widgets().size())
            {
                // Gtk::TextIter textIter = rTextBuffer->get_iter_at_child_anchor(rChildAnchor);
                pTextView->add_child_at_anchor(*pCtAnchoredWidget, rChildAnchor);
                pCtAnchoredWidget->apply_width_height(pTextView->get_allocation().get_width());
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
    // we shouldn't lose focus from TREE because TREE shortcuts/arrays movement stop working
    // pTextView->grab_focus();

    // connect signals
    _curr_node_sigc_conn.push_back(
        rTextBuffer->signal_modified_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TextBuffer>>(
            sigc::mem_fun(*this, &CtTreeStore::_on_textbuffer_modified_changed), rTextBuffer
        ))
    );
    _curr_node_sigc_conn.push_back(
        rTextBuffer->signal_insert().connect(sigc::mem_fun(*this, &CtTreeStore::_on_textbuffer_insert))
    );
    _curr_node_sigc_conn.push_back(
        rTextBuffer->signal_erase().connect(sigc::mem_fun(*this, &CtTreeStore::_on_textbuffer_erase))
    );
}

void CtTreeStore::addAnchoredWidgets(Gtk::TreeIter treeIter, std::list<CtAnchoredWidget*> anchoredWidgetList, Gtk::TextView* pTextView)
{
    auto widgets = treeIter->get_value(_columns.colAnchoredWidgets);
    for (auto new_widget: anchoredWidgetList)
        widgets.push_back(new_widget);
    treeIter->set_value(_columns.colAnchoredWidgets, widgets);

    for (CtAnchoredWidget* pCtAnchoredWidget : anchoredWidgetList)
    {
        Glib::RefPtr<Gtk::TextChildAnchor> rChildAnchor = pCtAnchoredWidget->getTextChildAnchor();
        if (rChildAnchor)
        {
            if (0 == rChildAnchor->get_widgets().size())
            {
                pTextView->add_child_at_anchor(*pCtAnchoredWidget, rChildAnchor);
                pCtAnchoredWidget->apply_width_height(pTextView->get_allocation().get_width());
            }
        }
    }
}

gint64 CtTreeStore::node_id_get(gint64 original_id, std::unordered_map<gint64,gint64> remapping_ids)
{
    // check if remapping was set
    if ((original_id > 0) and (1 == remapping_ids.count(original_id)))
    {
        return remapping_ids[original_id];
    }

    // prepare sets of ids not to be used
    std::set<gint64> allocated_for_remapping_ids;
    for (const auto& curr_pair : remapping_ids)
    {
        allocated_for_remapping_ids.insert(curr_pair.second);
    }
    std::set<gint64> nodes_pending_rm;
    if (nullptr != _pCtSQLite)
    {
        nodes_pending_rm = _pCtSQLite->get_nodes_pending_rm();
    }
    // (@txe) this function works differently from python code
    // it's easer to find max than check every id is not used through all tree
    gint64 max_node_id{0};
    _rTreeStore->foreach_iter([&max_node_id, this](const Gtk::TreeIter& iter)
    {
        if (iter->get_value(_columns.colNodeUniqueId) > max_node_id)
        {
            max_node_id = iter->get_value(_columns.colNodeUniqueId);
        }
        return false; /* continue */
    });
    for (const gint64 curr_id : allocated_for_remapping_ids)
    {
        if (curr_id > max_node_id)
        {
            max_node_id = curr_id;
        }
    }
    for (const gint64 curr_id : nodes_pending_rm)
    {
        if (curr_id > max_node_id)
        {
            max_node_id = curr_id;
        }
    }
    const gint64 new_node_id = max_node_id+1;

    // remapping set up
    if (original_id > 0)
    {
        remapping_ids[original_id] = new_node_id;
    }

    return new_node_id;
}

void CtTreeStore::add_used_tags(const Glib::ustring& tags)
{
    std::vector<Glib::ustring> tagVec = str::split(tags, CtConst::CHAR_SPACE.c_str());
    for (auto& tag : tagVec)
    {
        tag = str::trim(tag);
        if (not tag.empty())
        {
            _usedTags.insert(tag);
        }
    }
}

bool CtTreeStore::is_node_bookmarked(const gint64 node_id)
{
    return vec::exists(_bookmarks, node_id);
}

std::string CtTreeStore::get_node_name_from_node_id(const gint64 node_id)
{
    return _nodes_names_dict.at(node_id);
}

CtTreeIter CtTreeStore::get_node_from_node_id(const gint64 node_id)
{
    Gtk::TreeIter find_iter;
    _rTreeStore->foreach_iter([&node_id, &find_iter, this](const Gtk::TreeIter& iter) {
        if (iter->get_value(_columns.colNodeUniqueId) != node_id) return false; /* continue */
        find_iter = iter;
        return true;
    });
    return to_ct_tree_iter(find_iter);
}

CtTreeIter CtTreeStore::get_node_from_node_name(const Glib::ustring& node_name)
{
    Gtk::TreeIter find_iter;
    _rTreeStore->foreach_iter([&node_name, &find_iter, this](const Gtk::TreeIter& iter) {
        if (iter->get_value(_columns.colNodeName) != node_name) return false; /* continue */
        find_iter = iter;
        return true;
    });
    return to_ct_tree_iter(find_iter);

}
const std::list<gint64>& CtTreeStore::get_bookmarks()
{
    return _bookmarks;
}

void CtTreeStore::set_bookmarks(const std::list<gint64>& bookmarks)
{
    _bookmarks = bookmarks;
}

std::string CtTreeStore::get_tree_expanded_collapsed_string(Gtk::TreeView& treeView)
{
    std::vector<std::string> expanded_collapsed_vec;
    _rTreeStore->foreach(
        [this, &treeView, &expanded_collapsed_vec](const Gtk::TreePath& path, const Gtk::TreeIter& iter)->bool
        {
            expanded_collapsed_vec.push_back(std::to_string(iter->get_value(_columns.colNodeUniqueId))
                                             + ","
                                             + (treeView.row_expanded(path) ? "True" : "False"));
            return false; /* false for continue */
        }
    );
    return str::join(expanded_collapsed_vec, "_");
}

void CtTreeStore::set_tree_expanded_collapsed_string(const std::string& expanded_collapsed_string, Gtk::TreeView& treeView, bool nodes_bookm_exp)
{
    std::map<gint64, bool> expanded_collapsed_dict;
    auto expand_collapsed_vec = str::split(expanded_collapsed_string, "_");
    for (const std::string& element: expand_collapsed_vec)
    {
        auto couple = str::split(element, ",");
        if (couple.size() == 2)
        {
            expanded_collapsed_dict[std::stoll(couple[0])] = CtStrUtil::is_str_true(couple[1]);
        }
    }
    treeView.collapse_all();
    _rTreeStore->foreach(
        [this, &treeView, &expanded_collapsed_dict, &nodes_bookm_exp](const Gtk::TreePath& path, const Gtk::TreeIter& iter)->bool
        {
            gint64 node_id = iter->get_value(_columns.colNodeUniqueId);
            if (map::exists(expanded_collapsed_dict, node_id) and expanded_collapsed_dict.at(node_id))
            {
                treeView.expand_row(path, false);
            }
            else if (nodes_bookm_exp and vec::exists(_bookmarks, node_id) and iter->parent())
            {
                treeView.expand_to_path(_rTreeStore->get_path(iter->parent()));
            }
            return false; /* false for continue */
        }
    );
}

Glib::RefPtr<Gtk::TreeStore> CtTreeStore::get_store()
{
    return _rTreeStore;
}

Gtk::TreeIter CtTreeStore::get_iter_first()
{
    return _rTreeStore->get_iter("0");
}

CtTreeIter CtTreeStore::get_ct_iter_first()
{
    return to_ct_tree_iter(get_iter_first());
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
    return CtTreeIter(tree_iter, &get_columns(), _pCtSQLite);
}

void CtTreeStore::nodes_sequences_fix(Gtk::TreeIter father_iter,  bool process_children)
{
    auto children = father_iter ? father_iter->children() : _rTreeStore->children();
    gint64 node_sequence = 0;
    for (auto& child: children)
    {
        ++node_sequence;
        auto ct_child = to_ct_tree_iter(child);
        if (ct_child.get_node_sequence() != node_sequence)
        {
            ct_child.set_node_sequence(node_sequence);
            ct_child.pending_edit_db_node_hier();
        }
        if (process_children)
            nodes_sequences_fix(child, process_children);
    }
}

void CtTreeStore::refresh_node_icons(Gtk::TreeIter father_iter, bool cherry_only)
{
    if (cherry_only)
        if (CtConst::NODE_ICON_TYPE_CHERRY != _pCtMainWin->get_ct_config()->nodesIcons)
            return;
    if (father_iter)
        update_node_icon(father_iter);
    for (auto& child: father_iter ? father_iter->children() : _rTreeStore->children())
        refresh_node_icons(child, cherry_only);
}

