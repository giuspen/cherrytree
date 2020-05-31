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

#include <utility>
#include <vector>
#include <list>
#include <set>
#include <glibmm/ustring.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml++/libxml++.h>
#include <filesystem>
#include <unordered_set>
#include <fstream>
#include <unordered_map>
#include <functional>

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
    bool                  _parsing_valid_tag;
    int                   _html_a_tag_counter;
    char                  _list_type;
    int                   _list_num;
    int                   _list_level;
    std::list<std::list<table_cell>> _table;

    // related to generating xml
    xmlpp::Document        _xml_doc;
    int                    _char_offset;
    xmlpp::Element*        _slot_root;
    std::string            _slot_text;
    int                    _slot_style_id;
    std::list<slot_styles> _slot_styles_cache;
};

/**
 * @class CtImportException
 * @brief Thrown when an exception occures during importing
 */
class CtImportException: public std::runtime_error {
    static constexpr std::string_view signature = "[Import Exception]: ";
public:
    explicit CtImportException(const std::string& msg) : std::runtime_error("[Import Exception]: " + msg) {}
};
class CtConfig;
class CtImportHandler;

/**
 * @brief A file imported by a CtImportHandler
 * @class CtImportFile
 */
class CtImportFile 
{
public:
    friend class CtImportHandler;
    
    std::filesystem::path path;
    uint32_t depth = 0;
    
    bool operator==(const CtImportFile& other) const 
    {
        return (path == other.path) && (depth == other.depth);
    }
    bool operator>(const CtImportFile& other) const { return depth > other.depth; }
    bool operator<(const CtImportFile& other) const { return depth < other.depth; }
    
    [[nodiscard]] std::ifstream file() const { return std::ifstream(path); }
    
    std::string to_string() 
    {
        if (!doc) throw std::logic_error("to_string called on CtImportFile without a valid document");
        return doc->write_to_string();
    }
    /**
     * @brief Fix the internal links for the file
     * Adds the link id to the proper link in the document
     * @param node_name
     * @param node_id
     */
    void fix_internal_links(const std::string& node_name, uint64_t node_id);
    
    std::shared_ptr<xmlpp::Document> doc;
    CtImportFile* parent = nullptr;
    std::vector<std::shared_ptr<CtImportFile>> children;
private:
    explicit CtImportFile(std::filesystem::path p, uint32_t rec_depth = 0) : path(std::move(p)), depth(rec_depth) {}
    
    /// Map of internal links for fixing
    std::unordered_map<std::string, std::vector<xmlpp::Element*>> _internal_links;
    
};

/**
 * @brief Base class for parsers
 * @class CtParser
 */
class CtParser
{
public:
    using tags_map_t = std::unordered_map<std::string_view, std::vector<const token_schema *>>;
protected:
    struct token_schema {
        
        std::string open_tag;
        
        bool has_closetag   = true;
        bool is_symmetrical = true;
        
        std::function<void(const std::string&)> action;
        
        std::string close_tag = "";
        
        /// Whether to capture all the tokens inside it without formatting
        bool capture_all = false;
        
        
        std::string data = "";
        
        bool operator==(const token_schema& other) const
        {
            if (is_symmetrical || !has_closetag) return open_tag == other.open_tag;
            else                                 return (open_tag == other.open_tag) && (close_tag == other.close_tag);
        }
    };
    
    enum class PARSING_STATE
    {
        HEAD,
        BODY
    } _parse_state = PARSING_STATE::HEAD;
    
    enum class CHECKBOX_STATE
    {
        UNCHECKED = 0,
        TICKED = 1,
        MARKED = 2
    };
    
    // XML generation
    xmlpp::Element* _current_element = nullptr;
    std::unique_ptr<xmlpp::Document> _document{std::make_unique<xmlpp::Document>()};
    std::unordered_map<std::string_view, bool> _open_tags;
    
    
    void _add_scale_tag(int level, std::optional<std::string> data);
    void _add_weight_tag(const Glib::ustring& level, std::optional<std::string> data);
    void _add_italic_tag(std::optional<std::string> data);
    void _add_strikethrough_tag(std::optional<std::string> data);
    void _add_list(uint8_t level, const std::string& data);
    void _add_ordered_list(unsigned int level, const std::string &data);
    void _add_todo_list(CHECKBOX_STATE state, const std::string& data);
    /// Add a link, text should contain the full qualified name (e.g https://example.com)
    void _add_link(const std::string& text);
    void _add_superscript_tag(std::optional<std::string> text);
    void _add_subscript_tag(std::optional<std::string> text);
    void _add_text(std::string text, bool close_tag = true);
    void _close_current_tag();
    void _add_newline();
    void _add_tag_data(std::string_view, std::string data);
    
