/*
 * ct_misc_utils.cc
 *
 * Copyright 2009-2024
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

#include "ct_main_win.h"
#include "ct_misc_utils.h"
#include <pangomm.h>
#include <iostream>
#include <cstring>
#include "ct_const.h"
#include "ct_logging.h"
#include "ct_list.h"
#include <ctime>
#include <regex>
#include <glib/gstdio.h> // to get stats
#include <curl/curl.h>
#include <fribidi.h>

#ifndef __APPLE__
#include <uchardet/uchardet.h>
#else
#include <uchardet.h>
#endif // __APPLE__
#include <thread> // for parallel_for
#include <future> // parallel_for

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif // _WIN32

CtCSV::CtStringTable CtCSV::table_from_csv(const std::string& filepath)
{
    CtStringTable tbl_matrix;

    std::vector<std::string> tbl_row;
    constexpr char cell_tag = '"';
    constexpr char cell_sep = ',';
    constexpr char esc = '\\';
    bool in_string = false;
    bool escape_next = false;

    std::string input = Glib::file_get_contents(filepath);
    std::string cell_buff;

    for (auto ch : input) {

        if (ch == '\0') break;
        if (escape_next) {
            escape_next = false;
            cell_buff += ch;
            continue;
        }
        if (ch == esc ) {
            // `\` escapes anything `"` escapes a quote
            escape_next = true;
            continue;
        }
        bool is_newline = ch == '\n';
        if ((ch == cell_sep || is_newline) && !in_string) {
            // Close the cell
            tbl_row.emplace_back(cell_buff);
            cell_buff.clear();

            if (is_newline) {
                tbl_matrix.emplace_back(tbl_row);
                tbl_row.clear();
                continue;
            }
        }
        else if (ch == cell_tag) {
            in_string = !in_string;
        }
        else {
            cell_buff += ch;
        }
    }

    return tbl_matrix;
}

std::string CtCSV::table_to_csv(const CtStringTable& table)
{
    std::string ret_str;
    for (const auto& row : table) {
        for (const auto& cell : row) {
            ret_str += fmt::format("\"{}\"", str::replace(cell, std::string{"\""}, std::string{"\\\""}));

            if (cell != row.back()) ret_str += ",";
        }
        ret_str += "\n";
    }
    return ret_str;
}

std::string CtMiscUtil::get_ct_language()
{
    std::string retLang{CtConst::LANG_DEFAULT};
    const fs::path langcfgFilepath = fs::get_cherrytree_langcfg_filepath();
    if (fs::is_regular_file(langcfgFilepath)) {
        const std::string langTxt = str::trim(Glib::file_get_contents(langcfgFilepath.string()));
        if (vec::exists(CtConst::AVAILABLE_LANGS, langTxt)) {
            retLang = langTxt;
        }
        else {
            g_critical("Unexpected %s file content %s", langcfgFilepath.c_str(), langTxt.c_str());
        }
    }
    return retLang;
}

std::string CtMiscUtil::get_doc_extension(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt)
{
    std::string ret_val;
    if (CtDocType::XML == ctDocType) {
        if (CtDocEncrypt::False == ctDocEncrypt) {
            ret_val = CtConst::CTDOC_XML_NOENC;
        }
        else if (CtDocEncrypt::True == ctDocEncrypt) {
            ret_val = CtConst::CTDOC_XML_ENC;
        }
    }
    else if (CtDocType::SQLite == ctDocType) {
        if (CtDocEncrypt::False == ctDocEncrypt) {
            ret_val = CtConst::CTDOC_SQLITE_NOENC;
        }
        else if (CtDocEncrypt::True == ctDocEncrypt) {
            ret_val = CtConst::CTDOC_SQLITE_ENC;
        }
    }
    return ret_val;
}

void CtMiscUtil::filepath_extension_fix(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt, std::string& filepath)
{
    const std::string docExt{get_doc_extension(ctDocType, ctDocEncrypt)};
    if (docExt.empty() or not Glib::str_has_suffix(filepath, docExt)) {
        if (Glib::str_has_suffix(filepath, CtConst::CTDOC_XML_NOENC) or
            Glib::str_has_suffix(filepath, CtConst::CTDOC_XML_ENC) or
            Glib::str_has_suffix(filepath, CtConst::CTDOC_SQLITE_NOENC) or
            Glib::str_has_suffix(filepath, CtConst::CTDOC_SQLITE_ENC))
        {
            filepath = filepath.substr(0, filepath.length()-4);
        }
        if (not docExt.empty()) {
            filepath += docExt;
        }
    }
}

std::string CtMiscUtil::get_node_hierarchical_name(const CtTreeIter tree_iter, const char* separator/*="--"*/,
                                                   const bool for_filename/*=true*/, const bool root_to_leaf/*=true*/,
                                                   const bool trail_node_id/*=false*/, const char* trailer/*=""*/)
{
    std::string hierarchical_name = str::trim(tree_iter.get_node_name());
    CtTreeIter father_iter = tree_iter.parent();
    while (father_iter) {
        std::string father_name = str::trim(father_iter.get_node_name());
        if (root_to_leaf)
            hierarchical_name = father_name + separator + hierarchical_name;
        else
            hierarchical_name = hierarchical_name + separator + father_name;
        father_iter = father_iter.parent();
    }
    if (trail_node_id) {
        hierarchical_name += fmt::format("_{:d}", tree_iter.get_node_id());
    }
    if (trailer) {
        hierarchical_name += trailer;
    }
    if (for_filename) {
        hierarchical_name = clean_from_chars_not_for_filename(hierarchical_name);
        if (hierarchical_name.size() > (size_t)CtConst::MAX_FILE_NAME_LEN)
            hierarchical_name = hierarchical_name.substr(hierarchical_name.size() - (size_t)CtConst::MAX_FILE_NAME_LEN);
    }
    return hierarchical_name;
}

