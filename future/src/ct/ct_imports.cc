/*
 * ct_imports.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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
#include "ct_misc_utils.h"
#include "ct_const.h"

// Parse plain text for possible web links
std::vector<std::pair<int, int>> CtImports::get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text)
{
    std::vector<std::pair<int, int>> web_links;
    int max_end_offset = (int)plain_text.size();
    int max_start_offset = max_end_offset - 7;
    int start_offset = 0;
    while (start_offset < max_start_offset)
    {
        if (CtTextIterUtil::get_first_chars_of_string_at_offset_are(plain_text, start_offset, CtConst::WEB_LINK_STARTERS))
        {
            int end_offset = start_offset + 3;
            while (end_offset < max_end_offset
                   && plain_text[(size_t)end_offset] != CtConst::CHAR_SPACE[0]
                   && plain_text[(size_t)end_offset] != CtConst::CHAR_NEWLINE[0])
                end_offset += 1;
            web_links.push_back(std::make_pair(start_offset, end_offset));
            start_offset = end_offset + 1;
        }
        else
            start_offset += 1;
    }
    return web_links;
}
