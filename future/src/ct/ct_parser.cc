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
#include "ct_clipboard.h"
#include "ct_main_win.h"



void CtDocumentBuilder::wipe()  
{ 
    _document = std::make_shared<xmlpp::Document>();
    _current_element = _build_root_el();
}

xmlpp::Element* CtDocumentBuilder::_build_root_el() {
    return _document->create_root_node("root")->add_child("slot")->add_child("rich_text");
}

CtDocumentBuilder::CtDocumentBuilder(const CtConfig* pCtConfig) : _pCtConfig{pCtConfig}, _current_element{_document->create_root_node("root")->add_child("slot")->add_child("rich_text")} {}


void CtDocumentBuilder::add_image(const std::string& path) noexcept 
{
    try {
        CtXML::image_to_xml(_current_element->get_parent(), path, 0, CtConst::TAG_PROP_VAL_LEFT);
        close_current_tag();
    } catch(std::exception& e) {
        spdlog::error("Exception occured while adding image: {}", e.what());
    }
}

void CtDocumentBuilder::add_broken_link(const std::string& link) {
    _broken_links[link].emplace_back(_current_element);
}

void CtDocumentBuilder::add_superscript_tag(std::optional<std::string> text)
{
    _current_element->set_attribute(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUP);
    
    if (text) add_text(*text);
}

void CtDocumentBuilder::add_subscript_tag(std::optional<std::string> text)
{
    _current_element->set_attribute(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SUB);
    
    if (text) add_text(*text);
}

void CtDocumentBuilder::add_codebox(const std::string& language, const std::string& text)
{
    close_current_tag();
    xmlpp::Element* p_codebox_node = CtXML::codebox_to_xml(_current_element->get_parent(), CtConst::TAG_PROP_VAL_LEFT, 0, 300, 150, true, language, false, false);
    p_codebox_node->add_child_text(text);
    close_current_tag();
}

void CtDocumentBuilder::add_ordered_list(unsigned int level, const std::string &data)
{
    add_text(fmt::format("{}. {}", level, data));
}


void CtDocumentBuilder::add_todo_list(checkbox_state state, const std::string& text)
{
    auto todo_index = static_cast<int>(state);
    add_text(_pCtConfig->charsTodo[todo_index] + CtConst::CHAR_SPACE + text);
}

void CtDocumentBuilder::add_list(uint8_t level, const std::string& data)
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

add_text(indent + _pCtConfig->charsListbul[level] + CtConst::CHAR_SPACE + data);
}
void CtDocumentBuilder::add_weight_tag(const Glib::ustring& level, std::optional<std::string> data)
{
    _current_element->set_attribute("weight", level);
    if (data) {
        add_text(*data, false);
    }
}

void CtDocumentBuilder::add_monospace_tag(std::optional<std::string> text)
{
    _current_element->set_attribute("family", "monospace");
    if (text) add_text(*text, false);
}

void CtDocumentBuilder::add_link(const std::string& text)
{
    auto val = CtStrUtil::get_internal_link_from_http_url(text);
    _current_element->set_attribute(CtConst::TAG_LINK, val);
}

