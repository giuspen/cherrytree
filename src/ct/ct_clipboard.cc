/*
 * ct_clipboard.cc
 *
 * Copyright 2009-2022
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

#include "ct_clipboard.h"
#include "ct_codebox.h"
#include "ct_main_win.h"
#include "ct_image.h"
#include "ct_export2html.h"
#include "ct_export2txt.h"
#include "ct_imports.h"
#include "ct_misc_utils.h"
#include "ct_actions.h"
#include "ct_storage_xml.h"
#include <gio/gio.h> // to get mime type
#include <glibmm/regex.h>
#include "ct_logging.h"
#include "ct_parser.h"

bool CtClipboard::_static_force_plain_text{false};

CtClipboard::CtClipboard(CtMainWin* pCtMainWin)
 : _pCtMainWin(pCtMainWin)
{
}

/*static*/ void CtClipboard::on_cut_clipboard(GtkTextView* pTextView,  gpointer pCtPairCodeboxMainWin)
{
    CtPairCodeboxMainWin& ctPairCodeboxMainWin = *static_cast<CtPairCodeboxMainWin*>(pCtPairCodeboxMainWin);
    auto clipb = CtClipboard(ctPairCodeboxMainWin.second);
    clipb._cut_clipboard(Glib::wrap(pTextView), ctPairCodeboxMainWin.first);
}

/*static*/ void CtClipboard::on_copy_clipboard(GtkTextView* pTextView, gpointer pCtPairCodeboxMainWin)
{
    CtPairCodeboxMainWin& ctPairCodeboxMainWin = *static_cast<CtPairCodeboxMainWin*>(pCtPairCodeboxMainWin);
    auto clipb = CtClipboard(ctPairCodeboxMainWin.second);
    clipb._copy_clipboard(Glib::wrap(pTextView), ctPairCodeboxMainWin.first);
}

/*static*/ void CtClipboard::on_paste_clipboard(GtkTextView* pTextView, gpointer pCtPairCodeboxMainWin)
{
    CtPairCodeboxMainWin& ctPairCodeboxMainWin = *static_cast<CtPairCodeboxMainWin*>(pCtPairCodeboxMainWin);
    auto clipb = CtClipboard(ctPairCodeboxMainWin.second);
    clipb._paste_clipboard(Glib::wrap(pTextView), ctPairCodeboxMainWin.first);
}

// Cut to Clipboard
void CtClipboard::_cut_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox)
{
    auto on_scope_exit = scope_guard([&](void*) { CtClipboard::_static_force_plain_text = false; });
    auto text_buffer = pTextView->get_buffer();
    if (text_buffer->get_has_selection())
    {
        Gtk::TextIter iter_sel_start, iter_sel_end;
        text_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
        int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
        if ((pCodebox or _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID) and num_chars > 30000)
        {
            spdlog::error("cut-clipboard is not overridden for num_chars {}", num_chars);
        }
        else
        {
            g_signal_stop_emission_by_name(G_OBJECT(pTextView->gobj()), "cut-clipboard");
            _selection_to_clipboard(text_buffer, pTextView, iter_sel_start, iter_sel_end, num_chars, pCodebox);
            if (_pCtMainWin->get_ct_actions()->_is_curr_node_not_read_only_or_error())
            {
                text_buffer->erase_selection(true, pTextView->get_editable());
                pTextView->grab_focus();
            }
        }
    }

}

// Copy to Clipboard
void CtClipboard::_copy_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox)
{
    auto on_scope_exit = scope_guard([&](void*) { CtClipboard::_static_force_plain_text = false; });
    auto text_buffer = pTextView->get_buffer();
    if (text_buffer->get_has_selection())
    {
        Gtk::TextIter iter_sel_start, iter_sel_end;
        text_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
        int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
        if ((pCodebox or _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID) and num_chars > 30000)
        {
            spdlog::error("copy-clipboard is not overridden for num_chars {}", num_chars);
        }
        else
        {
            g_signal_stop_emission_by_name(G_OBJECT(pTextView->gobj()), "copy-clipboard");
            _selection_to_clipboard(text_buffer, pTextView, iter_sel_start, iter_sel_end, num_chars, pCodebox);
        }
    }
}

