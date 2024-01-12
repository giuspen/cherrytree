/*
 * ct_parser_html.cc
 *
 * Copyright 2009-2024
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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

#include "ct_main_win.h"
#include "ct_parser.h"
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_storage_xml.h"

#include <cassert>

namespace {
std::vector<std::string> split_rednotebook_html_nodes(const std::string& input)
{
    static const auto reg = Glib::Regex::create("<p>[\\S\\s]<span id=\"(?:[12]\\d{3}-(?:0[1-9]|1[0-2])-(?:0[1-9]|[12]\\d|3[01]))\"></span>[\\S\\s]</p>");
    std::vector<std::string> out = reg->split(input);
    return out;
}

std::optional<std::string> match_rednotebook_title_h1(const Glib::ustring& input)
{
    static const auto reg = Glib::Regex::create("<h1>([\\S\\s]*?)<\\/h1>");

    Glib::MatchInfo mi;
    reg->match(input, mi);
    if (mi.matches()) {
        return mi.fetch(1);
    }
    return std::nullopt;
}





std::shared_ptr<xmlpp::Document> html_to_xml_doc(const std::string& contents, CtConfig* config)
{
    CtHtml2Xml parser{config};
    parser.feed(contents);
    auto doc = std::make_shared<xmlpp::Document>();
    doc->create_root_node_by_import(parser.doc().get_root_node());

    return doc;
}

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
        if (*atts != nullptr) {
            attr.value = *(atts++);
        }
        attr_list.push_back(attr);
    }
    return attr_list;
}

const std::set<std::string> CtHtml2Xml::HTML_A_TAGS{
    "p", "b", "i", "u", "s",
    CtConst::TAG_PROP_VAL_H1, CtConst::TAG_PROP_VAL_H2, CtConst::TAG_PROP_VAL_H3,
    CtConst::TAG_PROP_VAL_H4, CtConst::TAG_PROP_VAL_H5, CtConst::TAG_PROP_VAL_H6,
    "span", "font"};

CtHtml2Xml::CtHtml2Xml(CtConfig* config) : _pCtConfig{config}
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

    const Glib::ustring doctype = "<!DOCTYPE HTML";
    if (str::startswith(html, doctype) or str::startswith(html, doctype.lowercase())) {
        CtHtmlParser::feed(html);
    }
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
    auto parse_style_attribute = [&](const Glib::ustring& style_data){
        for (Glib::ustring& style_attribute: str::split(style_data, ";"))
        {
            int colon_pos = str::indexOf(style_attribute, CtConst::CHAR_COLON);
            if (colon_pos < 0) continue;
            auto attr_name = str::trim(style_attribute.substr(0, colon_pos).lowercase());
            Glib::ustring attr_value = str::trim(style_attribute.substr(colon_pos + 1, style_attribute.size() - colon_pos).lowercase());
            if (attr_name == "text-align") {
                if (attr_value == CtConst::TAG_PROP_VAL_LEFT || attr_value == CtConst::TAG_PROP_VAL_CENTER || attr_value == CtConst::TAG_PROP_VAL_RIGHT)
                    _add_tag_style(CtConst::TAG_JUSTIFICATION, attr_value);
            } else if (attr_name == "color") {
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
    };

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
                if (tag_attr.name == CtConst::TAG_STYLE)
                    parse_style_attribute(Glib::ustring(tag_attr.value.begin()));
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
                    _add_tag_style(CtConst::TAG_JUSTIFICATION, str::trim(Glib::ustring{tag_attr.value.begin()}.lowercase()));
                else if (tag_attr.name == CtConst::TAG_STYLE)
                    parse_style_attribute(Glib::ustring{tag_attr.value.begin()});
            }
        }
        else if (tag == CtConst::TAG_PROP_VAL_SUP || tag == CtConst::TAG_PROP_VAL_SUB)
        {
            _add_tag_style(CtConst::TAG_SCALE, tag.begin());
        }
        else if (tag == CtConst::TAG_PROP_VAL_H1 or tag == CtConst::TAG_PROP_VAL_H2 or tag == CtConst::TAG_PROP_VAL_H3 or
                 tag == CtConst::TAG_PROP_VAL_H4 or tag == CtConst::TAG_PROP_VAL_H5 or tag == CtConst::TAG_PROP_VAL_H6)
        {
            _rich_text_serialize(CtConst::CHAR_NEWLINE);
            _add_tag_style(CtConst::TAG_SCALE, tag.begin());
            for (auto& tag_attr : char2list_attrs(atts)) {
                if (tag_attr.name == "align")
                    _add_tag_style(CtConst::TAG_JUSTIFICATION, str::trim(Glib::ustring{tag_attr.value.begin()}.lowercase()));
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
            if (_list_level < static_cast<int>(_pCtConfig->charsListbul.size()) - 1) _list_level++;
        }
        else if (tag == "li") {
            if (_list_type == 'u') {
                if (_list_level < 0) {
                    // A <ul> _should_ have appeared before this
                    // but ok, use just some default list
                    _rich_text_serialize(_pCtConfig->charsListbul[0] + CtConst::CHAR_SPACE);
                }
                else {
                    _rich_text_serialize(_pCtConfig->charsListbul[_list_level] + CtConst::CHAR_SPACE);
                }
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
            if (_list_type == 'u') _table.back().back().text += _pCtConfig->charsListbul[0] + CtConst::CHAR_SPACE;
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
        else if (tag == CtConst::TAG_PROP_VAL_H1 or tag == CtConst::TAG_PROP_VAL_H2 or tag == CtConst::TAG_PROP_VAL_H3 or
                 tag == CtConst::TAG_PROP_VAL_H4 or tag == CtConst::TAG_PROP_VAL_H5 or tag == CtConst::TAG_PROP_VAL_H6)
        {
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

    // new method to remove too white and too black colors
    // from https://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color
    auto sRGBtoLin = [](double colorChannel) {
        if (colorChannel <= 0.04045) return colorChannel / 12.92;
        else return pow((( colorChannel + 0.055)/1.055),2.4);
    };
    double Y = (0.2126 * sRGBtoLin(rgba.get_red()) + 0.7152 * sRGBtoLin(rgba.get_green()) + 0.0722 * sRGBtoLin(rgba.get_blue()));
    if (Y < 0.05 /* <5% */ || Y > 0.95 /* >95% */)
        return ""; // though I think it should explicitly return black/white


    return CtRgbUtil::rgb_to_string_24(rgba);
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
        p_image_node->set_attribute("link", CtConst::LINK_TYPE_WEBS + CtConst::CHAR_SPACE + img_path);
        p_image_node->add_child_text(encodedBlob);
    };

    if (_status_bar) {
        _status_bar->update_status(std::string(_("Downloading")) + " " + img_path + " ...");
       while (gtk_events_pending()) gtk_main_iteration();
    }

    bool image_good = false;


    // 1. trying base64 encoding
    try
    {
        if (str::startswith(img_path, "data:image") && img_path.find(',') != std::string::npos)
        {
            const std::string image_base64 = img_path.substr(img_path.find(',') + 1);
            const std::string image_raw = Glib::Base64::decode(image_base64);
            Glib::RefPtr<Gdk::PixbufLoader> pixbuf_loader = Gdk::PixbufLoader::create();
            pixbuf_loader->write((const guint8*)image_raw.c_str(), image_raw.size());
            pixbuf_loader->close();
            auto pixbuf = pixbuf_loader->get_pixbuf();
            insert_image(pixbuf);
            image_good = true;
        }
    } catch (...) { }

    // 2. trying to download
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

    // 3. trying to load from disk
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

    if (_table.empty()) return;

    // add more cells for rowspan > 1
    for (auto& row : _table)
        for (auto iter = row.begin(); iter != row.end(); ++ iter)
            if (iter->rowspan > 1)
                row.insert(std::next(iter), iter->rowspan - 1, {1, ""});
    // find bigger row size
    size_t row_len = 0;
    for (auto& row : _table)
        row_len  = std::max(row_len, row.size());
    // add more cell for rowspan = 0
    for (auto& row : _table)
        for (auto iter = row.begin(); iter != row.end(); ++ iter)
            if (iter->rowspan == 0 && row.size() < row_len)
                row.insert(std::next(iter), row_len - row.size(), {1, ""});
    // add more cell just in case
    for (auto& row : _table)
        if (row.size() < row_len)
            row.insert(row.end(), row_len - row.size(), {1, ""});

    std::vector<std::vector<Glib::ustring>> table_matrix;
    table_matrix.reserve(_table.size());
    for (const auto& row : _table) {
        table_matrix.push_back(std::vector<Glib::ustring>{});
        table_matrix.back().reserve(row.size());
        for (const auto& cell : row)
            table_matrix.back().push_back(str::trim(cell.text));
    }

    const bool is_light = table_matrix.size() > 0 and table_matrix.size() * table_matrix.front().size() > static_cast<unsigned>(_pCtConfig->tableCellsGoLight);
    CtXmlHelper::table_to_xml(_slot_root, table_matrix, _char_offset, CtConst::TAG_PROP_VAL_LEFT, _pCtConfig->tableColWidthDefault, "", is_light);

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


