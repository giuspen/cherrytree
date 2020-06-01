/*
 * ct_text_parser.cc
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
#include <iostream>
#include <src/fmt/format.h>

template<class T>
std::unordered_set<char> build_pos_tokens(const T& tokens) {
    std::unordered_set<char> out;
    for (const auto& token : tokens) {
        if (!token.first.empty()) out.insert(token.first.begin(), token.first.end());
    }
    return out;
}

constexpr bool nor(bool a, bool b) {
    return !(a || b);
}
std::vector<std::string> CtTextParser::_tokenize(const std::string& text) {
    auto& open_tags = open_tokens_map();
    auto& close_tags = close_tokens_map();
    if (_possible_tokens.empty()) {
        _possible_tokens = build_pos_tokens(open_tags);
        auto other_pos  = build_pos_tokens(close_tags);
        _possible_tokens.insert(other_pos.begin(), other_pos.end());
    }
    
    std::vector<std::string> tokens;
    std::string::const_iterator last_pos = text.begin();
    for (auto ch = text.begin(); ch != text.end(); ++ch) {
        
        if (*ch == ' ') {
            if (last_pos != ch) tokens.emplace_back(last_pos, ch);
            last_pos = ch;
        } else if (_possible_tokens.find(*ch) != _possible_tokens.end() ){
            auto forward_pos = ch;
            
            std::string key(ch, ch + 1);
           // ++ch;
            while (ch != text.end() && nor(open_tags.find(key) != open_tags.end(), close_tags.find(key) != close_tags.end()) && (_possible_tokens.find(*ch) != _possible_tokens.end())) {
                if (*ch == ' ') {
                    if (last_pos != ch) tokens.emplace_back(last_pos, ch);
                    last_pos = ch;
                    break;
                }
                ++ch;
                if (ch != text.end()) key = std::string(forward_pos, ch + 1);
                else break;
            }
            bool in_open = open_tags.find(key) != open_tags.end();
            bool in_close =  close_tags.find(key) != close_tags.end();
            if (in_open || in_close) {
                if (last_pos != forward_pos) tokens.emplace_back(last_pos, forward_pos);
                
                tokens.emplace_back(key);
                last_pos = ch;
                ++last_pos;
            }
    
            // ++last_pos;
            if (ch == text.end()) break;
        }
    }
    if (last_pos != text.end()) {
        tokens.emplace_back(last_pos, text.end());
    }
    return tokens;
}

std::vector<std::pair<const CtParser::token_schema *, std::string>> CtTextParser::_parse_tokens(const std::vector<std::string>& tokens) const {
    
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
            if (!keep_parsing) {
                if (curr_open_tags.first.front()->close_tag != *token) {
                    curr_open_tags.second += *token;
                    continue;
                } else if (curr_open_tags.first.front()->close_tag == *token) {
                    if (nb_open_tags > 0) {
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


std::pair<Gtk::TextIter, Gtk::TextIter> CtTextParser::find_formatting_boundaries(Gtk::TextIter start_bounds, Gtk::TextIter word_end) {
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
    
    Glib::ustring token_test(start_bounds, word_end);
    if ((ftoken_iter == open_tags.cend() && rtoken_iter != close_tags.cend()) || (token_test == rtoken_iter->second->open_tag)) {
        // Found only a close tag, parse error
        throw CtParseError(fmt::format("Close tag without open tag while parsing: {}", token_str.c_str()));
    }
    
    
    return {start_bounds, word_end};
}

