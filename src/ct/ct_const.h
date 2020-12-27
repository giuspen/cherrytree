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

struct CtConst {

const inline static gchar* CT_VERSION  {PACKAGE_VERSION};
const inline static gchar* APP_NAME    {PACKAGE_NAME};

const inline static gchar* CTDOC_XML_NOENC          {".ctd"};
const inline static gchar* CTDOC_XML_ENC            {".ctz"};
const inline static gchar* CTDOC_SQLITE_NOENC       {".ctb"};
const inline static gchar* CTDOC_SQLITE_ENC         {".ctx"};
const inline static gchar* LINK_TYPE_WEBS           {"webs"};
const inline static gchar* LINK_TYPE_FILE           {"file"};
const inline static gchar* LINK_TYPE_FOLD           {"fold"};
const inline static gchar* LINK_TYPE_NODE           {"node"};
const inline static gchar* NODE_ICON_TYPE_CHERRY    {"c"};
const inline static gchar* NODE_ICON_TYPE_CUSTOM    {"b"};
const inline static gchar* NODE_ICON_TYPE_NONE      {"n"};
const inline static gchar* CHERRY_RED               {"cherry_red"};
const inline static gchar* CHERRY_BLUE              {"cherry_blue"};
const inline static gchar* CHERRY_ORANGE            {"cherry_orange"};
const inline static gchar* CHERRY_CYAN              {"cherry_cyan"};
const inline static gchar* CHERRY_ORANGE_DARK       {"cherry_orange_dark"};
const inline static gchar* CHERRY_SHERBERT          {"cherry_sherbert"};
const inline static gchar* CHERRY_YELLOW            {"cherry_yellow"};
const inline static gchar* CHERRY_GREEN             {"cherry_green"};
const inline static gchar* CHERRY_PURPLE            {"cherry_purple"};
const inline static gchar* CHERRY_BLACK             {"cherry_black"};
const inline static gchar* CHERRY_GRAY              {"cherry_grey"};
const inline static gchar* RICH_TEXT_ID             {"custom-colors"};
const inline static gchar* TABLE_CELL_TEXT_ID       {"table-cell-text"};
const inline static gchar* PLAIN_TEXT_ID            {"plain-text"};
const inline static gchar* STYLE_APPLIED_ID         {"<style-applied>"};
const inline static gchar* SYN_HIGHL_BASH           {"sh"};
const inline static gchar* STYLE_SCHEME_LIGHT       {"classic"};
const inline static gchar* STYLE_SCHEME_DARK        {"cobalt"};
const inline static gchar* TIMESTAMP_FORMAT_DEFAULT {"%Y/%m/%d - %H:%M"};

const inline static Glib::ustring SPECIAL_CHARS_DEFAULT {"“”„‘’•◇▪▸☐☑☒★…‰€©®™°↓↑→←↔↵⇓⇑⇒⇐⇔»«▼▲►◄≤≥≠≈±¹²³½¼⅛×÷∞ø∑σ√∫ΔδΠπΣΦΩωαβγεηλμ☺☻☼♥♣♦✔♀♂♪♫✝"};
const inline static Glib::ustring SPECIAL_CHAR_ARROW_RIGHT   {"→"};
const inline static Glib::ustring SPECIAL_CHAR_ARROW_RIGHT2  {"⇒"};
const inline static Glib::ustring SPECIAL_CHAR_ARROW_LEFT    {"←"};
const inline static Glib::ustring SPECIAL_CHAR_ARROW_LEFT2   {"⇐"};
const inline static Glib::ustring SPECIAL_CHAR_ARROW_DOUBLE  {"↔"};
const inline static Glib::ustring SPECIAL_CHAR_ARROW_DOUBLE2 {"⇔"};
const inline static Glib::ustring SPECIAL_CHAR_COPYRIGHT     {"©"};
const inline static Glib::ustring SPECIAL_CHAR_UNREGISTERED_TRADEMARK {"™"};
const inline static Glib::ustring SPECIAL_CHAR_REGISTERED_TRADEMARK   {"®"};
const inline static Glib::ustring SELWORD_CHARS_DEFAULT      {".-@"};
const inline static Glib::ustring CHARS_LISTBUL_DEFAULT      {"•◇▪-→⇒"};
const inline static Glib::ustring CHARS_TOC_DEFAULT          {"▸•◇▪"};
const inline static Glib::ustring CHARS_TODO_DEFAULT         {"☐☑☒"};
const inline static Glib::ustring CHARS_SMART_DQUOTE_DEFAULT {"“”"};
const inline static Glib::ustring CHARS_SMART_SQUOTE_DEFAULT {"‘’"};

const inline static gchar* COLOR_48_LINK_WEBS   {"#00008989ffff"};
const inline static gchar* COLOR_48_LINK_NODE   {"#071c838e071c"};
const inline static gchar* COLOR_48_LINK_FILE   {"#8b8b69691414"};
const inline static gchar* COLOR_48_LINK_FOLD   {"#7f7f7f7f7f7f"};
const inline static gchar* COLOR_48_YELLOW      {"#bbbbbbbb0000"};
const inline static gchar* COLOR_48_WHITE       {"#ffffffffffff"};
const inline static gchar* COLOR_48_BLACK       {"#000000000000"};
const inline static gchar* COLOR_24_BLACK       {"#000000"};
const inline static gchar* COLOR_24_WHITE       {"#ffffff"};
const inline static gchar* COLOR_24_BLUEBG      {"#001b33"};
const inline static gchar* COLOR_24_LBLACK      {"#0b0c0c"};
const inline static gchar* COLOR_24_GRAY        {"#e0e0e0"};
const inline static gchar* DEFAULT_MONOSPACE_BG {"#7f7f7f"};
const inline static gchar* TREE_TEXT_DARK_FG      {COLOR_24_WHITE};
const inline static gchar* TREE_TEXT_DARK_BG      {COLOR_24_BLUEBG};
const inline static gchar* TREE_TEXT_LIGHT_FG     {COLOR_24_LBLACK};
const inline static gchar* TREE_TEXT_LIGHT_BG     {COLOR_24_GRAY};

const inline static gchar* GTKSPELLCHECK_TAG_NAME  {"gtkspellchecker-misspelled"};
const inline static gchar* TAG_WEIGHT              {"weight"};
const inline static gchar* TAG_FOREGROUND          {"foreground"};
const inline static gchar* TAG_BACKGROUND          {"background"};
const inline static gchar* TAG_STYLE               {"style"};
const inline static gchar* TAG_UNDERLINE           {"underline"};
const inline static gchar* TAG_STRIKETHROUGH       {"strikethrough"};
const inline static gchar* TAG_INDENT              {"indent"};
const inline static gchar* TAG_SCALE               {"scale"};
const inline static gchar* TAG_FAMILY              {"family"};
const inline static gchar* TAG_JUSTIFICATION       {"justification"};
const inline static gchar* TAG_LINK                {"link"};
const inline static gchar* TAG_SEPARATOR           {"separator"};
const inline static gchar* TAG_SEPARATOR_ANSI_REPR {"---------"};

const inline static gchar* TAG_PROP_VAL_HEAVY        {"heavy"};
const inline static gchar* TAG_PROP_VAL_ITALIC       {"italic"};
const inline static gchar* TAG_PROP_VAL_MONOSPACE    {"monospace"};
const inline static gchar* TAG_PROP_VAL_SINGLE       {"single"};
const inline static gchar* TAG_PROP_VAL_SMALL        {"small"};
const inline static gchar* TAG_PROP_VAL_TRUE         {"true"};
const inline static gchar* TAG_PROP_VAL_H1           {"h1"};
const inline static gchar* TAG_PROP_VAL_H2           {"h2"};
const inline static gchar* TAG_PROP_VAL_H3           {"h3"};
const inline static gchar* TAG_PROP_VAL_H4           {"h4"};
const inline static gchar* TAG_PROP_VAL_H5           {"h5"};
const inline static gchar* TAG_PROP_VAL_H6           {"h6"};
const inline static gchar* TAG_PROP_VAL_SUP          {"sup"};
const inline static gchar* TAG_PROP_VAL_SUB          {"sub"};
const inline static gchar* TAG_PROP_VAL_LEFT         {"left"};
const inline static gchar* TAG_PROP_VAL_CENTER       {"center"};
const inline static gchar* TAG_PROP_VAL_RIGHT        {"right"};
const inline static gchar* TAG_PROP_VAL_FILL         {"fill"};
const inline static Glib::ustring TAG_ID_MONOSPACE {TAG_FAMILY + Glib::ustring{"_"} + TAG_PROP_VAL_MONOSPACE};

const inline static gchar* STR_KEY_UP                {"Up"};
const inline static gchar* STR_KEY_DOWN              {"Down"};
const inline static gchar* STR_KEY_LEFT              {"Left"};
const inline static gchar* STR_KEY_RIGHT             {"Right"};
const inline static gchar* STR_STOCK_CT_IMP          {"ct_import_in_cherrytree"};

const inline static int MAX_FILE_NAME_LEN              {142};
const inline static int WHITE_SPACE_BETW_PIXB_AND_TEXT {3};
const inline static int GRID_SLIP_OFFSET               {3};
const inline static int INDENT_MARGIN                  {50};
const inline static int TREE_DRAG_EDGE_PROX            {10};
const inline static int TREE_DRAG_EDGE_SCROLL          {15};

const inline static gchar* CHAR_SPACE             {" "};
const inline static gchar* CHAR_NEWLINE           {"\n"};
const inline static gchar* CHAR_NEWPAGE           {"\x0c"};
const inline static gchar* CHAR_CR                {"\r"};
const inline static gchar* CHAR_TAB               {"\t"};
const inline static std::array<gunichar, 4> CHARS_LISTNUM {'.', ')', '-', '>'};
const inline static gchar* CHAR_TILDE             {"~"};
const inline static gchar* CHAR_MINUS             {"-"};
const inline static gchar* CHAR_DQUOTE            {"\""};
const inline static gchar* CHAR_SQUOTE            {"'"};
const inline static gchar* CHAR_GRAVE             {"`"};
const inline static gchar* CHAR_SLASH             {"/"};
const inline static gchar* CHAR_BSLASH            {"\\"};
const inline static gchar* CHAR_SQ_BR_OPEN        {"["};
const inline static gchar* CHAR_SQ_BR_CLOSE       {"]"};
const inline static gchar* CHAR_PARENTH_OPEN      {"("};
const inline static gchar* CHAR_PARENTH_CLOSE     {")"};
const inline static gchar* CHAR_LESSER            {"<"};
const inline static gchar* CHAR_GREATER           {">"};
const inline static gchar* CHAR_STAR              {"*"};
const inline static gchar* CHAR_QUESTION          {"?"};
const inline static gchar* CHAR_COMMA             {","};
const inline static gchar* CHAR_COLON             {":"};
const inline static gchar* CHAR_SEMICOLON         {";"};
const inline static gchar* CHAR_USCORE            {"_"};
const inline static gchar* CHAR_EQUAL             {"="};
const inline static gchar* CHAR_BR_OPEN           {"{"};
const inline static gchar* CHAR_BR_CLOSE          {"}"};
const inline static gchar* CHAR_CARET             {"^"};
const inline static gchar* CHAR_PIPE              {"|"};
const inline static gchar* CHAR_AMPERSAND         {"&"};

const inline static std::array<std::string_view, 4> WEB_LINK_STARTERS {
    "http://",
    "https://",
    "www.",
    "ftp://"
};

const inline static std::array<std::string_view, 11> TAG_PROPERTIES {
    TAG_WEIGHT,
    TAG_FOREGROUND,
    TAG_BACKGROUND,
    TAG_STYLE,
    TAG_UNDERLINE,
    TAG_STRIKETHROUGH,
    TAG_SCALE,
    TAG_FAMILY,
    TAG_JUSTIFICATION,
    TAG_LINK,
    TAG_INDENT
};

const inline static std::array<const gchar*, 4> TAG_ALIGNMENTS {
    TAG_PROP_VAL_LEFT,
    TAG_PROP_VAL_RIGHT,
    TAG_PROP_VAL_CENTER,
    TAG_PROP_VAL_FILL
};

const inline static gchar* TOOLBAR_SPLIT {"toolbar_split"};
const inline static gchar* TOOLBAR_VEC_DEFAULT {
    "tree_add_node,tree_add_subnode,separator,go_node_prev,go_node_next,"
    "separator,*,ct_save,export_pdf,separator,"
    "find_in_allnodes,separator,handle_bull_list,handle_num_list,handle_todo_list,fmt_indent,fmt_unindent,"
    "separator,handle_image,handle_table,handle_codebox,handle_embfile,"
    "handle_link,handle_anchor,separator,fmt_rm,fmt_color_fg,"
    "fmt_color_bg,fmt_bold,fmt_italic,fmt_underline,fmt_strikethrough,"
    "fmt_h1,fmt_h2,fmt_h3,fmt_small,fmt_superscript,fmt_subscript,fmt_monospace"
};

const inline static std::array<const gchar*, 18>  TOOLBAR_VEC_BLACKLIST {
    "anch_cut", "anch_copy", "anch_del", "anch_edit", "emb_file_cut",
    "emb_file_copy", "emb_file_del", "emb_file_save", "emb_file_open", "emb_file_rename",
    "img_save", "img_edit", "img_cut", "img_copy", "img_del",
    "img_link_edit", "img_link_dismiss", "toggle_show_mainwin"
};

const inline static gchar* LANG_DEFAULT{"default"};

const inline static std::array<const gchar*, 22> AVAILABLE_LANGS {
    LANG_DEFAULT, "bg", "cs", "de", "el", "en", "es", "fi", "fr", "hy", "it",
    "ja", "lt", "nl", "pl", "pt_BR", "ru", "sl", "sv", "tr", "uk", "zh_CN"
};

const inline static int NODE_ICON_CODE_ID          {38};
const inline static int NODE_ICON_BULLET_ID        {25};
const inline static int NODE_ICON_NO_ICON_ID       {26};
const inline static int NODE_ICON_SIZE             {16};
const inline static int MAX_TOOLTIP_LINK_CHARS     {150};

// former NODES_STOCK
const inline static std::array<const gchar*, 49> NODE_CUSTOM_ICONS {
    nullptr,            // NEVER USED
    "ct_circle-green",  //  1
    "ct_circle-yellow", //  2
    "ct_circle-red",    //  3
    "ct_circle-grey",   //  4
    "ct_add",           //  5
    "ct_remove",        //  6
    "ct_done",          //  7
    "ct_cancel",        //  8
    "ct_edit_delete",   //  9
    "ct_warning",       // 10
    "ct_star",          // 11
    "ct_info",          // 12
    "ct_help",          // 13
    "ct_home",          // 14
    "ct_index",         // 15
    "ct_mail",          // 16
    "ct_html",          // 17
    "ct_notes",         // 18
    "ct_timestamp",     // 19
    "ct_calendar",      // 20
    "ct_term",          // 21
    "ct_term-red",      // 22
    "ct_python",        // 23
    "ct_java",          // 24
    "ct_node_bullet",   // 25
    "ct_node_no_icon",  // 26
    CHERRY_BLACK,       // 27
    CHERRY_BLUE,        // 28
    CHERRY_CYAN,        // 29
    CHERRY_GREEN,       // 30
    CHERRY_GRAY,        // 31
    CHERRY_ORANGE,      // 32
    CHERRY_ORANGE_DARK, // 33
    CHERRY_PURPLE,      // 34
    CHERRY_RED,         // 35
    CHERRY_SHERBERT,    // 36
    CHERRY_YELLOW,      // 37
    "ct_code",          // 38
    "ct_find",          // 39
    "ct_locked",        // 40
    "ct_unlocked",      // 41
    "ct_people",        // 42
    "ct_urgent",        // 43
    "ct_directory",     // 44
    "ct_leaf",          // 45
    "ct_xml",           // 46
    "ct_c",             // 47
    "ct_cpp",           // 48
};

// former NODES_ICONS
const inline static std::array<const gchar*, 11> NODE_CHERRY_ICONS {
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

const inline static std::array<std::pair<const gchar*, const gchar*>, 11> NODE_CODE_ICONS {
    std::make_pair("python", "ct_python"),
    std::make_pair("python3", "ct_python"),
    std::make_pair("perl", "ct_perl"),
    std::make_pair("sh", "ct_term"),
    std::make_pair("dosbatch", "ct_term-red"),
    std::make_pair("powershell", "ct_term-red"),
    std::make_pair("java", "ct_java"),
    std::make_pair("html", "ct_html"),
    std::make_pair("xml", "ct_xml"),
    std::make_pair("c", "ct_c"),
    std::make_pair("cpp", "ct_cpp")
};

const inline static gchar* CODE_EXEC_TMP_SRC  {"<tmp_src_path>"};
const inline static gchar* CODE_EXEC_TMP_BIN  {"<tmp_bin_path>"};
const inline static gchar* CODE_EXEC_COMMAND  {"<command>"};

const inline static std::array<std::pair<const std::string, const std::string>, 8> CODE_EXEC_TYPE_CMD_DEFAULT {
    std::make_pair("c",          std::string("gcc -o ") + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("cpp",        std::string("g++ -o ") + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("dosbatch",   std::string("call ") + CODE_EXEC_TMP_SRC),
    std::make_pair("perl",       std::string("perl ") + CODE_EXEC_TMP_SRC),
    std::make_pair("powershell", std::string("powershell -File ") + CODE_EXEC_TMP_SRC),
    std::make_pair("python",     std::string("python2 ") + CODE_EXEC_TMP_SRC),
    std::make_pair("python3",    std::string("python3 ") + CODE_EXEC_TMP_SRC),
    std::make_pair("sh",         std::string("sh ") + CODE_EXEC_TMP_SRC)
};

const inline static std::array<std::pair<const std::string, const std::string>, 8> CODE_EXEC_TYPE_EXT_DEFAULT {
    std::make_pair("c",          "c"),
    std::make_pair("cpp",        "cpp"),
    std::make_pair("dosbatch",   "bat"),
    std::make_pair("perl",       "pl"),
    std::make_pair("powershell", "ps1"),
    std::make_pair("python",     "py"),
    std::make_pair("python3",    "py"),
    std::make_pair("sh",         "sh")
};

const inline static std::unordered_map<std::string, std::string> CODE_EXEC_TERM_RUN_DEFAULT {
    {"linux", std::string("xterm -hold -geometry 180x45 -e \"") + CODE_EXEC_COMMAND + "\" &"},
    {"win",   std::string("start cmd /k \"") + CODE_EXEC_COMMAND + "\""}
};

const inline static std::array<std::string_view, 4>  INVALID_HTML_TAGS = {
    "script", "title", "head", "html"
};

// List of extensions for cherrytree save files, for use with gtk FileFilter
const inline static std::vector<std::string> CT_FILE_EXTENSIONS_FILTER = {
    "*.ctb", "*.ctx", "*.ctd", "*.ctz"
};

}; // struct CtConst