void CtDocumentBuilder::add_strikethrough_tag(std::optional<std::string> data)
{
    _current_element->set_attribute(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
    if (data) add_tag_data(CtConst::TAG_STRIKETHROUGH, *data);
}

void CtDocumentBuilder::add_text(std::string text, bool close_tag /* = true */)
{
    if (!text.empty()) {
        if (close_tag) close_current_tag();
    
        auto curr_text = _current_element->get_child_text();
        if (!curr_text) _current_element->set_child_text(std::move(text));
        else curr_text->set_content(curr_text->get_content() + std::move(text));
        if (close_tag) {
            _last_element = _current_element;
            close_current_tag();
        }
    }
}

void CtDocumentBuilder::with_last_element(const std::function<void()>& func) {
    xmlpp::Element* curr_tmp = _current_element;
    if (_last_element) _current_element = _last_element;

    try {
        func();
    } catch(const std::exception&) {
        _current_element = curr_tmp;
        throw;
    }
    _current_element = curr_tmp;
}

void CtDocumentBuilder::add_hrule() {
    add_text("\n" + _pCtConfig->hRule);
    add_newline();
}

void CtDocumentBuilder::close_current_tag()
{
    if (!tag_empty()) {
        _current_element = _current_element->get_parent()->add_child("rich_text");
        // Reset the tags
        _open_tags.clear();
    }
}

void CtDocumentBuilder::add_newline() 
{
    // Add a newline, if tags are empty no need to close the current tag
    add_text(CtConst::CHAR_NEWLINE, !_open_tags.empty());
}

void CtDocumentBuilder::add_italic_tag(std::optional<std::string> data)
{
    _current_element->set_attribute(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
    
    if (data) add_tag_data(CtConst::TAG_STYLE, *data);
}

void CtDocumentBuilder::add_scale_tag(int level, std::optional<std::string> data)
{
    _current_element->set_attribute(CtConst::TAG_SCALE, fmt::format("h{}", level));
    
    if (data) add_tag_data(CtConst::TAG_SCALE, *data);
}


void CtDocumentBuilder::add_tag_data(std::string_view tag, std::string data) 
{
    bool do_close = _open_tags[tag];
    
    add_text(std::move(data), do_close);
    _open_tags[tag] = true;
}



void CtDocumentBuilder::add_table(const std::vector<std::vector<std::string>>& table_matrix)
{
    CtXML::table_to_xml(table_matrix, _current_element->get_parent(), 0, CtConst::TAG_PROP_VAL_LEFT, 40, 400);
    close_current_tag();
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

const std::set<std::string> CtHtml2Xml::HTML_A_TAGS{"p", "b", "i", "u", "s", CtConst::TAG_PROP_VAL_H1,
            CtConst::TAG_PROP_VAL_H2, CtConst::TAG_PROP_VAL_H3, "span", "font"};


CtHtml2Xml::CtHtml2Xml(CtConfig* config) : _config(config)
{

}

void CtHtml2Xml::feed(const std::string& html)
{
    _xml_doc = _outter_doc ? _outter_doc : &_temp_doc;

    _state = ParserState::WAIT_BODY;
    _tag_id_generator = 0;
    _tag_styles.clear();
    _html_pre_tag_open = false;
    _html_td_tag_open = false;
    _parsing_valid_tag = true;
    _html_a_tag_counter = 0;
    _list_type = 'u';
    _list_num = 0;
    _list_level = -1;
    _table.clear();

    _slot_root = _xml_doc->create_root_node("root")->add_child("slot");
    _char_offset = 0;
    _slot_text = "";
    _slot_style_id = -1;
    _slot_styles_cache.clear();

    if (str::startswith(html, "<!doctype html>"))
        CtHtmlParser::feed(html);
    else {
        // if not fixed, we can skip some items
        std::string fixed_html = "<!doctype html><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"</head><body>"
                + html + "</body></html>";
        CtHtmlParser::feed(fixed_html);
    }


    _rich_text_save_pending();
}

void CtHtml2Xml::handle_starttag(std::string_view tag, const char** atts)
{
    _start_adding_tag_styles();

    if (vec::exists(CtConst::INVALID_HTML_TAGS, tag)) {
        _parsing_valid_tag = false;
        return;
    }
    _parsing_valid_tag = true;
    
    if (HTML_A_TAGS.count(tag.begin())) _html_a_tag_counter += 1;
    if (_state == ParserState::WAIT_BODY)
    {
        if (tag == "body") {
            _state = ParserState::PARSING_BODY;
        }
    }
    else if (_state == ParserState::PARSING_BODY)
    {
        if (tag == "table")
        {
            _table.clear();
            _html_td_tag_open = false;
            _state = ParserState::PARSING_TABLE;
        }
        else if (tag == "strong") _add_tag_style(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
        else if (tag == "b") _add_tag_style(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
        else if (tag == "i") _add_tag_style(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
        else if (tag == "em") _add_tag_style(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
        else if (tag == "u") _add_tag_style(CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE);
        else if (tag == "s") _add_tag_style(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
        else if (tag == CtConst::TAG_STYLE) _state = ParserState::WAIT_BODY;
        else if (tag == "span")
        {
            for (auto& tag_attr: char2list_attrs(atts))
            {
                if (tag_attr.name != CtConst::TAG_STYLE)
                    continue;
                for (Glib::ustring& style_attribute: str::split(Glib::ustring(tag_attr.value.begin()), ";"))
                {
                    int colon_pos = str::indexOf(style_attribute, CtConst::CHAR_COLON);
                    if (colon_pos < 0) continue;
                    auto attr_name = str::trim(style_attribute.substr(0, colon_pos).lowercase());
                    Glib::ustring attr_value = str::trim(style_attribute.substr(colon_pos + 1, style_attribute.size() - colon_pos).lowercase());
                    if (attr_name == "color") {
                        auto color = _convert_html_color(attr_value);
                        if (!color.empty())
                            _add_tag_style(CtConst::TAG_FOREGROUND, color);
                    } else if (attr_name == CtConst::TAG_BACKGROUND || attr_name == "background-color") {
                        auto color = _convert_html_color(attr_value);
                        if (!color.empty())
                            _add_tag_style(CtConst::TAG_BACKGROUND, color);
                    } else if (attr_name == "text-decoration") {
                        if (attr_value == CtConst::TAG_UNDERLINE || str::startswith(attr_value, "underline"))
                            _add_tag_style(CtConst::TAG_UNDERLINE, CtConst::TAG_PROP_VAL_SINGLE);
                        else if (attr_value == "line-through")
                            _add_tag_style(CtConst::TAG_STRIKETHROUGH, CtConst::TAG_PROP_VAL_TRUE);
                    } else if (attr_name == "font-weight") {
                        if (attr_value == "bold" || attr_value == "bolder" || attr_value == "700")
                            _add_tag_style(CtConst::TAG_WEIGHT, CtConst::TAG_PROP_VAL_HEAVY);
                    } else if (attr_name == "font-style") {
                        if (attr_value == CtConst::TAG_PROP_VAL_ITALIC)
                            _add_tag_style(CtConst::TAG_STYLE, CtConst::TAG_PROP_VAL_ITALIC);
                    } else if (attr_name == "font-size") {
                        try
                        {
                            attr_value = str::replace(attr_value, "pt", "");
                            // Can throw std::invalid_argument or std::out_of_range
                            int font_size = std::stoi(attr_value, nullptr); 
                            if (font_size > 0 && font_size < 11)
                                _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_SMALL);
                            else if (font_size > 13 && font_size < 19)
                                _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3);
                            else if (font_size >= 19)
                                _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H2);

                        } catch (std::invalid_argument&) {}
                    }
                }
            }
        }
        else if (tag == "font")
        {
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "color") {
                    auto color = _convert_html_color(str::trim(Glib::ustring(tag_attr.value.begin())));
                    if (color != "")
                        _add_tag_style(CtConst::TAG_FOREGROUND, color);
                }
            }
        }
        else if (tag == "p")
        {
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "align")
                    _add_tag_style(CtConst::TAG_JUSTIFICATION, str::trim(Glib::ustring(tag_attr.value.begin()).lowercase()));
            }
        }
        else if (tag == CtConst::TAG_PROP_VAL_SUP || tag == CtConst::TAG_PROP_VAL_SUB)
        {
            _add_tag_style(CtConst::TAG_SCALE, tag.begin());
        }
        else if (tag  == CtConst::TAG_PROP_VAL_H1 || tag == CtConst::TAG_PROP_VAL_H2  || tag == CtConst::TAG_PROP_VAL_H3
                 || tag == CtConst::TAG_PROP_VAL_H4  || tag == CtConst::TAG_PROP_VAL_H5  || tag == CtConst::TAG_PROP_VAL_H6)
        {
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
            if (tag == CtConst::TAG_PROP_VAL_H1 || tag == CtConst::TAG_PROP_VAL_H2) _add_tag_style(CtConst::TAG_SCALE, tag.begin());
            else _add_tag_style(CtConst::TAG_SCALE, CtConst::TAG_PROP_VAL_H3);
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "align")
                    _add_tag_style(CtConst::TAG_JUSTIFICATION, str::trim(Glib::ustring(tag_attr.value.begin()).lowercase()));
            }
        }
        else if (tag == "a")
        {
            for (auto& tag_attr: char2list_attrs(atts)) {
                if (tag_attr.name == "href" && tag_attr.value.size() > 7)
                    _add_tag_style(CtConst::TAG_LINK, CtStrUtil::get_internal_link_from_http_url(tag_attr.value.begin()));
            }
        }
        else if (tag == "br") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "ol")  { _list_type = 'o'; _list_num = 1; }
        else if (tag == "ul")  { 
            _list_type = 'u'; 
            _list_num = 0;
            if (_list_level < static_cast<int>(_config->charsListbul.size()) - 1) _list_level++;
        }
        else if (tag == "li") {
            if (_list_type == 'u') {
                if (_list_level < 0) {
                    // A ul _should_ have appeared before this
                    throw std::runtime_error("List item appeared before list declaration");
                }
                _rich_text_serialize(_config->charsListbul[_list_level] + CtConst::CHAR_SPACE);

            }
            else {
                _rich_text_serialize(std::to_string(_list_num) + ". ");
                _list_num += 1;
            }
        }
        else if (tag  == "img" || tag == "v:imagedata") {
            for (auto& tag_attr: char2list_attrs(atts))
                if (tag_attr.name == "src")
                    _insert_image(tag_attr.value.begin(), "");
        }
        else if (tag == "pre") _html_pre_tag_open = true;
        else if (tag == "code") _add_tag_style(CtConst::TAG_FAMILY, CtConst::TAG_PROP_VAL_MONOSPACE);
        else if (tag == "dt") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "dd") _rich_text_serialize(CtConst::CHAR_NEWLINE + Glib::ustring(CtConst::CHAR_TAB));
    }
    else if (_state == ParserState::PARSING_TABLE)
    {
        if (tag == "div") { // table is used as layout
            if (_table.empty()) {
                _table.clear();
                _html_td_tag_open = false;
                _state = ParserState::PARSING_BODY;
            }
        }
        else if (tag == "table") { // nested tables
            _table.clear();
            _html_td_tag_open = false;
        }
        else if (tag == "tr") {
            _table.push_back(std::list<table_cell>());
            _html_td_tag_open = false;
        }
        else if (tag == "td" || tag == "th") {
            _html_td_tag_open = true;
            if (_table.empty()) // case of first missing <tr>, this is the header even if <td>
                _table.push_back(std::list<table_cell>());
            int rowspan = 1;
            for (auto& tag_attr: char2list_attrs(atts))
                if (tag_attr.name == "rowspan")
                    rowspan = std::atoi(tag_attr.value.begin());
            _table.back().push_back(table_cell{rowspan, ""});
        }
        else if (tag == "img" || tag == "v:imagedata") {
            for (auto& tag_attr: char2list_attrs(atts))
                if (tag_attr.name == "src")
                    _insert_image(tag_attr.value.begin(), str::repeat(CtConst::CHAR_NEWLINE, 2));
        }
        else if (tag == "br" && _html_td_tag_open) _table.back().back().text += CtConst::CHAR_NEWLINE;
        else if (tag == "ol" && _html_td_tag_open) { _list_type = 'o'; _list_num = 1; }
        else if (tag == "ul" && _html_td_tag_open) { _list_type = 'u'; _list_num = 0; }
        else if (tag == "li" && _html_td_tag_open) {
            if (_list_type == 'u') _table.back().back().text += _config->charsListbul[0] + CtConst::CHAR_SPACE;
            else {
                _table.back().back().text += std::to_string(_list_num) + ". ";
                _list_num += 1;
            }
        }
    }
    _end_adding_tag_styles();
}

