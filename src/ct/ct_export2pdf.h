/*
 * ct_export2pdf.h
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
struct CtPangoText : public CtPangoObject
{
    CtPangoText(const Glib::ustring& text, const Glib::ustring& synt_highl, int indent) : text(text), synt_highl(synt_highl), indent(indent) {}
    Glib::ustring text;
    Glib::ustring synt_highl;
    int           indent;  // paragraph indent
};
struct CtPangoLink : public CtPangoText
{
    CtPangoLink(const Glib::ustring& text, int indent, const Glib::ustring& link) : CtPangoText(text, CtConst::RICH_TEXT_ID, indent), link(link) {}
    Glib::ustring link;
};
struct CtPangoDest : public CtPangoText
{
    CtPangoDest(const Glib::ustring& text, const Glib::ustring& synt_highl, int indent, const Glib::ustring& dest) : CtPangoText(text, synt_highl, indent), dest(dest) {}
    Glib::ustring dest;
};
struct CtPangoWidget : public CtPangoObject
{
    CtPangoWidget(CtAnchoredWidget* widget, int indent) : widget(widget), indent(indent) {}
    CtAnchoredWidget* widget;
    int               indent;
};
using CtPangoObjectPtr = std::shared_ptr<CtPangoObject>;



class CtExport2Pango
{
public:
    CtExport2Pango(CtMainWin* pCtMainWin) : _pCtMainWin{pCtMainWin} {}

    Glib::ustring pango_get_from_code_buffer(Glib::RefPtr<Gsv::Buffer> code_buffer,
                                             int sel_start,
                                             int sel_end);
    void pango_get_from_treestore_node(CtTreeIter node_iter,
                                       int sel_start,
                                       int sel_end,
                                       std::vector<CtPangoObjectPtr>& out_slots);

private:
    void                         _pango_process_slot(int start_offset,
                                                     int end_offset,
                                                     Glib::RefPtr<Gtk::TextBuffer> curr_buffer,
                                                     std::vector<CtPangoObjectPtr>& out_slots);
    void                         _pango_text_serialize(const Gtk::TextIter& start_iter,
                                                       Gtk::TextIter end_iter,
                                                       const CtCurrAttributesMap& curr_attributes,
                                                       std::vector<CtPangoObjectPtr>& out_slots);
    std::shared_ptr<CtPangoText> _pango_link_url(const Glib::ustring& tagged_text, const Glib::ustring& link, int indent);

private:
    CtMainWin* const _pCtMainWin;
};



class CtExport2Pdf
{
public:
    CtExport2Pdf(CtMainWin* pCtMainWin) : _pCtMainWin{pCtMainWin} {}

    void node_export_print(const fs::path& pdf_filepath,
                           CtTreeIter tree_iter,
                           const CtExportOptions& options,
                           int sel_start,
                           int sel_end);
    void node_and_subnodes_export_print(const fs::path& pdf_filepath,
                                        CtTreeIter tree_iter,
                                        const CtExportOptions& options);
    void tree_export_print(const fs::path& pdf_filepath,
                           CtTreeIter tree_iter,
                           const CtExportOptions& options);

private:
    void             _nodes_all_export_print_iter(CtTreeIter tree_iter,
                                                  const CtExportOptions& options,
                                                  std::vector<CtPangoObjectPtr>& tree_pango_slots);
    CtPangoObjectPtr _generate_pango_node_name(CtTreeIter tree_iter);

private:
    CtMainWin* const _pCtMainWin;
};



struct CtPageElement
{
    CtPageElement(int x) : x(x) {}
    virtual ~CtPageElement() = default;
    int x;
};
struct CtPageText : public CtPageElement
{
    CtPageText(int x, Glib::RefPtr<Pango::LayoutLine> layout_line) : CtPageElement(x), layout_line(layout_line) {}
    Glib::RefPtr<Pango::LayoutLine> layout_line;
};
struct CtPageTag : public CtPageElement
{
    CtPageTag(int x, Glib::RefPtr<Pango::LayoutLine> layout_line, const std::string& tag_name, const std::string& tag_attr)
        : CtPageElement(x), layout_line(layout_line), tag_name(tag_name), tag_attr(tag_attr) {}
    Glib::RefPtr<Pango::LayoutLine> layout_line;
    std::string tag_name;
    std::string tag_attr;
};
struct CtPageImage : public CtPageElement
{
    CtPageImage(int x, CtImage* image, double scale) : CtPageElement(x), image(image), scale(scale) {}
    CtImage* image;
    double   scale;
};
struct CtPageCodebox : public CtPageElement
{
    CtPageCodebox(int x, Glib::RefPtr<Pango::Layout> layout) : CtPageElement(x), layout(layout) {}
    Glib::RefPtr<Pango::Layout> layout;
};
struct CtPageTable : public CtPageElement
{
    using TableLayouts = std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>;
    CtPageTable(const int x, const TableLayouts layouts, const CtTableColWidths& col_widths, const double page_dpi_scale)
     : CtPageElement{x}
     , layouts{layouts}
    {
        for (auto col_width : col_widths) {
            colWidths.push_back(col_width*page_dpi_scale);
        }
    }
    TableLayouts layouts;
    CtTableColWidths colWidths;
};
using CtPageElementPtr = std::shared_ptr<CtPageElement>;


struct CtPrintPages
{
    struct CtPageLine
    {
        CtPageLine(int y) : y(y) {};

        bool test_element_height(int el_height, int page_height) { return (y - height) + el_height <= page_height; }
        void set_height(int el_height) { if (el_height > height) { y = y - height + el_height; height = el_height; }}

        std::vector<CtPageElementPtr> elements;
        int height {0}; // height of the line
        int y {0};      // bottom y of the line
        int cur_x {0};
    };

    struct CtPrintPage
    {
        std::vector<CtPageLine> lines {CtPageLine(0)};
    };

public:
    int          size()          { return (int)_pages.size(); }
    CtPrintPage& get_page(int i) { return _pages[i]; }
    CtPrintPage& last_page()     { return _pages.back(); }
    CtPageLine&  last_line()     { return _pages.back().lines.back(); }
    void         new_page()      { _pages.emplace_back(CtPrintPage());  }
    void         new_line()      { last_page().lines.emplace_back(CtPageLine(last_line().y + 2)); }
    void         line_on_new_page() {
        CtPageLine line = last_line();
        new_page();
        last_line() = line;
        last_line().y = last_line().height;
    }

private:
    std::vector<CtPrintPage> _pages {CtPrintPage()};
};


// Print Operation Data
struct CtPrintData
{
    std::vector<CtPangoObjectPtr>      slots;

    Glib::RefPtr<Gtk::PrintOperation>  operation;
    Glib::RefPtr<Gtk::PrintContext>    context;

    CtPrintPages                       pages;
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
    void _process_pango_text(CtPrintData* print_data, CtPangoText* text_slot);
    void _process_pango_image(CtPrintData* print_data, CtImage* image, int indent, bool& any_image_resized);
    void _process_pango_codebox(CtPrintData* print_data, CtCodebox* codebox, int indent);
    void _process_pango_table(CtPrintData* print_data, CtTable* table, int indent);

    Glib::RefPtr<Pango::Layout> _codebox_get_layout(CtCodebox* codebox, Glib::ustring content, Glib::RefPtr<Gtk::PrintContext> context);
    void                        _codebox_split_content(CtCodebox* codebox, Glib::ustring original_content, const int check_height, const Glib::RefPtr<Gtk::PrintContext>& context,
                                                       Glib::ustring& first_split, Glib::ustring& second_split);

    CtPageTable::TableLayouts   _table_get_layouts(CtTable* table, const int first_row, const int last_row, const Glib::RefPtr<Gtk::PrintContext>& context);
    void                        _table_get_grid(const CtPageTable::TableLayouts& table_layouts,
                                                const CtTableColWidths& col_widths,
                                                std::vector<double>& rows_h,
                                                std::vector<double>& cols_w);
    double                      _table_get_width_height(std::vector<double>& data);
    int                         _table_split_content(CtTable* table,
                                                     const int start_row,
                                                     const int check_height,
                                                     const Glib::RefPtr<Gtk::PrintContext>& context);

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
    double                           _page_dpi_scale;
    double                           _page_width;
    double                           _page_height;
};
