/*
 * str_utils.h
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

#pragma once

#include <glibmm.h>


Glib::ustring replace_in_string(Glib::ustring &subject_str, const Glib::ustring &search_str, const Glib::ustring &replace_str);

Glib::ustring trim_string(Glib::ustring &s);

gint64 gint64_from_gstring(const gchar *in_gstring, bool force_hex=false);

guint32 get_uint_from_hex_chars(const char *hex_chars, guint8 num_chars);

std::list<Glib::ustring> gstring_split2ustring(const gchar *in_str, const gchar *delimiter, gint max_tokens=-1);

std::list<gint64> gstring_split2int64(const gchar *in_str, const gchar *delimiter, gint max_tokens=-1);

Glib::ustring ustring_join4ustring(const std::list<Glib::ustring> &in_str_list, const gchar *delimiter);

Glib::ustring ustring_join4int64(const std::list<gint64>& in_int64_list, const gchar *delimiter);

void set_rgb24_str_from_rgb24_int(guint32 rgb24_int, char *rgb24_str_out);

guint32 get_rgb24_int_from_rgb24_str(const char *rgb24_str);

void set_rgb24_str_from_str_any(const char *rgb_str_any, char *rgb24_str_out);

guint32 get_rgb24_int_from_str_any(const char *rgb_str_any);
