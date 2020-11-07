/*
 * ct_table.cc
 *
 * Copyright 2009-2020
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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
#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_storage_sqlite.h"
#include "ct_logging.h"
#include "ct_misc_utils.h"

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
                 const int colWidthDefault,
                 const int charOffset,
                 const std::string& justification,
                 const CtTableColWidths& colWidths,
                 const int currRow,
                 const int currCol)
 : CtAnchoredWidget{pCtMainWin, charOffset, justification}
 , _colWidthDefault{colWidthDefault}
 , _colWidths{colWidths}
 , _currentRow{currRow}
 , _currentColumn{currCol}
{
    _setup_new_matrix(tableMatrix, false/*apply style when node shows*/);

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

void CtTable::_setup_new_matrix(const CtTableMatrix& tableMatrix, bool apply_style /* = true*/)
{
    for (auto widget : _grid.get_children()) {
        _grid.remove(*widget);
    }
    _tableMatrix = tableMatrix;
    for (size_t row = 0; row < _tableMatrix.size(); ++row) {
        for (size_t col = 0; col < _tableMatrix[row].size(); ++col) {
            CtTableCell* pTableCell = _tableMatrix[row][col];
            CtTextView& textView = pTableCell->get_text_view();
            bool is_header = 0 == row;
            textView.set_size_request(get_col_width(col), -1);
            textView.set_highlight_current_line(false);
            if (is_header) {
                textView.get_style_context()->add_class("ct-table-header-cell");
                textView.set_wrap_mode(Gtk::WrapMode::WRAP_NONE);
            }
            textView.signal_populate_popup().connect(
                    sigc::bind(sigc::mem_fun(*this, &CtTable::_on_populate_popup_cell), row, col));
            textView.signal_key_press_event().connect(
                    sigc::bind(sigc::mem_fun(*this, &CtTable::_on_key_press_event_cell), row, col), false);

            _grid.attach(*pTableCell, col, row, 1 /*1 cell horiz*/, 1 /*1 cell vert*/);
        }
    }
    if (apply_style) {
        _apply_styles_to_cells(false/*forceReApply*/);
    }
    _grid.show_all();
}

void CtTable::_apply_styles_to_cells(const bool forceReApply)
{
    for (CtTableRow& tableRow : _tableMatrix) {
        for (CtTableCell* pTableCell : tableRow) {
            _pCtMainWin->apply_syntax_highlighting(pTableCell->get_buffer(), pTableCell->get_syntax_highlighting(), forceReApply);
        }
    }
}

void CtTable::apply_syntax_highlighting(const bool forceReApply)
{
    _apply_styles_to_cells(forceReApply);
}

void CtTable::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache*)
{
    // todo: fix a duplicate in imports.cc
    xmlpp::Element* p_table_node = p_node_parent->add_child("table");
    p_table_node->set_attribute("char_offset", std::to_string(_charOffset+offset_adjustment));
    p_table_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_table_node->set_attribute("col_min", std::to_string(_colWidthDefault)); // todo get rid of column min
    p_table_node->set_attribute("col_max", std::to_string(_colWidthDefault));
    p_table_node->set_attribute("col_widths", str::join_numbers(_colWidths, ","));
    _populate_xml_rows_cells(p_table_node);
}

void CtTable::_populate_xml_rows_cells(xmlpp::Element* p_table_node)
{
    auto row_to_xml = [&](const CtTableRow& tableRow) {
        xmlpp::Element* p_row_node = p_table_node->add_child("row");
        for (const CtTableCell* pTableCell : tableRow)
        {
            xmlpp::Element* p_cell_node = p_row_node->add_child("cell");
            p_cell_node->add_child_text(pTableCell->get_text_content());
        }
    };

    // put header at the end
    bool is_header = true;
    for (const CtTableRow& tableRow : _tableMatrix)
    {
        if (is_header) { is_header = false; continue; }
        row_to_xml(tableRow);
    }
    row_to_xml(_tableMatrix.front());
}

