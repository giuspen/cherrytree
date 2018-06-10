/*
 * strutils.cc
 * 
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
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

// https://developer.gnome.org/glib/stable/glib-String-Utility-Functions.html
// g++ strutils.cc -o strutils `pkg-config glibmm-2.4 --cflags --libs`

#include <assert.h>
#include <iostream>
#include <glibmm.h>


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


std::list<Glib::ustring> gstring_split2ustring(const gchar *in_str, const gchar *delimiter, gint max_tokens=-1)
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


gint64 gint64_from_gstring(const gchar *in_gstring)
{
    return g_ascii_strtoll(in_gstring, NULL, 10);
}


std::list<gint64> gstring_split2int64(const gchar *in_str, const gchar *delimiter, gint max_tokens=-1)
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


int main(int argc, char *argv[])
{
    std::locale::global(std::locale("")); // Set the global C++ locale to the user-specified locale

    const gchar str_orig[] = ":a:bc::d:";
    const gchar str_delimiter[] = ":";

    std::cout << str_orig << std::endl;
    std::list<Glib::ustring> splitted_list = gstring_split2ustring(str_orig, str_delimiter);
    for(Glib::ustring str_elem : splitted_list)
    {
        std::cout << str_elem << std::endl;
    }
    Glib::ustring rejoined = ustring_join4ustring(splitted_list, str_delimiter);
    std::cout << rejoined << std::endl;

    assert(rejoined == str_orig);

    const gchar str_int64_orig[] = "-1,1,0,1000";
    const gchar str_delimiter_int64[] = ",";

    std::cout << str_int64_orig << std::endl;
    std::list<gint64> splitted_list_int64 = gstring_split2int64(str_int64_orig, str_delimiter_int64);
    for(gint64 int_elem : splitted_list_int64)
    {
        std::cout << int_elem << std::endl;
    }
    Glib::ustring rejoined_int64 = ustring_join4int64(splitted_list_int64, str_delimiter_int64);
    std::cout << rejoined_int64 << std::endl;

    assert(rejoined_int64 == str_int64_orig);

    Glib::ustring test_replaces_str = "one two threetwo";
    assert(replace_in_string(test_replaces_str, "two", "four") == "one four threefour");
    std::cout << test_replaces_str << std::endl;

    Glib::ustring test_trim_str = "\t one two three ";
    assert(trim_string(test_trim_str) == "one two three");
    std::cout << test_trim_str << std::endl;
}
