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


// Ask the user what file to import for the node
void CtActions::import_node_from_html_file() noexcept {
    try {
        CtDialogs::file_select_args args(_pCtMainWin);
        args.filter_mime = {"text/html"};
        auto filepath = CtDialogs::file_select_dialog(args);

        _import_node_from_html(filepath);
    } catch(std::exception& e) {
        std::cerr << "Exception caught while importing node from file: " << e.what() << "\n";
    }
}

// Import a node from a html file
void CtActions::_import_node_from_html(const std::filesystem::path& filepath) {
    
    CtNodeData nodeData;
    std::shared_ptr<CtNodeState> node_state;
    
    if (!_is_there_selected_node_or_error()) return;
    nodeData.isBold = false;
    nodeData.customIconId = 0;
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    nodeData.isRO = false;
    nodeData.name = filepath.stem();
    
    nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer();


    CtHtml2Xml parser(_pCtMainWin);

    try {

        parser.add_file(filepath);
        
        CtClipboard(_pCtMainWin).from_xml_string_to_buffer(nodeData.rTextBuffer, parser.to_string());
        auto iter = _pCtMainWin->curr_tree_iter();
        _node_add_with_data(iter, nodeData, false, node_state);
    } catch(std::exception& e) {
        std::cerr << "Exception caught while parsing the document: " << e.what() << "\n";
    }
}

// Import a directory of html files - non recursive
void CtActions::import_node_from_html_directory() noexcept {
    namespace fs = std::filesystem;
    
    try {
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



