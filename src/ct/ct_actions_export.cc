/*
 * ct_actions_export.cc
 *
 * Copyright 2009-2023
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

#include "ct_actions.h"
#include "ct_export2pdf.h"
#include "ct_export2html.h"
#include "ct_export2txt.h"
#include "ct_storage_control.h"
#include <glib/gstdio.h>
#include "ct_logging.h"

// Print Page Setup Operations
void CtActions::export_print_page_setup()
{
    _pCtMainWin->get_ct_print().run_page_setup_dialog(_pCtMainWin);
}

void CtActions::export_print()
{
    _export_print(false, "", false);
}

void CtActions::export_to_pdf()
{
    _export_print(true, "", false);
}

void CtActions::export_to_html()
{
    _export_to_html("", false);
}

void CtActions::export_to_txt()
{
    _export_to_txt("", false);
}

void CtActions::export_to_ct()
{
    if (not _is_there_selected_node_or_error()) {
        return;
    }
    const CtExporting export_type = CtDialogs::selnode_selnodeandsub_alltree_dialog(
        *_pCtMainWin,
        true/*also_selection*/,
        nullptr/*include_node_name*/,
        nullptr/*new_node_page*/,
        nullptr/*last_index_in_page*/,
        nullptr/*last_single_file*/
    );
    if (CtExporting::NONESAVE == export_type) {
        return;
    }
    int start_offset{0};
    int end_offset{-1};
    if (CtExporting::SELECTED_TEXT == export_type) {
        if (not _is_there_text_selection_or_error()) {
            return;
        }
        Gtk::TextIter iter_sel_start, iter_sel_end;
        _curr_buffer()->get_selection_bounds(iter_sel_start, iter_sel_end);
        start_offset = iter_sel_start.get_offset();
        end_offset = iter_sel_end.get_offset();
    }
    const fs::path currDocFilepath = _pCtMainWin->get_ct_storage()->get_file_path();
    CtDialogs::CtStorageSelectArgs storageSelArgs{};
    if (not currDocFilepath.empty()) {
        storageSelArgs.ctDocType = fs::get_doc_type_from_file_ext(currDocFilepath);
        storageSelArgs.ctDocEncrypt = fs::get_doc_encrypt_from_file_ext(currDocFilepath);
    }
    if (not CtDialogs::choose_data_storage_dialog(_pCtMainWin, storageSelArgs)) {
        return;
    }
    const std::string fileExtension = CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
    std::string proposed_name;
    if (CtExporting::ALL_TREE == export_type) {
        proposed_name = currDocFilepath.string();
        CtMiscUtil::filepath_extension_fix(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt, proposed_name);
    }
    else {
        proposed_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter()) + fileExtension;
    }
    CtDialogs::CtFileSelectArgs fileSelArgs{};
    fileSelArgs.curr_file_name = proposed_name;
    if (not currDocFilepath.empty()) {
        fileSelArgs.curr_folder = currDocFilepath.parent_path();
    }
    std::string new_filepath;
    if (CtDocType::MultiFile == storageSelArgs.ctDocType) {
        new_filepath = CtDialogs::folder_save_as_dialog(_pCtMainWin, fileSelArgs);
    }
    else {
        fileSelArgs.filter_name = _("CherryTree File");
        fileSelArgs.filter_pattern.push_back(std::string{CtConst::CHAR_STAR}+fileExtension);
        new_filepath = CtDialogs::file_save_as_dialog(_pCtMainWin, fileSelArgs);
    }
    if (new_filepath.empty()) {
        return;
    }
    CtMiscUtil::filepath_extension_fix(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt, new_filepath);
    Glib::ustring error;
    std::unique_ptr<CtStorageControl> new_storage{
        CtStorageControl::save_as(_pCtMainWin,
                                  new_filepath,
                                  storageSelArgs.ctDocType,
                                  storageSelArgs.password,
                                  error,
                                  export_type,
                                  start_offset,
                                  end_offset)};
    if (not new_storage) {
        CtDialogs::error_dialog(str::xml_escape(error), *_pCtMainWin);
    }
}

void CtActions::export_to_pdf_auto(const std::string& dir, bool overwrite)
{
    spdlog::debug("pdf export to: {}", dir);
    spdlog::debug("overwrite: {}", overwrite);
    _export_print(true, dir, overwrite);
}

void CtActions::export_to_html_auto(const std::string& dir, bool overwrite, bool single_file)
{
    spdlog::debug("html export to: {}", dir);
    spdlog::debug("overwrite: {} single_file: {}", overwrite, single_file);
    _export_options.single_file = single_file;
    _export_to_html(dir, overwrite);
}

