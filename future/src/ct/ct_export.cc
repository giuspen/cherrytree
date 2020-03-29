/*
 * ct_export.cc
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

#include "ct_export.h"
#include "ct_dialogs.h"


void CtExportPrint::node_export_print(const Glib::ustring& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options, int sel_start, int sel_end)
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

void CtExportPrint::node_and_subnodes_export_print(const Glib::ustring& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
{
    std::vector<Glib::ustring> tree_pango_slots;
    std::list<CtAnchoredWidget*> tree_widgets;
    Glib::ustring text_font = _pCtMainWin->get_ct_config()->codeFont;
    _nodes_all_export_print_iter(tree_iter, options, tree_pango_slots, tree_widgets, text_font);

    _pCtMainWin->get_ct_print().print_text(_pCtMainWin, pdf_filepath, tree_pango_slots, text_font, _pCtMainWin->get_ct_config()->codeFont,
                                           tree_widgets, _pCtMainWin->get_text_view().get_allocation().get_width());
}

void CtExportPrint::tree_export_print(const Glib::ustring& pdf_filepath, CtTreeIter tree_iter, const CtExportOptions& options)
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

void CtExportPrint::_nodes_all_export_print_iter(CtTreeIter tree_iter, const CtExportOptions& options,
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
            node_pango_slots[0] = CtConst::CHAR_NEWPAGE + CtConst::CHAR_NEWPAGE + node_pango_slots[0];
            vec::vector_extend(tree_pango_slots, node_pango_slots);
            node_widgets.insert(node_widgets.begin(), nullptr);
        }
        else
        {
            tree_pango_slots[tree_pango_slots.size() - 1] += CtConst::CHAR_NEWLINE + CtConst::CHAR_NEWLINE + CtConst::CHAR_NEWLINE + node_pango_slots[0];
            if (node_pango_slots.size() > 1)
            {
                node_pango_slots.erase(node_pango_slots.begin());
                vec::vector_extend(tree_pango_slots, node_pango_slots);
            }
        }
    }
    tree_widgets.insert(std::end(tree_widgets), std::begin(node_widgets), std::end(node_widgets));
    for (auto iter: tree_iter->children())
        _nodes_all_export_print_iter(_pCtMainWin->curr_tree_store().to_ct_tree_iter(iter), options, tree_pango_slots, tree_widgets, text_font);
}

// Add Node Name to Pango Text Vector
void CtExportPrint::_add_node_name(Glib::ustring node_name, std::vector<Glib::ustring>& pango_slots)
{
    pango_slots[0] = "<b><i><span size=\"xx-large\">" + str::xml_escape(node_name)
            + "</span></i></b>" + CtConst::CHAR_NEWLINE + CtConst::CHAR_NEWLINE + pango_slots[0];
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
    while (1)
    {
        auto curr_tags = curr_iter.get_tags();
        if (curr_tags.size() > 0)
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
    if (pango_text.size() == 0 || pango_text[pango_text.size()-1] != CtConst::CHAR_NEWLINE[0])
        pango_text += CtConst::CHAR_NEWLINE;
    return pango_text;
}

// Given a treestore iter returns the Pango rich text
void CtExport2Pango::pango_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end, std::vector<Glib::ustring>& out_slots,
                                                   bool exclude_anchors, std::list<CtAnchoredWidget*>& out_widgets)
{
    auto curr_buffer = node_iter.get_node_text_buffer();
    out_widgets = node_iter.get_embedded_pixbufs_tables_codeboxes(std::make_pair(sel_start, sel_end));
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
                                         [&](Gtk::TextIter& start_iter, Gtk::TextIter& curr_iter, std::map<const gchar*, std::string>& curr_attributes) {
        curr_pango_text += _pango_text_serialize(start_iter, curr_iter, curr_attributes);
    });
    return curr_pango_text;
}

// Adds a slice to the Pango Text
Glib::ustring CtExport2Pango::_pango_text_serialize(Gtk::TextIter start_iter, Gtk::TextIter end_iter, const std::map<const gchar*, std::string>& curr_attributes)
{
    Glib::ustring pango_attrs;
    bool superscript_active = false;
    bool subscript_active = false;
    bool monospace_active = false;
    for (auto tag_property: CtConst::TAG_PROPERTIES)
    {
        if ((tag_property != CtConst::TAG_JUSTIFICATION && tag_property != CtConst::TAG_LINK) && curr_attributes.at(tag_property) != "")
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
            pango_attrs += std::string(" ") + tag_property + "=\"" + property_value + "\"";
        }
    }
    Glib::ustring tagged_text;
    if (pango_attrs == "")
        tagged_text = str::xml_escape(start_iter.get_text(end_iter));
    else
        tagged_text = "<span" + pango_attrs + ">" + str::xml_escape(start_iter.get_text(end_iter)) + "</span>";
    if (superscript_active) tagged_text = "<sup>" + tagged_text + "</sup>";
    if (subscript_active) tagged_text = "<sub>" + tagged_text + "</sub>";
    if (monospace_active) tagged_text = "<tt>" + tagged_text + "</tt>";
    return tagged_text;
}
