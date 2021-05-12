/*
 * ct_table.cc
 *
 * Copyright 2009-2021
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
#include "ct_storage_xml.h"
#include "ct_logging.h"
#include "ct_misc_utils.h"
#include <fstream>

CtTable::CtTable(CtMainWin* pCtMainWin,
                 const CtTableMatrix& tableMatrix,
                 const int colWidthDefault,
                 const int charOffset,
                 const std::string& justification,
                 const CtTableColWidths& colWidths,
                 const size_t currRow,
                 const size_t currCol)
 : CtAnchoredWidget{pCtMainWin, charOffset, justification}
 , _tableMatrix{tableMatrix}
 , _colWidthDefault{colWidthDefault}
 , _colWidths{colWidths}
 , _currentRow{currRow}
 , _currentColumn{currCol}
{
    // column widths can be empty or wrong, trying to fix it
    // so we don't need to check it again and again
    while (_colWidths.size() < tableMatrix[0].size())
        _colWidths.push_back(0); // 0 means we use default width

    for (size_t rowIdx = 0; rowIdx < _tableMatrix.size(); ++rowIdx) {
        for (size_t colIdx = 0; colIdx < _tableMatrix[rowIdx].size(); ++colIdx) {
            _new_text_cell_attach(rowIdx, colIdx, _tableMatrix.at(rowIdx).at(colIdx));
        }
    }

    _grid.set_column_spacing(1);
    _grid.set_row_spacing(1);
    _grid.signal_button_press_event().connect(sigc::mem_fun(*this, &CtTable::_on_button_press_event_grid), false);
    _grid.signal_set_focus_child().connect(sigc::mem_fun(*this, &CtTable::_on_set_focus_child_grid));

    _frame.get_style_context()->add_class("ct-table");
    _frame.add(_grid);
    _frame.signal_size_allocate().connect(sigc::mem_fun(*this, &CtTable::_on_size_allocate_frame));
    show_all();
}

CtTable::~CtTable()
{
    for (CtTableRow& tableRow : _tableMatrix) {
        for (CtTextCell* pTextCell : tableRow) {
            delete pTextCell;
        }
    }
}

void CtTable::_new_text_cell_attach(const size_t rowIdx, const size_t colIdx, CtTextCell* pTextCell)
{
    CtTextView& textView = pTextCell->get_text_view();
    const bool is_header = 0 == rowIdx;
    textView.set_size_request(get_col_width(colIdx), -1);
    textView.set_highlight_current_line(false);
    if (is_header) {
        _apply_remove_header_style(true/*isApply*/, textView);
    }
    textView.signal_populate_popup().connect(sigc::mem_fun(*this, &CtTable::_on_populate_popup_cell));
    textView.signal_key_press_event().connect(sigc::mem_fun(*this, &CtTable::_on_key_press_event_cell), false);

    _grid.attach(pTextCell->get_text_view(), colIdx, rowIdx, 1/*# cell horiz*/, 1/*# cell vert*/);

    _pCtMainWin->apply_syntax_highlighting(pTextCell->get_buffer(), pTextCell->get_syntax_highlighting(), false/*forceReApply*/);
    textView.show();
}

void CtTable::_apply_styles_to_cells(const bool forceReApply)
{
    for (CtTableRow& tableRow : _tableMatrix) {
        for (CtTextCell* pTextCell : tableRow) {
            _pCtMainWin->apply_syntax_highlighting(pTextCell->get_buffer(), pTextCell->get_syntax_highlighting(), forceReApply);
        }
    }
}

void CtTable::apply_syntax_highlighting(const bool forceReApply)
{
    _apply_styles_to_cells(forceReApply);
}

void CtTable::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache*)
{
    std::vector<std::vector<Glib::ustring>> rows;
    rows.reserve(_tableMatrix.size());
    for (auto& row: _tableMatrix)
    {
        rows.push_back(std::vector<Glib::ustring>());
        rows.back().reserve(row.size());
        for (auto& cell : row)  rows.back().push_back(cell->get_text_content());
    }

    CtXmlHelper::table_to_xml(p_node_parent, rows, _charOffset+offset_adjustment, _justification, _colWidthDefault, str::join_numbers(_colWidths, ","));
}

