/*
 * ct_parser_text.cc
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

#include "ct_parser.h"
#include "ct_logging.h"

namespace {

template<class ITER_T>
bool do_token_branch(ITER_T begin, ITER_T end, std::string_view match) {
    std::string buff;
    while ((begin != end) && (buff.size() < match.size())) {
        buff += *begin;
        if (buff == match) {
            return true;
        }
        ++begin;
    }
    return false;
}

template<class ITER_T, class STR_T>
std::optional<STR_T> branch_token(ITER_T begin, ITER_T end, const std::vector<STR_T>& options) {
    std::size_t largest_len = 0;
    std::optional<STR_T> largest = std::nullopt;
    for (const auto& opt : options) {
        if (do_token_branch(begin, end, opt)) {
            // Do a greedy match
            if (opt.length() > largest_len) {
                largest_len = opt.length();
                largest = opt;
            }
        }
    }
    return largest;
}

void build_pos_tokens(const CtTextParser::tags_map_t& in, CtTextParser::pos_tokens_t& out) {
    for (const auto& token : in) {
        if (!token.first.empty()) {
            out[token.first.front()].emplace_back(token.first);
        }
    }
}

CtTextParser::pos_tokens_t build_pos_tokens(const CtTextParser::tags_map_t& open_tokens, const CtTextParser::tags_map_t& close_tokens) {

    CtTextParser::pos_tokens_t pos_tokens;
    build_pos_tokens(open_tokens, pos_tokens);
    build_pos_tokens(close_tokens, pos_tokens);

    return pos_tokens;
}

CtTextParser::tags_map_t build_tags_map(const std::vector<CtTextParser::token_schema>& tokens, bool open_tags_map) {
    CtTextParser::tags_map_t tags_map;
    for (const auto& token : tokens) {
        std::string_view key = (open_tags_map || token.is_symmetrical) ? token.open_tag : token.close_tag;
        tags_map[key] = &token;
    }
    return tags_map;
}

template<typename ITER>
void feed_tags(ITER start, ITER end, CtTokenMatcher::pos_tokens_t& to) {
    static const std::unordered_set<std::string_view> ignored_tags = {"|", "|\n", "\n==", "\n----"};
    while (start != end) {
        if (ignored_tags.find(*start) == ignored_tags.end() && !start->empty()) {
            to.emplace_back(*start);
        }
        ++start;
    }
}
}


CtTextParser::CtTextParser(std::vector<token_schema>&& token_schemas) : _token_schemas{std::move(token_schemas)}, _open_tokens_map{build_tags_map(_token_schemas, true)},
    _close_tokens_map{build_tags_map(_token_schemas, false)}, _possible_tokens{build_pos_tokens(_open_tokens_map, _close_tokens_map)}  {}



std::vector<std::string> CtTextParser::tokenize(const std::string& text) const
{

    std::vector<std::string> tokens;
    std::string::const_iterator last_pos = text.begin();
    for (auto ch = text.begin(); ch != text.end(); ++ch) {

        if (*ch == ' ') {
            if (last_pos != ch) tokens.emplace_back(last_pos, ch);
            last_pos = ch;
            continue;
        }
        if (*ch == '\\') {
            // Escape next char
            if (last_pos != ch) tokens.emplace_back(last_pos, ch);
            ++ch;
            last_pos = ch;
            if (ch == text.end()) break;
            continue;
        }

        auto pos_token = _possible_tokens.find(*ch);
        if (pos_token != _possible_tokens.end()){
            auto found_token = branch_token(ch, text.end(), pos_token->second);
            if (found_token) {
                spdlog::debug("TOKEN: {}", *found_token);
                tokens.emplace_back(last_pos, ch);
                tokens.emplace_back(*found_token);
                ch += found_token->length() - 1;
                last_pos = ch + 1;
            }

            if (ch == text.end()) break;
        }
    }
    if (last_pos != text.end()) {
        tokens.emplace_back(last_pos, text.end());
    }
    return tokens;
}

std::vector<std::pair<const CtTextParser::token_schema *, std::string>> CtTextParser::parse_tokens(const std::vector<std::string>& tokens) const
{
    std::vector<std::pair<const token_schema *, std::string>> token_stream;
    std::unordered_map<std::string_view, bool>                open_tags;
    std::pair<std::vector<const token_schema *>, std::string> curr_open_tags;
    bool                                                      keep_parsing     = true;
    auto                                                      &token_map_open  = open_tokens_map();
    auto                                                      &token_map_close = close_tokens_map();
    int  nb_open_tags = 0;

    for (auto token = tokens.begin(); token != tokens.end(); ++token) {

        if (token->empty()) continue;

        auto tokens_iter = token_map_open.find(*token);
        if (tokens_iter != token_map_open.end()) {

            if (!curr_open_tags.first.empty() && !keep_parsing) {
                if (tokens_iter->second->open_tag == curr_open_tags.first.front()->open_tag) {
                    ++nb_open_tags;
                }
            }

            if (!(tokens_iter->second->is_symmetrical && open_tags[tokens_iter->first]) && keep_parsing) {
                curr_open_tags.first.emplace_back(tokens_iter->second);
                keep_parsing = !tokens_iter->second->capture_all;

                if (!tokens_iter->second->has_closetag) {
                    // Token will go till end of stream
                    ++token;
                    if (keep_parsing) {
                        // Parse the other data in the stream
                        token_stream.emplace_back(tokens_iter->second, "");
                        std::vector<std::string> rem_tokens(token, tokens.end());
                        auto tokonised_stream = parse_tokens(rem_tokens);
                        token_stream.insert(token_stream.end(), tokonised_stream.begin(), tokonised_stream.end());
                    } else {
                        std::string buff;
                        while (token != tokens.end()) {
                            buff += *token;
                            ++token;
                        }
                        token_stream.emplace_back(tokens_iter->second, buff);
                    }
                    return token_stream;
                }
                open_tags[tokens_iter->first] = true;
                continue;
            }
        }


        auto token_iter  = token_map_close.find(*token);
        if (token_iter != token_map_close.end()) {
            if (curr_open_tags.first.empty()) {
                spdlog::debug("Found close tag without open: {}, assuming escaped", *token);
                token_stream.emplace_back(nullptr, *token);
                continue;
            }

            if (!keep_parsing) {
                if (curr_open_tags.first.front()->close_tag != *token) {
                    curr_open_tags.second += *token;
                    continue;
                } else if (curr_open_tags.first.front()->close_tag == *token) {
                    if (nb_open_tags > 1) {
                        --nb_open_tags;
                        curr_open_tags.second += *token;
                        continue;
                    }
                }
            }


            token_stream.emplace_back(curr_open_tags.first.front(), curr_open_tags.second);

            if (curr_open_tags.first.size() >= 2) {
                // Found more than one tag
                for (auto iter = curr_open_tags.first.begin() + 1; iter != curr_open_tags.first.end(); ++iter) {
                    if ((*iter)->open_tag != curr_open_tags.first.front()->open_tag) token_stream.emplace_back(*iter, "");
                }
            }
            open_tags[token_iter->first] = false;
            keep_parsing = true;

            nb_open_tags = 0;
            curr_open_tags.second.clear();
            curr_open_tags.first.clear();
        } else if (curr_open_tags.first.empty()) {
            token_stream.emplace_back(nullptr, *token);
        } else if (!curr_open_tags.first.empty()) {
            curr_open_tags.second += *token;
        }
    }

    return token_stream;
}

std::unordered_set<char> shred(const std::vector<std::string>& strings) {
    std::unordered_set<char> chars;
    for (const auto& str : strings) {
        chars.insert(str.begin(), str.end());
    }
    return chars;
}

void CtTokenMatcher::pop_back() {
    if (!_finished && !_open_token.empty()) {
        if (!_found_open || _token_contents.empty()) {
            _open_token.pop_back();
            _token_buff = _open_token;
        } else {
            _token_contents.pop_back();
        }
    }
}



bool CtTokenMatcher::_is_valid_token(std::string_view token) {
    auto& open_tkns = _text_parser->open_tokens_map();
    auto& close_tkns = _text_parser->close_tokens_map();

    return ((open_tkns.find(token) != open_tkns.end()) || (close_tkns.find(token) != close_tkns.end()));
}


void CtTokenMatcher::feed(char ch) {
    bool found = false;
    auto token_pos = _text_parser->pos_tokens().find(ch);
    if (token_pos != _text_parser->pos_tokens().end()) {
        if (_pos_tokens.empty()) {
            feed_tags(token_pos->second.begin(), token_pos->second.end(), _pos_tokens);
        }

        if (_pos_chars.empty()) _pos_chars = shred(_pos_tokens);

        if (_pos_chars.find(ch) != _pos_chars.end()) {
            _token_buff += ch;
            found = true;
        } else if (!_found_open) {
            // A different tag before found a full open, switch to it
            if (!_token_buff.empty()) {
                // Attempt a match beginning with the last character (e.g \n-_ becomes -_ and matches)
                char prev_char = _token_buff.front();
                _token_buff.clear();
                _pos_tokens.clear();
                _pos_chars.clear();

                std::string token(1, prev_char);
                token += ch;
                if (_is_valid_token(token)) {
                    feed(prev_char);
                    feed(ch);
                    return;
                }
            }
            _token_buff.clear();
            _token_buff += ch;
            found = true;
            _pos_tokens.clear();
            feed_tags(token_pos->second.begin(), token_pos->second.end(), _pos_tokens);

            _pos_chars.clear();
        }

    }

    _update_tokens();

    if (_found_open && !found) _token_contents += ch;
}

#ifndef NDEBUG // Mimic behaviour of assert(), doesnt abort in rel builds
constexpr void require(bool expr, std::string_view msg) {
    if (!expr) {
        spdlog::error("Requirement failed: <{}>", msg);
        std::abort();
    }
}
#else
constexpr void require(bool /*expr*/, std::string_view /*msg*/) {}
#endif

