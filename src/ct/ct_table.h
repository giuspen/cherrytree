/*
 * ct_table.h
 *
 * Copyright 2009-2022
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

#pragma once

#include "ct_codebox.h"
#include "ct_widgets.h"
#include <optional>

class CtAnchoredWidgetState_TableCommon;
class CtTableCommon : public CtAnchoredWidget
{
public:
    CtTableCommon(CtMainWin* pCtMainWin,
                  const int colWidthDefault,
                  const int charOffset,
                  const std::string& justification,
                  const CtTableColWidths& colWidths,
                  const size_t currRow,
                  const size_t currCol);

    std::shared_ptr<CtAnchoredWidgetState_TableCommon> get_state_common() const;

    void apply_width_height(const int /*parentTextWidth*/) override {}

    const CtTableColWidths& get_col_widths_raw() const { return _colWidths; }
    int get_col_width_default() const { return _colWidthDefault; }
    bool get_is_light() const;
    int get_col_width(const std::optional<size_t> optColIdx = std::nullopt) const {
        const size_t colIdx = optColIdx.value_or(_currentColumn);
        return _colWidths.at(colIdx) != 0 ? _colWidths.at(colIdx) : _colWidthDefault;
    }
    CtTableColWidths get_col_widths() const {
        CtTableColWidths colWidths;
        for (size_t colIdx = 0; colIdx < _colWidths.size(); ++colIdx) {
            colWidths.push_back(get_col_width(colIdx));
        }
        return colWidths;
    }
    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;

    // Build a table from csv; The input csv should be compatable with the excel csv format
    static void populate_table_matrix_from_csv(const std::string& filepath,
                                               CtMainWin* main_win,
                                               const bool is_light,
                                               CtTableMatrix& tbl_matrix);

    // Serialise to csv format; The output CSV excel csv with double quotes around cells and newlines for each record
    virtual std::string to_csv() const = 0;

    virtual void write_strings_matrix(std::vector<std::vector<Glib::ustring>>& rows) const = 0;
    virtual size_t get_num_rows() const = 0;
    virtual size_t get_num_columns() const = 0;
    size_t current_row() const { return _currentRow < get_num_rows() ? _currentRow : 0; }
    size_t current_column() const { return _currentColumn < get_num_columns() ? _currentColumn : 0; }
    bool row_sort_asc() { return _row_sort(true/*sortAsc*/); }
    bool row_sort_desc() { return _row_sort(false/*sortAsc*/); }

    virtual void column_add(const size_t afterColIdx) = 0;
    virtual void column_delete(const size_t colIdx) = 0;
    virtual void column_move_left(const size_t colIdx) = 0;
    virtual void column_move_right(const size_t colIdx) = 0;
    virtual void row_add(const size_t afterRowIdx, const std::vector<Glib::ustring>* pNewRow = nullptr) = 0;
    virtual void row_delete(const size_t rowIdx) = 0;
    virtual void row_move_up(const size_t rowIdx) = 0;
    virtual void row_move_down(const size_t rowIdx) = 0;

    virtual void set_col_width_default(const int colWidthDefault) = 0;
    virtual void set_col_width(const int colWidth, std::optional<size_t> optColIdx = std::nullopt) = 0;

    virtual void grab_focus() const = 0;

    bool on_table_button_press_event(GdkEventButton* event);
    void on_cell_populate_popup(Gtk::Menu* menu);
    bool on_cell_key_press_event(GdkEventKey* event);

protected:
    virtual void _populate_xml_rows_cells(xmlpp::Element* p_table_node) const = 0;
    virtual bool _row_sort(const bool sortAsc) = 0;

    int              _colWidthDefault;
    CtTableColWidths _colWidths;
    size_t           _currentRow{0u};
    size_t           _currentColumn{0u};
};

struct CtTableLightColumns : public Gtk::TreeModelColumnRecord
{
    CtTableLightColumns(const size_t numColumns) {
        columnsText.resize(numColumns);
        for (size_t i = 0u; i < numColumns; ++i) {
            add(columnsText.at(i));
        }
        add(columnWeight);
    }
    std::vector<Gtk::TreeModelColumn<Glib::ustring>> columnsText;
    Gtk::TreeModelColumn<int>                        columnWeight;
};