void CtRedNotebookParser::feed(const std::string& in)
{
    _feed_str(in);
}

void CtRedNotebookParser::_feed_str(const std::string& in)
{
    auto pages = split_rednotebook_html_nodes(in);
    pages.erase(pages.cbegin(), pages.cbegin() + 1);

    for (const auto& page : pages) {
        auto name = match_rednotebook_title_h1(page);
        _add_node(name ? std::move(*name) : "?", page);
    }
}


void CtRedNotebookParser::_add_node(std::string&& name, const std::string& contents)
{
    node new_node{
        std::move(name), html_to_xml_doc(contents, _ct_config)
    };

    _nodes.emplace_back(std::move(new_node));
}

void CtNoteCaseHTMLParser::feed(const std::string& input)
{
    _feed_str(input);
}

void CtNoteCaseHTMLParser::_feed_str(const std::string& str) {
    auto split_nodes = _split_notecase_html_nodes(str);
    std::vector<node> new_nodes = _generate_notecase_nodes(std::make_move_iterator(split_nodes.begin()),
                                                        std::make_move_iterator(split_nodes.end()), _ct_config);
    _nodes.insert(_nodes.cend(), std::make_move_iterator(new_nodes.cbegin()), std::make_move_iterator(new_nodes.cend()));
}

std::vector<CtNoteCaseHTMLParser::notecase_split_output> CtNoteCaseHTMLParser::_split_notecase_html_nodes(const std::string& input)
{
    /*
     * Each "note" is seperated by a DT with font weight as bold and then a link tag with a 22 length string
     * (I assume the string is some sort of hash since it is just a jumble of characters, I guess it is used
     * as anchor points for links). Links are not handled because they require a pro version.
     */
    static const auto reg = Glib::Regex::create("<DT style=\"font-weight: bold;\"><A name=\"[a-z|A-Z|0-9]{22}\"></A>([\\S\\s]*?)</DT>");
    std::vector<std::string> split_strs = reg->split(input);
    return _handle_notecase_split_strings(split_strs.cbegin() + 1, split_strs.cend());
}

CtNoteCaseHTMLParser::node CtNoteCaseHTMLParser::_generate_notecase_node(const notecase_split_output& split_node, CtConfig* config) {
    CtNoteCaseHTMLParser::node nout{split_node.note_name, html_to_xml_doc(split_node.note_contents, config)};
    return nout;
}

