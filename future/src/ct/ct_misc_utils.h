/*
 * ct_misc_utils.h
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

#include <gtksourceviewmm.h>
#include <gtkmm/treeiter.h>
#include <gtkmm/treestore.h>
#include <unordered_set>
#include "ct_treestore.h"
#include "ct_types.h"
#include "src/fmt/fmt.h"
#include "src/fmt/ostream.h" // to support Glib::ustring formatting
#include <type_traits>

template<class F> auto scope_guard(F&& f) {
    return std::unique_ptr<void, typename std::decay<F>::type>{(void*)1, std::forward<F>(f)};
}

namespace CtMiscUtil {

CtDocType get_doc_type(const std::string& fileName);

CtDocEncrypt get_doc_encrypt(const std::string& fileName);

const gchar* get_doc_extension(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt);

void filepath_extension_fix(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt, std::string& filepath);

void widget_set_colors(Gtk::Widget& widget, const std::string& fg, const std::string& bg,
                       bool syntax_highl, const std::string& gdk_col_fg);

bool node_siblings_sort_iteration(Glib::RefPtr<Gtk::TreeStore> model, const Gtk::TreeNodeChildren& children,
                                  std::function<bool(Gtk::TreeIter&, Gtk::TreeIter&)> need_swap);

std::string get_node_hierarchical_name(CtTreeIter tree_iter, const char* separator="--",
                                       bool for_filename=true, bool root_to_leaf=true, const char* trailer="");

std::string clean_from_chars_not_for_filename(std::string filename);

Gtk::BuiltinIconSize getIconSize(int size);

} // namespace CtMiscUtil

namespace CtTextIterUtil {

bool get_is_camel_case(Gtk::TextIter iter_start, int num_chars);

bool get_first_chars_of_string_are(const Glib::ustring& text, const std::vector<Glib::ustring>& chars_list);

bool get_next_chars_from_iter_are(Gtk::TextIter text_iter, const Glib::ustring& chars_list);

bool get_next_chars_from_iter_are(Gtk::TextIter text_iter, const std::vector<Glib::ustring>& chars_list_vec);

bool get_first_chars_of_string_at_offset_are(const Glib::ustring& in_string, int offset, const std::vector<Glib::ustring>& chars_list_vec);

void rich_text_attributes_update(const Gtk::TextIter& text_iter, std::map<const gchar*, std::string>& curr_attributes);

bool tag_richtext_toggling_on_or_off(const Gtk::TextIter& text_iter);

void generic_process_slot(int start_offset,
                          int end_offset,
                          Glib::RefPtr<Gtk::TextBuffer>& text_buffer,
                          std::function<void(Gtk::TextIter&/*start_iter*/, Gtk::TextIter&/*curr_iter*/, std::map<const gchar*, std::string>&/*curr_attributes*/)> serialize_func);

const gchar* get_text_iter_alignment(const Gtk::TextIter& textIter, CtMainWin* pCtMainWin);

} // namespace CtTextIterUtil

namespace CtStrUtil {

bool is_str_true(const Glib::ustring& inStr);

gint64 gint64_from_gstring(const gchar* inGstring, bool hexPrefix=false);

guint32 guint32_from_hex_chars(const char* hexChars, guint8 numChars);

std::vector<gint64> gstring_split_to_int64(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

template<class IterableOfPgchar>
bool is_pgchar_in_pgchar_iterable(const gchar* pGcharNeedle, const IterableOfPgchar& haystackOfPgchar)
{
    bool gotcha{false};
    for (const gchar* pGcharHaystack : haystackOfPgchar)
    {
        if (0 == g_strcmp0(pGcharHaystack, pGcharNeedle))
        {
            gotcha = true;
            break;
        }
    }
    return gotcha;
}

// https://stackoverflow.com/questions/642213/how-to-implement-a-natural-sort-algorithm-in-c
int natural_compare(const Glib::ustring& left, const Glib::ustring& right);


} // namespace CtStrUtil

namespace CtFontUtil {

std::string get_font_family(const std::string& fontStr);

std::string get_font_size_str(const std::string& fontStr);

} // namespace CtFontUtil

namespace CtRgbUtil {

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

bool endswith(const std::string& str, const std::string& ending);

int indexOf(const Glib::ustring& str, const Glib::ustring& lookup_str);

int indexOf(const Glib::ustring& str, const gunichar& uc);

std::string xml_escape(const std::string& text);

std::string re_escape(const std::string& text);

std::string time_format(const std::string& format, const gint64& time);

int symb_pos_to_byte_pos(const Glib::ustring& text, int symb_pos);
int byte_pos_to_symb_pos(const Glib::ustring& text, int byte_pos);

Glib::ustring swapcase(const Glib::ustring& text);

template<class String>
String replace(String& subjectStr, const Glib::ustring& searchStr, const Glib::ustring& replaceStr)
{
    size_t pos = 0;
    while ((pos = subjectStr.find(searchStr, pos)) != std::string::npos)
    {
        subjectStr.replace(pos, searchStr.size(), replaceStr);
        pos += replaceStr.size();
    }
    return subjectStr;
}

template<class String>
String trim(String s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

template<typename ...Args>
std::string format(std::string in_str, const Args&... args)
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

namespace CtFileSystem {

// From Slash to Backslash when needed
std::string get_proper_platform_filepath(std::string filepath);

void copy_file(Glib::ustring from_file, Glib::ustring to_file);

std::string abspath(const std::string& path);

time_t getmtime(const std::string& path);

int getsize(const std::string& path);


void external_filepath_open(const std::string& filepath, bool open_fold_if_no_app_error);
void external_folderpath_open(const std::string& folderpath);

Glib::ustring prepare_export_folder(Glib::ustring dir_place, Glib::ustring new_folder, bool overwrite_existing);

} // namespace CtFileSystem
