/*
 * ct_parser.h
 *
 * Copyright 2009-2021
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

#pragma once

#include <utility>
#include <vector>
#include <list>
#include <set>
#include <glibmm/ustring.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml++/libxml++.h>
#include <gtkmm.h>
#include <unordered_set>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <array>
#include <string_view>
#include <string>
#include <optional>

class CtClipboard;
struct CtStatusBar;
/**
 * @class CtParseError
 * @brief Thrown when an exception occures during parsing
 */
class CtParseError : public std::runtime_error
{
public:
    explicit CtParseError(const std::string& msg) : std::runtime_error("[Parse Exception]: " + msg) {}
};

class CtParserInterface
{
public:
    virtual void feed(std::istream&) = 0;
    virtual ~CtParserInterface() = default;
};

/**
 * @brief Base class for parsers
 */
class CtConfig;
class CtDocumentBuilder
{
    using broken_links_t = std::map<Glib::ustring, std::vector<xmlpp::Element*>>;

public:
    enum class checkbox_state
    {
        unchecked = 0,
        ticked = 1,
        marked = 2
    };

    explicit CtDocumentBuilder(const CtConfig* pCtConfig);

    void add_scale_tag(int level, std::optional<std::string> data);
    void add_weight_tag(const Glib::ustring& level, std::optional<std::string> data);
    void add_italic_tag(std::optional<std::string> data);
    void add_strikethrough_tag(std::optional<std::string> data);
    void add_list(uint8_t level, const std::string& data);
    void add_ordered_list(unsigned int level, const std::string &data);
    void add_todo_list(checkbox_state state, const std::string& data);
    /// Add a link, text should contain the full qualified name (e.g https://example.com)
    void add_link(const std::string& text);
    void add_superscript_tag(std::optional<std::string> text);
    void add_subscript_tag(std::optional<std::string> text);
    void add_text(std::string text, bool close_tag = true);
    void close_current_tag();
    void add_newline();
    void add_monospace_tag(std::optional<std::string> text);
    void add_tag_data(std::string_view, std::string data);
    void add_codebox(const std::string& language, const std::string& text);
    void add_table(const std::vector<std::vector<Glib::ustring>>& table_matrix);
    void add_image(const std::string& path) noexcept;
    void add_hrule();
    void add_broken_link(const std::string& link);

    /// Rollback to last element and run func
    void with_last_element(const std::function<void()>& func);

    [[nodiscard]] constexpr bool tag_empty() const
    {
        if (!_current_element) return true;
        else                   return !_current_element->get_child_text();
    }

    std::string to_string() { return _document->write_to_string(); }
    xmlpp::Node* root_node() const { return _document->get_root_node(); }
    const broken_links_t& broken_links() const { return _broken_links; }
    std::shared_ptr<xmlpp::Document> document() const { return _document; }

    void wipe();

private:
    const CtConfig* const _pCtConfig;
    std::unordered_map<std::string_view, bool> _open_tags;

    xmlpp::Element* _build_root_el();

    // XML generation
    std::shared_ptr<xmlpp::Document> _document{std::make_unique<xmlpp::Document>()};

    xmlpp::Element* _current_element = nullptr;
    xmlpp::Element* _last_element = nullptr;
    broken_links_t _broken_links;
    int _currOffset{0};
};

/**
 * @brief Base class for parsing text data
 * @class CtTextParser
 */
class CtTextParser
{
public:
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

    using tags_map_t = std::unordered_map<std::string_view, const token_schema*>;
    using pos_tokens_t = std::unordered_map<char, std::vector<std::string_view>>;

    explicit CtTextParser(std::vector<token_schema>&& token_schemas);

    /**
     * @brief Find the formatting boundries for a word based on stored tags
     * @param word_end
     * @return
     */
    std::pair<Gtk::TextIter, Gtk::TextIter> find_formatting_boundaries(Gtk::TextIter start_bounds, Gtk::TextIter word_end) const;

    const tags_map_t& open_tokens_map() const { return _open_tokens_map; }
    const tags_map_t& close_tokens_map() const { return _close_tokens_map; }
    const std::vector<token_schema>& token_schemas() const { return _token_schemas; }
    const pos_tokens_t& pos_tokens() const { return _possible_tokens; }
     /**
     * @brief Transform an input string into a token stream
     * @param tokens
     * @return
     */
    std::vector<std::pair<const token_schema*, std::string>> parse_tokens(const std::vector<std::string>& tokens) const;

    std::vector<std::string> tokenize(const std::string& text) const;

private:
    /// Tokens to be cached by the parser
    const std::vector<token_schema> _token_schemas;

    const tags_map_t _open_tokens_map;
    const tags_map_t _close_tokens_map;

    const pos_tokens_t _possible_tokens;
};

