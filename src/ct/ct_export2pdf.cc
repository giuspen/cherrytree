/*
 * ct_export2pdf.cc
 *
 * Copyright 2009-2020
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

#include "ct_export2pdf.h"
#include "ct_dialogs.h"
#include <utility>

namespace {
    template<typename NodeIdType>
    Glib::ustring generate_tag(NodeIdType node_id, const Glib::ustring& anchor) { return fmt::format("n.{}.{}", node_id, anchor); }
}

// Given a treestore iter returns the Pango rich text
void CtExport2Pango::pango_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end, std::vector<CtPangoObjectPtr>& out_slots)
{
    auto curr_buffer = node_iter.get_node_text_buffer();

    std::list<CtAnchoredWidget*> out_widgets = node_iter.get_embedded_pixbufs_tables_codeboxes(sel_start, sel_end);
    int start_text_offset = sel_start < 1 ? 0 : sel_start;
    for (auto widget: out_widgets)
    {
        int end_text_offset = widget->getOffset();
        _pango_process_slot(start_text_offset, end_text_offset, curr_buffer, out_slots);

        int widget_indent = 0;
        CtCurrAttributesMap emtpy_attributes, iter_attributes;
        if (CtTextIterUtil::rich_text_attributes_update(curr_buffer->get_iter_at_offset(widget->getOffset()), emtpy_attributes, iter_attributes))
        {
            auto attr_iter = iter_attributes.find(CtConst::TAG_INDENT);
            if (attr_iter != iter_attributes.end())
                widget_indent = CtConst::INDENT_MARGIN * std::stoi(attr_iter->second);
        }

        if (CtImageAnchor* anchor = dynamic_cast<CtImageAnchor*>(widget))
            out_slots.emplace_back(std::make_shared<CtPangoDest>("<sup>⚓</sup>", CtConst::RICH_TEXT_ID, widget_indent, "name='" + generate_tag(node_iter.get_node_id(), anchor->get_anchor_name()) + "'"));
        else
            out_slots.emplace_back(std::make_shared<CtPangoWidget>(widget, widget_indent));
        start_text_offset = end_text_offset;
    }

    int end_offset = sel_end < 0 ? curr_buffer->end().get_offset() : sel_end;
    _pango_process_slot(start_text_offset, end_offset, curr_buffer, out_slots);
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
    //if (pango_text.empty() || pango_text[pango_text.size()-1] != g_utf8_get_char(CtConst::CHAR_NEWLINE))
    //    pango_text += CtConst::CHAR_NEWLINE;
    return pango_text;
}

// Process a Single Pango Slot
void CtExport2Pango::_pango_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer, std::vector<CtPangoObjectPtr>& out_slots)
{
    CtTextIterUtil::generic_process_slot(start_offset, end_offset, curr_buffer,
                                         [&](Gtk::TextIter& start_iter, Gtk::TextIter& curr_iter, CtCurrAttributesMap& curr_attributes) {
            _pango_text_serialize(start_iter, curr_iter, curr_attributes, out_slots);
    });
}

// Adds a slice to the Pango Text
void CtExport2Pango::_pango_text_serialize(const Gtk::TextIter& start_iter, Gtk::TextIter end_iter,
                                           const CtCurrAttributesMap& curr_attributes, std::vector<CtPangoObjectPtr>& out_slots)
{
    Glib::ustring pango_attrs;
    bool superscript_active = false;
    bool subscript_active = false;
    bool monospace_active = false;
    int indent = 0;
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
            else if (tag_property == CtConst::TAG_INDENT)
            {
                indent = CtConst::INDENT_MARGIN * std::stoi(property_value);
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

    // split by \n to use Layout::set_indent properly
    std::vector<Glib::ustring> lines = str::split(start_iter.get_text(end_iter), "\n");
    for (size_t i = 0; i < lines.size(); ++ i)
    {
        Glib::ustring tagged_text = str::xml_escape(lines[i]);
        if (!pango_attrs.empty())
            tagged_text = "<span" + pango_attrs + ">" + tagged_text + "</span>";

        if (superscript_active) tagged_text = "<sup>" + tagged_text + "</sup>";
        if (subscript_active) tagged_text = "<sub>" + tagged_text + "</sub>";
        if (monospace_active) tagged_text = "<tt>" + tagged_text + "</tt>";

        if (!link_url.empty()) {
            out_slots.emplace_back(_pango_link_url(tagged_text, link_url, indent));
        } else {
            out_slots.emplace_back(std::make_shared<CtPangoText>(tagged_text, CtConst::RICH_TEXT_ID, indent));
        }

        // add '\n' between lines
        if (lines.size() > 1 && i < lines.size() - 1) {
            out_slots.emplace_back(std::make_shared<CtPangoText>("\n", CtConst::RICH_TEXT_ID, indent));
        }
    }
}

std::shared_ptr<CtPangoText> CtExport2Pango::_pango_link_url(const Glib::ustring& tagged_text, const Glib::ustring& link, int indent)
{
    CtLinkEntry link_entry = CtMiscUtil::get_link_entry(link);
    Glib::ustring uri;
    if (link_entry.type == CtConst::LINK_TYPE_NODE)
        uri = "dest='" + str::xml_escape(generate_tag(link_entry.node_id, link_entry.anch)) + "'";
    else if (link_entry.type == CtConst::LINK_TYPE_WEBS)
        uri = "uri='" + str::xml_escape(link_entry.webs) + "'";
    else if (link_entry.type == CtConst::LINK_TYPE_FILE)
        uri = "uri='" + str::xml_escape(link_entry.file) + "'";
    else if (link_entry.type == CtConst::LINK_TYPE_FOLD)
        uri = "uri='" + str::xml_escape(link_entry.fold) + "'";
    else
    {
        spdlog::debug("invalid link entry {}, text {}", link, tagged_text);
        return std::make_shared<CtPangoText>(tagged_text, CtConst::RICH_TEXT_ID, indent);
    }

    Glib::ustring blue_text = "<span fgcolor='blue'><u>" + tagged_text + "</u></span>";
    return std::make_shared<CtPangoLink>(blue_text, indent, uri);
}







void CtExport2Pdf::node_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options, int sel_start, int sel_end)
{
    std::vector<CtPangoObjectPtr> pango_slots;
    if (tree_iter.get_node_is_rich_text())
        CtExport2Pango().pango_get_from_treestore_node(tree_iter, sel_start, sel_end, pango_slots);
    else {
        Glib::ustring text = CtExport2Pango().pango_get_from_code_buffer(tree_iter.get_node_text_buffer(), sel_start, sel_end);
        pango_slots.push_back(std::make_shared<CtPangoText>(text, tree_iter.get_node_syntax_highlighting(), 0));
    }

    if (options.include_node_name)
        pango_slots.emplace(pango_slots.begin(), _generate_pango_node_name(tree_iter));

    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, pango_slots);
}

void CtExport2Pdf::node_and_subnodes_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
{
    std::vector<CtPangoObjectPtr> tree_pango_slots;
    _nodes_all_export_print_iter(tree_iter, options, tree_pango_slots);

    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, tree_pango_slots);
}

void CtExport2Pdf::tree_export_print(const fs::path& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
{
    std::vector<CtPangoObjectPtr> tree_pango_slots;
    Glib::ustring text_font = _pCtMainWin->get_ct_config()->codeFont;
    while (tree_iter)
    {
        _nodes_all_export_print_iter(tree_iter, options, tree_pango_slots);
        ++tree_iter;
    }
    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, tree_pango_slots);
}

void CtExport2Pdf::_nodes_all_export_print_iter(CtTreeIter tree_iter, const CtExportOptions& options, std::vector<CtPangoObjectPtr>& tree_pango_slots)
{
    std::vector<CtPangoObjectPtr> node_pango_slots;
    if (tree_iter.get_node_is_rich_text())
        CtExport2Pango().pango_get_from_treestore_node(tree_iter, -1, -1, node_pango_slots);
    else {
        Glib::ustring text = CtExport2Pango().pango_get_from_code_buffer(tree_iter.get_node_text_buffer(), -1, -1);
        node_pango_slots.push_back(std::make_shared<CtPangoText>(text, tree_iter.get_node_syntax_highlighting(), 0));
    }

    if (options.include_node_name)
        node_pango_slots.emplace(node_pango_slots.begin(), _generate_pango_node_name(tree_iter));

    if (tree_pango_slots.empty())
        tree_pango_slots = node_pango_slots;
    else
    {
        if (options.new_node_page)
            tree_pango_slots.push_back(std::make_shared<CtPangoNewPage>());
        else
            tree_pango_slots.push_back(std::make_shared<CtPangoText>(str::repeat(CtConst::CHAR_NEWLINE, 3), tree_iter.get_node_syntax_highlighting(), 0));
        vec::vector_extend(tree_pango_slots, node_pango_slots);
    }

    for (auto& iter: tree_iter->children())
        _nodes_all_export_print_iter(_pCtMainWin->get_tree_store().to_ct_tree_iter(iter), options, tree_pango_slots);
}

CtPangoObjectPtr CtExport2Pdf::_generate_pango_node_name(CtTreeIter tree_iter)
{
    Glib::ustring text = "<b><i><span size=\"xx-large\">" + str::xml_escape(tree_iter.get_node_name()) + "</span></i></b>" + CtConst::CHAR_NEWLINE + CtConst::CHAR_NEWLINE;
    auto slot = std::make_shared<CtPangoDest>(text, tree_iter.get_node_syntax_highlighting(), 0, "name='" + generate_tag(tree_iter.get_node_id(), "") + "'");
    return slot;
}






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
void CtPrint::print_text(CtMainWin* pCtMainWin, const fs::path& pdf_filepath, const std::vector<CtPangoObjectPtr>& slots)
{
    _pCtMainWin = pCtMainWin;

    CtPrintData print_data;
    print_data.slots = slots;

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
}

// Here we Compute the Lines Positions, the Number of Pages Needed and the Page Breaks
void CtPrint::_on_begin_print_text(const Glib::RefPtr<Gtk::PrintContext>& context, CtPrintData* print_data)
{
    auto get_font_with_fallback_ = [](Pango::FontDescription font, const std::string& fallbackFont) {
#ifdef _WIN32
        // explicit fallback is needed on Win32, linux works OK without it
        font.set_family(font.get_family() + "," + fallbackFont);
#else
        (void)fallbackFont; // to silence the warning
#endif
        return font;
    };

    print_data->context = context;
    _rich_font = get_font_with_fallback_(Pango::FontDescription(_pCtMainWin->get_ct_config()->rtFont), _pCtMainWin->get_ct_config()->fallbackFontFamily);
    _plain_font = get_font_with_fallback_(Pango::FontDescription(_pCtMainWin->get_ct_config()->ptFont), _pCtMainWin->get_ct_config()->fallbackFontFamily);
    _code_font = get_font_with_fallback_(Pango::FontDescription(_pCtMainWin->get_ct_config()->codeFont), "monospace");
    _text_window_width = _pCtMainWin->get_text_view().get_allocation().get_width();
    _table_text_row_height = _rich_font.get_size()/Pango::SCALE;
    _table_line_thickness = 6;
    _page_width = context->get_width();
    _page_height = context->get_height() * 1.02; // tolerance at bottom of the page
    _layout_newline_height = [&](){
        auto layout_newline = context->create_pango_layout();
        layout_newline->set_font_description(_rich_font);
        layout_newline->set_width(int(_page_width * Pango::SCALE));
        layout_newline->set_markup(CtConst::CHAR_NEWLINE);
        return _get_width_height_from_layout_line(layout_newline->get_line(0)).height;
    }();

    bool any_image_resized = false;
    for (auto slot: print_data->slots)
    {
        if (dynamic_cast<CtPangoNewPage*>(slot.get()))
            print_data->pages.new_page();
        else if (auto pango_text = dynamic_cast<CtPangoText*>(slot.get()))
            _process_pango_text(print_data, pango_text);
        else if (auto pango_widget = dynamic_cast<CtPangoWidget*>(slot.get()))
        {
            if (CtImage* image = dynamic_cast<CtImage*>(pango_widget->widget))
                _process_pango_image(print_data, image, pango_widget->indent, any_image_resized);
            else if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(pango_widget->widget))
                _process_pango_codebox(print_data, codebox, pango_widget->indent);
            else if (CtTable* table = dynamic_cast<CtTable*>(pango_widget->widget))
                _process_pango_table(print_data, table, pango_widget->indent);
        }
    }

    print_data->operation->set_n_pages(print_data->pages.size());
    if (any_image_resized)
    {
        print_data->warning = Glib::ustring(_("Warning: One or More Images Were Reduced to Enter the Page")) + " ("
                                       + std::to_string(static_cast<int>(_page_width))+ "x" + std::to_string(static_cast<int>(_page_height)) + ")";
    }
}

void CtPrint::_on_draw_page_text(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr, CtPrintData* print_data)
{
    auto operation = print_data->operation;
    auto cairo_context = context->get_cairo_context();

    // draw page number
    cairo_context->set_source_rgb(0.5, 0.5, 0.5);
    cairo_context->set_font_size(12);
    Glib::ustring page_num_str = std::to_string(page_nr+1) + "/" + std::to_string(operation->property_n_pages());
    cairo_context->move_to(_page_width/2., _page_height+17);
    cairo_context->show_text(page_num_str);

    //cairo_context->set_source_rgba(0.3, 0, 0, 0.3);
    //cairo_context->rectangle(0, 0, _page_width, _page_height);
    //cairo_context->stroke();

    auto& page = print_data->pages.get_page(page_nr);
    for (auto& line: page.lines)
    {
        for (CtPageElementPtr element: line.elements)
        {
            if (auto page_text = dynamic_cast<CtPageText*>(element.get()))
            {
                cairo_context->set_source_rgb(0, 0, 0);
                cairo_context->move_to(page_text->x, line.y);
                page_text->layout_line->show_in_cairo_context(cairo_context);

                //auto size = _get_width_height_from_layout_line(page_text->layout_line);
                //cairo_context->set_source_rgba(0.3, 0, 0, 0.3);
                //cairo_context->rectangle(page_text->x, page_text->y - size.height, size.width, size.height);
                //cairo_context->stroke();
            }
            else if (auto page_tag = dynamic_cast<CtPageTag*>(element.get()))
            {
                cairo_context->set_source_rgb(0, 0, 0);

                cairo_tag_begin(cairo_context->cobj(), page_tag->tag_name.c_str(), page_tag->tag_attr.c_str());
                cairo_context->move_to(page_tag->x, line.y);
                page_tag->layout_line->show_in_cairo_context(cairo_context);
                cairo_tag_end(cairo_context->cobj(), page_tag->tag_name.c_str());
            }
            else if (CtPageImage* page_image = dynamic_cast<CtPageImage*>(element.get()))
            {
                auto scale = page_image->scale;
                auto pixbuf = page_image->image->get_pixbuf();
                double pixbuf_height = pixbuf->get_height() * scale;
                cairo_context->save();
                cairo_context->scale(scale, scale);
                Gdk::Cairo::set_source_pixbuf(cairo_context, pixbuf, page_image->x / scale, (line.y - pixbuf_height) / scale);
                cairo_context->paint();
                cairo_context->restore();
            }
            else if (CtPageCodebox* page_codebox = dynamic_cast<CtPageCodebox*>(element.get()))
            {
                double codebox_height = _get_height_from_layout(page_codebox->layout);
                double codebox_width = _get_width_from_layout(page_codebox->layout);
                _draw_codebox_box(cairo_context, page_codebox->x, line.y - codebox_height, codebox_width, codebox_height);
                _draw_codebox_code(cairo_context, page_codebox->layout, page_codebox->x, line.y - codebox_height);
            }
            else if (CtPageTable* page_table = dynamic_cast<CtPageTable*>(element.get()))
            {
                std::vector<double> rows_h, cols_w;
                _table_get_grid(page_table->layouts, page_table->col_min, rows_h, cols_w);
                double table_width = _table_get_width_height(cols_w);
                double table_height = _table_get_width_height(rows_h);
                _draw_table_grid(cairo_context, rows_h, cols_w, page_table->x, line.y - table_height, table_width, table_height);
                _draw_table_text(cairo_context, rows_h, cols_w, page_table->layouts, page_table->x, line.y - table_height);
            }
        }
    }
}

void CtPrint::_process_pango_text(CtPrintData* print_data, CtPangoText* text_slot)
{
    auto context = print_data->context;
    CtPrintPages& pages = print_data->pages;
    Pango::FontDescription* font = [&]() {
        if (text_slot->synt_highl == CtConst::RICH_TEXT_ID) return &_rich_font;
        if (text_slot->synt_highl == CtConst::PLAIN_TEXT_ID) return &_plain_font;
        return &_code_font;
    }();

    Glib::ustring tag_name, tag_attr;
    if (CtPangoLink* pango_link = dynamic_cast<CtPangoLink*>(text_slot)) {
        tag_name = CAIRO_TAG_LINK;
        tag_attr = pango_link->link;
    }
    else if (CtPangoDest* pango_dest = dynamic_cast<CtPangoDest*>(text_slot)) {
        tag_name = CAIRO_TAG_DEST;
        tag_attr = pango_dest->dest;
    }

    int paragraph_width = _page_width - text_slot->indent;

    auto layout = context->create_pango_layout();
    layout->set_font_description(*font);
    layout->set_width(int(paragraph_width  * Pango::SCALE));
    // the next line fixes the link issue, allowing to start paragraphs from where a link ends
    // don't apply paragraph indent because set_indent will work only for the first line
    // also avoid `\n` because new lines also got indent
    if (text_slot->text != "\n")
        layout->set_indent(int(pages.last_line().cur_x  * Pango::SCALE));
    layout->set_markup(text_slot->text);

    int layout_count = layout->get_line_count();
    for (int i = 0; i < layout_count; ++i)
    {
        auto layout_line = layout->get_line(i);
        auto size = _get_width_height_from_layout_line(layout_line);

        if (!pages.last_line().test_element_height(size.height, _page_height))
            pages.line_on_new_page();

        // situation when a bit of space is left but pango cannot wrap the first word
        // make it on a new line
        if (text_slot->text != "\n" && pages.last_line().cur_x > 0)
            if (pages.last_line().cur_x + size.width > paragraph_width)
            {
                pages.new_line();
                _process_pango_text(print_data, text_slot);
                return;
            }

        // move x before applying a new element
        if (pages.last_line().cur_x == 0)
            pages.last_line().cur_x = text_slot->indent;

        pages.last_line().set_height(size.height);
        if (tag_name.empty())
            pages.last_line().elements.push_back(std::make_shared<CtPageText>(pages.last_line().cur_x, layout_line));
        else
            pages.last_line().elements.push_back(std::make_shared<CtPageTag>(pages.last_line().cur_x, layout_line, tag_name, tag_attr));

        if (i < layout_count - 1) // the paragragh was wrapped, so it's  multiline
            pages.new_line();
        else
            pages.last_line().cur_x += size.width;
    }
}

void CtPrint::_process_pango_image(CtPrintData* print_data, CtImage* image, int indent, bool& any_image_resized)
{
    auto context = print_data->context;
    CtPrintPages& pages = print_data->pages;

    // calculate image
    auto pixbuf = image->get_pixbuf();
    double scale_w = (_page_width - pages.last_line().cur_x) / pixbuf->get_width();
    double scale_h = (_page_height - _layout_newline_height - CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT)/pixbuf->get_height();
    double scale = std::min(scale_w, scale_h);
    if (scale > 1.0) scale = 1.0;
    if (scale < 1.0) any_image_resized = true;

    double pixbuf_width = pixbuf->get_width() * scale;
    double pixbuf_height = pixbuf->get_height() * scale + CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT;

    // calculate label if it exists
    Cairo::Rectangle label_size{0,0,0,0};
    auto label_layout = context->create_pango_layout();
    label_layout->set_font_description(_plain_font);
    if (auto emb_file = dynamic_cast<CtImageEmbFile*>(image))
    {
        label_layout->set_markup("<b><small>"+str::xml_escape(emb_file->get_file_name().string())+"</small></b>");
        label_size = _get_width_height_from_layout_line(label_layout->get_line(0));
    }


    if (!pages.last_line().test_element_height(pixbuf_height + label_size.height, _page_height))
        pages.line_on_new_page();

    if (pages.last_line().cur_x == 0)
        pages.last_line().cur_x = indent;

    // insert label line
    if (label_size.height != 0)
    {
        int prev_x = pages.last_line().cur_x;
        int prev_y = pages.last_line().y;
        pages.last_line().set_height(label_size.height);
        pages.last_line().elements.push_back(std::make_shared<CtPageText>(pages.last_line().cur_x, label_layout->get_line(0)));
        pages.new_line();
        pages.last_line().cur_x = prev_x;
        pages.last_line().y = prev_y - CtConst::WHITE_SPACE_BETW_PIXB_AND_TEXT; // this removes additional 2px;
    }

    // insert image
    pages.last_line().set_height(pixbuf_height);
    pages.last_line().elements.push_back(std::make_shared<CtPageImage>(pages.last_line().cur_x, image, scale));

    // move to right
    pages.last_line().cur_x += std::max(pixbuf_width, label_size.width);
}

void CtPrint::_process_pango_codebox(CtPrintData* print_data, CtCodebox* codebox, int indent)
{
    auto context = print_data->context;
    CtPrintPages& pages = print_data->pages;

    codebox->apply_syntax_highlighting(false/*forceReApply*/);
    Glib::ustring original_content = CtExport2Pango().pango_get_from_code_buffer(codebox->get_buffer(), -1, -1);
    while (true)
    {
        // use content if it's ok
        auto codebox_layout = _codebox_get_layout(codebox, original_content, context);
        double codebox_height = _get_height_from_layout(codebox_layout);
        if (pages.last_line().test_element_height(codebox_height + BOX_OFFSET, _page_height))
        {
            if (pages.last_line().cur_x == 0)
                pages.last_line().cur_x = indent;

            pages.last_line().set_height(codebox_height + BOX_OFFSET);
            pages.last_line().elements.push_back(std::make_shared<CtPageCodebox>(pages.last_line().cur_x, codebox_layout));
            pages.last_line().cur_x += _get_width_from_layout(codebox_layout);
            return;
        }

        // if content is too long, split it
        Glib::ustring first_split, second_split;
        _codebox_split_content(codebox, original_content, _page_height - pages.last_line().y, context, first_split, second_split);
        if (first_split.empty()) // need a new page
        {
            pages.new_page();
        }
        else
        {
            auto first_split_layout = _codebox_get_layout(codebox, first_split, context);
            double codebox_height = _get_height_from_layout(first_split_layout);

            if (pages.last_line().cur_x == 0)
                pages.last_line().cur_x = indent;

            pages.last_line().set_height(codebox_height + BOX_OFFSET);
            pages.last_line().elements.push_back(std::make_shared<CtPageCodebox>(pages.last_line().cur_x, first_split_layout));
            pages.new_page();

            // go to to check the second part
            original_content = second_split;
        }
    }
}

