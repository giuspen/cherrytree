/*
 * ct_imports.h
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

#pragma once

#include <vector>
#include <list>
#include <set>
#include <glibmm/ustring.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml++/libxml++.h>
#include <filesystem>


namespace CtImports {

std::vector<std::pair<int, int>> get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text);
std::string get_internal_link_from_http_url(std::string link_url);
}

class CtHtmlParser
{
public:
    struct html_attr
    {
        std::string_view name;
        std::string_view value;
    };

public:
    CtHtmlParser() = default;
    virtual ~CtHtmlParser() = default;

    virtual void feed(const std::string& html);

    virtual void handle_starttag(std::string_view tag, const char** atts);
    virtual void handle_endtag(std::string_view tag);
    virtual void handle_data(std::string_view text);
    virtual void handle_charref(std::string_view name);

public:
    std::list<html_attr> char2list_attrs(const char** atts);
};




// pygtk: HTMLHandler
class CtMainWin;
class CtHtml2Xml : public CtHtmlParser
{
private:
    enum class ParserState {WAIT_BODY, PARSING_BODY, PARSING_TABLE};
    struct tag_style
    {
        int         tag_id;
        std::string style;
        std::string value;
    };
    struct table_cell
    {
        int         rowspan;
        std::string text;
    };
    struct slot_styles
    {
        int slot_style_id;
        std::map<std::string, std::string> styles;
    };


    static const std::set<std::string> HTML_A_TAGS;

public:
    CtHtml2Xml(CtMainWin* pCtMainWin);

    virtual void feed(const std::string& html);
    virtual void handle_starttag(std::string_view tag, const char** atts);
    virtual void handle_endtag(std::string_view tag);
    virtual void handle_data(std::string_view text);
    virtual void handle_charref(std::string_view name);

    void add_file(const std::filesystem::path& path) noexcept;
    
    Glib::ustring to_string() { return _xml_doc.write_to_string(); }

private:
    void _start_adding_tag_styles();
    void _add_tag_style(const std::string& style, const std::string& value);
    void _end_adding_tag_styles();
    void _pop_tag_styles();
    int  _get_tag_style_id();
    void _put_tag_styles_on_top_cache();

    std::string _convert_html_color(const std::string& html_color);
    void        _insert_image(std::string img_path, std::string trailing_chars);
    void        _insert_table();
    void        _insert_codebox();
    void        _rich_text_serialize(std::string text);
    void        _rich_text_save_pending();

private:
    CtMainWin*            _pCtMainWin;

    // releated to parsing html
    ParserState           _state;
    int                   _tag_id_generator;
    std::list<tag_style>  _tag_styles;
    bool                  _html_pre_tag_open;
    bool                  _html_td_tag_open;
    bool                  _parsing_valid_tag = true;
    int                   _html_a_tag_counter;
    char                  _list_type;
    int                   _list_num;
    int                   _list_level = -1;
    std::list<std::list<table_cell>> _table;

    // related to generating xml
    xmlpp::Document        _xml_doc;
    int                    _char_offset;
    xmlpp::Element*        _slot_root;
    std::string            _slot_text;
    int                    _slot_style_id;
    std::list<slot_styles> _slot_styles_cache;
};
