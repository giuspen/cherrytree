/*
 * ct_misc_utils.h
 *
 * Copyright 2009-2025
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

#include "ct_types.h"
#include "ct_logging.h"
#include <gtkmm.h>
#include <gtksourceview/gtksource.h>

/*
 * Compatibility shim: gtkmm4 removed the BuiltinIconSize enum that existed in
 * gtkmm3. Multiple places in the codebase use Gtk::BuiltinIconSize. To allow
 * building the same source against gtkmm4 while keeping gtkmm3 support, we
 * declare a minimal enum with the same names under the Gtk namespace when
 * compiling against gtkmm4.
 */
#if GTKMM_MAJOR_VERSION >= 4
namespace Gtk {
    enum BuiltinIconSize {
        ICON_SIZE_MENU,
        ICON_SIZE_SMALL_TOOLBAR,
        ICON_SIZE_LARGE_TOOLBAR,
        ICON_SIZE_DND,
        ICON_SIZE_DIALOG
    };
}

/*
 * Compatibility shim: gtkmm4 removed Gdk::Point. Some code uses this for
 * storing x,y coordinates. We provide a simple struct replacement.
 */
namespace Gdk {
    struct Point {
        int x;
        int y;
        Point() : x(0), y(0) {}
        Point(int x_, int y_) : x(x_), y(y_) {}
        bool operator==(const Point& other) const { return x == other.x && y == other.y; }
        bool operator!=(const Point& other) const { return !(*this == other); }
    };
}

/*
 * Compatibility shim: gtkmm4 changed Gtk::ORIENTATION_HORIZONTAL from an enum
 * value to Gtk::Orientation::HORIZONTAL. For code compatibility, we provide
 * global constants with the old-style enum value names.
 */
namespace Gtk {
    // In gtkmm4, use Gtk::Orientation::HORIZONTAL directly.
    // These inline constants help with porting code that uses Gtk::ORIENTATION_HORIZONTAL syntax.
    inline constexpr Gtk::Orientation ORIENTATION_HORIZONTAL = Gtk::Orientation::HORIZONTAL;
    inline constexpr Gtk::Orientation ORIENTATION_VERTICAL = Gtk::Orientation::VERTICAL;
}

/*
 * Compatibility shim: gtkmm4 removed Gtk::EventBox. EventBox was used to
 * catch events for widgets that couldn't normally do so. In gtkmm4, we use
 * a regular Gtk::Widget or Gtk::Box with an event controller instead.
 * For header file declarations, we create a type alias.
 */
namespace Gtk {
    using EventBox = Gtk::Box;
}

/*
 * Compatibility shim: gtkmm4 removed Gtk::ButtonBox. ButtonBox was a
 * convenience container for arranging buttons. In gtkmm4, use Gtk::Box with
 * Gtk::Box::set_spacing() and manual layout instead. For compatibility, alias it.
 */
namespace Gtk {
    using ButtonBox = Gtk::Box;
}

/*
 * Compatibility shim: gtkmm4 changed Glib::FILE_TEST_* enum values to
 * Glib::FileTest::* scoped enum values. Provide compatibility aliases.
 */
namespace Glib {
    inline constexpr Glib::FileTest FILE_TEST_EXISTS = Glib::FileTest::EXISTS;
    inline constexpr Glib::FileTest FILE_TEST_IS_REGULAR = Glib::FileTest::IS_REGULAR;
    inline constexpr Glib::FileTest FILE_TEST_IS_SYMLINK = Glib::FileTest::IS_SYMLINK;
    inline constexpr Glib::FileTest FILE_TEST_IS_DIR = Glib::FileTest::IS_DIR;
    inline constexpr Glib::FileTest FILE_TEST_IS_EXECUTABLE = Glib::FileTest::IS_EXECUTABLE;
}

/*
 * Compatibility shim: gtkmm4 changed Gtk::POLICY_* enum values to
 * Gtk::PolicyType::* scoped enum values. Provide compatibility aliases.
 */
namespace Gtk {
    inline constexpr Gtk::PolicyType POLICY_ALWAYS = Gtk::PolicyType::ALWAYS;
    inline constexpr Gtk::PolicyType POLICY_AUTOMATIC = Gtk::PolicyType::AUTOMATIC;
    inline constexpr Gtk::PolicyType POLICY_NEVER = Gtk::PolicyType::NEVER;
    inline constexpr Gtk::PolicyType POLICY_EXTERNAL = Gtk::PolicyType::EXTERNAL;
}