void CtHtml2Xml::handle_endtag(std::string_view tag)
{
    _pop_tag_styles();
    if (HTML_A_TAGS.count(tag.begin())) _html_a_tag_counter -= 1;
    if (_state == ParserState::WAIT_BODY)
    {
        if (tag == CtConst::TAG_STYLE)
            _state = ParserState::PARSING_BODY;
    }
    else if (_state == ParserState::PARSING_BODY)
    {
        if (tag == "p") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "div") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "pre") _html_pre_tag_open = false;
        else if (tag == CtConst::TAG_PROP_VAL_H1 || tag == CtConst::TAG_PROP_VAL_H2 || tag == CtConst::TAG_PROP_VAL_H3
                 || tag == CtConst::TAG_PROP_VAL_H4 || tag == CtConst::TAG_PROP_VAL_H5 || tag == CtConst::TAG_PROP_VAL_H6) {
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
        }
        else if (tag == "li") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "ul") {
            // Move back up a list level
            if (_list_level > 0) _list_level--;
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
        }
    }
    else if (_state == ParserState::PARSING_TABLE)
    {
        if (tag == "p" || tag == "li")
        {
            if (_html_td_tag_open)
                _table.back().back().text += CtConst::CHAR_NEWLINE;
        }
        else if (tag == "td" || tag == "th")
        {
            _html_td_tag_open = false;
        }
        else if (tag == "table")
        {
            _state = ParserState::PARSING_BODY;
            if (_table.size() && _table.back().size() == 0) // case of latest <tr> without any <tr> afterwards
                _table.pop_back();
            if (_table.size() == 1 && _table.back().size() == 1) // it's a codebox
                _insert_codebox();
            else // it's a table
                _insert_table();
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
        }
    }
}

