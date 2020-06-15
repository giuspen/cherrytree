/*
 * ct_parser.h
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
#include <gtkmm.h>
#include <filesystem>
#include <unordered_set>
#include <fstream>
#include <unordered_map>
#include <functional>


/**
 * @class CtParseError
 * @brief Thrown when an exception occures during parsing
 */
class CtParseError: public std::runtime_error {
public:
    explicit CtParseError(const std::string& msg) : std::runtime_error("[Parse Exception]: " + msg) {}
};


/**
 * @brief Base class for parsers
 * @class CtParser
 */
class CtConfig;
class CtParser
{

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
    xmlpp::Element* _last_element = nullptr;
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
    void _add_monospace_tag(std::optional<std::string> text);
    void _add_tag_data(std::string_view, std::string data);
    void _add_codebox(const std::string& language, const std::string& text);
    void _add_table(const std::vector<std::vector<std::string>>& table_matrix);
    void _add_image(const std::string& path) noexcept;
    
    [[nodiscard]] constexpr bool _tag_empty() const
    {
        if (!_current_element) return true;
        else                   return !_current_element->get_child_text();
    }
    
    virtual std::vector<std::pair<const token_schema *, std::string>> _parse_tokens(const std::vector<std::string>& tokens) const = 0;
    
    /**
     * @brief Initalise _tokens_schemas with the tokens map
     * 
     */
    virtual void _init_tokens() = 0;
    
    
    const CtConfig* _pCtConfig = nullptr;


private:
    using tags_map_t = std::unordered_map<std::string_view, const token_schema *>;
    tags_map_t _open_tokens_map;
    tags_map_t _close_tokens_map;
    
protected:
    /// Tokens to be cached by the parser
    std::vector<token_schema> _token_schemas;
    
    void _build_token_maps();
    
public:
    explicit CtParser(const CtConfig* pCtConfig) : _pCtConfig(pCtConfig) {}
    
    virtual void feed(std::istream& data) = 0;
    
    std::string to_string() { return _document->write_to_string(); }
    xmlpp::Node*  get_root_node() { return _document->get_root_node(); }

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
    const tags_map_t& open_tokens_map() const { return _open_tokens_map; }
    const tags_map_t& close_tokens_map() const { return _close_tokens_map; }
    
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
     * @param tokens
     * @return
     */
    std::vector<std::pair<const token_schema *, std::string>> _parse_tokens(const std::vector<std::string>& tokens) const override;
    uint8_t _list_level = 0;

    std::vector<std::string> _tokenize(const std::string& text);

private:
    std::unordered_map<char, std::vector<std::string>> _possible_tokens;

public:
    using CtParser::CtParser;

    /**
     * @brief Find the formatting boundries for a word based on stored tags
     * @param word_end
     * @return
     */
    std::pair<Gtk::TextIter, Gtk::TextIter> find_formatting_boundaries(Gtk::TextIter start_bounds, Gtk::TextIter word_end);
};




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
    static std::list<html_attr> char2list_attrs(const char** atts);
};