// Paste from Clipboard
void CtClipboard::_paste_clipboard(Gtk::TextView* pTextView, CtCodebox* /*pCodebox*/)
{
    auto on_scope_exit = scope_guard([&](void*) { CtClipboard::_static_force_plain_text = false; });

    g_signal_stop_emission_by_name(G_OBJECT(pTextView->gobj()), "paste-clipboard");
    if (_pCtMainWin->curr_tree_iter().get_node_read_only())
        return;
    std::vector<Glib::ustring> targets = Gtk::Clipboard::get()->wait_for_targets();
    if (targets.empty())
        return;
    auto text_buffer = pTextView->get_buffer();
    text_buffer->erase_selection(true, pTextView->get_editable());

    // well, it's quite ugly code ...
    // need to recreate CtClipboard, because 'this' will be destroyed
    auto get_target = [&](const std::vector<Glib::ustring>& targets) -> std::tuple<Glib::ustring, std::function<void(const Gtk::SelectionData&, CtMainWin*, Gtk::TextView*, bool)>, bool>
    {
        auto received_plain_text = [](const Gtk::SelectionData& s, CtMainWin* win, Gtk::TextView* v, bool force) { CtClipboard{win}.on_received_to_plain_text(s, v, force); };
        auto received_rich_text = [](const Gtk::SelectionData& s, CtMainWin* win, Gtk::TextView* v, bool force) { CtClipboard{win}.on_received_to_rich_text(s, v, force); };
        auto received_codebox = [](const Gtk::SelectionData& s, CtMainWin* win, Gtk::TextView* v, bool force) { CtClipboard{win}.on_received_to_codebox(s, v, force); };
        auto received_table = [](const Gtk::SelectionData& s, CtMainWin* win, Gtk::TextView* v, bool force) { CtClipboard{win}.on_received_to_table(s, v, force, nullptr); };
        auto received_html = [](const Gtk::SelectionData& s, CtMainWin* win, Gtk::TextView* v, bool force) { CtClipboard{win}.on_received_to_html(s, v, force); };
        auto received_image = [](const Gtk::SelectionData& s, CtMainWin* win, Gtk::TextView* v, bool force) { CtClipboard{win}.on_received_to_image(s, v, force); };
        auto received_uri = [](const Gtk::SelectionData& s, CtMainWin* win, Gtk::TextView* v, bool force) { CtClipboard{win}.on_received_to_uri_list(s, v, force); };

        if (CtClipboard::_static_force_plain_text)
            for (auto& target : CtConst::TARGETS_PLAIN_TEXT)
                if (vec::exists(targets, target))
                    return std::make_tuple(target, received_plain_text, true);
        const bool is_rich_text = _pCtMainWin->curr_tree_iter().get_node_is_rich_text();
        if (is_rich_text) {
            if (vec::exists(targets, CtConst::TARGET_CTD_RICH_TEXT))
                return std::make_tuple(CtConst::TARGET_CTD_RICH_TEXT, received_rich_text, false);
            if (vec::exists(targets, CtConst::TARGET_CTD_CODEBOX))
                return std::make_tuple(CtConst::TARGET_CTD_CODEBOX, received_codebox, false);
            if (vec::exists(targets, CtConst::TARGET_CTD_TABLE))
                return std::make_tuple(CtConst::TARGET_CTD_TABLE, received_table, false);
#if defined(_WIN32)
            if (vec::exists(targets, CtConst::TARGET_WIN_HTML))
                return std::make_tuple(CtConst::TARGET_WIN_HTML, received_html, false);
#endif // _WIN32
            for (auto& target : CtConst::TARGETS_HTML)
                if (vec::exists(targets, target))
                    return std::make_tuple(target, received_html, false);
        }
#if defined(_WIN32)
        if (vec::exists(targets, CtConst::TARGET_WINDOWS_FILE_NAME))
            return std::make_tuple(CtConst::TARGET_WINDOWS_FILE_NAME, received_uri, false);
#endif // _WIN32
#ifdef __APPLE__
        if (vec::exists(targets, "NSFilenamesPboardType"))
            return std::make_tuple("NSFilenamesPboardType", received_uri, false);
#endif // __APPLE__
        if (vec::exists(targets, CtConst::TARGET_URI_LIST))
            return std::make_tuple(CtConst::TARGET_URI_LIST, received_uri, false);
        for (auto& target : CtConst::TARGETS_PLAIN_TEXT)
            if (vec::exists(targets, target))
                return std::make_tuple(target, received_plain_text, false);
        if (is_rich_text) {
            // images at very last because of mac os target of mime type icon
            for (auto& target : CtConst::TARGETS_IMAGES)
                if (vec::exists(targets, target))
                    return std::make_tuple(target, received_image, false);
        }
        return std::make_tuple(Glib::ustring(), received_plain_text, false);
    };

    auto [target, target_fun, force_plain_text] = get_target(targets);
    if (target.empty()) {
        spdlog::warn("targets not handled {}", str::join(targets, ", "));
        return;
    };
    //spdlog::debug("targets: {}", str::join(targets, ", "));

    auto receive_fun = sigc::bind(target_fun, _pCtMainWin, pTextView, force_plain_text);
    Gtk::Clipboard::get()->request_contents(target, receive_fun);
}

void CtClipboard::table_row_to_clipboard(CtTable* pTable)
{
    CtClipboardData* clip_data = new CtClipboardData();
    pTable->to_xml(clip_data->xml_doc.create_root_node("root"), 0, nullptr);
    clip_data->html_text = CtExport2Html(_pCtMainWin).table_export_to_html(pTable);

    _set_clipboard_data({CtConst::TARGET_CTD_TABLE, CtConst::TARGETS_HTML[0]}, clip_data);
}

void CtClipboard::table_row_paste(CtTable* pTable)
{
    std::vector<Glib::ustring> targets = Gtk::Clipboard::get()->wait_for_targets();
    if (vec::exists(targets, CtConst::TARGET_CTD_TABLE))
    {
        auto win = _pCtMainWin;
        Gtk::TextView* view = &_pCtMainWin->get_text_view();
        auto received_table = [win, view, pTable](const Gtk::SelectionData& s) { CtClipboard{win}.on_received_to_table(s, view, false, pTable);};
        Gtk::Clipboard::get()->request_contents(CtConst::TARGET_CTD_TABLE, received_table);
    }
}

