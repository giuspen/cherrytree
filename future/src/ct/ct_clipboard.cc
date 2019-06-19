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
#include <ct_image.h>


CtClipboard::CtClipboard()
{
    _force_plain_text = false;
}

/*static*/ void CtClipboard::on_cut_clipboard(GtkTextView* pTextView,  gpointer codebox)
{
    CtClipboard().cut_clipboard(Glib::wrap(pTextView), static_cast<CtCodebox*>(codebox));
}

/*static*/ void CtClipboard::on_copy_clipboard(GtkTextView* pTextView, gpointer codebox)
{

}

/*static*/ void CtClipboard::on_paste_clipboard(GtkTextView* pTextView, gpointer codebox)
{

}

// Cut to Clipboard"
void CtClipboard::cut_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox)
{
    auto text_buffer = pTextView->get_buffer();
    if (text_buffer->get_has_selection())
    {
        Gtk::TextIter iter_sel_start, iter_sel_end;
        text_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
        int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
        if ((pCodebox || CtApp::P_ctActions->getCtMainWin()->curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID) && num_chars > 30000)
        {
            std::cout << "cut-clipboard is not overridden for num_chars " << num_chars << std::endl;
        }
        else
        {
            g_signal_stop_emission_by_name(G_OBJECT(pTextView->gobj()), "cut-clipboard");
            _selection_to_clipboard(text_buffer, pTextView, iter_sel_start, iter_sel_end, num_chars, pCodebox);
            if (CtApp::P_ctActions->_is_curr_node_not_read_only_or_error())
            {
                text_buffer->erase_selection(true, pTextView->get_editable());
                pTextView->grab_focus();
            }
        }
    }
    _force_plain_text = false;
}

void copy_clipboard(Gtk::TextView* pTextView)
{
    // todo:
}

void paste_clipboard(Gtk::TextView* pTextView)
{
    // todo:
}

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

// Write the Selected Content to the Clipboard
void CtClipboard::_selection_to_clipboard(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextView* sourceview, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end, int num_chars, CtCodebox* pCodebox)
{
     CtImage* pixbuf_target = nullptr;
     if (!pCodebox && CtApp::P_ctActions->getCtMainWin()->curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID && num_chars == 1)
     {
         std::list<CtAnchoredWidget*> widget_vector = CtApp::P_ctActions->getCtMainWin()->curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(CtForPrint::No, std::make_pair(iter_sel_start.get_offset(), iter_sel_end.get_offset()));
         if (widget_vector.size() > 0)
         {
             if (CtImage* ctImage = dynamic_cast<CtImage*>(widget_vector.front()))
             {
                 pixbuf_target = ctImage;
             }
             else if (CtTable* ctTable = dynamic_cast<CtTable*>(widget_vector.front()))
             {
                /*
                 table_dict = self.dad.state_machine.table_to_dict(anchor)
                 html_text = self.dad.html_handler.table_export_to_html(table_dict)
                 txt_handler = exports.Export2Txt(self.dad)
                 text_offsets_range = [iter_sel_start.get_offset(), iter_sel_end.get_offset()]
                 plain_text = txt_handler.node_export_to_txt(text_buffer, "", sel_range=text_offsets_range, check_link_target=True)
                 self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_TABLE, TARGETS_HTML[0], TARGET_CTD_PLAIN_TEXT)],
                                              self.get_func,
                                              self.clear_func,
                                              (plain_text, None, html_text, table_dict))
                 */
                 return;
             }
             else if (CtCodebox* ctCodebox = dynamic_cast<CtCodebox*>(widget_vector.front()))
             {
                 /*
                 codebox_dict = self.dad.state_machine.codebox_to_dict(anchor, for_print=0)
                 codebox_dict_html = self.dad.state_machine.codebox_to_dict(anchor, for_print=2)
                 html_text = self.dad.html_handler.codebox_export_to_html(codebox_dict_html)
                 txt_handler = exports.Export2Txt(self.dad)
                 text_offsets_range = [iter_sel_start.get_offset(), iter_sel_end.get_offset()]
                 plain_text = txt_handler.node_export_to_txt(text_buffer, "", sel_range=text_offsets_range, check_link_target=True)
                 self.clipboard.set_with_data([(t, 0, 0) for t in (TARGET_CTD_CODEBOX, TARGETS_HTML[0], TARGET_CTD_PLAIN_TEXT)],
                                              self.get_func,
                                              self.clear_func,
                                              (plain_text, None, html_text, codebox_dict))
                                              */
                 return;
             }
         }
     }
     /*
     html_text = self.dad.html_handler.selection_export_to_html(text_buffer, iter_sel_start, iter_sel_end,
         self.dad.syntax_highlighting if not from_codebox else cons.PLAIN_TEXT_ID)
     if not from_codebox and self.dad.syntax_highlighting == cons.RICH_TEXT_ID:
         txt_handler = exports.Export2Txt(self.dad)
         text_offsets_range = [iter_sel_start.get_offset(), iter_sel_end.get_offset()]
         plain_text = txt_handler.node_export_to_txt(text_buffer, "", sel_range=text_offsets_range, check_link_target=True)
         rich_text = self.rich_text_get_from_text_buffer_selection(text_buffer, iter_sel_start, iter_sel_end)
         if not self.force_plain_text:
             targets_vector = [TARGET_CTD_PLAIN_TEXT, TARGET_CTD_RICH_TEXT, TARGETS_HTML[0], TARGETS_HTML[1]]
             if pixbuf_target:
                 targets_vector.append(TARGETS_IMAGES[0])
         else:
             targets_vector = [TARGET_CTD_PLAIN_TEXT]
         self.clipboard.set_with_data([(t, 0, 0) for t in targets_vector],
             self.get_func,
             self.clear_func,
             (plain_text, rich_text, html_text, pixbuf_target))
     else:
         plain_text = text_buffer.get_text(iter_sel_start, iter_sel_end)
         if not self.force_plain_text:
             targets_vector = [TARGET_CTD_PLAIN_TEXT, TARGETS_HTML[0], TARGETS_HTML[1]]
         else:
             targets_vector = [TARGET_CTD_PLAIN_TEXT]
         self.clipboard.set_with_data([(t, 0, 0) for t in targets_vector],
                                      self.get_func,
                                      self.clear_func,
                                      (plain_text, None, html_text, pixbuf_target))

                                      */
}
