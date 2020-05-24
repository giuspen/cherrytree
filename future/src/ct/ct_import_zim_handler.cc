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



std::vector<std::shared_ptr<CtImportFile>> CtZimImportHandler::_get_files(const fs::path &path, uint32_t current_depth, CtImportFile* parent) {
    std::vector<fs::path> second_dirs;
    std::vector<std::shared_ptr<CtImportFile>> ret_files;
    auto& accepted_extensions = _get_accepted_file_extensions();
    
    for (const auto& dir_entry : fs::directory_iterator(path)) {
        auto fpath = dir_entry.path();
        
        if (accepted_extensions.find(fpath.extension().string()) != accepted_extensions.end()) {
            std::shared_ptr file = CtImportHandler::_new_import_file(fpath, current_depth);
            file->parent = parent;
            if (parent) {
                parent->children.emplace_back(file);
            }
            
            auto sub_path = fpath.parent_path() / fpath.stem();
            if (fs::is_directory(sub_path) /* This also checks if it exists */) {
                auto files = _get_files(sub_path, current_depth + 1, file.get());

                ret_files.insert(ret_files.end(), files.begin(), files.end());
            }
        
            ret_files.emplace_back(file);

        }
    }
    
    return ret_files;
}

void CtZimImportHandler::_process_files(const std::filesystem::path &path) {
    _import_files = _get_files(path, 0, nullptr);
}

bool CtZimImportHandler::_has_notebook_file() {
    static const auto notebook_file = _new_import_file(notebook_filename, 0);

    for (auto file_iter = _import_files.begin(); file_iter != _import_files.end(); ++file_iter) {
        if ((*file_iter)->path.filename().string() == notebook_filename) {
            _import_files.erase(file_iter, file_iter + 1); // Remove the notebook file from the files to be processed
            return true;
        }
    }
    return false;
}

void CtZimImportHandler::add_directory(const fs::path& path) {
    _process_files(path);
    if (!_has_notebook_file()) {
        throw CtImportException(fmt::format("Directory: {} does not contain a notebook.zim file", path.string()));
    }
    
    for (const auto& file : _import_files) {
        auto file_stream = file->file();
        feed(file_stream);
    }
    
}

void CtZimImportHandler::feed(std::istream& data) {
    
   
    _docs.emplace_back(std::make_shared<xmlpp::Document>());
    _import_files.at(_docs.size() - 1)->doc = _xml_doc();
    _current_element = _xml_doc()->create_root_node("root")->add_child("slot");
    _current_element = _current_element->add_child("rich_text");
    
    
    std::string line;
    while(std::getline(data, line, '\n')) {
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
    // Reset
    _parse_state = PARSING_STATE::HEAD;
    

}

const std::unordered_set<std::string>& CtZimImportHandler::_get_accepted_file_extensions() const {
    static std::unordered_set<std::string> extensions =  {".txt", ".zim"};
    return extensions;
}

void CtZimImportHandler::_parse_body_line(const std::string& line) {
    
    auto tokens = tokonise(line);
    
    for (const auto& token : tokens) {
        if (token.first) {
            token.first->action(token.second);
        } else {
            _add_text(token.second);
        }
        
    }
    _add_newline();
    


}



const std::vector<CtImportHandler::token_schema>& CtZimImportHandler::_get_tokens()
{
    static std::vector<token_schema> tokens_vect = {
        {"**", true, true, [this](const std::string& data){
            _close_current_tag();
            _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
            _close_current_tag();
        }},
        {"\t", false, false, [this](const std::string& data) {
            _list_level++;
            // Did a double match for even number of \t tags
            if (data.empty()) _list_level++;
        }},
        // TODO: Add links
        {"https://", false, false, [this](const std::string& data) {
            _add_text("https://"+data);
        }},
        {"http://", false, false, [this](const std::string& data) {
            _add_text("http://"+data);
        }},
        
        {"* ", false, false, [this](const std::string& data) {
            _add_list(_list_level, data);
            _list_level = 0;
        }},
        {"//", true, true, [this](const std::string& data) {
            _close_current_tag();
            _add_italic_tag(data);
            _close_current_tag();
        }},
        {"~~", true, true, [this](const std::string& data){
            _close_current_tag();
            _add_strikethrough_tag(data);
            _close_current_tag();
        }},
        {"====", false, false, [this](const std::string &data) {
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
                _close_current_tag();
            }},
            {"{{", true, false, [this](const std::string& data) {
                std::cout << "GOT LINK: " << data << std::endl;
            },"}}"}
    };
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
        }},
        {"{{", [this](const std::string& data) {
            std::cout << "GOT LINK: " << data << std::endl;
        }}
    };
    
    
    return map;
}