void CtClipboard::node_link_to_clipboard(CtTreeIter node)
{
    CtClipboardData* clip_data = new CtClipboardData();
    std::string tml = R"XML(<?xml version="1.0" encoding="UTF-8"?><root><slot><rich_text link="node {}">{}</rich_text></slot></root>)XML";
    clip_data->rich_text = fmt::format(tml, node.get_node_id(), str::xml_escape(node.get_node_name()));
    clip_data->plain_text = "node: " + node.get_node_name();

    _set_clipboard_data({CtConst::TARGET_CTD_RICH_TEXT, CtConst::TARGET_CTD_PLAIN_TEXT}, clip_data);
}

void CtClipboard::anchor_link_to_clipboard(CtTreeIter node, const Glib::ustring& anchor_name)
{
    CtClipboardData* clip_data = new CtClipboardData();
    std::string tml = R"XML(<?xml version="1.0" encoding="UTF-8"?><root><slot><rich_text link="node {} {}">{}</rich_text></slot></root>)XML";
    clip_data->rich_text = fmt::format(tml, node.get_node_id(), str::xml_escape(anchor_name), str::xml_escape(anchor_name));
    clip_data->plain_text = "anchor: " + anchor_name;

    _set_clipboard_data({CtConst::TARGET_CTD_RICH_TEXT, CtConst::TARGET_CTD_PLAIN_TEXT}, clip_data);
}

// Given text_buffer and selection, returns the rich text xml
Glib::ustring CtClipboard::rich_text_get_from_text_buffer_selection(CtTreeIter node_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end,
                                                 gchar change_case /*="n"*/, bool exclude_iter_sel_end /*=false*/)
{
    int iter_sel_start_offset = iter_sel_start.get_offset();
    int iter_sel_end_offset = iter_sel_end.get_offset();
    if (exclude_iter_sel_end)
        iter_sel_end_offset -= 1;
    std::list<CtAnchoredWidget*> widget_vector = node_iter.get_anchored_widgets(iter_sel_start_offset, iter_sel_end_offset);

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
    CtStorageXmlHelper::save_buffer_no_widgets_to_xml(dom_iter, text_buffer, start_offset, end_offset, change_case);

    if (obj_element != nullptr)
    {
        xmlpp::Element* elm_dom_iter = root->add_child("slot");
        obj_element->to_xml(elm_dom_iter, 0, nullptr);
    }
}

// From XML String to Text Buffer
void CtClipboard::from_xml_string_to_buffer(Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                            const Glib::ustring& xml_string,
                                            bool* const pPasteHadWidgets/*= nullptr*/)
{
    xmlpp::DomParser parser;
    if (not CtXmlHelper::safe_parse_memory(parser, xml_string) or
        parser.get_document()->get_root_node()->get_name() != "root")
    {
        throw std::invalid_argument("rich text from clipboard error");
    }

    auto on_scope_exit = scope_guard([&](void*) {
        _pCtMainWin->get_state_machine().not_undoable_timeslot_set(false);
        _pCtMainWin->get_state_machine().update_state();
    });
    _pCtMainWin->get_state_machine().not_undoable_timeslot_set(true);

    std::list<CtAnchoredWidget*> widgets;
    for (xmlpp::Node* slot_node: parser.get_document()->get_root_node()->get_children())
    {
        if (slot_node->get_name() != "slot")
            continue;
        for (xmlpp::Node* child_node: slot_node->get_children())
        {
            Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(text_buffer);
            Gtk::TextIter insert_iter = text_buffer->get_insert()->get_iter();
            CtStorageXmlHelper(_pCtMainWin).get_text_buffer_one_slot_from_xml(gsv_buffer, child_node, widgets, &insert_iter, insert_iter.get_offset());
        }
    }
    if (not widgets.empty()) {
        _pCtMainWin->get_tree_store().addAnchoredWidgets(
                    _pCtMainWin->curr_tree_iter(),
                    widgets,
                    &_pCtMainWin->get_text_view());
        if (pPasteHadWidgets) {
            *pPasteHadWidgets = true;
        }
    }
    else if (pPasteHadWidgets) {
        *pPasteHadWidgets = false;
    }
}

