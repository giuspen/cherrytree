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
#include "ct_logging.h"

#include <utility>


std::string dest_id_from_node_and_anchor(int node_id, const std::string& anchor_name)
{
    return fmt::format("n.{}.{}", node_id, anchor_name);
}

std::string dest_id_from_node_id(int node_id) 
{
    return dest_id_from_node_and_anchor(node_id, "");
}

// Generate node name text
std::unique_ptr<CtDestPrintable> generate_node_name_printable(Glib::ustring node_name, int node_id)
{
    auto printable = std::make_unique<CtDestPrintable>("<b><i><span size=\"xx-large\">" + str::xml_escape(node_name)
            + "</span></i></b>" + CtConst::CHAR_NEWLINE + CtConst::CHAR_NEWLINE, dest_id_from_node_id(node_id));
    return printable;
}



void CtExport2Pdf::node_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options, int sel_start, int sel_end)
{
    CtPrintableVector printable_slots;
    Glib::ustring text_font;
    if (tree_iter.get_node_is_rich_text())
    {
        CtExport2Pango::pango_get_from_treestore_node(tree_iter, sel_start, sel_end, printable_slots,
                                                      false /*exclude anchors*/);
        text_font = _pCtMainWin->get_ct_config()->rtFont;
    }
    else
    {
        printable_slots.emplace_back(std::make_shared<CtTextPrintable>(CtExport2Pango().pango_get_from_code_buffer(tree_iter.get_node_text_buffer(), sel_start, sel_end)));
        text_font = tree_iter.get_node_syntax_highlighting() != CtConst::PLAIN_TEXT_ID ? _pCtMainWin->get_ct_config()->codeFont : _pCtMainWin->get_ct_config()->ptFont;
    }
    if (options.include_node_name) {
        printable_slots.emplace(printable_slots.begin(), generate_node_name_printable(tree_iter.get_node_name(), tree_iter.get_node_id()));
    }

    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, printable_slots, text_font, _pCtMainWin->get_ct_config()->codeFont,
                                            _pCtMainWin->get_text_view().get_allocation().get_width());
}

void CtExport2Pdf::node_and_subnodes_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
{
    CtPrintableVector tree_pango_slots;
    Glib::ustring text_font = _pCtMainWin->get_ct_config()->codeFont;
    _nodes_all_export_print_iter(tree_iter, options, tree_pango_slots, text_font);

    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, tree_pango_slots, text_font, _pCtMainWin->get_ct_config()->codeFont,
                                            _pCtMainWin->get_text_view().get_allocation().get_width());
}

void CtExport2Pdf::tree_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
{
    CtPrintableVector tree_printables;
    Glib::ustring text_font = _pCtMainWin->get_ct_config()->codeFont;
    while (tree_iter)
    {
        _nodes_all_export_print_iter(tree_iter, options, tree_printables, text_font);
        ++tree_iter;
    }
    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, tree_printables, text_font, _pCtMainWin->get_ct_config()->codeFont, 
                                            _pCtMainWin->get_text_view().get_allocation().get_width());
}

