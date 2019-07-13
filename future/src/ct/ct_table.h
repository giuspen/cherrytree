/*
 * ct_table.h
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

#pragma once

#include "ct_codebox.h"
#include "ct_widgets.h"

class CtTableCell : public CtTextCell, public Gtk::Bin
{
public:
    CtTableCell(const Glib::ustring& textContent,
                const Glib::ustring& syntaxHighlighting);
    virtual ~CtTableCell();
};

typedef std::list<CtTableCell*> CtTableRow;
typedef std::list<CtTableRow> CtTableMatrix;

class CtTable : public CtAnchoredWidget
{
public:
    CtTable(const CtTableMatrix& tableMatrix,
            const int colMin,
            const int colMax,
            const bool headFront,
            const int charOffset,
            const std::string& justification);
    virtual ~CtTable();

    virtual void applyWidthHeight(const int /*parentTextWidth*/) {}
    virtual void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment);

    const CtTableMatrix& getTableMatrix() { return _tableMatrix; }

protected:
    CtTableMatrix _tableMatrix;
    int _colMin;
    int _colMax;
    Gtk::Grid _grid;
};
