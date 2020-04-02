/*
 * ct_misc_utils.cc
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

#include <pangomm.h>
#include <iostream>
#include <string.h>
#include "ct_misc_utils.h"
#include "ct_const.h"
#include "ct_main_win.h"
#include <ctime>
#include <regex>
#include <glib/gstdio.h> // to get stats
#include <fstream>

CtDocType CtMiscUtil::get_doc_type(const std::string& fileName)
{
    CtDocType retDocType{CtDocType::None};
    if ( (Glib::str_has_suffix(fileName, CtConst::CTDOC_XML_NOENC)) or
         (Glib::str_has_suffix(fileName, CtConst::CTDOC_XML_ENC)) )
    {
        retDocType = CtDocType::XML;
    }
    else if ( (Glib::str_has_suffix(fileName, CtConst::CTDOC_SQLITE_NOENC)) or
              (Glib::str_has_suffix(fileName, CtConst::CTDOC_SQLITE_ENC)) )
    {
        retDocType = CtDocType::SQLite;
    }
    return retDocType;
}

CtDocEncrypt CtMiscUtil::get_doc_encrypt(const std::string& fileName)
{
    CtDocEncrypt retDocEncrypt{CtDocEncrypt::None};
    if ( (Glib::str_has_suffix(fileName, CtConst::CTDOC_XML_NOENC)) or
         (Glib::str_has_suffix(fileName, CtConst::CTDOC_SQLITE_NOENC)) )
    {
        retDocEncrypt = CtDocEncrypt::False;
    }
    else if ( (Glib::str_has_suffix(fileName, CtConst::CTDOC_XML_ENC)) or
              (Glib::str_has_suffix(fileName, CtConst::CTDOC_SQLITE_ENC)) )
    {
        retDocEncrypt = CtDocEncrypt::True;
    }
    return retDocEncrypt;
}

const gchar* CtMiscUtil::get_doc_extension(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt)
{
    const gchar* retVal{""};
    if (CtDocType::XML == ctDocType)
    {
        if (CtDocEncrypt::False == ctDocEncrypt)
        {
            retVal = CtConst::CTDOC_XML_NOENC;
        }
        else if (CtDocEncrypt::True == ctDocEncrypt)
        {
            retVal = CtConst::CTDOC_XML_ENC;
        }
    }
    else if (CtDocType::SQLite == ctDocType)
    {
        if (CtDocEncrypt::False == ctDocEncrypt)
        {
            retVal = CtConst::CTDOC_SQLITE_NOENC;
        }
        else if (CtDocEncrypt::True == ctDocEncrypt)
        {
            retVal = CtConst::CTDOC_SQLITE_ENC;
        }
    }
    return retVal;
}

void CtMiscUtil::filepath_extension_fix(const CtDocType ctDocType, const CtDocEncrypt ctDocEncrypt, std::string& filepath)
{
    const std::string docExt{get_doc_extension(ctDocType, ctDocEncrypt)};
    if (false == Glib::str_has_suffix(filepath, docExt))
    {
        filepath += docExt;
    }
}

void CtMiscUtil::widget_set_colors(Gtk::Widget& widget, const std::string& fg, const std::string& /*bg*/,
                       bool syntax_highl, const std::string& gdk_col_fg)
{
    if (syntax_highl) return;
    //widget.override_background_color(Gdk::RGBA(bg), Gtk::StateFlags::STATE_FLAG_NORMAL);
    widget.override_color(Gdk::RGBA(fg), Gtk::StateFlags::STATE_FLAG_NORMAL);
    Glib::RefPtr<Gtk::StyleContext> style = widget.get_style_context();
    // gtk.STATE_NORMAL, gtk.STATE_ACTIVE, gtk.STATE_PRELIGHT, gtk.STATE_SELECTED, gtk.STATE_INSENSITIVE
    widget.override_color(gdk_col_fg.empty()? style->get_color(Gtk::StateFlags::STATE_FLAG_SELECTED) : Gdk::RGBA(gdk_col_fg), Gtk::StateFlags::STATE_FLAG_SELECTED);
    widget.override_color(gdk_col_fg.empty()? style->get_color(Gtk::StateFlags::STATE_FLAG_SELECTED) : Gdk::RGBA(gdk_col_fg), Gtk::StateFlags::STATE_FLAG_ACTIVE);
    widget.override_background_color(style->get_background_color(Gtk::StateFlags::STATE_FLAG_SELECTED), Gtk::StateFlags::STATE_FLAG_ACTIVE);
}

