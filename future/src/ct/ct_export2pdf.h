/*
 * ct_export2pdf.h
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
#include "ct_dialogs.h"
#include <iterator>


struct CtPangoObject
{
    virtual ~CtPangoObject() = default;
};
struct CtPangoNewPage : public CtPangoObject
{

};
struct CtPangoWord : public CtPangoObject
{
    CtPangoWord(const Glib::ustring& text, const Glib::ustring& synt_highl) : text(text), synt_highl(synt_highl) {}
    Glib::ustring text;
    Glib::ustring synt_highl;
};
struct CtPangoLink : public CtPangoWord
{
    CtPangoLink(const Glib::ustring& text, const Glib::ustring& link) : CtPangoWord(text, CtConst::RICH_TEXT_ID), link(link) {}
    Glib::ustring link;
};
struct CtPangoDest : public CtPangoWord
{
    CtPangoDest(const Glib::ustring& text, const Glib::ustring& synt_highl, const Glib::ustring& dest) : CtPangoWord(text, synt_highl), dest(dest) {}
    Glib::ustring dest;
};
struct CtPangoWidget : public CtPangoObject
{
    CtPangoWidget(CtAnchoredWidget* widget) : widget(widget) {}
    CtAnchoredWidget* widget;
};
using CtPangoObjectPtr = std::shared_ptr<CtPangoObject>;



class CtExport2Pango
{
public:
    Glib::ustring pango_get_from_code_buffer(Glib::RefPtr<Gsv::Buffer> code_buffer, int sel_start, int sel_end);
    void pango_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end, std::vector<CtPangoObjectPtr>& out_slots);

private:
    std::vector<CtPangoObjectPtr> _pango_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer);
    std::shared_ptr<CtPangoWord>  _pango_text_serialize(const Gtk::TextIter& start_iter, Gtk::TextIter end_iter, const std::map<std::string_view, std::string> &curr_attributes);
    std::shared_ptr<CtPangoWord>  _pango_link(const Glib::ustring& pango_text, const Glib::ustring& url);
};



class CtExport2Pdf
{
public:
    CtExport2Pdf(CtMainWin* pCtMainWin) { _pCtMainWin = pCtMainWin; }

    void node_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options, int sel_start, int sel_end);
    void node_and_subnodes_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options);
    void tree_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options);

private:
    void             _nodes_all_export_print_iter(CtTreeIter tree_iter, const CtExportOptions& options, std::vector<CtPangoObjectPtr>& tree_pango_slots);
    CtPangoObjectPtr _generate_pango_node_name(CtTreeIter tree_iter);

private:
    CtMainWin* _pCtMainWin;
};



struct CtPageElement
{
    CtPageElement(int x, int y) : x(x), y(y) {}
    virtual ~CtPageElement() = default;
    int x;
    int y;
};
struct CtPageWord : public CtPageElement
{
    CtPageWord(int x, int y, Glib::RefPtr<Pango::LayoutLine> layout_line) : CtPageElement(x, y), layout_line(layout_line) {}
    Glib::RefPtr<Pango::LayoutLine> layout_line;
};
struct CtPageTagWord : public CtPageElement
{
    CtPageTagWord(int x, int y, Glib::RefPtr<Pango::LayoutLine> layout_line, const std::string& tag_name, const std::string& tag_attr)
        : CtPageElement(x, y), layout_line(layout_line), tag_name(tag_name), tag_attr(tag_attr) {}
    Glib::RefPtr<Pango::LayoutLine> layout_line;
    std::string tag_name;
    std::string tag_attr;
};
struct CtPageImage : public CtPageElement
{
    CtPageImage(int x, int y, CtImage* image, double scale) : CtPageElement(x, y), image(image), scale(scale) {}
    CtImage* image;
    double   scale;
};
struct CtPageCodebox : public CtPageElement
{
    CtPageCodebox(int x, int y, Glib::RefPtr<Pango::Layout> layout) : CtPageElement(x, y), layout(layout) {}
    Glib::RefPtr<Pango::Layout> layout;
};
struct CtPageTable : public CtPageElement
{
    using TableLayouts = std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>;
    CtPageTable(int x, int y, TableLayouts layouts, int col_min) : CtPageElement(x, y), layouts(layouts), col_min(col_min) {}
    TableLayouts layouts;
    int          col_min;
};
using CtPageElementPtr = std::shared_ptr<CtPageElement>;


struct CtPrintPage
{
    std::vector<CtPageElementPtr> elements;
    int cur_x {0};
    int cur_y {0};
};

// Print Operation Data
struct CtPrintData
{
    std::vector<CtPangoObjectPtr>      slots;

    Glib::RefPtr<Gtk::PrintOperation>  operation;
    Glib::RefPtr<Gtk::PrintContext>    context;

    std::vector<CtPrintPage>           pages;
    Glib::ustring                      warning;
};


class CtPrint
{
public:
    const int BOX_OFFSET = 4;
    const int LINE_SPACE_OFFSET = 2;

public:
    CtPrint();

public:
    void run_page_setup_dialog(Gtk::Window* pMainWin);
    void print_text(CtMainWin* pCtMainWin, const fs::path& pdf_filepath, const std::vector<CtPangoObjectPtr>& slots);

private:
    void _on_begin_print_text(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data);
    void _on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data);

private:
    void _process_pango_text(CtPrintData* print_data, CtPangoWord* text_slot);
    void _process_pango_image(CtPrintData* print_data, CtImage* image, bool& any_image_resized);
    void _process_pango_codebox(CtPrintData* print_data, CtCodebox* codebox);
    void _process_pango_table(CtPrintData* print_data, CtTable* table);

    Glib::RefPtr<Pango::Layout> _codebox_get_layout(CtCodebox* codebox, Glib::ustring content, Glib::RefPtr<Gtk::PrintContext> context);
    void                        _codebox_split_content(CtCodebox* codebox, Glib::ustring original_content, const int check_height, const Glib::RefPtr<Gtk::PrintContext>& context,
                                                       Glib::ustring& first_split, Glib::ustring& second_split);

    CtPageTable::TableLayouts   _table_get_layouts(CtTable* table, const int first_row, const int last_row, const Glib::RefPtr<Gtk::PrintContext>& context);
    void                        _table_get_grid(const CtPageTable::TableLayouts& table_layouts, const int col_min, std::vector<double>& rows_h, std::vector<double>& cols_w);
    double                      _table_get_width_height(std::vector<double>& data);
    int                         _table_split_content(CtTable* table, const int start_row, const int check_height, const Glib::RefPtr<Gtk::PrintContext>& context);

    void _draw_codebox_box(Cairo::RefPtr<Cairo::Context> cairo_context, double x0, double y0, double codebox_width, double codebox_height);
    void _draw_codebox_code(Cairo::RefPtr<Cairo::Context> cairo_context, Glib::RefPtr<Pango::Layout> codebox_layout, double x0, double y0);
    void _draw_table_grid(Cairo::RefPtr<Cairo::Context> cairo_context, const std::vector<double>& rows_h, const std::vector<double>& cols_w,
                          double x0, double y0, double table_width, double table_height);
    void _draw_table_text(Cairo::RefPtr<Cairo::Context> cairo_context, const std::vector<double>& rows_h, const std::vector<double>& cols_w,
                          const CtPageTable::TableLayouts& table_layouts, double x0, double y0);

    Cairo::Rectangle _get_width_height_from_layout_line(Glib::RefPtr<const Pango::LayoutLine> line);
    double           _get_height_from_layout(Glib::RefPtr<Pango::Layout> layout);
    double           _get_width_from_layout(Glib::RefPtr<Pango::Layout> layout);

private:
    CtMainWin*                       _pCtMainWin;
    Glib::RefPtr<Gtk::PrintSettings> _pPrintSettings;
    Glib::RefPtr<Gtk::PageSetup>     _pPageSetup;

    Pango::FontDescription           _rich_font;
    Pango::FontDescription           _plain_font;
    Pango::FontDescription           _code_font;
    int                              _text_window_width;

    int                              _table_text_row_height;
    int                              _table_line_thickness;
    double                           _layout_newline_height;
    double                           _page_width;
    double                           _page_height;
};
