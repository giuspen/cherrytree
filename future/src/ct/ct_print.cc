/*
 * ct_print.cc
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

#include "ct_print.h"
#include "ct_dialogs.h"

CtPrint::CtPrint()
{
    _pPrintSettings = Gtk::PrintSettings::create();
    _pPageSetup = Gtk::PageSetup::create();
}

void CtPrint::run_page_setup_dialog(Gtk::Window* pMainWin)
{
    _pPageSetup = Gtk::run_page_setup_dialog(*pMainWin, _pPageSetup, _pPrintSettings);
}

// Start the Print Operations for Text
void CtPrint::print_text(CtMainWin* pCtMainWin, const Glib::ustring& pdf_filepath,
                         const std::vector<Glib::ustring>& pango_text, const Glib::ustring& text_font, const Glib::ustring& code_font,
                         const std::list<CtAnchoredWidget*>& widgets, int text_window_width)
{
    _pango_font = Pango::FontDescription(text_font);
    _codebox_font = Pango::FontDescription(code_font);
    _text_window_width = text_window_width;
    _table_text_row_height = _pango_font.get_size()/Pango::SCALE;
    _table_line_thickness = 6;
    _widgets.clear();
    for (auto& widget: widgets)
    {
        if (CtImage* image = dynamic_cast<CtImage*>(widget)) _widgets.push_back(std::shared_ptr<CtPrintImageProxy>(new CtPrintImageProxy(image)));
        else if (CtTable* table = dynamic_cast<CtTable*>(widget)) _widgets.push_back(std::shared_ptr<CtPrintTableProxy>(new CtPrintTableProxy(table, 1, table->get_table_matrix().size() - 1)));
        else if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widget)) _widgets.push_back(std::shared_ptr<CtPrintCodeboxProxy>(new CtPrintCodeboxProxy(codebox, "")));
        else _widgets.push_back(std::shared_ptr<CtPrintSomeProxy>(new CtPrintSomeProxy(widget)));
    }

    CtPrintData* print_data = new CtPrintData();
    print_data->text = pango_text;

    sigc::slot<void, const Glib::RefPtr<Gtk::PrintContext>&, CtPrintData*> fun_begin_print_text = sigc::mem_fun(*this, &CtPrint::_on_begin_print_text);
    sigc::slot<void,const Glib::RefPtr<Gtk::PrintContext>&,int, CtPrintData*> fun_draw_page_text = sigc::mem_fun(*this, &CtPrint::_on_draw_page_text);

    print_data->operation = Gtk::PrintOperation::create();
    print_data->operation->set_show_progress(true);
    print_data->operation->set_default_page_setup(_pPageSetup);
    print_data->operation->set_print_settings(_pPrintSettings);
    print_data->operation->signal_begin_print().connect(sigc::bind(fun_begin_print_text, print_data));
    print_data->operation->signal_draw_page().connect(sigc::bind(fun_draw_page_text, print_data));
    print_data->operation->set_export_filename(pdf_filepath);
    try
    {
        auto res = print_data->operation->run(pdf_filepath != "" ? Gtk::PRINT_OPERATION_ACTION_EXPORT : Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG);
        if (res == Gtk::PRINT_OPERATION_RESULT_ERROR)
            CtDialogs::error_dialog("Error printing file: (bad res)", *pCtMainWin);
        else if (res == Gtk::PRINT_OPERATION_RESULT_APPLY)
            _pPrintSettings = print_data->operation->get_print_settings();
    }
    catch (Glib::Error& ex)
    {
        CtDialogs::error_dialog("Error printing file:\n" + ex.what() + " (exception caught)", *pCtMainWin);
    }
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
    double line_width;
    std::tie(line_width, _layout_newline_height) = _layout_line_get_width_height(layout_newline->get_line(0));
    int codebox_height, table_height;

    while (1)
    {
        // todo clean up
        bool exit_ok = true;
        for (Glib::ustring& text_slot: print_data->text)
        {
            bool is_forced_page_break = str::startswith(text_slot, CtConst::CHAR_NEWPAGE + CtConst::CHAR_NEWPAGE);
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
                auto [line_width, line_height] = _layout_line_get_width_height(layout_line);
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
                            CtPrintCodeboxProxy* codebox= dynamic_cast<CtPrintCodeboxProxy*>(_widgets[i-1].get());
                            CtPrintTableProxy* table = dynamic_cast<CtPrintTableProxy*>(_widgets[i-1].get());
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
                if (CtPrintImageProxy* imageProxy = dynamic_cast<CtPrintImageProxy*>(_widgets[i].get()))
                {
                    auto image = imageProxy->get_image();
                    auto pixbuf = image->get_pixbuf();
                    bool pixbuf_was_resized = false;
                    int pixbuf_width = pixbuf->get_width();
                    int pixbuf_height = pixbuf->get_height();
                    if (pixbuf_width > _page_width)
                    {
                        double image_w_h_ration = double(pixbuf_width)/pixbuf_height;
                        double image_width = _page_width;
                        double image_height = image_width / image_w_h_ration;
                        pixbuf = pixbuf->scale_simple(int(image_width), int(image_height), Gdk::INTERP_BILINEAR);
                        pixbuf_width = pixbuf->get_width();
                        pixbuf_height = pixbuf->get_height();
                        pixbuf_was_resized = true;
                    }
                    if (pixbuf_height > (_page_height - _layout_newline_height - CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT))
                    {
                        double image_w_h_ration = double(pixbuf_width)/pixbuf_height;
                        double image_height = _page_height - _layout_newline_height - CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT;
                        double image_width = image_height * image_w_h_ration;
                        pixbuf = pixbuf->scale_simple(int(image_width), int(image_height), Gdk::INTERP_BILINEAR);
                        pixbuf_width = pixbuf->get_width();
                        pixbuf_height = pixbuf->get_height();
                        pixbuf_was_resized = true;
                    }
                    if (pixbuf_was_resized)
                    {
                        any_image_resized = true;
                        print_data->modified_pixbuf[image] = pixbuf;
                    }
                    pixbuf_height = pixbuf->get_height() + CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT;
                    if (inline_pending_height < pixbuf_height)
                        inline_pending_height = pixbuf_height;
                }
                else if (CtPrintTableProxy* tableProxy = dynamic_cast<CtPrintTableProxy*>(_widgets[i].get()))
                {
                    auto table_layouts = _get_table_layouts(context, tableProxy);
                    auto table_grid = _get_table_grid(table_layouts, tableProxy->get_table()->get_col_min());
                    double table_height = _get_table_height_from_grid(table_grid);
                    if (inline_pending_height < table_height + BOX_OFFSET)
                        inline_pending_height = table_height + BOX_OFFSET;
                }
                else if (CtPrintCodeboxProxy* codeboxProxy = dynamic_cast<CtPrintCodeboxProxy*>(_widgets[i].get()))
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
    print_data->operation->set_n_pages((int)print_data->page_breaks.size() + 1);
    if (any_image_resized)
    {
        Glib::ustring message = Glib::ustring(_("Warning: One or More Images Were Reduced to Enter the Page")) + " ("
                + std::to_string(int(_page_width))+ "x" + std::to_string(int(_page_height)) + ")";
        _pCtMainWin->get_status_bar().update_status(message);
    }
}

// This Function is Called For Each Page Set in on_begin_print_text
void CtPrint::_on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data)
{

}

// Returns Width and Height of a layout line
std::pair<double, double> CtPrint::_layout_line_get_width_height(Glib::RefPtr<const Pango::LayoutLine> line)
{
    Pango::Rectangle ink_rect, logical_rect;
    line->get_extents(ink_rect, logical_rect);
    return std::make_pair(logical_rect.get_width() / 1024.0, logical_rect.get_height() / 1024.0);
}

// Returns the Height given the Layout
double CtPrint::_get_height_from_layout(Glib::RefPtr<Pango::Layout> layout)
{
    double height = 0;
    for (int layout_line_idx = 0; layout_line_idx < layout->get_line_count(); ++layout_line_idx)
    {
        Glib::RefPtr<const Pango::LayoutLine> layout_line = layout->get_line(layout_line_idx);
        auto [line_width, line_height] = _layout_line_get_width_height(layout_line);
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
        auto [line_width, line_height] = _layout_line_get_width_height(layout_line);
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
        for (int j = 0; j < tableProxy->get_col_num(); ++i)
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
                auto [line_width, line_height] = _layout_line_get_width_height(layout_line);
                cell_height += line_height;
                if (cols_w[j] < line_width) cols_w[j] = line_width;
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
    CtPrintTableProxy* tableProxy = dynamic_cast<CtPrintTableProxy*>(_widgets[(size_t)idx].get());
    std::vector<std::shared_ptr<CtPrintTableProxy>> new_tables;
    std::shared_ptr<CtPrintTableProxy> new_table;
    for (int row_num = 1 /* with header */; row_num < tableProxy->get_row_num() - 1 /* minus header */; ++row_num)
    {
        new_table = tableProxy->copy(row_num);
        auto table_layouts = _get_table_layouts(context, new_table.get());
        auto table_grid = _get_table_grid(table_layouts, new_table->get_table()->get_col_min());
        double table_height = _get_table_height_from_grid(table_grid);
        if (table_height + BOX_OFFSET > (_page_height - _layout_newline_height)) // hit upper limit
        {
            // this slot is done
            new_table = tableProxy->copy(row_num - 1); // todo: hmm, what if it will be just header?
            new_tables.push_back(new_table);
            new_table.reset();
            // start cycle again with reduced tabled
            tableProxy->remove_first_rows(row_num -1);
            row_num = 0; // will be incremented to 1
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
    CtPrintCodeboxProxy* codeboxProxy = dynamic_cast<CtPrintCodeboxProxy*>(_widgets[(size_t)idx].get());
    std::vector<Glib::ustring> original_splitted_pango = str::split(codeboxProxy->get_text_content(), CtConst::CHAR_NEWLINE.c_str());
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
        if (partial_pango != "")
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
            codeboxProxy->set_text_contenxt(result_pango[i]);
        else
        {
            size_t index = idx+i;
            // add a newline
            print_data->text.insert(print_data->text.begin() + index, CtConst::CHAR_NEWLINE);
            // add a codebox
            _widgets.insert(_widgets.begin() + index, std::shared_ptr<CtPrintCodeboxProxy>(new CtPrintCodeboxProxy(codeboxProxy->get_codebox(), result_pango[i])));
        }
    }
}