void CtHtml2Xml::handle_data(std::string_view text)
{
    if (_state == ParserState::WAIT_BODY || !_parsing_valid_tag)
        return;
    if (_html_pre_tag_open) {
         _rich_text_serialize(text.begin());
         return;
    }
    Glib::ustring clean_data(text.begin());
    if (_html_a_tag_counter > 0) clean_data = str::replace(clean_data, CtConst::CHAR_NEWLINE, CtConst::CHAR_SPACE);
    else                         clean_data = str::replace(clean_data, CtConst::CHAR_NEWLINE, "");
    if (clean_data.empty() || clean_data == CtConst::CHAR_TAB)
        return;
    clean_data = str::replace(clean_data, "\x20", CtConst::CHAR_SPACE); // replace non-breaking space
    // not a good idea, if it's UTF-16, it should be converted
    // clean_data = str::replace(clean_data, "\xfeff", "");
    if (_state == ParserState::PARSING_BODY) {
        clean_data = str::replace(clean_data, CtConst::CHAR_TAB, CtConst::CHAR_SPACE);
        _rich_text_serialize(clean_data);
    }
    if (_state == ParserState::PARSING_TABLE && _html_td_tag_open) {
        clean_data = str::replace(clean_data, CtConst::CHAR_TAB, "");
        _table.back().back().text += clean_data;
    }
}




// Found Entity Reference like &name;
void CtHtml2Xml::handle_charref(std::string_view /*name*/)
{
    // todo: test it
}

void CtHtml2Xml::set_status_bar(CtStatusBar* status_bar)
{
    _status_bar = status_bar;
}

void CtHtml2Xml::_start_adding_tag_styles()
{
    // every tag will have style, even if it's empty
    _tag_styles.push_back(tag_style{++_tag_id_generator, "", ""});
}

void CtHtml2Xml::_add_tag_style(const std::string& style, const std::string& value)
{
    auto& current = _tag_styles.back();
    if (current.style.empty()) {
        current.style = style;
        current.value = value;
    } else {
        _tag_styles.push_back(tag_style{current.tag_id, style, value});
    }
}

void CtHtml2Xml::_end_adding_tag_styles()
{
    // actually, nothing to do here
}

void CtHtml2Xml::_pop_tag_styles()
{
    // every tag has at least one style
    int tag_id = _tag_styles.back().tag_id;
    while (_tag_styles.back().tag_id == tag_id)
        _tag_styles.pop_back();
}

int CtHtml2Xml::_get_tag_style_id()
{
    // use tag id as style id because they are unique
    for (auto tag_style = _tag_styles.rbegin(); tag_style != _tag_styles.rend(); ++tag_style)
        if (!tag_style->style.empty()) // skip empty because they don't matter
            return tag_style->tag_id;
    return 0;
}

void CtHtml2Xml::_put_tag_styles_on_top_cache()
{
    int current_tag_style = _get_tag_style_id();

    // check cache first if it exist move it on top
    auto exists_in_cache = [&]() {
        for (auto& style: _slot_styles_cache)
            if (style.slot_style_id == current_tag_style)
                return true;
        return false;
    };
    if (exists_in_cache())
    {
        // remove others, they won't need
        while (_slot_styles_cache.front().slot_style_id != current_tag_style)
            _slot_styles_cache.pop_front();
        return;
    }

    // or create and put in cache top
    _slot_styles_cache.push_front(slot_styles());
    auto& style = _slot_styles_cache.front();
    style.slot_style_id = current_tag_style;
    for (auto& tag_style: _tag_styles)
        if (!tag_style.style.empty())
            style.styles[tag_style.style] = tag_style.value;

    // clean up cache
    if (_slot_styles_cache.size() > 10)
        _slot_styles_cache.pop_back();
}

std::string CtHtml2Xml::_convert_html_color(const std::string& html_color)
{
    Gdk::RGBA rgba;
    if (!rgba.set(html_color))
        return "";
    // r+g+b black is 0
    // r+g+b white is 3*1.0 = 3.0
    double black_level = 3.0 / 100. * 15;    // lower is black-ish
    double white_level = 3.0 - black_level;  // higher is white-ish
    double sum_rgb = rgba.get_blue() + rgba.get_green() + rgba.get_red();
    if (sum_rgb < black_level || sum_rgb > white_level)
        return "";
    return CtRgbUtil::rgb_any_to_24(rgba);
}

// Insert Image in Buffer
void CtHtml2Xml::_insert_image(std::string img_path, std::string trailing_chars)
{
    _rich_text_save_pending();

    // todo: remove this copy-paste (image.cc)
    auto insert_image = [&](Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
        g_autofree gchar* pBuffer{NULL};
        gsize buffer_size;
        pixbuf->save_to_buffer(pBuffer, buffer_size, "png");
        const std::string rawBlob = std::string(pBuffer, buffer_size);
        const std::string encodedBlob = Glib::Base64::encode(rawBlob);

        xmlpp::Element* p_image_node = _slot_root->add_child("encoded_png");
        p_image_node->set_attribute("char_offset", std::to_string(_char_offset));
        p_image_node->set_attribute(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT);
        p_image_node->set_attribute("link", std::string(CtConst::LINK_TYPE_WEBS) + " " + img_path);
        p_image_node->add_child_text(encodedBlob);
    };

    if (_status_bar) {
        _status_bar->update_status(std::string(_("Downloading")) + " " + img_path + " ...");
       while (gtk_events_pending()) gtk_main_iteration();
    }

    bool image_good = false;

    // trying to download
    try {
        std::string file_buffer = fs::download_file(img_path);
        if (!file_buffer.empty()) {
            Glib::RefPtr<Gdk::PixbufLoader> pixbuf_loader = Gdk::PixbufLoader::create();
            pixbuf_loader->write((const guint8*)file_buffer.c_str(), file_buffer.size());
            pixbuf_loader->close();
            auto pixbuf = pixbuf_loader->get_pixbuf();
            insert_image(pixbuf);
            image_good = true;
        }
    }  catch (...) { }

    // trying to load from disk
    try {
        if (!image_good) {
            std::string local_image = Glib::build_filename(_local_dir, img_path);
            if (Glib::file_test(local_image, Glib::FILE_TEST_IS_REGULAR)) {
                Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(local_image);
                if (pixbuf) {
                    insert_image(pixbuf);
                    image_good = true;
                }
            }
        }
    }
    catch (...) {}

    if (image_good) {
        _char_offset += 1;
        if (!trailing_chars.empty())
            _rich_text_serialize(trailing_chars);
    } else {
        spdlog::error("Failed to download {}", img_path);
    }

    if (_status_bar)
        _status_bar->update_status("");
}

