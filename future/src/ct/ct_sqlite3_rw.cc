/*
 * ct_sqlite3_rw.cc
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

#include <iostream>
#include "ct_doc_rw.h"
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_codebox.h"
#include "ct_image.h"
#include "ct_table.h"

const char CtSQLite::TABLE_NODE_CREATE[]{"CREATE TABLE node ("
"node_id INTEGER UNIQUE,"
"name TEXT,"
"txt TEXT,"
"syntax TEXT,"
"tags TEXT,"
"is_ro INTEGER,"
"is_richtxt INTEGER,"
"has_codebox INTEGER,"
"has_table INTEGER,"
"has_image INTEGER,"
"level INTEGER,"
"ts_creation INTEGER,"
"ts_lastsave INTEGER"
")"
};
const char CtSQLite::TABLE_NODE_INSERT[]{"INSERT INTO node VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)"};
const char CtSQLite::TABLE_NODE_DELETE[]{"DELETE FROM node WHERE node_id=?"};

const char CtSQLite::TABLE_CODEBOX_CREATE[]{"CREATE TABLE codebox ("
"node_id INTEGER,"
"offset INTEGER,"
"justification TEXT,"
"txt TEXT,"
"syntax TEXT,"
"width INTEGER,"
"height INTEGER,"
"is_width_pix INTEGER,"
"do_highl_bra INTEGER,"
"do_show_linenum INTEGER"
")"
};
const char CtSQLite::TABLE_CODEBOX_INSERT[]{"INSERT INTO codebox VALUES(?,?,?,?,?,?,?,?,?,?)"};
const char CtSQLite::TABLE_CODEBOX_DELETE[]{"DELETE FROM codebox WHERE node_id=?"};

const char CtSQLite::TABLE_TABLE_CREATE[]{"CREATE TABLE grid ("
"node_id INTEGER,"
"offset INTEGER,"
"justification TEXT,"
"txt TEXT,"
"col_min INTEGER,"
"col_max INTEGER"
")"
};
const char CtSQLite::TABLE_TABLE_INSERT[]{"INSERT INTO grid VALUES(?,?,?,?,?,?)"};
const char CtSQLite::TABLE_TABLE_DELETE[]{"DELETE FROM grid WHERE node_id=?"};

const char CtSQLite::TABLE_IMAGE_CREATE[]{"CREATE TABLE image ("
"node_id INTEGER,"
"offset INTEGER,"
"justification TEXT,"
"anchor TEXT,"
"png BLOB,"
"filename TEXT,"
"link TEXT,"
"time INTEGER"
")"
};
const char CtSQLite::TABLE_IMAGE_INSERT[]{"INSERT INTO image VALUES(?,?,?,?,?,?,?,?)"};
const char CtSQLite::TABLE_IMAGE_DELETE[]{"DELETE FROM image WHERE node_id=?"};

const char CtSQLite::TABLE_CHILDREN_CREATE[]{"CREATE TABLE children ("
"node_id INTEGER UNIQUE,"
"father_id INTEGER,"
"sequence INTEGER"
")"
};
const char CtSQLite::TABLE_CHILDREN_INSERT[]{"INSERT INTO children VALUES(?,?,?)"};
const char CtSQLite::TABLE_CHILDREN_DELETE[]{"DELETE FROM children WHERE node_id=?"};

const char CtSQLite::TABLE_BOOKMARK_CREATE[]{"CREATE TABLE bookmark ("
"node_id INTEGER UNIQUE,"
"sequence INTEGER"
")"
};
const char CtSQLite::TABLE_BOOKMARK_INSERT[]{"INSERT INTO bookmark VALUES(?,?)"};
const char CtSQLite::TABLE_BOOKMARK_DELETE[]{"DELETE FROM bookmark"};

const char CtSQLite::ERR_SQLITE_PREPV2[]{"!! sqlite3_prepare_v2: "};
const char CtSQLite::ERR_SQLITE_STEP[]{"!! sqlite3_step: "};

CtSQLite::CtSQLite(const char* filepath)
{
    const int ret_code = sqlite3_open(filepath, &_pDb);
    if (SQLITE_OK == ret_code)
    {
        _dbOpenOk = true;
    }
    else
    {
        std::cerr << "!! sqlite3_open: " << sqlite3_errmsg(_pDb) << std::endl;
    }
}

CtSQLite::~CtSQLite()
{
    if (_dbOpenOk)
    {
        sqlite3_close(_pDb);
        //printf("db closed\n");
    }
}

bool CtSQLite::_exec_no_callback(const char* sqlCmd)
{
    bool retVal{true};
    char *p_err_msg{nullptr};
    const int ret_code = sqlite3_exec(_pDb, sqlCmd, 0, 0, &p_err_msg);
    if (SQLITE_OK != ret_code)
    {
        std::cerr << "!! sqlite3 '" << sqlCmd << "': " << p_err_msg << std::endl;
        sqlite3_free(p_err_msg);
        retVal = false;
    }
    return retVal;
}

bool CtSQLite::_exec_bind_int64(const char* sqlCmd, const gint64 bind_int64)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, sqlCmd, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
        retVal = false;
    }
    else
    {
        sqlite3_bind_int64(p_stmt, 1, bind_int64);
        if (sqlite3_step(p_stmt) != SQLITE_DONE)
        {
            std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(_pDb) << std::endl;
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

bool CtSQLite::read_populate_tree(const Gtk::TreeIter* pParentIter)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT node_id FROM bookmark ORDER BY sequence ASC", -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
        retVal = false;
    }
    else
    {
        while (sqlite3_step(p_stmt) == SQLITE_ROW)
        {
            gint64 nodeId = sqlite3_column_int64(p_stmt, 0);
            signalAddBookmark.emit(nodeId);
        }
        sqlite3_finalize(p_stmt);

        std::list<gint64> top_nodes_ids;
        retVal = _get_children_node_ids_from_father_id(0, top_nodes_ids);
        if (retVal)
        {
            for (gint64 &top_node_id : top_nodes_ids)
            {
                if (!_sqlite3TreeWalkIter(top_node_id, pParentIter))
                {
                    retVal = false;
                    break;
                }
            }
        }
    }
    return retVal;
}

Glib::RefPtr<Gsv::Buffer> CtSQLite::getTextBuffer(const std::string& syntax,
                                                  std::list<CtAnchoredWidget*>& anchoredWidgets,
                                                  const gint64& nodeId) const
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};

    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT txt, has_codebox, has_table, has_image FROM node WHERE node_id=?", -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
    }
    else
    {
        bool has_codebox{false};
        bool has_table{false};
        bool has_image{false};

        sqlite3_bind_int64(p_stmt, 1, nodeId);
        if (sqlite3_step(p_stmt) == SQLITE_ROW)
        {
            const char* textContent = reinterpret_cast<const char*>(sqlite3_column_text(p_stmt, 0));
            if (CtConst::RICH_TEXT_ID != syntax)
            {
                rRetTextBuffer = CtMiscUtil::get_new_text_buffer(syntax, textContent);
            }
            else
            {
                CtXmlRead ctXmlRead(nullptr, textContent);
                if (nullptr != ctXmlRead.get_document())
                {
                    rRetTextBuffer = ctXmlRead.getTextBuffer(syntax, anchoredWidgets);
                    if (rRetTextBuffer)
                    {
                        has_codebox = sqlite3_column_int64(p_stmt, 1);
                        has_table = sqlite3_column_int64(p_stmt, 2);
                        has_image = sqlite3_column_int64(p_stmt, 3);
                    }
                }
                else
                {
                    std::cerr << "!! xml read: " << textContent << std::endl;
                }
            }
        }
        else
        {
            std::cerr << "!! missing node properties for id " << nodeId << std::endl;
        }
        sqlite3_finalize(p_stmt);

        if (has_codebox || has_table || has_image)
        {
            _get_text_buffer_anchored_widgets(rRetTextBuffer, anchoredWidgets, nodeId, has_codebox, has_table, has_image);
        }
    }

    return rRetTextBuffer;
}

void CtSQLite::_get_text_buffer_anchored_widgets(Glib::RefPtr<Gsv::Buffer>& rTextBuffer,
                                                 std::list<CtAnchoredWidget*>& anchoredWidgets,
                                                 const gint64& nodeId,
                                                 const bool& has_codebox,
                                                 const bool& has_table,
                                                 const bool& has_image) const
{
    const bool has_it[3]{has_codebox, has_table, has_image};
    sqlite3_stmt *pp_stmt[3]{nullptr, nullptr, nullptr};
    const int cOffsetNone{-1};
    const int cOffsetRead{-2};
    int charOffset[3]{cOffsetNone, cOffsetNone, cOffsetNone};
    Glib::ustring justification[3];
    const char* table_name[3]{"codebox", "grid", "image"};

    for (int i=0; i<3; i++)
    {
        if (has_it[i])
        {
            char query_buff[64];
            snprintf(query_buff, 64, "SELECT * FROM %s WHERE node_id=? ORDER BY offset ASC", table_name[i]);
            //std::cout << query_buff << std::endl;
            if (SQLITE_OK != sqlite3_prepare_v2(_pDb, query_buff, -1, &pp_stmt[i], nullptr))
            {
                std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
            }
            else
            {
                sqlite3_bind_int64(pp_stmt[i], 1, nodeId);
                charOffset[i] = cOffsetRead;
            }
        }
    }

    do
    {
        for (int i=0; i<3; i++)
        {
            if (cOffsetRead == charOffset[i])
            {
                if (SQLITE_ROW == sqlite3_step(pp_stmt[i]))
                {
                    charOffset[i] = sqlite3_column_int64(pp_stmt[i], 1);
                    justification[i] = reinterpret_cast<const char*>(sqlite3_column_text(pp_stmt[i], 2));
                    if (justification[i].empty())
                    {
                        justification[i] = CtConst::TAG_PROP_VAL_LEFT;
                    }
                }
                else
                {
                    charOffset[i] = cOffsetNone;
                }
            }
        }
        CtAnchoredWidget* pAnchoredWidget{nullptr};
        if (charOffset[0] >= 0 &&
            (charOffset[1] < 0 || charOffset[1] >= charOffset[0]) &&
            (charOffset[2] < 0 || charOffset[2] >= charOffset[0]))
        {
            // codebox
            const int i{0};
            const Glib::ustring textContent = reinterpret_cast<const char*>(sqlite3_column_text(pp_stmt[i], 3));
            const Glib::ustring syntaxHighlighting = reinterpret_cast<const char*>(sqlite3_column_text(pp_stmt[i], 4));
            const int frameWidth = sqlite3_column_int64(pp_stmt[i], 5);
            const int frameHeight = sqlite3_column_int64(pp_stmt[i], 6);
            const bool widthInPixels = sqlite3_column_int64(pp_stmt[i], 7);
            const bool highlightBrackets = sqlite3_column_int64(pp_stmt[i], 8);
            const bool showLineNumbers = sqlite3_column_int64(pp_stmt[i], 9);

            CtCodebox* pCtCodebox = new CtCodebox(textContent,
                                                  syntaxHighlighting,
                                                  frameWidth,
                                                  frameHeight,
                                                  charOffset[i],
                                                  justification[i]);
            pCtCodebox->setWidthInPixels(widthInPixels);
            pCtCodebox->setHighlightBrackets(highlightBrackets);
            pCtCodebox->setShowLineNumbers(showLineNumbers);
            pAnchoredWidget = pCtCodebox;
            //std::cout << "codebox " << charOffset[i] << std::endl;
            charOffset[i] = cOffsetRead;
        }
        else if (charOffset[1] >= 0 &&
                 (charOffset[0] < 0 || charOffset[0] >= charOffset[1]) &&
                 (charOffset[2] < 0 || charOffset[2] >= charOffset[1]))
        {
            // table
            const int i{1};
            const char* textContent = reinterpret_cast<const char*>(sqlite3_column_text(pp_stmt[i], 3));
            const int colMin = sqlite3_column_int64(pp_stmt[i], 4);
            const int colMax = sqlite3_column_int64(pp_stmt[i], 5);
            CtXmlRead ctXmlRead(nullptr, textContent);
            CtTableMatrix tableMatrix;
            if (nullptr != ctXmlRead.get_document())
            {
                const bool isHeadFront = ctXmlRead.populateTableMatrixGetIsHeadFront(tableMatrix, ctXmlRead.get_document()->get_root_node());

                pAnchoredWidget = new CtTable(tableMatrix, colMin, colMax, isHeadFront, charOffset[i], justification[i]);
                //std::cout << "table " << charOffset[i] << std::endl;
                charOffset[i] = cOffsetRead;
            }
            else
            {
                std::cerr << "!! table xml read: " << textContent << std::endl;
            }
        }
        else if (charOffset[2] >= 0 &&
                 (charOffset[0] < 0 || charOffset[0] >= charOffset[2]) &&
                 (charOffset[1] < 0 || charOffset[1] >= charOffset[2]))
        {
            // image
            const int i{2};
            const Glib::ustring anchorName = reinterpret_cast<const char*>(sqlite3_column_text(pp_stmt[i], 3));
            if (!anchorName.empty())
            {
                pAnchoredWidget = new CtImageAnchor(anchorName, charOffset[i], justification[i]);
            }
            else
            {
                const Glib::ustring fileName = reinterpret_cast<const char*>(sqlite3_column_text(pp_stmt[i], 5));
                const void* pBlob = sqlite3_column_blob(pp_stmt[i], 4);
                const int blobSize = sqlite3_column_bytes(pp_stmt[i], 4);
                const std::string rawBlob(reinterpret_cast<const char*>(pBlob), static_cast<size_t>(blobSize));
                if (!fileName.empty())
                {
                    const double timeDouble = sqlite3_column_int64(pp_stmt[i], 7);
                    pAnchoredWidget = new CtImageEmbFile(fileName, rawBlob, timeDouble, charOffset[i], justification[i]);
                }
                else
                {
                    const Glib::ustring link = reinterpret_cast<const char*>(sqlite3_column_text(pp_stmt[i], 6));
                    pAnchoredWidget = new CtImagePng(rawBlob, link, charOffset[i], justification[i]);
                }
            }
            //std::cout << "image " << charOffset[i] << std::endl;
            charOffset[i] = cOffsetRead;
        }
        if (nullptr != pAnchoredWidget)
        {
            rTextBuffer->begin_not_undoable_action();
            pAnchoredWidget->insertInTextBuffer(rTextBuffer);
            rTextBuffer->end_not_undoable_action();
            rTextBuffer->set_modified(false);
            anchoredWidgets.push_back(pAnchoredWidget);
        }
    }
    while (cOffsetNone != charOffset[0] ||
           cOffsetNone != charOffset[1] ||
           cOffsetNone != charOffset[2]);

    for (int i=0; i<3; i++)
    {
        if (has_it[i] && nullptr != pp_stmt[i])
        {
            sqlite3_finalize(pp_stmt[i]);
        }
    }
}

bool CtSQLite::_sqlite3TreeWalkIter(gint64 nodeId, const Gtk::TreeIter* pParentIter)
{
    Gtk::TreeIter newIter;
    bool retVal = _sqlite3NodeProcess(nodeId, pParentIter, newIter);
    if (retVal)
    {
        std::list<gint64> children_nodes_ids;
        retVal = _get_children_node_ids_from_father_id(nodeId, children_nodes_ids);
        if (retVal)
        {
            for (gint64 &child_node_id : children_nodes_ids)
            {
                if (!_sqlite3TreeWalkIter(child_node_id, &newIter))
                {
                    retVal = false;
                    break;
                }
            }
        }
    }
    return retVal;
}

bool CtSQLite::_get_children_node_ids_from_father_id(gint64 father_id, std::list<gint64>& ret_children)
{
    bool retVal{false};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT node_id FROM children WHERE father_id=? ORDER BY sequence ASC", -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
    }
    else
    {
        sqlite3_bind_int64(p_stmt, 1, father_id);
        while (sqlite3_step(p_stmt) == SQLITE_ROW)
        {
            gint64 nodeId = sqlite3_column_int64(p_stmt, 0);
            ret_children.push_back(nodeId);
        }
        sqlite3_finalize(p_stmt);
        retVal = true;
    }
    return retVal;
}

bool CtSQLite::_sqlite3GetNodeProperties(gint64 nodeId, CtNodeData& nodeData)
{
    bool retVal{false};
    nodeData.nodeId = nodeId;
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT name, syntax, tags, is_ro, is_richtxt, ts_creation, ts_lastsave FROM node WHERE node_id=?", -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
    }
    else
    {
        sqlite3_bind_int64(p_stmt, 1, nodeId);
        if (sqlite3_step(p_stmt) == SQLITE_ROW)
        {
            nodeData.name = reinterpret_cast<const char*>(sqlite3_column_text(p_stmt, 0));
            nodeData.syntax = reinterpret_cast<const char*>(sqlite3_column_text(p_stmt, 1));
            nodeData.tags = reinterpret_cast<const char*>(sqlite3_column_text(p_stmt, 2));
            gint64 readonly_n_custom_icon_id = sqlite3_column_int64(p_stmt, 3);
            nodeData.isRO = static_cast<bool>(readonly_n_custom_icon_id & 0x01);
            nodeData.customIconId = readonly_n_custom_icon_id >> 1;
            gint64 richtxt_bold_foreground = sqlite3_column_int64(p_stmt, 4);
            nodeData.isBold = static_cast<bool>((richtxt_bold_foreground >> 1) & 0x01);
            if (static_cast<bool>((richtxt_bold_foreground >> 2) & 0x01))
            {
                char foregroundRgb24[8];
                CtRgbUtil::setRgb24StrFromRgb24Int((richtxt_bold_foreground >> 3) & 0xffffff, foregroundRgb24);
                nodeData.foregroundRgb24 = foregroundRgb24;
            }
            nodeData.tsCreation = sqlite3_column_int64(p_stmt, 5);
            nodeData.tsLastSave = sqlite3_column_int64(p_stmt, 6);
            retVal = true;
        }
        else
        {
            std::cerr << "!! missing node properties for id " << nodeId << std::endl;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

bool CtSQLite::_sqlite3NodeProcess(gint64 nodeId, const Gtk::TreeIter* pParentIter, Gtk::TreeIter& newIter)
{
    CtNodeData nodeData;
    bool retVal = _sqlite3GetNodeProperties(nodeId, nodeData);
    if (retVal)
    {
        newIter = signalAppendNode.emit(&nodeData, pParentIter);
    }
    return retVal;
}

bool CtSQLite::_create_all_tables()
{
    bool retVal{false};
    if ( _exec_no_callback(TABLE_NODE_CREATE) &&
         _exec_no_callback(TABLE_CODEBOX_CREATE) &&
         _exec_no_callback(TABLE_TABLE_CREATE) &&
         _exec_no_callback(TABLE_IMAGE_CREATE) &&
         _exec_no_callback(TABLE_CHILDREN_CREATE) &&
         _exec_no_callback(TABLE_BOOKMARK_CREATE) )
    {
        retVal = true;
    }
    return retVal;
}

bool CtSQLite::_write_db_bookmarks(const std::list<gint64>& bookmarks)
{
    bool soFarSoGood = _exec_no_callback(CtSQLite::TABLE_BOOKMARK_DELETE);
    if (soFarSoGood)
    {
        gint64 sequence{0};
        for (gint64 bookmark : bookmarks)
        {
            sequence++;
            sqlite3_stmt *p_stmt;
            if (sqlite3_prepare_v2(_pDb, CtSQLite::TABLE_BOOKMARK_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
            {
                std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
                soFarSoGood = false;
            }
            else
            {
                sqlite3_bind_int64(p_stmt, 1, bookmark);
                sqlite3_bind_int64(p_stmt, 2, sequence);
                if (sqlite3_step(p_stmt) != SQLITE_DONE)
                {
                    std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(_pDb) << std::endl;
                    soFarSoGood = false;
                }
                sqlite3_finalize(p_stmt);
            }
        }
    }
    return soFarSoGood;
}

bool CtSQLite::write_db_full(const std::list<gint64>& bookmarks,
                             CtTreeIter ct_tree_iter,
                             const CtExporting exporting,
                             const std::pair<int,int>& offset_range)
{
    gint64 sequence{0};
    const gint64 node_father_id{0};
    CtNodeWriteDict write_dict;
    write_dict.prop = true;
    write_dict.buff = true;
    write_dict.hier = true;
    write_dict.child = (CtExporting::NodeOnly != exporting);
    bool soFarSoGood = _create_all_tables();
    while (soFarSoGood && ct_tree_iter)
    {
        sequence++;
        soFarSoGood = _write_db_node(ct_tree_iter,
                                     sequence,
                                     node_father_id,
                                     write_dict,
                                     exporting,
                                     offset_range);
        if (CtExporting::No == exporting || CtExporting::All == exporting)
        {
            ct_tree_iter++;
        }
        else
        {
            break;
        }
    }
    if (soFarSoGood && (CtExporting::No == exporting || CtExporting::All == exporting))
    {
        soFarSoGood = _write_db_bookmarks(bookmarks);
    }
    if (soFarSoGood && (CtExporting::No == exporting))
    {
        _syncPending.bookmarks_to_write = false;
        _syncPending.nodes_to_rm_set.clear();
        _syncPending.nodes_to_write_dict.clear();
    }
    return soFarSoGood;
}

void CtSQLite::pending_edit_db_bookmarks()
{
    _syncPending.bookmarks_to_write = true;
}

void CtSQLite::pending_edit_db_node_prop(const gint64 node_id)
{
    if (0 != _syncPending.nodes_to_write_dict.count(node_id))
    {
        _syncPending.nodes_to_write_dict[node_id].prop = true;
    }
    else
    {
        CtNodeWriteDict write_dict;
        write_dict.upd = true;
        write_dict.prop = true;
        _syncPending.nodes_to_write_dict[node_id] = write_dict;
    }
}

void CtSQLite::pending_edit_db_node_buff(const gint64 node_id)
{
    if (0 != _syncPending.nodes_to_write_dict.count(node_id))
    {
        _syncPending.nodes_to_write_dict[node_id].buff = true;
    }
    else
    {
        CtNodeWriteDict write_dict;
        write_dict.upd = true;
        write_dict.buff = true;
        _syncPending.nodes_to_write_dict[node_id] = write_dict;
    }
}

void CtSQLite::pending_edit_db_node_hier(const gint64 node_id)
{
    if (0 != _syncPending.nodes_to_write_dict.count(node_id))
    {
        _syncPending.nodes_to_write_dict[node_id].hier = true;
    }
    else
    {
        CtNodeWriteDict write_dict;
        write_dict.upd = true;
        write_dict.hier = true;
        _syncPending.nodes_to_write_dict[node_id] = write_dict;
    }
}

void CtSQLite::pending_new_db_node(const gint64 node_id)
{
    CtNodeWriteDict write_dict;
    write_dict.prop = true;
    write_dict.buff = true;
    write_dict.hier = true;
    _syncPending.nodes_to_write_dict[node_id] = write_dict;
}

void CtSQLite::pending_rm_db_nodes(const std::vector<gint64>& node_ids)
{
    for (const gint64 node_id : node_ids)
    {
        if (0 != _syncPending.nodes_to_write_dict.count(node_id))
        {
            // no need to write changes to a node that got to be removed
            _syncPending.nodes_to_write_dict.erase(node_id);
        }
        _syncPending.nodes_to_rm_set.insert(node_id);
    }
}

bool CtSQLite::pending_data_write(CtTreeStore* pTreeStore,
                                  const std::list<gint64>& bookmarks,
                                  const bool run_vacuum)
{
    bool allGood{true};
    if (_syncPending.bookmarks_to_write)
    {
        if (false == _write_db_bookmarks(bookmarks))
        {
            allGood = false;
        }
    }
    if (allGood)
    {
        _syncPending.bookmarks_to_write = false;
        for (const auto& node_pair : _syncPending.nodes_to_write_dict)
        {
            CtTreeIter ct_tree_iter = pTreeStore->get_node_from_node_id(node_pair.first);
            CtTreeIter ct_tree_iter_parent = ct_tree_iter.parent();
            if (false == _write_db_node(ct_tree_iter,
                           ct_tree_iter.get_node_sequence(),
                           ct_tree_iter_parent ? ct_tree_iter_parent.get_node_id() : 0,
                           node_pair.second))
            {
                allGood = false;
                break;
            }
        }
    }
    if (allGood)
    {
        _syncPending.nodes_to_write_dict.clear();
        for (const auto node_id : _syncPending.nodes_to_rm_set)
        {
            if (false == _remove_db_node_n_children(node_id))
            {
                allGood = false;
                break;
            }
        }
    }
    if (allGood)
    {
        _syncPending.nodes_to_rm_set.clear();
        if (run_vacuum)
        {
            (void)_exec_no_callback("VACUUM");
            (void)_exec_no_callback("REINDEX");
        }
    }
    return allGood;
}

bool CtSQLite::_remove_db_node_n_children(const gint64 node_id)
{
    bool soFarSoGood = ( _exec_bind_int64(CtSQLite::TABLE_CODEBOX_DELETE, node_id) &&
                         _exec_bind_int64(CtSQLite::TABLE_TABLE_DELETE, node_id) &&
                         _exec_bind_int64(CtSQLite::TABLE_IMAGE_DELETE, node_id) &&
                         _exec_bind_int64(CtSQLite::TABLE_NODE_DELETE, node_id) &&
                         _exec_bind_int64(CtSQLite::TABLE_CHILDREN_DELETE, node_id) );
    if (soFarSoGood)
    {
        std::list<gint64> children_node_ids;
        soFarSoGood = _get_children_node_ids_from_father_id(node_id, children_node_ids);
        if (soFarSoGood)
        {
            for (const gint64 child_node_id : children_node_ids)
            {
                soFarSoGood = _remove_db_node_n_children(child_node_id);
                if (!soFarSoGood)
                {
                    break;
                }
            }
        }
    }
    return soFarSoGood;
}

bool CtSQLite::_write_db_node(CtTreeIter ct_tree_iter,
                              const gint64 sequence,
                              const gint64 node_father_id,
                              const CtNodeWriteDict write_dict,
                              const CtExporting exporting,
                              const std::pair<int,int>& offset_range)
{
    bool soFarSoGood{true};
    const gint64 node_id = ct_tree_iter.get_node_id();
    // is_ro is packed with additional bitfield data
    gint64 is_ro = ct_tree_iter.get_node_read_only() ? 0x01 : 0x00;
    is_ro |= ct_tree_iter.get_node_custom_icon_id() << 1;
    // is_richtxt is packed with additional bitfield data
    gint64 is_richtxt = ct_tree_iter.get_node_is_rich_text() ? 0x01 : 0x00;
    if (ct_tree_iter.get_node_is_bold())
    {
        is_richtxt |= 0x02;
    }
    if (!ct_tree_iter.get_node_foreground().empty())
    {
        is_richtxt |= 0x04;
        is_richtxt |= CtRgbUtil::getRgb24IntFromStrAny(ct_tree_iter.get_node_foreground().c_str()+1) << 3;
    }
    bool has_codebox{false};
    bool has_table{false};
    bool has_image{false};
    std::string node_txt;
    if (write_dict.buff)
    {
        // prepare node txt for later
        CtXmlWrite ctXmlWrite("node");
        ctXmlWrite.append_node_buffer(ct_tree_iter, ctXmlWrite.get_root_node(), false/*serialise_anchored_widgets*/, offset_range);
        if (is_richtxt & 0x01)
        {
            node_txt = Glib::locale_from_utf8(ctXmlWrite.write_to_string());
            // anchored widgets
            if (write_dict.upd)
            {
                soFarSoGood = ( _exec_bind_int64(CtSQLite::TABLE_CODEBOX_DELETE, node_id) &&
                                _exec_bind_int64(CtSQLite::TABLE_TABLE_DELETE, node_id) &&
                                _exec_bind_int64(CtSQLite::TABLE_IMAGE_DELETE, node_id) );
            }
            if (soFarSoGood)
            {
                for (CtAnchoredWidget* pAnchoredWidget : ct_tree_iter.get_embedded_pixbufs_tables_codeboxes(offset_range))
                {
                    soFarSoGood = pAnchoredWidget->to_sqlite(_pDb, node_id, offset_range.first >= 0 ? -offset_range.first : 0);
                    if (!soFarSoGood)
                    {
                        break;
                    }
                    switch (pAnchoredWidget->get_type())
                    {
                        case CtAnchWidgType::CodeBox: has_codebox = true; break;
                        case CtAnchWidgType::Table: has_table = true; break;
                        default: has_image = true;
                    }
                }
            }
        }
        else
        {
            auto matches = ctXmlWrite.get_root_node()->find("rich_text");
            if (1 == matches.size())
            {
                xmlpp::TextNode* pTextNode = static_cast<xmlpp::Element*>(matches[0])->get_child_text();
                if (pTextNode)
                {
                    node_txt = Glib::locale_from_utf8(pTextNode->get_content());
                }
            }
        }
    }
    if (soFarSoGood)
    {
        if (write_dict.prop && write_dict.buff)
        {
            // full node rewrite
            if (write_dict.upd)
            {
                soFarSoGood = _exec_bind_int64(CtSQLite::TABLE_NODE_DELETE, node_id);
            }
            if (soFarSoGood)
            {
                sqlite3_stmt *p_stmt;
                if (sqlite3_prepare_v2(_pDb, CtSQLite::TABLE_NODE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
                {
                    std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
                    soFarSoGood = false;
                }
                else
                {
                    const std::string node_name = Glib::locale_from_utf8(ct_tree_iter.get_node_name());
                    const std::string node_syntax = ct_tree_iter.get_node_syntax_highlighting();
                    const std::string node_tags = Glib::locale_from_utf8(ct_tree_iter.get_node_tags());
                    sqlite3_bind_int64(p_stmt, 1, node_id);
                    sqlite3_bind_text(p_stmt, 2, node_name.c_str(), node_name.size(), SQLITE_STATIC);
                    sqlite3_bind_text(p_stmt, 3, node_txt.c_str(), node_txt.size(), SQLITE_STATIC);
                    sqlite3_bind_text(p_stmt, 4, node_syntax.c_str(), node_syntax.size(), SQLITE_STATIC);
                    sqlite3_bind_text(p_stmt, 5, node_tags.c_str(), node_tags.size(), SQLITE_STATIC);
                    sqlite3_bind_int64(p_stmt, 6, is_ro);
                    sqlite3_bind_int64(p_stmt, 7, is_richtxt);
                    sqlite3_bind_int64(p_stmt, 8, has_codebox);
                    sqlite3_bind_int64(p_stmt, 9, has_table);
                    sqlite3_bind_int64(p_stmt, 10, has_image);
                    sqlite3_bind_int64(p_stmt, 11, 0); // todo: get rid of unused column 'level'
                    sqlite3_bind_int64(p_stmt, 12, ct_tree_iter.get_node_creating_time());
                    sqlite3_bind_int64(p_stmt, 13, ct_tree_iter.get_node_modification_time());
                    if (sqlite3_step(p_stmt) != SQLITE_DONE)
                    {
                        std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(_pDb) << std::endl;
                        soFarSoGood = false;
                    }
                    sqlite3_finalize(p_stmt);
                }
            }
        }
        else if (write_dict.buff)
        {
            // only node buff rewrite
            sqlite3_stmt *p_stmt;
            if (sqlite3_prepare_v2(_pDb, "UPDATE node SET txt=?, syntax=?, is_richtxt=?, has_codebox=?, has_table=?, has_image=?, ts_lastsave=? WHERE node_id=?", -1, &p_stmt, nullptr) != SQLITE_OK)
            {
                std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
                soFarSoGood = false;
            }
            else
            {
                const std::string node_syntax = ct_tree_iter.get_node_syntax_highlighting();
                sqlite3_bind_text(p_stmt, 1, node_txt.c_str(), node_txt.size(), SQLITE_STATIC);
                sqlite3_bind_text(p_stmt, 2, node_syntax.c_str(), node_syntax.size(), SQLITE_STATIC);
                sqlite3_bind_int64(p_stmt, 3, is_richtxt);
                sqlite3_bind_int64(p_stmt, 4, has_codebox);
                sqlite3_bind_int64(p_stmt, 5, has_table);
                sqlite3_bind_int64(p_stmt, 6, has_image);
                sqlite3_bind_int64(p_stmt, 7, ct_tree_iter.get_node_modification_time());
                sqlite3_bind_int64(p_stmt, 8, node_id);
                if (sqlite3_step(p_stmt) != SQLITE_DONE)
                {
                    std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(_pDb) << std::endl;
                    soFarSoGood = false;
                }
                sqlite3_finalize(p_stmt);
            }
        }
        else if (write_dict.prop)
        {
            // only node prop rewrite
            sqlite3_stmt *p_stmt;
            if (sqlite3_prepare_v2(_pDb, "UPDATE node SET name=?, syntax=?, tags=?, is_ro=?, is_richtxt=? WHERE node_id=?", -1, &p_stmt, nullptr) != SQLITE_OK)
            {
                std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
                soFarSoGood = false;
            }
            else
            {
                const std::string node_name = Glib::locale_from_utf8(ct_tree_iter.get_node_name());
                const std::string node_syntax = ct_tree_iter.get_node_syntax_highlighting();
                const std::string node_tags = Glib::locale_from_utf8(ct_tree_iter.get_node_tags());
                sqlite3_bind_text(p_stmt, 1, node_name.c_str(), node_name.size(), SQLITE_STATIC);
                sqlite3_bind_text(p_stmt, 2, node_syntax.c_str(), node_syntax.size(), SQLITE_STATIC);
                sqlite3_bind_text(p_stmt, 3, node_tags.c_str(), node_tags.size(), SQLITE_STATIC);
                sqlite3_bind_int64(p_stmt, 4, is_ro);
                sqlite3_bind_int64(p_stmt, 5, is_richtxt);
                sqlite3_bind_int64(p_stmt, 6, node_id);
                if (sqlite3_step(p_stmt) != SQLITE_DONE)
                {
                    std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(_pDb) << std::endl;
                    soFarSoGood = false;
                }
                sqlite3_finalize(p_stmt);
            }
        }
    }
    if (soFarSoGood && write_dict.hier)
    {
        if (write_dict.upd)
        {
            soFarSoGood = _exec_bind_int64(CtSQLite::TABLE_CHILDREN_DELETE, node_id);
        }
        if (soFarSoGood)
        {
            sqlite3_stmt *p_stmt;
            if (sqlite3_prepare_v2(_pDb, CtSQLite::TABLE_CHILDREN_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
            {
                std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(_pDb) << std::endl;
                soFarSoGood = false;
            }
            else
            {
                sqlite3_bind_int64(p_stmt, 1, node_id);
                sqlite3_bind_int64(p_stmt, 2, node_father_id);
                sqlite3_bind_int64(p_stmt, 3, sequence);
                if (sqlite3_step(p_stmt) != SQLITE_DONE)
                {
                    std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(_pDb) << std::endl;
                    soFarSoGood = false;
                }
                sqlite3_finalize(p_stmt);
            }
        }
    }
    if (soFarSoGood && write_dict.child)
    {
        CtTreeIter ct_tree_iter_child = ct_tree_iter.first_child();
        gint64 child_sequence{0};
        while (ct_tree_iter_child)
        {
            child_sequence++;
            _write_db_node(ct_tree_iter_child,
                           child_sequence,
                           node_id,
                           write_dict,
                           exporting,
                           offset_range);
            ct_tree_iter_child++;
        }
    }
    return soFarSoGood;
}
