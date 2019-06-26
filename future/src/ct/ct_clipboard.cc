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
#include "ct_export2html.h"
#include "ct_export2txt.h"
#include "src/fmt/ostream.h"


const Glib::ustring TARGET_CTD_PLAIN_TEXT = "UTF8_STRING";
const Glib::ustring TARGET_CTD_RICH_TEXT = "CTD_RICH";
const Glib::ustring TARGET_CTD_TABLE = "CTD_TABLE";
const Glib::ustring TARGET_CTD_CODEBOX = "CTD_CODEBOX";
const std::vector<Glib::ustring> TARGETS_HTML = {"text/html", "HTML Format"};
const Glib::ustring TARGET_URI_LIST = "text/uri-list";
const std::vector<Glib::ustring> TARGETS_PLAIN_TEXT = {"UTF8_STRING", "COMPOUND_TEXT", "STRING", "TEXT"};
const std::vector<Glib::ustring> TARGETS_IMAGES = {"image/png", "image/jpeg", "image/bmp", "image/tiff", "image/x-MS-bmp", "image/x-bmp"};
const Glib::ustring TARGET_WINDOWS_FILE_NAME = "FileName";


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
    CtTextIterUtil::generic_process_slot(start_offset, end_offset, text_buffer,
                                         [&](Gtk::TextIter& start_iter, Gtk::TextIter& curr_iter, std::map<const gchar*, std::string>& curr_attributes) {
        CtXmlWrite::rich_txt_serialize(dom_iter, start_iter, curr_iter, curr_attributes, change_case);
    });

    if (obj_element != nullptr)
    {
        obj_element->to_xml(root, 0);
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
    Glib::ustring node_syntax_high = CtApp::P_ctActions->getCtMainWin()->curr_tree_iter().get_node_syntax_highlighting();
    CtImage* pixbuf_target = nullptr;
    if (!pCodebox && node_syntax_high == CtConst::RICH_TEXT_ID && num_chars == 1)
    {
        std::list<CtAnchoredWidget*> widget_vector = CtApp::P_ctActions->getCtMainWin()->curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(CtForPrint::No, std::make_pair(iter_sel_start.get_offset(), iter_sel_end.get_offset()));
        if (widget_vector.size() > 0)
        {
            if (CtImage* image = dynamic_cast<CtImage*>(widget_vector.front()))
            {
                pixbuf_target = image;
            }
            else if (CtTable* table = dynamic_cast<CtTable*>(widget_vector.front()))
            {
                CtClipboardData* clip_data = new CtClipboardData();
                table->to_xml(clip_data->xml_doc.create_root_node("root"), 0);
                clip_data->html_text = CtExport2Html().table_export_to_html(table);
                clip_data->plain_text = CtExport2Txt().get_table_plain(table);

                _set_clipboard_data({TARGET_CTD_TABLE, TARGETS_HTML[0], TARGET_CTD_PLAIN_TEXT}, clip_data);
                return;
            }
            else if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widget_vector.front()))
            {
                CtClipboardData* clip_data = new CtClipboardData();
                codebox->to_xml(clip_data->xml_doc.create_root_node("root"), 0);
                clip_data->html_text = CtExport2Html().codebox_export_to_html(codebox);
                clip_data->plain_text = CtExport2Txt().get_codebox_plain(codebox);

                _set_clipboard_data({TARGET_CTD_CODEBOX, TARGETS_HTML[0], TARGET_CTD_PLAIN_TEXT}, clip_data);
                return;
            }
        }
    }

    CtClipboardData* clip_data = new CtClipboardData();
    clip_data->html_text = CtExport2Html().selection_export_to_html(text_buffer, iter_sel_start, iter_sel_end, !pCodebox ? node_syntax_high : CtConst::PLAIN_TEXT_ID);
    if (!pCodebox && node_syntax_high == CtConst::RICH_TEXT_ID)
    {
        std::vector<std::string> targets_vector;
        std::pair<int,int> text_offsets_range = std::make_pair(iter_sel_start.get_offset(), iter_sel_end.get_offset());
        clip_data->plain_text = CtExport2Txt().node_export_to_txt(text_buffer, text_offsets_range, true);
        clip_data->rich_text = rich_text_get_from_text_buffer_selection(CtApp::P_ctActions->getCtMainWin()->curr_tree_iter(), text_buffer, iter_sel_start, iter_sel_end);
        if (!_force_plain_text)
        {
            targets_vector = {TARGET_CTD_PLAIN_TEXT, TARGET_CTD_RICH_TEXT, TARGETS_HTML[0], TARGETS_HTML[1]};
            if (pixbuf_target)
            {
                clip_data->pix_buf = pixbuf_target->getPixBuf();
                targets_vector.push_back(TARGETS_IMAGES[0]);
            }
        }
        else
            targets_vector = {TARGET_CTD_PLAIN_TEXT};

        _set_clipboard_data(targets_vector, clip_data);
    }
    else
    {
        clip_data->plain_text = text_buffer->get_text(iter_sel_start, iter_sel_end);
        std::vector<std::string> targets_vector;
        if (!_force_plain_text)
            targets_vector = {TARGET_CTD_PLAIN_TEXT, TARGETS_HTML[0], TARGETS_HTML[1]};
        else
            targets_vector = {TARGET_CTD_PLAIN_TEXT};
        _set_clipboard_data(targets_vector, clip_data);
    }
}

