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

template<class String> String replaceInString(String& subjectStr, const gchar* searchStr, const gchar* replaceStr)
{
    size_t pos = 0;
    while ((pos = subjectStr.find(searchStr, pos)) != std::string::npos)
    {
        subjectStr.replace(pos, strlen(searchStr), replaceStr);
        pos += strlen(replaceStr);
    }
    return subjectStr;
}

template<class String> String trimString(String& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

gint64 gint64FromGstring(const gchar* inGstring, bool hexPrefix=false);

guint32 getUint32FromHexChars(const char* hexChars, guint8 numChars);

template<class VecOfStrings> void gstringSplit2string(const gchar *inStr, VecOfStrings& vecOfStrings, const gchar *delimiter=" ", gint max_tokens=-1)
{
    gchar **arrayOfStrings = g_strsplit(inStr, delimiter, max_tokens);
    for(gchar **ptr = arrayOfStrings; *ptr; ptr++)
    {
        vecOfStrings.push_back(*ptr);
    }
    g_strfreev(arrayOfStrings);
}

std::vector<gint64> gstringSplit2int64(const gchar* inStr, const gchar* delimiter, gint max_tokens=-1);

template<class String> void stringJoin4string(const std::vector<String>& inStrVec, String& outString, const gchar* delimiter=" ")
{
    bool firstIteration{true};
    for (const String& element : inStrVec)
    {
        if(!firstIteration) outString += delimiter;
        else firstIteration = false;
        outString += element;
    }
}

template<class String> void stringJoin4int64(const std::vector<gint64>& inInt64Vec, String& outString, const gchar* delimiter=" ")
{
    bool firstIteration{true};
    for(const gint64& element : inInt64Vec)
    {
        if(!firstIteration) outString += delimiter;
        else firstIteration = false;
        outString += std::to_string(element);
    }
}

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

guint32 getRgb24IntFromStrAny(const char* rgbStrAny);

} // namespace CtRgbUtil
