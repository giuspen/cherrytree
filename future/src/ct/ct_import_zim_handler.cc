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

namespace fs = std::filesystem;

constexpr auto notebook_filename = "notebook.zim";



static const std::unordered_map<std::string_view, std::vector<std::string>> tags_mapping = {
        {"======", {R"(scale="h1")"}}
};


bool CtZimImportHandler::_has_notebook_file() {
    static const CtImportHandler::ImportFile notebook_file(notebook_filename, 0);
    
    return vec::exists(_import_files, notebook_file);
}

void CtZimImportHandler::add_file(const std::filesystem::path &path) {




}

void CtZimImportHandler::add_directory(const std::filesystem::path &dir_path) {
    
    _process_files(dir_path);
    if (!_has_notebook_file()) {
        throw CtImportException(fmt::format("Directory: {} does not contain a notebook.zim file", dir_path.string()));
    }
    
    for (const auto& file : _import_files) {
        auto file_stream = file.file();
        
        std::string line;
        while(std::getline(file_stream, line, '\n')) {
            if (_parser_state == PARSER_STATE::HEAD) {
                // Creation-Date: .* is the final line of the header
                if (line.find("Creation-Date:") != line.end()) {
                    // TODO: Read the creation date and use it for ts_creation
                    _parser_state = PARSER_STATE::BODY;
                }
            }
        
        
        }
        
        
    }

}

void CtZimImportHandler::_parse_body_line(const std::string& line) {

    
    auto space_pos = line.find_first_of(' ');
    
    std::string parse_str(line.begin(), line.begin() + space_pos);
    
    auto props = tags_mapping.find(parse_str);
    if (props != tags_mapping.end()) {
        _current_element = _current_element->
    }


}





