/*
 * ct_types.h
 *
 * Copyright 2009-2020
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

#include <string>
#include <list>
#include <set>
#include <unordered_map>
#include <glibmm/ustring.h>
#include <gtksourceviewmm/buffer.h>


namespace fs {
class path;
}

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

class CtCodebox;
class CtMainWin;
typedef std::pair<CtCodebox*, CtMainWin*>   CtPairCodeboxMainWin;

struct CtListInfo
{
    CtListType type = CtListType::None;
    int        num = -1;   // todo: fix that for bullet and number it has different meanings
    int        level = -1; // can be filled for NONE to use with shift+return
    int        aux = -1;
    int        startoffs = -1;

    operator bool() { return type != CtListType::None; }
};

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
        while (std::list<TYPE>::size() > (size_t)maxSize)
        {
            std::list<TYPE>::pop_back();
        }
    }
};

struct CtRecentDocsFilepaths : public CtMaxSizedList<fs::path>
{
    CtRecentDocsFilepaths() : CtMaxSizedList<fs::path>{10} {}
};


struct CtStorageNodeState
{
    bool upd{false};
    bool prop{false};
    bool buff{false};
    bool hier{false};
};

struct CtStorageSyncPending
{
    bool                                           bookmarks_to_write{false};
    std::unordered_map<gint64, CtStorageNodeState> nodes_to_write_dict;
    std::set<gint64>                               nodes_to_rm_set;
};

struct CtNodeData;
class CtAnchoredWidget;
class CtStorageEntity
{
public:
    CtStorageEntity() = default;
    virtual ~CtStorageEntity() = default;

    virtual void close_connect() = 0;
    virtual void reopen_connect() = 0;
    virtual void test_connection() = 0;

    virtual bool populate_treestore(const fs::path& file_path, Glib::ustring& error) = 0;
    virtual bool save_treestore(const fs::path& file_path, const CtStorageSyncPending& syncPending, Glib::ustring& error) = 0;
    virtual void vacuum() = 0;
    virtual void import_nodes(const fs::path& path) = 0;

    virtual Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                              const std::string& syntax,
                                                              std::list<CtAnchoredWidget*>& widgets) const = 0;

};

struct CtExportOptions
{
    bool include_node_name{true};
    bool new_node_page{false};
    bool index_in_page{true};
};

struct CtSummaryInfo
{
    size_t nodes_rich_text_num{0};
    size_t nodes_plain_text_num{0};
    size_t nodes_code_num{0};
    size_t images_num{0};
    size_t embfile_num{0};
    size_t tables_num{0};
    size_t codeboxes_num{0};
    size_t anchors_num{0};
};
