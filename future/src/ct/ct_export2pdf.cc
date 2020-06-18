/*
 * ct_export2pdf.cc
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

#include "ct_export2pdf.h"
#include "ct_dialogs.h"

#include <utility>

void CtExport2Pdf::node_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options, int sel_start, int sel_end)
{
    std::vector<Glib::ustring> pango_slots;
    std::list<CtAnchoredWidget*> widgets;
    Glib::ustring text_font;
    if (tree_iter.get_node_is_rich_text())
    {
        CtExport2Pango().pango_get_from_treestore_node(tree_iter, sel_start, sel_end, pango_slots, true /*exclude anchors*/, widgets);
        text_font = _pCtMainWin->get_ct_config()->rtFont;
    }
    else
    {
        pango_slots.push_back(CtExport2Pango().pango_get_from_code_buffer(tree_iter.get_node_text_buffer(), sel_start, sel_end));
        text_font = tree_iter.get_node_syntax_highlighting() != CtConst::PLAIN_TEXT_ID ? _pCtMainWin->get_ct_config()->codeFont : _pCtMainWin->get_ct_config()->ptFont;
    }
    if (options.include_node_name)
        _add_node_name(tree_iter.get_node_name(), pango_slots);

    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, pango_slots, text_font, _pCtMainWin->get_ct_config()->codeFont,
                                           widgets, _pCtMainWin->get_text_view().get_allocation().get_width());
}

void CtExport2Pdf::node_and_subnodes_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
{
    std::vector<Glib::ustring> tree_pango_slots;
    std::list<CtAnchoredWidget*> tree_widgets;
    Glib::ustring text_font = _pCtMainWin->get_ct_config()->codeFont;
    _nodes_all_export_print_iter(tree_iter, options, tree_pango_slots, tree_widgets, text_font);

    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, tree_pango_slots, text_font, _pCtMainWin->get_ct_config()->codeFont,
                                           tree_widgets, _pCtMainWin->get_text_view().get_allocation().get_width());
}

void CtExport2Pdf::tree_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
{
    std::vector<Glib::ustring> tree_pango_slots;
    std::list<CtAnchoredWidget*> tree_widgets;
    Glib::ustring text_font = _pCtMainWin->get_ct_config()->codeFont;
    while (tree_iter)
    {
        _nodes_all_export_print_iter(tree_iter, options, tree_pango_slots, tree_widgets, text_font);
        ++tree_iter;
    }
    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, tree_pango_slots, text_font, _pCtMainWin->get_ct_config()->codeFont,
                                           tree_widgets, _pCtMainWin->get_text_view().get_allocation().get_width());
}

void CtExport2Pdf::_nodes_all_export_print_iter(CtTreeIter tree_iter, const CtExportOptions& options,
                                                 std::vector<Glib::ustring>& tree_pango_slots, std::list<CtAnchoredWidget*>& tree_widgets, Glib::ustring& text_font)
{
    std::vector<Glib::ustring> node_pango_slots;
    std::list<CtAnchoredWidget*> node_widgets;
    if (tree_iter.get_node_is_rich_text())
    {
        CtExport2Pango().pango_get_from_treestore_node(tree_iter, -1, -1, node_pango_slots, true /*exclude anchors*/, node_widgets);
        text_font =_pCtMainWin->get_ct_config()->rtFont; // text font for all (also eventual code nodes)
    }
    else
    {
        node_pango_slots.push_back(CtExport2Pango().pango_get_from_code_buffer(tree_iter.get_node_text_buffer(), -1, -1));
    }
    if (options.include_node_name)
        _add_node_name(tree_iter.get_node_name(), node_pango_slots);
    if (tree_pango_slots.empty())
        tree_pango_slots = node_pango_slots;
    else
    {
        if (options.new_node_page)
        {
            node_pango_slots[0] = str::repeat(CtConst::CHAR_NEWPAGE, 2) + node_pango_slots[0];
            vec::vector_extend(tree_pango_slots, node_pango_slots);
            node_widgets.insert(node_widgets.begin(), nullptr);
        }
        else
        {
            tree_pango_slots[tree_pango_slots.size() - 1] += str::repeat(CtConst::CHAR_NEWLINE, 3) + node_pango_slots[0];
            if (node_pango_slots.size() > 1)
            {
                node_pango_slots.erase(node_pango_slots.begin());
                vec::vector_extend(tree_pango_slots, node_pango_slots);
            }
        }
    }
    tree_widgets.insert(std::end(tree_widgets), std::begin(node_widgets), std::end(node_widgets));
    for (auto& iter: tree_iter->children())
        _nodes_all_export_print_iter(_pCtMainWin->get_tree_store().to_ct_tree_iter(iter), options, tree_pango_slots, tree_widgets, text_font);
}

// Add Node Name to Pango Text Vector
void CtExport2Pdf::_add_node_name(Glib::ustring node_name, std::vector<Glib::ustring>& pango_slots)
{
    pango_slots[0] = "<b><i><span size=\"xx-large\">" + str::xml_escape(node_name)
            + "</span></i></b>" + CtConst::CHAR_NEWLINE + CtConst::CHAR_NEWLINE + pango_slots[0];
}