class CtTableLight : public CtTableCommon
{
public:
    CtTableLight(CtMainWin* pCtMainWin,
                 CtTableMatrix& tableMatrix,
                 const int colWidthDefault,
                 const int charOffset,
                 const std::string& justification,
                 const CtTableColWidths& colWidths,
                 const size_t currRow = 0,
                 const size_t currCol = 0);

    const CtTableLightColumns& get_columns() const { return *_pColumns; }

    void apply_syntax_highlighting(const bool /*forceReApply*/) override {}
    std::string to_csv() const override;
    void set_modified_false() override {}
    CtAnchWidgType get_type() override { return CtAnchWidgType::TableLight; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    void write_strings_matrix(std::vector<std::vector<Glib::ustring>>& rows) const override;
    size_t get_num_rows() const override { return _pListStore->children().size(); }
    size_t get_num_columns() const override { return _pColumns->columnsText.size(); }

    void column_add(const size_t afterColIdx) override;
    void column_delete(const size_t colIdx) override;
    void column_move_left(const size_t colIdx) override;
    void column_move_right(const size_t colIdx) override;
    void row_add(const size_t afterRowIdx, const std::vector<Glib::ustring>* pNewRow = nullptr) override;
    void row_delete(const size_t rowIdx) override;
    void row_move_up(const size_t rowIdx) override;
    void row_move_down(const size_t rowIdx) override;

    void set_col_width_default(const int colWidthDefault) override;
    void set_col_width(const int colWidth, std::optional<size_t> optColIdx = std::nullopt) override;

    void grab_focus() const override;

protected:
    void _reset(CtTableMatrix& tableMatrix);
    static void _free_matrix(CtTableMatrix& tableMatrix);

    void _populate_xml_rows_cells(xmlpp::Element* p_table_node) const override;
    bool _row_sort(const bool sortAsc) override;

    bool _on_entry_focus_out_event(GdkEventFocus* gdk_event, Gtk::Entry* pEntry, const Glib::ustring& path, const size_t column);
    void _on_cell_renderer_text_edited(const Glib::ustring& path, const Glib::ustring& new_text, const size_t column);
    void _on_cell_renderer_editing_started(Gtk::CellEditable* editable, const Glib::ustring& path, const size_t column);

    std::unique_ptr<CtTableLightColumns> _pColumns;
    Gtk::TreeView* _pManagedTreeView{nullptr};
    Glib::RefPtr<Gtk::ListStore> _pListStore;
};

class CtTableHeavy : public CtTableCommon
{
public:
    CtTableHeavy(CtMainWin* pCtMainWin,
            CtTableMatrix& tableMatrix,
            const int colWidthDefault,
            const int charOffset,
            const std::string& justification,
            const CtTableColWidths& colWidths,
            const size_t currRow = 0,
            const size_t currCol = 0);
    ~CtTableHeavy() override;

    void apply_syntax_highlighting(const bool forceReApply) override;
    std::string to_csv() const override;
    void set_modified_false() override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::TableHeavy; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    CtTextView& curr_cell_text_view() const;

    void write_strings_matrix(std::vector<std::vector<Glib::ustring>>& rows) const override;
    size_t get_num_rows() const override { return _tableMatrix.size(); }
    size_t get_num_columns() const override { return _tableMatrix.front().size(); }

    void column_add(const size_t afterColIdx) override;
    void column_delete(const size_t colIdx) override;
    void column_move_left(const size_t colIdx) override;
    void column_move_right(const size_t colIdx) override;
    void row_add(const size_t afterRowIdx, const std::vector<Glib::ustring>* pNewRow = nullptr) override;
    void row_delete(const size_t rowIdx) override;
    void row_move_up(const size_t rowIdx) override;
    void row_move_down(const size_t rowIdx) override;

    void set_col_width_default(const int colWidthDefault) override;
    void set_col_width(const int colWidth, std::optional<size_t> optColIdx = std::nullopt) override;

    void grab_focus() const override;

protected:
    void _apply_styles_to_cells(const bool forceReApply);
    void _new_text_cell_attach(const size_t rowIdx, const size_t colIdx, CtTextCell* pTextCell);
    void _apply_remove_header_style(const bool isApply, CtTextView& textView);

    bool _row_sort(const bool sortAsc) override;
    void _populate_xml_rows_cells(xmlpp::Element* p_table_node) const override;

    void _on_grid_set_focus_child(Gtk::Widget* pWidget);

protected:
    CtTableMatrix    _tableMatrix;
    Gtk::Grid        _grid;
};
