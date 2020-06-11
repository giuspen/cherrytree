/*
 * ct_import_handler.cc
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
#include "ct_imports.h"
#include "ct_const.h"
#include "ct_config.h"
#include <fmt/fmt.h>

namespace fs = std::filesystem;



void CtImportFile::fix_internal_links(const std::string &node_name, uint64_t node_id) 
{
    auto iter = _internal_links.find(node_name);
    if (iter != _internal_links.end()) {
        for (auto* link : iter->second) {
            link->set_attribute(CtConst::TAG_LINK, fmt::format("node {}", node_id));
        }
    }
    
    
}


void CtImportHandler::_add_internal_link(const std::string& text)
{
    _add_internal_link_to_curr_file(text, _current_element);
    _add_text(text);
}

void CtImportHandler::_init_new_doc() {
    _docs.emplace_back(std::make_shared<xmlpp::Document>());
    _current_import_file()->doc = _xml_doc();
    _current_element = _xml_doc()->create_root_node("root")->add_child("slot");
    _current_element = _current_element->add_child("rich_text");
}

