/*
 * ct_table.cc
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

#include "ct_table.h"
#include "ct_app.h"

CtTableCell::CtTableCell(const Glib::ustring& textContent,
                         const Glib::ustring& syntaxHighlighting)
 : CtTextCell(textContent, syntaxHighlighting)
{
    add(_ctTextview);
}

CtTableCell::~CtTableCell()
{
}


CtTable::CtTable(const CtTableMatrix& tableMatrix,
                 const int colMin,
                 const int colMax,
                 const bool headFront,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(charOffset, justification),
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
    CtAnchoredWidget::to_xml(p_node_parent, offset_adjustment);
    xmlpp::Element* p_table_node = p_node_parent->add_child("table");
    p_table_node->set_attribute("char_offset", std::to_string(_charOffset));
    p_table_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_table_node->set_attribute("col_min", std::to_string(_colMin));
    p_table_node->set_attribute("col_max", std::to_string(_colMax));
    p_table_node->set_attribute("head_front", std::to_string(true));
    for (const CtTableRow& tableRow : _tableMatrix)
    {
        xmlpp::Element* p_row_node = p_table_node->add_child("row");
        for (const CtTableCell* pTableCell : tableRow)
        {
            xmlpp::Element* p_cell_node = p_row_node->add_child("cell");
            p_cell_node->add_child_text(pTableCell->getTextContent());
        }
    }
}