Glib::ustring CtPrintCodeboxProxy::pango_from_code_buffer(CtCodebox* codebox) { return CtExport2Pango().pango_get_from_code_buffer(codebox->get_buffer(), -1, -1); }

CtPrint::CtPrint()
{
    _pPrintSettings = Gtk::PrintSettings::create();
    _pPageSetup = Gtk::PageSetup::create();
    _pPageSetup->set_paper_size(Gtk::PaperSize("iso_a4"));
}

void CtPrint::run_page_setup_dialog(Gtk::Window* pWin)
{
    _pPageSetup = Gtk::run_page_setup_dialog(*pWin, _pPageSetup, _pPrintSettings);
}

// Start the Print Operations for Text
void CtPrint::print_text(CtMainWin* pCtMainWin, const fs::path& pdf_filepath,
                         const std::vector<Glib::ustring>& pango_text, const Glib::ustring& text_font, const Glib::ustring& code_font,
                         const std::list<CtAnchoredWidget*>& widgets, int text_window_width)
{
    _pCtMainWin = pCtMainWin;
    _pango_font = Pango::FontDescription(text_font);
    _codebox_font = Pango::FontDescription(code_font);
    _text_window_width = text_window_width;
    _table_text_row_height = _pango_font.get_size()/Pango::SCALE;
    _table_line_thickness = 6;
    // convert to proxy widgets
    for (auto& widget: widgets)
    {
        if (auto* image = dynamic_cast<CtImage*>(widget))            _widgets.push_back(std::shared_ptr<CtPrintImageProxy>(new CtPrintImageProxy(image)));
        else if (auto* table = dynamic_cast<CtTable*>(widget))       _widgets.push_back(std::shared_ptr<CtPrintTableProxy>(new CtPrintTableProxy(table, 1, table->get_table_matrix().size())));
        else if (auto* codebox = dynamic_cast<CtCodebox*>(widget))   _widgets.push_back(std::shared_ptr<CtPrintCodeboxProxy>(new CtPrintCodeboxProxy(codebox)));
        else                                                         _widgets.push_back(std::shared_ptr<CtPrintSomeProxy>(new CtPrintSomeProxy(widget)));
    }

    CtPrintData print_data;
    print_data.text = pango_text;

    sigc::slot<void, const Glib::RefPtr<Gtk::PrintContext>&, CtPrintData*> fun_begin_print_text = sigc::mem_fun(*this, &CtPrint::_on_begin_print_text);
    sigc::slot<void,const Glib::RefPtr<Gtk::PrintContext>&,int, CtPrintData*> fun_draw_page_text = sigc::mem_fun(*this, &CtPrint::_on_draw_page_text);

    print_data.operation = Gtk::PrintOperation::create();
    print_data.operation->set_show_progress(true);
    print_data.operation->set_default_page_setup(_pPageSetup);
    print_data.operation->set_print_settings(_pPrintSettings);
    print_data.operation->signal_begin_print().connect(sigc::bind(fun_begin_print_text, &print_data));
    print_data.operation->signal_draw_page().connect(sigc::bind(fun_draw_page_text, &print_data));
    print_data.operation->set_export_filename(pdf_filepath.string());
    try
    {
        auto res = print_data.operation->run(!pdf_filepath.empty() ? Gtk::PRINT_OPERATION_ACTION_EXPORT : Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG);
        if (res == Gtk::PRINT_OPERATION_RESULT_ERROR)
            CtDialogs::error_dialog("Error printing file: (bad res)", *pCtMainWin);
        else if (res == Gtk::PRINT_OPERATION_RESULT_APPLY)
            _pPrintSettings = print_data.operation->get_print_settings();
    }
    catch (Glib::Error& ex)
    {
        CtDialogs::error_dialog("Error printing file:\n" + ex.what() + " (exception caught)", *pCtMainWin);
    }
    if (!print_data.warning.empty())
        _pCtMainWin->get_status_bar().update_status(print_data.warning);

    // remove proxy widgets
    _widgets.clear();
  }


