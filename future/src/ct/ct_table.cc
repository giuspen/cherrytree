/*
 * ct_table.cc
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

#include "ct_table.h"
#include "ct_doc_rw.h"

CtTableCell::CtTableCell(CtMainWin* pCtMainWin,
                         const Glib::ustring& textContent,
                         const Glib::ustring& syntaxHighlighting)
 : CtTextCell(pCtMainWin, textContent, syntaxHighlighting)
{
    add(_ctTextview);
}

CtTableCell::~CtTableCell()
{
}


CtTable::CtTable(CtMainWin* pCtMainWin,
                 const CtTableMatrix& tableMatrix,
                 const int colMin,
                 const int colMax,
                 const bool headFront,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(pCtMainWin, charOffset, justification),
   _tableMatrix(tableMatrix),
   _colMin(colMin),
   _colMax(colMax)
{
    if (!headFront)
    {
        CtTableRow headerRow = _tableMatrix.back();
        _tableMatrix.pop_back();
        _tableMatrix.push_front(headerRow);
    }
    int i{0};
    for (CtTableRow& tableRow : _tableMatrix)
    {
        int j{0};
        for (CtTableCell* pTableCell : tableRow)
        {
            _grid.attach(*pTableCell, j, i, 1, 1);
            j++;
        }
        i++;
    }
    _frame.add(_grid);
    show_all();
}

CtTable::~CtTable()
{
    for (CtTableRow& tableRow : _tableMatrix)
    {
        for (CtTableCell* pTableCell : tableRow)
        {
            delete pTableCell;
        }
    }
}

void CtTable::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment)
{
    xmlpp::Element* p_table_node = p_node_parent->add_child("table");
    p_table_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_table_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_table_node->set_attribute("col_min", std::to_string(_colMin));
    p_table_node->set_attribute("col_max", std::to_string(_colMax));
    _populate_xml_rows_cells(p_table_node);
}

void CtTable::_populate_xml_rows_cells(xmlpp::Element* p_table_node)
{
    p_table_node->set_attribute("head_front", std::to_string(true));
    for (const CtTableRow& tableRow : _tableMatrix)
    {
        xmlpp::Element* p_row_node = p_table_node->add_child("row");
        for (const CtTableCell* pTableCell : tableRow)
        {
            xmlpp::Element* p_cell_node = p_row_node->add_child("cell");
            p_cell_node->add_child_text(pTableCell->get_text_content());
        }
    }
}

bool CtTable::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtSQLite::TABLE_TABLE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << CtSQLite::ERR_SQLITE_PREPV2 << sqlite3_errmsg(pDb) << std::endl;
        retVal = false;
    }
    else
    {
        CtXmlWrite ctXmlWrite("table");
        _populate_xml_rows_cells(ctXmlWrite.get_root_node());
        const std::string table_txt = Glib::locale_from_utf8(ctXmlWrite.write_to_string());
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, table_txt.c_str(), table_txt.size(), SQLITE_STATIC);
        sqlite3_bind_int64(p_stmt, 5, _colMin);
        sqlite3_bind_int64(p_stmt, 6, _colMax);
        if (sqlite3_step(p_stmt) != SQLITE_DONE)
        {
            std::cerr << CtSQLite::ERR_SQLITE_STEP << sqlite3_errmsg(pDb) << std::endl;
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

CtAnchoredWidget* CtTable::clone()
{
    return new CtTable(_pCtMainWin, _tableMatrix, _colMin, _colMax, true, _charOffset, _justification);
}

bool CtTable::equal(CtAnchoredWidget* other)
{
    if (get_type() != other->get_type()) return false;

    auto compare_cell = [](CtTableCell* lhs, CtTableCell* rhs) {
        return lhs->get_text_content() == rhs->get_text_content() && lhs->get_syntax_highlighting() == rhs->get_syntax_highlighting();
    };
    auto compare_row = [compare_cell](const CtTableRow& lhs, const CtTableRow& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), compare_cell);
    };
    auto compare_table = [compare_row](const CtTableMatrix& lhs, const CtTableMatrix& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), compare_row);
    };

    if (CtTable* other_table = dynamic_cast<CtTable*>(other))
        if (_colMin == other_table->_colMin
                && _colMax == other_table->_colMax
                && _charOffset == other_table->_charOffset
                && _justification == other_table->_justification
                && compare_table(_tableMatrix, other_table->_tableMatrix))
            return true;
    return false;
}

void CtTable::set_modified_false()
{
    for (CtTableRow& tableRow : _tableMatrix)
    {
        for (CtTableCell* pTableCell : tableRow)
        {
            pTableCell->set_text_buffer_modified_false();
        }
    }
}
