/*
 * ct_export2pdf.h
 *
 * Copyright 2009-2024
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
    CtPangoText(const Glib::ustring& text_,
                const Glib::ustring& synt_highl_,
                const int indent_,
                const PangoDirection pango_dir_)
     : text{text_}
     , synt_highl{synt_highl_}
     , indent{indent_}
     , pango_dir{pango_dir_} {}
    const Glib::ustring     text;
    const Glib::ustring     synt_highl;
    const int               indent{0};
    const PangoDirection    pango_dir{PANGO_DIRECTION_NEUTRAL};
};

struct CtPangoLink : public CtPangoText
{
    CtPangoLink(const Glib::ustring& text,
                const int indent,
                const Glib::ustring& link_,
                const PangoDirection pango_dir)
     : CtPangoText{text, CtConst::RICH_TEXT_ID, indent, pango_dir}
     , link{link_} {}
    const Glib::ustring link;
};

struct CtPangoDest : public CtPangoText
{
    CtPangoDest(const Glib::ustring& text,
                const Glib::ustring& synt_highl,
                const int indent,
                const Glib::ustring& dest_,
                const PangoDirection pango_dir)
     : CtPangoText{text, synt_highl, indent, pango_dir}
     , dest{dest_} {}
    const Glib::ustring dest;
};

struct CtPangoWidget : public CtPangoObject
{
    CtPangoWidget(const CtAnchoredWidget* widget_,
                  const int indent_,
                  const PangoDirection pango_dir_)
     : widget{widget_}
     , indent{indent_}
     , pango_dir{pango_dir_} {}
    const CtAnchoredWidget* widget{nullptr};
    const int               indent{0};
    const PangoDirection    pango_dir{PANGO_DIRECTION_NEUTRAL};
};

using CtPangoObjectPtr = std::shared_ptr<CtPangoObject>;

class CtExport2Pango
{
public:
    CtExport2Pango(CtMainWin* pCtMainWin);

    Glib::ustring pango_get_from_code_buffer(Glib::RefPtr<Gtk::TextBuffer> code_buffer,
                                             int sel_start,
                                             int sel_end,
                                             const std::string& syntax_highlighting);
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
    std::shared_ptr<CtPangoText> _pango_link_url(const Glib::ustring& tagged_text, const Glib::ustring& link, const int indent, const PangoDirection pango_dir);

private:
    CtMainWin* const _pCtMainWin;
    const CtConfig* const _pCtConfig;
};

class CtExport2Pdf
{
public:
    CtExport2Pdf(CtMainWin* pCtMainWin)
     : _pCtMainWin{pCtMainWin} {}

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
    CtPageElement(const int x_)
     : x{x_} {}
    virtual ~CtPageElement() = default;
    int x;
};

struct CtPageText : public CtPageElement
{
    CtPageText(const int x_,
               Glib::RefPtr<Pango::LayoutLine> layout_line_)
     : CtPageElement{x_}
     , layout_line{layout_line_} {}
    Glib::RefPtr<Pango::LayoutLine> layout_line;
};

struct CtPageTag : public CtPageElement
{
    CtPageTag(const int x_,
              Glib::RefPtr<Pango::LayoutLine> layout_line_,
              const std::string& tag_name_,
              const std::string& tag_attr_)
        : CtPageElement{x_}
        , layout_line{layout_line_}
        , tag_name{tag_name_}
        , tag_attr(tag_attr_) {}
    Glib::RefPtr<Pango::LayoutLine> layout_line;
    std::string tag_name;
    std::string tag_attr;
};

struct CtPageImage : public CtPageElement
{
    CtPageImage(const int x_,
                const CtImage* image_,
                const double scale_)
     : CtPageElement{x_}
     , image{image_}
     , scale{scale_} {}
    const CtImage* image;
    const double   scale;
};

struct CtPageCodebox : public CtPageElement
{
    CtPageCodebox(const int x_,
                  Glib::RefPtr<Pango::Layout> layout_)
     : CtPageElement{x_}
     , layout{layout_} {}
    Glib::RefPtr<Pango::Layout> layout;
};

struct CtPageTable : public CtPageElement
{
    using TableLayouts = std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>;
    CtPageTable(const int x_,
                const TableLayouts layouts_,
                const CtTableColWidths& col_widths,
                const double page_dpi_scale)
     : CtPageElement{x_}
     , layouts{layouts_}
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
        CtPageLine(const int y_)
         : y{y_} {}

        bool test_element_height(int el_height, int page_height) {
            return (y - max_height) + el_height <= page_height;
        }
        void set_max_height(int el_height) {
            if (el_height > max_height) {
                y = y - max_height + el_height;
                max_height = el_height;
            }
        }

        std::vector<CtPageElementPtr> elements;
        int max_height{0}; // height of the line
        int y{0};      // bottom y of the line
        int cur_x{-1};
        bool evaluated_pango_dir{false};
        PangoDirection pango_dir{PANGO_DIRECTION_NEUTRAL};
        int  changed_rtl_in_line_prev_x{-1};
    };

    struct CtPrintPage
    {
        std::vector<CtPageLine> lines{CtPageLine{0}};
    };

public:
    int          size()          { return static_cast<int>(_pages.size()); }
    CtPrintPage& get_page(int i) { return _pages[i]; }
    CtPrintPage& last_page()     { return _pages.back(); }
    CtPageLine&  last_line()     { return _pages.back().lines.back(); }
    void         new_page()      { _pages.emplace_back(CtPrintPage{});  }
    void         new_line()      { last_page().lines.emplace_back(CtPageLine{last_line().y + 2}); }
    void         line_on_new_page() {
        CtPageLine line = last_line();
        new_page();
        last_line() = line;
        last_line().y = last_line().max_height;
    }

private:
    std::vector<CtPrintPage> _pages{CtPrintPage{}};
};

// Print Operation Data
struct CtPrintData
{
    std::vector<CtPangoObjectPtr>      slots;

    Glib::RefPtr<Gtk::PrintOperation>  operation;
    Glib::RefPtr<Gtk::PrintContext>    context;

    CtPrintPages                       pages;
    Glib::ustring                      warning;

    std::list<Glib::ustring>           cairo_names;
};

class CtPrint
{
public:
    const int BOX_OFFSET{4};
    const int LINE_SPACE_OFFSET{2};

public:
    CtPrint(CtMainWin* pCtMainWin);

public:
    void run_page_setup_dialog(Gtk::Window* pMainWin);
    void print_text(const fs::path& pdf_filepath, const std::vector<CtPangoObjectPtr>& slots);

private:
    void _on_begin_print_text(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data);
    void _on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data);
    bool _cairo_tag_can_apply(const Glib::ustring& tag_name, const Glib::ustring& tag_attr, const CtPrintData* print_data);

private:
    void _process_pango_text(CtPrintData* print_data, CtPangoText* text_slot);
    void _process_pango_image(CtPrintData* print_data, const CtImage* image, const CtPangoWidget* pango_widget, bool& any_image_resized);
    void _process_pango_codebox(CtPrintData* print_data, const CtCodebox* codebox, const CtPangoWidget* pango_widget);
    void _process_pango_table(CtPrintData* print_data, const CtTableCommon* table, const CtPangoWidget* pango_widget);

    Glib::RefPtr<Pango::Layout> _codebox_get_layout(const CtCodebox* codebox,
                                                    Glib::ustring content,
                                                    Glib::RefPtr<Gtk::PrintContext> context,
                                                    const int codebox_width);
    void                        _codebox_split_content(const CtCodebox* codebox,
                                                       Glib::ustring original_content,
                                                       const int check_height,
                                                       const Glib::RefPtr<Gtk::PrintContext>& context,
                                                       Glib::ustring& first_split,
                                                       Glib::ustring& second_split,
                                                       const int codebox_width);

    CtPageTable::TableLayouts   _table_get_layouts(const CtTableCommon* table,
                                                   const int first_row,
                                                   const int last_row,
                                                   const Glib::RefPtr<Gtk::PrintContext>& context);
    void                        _table_get_grid(const CtPageTable::TableLayouts& table_layouts,
                                                const CtTableColWidths& col_widths,
                                                std::vector<double>& rows_h,
                                                std::vector<double>& cols_w);
    double                      _table_get_width_height(std::vector<double>& data);
    int                         _table_split_content(const CtTableCommon* table,
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
    CtMainWin* const _pCtMainWin;
    const CtConfig* const _pCtConfig;

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