std::string CtMiscUtil::clean_from_chars_not_for_filename(std::string filename)
{
    filename = str::replace(filename, CtConst::CHAR_SLASH, CtConst::CHAR_MINUS);
    filename = str::replace(filename, CtConst::CHAR_BSLASH, CtConst::CHAR_MINUS);
    for (auto& str: {CtConst::CHAR_STAR, CtConst::CHAR_QUESTION, CtConst::CHAR_COLON, CtConst::CHAR_LESSER,
         CtConst::CHAR_GREATER, CtConst::CHAR_PIPE, CtConst::CHAR_DQUOTE, CtConst::CHAR_NEWLINE, CtConst::CHAR_CR}) {
        filename = str::replace(filename, str, "");
    }
    filename = str::trim(filename);
    filename = str::replace(filename, " ", CtConst::CHAR_USCORE);
    return filename;
}

Gtk::BuiltinIconSize CtMiscUtil::getIconSize(int size)
{
    switch (size) {
        case 1:  return Gtk::BuiltinIconSize::ICON_SIZE_MENU;
        case 2:  return Gtk::BuiltinIconSize::ICON_SIZE_SMALL_TOOLBAR;
        case 3:  return Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
        case 4:  return Gtk::BuiltinIconSize::ICON_SIZE_DND;
        case 5:  return Gtk::BuiltinIconSize::ICON_SIZE_DIALOG;
        default: return Gtk::BuiltinIconSize::ICON_SIZE_MENU;
    }
}

CtLinkEntry CtMiscUtil::get_link_entry(const Glib::ustring& link)
{
    CtLinkEntry link_entry;
    auto vec = str::split(link, " ");
    if (vec.empty()) return CtLinkEntry();
    link_entry.type = vec[0];
    if (link_entry.type == CtConst::LINK_TYPE_WEBS)        link_entry.webs = vec[1];
    else if (link_entry.type == CtConst::LINK_TYPE_FILE)   link_entry.file = Glib::Base64::decode(vec[1]);
    else if (link_entry.type == CtConst::LINK_TYPE_FOLD)   link_entry.fold = Glib::Base64::decode(vec[1]);
    else if (link_entry.type == CtConst::LINK_TYPE_NODE) {
        link_entry.node_id = std::stol(vec[1]);
        if (vec.size() >= 3) {
            if (vec.size() == 3) link_entry.anch = vec[2];
            else                 link_entry.anch = link.substr(vec[0].size() + vec[1].size() + 2);
        }
    } else {
        return CtLinkEntry();
    }
    return link_entry;
}

bool CtMiscUtil::mime_type_contains(const std::string &filepath, const char* type)
{
    GFile* pGFile = g_file_new_for_path(filepath.c_str());
    g_autoptr(GError) pGError{nullptr};
    GFileInfo* pGFileInfo = g_file_query_info(pGFile,
                                              "standard::*",
                                              G_FILE_QUERY_INFO_NONE,
                                              NULL,
                                              &pGError);
    const char* content_type = g_file_info_get_content_type(pGFileInfo);
    g_autofree gchar* mime_type = g_content_type_get_mime_type(content_type);
    g_object_unref(pGFileInfo);
    g_object_unref(pGFile);

    return strstr(mime_type, type);
}

bool CtMiscUtil::text_file_set_contents_add_cr_on_win(const std::string& filepath, const std::string& text_content)
{
    const char* pTextContent = text_content.c_str();
#if defined(_WIN32)
    const std::string mod_text_content = str::replace(text_content, "\n", "\r\n");
    pTextContent = mod_text_content.c_str();
#endif // _WIN32
    return g_file_set_contents(filepath.c_str(), pTextContent, -1, NULL);
}

CtMiscUtil::URI_TYPE CtMiscUtil::get_uri_type(const std::string &uri)
{
    constexpr std::array<std::string_view, 2> http_ids = {"https://", "http://"};
    constexpr std::array<std::string_view, 2> fs_ids = {"/", "C:\\\\"};
    if (str::startswith_any(uri, http_ids)) return URI_TYPE::WEB_URL;
    else if (str::startswith_any(uri, fs_ids) || Glib::file_test(uri, Glib::FILE_TEST_EXISTS)) return URI_TYPE::LOCAL_FILEPATH;
    else return URI_TYPE::UNKNOWN;
}

// analog to tbb::parallel_for
void CtMiscUtil::parallel_for(size_t first, size_t last, std::function<void(size_t)> f)
{
    size_t concur_num = std::thread::hardware_concurrency();
    if (concur_num == 0) concur_num = 4;
    if (first == last) return;
    if (last < first) return;
    if (last - first < concur_num) // to make slice calc simpler
        concur_num = last - first;
    size_t slice_item_num = (last - first) / concur_num;
    size_t slice_leftover = (last - first) % concur_num;

    std::list<std::thread> td_tasks;
    size_t td_first = first;
    for  (size_t thread_index = 0; thread_index < concur_num; ++thread_index)
    {
        std::packaged_task<void(size_t, size_t)> task([f](size_t slice_start, size_t slice_end)
        {
            for (size_t index = slice_start; index < slice_end; ++index)
                f(index);
        });

        size_t td_slice = slice_item_num;
        if (slice_leftover != 0) {
            td_slice += 1;
            slice_leftover -= 1;
        }
        if (td_first + td_slice > last)
            td_slice = last - td_first;

        td_tasks.emplace_back(std::thread(std::move(task), td_first, td_first + td_slice));
        td_first += td_slice;
    }

    for (auto& task: td_tasks)
        task.join();
}