void CtActions::export_to_txt_auto(const std::string& dir, bool overwrite, bool single_file)
{
    spdlog::debug("txt export to: {}", dir);
    spdlog::debug("overwrite: {} single_file: {}", overwrite, single_file);
    _export_options.single_file = single_file;
    _export_to_txt(dir, overwrite);
}

void CtActions::_export_print(bool save_to_pdf, const fs::path& auto_path, bool auto_overwrite)
{
    CtExporting export_type;
    if (!auto_path.empty())
    {
        export_type = CtExporting::ALL_TREE;
    }
    else
    {
        if (!_is_there_selected_node_or_error()) return;
        export_type = CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name,
                                                                      &_export_options.new_node_page, nullptr, nullptr);
    }
    if (export_type == CtExporting::NONESAVE) return;

    fs::path pdf_filepath;
    if (export_type == CtExporting::CURRENT_NODE)
    {
        if (save_to_pdf)
        {
            pdf_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
            pdf_filepath = _get_pdf_filepath(pdf_filepath);
            if (pdf_filepath.empty()) return;
        }
        CtExport2Pdf(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options, -1, -1);
    }
    else if (export_type == CtExporting::CURRENT_NODE_AND_SUBNODES)
    {
        if (save_to_pdf)
        {
            pdf_filepath = _get_pdf_filepath(_pCtMainWin->get_ct_storage()->get_file_name());
            if (pdf_filepath == "") return;
        }
        CtExport2Pdf(_pCtMainWin).node_and_subnodes_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options);
    }
    else if (export_type == CtExporting::ALL_TREE)
    {
        if (!auto_path.empty())
        {
            pdf_filepath = auto_path / (_pCtMainWin->get_ct_storage()->get_file_name().string() + ".pdf");
            if (!auto_overwrite && fs::is_regular_file(pdf_filepath)) {
                spdlog::info("pdf exists and overwrite is off, export is stopped"); 
                return;
            }
        }
        else if (save_to_pdf)
        {
            pdf_filepath = _get_pdf_filepath(_pCtMainWin->get_ct_storage()->get_file_name());
            if (pdf_filepath.empty()) return;
        }
        CtExport2Pdf(_pCtMainWin).tree_export_print(pdf_filepath, _pCtMainWin->get_tree_store().get_ct_iter_first(), _export_options);
    }
    else if (export_type == CtExporting::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        if (save_to_pdf)
        {
            pdf_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
            pdf_filepath = _get_pdf_filepath(pdf_filepath);
            if (pdf_filepath == "") return;
        }
        CtExport2Pdf(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options, iter_start.get_offset(), iter_end.get_offset());
    }
}

// Export to HTML
void CtActions::_export_to_html(const fs::path& auto_path, bool auto_overwrite)
{
    CtExporting export_type;
    if (!auto_path.empty())
    {
        export_type = CtExporting::ALL_TREE;
    }
    else
    {
        if (!_is_there_selected_node_or_error()) return;
        export_type = CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name,
                                                                      nullptr, &_export_options.index_in_page, &_export_options.single_file);
    }
    if (export_type == CtExporting::NONESAVE) return;

    CtExport2Html export2html(_pCtMainWin);
    fs::path ret_html_path;
    if (export_type == CtExporting::CURRENT_NODE)
    {
        std::string folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false, ret_html_path))
            export2html.node_export_to_html(_pCtMainWin->curr_tree_iter(), _export_options, "", -1, -1);
    }
    else if (export_type == CtExporting::CURRENT_NODE_AND_SUBNODES)
    {
        std::string folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false, ret_html_path)) {
            if (_export_options.single_file) {
                export2html.nodes_all_export_to_single_html(false, _export_options);
            } else {
                export2html.nodes_all_export_to_multiple_html(false, _export_options);
            }
        }
    }
    else if (export_type == CtExporting::ALL_TREE)
    {
        fs::path folder_name = _pCtMainWin->get_ct_storage()->get_file_name();
        if (export2html.prepare_html_folder(auto_path, folder_name, auto_overwrite, ret_html_path)) {
            if (_export_options.single_file) {
                export2html.nodes_all_export_to_single_html(true, _export_options);
            } else {
                export2html.nodes_all_export_to_multiple_html(true, _export_options);
            }
        }
    }
    else if (export_type == CtExporting::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        std::string folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false, ret_html_path))
            export2html.node_export_to_html(_pCtMainWin->curr_tree_iter(), _export_options, "", iter_start.get_offset(), iter_end.get_offset());
    }
    if (!ret_html_path.empty() && auto_path.empty()) {
       fs::open_folderpath(ret_html_path, _pCtConfig);
    }
}