// Write the Selected Content to the Clipboard
void CtClipboard::_selection_to_clipboard(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextView* /*sourceview*/, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end, int num_chars, CtCodebox* pCodebox)
{
    Glib::ustring node_syntax_high = _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting();
    CtImage* pixbuf_target = nullptr;
    if (not pCodebox and node_syntax_high == CtConst::RICH_TEXT_ID and num_chars == 1)
    {
        std::list<CtAnchoredWidget*> widget_vector = _pCtMainWin->curr_tree_iter().get_anchored_widgets(iter_sel_start.get_offset(), iter_sel_end.get_offset());
        if (widget_vector.size() > 0)
        {
            if (CtImage* image = dynamic_cast<CtImage*>(widget_vector.front()))
            {
                pixbuf_target = image;
#ifdef _WIN32
                // image target doesn't work on Win32 with other targets, so have to set it directly
                // then copy/paste into MS Paint will work. Pasting into CT back also will work
                if (image->get_type() == CtAnchWidgType::ImagePng) {
                    Gtk::Clipboard::get()->set_image(image->get_pixbuf());
                    return;
                }
#endif
            }
            else if (CtTable* table = dynamic_cast<CtTable*>(widget_vector.front()))
            {
                CtClipboardData* clip_data = new CtClipboardData();
                table->to_xml(clip_data->xml_doc.create_root_node("root"), 0, nullptr);
                clip_data->html_text = CtExport2Html(_pCtMainWin).table_export_to_html(table);
                clip_data->plain_text = CtExport2Txt(_pCtMainWin).get_table_plain(table);

                _set_clipboard_data({CtConst::TARGET_CTD_TABLE, CtConst::TARGETS_HTML[0], CtConst::TARGET_CTD_PLAIN_TEXT}, clip_data);
                return;
            }
            else if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widget_vector.front()))
            {
                CtClipboardData* clip_data = new CtClipboardData();
                codebox->to_xml(clip_data->xml_doc.create_root_node("root"), 0, nullptr);
                clip_data->html_text = CtExport2Html(_pCtMainWin).codebox_export_to_html(codebox);
                if (num_chars == 1) // just copy one codebox
                    clip_data->plain_text = _codebox_to_yaml(codebox);
                else
                    clip_data->plain_text = CtExport2Txt(_pCtMainWin).get_codebox_plain(codebox);

                _set_clipboard_data({CtConst::TARGET_CTD_CODEBOX, CtConst::TARGETS_HTML[0], CtConst::TARGET_CTD_PLAIN_TEXT}, clip_data);
                return;
            }
        }
    }

    CtClipboardData* clip_data = new CtClipboardData();
    clip_data->html_text = CtExport2Html(_pCtMainWin).selection_export_to_html(text_buffer, iter_sel_start, iter_sel_end, !pCodebox ? node_syntax_high : CtConst::PLAIN_TEXT_ID);
    if (not pCodebox and node_syntax_high == CtConst::RICH_TEXT_ID)
    {
        std::vector<std::string> targets_vector;
        clip_data->plain_text = CtExport2Txt{_pCtMainWin}.selection_export_to_txt(_pCtMainWin->curr_tree_iter(), text_buffer, iter_sel_start.get_offset(), iter_sel_end.get_offset(), true);
        clip_data->rich_text = rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer, iter_sel_start, iter_sel_end);
        if (not CtClipboard::_static_force_plain_text)
        {
            targets_vector = {CtConst::TARGET_CTD_PLAIN_TEXT, CtConst::TARGET_CTD_RICH_TEXT, CtConst::TARGETS_HTML[0], CtConst::TARGETS_HTML[1]};
            if (pixbuf_target)
            {
                clip_data->pix_buf = pixbuf_target->get_pixbuf();
                targets_vector.push_back(CtConst::TARGETS_IMAGES[0]);
            }
        }
        else
            targets_vector = {CtConst::TARGET_CTD_PLAIN_TEXT};

        _set_clipboard_data(targets_vector, clip_data);
    }
    else
    {
        clip_data->plain_text = text_buffer->get_text(iter_sel_start, iter_sel_end);
        std::vector<std::string> targets_vector;
        if (not CtClipboard::_static_force_plain_text)
            targets_vector = {CtConst::TARGET_CTD_PLAIN_TEXT, CtConst::TARGETS_HTML[0], CtConst::TARGETS_HTML[1]};
        else
            targets_vector = {CtConst::TARGET_CTD_PLAIN_TEXT};
        _set_clipboard_data(targets_vector, clip_data);
    }
}

void CtClipboard::_set_clipboard_data(const std::vector<std::string>& targets_list, CtClipboardData* clip_data)
{
    std::vector<Gtk::TargetEntry> target_entries;
    for (auto& target: targets_list)
        target_entries.push_back(Gtk::TargetEntry(target));

    CtMainWin*  win = _pCtMainWin;
    // can't use this, because it will invalid, so make a copy
    auto clip_data_get = [win, clip_data](Gtk::SelectionData& selection_data, guint /*info*/){
        CtClipboard{win}._on_clip_data_get(selection_data, clip_data);
    };
    auto clip_data_clear = [clip_data]() {
       delete clip_data;
    };
    Gtk::Clipboard::get()->set(target_entries, clip_data_get, clip_data_clear);
}

// based on def get_func(self, clipboard, selectiondata, info, data)
void  CtClipboard::_on_clip_data_get(Gtk::SelectionData& selection_data, CtClipboardData* clip_data)
{
    Glib::ustring target = selection_data.get_target();
    if (target == CtConst::TARGET_CTD_PLAIN_TEXT)
        selection_data.set(target, 8, (const guint8*)clip_data->plain_text.c_str(), (int)clip_data->plain_text.bytes());
    else if (target == CtConst::TARGET_CTD_RICH_TEXT)
        selection_data.set("UTF8_STRING", 8, (const guint8*)clip_data->rich_text.c_str(), (int)clip_data->rich_text.bytes());
    else if (vec::exists(CtConst::TARGETS_HTML, target))
    {
#ifndef _WIN32
        selection_data.set(target, 8, (const guint8*)clip_data->html_text.c_str(), (int)clip_data->html_text.bytes());
#else
        if (target == CtConst::TARGETS_HTML[0])
        {
            glong utf16text_len = 0;
            g_autofree gunichar2* utf16text = g_utf8_to_utf16(clip_data->html_text.c_str(), (glong)clip_data->html_text.bytes(), nullptr, &utf16text_len, nullptr);
            if (utf16text and utf16text_len > 0)
                selection_data.set(target, 8, (guint8*)utf16text, (int)utf16text_len);
        }
        else {
            const auto bodyStart = clip_data->html_text.find("<body>");
            const auto bodyEnd = clip_data->html_text.rfind("</body>");
            std::string html = Win32HtmlFormat{}.encode(bodyStart != std::string::npos and bodyEnd != std::string::npos ?
                clip_data->html_text.substr(bodyStart, bodyEnd - bodyStart) : clip_data->html_text);
            selection_data.set(target, 8, (const guint8*)html.c_str(), (int)html.size());
        }
#endif // _WIN32
    }
    else if (target == CtConst::TARGET_CTD_CODEBOX)
    {
        Glib::ustring xml = clip_data->xml_doc.write_to_string();
        selection_data.set("UTF8_STRING", 8, (const guint8*)xml.c_str(), (int)xml.bytes());
    }
    else if (target == CtConst::TARGET_CTD_TABLE)
    {
        Glib::ustring xml = clip_data->xml_doc.write_to_string();
        selection_data.set("UTF8_STRING", 8, (const guint8*)xml.c_str(), (int)xml.bytes());
    }
    else if (target == CtConst::TARGETS_IMAGES[0])
        selection_data.set_pixbuf(clip_data->pix_buf);
}