void CtExport2Pdf::_nodes_all_export_print_iter(const CtTreeIter& tree_iter, const CtExportOptions& options,
                                                CtPrintableVector& tree_printables, Glib::ustring& text_font)
{
    CtPrintableVector node_printables;
    if (tree_iter.get_node_is_rich_text())
    {
        CtExport2Pango().pango_get_from_treestore_node(tree_iter, -1, -1, node_printables, false /*exclude anchors*/);
        text_font =_pCtMainWin->get_ct_config()->rtFont; // text font for all (also eventual code nodes)
    }
    else
    {
        node_printables.emplace_back(std::make_shared<CtTextPrintable>(CtExport2Pango().pango_get_from_code_buffer(tree_iter.get_node_text_buffer(), -1, -1)));
    }
    if (options.include_node_name) {
        node_printables.insert(node_printables.cbegin(), generate_node_name_printable(tree_iter.get_node_name(), tree_iter.get_node_id()));
    }
    if (tree_printables.empty()) {
        tree_printables = node_printables;
    }
    else
    {
        if (options.new_node_page)
        {
            node_printables.emplace(node_printables.cbegin(), std::make_shared<CtPageBreakPrintable>());
            tree_printables.insert(tree_printables.cend(), node_printables.cbegin(), node_printables.cend());
        }
        else
        {
            tree_printables.emplace_back(std::make_shared<CtTextPrintable>(str::repeat(CtConst::CHAR_NEWLINE, 3)));
            if (node_printables.size() > 1)
            {
                node_printables.erase(node_printables.cbegin());
                tree_printables.insert(tree_printables.cend(), node_printables.cbegin(), node_printables.cend());
            }
        }
    }
    
    for (auto& iter: tree_iter->children()) {
        _nodes_all_export_print_iter(_pCtMainWin->get_tree_store().to_ct_tree_iter(iter), options, tree_printables, text_font);
    }
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
                         CtPrintableVector printables, const Glib::ustring& text_font, const Glib::ustring& code_font,
                         int text_window_width)
{
    _pCtMainWin = pCtMainWin;
    _print_info.font = Pango::FontDescription(text_font);
    _print_info.codebox_font = Pango::FontDescription(code_font);
    _print_info.text_window_width = text_window_width;
    _print_info.table_line_thickness = 6;
    

    CtPrintData print_data;
    print_data.printables = std::move(printables);

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

    if (!print_data.warning.empty()){
        _pCtMainWin->get_status_bar().update_status(print_data.warning);
    }
}


// Here we Compute the Lines Positions, the Number of Pages Needed and the Page Breaks
void CtPrint::_on_begin_print_text(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data)
{
    _print_info.page_width = context->get_width();
    _print_info.page_height = context->get_height() * 1.02; // tolerance at bottom of the page

    auto layout_newline = context->create_pango_layout();
    layout_newline->set_font_description(_print_info.font);
    layout_newline->set_width(static_cast<int>(_print_info.page_width * Pango::SCALE));
    layout_newline->set_markup(CtConst::CHAR_NEWLINE);
    _print_info.newline_height = layout_line_get_width_height(layout_newline->get_line(0)).height;
    CtPrintable::PrintInfo p_info = _print_info;
    p_info.print_context = context;


    double total_height = 0;
    for (const auto& printable : print_data->printables)
    {

        printable->setup(p_info);
        auto add_height = printable->height();
        if (add_height < 0) {
            // Forced page break
            add_height = _print_info.page_height - fmod(total_height, _print_info.page_height);
        } 
        total_height += add_height;

    }

    
    auto nb_pages = std::ceil(total_height / _print_info.page_height) + 1;
    if (fmod(total_height, _print_info.page_height) > 0) {
        // Remainder page
        spdlog::debug("Added remainder page");
        ++nb_pages;
    }
    print_data->operation->set_n_pages(nb_pages);
    print_data->nb_pages = nb_pages;
    
}

// This Function is Called For Each Page Set in on_begin_print_text
void CtPrint::_on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data)
{
    // layout num, line num
    auto operation = print_data->operation;
    auto cairo_context = context->get_cairo_context();
    cairo_context->set_source_rgb(0.5, 0.5, 0.5);
    cairo_context->set_font_size(12);
    Glib::ustring page_num_str = std::to_string(page_nr+1) + "/" + std::to_string(operation->property_n_pages());
    cairo_context->move_to(_print_info.page_width/2., _print_info.page_height+17);
    cairo_context->show_text(page_num_str);

    _print_info.print_context = context;
    CtPrintable::PrintingContext print_context {
        .cairo_context = cairo_context,
        .print_info = _print_info,
        .print_data = *print_data,
        .position = {
                .x = 0,
                .y = 0
        }
    };

    while(!print_data->printables.empty() && print_data->curr_printable_i < print_data->printables.size()) {
        try {
            cairo_context->set_source_rgb(0, 0, 0);
            auto printable = print_data->printables.at(print_data->curr_printable_i);

            print_context.position = printable->print(print_context);
            if (printable->done()){
                // Printable has printed all its lines or drawn itself in full or whatever
                print_data->curr_printable_i += 1;
            }
            if (print_context.position.y >= _print_info.page_height) {
                break;
            }

        } catch(std::exception& e) {
            spdlog::error("Exception caught during printing: {}", e.what());
            return;
        }

    }
    spdlog::debug("Finished, page num: {}/{}", page_nr + 1, print_data->nb_pages);
}

