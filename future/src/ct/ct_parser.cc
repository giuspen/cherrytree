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

#include "ct_parser.h"
#include "ct_const.h"
#include "ct_config.h"
#include "ct_misc_utils.h"
#include "ct_imports.h"
#include "ct_logging.h"
#include <iostream>

void CtParser::wipe()  
{ 
    _document = std::make_unique<xmlpp::Document>();
    _current_element = nullptr;
}

void CtParser::_add_image(const std::string& path) noexcept 
{
    try {
        CtXML::image_to_xml(_current_element->get_parent(), path, 0, CtConst::TAG_PROP_VAL_LEFT);
        _close_current_tag();
    } catch(std::exception& e) {
        spdlog::error("Exception occured while adding image: {}", e.what());
    }
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

void CtParser::_add_codebox(const std::string& language, const std::string& text)
{
    _close_current_tag();
    xmlpp::Element* p_codebox_node = CtXML::codebox_to_xml(_current_element->get_parent(), CtConst::TAG_PROP_VAL_LEFT, 0, 300, 150, true, language, false, false);
    p_codebox_node->add_child_text(text);
    _close_current_tag();
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

void CtParser::_add_monospace_tag(std::optional<std::string> text)
{
    _current_element->set_attribute("family", "monospace");
    if (text) _add_text(*text, false);
}

void CtParser::_add_link(const std::string& text)
{
    auto val = CtStrUtil::get_internal_link_from_http_url(text);
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
        if (close_tag) {
            _last_element = _current_element;
            _close_current_tag();
        }
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

void CtParser::_add_newline() 
{
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


void CtParser::_add_tag_data(std::string_view tag, std::string data) 
{
    bool do_close = _open_tags[tag];
    
    _add_text(std::move(data), do_close);
    _open_tags[tag] = true;
}


void CtParser::_build_token_maps() 
{
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
                    if (token.is_symmetrical || !token.has_closetag) _close_tokens_map[token.open_tag]  = &token;
                    else                                             _close_tokens_map[token.close_tag] = &token;
                }
            }
        }
    }
}


void CtParser::_add_table(const std::vector<std::vector<std::string>>& table_matrix)
{
    CtXML::table_to_xml(table_matrix, _current_element->get_parent(), 0, CtConst::TAG_PROP_VAL_LEFT, 40, 400);
    _close_current_tag();
}


void CtHtmlParser::feed(const std::string& html)
{
    struct helper_function
    {
        static void start_element(void *ctx, const xmlChar *name, const xmlChar **atts)
        {
            reinterpret_cast<CtHtmlParser*>(ctx)->handle_starttag((const char*)name, (const char**)atts);
        }
        static void end_element(void* ctx, const xmlChar* name)
        {
            reinterpret_cast<CtHtmlParser*>(ctx)->handle_endtag((const char*)name);
        }
        static void characters(void *ctx, const xmlChar *ch, int len)
        {
            reinterpret_cast<CtHtmlParser*>(ctx)->handle_data(std::string_view((const char*)ch, len));
        }
        static void reference(void *ctx, const xmlChar *name)
        {
            reinterpret_cast<CtHtmlParser*>(ctx)->handle_charref((const char*)name);
        }
    };

    htmlSAXHandler sax2Handler;
    memset(&sax2Handler, 0, sizeof(sax2Handler));
    sax2Handler.initialized = XML_SAX2_MAGIC;
    sax2Handler.startElement = helper_function::start_element;
    sax2Handler.endElement = helper_function::end_element;
    sax2Handler.characters = helper_function::characters;
    sax2Handler.reference = helper_function::reference;

    htmlSAXParseDoc((xmlChar*)html.c_str(), "UTF-8", &sax2Handler, this);
}

void CtHtmlParser::handle_starttag(std::string_view /*tag*/, const char **/*atts*/)
{
    // spdlog::debug("SAX tag: {}", tag);
}

void CtHtmlParser::handle_endtag(std::string_view /*tag*/)
{
    // spdlog::debug("SAX endtag: {}", tag);
}

void CtHtmlParser::handle_data(std::string_view /*tag*/)
{
    // spdlog::debug("SAX data: {}", text);
}

void CtHtmlParser::handle_charref(std::string_view /*tag*/)
{
    // spdlog::debug("SAX ref: {}", name);
}

/*static*/ std::list<CtHtmlParser::html_attr> CtHtmlParser::char2list_attrs(const char** atts)
{
    std::list<html_attr> attr_list;
    if (atts == nullptr)  return attr_list;
    while (*atts != nullptr)
    {
        html_attr attr;
        attr.name = *(atts++);
        attr.value = *(atts++);
        attr_list.push_back(attr);
    }
    return attr_list;
}