/*
 * Compatibility shim: gtkmm4 replaced C-style GdkEvent* union types with
 * proper C++ event classes in the Gdk namespace. We provide typedef aliases
 * for the old C-style event types.
 * 
 * IMPORTANT: In gtkmm4, event handlers receive events by const reference,
 * not by pointer. This means function signatures using these types need to be
 * adjusted for gtkmm4. The typedefs below are for declaration purposes only.
 * Actual implementations will need conditional compilation for proper event handling.
 */
#if GTKMM_MAJOR_VERSION >= 4
// Forward declare as opaque types for gtkmm4 compatibility
struct GdkEventButton;
struct GdkEventKey;
struct GdkEventFocus;
struct GdkEventConfigure;
struct GdkEventScroll;
#endif

/*
 * Compatibility shim: gtkmm4 moved EntryIconPosition from Gtk namespace.
 */
#if GTKMM_MAJOR_VERSION >= 4
namespace Gtk {
    using EntryIconPosition = Gtk::Entry::IconPosition;
}

/*
 * Compatibility shim: gtkmm4 changed Glib regex flags from global enums to
 * scoped enums within the Regex class. Provide compatibility type aliases.
 */
namespace Glib {
    using RegexCompileFlags = Glib::Regex::CompileFlags;
    using RegexMatchFlags = Glib::Regex::MatchFlags;
}
#endif
#endif /*GTKMM_MAJOR_VERSION >= 4*/

class CtConfig;
class CtTreeIter;

namespace CtCSV {

using CtStringTable = std::vector<std::vector<std::string>>;

CtStringTable table_from_csv(const std::string& filepath);

std::string table_to_csv(const CtStringTable& table);

} // namespace CtCSV

namespace CtMiscUtil {

bool system_cmd(const char* shell_cmd, const char* cwd);

std::string get_ct_language();

std::string get_doc_extension(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt);

void filepath_extension_fix(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt, std::string& filepath);

template<class TreeOrListStore>
bool node_siblings_sort(Glib::RefPtr<TreeOrListStore> model,
                        const Gtk::TreeNodeChildren& children,
                        std::function<bool(Gtk::TreeModel::iterator&, Gtk::TreeModel::iterator&)> f_need_swap,
                        const size_t start_offset = 0u)
{
    if (children.size() <= start_offset) {
        return false;
    }
    auto next_iter = [](Gtk::TreeModel::iterator iter) { return ++iter; };
    auto sort_iteration = [&f_need_swap, &model, &next_iter, &children](size_t offset)->bool{
        bool swap_executed{false};
        #if GTKMM_MAJOR_VERSION >= 4
        // GTK4: children.begin() returns const iterator; get mutable from model via path
        auto const_iter = children.begin();
        if (!const_iter) return false;
        Gtk::TreeModel::iterator curr_sibling = model->get_iter(model->get_path(const_iter));
        #else
        Gtk::TreeModel::iterator curr_sibling = children.begin();
        #endif
        while (offset-- > 0) {
            ++curr_sibling;
            if (not curr_sibling) {
                return false;
            }
        }
        Gtk::TreeModel::iterator next_sibling{next_iter(curr_sibling)};
        while (next_sibling) {
            if (f_need_swap(curr_sibling, next_sibling)) {
                model->iter_swap(curr_sibling, next_sibling);
                swap_executed = true;
            }
            else {
                curr_sibling = next_sibling;
            }
            next_sibling = next_iter(curr_sibling);
        }
        return swap_executed;
    };
    bool swap_executed{false};
    while (sort_iteration(start_offset)) {
        if (not swap_executed) {
            swap_executed = true;
        }
    }
    return swap_executed;
}

std::string get_node_hierarchical_name(const CtTreeIter tree_iter, const char* separator="--",
                                       const bool for_filename=true, const bool root_to_leaf=true,
                                       const bool trail_node_id=false, const char* trailer="");

std::string clean_from_chars_not_for_filename(std::string filename);

Gtk::BuiltinIconSize getIconSize(int size);

void set_widget_margins(Gtk::Widget& widget, int top, int bottom, int left, int right);
Gtk::Button* dialog_add_button(Gtk::Dialog* pDialog, const char* text, Gtk::ResponseType response, const char* stock_id, const bool isDefault = false);

CtLinkEntry get_link_entry_from_property(const Glib::ustring& link);
Glib::ustring get_link_property_from_entry(const CtLinkEntry& link_entry);
Glib::ustring link_check_around_cursor(Glib::RefPtr<Gtk::TextBuffer> pTextBuffer, std::optional<Gtk::TextIter> optTextIter = std::nullopt);

/**
 * @brief Check if the the mime for a file contains a given string
 * @return
 */
bool mime_type_contains(const std::string& filepath, const char* type);

enum class URI_TYPE { LOCAL_FILEPATH, WEB_URL, UNKNOWN };
URI_TYPE get_uri_type(const std::string& uri);

void parallel_for(size_t first, size_t last, std::function<void(size_t)> f);

bool text_file_set_contents_add_cr_on_win(const std::string& filepath, const std::string& text_content);

} // namespace CtMiscUtil