    [[nodiscard]] constexpr bool _tag_empty() const
    {
        if (!_current_element) return true;
        else                   return !_current_element->get_child_text();
    }
    
    virtual std::vector<std::pair<const token_schema *, std::string>> _tokenize(const std::string& stream) const = 0;
    
    /**
     * @brief Initalise _tokens_schemas with the tokens map
     * 
     */
    virtual void _init_tokens() = 0;
    
    
    const CtConfig* _pCtConfig = nullptr;
    
    /// Tokens to be cached by the parser
    std::vector<token_schema> _token_schemas;
    
private:
    tags_map_t _open_tokens_map;
    tags_map_t _close_tokens_map;
    
    void _build_token_maps();
public:
    explicit CtParser(const CtConfig* pCtConfig) : _pCtConfig(pCtConfig) {}
    
    virtual void feed(std::istream& data) = 0;
    
    std::string to_string() { return _document->write_to_string(); }

    void wipe();
    
    const tags_map_t& open_tokens_map()
    {
        _build_token_maps();
        return _open_tokens_map;
    };
    const tags_map_t& close_tokens_map()
    {
        _build_token_maps();
        return _close_tokens_map;
    }
    
};

/**
 * @brief Base class/interface for import handlers
 * @class CtImportHandler
 */
class CtImportHandler: public virtual CtParser
{
public:
    using token_map_t = std::unordered_map<std::string_view, std::function<void(const std::string&)>>;
    

protected:
    // XML generation
    std::vector<std::shared_ptr<xmlpp::Document>> _docs;
    std::vector<std::shared_ptr<CtImportFile>> _import_files;
    bool                      _processed_files = false;
    
    
    void _add_internal_link(const std::string& text);
    /**
     * @brief Create and setup a new document to be fed into
     * 
     */
    void _init_new_doc();


    [[nodiscard]] const std::shared_ptr<xmlpp::Document>& _xml_doc() const { return _docs.back(); }
    
    [[nodiscard]] std::shared_ptr<CtImportFile>& _current_import_file() { return _import_files.at(_docs.size() - 1); }
    
    
    static std::unique_ptr<CtImportFile> _new_import_file(std::filesystem::path path, uint32_t depth) { return std::unique_ptr<CtImportFile>(new CtImportFile(std::move(path), depth)); }
    static void _add_internal_link_to_file(CtImportFile& file, std::string link_name, xmlpp::Element* element) { file._internal_links[link_name].emplace_back(element); }
    void _add_internal_link_to_curr_file(std::string link_name, xmlpp::Element* element) { _add_internal_link_to_file(*_current_import_file(), std::move(link_name), element); }
    
    
    /**
     * @brief Get a list of file extensions supported by the import handler
     * @return
     */
    [[nodiscard]] virtual const std::unordered_set<std::string>& _get_accepted_file_extensions() const = 0;

public:
    using CtParser::CtParser;
    
    std::vector<std::shared_ptr<CtImportFile>>& imported_files() { return _import_files; }

    virtual void add_directory(const std::filesystem::path& path) = 0;
};

/**
 * @brief Base class for parsing text data
 * @class CtTextParser
 */
class CtTextParser: public virtual CtParser
{
protected:
    /**
     * @brief Transform an input string into a token stream
     * @param stream
     * @return
     */
    std::vector<std::pair<const token_schema *, std::string>> _tokenize(const std::string& stream) const override;
    uint8_t _list_level = 0;
    
public:
    using CtParser::CtParser;
    
};

/**
 * @brief Import handler for Zim Wiki directories
 * @class CtZimImportHandler
 */
class CtZimImportHandler: public CtImportHandler, public CtTextParser
{
private:
    bool _has_notebook_file();
    
    void _parse_body_line(const std::string& line);

protected:
    void _init_tokens() override;
    [[nodiscard]] const std::unordered_set<std::string>& _get_accepted_file_extensions() const override;
    /**
    * @brief Process the files to import based on the import list
    * @param path
    */
    void _process_files(const std::filesystem::path& path);
    
    
    std::vector<std::shared_ptr<CtImportFile>> _get_files(const std::filesystem::path& path, uint32_t current_depth, CtImportFile* parent);
public:
    using CtImportHandler::CtImportHandler;

    void feed(std::istream& data) override;
    
    void add_directory(const std::filesystem::path& path) override;

};

/**
 * @brief Markdown parser
 * @class CtMDParser
 */
class CtMDParser: public CtTextParser
{
protected:

    void _init_tokens() override;
    
    bool _in_link = false;
public:
    using CtTextParser::CtTextParser;

    void feed(std::istream& stream) override;

};