void CtTokenMatcher::erase(int offset) {
    require(offset >= 0, "TokenMatcher::erase: offset >= 0");
    if (!finished()) {
        int lr_offset = static_cast<int>(raw_str().size() - offset);
        if (lr_offset < 0) throw std::logic_error("lr_offset < 0, this is invalid and should not have happened");

        if (offset == 0) pop_back();
        else if (offset >= static_cast<int>(contents_end_offset()) && offset <= static_cast<int>(contents_start_offset())) {
            // Contents
            spdlog::debug("Erase contents");
            int erase_offset = lr_offset - static_cast<int>(_open_token.size());
            spdlog::debug("Erased <{}>", std::string(_token_contents.begin() + erase_offset, _token_contents.begin() + erase_offset + 1));
            _token_contents.erase(_token_contents.begin() + erase_offset, _token_contents.begin() + erase_offset + 1);
        }
        else if (offset >= static_cast<int>(raw_end_offset() + _token_contents.size()) && lr_offset < static_cast<int>(_open_token.size())) {
            // Open
            spdlog::debug("Erase open");
            _open_token.erase(_open_token.begin() + lr_offset, _open_token.begin() + lr_offset + 1);

            // Update tokens
            _token_buff = _open_token;
            _found_open = false;
            _update_tokens();
        }
        else if (offset >= static_cast<int>((_open_token.size() + _token_contents.size()))) {
            // Close
            spdlog::debug("Erase close");
            int erase_offset = static_cast<int>(_close_token.size()) - offset;
            if (erase_offset >= 0) {
                _close_token.erase(_close_token.begin() + erase_offset, _close_token.begin() + erase_offset + 1);
                _token_buff = _close_token;
                _update_tokens();
            } else {
                spdlog::warn("Got an invalid erase_offset: <{}>", erase_offset);
            }
        }
    }
}