// From Clipboard to Plain Text
void CtClipboard::on_received_to_plain_text(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool force_plain_text)
{
    std::string plain_text = selection_data.get_text(); // returns UTF-8 string if text type recognised and could be converted to UTF-8; empty otherwise
    if (plain_text.empty()) {
        plain_text = selection_data.get_data_as_string();
        CtStrUtil::convert_if_not_utf8(plain_text, false/*sanitise*/);
    }
    plain_text = str::sanitize_bad_symbols(plain_text);

    if (plain_text.empty())
    {
        spdlog::error("? no clipboard plain text");
        return;
    }

    if (_pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID)
        if (str::startswith(plain_text, "- codebox:")) {
            _yaml_to_codebox(plain_text, pTextView);
            return;
        }

    auto curr_buffer = pTextView->get_buffer();
    Gtk::TextIter iter_insert = curr_buffer->get_insert()->get_iter();
    int start_offset = iter_insert.get_offset();
    curr_buffer->insert(iter_insert, plain_text);
    if (_pCtMainWin->curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID and !force_plain_text)
    {
        auto web_links_offsets = CtImports::get_web_links_offsets_from_plain_text(plain_text);
        if (web_links_offsets.size()) {
            for (auto& offset : web_links_offsets) {
                Gtk::TextIter iter_sel_start = curr_buffer->get_iter_at_offset(start_offset + offset.first);
                Gtk::TextIter iter_sel_end = curr_buffer->get_iter_at_offset(start_offset + offset.second);
                Glib::ustring link_url = iter_sel_start.get_text(iter_sel_end);
                if (not str::startswith(link_url, "htt") and not str::startswith(link_url, "ftp"))
                    link_url = "http://" + link_url;
                Glib::ustring property_value = "webs " + link_url;
                curr_buffer->apply_tag_by_name(_pCtMainWin->get_text_tag_name_exist_or_create(CtConst::TAG_LINK, property_value),
                                               iter_sel_start, iter_sel_end);
            }
        }
        else
        {
            // check for file or folder path
            if (plain_text.find(CtConst::CHAR_NEWLINE) == Glib::ustring::npos)
            {
                Glib::ustring property_value;
                if (Glib::file_test(plain_text, Glib::FILE_TEST_IS_DIR))
                    property_value = "fold " + Glib::Base64::encode(plain_text);
                else if (Glib::file_test(plain_text, Glib::FILE_TEST_IS_REGULAR))
                    property_value = "file " + Glib::Base64::encode(plain_text);
                if (property_value != "")
                {
                    Gtk::TextIter iter_sel_end = curr_buffer->get_insert()->get_iter();
                    Gtk::TextIter iter_sel_start = iter_sel_end;
                    iter_sel_start.backward_chars((int)plain_text.size());
                    curr_buffer->apply_tag_by_name(_pCtMainWin->get_text_tag_name_exist_or_create(CtConst::TAG_LINK, property_value),
                                                   iter_sel_start, iter_sel_end);
                }
            }
        }
    }
    pTextView->scroll_to(curr_buffer->get_insert());
}

// From Clipboard to Rich Text
void CtClipboard::on_received_to_rich_text(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool)
{
#ifdef __APPLE__
    Glib::ustring rich_text = selection_data.get_data_as_string();
#else
    Glib::ustring rich_text = selection_data.get_text();
#endif
    if (rich_text.empty())
    {
        spdlog::error("? no clipboard rich text");
        return;
    }
    bool pasteHadWidgets{false};
    from_xml_string_to_buffer(pTextView->get_buffer(), rich_text, &pasteHadWidgets);
    pTextView->scroll_to(pTextView->get_buffer()->get_insert());
}

// From Clipboard to CodeBox
void CtClipboard::on_received_to_codebox(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool)
{
#ifdef __APPLE__
    Glib::ustring xml_text = selection_data.get_data_as_string();
#else
    Glib::ustring xml_text = selection_data.get_text();
#endif
    if (xml_text.empty())
    {
        spdlog::error("? no clipboard xml text");
        return;
    }

    _xml_to_codebox(xml_text, pTextView);
}

