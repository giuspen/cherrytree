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
#include "ct_main_win.h"
#include "ct_actions.h"

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
    int row{0};
    for (CtTableRow& tableRow : _tableMatrix)
    {
        int col{0};
        for (CtTableCell* pTableCell : tableRow)
        {
            bool is_header = row == 0;
            // todo: don't know how to use colMax and colMin, so use just colMax
            pTableCell->get_text_view().set_size_request(colMax, -1);
            if (is_header)
            {
                pTableCell->get_text_view().get_style_context()->add_class("ct-table-header-cell");
                pTableCell->get_text_view().set_wrap_mode(Gtk::WrapMode::WRAP_NONE);
                pTableCell->get_text_view().signal_populate_popup().connect(sigc::mem_fun(*this, &CtTable::_on_populate_popup_header_cell));
            }
            else
            {
                pTableCell->get_text_view().signal_populate_popup().connect(sigc::mem_fun(*this, &CtTable::_on_populate_popup_cell));
            }


            _grid.attach(*pTableCell, col, row, 1 /*1 cell horiz*/, 1 /*1 cell vert*/);
            col++;
        }
        row++;
    }
    _frame.get_style_context()->add_class("ct-table");
    _frame.set_border_width(1);
    _frame.add(_grid);
    show_all();
}

CtTable::~CtTable()
{
    // no need for deleting cells, _grid will clean up cells
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

std::shared_ptr<CtAnchoredWidgetState> CtTable::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_Table(this));
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

void CtTable::_on_populate_popup_header_cell(Gtk::Menu* menu)
{
    if (not _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active()) return;
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::TableHeaderCell);
}

void CtTable::_on_populate_popup_cell(Gtk::Menu* menu)
{
    if (not _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active()) return;
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::TableCell);
}

void CtTable::_on_button_press_event_cell()
{
    if (not _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active()) return;
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
}
