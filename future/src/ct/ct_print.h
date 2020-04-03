/*
 * ct_print.h
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

#include "ct_main_win.h"
#include <iterator>

// base class for proxy
class CtPrintWidgetProxy
{
public:
    virtual ~CtPrintWidgetProxy() {}
};

// proxy to keep pixbuf
class CtPrintImageProxy : public CtPrintWidgetProxy
{
public:
    CtPrintImageProxy(CtImage* image) : _image(image) {}
    CtImage*                  get_image()  { return _image; }
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf() { return _image->get_pixbuf(); }

private:
    CtImage* _image;
};

// proxy to split tables
class CtPrintTableProxy : public CtPrintWidgetProxy
{
public:
    CtPrintTableProxy(CtTable* table, int startRow, int rowNum): _table(table), _startRow(startRow), _rowNum(rowNum) {}

    std::shared_ptr<CtPrintTableProxy> create_new_with(int row_num) { return std::shared_ptr<CtPrintTableProxy>(new CtPrintTableProxy(_table, _startRow, row_num)); }

    void     remove_first_rows(int remove_row_num) { _startRow += remove_row_num; _rowNum -= remove_row_num; }
    CtTable* get_table()                    { return _table; }
    int      get_row_num()                  { return _rowNum; }
    int      get_col_num()                  { return (int)_table->get_table_matrix().begin()->size(); }
    // todo: it can work slow because of moving iterators every time; to fix: replace list by vector or use cache
    Glib::ustring get_cell(int row, int col) {
        // 0 row is always header row, 1 row starts from _startRow
        row = (row == 0) ? 0 : row - 1 + _startRow;
        auto tableRow = _table->get_table_matrix().begin();
        std::advance(tableRow, row);
        auto tableCell = tableRow->begin();
        std::advance(tableCell, col);
        return (*tableCell)->get_text_content();
    }

private:
    CtTable* _table;
    int      _startRow;  // never starts from header row (because proxies for the same table have the same header)
    int      _rowNum;    // includes header row

};

// proxy to split codebox
class CtPrintCodeboxProxy : public CtPrintWidgetProxy
{
public:
    CtPrintCodeboxProxy(CtCodebox* codebox) : _codebox(codebox), _use_proxy_text(false) {}
    CtPrintCodeboxProxy(CtCodebox* codebox, const Glib::ustring& proxy_text) : _codebox(codebox),_proxy_text(proxy_text), _use_proxy_text(true)  {}
    CtCodebox*          get_codebox()          { return _codebox; }
    bool                get_width_in_pixels()  { return _codebox->get_width_in_pixels(); }
    int                 get_frame_width()      { return _codebox->get_frame_width(); }
    const Glib::ustring get_text_content()     { return _use_proxy_text ? _proxy_text : pango_from_code_buffer(_codebox); }
    void                set_proxy_content(const Glib::ustring& text) { _proxy_text = text; _use_proxy_text = true; }

    Glib::ustring       pango_from_code_buffer(CtCodebox* codebox); // couldn't use CtExport2Pango in .h, so created helper function

private:
    CtCodebox*    _codebox;
    Glib::ustring _proxy_text;
    bool          _use_proxy_text;
};

// proxy for nullptr and others
class CtPrintSomeProxy : public CtPrintWidgetProxy
{
public:
    CtPrintSomeProxy(CtAnchoredWidget*) {}
};


// Print Operation Data
struct CtPrintData
{
    Glib::RefPtr<Gtk::PrintOperation>               operation;
    std::vector<Glib::ustring>                      text;
    std::vector<Glib::RefPtr<Pango::Layout>>        layout;
    std::vector<bool>                               forced_page_break;
    std::vector<bool>                               layout_is_new_line;
    std::vector<int>                                layout_num_lines;
    std::vector<std::pair<int, int>>                page_breaks;
    std::vector<double>                             all_lines_y;
    Glib::ustring                                   warning;
};



class CtPrint
{
public:
    const int BOX_OFFSET = 4;

public:
    CtPrint();

public:
    void run_page_setup_dialog(Gtk::Window* pMainWin);

    void print_text(CtMainWin* pCtMainWin, const Glib::ustring& pdf_filepath,
                    const std::vector<Glib::ustring>& pango_text, const Glib::ustring& text_font, const Glib::ustring& code_font,
                    const std::list<CtAnchoredWidget*>& widgets, int text_window_width);

private:
    void _on_begin_print_text(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data);
    void _on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data);

private:
    Cairo::Rectangle            _layout_line_get_width_height(Glib::RefPtr<const Pango::LayoutLine> line);
    double                      _get_height_from_layout(Glib::RefPtr<Pango::Layout> layout);
    double                      _get_width_from_layout(Glib::RefPtr<Pango::Layout> layout);
    Glib::RefPtr<Pango::Layout> _get_codebox_layout(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintCodeboxProxy* codeboxProxy);
    std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>
                                _get_table_layouts(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintTableProxy* tableProxy);
    std::pair<std::vector<double>, std::vector<double>>
                                _get_table_grid(std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>& table_layouts, int col_min);
    double                      _get_table_width_from_grid(std::pair<std::vector<double>, std::vector<double>>& table_grid);
    double                      _get_table_height_from_grid(std::pair<std::vector<double>, std::vector<double>>& table_grid);
    void                        _table_long_split(size_t idx, const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data);
    void                        _codebox_long_split(size_t idx, const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data);

    void _codebox_draw_box(Cairo::RefPtr<Cairo::Context> cairo_context, double x0, double y0, double codebox_width, double codebox_height);
    void _codebox_draw_code(Cairo::RefPtr<Cairo::Context> cairo_context, Glib::RefPtr<Pango::Layout> codebox_layout, double x0, double y0);
    void _table_draw_grid(Cairo::RefPtr<Cairo::Context> cairo_context, const std::pair<std::vector<double>, std::vector<double>>& table_grid,
                          double x0, double y0, double table_width, double table_height);
    void _table_draw_text(Cairo::RefPtr<Cairo::Context> cairo_context,
                          const std::pair<std::vector<double>, std::vector<double>>& table_grid,
                          const std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>& table_layouts,
                          double x0, double y0);

private:
    std::vector<std::shared_ptr<CtPrintWidgetProxy>> _widgets;

    CtMainWin*                       _pCtMainWin;
    Glib::RefPtr<Gtk::PrintSettings> _pPrintSettings;
    Glib::RefPtr<Gtk::PageSetup>     _pPageSetup;
    Pango::FontDescription           _pango_font;
    Pango::FontDescription           _codebox_font;
    int                              _text_window_width;
    int                              _table_text_row_height;
    int                              _table_line_thickness;
    double                           _layout_newline_height;
    double                           _page_width;
    double                           _page_height;
    double                           _y_idx;
};