void CtClipboard::_set_clipboard_data(const std::vector<std::string>& targets_list, CtClipboardData* clip_data)
{
    std::vector<Gtk::TargetEntry> target_entries;
    for (auto& target: targets_list)
        target_entries.push_back(Gtk::TargetEntry(target));
    sigc::slot<void, Gtk::SelectionData&, guint, CtClipboardData*> clip_data_get = sigc::mem_fun(*this, &CtClipboard::_clip_data_get_signal);
    sigc::slot<void, CtClipboardData*> clip_data_clear = sigc::mem_fun(*this, &CtClipboard::_clip_data_clear_signal);
    Gtk::Clipboard::get()->set(target_entries, sigc::bind(clip_data_get, clip_data), sigc::bind(clip_data_clear, clip_data));
}

// based on def get_func(self, clipboard, selectiondata, info, data)
void  CtClipboard::_clip_data_get_signal(Gtk::SelectionData& selection_data, guint info, CtClipboardData* clip_data)
{
    Glib::ustring target = selection_data.get_target();
    if (target == TARGET_CTD_PLAIN_TEXT)
        selection_data.set(target, 8, (guint8*)clip_data->plain_text.c_str(), clip_data->plain_text.bytes());
    else if (target == TARGET_CTD_RICH_TEXT)
        selection_data.set("UTF8_STRING", 8, (guint8*)clip_data->rich_text.c_str(), clip_data->rich_text.bytes());
    else if (vec::exists(TARGETS_HTML, target))
    {
        if (!CtConst::IS_WIN_OS)
            selection_data.set(target, 8, (guint8*)clip_data->html_text.c_str(), clip_data->html_text.bytes());
        else
            if (target == TARGETS_HTML[0])
            {
                glong utf16text_len = 0;
                gunichar2* utf16text = g_utf8_to_utf16(clip_data->html_text.c_str(), clip_data->html_text.bytes(), 0, &utf16text_len, 0);
                if (utf16text && utf16text_len > 0)
                    selection_data.set(target, 8, (guint8*)utf16text, utf16text_len);
                g_free(utf16text);
            }
            else
            {
                Glib::ustring html = Win32HtmlFormat().encode(clip_data->html_text);
                selection_data.set(target, 8, (guint8*)html.c_str(), html.bytes());
            }
    }
    else if (target == TARGET_CTD_CODEBOX)
    {
        Glib::ustring xml = clip_data->xml_doc.write_to_string();
        selection_data.set("UTF8_STRING", 8, (guint8*)xml.c_str(), xml.bytes());
    }
    else if (target == TARGET_CTD_TABLE)
    {
        Glib::ustring xml = clip_data->xml_doc.write_to_string();
        selection_data.set("UTF8_STRING", 8, (guint8*)xml.c_str(), xml.bytes());
    }
    else if (target == TARGETS_IMAGES[0])
        selection_data.set_pixbuf(clip_data->pix_buf);
}

void CtClipboard::_clip_data_clear_signal(CtClipboardData* clip_data)
{
    delete clip_data;
}


Glib::ustring Win32HtmlFormat::encode(Glib::ustring html_in)
{
    std::string MARKER_BLOCK_OUTPUT = \
            "Version:1.0\r\n" \
            "StartHTML:{:09d}\r\n" \
            "EndHTML:{:09d}\r\n" \
            "StartFragment:{:09d}\r\n" \
            "EndFragment:{:09d}\r\n" \
            "StartSelection:{:09d}\r\n" \
            "EndSelection{:09d}\r\n" \
            "SourceURL:{}\r\n";

    std::string DEFAULT_HTML_BODY = \
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">" \
            "<HTML><HEAD>{}</HEAD><BODY><!--StartFragment-->{}<!--EndFragment--></BODY></HTML>";

    Glib::ustring head = "", source = CtConst::APP_NAME + Glib::ustring(CtConst::CT_VERSION);
    Glib::ustring html = fmt::format(DEFAULT_HTML_BODY, head, html_in);
    Glib::ustring::size_type fragmentStart = html.find(html_in);
    Glib::ustring::size_type fragmentEnd = fragmentStart + html_in.size();

    // How long is the prefix going to be?
    Glib::ustring dummyPrefix = fmt::format(MARKER_BLOCK_OUTPUT, 0, 0, 0, 0, 0, 0, source);
    Glib::ustring::size_type lenPrefix = dummyPrefix.size();

    Glib::ustring prefix = fmt::format(MARKER_BLOCK_OUTPUT,
                lenPrefix, html.size() + lenPrefix,
                fragmentStart + lenPrefix, fragmentEnd + lenPrefix,
                fragmentStart + lenPrefix, fragmentEnd + lenPrefix,
                source);
    return prefix + html;
}
