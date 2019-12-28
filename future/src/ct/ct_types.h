/*
 * ct_types.h
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

#include <string>
#include <list>
#include <unordered_map>

enum class CtYesNoCancel { Yes, No, Cancel };

enum class CtDocType { None, XML, SQLite };

enum class CtDocEncrypt { None, True, False };

enum class CtAnchWidgType { CodeBox, Table, ImagePng, ImageAnchor, ImageEmbFile };

enum class CtPixTabCBox { Pixbuf, Table, CodeBox };

enum class CtSaveNeededUpdType { None, nbuf, npro, ndel, book };

enum class CtXmlNodeType { None, RichText, EncodedPng, Table, CodeBox };

enum class CtExporting { No, All, NodeOnly, NodeAndSubnodes };

enum class CtListType { None, Todo, Bullet, Number };

enum class CtRestoreExpColl : int { FROM_STR=0, ALL_EXP=1, ALL_COLL=2 };

enum class CtTableColMode : int { RENAME=0, ADD=1, DELETE=2, RIGHT=3, LEFT=4 };

struct CtRecentDocRestore
{
    std::string   exp_coll_str;
    std::string   node_path;
    int           cursor_pos{0};
};

typedef std::unordered_map<std::string, CtRecentDocRestore>   CtRecentDocsRestore;

template<class TYPE>
class CtMaxSizedList : public std::list<TYPE>
{
public:
    CtMaxSizedList(const int size) : maxSize{size} {}
    const int maxSize;
    void move_or_push_back(const TYPE& element)
    {
        std::list<TYPE>::remove(element);
        std::list<TYPE>::push_back(element);
        _check_size();
    }
    void move_or_push_front(const TYPE& element)
    {
        std::list<TYPE>::remove(element);
        std::list<TYPE>::push_front(element);
        _check_size();
    }
private:
    void _check_size()
    {
        while (std::list<TYPE>::size() > maxSize)
        {
            std::list<TYPE>::pop_back();
        }
    }
};

struct CtRecentDocsFilepaths : public CtMaxSizedList<std::string>
{
    CtRecentDocsFilepaths() : CtMaxSizedList<std::string>{10} {}
};
