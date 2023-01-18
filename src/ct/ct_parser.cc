/*
 * ct_parser.cc
 *
 * Copyright 2009-2023
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

#include "ct_parser.h"
#include "ct_const.h"
#include "ct_config.h"
#include "ct_misc_utils.h"
#include "ct_imports.h"
#include "ct_logging.h"
#include "ct_clipboard.h"
#include "ct_main_win.h"
#include "ct_storage_xml.h"

void CtDocumentBuilder::wipe()
{
    _document = std::make_shared<xmlpp::Document>();
    _current_element = _build_root_el();
    _currOffset = 0;
}

xmlpp::Element* CtDocumentBuilder::_build_root_el()
{
    return _document->create_root_node("root")->add_child("slot")->add_child("rich_text");
}

CtDocumentBuilder::CtDocumentBuilder(const CtConfig* pCtConfig)
 : _pCtConfig{pCtConfig}
 , _current_element{_document->create_root_node("root")->add_child("slot")->add_child("rich_text")}
{}

void CtDocumentBuilder::add_image(const std::string& path) noexcept
{
    try {
        CtXML::image_to_xml(_current_element->get_parent(), path, _currOffset, CtConst::TAG_PROP_VAL_LEFT);
        close_current_tag();
        ++_currOffset;
    } catch(std::exception& e) {
        spdlog::error("Exception occured while adding image: {}", e.what());
    }
}

void CtDocumentBuilder::add_broken_link(const std::string& link)
{
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
    xmlpp::Element* p_codebox_node = CtXML::codebox_to_xml(_current_element->get_parent(), CtConst::TAG_PROP_VAL_LEFT, _currOffset, _pCtConfig->codeboxWidth, _pCtConfig->codeboxHeight, true, language, false, false);
    p_codebox_node->add_child_text(text);
    close_current_tag();
    ++_currOffset;
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
        _currOffset += text.size();
        //spdlog::debug("_currOffset {} '{}'", _currOffset, text);
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

void CtDocumentBuilder::with_last_element(const std::function<void()>& func)
{
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

void CtDocumentBuilder::add_hrule()
{
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

void CtDocumentBuilder::add_table(const std::vector<std::vector<Glib::ustring>>& table_matrix)
{
    const bool is_light = table_matrix.size() > 0 and table_matrix.size() * table_matrix.front().size() > _pCtConfig->tableCellsGoLight;
    CtXmlHelper::table_to_xml(_current_element->get_parent(), table_matrix, _currOffset, CtConst::TAG_PROP_VAL_LEFT, _pCtConfig->tableColWidthDefault, "", is_light);
    close_current_tag();
    ++_currOffset;
}

#ifdef MD_AUTO_REPLACEMENT
CtMarkdownFilter::CtMarkdownFilter(std::unique_ptr<CtClipboard> clipboard, Glib::RefPtr<Gtk::TextBuffer> buff, CtConfig* config)
 : _config(config)
 , _clipboard(std::move(clipboard))
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

void CtMarkdownFilter::_markdown_insert()
{
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

            _md_parser->feed(raw_token);

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

void CtMarkdownFilter::_apply_tag(const Glib::ustring& tag, const Gtk::TextIter& start, const Gtk::TextIter& end)
{
    spdlog::debug("Applying tag: {}", tag);
    bool has_tag = static_cast<bool>(_buffer->get_tag_table()->lookup(tag));
    if (!has_tag) {
        // create if not exists
        _buffer->create_tag(tag);
    }
    _buffer->apply_tag_by_name(tag, start, end);
}

std::string CtMarkdownFilter::_get_new_md_tag_name() const
{
    return fmt::format("md-formatting-{}", _md_matchers.size());
}

bool CtMarkdownFilter::active() const noexcept
{
    return _active && _config->enableMdFormatting;
}
#endif // MD_AUTO_REPLACEMENT

namespace {

std::vector<CtMempadParser::page> parse_mempad_strings(const std::vector<std::string>& mem_strs)
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

} // namespace (anonymous)

void CtMempadParser::feed(const std::string& data)
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
    std::vector<std::string> strings;
    size_t prev_pos{0};
    do {
        const auto found = data.find('\0', prev_pos);
        if (std::string::npos == found) {
            strings.push_back(data.substr(prev_pos));
            break;
        }
        strings.push_back(data.substr(prev_pos, found - prev_pos));
        prev_pos = found + 1;
    } while (prev_pos < data.size());

    std::vector<page> new_pages = parse_mempad_strings(strings);
    _parsed_pages.insert(_parsed_pages.cend(), new_pages.begin(), new_pages.end());
}

void CtTreepadParser::feed(const std::string& data)
{
    Glib::RefPtr<Glib::Regex> rRegExpInteger = Glib::Regex::create("\\d+");
    for (auto& lineStr : str::split(data, CtConst::CHAR_NEWLINE)) {
        size_t currSize = lineStr.size();
        if (currSize > 0 and lineStr.at(currSize - 1) == '\r') {
            lineStr.erase(currSize - 1);
        }
        switch (_currState) {
            case States::CollectingNodeContent: {
                if (str::startswith(lineStr, "<end node>")) {
                    _currState = States::WaitingForNodeTag;
                }
                else {
                    _parsed_pages.back().contents += lineStr + '\n';
                }
            } break;
            case States::WaitingForNodeTag: {
                if (str::startswith(lineStr, "<node>")) {
                    _currState = States::WaitingForNodeName;
                }
            } break;
            case States::WaitingForNodeName: {
                _parsed_pages.emplace_back(CtMempadParser::page{
                    .level = 0,
                    .name = lineStr,
                    .contents = std::string{}
                });
                _currState = States::WaitingForNodeLevel;
            } break;
            case States::WaitingForNodeLevel: {
                if (rRegExpInteger->match(lineStr)) {
                    _parsed_pages.back().level = std::stoi(lineStr);
                    _currState = States::CollectingNodeContent;
                }
                else {
                    _parsed_pages.back().name += " " + lineStr;
                }
            } break;
        }
    }
}

CtZimParser::CtZimParser(CtConfig* config)
 : CtDocBuildingParser{config}
 , _text_parser{std::make_shared<CtTextParser>(_token_schemas())}
{}

void CtZimParser::feed(const std::string& data)
{
    bool found_header{false};
    try {
        for (const auto& line : str::split(data, CtConst::CHAR_NEWLINE)) {
            if (not found_header) {
                // Creation-Date: .* is the final line of the header
                if (line.find("Creation-Date:") != std::string::npos) {
                    found_header = true;
                }
            }
            else {
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
                // false alarm, it's just a plain text separator
                doc_builder().add_text(data);
                return;
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

std::pair<std::string, Glib::ustring> read_leo_vnode(const xmlpp::Element& v_el)
{
    assert(v_el.get_name() == "v");

    std::string tx_id = v_el.get_attribute("t")->get_value();
    if (auto* vh_el = dynamic_cast<const xmlpp::Element*>(v_el.get_first_child())) {
        Glib::ustring contents = vh_el->get_child_text()->get_content();
        return {tx_id, contents};
    } else {
        return {tx_id, ""};
    }
}

void find_node_children(const xmlpp::Element& v_el, children_lookup_t& children)
{
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

void build_tx_lookup_table(const xmlpp::Node& vnode, name_lookup_table_t& table)
{
    for (auto* node : vnode.get_children("v")) {
        if (auto* el = dynamic_cast<xmlpp::Element*>(node)) {
            table.emplace(read_leo_vnode(*el));
            build_tx_lookup_table(*el, table);
        }
    }
}

name_lookup_table_t generate_tx_lookup_table(const xmlpp::Node& vnode_root)
{
    assert(vnode_root.get_name() == "vnodes");

    name_lookup_table_t tx_id_to_names;
    build_tx_lookup_table(vnode_root, tx_id_to_names);

    return tx_id_to_names;
}

children_lookup_t generate_children_lookup_table(const xmlpp::Node& vnode_root)
{
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

xmlpp::Node* find_leo_vnodes_el(const xmlpp::Node& root)
{
    return root.get_children("vnodes").front();
}

xmlpp::Node* find_leo_tnodes_el(const xmlpp::Node& root)
{
    return root.get_children("tnodes").front();
}

std::vector<CtLeoParser::leo_node> parse_leo_tree(const xmlpp::Node& vnode_root, xmlpp::Node& tnode_root)
{
    auto l_nodes = parse_leo_tnodes(tnode_root);
    populate_leo_nodes(vnode_root, l_nodes);

    std::vector<CtLeoParser::leo_node> populated_nodes;
    for (auto& node : l_nodes) {
        populated_nodes.emplace_back(std::move(node.second));
    }

    return populated_nodes;
}

std::vector<CtLeoParser::leo_node> walk_leo_xml(const xmlpp::Node& root)
{
    auto* vnode = find_leo_vnodes_el(root);
    auto* tnode = find_leo_tnodes_el(root);

    if (!vnode || !tnode) throw std::runtime_error("Leo XML is malformed");

    return parse_leo_tree(*vnode, *tnode);
}

} // namespace (anonymous)

void CtLeoParser::feed(const std::string& in)
{
    xmlpp::DomParser p;
    p.parse_memory(in);

    xmlpp::Element* root = p.get_document()->get_root_node();

    auto new_nodes = walk_leo_xml(*root);
    _leo_nodes.insert(_leo_nodes.cend(), new_nodes.begin(), new_nodes.end());
}
