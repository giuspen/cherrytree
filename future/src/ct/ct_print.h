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

// helper to split tables and codebox
class CtPrintWidgetProxy
{
public:
    virtual ~CtPrintWidgetProxy() {}
};

class CtPrintImageProxy : public CtPrintWidgetProxy
{
public:
    CtPrintImageProxy(CtImage* image) : _image(image) {}
    CtImage* get_image() { return _image; }

private:
    CtImage* _image;
};

// helper to split tables
class CtPrintTableProxy : public CtPrintWidgetProxy
{
public:
    CtPrintTableProxy(CtTable* table, int startRow, int rowNum): _table(table), _startRow(startRow), _rowNum(rowNum) {}

    std::shared_ptr<CtPrintTableProxy> copy(int row_num) { return std::shared_ptr<CtPrintTableProxy>(new CtPrintTableProxy(_table, _startRow, row_num)); }
    void remove_first_rows(int row_num) { _startRow + row_num; }    CtTable* get_table() { return _table; }
    int  get_row_num()                  { return _rowNum + 1 /* header */; }
    int  get_col_num()                  { return (int)_table->get_table_matrix().begin()->size(); }
    // todo: it works quite slow because of moving iterators every time, to fix: replace list by vector
    Glib::ustring get_cell(int row, int col) {
        row = (row == 0) ? row : row - 1 + _startRow;
        auto tableRow = _table->get_table_matrix().begin();
        std::advance(tableRow, row);
        auto tableCell = tableRow->begin();
        std::advance(tableCell, col);
        return (*tableCell)->get_text_content();
    }

private:
    CtTable* _table;
    int _startRow; // can be -1;
    int _rowNum;   // can be -1; don't include header row

};

// helper to split codebox
class CtPrintCodeboxProxy : public CtPrintWidgetProxy
{
public:
    CtPrintCodeboxProxy(CtCodebox* codebox, const Glib::ustring& proxy_text) : _codebox(codebox), _proxy_text(proxy_text) {}
    CtCodebox*          get_codebox()          { return _codebox; }
    bool                get_width_in_pixels()  { return _codebox->get_width_in_pixels(); }
    int                 get_frame_width()      { return _codebox->get_frame_width(); }
    const Glib::ustring get_text_content()     { return _proxy_text.empty() ? _codebox->get_text_content() : _proxy_text; }
    void                set_text_contenxt(const Glib::ustring& text) { _proxy_text = text; }

private:
    CtCodebox* _codebox;
    Glib::ustring _proxy_text;
};

class CtPrintSomeProxy : public CtPrintWidgetProxy
{
public:
    CtPrintSomeProxy(CtAnchoredWidget* widget) : _widget(widget) {}

private:
    CtAnchoredWidget* _widget;
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
    std::map<CtImage*, Glib::RefPtr<Gdk::Pixbuf>>   modified_pixbuf;
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
    std::pair<double, double>   _layout_line_get_width_height(Glib::RefPtr<const Pango::LayoutLine> line);
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


