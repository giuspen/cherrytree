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

            auto erase_tag_seg = [this](const std::string& tag, CtTextParser::TokenMatcher& matcher,
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

            _md_parser->wipe();
            _md_matcher.reset();

            auto iter_begin = _buffer->get_insert()->get_iter();
            auto iter_end = iter_begin;
            iter_begin.backward_chars(start_offset);
            iter_end.backward_chars(end_offset);

            std::stringstream txt(raw_token);
            _md_parser->feed(txt);

            _buffer->place_cursor(iter_begin);
            _buffer->erase(iter_begin, iter_end);
            
        
            _clipboard->from_xml_string_to_buffer(_buffer, _md_parser->to_string());

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
            std::shared_ptr<CtTextParser::TokenMatcher> md_matcher;
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
                    md_matcher = std::make_unique<CtTextParser::TokenMatcher>(md_parser);

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
                    md_parser->wipe();
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

