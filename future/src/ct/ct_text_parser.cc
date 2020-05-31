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

std::vector<std::pair<const CtParser::token_schema *, std::string>> CtTextParser::_tokenize(const std::string& stream) const
{
    
    std::vector<std::pair<const token_schema *, std::string>> token_stream;
    std::unordered_map<std::string_view, bool> open_tags;
    std::vector<const token_schema *> curr_open_tags;
    std::string buff;
    std::size_t pos = 0;
    bool keep_parsing = true;
    const token_schema* curr_token;
    auto& token_map_open = open_tokens_map();
    auto& token_map_close = close_tokens_map();
    for (const auto& ch : stream) {
        buff += ch;
        pos++;
        
        
        auto tokens_iter = token_map_open.find(buff);
        if (tokens_iter != token_map_open.end() && keep_parsing) {
            curr_open_tags.emplace_back(tokens_iter->second);
            keep_parsing = !tokens_iter->second->capture_all;
    
            if (!tokens_iter->second->has_closetag) {
                // Token will go till end of stream
                token_stream.emplace_back(curr_token, std::string(stream.begin() + pos, stream.end()));
        
                if (keep_parsing) {
                    // Parse the other data in the stream
                    auto tokonised_stream = _tokenize(token_stream.back().second);
                    for (const auto& token : tokonised_stream) {
                        if (token.first) {
                            token_stream.emplace_back(token);
                        }
                    }
                }
                return token_stream;
            }
            
        } else {
            auto close_iter = token_map_close.find(buff);
            if (close_iter != token_map_close.end()) {
    
                token_stream.emplace_back(curr_open_tags.front(), std::string(stream.begin() + pos - buff.length(), stream.begin() + pos - close_iter->first.length()));
    
                if (curr_open_tags.size() >= 2) {
                    // Found more than one tag
                    for (auto iter = curr_open_tags.begin() + 1; iter != curr_open_tags.end(); ++iter) {
                        token_stream.emplace_back(*iter, "");
                    }
                }
                //open_tags[token.open_tag] = false;
                keep_parsing = true;
    
                curr_open_tags.clear();
                buff.clear();
            }
        }
        /*
        
        for (const auto &token : _token_schemas) {
            auto tag_open = open_tags[token.open_tag];
            
            
            auto buff_pos = buff.find(token.open_tag);
            auto has_opentag = buff_pos != std::string::npos;
            
            
            
            if (keep_parsing) {
                if (has_opentag && !tag_open) {
                    curr_open_tags.emplace_back(&token);
                    // First token
                    curr_token = &token;

                    keep_parsing = !curr_token->capture_all;

                    if (!token.has_closetag) {
                        // Token will go till end of stream
                        token_stream.emplace_back(curr_token, std::string(stream.begin() + pos, stream.end()));
                        
                        if (keep_parsing) {
                            // Parse the other data in the stream 
                            auto tokonised_stream = _tokenize(token_stream.back().second);
                            for (const auto& token : tokonised_stream) {
                                if (token.first) {
                                    token_stream.emplace_back(token);
                                }
                             } 
                        }
                        return token_stream;
                    }
                    open_tags[token.open_tag] = true;
                    if (pos - buff_pos != pos - buff.length()) {
                        // Characters may be chopped
                        token_stream.emplace_back(nullptr, std::string(buff.begin(), buff.begin() + buff_pos));
                    }
                    
                    buff.clear();

                    break;
                }
                
            }
            auto has_closetag = has_opentag && token.is_symmetrical;
            if (!token.is_symmetrical && token.has_closetag) {
                if (token.close_tag.empty()) throw CtImportException("Token has no close tag");
                has_closetag = buff.find(token.close_tag) != std::string::npos;
            }

            if (has_closetag && tag_open) {
                auto tag = token.is_symmetrical ? token.open_tag : token.close_tag;
                
                token_stream.emplace_back(curr_open_tags.front(), std::string(stream.begin() + pos - buff.length(), stream.begin() + pos - tag.length()));
    
                if (curr_open_tags.size() >= 2) {
                    // Found more than one tag
                    for (auto iter = curr_open_tags.begin() + 1; iter != curr_open_tags.end(); ++iter) {
                        token_stream.emplace_back(*iter, "");
                    }
                }
                open_tags[token.open_tag] = false;
                keep_parsing = true;
                
                curr_open_tags.clear();
                buff.clear();
                break;
            }
            
        }*/
        
    }
    if (!buff.empty()) {
        // No token
        token_stream.emplace_back(nullptr, buff);
    }
    
    return token_stream;
}