// Returns True if the characters compose a camel case word
bool CtTextIterUtil::get_is_camel_case(Gtk::TextIter iter_start, int num_chars)
{
    Gtk::TextIter text_iter = iter_start;
    int curr_state = 0;
    auto re = Glib::Regex::create("\\w");
    for (int i = 0; i < num_chars; ++i)
    {
        auto curr_char = text_iter.get_char();
        bool alphanumeric = re->match(Glib::ustring(1, curr_char));
        if (not alphanumeric)
        {
            curr_state = -1;
            break;
        }
        if (curr_state == 0)
        {
            if (g_unichar_islower(curr_char))
                curr_state = 1;
        }
        else if (curr_state == 1)
        {
            if (g_unichar_isupper(curr_char))
                curr_state = 2;
        }
        else if (curr_state == 2)
        {
            if (g_unichar_islower(curr_char))
                curr_state = 3;
        }
        text_iter.forward_char();
    }
    return curr_state == 3;
}

bool CtTextIterUtil::rich_text_attributes_update(const Gtk::TextIter& text_iter, const CtCurrAttributesMap& curr_attributes, CtCurrAttributesMap& delta_attributes)
{
    delta_attributes.clear();
    std::vector<Glib::RefPtr<const Gtk::TextTag>> toggled_off = text_iter.get_toggled_tags(false/*toggled_on*/);
    for (const auto& r_curr_tag : toggled_off) {
        const Glib::ustring tag_name = r_curr_tag->property_name();
        if (tag_name.empty() or CtConst::GTKSPELLCHECK_TAG_NAME == tag_name) {
            continue;
        }
        if (str::startswith(tag_name, "weight_")) delta_attributes[CtConst::TAG_WEIGHT] = "";
        else if (str::startswith(tag_name, "foreground_")) delta_attributes[CtConst::TAG_FOREGROUND] = "";
        else if (str::startswith(tag_name, "background_")) delta_attributes[CtConst::TAG_BACKGROUND] = "";
        else if (str::startswith(tag_name, "style_")) delta_attributes[CtConst::TAG_STYLE] = "";
        else if (str::startswith(tag_name, "underline_")) delta_attributes[CtConst::TAG_UNDERLINE] = "";
        else if (str::startswith(tag_name, "strikethrough_")) delta_attributes[CtConst::TAG_STRIKETHROUGH] = "";
        else if (str::startswith(tag_name, "indent_")) delta_attributes[CtConst::TAG_INDENT] = "";
        else if (str::startswith(tag_name, "scale_")) delta_attributes[CtConst::TAG_SCALE] = "";
        else if (str::startswith(tag_name, "justification_")) delta_attributes[CtConst::TAG_JUSTIFICATION] = "";
        else if (str::startswith(tag_name, "link_")) delta_attributes[CtConst::TAG_LINK] = "";
        else if (str::startswith(tag_name, "family_")) delta_attributes[CtConst::TAG_FAMILY] = "";
    }
    std::vector<Glib::RefPtr<const Gtk::TextTag>> toggled_on = text_iter.get_toggled_tags(true/*toggled_on*/);
    for (const auto& r_curr_tag : toggled_on) {
        const Glib::ustring tag_name = r_curr_tag->property_name();
        if (tag_name.empty() or CtConst::GTKSPELLCHECK_TAG_NAME == tag_name) {
            continue;
        }
        if (str::startswith(tag_name, "weight_")) delta_attributes[CtConst::TAG_WEIGHT] = tag_name.substr(7);
        else if (str::startswith(tag_name, "foreground_")) delta_attributes[CtConst::TAG_FOREGROUND] = tag_name.substr(11);
        else if (str::startswith(tag_name, "background_")) delta_attributes[CtConst::TAG_BACKGROUND] = tag_name.substr(11);
        else if (str::startswith(tag_name, "scale_")) delta_attributes[CtConst::TAG_SCALE] = tag_name.substr(6);
        else if (str::startswith(tag_name, "justification_")) delta_attributes[CtConst::TAG_JUSTIFICATION] = tag_name.substr(14);
        else if (str::startswith(tag_name, "style_")) delta_attributes[CtConst::TAG_STYLE] = tag_name.substr(6);
        else if (str::startswith(tag_name, "underline_")) delta_attributes[CtConst::TAG_UNDERLINE] = tag_name.substr(10);
        else if (str::startswith(tag_name, "strikethrough_")) delta_attributes[CtConst::TAG_STRIKETHROUGH] = tag_name.substr(14);
        else if (str::startswith(tag_name, "indent_")) delta_attributes[CtConst::TAG_INDENT] = tag_name.substr(7);
        else if (str::startswith(tag_name, "link_")) delta_attributes[CtConst::TAG_LINK] = tag_name.substr(5);
        else if (str::startswith(tag_name, "family_")) delta_attributes[CtConst::TAG_FAMILY] = tag_name.substr(7);
    }
    bool anyDelta{false};
    for (const auto& currDelta : delta_attributes) {
        auto keyFound = curr_attributes.find(currDelta.first);
        if (keyFound == curr_attributes.end() or keyFound->second != currDelta.second) {
            anyDelta = true;
            break;
        }
    }
    return anyDelta;
}

