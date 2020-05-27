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
#include "ct_imports.h"
#include "ct_const.h"
#include "ct_config.h"

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

void CtImportHandler::_add_superscript_tag(std::optional<std::string> text) 
{
    _current_element->set_attribute(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP);
    
    if (text) _add_text(*text);
}

void CtImportHandler::_add_subscript_tag(std::optional<std::string> text) 
{
    _current_element->set_attribute(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB);
    
    if (text) _add_text(*text);
}

void CtImportHandler::_add_ordered_list(unsigned int level, const std::string &data) 
{
    _add_text(fmt::format("{}. {}", level, data));
}

void CtImportHandler::_add_internal_link(const std::string& text) 
{
    _add_internal_link_to_curr_file(text, _current_element);
    _add_text(text);
}

void CtImportHandler::_add_todo_list(CHECKBOX_STATE state, const std::string& text) 
{
    auto todo_index = static_cast<int>(state);
    _add_text(_pCtConfig->charsTodo[todo_index] + CtConst::CHAR_SPACE + text);
}

void CtImportHandler::_add_list(uint8_t level, const std::string& data) 
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
void CtImportHandler::_add_weight_tag(const Glib::ustring& level, std::optional<std::string> data) 
{
    _current_element->set_attribute("weight", level);
    if (data) {
        _add_text(*data);
    }
}

void CtImportHandler::_add_link(const std::string& text) 
{
    auto val = CtImports::get_internal_link_from_http_url(text);
    _current_element->set_attribute(CtConst::TAG_LINK, val);
    _add_text(text);
}

void CtImportHandler::_add_strikethrough_tag(std::optional<std::string> data) 
{
    _current_element->set_attribute(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
    if (data) _add_text(*data);
}

void CtImportHandler::_add_text(std::string text) 
{
    auto curr_text = _current_element->get_child_text();
    if (!curr_text) _current_element->set_child_text(std::move(text));
    else            curr_text->set_content(curr_text->get_content() + std::move(text));
}

void CtImportHandler::_close_current_tag() 
{
    if (!_tag_empty()) _current_element = _current_element->get_parent()->add_child("rich_text");
}

void CtImportHandler::_add_newline() {
    _add_text(CtConst::CHAR_NEWLINE);
}

void CtImportHandler::_add_italic_tag(std::optional<std::string> data) 
{
    _current_element->set_attribute(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
    if (data) {
        _add_text(*data);
    }
}

void CtImportHandler::_add_scale_tag(int level, std::optional<std::string> data) 
{
    _current_element->set_attribute("scale", fmt::format("h{}", level));
    if (data) {
        _add_text(*data);
    }
}