void CtTable::_populate_xml_rows_cells(xmlpp::Element* p_table_node)
{
    auto row_to_xml = [&](const CtTableRow& tableRow) {
        xmlpp::Element* p_row_node = p_table_node->add_child("row");
        for (const CtTextCell* pTextCell : tableRow)
        {
            xmlpp::Element* p_cell_node = p_row_node->add_child("cell");
            p_cell_node->add_child_text(pTextCell->get_text_content());
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

std::unique_ptr<CtTable> CtTable::from_csv(const std::string& filepath,
                                           CtMainWin* main_win,
                                           const int offset,
                                           const Glib::ustring& justification)
{
    std::ifstream input(filepath);
    if (not input.is_open()) {
        return std::unique_ptr<CtTable>{nullptr};
    }
    CtCSV::CtStringTable str_tbl = CtCSV::table_from_csv(input);

    CtTableMatrix tbl_matrix;
    if (str_tbl.size() and str_tbl.front().size()) {
        const size_t numColumns = str_tbl.front().size();
        size_t currRow{0};
        for (const auto& row : str_tbl) {
            ++currRow;
            CtTableRow tbl_row;
            size_t currCol{0};
            for (const auto& cell : row) {
                ++currCol;
                if (currCol > numColumns) {
                    spdlog::warn("from_csv row {} col {} > {}", currRow, currCol, numColumns);
                    break;
                }
                auto* ct_cell = new CtTextCell{main_win, cell, CtConst::TABLE_CELL_TEXT_ID};
                tbl_row.emplace_back(ct_cell);
            }
            while (currCol < numColumns) {
                ++currCol;
                auto* ct_cell = new CtTextCell{main_win, "", CtConst::TABLE_CELL_TEXT_ID};
                tbl_row.emplace_back(ct_cell);
            }
            tbl_matrix.emplace_back(tbl_row);
        }
    }

    return std::make_unique<CtTable>(main_win, tbl_matrix, 60, offset, justification, CtTableColWidths{});
}

std::shared_ptr<CtAnchoredWidgetState> CtTable::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_Table(this));
}

void CtTable::set_modified_false()
{
    for (CtTableRow& tableRow : _tableMatrix) {
        for (CtTextCell* pTextCell : tableRow) {
            pTextCell->set_text_buffer_modified_false();
        }
    }
}

void CtTable::column_add(const size_t afterColIdx)
{
    const size_t newColIdx = afterColIdx + 1;
    _grid.insert_column(newColIdx);
    _colWidths.insert(_colWidths.begin()+newColIdx, 0);
    for (size_t rowIdx = 0; rowIdx < _tableMatrix.size(); ++rowIdx) {
        auto pTextCell = new CtTextCell{_pCtMainWin, "", CtConst::TABLE_CELL_TEXT_ID};
        _tableMatrix.at(rowIdx).insert(_tableMatrix.at(rowIdx).begin()+newColIdx, pTextCell);
        _new_text_cell_attach(rowIdx, newColIdx, pTextCell);
    }
}

void CtTable::column_delete(const size_t colIdx)
{
    if (1 == _tableMatrix.front().size() or colIdx >= _tableMatrix.front().size()) {
        return;
    }
    _grid.remove_column(colIdx);
    _colWidths.erase(_colWidths.begin()+colIdx);
    for (CtTableRow& tableRow : _tableMatrix) {
        delete tableRow.at(colIdx);
        tableRow.erase(tableRow.begin()+colIdx);
    }
}

void CtTable::column_move_left(const size_t colIdx)
{
    if (0 == colIdx) {
        return;
    }
    const size_t colIdxLeft = colIdx - 1;
    std::swap(_colWidths[colIdxLeft], _colWidths[colIdx]);
    _grid.remove_column(colIdxLeft);
    _grid.insert_column(colIdx);
    for (size_t rowIdx = 0; rowIdx < _tableMatrix.size(); ++rowIdx) {
        std::swap(_tableMatrix[rowIdx][colIdxLeft], _tableMatrix[rowIdx][colIdx]);
        CtTextView& textView = _tableMatrix.at(rowIdx).at(colIdx)->get_text_view();
        _grid.attach(textView, colIdx, rowIdx, 1/*# cell horiz*/, 1/*# cell vert*/);
    }
    _currentColumn = colIdxLeft;
}

void CtTable::column_move_right(const size_t colIdx)
{
    if (colIdx == _tableMatrix.front().size()-1) {
        return;
    }
    column_move_left(colIdx+1);
}

void CtTable::row_add(const size_t afterRowIdx, const std::vector<Glib::ustring>* pNewRow/*= nullptr*/)
{
    const size_t newRowIdx = afterRowIdx + 1;
    _tableMatrix.insert(_tableMatrix.begin()+newRowIdx, CtTableRow{});
    _grid.insert_row(newRowIdx);
    const Glib::ustring emptyCell;
    for (size_t colIdx = 0; colIdx < _tableMatrix.front().size(); ++colIdx) {
        const Glib::ustring* pStr = not pNewRow or pNewRow->size() <= colIdx ? &emptyCell : &pNewRow->at(colIdx);
        auto pTextCell = new CtTextCell{_pCtMainWin, *pStr, CtConst::TABLE_CELL_TEXT_ID};
        _tableMatrix.at(newRowIdx).push_back(pTextCell);
        _new_text_cell_attach(newRowIdx, colIdx, pTextCell);
    }
}

void CtTable::row_delete(const size_t rowIdx)
{
    if (1 == _tableMatrix.size() or rowIdx >= _tableMatrix.size()) {
        return;
    }
    _grid.remove_row(rowIdx);
    for (CtTextCell* pTextCell : _tableMatrix.at(rowIdx)) {
        delete pTextCell;
    }
    _tableMatrix.erase(_tableMatrix.begin()+rowIdx);
}

void CtTable::_apply_remove_header_style(const bool isApply, CtTextView& textView)
{
    const char headerStyle[] = "ct-table-header-cell";
    auto rStyleContext = textView.get_style_context();
    if (isApply) {
        if (not rStyleContext->has_class(headerStyle)) {
            rStyleContext->add_class(headerStyle);
            textView.set_wrap_mode(Gtk::WrapMode::WRAP_NONE);
        }
    }
    else {
        if (rStyleContext->has_class(headerStyle)) {
            rStyleContext->remove_class(headerStyle);
            textView.set_wrap_mode(_pCtMainWin->get_ct_config()->lineWrapping ?
                                   Gtk::WrapMode::WRAP_WORD_CHAR : Gtk::WrapMode::WRAP_NONE);
        }
    }
}

void CtTable::row_move_up(const size_t rowIdx)
{
    if (0 == rowIdx) {
        return;
    }
    const size_t rowIdxUp = rowIdx - 1;
    _grid.remove_row(rowIdxUp);
    _grid.insert_row(rowIdx);
    std::swap(_tableMatrix[rowIdxUp], _tableMatrix[rowIdx]);
    for (size_t colIdx = 0; colIdx < _tableMatrix.front().size(); ++colIdx) {
        CtTextView& textView = _tableMatrix.at(rowIdx).at(colIdx)->get_text_view();
        _grid.attach(textView, colIdx, rowIdx, 1/*# cell horiz*/, 1/*# cell vert*/);
        if (0 == rowIdxUp) {
            // we swapped header
            CtTextView& textViewUp = _tableMatrix.at(rowIdxUp).at(colIdx)->get_text_view();
            _apply_remove_header_style(true/*isApply*/, textViewUp);
            _apply_remove_header_style(false/*isApply*/, textView);
        }
    }
    _currentRow = rowIdxUp;
}

void CtTable::row_move_down(const size_t rowIdx)
{
    if (rowIdx == _tableMatrix.size()-1) {
        return;
    }
    row_move_up(rowIdx+1);
}

bool CtTable::_row_sort(const bool sortAsc)
{
    auto f_tableCompare = [sortAsc](CtTableRow& l, CtTableRow& r)->bool{
        auto cmpResult = CtStrUtil::natural_compare(l.front()->get_text_content(), r.front()->get_text_content());
        return sortAsc ? cmpResult < 0 : cmpResult > 0;
    };
    auto pPrevState = std::static_pointer_cast<CtAnchoredWidgetState_Table>(get_state());
    std::sort(_tableMatrix.begin()+1, _tableMatrix.end(), f_tableCompare);
    auto pCurrState = std::static_pointer_cast<CtAnchoredWidgetState_Table>(get_state());
    std::list<size_t> changed;
    for (size_t rowIdx = 1; rowIdx < _tableMatrix.size(); ++rowIdx) {
        if (pPrevState->rows.at(rowIdx) != pCurrState->rows.at(rowIdx)) {
            changed.push_back(rowIdx);
            _grid.remove_row(rowIdx);
            _grid.insert_row(rowIdx);
        }
    }
    if (changed.size()) {
        for (auto rowIdx : changed) {
            for (size_t colIdx = 0; colIdx < _tableMatrix.at(rowIdx).size(); ++colIdx) {
                CtTextView& textView = _tableMatrix.at(rowIdx).at(colIdx)->get_text_view();
                _grid.attach(textView, colIdx, rowIdx, 1/*# cell horiz*/, 1/*# cell vert*/);
            }
        }
        return true;
    }
    return false;
}

void CtTable::set_col_width_default(const int colWidthDefault)
{
    _colWidthDefault = colWidthDefault;
    bool has_default_widths = vec::exists(_colWidths, 0);
    if (has_default_widths) {
        for (size_t rowIdx = 0; rowIdx < _tableMatrix.size(); ++rowIdx) {
            for (size_t colIdx = 0; colIdx < _tableMatrix[rowIdx].size(); ++colIdx) {
                CtTextCell* pTextCell = _tableMatrix[rowIdx][colIdx];
                CtTextView& textView = pTextCell->get_text_view();
                textView.set_size_request(get_col_width(colIdx), -1);
            }
        }
    }
}

void CtTable::set_col_width(const int colWidth, std::optional<size_t> optColIdx/*= std::nullopt*/)
{
    const size_t colIdx = optColIdx.value_or(_currentColumn);
    _colWidths[colIdx] = colWidth;
    for (size_t rowIdx = 0; rowIdx < _tableMatrix.size(); ++rowIdx) {
        CtTextCell* pTextCell = _tableMatrix[rowIdx][colIdx];
        CtTextView& textView = pTextCell->get_text_view();
        textView.set_size_request(get_col_width(colIdx), -1);
    }
}

void CtTable::_on_populate_popup_cell(Gtk::Menu* menu)
{
    if (not _pCtMainWin->user_active()) return;
    const size_t rowIdx = current_row();
    const size_t colIdx = current_column();
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    const bool first_row = 0 == rowIdx;
    const bool first_col = 0 == colIdx;
    const bool last_row = _tableMatrix.size()-1 == rowIdx;
    const bool last_col = _tableMatrix.size() and _tableMatrix.front().size()-1 == colIdx;
    _pCtMainWin->get_ct_menu().build_popup_menu_table_cell(menu, first_row, first_col, last_row, last_col);
}

void CtTable::_on_set_focus_child_grid(Gtk::Widget* pWidget)
{
    for (size_t rowIdx = 0; rowIdx < _tableMatrix.size(); ++rowIdx) {
        for (size_t colIdx = 0; colIdx < _tableMatrix[rowIdx].size(); ++colIdx) {
            if (pWidget == &_tableMatrix.at(rowIdx).at(colIdx)->get_text_view()) {
                _currentRow = rowIdx;
                _currentColumn = colIdx;
                return;
            }
        }
    }
}

bool CtTable::_on_button_press_event_grid(GdkEventButton* event)
{
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    if ( event->button != 3/*right button*/ and event->type != GDK_3BUTTON_PRESS) {
        _pCtMainWin->get_ct_actions()->object_set_selection(this);
    }
    return false;
}

bool CtTable::_on_key_press_event_cell(GdkEventKey* event)
{
    if (not _pCtMainWin->user_active()) return false;
    const size_t rowIdx = current_row();
    const size_t colIdx = current_column();
    _pCtMainWin->get_ct_actions()->curr_table_anchor = this;
    int index{-1};
    if (event->keyval == GDK_KEY_Tab or event->keyval == GDK_KEY_ISO_Left_Tab) {
        if (event->state & Gdk::SHIFT_MASK) {
            index = rowIdx * _tableMatrix.front().size() + colIdx - 1;
        }
        else {
            index = rowIdx * _tableMatrix.front().size() + colIdx + 1;
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
        if (event->keyval == GDK_KEY_comma) {
            if (event->state & Gdk::MOD1_MASK) {
                _pCtMainWin->get_ct_actions()->table_row_delete();
            }
            else {
                _pCtMainWin->get_ct_actions()->table_row_add();
            }
            return true;
        }
        if (event->keyval == GDK_KEY_parenleft) {
            if (rowIdx > 0) {
                index = (rowIdx-1) * _tableMatrix.front().size() + colIdx;
            }
        }
        else if (event->keyval == GDK_KEY_parenright) {
            if (rowIdx+1 < _tableMatrix.size()) {
                index = (rowIdx+1) * _tableMatrix.front().size() + colIdx;
            }
        }
    }
    if (index >= 0) {
        const size_t nextRowIdx = index / _tableMatrix.front().size();
        const size_t nextColIdx = index % _tableMatrix.front().size();
        if ( nextRowIdx < _tableMatrix.size() and
             nextColIdx < _tableMatrix.front().size() )
        {
            _tableMatrix[nextRowIdx][nextColIdx]->get_text_view().grab_focus();
        }
        return true;
    }
    return false;
}