void CtPrint::_process_pango_table(CtPrintData *print_data, CtTable *table, int indent)
{
    auto context = print_data->context;
    CtPrintPages& pages = print_data->pages;

    int first_row = 1;
    while (true)
    {
        // use table is length is ok
        std::vector<double> rows_h, cols_w;
        auto table_layouts = _table_get_layouts(table, first_row, -1, context);
        _table_get_grid(table_layouts, table->get_col_width(), rows_h, cols_w);
        double table_height = _table_get_width_height(rows_h);
        if (pages.last_line().test_element_height(table_height + BOX_OFFSET, _page_height))
        {
            if (pages.last_line().cur_x == 0)
                pages.last_line().cur_x = indent;
            pages.last_line().set_height(table_height + BOX_OFFSET);
            pages.last_line().elements.push_back(std::make_shared<CtPageTable>(pages.last_line().cur_x, table_layouts, table->get_col_width()));
            pages.last_line().cur_x += _table_get_width_height(cols_w);
            return;
        }

        // if table is too long, split it
        int split_row = _table_split_content(table, first_row, _page_height - pages.last_line().y - BOX_OFFSET, context);
        if (split_row == -1) // need a new page
        {
            pages.new_page();
        }
        else
        {
            auto split_layouts = _table_get_layouts(table, first_row, split_row, context);
            _table_get_grid(split_layouts, table->get_col_width(), rows_h, cols_w);
            double table_height = _table_get_width_height(rows_h);
            if (pages.last_line().cur_x == 0)
                pages.last_line().cur_x = indent;
            pages.last_line().set_height(table_height + BOX_OFFSET);
            pages.last_line().elements.push_back(std::make_shared<CtPageTable>(pages.last_line().cur_x, split_layouts, table->get_col_width()));
            pages.new_page();

            // go to to check the second part
            first_row = split_row + 1;
        }
    }
}