class CtDocBuildingParser : public CtParserInterface
{
public:
    explicit CtDocBuildingParser(CtConfig* config) : _doc_builder{config} {}
    void wipe_doc() { _doc_builder.wipe(); }

    constexpr const CtDocumentBuilder& doc() const { return _doc_builder; }

    std::string xml_doc_string() { return _doc_builder.to_string(); }

protected:
    constexpr CtDocumentBuilder& doc_builder() { return _doc_builder; }

private:
    CtDocumentBuilder _doc_builder;
};

/**
 * @brief Markdown parser
 * @class CtMDParser
 */
class CtMDParser : public CtDocBuildingParser
{
public:
    explicit CtMDParser(CtConfig* config);
    CtMDParser(CtConfig* config, std::shared_ptr<CtTextParser> parser) : CtDocBuildingParser{config} , _text_parser{std::move(parser)}{}

    void feed(std::istream& stream) override;

    virtual ~CtMDParser() = default;

    std::shared_ptr<CtTextParser> text_parser() const { return _text_parser; }

private:
    void _place_free_text();
    void _add_scale_to_last(int level);
    void _add_table_cell(std::string text);
    /// Add the current table row to the table
    void _pop_table_row();
    /// Add the current table to the output xml
    void _pop_table();
    std::vector<CtTextParser::token_schema> _token_schemas();

    using TableRow = std::vector<Glib::ustring>;
    using TableMatrix = std::vector<TableRow>;
    TableRow _current_table_row;
    TableMatrix _current_table;

    const CtTextParser::token_schema* _last_encountered_token = nullptr;
    std::ostringstream _free_text;

    uint8_t _list_level = 0;
    std::shared_ptr<CtTextParser> _text_parser;
};

class CtTokenMatcher
{
    using size_type = std::string::size_type;

public:
    using pos_tokens_t = std::vector<std::string>;

    explicit CtTokenMatcher(std::shared_ptr<CtTextParser> text_parser) : _text_parser(std::move(text_parser)) {}
    /// Feed a single character to the end of the matcher
    void feed(char ch);
    void pop_back();
    /// Insert at specific position, offset is offset from front (right) of the raw token
    void insert(char ch, int offset);
    /// Erase at the specified position, offset is from the front
    void erase(int offset);

    /**
     * @brief Reset the matcher so that it can be reused for a different stream
     * @warning Does not wipe its companion CtTextParser
     */
    void reset() noexcept;

    /// If the matcher is finished then feed(), erase(), etc will have no effect until a call to reset()
    [[nodiscard]] constexpr bool finished() const noexcept { return _finished; }
    [[nodiscard]] constexpr bool has_open() const noexcept { return _found_open; }
    [[nodiscard]] size_type contents_end_offset() const noexcept;
    [[nodiscard]] size_type contents_start_offset() const noexcept { return contents_end_offset() + _token_contents.size(); }
    [[nodiscard]] size_type raw_start_offset() const noexcept { return contents_start_offset() + _open_token.size(); }
    [[nodiscard]] constexpr size_type raw_end_offset() const noexcept { return 0; }
    [[nodiscard]] const std::string& contents() const noexcept { return _token_contents; }
    [[nodiscard]] std::string raw_str() const { return _open_token + _token_contents + _close_token; }

private:
    pos_tokens_t _pos_tokens;
    std::unordered_set<char> _pos_chars;
    std::string _token_buff;
    std::string _token_contents;
    std::string _open_token;
    std::string _close_token;
    bool _found_open = false;
    bool _finished = false;
    std::shared_ptr<CtTextParser> _text_parser;

    void _update_tokens();
    bool _is_valid_token(std::string_view token);
    /// Reset the token matcher and refeed all contents (e.g to reevaluate tags)
    void _reset_and_refeed();
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
    const xmlpp::Document& doc() const { return *_xml_doc; }

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
    CtConfig*             _config{nullptr};
    CtStatusBar*          _status_bar{nullptr};
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
    xmlpp::Document*       _outter_doc{nullptr};
    xmlpp::Document*       _xml_doc{nullptr};
    xmlpp::Element*        _slot_root;
    int                    _char_offset;
    std::string            _slot_text;
    int                    _slot_style_id;
    std::list<slot_styles> _slot_styles_cache;
};

#ifdef MD_AUTO_REPLACEMENT
/**
 * @brief Watches a TextBuffer and applies markdown formatting to it
 * @class CtMarkdownFilter
 */
class CtMarkdownFilter
{
public:
    CtMarkdownFilter(std::unique_ptr<CtClipboard> clipboard, Glib::RefPtr<Gtk::TextBuffer> buffer, CtConfig* config);
    /// Reset parser state
    void reset() noexcept;

    /// Connect to a new buffer
    void buffer(Glib::RefPtr<Gtk::TextBuffer> text_buffer);

