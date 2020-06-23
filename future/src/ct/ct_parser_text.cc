/*
 * ct_parser_text.cc
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

#include "ct_parser.h"
#include "ct_logging.h"
#include <iostream>

template<class T>
std::unordered_map<char, std::vector<std::string>> build_pos_tokens(const T& tokens) {
    std::unordered_map<char, std::vector<std::string>> out;
    for (const auto& token : tokens) {
        if (!token.first.empty()) out[token.first.front()].emplace_back(token.first);
    }
    return out;
}

constexpr bool nor(bool a, bool b) {
    return !(a || b);
}

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

template<class ITER_T>
std::optional<std::string> branch_token(ITER_T begin, ITER_T end, const std::vector<std::string>& options) {
    std::size_t largest_len = 0;
    std::optional<std::string> largest = std::nullopt;
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


void CtTextParser::_build_pos_tokens() {
    if (_possible_tokens.empty()) {
        auto& open_tags = open_tokens_map();
        auto& close_tags = close_tokens_map();
        
        _possible_tokens = build_pos_tokens(open_tags);
        auto other_pos  = build_pos_tokens(close_tags);
        for (const auto& token : other_pos) {
            auto& tokens_full =  _possible_tokens[token.first];
            tokens_full.insert(tokens_full.end(), token.second.begin(), token.second.end());
        }
    }
}

std::vector<std::string> CtTextParser::_tokenize(const std::string& text)
{
    _build_pos_tokens();
    
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

std::vector<std::pair<const CtParser::token_schema *, std::string>> CtTextParser::_parse_tokens(const std::vector<std::string>& tokens) const
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
                        auto tokonised_stream = _parse_tokens(rem_tokens);
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

void CtTextParser::TokenMatcher::erase_last() {
    if (!_finished && !_open_token.empty()) {
        if (!_found_open || _token_contents.empty()) {
            _open_token.pop_back();
            _token_buff = _open_token;
            
            spdlog::debug("POPPED OPEN: {}", _open_token);
        } else {
            _token_contents.pop_back();
        }
    }
}


void CtTextParser::TokenMatcher::feed(char ch) {    
    _text_parser->_build_pos_tokens();
    bool found = false;
    auto token_pos = _text_parser->_possible_tokens.find(ch);
    if (token_pos != _text_parser->_possible_tokens.end()) {
        spdlog::debug("POS TOKEN: <{}>", std::string(1, ch));
        if (_pos_tokens.empty()) {
            _pos_tokens = token_pos->second;
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
                feed(prev_char);
                feed(ch);
                return;
            } 
            _token_buff.clear();
            _token_buff += ch;
            found = true;
            _pos_tokens = token_pos->second;
            _pos_chars.clear();
            spdlog::debug("Swapped on: <{}>", std::string(1, ch));
        }
                
    }
    _update_tokens();

    if (_found_open && !found) _token_contents += ch;
}

void CtTextParser::TokenMatcher::_update_tokens() {
    if (!_pos_tokens.empty()) {
        auto pos_token = branch_token(_token_buff.begin(), _token_buff.end(), _pos_tokens);
        if (pos_token) {
            spdlog::debug("TKN: <{}>", *pos_token);
            // Found a possible token
            if (*pos_token == _open_token && _token_contents.empty()) {
                // Repeat, it actually failed but matched the start
                _found_open = true;

                _token_buff.clear();
                _text_parser->_init_tokens();
                for (const auto& tkn : _text_parser->_token_schemas) {
                    if (tkn.open_tag == _open_token) {
                        _pos_tokens = {(tkn.is_symmetrical ? tkn.open_tag : tkn.close_tag)};
                        break;
                    }
                }
                spdlog::debug("FOUND OPEN");
                _pos_chars.clear();
            } else if (!_found_open) {
                // Open tag
                auto& open_tkns = _text_parser->open_tokens_map();
                if (open_tkns.find(_token_buff) != open_tkns.end()) {
                    _open_token = *pos_token;
                } else {
                    _token_buff.clear();
                }

            } else {
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

std::pair<Gtk::TextIter, Gtk::TextIter> CtTextParser::find_formatting_boundaries(Gtk::TextIter start_bounds, Gtk::TextIter word_end)
{
    _build_token_maps();
    
    Glib::ustring token_str(start_bounds, word_end);
   
    auto tokens = _tokenize(token_str);
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