namespace CtTextIterUtil {

bool extend_selection_if_collapsed_text(Gtk::TextIter& iter_sel_end, const CtTreeIter& ctTreeIter, CtMainWin* pCtMainWin);

std::optional<Glib::ustring> iter_get_tag_startingwith(const Gtk::TextIter& iter, const Glib::ustring& tag_startwith);

bool get_is_camel_case(Gtk::TextIter iter_start, int num_chars);

bool startswith(Gtk::TextIter text_iter, const gchar* pChar);

bool startswith_url(Gtk::TextIter text_iter);

Glib::ustring get_selected_text(Glib::RefPtr<Gtk::TextBuffer> pTextBuffer);

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

bool rich_text_attributes_update(const Gtk::TextIter& text_iter, const CtCurrAttributesMap& curr_attributes, CtCurrAttributesMap& delta_attributes);

using SerializeFunc = std::function<void(Gtk::TextIter& start_iter,
                                         Gtk::TextIter& end_iter,
                                         CtCurrAttributesMap& curr_attributes,
                                         CtListInfo* pCurrListInfo)>;
void generic_process_slot(const CtConfig* const pCtConfig,
                          const int start_offset,
                          const int end_offset,
                          const Glib::RefPtr<Gtk::TextBuffer>& pTextBuffer,
                          SerializeFunc serialize_func,
                          const bool list_info = false);

const gchar* get_text_iter_alignment(const Gtk::TextIter& textIter, CtMainWin* pCtMainWin);

PangoDirection get_pango_direction(const Gtk::TextIter& textIter);

int get_words_count(const Glib::RefPtr<Gtk::TextBuffer>& text_buffer);

const inline static size_t LINE_CONTENT_LIMIT{100u};
Glib::ustring get_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer, const int match_end_offset);
Glib::ustring get_line_content(const Glib::ustring& text_multiline, const int match_end_offset);
Glib::ustring get_first_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer);

} // namespace CtTextIterUtil

namespace CtStrUtil {

// copied from https://gitlab.gnome.org/GNOME/gtk.git  origin/gtk-3-24  gtkpango.c  _gtk_pango_unichar_direction
PangoDirection gtk_pango_unichar_direction(gunichar ch);
// copied from https://gitlab.gnome.org/GNOME/gtk.git  origin/gtk-3-24  gtkpango.c  _gtk_pango_find_base_dir
PangoDirection gtk_pango_find_base_dir(const gchar *text, gint length);

int gtk_pango_find_start_of_dir(const gchar* text, const PangoDirection dir);

std::vector<bool> get_rtl_for_lines(const Glib::ustring& text);

bool is_str_true(const Glib::ustring& inStr);

int is_header_anchor_name(const Glib::ustring& anchorName);

bool is_256sum(const Glib::ustring& in_string);

gint64 gint64_from_gstring(const gchar* inGstring, bool hexPrefix=false);

guint32 guint32_from_hex_chars(const char* hexChars, guint8 numChars);

std::vector<int> gstring_split_to_int(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);
std::vector<gint64> gstring_split_to_int64(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

// returned pointer must be freed with g_strfreev()
gchar** vector_to_array(const std::vector<std::string>& inVec);

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
    for (auto& word : words) {
        if (text.find(word) != String::npos) {
            if (not require_all) {
                return true;
            }
        }
        else if (require_all) {
            return false;
        }
    }
    return require_all;
}

// https://stackoverflow.com/questions/642213/how-to-implement-a-natural-sort-algorithm-in-c
int natural_compare(const Glib::ustring& left, const Glib::ustring& right);

// Returns a version of text in which all occurrences of words
// are highlighted using Pango markup
Glib::ustring highlight_words(const Glib::ustring& text,
                              std::vector<Glib::ustring> words,
                              const Glib::ustring& markup_tag = "b");

Glib::ustring get_accelerator_label(const std::string& accelerator);

std::string get_internal_link_from_http_url(std::string link_url);

std::string get_encoding(const char* const pData, const size_t dataLen);

bool is_codeset_not_utf8(const std::string& codeset);

void convert_if_not_utf8(std::string& inOutText, const bool sanitise);

bool string_any_encoding_load_into_source_buffer(const char* pChar, GtkSourceBuffer* pGtkSourceBuffer);
bool string_any_encoding_to_utf8(const char* pChar, Glib::ustring& utf8_text);

bool file_any_encoding_load_into_source_buffer(const std::string& filepath, GtkSourceBuffer* pGtkSourceBuffer);
bool file_any_encoding_to_utf8(const std::string& filepath, Glib::ustring& utf8_text);

} // namespace CtStrUtil