// Returns Width and Height of a layout line
Cairo::Rectangle CtPrint::layout_line_get_width_height(Glib::RefPtr<const Pango::LayoutLine> line)
{
    Pango::Rectangle ink_rect, logical_rect;
    line->get_extents(ink_rect, logical_rect);
    Cairo::Rectangle rect;
    rect.width = logical_rect.get_width() / Pango::SCALE;
    rect.height = logical_rect.get_height() / Pango::SCALE;
    return rect;
}

// Returns the Height given the Layout
double CtPrint::get_height_from_layout(Glib::RefPtr<Pango::Layout> layout)
{
    double height = 0;
    for (int layout_line_idx = 0; layout_line_idx < layout->get_line_count(); ++layout_line_idx)
    {
        Glib::RefPtr<const Pango::LayoutLine> layout_line = layout->get_line(layout_line_idx);
        double line_height = layout_line_get_width_height(layout_line).height;
        height += line_height;
    }

    return height + 2 * CtConst::GRID_SLIP_OFFSET;
}

// Returns the Height given the Layout
double CtPrint::get_width_from_layout(Glib::RefPtr<Pango::Layout> layout)
{
    double width = 0;
    for (int layout_line_idx = 0; layout_line_idx < layout->get_line_count(); ++layout_line_idx)
    {
        Glib::RefPtr<const Pango::LayoutLine> layout_line = layout->get_line(layout_line_idx);
        double line_width = layout_line_get_width_height(layout_line).width;
        if (line_width > width)
            width = line_width;
    }
    return width + 2 * CtConst::GRID_SLIP_OFFSET;
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



std::unique_ptr<CtPrintable> printable_from_widget(CtAnchoredWidget* widget, int node_id)
{
    if (!widget) {
        throw std::logic_error("CtWidgetPrintable::from_widget passed nullptr!");
    }

    if (auto* img_anchor = dynamic_cast<CtImageAnchor*>(widget)) return std::make_unique<CtDestPrintable>(img_anchor, node_id);
    else if (auto* image = dynamic_cast<CtImage*>(widget))       return std::make_unique<CtWidgetImagePrintable>(std::make_shared<CtPrintImageProxy>(image));
    else if (auto* table = dynamic_cast<CtTable*>(widget))       return std::make_unique<CtWidgetTablePrintable>(std::make_shared<CtPrintTableProxy>(table, 1, table->get_table_matrix().size()));
    else if (auto* codebox = dynamic_cast<CtCodebox*>(widget))   return std::make_unique<CtWidgetCodeboxPrintable>(std::make_shared<CtPrintCodeboxProxy>(codebox));

    else {
        throw std::logic_error("CtWidgetPrintable::from_widget passed unknown widget type");
    }
}

// Given a treestore iter returns the Pango rich text
void CtExport2Pango::pango_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end,
                                                   CtPrintableVector& out_printables, bool exclude_anchors)
{
    auto curr_buffer = node_iter.get_node_text_buffer();
    std::list<CtAnchoredWidget*> out_widgets = node_iter.get_embedded_pixbufs_tables_codeboxes(sel_start, sel_end);
    if (exclude_anchors) {
        out_widgets.remove_if([](CtAnchoredWidget* widget) { return dynamic_cast<CtImageAnchor*>(widget); });
    }
    out_printables.clear();
    int start_offset = sel_start < 1 ? 0 : sel_start;
    for (auto widget: out_widgets)
    {
        int end_offset = widget->getOffset();
        CtPrintableVector slots = _pango_process_slot(start_offset, end_offset, curr_buffer);
        out_printables.insert(out_printables.cend(), slots.begin(), slots.end());
        try {
            std::shared_ptr<CtPrintable> p_widget = printable_from_widget(widget, node_iter.get_node_id());
            out_printables.emplace_back(std::move(p_widget));
        } catch(std::exception& e) {
            spdlog::error("Exception occurred while trying to convert widget to printable: {}", e.what());
        }
        
        start_offset = end_offset;
    }
    CtPrintableVector slots;
    if (sel_start < 0) {
        slots = _pango_process_slot(start_offset, curr_buffer->end().get_offset(), curr_buffer);
    }
    else {
        slots = _pango_process_slot(start_offset, sel_end, curr_buffer);
    }
    out_printables.insert(out_printables.end(), slots.begin(), slots.end());
    
}

