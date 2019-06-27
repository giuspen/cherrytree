/*
 * ct_export2html.h
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

#include "ct_table.h"
#include "ct_codebox.h"
#include "ct_image.h"

class CtExport2Html
{
private:
    const Glib::ustring HTML_HEADER = R"HTML(<!doctype html><html>
    <head>
      <meta http-equiv="content-type" content="text/html; charset=utf-8">
      <title>%s</title>
      <meta name="generator" content="CherryTree">
      <link rel="stylesheet" href="styles.css" type="text/css" />
    </head>
    <body>)HTML";
    const Glib::ustring HTML_FOOTER = R"HTML(</body></html>)HTML";

public:
    CtExport2Html();

    Glib::ustring selection_export_to_html(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter start_iter,
                                           Gtk::TextIter end_iter, const Glib::ustring& syntax_highlighting);
    Glib::ustring table_export_to_html(CtTable* table);
    Glib::ustring codebox_export_to_html(CtCodebox* codebox);

private:
    Glib::ustring _get_image_html(CtImage* image, const Glib::ustring& images_dir, int& images_count, CtTreeIter* tree_iter);
    Glib::ustring _get_codebox_html(CtCodebox* codebox);
    Glib::ustring _get_table_html(CtTable* table);
    Glib::ustring _html_get_from_code_buffer(Glib::RefPtr<Gsv::Buffer> code_buffer, std::pair<int, int> sel_range);
    Glib::ustring _html_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer);
    Glib::ustring _html_text_serialize(Gtk::TextIter start_iter, Gtk::TextIter end_iter, const std::map<const gchar*, std::string>& curr_attributes);
    Glib::ustring _get_href_from_link_prop_val(Glib::ustring link_prop_val);
    Glib::ustring _link_process_filepath(const Glib::ustring& filepath_raw);
    Glib::ustring _link_process_folderpath(const Glib::ustring& folderpath_raw);
    Glib::ustring _get_html_filename(CtTreeIter tree_iter);
};

