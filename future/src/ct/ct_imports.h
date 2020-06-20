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

#include "ct_const.h"
#include "ct_parser.h"
#include "ct_filesystem.h"

#include <glibmm/ustring.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml++/libxml++.h>
#include <queue>
#include <utility>

namespace {
using TableMatrx = std::queue<std::queue<std::string>>;
}

struct ct_imported_node
{
    fs::path                         path;
    Glib::ustring                    node_name;
    gint64                           node_id {-1};     // generated at the end
    std::string                      node_syntax {CtConst::RICH_TEXT_ID};
    xmlpp::Document                  xml_content;
    std::map<Glib::ustring, std::vector<xmlpp::Element*>> content_broken_links;
    std::list<std::unique_ptr<ct_imported_node>> children;

    ct_imported_node(fs::path _path, const Glib::ustring& _name) : path(std::move(_path)), node_name(_name) {}
    void add_broken_link(const Glib::ustring& link, xmlpp::Element* el) { content_broken_links[link].push_back(el); }
    bool has_content() { return xml_content.get_root_node(); }
};


class CtImporterInterface
{
public:
    virtual std::unique_ptr<ct_imported_node> import_file(const fs::path& file) = 0;
    virtual std::vector<std::string>          file_mimes() { return {}; }
    virtual std::vector<std::string>          file_patterns() { return {}; }
};


namespace CtImports {

    std::vector<std::pair<int, int>> get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text);
    std::unique_ptr<ct_imported_node> traverse_dir(const fs::path& dir, CtImporterInterface* importer);

}

struct CtStatusBar;

namespace CtXML {
xmlpp::Element* codebox_to_xml(xmlpp::Element* parent, const Glib::ustring& justification, int char_offset, int frame_width, int frame_height, int width_in_pixels, const Glib::ustring& syntax_highlighting, bool highlight_brackets, bool show_line_numbers);
xmlpp::Element* table_to_xml(const std::vector<std::vector<std::string>> &matrix, xmlpp::Element *parent, int char_offset, Glib::ustring justification, int col_min, int col_max);
xmlpp::Element* image_to_xml(xmlpp::Element* parent, const std::string& path, int char_offset, const Glib::ustring& justification, CtStatusBar* status_bar = nullptr);

} // CtXML

/**
 * @class CtImportException
 * @brief Thrown when an exception occures during importing
 */
class CtImportException: public std::runtime_error {
public:
    explicit CtImportException(const std::string& msg) : std::runtime_error("[Import Exception]: " + msg) {}
};



// pygtk: HTMLHandler
class CtConfig;
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
    CtHtml2Xml(CtConfig* config);


public:
    // virtuals of CtHtmlParser
    virtual void feed(const std::string& html);
    virtual void handle_starttag(std::string_view tag, const char** atts);
    virtual void handle_endtag(std::string_view tag);
    virtual void handle_data(std::string_view text);
    virtual void handle_charref(std::string_view name);

public:
    void set_local_dir(const std::string& dir)    { _local_dir = dir; }
    void set_outter_xml_doc(xmlpp::Document* doc) { _outter_doc = doc; }
    void set_status_bar(CtStatusBar* status_bar);

    Glib::ustring to_string() { return _xml_doc->write_to_string(); }


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
    CtConfig*             _config {nullptr};
    CtStatusBar*          _status_bar {nullptr};
    std::string           _local_dir;

    // releated to parsing html
    ParserState           _state;
    int                   _tag_id_generator;
    std::list<tag_style>  _tag_styles;
    bool                  _html_pre_tag_open;
    bool                  _html_td_tag_open;
    bool                  _parsing_valid_tag;
    int                   _html_a_tag_counter;
    char                  _list_type;
    int                   _list_num;
    int                   _list_level;
    std::list<std::list<table_cell>> _table;

    // related to generating xml
    xmlpp::Document        _temp_doc;
    xmlpp::Document*       _outter_doc {nullptr};
    xmlpp::Document*       _xml_doc {nullptr};
    xmlpp::Element*        _slot_root;
    int                    _char_offset;
    std::string            _slot_text;
    int                    _slot_style_id;
    std::list<slot_styles> _slot_styles_cache;
};


class CtHtmlImport : public CtImporterInterface
{
public:
    CtHtmlImport(CtConfig* config);

    // virtuals of CtImporterInterface
    std::unique_ptr<ct_imported_node> import_file(const fs::path& file) override;
    std::vector<std::string>          file_mimes() override { return {"text/html"}; };

private:
    CtConfig* _config;
};


class CtTomboyImport : public CtImporterInterface
{
public:
    CtTomboyImport(CtConfig* config);

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<ct_imported_node> import_file(const fs::path& file) override;

private:
    void            _iterate_tomboy_note(xmlpp::Element* iter, std::unique_ptr<ct_imported_node>& node);
    xmlpp::Element* _rich_text_serialize(const Glib::ustring& text_data);

private:
    CtConfig*       _config;

    xmlpp::Element* _current_node;
    int             _chars_counter;
    bool            _is_list_item;
    bool            _is_link_to_node;
    std::map<std::string, std::string> _curr_attributes;
};



/**
 * @brief Import handler for Zim Wiki directories
 * @class CtZimImport
 */
class CtZimImport: public CtTextParser, public CtImporterInterface
{
public:
    CtZimImport(CtConfig* config);

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<ct_imported_node> import_file(const fs::path& file) override;

protected:
    // virtuals of CtTextParser
    void _init_tokens() override;
    void feed(std::istream& data) override;

private:
    void _parse_body_line(const std::string& line);
    void _ensure_notebook_file_in_dir(const fs::path& dir);

private:
    bool              _has_notebook_file {false};
    ct_imported_node* _current_node;
};



class CtPlainTextImport: public CtImporterInterface
{
public:
    CtPlainTextImport(CtConfig*) {}

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<ct_imported_node> import_file(const fs::path& file) override;
    std::vector<std::string>          file_mimes() override { return {"text/plain"}; };
};


/**
 * @brief Markdown parser
 * @class CtMDParser
 */
class CtMDParser: public CtTextParser
{
protected:

    void _init_tokens() override;
    
    const token_schema* _last_encountered_token = nullptr;
    std::ostringstream _free_text;
    
    void _place_free_text();
    void _add_scale_to_last(int level);
    void _add_table_cell(std::string text);
    /// Add the current table row to the table
    void _pop_table_row();
    /// Add the current table to the output xml
    void _pop_table();
    
    using TableRow = std::vector<std::string>;
    using TableMatrix = std::vector<TableRow>;
    TableRow _current_table_row;
    TableMatrix _current_table;
public:
    CtMDParser(CtConfig* config) : CtParser(config), CtTextParser(config) {}

    void feed(std::istream& stream) override;

};


class CtMDImport: public CtImporterInterface
{
public:
    CtMDImport(CtConfig* config);

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<ct_imported_node> import_file(const fs::path& file) override;
    std::vector<std::string>          file_mimes() override { return {"text/plain"}; };
    std::vector<std::string>          file_patterns() override { return {"*.md"}; };

private:
    CtMDParser _parser;
};


class CtPandocImport: public CtImporterInterface
{
public:
    CtPandocImport(CtConfig* config);

public:
    // virtuals of CtImporterInterface
    std::unique_ptr<ct_imported_node> import_file(const fs::path& file) override;

private:
    CtConfig* _config;
};