// Process a Single Pango Slot
CtPrintableVector CtExport2Pango::_pango_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer)
{
    CtPrintableVector curr_printables;
    CtTextIterUtil::generic_process_slot(start_offset, end_offset, curr_buffer,
                                         [&curr_printables](Gtk::TextIter& start_iter, Gtk::TextIter& curr_iter, std::map<std::string_view, std::string>& curr_attributes) {
        curr_printables.emplace_back(CtExport2Pango::_pango_text_serialize(start_iter, curr_iter, curr_attributes));
    });
    return curr_printables;
}

// Adds a slice to the Pango Text
std::unique_ptr<CtPrintable>
CtExport2Pango::_pango_text_serialize(const Gtk::TextIter& start_iter, Gtk::TextIter end_iter, const std::map<std::string_view, std::string> &curr_attributes)
{
    Glib::ustring pango_attrs;
    bool superscript_active = false;
    bool subscript_active = false;
    bool monospace_active = false;
    std::string link_url;
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
        if (tag_property == CtConst::TAG_LINK) {
            link_url = curr_attributes.at(tag_property);
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

    if (!link_url.empty()) {
        return std::make_unique<CtLinkPrintable>(std::move(tagged_text), std::move(link_url));
    }
    return std::make_unique<CtTextPrintable>(tagged_text);
}


void CtWidgetImagePrintable::setup(const PrintInfo& print_info)
{
    auto pixbuf = _widget_proxy->get_pixbuf();
    // don't know curr_x, so will recalc scale again in draw function
    double scale_w = print_info.page_width / pixbuf->get_width();
    double scale_h =
            (print_info.page_height - print_info.newline_height - CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT) /
            pixbuf->get_height();
    double scale = std::min(scale_w, scale_h);
    if (scale > 1.0) scale = 1.0;
    double pixbuf_height = pixbuf->get_height() * scale + CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT;

    _last_height = pixbuf_height;
}

CtPrintable::PrintPosition CtWidgetImagePrintable::print(const CtPrintable::PrintingContext& context)
{
    auto pos = context.position;
    if (!_done) {
        auto pixbuf = _widget_proxy->get_pixbuf();

        if (pixbuf->get_height() < context.print_info.page_height && context.position.y + pixbuf->get_height() > context.print_info.page_height) {
            // Possible to fit on single page, but not this one
            pos.y += pixbuf->get_height();
            return pos;
        }

        // should recalc scale because curr_x is changed
        double scale_w = (context.print_info.page_width - context.position.x) / pixbuf->get_width();
        double scale_h = (context.print_info.page_height - context.print_info.newline_height -
                          CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT) / pixbuf->get_height();
        double scale = std::min(scale_w, scale_h);

        if (scale > 1.0) scale = 1.0; // Cap scale

        double pixbuf_width = pixbuf->get_width() * scale;
        double pixbuf_height = pixbuf->get_height() * scale;

        auto& cairo_context = context.cairo_context;
        cairo_context->save();
        cairo_context->scale(scale, scale);
        Gdk::Cairo::set_source_pixbuf(cairo_context, pixbuf, context.position.x,
                                      context.position.y);
        cairo_context->paint();
        cairo_context->restore();

        pos.x += pixbuf_width;
        pos.y += pixbuf_height;
        _done = true;
    }
    return pos;
}

double CtWidgetImagePrintable::width() const
{
    return _last_width;
}

double CtWidgetImagePrintable::height() const
{
    return _last_height;
}


std::pair<std::vector<double>, std::vector<double>> CtWidgetTablePrintable::_get_table_grid(std::vector<std::vector<Glib::RefPtr<Pango::Layout>>>& table_layouts, int col_min) const
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
                auto line_size = CtPrint::layout_line_get_width_height(layout_line);
                cell_height += line_size.height;
                if (cols_w[j] < line_size.width) cols_w[j] = line_size.width;
            }
            if (rows_h[i] < cell_height) rows_h[i] = cell_height;
        }
    }
    return std::make_pair(rows_h, cols_w);
}