bool CtMiscUtil::node_siblings_sort_iteration(Glib::RefPtr<Gtk::TreeStore> model, const Gtk::TreeNodeChildren& children,
                                              std::function<bool(Gtk::TreeIter&, Gtk::TreeIter&)> need_swap)
{
    if (children.empty()) return false;
    auto next_iter = [](Gtk::TreeIter iter) { return ++iter; };
    auto sort_iteration = [&need_swap, &model, &next_iter](Gtk::TreeIter curr_sibling) -> bool {
        bool swap_executed = false;
        Gtk::TreeIter next_sibling = next_iter(curr_sibling);
        while (next_sibling) {
            if (need_swap(curr_sibling, next_sibling)) {
                model->iter_swap(curr_sibling, next_sibling);
                swap_executed = true;
            } else {
                curr_sibling = next_sibling;
            }
            next_sibling = next_iter(curr_sibling);
        }
        return swap_executed;
    };

    bool swap_executed = false;
    while (sort_iteration(children.begin()))
        swap_executed = true;
    return swap_executed;
}

//"""Get the Node Hierarchical Name"""
std::string CtMiscUtil::get_node_hierarchical_name(CtTreeIter tree_iter, const char* separator/*="--"*/,
                                                   bool for_filename/*=true*/, bool root_to_leaf/*=true*/, const char* trailer/*=""*/)
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
    if (trailer)
        hierarchical_name += trailer;
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

// Returns True if one set of the Given Chars are the first of in_string
bool CtTextIterUtil::get_first_chars_of_string_are(const Glib::ustring& text, const std::vector<Glib::ustring>& chars_list)
{
    for (auto& chars: chars_list)
        if (str::startswith(text, chars))
            return true;
    return false;
}

bool CtTextIterUtil::get_next_chars_from_iter_are(Gtk::TextIter text_iter, const Glib::ustring& chars_list)
{
    for (size_t i = 0; i < chars_list.size(); ++i)
    {
        if (text_iter.get_char() != chars_list[i])
            return false;
        if (not text_iter.forward_char() and i+1 != chars_list.size())
            return false;
    }
    return true;
}

bool CtTextIterUtil::get_next_chars_from_iter_are(Gtk::TextIter text_iter, const std::vector<Glib::ustring>& chars_list_vec)
{
    for (const auto& chars_list: chars_list_vec)
        if (get_next_chars_from_iter_are(text_iter, chars_list))
            return true;
    return false;
}

// Returns True if one set of the Given Chars are the first of in_string
bool CtTextIterUtil::get_first_chars_of_string_at_offset_are(const Glib::ustring& in_string, int offset, const std::vector<Glib::ustring>& chars_list_vec)
{
    for (const auto& chars_list: chars_list_vec)
    {
        size_t len = chars_list.size();
        if (in_string.size() < (size_t)offset + len)
            continue;
        bool good = true;
        for (size_t i = 0; good and i < len; ++i)
            good = in_string[(size_t)offset + i] == chars_list[i];
        if (good)
            return true;
    }
    return false;
}