void CtTextIterUtil::generic_process_slot(const CtConfig* const pCtConfig,
                                          const int start_offset,
                                          const int end_offset,
                                          const Glib::RefPtr<Gtk::TextBuffer>& rTextBuffer,
                                          SerializeFunc f_serialize_func,
                                          const bool list_info/*= false*/)
{
    CtCurrAttributesMap curr_attributes;
    CtCurrAttributesMap delta_attributes;
    for (const auto& tag_property : CtConst::TAG_PROPERTIES) {
        curr_attributes[tag_property].clear();
    }
    Gtk::TextIter curr_start_iter = rTextBuffer->get_iter_at_offset(start_offset);
    Gtk::TextIter curr_end_iter = curr_start_iter;
    Gtk::TextIter real_end_iter = end_offset == -1 ? rTextBuffer->end() : rTextBuffer->get_iter_at_offset(end_offset);

    if (CtTextIterUtil::rich_text_attributes_update(curr_end_iter, curr_attributes, delta_attributes)) {
        for (auto& currDelta : delta_attributes) {
            curr_attributes[currDelta.first] = currDelta.second;
        }
    }

    CtListInfo curr_list_info;
    bool last_was_newline{false};
    if (not curr_end_iter.backward_char()) {
        last_was_newline = true;
    }
    else {
        last_was_newline = '\n' == curr_end_iter.get_char();
        curr_end_iter.forward_char();
    }

    while (curr_end_iter.forward_char()) {
        if (curr_end_iter.compare(real_end_iter) >= 0) {
            break;
        }

        if (list_info and last_was_newline) {
            curr_list_info = CtList{pCtConfig, rTextBuffer}.get_paragraph_list_info(curr_end_iter);
        }

        last_was_newline = '\n' == curr_end_iter.get_char();

        if (CtTextIterUtil::rich_text_attributes_update(curr_end_iter, curr_attributes, delta_attributes) or
            (list_info and last_was_newline))
        {
            f_serialize_func(curr_start_iter, curr_end_iter, curr_attributes, &curr_list_info);
            for (auto& currDelta : delta_attributes) curr_attributes[currDelta.first] = currDelta.second;
            curr_start_iter = curr_end_iter;
        }
    }

    if (curr_start_iter.compare(real_end_iter) < 0) {
        f_serialize_func(curr_start_iter, real_end_iter, curr_attributes, &curr_list_info);
    }
}

const gchar* CtTextIterUtil::get_text_iter_alignment(const Gtk::TextIter& textIter, CtMainWin* pCtMainWin)
{
    const char* retVal{CtConst::TAG_PROP_VAL_LEFT};
    for (const gchar* currAlignType : CtConst::TAG_ALIGNMENTS)
    {
        const std::string tagName = pCtMainWin->get_text_tag_name_exist_or_create(CtConst::TAG_JUSTIFICATION, currAlignType);
        if (textIter.has_tag(pCtMainWin->get_text_tag_table()->lookup(tagName)))
        {
            retVal = currAlignType;
            break;
        }
    }
    return retVal;
}

PangoDirection CtTextIterUtil::get_pango_direction(const Gtk::TextIter& textIter)
{
    {
        Gtk::TextIter backwards{textIter};
        while (backwards.backward_char()) {
            const gunichar wc = backwards.get_char();
            if ('\n' == wc) {
                break;
            }
            PangoDirection retDir = CtStrUtil::gtk_pango_unichar_direction(wc);
            if (PANGO_DIRECTION_NEUTRAL != retDir) {
                return retDir;
            }
        }
    }
    {
        Gtk::TextIter forwards{textIter};
        while (forwards.forward_char()) {
            const gunichar wc = forwards.get_char();
            if ('\n' == wc) {
                break;
            }
            PangoDirection retDir = CtStrUtil::gtk_pango_unichar_direction(wc);
            if (PANGO_DIRECTION_NEUTRAL != retDir) {
                return retDir;
            }
        }
    }
    return PANGO_DIRECTION_NEUTRAL;
}

int CtTextIterUtil::get_words_count(const Glib::RefPtr<Gtk::TextBuffer>& text_buffer)
{
    int words = 0;
    Glib::ustring text = text_buffer->get_text(true);
    if (!text.empty())
    {
        int text_size = text.size();
        PangoLogAttr* attrs = g_new0(PangoLogAttr, text_size + 1);
        pango_get_log_attrs(text.c_str(), -1, 0,
                            pango_language_from_string ("C"),
                            attrs,
                            text_size + 1);
        for (int i = 0; i < text_size; ++i)
            if (attrs[i].is_word_start)
                ++words;
        g_free (attrs);
    }
    return words;
}

// copied from https://gitlab.gnome.org/GNOME/gtk.git  origin/gtk-3-24  gtkpango.c  _gtk_pango_unichar_direction
PangoDirection CtStrUtil::gtk_pango_unichar_direction(gunichar ch)
{
  FriBidiCharType fribidi_ch_type;

  G_STATIC_ASSERT (sizeof (FriBidiChar) == sizeof (gunichar));

  fribidi_ch_type = fribidi_get_bidi_type (ch);

  if (!FRIBIDI_IS_STRONG (fribidi_ch_type))
    return PANGO_DIRECTION_NEUTRAL;
  else if (FRIBIDI_IS_RTL (fribidi_ch_type))
    return PANGO_DIRECTION_RTL;
  else
    return PANGO_DIRECTION_LTR;
}
// copied from https://gitlab.gnome.org/GNOME/gtk.git  origin/gtk-3-24  gtkpango.c  _gtk_pango_find_base_dir
PangoDirection CtStrUtil::gtk_pango_find_base_dir(const gchar *text, gint length)
{
  PangoDirection dir = PANGO_DIRECTION_NEUTRAL;
  const gchar *p;

  g_return_val_if_fail (text != NULL || length == 0, PANGO_DIRECTION_NEUTRAL);

  p = text;
  while ((length < 0 || p < text + length) && *p)
    {
      gunichar wc = g_utf8_get_char (p);

      dir = CtStrUtil::gtk_pango_unichar_direction (wc);

      if (dir != PANGO_DIRECTION_NEUTRAL)
        break;

      p = g_utf8_next_char (p);
    }

  return dir;
}

int CtStrUtil::gtk_pango_find_start_of_dir(const gchar* text, const PangoDirection dir)
{
    const gchar* p{text};
    while (*p) {
        gunichar wc = g_utf8_get_char(p);
        if (CtStrUtil::gtk_pango_unichar_direction(wc) == dir) {
            return p - text;
        }
        p = g_utf8_next_char(p);
    }
    return -1;
}

std::vector<bool> CtStrUtil::get_rtl_for_lines(const Glib::ustring& text)
{
    std::vector<bool> ret_vec;
    std::vector<Glib::ustring> lines = str::split(text, "\n");
    for (const auto& curr_line : lines) {
        ret_vec.push_back(PANGO_DIRECTION_RTL == CtStrUtil::gtk_pango_find_base_dir(curr_line.c_str(), -1));
    }
    return ret_vec;
}