// Here we Compute the Lines Positions, the Number of Pages Needed and the Page Breaks
void CtPrint::_on_begin_print_text(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data)
{
    _page_width = context->get_width();
    _page_height = context->get_height() * 1.02; // tolerance at bottom of the page
    bool any_image_resized = false;
    auto layout_newline = context->create_pango_layout();
    layout_newline->set_font_description(_pango_font);
    layout_newline->set_width(int(_page_width * Pango::SCALE));
    layout_newline->set_markup(CtConst::CHAR_NEWLINE);
    _layout_newline_height = _layout_line_get_width_height(layout_newline->get_line(0)).height;
    int codebox_height = 0, table_height = 0; // to keep data for current slot from previous slot

    while (1)
    {
        print_data->layout.clear();
        print_data->forced_page_break.clear();
        print_data->layout_is_new_line.clear();
        print_data->layout_num_lines.clear();
        print_data->all_lines_y.clear();

        bool exit_ok = true;
        for (Glib::ustring& text_slot: print_data->text)
        {
            bool is_forced_page_break = str::startswith(text_slot, str::repeat(CtConst::CHAR_NEWPAGE, 2));
            print_data->forced_page_break.push_back(is_forced_page_break);
            // in other cases we detect the newline from a following line
            // but here we have a single layout line
            print_data->layout_is_new_line.push_back(text_slot == CtConst::CHAR_NEWLINE);

            auto layout = context->create_pango_layout();
            print_data->layout.push_back(layout);
            layout->set_font_description(_pango_font);
            layout->set_width(int(_page_width * Pango::SCALE));
            layout->set_markup(is_forced_page_break ? text_slot.substr(2) : text_slot);
            print_data->layout_num_lines.push_back(layout->get_line_count());
        }

        print_data->page_breaks.clear();
        _y_idx = 0;
        double curr_y = 0;
        double inline_pending_height = 0;
        auto inline_starter = std::make_pair(0, 0);
        for (size_t i = 0; i < print_data->layout.size(); ++i)
        {
            auto layout = print_data->layout[i];
            if (print_data->forced_page_break[i] && curr_y > 0)
            {
                print_data->page_breaks.push_back(inline_starter);
                curr_y = 0;
            }
            int layout_line_idx = 0;
            while (layout_line_idx < print_data->layout_num_lines[i])
            {
                auto layout_line = layout->get_line(layout_line_idx);
                auto line_height = _layout_line_get_width_height(layout_line).height;
                // process the line
                if (line_height > inline_pending_height) inline_pending_height = line_height;
                if (layout_line_idx < print_data->layout_num_lines[i] - 1 || print_data->layout_is_new_line[i])
                {
                    if (curr_y + inline_pending_height > _page_height)
                    {
                        print_data->page_breaks.push_back(inline_starter);
                        curr_y = 0;
                        if (inline_pending_height > _page_height)
                        {
                            auto* codebox = dynamic_cast<CtPrintCodeboxProxy*>(_widgets[i-1].get());
                            auto* table = dynamic_cast<CtPrintTableProxy*>(_widgets[i-1].get());
                            if (codebox && codebox_height > _page_height)
                            {
                                _codebox_long_split(i-1, context, print_data);
                                exit_ok = false;
                                break; // go to a new main loop
                            }
                            else if (table && table_height > _page_height)
                            {
                                _table_long_split(i-1, context, print_data);
                                exit_ok = false;
                                break; // go to a new main loop
                            }
                        }
                    }
                    curr_y += inline_pending_height;
                    print_data->all_lines_y.push_back(curr_y);
                    inline_pending_height = 0; // reset the pending elements line to append
                    inline_starter = std::make_pair(i, layout_line_idx+1);
                }
                layout_line_idx += 1;
            }
            if (!exit_ok)
                break; // go to a new main loop
            // pixbuf or table or codebox
            if (i < print_data->layout.size() - 1) // the latest element is supposed to be text
            {
                if (auto* imageProxy = dynamic_cast<CtPrintImageProxy*>(_widgets[i].get()))
                {
                    auto pixbuf = imageProxy->get_pixbuf();
                    // don't know curr_x, so will recalc scale again in draw function
                    double scale_w = _page_width / pixbuf->get_width();
                    double scale_h = (_page_height - _layout_newline_height - CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT)/pixbuf->get_height();
                    double scale = std::min(scale_w, scale_h);
                    if (scale > 1.0) scale = 1.0;
                    double pixbuf_height = pixbuf->get_height() * scale + CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT;

                    if (inline_pending_height < pixbuf_height)
                        inline_pending_height = pixbuf_height;
                }
                else if (auto* tableProxy = dynamic_cast<CtPrintTableProxy*>(_widgets[i].get()))
                {
                    auto table_layouts = _get_table_layouts(context, tableProxy);
                    auto table_grid = _get_table_grid(table_layouts, tableProxy->get_table()->get_col_min());
                    double table_height = _get_table_height_from_grid(table_grid);
                    if (inline_pending_height < table_height + BOX_OFFSET)
                        inline_pending_height = table_height + BOX_OFFSET;
                }
                else if (auto* codeboxProxy = dynamic_cast<CtPrintCodeboxProxy*>(_widgets[i].get()))
                {
                    auto codebox_layout = _get_codebox_layout(context, codeboxProxy);
                    double codebox_height = _get_height_from_layout(codebox_layout);
                    if (inline_pending_height < codebox_height + BOX_OFFSET)
                        inline_pending_height = codebox_height + BOX_OFFSET;
                }
            }
        }
        if (exit_ok)
            break;
    }
    print_data->operation->set_n_pages(static_cast<int>(print_data->page_breaks.size() + 1));
    if (any_image_resized)
    {
        print_data->warning = Glib::ustring(_("Warning: One or More Images Were Reduced to Enter the Page")) + " ("
                                       + std::to_string(static_cast<int>(_page_width))+ "x" + std::to_string(static_cast<int>(_page_height)) + ")";
    }
}