// From Clipboard to Table
void CtClipboard::on_received_to_table(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool, CtTable* parentTable)
{
#ifdef __APPLE__
    Glib::ustring xml_text = selection_data.get_data_as_string();
#else
    Glib::ustring xml_text = selection_data.get_text();
#endif
    if (xml_text.empty())
    {
        spdlog::error("? no clipboard xml text");
        return;
    }

    xmlpp::DomParser parser;
    if (not CtXmlHelper::safe_parse_memory(parser, xml_text) or
        parser.get_document()->get_root_node()->get_name() != "root" or
        not parser.get_document()->get_root_node()->get_first_child("table"))
    {
        spdlog::error("table from clipboard error");
        return;
    }

    if (parentTable) {
        CtTableMatrix tableMatrix;
        CtTableColWidths tableColWidths;
        CtStorageXmlHelper{_pCtMainWin}.populate_table_matrix(
            tableMatrix,
            static_cast<xmlpp::Element*>(parser.get_document()->get_root_node()->get_first_child("table")),
            tableColWidths);

        int col_num = (int)parentTable->get_table_matrix()[0].size();
        int insert_after = parentTable->current_row() - 1;
        if (insert_after < 0) insert_after = 0;
        for (int row = 1/*skip header*/; row < (int)tableMatrix.size(); ++row) {
            std::vector<Glib::ustring> new_row;
            std::transform(tableMatrix[row].begin(), tableMatrix[row].end(), std::back_inserter(new_row), [](void* cell) {
                return static_cast<CtTextCell*>(cell)->get_text_content();
            });
            while ((int)new_row.size() > col_num) new_row.pop_back();
            while ((int)new_row.size() < col_num) new_row.push_back("");
            parentTable->row_add(insert_after + (row-1), &new_row);
        }
        for (auto& row : tableMatrix) {
            for (void* cell : row) {
                delete static_cast<CtTextCell*>(cell);
            }
        }
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true /*new_machine_state*/);
    }
    else {
        std::list<CtAnchoredWidget*> widgets;
        Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(pTextView->get_buffer());
        Gtk::TextIter insert_iter = pTextView->get_buffer()->get_insert()->get_iter();
        CtStorageXmlHelper(_pCtMainWin).get_text_buffer_one_slot_from_xml(gsv_buffer, parser.get_document()->get_root_node()->get_first_child("table"), widgets, &insert_iter, insert_iter.get_offset());
        if (not widgets.empty()) {
            _pCtMainWin->get_tree_store().addAnchoredWidgets(
                        _pCtMainWin->curr_tree_iter(),
                        widgets, &_pCtMainWin->get_text_view());
            _pCtMainWin->get_state_machine().update_state();
        }

        pTextView->scroll_to(pTextView->get_buffer()->get_insert());
    }
}

// From Clipboard to HTML Text
void CtClipboard::on_received_to_html(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool)
{
    std::string html_content = selection_data.get_text(); // returns UTF-8 string if text type recognised and could be converted to UTF-8; empty otherwise
    if (html_content.empty()) {
        html_content = selection_data.get_data_as_string();
#ifdef _WIN32
        html_content = Win32HtmlFormat().convert_from_ms_clipboard(html_content);
#else
        CtStrUtil::convert_if_not_utf8(html_content, false/*sanitise*/);
#endif
    }
    html_content = str::sanitize_bad_symbols(html_content);

    CtHtml2Xml parser(_pCtMainWin->get_ct_config());
    parser.feed(html_content);
    bool pasteHadWidgets{false};
    from_xml_string_to_buffer(pTextView->get_buffer(), parser.to_string(), &pasteHadWidgets);
    pTextView->scroll_to(pTextView->get_buffer()->get_insert());
}

// From Clipboard to Image
void CtClipboard::on_received_to_image(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool)
{
    Glib::RefPtr<const Gdk::Pixbuf> rPixbuf = selection_data.get_pixbuf();
    if (rPixbuf) {
        Glib::ustring link = "";
        _pCtMainWin->get_ct_actions()->image_insert_png(pTextView->get_buffer()->get_insert()->get_iter(), rPixbuf->copy(), link, "");
        pTextView->scroll_to(pTextView->get_buffer()->get_insert());
    }
    else {
        spdlog::debug("invalid image in clipboard");
    }
}