void CtTextIterUtil::rich_text_attributes_update(const Gtk::TextIter& text_iter, std::map<const gchar*, std::string>& curr_attributes)
{
    std::vector<Glib::RefPtr<const Gtk::TextTag>> toggled_off = text_iter.get_toggled_tags(false/*toggled_on*/);
    for (const auto& r_curr_tag : toggled_off)
    {
        const Glib::ustring tag_name = r_curr_tag->property_name();
        if (tag_name.empty() or CtConst::GTKSPELLCHECK_TAG_NAME == tag_name)
        {
            continue;
        }
        if (str::startswith(tag_name, "weight_")) curr_attributes[CtConst::TAG_WEIGHT] = "";
        else if (str::startswith(tag_name, "foreground_")) curr_attributes[CtConst::TAG_FOREGROUND] = "";
        else if (str::startswith(tag_name, "background_")) curr_attributes[CtConst::TAG_BACKGROUND] = "";
        else if (str::startswith(tag_name, "style_")) curr_attributes[CtConst::TAG_STYLE] = "";
        else if (str::startswith(tag_name, "underline_")) curr_attributes[CtConst::TAG_UNDERLINE] = "";
        else if (str::startswith(tag_name, "strikethrough_")) curr_attributes[CtConst::TAG_STRIKETHROUGH] = "";
        else if (str::startswith(tag_name, "scale_")) curr_attributes[CtConst::TAG_SCALE] = "";
        else if (str::startswith(tag_name, "justification_")) curr_attributes[CtConst::TAG_JUSTIFICATION] = "";
        else if (str::startswith(tag_name, "link_")) curr_attributes[CtConst::TAG_LINK] = "";
        else if (str::startswith(tag_name, "family_")) curr_attributes[CtConst::TAG_FAMILY] = "";
        else std::cerr << "Failure processing the toggling OFF tag " << tag_name << std::endl;
    }
    std::vector<Glib::RefPtr<const Gtk::TextTag>> toggled_on = text_iter.get_toggled_tags(true/*toggled_on*/);
    for (const auto& r_curr_tag : toggled_on)
    {
        const Glib::ustring tag_name = r_curr_tag->property_name();
        if (tag_name.empty() or CtConst::GTKSPELLCHECK_TAG_NAME == tag_name)
        {
            continue;
        }
        if (str::startswith(tag_name, "weight_")) curr_attributes[CtConst::TAG_WEIGHT] = tag_name.substr(7);
        else if (str::startswith(tag_name, "foreground_")) curr_attributes[CtConst::TAG_FOREGROUND] = tag_name.substr(11);
        else if (str::startswith(tag_name, "background_")) curr_attributes[CtConst::TAG_BACKGROUND] = tag_name.substr(11);
        else if (str::startswith(tag_name, "scale_")) curr_attributes[CtConst::TAG_SCALE] = tag_name.substr(6);
        else if (str::startswith(tag_name, "justification_")) curr_attributes[CtConst::TAG_JUSTIFICATION] = tag_name.substr(14);
        else if (str::startswith(tag_name, "style_")) curr_attributes[CtConst::TAG_STYLE] = tag_name.substr(6);
        else if (str::startswith(tag_name, "underline_")) curr_attributes[CtConst::TAG_UNDERLINE] = tag_name.substr(10);
        else if (str::startswith(tag_name, "strikethrough_")) curr_attributes[CtConst::TAG_STRIKETHROUGH] = tag_name.substr(14);
        else if (str::startswith(tag_name, "link_")) curr_attributes[CtConst::TAG_LINK] = tag_name.substr(5);
        else if (str::startswith(tag_name, "family_")) curr_attributes[CtConst::TAG_FAMILY] = tag_name.substr(7);
        else std::cerr << "Failure processing the toggling ON tag " << tag_name << std::endl;
    }
}

bool CtTextIterUtil::tag_richtext_toggling_on_or_off(const Gtk::TextIter& text_iter)
{
    bool retVal{false};
    std::vector<Glib::RefPtr<const Gtk::TextTag>> toggled_tags = text_iter.get_toggled_tags(false/*toggled_on*/);
    ::vec::vector_extend(toggled_tags, text_iter.get_toggled_tags(true/*toggled_on*/));
    for (const Glib::RefPtr<const Gtk::TextTag>& r_curr_tag : toggled_tags)
    {
        const Glib::ustring tag_name = r_curr_tag->property_name();
        if (tag_name.empty() or CtConst::GTKSPELLCHECK_TAG_NAME == tag_name)
        {
            continue;
        }
        if ( (str::startswith(tag_name, "weight_")) or
             (str::startswith(tag_name, "foreground_")) or
             (str::startswith(tag_name, "background_")) or
             (str::startswith(tag_name, "scale_")) or
             (str::startswith(tag_name, "justification_")) or
             (str::startswith(tag_name, "style_")) or
             (str::startswith(tag_name, "underline_")) or
             (str::startswith(tag_name, "strikethrough_")) or
             (str::startswith(tag_name, "link_")) or
             (str::startswith(tag_name, "family_")) )
        {
            retVal = true;
            break;
        }
    }
    return retVal;
}