// Export To Plain Text Multiple (or single) Files
void CtActions::_export_to_txt(const fs::path& auto_path, bool auto_overwrite)
{
    CtExporting export_type;
    if (!auto_path.empty())
    {
        _export_options.include_node_name = true;
        export_type = CtExporting::ALL_TREE;
    }
    else
    {
        if (!_is_there_selected_node_or_error()) return;
        export_type = CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name, nullptr, nullptr, &_export_options.single_file);
    }
    if (export_type == CtExporting::NONESAVE) return;

    if (export_type == CtExporting::CURRENT_NODE)
    {
        fs::path txt_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        txt_filepath = _get_txt_filepath("", txt_filepath);
        if (txt_filepath.empty()) return;
        CtExport2Txt(_pCtMainWin).node_export_to_txt(_pCtMainWin->curr_tree_iter(), txt_filepath, _export_options, -1, -1);
    }
    else if (export_type == CtExporting::CURRENT_NODE_AND_SUBNODES)
    {
        if (_export_options.single_file)
        {
           fs::path txt_filepath = _get_txt_filepath("", _pCtMainWin->get_ct_storage()->get_file_name());
           if (txt_filepath.empty()) return;
           CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(false, "", txt_filepath, _export_options);
        }
        else
        {
            fs::path folder_path = _get_txt_folder("", CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter()), false);
            if (folder_path.empty()) return;
            CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(false, folder_path, "", _export_options);
        }
    }
    else if (export_type == CtExporting::ALL_TREE)
    {
        if (_export_options.single_file)
        {
            fs::path txt_filepath = _get_txt_filepath(auto_path, _pCtMainWin->get_ct_storage()->get_file_name());
            if (txt_filepath.empty()) return;
            CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(true, "", txt_filepath, _export_options);
        }
        else
        {
            auto folder_path = _get_txt_folder(auto_path, _pCtMainWin->get_ct_storage()->get_file_name(), auto_overwrite);
            if (folder_path.empty()) return;
            CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(true, folder_path, "", _export_options);
        }
    }
    else if (export_type == CtExporting::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        fs::path txt_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        txt_filepath = _get_txt_filepath("", txt_filepath);
        if (txt_filepath.empty()) return;
        CtExport2Txt(_pCtMainWin).node_export_to_txt(_pCtMainWin->curr_tree_iter(), txt_filepath, _export_options, iter_start.get_offset(), iter_end.get_offset());
    }
}

fs::path CtActions::_get_pdf_filepath(const fs::path& proposed_name)
{
    CtDialogs::CtFileSelectArgs args{};
    args.curr_folder = _pCtConfig->pickDirExport;
    args.curr_file_name = proposed_name.string() + ".pdf";
    args.filter_name = _("PDF File");
    args.filter_pattern = {"*.pdf"};

    fs::path filename = CtDialogs::file_save_as_dialog(_pCtMainWin, args);
    if (!filename.empty())
    {
        if (filename.extension() != ".pdf") filename += ".pdf";
        _pCtConfig->pickDirExport = filename.parent_path().string();
    }
    return filename;
}

// Prepare for the txt file save
fs::path CtActions::_get_txt_filepath(const fs::path& dir_place, const fs::path& proposed_name)
{
    fs::path filename;
    if (dir_place.empty())
    {
        CtDialogs::CtFileSelectArgs args{};
        args.curr_folder = _pCtConfig->pickDirExport;
        args.curr_file_name = proposed_name.string() + ".txt";
        args.filter_name = _("Plain Text Document");
        args.filter_pattern = {"*.txt"};

        filename = CtDialogs::file_save_as_dialog(_pCtMainWin, args);
    }
    else
    {
        filename = dir_place / proposed_name.string();
    }

    if (!filename.empty())
    {
        if (filename.extension() != ".txt") filename += ".txt";
        _pCtConfig->pickDirExport = filename.parent_path().string();

        if (fs::is_regular_file(filename)) fs::remove(filename);
    }
    return filename;
}

fs::path CtActions::_get_txt_folder(fs::path dir_place, fs::path new_folder, bool export_overwrite)
{
    if (dir_place.empty())
    {
        dir_place = CtDialogs::folder_select_dialog(_pCtMainWin, _pCtConfig->pickDirExport);
        if (dir_place.empty())
            return "";
    }
    new_folder = CtMiscUtil::clean_from_chars_not_for_filename(new_folder.string()) + "_TXT";
    new_folder = fs::prepare_export_folder(dir_place, new_folder, export_overwrite);
    fs::path export_dir = dir_place / new_folder;
    g_mkdir_with_parents(export_dir.c_str(), 0777);

    return export_dir;
}
