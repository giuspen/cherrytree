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
            spdlog::debug("CODEBOX: {}", data);
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
                    _add_list(0, data);
                }, "\n"},
                // Also list
                {"- ", true, false, [this](const std::string& data){
                    _add_list(0, data);
                }, "\n"},
                // Strikethrough
                {"~~", true,  true,  [this](const std::string &data) {
                    _add_strikethrough_tag(data);
                }},
                // Headers (h1, h2, etc)
                {"#",  true, false, [this](const std::string &data) {
                    auto tag_num = 1;
                    auto iter    = data.begin();
                    while (*iter == '#') {
                        ++tag_num;
                        ++iter;
                    }
                
                
                    if (tag_num > 3) tag_num = 3; // Reset to 3 if too large
                
                    auto str = str::replace(data, "# ", "");
                    str = str::replace(str, "#", "");
                
                    // Remove the extra space if single front
                    if (tag_num == 1 && str.front() == ' ') {
                        str.replace(str.begin(), str.begin() + 1, "");
                    }
                    _close_current_tag();
                    _add_scale_tag(tag_num, str);
                }, "\n", true},
                // H1
                {"==", true, false, [this](const std::string&){
                    _add_scale_to_last(1);
                }, "\n"},
                // H2
                {"----", true, false, [this](const std::string&){
                    _add_scale_to_last(2);
                }, "\n"},
                // Horizontal divider
                {"---", true, false, [this](const std::string&){
                    _add_text(_pCtConfig->hRule, true);
                }, "\n"},
                {"***\n", true, false, [this](const std::string&){
                    _add_text(_pCtConfig->hRule, true);          
                }, " "}
        
        };
    }
}

void CtMDParser::_place_free_text() {
    std::string free_txt = _free_text.str();
    if (!free_txt.empty()) {
        _add_text(free_txt);
    }
    std::ostringstream tmp_ss;
    _free_text.swap(tmp_ss);
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
                    _free_text.write(iter->second.c_str(), iter->second.size());
                }
            }
            _last_encountered_token = iter->first;
        }
        _place_free_text();
        //if (!stream.eof()) _add_newline();
    } catch (std::exception& e) {
        spdlog::error("Exception while parsing line: '{}': {}", line, e.what());
    }
    
}


