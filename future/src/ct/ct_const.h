/*
 * ct_extern const.h
 *
 * Copyright 2009-2020
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <glibmm.h>
#include <array>
#include "config.h"

namespace CtConst {

const constexpr gchar* CT_VERSION  {PACKAGE_VERSION};
const constexpr gchar* APP_NAME    {PACKAGE_NAME};

const constexpr gchar* CTDOC_XML_NOENC          {".ctd"};
const constexpr gchar* CTDOC_XML_ENC            {".ctz"};
const constexpr gchar* CTDOC_SQLITE_NOENC       {".ctb"};
const constexpr gchar* CTDOC_SQLITE_ENC         {".ctx"};
const constexpr gchar* LINK_TYPE_WEBS           {"webs"};
const constexpr gchar* LINK_TYPE_FILE           {"file"};
const constexpr gchar* LINK_TYPE_FOLD           {"fold"};
const constexpr gchar* LINK_TYPE_NODE           {"node"};
const constexpr gchar* NODE_ICON_TYPE_CHERRY    {"c"};
const constexpr gchar* NODE_ICON_TYPE_CUSTOM    {"b"};
const constexpr gchar* NODE_ICON_TYPE_NONE      {"n"};
const constexpr gchar* CHERRY_RED               {"cherry_red"};
const constexpr gchar* CHERRY_BLUE              {"cherry_blue"};
const constexpr gchar* CHERRY_ORANGE            {"cherry_orange"};
const constexpr gchar* CHERRY_CYAN              {"cherry_cyan"};
const constexpr gchar* CHERRY_ORANGE_DARK       {"cherry_orange_dark"};
const constexpr gchar* CHERRY_SHERBERT          {"cherry_sherbert"};
const constexpr gchar* CHERRY_YELLOW            {"cherry_yellow"};
const constexpr gchar* CHERRY_GREEN             {"cherry_green"};
const constexpr gchar* CHERRY_PURPLE            {"cherry_purple"};
const constexpr gchar* CHERRY_BLACK             {"cherry_black"};
const constexpr gchar* CHERRY_GRAY              {"cherry_grey"};
const constexpr gchar* RICH_TEXT_ID             {"custom-colors"};
const constexpr gchar* TABLE_CELL_TEXT_ID       {"table-cell-text"};
const constexpr gchar* PLAIN_TEXT_ID            {"plain-text"};
const constexpr gchar* STYLE_APPLIED_ID          {"<style-applied>"};
const constexpr gchar* SYN_HIGHL_BASH           {"sh"};
const constexpr gchar* STYLE_SCHEME_LIGHT       {"classic"};
const constexpr gchar* STYLE_SCHEME_DARK        {"cobalt"};
const constexpr gchar* STYLE_SCHEME_GRAY        {"oblivion"};
const constexpr gchar* TIMESTAMP_FORMAT_DEFAULT {"%Y/%m/%d - %H:%M"};

const constexpr gchar* SPECIAL_CHARS_DEFAULT     {"“”„‘’•◇▪▸☐☑☒★…‰€©®™°↓↑→←↔↵⇓⇑⇒⇐⇔»«▼▲►◄≤≥≠≈±¹²³½¼⅛×÷∞ø∑σ√∫ΔδΠπΣΦΩωαβγεηλμ☺☻☼♥♣♦✔♀♂♪♫✝"};
const constexpr gchar* SPECIAL_CHAR_ARROW_RIGHT   {"→"};
const constexpr gchar* SPECIAL_CHAR_ARROW_RIGHT2  {"⇒"};
const constexpr gchar* SPECIAL_CHAR_ARROW_LEFT    {"←"};
const constexpr gchar* SPECIAL_CHAR_ARROW_LEFT2   {"⇐"};
const constexpr gchar* SPECIAL_CHAR_ARROW_DOUBLE  {"↔"};
const constexpr gchar* SPECIAL_CHAR_ARROW_DOUBLE2 {"⇔"};
const constexpr gchar* SPECIAL_CHAR_COPYRIGHT     {"©"};
const constexpr gchar* SPECIAL_CHAR_UNREGISTERED_TRADEMARK {"™"};
const constexpr gchar* SPECIAL_CHAR_REGISTERED_TRADEMARK   {"®"};
const constexpr gchar* SELWORD_CHARS_DEFAULT {".-@"};
const constexpr gchar* CHARS_LISTBUL_DEFAULT {"•◇▪-→⇒"};
const constexpr gchar* CHARS_TOC_DEFAULT     {"▸•◇▪"};
const constexpr gchar* CHARS_TODO_DEFAULT    {"☐☑☒"};
const constexpr gchar* CHARS_SMART_DQUOTE_DEFAULT    {"“”"};
const constexpr gchar* CHARS_SMART_SQUOTE_DEFAULT    {"‘’"};

const constexpr gchar* COLOR_48_LINK_WEBS   {"#00008989ffff"};
const constexpr gchar* COLOR_48_LINK_NODE   {"#071c838e071c"};
const constexpr gchar* COLOR_48_LINK_FILE   {"#8b8b69691414"};
const constexpr gchar* COLOR_48_LINK_FOLD   {"#7f7f7f7f7f7f"};
const constexpr gchar* COLOR_48_YELLOW      {"#bbbbbbbb0000"};
const constexpr gchar* COLOR_48_WHITE       {"#ffffffffffff"};
const constexpr gchar* COLOR_48_BLACK       {"#000000000000"};
const constexpr gchar* COLOR_24_BLACK       {"#000000"};
const constexpr gchar* COLOR_24_WHITE       {"#ffffff"};
const constexpr gchar* COLOR_24_BLUEBG      {"#001b33"};
const constexpr gchar* COLOR_24_LBLACK      {"#0b0c0c"};
const constexpr gchar* COLOR_24_GRAY        {"#e0e0e0"};
const constexpr gchar* DEFAULT_MONOSPACE_BG {"#7f7f7f"};
const constexpr gchar* RICH_TEXT_DARK_FG      {COLOR_24_WHITE};
const constexpr gchar* RICH_TEXT_DARK_BG      {COLOR_24_BLUEBG};
const constexpr gchar* RICH_TEXT_LIGHT_FG     {COLOR_24_BLACK};
const constexpr gchar* RICH_TEXT_LIGHT_BG     {COLOR_24_WHITE};
const constexpr gchar* TREE_TEXT_DARK_FG      {COLOR_24_WHITE};
const constexpr gchar* TREE_TEXT_DARK_BG      {COLOR_24_BLUEBG};
const constexpr gchar* TREE_TEXT_LIGHT_FG     {COLOR_24_LBLACK};
const constexpr gchar* TREE_TEXT_LIGHT_BG     {COLOR_24_GRAY};

const constexpr gchar* GTKSPELLCHECK_TAG_NAME {"gtkspellchecker-misspelled"};
const constexpr gchar* TAG_WEIGHT           {"weight"};
const constexpr gchar* TAG_FOREGROUND       {"foreground"};
const constexpr gchar* TAG_BACKGROUND       {"background"};
const constexpr gchar* TAG_STYLE            {"style"};
const constexpr gchar* TAG_UNDERLINE        {"underline"};
const constexpr gchar* TAG_STRIKETHROUGH    {"strikethrough"};
const constexpr gchar* TAG_SCALE            {"scale"};
const constexpr gchar* TAG_FAMILY           {"family"};
const constexpr gchar* TAG_JUSTIFICATION    {"justification"};
const constexpr gchar* TAG_LINK             {"link"};
const constexpr gchar* TAG_SEPARATOR        {"separator"};
const constexpr gchar* TAG_SEPARATOR_ANSI_REPR {"---------"};

const constexpr gchar* TAG_PROP_VAL_HEAVY        {"heavy"};
const constexpr gchar* TAG_PROP_VAL_ITALIC       {"italic"};
const constexpr gchar* TAG_PROP_VAL_MONOSPACE    {"monospace"};
const constexpr gchar* TAG_PROP_VAL_SINGLE       {"single"};
const constexpr gchar* TAG_PROP_VAL_SMALL        {"small"};
const constexpr gchar* TAG_PROP_VAL_TRUE         {"true"};
const constexpr gchar* TAG_PROP_VAL_H1           {"h1"};
const constexpr gchar* TAG_PROP_VAL_H2           {"h2"};
const constexpr gchar* TAG_PROP_VAL_H3           {"h3"};
const constexpr gchar* TAG_PROP_VAL_H4           {"h4"};
const constexpr gchar* TAG_PROP_VAL_H5           {"h5"};
const constexpr gchar* TAG_PROP_VAL_H6           {"h6"};
const constexpr gchar* TAG_PROP_VAL_SUP          {"sup"};
const constexpr gchar* TAG_PROP_VAL_SUB          {"sub"};
const constexpr gchar* TAG_PROP_VAL_LEFT         {"left"};
const constexpr gchar* TAG_PROP_VAL_CENTER       {"center"};
const constexpr gchar* TAG_PROP_VAL_RIGHT        {"right"};
const constexpr gchar* TAG_PROP_VAL_FILL         {"fill"};

const constexpr gchar* STR_KEY_UP                {"Up"};
const constexpr gchar* STR_KEY_DOWN              {"Down"};
const constexpr gchar* STR_KEY_LEFT              {"Left"};
const constexpr gchar* STR_KEY_RIGHT             {"Right"};
const constexpr gchar* STR_STOCK_CT_IMP          {"import_in_cherrytree"};

const constexpr int MAX_FILE_NAME_LEN              {142};
const constexpr int WHITE_SPACE_BETW_PIXB_AND_TEXT {3};
const constexpr int GRID_SLIP_OFFSET               {3};

const constexpr gchar* CHAR_SPACE             {" "};
const constexpr gchar* CHAR_NEWLINE           {"\n"};
const constexpr gchar* CHAR_NEWPAGE           {"\x0c"};
const constexpr gchar* CHAR_CR                {"\r"};
const constexpr gchar* CHAR_TAB               {"\t"};
const constexpr std::array<gunichar, 4> CHARS_LISTNUM {'.', ')', '-', '>'};
const constexpr gchar* CHAR_TILDE             {"~"};
const constexpr gchar* CHAR_MINUS             {"-"};
const constexpr gchar* CHAR_DQUOTE            {"\""};
const constexpr gchar* CHAR_SQUOTE            {"'"};
const constexpr gchar* CHAR_GRAVE             {"`"};
const constexpr gchar* CHAR_SLASH             {"/"};
const constexpr gchar* CHAR_BSLASH            {"\\"};
const constexpr gchar* CHAR_SQ_BR_OPEN        {"["};
const constexpr gchar* CHAR_SQ_BR_CLOSE       {"]"};
const constexpr gchar* CHAR_PARENTH_OPEN      {"("};
const constexpr gchar* CHAR_PARENTH_CLOSE     {")"};
const constexpr gchar* CHAR_LESSER            {"<"};
const constexpr gchar* CHAR_GREATER           {">"};
const constexpr gchar* CHAR_STAR              {"*"};
const constexpr gchar* CHAR_QUESTION          {"?"};
const constexpr gchar* CHAR_COMMA             {","};
const constexpr gchar* CHAR_COLON             {":"};
const constexpr gchar* CHAR_SEMICOLON         {";"};
const constexpr gchar* CHAR_USCORE            {"_"};
const constexpr gchar* CHAR_EQUAL             {"="};
const constexpr gchar* CHAR_BR_OPEN           {"{"};
const constexpr gchar* CHAR_BR_CLOSE          {"}"};
const constexpr gchar* CHAR_CARET             {"^"};
const constexpr gchar* CHAR_PIPE              {"|"};
const constexpr gchar* CHAR_AMPERSAND         {"&"};



const constexpr std::array<const gchar*, 4> WEB_LINK_STARTERS {
    "http://",
    "https://",
    "www.",
    "ftp://"
};

const constexpr std::array<const gchar*, 10> TAG_PROPERTIES {
    TAG_WEIGHT,
    TAG_FOREGROUND,
    TAG_BACKGROUND,
    TAG_STYLE,
    TAG_UNDERLINE,
    TAG_STRIKETHROUGH,
    TAG_SCALE,
    TAG_FAMILY,
    TAG_JUSTIFICATION,
    TAG_LINK
};

const constexpr gchar* TOOLBAR_VEC_DEFAULT {
    "tree_add_node,tree_add_subnode,separator,go_node_prev,go_node_next,"
    "separator,*,ct_save,export_pdf,separator,"
    "find_in_allnodes,separator,handle_bull_list,handle_num_list,handle_todo_list,"
    "separator,handle_image,handle_table,handle_codebox,handle_embfile,"
    "handle_link,handle_anchor,separator,fmt_rm,fmt_color_fg,"
    "fmt_color_bg,fmt_bold,fmt_italic,fmt_underline,fmt_strikethrough,"
    "fmt_h1,fmt_h2,fmt_h3,fmt_small,fmt_superscript,fmt_subscript,fmt_monospace"
};

const constexpr std::array<const gchar*, 17>  TOOLBAR_VEC_BLACKLIST {
    "anch_cut", "anch_copy", "anch_del", "anch_edit", "emb_file_cut",
    "emb_file_copy", "emb_file_del", "emb_file_save", "emb_file_open",
    "img_save", "img_edit", "img_cut", "img_copy", "img_del",
    "img_link_edit", "img_link_dismiss", "toggle_show_mainwin"
};

const constexpr std::array<const gchar*, 21> AVAILABLE_LANGS {
    "default", "cs", "de", "el", "en", "es", "fi", "fr", "hy", "it",
    "ja", "lt", "nl", "pl", "pt_BR", "ru", "sl", "sv", "tr", "uk", "zh_CN"
};

const constexpr int NODE_ICON_CODE_ID          {38};
const constexpr int NODE_ICON_BULLET_ID        {25};
const constexpr int NODE_ICON_NO_ICON_ID       {26};
const constexpr int NODE_ICON_SIZE             {16};
const constexpr int MAX_TOOLTIP_LINK_CHARS     {150};

// former NODES_STOCK
const constexpr std::array<const gchar*, 49> NODE_CUSTOM_ICONS {
    nullptr,           // NEVER USED
    "circle-green",    //  1
    "circle-yellow",   //  2
    "circle-red",      //  3
    "circle-grey",     //  4
    "add",             //  5
    "remove",          //  6
    "done",            //  7
    "cancel",          //  8
    "edit_delete",     //  9
    "warning",         // 10
    "star",            // 11
    "info",            // 12
    "help",            // 13
    "home",            // 14
    "index",           // 15
    "mail",            // 16
    "html",            // 17
    "notes",           // 18
    "timestamp",       // 19
    "calend",          // 20
    "terminal",        // 21
    "terminal-red",    // 22
    "python",          // 23
    "java",            // 24
    "node_bullet",     // 25
    "node_no_icon",    // 26
    CHERRY_BLACK,      // 27
    CHERRY_BLUE,       // 28
    CHERRY_CYAN,       // 29
    CHERRY_GREEN,      // 30
    CHERRY_GRAY,       // 31
    CHERRY_ORANGE,     // 32
    CHERRY_ORANGE_DARK,// 33
    CHERRY_PURPLE,     // 34
    CHERRY_RED,        // 35
    CHERRY_SHERBERT,   // 36
    CHERRY_YELLOW,     // 37
    "code",            // 38
    "find",            // 39
    "locked",          // 40
    "unlocked",        // 41
    "people",          // 42
    "urgent",          // 43
    "directory",       // 44
    "leaf",            // 45
    "xml",             // 46
    "c",               // 47
    "cpp",             // 48
};

// former NODES_ICONS
const constexpr std::array<const gchar*, 11> NODE_CHERRY_ICONS {
    CHERRY_RED,         //  0
    CHERRY_BLUE,        //  1
    CHERRY_ORANGE,      //  2
    CHERRY_CYAN,        //  3
    CHERRY_ORANGE_DARK, //  4
    CHERRY_SHERBERT,    //  5
    CHERRY_YELLOW,      //  6
    CHERRY_GREEN,       //  7
    CHERRY_PURPLE,      //  8
    CHERRY_BLACK,       //  9
    CHERRY_GRAY         // 10
};

const constexpr std::array<std::pair<const gchar*, const gchar*>, 11> CODE_ICONS {
    std::make_pair("python", "python"),
    std::make_pair("python3", "python"),
    std::make_pair("perl", "perl"),
    std::make_pair("sh", "terminal"),
    std::make_pair("dosbatch", "terminal-red"),
    std::make_pair("powershell", "terminal-red"),
    std::make_pair("java", "java"),
    std::make_pair("html", "html"),
    std::make_pair("xml", "xml"),
    std::make_pair("c", "c"),
    std::make_pair("cpp", "cpp")
};


const constexpr gchar* CODE_EXEC_TMP_SRC  {"<tmp_src_path>"};
const constexpr gchar* CODE_EXEC_TMP_BIN  {"<tmp_bin_path>"};
const constexpr gchar* CODE_EXEC_COMMAND  {"<command>"};

const std::array<std::pair<const std::string, const std::string>, 8> CODE_EXEC_TYPE_CMD_DEFAULT {
    std::make_pair("c",          std::string("gcc -o ") + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("cpp",        std::string("g++ -o ") + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("dosbatch",   std::string("call ") + CODE_EXEC_TMP_SRC),
    std::make_pair("perl",       std::string("perl ") + CODE_EXEC_TMP_SRC),
    std::make_pair("powershell", std::string("powershell -File ") + CODE_EXEC_TMP_SRC),
    std::make_pair("python",     std::string("python2 ") + CODE_EXEC_TMP_SRC),
    std::make_pair("python3",    std::string("python3 ") + CODE_EXEC_TMP_SRC),
    std::make_pair("sh",         std::string("sh ") + CODE_EXEC_TMP_SRC)
};

const constexpr std::array<std::pair<const gchar*, const gchar*>, 11> CODE_EXEC_TYPE_EXT_DEFAULT {
    std::make_pair("c",          "c"),
    std::make_pair("cpp",        "cpp"),
    std::make_pair("dosbatch",   "bat"),
    std::make_pair("perl",       "pl"),
    std::make_pair("powershell", "ps1"),
    std::make_pair("python",     "py"),
    std::make_pair("python3",    "py"),
    std::make_pair("sh",         "sh")
};

const std::unordered_map<std::string, std::string> CODE_EXEC_TERM_RUN_DEFAULT {
    {"linux", std::string("xterm -hold -geometry 180x45 -e \"") + CODE_EXEC_COMMAND + "\""},
    {"win",   std::string("start cmd /k \"") + CODE_EXEC_COMMAND + "\""}
};



}; // namespace CtConst
