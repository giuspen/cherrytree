/*
 * ct_md_handler.cc
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



const std::vector<CtImportHandler::token_schema>& CtMDParser::_get_tokens() {
    static const std::vector<token_schema> tokens = {
        // Italic
        {"__", true, true, [this](const std::string& data){
            _add_italic_tag(data);
        }},
        // Bold
        {"**", true, true, [this](const std::string& data){
            _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
        }},
        // First part of a link
        {"[", true, false, [this](const std::string& data) mutable {
            _add_text(data, false);
        }, "]"},
        // Second half of a link
        {"(", true, false, [this](const std::string& data){
            _add_link(data);
        }, ")"}

    };

    return tokens;
}



void CtMDParser::feed(std::istream& stream) {
    if (!_current_element) _current_element = _document.create_root_node("root")->add_child("slot")->add_child("rich_text");
    
    
    std::string line;
    while(std::getline(stream, line, '\n')) {
        // Feed the line
        auto tokens = _tokenize(line);

        for (const auto& token : tokens) {
            if (token.first) {
                token.first->action(token.second);
            } else {
                if (!token.second.empty()) _add_text(token.second);
            }
        }
        _add_newline();
    }

}

const std::unordered_set<std::string>& CtMDImportHandler::_get_accepted_file_extensions() const {
    static std::unordered_set<std::string> extens = {
            ".md"
    };
    return extens;
}

void CtMDImportHandler::add_directory(const std::filesystem::path &path) {

}
