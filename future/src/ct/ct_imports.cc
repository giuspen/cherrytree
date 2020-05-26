/*
 * ct_imports.cc
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
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_main_win.h"
#include <libxml2/libxml/SAX.h>
#include <iostream>
#include <glibmm/base64.h>
#include <fstream>
#include <sstream>

const std::set<std::string> CtHtml2Xml::HTML_A_TAGS{"p", "b", "i", "u", "s", CtConst::TAG_PROP_VAL_H1,
            CtConst::TAG_PROP_VAL_H2, CtConst::TAG_PROP_VAL_H3, "span", "font"};

// Parse plain text for possible web links
std::vector<std::pair<int, int>> CtImports::get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text)
{
    std::vector<std::pair<int, int>> web_links;
    int max_end_offset = (int)plain_text.size();
    int max_start_offset = max_end_offset - 7;
    int start_offset = 0;
    while (start_offset < max_start_offset)
    {
        if (str::startswith_any(plain_text.substr(start_offset), CtConst::WEB_LINK_STARTERS))
        {
            int end_offset = start_offset + 3;
            while (end_offset < max_end_offset
                   && plain_text[(size_t)end_offset] != g_utf8_get_char(CtConst::CHAR_SPACE)
                   && plain_text[(size_t)end_offset] != g_utf8_get_char(CtConst::CHAR_NEWLINE))
                end_offset += 1;
            web_links.push_back(std::make_pair(start_offset, end_offset));
            start_offset = end_offset + 1;
        }
        else
            start_offset += 1;
    }
    return web_links;
}

std::string CtImports::get_internal_link_from_http_url(std::string link_url)
{
    if (str::startswith(link_url, "http"))          return "webs " + link_url;
    else if (str::startswith(link_url, "file://"))  return "file " + Glib::Base64::encode(link_url.substr(7));
    else                                            return "webs http://" + link_url;
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

void CtHtmlParser::handle_starttag(std::string_view tag, const char **/*atts*/)
{
    std::cout << "SAX tag: " << tag << std::endl;
}

void CtHtmlParser::handle_endtag(std::string_view tag)
{
    std::cout << "SAX endtag: " << tag << std::endl;
}

void CtHtmlParser::handle_data(std::string_view text)
{
    std::cout << "SAX data: " << text << std::endl;
}

void CtHtmlParser::handle_charref(std::string_view name)
{
    std::cout << "SAX ref: " << name << std::endl;
}

std::list<CtHtmlParser::html_attr> CtHtmlParser::char2list_attrs(const char** atts)
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



CtHtml2Xml::CtHtml2Xml(CtMainWin *pCtMainWin)
    :_pCtMainWin(pCtMainWin)
{

}

void CtHtml2Xml::feed(const std::string& html)
{
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

    _slot_root = _xml_doc.create_root_node("root")->add_child("slot");
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
                            if (font_size > 7 && font_size < 11)
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
                    _add_tag_style(CtConst::TAG_LINK, CtImports::get_internal_link_from_http_url(tag_attr.value.begin()));
            }
        }
        else if (tag == "br") _rich_text_serialize(CtConst::CHAR_NEWLINE);
        else if (tag == "ol")  { _list_type = 'o'; _list_num = 1; }
        else if (tag == "ul")  { 
            _list_type = 'u'; 
            _list_num = 0;
            if (_list_level < static_cast<int>(_pCtMainWin->get_ct_config()->charsListbul.size()) - 1) _list_level++;
        }
        else if (tag == "li") {
            if (_list_type == 'u') {
                if (_list_level < 0) {
                    // A ul _should_ have appeared before this
                    throw std::runtime_error("List item appeared before list declaration");
                }
                _rich_text_serialize(_pCtMainWin->get_ct_config()->charsListbul[_list_level] + CtConst::CHAR_SPACE);

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
            if (_list_type == 'u') _table.back().back().text += _pCtMainWin->get_ct_config()->charsListbul[0] + CtConst::CHAR_SPACE;
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
void CtHtml2Xml::_insert_image(std::string /*img_path*/, std::string /*trailing_chars*/)
{
    _rich_text_save_pending();
    // todo:
    /*try:
        self.dad.statusbar.push(self.dad.statusbar_context_id, _("Downloading") + " %s ..." % img_path)
        while gtk.events_pending(): gtk.main_iteration()
        url_desc = urllib2.urlopen(img_path, timeout=3)
        image_file = url_desc.read()
        pixbuf_loader = gtk.gdk.PixbufLoader()
        pixbuf_loader.write(image_file)
        pixbuf_loader.close()
        pixbuf = pixbuf_loader.get_pixbuf()
        self.dad.xml_handler.pixbuf_element_to_xml([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT], self.nodes_list[-1], self.dom)
        self.chars_counter += 1
        self.dad.statusbar.pop(self.dad.statusbar_context_id)
        if trailing_chars: self.rich_text_serialize(trailing_chars)
    except:
        if os.path.isfile(os.path.join(self.local_dir, img_path)):
            pixbuf = gtk.gdk.pixbuf_new_from_file(os.path.join(self.local_dir, img_path))
            self.dad.xml_handler.pixbuf_element_to_xml([self.chars_counter, pixbuf, cons.TAG_PROP_LEFT], self.nodes_list[-1], self.dom)
            self.chars_counter += 1
        else: print "failed download of", img_path
        self.dad.statusbar.pop(self.dad.statusbar_context_id)
*/

    // don't forget to move offset
    // _char_offset += 1;
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

void CtHtml2Xml::add_file(const std::filesystem::path &path) noexcept 
{
    
    try {
        std::ifstream infile;
        infile.exceptions(std::ios_base::failbit);
        infile.open(path);
        
        std::ostringstream ss;
        ss << infile.rdbuf();
        
        feed(ss.str());
        
        
    } catch(std::exception& e) {
        std::cerr << "Exception caught while adding file to XML: " << e.what() << "\n";
    }
    
    
}