bool CtStrUtil::is_str_true(const Glib::ustring& inStr)
{
    bool retVal{false};
    if (CtConst::TAG_PROP_VAL_TRUE == inStr.lowercase() or
        "1" == inStr)
    {
        retVal = true;
    }
    return retVal;
}

gint64 CtStrUtil::gint64_from_gstring(const gchar* inGstring, bool hexPrefix)
{
    gint64 retVal;
    if (hexPrefix or g_strrstr(inGstring, "0x"))
    {
        retVal = g_ascii_strtoll(inGstring, nullptr, 16);
    }
    else
    {
        retVal = g_ascii_strtoll(inGstring, nullptr, 10);
    }
    return retVal;
}

guint32 CtStrUtil::guint32_from_hex_chars(const char* hexChars, guint8 numChars)
{
    char hexstring[9];
    if (numChars > 8)
    {
        numChars = 8;
    }
    strncpy(hexstring, hexChars, numChars);
    hexstring[numChars] = 0;
    return (guint32)strtoul(hexstring, nullptr, 16);
}

std::vector<int> CtStrUtil::gstring_split_to_int(const gchar* inStr, const gchar* delimiter, gint max_tokens)
{
    std::vector<int> retVec;
    gchar** arrayOfStrings = g_strsplit(inStr, delimiter, max_tokens);
    for (gchar** ptr = arrayOfStrings; *ptr; ptr++) {
        int curr_int = gint64_from_gstring(*ptr);
        retVec.push_back(curr_int);
    }
    g_strfreev(arrayOfStrings);
    return retVec;
}
std::vector<gint64> CtStrUtil::gstring_split_to_int64(const gchar* inStr, const gchar* delimiter, gint max_tokens)
{
    std::vector<gint64> retVec;
    gchar** arrayOfStrings = g_strsplit(inStr, delimiter, max_tokens);
    for (gchar** ptr = arrayOfStrings; *ptr; ptr++) {
        gint64 curr_int = gint64_from_gstring(*ptr);
        retVec.push_back(curr_int);
    }
    g_strfreev(arrayOfStrings);
    return retVec;
}

// returned pointer must be freed with g_strfreev()
gchar** CtStrUtil::vector_to_array(const std::vector<std::string>& inVec)
{
    gchar** array = g_new(gchar*, inVec.size()+1);
    for (size_t i = 0; i < inVec.size(); ++i) {
        array[i] = g_strdup(inVec[i].c_str());
    }
    array[inVec.size()] = nullptr;
    return array;
}

int CtStrUtil::natural_compare(const Glib::ustring& left, const Glib::ustring& right)
{
    enum mode_t { STRING, NUMBER } mode = STRING;
    auto l = left.begin();
    auto r = right.begin();
    while (l != left.end() && r != right.end())
    {
        if (mode == STRING)
        {
            while (l != left.end() && r != right.end())
            {
                auto l_char = *l, r_char = *r;
                const gint l_digit = g_unichar_digit_value(l_char), r_digit = g_unichar_digit_value(r_char);
                if (l_digit != -1 && r_digit != -1)
                {
                    mode = NUMBER;
                    break;
                }
                if (l_digit != -1) return -1;       // if only the left character is a digit, we have a result
                if(r_digit != -1) return +1;        // if only the right character is a digit, we have a result
                const int diff = Glib::ustring(1, l_char).compare(Glib::ustring(1, r_char)); // compute the difference of both characters
                if (diff != 0) return diff;         // if they differ we have a result
                // otherwise process the next characters
                ++l;
                ++r;
            }
        }
        else // mode = NUMBER
        {
            Glib::ustring l_number;
            for (; l != left.end() && g_unichar_digit_value(*l) != -1; ++l)
                l_number += *l;
            Glib::ustring r_number;
            for (; r != right.end() && g_unichar_digit_value(*r) != -1; ++r)
                r_number += *r;
            // converting number to integer can give INT overflow, so comparing them as strings
            int max_str_len = std::max(l_number.size(), r_number.size());
            l_number = str::repeat("0", max_str_len - l_number.size()) + l_number; // padding with '0' for easer comparing
            r_number = str::repeat("0", max_str_len - r_number.size()) + r_number; // padding with '0' for easer comparing
            const int diff = l_number.compare(r_number);
            if (diff != 0)
                return diff;
            // continue the next substring
            mode = STRING;
        }
    }
    if (l != left.end()) return +1;
    if (r != right.end()) return -1;
    return 0;
}

Glib::ustring CtStrUtil::highlight_words(const Glib::ustring& text, std::vector<Glib::ustring> words, const Glib::ustring& markup_tag /* = "b" */)
{
    if (words.empty())
        return str::xml_escape(text);
    for (auto& word: words)
        word = str::re_escape(word);

    // Build a regular expression of the form "(word1|word2|...)", matching any of the words.
    // The outer parentheses also define a capturing group, which is important (see below).
    Glib::ustring pattern = "(" + str::join(words, "|") + ")";
    auto regex = Glib::Regex::create(pattern.c_str(), Glib::RegexCompileFlags::REGEX_CASELESS);

    Glib::ustring builder;
    // Regex.split also returns capturing group matches from the "delimiter",
    // and since the entire pattern is a capturing group, the result is all of the text,
    // split into matches and non-matches of words
    for (auto part: regex->split(text)) {
        auto part_markup = str::xml_escape(part);

        // Note that while Regex.match looks for matches anywhere within a string,
        // partial matches cannot occur here because the parts are already split
        // along pattern boundaries
        if (regex->match(part)) {
            builder.append("<").append(markup_tag).append(">");
            builder.append(part_markup);
            builder.append("</").append(markup_tag).append(">");
        } else {
            builder.append(part_markup);
        }
    }

    return builder;
}

