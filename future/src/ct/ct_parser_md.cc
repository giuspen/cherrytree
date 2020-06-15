/*
 * ct_handler_md.cc
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
#include "ct_misc_utils.h"
#include "ct_logging.h"
#include "ct_config.h"

void CtMDParser::_add_scale_to_last(int level) {
    xmlpp::Element* curr_tmp = _current_element;
    if (_last_element) _current_element = _last_element;

    _add_scale_tag(level, std::nullopt);
    _current_element = curr_tmp;
}

void CtMDParser::_init_tokens()
{
    if (_token_schemas.empty()) {
        auto add_codebox = [this](const std::string& data) {
            auto data_iter = data.begin();
            while (data_iter != data.end() && *data_iter != '\n') {
                ++data_iter;
            }
            std::string text(data_iter, data.end());
            std::string lang;
            if (data_iter != data.begin()) {
                lang.append(data.begin(), data_iter);

            }
            spdlog::debug("CODEBOX: {}, lang: {}", text, lang);
            _add_codebox(lang, text);
            

        };
        auto add_h3 = [this](const std::string& text) {
            _close_current_tag();
            _add_scale_tag(3, text + "\n");
        };
        _token_schemas = {
                // Bold
                {"__", true,  true,  [this](const std::string &data) {
                    _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
                }},
                // Italic
                {"*", true, true, [this](const std::string& data){
                    _add_italic_tag(data);
                }},
                // Bold and italic
                {"***", true, true, [this](const std::string& data){
                    _add_italic_tag(data);
                    _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, std::nullopt);
                }},
                // Bold
                {"**", true,  true,  [this](const std::string &data) {
                    _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
                }},
                // First part of a link
                {"[",  true,  false, [this](const std::string &data) {
                    _add_text(data, false);
                }, "]", true},
                // Second half of a link
                {"(",  true,  false, [this](const std::string &data) {
                    if (_last_encountered_token) {
                        if ((_last_encountered_token->open_tag == "[") && (_last_encountered_token->close_tag == "]")) {
                            _add_link(data);
                            return;
                        } else if ((_last_encountered_token->open_tag == "![") && (_last_encountered_token->close_tag == "]")) {
                            // Image link
                            _add_image(data);
                            return;
                        }
                    }
                    // Just text in brackets
                    _add_text("(" + data + ")");
                }, ")", true},
                // Monospace
                {"`", true, true, [this](const std::string& data){
                    _add_monospace_tag(data);
                }},
                // Footnote
                {"[^", true, false, [this](const std::string& data){
                    // Todo: Implement footnotes
                    _add_text("[^" + data + "]");
                }, "]"},
                // Codebox(s)
                {"~~~", true, true, add_codebox, "~~~", true},
                {"```", true, true, add_codebox, "```", true},
                // List
                {"* ", true, false, [this](const std::string &data) {
                    _add_list(_list_level, data + "\n");
                    _list_level = 0;
                }, "\n"},
                // Also list
                {"- ", true, false, [this](const std::string& data){
                    _add_list(_list_level, data + "\n");
                    _list_level = 0;
                }, "\n"},
                // Strikethrough
                {"~~", true,  true,  [this](const std::string &data) {
                    _add_strikethrough_tag(data);
                }},
                // Headers (h1, h2, etc)
                {"# ",  true, false, [this](const std::string &data) {
                    _close_current_tag();
                    _add_scale_tag(1, data + "\n");
                }, "\n"},
                {"## ",  true, false, [this](const std::string &data) {
                    _close_current_tag();
                    _add_scale_tag(2, data + "\n");
                }, "\n"},
                {"### ",  true, false,  add_h3, "\n"},
                {"#### ",  true, false,  add_h3, "\n"},
                {"##### ",  true, false,  add_h3, "\n"},
                {"###### ",  true, false,  add_h3, "\n"},
                
                // H1
                {"\n==", true, false, [this](const std::string&){
                    _add_scale_to_last(1);
                    _add_newline();
                }, "\n"},
                // H2
                {"\n----", true, false, [this](const std::string&){
                    _add_scale_to_last(2);
                    _add_newline();
                }, "\n"},
                // Horizontal divider
                {"***\n", true, false, [this](const std::string&){
                    _add_text(_pCtConfig->hRule + "\n", true);
                    _add_newline();
                }, " "},
                // Tables
                
                // Table row
                {"| ", true, false, [this](const std::string& data){
                    spdlog::debug("Got row: {}", data);
                    _add_table_cell(data);
                }, " |\n"},
                // Table header divider
                {"| -", true, false, [](const std::string& data){
                    // Since cherrytree tables don't use headers, this is not needed
                    spdlog::debug("Got divider: {}", data);
                }, "- |\n"},
                // Image link
                {"![", true, false, [this](const std::string&){}, "]"}
        
        };
    }
}

void CtMDParser::_place_free_text() 
{
    std::string free_txt = _free_text.str();
    if (!free_txt.empty()) {
        auto iter = free_txt.crbegin();
        for (;iter != free_txt.crend(); ++iter) {
            if (*iter == '\n') break;
        }
        std::string last_line(iter.base(), free_txt.cend());
        std::string other_txt(free_txt.cbegin(), iter.base());
        _add_text(other_txt);
        _add_text(last_line); // This may be needed for headers

        std::ostringstream tmp_ss;
        _free_text.swap(tmp_ss);
    }
}

void CtMDParser::feed(std::istream& stream)
{
    if (!_current_element) _current_element = _document->create_root_node("root")->add_child("slot")->add_child("rich_text");
    _init_tokens();
    _build_token_maps();
    
    std::string line;
    std::ostringstream in_stream;
    in_stream << stream.rdbuf();
    
    // Feed the line
    try {
        auto tokens_raw = _tokenize(in_stream.str());
        auto tokens     = _parse_tokens(tokens_raw);
        
        for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
            if (iter->first) {
                _place_free_text();
                // This is needed for links with () in them
                if ((iter + 1) != tokens.end()) {
                    if (!(iter + 1)->first && ((iter + 1)->second == ")")) {
                        // Excess bracket from link
                        iter->first->action(iter->second + ")");
                        ++iter;
                        if ((iter + 1) != tokens.end()) ++iter;
    
                        continue;
                    }
                }
                
                iter->first->action(iter->second);
            } else {
                if (!iter->second.empty()) {
                    if (!_current_table.empty()) _pop_table();
                    _free_text.write(iter->second.c_str(), iter->second.size());
                }
            }
            _last_encountered_token = iter->first;
        }
        _place_free_text();
    } catch (std::exception& e) {
        spdlog::error("Exception while parsing line: '{}': {}", line, e.what());
    }
    
}

void CtMDParser::_add_table_cell(std::string text) 
{
    if (!text.empty()) {
        // Parse it to see if a cell or end of row
        char last_ch = text.back();
        if (last_ch == '|') {
            // End of row
            _current_table_row.emplace_back(text.begin(), text.end() - 1);
            _pop_table_row();
        } else {
            // Just a cell
            _current_table_row.emplace_back(text);
        }
    } else {
        spdlog::warn("_add_table_cell called without text, the document may contain invalid or unknown formatting");
    }
}

void CtMDParser::_pop_table() 
{
    _add_table(_current_table);
    _current_table.clear();
}

void CtMDParser::_pop_table_row() 
{
    _current_table.emplace_back(_current_table_row);
    _current_table_row.clear();
}



