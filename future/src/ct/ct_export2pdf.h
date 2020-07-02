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
#include <queue>


struct CtPrintData;
class CtPrintWidgetProxy;
/**
 * @brief Interface for printables used by pdf exporter
 */
class CtPrintable 
{
public:
    struct PrintInfo {
        Glib::RefPtr<Gtk::PrintContext> print_context;
        Pango::FontDescription font;
        Pango::FontDescription codebox_font;
        double page_width;
        double page_height;
        double newline_height;
        int table_line_thickness;
        int text_window_width;
    };
    struct PrintPosition {
        double x;
        double y;
    };
    struct PrintingContext {
        Cairo::RefPtr<Cairo::Context> cairo_context;
        PrintInfo print_info;
        CtPrintData& print_data;
        PrintPosition position;
    };

    virtual void setup(const PrintInfo& print_info, CtPrintData& print_data) = 0;
    virtual PrintPosition print(const PrintingContext& context) = 0;
    [[nodiscard]] virtual double height() const = 0;
    [[nodiscard]] virtual double width() const = 0;
    [[nodiscard]] constexpr bool done() const { return _done; }

    virtual ~CtPrintable() = default;
protected:
    bool _done = false;
};

class CtTextPrintable: public CtPrintable
{
public:
    explicit CtTextPrintable(Glib::ustring text) : _text(std::move(text)) {}

    void setup(const PrintInfo& print_info, CtPrintData& print_data) override;

    PrintPosition print(const PrintingContext& context) override;

    [[nodiscard]] double height() const override;
    [[nodiscard]] double width() const override;
    [[nodiscard]] std::size_t lines() const;
    [[nodiscard]] const Glib::RefPtr<Pango::Layout>& layout() const { return _layout; }
    [[nodiscard]] constexpr bool is_newline() const { return _is_newline; }


private:
    Glib::ustring _text;
    Glib::RefPtr<Pango::Layout> _layout;
    bool _is_newline = false;
    int _line_index = 0;
};

class CtPageBreakPrintable: public CtPrintable {
public:
    CtPageBreakPrintable() = default;

    void setup(const PrintInfo& print_info, CtPrintData& print_data) override;

    PrintPosition print(const PrintingContext& context) override;

    double height() const override;
    double width() const override;

private:
    double _p_height;
};

template<class WIDGET_TYPE_T>
class CtWidgetPrintable : public CtPrintable {
    using wrap_ptr_t = std::shared_ptr<WIDGET_TYPE_T>;
public:
    explicit CtWidgetPrintable(wrap_ptr_t widget_proxy) : _widget_proxy(std::move(widget_proxy)) {}

protected:
    wrap_ptr_t _widget_proxy;
    mutable std::size_t _last_height = 0;
};

std::unique_ptr<CtPrintable> printable_from_widget(CtAnchoredWidget* widget);


using CtPrintableVector = std::vector<std::shared_ptr<CtPrintable>>;

class CtExport2Pdf
{
public:
    CtExport2Pdf(CtMainWin* pCtMainWin) { _pCtMainWin = pCtMainWin; }

    void node_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options, int sel_start, int sel_end);
    void node_and_subnodes_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options);
    void tree_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options);

private:
    void _nodes_all_export_print_iter(const CtTreeIter& tree_iter, const CtExportOptions& options,
                                      CtPrintableVector& tree_pango_slots, std::list<CtAnchoredWidget*>& tree_widgets, Glib::ustring& text_font);
    std::unique_ptr<CtTextPrintable> _add_node_name(Glib::ustring node_name);

private:
    CtMainWin* _pCtMainWin;
};



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

class CtWidgetImagePrintable: public CtWidgetPrintable<CtPrintImageProxy> {
public:
    using CtWidgetPrintable::CtWidgetPrintable;

    PrintPosition print(const PrintingContext& context) override;

    void setup(const PrintInfo& print_info, CtPrintData& print_data) override;

    double width() const override;

    double height() const override;

private:
    double _last_width = 0;
    double _last_height = 0;
};

class CtWidgetTablePrintable: public CtWidgetPrintable<CtPrintTableProxy> {
public:
    using CtWidgetPrintable::CtWidgetPrintable;

    PrintPosition print(const PrintingContext& context) override;

    double width() const override;

    double height() const override;

