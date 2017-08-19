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


Glib::ustring ustring_join4ustring(std::list<Glib::ustring> &in_str_list, const gchar *delimiter)
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


Glib::ustring ustring_join4int64(std::list<gint64>& in_int64_list, const gchar *delimiter)
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


void set_rgb24_str_from_int24(guint32 int24, char *foreground_rgb24)
{
    guint8 r = (int24 >> 16) & 0xff;
    guint8 g = (int24 >> 8) & 0xff;
    guint8 b = int24 & 0xff;
    sprintf(foreground_rgb24, "#%.2x%.2x%.2x", r, g, b);
}