bool CtTable::to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache*)
{
    bool retVal{true};
    sqlite3_stmt *p_stmt;
    if (sqlite3_prepare_v2(pDb, CtStorageSqlite::TABLE_TABLE_INSERT, -1, &p_stmt, nullptr) != SQLITE_OK)
    {
        spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_PREPV2, sqlite3_errmsg(pDb));
        retVal = false;
    }
    else
    {
        xmlpp::Document xml_doc;
        xml_doc.create_root_node("table");
        xml_doc.get_root_node()->set_attribute("col_widths", str::join_numbers(_colWidths, ","));
        _populate_xml_rows_cells(xml_doc.get_root_node());
        const std::string table_txt = xml_doc.write_to_string();
        sqlite3_bind_int64(p_stmt, 1, node_id);
        sqlite3_bind_int64(p_stmt, 2, _charOffset+offset_adjustment);
        sqlite3_bind_text(p_stmt, 3, _justification.c_str(), _justification.size(), SQLITE_STATIC);
        sqlite3_bind_text(p_stmt, 4, table_txt.c_str(), table_txt.size(), SQLITE_STATIC);
        sqlite3_bind_int64(p_stmt, 5, _colWidthDefault); // todo get rid of column min
        sqlite3_bind_int64(p_stmt, 6, _colWidthDefault);
        if (sqlite3_step(p_stmt) != SQLITE_DONE)
        {
            spdlog::error("{}: {}", CtStorageSqlite::ERR_SQLITE_STEP, sqlite3_errmsg(pDb));
            retVal = false;
        }
        sqlite3_finalize(p_stmt);
    }
    return retVal;
}

void CtTable::to_csv(std::ostream& output) const {
    CtCSV::CtStringTable tbl;
    for (const CtTableRow& ct_row : _tableMatrix) {
        std::vector<std::string> row;

        for (const auto* ct_cell : ct_row) {
            row.emplace_back(ct_cell->get_text_content());
        }
        tbl.emplace_back(row);
    }
    CtCSV::table_to_csv(tbl, output);
}

std::unique_ptr<CtTable> CtTable::from_csv(const std::string& csv_content,
                                           CtMainWin* main_win,
                                           const int col_width,
                                           const int offset,
                                           const Glib::ustring& justification)
{
    std::stringstream input(csv_content);
    CtCSV::CtStringTable str_tbl = CtCSV::table_from_csv(input);

    CtTableMatrix tbl_matrix;
    for (const auto& row : str_tbl) {
        CtTableRow tbl_row;
        for (const auto& cell : row) {
            auto* ct_cell = new CtTableCell(main_win, cell, CtConst::TABLE_CELL_TEXT_ID);
            tbl_row.emplace_back(ct_cell);
        }
        tbl_matrix.emplace_back(tbl_row);
    }

    return std::make_unique<CtTable>(main_win, tbl_matrix, col_width, offset, justification, CtTableColWidths{});
}

std::shared_ptr<CtAnchoredWidgetState> CtTable::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_Table(this));
}

