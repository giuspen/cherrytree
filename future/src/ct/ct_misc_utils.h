/*
 * ct_misc_utils.h
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

#pragma once

#include <gtksourceviewmm.h>
#include <gtkmm/treeiter.h>
#include <gtkmm/treestore.h>
#include <set>
#include "ct_treestore.h"
#include "src/fmt/fmt.h"
#include <type_traits>

enum class CtDocType : int {None=0, XML=1, SQLite=2};
enum class CtDocEncrypt : int {None=0, True=1, False=2};

template<class F> auto scope_guard(F&& f) {
    return std::unique_ptr<void, typename std::decay<F>::type>{(void*)1, std::forward<F>(f)};
}

namespace CtMiscUtil {

CtDocType getDocType(std::string fileName);

CtDocEncrypt getDocEncrypt(std::string fileName);

Glib::RefPtr<Gsv::Buffer> get_new_text_buffer(const std::string& syntax, const Glib::ustring& textContent=""); // pygtk: buffer_create

const Glib::ustring getTextTagNameExistOrCreate(Glib::ustring propertyName, Glib::ustring propertyValue);

const gchar* getTextIterAlignment(const Gtk::TextIter& textIter);

Glib::ustring sourceview_hovering_link_get_tooltip(const Glib::ustring& link);

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

bool apply_tag_try_automatic_bounds(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter iter_start);

bool get_is_camel_case(Gtk::TextIter iter_start, int num_chars);

bool get_first_chars_of_string_are(const Glib::ustring& text, const std::vector<Glib::ustring>& chars_list);

bool get_next_chars_from_iter_are(Gtk::TextIter text_iter, const Glib::ustring& chars_list);

bool get_next_chars_from_iter_are(Gtk::TextIter text_iter, const std::vector<Glib::ustring>& chars_list_vec);

bool get_first_chars_of_string_at_offset_are(const Glib::ustring& in_string, int offset, const std::vector<Glib::ustring>& chars_list_vec);

void rich_text_attributes_update(const Gtk::TextIter& text_iter, std::map<const gchar*, std::string>& curr_attributes);

bool tag_richtext_toggling_on_or_off(const Gtk::TextIter& text_iter);

void generic_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                          std::function<void(Gtk::TextIter&/*start_iter*/, Gtk::TextIter&/*curr_iter*/, std::map<const gchar*, std::string>&/*curr_attributes*/)> serialize_func);

} // namespace CtTextIterUtil

namespace CtStrUtil {

bool isStrTrue(const Glib::ustring& inStr);

gint64 gint64FromGstring(const gchar* inGstring, bool hexPrefix=false);

guint32 getUint32FromHexChars(const char* hexChars, guint8 numChars);

std::vector<gint64> gstringSplit2int64(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

bool isPgcharInPgcharSet(const gchar* pGcharNeedle, const std::set<const gchar*>& setPgcharHaystack);

} // namespace CtStrUtil

namespace CtFontUtil {

std::string getFontFamily(const std::string& fontStr);

std::string getFontSizeStr(const std::string& fontStr);

std::string getFontCss(const std::string& fontStr);

const std::string& getFontForSyntaxHighlighting(const std::string& syntaxHighlighting);

std::string getFontCssForSyntaxHighlighting(const std::string& syntaxHighlighting);

} // namespace CtFontUtil

namespace CtRgbUtil {

void setRgb24StrFromRgb24Int(guint32 rgb24Int, char* rgb24StrOut);

guint32 getRgb24IntFromRgb24Str(const char* rgb24Str);

char* setRgb24StrFromStrAny(const char* rgbStrAny, char* rgb24StrOut);

Glib::ustring rgb_to_no_white(Glib::ustring in_rgb);

std::string getRgb24StrFromStrAny(const std::string& rgbStrAny);

guint32 getRgb24IntFromStrAny(const char* rgbStrAny);

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

std::string time_format(const std::string& format, const std::time_t& time);

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

template<class STRING = std::string>
std::vector<STRING> split(const std::string& str, const std::string& delimiter)
{
    std::vector<STRING> vecOfStrings;
    gchar** arrayOfStrings = g_strsplit(str.c_str(), delimiter.c_str(), -1);
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
        if (!firstTime) ss << delimer;
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
        if (!firstIteration) outString += delimiter;
        else firstIteration = false;
        outString += std::to_string(element);
    }
}

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
template<typename VEC>
void vector_extend(std::vector<VEC>& v, const std::vector<VEC>& ext)
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

template<class SET, class KEY>
bool exists(const SET& s, const KEY& key)
{
    return s.find(key) != s.end();
}

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
Glib::ustring get_proper_platform_filepath(Glib::ustring filepath);

bool isdir(const Glib::ustring& path);
bool isfile(const Glib::ustring& path);
Glib::ustring basename(const Glib::ustring& path);
Glib::ustring dirname(const Glib::ustring& path);
Glib::ustring abspath(const Glib::ustring& path);
Glib::ustring join(const Glib::ustring& path1, const Glib::ustring& path2);

time_t getmtime(const Glib::ustring& path);

void external_filepath_open(const Glib::ustring& filepath, bool open_fold_if_no_app_error);
void external_folderpath_open(const Glib::ustring& folderpath);

} // namespace CtFileSystem
