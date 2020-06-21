/*
 * ct_misc_utils.h
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

#include <gtksourceviewmm.h>
#include <gtkmm/treeiter.h>
#include <gtkmm/treestore.h>
#include <unordered_set>
#include "ct_treestore.h"
#include "ct_types.h"
#include "ct_logging.h"
#include <type_traits>



class CtConfig;

template<class F> auto scope_guard(F&& f) {
    return std::unique_ptr<void, typename std::decay<F>::type>{(void*)1, std::forward<F>(f)};
}


namespace CtCSV {
    using CtStringTable = std::vector<std::vector<std::string>>;
    CtStringTable table_from_csv(std::istream& input);
    void table_to_csv(const CtStringTable& table, std::ostream& output);
}

namespace CtMiscUtil {

std::string get_ct_language();


std::string get_doc_extension(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt);

void filepath_extension_fix(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt, std::string& filepath);

void widget_set_colors(Gtk::Widget& widget, const std::string& fg, const std::string& bg,
                       bool syntax_highl, const std::string& gdk_col_fg);

bool node_siblings_sort_iteration(Glib::RefPtr<Gtk::TreeStore> model, const Gtk::TreeNodeChildren& children,
                                  std::function<bool(Gtk::TreeIter&, Gtk::TreeIter&)> need_swap);

std::string get_node_hierarchical_name(CtTreeIter tree_iter, const char* separator="--",
                                       bool for_filename=true, bool root_to_leaf=true, const char* trailer="");

std::string clean_from_chars_not_for_filename(std::string filename);

Gtk::BuiltinIconSize getIconSize(int size);

/**
 * @brief Check if the the mime for a file contains a given string
 * @return
 */
bool mime_type_contains(const std::string& filepath, const std::string& type);

enum class URI_TYPE { LOCAL_FILEPATH, WEB_URL, UNKNOWN };
URI_TYPE get_uri_type(const std::string& uri);

void parallel_for(size_t first, size_t last, std::function<void(size_t)> f);

} // namespace CtMiscUtil

namespace CtTextIterUtil {

bool get_is_camel_case(Gtk::TextIter iter_start, int num_chars);

inline bool startswith(Gtk::TextIter text_iter, const gchar* str)
{
    gunichar ch = g_utf8_get_char(str);
    while (true)
    {
        if (text_iter.get_char() != ch)
            return false;
        str = g_utf8_next_char(str);
        ch = g_utf8_get_char(str);
        if (ch == 0)
            return true;
        if (!text_iter.forward_char())
            return false;
    }
}

template<class type>
const gchar* get_str_pointer(const type& str)
{
    return str;
}

template<>
inline const gchar* get_str_pointer<std::string_view>(const std::string_view& str)
{
    return str.data();
}

template<class container>
bool startswith_any(Gtk::TextIter text_iter, const container& str_list)
{
    for (auto it = std::begin(str_list); it != std::end(str_list); ++it)
        if (startswith(text_iter, get_str_pointer(*it)))
            return true;
    return false;
}

void rich_text_attributes_update(const Gtk::TextIter& text_iter, std::map<std::string_view, std::string>& curr_attributes);

bool tag_richtext_toggling_on_or_off(const Gtk::TextIter& text_iter);

void generic_process_slot(int start_offset,
                          int end_offset,
                          Glib::RefPtr<Gtk::TextBuffer>& text_buffer,
                          std::function<void(Gtk::TextIter&/*start_iter*/, Gtk::TextIter&/*curr_iter*/, std::map<std::string_view, std::string>&/*curr_attributes*/)> serialize_func);

const gchar* get_text_iter_alignment(const Gtk::TextIter& textIter, CtMainWin* pCtMainWin);

int get_words_count(const Glib::RefPtr<Gtk::TextBuffer>& text_buffer);

} // namespace CtTextIterUtil