double CtWidgetTablePrintable::_get_table_height_from_grid(const CtWidgetTablePrintable::tbl_grid_t& table_grid, int table_line_thickness)
{
    double table_height = 0;
    for (auto& row_h : table_grid.first)
        table_height += row_h + table_line_thickness;
    return table_height;
}

std::vector<std::vector<Glib::RefPtr<Pango::Layout>>> CtWidgetTablePrintable::_get_table_layouts(const PrintInfo& print_info) const
{
    std::vector<std::vector<Glib::RefPtr<Pango::Layout>>> table_layouts;
    for (int i = 0; i < _widget_proxy->get_row_num(); ++i) {
        std::vector<Glib::RefPtr<Pango::Layout>> layouts;
        for (int j = 0; j < _widget_proxy->get_col_num(); ++j) {
            Glib::ustring text = str::xml_escape(_widget_proxy->get_cell(i, j));
            if (i == 0) text = "<b>" + text + "</b>";
            auto layout = print_info.print_context->create_pango_layout();
            layout->set_font_description(print_info.font);
            layout->set_width(static_cast<int>(_widget_proxy->get_table()->get_col_max() * Pango::SCALE));
            layout->set_wrap(Pango::WRAP_WORD_CHAR);
            layout->set_markup(text);
            layouts.push_back(layout);
        }
        table_layouts.push_back(std::move(layouts));
    }
    return table_layouts;
}


void CtWidgetTablePrintable::setup(const PrintInfo& print_info)
{
    _tbl_layouts = _get_table_layouts(print_info);
    _tbl_grid = _get_table_grid(_tbl_layouts, _widget_proxy->get_table()->get_col_min());
}

Glib::RefPtr<Pango::Layout> CtWidgetCodeboxPrintable::_calc_layout(const PrintInfo& print_info) const
{
    auto layout = print_info.print_context->create_pango_layout();
    layout->set_font_description(print_info.codebox_font);
    double codebox_width = _widget_proxy->get_width_in_pixels() ? _widget_proxy->get_frame_width() : print_info.text_window_width * _widget_proxy->get_frame_width()/100.;
    if (codebox_width > print_info.page_width) {
        codebox_width = print_info.page_width;
    }

    layout->set_width(static_cast<int>(codebox_width * Pango::SCALE));
    layout->set_wrap(Pango::WRAP_WORD_CHAR);
    layout->set_markup(_widget_proxy->get_text_content());
    return layout;
}

void CtWidgetCodeboxPrintable::setup(const PrintInfo& print_info)
{
    _layout = _calc_layout(print_info);
}