namespace CtFontUtil {

Glib::ustring get_font_family(const Glib::ustring& fontStr);

int get_font_size(const Glib::ustring& fontStr);

Glib::ustring get_font_str(const Glib::ustring& fontFamily, const int fontSize);

} // namespace CtFontUtil

namespace CtRgbUtil {

// todo: normalized color function, better use RGBA as inner presentation instead of strings

void set_rgb24str_from_rgb24int(guint32 rgb24Int, char* rgb24StrOut);

guint32 get_rgb24int_from_rgb24str(const char* rgb24Str);

char* set_rgb24str_from_str_any(const char* rgbStrAny, char* rgb24StrOut);

Glib::ustring rgb_to_no_white(Glib::ustring in_rgb);

std::string get_rgb24str_from_str_any(const std::string& rgbStrAny);

guint32 get_rgb24int_from_str_any(const char* rgbStrAny);

std::string rgb_to_string_48(const Gdk::RGBA& color);

std::string rgb_to_string_24(const Gdk::RGBA& color);

} // namespace CtRgbUtil

namespace str {

bool startswith(const std::string& str, const std::string& starting);

bool startswith_url(const gchar* pChar);

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

std::string xml_escape(const Glib::ustring& text);

Glib::ustring sanitize_bad_symbols(const Glib::ustring& xml_content);

Glib::ustring diacritical_to_ascii(const Glib::ustring& in_text);

Glib::ustring re_escape(const Glib::ustring& text);

Glib::ustring time_format(const std::string& format, const time_t& time);

int symb_pos_to_byte_pos(const Glib::ustring& text, int symb_pos);
int byte_pos_to_symb_pos(const Glib::ustring& text, int byte_pos);

Glib::ustring swapcase(const Glib::ustring& text);

template<class STRING>
inline STRING replace(const /* keep const to make sure source is not changed */ STRING& subjectStr, const std::string& searchStr, const std::string& replaceStr)
{
    STRING text = subjectStr; // Glib::ustring works with unicode
    size_t pos = 0;
    while ((pos = text.find(searchStr, pos)) != std::string::npos)
    {
        text.replace(pos, searchStr.size(), replaceStr);
        pos += replaceStr.size();
    }
    return text;
}

inline Glib::ustring trim(Glib::ustring s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](gunichar ch) { return !g_unichar_isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](gunichar ch) { return !g_unichar_isspace(ch); }).base(), s.end());
    return s;
}

template<typename ...Args>
std::string format(const std::string& in_str, const Args&... args)
{
    // Use fmt::runtime to indicate the format string is not a compile-time constant.
    // This avoids fmt's consteval/constexpr format-string checks which fail when
    // passing a non-constexpr c_str() from a temporary std::string.
    return fmt::format(fmt::runtime(str::replace(in_str, "%s", "{}")), args...);
}

template<class STRING>
std::vector<STRING> split(const STRING& strToSplit, const char* delimiter, const bool compress = false)
{
    // maybe replace by Glib::Regex::split_simple
    std::vector<STRING> vecOfStrings;
    gchar** arrayOfStrings = g_strsplit(strToSplit.c_str(), delimiter, -1);
    for (gchar** ptr = arrayOfStrings; *ptr; ptr++) {
        STRING curr_str = *ptr;
        if (not compress or not curr_str.empty()) {
            vecOfStrings.push_back(curr_str);
        }
    }
    g_strfreev(arrayOfStrings);
    return vecOfStrings;
}

template<class STRLIST>
std::string join(const STRLIST& cnt, const std::string& delim)
{
    std::string retStr;
    bool firstTime{true};
    for (auto& v : cnt) {
        if (not firstTime) retStr += delim;
        else firstTime = false;
        retStr += v;
    }
    return retStr;
}

template<class Vector>
std::string join_numbers(const Vector& in_numbers_vec, const gchar* delimiter=" ")
{
    std::string outString;
    bool firstIteration{true};
    for(const auto& element : in_numbers_vec)
    {
        if (not firstIteration) outString += delimiter;
        else firstIteration = false;
        outString += std::to_string(element);
    }
    return outString;
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