// This Function is Called For Each Page Set in on_begin_print_text
void CtPrint::_on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data)
{
    // layout num, line num
    std::pair<int, int> start_line_num = page_nr == 0 ? std::make_pair(0, 0) : print_data->page_breaks[page_nr - 1];
    std::pair<int, int> end_line_num = page_nr < static_cast<int>(print_data->page_breaks.size()) ? print_data->page_breaks[page_nr] : std::make_pair(static_cast<int>(print_data->layout.size()-1), print_data->layout_num_lines.back());
    auto operation = print_data->operation;
    auto cairo_context = context->get_cairo_context();
    cairo_context->set_source_rgb(0.5, 0.5, 0.5);
    cairo_context->set_font_size(12);
    Glib::ustring page_num_str = std::to_string(page_nr+1) + "/" + std::to_string(operation->property_n_pages());
    cairo_context->move_to(_page_width/2., _page_height+17);
    cairo_context->show_text(page_num_str);

    double curr_x = 0.;
    int i = start_line_num.first;
    int layout_line_idx = start_line_num.second;
    while (i <= end_line_num.first)
    {
        // text
        cairo_context->set_source_rgb(0, 0, 0);
        if (i > start_line_num.first)
            layout_line_idx = 0; //reset line idx
        while (layout_line_idx < print_data->layout_num_lines[i])
        {
            auto layout_line = print_data->layout[i]->get_line(layout_line_idx);
            double line_width = _layout_line_get_width_height(layout_line).width;
            // process the line
            if (line_width > 0)
            {
                cairo_context->move_to(curr_x, print_data->all_lines_y[_y_idx]);
                layout_line->show_in_cairo_context(cairo_context);
                curr_x += line_width;
            }
            if (layout_line_idx < print_data->layout_num_lines[i]-1 || print_data->layout_is_new_line[i])
            {
                curr_x = 0.0;
                _y_idx += 1;
            }
            layout_line_idx += 1;
            if (i >= end_line_num.first && layout_line_idx >= end_line_num.second)
                return;
        }
        // pixbuf or table or codebox
        if (i < static_cast<int>(print_data->layout.size() - 1)) // the latest element is supposed to be text
        {
            if (auto* imageProxy = dynamic_cast<CtPrintImageProxy*>(_widgets[i].get()))
            {
                auto pixbuf = imageProxy->get_pixbuf();
                // should recalc scale because curr_x is changed
                double scale_w = (_page_width - curr_x) / pixbuf->get_width();
                double scale_h = (_page_height - _layout_newline_height - CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT)/pixbuf->get_height();
                double scale = std::min(scale_w, scale_h);
                if (scale > 1.0) scale = 1.0;
                double pixbuf_width = pixbuf->get_width() * scale;
                double pixbuf_height = pixbuf->get_height() * scale;

                cairo_context->save();
                cairo_context->scale(scale, scale);
                Gdk::Cairo::set_source_pixbuf(cairo_context, pixbuf, curr_x / scale, (print_data->all_lines_y[_y_idx] - pixbuf_height) / scale);
                cairo_context->paint();
                cairo_context->restore();

                curr_x += pixbuf_width;
            }
            else if (auto* tableProxy = dynamic_cast<CtPrintTableProxy*>(_widgets[i].get()))
            {
                auto table_layouts = _get_table_layouts(context, tableProxy);
                auto table_grid = _get_table_grid(table_layouts, tableProxy->get_table()->get_col_min());
                double table_width = _get_table_width_from_grid(table_grid);
                double table_height = _get_table_height_from_grid(table_grid);
                _table_draw_grid(cairo_context, table_grid, curr_x,
                                 print_data->all_lines_y[_y_idx] - table_height,
                                 table_width, table_height);
                _table_draw_text(cairo_context, table_grid, table_layouts, curr_x,
                                 print_data->all_lines_y[_y_idx] - table_height);
                curr_x += table_width;
            }
            else if (auto* codeboxProxy = dynamic_cast<CtPrintCodeboxProxy*>(_widgets[i].get()))
            {
                auto codebox_layout = _get_codebox_layout(context, codeboxProxy);
                double codebox_height = _get_height_from_layout(codebox_layout);
                double codebox_width = _get_width_from_layout(codebox_layout);
                _codebox_draw_box(cairo_context, curr_x,
                                  print_data->all_lines_y[_y_idx] - codebox_height,
                                  codebox_width, codebox_height);
                _codebox_draw_code(cairo_context, codebox_layout, curr_x,
                                   print_data->all_lines_y[_y_idx] - codebox_height);
                curr_x += codebox_width;
            }
        }
        i += 1; // layout increment
    }
}