void CtTextIterUtil::generic_process_slot(int start_offset,
                                          int end_offset,
                                          Glib::RefPtr<Gtk::TextBuffer>& text_buffer,
                                          std::function<void(Gtk::TextIter&/*start_iter*/, Gtk::TextIter&/*curr_iter*/, std::map<const gchar*, std::string>&/*curr_attributes*/)> serialize_func)
{
/*    if (end_offset == -1)
        end_offset = text_buffer->end().get_offset();

    std::map<const gchar*, std::string> curr_attributes;
     for (auto tag_property: CtConst::TAG_PROPERTIES)
         curr_attributes[tag_property] = "";
     Gtk::TextIter start_iter = text_buffer->get_iter_at_offset(start_offset);
     Gtk::TextIter curr_iter = start_iter;
     CtTextIterUtil::rich_text_attributes_update(curr_iter, curr_attributes);

     bool tag_found = curr_iter.forward_to_tag_toggle(Glib::RefPtr<Gtk::TextTag>{nullptr});
     bool one_more_serialize = true;
     while (tag_found)
     {
         if (curr_iter.get_offset() > end_offset)
             curr_iter = text_buffer->get_iter_at_offset(end_offset);
         serialize_func(start_iter, curr_iter, curr_attributes);

         int offset_old = curr_iter.get_offset();
         if (offset_old >= end_offset)
         {
             one_more_serialize = false;
             break;
         }
         else
         {
             CtTextIterUtil::rich_text_attributes_update(curr_iter, curr_attributes);
             start_iter.set_offset(offset_old);
             tag_found = curr_iter.forward_to_tag_toggle(Glib::RefPtr<Gtk::TextTag>{nullptr});
             if (curr_iter.get_offset() == offset_old)
             {
                 one_more_serialize = false;
                 break;
             }
         }
     }
     if (one_more_serialize)
     {
         if (curr_iter.get_offset() > end_offset)
             curr_iter = text_buffer->get_iter_at_offset(end_offset);
         serialize_func(start_iter, curr_iter, curr_attributes);
     }
     */
    // todo: make the upper code less ugly
    // if there is an issue, then try the upper code

    std::map<const gchar*, std::string> curr_attributes;
    for (auto tag_property: CtConst::TAG_PROPERTIES)
        curr_attributes[tag_property] = "";

    Gtk::TextIter curr_start_iter = text_buffer->get_iter_at_offset(start_offset);
    Gtk::TextIter curr_end_iter = curr_start_iter;
    Gtk::TextIter real_end_iter = end_offset == -1 ? text_buffer->end() : text_buffer->get_iter_at_offset(end_offset);

    CtTextIterUtil::rich_text_attributes_update(curr_end_iter, curr_attributes);
    while (curr_end_iter.forward_to_tag_toggle(Glib::RefPtr<Gtk::TextTag>{nullptr}))
    {
        if (curr_end_iter.compare(real_end_iter) >= 0)
            break;

        serialize_func(curr_start_iter, curr_end_iter, curr_attributes);

        CtTextIterUtil::rich_text_attributes_update(curr_end_iter, curr_attributes);
        curr_start_iter = curr_end_iter;
    }

    if (curr_start_iter.compare(real_end_iter) < 0)
    {
        serialize_func(curr_start_iter, real_end_iter, curr_attributes);
    }

}