void CtTokenMatcher::_reset_and_refeed() {
    std::string raw = raw_str();
    reset();

    for (char ch : raw) {
        feed(ch);
    }
}

void CtTokenMatcher::reset()
{
    _token_buff.clear();
    _pos_tokens.clear();
    _pos_chars.clear();
    _open_token.clear();
    _token_contents.clear();
    _close_token.clear();
    _found_open = false;
    _finished = false;
}

void CtTokenMatcher::insert(char ch, int offset) {
    require(offset >= 0, "TokenMatcher::insert: offset >= 0");
    if (!finished()) {
        std::string raw = raw_str();
        int lr_offset = static_cast<int>(raw.size()) - offset;
        if (offset == 0) feed(ch);
        else if (offset >= static_cast<int>(contents_start_offset())) {
            // Open token
            spdlog::trace("Insert open_token");
            _open_token.insert(_open_token.begin() + lr_offset, ch);

            _reset_and_refeed();
        }
        else if (offset <= static_cast<int>(contents_end_offset())) {
            // Close token
            spdlog::trace("Insert close token");
            _close_token.insert(_close_token.begin() + (_close_token.size() - offset), ch);

            _reset_and_refeed();
        } else if (offset > static_cast<int>(contents_end_offset()) && offset < static_cast<int>(contents_start_offset())) {
            // Edit to contents, no need to reset
            spdlog::trace("Insert contents");
            _token_contents.insert(_token_contents.begin() + lr_offset - _open_token.size(), ch);
        }
        else {
            spdlog::warn("Invalid offset: <{}>; appending (feed()) instead", offset);
            feed(ch);
        }
    }



}

