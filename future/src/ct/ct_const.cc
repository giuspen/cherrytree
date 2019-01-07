/*
 * ct_const.cc
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

#include "ct_const.h"

const gchar    CtConst::CT_VERSION[]           {"0.0.1"};
const gchar    CtConst::APP_NAME[]             {"cherrytree"};
const int      CtConst::MAX_RECENT_DOCS             {10};
const int      CtConst::MAX_RECENT_DOCS_RESTORE      {3};
const int      CtConst::NODE_ICON_CODE_ID           {38};
const int      CtConst::NODE_ICON_BULLET_ID         {25};
const int      CtConst::NODE_ICON_NO_ICON_ID        {26};
const int      CtConst::NODE_ICON_SIZE              {16};
const gchar    CtConst::LINK_TYPE_WEBS[]       {"webs"};
const gchar    CtConst::LINK_TYPE_FILE[]       {"file"};
const gchar    CtConst::LINK_TYPE_FOLD[]       {"fold"};
const gchar    CtConst::LINK_TYPE_NODE[]       {"node"};
const gchar    CtConst::NODE_ICON_TYPE_CHERRY[]    {"c"};
const gchar    CtConst::NODE_ICON_TYPE_CUSTOM[]    {"b"};
const gchar    CtConst::NODE_ICON_TYPE_NONE[]      {"n"};
const gchar    CtConst::CHERRY_RED[]           {"cherry_red"};
const gchar    CtConst::CHERRY_BLUE[]          {"cherry_blue"};
const gchar    CtConst::CHERRY_ORANGE[]        {"cherry_orange"};
const gchar    CtConst::CHERRY_CYAN[]          {"cherry_cyan"};
const gchar    CtConst::CHERRY_ORANGE_DARK[]   {"cherry_orange_dark"};
const gchar    CtConst::CHERRY_SHERBERT[]      {"cherry_sherbert"};
const gchar    CtConst::CHERRY_YELLOW[]        {"cherry_yellow"};
const gchar    CtConst::CHERRY_GREEN[]         {"cherry_green"};
const gchar    CtConst::CHERRY_PURPLE[]        {"cherry_purple"};
const gchar    CtConst::CHERRY_BLACK[]         {"cherry_black"};
const gchar    CtConst::CHERRY_GRAY[]          {"cherry_gray"};
const gchar    CtConst::RICH_TEXT_ID[]         {"custom-colors"};
const gchar    CtConst::PLAIN_TEXT_ID[]        {"plain-text"};
const gchar    CtConst::SYN_HIGHL_BASH[]       {"sh"};
const gchar    CtConst::STYLE_SCHEME_LIGHT[]   {"classic"};
const gchar    CtConst::STYLE_SCHEME_DARK[]    {"cobalt"};
const gchar    CtConst::STYLE_SCHEME_GRAY[]    {"oblivion"};
const gchar    CtConst::TIMESTAMP_FORMAT_DEFAULT[] {"%Y/%m/%d - %H:%M"};
const gchar    CtConst::SPECIAL_CHARS_DEFAULT[]    {"“”„‘’•◇▪▸☐☑☒★…‰€©®™°↓↑→←↔↵⇓⇑⇒⇐⇔»«▼▲►◄≤≥≠≈±¹²³½¼⅛×÷∞ø∑√∫ΔδΠπΣΦΩωαβγεηλμ☺☻☼♥♣♦✔♀♂♪♫✝"};
const gchar    CtConst::SELWORD_CHARS_DEFAULT[]    {".-@"};
const gchar    CtConst::CHARS_LISTBUL_DEFAULT[]    {"•◇▪-→⇒"};
const gchar    CtConst::CHARS_TOC_DEFAULT[]    {"▸•◇▪"};
const gchar    CtConst::CHARS_TODO_DEFAULT[]   {"☐☑☒"};
const gchar    CtConst::COLOR_48_LINK_WEBS[]   {"#00008989ffff"};
const gchar    CtConst::COLOR_48_LINK_NODE[]   {"#071c838e071c"};
const gchar    CtConst::COLOR_48_LINK_FILE[]   {"#8b8b69691414"};
const gchar    CtConst::COLOR_48_LINK_FOLD[]   {"#7f7f7f7f7f7f"};
const gchar    CtConst::COLOR_48_YELLOW[]      {"#bbbbbbbb0000"};
const gchar    CtConst::COLOR_48_WHITE[]       {"#ffffffffffff"};
const gchar    CtConst::COLOR_48_BLACK[]       {"#000000000000"};
const gchar    CtConst::COLOR_24_BLACK[]       {"#000000"};
const gchar    CtConst::COLOR_24_WHITE[]       {"#ffffff"};
const gchar    CtConst::COLOR_24_BLUEBG[]      {"#001b33"};
const gchar    CtConst::COLOR_24_LBLACK[]      {"#0b0c0c"};
const gchar    CtConst::COLOR_24_GRAY[]        {"#e0e0e0"};
const gchar    CtConst::DEFAULT_MONOSPACE_BG[] {"#7f7f7f"};
const gchar*   CtConst::RICH_TEXT_DARK_FG      {COLOR_24_WHITE};
const gchar*   CtConst::RICH_TEXT_DARK_BG      {COLOR_24_BLUEBG};
const gchar*   CtConst::RICH_TEXT_LIGHT_FG     {COLOR_24_BLACK};
const gchar*   CtConst::RICH_TEXT_LIGHT_BG     {COLOR_24_WHITE};
const gchar*   CtConst::TREE_TEXT_DARK_FG      {COLOR_24_WHITE};
const gchar*   CtConst::TREE_TEXT_DARK_BG      {COLOR_24_BLUEBG};
const gchar*   CtConst::TREE_TEXT_LIGHT_FG     {COLOR_24_LBLACK};
const gchar*   CtConst::TREE_TEXT_LIGHT_BG     {COLOR_24_GRAY};

const gchar    CtConst::TAG_WEIGHT[]           {"weight"};
const gchar    CtConst::TAG_FOREGROUND[]       {"foreground"};
const gchar    CtConst::TAG_BACKGROUND[]       {"background"};
const gchar    CtConst::TAG_STYLE[]            {"style"};
const gchar    CtConst::TAG_UNDERLINE[]        {"underline"};
const gchar    CtConst::TAG_STRIKETHROUGH[]    {"strikethrough"};
const gchar    CtConst::TAG_SCALE[]            {"scale"};
const gchar    CtConst::TAG_FAMILY[]           {"family"};
const gchar    CtConst::TAG_JUSTIFICATION[]    {"justification"};
const gchar    CtConst::TAG_LINK[]             {"link"};

const gchar    CtConst::TAG_PROP_VAL_HEAVY[]        {"heavy"};
const gchar    CtConst::TAG_PROP_VAL_ITALIC[]       {"italic"};
const gchar    CtConst::TAG_PROP_VAL_MONOSPACE[]    {"monospace"};
const gchar    CtConst::TAG_PROP_VAL_SINGLE[]       {"single"};
const gchar    CtConst::TAG_PROP_VAL_SMALL[]        {"small"};
const gchar    CtConst::TAG_PROP_VAL_TRUE[]         {"true"};
const gchar    CtConst::TAG_PROP_VAL_H1[]           {"h1"};
const gchar    CtConst::TAG_PROP_VAL_H2[]           {"h2"};
const gchar    CtConst::TAG_PROP_VAL_H3[]           {"h3"};
const gchar    CtConst::TAG_PROP_VAL_H4[]           {"h4"};
const gchar    CtConst::TAG_PROP_VAL_H5[]           {"h5"};
const gchar    CtConst::TAG_PROP_VAL_H6[]           {"h6"};
const gchar    CtConst::TAG_PROP_VAL_SUP[]          {"sup"};
const gchar    CtConst::TAG_PROP_VAL_SUB[]          {"sub"};
const gchar    CtConst::TAG_PROP_VAL_LEFT[]         {"left"};
const gchar    CtConst::TAG_PROP_VAL_CENTER[]       {"center"};
const gchar    CtConst::TAG_PROP_VAL_RIGHT[]        {"right"};
const gchar    CtConst::TAG_PROP_VAL_FILL[]         {"fill"};

const gchar    CtConst::STR_KEY_UP[]                {"Up"};
const gchar    CtConst::STR_KEY_DOWN[]              {"Down"};
const gchar    CtConst::STR_KEY_LEFT[]              {"Left"};
const gchar    CtConst::STR_KEY_RIGHT[]             {"Right"};
const gchar    CtConst::STR_STOCK_CT_IMP[]          {"import_in_cherrytree"};
const std::set<const gchar*> CtConst::TEXT_SYNTAXES {
    RICH_TEXT_ID,
    PLAIN_TEXT_ID};

const std::set<const gchar*> CtConst::TAG_PROPERTIES {
    TAG_WEIGHT,
    TAG_FOREGROUND,
    TAG_BACKGROUND,
    TAG_STYLE,
    TAG_UNDERLINE,
    TAG_STRIKETHROUGH,
    TAG_SCALE,
    TAG_FAMILY,
    TAG_JUSTIFICATION,
    TAG_LINK};

const gchar CtConst::TOOLBAR_VEC_DEFAULT[] { 
    "tree_add_node,tree_add_subnode,sep,go_node_prev,go_node_next,"
    "sep,*,ct_save,export_pdf,sep,"
    "find_in_allnodes,sep,handle_bull_list,handle_num_list,handle_todo_list,"
    "sep,handle_image,handle_table,handle_codebox,handle_embfile,"
    "handle_link,handle_anchor,sep,fmt_rm,fmt_color_fg,"
    "fmt_color_bg,fmt_bold,fmt_italic,fmt_underline,fmt_strikethrough,"
    "fmt_h1,fmt_h2,fmt_h3,fmt_small,fmt_superscript,fmt_subscript,fmt_monospace"};

const std::unordered_map<int, Glib::ustring> CtConst::NODES_STOCKS {
    { 1, "circle-green"},
    { 2, "circle-yellow"},
    { 3, "circle-red"},
    { 4, "circle-grey"},
    { 5, "add"},
    { 6, "remove"},
    { 7, "done"},
    { 8, "cancel"},
    { 9, "edit-delete"},
    {10, "warning"},
    {11, "star"},
    {12, "information"},
    {13, "help-contents"},
    {14, "home"},
    {15, "index"},
    {16, "mail"},
    {17, "html"},
    {18, "notes"},
    {19, "timestamp"},
    {20, "calendar"},
    {21, "terminal"},
    {22, "terminal-red"},
    {23, "python"},
    {24, "java"},
    {25, "node_bullet"},
    {26, "node_no_icon"},
    {27, CHERRY_BLACK},
    {28, CHERRY_BLUE},
    {29, CHERRY_CYAN},
    {30, CHERRY_GREEN},
    {31, CHERRY_GRAY},
    {32, CHERRY_ORANGE},
    {33, CHERRY_ORANGE_DARK},
    {34, CHERRY_PURPLE},
    {35, CHERRY_RED},
    {36, CHERRY_SHERBERT},
    {37, CHERRY_YELLOW},
    {38, "code"},
    {39, "find"},
    {40, "locked"},
    {41, "unlocked"},
    {42, "people"},
    {43, "urgent"},
    {44, "folder"},
    {45, "leaf"},
    {46, "xml"},
    {47, "c"},
    {48, "cpp"},
};

const std::unordered_map<int, Glib::ustring> CtConst::NODES_ICONS {
    { 0, CHERRY_RED},
    { 1, CHERRY_BLUE},
    { 2, CHERRY_ORANGE},
    { 3, CHERRY_CYAN},
    { 4, CHERRY_ORANGE_DARK},
    { 5, CHERRY_SHERBERT},
    { 6, CHERRY_YELLOW},
    { 7, CHERRY_GREEN},
    { 8, CHERRY_PURPLE},
    { 9, CHERRY_BLACK},
    {10, CHERRY_GRAY},
    {-1, CHERRY_GRAY},
};

const std::map<Glib::ustring, Glib::ustring> CtConst::CODE_ICONS {
    {"python", "python"},
    {"python3", "python"},
    {"perl", "perl"},
    {"sh", "terminal"},
    {"dosbatch", "terminal-red"},
    {"powershell", "terminal-red"},
    {"java", "java"},
    {"html", "html"},
    {"xml", "xml"},
    {"c", "c"},
    {"cpp", "cpp"},
};

Glib::ustring CtConst::getStockIdForCodeType(Glib::ustring code_type)
{
    return (1 == CODE_ICONS.count(code_type) ? CODE_ICONS.at(code_type) : NODES_STOCKS.at(NODE_ICON_CODE_ID));
}