// Returns Width and Height of a layout line
Cairo::Rectangle CtPrint::_layout_line_get_width_height(Glib::RefPtr<const Pango::LayoutLine> line)
{
    Pango::Rectangle ink_rect, logical_rect;
    line->get_extents(ink_rect, logical_rect);
    Cairo::Rectangle rect;
    rect.width = logical_rect.get_width() / Pango::SCALE;
    rect.height = logical_rect.get_height() / Pango::SCALE;
    return rect;
}

// Returns the Height given the Layout
double CtPrint::_get_height_from_layout(Glib::RefPtr<Pango::Layout> layout)
{
    double height = 0;
    for (int layout_line_idx = 0; layout_line_idx < layout->get_line_count(); ++layout_line_idx)
    {
        Glib::RefPtr<const Pango::LayoutLine> layout_line = layout->get_line(layout_line_idx);
        double line_height = _layout_line_get_width_height(layout_line).height;
        height += line_height;
    }

    return height + 2 * CtConst::GRID_SLIP_OFFSET;
}

// Returns the Height given the Layout
double CtPrint::_get_width_from_layout(Glib::RefPtr<Pango::Layout> layout)
{
    double width = 0;
    for (int layout_line_idx = 0; layout_line_idx < layout->get_line_count(); ++layout_line_idx)
    {
        Glib::RefPtr<const Pango::LayoutLine> layout_line = layout->get_line(layout_line_idx);
        double line_width = _layout_line_get_width_height(layout_line).width;
        if (line_width > width)
            width = line_width;
    }
    return width + 2 * CtConst::GRID_SLIP_OFFSET;
}

// Return the CodeBox Layout
Glib::RefPtr<Pango::Layout> CtPrint::_get_codebox_layout(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintCodeboxProxy* codeboxProxy)
{
    auto layout = context->create_pango_layout();
    layout->set_font_description(_codebox_font);
    double codebox_width = codeboxProxy->get_width_in_pixels() ? codeboxProxy->get_frame_width() : _text_window_width * codeboxProxy->get_frame_width()/100.;
    if (codebox_width > _page_width)
        codebox_width = _page_width;
    layout->set_width(int(codebox_width * Pango::SCALE));
    layout->set_wrap(Pango::WRAP_WORD_CHAR);
    layout->set_markup(codeboxProxy->get_text_content());
    return layout;
}

// Return the Table Cells Layouts
std::vector<std::vector<Glib::RefPtr<Pango::Layout>>> CtPrint::_get_table_layouts(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintTableProxy* tableProxy)
{
    std::vector<std::vector<Glib::RefPtr<Pango::Layout>>> table_layouts;
    for (int i = 0; i < tableProxy->get_row_num(); ++i)
    {
        std::vector<Glib::RefPtr<Pango::Layout>> layouts;
        for (int j = 0; j < tableProxy->get_col_num(); ++j)
        {
            Glib::ustring text = str::xml_escape(tableProxy->get_cell(i, j));
            if (i == 0) text = "<b>" + text + "</b>";
            auto layout = context->create_pango_layout();
            layout->set_font_description(_pango_font);
            layout->set_width(int(tableProxy->get_table()->get_col_max() * Pango::SCALE));
            layout->set_wrap(Pango::WRAP_WORD_CHAR);
            layout->set_markup(text);
            layouts.push_back(layout);
        }
        table_layouts.push_back(std::move(layouts));
    }
    return table_layouts;
}

// Returns the Dimensions of Rows and Columns
std::pair<std::vector<double>, std::vector<double>> CtPrint::_get_table_grid(std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>& table_layouts, int col_min)
{
    std::vector<double> rows_h(table_layouts.size(), 0);
    std::vector<double> cols_w(table_layouts[0].size(), col_min);
    for (size_t i = 0; i < table_layouts.size(); ++i)
    {
        auto layout_row = table_layouts[i];
        for (size_t j = 0; j < layout_row.size(); ++j)
        {
            auto layout_cell = layout_row[j];
            double cell_height = 0;
            for (int layout_line_idx = 0; layout_line_idx < layout_cell->get_line_count(); ++ layout_line_idx)
            {
                auto layout_line = layout_cell->get_line(layout_line_idx);
                auto line_size = _layout_line_get_width_height(layout_line);
                cell_height += line_size.height;
                if (cols_w[j] < line_size.width) cols_w[j] = line_size.width;
            }
            if (rows_h[i] < cell_height) rows_h[i] = cell_height;
        }
    }
    return std::make_pair(rows_h, cols_w);
}

// Returns the Table Width given the table_grid vector
double CtPrint::_get_table_width_from_grid(std::pair<std::vector<double>, std::vector<double>>& table_grid)
{
    double table_width = 0;
    for (auto& col_w: table_grid.second)
        table_width += col_w + _table_line_thickness;
    return table_width;
}

// Returns the Table Height given the table_grid vector
double CtPrint::_get_table_height_from_grid(std::pair<std::vector<double>, std::vector<double>>& table_grid)
{
    double table_height = 0;
    for (auto& row_h: table_grid.first)
        table_height += row_h + _table_line_thickness;
    return table_height;
}