    void setup(const PrintInfo& print_info, CtPrintData& print_data) override;
private:
    using tbl_layouts_t = std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>;
    using tbl_grid_t = std::pair<std::vector<double>, std::vector<double>>;
    tbl_layouts_t _tbl_layouts;
    tbl_grid_t _tbl_grid;

    struct DrawingContext {
        Cairo::RefPtr<Cairo::Context> cairo_context;
        std::pair<std::vector<double>, std::vector<double>> table_grid;
        std::vector<std::vector<Glib::RefPtr<Pango::Layout>>> table_layouts;
        double x0;
        double y0;
        double width;
        double height;
        int line_thickness;
    };

    tbl_layouts_t _get_table_layouts(const PrintInfo& print_info) const;
    void _split_too_long(const PrintInfo& print_info, CtPrintData& print_data) const;

    std::pair<std::vector<double>, std::vector<double>> _get_table_grid(std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>& table_layouts, int col_min) const;
private:
    static double _get_table_height_from_grid(const tbl_grid_t& table_grid, int table_line_thickness);
    static double _get_table_width_from_grid(const tbl_grid_t& table_grid, int table_line_thickness);
    static void _table_draw_grid(const DrawingContext& context);
    static void _table_draw_text(const DrawingContext& context);
};

class CtWidgetCodeboxPrintable: public CtWidgetPrintable<CtPrintCodeboxProxy> {
public:
    using CtWidgetPrintable::CtWidgetPrintable;

    void setup(const PrintInfo& print_info, CtPrintData& print_data) override;

    PrintPosition print(const PrintingContext& context) override;

    double height() const override;

    double width() const override;


private:
    struct DrawContext {
        double height;
        double width;
        double x;
        double y;
    };

    Glib::RefPtr<Pango::Layout> _layout;
    Glib::RefPtr<Pango::Layout> _calc_layout(const PrintInfo& print_info) const;

    static void _draw_box(const Cairo::RefPtr<Cairo::Context>& cairo_context, const DrawContext& draw_context);
    void _draw_code(const Cairo::RefPtr<Cairo::Context>& cairo_context, const DrawContext& draw_context);
};

using CtWidgetSomePrintable = CtWidgetPrintable<CtPrintSomeProxy>;

class CtPrintable;
// Print Operation Data
struct CtPrintData
{
    std::vector<std::shared_ptr<CtPrintable>> printables;
    std::size_t                               curr_printable_i = 0;
    Glib::RefPtr<Gtk::PrintOperation>         operation;
    Glib::ustring                             warning;
    int                                       nb_pages;
};


class CtPrint
{
public:
    static const int BOX_OFFSET = 4;

public:
    CtPrint();

public:
    void run_page_setup_dialog(Gtk::Window* pMainWin);

    void print_text(CtMainWin* pCtMainWin, const fs::path& pdf_filepath,
                    CtPrintableVector printables, const Glib::ustring& text_font, const Glib::ustring& code_font,
                    const std::list<CtAnchoredWidget*>& widgets, int text_window_width);
    static          Cairo::Rectangle layout_line_get_width_height(Glib::RefPtr<const Pango::LayoutLine> line);
    static double   get_height_from_layout(Glib::RefPtr<Pango::Layout> layout);
    static  double  get_width_from_layout(Glib::RefPtr<Pango::Layout> layout);
private:
    void _on_begin_print_text(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data);
    void _on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data);

private:
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
    CtMainWin*                       _pCtMainWin;
    Glib::RefPtr<Gtk::PrintSettings> _pPrintSettings;
    Glib::RefPtr<Gtk::PageSetup>     _pPageSetup;
    CtPrintable::PrintInfo           _print_info;
    int                              _text_window_width;
    int                              _table_text_row_height;
    double                           _y_idx;
};

class CtExport2Pango
{
public:
    Glib::ustring pango_get_from_code_buffer(Glib::RefPtr<Gsv::Buffer> code_buffer, int sel_start, int sel_end);
    static void pango_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end,
                                              CtPrintableVector& out_printables, bool exclude_anchors);
private:
    static CtPrintableVector _pango_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer);
    static std::unique_ptr<CtPrintable> _pango_text_serialize(const Gtk::TextIter& start_iter, Gtk::TextIter end_iter, const std::map<std::string_view, std::string> &curr_attributes);
};