CtPrintable::PrintPosition CtWidgetCodeboxPrintable::print(const CtPrintable::PrintingContext& context)
{
    auto pos = context.position;
    if (!_done) {
        auto c_height = height();
        if (pos.y + c_height > context.print_info.page_height) {
            spdlog::debug("Codebox too big to fit, needs {} - {} is available", c_height, context.print_info.page_height - pos.y);
            return pos;
        }
        pos.y += c_height;
        pos.x += width();


        DrawContext draw_context{
                .height = c_height,
                .width = width(),
                .x = context.position.x,
                .y = context.position.y - c_height
        };

        _draw_box(context.cairo_context, draw_context);
        _draw_code(context.cairo_context, draw_context);

        _done = true;
    }
    return pos;
}

void CtWidgetCodeboxPrintable::_draw_box(const Cairo::RefPtr<Cairo::Context>& cairo_context, const DrawContext& draw_context) {
    cairo_context->set_source_rgba(0, 0, 0, 0.3);
    cairo_context->rectangle(draw_context.x, draw_context.y, draw_context.width, draw_context.height);
    cairo_context->stroke();
}

void CtWidgetCodeboxPrintable::_draw_code(const Cairo::RefPtr<Cairo::Context>& cairo_context, const DrawContext& draw_context) {
    double y = draw_context.y;
    cairo_context->set_source_rgb(0, 0, 0);
    for (int layout_line_idx = 0; layout_line_idx < _layout->get_line_count(); ++layout_line_idx)
    {
        auto layout_line = _layout->get_line(layout_line_idx);
        double line_height = CtPrint::layout_line_get_width_height(layout_line).height;
        cairo_context->move_to(draw_context.x + CtConst::GRID_SLIP_OFFSET, y + line_height);
        y += line_height;
        layout_line->show_in_cairo_context(cairo_context);
    }
}

double CtWidgetCodeboxPrintable::width() const
{
    return CtPrint::get_width_from_layout(_layout);
}

double CtWidgetCodeboxPrintable::height() const
{
    return CtPrint::get_height_from_layout(_layout);
}


void CtTextPrintable::setup(const PrintInfo& print_info)
{
    auto& print_context = print_info.print_context;
    _is_newline = _text == CtConst::CHAR_NEWLINE;

    _layout = print_context->create_pango_layout();
    _layout->set_font_description(print_info.font);
    auto page_width = static_cast<int>(print_info.page_width);
    _layout->set_width(page_width * Pango::SCALE);
    _layout->set_markup(_text);
}

double CtTextPrintable::height() const
{
    double height = 0;
    if (_is_newline) {
        height = CtPrint::get_height_from_layout(_layout);
    } else {
        height += _calc_lines_heights();
    }
    return height;
}

double CtTextPrintable::_calc_lines_heights() const 
{
    // Both of these start are 0 ... (n - 1)
    auto nb_lines = _layout->get_line_count() - 1;
    int line_num = 0;
    double total_height = 0;
    while(line_num < nb_lines || _is_newline) {
        auto line = _layout->get_line(line_num);
        total_height += CtPrint::layout_line_get_width_height(line).height;
        ++line_num;
        if (_is_newline) break;
    }
    return total_height;
}

CtPrintable::PrintPosition CtTextPrintable::print(const PrintingContext& context)
{
    PrintPosition pos = context.position;

    auto line_count = _layout->get_line_count();
    while(true) {
        if (_line_index >= line_count) {
            _done = true;
            break;
        }
        if (pos.y >= context.print_info.page_height) {
            // Out of space
            spdlog::debug("Out of space on pos: {}/{}", pos.y, context.print_info.page_height);
            break;
        }

        auto line = _layout->get_line(_line_index);
        context.cairo_context->move_to(pos.x, pos.y);
        line->show_in_cairo_context(context.cairo_context);

        if(_line_index < line_count - 1 || is_newline()) {
            pos.y += CtPrint::layout_line_get_width_height(line).height;
            pos.x = 0;
        } else {
            pos.x += CtPrint::layout_line_get_width_height(line).width;
        }

        ++_line_index;
    }
    return pos;
}

