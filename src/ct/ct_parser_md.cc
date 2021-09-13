/*
 * ct_parser_md.cc
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

#include "ct_parser.h"
#include "ct_const.h"
#include "ct_misc_utils.h"
#include "ct_logging.h"
#include "ct_config.h"

CtMDParser::CtMDParser(CtConfig* config) : CtDocBuildingParser{config}, _text_parser{std::make_unique<CtTextParser>(_token_schemas())} {}

void CtMDParser::_add_scale_to_last(int level) {
    doc_builder().with_last_element([this, level]{ doc_builder().add_scale_tag(level, std::nullopt); });
}

std::vector<CtTextParser::token_schema> CtMDParser::_token_schemas()
{
    auto add_codebox = [this](const std::string& data) {
        auto data_iter = data.begin();
        while (data_iter != data.end() && *data_iter != '\n') {
            ++data_iter;
        }
        std::string text(data_iter, data.end());
        std::string lang;
        if (data_iter != data.begin()) {
            lang.append(data.begin(), data_iter);
        }
        spdlog::debug("CODEBOX: {}, lang: {}", text, lang);
        doc_builder().add_codebox(lang, text);

    };
    auto add_h3 = [this](const std::string& text) {
        doc_builder().close_current_tag();
        doc_builder().add_scale_tag(3, text + "\n");
        doc_builder().close_current_tag();
    };
    auto add_list = [this](const std::string& text) {
        doc_builder().add_list(_list_level, "");
        std::istringstream ss(text);
        feed(ss);
        doc_builder().add_newline();
        _list_level = 0;
    };

    return {
        // Bold
        {"__", true,  true,  [this](const std::string &data) {
            doc_builder().add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
        }},
        // Italic
        {"_", true, true, [this](const std::string& data){
            doc_builder().add_italic_tag(data);
        }},
        // Italic
        {"*", true, true, [this](const std::string& data){
            doc_builder().add_italic_tag(data);
        }},
        // Bold and italic
        {"***", true, true, [this](const std::string& data){
            doc_builder().add_italic_tag(data);
            doc_builder().add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, std::nullopt);
        }},
        // Bold
        {"**", true,  true,  [this](const std::string &data) {
            doc_builder().add_weight_tag(CtConst::TAG_PROP_VAL_HEAVY, data);
        }},
        // First part of a link
        {"[",  true,  false, [this](const std::string &data) {
            // Parse for end of display
            auto last_pos = data.find_last_of(']');
            if (last_pos == std::string::npos) {
                spdlog::warn("Unknown data captured: {}; printing as plaintext", data);
                doc_builder().add_text(data);
                return;
            }

            std::string title(data.begin(), data.begin() + last_pos);
            std::string url(data.begin() + last_pos + 2, data.end());

            doc_builder().add_text(title, false);
            doc_builder().add_link(url);
        }, ")", true},
        // Monospace
        {"`", true, true, [this](const std::string& data){
            doc_builder().add_monospace_tag(data);
        }, "`", true},
        // Footnote
        {"[^", true, false, [this](const std::string& data){
            // Todo: Implement footnotes
            doc_builder().add_text("[^" + data + "]");
        }, "]"},
        // Codebox(s)
        {"~~~", true, true, add_codebox, "~~~", true},
        {"```", true, true, add_codebox, "```", true},
        // List
        {"* ", true, false, add_list, "\n", true},
        // Also list
        {"- ", true, false, add_list, "\n", true},

        // Passthroughs for lists
        {" -", true, false, [](const std::string&){}, " "},
        {"-", true, true, [](const std::string&){}},
        // Strikethrough
        {"~~", true,  true,  [this](const std::string &data) {
            doc_builder().add_strikethrough_tag(data);
        }},
        // Passthrough for ``` and `
        {"``", true, true, [this](const std::string& data){ doc_builder().add_text("``" + data + "``"); }},
        // Headers (h1, h2, etc)
        {"# ",  true, false, [this](const std::string &data) {
            doc_builder().close_current_tag();
            doc_builder().add_scale_tag(1, data + "\n");
            doc_builder().close_current_tag();
        }, "\n"},
        {"## ",  true, false, [this](const std::string &data) {
            doc_builder().close_current_tag();
            doc_builder().add_scale_tag(2, data + "\n");
            doc_builder().close_current_tag();
        }, "\n"},
        {"### ",  true, false,  add_h3, "\n"},
        {"#### ",  true, false,  add_h3, "\n"},
        {"##### ",  true, false,  add_h3, "\n"},
        {"###### ",  true, false,  add_h3, "\n"},

        // H1
        {"\n==", true, false, [this](const std::string&){
            _add_scale_to_last(1);
            doc_builder().add_newline();
        }, "\n"},
        // H2
        {"\n----", true, false, [this](const std::string&){
            _add_scale_to_last(2);
            doc_builder().add_newline();
        }, "\n"},
        // Horizontal divider
        {"***\n", true, false, [this](const std::string&){
            doc_builder().add_hrule();
        }, " "},
        // Tables

        // Table row
        {"|", true, false, [this](const std::string& data){
            //spdlog::debug("Got end: {}", data);
            _add_table_cell(data);
        }, "\n"},
        // Table header divider
        {"| -", true, false, [](const std::string&){
            // Since cherrytree tables don't use headers, this is not needed
        }, "- |\n"},
        // Image link
        {"![", true, false, [this](const std::string& data){
            auto last_pos = data.find_last_of(']');
            if (last_pos == std::string::npos) {
                spdlog::warn("Image captured unknown data: <{}>; printing as plaintext", data);
                doc_builder().add_text(data);
                return;
            }

            std::string title(data.begin(), data.begin() + last_pos);
            std::string uri(data.begin() + last_pos + 2, data.end());

            doc_builder().add_text(title, false);
            doc_builder().add_image(uri);
        }, ")", true},
        // Link
        {"<", true, false, [this](const std::string& data){
            doc_builder().add_text(data, false);
            doc_builder().add_link(data);
        }, ">", true}
    };
}

void CtMDParser::_place_free_text()
{
    std::string free_txt = _free_text.str();
    if (!free_txt.empty()) {
        auto iter = free_txt.crbegin();
        for (;iter != free_txt.crend(); ++iter) {
            if (*iter == '\n') break;
        }
        std::string last_line(iter.base(), free_txt.cend());
        std::string other_txt(free_txt.cbegin(), iter.base());
        doc_builder().add_text(other_txt);
        doc_builder().add_text(last_line); // This may be needed for headers

        std::ostringstream tmp_ss;
        _free_text.swap(tmp_ss);
    }
}

void CtMDParser::feed(std::istream& stream)
{
    std::string line;
    std::ostringstream in_stream;
    in_stream << stream.rdbuf();

    // Feed the line
    try {
        auto tokens_raw = _text_parser->tokenize(in_stream.str());
        auto tokens     = _text_parser->parse_tokens(tokens_raw);

        for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
            if (iter->first) {
                _place_free_text();
                // This is needed for links with () in them
                if ((iter + 1) != tokens.end()) {
                    if (!(iter + 1)->first && ((iter + 1)->second == ")")) {
                        // Excess bracket from link
                        iter->first->action(iter->second + ")");
                        ++iter;
                        if ((iter + 1) != tokens.end()) ++iter;

                        continue;
                    }
                }

                iter->first->action(iter->second);
            } else {
                if (!iter->second.empty()) {
                    if (!_current_table.empty() && iter->second == "\n") {
                        _pop_table();
                        doc_builder().add_newline();
                    }
                    if (_current_table.empty()) {
                        _free_text.write(iter->second.c_str(), iter->second.size());
                    }
                }
            }
            _last_encountered_token = iter->first;
        }
        if (!_current_table.empty()) _pop_table();
        _place_free_text();
    } catch (std::exception& e) {
        spdlog::error("Exception while parsing line: '{}': {}", line, e.what());
    }
}

void CtMDParser::_add_table_cell(std::string text)
{
    if (!text.empty()) {
        // Parse it to see if a cell or end of row
        char last_ch = text.back();
        if (last_ch == '|') {
            // End of row
            _current_table_row.emplace_back(text.begin(), text.end() - 1);
            _pop_table_row();
        } else {
            // Just a cell
            _current_table_row.emplace_back(text);
        }
    } else {
        if (not _current_table_row.empty()) { _pop_table_row(); }
        //spdlog::warn("_add_table_cell called without text, the document may contain invalid or unknown formatting");
    }
}

void CtMDParser::_pop_table()
{
    doc_builder().add_table(_current_table);
    _current_table.clear();
}

void CtMDParser::_pop_table_row()
{
    _current_table.emplace_back(_current_table_row);
    _current_table_row.clear();
}
