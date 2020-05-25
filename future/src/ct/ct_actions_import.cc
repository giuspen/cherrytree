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

#include <fstream>


CtNodeData setup_node(CtMainWin* pWin, const std::filesystem::path& path)
{
    CtNodeData nodeData;
    nodeData.isBold = false;
    nodeData.customIconId = 0;
    nodeData.isRO = false;
    nodeData.rTextBuffer = pWin->get_new_text_buffer();
    nodeData.name = path.stem().string();
    return nodeData;
}

// Ask the user what file to import for the node
void CtActions::import_node_from_html_file() noexcept 
{
    try {
        if(!_is_there_selected_node_or_error()) return;

        CtDialogs::file_select_args args(_pCtMainWin);
        args.filter_mime = {"text/html"};
        auto filepath = CtDialogs::file_select_dialog(args);

        _import_node_from_html(filepath);
    } catch(std::exception& e) {
        std::cerr << "Exception caught while importing node from file: " << e.what() << "\n";
    }
}

// Import a node from a html file
void CtActions::_import_node_from_html(const std::filesystem::path& filepath) 
{
    if (!_is_there_selected_node_or_error()) return;
    auto nodeData = setup_node(_pCtMainWin, filepath);
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    
    CtHtml2Xml parser(_pCtMainWin);

    try {
        parser.add_file(filepath);
        
        CtClipboard(_pCtMainWin).from_xml_string_to_buffer(nodeData.rTextBuffer, parser.to_string());
        auto iter = _pCtMainWin->curr_tree_iter();
        std::shared_ptr<CtNodeState> node_state;
        _node_add_with_data(iter, nodeData, false, node_state);
    } catch(std::exception& e) {
        std::cerr << "Exception caught while parsing the document: " << e.what() << "\n";
    }
}
// Import a directory of html files - non recursive
void CtActions::import_node_from_html_directory() noexcept 
{
    namespace fs = std::filesystem;
    
    try {
        if(!_is_there_selected_node_or_error()) return;

        fs::path dirpath = CtDialogs::folder_select_dialog("", _pCtMainWin);

        for (const auto& file : fs::directory_iterator(dirpath)) {
            
            const auto& f_path = file.path();
            if (f_path.extension() == ".html" || f_path.extension() == ".htm") {
                _import_node_from_html(f_path);
            }

        }
    } catch(std::exception& e) {
        std::cerr << "Exception caught while importing nodes from directory: " << e.what() << "\n";
    }

}

void CtActions::_import_node_from_plaintext(const std::filesystem::path &filepath)
{
    
    if (!_is_there_selected_node_or_error()) return;
    auto nodeData = setup_node(_pCtMainWin, filepath);
    nodeData.syntax = CtConst::PLAIN_TEXT_ID;
    
    try {

        std::ifstream infile;
        infile.exceptions(std::ios_base::failbit);
        infile.open(filepath);
        
        std::ostringstream data;
        data << infile.rdbuf();
        nodeData.rTextBuffer->insert_at_cursor(data.str());
        
        auto                         iter = _pCtMainWin->curr_tree_iter();
        std::shared_ptr<CtNodeState> node_state;
        
        _node_add_with_data(iter, nodeData, false, node_state);
    }
    catch (std::exception &e) {
        std::cerr << "Exception caught while importing plaintext file (" << filepath.string() << "): " << e.what() << "\n";
    }
}

void CtActions::import_nodes_from_ct_file() noexcept
{
    try {
        if(!_is_there_selected_node_or_error()) return;
        CtDialogs::file_select_args args(_pCtMainWin);
        args.filter_pattern = CtConst::CT_FILE_EXTENSIONS_FILTER;
        
        auto fpath = CtDialogs::file_select_dialog(args);
        if (fpath.empty()) return; // No file selected
        
        // Add the nodes through the storage type
        _pCtMainWin->get_ct_storage()->add_nodes_from_storage(fpath);
        
    } catch(std::exception& e) {
        std::cerr << "Exception caught while importing node from CT file: " << e.what() << "\n";
    }
}

void CtActions::import_node_from_plaintext_file() noexcept
{
    try {
        if(!_is_there_selected_node_or_error()) return;

        CtDialogs::file_select_args args(_pCtMainWin);
        args.filter_mime = {"text/plain"};
        auto fpath = CtDialogs::file_select_dialog(args);
        
        _import_node_from_plaintext(fpath);
        
    } catch(std::exception& e) {
        std::cerr << "Exception caught while importing plaintext file: " << e.what();
    }
}


void CtActions::import_nodes_from_plaintext_directory() noexcept
{
    try {
        if(!_is_there_selected_node_or_error()) return;
        
        auto fdir = CtDialogs::folder_select_dialog("", _pCtMainWin);
        
        for (const auto& pair : std::filesystem::directory_iterator(fdir)) {
            auto& filepath = pair.path();
            
            if (CtMiscUtil::mime_type_contains(filepath.string(), "text/")) {
                _import_node_from_plaintext(filepath);
            }
        }
        
    } catch(std::exception& e) {
        std::cerr << "Exception caught while importing directory of plaintext files: " << e.what();
    }
}



