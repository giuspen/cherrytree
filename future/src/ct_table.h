/*
 * ct_table.h
 * 
 * Copyright 2018 Giuseppe Penone <giuspen@gmail.com>
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

#pragma once

#include <gtkmm.h>
#include "ct_const.h"
#include "ct_main_win.h"

typedef std::list<std::list<Glib::ustring>> CtTableMatrix;

class CtTable : public CtAnchoredWidget
{
public:
    CtTable(const CtTableMatrix& tableMatrix,
            const int& colMin,
            const int& colMax,
            const int& charOffset,
            const std::string& justification);
    virtual ~CtTable() {}

protected:
    CtTableMatrix _tableMatrix;
    int _colMin;
    int _colMax;
};