// From Clipboard to URI list
void CtClipboard::on_received_to_uri_list(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, const bool /*forcePlain*/, const bool /*fromDragNDrop = false*/)
{
    std::string uri_content = selection_data.get_text(); // returns UTF-8 string if text type recognised and could be converted to UTF-8; empty otherwise
    //spdlog::debug("1: '{}'", uri_content);
    if (uri_content.empty()) {
        uri_content = selection_data.get_data_as_string();
        //spdlog::debug("2: '{}'", uri_content);
        CtStrUtil::convert_if_not_utf8(uri_content, false/*sanitise*/);
    }
    uri_content = str::sanitize_bad_symbols(uri_content);

    auto pCtTextView = dynamic_cast<CtTextView*>(pTextView);
    const std::string syntax_highlighting = pCtTextView ? pCtTextView->get_syntax_highlighting() : _pCtMainWin->curr_tree_iter().get_node_syntax_highlighting();
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = pTextView->get_buffer();
    if (syntax_highlighting != CtConst::RICH_TEXT_ID) {
        rTextBuffer->insert(rTextBuffer->get_insert()->get_iter(), uri_content);
    }
    else {
        std::vector<Glib::ustring> uri_list = selection_data.get_uris();
        //spdlog::debug("3: {}", uri_list.size());
        if (uri_list.empty() and not uri_content.empty()) {
#ifdef __APPLE__
            std::string::size_type startOffset{0};
            const std::string tagStart{"<string>"};
            const std::string tagEnd{"</string>"};
            do {
                startOffset = uri_content.find(tagStart, startOffset);
                if (std::string::npos == startOffset) {
                    break;
                }
                startOffset += tagStart.size();
                std::string::size_type endOffset = uri_content.find(tagEnd, startOffset);
                if (std::string::npos == endOffset) {
                    break;
                }
                uri_list.push_back(uri_content.substr(startOffset, endOffset-startOffset));
                startOffset = endOffset + tagEnd.size();
            } while (true);
#else // !__APPLE__
            for (const auto& uri : str::split(uri_content, "\r\n")) {
                if (not uri.empty()) {
                    uri_list.push_back(uri);
                }
            }
#endif // !__APPLE__
        }
        bool subsequent_insert{false};
        for (auto& element : uri_list) {
            if (element.empty()) continue;
            Glib::ustring property_value;
            if (str::startswith_any(element, CtConst::WEB_LINK_STARTERS)) {
                property_value = "webs " + element;
            }
            else if (str::startswith(element, "file://")) {
                Glib::ustring file_path = element.substr(7);
                file_path = str::replace(file_path, "%20", CtConst::CHAR_SPACE);
                g_autofree gchar* mimetype = g_content_type_guess(file_path.c_str(), nullptr, 0, nullptr);
                if (mimetype and str::startswith(mimetype, "image/") and Glib::file_test(file_path, Glib::FILE_TEST_IS_REGULAR)) {
                    try {
                        auto pixbuf = Gdk::Pixbuf::create_from_file(file_path);
                        _pCtMainWin->get_ct_actions()->image_insert_png(rTextBuffer->get_insert()->get_iter(), pixbuf, "", "");
                        for (int i = 0; i < 3; ++i) {
                            rTextBuffer->insert(rTextBuffer->get_insert()->get_iter(), CtConst::CHAR_SPACE);
                        }
                        continue;
                    }
                    catch (...) {}
                }
                if (Glib::file_test(file_path, Glib::FILE_TEST_IS_DIR)) {
                    property_value = "fold " + Glib::Base64::encode(file_path);
                }
                else if (Glib::file_test(file_path, Glib::FILE_TEST_IS_REGULAR)) {
                    if (subsequent_insert) {
                        rTextBuffer->insert(rTextBuffer->get_insert()->get_iter(), CtConst::CHAR_NEWLINE);
                    }
                    _pCtMainWin->get_ct_actions()->embfile_insert_path(file_path);
                }
                else {
                    spdlog::debug("'{}' not dir or file", file_path);
                }
            }
            else {
                if (Glib::file_test(element, Glib::FILE_TEST_IS_DIR)) {
                    property_value = "fold " + Glib::Base64::encode(element);
                }
                else if (Glib::file_test(element, Glib::FILE_TEST_IS_REGULAR)) {
                    if (subsequent_insert) {
                        rTextBuffer->insert(rTextBuffer->get_insert()->get_iter(), CtConst::CHAR_NEWLINE);
                    }
                    _pCtMainWin->get_ct_actions()->embfile_insert_path(element);
                }
                else {
                    spdlog::debug("'{}' not dir or file", element);
                }
            }
            if (not property_value.empty()) {
                if (subsequent_insert) {
                    rTextBuffer->insert(rTextBuffer->get_insert()->get_iter(), CtConst::CHAR_NEWLINE);
                }
                const int start_offset = rTextBuffer->get_insert()->get_iter().get_offset();
                rTextBuffer->insert(rTextBuffer->get_insert()->get_iter(), element);
                Gtk::TextIter iter_sel_start = rTextBuffer->get_iter_at_offset(start_offset);
                Gtk::TextIter iter_sel_end = rTextBuffer->get_iter_at_offset(start_offset + (int)element.length());
                rTextBuffer->apply_tag_by_name(_pCtMainWin->get_text_tag_name_exist_or_create(CtConst::TAG_LINK, property_value),
                                               iter_sel_start, iter_sel_end);
            }
            subsequent_insert = true;
        }
    }
    pTextView->scroll_to(rTextBuffer->get_insert());
}

Glib::ustring CtClipboard::_codebox_to_yaml(CtCodebox *codebox)
{
    // indent every line by 6 spaces to use them in yaml block
    Glib::ustring indent = "      ";
    Glib::ustring source = codebox->get_text_content();
    source = indent + source;
    source = str::replace(source, "\n", "\n" + indent);

    Glib::ustring yaml_text;
    yaml_text.append("- codebox:\n");
    yaml_text.append("    syntax: " + codebox->get_syntax_highlighting() + "\n");
    yaml_text.append("    width: " + std::to_string(codebox->get_frame_width()) + "\n");
    yaml_text.append("    height: " + std::to_string(codebox->get_frame_height()) + "\n");
    yaml_text.append("    width_in_pixels: " + std::string(codebox->get_frame_width() ? "true" : "false") + "\n");
    yaml_text.append("    highlight_brackets: " + std::string(codebox->get_highlight_brackets() ? "true" : "false") + "\n");
    yaml_text.append("    source: |-\n");
    yaml_text.append(source);
    yaml_text.append("\n");

    return yaml_text;
}

