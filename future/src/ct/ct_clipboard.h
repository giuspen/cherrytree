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

class CtClipboard
{
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
    Glib::ustring rich_text_get_from_text_buffer_selection(CtTreeIter node_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end,
                                                     gchar change_case = 'n', bool exclude_iter_sel_end = false);
    void from_xml_string_to_buffer(Glib::RefPtr<Gtk::TextBuffer> text_buffer, const Glib::ustring& xml_string);

private:
    void _rich_text_process_slot(xmlpp::Element* root, int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                 CtAnchoredWidget* obj_element, gchar change_case = 'n');
    void _dom_node_to_rich_text(Glib::RefPtr<Gtk::TextBuffer> text_buffer, xmlpp::Node* dom_node);

private:
    void _selection_to_clipboard(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextView* sourceview, Gtk::TextIter iter_sel_start, Gtk::TextIter iter_sel_end, int num_chars, CtCodebox* pCodebox);

private:
    bool _force_plain_text;
};
