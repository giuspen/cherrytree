/*
 * ct_actions_export.cc
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
#include <filesystem>


void CtActions::import_node_from_html_file() {
    
    CtDialogs::file_select_args args(_pCtMainWin);
    args.filter_mime = {"text/html"};
    auto filepath = CtDialogs::file_select_dialog(args);

    _import_node_from_html(filepath);
}


void CtActions::_import_node_from_html(const std::string& path) {
    
    std::filesystem::path fs_path(path);
    CtNodeData nodeData;
    std::shared_ptr<CtNodeState> node_state;
    
    if (!_is_there_selected_node_or_error()) return;
    nodeData.isBold = false;
    nodeData.customIconId = 0;
    nodeData.syntax = CtConst::RICH_TEXT_ID;
    nodeData.isRO = false;
    nodeData.name = fs_path.stem();
    
    nodeData.rTextBuffer = _pCtMainWin->get_new_text_buffer();


    CtHtml2Xml parser(_pCtMainWin);
    parser.add_file(path);

    CtClipboard(_pCtMainWin).from_xml_string_to_buffer(nodeData.rTextBuffer, parser.to_string());
    auto iter = _pCtMainWin->curr_tree_iter();
    _node_add_with_data(iter, nodeData, false, node_state);
}