Glib::RefPtr<Pango::Layout> CtPrint::_codebox_get_layout(CtCodebox* codebox, Glib::ustring content, Glib::RefPtr<Gtk::PrintContext> context)
{
    auto layout = context->create_pango_layout();
    layout->set_font_description(_code_font);
    double codebox_width = codebox->get_width_in_pixels() ? codebox->get_frame_width() : _text_window_width * codebox->get_frame_width()/100.;
    if (codebox_width > _page_width)
        codebox_width = _page_width;
    layout->set_width(int(codebox_width * Pango::SCALE));
    layout->set_wrap(Pango::WRAP_WORD_CHAR);
    layout->set_markup(content);
    return layout;
}

// Split Long CodeBoxes
void CtPrint::_codebox_split_content(CtCodebox* codebox, Glib::ustring original_content, const int check_height, const Glib::RefPtr<Gtk::PrintContext>& context,
                                     Glib::ustring& first_split, Glib::ustring& second_split)
{
    std::vector<Glib::ustring> original_splitted_pango = str::split(original_content, CtConst::CHAR_NEWLINE);
    // fix for not-closed span, I suppose
    for (size_t i = 0; i < original_splitted_pango.size(); ++i)
    {
        auto& element = original_splitted_pango[i];
        Glib::ustring::size_type last_close = element.rfind("</span");
        Glib::ustring::size_type last_open = element.rfind("<span");
        if (last_close < element.size() and last_open < element.size() and last_close < last_open)
        {
            auto non_closed_span = element.substr(last_open);
            Glib::ustring::size_type end_non_closed_span_idx = non_closed_span.find(">");
            if (end_non_closed_span_idx < non_closed_span.size()) {
                non_closed_span = non_closed_span.substr(0, end_non_closed_span_idx+1);
                original_splitted_pango[i] += "</span>";
                original_splitted_pango[i+1] = non_closed_span + original_splitted_pango[i+1];
            }
        }
    }

    std::vector<Glib::ustring> splitted_pango;
    while (splitted_pango.size() < original_splitted_pango.size())
    {
        splitted_pango.push_back(original_splitted_pango[splitted_pango.size()]);
        Glib::ustring new_content = str::join(splitted_pango, CtConst::CHAR_NEWLINE);
        auto codebox_layout = _codebox_get_layout(codebox, new_content, context);
        double codebox_height = _get_height_from_layout(codebox_layout);
        if (codebox_height + BOX_OFFSET > check_height)
        {
            if (splitted_pango.size() == 1)
            {
                // check_height is not enough, so we need a new page
                first_split = "";
                second_split = original_content;
                return;
            }

            splitted_pango.erase(splitted_pango.end());
            original_splitted_pango.erase(original_splitted_pango.begin(), original_splitted_pango.begin() + splitted_pango.size());
            first_split = str::join(splitted_pango, CtConst::CHAR_NEWLINE);
            second_split = str::join(original_splitted_pango, CtConst::CHAR_NEWLINE);
            return;
        }
    }
}