const gchar* CtTextIterUtil::get_text_iter_alignment(const Gtk::TextIter& textIter, CtMainWin* pCtMainWin)
{
    const char* retVal{CtConst::TAG_PROP_VAL_LEFT};
    for (const char* currAlignType : std::list{CtConst::TAG_PROP_VAL_LEFT,
                                               CtConst::TAG_PROP_VAL_CENTER,
                                               CtConst::TAG_PROP_VAL_FILL,
                                               CtConst::TAG_PROP_VAL_RIGHT})
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

std::vector<gint64> CtStrUtil::gstring_split_to_int64(const gchar* inStr, const gchar* delimiter, gint max_tokens)
{
    std::vector<gint64> retVec;
    gchar** arrayOfStrings = g_strsplit(inStr, delimiter, max_tokens);
    for (gchar** ptr = arrayOfStrings; *ptr; ptr++)
    {
        gint64 curr_int = gint64_from_gstring(*ptr);
        retVec.push_back(curr_int);
    }
    g_strfreev(arrayOfStrings);
    return retVec;
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


std::string CtFontUtil::get_font_family(const std::string& fontStr)
{
    return Pango::FontDescription(fontStr).get_family();
}

std::string CtFontUtil::get_font_size_str(const std::string& fontStr)
{
    return std::to_string(Pango::FontDescription(fontStr).get_size()/Pango::SCALE);
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
            std::cerr << "!! set_rgb24str_from_str_any " << rgbStrAny << std::endl;
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

std::string CtRgbUtil::rgb_to_string(Gdk::RGBA color)
{
    char rgbStrOut[16];
    sprintf(rgbStrOut, "#%.4x%.4x%.4x", color.get_red_u(), color.get_green_u(), color.get_blue_u());
    return rgbStrOut;
}

std::string CtRgbUtil::rgb_any_to_24(Gdk::RGBA color)
{
    char rgb24StrOut[16];
    CtRgbUtil::set_rgb24str_from_str_any(CtRgbUtil::rgb_to_string(color).c_str(), rgb24StrOut);
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

std::string str::xml_escape(const std::string& text)
{
    std::string buffer;
    buffer.reserve(text.size());
    for(size_t pos = 0; pos != text.size(); ++pos) {
        switch(text[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&text[pos], 1); break;
        }
    }
    return buffer;
}

std::string str::re_escape(const std::string& text)
{
    return Glib::Regex::escape_string(text);
}

std::string str::time_format(const std::string& format, const gint64& time)
{
    std::tm* localtime = std::localtime(&time);
    char buffer[100];
    if (strftime(buffer, sizeof(buffer), format.c_str(), localtime))
        return buffer;
    else if (strftime(buffer, sizeof(buffer), CtConst::TIMESTAMP_FORMAT_DEFAULT, localtime))
        return buffer;
    return "";
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

std::string CtFileSystem::get_proper_platform_filepath(std::string filepath)
{
    if (CtConst::IS_WIN_OS)
        filepath = str::replace(filepath, CtConst::CHAR_SLASH, CtConst::CHAR_BSLASH);
    else
        filepath = str::replace(filepath, CtConst::CHAR_BSLASH, CtConst::CHAR_SLASH);
    return filepath;
}

void CtFileSystem::copy_file(Glib::ustring from_file, Glib::ustring to_file)
{
    std::ifstream  src(from_file, std::ios::binary);
    std::ofstream  dst(to_file,   std::ios::binary);
    dst << src.rdbuf();
}

std::string CtFileSystem::abspath(const std::string& path)
{
    Glib::RefPtr<Gio::File> rFile = Gio::File::create_for_path(path);
    return rFile->get_path();
}

time_t CtFileSystem::getmtime(const std::string& path)
{
    time_t time = 0;
    GStatBuf st;
    if (g_stat(path.c_str(), &st) == 0)
        time = st.st_mtime;
    return time;
}

int CtFileSystem::getsize(const std::string& path)
{
    GStatBuf st;
    if (g_stat(path.c_str(), &st) == 0)
        return st.st_size;
    return 0;
}

// Open Filepath with External App
void CtFileSystem::external_filepath_open(const std::string& filepath, bool open_fold_if_no_app_error)
{
    /* todo:
    if self.filelink_custom_action[0]:
        if cons.IS_WIN_OS: filepath = cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE
        else: filepath = re.escape(filepath)
        subprocess.call(self.filelink_custom_action[1] % filepath, shell=True)
    else:
        if cons.IS_WIN_OS:
            try: os.startfile(filepath)
            except:
                if open_fold_if_no_app_error: os.startfile(os.path.dirname(filepath))
        else: subprocess.call(config.LINK_CUSTOM_ACTION_DEFAULT_FILE % re.escape(filepath), shell=True)
        */
    g_app_info_launch_default_for_uri(("file://" + filepath).c_str(), nullptr, nullptr);
}

// Open Folderpath with External App
void CtFileSystem::external_folderpath_open(const std::string& folderpath)
{
    /* todo:
    if self.folderlink_custom_action[0]:
        if cons.IS_WIN_OS: filepath = cons.CHAR_DQUOTE + filepath + cons.CHAR_DQUOTE
        else: filepath = re.escape(filepath)
        subprocess.call(self.folderlink_custom_action[1] % filepath, shell=True)
    else:
        if cons.IS_WIN_OS: os.startfile(filepath)
        else: subprocess.call(config.LINK_CUSTOM_ACTION_DEFAULT_FILE % re.escape(filepath), shell=True)
        */

    // https://stackoverflow.com/questions/42442189/how-to-open-spawn-a-file-with-glib-gtkmm-in-windows
#ifdef _WIN32
    //ShellExecute(NULL, "open", relatedEntry->get_text().c_str(), NULL, NULL, SW_SHOWDEFAULT);
    g_warning ("Failed to open uri: %s", folderpath.c_str());
#elif defined(__APPLE__)
    //system(("open " + relatedEntry->get_text()).c_str());
#else
    gchar *path = g_filename_to_uri(folderpath.c_str(), NULL, NULL);
    Glib::ustring xgd = "xdg-open " + std::string(path);
    system(xgd.c_str());
    g_free(path);
#endif
}

Glib::ustring CtFileSystem::prepare_export_folder(Glib::ustring dir_place, Glib::ustring new_folder, bool overwrite_existing)
{
    if (Glib::file_test(Glib::build_filename(dir_place, new_folder), Glib::FILE_TEST_IS_DIR))
    {
        // todo:
        if (overwrite_existing)
            throw "work in progress"; // todo: shutil.rmtree(os.path.join(dir_place, new_folder))

        int n = 2;
        while (Glib::file_test(Glib::build_filename(dir_place, new_folder + str::format("{:03d}", n)), Glib::FILE_TEST_IS_DIR))
            n += 1;
        new_folder += str::format("{:03d}", n);
    }
    return new_folder;
}