// Split Long Tables
void CtPrint::_table_long_split(size_t idx, const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data)
{
    auto* tableProxy = dynamic_cast<CtPrintTableProxy*>(_widgets[(size_t)idx].get());
    std::vector<std::shared_ptr<CtPrintTableProxy>> new_tables;
    std::shared_ptr<CtPrintTableProxy> new_table;
    for (int row_num = 1 /*header row*/ + 1 /*next row*/; row_num < tableProxy->get_row_num(); ++row_num)
    {
        new_table = tableProxy->create_new_with(row_num);
        auto table_layouts = _get_table_layouts(context, new_table.get());
        auto table_grid = _get_table_grid(table_layouts, new_table->get_table()->get_col_min());
        double table_height = _get_table_height_from_grid(table_grid);
        if (table_height + BOX_OFFSET > (_page_height - _layout_newline_height)) // hit upper limit
        {
            // this slot is done
            new_table = tableProxy->create_new_with(row_num - 1); // todo: hmm, what if it will be just header?
            new_tables.push_back(new_table);
            new_table.reset();
            // start cycle again with reduced tabled
            tableProxy->remove_first_rows(row_num - 1 - 1 /* skip header */);
            row_num = 1; // will be incremented to 2
        }
    }
    if (new_table && new_table->get_row_num() > 1)
        new_tables.push_back(new_table);

    for (size_t i = 0; i < new_tables.size(); ++i)
    {
        if (i == 0)
            *tableProxy = *new_tables[i];
        else
        {
            //  add a newline
            print_data->text.insert(print_data->text.begin() + idx + i, CtConst::CHAR_NEWLINE);
            // add a table
            _widgets.insert(_widgets.begin() + idx + i, new_tables[i]);
        }
    }
}

// Split Long CodeBoxes
void CtPrint::_codebox_long_split(size_t idx, const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data)
{
    auto* codeboxProxy = dynamic_cast<CtPrintCodeboxProxy*>(_widgets[(size_t)idx].get());
    std::vector<Glib::ustring> original_splitted_pango = str::split(codeboxProxy->get_text_content(), CtConst::CHAR_NEWLINE);
    // fix for not-closed span, I suppose
    for (size_t i = 0; i < original_splitted_pango.size(); ++i)
    {
        auto& element = original_splitted_pango[i];
        auto last_close = element.rfind("</span");
        auto last_open = element.rfind("<span");
        if (last_close < last_open)
        {
            auto non_closed_span = element.substr(last_open);
            auto end_non_closed_span_idx = non_closed_span.find(">");
            non_closed_span = non_closed_span.substr(0, end_non_closed_span_idx+1);
            original_splitted_pango[i] += "</span>";
            original_splitted_pango[i+1] = non_closed_span + original_splitted_pango[i+1];
        }
    }

    std::vector<Glib::ustring> result_pango;
    std::vector<Glib::ustring> splitted_pango;
    Glib::ustring partial_pango = "";
    Glib::ustring partial_pango_prev = "";
    while (splitted_pango.size() < original_splitted_pango.size())
    {
        splitted_pango.push_back(original_splitted_pango[splitted_pango.size()]);
        if (!partial_pango.empty())
            partial_pango_prev = partial_pango;
        partial_pango = str::join(splitted_pango, CtConst::CHAR_NEWLINE);
        CtPrintCodeboxProxy temp_proxy(codeboxProxy->get_codebox(), partial_pango);
        auto codebox_layout = _get_codebox_layout(context, &temp_proxy);
        double codebox_height = _get_height_from_layout(codebox_layout);
        if (codebox_height + BOX_OFFSET > (_page_height - _layout_newline_height))
        {
            // this slot is done
            result_pango.push_back(partial_pango_prev);
            original_splitted_pango.erase(original_splitted_pango.begin(), original_splitted_pango.begin() + (splitted_pango.size() - 1)); // todo: not sure, need to test it
            splitted_pango.clear();
        }
    }
    // this is the last piece
    result_pango.push_back(partial_pango);
    for (size_t i = 0; i < result_pango.size(); ++i)
    {
        if (i == 0)
            codeboxProxy->set_proxy_content(result_pango[i]);
        else
        {
            size_t index = idx+i;
            // add a newline
            print_data->text.insert(print_data->text.begin() + index, CtConst::CHAR_NEWLINE);
            // add a codebox
            auto proxy = std::make_shared<CtPrintCodeboxProxy>(codeboxProxy->get_codebox(), result_pango[i]);
            _widgets.insert(_widgets.begin() + index, proxy);
        }
    }
}

// Draw the CodeBox Box
void CtPrint::_codebox_draw_box(Cairo::RefPtr<Cairo::Context> cairo_context, double x0, double y0, double codebox_width, double codebox_height)
{
    cairo_context->set_source_rgba(0, 0, 0, 0.3);
    cairo_context->rectangle(x0, y0, codebox_width, codebox_height);
    cairo_context->stroke();
}