void CtTokenMatcher::_update_tokens() {
    if (!_pos_tokens.empty()) {
        auto pos_token = branch_token(_token_buff.begin(), _token_buff.end(), _pos_tokens);
        if (pos_token) {
            // Found a possible token
            if (*pos_token == _open_token && _token_contents.empty()) {
                // Repeat, it actually failed but matched the start
                _found_open = true;

                _token_buff.clear();
                for (const auto& tkn : _text_parser->token_schemas()) {
                    if (tkn.open_tag == _open_token) {
                        _pos_tokens = {(tkn.is_symmetrical ? tkn.open_tag : tkn.close_tag)};
                        break;
                    }
                }
                spdlog::debug("TOKEN MATCHER: FOUND OPEN");
                _pos_chars.clear();
            } else if (!_found_open) {
                // Open tag
                auto& open_tkns = _text_parser->open_tokens_map();
                if (open_tkns.find(_token_buff) != open_tkns.end()) {
                    _open_token = *pos_token;
                } else {
                    _token_buff.clear();
                }

            } else if (!_token_contents.empty()) {
                // Close
                auto& open_tkns = _text_parser->open_tokens_map();
                auto* open_token = open_tkns.at(_open_token);
                if (*pos_token == open_token->close_tag || (open_token->is_symmetrical && *pos_token == open_token->open_tag)) {
                    _finished = true;
                    _close_token = _token_buff;
                    spdlog::debug("Finished, open: <{}> contents: <{}> close: <{}> ", _open_token, _token_contents, _close_token);
                }
            }
        }

    }

}

CtTokenMatcher::size_type CtTokenMatcher::contents_end_offset() const
{
    if (_found_open && _close_token.empty()) return _token_buff.size();
    else                                     return _close_token.size();
}

std::pair<Gtk::TextIter, Gtk::TextIter> CtTextParser::find_formatting_boundaries(Gtk::TextIter start_bounds, Gtk::TextIter word_end) const
{

    Glib::ustring token_str(start_bounds, word_end);

    auto tokens = tokenize(token_str);
    auto& close_tags = close_tokens_map();

    // Forward match
    auto token = tokens.cbegin();
    auto& open_tags = open_tokens_map();
    auto ftoken_iter = open_tags.find(*token);
    while (ftoken_iter == open_tags.cend() && token != tokens.cend()) {
        if (!start_bounds.forward_chars(token->size())) break;
        ++token;
        if (token != tokens.cend()) ftoken_iter = open_tags.find(*token);
        else break;
    }
    if (ftoken_iter != open_tags.end()) {
        // Check for close tag
        if (!ftoken_iter->second->has_closetag) {
            return {start_bounds, word_end};
        }
    }

    // Backwards match
    auto rtoken = tokens.crbegin();
    auto rtoken_iter = close_tags.find(*rtoken);
    while (rtoken_iter == close_tags.cend() && rtoken != tokens.crend()) {
        if (!word_end.backward_chars(rtoken->size())) break;
        ++rtoken;
        if (rtoken != tokens.crend()) rtoken_iter = close_tags.find(*rtoken);
        else break;
    }
    if (rtoken_iter == close_tags.cend() && ftoken_iter == open_tags.cend()) {
        // Didn't find anything
        throw CtParseError(fmt::format("Could not find any valid tags in: {}", token_str.c_str()));
    }

    return {start_bounds, word_end};
}

