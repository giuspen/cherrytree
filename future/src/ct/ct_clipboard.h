/*
 * ct_clipboard.h
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

#pragma once

#include <gtkmm/textiter.h>
#include <ct_treestore.h>
#include <libxml++/libxml++.h>
#include "ct_codebox.h"

struct CtClipboardData
{
    xmlpp::Document xml_doc;
    Glib::ustring html_text;
    Glib::ustring plain_text;
    Glib::ustring rich_text;
    Glib::RefPtr<Gdk::Pixbuf> pix_buf;
};

class CtClipboard
{
private:
    const Glib::ustring TARGET_CTD_PLAIN_TEXT = "UTF8_STRING";
    const Glib::ustring TARGET_CTD_RICH_TEXT = "CTD_RICH";
    const Glib::ustring TARGET_CTD_TABLE = "CTD_TABLE";
    const Glib::ustring TARGET_CTD_CODEBOX = "CTD_CODEBOX";
    const std::vector<Glib::ustring> TARGETS_HTML = {"text/html", "HTML Format"};
    const Glib::ustring TARGET_URI_LIST = "text/uri-list";
    const std::vector<Glib::ustring> TARGETS_PLAIN_TEXT = {"UTF8_STRING", "COMPOUND_TEXT", "STRING", "TEXT"};
    const std::vector<Glib::ustring> TARGETS_IMAGES = {"image/png", "image/jpeg", "image/bmp", "image/tiff", "image/x-MS-bmp", "image/x-bmp"};
    const Glib::ustring TARGET_WINDOWS_FILE_NAME = "FileName";

public:
    CtClipboard();

public:
    static void on_cut_clipboard(GtkTextView* pTextView, gpointer codebox);
    static void on_copy_clipboard(GtkTextView* pTextView, gpointer codebox);
    static void on_paste_clipboard(GtkTextView* pTextView, gpointer codebox);

public:
    void cut_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox);
    void copy_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox);
    void paste_clipboard(Gtk::TextView* pTextView, CtCodebox* pCodebox);

public:
    Glib::ustring rich_text_get_from_text_buffer_selection(CtTreeIter node_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                                           Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end,
                                                           gchar change_case = 'n', bool exclude_iter_sel_end = false);
    void from_xml_string_to_buffer(Glib::RefPtr<Gtk::TextBuffer> text_buffer, const Glib::ustring& xml_string);

private:
    void _rich_text_process_slot(xmlpp::Element* root, int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                 CtAnchoredWidget* obj_element, gchar change_case = 'n');
    void _dom_node_to_rich_text(Glib::RefPtr<Gtk::TextBuffer> text_buffer, xmlpp::Node* dom_node);

private:
    void _selection_to_clipboard(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextView* sourceview, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end, int num_chars, CtCodebox* pCodebox);

private:
    void _set_clipboard_data(const std::vector<std::string>& targets_list, CtClipboardData* clip_data);
    void _clip_data_get_signal(Gtk::SelectionData& selection_data, guint info, CtClipboardData* clip_data);
    void _clip_data_clear_signal(CtClipboardData* clip_data);

private:
    bool _force_plain_text;
};