void CtHtml2Xml::_insert_table()
{
    _rich_text_save_pending();

    // add more cells for rowspan > 1
    for (auto& row: _table)
        for (auto iter = row.begin(); iter != row.end(); ++ iter)
            if (iter->rowspan > 1)
                row.insert(std::next(iter), iter->rowspan - 1, {1, ""});
    // find bigger row size
    size_t row_len = 0;
    for (auto& row: _table)
        row_len  = std::max(row_len, row.size());
    // add more cell for rowspan = 0
    for (auto& row: _table)
        for (auto iter = row.begin(); iter != row.end(); ++ iter)
            if (iter->rowspan == 0 && row.size() < row_len)
                row.insert(std::next(iter), row_len - row.size(), {1, ""});
    // add more cell just in case
    for (auto& row: _table)
        if (row.size() < row_len)
            row.insert(row.end(), row_len - row.size(), {1, ""});

    // todo: remove this copy-paste (table.cc)
    xmlpp::Element* p_table_node = _slot_root->add_child("table");
    p_table_node->set_attribute("char_offset", std::to_string(_char_offset));
    p_table_node->set_attribute(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT);
    p_table_node->set_attribute("col_min", std::to_string(40));
    p_table_node->set_attribute("col_max", std::to_string(400));


    auto row_to_xml = [&](const std::list<table_cell>& row) {
        xmlpp::Element* p_row_node = p_table_node->add_child("row");
        for (const auto& cell: row)
        {
            xmlpp::Element* p_cell_node = p_row_node->add_child("cell");
            p_cell_node->add_child_text(str::trim(cell.text));
        }
    };
    // put header at the end
    bool is_header = true;
    for (const auto& row: _table)
    {
        if (is_header) { is_header = false; continue; }
        row_to_xml(row);
    }
    row_to_xml(_table.front());

    _char_offset += 1;
}

void CtHtml2Xml::_insert_codebox()
{
    _rich_text_save_pending();

    // todo: fix this copy-paste from codebox.cc
    xmlpp::Element* p_codebox_node = _slot_root->add_child("codebox");
    p_codebox_node->set_attribute("char_offset", std::to_string(_char_offset));
    p_codebox_node->set_attribute(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_LEFT);
    p_codebox_node->set_attribute("frame_width", std::to_string(300));
    p_codebox_node->set_attribute("frame_height", std::to_string(150));
    p_codebox_node->set_attribute("width_in_pixels", std::to_string(true));
    p_codebox_node->set_attribute("syntax_highlighting", CtConst::PLAIN_TEXT_ID);
    p_codebox_node->set_attribute("highlight_brackets", std::to_string(false));
    p_codebox_node->set_attribute("show_line_numbers", std::to_string(false));
    p_codebox_node->add_child_text(str::trim(_table.back().back().text));

    _char_offset += 1;
}

// Appends a new part to the XML rich text
void CtHtml2Xml::_rich_text_serialize(std::string text)
{
    if (text.empty()) return;
    int current_tag_style_id = _get_tag_style_id();

    // fist time -> put styles on cache top
    if (_slot_style_id == -1) {
        _put_tag_styles_on_top_cache();
        _slot_style_id = current_tag_style_id;
    }
    // same style, text in the same slot
    if (_slot_style_id == current_tag_style_id)
    {
        _slot_text += text;
        return;
    }
    // styles changed, so
    // create slot with prevous text
    _rich_text_save_pending();

    //
    _put_tag_styles_on_top_cache();
    _slot_text = text;
    _slot_style_id = current_tag_style_id;
}

void CtHtml2Xml::_rich_text_save_pending()
{
    // the style is always on cache top
    if (_slot_text != "")
    {
        auto& s_style = _slot_styles_cache.front();

        xmlpp::Element* s = _slot_root->add_child("rich_text");
        for (auto& attr: s_style.styles)
            s->set_attribute(attr.first, attr.second);
        s->set_child_text(_slot_text);
        _char_offset += _slot_text.size();
    }

    _slot_text = "";
    _slot_style_id = -1;
}



CtMarkdownFilter::CtMarkdownFilter(std::unique_ptr<CtClipboard> clipboard, Glib::RefPtr<Gtk::TextBuffer> buff, CtConfig* config) : 
_config(config), _clipboard(std::move(clipboard)) 
{
    try {
        buffer(std::move(buff));
    } catch(const std::exception& e) {
        spdlog::error("Exception caught in CtMarkdownFilter ctor: <{}>; it may be left in an invalid state", e.what());
    }
}

void CtMarkdownFilter::buffer(Glib::RefPtr<Gtk::TextBuffer> text_buffer)
{   
    // Disconnect previous
    for (auto& sig : _buff_connections) {
        if (sig.connected()) sig.disconnect();
    }
    if (active()) {
        // Connect new
        _buff_connections[0] = text_buffer->signal_insert().connect(sigc::mem_fun(this, &CtMarkdownFilter::_on_buffer_insert));
        _buff_connections[1] = text_buffer->signal_end_user_action().connect([this]() mutable {
            _buff_connections[0].block();
            _buff_connections[3].block();
            if (active()) {
                _markdown_insert();
            }
        });
        _buff_connections[2] = text_buffer->signal_begin_user_action().connect([this]() mutable {
            _buff_connections[0].unblock();
            _buff_connections[3].unblock();
        });

        _buff_connections[3] = text_buffer->signal_erase().connect(sigc::mem_fun(this, &CtMarkdownFilter::_on_buffer_erase));
    }


    _buffer = std::move(text_buffer);
}

void CtMarkdownFilter::reset() noexcept
{
    if (active()) {
        _md_matchers.clear();
        _md_parser.reset();
        _md_matcher.reset();
    }
}


