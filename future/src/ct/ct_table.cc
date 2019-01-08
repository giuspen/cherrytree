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
                 const int& colMin,
                 const int& colMax,
                 const int& charOffset,
                 const std::string& justification)
 : _tableMatrix(tableMatrix),
   _colMin(colMin),
   _colMax(colMax),
   CtAnchoredWidget(charOffset, justification)
{
    CtTableRow headerRow = _tableMatrix.back();
    _tableMatrix.pop_back();
    _tableMatrix.push_front(headerRow);
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
