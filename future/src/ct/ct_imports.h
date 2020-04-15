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

#include <vector>
#include <glibmm/ustring.h>
#include <libxml2/libxml/HTMLparser.h>

namespace CtImports {

std::vector<std::pair<int, int>> get_web_links_offsets_from_plain_text(const Glib::ustring& plain_text);

}

class CtHtmlParser
{
public:
    CtHtmlParser() = default;
    virtual ~CtHtmlParser() = default;

    void feed(const std::string& html);
    std::string fix_body(const std::string& html);

    virtual void handle_starttag(std::string_view name, const char** atts);
    virtual void handle_endtag(std::string_view localname);
    virtual void handle_data(std::string_view text);
    virtual void handle_charref(std::string_view name);
};

