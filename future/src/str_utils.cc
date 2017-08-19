/*
 * str_utils.cc
 * 
 * Copyright 2017 giuspen <giuspen@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <iostream>
#include <string.h>
#include <assert.h>
#include "str_utils.h"


Glib::ustring replace_in_string(Glib::ustring &subject_str, const Glib::ustring &search_str, const Glib::ustring &replace_str)
{
    size_t pos = 0;
    while ((pos = subject_str.find(search_str, pos)) != std::string::npos)
    {
        subject_str.replace(pos, search_str.length(), replace_str);
        pos += replace_str.length();
    }
    return subject_str;
}


Glib::ustring trim_string(Glib::ustring &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}


gint64 gint64_from_gstring(const gchar *in_gstring, bool force_hex)
{
    gint64 ret_val;
    if(force_hex || g_strrstr(in_gstring, "0x"))
    {
        ret_val = g_ascii_strtoll(in_gstring, NULL, 16);
    }
    else
    {
        ret_val = g_ascii_strtoll(in_gstring, NULL, 10);
    }
    return ret_val;
}


guint32 get_uint_from_hex_chars(const char *hex_chars, guint8 num_chars)
{
    char hexstring[9];
    assert(num_chars < 9);
    strncpy(hexstring, hex_chars, num_chars);
    hexstring[num_chars] = 0;
    return (guint32)strtoul(hexstring, NULL, 16);
}


std::list<Glib::ustring> gstring_split2ustring(const gchar *in_str, const gchar *delimiter, gint max_tokens)
{
    std::list<Glib::ustring> ret_list;
    gchar **array_of_strings = g_strsplit(in_str, delimiter, max_tokens);
    for(gchar **ptr = array_of_strings; *ptr; ptr++)
    {
        ret_list.push_back(*ptr);
    }
    g_strfreev(array_of_strings);
    return ret_list;
}


std::list<gint64> gstring_split2int64(const gchar *in_str, const gchar *delimiter, gint max_tokens)
{
    std::list<gint64> ret_list;
    gchar **array_of_strings = g_strsplit(in_str, delimiter, max_tokens);
    for(gchar **ptr = array_of_strings; *ptr; ptr++)
    {
        gint64 curr_int = gint64_from_gstring(*ptr);
        ret_list.push_back(curr_int);
    }
    g_strfreev(array_of_strings);
    return ret_list;
}


Glib::ustring ustring_join4ustring(const std::list<Glib::ustring> &in_str_list, const gchar *delimiter)
{
    Glib::ustring ret_str;
    bool first_iteration = true;
    for(Glib::ustring element : in_str_list)
    {
        if(!first_iteration) ret_str += delimiter;
        else first_iteration = false;
        ret_str += element;
    }
    return ret_str;
}


Glib::ustring ustring_join4int64(const std::list<gint64>& in_int64_list, const gchar *delimiter)
{
    Glib::ustring ret_str;
    bool first_iteration = true;
    for(gint64 element : in_int64_list)
    {
        if(!first_iteration) ret_str += delimiter;
        else first_iteration = false;
        ret_str += std::to_string(element);
    }
    return ret_str;
}


void set_rgb24_str_from_rgb24_int(guint32 rgb24_int, char *rgb24_str_out)
{
    guint8 r = (rgb24_int >> 16) & 0xff;
    guint8 g = (rgb24_int >> 8) & 0xff;
    guint8 b = rgb24_int & 0xff;
    sprintf(rgb24_str_out, "#%.2x%.2x%.2x", r, g, b);
}


guint32 get_rgb24_int_from_rgb24_str(const char *rgb24_str)
{
    const char *scan_start = g_str_has_prefix(rgb24_str, "#") ? rgb24_str + 1 : rgb24_str;
    guint8 r = (guint8)get_uint_from_hex_chars(scan_start, 2);
    guint8 g = (guint8)get_uint_from_hex_chars(scan_start+2, 2);
    guint8 b = (guint8)get_uint_from_hex_chars(scan_start+4, 2);
    return (r << 16 | g << 8 | b);
}


void set_rgb24_str_from_str_any(const char *rgb_str_any, char *rgb24_str_out)
{
    const char *scan_start = g_str_has_prefix(rgb_str_any, "#") ? rgb_str_any + 1 : rgb_str_any;
    switch(strlen(scan_start))
    {
        case 12:
        {
            guint16 r = (guint16)get_uint_from_hex_chars(scan_start, 4);
            guint16 g = (guint16)get_uint_from_hex_chars(scan_start+4, 4);
            guint16 b = (guint16)get_uint_from_hex_chars(scan_start+8, 4);
            r >>= 8;
            g >>= 8;
            b >>= 8;
            sprintf(rgb24_str_out, "#%.2x%.2x%.2x", r, g, b);
        }
        break;
        case 6:
            sprintf(rgb24_str_out, "#%s", scan_start);
        break;
        case 3:
            sprintf(rgb24_str_out, "#%c%c%c%c%c%c", scan_start[0], scan_start[0], scan_start[1], scan_start[1], scan_start[2], scan_start[2]);
        break;
        default:
            std::cerr << "!! set_rgb24_str_from_str_any " << rgb_str_any << std::endl;
            sprintf(rgb24_str_out, "#");
    }
}


guint32 get_rgb24_int_from_str_any(const char *rgb_str_any)
{
    char rgb24_str[8];
    set_rgb24_str_from_str_any(rgb_str_any, rgb24_str);
    return get_rgb24_int_from_rgb24_str(rgb24_str);
}
