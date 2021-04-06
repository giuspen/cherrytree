/*
 * ct_export2html.h
 *
 * Copyright 2009-2021
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

#include "ct_table.h"
#include "ct_codebox.h"
#include "ct_image.h"
#include "ct_treestore.h"
#include "ct_dialogs.h" // CtExportOptions
#include "ct_misc_utils.h"

class CtExport2Html
{
private:
    const Glib::ustring HTML_HEADER = R"HTML(<!doctype html>
<html>
<head>
  <meta http-equiv="content-type" content="text/html; charset=utf-8">
  <title>%s</title>
  <meta name="generator" content="CherryTree">
  <link rel="stylesheet" href="res/styles3.css" type="text/css" />
</head>
<body>
)HTML"; // after <body> should not be any whitespaces
    const Glib::ustring HTML_FOOTER = R"HTML(
</body>
</html>
)HTML";

public:
    CtExport2Html(CtMainWin* pCtMainWin);

    void          node_export_to_html(CtTreeIter tree_iter, const CtExportOptions& options, const Glib::ustring& index, int sel_start, int sel_end);
    void          nodes_all_export_to_multiple_html(bool all_tree, const CtExportOptions& options);
    void          nodes_all_export_to_single_html(bool all_tree, const CtExportOptions& options);
    Glib::ustring selection_export_to_html(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter start_iter,
                                           Gtk::TextIter end_iter, const Glib::ustring& syntax_highlighting);
    Glib::ustring table_export_to_html(CtTable* table);
    Glib::ustring codebox_export_to_html(CtCodebox* codebox);
    bool          prepare_html_folder(fs::path dir_place, fs::path new_folder, bool export_overwrite, fs::path& export_path);

private:
    Glib::ustring _get_embfile_html(CtImageEmbFile* embfile, CtTreeIter tree_iter, fs::path embed_dir);
    Glib::ustring _get_image_html(CtImage* image, const fs::path& images_dir, int& images_count, CtTreeIter* tree_iter);
    Glib::ustring _get_codebox_html(CtCodebox* codebox);
    Glib::ustring _get_table_html(CtTable* table);

    Glib::ustring _html_get_from_code_buffer(const Glib::RefPtr<Gsv::Buffer>& code_buffer, int sel_start, int sel_end, const std::string &syntax_highlighting);
    void          _html_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end,
                                       std::vector<Glib::ustring>& out_slots, std::vector<CtAnchoredWidget*>& out_widgets);
    Glib::ustring _html_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer);
    Glib::ustring _html_text_serialize(Gtk::TextIter start_iter, Gtk::TextIter end_iter, const CtCurrAttributesMap& curr_attributes);
    std::string _get_href_from_link_prop_val(Glib::ustring link_prop_val);
    Glib::ustring _get_object_alignment_string(Glib::ustring alignment);

    void          _tree_links_text_iter(CtTreeIter tree_iter, Glib::ustring& tree_links_text, int tree_count_level, bool index_in_page);

    Glib::ustring _get_html_filename(CtTreeIter tree_iter);

public:
    static std::string _link_process_filepath(const std::string& filepath_raw, const std::string& relative_to);
    static std::string _link_process_folderpath(const std::string& folderpath_raw, const std::string& relative_to);

private:
    CtMainWin*    _pCtMainWin;
    fs::path _export_dir;
    fs::path _images_dir;
    fs::path _embed_dir;
    fs::path _res_dir;
};



/**
 * @brief Contains functions which provide an interface between a pandoc binary and cherrytree
 * @namespace CtPandoc
 */
namespace CtPandoc {

bool has_pandoc();

constexpr bool supports_syntax(std::string_view syntax) {
    constexpr const std::array<std::string_view, 2> supported_syntaxes = {"markdown", "latex"};
    for (const auto& supported_syntax : supported_syntaxes) {
        if (syntax == supported_syntax) {
            return true;
        }
    }
    return false;
}

void to_html(std::istream& input, std::ostream& output, std::string from_format);

void to_html(const fs::path& file, std::ostream& output);

} // CtPandoc
