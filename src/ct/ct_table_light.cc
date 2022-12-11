/*
 * ct_table_light.cc
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

#include "ct_table.h"
#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_storage_sqlite.h"
#include "ct_storage_xml.h"
#include "ct_logging.h"
#include "ct_misc_utils.h"

/*static*/void CtTableLight::_free_matrix(CtTableMatrix& tableMatrix)
{
    for (CtTableRow& tableRow : tableMatrix) {
        for (void* pText : tableRow) {
            delete static_cast<Glib::ustring*>(pText);
            pText = nullptr;
        }
    }
}

CtTableLight::CtTableLight(CtMainWin* pCtMainWin,
                           CtTableMatrix& tableMatrix,
                           const int colWidthDefault,
                           const int charOffset,
                           const std::string& justification,
                           const CtTableColWidths& colWidths,
                           const size_t currRow,
                           const size_t currCol)
 : CtTableCommon{pCtMainWin, colWidthDefault, charOffset, justification, colWidths, currRow, currCol}
{
    const size_t numRows{tableMatrix.size()};
    size_t numColumns{0u};
    // enforce same number of columns per row
    for (size_t r = 0u; r < numRows; ++r) {
        if (tableMatrix[r].size() > numColumns) { numColumns = tableMatrix[r].size(); }
    }
    for (size_t r = 0u; r < numRows; ++r) {
        while (tableMatrix[r].size() < numColumns) { tableMatrix[r].push_back(new Glib::ustring{}); }
    }

    // column widths can be empty or wrong, trying to fix it
    // so we don't need to check it again and again
    while (_colWidths.size() < numColumns) {
        _colWidths.push_back(0); // 0 means we use default width
    }

    _pColumns.reset(new CtTableLightColumns{numColumns});
    _pListStore = Gtk::ListStore::create(*_pColumns);

    for (size_t r = 0u; r < numRows; ++r) {
        Gtk::TreeModel::Row row = *(_pListStore->append());
        row[_pColumns->columnWeight] = CtTreeIter::get_pango_weight_from_is_bold(0u == r);
        for (size_t c = 0u; c < numColumns; ++c) {
            row[_pColumns->columnsText.at(c)] = *static_cast<Glib::ustring*>(tableMatrix.at(r).at(c));
        }
    }
    _free_matrix(tableMatrix);

    _pManagedTreeView = Gtk::manage(new Gtk::TreeView{_pListStore});
    _pManagedTreeView->set_headers_visible(false);
    for (size_t c = 0u; c < numColumns; ++c) {
        const int width = get_col_width(c);
        _pManagedTreeView->append_column_editable(""/*header*/, _pColumns->columnsText.at(c));
        Gtk::TreeViewColumn* pTVColumn = _pManagedTreeView->get_column(c);
        if (pTVColumn) {
            auto pCellRendererText = static_cast<Gtk::CellRendererText*>(pTVColumn->get_first_cell());
            pTVColumn->add_attribute(pCellRendererText->property_weight(), _pColumns->columnWeight);
            pCellRendererText->property_wrap_width() = width;
            pCellRendererText->property_wrap_mode() = Pango::WrapMode::WRAP_WORD_CHAR;
            pTVColumn->property_sizing() = Gtk::TREE_VIEW_COLUMN_AUTOSIZE;
            pTVColumn->property_min_width() = width/2;
        }
    }
    _pManagedTreeView->get_style_context()->add_class("ct-table-light");

    _frame.add(*_pManagedTreeView);
    show_all();
}

void CtTableLight::write_strings_matrix(std::vector<std::vector<Glib::ustring>>& rows) const
{
    rows.reserve(get_num_rows());
    auto f_action = [&](const Gtk::TreeIter& treeIter)->bool{
        Gtk::TreeRow treeRow = *treeIter;
        rows.push_back(std::vector<Glib::ustring>{});
        const size_t numCols = get_num_columns();
        rows.back().reserve(numCols);
        const CtTableLightColumns& cols = get_columns();
        for (size_t c = 0u; c < numCols; ++c) {
            rows.back().push_back(treeRow[cols.columnsText.at(c)]);
        }
        return false; /* to continue */
    };
    _pListStore->foreach_iter(f_action);
}

void CtTableLight::_populate_xml_rows_cells(xmlpp::Element* p_table_node) const
{
    // put header at the end
    Gtk::TreeIter headerIter;
    bool is_header{true};
    auto f_action = [&](const Gtk::TreeIter& treeIter)->bool{
        if (is_header) {
            is_header = false;
            headerIter = treeIter;
        }
        else {
            Gtk::TreeRow treeRow = *treeIter;
            xmlpp::Element* p_row_node = p_table_node->add_child("row");
            const size_t numCols = get_num_columns();
            const CtTableLightColumns& cols = get_columns();
            for (size_t c = 0u; c < numCols; ++c) {
                xmlpp::Element* p_cell_node = p_row_node->add_child("cell");
                p_cell_node->add_child_text(treeRow[cols.columnsText.at(c)]);
            }
        }
        return false; /* to continue */
    };
    _pListStore->foreach_iter(f_action);
    if (not is_header) f_action(headerIter);
}

std::shared_ptr<CtAnchoredWidgetState> CtTableLight::get_state()
{
    return std::shared_ptr<CtAnchoredWidgetState>(new CtAnchoredWidgetState_TableLight{this});
}