Glib::ustring CtStrUtil::get_accelerator_label(const std::string& accelerator)
{
    guint key;
    GdkModifierType mod;
    gtk_accelerator_parse(accelerator.c_str(), &key, &mod);
    g_autofree gchar* label = gtk_accelerator_get_label(key, mod);
    return Glib::ustring(label);
}

std::string CtStrUtil::get_internal_link_from_http_url(std::string link_url)
{
    if (str::startswith(link_url, "http"))          return "webs " + link_url;
    else if (str::startswith(link_url, "file://"))  return "file " + Glib::Base64::encode(link_url.substr(7));
    else                                            return "webs http://" + link_url;
}

std::string CtStrUtil::get_encoding(const char* const pData, const size_t dataLen)
{
    std::string charset;
    // look at the BOM first if present
    // https://en.wikipedia.org/wiki/Byte_order_mark
    if      (g_str_has_prefix(pData,             "\xEF\xBB\xBF"))        { charset = "UTF-8"; }
    else if (dataLen >= 4 and 0 == memcmp(pData, "\x00\x00\xFE\xFF", 4)) { charset = "UTF-32BE"; }
    else if (dataLen >= 4 and 0 == memcmp(pData, "\xFF\xFE\x00\x00", 4)) { charset = "UTF-32LE"; }
    else if (g_str_has_prefix(pData,             "\xFE\xFF"))            { charset = "UTF-16BE"; }
    else if (g_str_has_prefix(pData,             "\xFF\xFE"))            { charset = "UTF-16LE"; }
    else if (g_str_has_prefix(pData,             "\x2B\x2F\x76"))        { charset = "UTF-7"; }
    else if (g_str_has_prefix(pData,             "\xF7\x64\x4C"))        { charset = "UTF-1"; }
    else if (g_str_has_prefix(pData,             "\xDD\x73\x66\x73"))    { charset = "UTF-EBCDIC"; }
    else if (g_str_has_prefix(pData,             "\x0E\xFE\xFF"))        { charset = "SCSU"; }
    else if (g_str_has_prefix(pData,             "\xFB\xEE\x28"))        { charset = "BOCU-1"; }
    else if (g_str_has_prefix(pData,             "\x84\x31\x95\x33"))    { charset = "GB-18030"; }
    else {
        // no BOM, try with uchardet
        uchardet_t handle = uchardet_new();
        if (0 == uchardet_handle_data(handle, pData, dataLen)) {
            uchardet_data_end(handle);
            charset = uchardet_get_charset(handle);
        }
        uchardet_delete(handle);
    }
    spdlog::debug("{} -> charset: {}", dataLen, charset);
    return charset;
}

bool CtStrUtil::is_codeset_not_utf8(const std::string& codeset)
{
    return not codeset.empty() and codeset != "ASCII" and codeset != "UTF-8";
}

void CtStrUtil::convert_if_not_utf8(std::string& inOutText, const bool sanitise)
{
    const std::string codeset = CtStrUtil::get_encoding(inOutText.c_str(), inOutText.size());
    if (CtStrUtil::is_codeset_not_utf8(codeset)) {
        gsize bytes_read, bytes_written;
        g_autofree gchar* pConverted = g_convert_with_fallback(
            inOutText.c_str(), inOutText.size(), "UTF-8", codeset.c_str(), "?"/*fallback*/,
            &bytes_read, &bytes_written, NULL);
        if (pConverted) {
            // ok converted
            if (g_str_has_prefix(pConverted, "\xEF\xBB\xBF")) {
                // remove UTF-8 BOM
                inOutText = std::string{pConverted+3, bytes_written-3};
            }
            else {
                inOutText = std::string{pConverted, bytes_written};
            }
        }
    }
    else if (Glib::str_has_prefix(inOutText, "\xEF\xBB\xBF")) {
        // remove UTF-8 BOM
        inOutText = inOutText.substr(3);
    }
    if (sanitise) {
        inOutText = str::sanitize_bad_symbols(inOutText);
    }
}

Glib::ustring CtFontUtil::get_font_family(const Glib::ustring& fontStr)
{
    try {
        std::vector<Glib::ustring> fontStr_splitted = str::split(fontStr, " ");
        fontStr_splitted.pop_back();
        Glib::ustring retVal = str::join(fontStr_splitted, CtConst::CHAR_SPACE);
        return retVal;
    }
    catch (...) {
        spdlog::warn("{} {}", __FUNCTION__, fontStr.raw());
    }
    return Pango::FontDescription(fontStr).get_family();
}

int CtFontUtil::get_font_size(const Glib::ustring& fontStr)
{
    try {
        const std::vector<Glib::ustring> fontStr_splitted = str::split(fontStr, " ");
        const int retVal = std::stoi(fontStr_splitted.back());
        return retVal;
    }
    catch (...) {
        spdlog::warn("{} {}", __FUNCTION__, fontStr.raw());
    }
    return Pango::FontDescription(fontStr).get_size()/Pango::SCALE;
}

Glib::ustring CtFontUtil::get_font_str(const Glib::ustring& fontFamily, const int fontSize)
{
    return fontFamily + CtConst::CHAR_SPACE + std::to_string(fontSize);
}

void CtRgbUtil::set_rgb24str_from_rgb24int(guint32 rgb24Int, char* rgb24StrOut)
{
    guint8 r = (rgb24Int >> 16) & 0xff;
    guint8 g = (rgb24Int >> 8) & 0xff;
    guint8 b = rgb24Int & 0xff;
    sprintf(rgb24StrOut, "#%.2x%.2x%.2x", r, g, b);
}