void CtTable::set_modified_false()
{
    for (CtTableRow& tableRow : _tableMatrix) {
        for (CtTableCell* pTableCell : tableRow) {
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
    if (_tableMatrix.front().size() == 1) return;
    auto matrix = _copy_matrix(-1, column, -1, -1, -1, -1);
    _setup_new_matrix(matrix);
}

void CtTable::column_move_left(int column)
{
    if (column == 0) return;
    auto matrix = _copy_matrix(-1, -1, -1, -1, column, -1);
    const int colWidthTmp = get_col_width(column-1);
    _colWidths[column-1] = _colWidths[column];
    _colWidths[column] = colWidthTmp;
    _setup_new_matrix(matrix);
}

void CtTable::column_move_right(int column)
{
    if (column == (int)_tableMatrix.front().size()-1) return;
    // moving to right is same as moving to left for other column
    auto matrix = _copy_matrix(-1, -1, -1, -1, column + 1, -1);
    const int colWidthTmp = get_col_width(column+1);
    _colWidths[column+1] = _colWidths[column];
    _colWidths[column] = colWidthTmp;
    _setup_new_matrix(matrix);
}

void CtTable::row_add(int after_row, std::vector<Glib::ustring>* row /*= nullptr*/)
{
    auto matrix = _copy_matrix(-1, -1, after_row, -1, -1, -1);
    if (row && row->size() == matrix[0].size())
        for (int col = 0; col < (int)matrix[0].size(); ++col)
            matrix[after_row + 1][col]->get_text_view().get_buffer()->set_text(row->at(col));
    _setup_new_matrix(matrix);
}

void CtTable::row_delete(int row)
{
    if ((int)_tableMatrix.size() == 1) return;
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

void CtTable::set_col_width_default(const int colWidthDefault)
{
    _colWidthDefault = colWidthDefault;
    if (_colWidths.empty()) {
        auto matrix = _copy_matrix(-1, -1, -1, -1, -1, -1);
        _setup_new_matrix(matrix);
    }
}

void CtTable::set_col_width(const int colWidth, std::optional<size_t> optColIdx/* = std::nullopt*/)
{
    const size_t colIdx = optColIdx.value_or(_currentColumn);
    // ensure the column widths are of the right size
    while (_colWidths.size() < _tableMatrix.front().size()) {
        // the table is using the default width for this column
        _colWidths.push_back(_colWidthDefault);
    }
    _colWidths[colIdx] = colWidth;
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
            while (matrix.back().size() != _tableMatrix.front().size())
                matrix.back().push_back(new CtTableCell(_pCtMainWin, "", CtConst::TABLE_CELL_TEXT_ID));
        }
        if (row == row_move_up) std::swap(matrix[row-1], matrix[row]);
    }
    return matrix;
}

void CtTable::_on_populate_popup_cell(Gtk::Menu* menu, int row, int col)
{
    if (not _pCtMainWin->user_active()) return;
    for (auto iter : menu->get_children()) menu->remove(*iter);
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    _currentRow = row;
    _currentColumn = col;
    const bool first_row = 0 == row;
    const bool first_col = 0 == col;
    const bool last_row = static_cast<int>(_tableMatrix.size() - 1) == row;
    const bool last_col = _tableMatrix.size() and static_cast<int>(_tableMatrix.front().size() - 1) == col;
    _pCtMainWin->get_ct_menu().build_popup_menu_table_cell(menu, first_row, first_col, last_row, last_col);
}

bool CtTable::_on_key_press_event_cell(GdkEventKey* event, int row, int col)
{
    if (not _pCtMainWin->user_active()) return false;
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    _currentRow = row;
    _currentColumn = col;
    int index{-1};
    if (event->keyval == GDK_KEY_Tab or event->keyval == GDK_KEY_ISO_Left_Tab) {
        if (event->state & Gdk::SHIFT_MASK) {
            index = row * _tableMatrix.front().size() + col - 1;
        }
        else {
            index = row * _tableMatrix.front().size() + col + 1;
        }
    }
    else if (event->state & Gdk::CONTROL_MASK) {
        if (event->keyval == GDK_KEY_space) {
            CtTextView& textView = _pCtMainWin->get_text_view();
            Gtk::TextIter text_iter = textView.get_buffer()->get_iter_at_child_anchor(getTextChildAnchor());
            text_iter.forward_char();
            textView.get_buffer()->place_cursor(text_iter);
            textView.grab_focus();
            return true;
        }
        if (event->keyval == GDK_KEY_period) {
            if (event->state & Gdk::MOD1_MASK) {
                _pCtMainWin->get_ct_actions()->table_column_decrease_width();
            }
            else {
                _pCtMainWin->get_ct_actions()->table_column_increase_width();
            }
            return true;
        }
        if (event->keyval == GDK_KEY_Up) {
            if (row > 0) {
                index = (row-1) * _tableMatrix.front().size() + col;
            }
        }
        else if (event->keyval == GDK_KEY_Down) {
            if (row+1 < static_cast<int>(_tableMatrix.size())) {
                index = (row+1) * _tableMatrix.front().size() + col;
            }
        }
        else if (event->keyval == GDK_KEY_Left) {
            index = row * _tableMatrix.front().size() + col - 1;
        }
        else if (event->keyval == GDK_KEY_Right) {
            index = row * _tableMatrix.front().size() + col + 1;
        }
    }
    if (index >= 0) {
        row = index / _tableMatrix.front().size();
        col = index % _tableMatrix.front().size();
        if ( row < static_cast<int>(_tableMatrix.size()) and
             col < static_cast<int>(_tableMatrix.front().size()) )
        {
            _currentRow = row;
            _currentColumn = col;
            _tableMatrix[row][col]->get_text_view().grab_focus();
        }
        return true;
    }
    return false;
}