double CtTextPrintable::width() const
{
    if (!_layout) throw std::logic_error("CtTextPrintable::width called before setup");
    return CtPrint::get_width_from_layout(_layout);
}




std::size_t CtTextPrintable::lines() const
{
    return _layout->get_line_count();
}


CtPrintable::PrintPosition CtWidgetTablePrintable::print(const CtPrintable::PrintingContext& context)
{
    auto pos = context.position;
    pos.x += width();
    pos.y += height();

    if (pos.y < context.print_info.page_height && !_done) {
        // Only print if it fits
        DrawingContext draw_context;
        draw_context.cairo_context = context.cairo_context;
        draw_context.table_layouts = _tbl_layouts;
        draw_context.table_grid = _tbl_grid;
        draw_context.width = _get_table_width_from_grid(draw_context.table_grid, context.print_info.table_line_thickness);
        draw_context.height = _get_table_height_from_grid(draw_context.table_grid, context.print_info.table_line_thickness);
        draw_context.x0 = context.position.x;
        draw_context.y0 = context.position.y;
        draw_context.line_thickness = context.print_info.table_line_thickness;

        _table_draw_grid(draw_context);
        _table_draw_text(draw_context);


        _done = true;
    }
    return pos;
}

void CtWidgetTablePrintable::_table_draw_grid(const DrawingContext& draw_context)
{
    auto& cairo_context = draw_context.cairo_context;
    double x = draw_context.x0;
    double y = draw_context.y0;
    cairo_context->set_source_rgba(0, 0, 0, 0.3);
    // draw lines
    cairo_context->move_to(x, y);
    cairo_context->line_to(x + draw_context.width, y);
    for (auto& row_h: draw_context.table_grid.first)
    {
        y += row_h + draw_context.line_thickness;
        cairo_context->move_to(x, y);
        cairo_context->line_to(x + draw_context.width, y);
    }
    // draw columns
    y = draw_context.y0;
    cairo_context->move_to(x, y);
    cairo_context->line_to(x, y + draw_context.height);
    for (auto& col_w: draw_context.table_grid.second)
    {
        x += col_w + draw_context.line_thickness;
        cairo_context->move_to(x, y);
        cairo_context->line_to(x, y + draw_context.height);
    }
    cairo_context->stroke();
}


// Draw the text inside of the Table Cells
void CtWidgetTablePrintable::_table_draw_text(const DrawingContext& context)
{
    auto& cairo_context = context.cairo_context;
    auto& table_grid = context.table_grid;
    cairo_context->set_source_rgb(0, 0, 0);
    double y = context.y0;
    for (size_t i = 0; i < table_grid.first.size(); ++i)
    {
        double row_h = table_grid.first.at(i);
        double x = context.x0 + CtConst::GRID_SLIP_OFFSET;
        for (size_t j = 0; j < table_grid.second.size(); ++j) {
            double col_w = table_grid.second.at(j);
            auto layout_cell = context.table_layouts.at(i).at(j);
            double local_y = y;
            for (int layout_line_id = 0; layout_line_id < layout_cell->get_line_count(); ++layout_line_id)
            {
                auto layout_line = layout_cell->get_line(layout_line_id);
                double line_height = CtPrint::layout_line_get_width_height(layout_line).height;
                cairo_context->move_to(x, local_y + line_height);
                local_y += line_height;
                layout_line->show_in_cairo_context(cairo_context);
            }
            x += col_w + context.line_thickness;
        }
        y += row_h + context.line_thickness;
    }
}

// Returns the Table Width given the table_grid vector
double CtWidgetTablePrintable::_get_table_width_from_grid(const std::pair<std::vector<double>, std::vector<double>>& table_grid, int table_line_thickness)
{
    double table_width = 0;
    for (auto& col_w: table_grid.second)
        table_width += col_w + table_line_thickness;
    return table_width;
}

