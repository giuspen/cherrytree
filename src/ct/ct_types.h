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

enum class CtExporting { NONE, SELECTED_TEXT, CURRENT_NODE, CURRENT_NODE_AND_SUBNODES, ALL_TREE };

enum class CtListType { None, Todo, Bullet, Number };

enum class CtRestoreExpColl : int { FROM_STR=0, ALL_EXP=1, ALL_COLL=2 };

class CtCodebox;
class CtMainWin;
typedef std::pair<CtCodebox*, CtMainWin*>   CtPairCodeboxMainWin;

struct CtLinkEntry
{
    Glib::ustring type;
    gint64        node_id{-1};
    Glib::ustring webs;
    Glib::ustring file;
    Glib::ustring fold;
    Glib::ustring anch;
};

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

typedef std::unordered_map<std::string, CtRecentDocRestore> CtRecentDocsRestore;
class CtTableCell;
typedef std::vector<CtTableCell*>   CtTableRow;
typedef std::vector<CtTableRow>     CtTableMatrix;
typedef std::vector<int>            CtTableColWidths;

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

class CtStringSplittable
{
private:
    using vect_t = std::vector<Glib::ustring>;

public:
    CtStringSplittable(const Glib::ustring& str) : _string_cache(str) {
        for (const auto& ch : _string_cache) {
            _internal_vec.emplace_back(1, ch);
        }
    }

    const Glib::ustring& operator[](size_t index) const { return _internal_vec[index]; }

    size_t size() const { return _internal_vec.size(); }

    template<typename T>
    typename vect_t::const_iterator find(const T& item) const { return std::find(_internal_vec.begin(), _internal_vec.end(), item); }

    template<typename T>
    bool contains(const T& item) const { return std::find(_internal_vec.begin(), _internal_vec.end(), item) != _internal_vec.end(); }

    typename vect_t::const_iterator end() const noexcept { return _internal_vec.end(); }
    typename vect_t::const_iterator begin() const noexcept { return _internal_vec.begin(); }

    const Glib::ustring& item() const { return _string_cache; }

private:
    Glib::ustring _string_cache;
    vect_t        _internal_vec;
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
    virtual bool save_treestore(const fs::path& file_path,
                                const CtStorageSyncPending& syncPending,
                                Glib::ustring& error,
                                const CtExporting exporting = CtExporting::NONE,
                                const int start_offset = 0,
                                const int end_offset = -1) = 0;
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
    bool single_file{false};
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
