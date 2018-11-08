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
#include <set>

enum class CtDocType : int {None=0, XML=1, SQLite=2};
enum class CtDocEncrypt : int {None=0, True=1, False=2};

namespace CtMiscUtil {

CtDocType getDocType(std::string fileName);

CtDocEncrypt getDocEncrypt(std::string fileName);

const Glib::ustring getTextTagNameExistOrCreate(Glib::ustring propertyName, Glib::ustring propertyValue);

} // namespace CtMiscUtil

namespace CtStrUtil {

bool isStrTrue(const Glib::ustring& inStr);

std::string replaceInString(std::string& subjectStr, const std::string& searchStr, const std::string& replaceStr);
Glib::ustring replaceInString(Glib::ustring& subjectStr, const Glib::ustring& searchStr, const Glib::ustring& replaceStr);

Glib::ustring trimString(Glib::ustring& s);

gint64 gint64FromGstring(const gchar* inGstring, bool hexPrefix=false);

guint32 getUint32FromHexChars(const char* hexChars, guint8 numChars);

std::list<Glib::ustring> gstringSplit2ustring(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

std::list<gint64> gstringSplit2int64(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

Glib::ustring ustringJoin4ustring(const std::list<Glib::ustring>& inStrList, const gchar* delimiter);

Glib::ustring ustringJoin4int64(const std::list<gint64>& inInt64List, const gchar* delimiter);

bool isPgcharInPgcharSet(const gchar* pGcharNeedle, const std::set<const gchar*>& setPgcharHaystack);

} // namespace CtStrUtil

namespace CtRgbUtil {

void setRgb24StrFromRgb24Int(guint32 rgb24Int, char* rgb24StrOut);

guint32 getRgb24IntFromRgb24Str(const char* rgb24Str);

char* setRgb24StrFromStrAny(const char* rgbStrAny, char* rgb24StrOut);

guint32 getRgb24IntFromStrAny(const char* rgbStrAny);

} // namespace CtRgbUtil
