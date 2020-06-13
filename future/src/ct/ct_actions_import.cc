/*
 * ct_actions_import.cc
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
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
#include "ct_clipboard.h"
#include "ct_imports.h"
#include "ct_storage_control.h"
#include "ct_export2html.h"

#include "ct_logging.h"
#include <fstream>

namespace fs = std::filesystem;



static std::unique_ptr<ct_imported_node> traverse_dir(const std::string& dir, CtImporterInterface* importer)
{
    auto dir_node = std::make_unique<ct_imported_node>(dir, Glib::path_get_basename(dir));
    for (const auto& dir_item: fs::directory_iterator(dir))
    {
        if (fs::is_directory(dir_item))
            dir_node->children.emplace_back(traverse_dir(dir_item.path(), importer));
        else if (auto node = importer->import_file(dir_item.path()))
            dir_node->children.emplace_back(std::move(node));
    }
    return dir_node;
}


static void created_nodes(std::unique_ptr<ct_imported_node>& imported_node)
{

}

void CtActions::_import_from_file(CtImporterInterface* importer)
{
    CtDialogs::file_select_args args(_pCtMainWin);
    args.curr_folder = _pCtMainWin->get_ct_config()->pickDirImport;
    args.filter_mime = importer->file_mimes();
    auto filepath = CtDialogs::file_select_dialog(args);
    if (filepath.empty()) return;
    _pCtMainWin->get_ct_config()->pickDirImport = Glib::path_get_dirname(filepath);

    try
    {
       std::unique_ptr<ct_imported_node> node = importer->import_file(filepath);
       created_nodes(node);
    }
    catch (std::exception& ex)
    {
        spdlog::error("import exception: {}", ex.what());
    }
}

void CtActions::_import_from_dir(CtImporterInterface* importer, const std::string& custom_dir)
{
    std::string start_dir = custom_dir.empty() ? _pCtMainWin->get_ct_config()->pickDirImport : custom_dir;
    std::string import_dir = CtDialogs::folder_select_dialog(start_dir, _pCtMainWin);
    if (import_dir.empty()) return;
    if (custom_dir.empty())
        _pCtMainWin->get_ct_config()->pickDirImport = import_dir;

    try
    {
        auto dir_node = traverse_dir(import_dir, importer);
        created_nodes(dir_node);
    }
    catch (std::exception& ex)
    {
        spdlog::error("import exception: {}", ex.what());
    }
}


// Import a node from a html file
void CtActions::import_node_from_html_file() noexcept 
{
    CtHtmlImport importer(_pCtMainWin->get_ct_config());
    _import_from_file(&importer);
}

// Import a directory of html files - non recursive
void CtActions::import_node_from_html_directory() noexcept 
{
    CtHtmlImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, "");
}

void CtActions::import_nodes_from_ct_file() noexcept
{
//    CtCherrytreeImport importer;
//    _import_from_file(&importer);
}

void CtActions::import_node_from_plaintext_file() noexcept
{
//    CtPlainTextImport importer;
//    _import_from_file(&importer);
}

void CtActions::import_nodes_from_plaintext_directory() noexcept
{
//    CtPlainTextImport importer;
//    _import_from_dir(&importer, "");
}

void CtActions::import_node_from_md_file() noexcept
{
//    CtMDImport importer;
//    _import_from_file(&importer);
}

void CtActions::import_nodes_from_md_directory() noexcept
{
//    CtMDImport importer;
//    _import_from_dir(&importer, "");
}

void CtActions::import_nodes_from_zim_directory() noexcept
{
    CtZimImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, "");
}

void CtActions::import_node_from_pandoc() noexcept 
{
    if (!CtPandoc::has_pandoc()) {
        CtDialogs::warning_dialog(_("Pandoc executable could not be found, please ensure it is in your path"), *_pCtMainWin);
        return;
    }

//    CtPandocImport importer;
//    _import_from_file(&importer);
}

void CtActions::import_directory_from_pandoc() noexcept
{
    if (!CtPandoc::has_pandoc()) {
        CtDialogs::warning_dialog(_("Pandoc executable could not be found, please ensure it is in your path"), *_pCtMainWin);
        return;
    }

//    CtPandocImport importer;
//    _import_from_dir(&importer, "");
};

void CtActions::import_nodes_from_gnote_directory() noexcept
{
    CtTomboyImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, Glib::build_filename(g_get_user_data_dir(), "gnote"));
}

void CtActions::import_nodes_from_tomboy_directory() noexcept
{
    CtTomboyImport importer(_pCtMainWin->get_ct_config());
    _import_from_dir(&importer, Glib::build_filename(g_get_user_data_dir(), "tomboy"));
}