CtPageTable::TableLayouts CtPrint::_table_get_layouts(CtTable* table, const int first_row, const int last_row, const Glib::RefPtr<Gtk::PrintContext>& context)
{
    CtPageTable::TableLayouts table_layouts;
    for (size_t row = 0; row < table->get_table_matrix().size(); ++row)
    {
        if (first_row != -1 && row > 0 && (int)row < first_row) continue; // skip row out of range except header
        if (last_row != -1 && (int)row > last_row) break;

        std::vector<Glib::RefPtr<Pango::Layout>> layouts;
        for (size_t col = 0; col < table->get_table_matrix()[0].size(); ++col)
        {
            Glib::ustring text = str::xml_escape(table->get_table_matrix()[row][col]->get_text_content());
            if (row == 0) text = "<b>" + text + "</b>";
            auto cell_layout = context->create_pango_layout();
            cell_layout->set_font_description(_rich_font);
            cell_layout->set_width(int(table->get_col_width() * Pango::SCALE));
            cell_layout->set_wrap(Pango::WRAP_WORD_CHAR);
            cell_layout->set_markup(text);
            layouts.push_back(cell_layout);
        }
        table_layouts.push_back(std::move(layouts));
    }

    return table_layouts;
}

void CtPrint::_table_get_grid(const CtPageTable::TableLayouts& table_layouts, const int col_min, std::vector<double>& rows_h, std::vector<double>& cols_w)
{
    rows_h = std::vector<double>(table_layouts.size(), 0);
    cols_w = std::vector<double>(table_layouts[0].size(), col_min);
    for (size_t row = 0; row < table_layouts.size(); ++row)
    {
        for (size_t col = 0; col < table_layouts[0].size(); ++col)
        {
            auto cell_layout = table_layouts[row][col];
            double cell_height = 0;
            for (int layout_line_idx = 0; layout_line_idx < cell_layout->get_line_count(); ++ layout_line_idx)
            {
                auto line_size = _get_width_height_from_layout_line(cell_layout->get_line(layout_line_idx));
                cell_height += line_size.height;
                cols_w[col] = std::max(cols_w[col], line_size.width);
            }
            rows_h[row] = std::max(rows_h[row], cell_height);
        }
    }
}

