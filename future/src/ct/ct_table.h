/*
 * ct_table.h
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

#pragma once

#include "ct_codebox.h"
#include "ct_widgets.h"

class CtTextCell;
class CtTableCell : public CtTextCell, public Gtk::EventBox
{
public:
    CtTableCell(CtMainWin* pCtMainWin,
                const Glib::ustring& textContent,
                const Glib::ustring& syntaxHighlighting);
    virtual ~CtTableCell();
};

typedef std::vector<CtTableCell*> CtTableRow;
typedef std::vector<CtTableRow> CtTableMatrix;

class CtTable : public CtAnchoredWidget
{
public:
    CtTable(CtMainWin* pCtMainWin,
            CtTableMatrix tableMatrix,
            const int colMin,
            const int colMax,
            const bool headFront,
            const int charOffset,
            const std::string& justification);
    virtual ~CtTable();

    void apply_width_height(const int /*parentTextWidth*/) override {}
    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment) override;
    void set_modified_false() override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::Table; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    const CtTableMatrix& get_table_matrix() { return _tableMatrix; }
    int get_col_max() { return _colMax; }
    int get_col_min() { return _colMin; }

public:
    int  current_row() { return _currentRow < _tableMatrix.size() ? _currentRow : 0; }
    int  current_column() { return _currentColumn < _tableMatrix[0].size() ? _currentColumn : 0; }

    void column_add(int after_column);
    void column_delete(int column);
    void column_move_left(int column);
    void column_move_right(int column);
    void row_add(int after_row, std::vector<Glib::ustring>* row = nullptr);
    void row_delete(int row);
    void row_move_up(int row);
    void row_move_down(int row);
    bool row_sort_asc();
    bool row_sort_desc();

    void set_col_min_max(int col_min, int col_max);

private:
    void _setup_new_matrix(const CtTableMatrix& tableMatrix);
    CtTableMatrix _copy_matrix(int col_add, int col_del, int row_add, int row_del, int col_move_left, int row_move_up);

protected:
    void _populate_xml_rows_cells(xmlpp::Element* p_table_node);

private:
    void _on_populate_popup_header_cell(Gtk::Menu* menu, int row, int col);
    void _on_populate_popup_cell(Gtk::Menu* menu, int row, int col);
    bool _on_key_press_event_cell(GdkEventKey* event, int row, int co);

protected:
    CtTableMatrix _tableMatrix;
    Gtk::Grid     _grid;
    int           _colMin;
    int           _colMax;
    int           _currentRow = 0;
    int           _currentColumn = 0;
};