// Draw the code inside of the Box
void CtPrint::_codebox_draw_code(Cairo::RefPtr<Cairo::Context> cairo_context, Glib::RefPtr<Pango::Layout> codebox_layout, double x0, double y0)
{
    double y = y0;
    cairo_context->set_source_rgb(0, 0, 0);
    for (int layout_line_idx = 0; layout_line_idx < codebox_layout->get_line_count(); ++layout_line_idx)
    {
        auto layout_line = codebox_layout->get_line(layout_line_idx);
        double line_height = _layout_line_get_width_height(layout_line).height;
        cairo_context->move_to(x0 + CtConst::GRID_SLIP_OFFSET, y + line_height);
        y += line_height;
        layout_line->show_in_cairo_context(cairo_context);
    }
}

// Draw the Table Grid
void CtPrint::_table_draw_grid(Cairo::RefPtr<Cairo::Context> cairo_context, const std::pair<std::vector<double>, std::vector<double>>& table_grid,
                               double x0, double y0, double table_width, double table_height)
{
    double x = x0;
    double y = y0;
    cairo_context->set_source_rgba(0, 0, 0, 0.3);
    // draw lines
    cairo_context->move_to(x, y);
    cairo_context->line_to(x + table_width, y);
    for (auto& row_h: table_grid.first)
    {
        y += row_h + _table_line_thickness;
        cairo_context->move_to(x, y);
        cairo_context->line_to(x + table_width, y);
    }
    // draw columns
    y = y0;
    cairo_context->move_to(x, y);
    cairo_context->line_to(x, y + table_height);
    for (auto& col_w: table_grid.second)
    {
        x += col_w + _table_line_thickness;
        cairo_context->move_to(x, y);
        cairo_context->line_to(x, y + table_height);
    }
    cairo_context->stroke();
}

// Draw the text inside of the Table Cells
void CtPrint::_table_draw_text(Cairo::RefPtr<Cairo::Context> cairo_context,
                              const std::pair<std::vector<double>, std::vector<double>>& table_grid,
                              const std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>& table_layouts,
                              double x0, double y0)
{
    cairo_context->set_source_rgb(0, 0, 0);
    double y = y0;
    for (size_t i = 0; i < table_grid.first.size(); ++i)
    {
        double row_h = table_grid.first[i];
        double x = x0 + CtConst::GRID_SLIP_OFFSET;
        for (size_t j = 0; j < table_grid.second.size(); ++j)
        {
            double col_w = table_grid.second[j];
            auto layout_cell = table_layouts[i][j];
            double local_y = y;
            for (int layout_line_id = 0; layout_line_id < layout_cell->get_line_count(); ++layout_line_id)
            {
                auto layout_line = layout_cell->get_line(layout_line_id);
                double line_height = _layout_line_get_width_height(layout_line).height;
                cairo_context->move_to(x, local_y + line_height);
                local_y += line_height;
                layout_line->show_in_cairo_context(cairo_context);
            }
            x += col_w + _table_line_thickness;
        }
        y += row_h + _table_line_thickness;
    }
}




// Get rich text from syntax highlighted code node
Glib::ustring CtExport2Pango::pango_get_from_code_buffer(Glib::RefPtr<Gsv::Buffer> code_buffer, int sel_start, int sel_end)
{
    Gtk::TextIter curr_iter = sel_start < 0 ? code_buffer->begin() : code_buffer->get_iter_at_offset(sel_start);
    Gtk::TextIter end_iter = sel_start < 0 ? code_buffer->end() : code_buffer->get_iter_at_offset(sel_end);
    code_buffer->ensure_highlight(curr_iter, end_iter);
    Glib::ustring pango_text = "";
    Glib::ustring former_tag_str = CtConst::COLOR_48_BLACK;
    bool span_opened = false;
    while (true)
    {
        auto curr_tags = curr_iter.get_tags();
        if (!curr_tags.empty())
        {
            Glib::ustring curr_tag_str = curr_tags[0]->property_foreground_gdk().get_value().to_string();
            int font_weight = curr_tags[0]->property_weight();
            if (curr_tag_str == CtConst::COLOR_48_BLACK)
            {
                if (former_tag_str != curr_tag_str)
                {
                    former_tag_str = curr_tag_str;
                    // end of tag
                    pango_text += "</span>";
                    span_opened = false;
                }
            }
            else
            {
                if (former_tag_str != curr_tag_str)
                {
                    former_tag_str = curr_tag_str;
                    if (span_opened) pango_text += "</span>";
                    // start of tag
                    Glib::ustring color = CtRgbUtil::get_rgb24str_from_str_any(CtRgbUtil::rgb_to_no_white(curr_tag_str));
                    pango_text += "<span foreground=\"" + curr_tag_str + "\" font_weight=\"" + std::to_string(font_weight) + "\">";
                    span_opened = true;
                }
            }
        }
        else if (span_opened)
        {
            span_opened = false;
            former_tag_str = CtConst::COLOR_48_BLACK;
            pango_text += "</span>";
        }
        pango_text += str::xml_escape(Glib::ustring(1, curr_iter.get_char()));
        if (!curr_iter.forward_char() || (sel_end >= 0 && curr_iter.get_offset() > sel_end))
        {
            if (span_opened) pango_text += "</span>";
            break;
        }
    }
    if (pango_text.empty() || pango_text[pango_text.size()-1] != g_utf8_get_char(CtConst::CHAR_NEWLINE))
        pango_text += CtConst::CHAR_NEWLINE;
    return pango_text;
}