void CtMarkdownFilter::_on_buffer_erase(const Gtk::TextIter& begin, const Gtk::TextIter& end) noexcept
{
    try {
        bool is_all = (begin == _buffer->begin()) && (end == _buffer->end());
        if (is_all && active()) {
            reset();
            spdlog::debug("Reset markdown matchers due to buffer clear");
            return;
        }
        if (active()) {
            Gtk::TextIter iter = begin;
            iter.backward_char();

            auto erase_tag_seg = [this](const std::string& tag, CtTokenMatcher& matcher,
                                        Gtk::TextIter start) {
                std::size_t offset = 0;
                auto tag_ptr = _buffer->get_tag_table()->lookup(tag);
                while (true) {
                    if (!start.has_tag(tag_ptr)) break;
                    if (!start.forward_char()) break;
                    ++offset;
                }
                spdlog::debug("Found erase offset: <{}>", offset);
                if (offset < matcher.raw_str().size()) matcher.erase(offset);
            };

            while (iter != end) {
                for (const auto& tag : iter.get_tags()) {
                    std::string name = tag->property_name().get_value();
                    auto matcher_iter = _md_matchers.find(name);
                    if (matcher_iter != _md_matchers.end()) {
                        auto& md_matcher = matcher_iter->second.second;
                        erase_tag_seg(name, *md_matcher, iter);
                    }
                }
                if (!iter.forward_char()) break;
            }
        }
    } catch(const std::exception& e) {
        spdlog::error("Exception caught while erasing buffer: <{}>", e.what());
    }
}


void CtMarkdownFilter::_markdown_insert() {
    if (active() && (_md_matcher && _md_parser)) {
        
        if (_md_matcher->finished()) {
            auto start_offset = _md_matcher->raw_start_offset();
            auto end_offset = _md_matcher->raw_end_offset();
            std::string raw_token = _md_matcher->raw_str();

            _md_parser->wipe_doc();
            _md_matcher.reset();

            auto iter_begin = _buffer->get_insert()->get_iter();
            auto iter_end = iter_begin;
            iter_begin.backward_chars(start_offset);
            iter_end.backward_chars(end_offset);

            std::stringstream txt(raw_token);
            _md_parser->feed(txt);

            _buffer->place_cursor(iter_begin);
            _buffer->erase(iter_begin, iter_end);
            
        
            _clipboard->from_xml_string_to_buffer(_buffer, _md_parser->xml_doc_string());

            auto iter_insert = _buffer->get_insert()->get_iter();
            iter_insert.forward_chars(end_offset);
            _buffer->place_cursor(iter_insert);
        }
    }
}

void CtMarkdownFilter::_on_buffer_insert(const Gtk::TextBuffer::iterator& pos, const Glib::ustring& text, int) noexcept
{
    try {
        if (active() &&
            text.size() == 1 /* For now, only handle single chars */ ) {
            std::shared_ptr<CtMDParser> md_parser;
            std::shared_ptr<CtTokenMatcher> md_matcher;
            Gtk::TextIter back_iter = pos;
            back_iter.backward_chars(text.length() + 1);
            for (char insert_ch : text) {

                auto tags = back_iter.get_tags();
                bool has_mark = false;
                std::string tag_name = _get_new_md_tag_name();

                for (const auto& tag : tags) {
                    auto iter = _md_matchers.find(tag->property_name().get_value());
                    if (iter != _md_matchers.end()) {
                        has_mark = true;
                        tag_name = tag->property_name().get_value();
                        auto& pair = iter->second;
                        md_parser = pair.first;
                        md_matcher = pair.second;
                        break;
                    }
                }
                _apply_tag(tag_name, back_iter, pos);

                if (!has_mark) {
                    spdlog::debug("Creating new tag: {}", tag_name);
                    md_parser = std::make_shared<CtMDParser>(_config);
                    md_matcher = std::make_unique<CtTokenMatcher>(md_parser->text_parser());

                    match_pair_t pair{md_parser, md_matcher};

                    _md_matchers[tag_name] = pair;
                }

                if (!md_matcher->finished()) {
                    // Determine offset
                    std::size_t offset = 0;
                    auto for_iter = pos;
                    for_iter.backward_char();
                    while (for_iter.forward_char()) {
                        if (!for_iter.has_tag(_buffer->get_tag_table()->lookup(tag_name))) {
                            break;
                        }
                        if (for_iter.get_char() == '\n') break;
                        ++offset;
                    }
                    spdlog::trace("Inserting: <{}> at offset: <{}>", std::string(1, insert_ch), offset);
                    md_matcher->insert(insert_ch, offset);


                    _md_parser = md_parser;
                    _md_matcher = md_matcher;
                } else {
                    _buffer->remove_tag_by_name(tag_name, _buffer->begin(), _buffer->end());
                    _md_matchers.erase(tag_name);
                    md_parser->wipe_doc();
                    md_matcher->reset();
                    md_parser.reset();
                    md_matcher.reset();
                }
                back_iter.forward_char();
            }
        }
    } catch(const std::exception& e) {
        spdlog::error("Exception caught in the buffer insert handler: <{}>", e.what());
    }
}

void CtMarkdownFilter::_apply_tag(const Glib::ustring& tag, const Gtk::TextIter& start, const Gtk::TextIter& end) {
    spdlog::debug("Applying tag: {}", tag);
    bool has_tag = static_cast<bool>(_buffer->get_tag_table()->lookup(tag));
    if (!has_tag) {
        // create if not exists
        _buffer->create_tag(tag);
    }
    _buffer->apply_tag_by_name(tag, start, end);
}

std::string CtMarkdownFilter::_get_new_md_tag_name() const {
    return fmt::format("md-formatting-{}", _md_matchers.size());
}

