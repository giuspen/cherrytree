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
                 CtTableMatrix tableMatrix,
                 const int colMin,
                 const int colMax,
                 const bool headFront,
                 const int charOffset,
                 const std::string& justification)
 : CtAnchoredWidget(pCtMainWin, charOffset, justification),
   _colMin(colMin),
   _colMax(colMax)
{
    if (!headFront)
    {
        CtTableRow headerRow = tableMatrix.back();
        tableMatrix.pop_back();
        tableMatrix.insert(tableMatrix.begin(), headerRow);
    }
    _setup_new_matrix(tableMatrix);

    _grid.set_column_spacing(1);
    _grid.set_row_spacing(1);
    _frame.get_style_context()->add_class("ct-table");
    //_frame.set_border_width(0);
    _frame.add(_grid);
    show_all();
}

CtTable::~CtTable()
{
    // no need for deleting cells, _grid will clean up cells
}

void CtTable::_setup_new_matrix(const CtTableMatrix& tableMatrix)
{
    for (auto widget: _grid.get_children())
        _grid.remove(*widget);

    _tableMatrix = tableMatrix;
    for (int row = 0; row < (int)_tableMatrix.size(); ++row)
    {
        for (int col = 0; col < (int)_tableMatrix[row].size(); ++col)
        {
            CtTableCell* pTableCell = _tableMatrix[row][col];
            bool is_header = row == 0;
            // todo: don't know how to use colMax and colMin, so use just colMax
            pTableCell->get_text_view().set_size_request(_colMax, -1);
            pTableCell->get_text_view().set_highlight_current_line(false);
            if (is_header)
            {
                pTableCell->get_text_view().get_style_context()->add_class("ct-table-header-cell");
                pTableCell->get_text_view().set_wrap_mode(Gtk::WrapMode::WRAP_NONE);
                pTableCell->get_text_view().signal_populate_popup().connect(
                            sigc::bind(sigc::mem_fun(*this, &CtTable::_on_populate_popup_header_cell), row, col));
            }
            else
            {
                pTableCell->get_text_view().signal_populate_popup().connect(
                        sigc::bind(sigc::mem_fun(*this, &CtTable::_on_populate_popup_cell), row, col));
            }
            pTableCell->get_text_view().signal_key_press_event().connect(
                        sigc::bind(sigc::mem_fun(*this, &CtTable::_on_key_press_event_cell), row, col), false);

            _grid.attach(*pTableCell, col, row, 1 /*1 cell horiz*/, 1 /*1 cell vert*/);
        }
    }
    _grid.show_all();
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

void CtTable::column_add(int after_column)
{
    auto matrix = _copy_matrix(after_column, -1, -1, -1, -1, -1);
    _setup_new_matrix(matrix);
}

void CtTable::column_delete(int column)
{
    if (_tableMatrix[0].size() == 1) return;
    auto matrix = _copy_matrix(-1, column, -1, -1, -1, -1);
    _setup_new_matrix(matrix);
}

void CtTable::column_move_left(int column)
{
    if (column == 0) return;
    auto matrix = _copy_matrix(-1, -1, -1, -1, column, -1);
    _setup_new_matrix(matrix);
}

void CtTable::column_move_right(int column)
{
    if (column == (int)_tableMatrix[0].size()-1) return;
    // moving to right is same as moving to left for other column
    auto matrix = _copy_matrix(-1, -1, -1, -1, column + 1, -1);
    _setup_new_matrix(matrix);
}

void CtTable::row_add(int after_row, std::vector<Glib::ustring>* row /*= nullptr*/)
{
    auto matrix = _copy_matrix(-1, -1, after_row, -1, -1, -1);
    if (row && row->size() == matrix[0].size())
        for (int col = 0; col < matrix[0].size(); ++col)
            matrix[after_row + 1][col]->get_text_view().get_buffer()->set_text(row->at(col));
    _setup_new_matrix(matrix);
}

void CtTable::row_delete(int row)
{
    if (_tableMatrix.size() == 1) return;
    auto matrix = _copy_matrix(-1, -1, -1, row, -1, -1);
    _setup_new_matrix(matrix);
}

void CtTable::row_move_up(int row)
{
    if (row == 0) return;
    auto matrix = _copy_matrix(-1, -1, -1, -1, -1, row);
    _setup_new_matrix(matrix);
}

void CtTable::row_move_down(int row)
{
    if (row == (int)_tableMatrix.size()-1) return;
    // moving up is same as moving down for other row
    auto matrix = _copy_matrix(-1, -1, -1, -1, -1, row + 1);
    _setup_new_matrix(matrix);
}

bool CtTable::row_sort_asc()
{
    auto matrix = _copy_matrix(-1, -1, -1, -1, -1, -1);
    std::sort(matrix.begin()+1, matrix.end(), [](CtTableRow& l, CtTableRow& r) { return CtStrUtil::natural_compare(l[0]->get_text_content(), r[0]->get_text_content()) < 0; });
    auto prev_state = get_state();
    _setup_new_matrix(matrix);
    return !prev_state->equal(get_state());
}

bool CtTable::row_sort_desc()
{
    auto matrix = _copy_matrix(-1, -1, -1, -1, -1, -1);
    std::sort(matrix.begin()+1, matrix.end(), [](CtTableRow& l, CtTableRow& r) { return CtStrUtil::natural_compare(l[0]->get_text_content(), r[0]->get_text_content()) > 0; });
    auto prev_state = get_state();
    _setup_new_matrix(matrix);
    return !prev_state->equal(get_state());
}

void CtTable::set_col_min_max(int col_min, int col_max)
{
    _colMin = col_min;
    _colMax = col_max;
    auto matrix = _copy_matrix(-1, -1, -1, -1, -1, -1);
    _setup_new_matrix(matrix);
}

CtTableMatrix CtTable::_copy_matrix(int col_add, int col_del, int row_add, int row_del, int col_move_left, int row_move_up)
{
    // don't check input indexes, it's already checked
    CtTableMatrix matrix;
    for (int row = 0; row < (int)_tableMatrix.size(); ++row)
    {
        if (row == row_del) continue;
        matrix.push_back(CtTableRow());
        for (int col = 0; col < (int)_tableMatrix[row].size(); ++col) {
            if (col == col_del) continue;
            matrix.back().push_back(new CtTableCell(_pCtMainWin, _tableMatrix[row][col]->get_text_content(), CtConst::TABLE_CELL_TEXT_ID));
            if (col == col_add) {
                matrix.back().push_back(new CtTableCell(_pCtMainWin, "", CtConst::TABLE_CELL_TEXT_ID));
            }
            if (col == col_move_left) std::swap(matrix[row][col-1], matrix[row][col]);
        }
        if (row == row_add) {
            matrix.push_back(CtTableRow());
            while (matrix.back().size() != _tableMatrix[0].size())
                matrix.back().push_back(new CtTableCell(_pCtMainWin, "", CtConst::TABLE_CELL_TEXT_ID));
        }
        if (row == row_move_up) std::swap(matrix[row-1], matrix[row]);
    }
    return matrix;
}

void CtTable::_on_populate_popup_header_cell(Gtk::Menu* menu, int row, int col)
{
    if (not _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active()) return;
    for (auto iter : menu->get_children()) menu->remove(*iter);
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    _currentRow = row;
    _currentColumn = col;
    _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::TableHeaderCell);
}

