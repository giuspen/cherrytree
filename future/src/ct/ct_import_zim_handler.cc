/*
 * ct_import_zim_handler.cc
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
#include "ct_misc_utils.h"
#include "ct_const.h"

#include <iostream>

namespace fs = std::filesystem;

constexpr auto notebook_filename = "notebook.zim";




bool CtZimImportHandler::_has_notebook_file() {
    static const CtImportHandler::ImportFile notebook_file(notebook_filename, 0);

    for (const auto& file : _import_files) {
        if (file.path.filename().string() == notebook_filename) return true;
    }
    return false;
}

void CtZimImportHandler::add_file(const std::filesystem::path &path) {

}

void CtZimImportHandler::add_directory(const std::filesystem::path &dir_path) {
    
    _process_files(dir_path);
    if (!_has_notebook_file()) {
        throw CtImportException(fmt::format("Directory: {} does not contain a notebook.zim file", dir_path.string()));
    }
    
    _current_element = _xml_doc.create_root_node("root")->add_child("slot");
    _current_element = _current_element->add_child("rich_text");
    
    for (const auto& file : _import_files) {
        auto file_stream = file.file();
        
        std::string line;
        while(std::getline(file_stream, line, '\n')) {
            if (_parse_state == PARSING_STATE::HEAD) {
                // Creation-Date: .* is the final line of the header
                if (line.find("Creation-Date:") != std::string::npos) {
                    // TODO: Read the creation date and use it for ts_creation
                    _parse_state = PARSING_STATE::BODY;
                }
            } else if (_parse_state == PARSING_STATE::BODY) {
                _parse_body_line(line);
            }
        
        
        }
        
        
    }

}

const std::unordered_set<std::string>& CtZimImportHandler::_get_accepted_file_extensions() const {
    static std::unordered_set<std::string> extensions =  {".txt", ".zim"};
    return extensions;
}

void CtZimImportHandler::_parse_body_line(const std::string& line) {
    
    auto tokens = tokonise(line);
    auto &token_table = _get_token_map();
    
    
    if (tokens.empty()) {
        _add_text(line);
        _add_newline();
        return;
    }
    
    std::cout << "LINE: " << line << std::endl;
    auto iter = tokens.begin();
    while (iter != tokens.end()) {
        auto token_key = token_table.find(*iter);
        if (token_key != token_table.end()) {
            // Found a token
            std::cout << "TOKEN: " << *iter << std::endl;
            ++iter;
            token_key->second(*iter);
        }
        
        ++iter;
        if (iter == tokens.end()) break;
        ++iter;
    }
    


}

constexpr std::array<std::string_view, 6> tokens = {
        "**", "\t",  "* ", "//", "~~", "===="
};

const std::vector<std::string_view>& CtZimImportHandler::_get_tokens() const
{
    static std::vector<std::string_view> tokens_vect(tokens.begin(), tokens.end());
    return tokens_vect;
}



const CtImportHandler::token_map_t& CtZimImportHandler::_get_token_map() {
    static token_map_t map = {
            
            // Scale tags
        {"====", [this](const std::string &data) {
            auto count = 3;
            
            if (str::startswith(data, "=")) count--;
            if (*(data.begin() + 1) == '=') count--;
            
            auto str = str::replace(data, "= ", "");
            str = str::replace(str, " =", "");
            if (count < 2) {
                str = str::replace(data, "=", "");
            }
            
            _close_current_tag();
            _add_scale_tag(count, str);
            _add_newline();
            _close_current_tag();
        }},
        {"\t", [this](const std::string& data) {
            _list_level++;
            // Did a double match for even number of \t tags
            if (data.empty()) _list_level++;
        }},
        
        {"* ", [this](const std::string& data) {
            _add_list(_list_level, data);
            _list_level = 0;
            _add_newline();
        }},
    
        {"**", [this](const std::string& data){
            _close_current_tag();
            _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
            _close_current_tag();
        }},
        {"//", [this](const std::string& data) {
            _close_current_tag();
            _add_italic_tag(data);
            _close_current_tag();
        }}
    };
    
    
    return map;
}