bool CtMarkdownFilter::active() const noexcept 
{ 
    return _active && _config->enableMdFormatting; 
}
namespace {
using c_string = std::vector<char>;

std::vector<c_string> split_by_null(std::istream& in) 
{
    // getline will throw on eof otherwise
    std::vector<c_string> file_strings;

    in.seekg(0, std::ios::end);
    std::vector<char> buff(in.tellg());
    in.seekg(0, std::ios::beg);

    in.read(buff.data(), buff.size());

    auto iter = buff.begin();
    auto last = iter;
    for(;iter != buff.end(); ++iter) {
        if (*iter == '\0') {
            file_strings.emplace_back(last, iter + 1);
            last = iter + 1;
        }
    }
    
    in.seekg(0, std::ios::beg);

    return file_strings;
}


std::vector<CtMempadParser::page> parse_mempad_strings(const std::vector<c_string>& mem_strs) 
{
    // Expected input is something like MeMpAd1.\0\0...\0...\0... etc
    
    std::vector<CtMempadParser::page> parsed_strs;

    auto iter = mem_strs.begin() + 2;
    while (iter != mem_strs.end()) {
        // First character will be the number
        int page_lvl = iter->front();
        std::string title{iter->begin() + 1, iter->end()};

        ++iter;
        if (iter == mem_strs.end()) throw std::runtime_error("Unexpected EOF");
    
        std::string contents{iter->begin(), iter->end()};

        CtMempadParser::page p{
            .level = page_lvl,
            .name = std::move(title),
            .contents = std::move(contents)

        };
        parsed_strs.emplace_back(std::move(p));

        ++iter;
    }

    return parsed_strs;
}
}

void CtMempadParser::feed(std::istream& data)
{
    /**
     * The mempad data format is pretty simple:
     * 
     * Each file has a header which starts with `MeMpAd` then there is an encoding character " " is ASCII . is UTF-8. Then the page number which is max 1-5 characters
     * MeMpAd[.| ][0-9]{0,5}
     * 
     * Then each page has a level which is (annoyingly) is raw binary, next is the page name and finally the contents
     */



    // mempad uses null terminated strings in its format 
    auto strings = split_by_null(data);


    std::vector<page> new_pages = parse_mempad_strings(strings);
    _parsed_pages.insert(_parsed_pages.cend(), new_pages.begin(), new_pages.end());

}

CtZimParser::CtZimParser(CtConfig* config) : CtDocBuildingParser{config}, _text_parser{std::make_shared<CtTextParser>(_token_schemas())} {}

void CtZimParser::feed(std::istream& data)
{
   
    std::string line;

    bool found_header = false;

    try {
        while(std::getline(data, line, '\n')) {
            if (!found_header && line.find("Creation-Date:") != std::string::npos) {
                // Creation-Date: .* is the final line of the header
                // TODO: Read the creation date and use it for ts_creation
                found_header = true;
                
            } else {
                _parse_body_line(line);
            }
        }
    } catch(const std::ios::failure&) {}

}

std::vector<CtTextParser::token_schema> CtZimParser::_token_schemas()
{
    return {
            // Bold
            {"**", true, true, [this](const std::string& data){
                doc_builder().close_current_tag();
                doc_builder().add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
                doc_builder().close_current_tag();
            }},
            // Indentation detection for lists
            {"\t", false, false, [this](const std::string& data) {
                _list_level++;
                // Did a double match for even number of \t tags
                if (data.empty()) _list_level++;
            }},
            {"https://", false, false, [this](const std::string& data) {
                doc_builder().close_current_tag();
                doc_builder().add_link("https://"+data);
                doc_builder().add_text("https://"+data);
            }},
            {"http://", false, false, [this](const std::string& data) {
                doc_builder().close_current_tag();
                doc_builder().add_link("http://"+data);
                doc_builder().add_text("http://"+data);
            }},
            // Bullet list
            {"* ", false, false, [this](const std::string& data) {
                doc_builder().add_list(_list_level, data);
                _list_level = 0;
            }},
            // Italic
            {"//", true, true, [this](const std::string& data) {
                doc_builder().close_current_tag();
                doc_builder().add_italic_tag(data);
                doc_builder().close_current_tag();
            }},
            // Strikethrough
            {"~~", true, true, [this](const std::string& data){
                doc_builder().close_current_tag();
                doc_builder().add_strikethrough_tag(data);
                doc_builder().close_current_tag();
            }},
            // Headers
            {"==", false, false, [this](const std::string &data) {
                int count = 5;
                auto iter = data.begin();
                while (*iter == '=') {
                    count--;
                    ++iter;
                }
                if (count < 0) {
                    throw CtImportException(fmt::format("Parsing error while parsing header data: {} - Too many '='", data));
                }

                if (count > 3) {
                    // Reset to smaller (h3 currently)
                    count = 3;
                }

                auto str = str::replace(data, "= ", "");
                str = str::replace(str, "=", "");

                doc_builder().close_current_tag();
                doc_builder().add_scale_tag(count, str);
                doc_builder().close_current_tag();
            }, "==", true},
            // External link (e.g https://example.com)
            {"{{", true, false, [](const std::string&) {
                // Todo: Implement this (needs image importing)
            },"}}"},
            // Todo list
           // {"[", true, false, links_match_func, "] "},
            {"[*", true, false, [this](const std::string& data) {
                doc_builder().add_todo_list(CtDocumentBuilder::checkbox_state::ticked, data);
            }, "]"},
            {"[x", true, false, [this](const std::string& data) {
                doc_builder().add_todo_list(CtDocumentBuilder::checkbox_state::marked, data);
            }, "]"},
            {"[>", true, false, [this](const std::string& data) {
                doc_builder().add_todo_list(CtDocumentBuilder::checkbox_state::marked, data);
            }, "]"},
            {"[ ", true, false, [this](const std::string& data) {
                doc_builder().add_todo_list(CtDocumentBuilder::checkbox_state::unchecked, data);
            }, "]"},
            // Internal link (e.g MyPage)
            {"[[", true, false, [this](const std::string& data){
                doc_builder().close_current_tag();
                doc_builder().add_broken_link(data);
                doc_builder().add_text(data);
                doc_builder().close_current_tag();
            }, "]]"},
            // Verbatum - captures all the tokens inside it and print without formatting
            {"''", true, true, [this](const std::string& data){
                doc_builder().add_text(data);
            }, "''", true},
            // Suberscript
            {"^{", true, false, [this](const std::string& data){
                doc_builder().close_current_tag();
                doc_builder().add_superscript_tag(data);
                doc_builder().close_current_tag();
            }, "}"},
            // Subscript
            {"_{", true, false, [this](const std::string& data){
                doc_builder().close_current_tag();
                doc_builder().add_subscript_tag(data);
                doc_builder().close_current_tag();
            }, "}"}

    };
    
}

