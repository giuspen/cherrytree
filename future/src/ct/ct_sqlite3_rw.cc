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

CtSQLiteRead::CtSQLiteRead(const char* filepath)
{
    int ret_code = sqlite3_open(filepath, &_pDb);
    if (ret_code != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_open: " << sqlite3_errmsg(_pDb) << std::endl;
        exit(EXIT_FAILURE);
    }
}

CtSQLiteRead::~CtSQLiteRead()
{
    sqlite3_close(_pDb);
}

void CtSQLiteRead::treeWalk(const Gtk::TreeIter* pParentIter)
{
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT node_id FROM bookmark ORDER BY sequence ASC", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(_pDb) << std::endl;
        exit(EXIT_FAILURE);
    }
    while (sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        gint64 nodeId = sqlite3_column_int64(p_stmt, 0);
        signalAddBookmark.emit(nodeId);
    }
    sqlite3_finalize(p_stmt);

    std::list<gint64> top_nodes_ids = _sqlite3GetChildrenNodeIdFromFatherId(0);
    for (gint64 &top_node_id : top_nodes_ids)
    {
        _sqlite3TreeWalkIter(top_node_id, pParentIter);
    }
}

Glib::RefPtr<Gsv::Buffer> CtSQLiteRead::getTextBuffer(const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& anchoredWidgets,
                                                      const gint64& nodeId)
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};

    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT txt, has_codebox, has_table, has_image FROM node WHERE node_id=?", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(_pDb) << std::endl;
        exit(EXIT_FAILURE);
    }

    bool has_codebox{false};
    bool has_table{false};
    bool has_image{false};

    sqlite3_bind_int(p_stmt, 1, nodeId);
    if (sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        const char* textContent = reinterpret_cast<const char*>(sqlite3_column_text(p_stmt, 0));
        if (CtConst::RICH_TEXT_ID != syntax)
        {
            rRetTextBuffer = CtMiscUtil::getNewTextBuffer(syntax, textContent);
        }
        else
        {
            CtXmlRead ctXmlRead(nullptr, textContent);
            rRetTextBuffer = ctXmlRead.getTextBuffer(syntax, anchoredWidgets);

            has_codebox = sqlite3_column_int64(p_stmt, 1);
            has_table = sqlite3_column_int64(p_stmt, 2);
            has_image = sqlite3_column_int64(p_stmt, 3);
        }
    }
    else
    {
        std::cerr << "!! missing node properties for id " << nodeId << std::endl;
    }
    sqlite3_finalize(p_stmt);

    if (has_codebox || has_table || has_image)
    {
        _getTextBufferAnchoredWidgets(rRetTextBuffer, anchoredWidgets, nodeId, has_codebox, has_table, has_image);
    }

    return rRetTextBuffer;
}

void CtSQLiteRead::_getTextBufferAnchoredWidgets(Glib::RefPtr<Gsv::Buffer>& rTextBuffer,
                                                 std::list<CtAnchoredWidget*>& anchoredWidgets,
                                                 const gint64& nodeId,
                                                 const bool& has_codebox,
                                                 const bool& has_table,
                                                 const bool& has_image)
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
            if (SQLITE_OK != sqlite3_prepare_v2(_pDb, query_buff, -1, &pp_stmt[i], 0))
            {
                std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(_pDb) << std::endl;
            }
            else
            {
                sqlite3_bind_int(pp_stmt[i], 1, nodeId);
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
            assert(nullptr != ctXmlRead.get_document());
            ctXmlRead.populateTableMatrix(tableMatrix, ctXmlRead.get_document()->get_root_node());

            pAnchoredWidget = new CtTable(tableMatrix, colMin, colMax, charOffset[i], justification[i]);
            //std::cout << "table " << charOffset[i] << std::endl;
            charOffset[i] = cOffsetRead;
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
                const std::string rawBlob(reinterpret_cast<const char*>(pBlob), blobSize);
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
            pAnchoredWidget->insertInTextBuffer(rTextBuffer);
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

void CtSQLiteRead::_sqlite3TreeWalkIter(gint64 nodeId, const Gtk::TreeIter* pParentIter)
{
    Gtk::TreeIter newIter = _sqlite3NodeProcess(nodeId, pParentIter);

    std::list<gint64> children_nodes_ids = _sqlite3GetChildrenNodeIdFromFatherId(nodeId);
    for (gint64 &child_node_id : children_nodes_ids)
    {
        _sqlite3TreeWalkIter(child_node_id, &newIter);
    }
}

std::list<gint64> CtSQLiteRead::_sqlite3GetChildrenNodeIdFromFatherId(gint64 father_id)
{
    std::list<gint64> ret_children;
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT node_id FROM children WHERE father_id=? ORDER BY sequence ASC", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(_pDb) << std::endl;
        exit(EXIT_FAILURE);
    }
    sqlite3_bind_int(p_stmt, 1, father_id);
    while (sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        gint64 nodeId = sqlite3_column_int64(p_stmt, 0);
        ret_children.push_back(nodeId);
    }
    sqlite3_finalize(p_stmt);
    return ret_children;
}

CtNodeData CtSQLiteRead::_sqlite3GetNodeProperties(gint64 nodeId)
{
    CtNodeData nodeData;
    nodeData.nodeId = nodeId;
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(_pDb, "SELECT name, syntax, tags, is_ro, is_richtxt, ts_creation, ts_lastsave FROM node WHERE node_id=?", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(_pDb) << std::endl;
        exit(EXIT_FAILURE);
    }
    sqlite3_bind_int(p_stmt, 1, nodeId);
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
    }
    else
    {
        std::cerr << "!! missing node properties for id " << nodeId << std::endl;
    }
    sqlite3_finalize(p_stmt);
    return nodeData;
}

Gtk::TreeIter CtSQLiteRead::_sqlite3NodeProcess(gint64 nodeId, const Gtk::TreeIter* pParentIter)
{
    CtNodeData nodeData = _sqlite3GetNodeProperties(nodeId);
    Gtk::TreeIter newIter = signalAppendNode.emit(&nodeData, pParentIter);
    return newIter;
}