double CtWidgetTablePrintable::width() const
{
    return _get_table_width_from_grid(_tbl_grid, 6);
}

double CtWidgetTablePrintable::height() const
{
    return _get_table_height_from_grid(_tbl_grid, 6);
}

void CtPageBreakPrintable::setup(const CtPrintable::PrintInfo& print_info)
{
    _p_height = print_info.page_height; // Just break by being the size of a page
}

CtPrintable::PrintPosition CtPageBreakPrintable::print(const CtPrintable::PrintingContext& context)
{
    PrintPosition pos = context.position;
    pos.y += context.print_info.page_height; // Add needed diff to break
    _done = true;
    return pos;
}

double CtPageBreakPrintable::height() const
{
    return -1; // < 0 is a forced page break
}

double CtPageBreakPrintable::width() const
{
    return 0;
}

CtLinkPrintable::CtLinkPrintable(Glib::ustring title, std::string url) : CtTextPrintable(fmt::format("<span fgcolor='blue'>{}</span>", std::move(title))), _url(std::move(url)) 
{
    try {
        Glib::RefPtr<Glib::Regex> node_link_reg = Glib::Regex::create("node\\s([0-9]+).?(.*)?");
        Glib::MatchInfo m_info;
        node_link_reg->match(_url, m_info);
        if (m_info.matches()) {
            // Internal url
            std::string node_id = m_info.fetch(1);
            std::string sect_id = m_info.fetch(2);

            _url = dest_id_from_node_and_anchor(std::stoi(node_id), sect_id);
            _is_internal = true;
            spdlog::debug("Created link to: {}", _url);
        } else {
            // Unknown, try and unwrap it
            _url = CtStrUtil::external_uri_from_internal(_url);
        }
    } catch(const std::exception& e) {
        spdlog::error("CtLinkPrintable failed to convert url ({}) to internal (error msg: {})", _url, e.what());
    }
}

template<typename PRINT_CALLBACK_T>
CtPrintable::PrintPosition print_with_cairo_tag(cairo_t* cairo_obj, const PRINT_CALLBACK_T& callback, const std::string& tag_name, const std::string& tag_attrs) 
{
    cairo_tag_begin(cairo_obj, tag_name.c_str(), tag_attrs.c_str());
    CtPrintable::PrintPosition pos = callback();
    cairo_tag_end(cairo_obj, tag_name.c_str());
    return pos;
}


// Does the same as CtTextPrintable but applies a link tag
CtPrintable::PrintPosition CtLinkPrintable::print(const PrintingContext& context) 
{
    auto pos = context.position;
    if (!done()) {
        try {
            std::string attrs = fmt::format("{}='{}'", _is_internal ? "dest" : "uri", _url);
            pos = print_with_cairo_tag(context.cairo_context->cobj(), [this, &context]() {
                return CtTextPrintable::print(context);
            }, CAIRO_TAG_LINK, attrs);
    
        } catch(const std::exception& e) {
            spdlog::error("Exception caught in CtLinkPrintable while printing: ({}); Link was: <{}>", e.what(), _url);
        }
    }
    return pos;
}





CtDestPrintable::CtDestPrintable(CtImageAnchor* anchor, int node_id) : CtTextPrintable("⚓"), _id(dest_id_from_node_and_anchor(node_id, anchor->get_anchor_name())) {}


CtPrintable::PrintPosition CtDestPrintable::print(const PrintingContext& context) 
{
    auto pos = context.position;
    if (!_done) {
        try {
            spdlog::debug("Printed dest: {}", _id);
            
            pos = print_with_cairo_tag(context.cairo_context->cobj(), [this, &context]{
                return CtTextPrintable::print(context); 
            }, CAIRO_TAG_DEST, fmt::format("name='{}'", _id));

        } catch(const std::exception& e) {
            spdlog::error("Exception while printing dest: {}", e.what());
        }
    }
    return pos;
}