void CtClipboard::_yaml_to_codebox(const Glib::ustring& yaml_text, Gtk::TextView* pTextView)
{
    // don't want to duplicate code, so convert yaml to xml
    // it has overhead, but text volume is small, so it's ok
    try
    {
        xmlpp::Document xml_doc;
        xmlpp::Element* p_node_parent = xml_doc.create_root_node("root");
        xmlpp::Element* p_codebox_node = p_node_parent->add_child("codebox");
        p_codebox_node->set_attribute("char_offset", "0");
        p_codebox_node->set_attribute(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT);
        p_codebox_node->set_attribute("show_line_numbers", std::to_string(false));

        auto get_value = [](const std::string& pair) -> std::string {
            return str::trim(str::split(pair, ":")[1]);
        };

        enum class ParseState {OUT_BLOCK, IN_BLOCK};
        ParseState state = ParseState::OUT_BLOCK;
        std::string block_indent = "      ";
        std::vector<Glib::ustring> block_lines;
        std::vector<Glib::ustring> lines = str::split(yaml_text, CtConst::CHAR_NEWLINE);
        for (const auto& curr_line : lines) {
            if (state == ParseState::OUT_BLOCK) {
                Glib::ustring trim_line = str::trim(curr_line);
                if (trim_line == "source: |-") {
                    state = ParseState::IN_BLOCK;
                }
                else if (str::startswith(trim_line, "syntax:"))             p_codebox_node->set_attribute("syntax_highlighting", get_value(trim_line));
                else if (str::startswith(trim_line, "width:"))              p_codebox_node->set_attribute("frame_width", get_value(trim_line));
                else if (str::startswith(trim_line, "height:"))             p_codebox_node->set_attribute("frame_height", get_value(trim_line));
                else if (str::startswith(trim_line, "width_in_pixels:"))    p_codebox_node->set_attribute("width_in_pixels", get_value(trim_line));
                else if (str::startswith(trim_line, "highlight_brackets:")) p_codebox_node->set_attribute("highlight_brackets", get_value(trim_line));
            }
            else {
                if (!str::startswith(curr_line, block_indent)) {
                    state = ParseState::OUT_BLOCK;
                }
                else {
                    block_lines.push_back(curr_line.substr(6)); // don't add \n at the end of the block
                }
            }
        }
        p_codebox_node->add_child_text(str::join(block_lines, "\n"));
        _xml_to_codebox(xml_doc.write_to_string(), pTextView);
    }
    catch (std::exception& e) {
        spdlog::error("_yaml_to_codebox exception: {}\n{}", e.what(), yaml_text);
    }
    catch (...) {
        spdlog::error("_yaml_to_codebox unknown exception\n{}", yaml_text);
    }
}

void CtClipboard::_xml_to_codebox(const Glib::ustring &xml_text, Gtk::TextView* pTextView)
{
    xmlpp::DomParser parser;
    if (not CtXmlHelper::safe_parse_memory(parser, xml_text) or
        parser.get_document()->get_root_node()->get_name() != "root" or
        not parser.get_document()->get_root_node()->get_first_child("codebox"))
    {
        spdlog::error("codebox from clipboard error");
        return;
    }

    std::list<CtAnchoredWidget*> widgets;
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(pTextView->get_buffer());
    Gtk::TextIter insert_iter = pTextView->get_buffer()->get_insert()->get_iter();
    CtStorageXmlHelper(_pCtMainWin).get_text_buffer_one_slot_from_xml(gsv_buffer, parser.get_document()->get_root_node()->get_first_child("codebox"), widgets, &insert_iter, insert_iter.get_offset());
    if (not widgets.empty())
    {
        _pCtMainWin->get_tree_store().addAnchoredWidgets(
                    _pCtMainWin->curr_tree_iter(),
                    widgets, &_pCtMainWin->get_text_view());
        _pCtMainWin->get_state_machine().update_state();
    }
    pTextView->scroll_to(pTextView->get_buffer()->get_insert());
}

std::string Win32HtmlFormat::encode(std::string html_in)
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

    std::string head = "", source = CtConst::APP_NAME + std::string(CtConst::CT_VERSION);
    std::string html = fmt::format(DEFAULT_HTML_BODY, head, html_in);
    std::string::size_type fragmentStart = html.find(html_in);
    std::string::size_type fragmentEnd = fragmentStart + html_in.size();

    // How long is the prefix going to be?
    std::string dummyPrefix = fmt::format(MARKER_BLOCK_OUTPUT, 0, 0, 0, 0, 0, 0, source);
    std::string::size_type lenPrefix = dummyPrefix.size();

    std::string prefix = fmt::format(MARKER_BLOCK_OUTPUT,
                lenPrefix, html.size() + lenPrefix,
                fragmentStart + lenPrefix, fragmentEnd + lenPrefix,
                fragmentStart + lenPrefix, fragmentEnd + lenPrefix,
                source);
    return prefix + html;
}

std::string Win32HtmlFormat::convert_from_ms_clipboard(std::string html_in)
{
    auto get_arg_value = [&](std::string arg_name) {
        auto re = Glib::Regex::create(arg_name + "\\s*:\\s*(.*?)$", Glib::RegexCompileFlags::REGEX_CASELESS | Glib::RegexCompileFlags::REGEX_MULTILINE);
        Glib::MatchInfo match;
        if (!re->match(html_in, match))
            return -1;
        return std::atoi(match.fetch(1).c_str());
    };

    int start = get_arg_value("StartHTML");
    int end = get_arg_value("EndHTML");
    if (start < 0 || end < 0 || end < start)
        return html_in;
    html_in = html_in.substr(start, end - start);
    html_in = str::replace(html_in, "\r", "");
    return html_in;
}