void CtZimParser::_parse_body_line(const std::string& line)
{
    auto tokens_raw = _text_parser->tokenize(line);
    auto tokens = _text_parser->parse_tokens(tokens_raw);

    for (const auto& token : tokens) {
        if (token.first) {
            token.first->action(token.second);
        } else {
            doc_builder().add_text(token.second);
        }

    }
    doc_builder().add_newline();
}


namespace {
// These hash tables use the tx or t attributes of the node as keys 
using name_lookup_table_t = std::unordered_map<std::string, Glib::ustring>;
using children_lookup_t = std::unordered_map<std::string, std::vector<std::string>>;
using node_loopup_table_t = std::unordered_map<std::string, CtLeoParser::leo_node>;

CtLeoParser::leo_node parse_leo_t_el(const xmlpp::Element& node) 
{
    assert(node.get_name() == "t");

    CtLeoParser::leo_node lnode;

    if (node.has_child_text()) {
        lnode.content = node.get_child_text()->get_content();
    }

    return lnode;
}

std::pair<std::string, Glib::ustring> read_leo_vnode(const xmlpp::Element& v_el) {
    assert(v_el.get_name() == "v");

    
    auto* vh_el = dynamic_cast<const xmlpp::Element*>(v_el.get_first_child());
    if (!vh_el) throw std::invalid_argument("v element does not have a <vh> child element!");

    std::string tx_id = v_el.get_attribute("t")->get_value();
    Glib::ustring contents = vh_el->get_child_text()->get_content();

    return {tx_id, contents};
}

void find_node_children(const xmlpp::Element& v_el, children_lookup_t& children) {
    assert(v_el.get_name() == "v");

    std::string tx_id = read_leo_vnode(v_el).first;
    for (auto* node : v_el.get_children("v")) {
        if (auto* el = dynamic_cast<xmlpp::Element*>(node)) {
            auto data = read_leo_vnode(*el);
            
            children[tx_id].emplace_back(data.first);
            find_node_children(*el, children);

        } else {
            throw std::runtime_error("Leo document is malformed");
        }
    }
}

void build_tx_lookup_table(const xmlpp::Node& vnode, name_lookup_table_t& table) {
    for (auto* node : vnode.get_children("v")) {
        if (auto* el = dynamic_cast<xmlpp::Element*>(node)) {
            table.emplace(read_leo_vnode(*el));
            build_tx_lookup_table(*el, table);
        }
    }
}

name_lookup_table_t generate_tx_lookup_table(const xmlpp::Node& vnode_root) {
    assert(vnode_root.get_name() == "vnodes");

    name_lookup_table_t tx_id_to_names;
    build_tx_lookup_table(vnode_root, tx_id_to_names);

    return tx_id_to_names;
}

children_lookup_t generate_children_lookup_table(const xmlpp::Node& vnode_root) {
    assert(vnode_root.get_name() == "vnodes");
    children_lookup_t lookup_tbl;
    for (auto* node : vnode_root.get_children()) {
        if (node->get_name() == "v") {
            if (auto* el = dynamic_cast<xmlpp::Element*>(node)) {
                find_node_children(*el, lookup_tbl);
            } else {
                throw std::runtime_error("Leo document is malformed");
            }
        }
    }
    return lookup_tbl;
}


void populate_leo_nodes(const xmlpp::Node& vnode_root, node_loopup_table_t& lnodes)  
{
    auto tx_lookup = generate_tx_lookup_table(vnode_root);
    auto child_lookup = generate_children_lookup_table(vnode_root);

    for (auto& node_pair : lnodes) {
        auto& node = node_pair.second;
        node.name = tx_lookup.at(node_pair.first);        
        auto node_children = child_lookup.find(node_pair.first);
        if (node_children != child_lookup.end()) {
            for (const auto& child_tx : node_children->second) {
                auto child = lnodes.at(child_tx);
                node.children.emplace_back(std::move(child));
            }
        }
    }    

}

node_loopup_table_t parse_leo_tnodes(const xmlpp::Node& tnodes_root) 
{
    assert(tnodes_root.get_name() == "tnodes");

    node_loopup_table_t leo_nodes;
    for (auto* node : tnodes_root.get_children()) {
        if (node->get_name() == "t") {
            if (auto* el = dynamic_cast<xmlpp::Element*>(node)) {
                std::string tx_id = el->get_attribute("tx")->get_value();
                leo_nodes.emplace(tx_id, parse_leo_t_el(*el));
            }
        }
    }

    return leo_nodes;
}

xmlpp::Node* find_leo_vnodes_el(const xmlpp::Node& root) {
    return root.get_children("vnodes").front();
}

xmlpp::Node* find_leo_tnodes_el(const xmlpp::Node& root) {
    return root.get_children("tnodes").front();
}

std::vector<CtLeoParser::leo_node> parse_leo_tree(const xmlpp::Node& vnode_root, xmlpp::Node& tnode_root) {
    auto l_nodes = parse_leo_tnodes(tnode_root);
    populate_leo_nodes(vnode_root, l_nodes);

    std::vector<CtLeoParser::leo_node> populated_nodes;
    for (auto& node : l_nodes) {
        populated_nodes.emplace_back(std::move(node.second));
    }

    return populated_nodes;
}

std::vector<CtLeoParser::leo_node> walk_leo_xml(const xmlpp::Node& root) {
    auto* vnode = find_leo_vnodes_el(root);
    auto* tnode = find_leo_tnodes_el(root);

    if (!vnode || !tnode) throw std::runtime_error("Leo XML is malformed");

    return parse_leo_tree(*vnode, *tnode);
}
}

void CtLeoParser::feed(std::istream& in) {

    xmlpp::DomParser p;
    p.parse_stream(in);

    xmlpp::Element* root = p.get_document()->get_root_node();

    auto new_nodes = walk_leo_xml(*root);
    _leo_nodes.insert(_leo_nodes.cend(), new_nodes.begin(), new_nodes.end());
}

