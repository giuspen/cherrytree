/*
 * ct_misc_utils.cc
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

#include <iostream>
#include <string.h>
#include <assert.h>
#include "ct_misc_utils.h"

CtDocType CtMiscUtil::getDocType(std::string fileName)
{
    CtDocType retDocType{CtDocType::None};
    if ( (Glib::str_has_suffix(fileName, ".ctd")) ||
         (Glib::str_has_suffix(fileName, ".ctz")) )
    {
        retDocType = CtDocType::XML;
    }
    else if ( (Glib::str_has_suffix(fileName, ".ctb")) ||
              (Glib::str_has_suffix(fileName, ".ctx")) )
    {
        retDocType = CtDocType::SQLite;
    }
    return retDocType;
}

CtDocEncrypt CtMiscUtil::getDocEncrypt(std::string fileName)
{
    CtDocEncrypt retDocEncrypt{CtDocEncrypt::None};
    if ( (Glib::str_has_suffix(fileName, ".ctd")) ||
         (Glib::str_has_suffix(fileName, ".ctb")) )
    {
        retDocEncrypt = CtDocEncrypt::False;
    }
    else if ( (Glib::str_has_suffix(fileName, ".ctz")) ||
              (Glib::str_has_suffix(fileName, ".ctx")) )
    {
        retDocEncrypt = CtDocEncrypt::True;
    }
    return retDocEncrypt;
}


Glib::ustring CtStrUtil::replaceInString(Glib::ustring &subjectStr, const Glib::ustring &searchStr, const Glib::ustring &replaceStr)
{
    size_t pos = 0;
    while ((pos = subjectStr.find(searchStr, pos)) != std::string::npos)
    {
        subjectStr.replace(pos, searchStr.length(), replaceStr);
        pos += replaceStr.length();
    }
    return subjectStr;
}

Glib::ustring CtStrUtil::trimString(Glib::ustring &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

gint64 CtStrUtil::gint64FromGstring(const gchar *inGstring, bool hexPrefix)
{
    gint64 retVal;
    if(hexPrefix || g_strrstr(inGstring, "0x"))
    {
        retVal = g_ascii_strtoll(inGstring, NULL, 16);
    }
    else
    {
        retVal = g_ascii_strtoll(inGstring, NULL, 10);
    }
    return retVal;
}

guint32 CtStrUtil::getUint32FromHexChars(const char *hexChars, guint8 numChars)
{
    char hexstring[9];
    assert(numChars < 9);
    strncpy(hexstring, hexChars, numChars);
    hexstring[numChars] = 0;
    return (guint32)strtoul(hexstring, NULL, 16);
}

std::list<Glib::ustring> CtStrUtil::gstringSplit2ustring(const gchar *inStr, const gchar *delimiter, gint max_tokens)
{
    std::list<Glib::ustring> retList;
    gchar **arrayOfStrings = g_strsplit(inStr, delimiter, max_tokens);
    for(gchar **ptr = arrayOfStrings; *ptr; ptr++)
    {
        retList.push_back(*ptr);
    }
    g_strfreev(arrayOfStrings);
    return retList;
}

std::list<gint64> CtStrUtil::gstringSplit2int64(const gchar *inStr, const gchar *delimiter, gint max_tokens)
{
    std::list<gint64> retList;
    gchar **arrayOfStrings = g_strsplit(inStr, delimiter, max_tokens);
    for(gchar **ptr = arrayOfStrings; *ptr; ptr++)
    {
        gint64 curr_int = gint64FromGstring(*ptr);
        retList.push_back(curr_int);
    }
    g_strfreev(arrayOfStrings);
    return retList;
}

Glib::ustring CtStrUtil::ustringJoin4ustring(const std::list<Glib::ustring> &inStrList, const gchar *delimiter)
{
    Glib::ustring retStr;
    bool firstIteration = true;
    for(Glib::ustring element : inStrList)
    {
        if(!firstIteration) retStr += delimiter;
        else firstIteration = false;
        retStr += element;
    }
    return retStr;
}

Glib::ustring CtStrUtil::ustringJoin4int64(const std::list<gint64>& inInt64List, const gchar *delimiter)
{
    Glib::ustring retStr;
    bool firstIteration{true};
    for(gint64 element : inInt64List)
    {
        if(!firstIteration) retStr += delimiter;
        else firstIteration = false;
        retStr += std::to_string(element);
    }
    return retStr;
}


void CtRgbUtil::setRgb24StrFromRgb24Int(guint32 rgb24Int, char *rgb24StrOut)
{
    guint8 r = (rgb24Int >> 16) & 0xff;
    guint8 g = (rgb24Int >> 8) & 0xff;
    guint8 b = rgb24Int & 0xff;
    sprintf(rgb24StrOut, "#%.2x%.2x%.2x", r, g, b);
}

guint32 CtRgbUtil::getRgb24IntFromRgb24Str(const char *rgb24Str)
{
    const char *scanStart = g_str_has_prefix(rgb24Str, "#") ? rgb24Str + 1 : rgb24Str;
    guint8 r = (guint8)CtStrUtil::getUint32FromHexChars(scanStart, 2);
    guint8 g = (guint8)CtStrUtil::getUint32FromHexChars(scanStart+2, 2);
    guint8 b = (guint8)CtStrUtil::getUint32FromHexChars(scanStart+4, 2);
    return (r << 16 | g << 8 | b);
}

char* CtRgbUtil::setRgb24StrFromStrAny(const char* rgbStrAny, char* rgb24StrOut)
{
    const char *scanStart = g_str_has_prefix(rgbStrAny, "#") ? rgbStrAny + 1 : rgbStrAny;
    switch(strlen(scanStart))
    {
        case 12:
        {
            guint16 r = (guint16)CtStrUtil::getUint32FromHexChars(scanStart, 4);
            guint16 g = (guint16)CtStrUtil::getUint32FromHexChars(scanStart+4, 4);
            guint16 b = (guint16)CtStrUtil::getUint32FromHexChars(scanStart+8, 4);
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
            std::cerr << "!! setRgb24StrFromStrAny " << rgbStrAny << std::endl;
            sprintf(rgb24StrOut, "#");
    }
    return rgb24StrOut;
}

guint32 CtRgbUtil::getRgb24IntFromStrAny(const char* rgbStrAny)
{
    char rgb24Str[8];
    setRgb24StrFromStrAny(rgbStrAny, rgb24Str);
    return getRgb24IntFromRgb24Str(rgb24Str);
}
