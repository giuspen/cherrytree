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
#include <src/fmt/format.h>
#include <iostream>
#include "ct_imports.h"
#include "ct_const.h"
#include "ct_config.h"

namespace fs = std::filesystem;


void CtImportHandler::_process_files(const std::filesystem::path &path) {
    if (!_processed_files) {
        auto &accepted_extensions = _get_accepted_file_extensions();
        if (!fs::is_directory(path)) {
            if (accepted_extensions.find(path.extension().string()) != accepted_extensions.end()) _import_files.emplace_back(path);
        } else {
            fs::recursive_directory_iterator rec_dir_iter(path);
            for (const auto &dir_entry : rec_dir_iter) {
                auto &fpath = dir_entry.path();
            
                if (accepted_extensions.find(fpath.extension().string()) != accepted_extensions.end()) {
                    ImportFile file(fpath, rec_dir_iter.depth());
                    _import_files.emplace_back(file);
                }
            
            }
        
        
        }
        _processed_files = true;
    }
    
    
    
}
std::vector<CtImportHandler::token_schema> CtImportHandler::tokonise(const std::string& stream) const {
    // TODO: Optimise this with a hash map for tags
    
    auto& tokens = _get_tokens();
    std::vector<std::pair<CtImportHandler::token_schema, std::string>> token_stream;
    std::unordered_map<std::string_view, bool> open_tags;
    std::string buff;
    std::size_t pos = 0;
    token_schema curr_token;
    for (const auto& ch : stream) {
        buff += ch;
        pos++;
        for (const auto &token : tokens) {
            auto tag_open = false;
            if (token.is_symmetrical) {
                tag_open = open_tags[token.open_tag];
            }
            auto has_opentag = buff.find(token.open_tag) != std::string::npos;
            if (has_opentag && !tag_open) {
                // First token
                curr_token = token;
                
                if (!token.has_closetag) {
                    // Token will go till end of stream
                    token_stream.emplace_back(std::move(curr_token));
                    auto tokonised_stream = tokonise(token_stream.back());
                    token_stream.insert(token_stream.end(), tokonised_stream.begin(), tokonised_stream.end());
                    return token_stream;
                }
                open_tags[token.open_tag] = true;
                buff.resize(0);
                break;
            }
            auto has_closetag = has_opentag && token.is_symmetrical;
            if (!token.is_symmetrical) {
                if (token.close_tag.empty()) throw CtImportException("Token has no close tag");
                has_closetag = buff.find(token.close_tag) != std::string::npos;
            }
            if (has_closetag) {
                auto& tag = token.is_symmetrical ? token.open_tag : token.close_tag;
                curr_token.close_tag = tag;
                curr_token.data = std::string(stream.begin() + pos - buff.length(), stream.begin() + pos - tag.length());
                token_stream.emplace_back(std::move(curr_token));
    
                buff.resize(0);
                break;
            }
        }
    }
    if (!buff.empty()) {
        token_schema token = {"", false, false, [this](const std::string& data){
            _add_text(data);
        }};
        token.data = buff;
        token_stream.emplace_back(std::move(token));
    }
    
    return token_stream;
}

void CtImportHandler::_add_ordered_list(unsigned int level, const std::string &data) {
    _add_text(fmt::format("{}. {}", level, data));
}

void CtImportHandler::_add_list(uint8_t level, const std::string& data) {
    if (level >= _pCtConfig->charsListbul.length()) {
        if (_pCtConfig->charsListbul.empty()) {
            throw std::runtime_error("No bullet-list characters set");
        }
        level = _pCtConfig->charsListbul.length() - 1;
    }
    std::string indent;
    auto i_lvl = 0;
    while(i_lvl < level) {
        ++i_lvl;
        indent += CtConst::CHAR_TAB;
    }
    
    Glib::ustring list_data(1, _pCtConfig->charsListbul[level]);
    list_data += CtConst::CHAR_SPACE;
    _add_text(indent + list_data + data);
}
void CtImportHandler::_add_weight_tag(const Glib::ustring& level, std::optional<std::string> data) {
    _current_element->set_attribute("weight", level);
    if (data) {
        _add_text(*data);
    }
}

void CtImportHandler::_add_text(std::string text) {
    std::cout << "ADDED TEXT: " << text << std::endl;
    auto curr_text = _current_element->get_child_text();
    if (!curr_text) _current_element->set_child_text(std::move(text));
    else            curr_text->set_content(curr_text->get_content() + std::move(text));
}

void CtImportHandler::_close_current_tag() {
    _current_element = _current_element->get_parent()->add_child("rich_text");
}

void CtImportHandler::_add_newline() {
    _add_text(CtConst::CHAR_NEWLINE);
}

void CtImportHandler::_add_italic_tag(std::optional<std::string> data) {
    _current_element->set_attribute(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
    if (data) {
        _add_text(*data);
    }
}

void CtImportHandler::_add_scale_tag(int level, std::optional<std::string> data) {
    _current_element->set_attribute("scale", fmt::format("h{}", level));
    std::cout << "SCALE CALLED: " << level << std::endl;
    if (data) {
        _add_text(*data);
    }
}