/*
 * ct_misc_utils.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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
#include <assert.h>
#include "ct_misc_utils.h"
#include "ct_app.h"
#include <ctime>
#include <regex>

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

Glib::RefPtr<Gsv::Buffer> CtMiscUtil::getNewTextBuffer(const std::string& syntax, const Glib::ustring& textContent)
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};
    rRetTextBuffer = Gsv::Buffer::create(CtApp::R_textTagTable);
    rRetTextBuffer->set_max_undo_levels(CtApp::P_ctCfg->limitUndoableSteps);
    if (CtConst::RICH_TEXT_ID != syntax)
    {
        rRetTextBuffer->set_style_scheme(CtApp::R_styleSchemeManager->get_scheme(CtApp::P_ctCfg->styleSchemeId));
        if (CtConst::PLAIN_TEXT_ID == syntax)
        {
            rRetTextBuffer->set_highlight_syntax(false);
        }
        else
        {
            rRetTextBuffer->set_language(CtApp::R_languageManager->get_language(syntax));
            rRetTextBuffer->set_highlight_syntax(true);
        }
        rRetTextBuffer->set_highlight_matching_brackets(true);
    }
    if (!textContent.empty())
    {
        rRetTextBuffer->begin_not_undoable_action();
        rRetTextBuffer->set_text(textContent);
        rRetTextBuffer->end_not_undoable_action();
        rRetTextBuffer->set_modified(false);
    }
    return rRetTextBuffer;
}

const Glib::ustring CtMiscUtil::getTextTagNameExistOrCreate(Glib::ustring propertyName, Glib::ustring propertyValue)
{
    const Glib::ustring tagName{propertyName + "_" + propertyValue};
    Glib::RefPtr<Gtk::TextTag> rTextTag = CtApp::R_textTagTable->lookup(tagName);
    if (!rTextTag)
    {
        bool identified{true};
        rTextTag = Gtk::TextTag::create(tagName);
        if (CtConst::TAG_WEIGHT == propertyName && CtConst::TAG_PROP_VAL_HEAVY == propertyValue)
        {
            rTextTag->property_weight() = PANGO_WEIGHT_HEAVY;
        }
        else if (CtConst::TAG_FOREGROUND == propertyName)
        {
            rTextTag->property_foreground() = propertyValue;
        }
        else if (CtConst::TAG_BACKGROUND == propertyName)
        {
            rTextTag->property_background() = propertyValue;
        }
        else if (CtConst::TAG_SCALE == propertyName)
        {
            if (CtConst::TAG_PROP_VAL_SMALL == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_SMALL;
            }
            else if (CtConst::TAG_PROP_VAL_H1 == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_XX_LARGE;
            }
            else if (CtConst::TAG_PROP_VAL_H2 == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_X_LARGE;
            }
            else if (CtConst::TAG_PROP_VAL_H3 == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_LARGE;
            }
            else if (CtConst::TAG_PROP_VAL_SUB == propertyValue || CtConst::TAG_PROP_VAL_SUP == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_X_SMALL;
                int propRise = Pango::FontDescription(CtApp::P_ctCfg->rtFont).get_size();
                if (CtConst::TAG_PROP_VAL_SUB == propertyValue)
                {
                    propRise /= -4;
                }
                else
                {
                    propRise /= 2;
                }
                rTextTag->property_rise() = propRise;
            }
            else
            {
                identified = false;
            }
        }
        else if (CtConst::TAG_STYLE == propertyName && CtConst::TAG_PROP_VAL_ITALIC == propertyValue)
        {
            rTextTag->property_style() = Pango::Style::STYLE_ITALIC;
        }
        else if (CtConst::TAG_UNDERLINE == propertyName && CtConst::TAG_PROP_VAL_SINGLE == propertyValue)
        {
            rTextTag->property_underline() = Pango::Underline::UNDERLINE_SINGLE;
        }
        else if (CtConst::TAG_JUSTIFICATION == propertyName)
        {
            if (CtConst::TAG_PROP_VAL_LEFT == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_LEFT;
            }
            else if (CtConst::TAG_PROP_VAL_RIGHT == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_RIGHT;
            }
            else if (CtConst::TAG_PROP_VAL_CENTER == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_CENTER;
            }
            else if (CtConst::TAG_PROP_VAL_FILL == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_FILL;
            }
            else
            {
                identified = false;
            }
        }
        else if (CtConst::TAG_FAMILY == propertyName && CtConst::TAG_PROP_VAL_MONOSPACE == propertyValue)
        {
            rTextTag->property_family() = CtConst::TAG_PROP_VAL_MONOSPACE;
            if (!CtApp::P_ctCfg->monospaceBg.empty())
            {
                rTextTag->property_background() = CtApp::P_ctCfg->monospaceBg;
            }
        }
        else if (CtConst::TAG_STRIKETHROUGH == propertyName && CtConst::TAG_PROP_VAL_TRUE == propertyValue)
        {
            rTextTag->property_strikethrough() = true;
        }
        else if (CtConst::TAG_LINK == propertyName && propertyValue.size() > 4)
        {
            if (CtApp::P_ctCfg->linksUnderline)
            {
                rTextTag->property_underline() = Pango::Underline::UNDERLINE_SINGLE;
            }
            Glib::ustring linkType = propertyValue.substr(0, 4);
            if (CtConst::LINK_TYPE_WEBS == linkType)
            {
                rTextTag->property_foreground() = CtApp::P_ctCfg->colLinkWebs;
            }
            else if (CtConst::LINK_TYPE_NODE == linkType)
            {
                rTextTag->property_foreground() = CtApp::P_ctCfg->colLinkNode;
            }
            else if (CtConst::LINK_TYPE_FILE == linkType)
            {
                rTextTag->property_foreground() = CtApp::P_ctCfg->colLinkFile;
            }
            else if (CtConst::LINK_TYPE_FOLD == linkType)
            {
                rTextTag->property_foreground() = CtApp::P_ctCfg->colLinkFold;
            }
            else
            {
                identified = false;
            }
        }
        else
        {
            identified = false;
        }
        if (!identified)
        {
            std::cerr << "!! unsupported propertyName=" << propertyName << " propertyValue=" << propertyValue << std::endl;
        }
        CtApp::R_textTagTable->add(rTextTag);
    }
    return tagName;
}

void CtMiscUtil::widget_set_colors(Gtk::Widget& widget, const std::string& fg, const std::string& bg,
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
    std::string hierarchical_name = str::trim(tree_iter.get_node_name());// todo: exports.clean_text_to_utf8(dad.treestore[tree_iter][1]).strip()
    CtTreeIter father_iter = tree_iter.parent();
    while (father_iter) {
        std::string father_name = str::trim(father_iter.get_node_name());// todo: exports.clean_text_to_utf8(dad.treestore[father_iter][1]).strip()
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
        if (hierarchical_name.size() > CtConst::MAX_FILE_NAME_LEN)
            hierarchical_name = hierarchical_name.substr(hierarchical_name.size() - CtConst::MAX_FILE_NAME_LEN);
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
    filename = str::replace(filename, CtConst::CHAR_SPACE, CtConst::CHAR_USCORE);
    return filename;
}


bool CtStrUtil::isStrTrue(const Glib::ustring& inStr)
{
    bool retVal{false};
    if (CtConst::TAG_PROP_VAL_TRUE == inStr.lowercase() ||
        "1" == inStr)
    {
        retVal = true;
    }
    return retVal;
}

gint64 CtStrUtil::gint64FromGstring(const gchar* inGstring, bool hexPrefix)
{
    gint64 retVal;
    if (hexPrefix || g_strrstr(inGstring, "0x"))
    {
        retVal = g_ascii_strtoll(inGstring, NULL, 16);
    }
    else
    {
        retVal = g_ascii_strtoll(inGstring, NULL, 10);
    }
    return retVal;
}

guint32 CtStrUtil::getUint32FromHexChars(const char* hexChars, guint8 numChars)
{
    char hexstring[9];
    assert(numChars < 9);
    strncpy(hexstring, hexChars, numChars);
    hexstring[numChars] = 0;
    return (guint32)strtoul(hexstring, NULL, 16);
}

std::vector<gint64> CtStrUtil::gstringSplit2int64(const gchar* inStr, const gchar* delimiter, gint max_tokens)
{
    std::vector<gint64> retVec;
    gchar** arrayOfStrings = g_strsplit(inStr, delimiter, max_tokens);
    for (gchar** ptr = arrayOfStrings; *ptr; ptr++)
    {
        gint64 curr_int = gint64FromGstring(*ptr);
        retVec.push_back(curr_int);
    }
    g_strfreev(arrayOfStrings);
    return retVec;
}

bool CtStrUtil::isPgcharInPgcharSet(const gchar* pGcharNeedle, const std::set<const gchar*>& setPgcharHaystack)
{
    bool gotcha{false};
    for (const gchar* pGcharHaystack : setPgcharHaystack)
    {
        if (0 == g_strcmp0(pGcharHaystack, pGcharNeedle))
        {
            gotcha = true;
            break;
        }
    }
    return gotcha;
}


std::string CtFontUtil::getFontFamily(const std::string& fontStr)
{
    std::vector<std::string> splFont = str::split(fontStr, " ");
    return splFont.size() > 0 ? splFont.at(0) : "";
}

std::string CtFontUtil::getFontSizeStr(const std::string& fontStr)
{
    std::vector<std::string> splFont = str::split(fontStr, " ");
    return splFont.size() > 1 ? splFont.at(1) : "";
}

std::string CtFontUtil::getFontCss(const std::string& fontStr)
{
    gchar* pFontCss = g_strdup_printf(
        "textview text {"
        "    font-family: %s;"
        "    font-size: %spx;"
        "}", getFontFamily(fontStr).c_str(), getFontSizeStr(fontStr).c_str());
    std::string fontCss(pFontCss);
    g_free(pFontCss);
    return fontCss;
}

const std::string& CtFontUtil::getFontForSyntaxHighlighting(const std::string& syntaxHighlighting)
{
    if (0 == syntaxHighlighting.compare(CtConst::RICH_TEXT_ID))
    {
        return CtApp::P_ctCfg->rtFont;
    }
    if (0 == syntaxHighlighting.compare(CtConst::PLAIN_TEXT_ID))
    {
        return CtApp::P_ctCfg->ptFont;
    }
    return CtApp::P_ctCfg->codeFont;
}

std::string CtFontUtil::getFontCssForSyntaxHighlighting(const std::string& syntaxHighlighting)
{
    return getFontCss(getFontForSyntaxHighlighting(syntaxHighlighting));
}


void CtRgbUtil::setRgb24StrFromRgb24Int(guint32 rgb24Int, char* rgb24StrOut)
{
    guint8 r = (rgb24Int >> 16) & 0xff;
    guint8 g = (rgb24Int >> 8) & 0xff;
    guint8 b = rgb24Int & 0xff;
    sprintf(rgb24StrOut, "#%.2x%.2x%.2x", r, g, b);
}

guint32 CtRgbUtil::getRgb24IntFromRgb24Str(const char* rgb24Str)
{
    const char* scanStart = g_str_has_prefix(rgb24Str, "#") ? rgb24Str + 1 : rgb24Str;
    guint8 r = (guint8)CtStrUtil::getUint32FromHexChars(scanStart, 2);
    guint8 g = (guint8)CtStrUtil::getUint32FromHexChars(scanStart+2, 2);
    guint8 b = (guint8)CtStrUtil::getUint32FromHexChars(scanStart+4, 2);
    return (r << 16 | g << 8 | b);
}

char* CtRgbUtil::setRgb24StrFromStrAny(const char* rgbStrAny, char* rgb24StrOut)
{
    const char* scanStart = g_str_has_prefix(rgbStrAny, "#") ? rgbStrAny + 1 : rgbStrAny;
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

std::string CtRgbUtil::getRgb24StrFromStrAny(const std::string& rgbStrAny)
{
    char rgb24Str[8];
    setRgb24StrFromStrAny(rgbStrAny.c_str(), rgb24Str);
    return rgb24Str;
}


guint32 CtRgbUtil::getRgb24IntFromStrAny(const char* rgbStrAny)
{
    char rgb24Str[8];
    setRgb24StrFromStrAny(rgbStrAny, rgb24Str);
    return getRgb24IntFromRgb24Str(rgb24Str);
}

std::string CtRgbUtil::rgb_to_string(Gdk::RGBA color)
{
    char rgbStrOut[16];
    sprintf(rgbStrOut, "#%.2x%.2x%.2x", color.get_red_u(), color.get_green_u(), color.get_blue_u());
    return rgbStrOut;
}

std::string CtRgbUtil::rgb_any_to_24(Gdk::RGBA color)
{
    char rgb24StrOut[16];
    CtRgbUtil::setRgb24StrFromStrAny(CtRgbUtil::rgb_to_string(color).c_str(), rgb24StrOut);
    return rgb24StrOut;
}

bool str::endswith(const std::string& str, const std::string& ending)
{
    if (str.length() >= ending.length())
        return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
    return false;
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

std::string str::time_format(const std::string& format, const std::time_t& time)
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
    return g_utf8_pointer_to_offset(text.data(), text.data() + byte_pos);
}
