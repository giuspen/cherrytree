/*
 * ct_parser.cc
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
#include <src/fmt/format.h>
#include <iostream>
#include <memory>

void CtParser::wipe()  
{ 
    _document = std::make_unique<xmlpp::Document>();
    _current_element = nullptr;
}

void CtParser::_add_superscript_tag(std::optional<std::string> text)
{
    _current_element->set_attribute(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP);
    
    if (text) _add_text(*text);
}

void CtParser::_add_subscript_tag(std::optional<std::string> text)
{
    _current_element->set_attribute(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB);
    
    if (text) _add_text(*text);
}

void CtParser::_add_ordered_list(unsigned int level, const std::string &data)
{
    _add_text(fmt::format("{}. {}", level, data));
}


void CtParser::_add_todo_list(CHECKBOX_STATE state, const std::string& text)
{
    auto todo_index = static_cast<int>(state);
    _add_text(_pCtConfig->charsTodo[todo_index] + CtConst::CHAR_SPACE + text);
}

void CtParser::_add_list(uint8_t level, const std::string& data)
{
    if (level >= _pCtConfig->charsListbul.size()) {
    if (_pCtConfig->charsListbul.size() == 0) {
        throw std::runtime_error("No bullet-list characters set");
    }
        level = _pCtConfig->charsListbul.size() - 1;
    }
    std::string indent;
    auto i_lvl = 0;
    while(i_lvl < level) {
    ++i_lvl;
    indent += CtConst::CHAR_TAB;
}

_add_text(indent + _pCtConfig->charsListbul[level] + CtConst::CHAR_SPACE + data);
}
void CtParser::_add_weight_tag(const Glib::ustring& level, std::optional<std::string> data)
{
    _current_element->set_attribute("weight", level);
    if (data) {
        _add_text(*data, false);
    }
}

void CtParser::_add_link(const std::string& text)
{
    auto val = CtImports::get_internal_link_from_http_url(text);
    _current_element->set_attribute(CtConst::TAG_LINK, val);
}

void CtParser::_add_strikethrough_tag(std::optional<std::string> data)
{
    _current_element->set_attribute(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
    if (data) _add_tag_data(CtConst::TAG_STRIKETHROUGH, *data);
}

void CtParser::_add_text(std::string text, bool close_tag /* = true */)
{
    if (!text.empty()) {
        if (close_tag) _close_current_tag();
    
        auto curr_text = _current_element->get_child_text();
        if (!curr_text) _current_element->set_child_text(std::move(text));
        else curr_text->set_content(curr_text->get_content() + std::move(text));
    
        if (close_tag) _close_current_tag();
    }
}

void CtParser::_close_current_tag()
{
    if (!_tag_empty()) {
        _current_element = _current_element->get_parent()->add_child("rich_text");
        // Reset the tags
        _open_tags.clear();
    }
}

void CtParser::_add_newline() {
    // Add a newline, if tags are empty no need to close the current tag
    _add_text(CtConst::CHAR_NEWLINE, !_open_tags.empty());
}

void CtParser::_add_italic_tag(std::optional<std::string> data)
{
    _current_element->set_attribute(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
    
    if (data) _add_tag_data(CtConst::TAG_STYLE, *data);
}

void CtParser::_add_scale_tag(int level, std::optional<std::string> data)
{
    _current_element->set_attribute(CtConst::TAG_SCALE, fmt::format("h{}", level));
    
    if (data) _add_tag_data(CtConst::TAG_SCALE, *data);
}


void CtParser::_add_tag_data(std::string_view tag, std::string data) {
    bool do_close = _open_tags[tag];
    
    _add_text(std::move(data), do_close);
    _open_tags[tag] = true;
}


void CtParser::_build_token_maps() {
    if (_open_tokens_map.empty() || _close_tokens_map.empty()) {
        _init_tokens();
        
        // Build open tokens
        if (_open_tokens_map.empty()) {
            for (const auto& token : _token_schemas) {
                _open_tokens_map[token.open_tag] = &token;
            }
        }
        // Build close tokens
        if (_close_tokens_map.empty()) {
            for (const auto& token : _token_schemas) {
                if (token.has_closetag) {
                    if (token.is_symmetrical) _close_tokens_map[token.open_tag] = &token;
                    else                      _close_tokens_map[token.close_tag] = &token;
                }
            }
        }
    }
}

std::pair<Gtk::TextIter, Gtk::TextIter> CtParser::find_formatting_boundaries(const Gtk::TextIter& start_bounds, const Gtk::TextIter& word_end) {
    _build_token_maps();
    
    auto word_start = word_end;
    
    int tokens_open = 0;
    std::string buff;
    bool found_open_token = false;
    while (word_start != start_bounds) {
        if (word_start.inside_word()) {
            --word_start;
            continue;
        } else if (word_start.ends_word()) {
            buff.clear();
            --word_start;
            continue;
        }
        if (word_start.get_char() == ' ' && !found_open_token) {
            --word_start;
            continue;
        }
        buff += std::string(1, word_start.get_char());
        
        
        
        if (_close_tokens_map.find(buff) != _close_tokens_map.end()) {
            ++tokens_open;
            found_open_token = true;
        } else {
            auto open_iter = _open_tokens_map.find(buff);
            if (open_iter != _open_tokens_map.end()) {
                --tokens_open;
            }
        }
    
        if (word_start.get_char() == ' ' && (tokens_open == 0)) {
            break;
        }
        
        --word_start;
    }
    
    return {word_start, word_end};
}

