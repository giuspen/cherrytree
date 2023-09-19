/*
 * ct_types.h
 *
 * Copyright 2009-2023
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
#include <deque>
#include <mutex>
#include <optional>
#include <condition_variable>
#include <type_traits>
#include <glibmm/ustring.h>
#include <gtksourceviewmm/buffer.h>
#include "ct_const.h"

namespace fs {
class path;
}

enum class CtYesNoCancel { Yes, No, Cancel };

enum class CtDocType { None, XML, SQLite, MultiFile };

enum class CtDocEncrypt { None, True, False };

enum class CtAnchWidgType { CodeBox, TableHeavy, TableLight, ImagePng, ImageAnchor, ImageLatex, ImageEmbFile };

enum class CtPixTabCBox { Pixbuf, Table, CodeBox };

enum class CtSaveNeededUpdType { None, nbuf, npro, ndel, book };

enum class CtXmlNodeType { None, RichText, EncodedPng, Table, CodeBox };

enum class CtExporting { NONESAVE, NONESAVEAS, SELECTED_TEXT, CURRENT_NODE, CURRENT_NODE_AND_SUBNODES, ALL_TREE };

enum class CtListType { None, Todo, Bullet, Number };

enum class CtRestoreExpColl : int { FROM_STR=0, ALL_EXP=1, ALL_COLL=2 };

class CtCodebox;
class CtMainWin;
using CtPairCodeboxMainWin = std::pair<CtCodebox*, CtMainWin*>;
namespace xmlpp {
class Document;
}
using CtDelayedTextBufferMap = std::unordered_map<gint64, std::shared_ptr<xmlpp::Document>>;
using CtCurrAttributesMap = std::unordered_map<std::string_view, std::string>;

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
    CtListType type{CtListType::None};
    int        num_seq{-1};
    int        level{-1};
    int        aux{-1};
    int        startoffs{-1};
    int        count_nl{-1};
    friend inline bool operator==(const CtListInfo& lhs, const CtListInfo& rhs) {
        return lhs.type == rhs.type and
               lhs.num_seq == rhs.num_seq and
               lhs.level == rhs.level and
               lhs.startoffs == rhs.startoffs and
               lhs.count_nl == rhs.count_nl;
    }
    friend inline bool operator!=(const CtListInfo& lhs, const CtListInfo& rhs) { return !(lhs == rhs); }
    operator bool() const { return type != CtListType::None; }
};

struct CtTextRange
{
    Gtk::TextIter iter_start;
    Gtk::TextIter iter_end;
    int leading_chars_num{0};
};

struct CtRecentDocRestore
{
    std::string   exp_coll_str;   // list of expanded nodes
    std::string   visited_nodes;
    std::string   node_path;      // the current node
    int           cursor_pos{0};  // cursor position in the current node
    int           v_adj_val{0};   // text vertical scrollbar position in the current node
};
using CtRecentDocsRestore = std::unordered_map<std::string, CtRecentDocRestore>;

class CtTextCell;
using CtTableRow = std::vector<void*>; // CtTextCell* (for CtTableHeavy) or Glib::ustring* (for CtTableLight)
using CtTableMatrix = std::vector<CtTableRow>;
using CtTableColWidths = std::vector<int>;

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

struct CtScalableTag
{
    const std::string sep{";"};
    CtScalableTag(const char* serialised, const char* fallback = "") {
        deserialise(serialised, fallback);
    }
    void deserialise(const char* serialised, const char* fallback = "") {
        if (strchr(serialised, '.') or strchr(serialised, ',')) {
            serialised = fallback; // legacy scale stored as double was not locale proof
        }
        gchar** arrayOfStrings = g_strsplit(serialised, sep.c_str(), -1);
        gchar** ptr = arrayOfStrings;
        if (*ptr) {
            scale = static_cast<double>(std::stoi(*ptr))/1000;
            ++ptr;
            if (*ptr) {
                foreground = *ptr;
                ++ptr;
                if (*ptr) {
                    background = *ptr;
                    ++ptr;
                    if (*ptr) {
                        bold = std::stoi(*ptr);
                        ++ptr;
                        if (*ptr) {
                            italic = std::stoi(*ptr);
                            ++ptr;
                            if (*ptr) {
                                underline = std::stoi(*ptr);
                            }
                        }
                    }
                }
            }
        }
        g_strfreev(arrayOfStrings);
    }
    std::string serialise() const {
        return std::to_string(static_cast<unsigned>(round(scale*1000))) + sep +
               foreground + sep +
               background + sep +
               std::to_string(bold) + sep +
               std::to_string(italic) + sep +
               std::to_string(underline);
    }
    double scale{1.0};
    std::string foreground;
    std::string background;
    bool bold{false};
    bool italic{false};
    bool underline{false};
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
        for (const auto ch : _string_cache) {
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
    bool is_update_of_existing{false};
    bool prop{false};
    bool buff{false};
    bool hier{false};
};

struct CtStorageSyncPending
{
    bool                                           fix_db_tables{true};
    bool                                           bookmarks_to_write{false};
    std::unordered_map<gint64, CtStorageNodeState> nodes_to_write_dict;
    std::unordered_set<gint64>                     nodes_to_rm_set;
};

enum class CtBackupType { None, SingleFile, MultiFile };
struct CtBackupEncryptData
{
    CtBackupType backupType;
    bool needEncrypt;
    std::string main_backup;
    std::string file_path;
    std::string password;
    std::string extracted_copy;
};

struct CtNodeData;
class CtAnchoredWidget;
namespace Gtk { class TreeIter; }
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
                                const CtExporting exporting,
                                const int start_offset = 0,
                                const int end_offset = -1) = 0;
    virtual void vacuum() = 0;
    virtual void import_nodes(const fs::path& path, const Gtk::TreeIter& parent_iter) = 0;

    virtual Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                              const std::string& syntax,
                                                              std::list<CtAnchoredWidget*>& widgets) const = 0;

    void set_is_dry_run() { _isDryRun = true; }

protected:
    bool _isDryRun{false};
};

struct CtStockIcon
{
    static const gchar* at(const size_t i) {
        if (i < CtConst::_NODE_CUSTOM_ICONS.size()) {
            const gchar* retVal = CtConst::_NODE_CUSTOM_ICONS.at(i);
            if (retVal) {
                return retVal;
            }
        }
        return CtConst::_NODE_CUSTOM_ICONS.at(CtConst::NODE_ICON_NO_ICON_ID);
    }
    static size_t size() { return CtConst::_NODE_CUSTOM_ICONS.size(); }
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
    size_t latexes_num{0};
    size_t embfile_num{0};
    size_t heavytables_num{0};
    size_t lighttables_num{0};
    size_t codeboxes_num{0};
    size_t anchors_num{0};
};

template<class F> auto scope_guard(F&& f) {
    return std::unique_ptr<void, typename std::decay<F>::type>{(void*)1, std::forward<F>(f)};
}

template <class T, size_t MAX> class ThreadSafeDEQueue
{
public:
    void push_back(T t) {
        std::lock_guard<std::mutex> lock(m);
        if (q.size() < MAX) {
            q.push_back(t);
            c.notify_one();
        }
    }
    T pop_front() {
        std::unique_lock<std::mutex> lock(m);
        while (q.empty()) {
            c.wait(lock);
        }
        T val = q.front();
        q.pop_front();
        return val;
    }
    std::optional<T> peek() const {
        std::optional<T> retVal;
        std::lock_guard<std::mutex> lock(m);
        if (not q.empty()) {
            retVal = q.front();
        }
        return retVal;
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return q.empty();
    }
    size_t size() const {
        std::lock_guard<std::mutex> lock(m);
        return q.size();
    }
    void clear() {
        std::lock_guard<std::mutex> lock(m);
        q.clear();
    }

private:
    std::deque<T> q{};
    mutable std::mutex m{};
    std::condition_variable c{};
};

struct CtSearchOptions {
    struct time_search {
        std::time_t time;
        bool        on;
    };

    time_search ts_cre_after;
    time_search ts_cre_before;
    time_search ts_mod_after;
    time_search ts_mod_before;
    std::string str_find;
    std::string str_replace;
    bool        match_case{false};
    bool        reg_exp{false};
    bool        accent_insensitive{false};
    bool        override_exclusions{false};
    bool        whole_word{false};
    bool        start_word{false};
    bool        direction_fw{true};
    int         all_firstsel_firstall{0};
    bool        iterative_dialog{true};
    bool        only_sel_n_subnodes{false};
    bool        node_content{true};
    bool        node_name_n_tags{true};
};

namespace Gtk { class Dialog; }
class CtMatchDialogStore;

enum class CtCurrFindType { None, SingleNode, MultipleNodes };

struct CtSearchState {
    bool           replace_active{false};
    bool           replace_subsequent{false};
    CtCurrFindType curr_find_type{CtCurrFindType::None};
    std::string    curr_find_pattern;
    bool           from_find_iterated{false};
    bool           from_find_back{false};
    bool           newline_trick{false};

    bool           first_useful_node{false};
    int            counted_nodes{0};
    int            processed_nodes{0};
    int            latest_matches{0};

    int            matches_num;
    bool           all_matches_first_in_node{false};

    int            latest_node_offset{-1};
    gint64         latest_node_offset_node_id{-1};

    std::unique_ptr<Gtk::Dialog> iteratedfinddialog;
    int            iterDialogPos[2]{-1,-1};

    std::pair<int,int>               latest_match_offsets{-1,-1};
    Glib::RefPtr<CtMatchDialogStore> match_store;
};