double CtPrint::_table_get_width_height(std::vector<double>& data)
{
    double acc = 0;
    for (auto& value: data)
        acc += value + _table_line_thickness;
    return acc;
}

int CtPrint::_table_split_content(CtTable* table, const int start_row, const int check_height, const Glib::RefPtr<Gtk::PrintContext>& context)
{
    int last_row = start_row;
    for (; last_row < (int)table->get_table_matrix().size(); ++last_row)
    {
        std::vector<double> rows_h, cols_w;
        auto table_layouts = _table_get_layouts(table, start_row, last_row, context);
        _table_get_grid(table_layouts, table->get_col_width(), rows_h, cols_w);
        double table_height = _table_get_width_height(rows_h);
        if (table_height > check_height)
        {
            if (start_row == last_row) // not enouth place for 1 row + a header, add a new page
                return -1;
            return last_row -1;
        }
    }
    return -1; // though we shouldn't be here
}

void CtPrint::_draw_codebox_box(Cairo::RefPtr<Cairo::Context> cairo_context, double x0, double y0, double codebox_width, double codebox_height)
{
    cairo_context->set_source_rgba(0, 0, 0, 0.3);
    cairo_context->rectangle(x0, y0, codebox_width, codebox_height);
    cairo_context->stroke();
}

void CtPrint::_draw_codebox_code(Cairo::RefPtr<Cairo::Context> cairo_context, Glib::RefPtr<Pango::Layout> codebox_layout, double x0, double y0)
{
    double y = y0;
    cairo_context->set_source_rgb(0, 0, 0);
    for (int layout_line_idx = 0; layout_line_idx < codebox_layout->get_line_count(); ++layout_line_idx)
    {
        auto layout_line = codebox_layout->get_line(layout_line_idx);
        double line_height = _get_width_height_from_layout_line(layout_line).height;
        cairo_context->move_to(x0 + CtConst::GRID_SLIP_OFFSET, y + line_height);
        y += line_height;
        layout_line->show_in_cairo_context(cairo_context);
    }
}

