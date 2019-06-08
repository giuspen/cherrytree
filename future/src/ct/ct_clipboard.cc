/*
 * ct_clipboard.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_clipboard.h"

#include "ct_codebox.h"
#include "ct_doc_rw.h"


// Given text_buffer and selection, returns the rich text xml
Glib::ustring CtClipboard::rich_text_get_from_text_buffer_selection(CtTreeIter node_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end,
                                                 gchar change_case /*="n"*/, bool exclude_iter_sel_end /*=false*/)
{
    int iter_sel_start_offset = iter_sel_start.get_offset();
    int iter_sel_end_offset = iter_sel_end.get_offset();
    if (exclude_iter_sel_end)
        iter_sel_end_offset -= 1;
    std::list<CtAnchoredWidget*> widget_vector = node_iter.get_embedded_pixbufs_tables_codeboxes(CtForPrint::No, std::make_pair(iter_sel_start_offset, iter_sel_end_offset));

    xmlpp::Document doc;
    auto root = doc.create_root_node("root");
    int start_offset = iter_sel_start_offset;
    for (CtAnchoredWidget* widget: widget_vector)
    {
        int end_offset = widget->getOffset();
        _rich_text_process_slot(root, start_offset, end_offset, text_buffer, widget, change_case);
        start_offset = end_offset;
    }
    _rich_text_process_slot(root, start_offset, iter_sel_end.get_offset(), text_buffer, nullptr, change_case);
    return doc.write_to_string();
}

// Process a Single Pango Slot
void CtClipboard::_rich_text_process_slot(xmlpp::Element* root, int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                          CtAnchoredWidget* obj_element, gchar change_case /*="n"*/)
{
    xmlpp::Element* dom_iter = root->add_child("slot");
    // #print "process slot (%s->%s)" % (start_offset, end_offset)
    // begin operations
    std::map<const gchar*, std::string> curr_attributes;
    for (auto tag_property: CtConst::TAG_PROPERTIES)
        curr_attributes[tag_property] = "";
    Gtk::TextIter start_iter = text_buffer->get_iter_at_offset(start_offset);
    Gtk::TextIter curr_iter = start_iter;
    CtTextIterUtil::rich_text_attributes_update(curr_iter, curr_attributes);

    bool tag_found = curr_iter.forward_to_tag_toggle(Glib::RefPtr<Gtk::TextTag>{nullptr});
    bool one_more_serialize = true;
    while (tag_found)
    {
        if (curr_iter.get_offset() > end_offset)
            curr_iter = text_buffer->get_iter_at_offset(end_offset);
        CtXmlWrite::rich_txt_serialize(dom_iter, start_iter, curr_iter, curr_attributes, change_case);

        int offset_old = curr_iter.get_offset();
        if (offset_old >= end_offset)
        {
            one_more_serialize = false;
            break;
        }
        else
        {
            CtTextIterUtil::rich_text_attributes_update(curr_iter, curr_attributes);
            start_iter.set_offset(offset_old);
            tag_found = curr_iter.forward_to_tag_toggle(Glib::RefPtr<Gtk::TextTag>{nullptr});
            if (curr_iter.get_offset() == offset_old)
            {
                one_more_serialize = false;
                break;
            }
        }
    }
    if (one_more_serialize)
    {
        if (curr_iter.get_offset() > end_offset)
            curr_iter = text_buffer->get_iter_at_offset(end_offset);
        CtXmlWrite::rich_txt_serialize(dom_iter, start_iter, curr_iter, curr_attributes, change_case);
    }

    if (obj_element != nullptr)
    {
        // todo:
        // if obj_element[0] == "pixbuf": self.dad.xml_handler.pixbuf_element_to_xml(obj_element[1], dom_iter, dom)
        // elif obj_element[0] == "table": self.dad.xml_handler.table_element_to_xml(obj_element[1], dom_iter, dom)
        // elif obj_element[0] == "codebox": self.dad.xml_handler.codebox_element_to_xml(obj_element[1], dom_iter, dom)
    }
}

// From XML String to Text Buffer
void CtClipboard::from_xml_string_to_buffer(Glib::RefPtr<Gtk::TextBuffer> text_buffer, const Glib::ustring& xml_string)
{
    xmlpp::DomParser parser;
    parser.parse_memory(xml_string);
    xmlpp::Document* doc = parser.get_document();
    if (doc->get_root_node()->get_name() != "root")
    {
        throw std::invalid_argument("rich text from clipboard error");
    }
    //todo: self.dad.state_machine.not_undoable_timeslot_set(True)
    std::list<CtAnchoredWidget*> widgets;
    for (xmlpp::Node* slot_node: doc->get_root_node()->get_children())
    {
        if (slot_node->get_name() != "slot")
            continue;
        for (xmlpp::Node* child_node: slot_node->get_children())
        {
            Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(text_buffer);
            Gtk::TextIter insert_iter = text_buffer->get_insert()->get_iter();
            CtXmlRead::getTextBufferIter(gsv_buffer, &insert_iter, widgets, child_node);
        }
    }
    // todo: self.dad.state_machine.not_undoable_timeslot_set(False)
}