void CtTable::_on_populate_popup_cell(Gtk::Menu* menu, int row, int col)
{
    if (not _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active()) return;
    for (auto iter : menu->get_children()) menu->remove(*iter);
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    _currentRow = row;
    _currentColumn = col;
    _pCtMainWin->get_ct_actions()->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::TableCell);
}

bool CtTable::_on_key_press_event_cell(GdkEventKey* event, int row, int col)
{
    if (not _pCtMainWin->get_ct_actions()->getCtMainWin()->user_active()) return false;
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    _currentRow = row;
    _currentColumn = col;
    // Ctrl+Return for multilines
    if (event->state & Gdk::CONTROL_MASK && event->keyval == GDK_KEY_Return)
        return false;
    if (event->state & Gdk::CONTROL_MASK) {

    }
    else if (event->state & Gdk::SHIFT_MASK) {

    }
    else if (event->state & Gdk::MOD1_MASK) {

    }
    else {
        if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down) {
            int index = row * _tableMatrix[0].size() + col;
            if (event->keyval == GDK_KEY_Up) index -= 1;
            else index +=1;
            row = index/_tableMatrix[0].size();
            col = index%_tableMatrix[0].size();
            if (index < 0) return true;
            if (row == _tableMatrix.size()) return true;
             _currentRow = row;
             _currentColumn = col;
             _tableMatrix[row][col]->get_text_view().grab_focus();
             return true;
        }
    }
    return false;
}