    constexpr void active(bool active) noexcept { _active = active; }
    [[nodiscard]] bool active() const noexcept;

private:
    void _on_buffer_insert(const Gtk::TextBuffer::iterator& position, const Glib::ustring& text, int bytes) noexcept;
    void _on_buffer_erase(const Gtk::TextIter& begin, const Gtk::TextIter& end) noexcept;

    void _markdown_insert();
    void _apply_tag(const Glib::ustring& tag, const Gtk::TextIter& start, const Gtk::TextIter& end);

    std::string _get_new_md_tag_name() const;

private:
    std::array<sigc::connection, 4> _buff_connections;
    bool _active = false;

    CtConfig* _config;
    Glib::RefPtr<Gtk::TextBuffer> _buffer;

    std::shared_ptr<CtTokenMatcher> _md_matcher;
    std::shared_ptr<CtMDParser> _md_parser;
    std::unique_ptr<CtClipboard> _clipboard;

    using match_pair_t = std::pair<std::shared_ptr<CtMDParser>, std::shared_ptr<CtTokenMatcher>>;
    std::unordered_map<std::string, match_pair_t> _md_matchers;
};
#endif // MD_AUTO_REPLACEMENT

class CtMempadParser : public CtParserInterface
{
public:
    struct page {
        int level;
        std::string name;
        std::string contents;
    };

    void feed(std::istream& data) override;

    const std::vector<page>& parsed_pages() const { return _parsed_pages; }

private:
    std::vector<page> _parsed_pages;
};

class CtTreepadParser : public CtParserInterface
{
public:
    void feed(std::istream& data) override;
    const std::vector<CtMempadParser::page>& parsed_pages() const { return _parsed_pages; }

private:
    enum class States {
        WaitingForNodeTag,
        WaitingForNodeName,
        WaitingForNodeLevel,
        CollectingNodeContent
    };
    States _currState{States::WaitingForNodeTag};
    std::vector<CtMempadParser::page> _parsed_pages;
};

class CtZimParser : public CtDocBuildingParser
{
public:
    explicit CtZimParser(CtConfig* config);

    void feed(std::istream& in) override;

private:
    std::vector<CtTextParser::token_schema> _token_schemas();

    void _parse_body_line(const std::string& line);

    int _list_level = 0;
    std::shared_ptr<CtTextParser> _text_parser;
};

class CtLeoParser : public CtParserInterface
{
public:
    struct leo_node {
        Glib::ustring content;
        Glib::ustring name = "";
        std::vector<leo_node> children;
    };

    void feed(std::istream& in) override;

    const std::vector<leo_node>& nodes() const { return _leo_nodes; }

private:
    std::vector<leo_node> _leo_nodes;
};

struct ct_basic_node {
    std::string name;
    std::shared_ptr<xmlpp::Document> doc = nullptr;
};

class CtRedNotebookParser : public CtParserInterface
{
public:
    using node = ct_basic_node;
    explicit CtRedNotebookParser(CtConfig* config) : _ct_config{config} {}

    void feed(std::istream& in) override;

    const std::vector<node>& nodes() const { return _nodes; }

private:
    void _feed_str(const std::string& in);
    void _add_node(std::string&& name, const std::string& contents);

    std::vector<node> _nodes;
    CtConfig* _ct_config;
};

class CtNoteCaseHTMLParser : public CtParserInterface
{
public:
    struct notecase_split_output {
        std::string note_name;
        std::string note_contents;
    };

    using node = ct_basic_node;
    explicit CtNoteCaseHTMLParser(CtConfig* config) : _ct_config{config} {}

    void feed(std::istream& input) override;

    const std::vector<node>& nodes() const { return _nodes; }

private:
    void _feed_str(const std::string& str);

    static std::vector<notecase_split_output> _split_notecase_html_nodes(const std::string& input);

    static node _generate_notecase_node(const notecase_split_output& split_node, CtConfig* config);
    template<typename ITER>
    static std::vector<notecase_split_output> _handle_notecase_split_strings(ITER begin, ITER end)
    {
        std::vector<notecase_split_output> out;
        while(begin != end) {
            notecase_split_output node{*begin, *(begin + 1)};
            out.emplace_back(std::move(node));
            if ((begin + 1) == end) break;
            begin += 2;
        }
        return out;
    }
    template<typename ITER>
    std::vector<CtNoteCaseHTMLParser::node> _generate_notecase_nodes(ITER begin, ITER end, CtConfig* config)
    {
        static_assert(std::is_same_v<typename std::iterator_traits<ITER>::value_type, notecase_split_output>, "generate_notecase_nodes provided with an invalid iterator");

        std::vector<CtNoteCaseHTMLParser::node> nodes;
        for(; begin != end; ++begin) {
            nodes.emplace_back(_generate_notecase_node(*begin, config));
        }
        return nodes;
    }

    CtConfig* _ct_config;
    std::vector<node> _nodes;
};