// Given a treestore iter returns the Pango rich text
void CtExport2Pango::pango_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end, std::vector<Glib::ustring>& out_slots,
                                                   bool exclude_anchors, std::list<CtAnchoredWidget*>& out_widgets)
{
    auto curr_buffer = node_iter.get_node_text_buffer();
    out_widgets = node_iter.get_embedded_pixbufs_tables_codeboxes(sel_start, sel_end);
    if (exclude_anchors)
        out_widgets.remove_if([](CtAnchoredWidget* widget) { return dynamic_cast<CtImageAnchor*>(widget);});
    out_slots.clear();
    int start_offset = sel_start < 1 ? 0 : sel_start;
    for (auto widget: out_widgets)
    {
        int end_offset = widget->getOffset();
        out_slots.push_back(_pango_process_slot(start_offset, end_offset, curr_buffer));
        start_offset = end_offset;
    }
    if (sel_start < 0)
        out_slots.push_back(_pango_process_slot(start_offset, curr_buffer->end().get_offset(), curr_buffer));
    else
        out_slots.push_back(_pango_process_slot(start_offset, sel_end, curr_buffer));
    // fix the problem of the latest char not being a new line char
    if (out_slots.size() > 0 && (out_slots[out_slots.size() - 1].size() == 0 || !str::endswith(out_slots[out_slots.size()-1], CtConst::CHAR_NEWLINE)))
        out_slots[out_slots.size()-1] += CtConst::CHAR_NEWLINE;
}

// Process a Single Pango Slot
Glib::ustring CtExport2Pango::_pango_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer)
{
    Glib::ustring curr_pango_text = "";
    CtTextIterUtil::generic_process_slot(start_offset, end_offset, curr_buffer,
                                         [&](Gtk::TextIter& start_iter, Gtk::TextIter& curr_iter, std::map<std::string_view, std::string>& curr_attributes) {
        curr_pango_text += _pango_text_serialize(start_iter, curr_iter, curr_attributes);
    });
    return curr_pango_text;
}

// Adds a slice to the Pango Text
Glib::ustring CtExport2Pango::_pango_text_serialize(Gtk::TextIter start_iter, Gtk::TextIter end_iter, const std::map<std::string_view, std::string> &curr_attributes)
{
    Glib::ustring pango_attrs;
    bool superscript_active = false;
    bool subscript_active = false;
    bool monospace_active = false;
    for (auto tag_property: CtConst::TAG_PROPERTIES)
    {
        if ((tag_property != CtConst::TAG_JUSTIFICATION && tag_property != CtConst::TAG_LINK) && !curr_attributes.at(tag_property).empty())
        {
            auto property_value = curr_attributes.at(tag_property);
            // tag names fix
            if (tag_property == CtConst::TAG_SCALE)
            {
                if (property_value == CtConst::TAG_PROP_VAL_SUP)
                {
                    superscript_active = true;
                    continue;
                }
                else if (property_value == CtConst::TAG_PROP_VAL_SUB)
                {
                    subscript_active = true;
                    continue;
                }
                else
                    tag_property = "size";
                // tag properties fix
                if (property_value == CtConst::TAG_PROP_VAL_SMALL) property_value = "x-small";
                else if (property_value == CtConst::TAG_PROP_VAL_H1) property_value = "xx-large";
                else if (property_value == CtConst::TAG_PROP_VAL_H2) property_value = "x-large";
                else if (property_value == CtConst::TAG_PROP_VAL_H3) property_value = "large";
            }
            else if (tag_property == CtConst::TAG_FAMILY)
            {
                monospace_active = true;
                continue;
            }
            else if (tag_property == CtConst::TAG_FOREGROUND)
            {
                Glib::ustring color_no_white = CtRgbUtil::rgb_to_no_white(property_value);
                property_value = CtRgbUtil::get_rgb24str_from_str_any(color_no_white);
            }
            pango_attrs += std::string(" ") + tag_property.data() + "=\"" + property_value + "\"";
        }
    }
    Glib::ustring tagged_text;
    if (pango_attrs.empty())
        tagged_text = str::xml_escape(start_iter.get_text(end_iter));
    else
        tagged_text = "<span" + pango_attrs + ">" + str::xml_escape(start_iter.get_text(end_iter)) + "</span>";
    if (superscript_active) tagged_text = "<sup>" + tagged_text + "</sup>";
    if (subscript_active) tagged_text = "<sub>" + tagged_text + "</sub>";
    if (monospace_active) tagged_text = "<tt>" + tagged_text + "</tt>";
    return tagged_text;
}