guint32 CtRgbUtil::get_rgb24int_from_rgb24str(const char* rgb24Str)
{
    const char* scanStart = g_str_has_prefix(rgb24Str, "#") ? rgb24Str + 1 : rgb24Str;
    guint32 r = CtStrUtil::guint32_from_hex_chars(scanStart, 2);
    guint32 g = CtStrUtil::guint32_from_hex_chars(scanStart+2, 2);
    guint32 b = CtStrUtil::guint32_from_hex_chars(scanStart+4, 2);
    return (r << 16 | g << 8 | b);
}

char* CtRgbUtil::set_rgb24str_from_str_any(const char* rgbStrAny, char* rgb24StrOut)
{
    const char* scanStart = g_str_has_prefix(rgbStrAny, "#") ? rgbStrAny + 1 : rgbStrAny;
    switch(strlen(scanStart))
    {
        case 12:
        {
            guint16 r = (guint16)CtStrUtil::guint32_from_hex_chars(scanStart, 4);
            guint16 g = (guint16)CtStrUtil::guint32_from_hex_chars(scanStart+4, 4);
            guint16 b = (guint16)CtStrUtil::guint32_from_hex_chars(scanStart+8, 4);
            r >>= 8;
            g >>= 8;
            b >>= 8;
            sprintf(rgb24StrOut, "#%.2x%.2x%.2x", r, g, b);
        }
        break;
        case 6:
            sprintf(rgb24StrOut, "#%s", scanStart);
        break;
        case 3:
            sprintf(rgb24StrOut, "#%c%c%c%c%c%c", scanStart[0], scanStart[0], scanStart[1], scanStart[1], scanStart[2], scanStart[2]);
        break;
        default:
            spdlog::error("!! set_rgb24str_from_str_any {}", rgbStrAny);
            sprintf(rgb24StrOut, "#");
    }
    return rgb24StrOut;
}


Glib::ustring CtRgbUtil::rgb_to_no_white(Glib::ustring in_rgb)
{
    char out_rgb[16] = {};
    const char* scanStart = in_rgb[0] == '#' ? in_rgb.c_str() + 1 : in_rgb.c_str();
    if (strlen(scanStart) == 12)
    {
        guint32 r = CtStrUtil::guint32_from_hex_chars(scanStart, 4);
        guint32 g = CtStrUtil::guint32_from_hex_chars(scanStart+4, 4);
        guint32 b = CtStrUtil::guint32_from_hex_chars(scanStart+8, 4);
        // r+g+b black is 0
        // r+g+b white is 3*65535
        guint32 max_48 = 65535;
        if (r+g+b > 2.2 * max_48)
        {
            r = max_48 - r;
            g = max_48 - g;
            b = max_48 - b;
            sprintf(out_rgb, "#%.4x%.4x%.4x", r, g, b);
            return out_rgb;
        }
    }
    else
    {
        guint32 rgb24Int = get_rgb24int_from_str_any(scanStart);
        guint32 r = (rgb24Int >> 16) & 0xff;
        guint32 g = (rgb24Int >> 8) & 0xff;
        guint32 b = rgb24Int & 0xff;
        // r+g+b black is 0
        // r+g+b white is 3*255
        guint32 max_24 = 255;
        if (r+g+b > 2.2*max_24)
        {
            r = max_24 - r;
            g = max_24 - g;
            b = max_24 - b;
            sprintf(out_rgb, "#%.2x%.2x%.2x", r, g, b);
            return out_rgb;
        }
    }
    return in_rgb;
}

std::string CtRgbUtil::get_rgb24str_from_str_any(const std::string& rgbStrAny)
{
    char rgb24Str[8];
    set_rgb24str_from_str_any(rgbStrAny.c_str(), rgb24Str);
    return rgb24Str;
}


guint32 CtRgbUtil::get_rgb24int_from_str_any(const char* rgbStrAny)
{
    char rgb24Str[8];
    set_rgb24str_from_str_any(rgbStrAny, rgb24Str);
    return get_rgb24int_from_rgb24str(rgb24Str);
}

std::string CtRgbUtil::rgb_to_string(const Gdk::RGBA& color)
{
    char rgbStrOut[16];
    sprintf(rgbStrOut, "#%.4x%.4x%.4x", color.get_red_u(), color.get_green_u(), color.get_blue_u());
    return rgbStrOut;
}

std::string CtRgbUtil::rgb_any_to_24(const Gdk::RGBA& color)
{
    char rgb24StrOut[16];
    set_rgb24str_from_str_any(CtRgbUtil::rgb_to_string(color).c_str(), rgb24StrOut);
    return rgb24StrOut;
}

bool str::startswith(const std::string& str, const std::string& starting)
{
    if (str.length() >= starting.length())
        return (0 == str.compare(0, starting.length(), starting));
    return false;
}

bool str::endswith(const std::string& str, const std::string& ending)
{
    if (str.length() >= ending.length())
        return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
    return false;
}

int str::indexOf(const Glib::ustring& str, const Glib::ustring& lookup_str)
{
    size_t index = str.find(lookup_str);
    return index != std::string::npos ? static_cast<int>(index) : -1;
}

int str::indexOf(const Glib::ustring& str, const gunichar& uc)
{
    size_t index = str.find(Glib::ustring(1, uc));
    return index != std::string::npos ? static_cast<int>(index) : -1;
}

Glib::ustring str::xml_escape(const Glib::ustring& text)
{
    Glib::ustring buffer;
    buffer.reserve(text.size());
    for (auto ch = text.begin(); ch != text.end(); ++ch)
    {
        switch(*ch) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&#39;");       break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(1, *ch);        break;
        }
    }
    return buffer;
}

Glib::ustring str::sanitize_bad_symbols(const Glib::ustring& xml_content)
{
    // remove everything forbidden by XML 1.0 specifications
    Glib::RefPtr<Glib::Regex> re_pattern = Glib::Regex::create("[^\x09\x0A\x0D\x20-\xFF\x85\xA0-\uD7FF\uE000-\uFDCF\uFDE0-\uFFFD]");
    return re_pattern->replace(xml_content, 0/*start_position*/, "", static_cast<Glib::RegexMatchFlags>(0u));
}