void CtPrint::_draw_table_grid(Cairo::RefPtr<Cairo::Context> cairo_context, const std::vector<double>& rows_h, const std::vector<double>& cols_w,
                               double x0, double y0, double table_width, double table_height)
{
    double x = x0;
    double y = y0;
    cairo_context->set_source_rgba(0, 0, 0, 0.3);
    // draw lines
    cairo_context->move_to(x, y);
    cairo_context->line_to(x + table_width, y);
    for (auto& row_h: rows_h)
    {
        y += row_h + _table_line_thickness;
        cairo_context->move_to(x, y);
        cairo_context->line_to(x + table_width, y);
    }
    // draw columns
    y = y0;
    cairo_context->move_to(x, y);
    cairo_context->line_to(x, y + table_height);
    for (auto& col_w: cols_w)
    {
        x += col_w + _table_line_thickness;
        cairo_context->move_to(x, y);
        cairo_context->line_to(x, y + table_height);
    }
    cairo_context->stroke();
}

void CtPrint::_draw_table_text(Cairo::RefPtr<Cairo::Context> cairo_context, const std::vector<double>& rows_h, const std::vector<double>& cols_w,
                               const CtPageTable::TableLayouts& table_layouts, double x0, double y0)
{
    cairo_context->set_source_rgb(0, 0, 0);
    double y = y0;
    for (size_t i = 0; i < rows_h.size(); ++i)
    {
        double row_h = rows_h[i];
        double x = x0 + CtConst::GRID_SLIP_OFFSET;
        for (size_t j = 0; j < cols_w.size(); ++j)
        {
            double col_w = cols_w[j];
            auto layout_cell = table_layouts[i][j];
            double local_y = y;
            for (int layout_line_id = 0; layout_line_id < layout_cell->get_line_count(); ++layout_line_id)
            {
                auto layout_line = layout_cell->get_line(layout_line_id);
                double line_height = _get_width_height_from_layout_line(layout_line).height;
                cairo_context->move_to(x, local_y + line_height);
                local_y += line_height;
                layout_line->show_in_cairo_context(cairo_context);
            }
            x += col_w + _table_line_thickness;
        }
        y += row_h + _table_line_thickness;
    }
}

Cairo::Rectangle CtPrint::_get_width_height_from_layout_line(Glib::RefPtr<const Pango::LayoutLine> line)
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
        double line_height = _get_width_height_from_layout_line(layout_line).height;
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
        double line_width = _get_width_height_from_layout_line(layout_line).width;
        if (line_width > width)
            width = line_width;
    }
    return width + 2 * CtConst::GRID_SLIP_OFFSET;
}