namespace CtStrUtil {

bool is_str_true(const Glib::ustring& inStr);

gint64 gint64_from_gstring(const gchar* inGstring, bool hexPrefix=false);

guint32 guint32_from_hex_chars(const char* hexChars, guint8 numChars);

std::vector<gint64> gstring_split_to_int64(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

template<class type>
int custom_compare(const type& str, const gchar* el)
{
   return g_strcmp0(str, el);
}
template<>
inline int custom_compare<std::string_view>(const std::string_view& str, const gchar* el)
{
   return g_strcmp0(str.data(), el);
}

template<class container>
bool contains(const container& array, const gchar* el)
{
    for (auto it = std::begin(array); it != std::end(array); ++it)
        if (0 == custom_compare(*it, el))
            return true;
    return false;
}

template<class String>
bool contains_words(const String& text, const std::vector<String>& words, bool require_all = true)
{
    for (auto& word: words) {
        if (text.find(word) != String::npos) {
            if (!require_all)
                return true;
            } else if (require_all) {
                return false;
            }
        }

  return require_all;
}

// https://stackoverflow.com/questions/642213/how-to-implement-a-natural-sort-algorithm-in-c
int natural_compare(const Glib::ustring& left, const Glib::ustring& right);

// Returns a version of text in which all occurrences of words
// are highlighted using Pango markup
Glib::ustring highlight_words(const Glib::ustring& text, std::vector<Glib::ustring> words, const Glib::ustring& markup_tag = "b");

Glib::ustring get_accelerator_label(const std::string& accelerator);

std::string get_internal_link_from_http_url(std::string link_url);

} // namespace CtStrUtil

namespace CtFontUtil {

std::string get_font_family(const std::string& fontStr);

std::string get_font_size_str(const std::string& fontStr);

} // namespace CtFontUtil

namespace CtRgbUtil {

// todo: normalized color function, better use RGBA as inner presentation instead of strings

void set_rgb24str_from_rgb24int(guint32 rgb24Int, char* rgb24StrOut);

guint32 get_rgb24int_from_rgb24str(const char* rgb24Str);

char* set_rgb24str_from_str_any(const char* rgbStrAny, char* rgb24StrOut);

Glib::ustring rgb_to_no_white(Glib::ustring in_rgb);

std::string get_rgb24str_from_str_any(const std::string& rgbStrAny);

guint32 get_rgb24int_from_str_any(const char* rgbStrAny);

std::string rgb_to_string(Gdk::RGBA color);

std::string rgb_any_to_24(Gdk::RGBA color);

} // namespace CtRgbUtil

namespace str {

bool startswith(const std::string& str, const std::string& starting);

template<class container>
bool startswith_any(const Glib::ustring& text, const container& chars_list)
{
    for (auto it = std::begin(chars_list); it != std::end(chars_list); ++it)
        if (str::startswith(text, CtTextIterUtil::get_str_pointer(*it)))
            return true;
    return false;
}

bool endswith(const std::string& str, const std::string& ending);

int indexOf(const Glib::ustring& str, const Glib::ustring& lookup_str);

int indexOf(const Glib::ustring& str, const gunichar& uc);

template <typename T, size_t size>
int indexOf(const std::array<T, size>& array, const T& uc)
{
    for (size_t i = 0; i < size; ++i)
        if (array[i] == uc)
            return (int)i;
    return -1;
}

std::string xml_escape(const std::string& text);

std::string re_escape(const std::string& text);

std::string time_format(const std::string& format, const time_t& time);

int symb_pos_to_byte_pos(const Glib::ustring& text, int symb_pos);
int byte_pos_to_symb_pos(const Glib::ustring& text, int byte_pos);

Glib::ustring swapcase(const Glib::ustring& text);

template<class String>
std::string replace(const /* const: func doens't change the source! */ String& subjectStr, const std::string& searchStr, const std::string& replaceStr)
{
    Glib::ustring text = subjectStr; // Glib::ustring works with unicode
    size_t pos = 0;
    while ((pos = text.find(searchStr, pos)) != std::string::npos)
    {
        text.replace(pos, searchStr.size(), replaceStr);
        pos += replaceStr.size();
    }
    return text;
}

template<class String>
String trim(String s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

template<typename ...Args>
std::string format(const std::string& in_str, const Args&... args)
{
    return fmt::format(str::replace(in_str, "%s", "{}"), args...);
}

template<class STRING>
std::vector<STRING> split(const STRING& strToSplit, const char* delimiter)
{
    std::vector<STRING> vecOfStrings;
    gchar** arrayOfStrings = g_strsplit(strToSplit.c_str(), delimiter, -1);
    for (gchar** ptr = arrayOfStrings; *ptr; ptr++)
    {
        vecOfStrings.push_back(*ptr);
    }
    g_strfreev(arrayOfStrings);
    return vecOfStrings;
}

template<class STRING>
std::string join(const std::vector<STRING>& cnt, const std::string& delimer)
{
    bool firstTime = true;
    std::stringstream ss;
    for (auto& v: cnt)
    {
        if (not firstTime) ss << delimer;
        else firstTime = false;
        ss << v;
    }
    return ss.str();
}

template<class String, class Vector>
void join_numbers(const Vector& in_numbers_vec, String& outString, const gchar* delimiter=" ")
{
    bool firstIteration{true};
    for(const auto& element : in_numbers_vec)
    {
        if (not firstIteration) outString += delimiter;
        else firstIteration = false;
        outString += std::to_string(element);
    }
}

Glib::ustring repeat(const Glib::ustring& input, int num);

} // namespace str

namespace vec {

template<class VEC, class VAL>
void remove(VEC& v, const VAL& val)
{
    auto it = std::find(v.begin(), v.end(), val);
    if (it != v.end())
    {
        v.erase(it);
    }
}

template<class VEC, class VAL>
bool exists(const VEC& v, const VAL& val)
{
    return std::find(v.begin(), v.end(), val) != v.end();
}

/**
 * Extend a vector with elements, without destroying source one.
 */
template<typename VEC, typename CONTAINER>
void vector_extend(std::vector<VEC>& v, const CONTAINER& ext)
{
    v.reserve(v.size() + ext.size());
    v.insert(std::end(v), std::begin(ext), std::end(ext));
}

/**
 * Extend a vector with elements with move semantics.
 */
template<typename VEC>
void vector_extend(std::vector<VEC>& v, std::vector<VEC>&& ext)
{
    if (v.empty())
    {
        v = std::move(ext);
    }
    else
    {
        v.reserve(v.size() + ext.size());
        std::move(std::begin(ext), std::end(ext), std::back_inserter(v));
        ext.clear();
    }
}

} // namespace vec

namespace set {

template<class SET, class VAL>
bool remove(SET& s, const VAL& val)
{
    auto it = s.find(val);
    if (it != s.end())
    {
        s.erase(it);
        return true;
    }
    return false;
}

} // namespace set

namespace map {

template<class MAP, class KEY>
bool exists(const MAP& m, const KEY& key)
{
    return m.find(key) != m.end();
}

} // namespace map