struct DiacrToAscii {
    DiacrToAscii(const char* replacement_, const char* pattern_)
     : replacement{replacement_}
     , pattern{pattern_}
    {}
    Glib::ustring replacement;
    Glib::ustring pattern;
    Glib::RefPtr<Glib::Regex> pRegExp;
};
static std::list<DiacrToAscii> list_DiacrToAscii{
    {"a", "[àáâãäåāăąạ]"},
    {"A", "[ÀÁÂÃÄÅĀĂĄẠ]"},
    {"b", "[ḅ]"},
    {"B", "[Ḅ]"},
    {"c", "[çćĉċč]"},
    {"C", "[ÇĆĈĊČ]"},
    {"d", "[ďđḍ]"},
    {"D", "[ĎĐḌ]"},
    {"e", "[èéêëēĕėęěẹ]"},
    {"E", "[ÈÉÊËĒĔĖĘĚẸ]"},
    {"g", "[ĝğġģ]"},
    {"G", "[ĜĞĠĢ]"},
    {"h", "[ĥħḥ]"},
    {"H", "[ĤĦḤ]"},
    {"i", "[ìíîïĩīĭįıị]"},
    {"I", "[ÌÍÎÏĨĪĬĮİỊ]"},
    {"j", "[ĵ]"},
    {"J", "[Ĵ]"},
    {"k", "[ķḳ]"},
    {"K", "[ĶḲ]"},
    {"l", "[ĺļľŀłḷ]"},
    {"L", "[ĹĻĽĿŁḶ]"},
    {"m", "[ṃ]"},
    {"M", "[Ṃ]"},
    {"n", "[ñńņňŉṇ]"},
    {"N", "[ÑŃŅŇṆ]"},
    {"o", "[òóôõöøōŏőọ]"},
    {"O", "[ÒÓÔÕÖØŌŎŐỌ]"},
    {"r", "[ŕŗřṛ]"},
    {"R", "[ŔŖŘṚ]"},
    {"s", "[śŝşšṣ]"},
    {"S", "[ŚŜŞŠṢ]"},
    {"t", "[ţťŧṭ]"},
    {"T", "[ŢŤŦṬ]"},
    {"u", "[ùúûüũūŭůűųụ]"},
    {"U", "[ÙÚÛÜŨŪŬŮŰŲỤ]"},
    {"w", "[ŵẉ]"},
    {"W", "[ŴẈ]"},
    {"y", "[ýŷÿỵ]"},
    {"Y", "[ÝŶŸỴ]"},
    {"z", "[źżžẓ]"},
    {"Z", "[ŹŻŽẒ]"},
};
static bool list_DiacrToAscii_initialised{false};

// https://docs.oracle.com/cd/E29584_01/webhelp/mdex_basicDev/src/rbdv_chars_mapping.html
Glib::ustring str::diacritical_to_ascii(const Glib::ustring& in_text)
{
    const Glib::RegexMatchFlags re_flags{static_cast<Glib::RegexMatchFlags>(0u)};
    Glib::ustring tmp_str{in_text};
    if (not list_DiacrToAscii_initialised) {
        list_DiacrToAscii_initialised = true;
        for (DiacrToAscii& curr_DiacrToAscii : list_DiacrToAscii) {
            curr_DiacrToAscii.pRegExp = Glib::Regex::create(curr_DiacrToAscii.pattern);
        }
    }
    for (DiacrToAscii& curr_DiacrToAscii : list_DiacrToAscii) {
        if (curr_DiacrToAscii.pRegExp->match(tmp_str, re_flags)) {
            tmp_str = curr_DiacrToAscii.pRegExp->replace(tmp_str, 0/*start_position*/, curr_DiacrToAscii.replacement, re_flags);
        }
    }
    return tmp_str;
}

Glib::ustring str::re_escape(const Glib::ustring& text)
{
    return Glib::Regex::escape_string(text);
}

Glib::ustring str::time_format(const std::string& format, const time_t& time)
{
    std::tm* localtime = std::localtime(&time);
    char buffer[100];
    size_t len = strftime(buffer, sizeof(buffer), format.c_str(), localtime);
    if (len == 0)
        len = strftime(buffer, sizeof(buffer), CtConst::TIMESTAMP_FORMAT_DEFAULT, localtime);

    if (len == 0) return "";
    if (g_utf8_validate(buffer, len, NULL)) return buffer;

    g_autofree gchar* date = g_locale_to_utf8(buffer, len, NULL, NULL, NULL);
    if (date) return date;

    const std::string codeset = CtStrUtil::get_encoding(buffer, len);
    std::string converted{buffer, len};
    CtStrUtil::convert_if_not_utf8(converted, true/*sanitise*/);
    return converted;
}

int str::symb_pos_to_byte_pos(const Glib::ustring& text, int symb_pos)
{
    gchar* pointer = g_utf8_offset_to_pointer(text.data(), symb_pos);
    return (int)(pointer - text.data());
}

int str::byte_pos_to_symb_pos(const Glib::ustring& text, int byte_pos)
{
    return (int)g_utf8_pointer_to_offset(text.data(), text.data() + byte_pos);
}

Glib::ustring str::swapcase(const Glib::ustring& text)
{
    Glib::ustring ret_text;
    for (size_t index = 0; index < text.size(); ++index)
    {
        // takes every symbol and tries to figure out if it's uppercase or not
        // to change the case
        Glib::ustring test_text(text, index, 1);
        if (test_text == test_text.uppercase())
            test_text = test_text.lowercase();
        else
            test_text = test_text.uppercase();
        ret_text += test_text;
    }
    return ret_text;
}

Glib::ustring str::repeat(const Glib::ustring& input, int num)
{
    Glib::ustring ret;
    if (num <= 0) return ret;
    ret.reserve(input.size() * num);
    while (num--)
        ret += input;
    return ret;
}




