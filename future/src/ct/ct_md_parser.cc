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
#include "ct_misc_utils.h"


void CtMDParser::_init_tokens()
{
    if (_token_schemas.empty()) {
        _token_schemas = {
                // Italic
                {
                        "__", true,  true,  [this](const std::string &data) {
                    _add_italic_tag(data);
                }},
                // Bold
                {
                        "**", true,  true,  [this](const std::string &data) {
                    _add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
                }},
                // First part of a link
                {
                        "[",  true,  false, [this](const std::string &data) {
                    _add_text(data, false);
                    _in_link = true;
                }, "]", true
                },
                // Second half of a link
                {
                        "(",  true,  false, [this](const std::string &data) {
                    if (_in_link) {
                        _add_link(data);
                        _in_link = false;
                    } else {
                        // Just text in brackets
                        _add_text("(" + data + ")");
                    }
                }, ")", true
                },
                // List
                {
                        "* ", false, false, [this](const std::string &data) {
                    _add_list(0, data);
                }},
                // Strikethrough
                {
                        "~~", true,  true,  [this](const std::string &data) {
                    _add_strikethrough_tag(data);
                }},
                // Headers (h1, h2, etc)
                {
                        "#",  false, false, [this](const std::string &data) {
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
                
                    _add_scale_tag(tag_num, str);
                }, "#", true
                }
        
        };
    }
}



void CtMDParser::feed(std::istream& stream)
{
    if (!_current_element) _current_element = _document->create_root_node("root")->add_child("slot")->add_child("rich_text");
    _init_tokens();
    _build_token_maps();
    
    std::string line;
    while(std::getline(stream, line, '\n')) {
        // Feed the line
        auto tokens_raw = _tokenize(line);
        auto tokens = _parse_tokens(tokens_raw);

        for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
            if (iter->first) {
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
                if (!iter->second.empty()) _add_text(iter->second);
            }
        }
        _add_newline();
    }

}


