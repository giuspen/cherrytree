/*
 * ct_misc_utils.h
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

namespace CtStrUtil {

Glib::ustring replaceInString(Glib::ustring& subjectStr, const Glib::ustring& searchStr, const Glib::ustring& replaceStr);

Glib::ustring trimString(Glib::ustring& s);

gint64 gint64FromGstring(const gchar* inGstring, bool hexPrefix=false);

guint32 getUint32FromHexChars(const char* hexChars, guint8 numChars);

std::list<Glib::ustring> gstringSplit2ustring(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

std::list<gint64> gstringSplit2int64(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

Glib::ustring ustringJoin4ustring(const std::list<Glib::ustring>& inStrList, const gchar* delimiter);

Glib::ustring ustringJoin4int64(const std::list<gint64>& inInt64List, const gchar* delimiter);

} // namespace CtStrUtil

namespace CtRgbUtil {

void setRgb24StrFromRgb24Int(guint32 rgb24Int, char* rgb24StrOut);

guint32 getRgb24IntFromRgb24Str(const char* rgb24Str);

char* setRgb24StrFromStrAny(const char* rgbStrAny, char* rgb24StrOut);

guint32 getRgb24IntFromStrAny(const char* rgbStrAny);

} // namespace CtRgbUtil
