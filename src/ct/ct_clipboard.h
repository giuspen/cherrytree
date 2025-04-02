/*
 * ct_clipboard.h
 *
 * Copyright 2009-2025
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

#pragma once

#include <gtkmm/textiter.h>
#include "ct_treestore.h"
#include "ct_codebox.h"
#include "ct_table.h"
#include <libxml++/libxml++.h>

struct CtClipboardData
{
    CtClipboardData() {}
    xmlpp::Document xml_doc;
    Glib::ustring html_text;
    Glib::ustring plain_text;
    Glib::ustring rich_text;
    Glib::RefPtr<Gdk::Pixbuf> pix_buf;
    bool from_column_edit{false};
};

class CtClipboard
{
public:
    CtClipboard(CtMainWin* pCtMainWin);

public:
    static void on_cut_clipboard(GtkTextView* pTextView, gpointer codebox);
    static void on_copy_clipboard(GtkTextView* pTextView, gpointer codebox);
    static void on_paste_clipboard(GtkTextView* pTextView, gpointer codebox);
    static void force_plain_text() { _static_force_plain_text = true; }

private:
    void _cut_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox);
    void _copy_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox);
    void _paste_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox);

public:
    void          table_row_to_clipboard(CtTableCommon* pTable);
    void          table_row_paste(CtTableCommon* pTable);
    void          table_column_to_clipboard(CtTableCommon* pTable);
    void          table_column_paste(CtTableCommon* pTable);
    void          node_link_to_clipboard(CtTreeIter node);
    void          anchor_link_to_clipboard(CtTreeIter node, const Glib::ustring& anchor_name);
    Glib::ustring rich_text_get_from_text_buffer_selection(CtTreeIter node_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                                           Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end,
                                                           gchar change_case = 'n', bool exclude_iter_sel_end = false);
    void from_xml_string_to_buffer(Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                   const Glib::ustring& xml_string,
                                   bool* const pPasteHadWidgets = nullptr);

private:
    void _rich_text_process_slot(xmlpp::Element* root, int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                 CtAnchoredWidget* obj_element, gchar change_case = 'n');
    void _dom_node_to_rich_text(Glib::RefPtr<Gtk::TextBuffer> text_buffer, xmlpp::Node* dom_node);

private:
    void _selection_to_clipboard(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextView* sourceview, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end, int num_chars, CtCodebox* pCodebox);
    void _set_clipboard_data(const std::vector<std::string>& targets_list, CtClipboardData* clip_data);

private:
    void _on_clip_data_get(Gtk::SelectionData& selection_data, CtClipboardData* clip_data);

public:
    void on_received_to_plain_text(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool force_plain_text);
    void on_received_to_rich_text(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool);
    void on_received_to_codebox(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool);
    void on_received_to_table(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, const bool is_column, CtTableCommon* parentTable);
    void on_received_to_html(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool);
    void on_received_to_image(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, bool);
    void on_received_to_uri_list(const Gtk::SelectionData& selection_data, Gtk::TextView* pTextView, const bool forcePlain, const bool fromDragNDrop = false);

private:
    Glib::ustring _codebox_to_yaml(CtCodebox* codebox);
    void          _yaml_to_codebox(const Glib::ustring& yaml_text, Gtk::TextView* pTextView);
    void          _xml_to_codebox(const Glib::ustring& xml_text, Gtk::TextView* pTextView);

private:
    static bool _static_force_plain_text;
    static bool _static_from_column_edit;
    CtMainWin* const _pCtMainWin;
};

// This class adds support for Windows "HTML Format" clipboard content type
// Code is based on example code from http://code.activestate.com/recipes/474121/
// written by Phillip Piper (jppx1[at]bigfoot.com)
class Win32HtmlFormat
{
public:
    std::string encode(std::string html_in);
    std::string convert_from_ms_clipboard(std::string html_in);
};
