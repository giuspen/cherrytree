/*
 * ct_const.h
 *
 * Copyright 2009-2024
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

const inline static std::string CTDOC_XML_NOENC     {".ctd"};
const inline static std::string CTDOC_XML_ENC       {".ctz"};
const inline static std::string CTDOC_SQLITE_NOENC  {".ctb"};
const inline static std::string CTDOC_SQLITE_ENC    {".ctx"};

const inline static std::string LINK_TYPE_WEBS      {"webs"};
const inline static std::string LINK_TYPE_FILE      {"file"};
const inline static std::string LINK_TYPE_FOLD      {"fold"};
const inline static std::string LINK_TYPE_NODE      {"node"};
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
const inline static gchar* SYN_HIGHL_SHELL          {"sh"};
#if defined(__APPLE__)
const inline static gchar* VTE_SHELL_DEFAULT        {"/bin/zsh"};
#else
const inline static gchar* VTE_SHELL_DEFAULT        {"/bin/bash"};
#endif
const inline static gchar* STYLE_SCHEME_LIGHT       {"classic"};
const inline static gchar* STYLE_SCHEME_DARK        {"cobalt"};
const inline static gchar* TIMESTAMP_FORMAT_DEFAULT {"%Y/%m/%d - %H:%M"};
const inline static gchar* FONT_RT_DEFAULT          {"Sans 11"};
const inline static gchar* FONT_PT_DEFAULT          {"Sans 11"};
const inline static gchar* FONT_TREE_DEFAULT        {"Sans 10"};
const inline static gchar* FONT_CODE_DEFAULT        {"Monospace 11"};
const inline static gchar* FONT_VTE_DEFAULT         {"Monospace 10"};
const inline static gchar* FONT_MS_DEFAULT          {"Monospace 11"};

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

const inline static gchar* COLOR_48_LINK_WEBS     {"#00008989ffff"};
const inline static gchar* COLOR_48_LINK_NODE     {"#071c838e071c"};
const inline static gchar* COLOR_48_LINK_FILE     {"#8b8b69691414"};
const inline static gchar* COLOR_48_LINK_FOLD     {"#7f7f7f7f7f7f"};
const inline static gchar* COLOR_48_YELLOW        {"#bbbbbbbb0000"};
const inline static gchar* COLOR_48_WHITE         {"#ffffffffffff"};
const inline static gchar* COLOR_48_BLACK         {"#000000000000"};
const inline static gchar* COLOR_24_BLACK         {"#000000"};
const inline static gchar* COLOR_24_WHITE         {"#ffffff"};
const inline static gchar* COLOR_24_BLUEBG        {"#001b33"};
const inline static gchar* COLOR_24_LBLACK        {"#0b0c0c"};
const inline static gchar* COLOR_24_LGRAY         {"#e0e0e0"};
const inline static gchar* COLOR_24_DGRAY         {"#7f7f7f"};
const inline static gchar* SCALABLE_H1_DEFAULT    {"1728;;;0;0;0"};
const inline static gchar* SCALABLE_H2_DEFAULT    {"1440;;;0;0;0"};
const inline static gchar* SCALABLE_H3_DEFAULT    {"1200;;;0;0;0"};
const inline static gchar* SCALABLE_H4_DEFAULT    {"1150;;;0;0;0"};
const inline static gchar* SCALABLE_H5_DEFAULT    {"1100;;;0;0;0"};
const inline static gchar* SCALABLE_H6_DEFAULT    {"1050;;;0;0;0"};
const inline static gchar* SCALABLE_SMALL_DEFAULT  {"833;;;0;0;0"};
const inline static gchar* TREE_TEXT_DARK_FG      {COLOR_24_WHITE};
const inline static gchar* TREE_TEXT_DARK_BG      {COLOR_24_BLUEBG};
const inline static gchar* TREE_TEXT_LIGHT_FG     {COLOR_24_LBLACK};
const inline static gchar* TREE_TEXT_LIGHT_BG     {COLOR_24_LGRAY};
const inline static gchar* TREE_TEXT_SEL_BG       {"#5294e2"};
const inline static gchar* DEFAULT_MONOSPACE_FG   {COLOR_24_LGRAY};
const inline static gchar* DEFAULT_MONOSPACE_BG   {COLOR_24_DGRAY};
const inline static unsigned NUM_USER_STYLES{2};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_TEXT_FG{COLOR_24_WHITE, COLOR_24_BLACK};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_TEXT_BG{COLOR_24_BLACK, COLOR_24_WHITE};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_SELECTION_FG{COLOR_24_WHITE, COLOR_24_WHITE};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_SELECTION_BG{"#0088ff", "#43ace8"};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_CURSOR{COLOR_24_WHITE, COLOR_24_BLACK};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_CURRENT_LINE_BG{"#003b70", "#eef6ff"};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_LINE_NUMBERS_FG{"#777777", COLOR_24_BLACK};
const inline static std::array<std::string,NUM_USER_STYLES> USER_STYLE_LINE_NUMBERS_BG{"#000d1a", "#d6d2d0"};

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

const inline static std::string CHAR_SPACE          {" "};
const inline static std::string CHAR_NEWLINE        {"\n"};
const inline static std::string CHAR_NEWPAGE        {"\x0c"};
const inline static std::string CHAR_CR             {"\r"};
const inline static std::string CHAR_TAB            {"\t"};
const inline static std::array<gunichar, 4> CHARS_LISTNUM {'.', ')', '-', '>'};
const inline static std::string CHAR_DOT            {"."};
const inline static std::string CHAR_TILDE          {"~"};
const inline static std::string CHAR_MINUS          {"-"};
const inline static std::string CHAR_DQUOTE         {"\""};
const inline static std::string CHAR_SQUOTE         {"'"};
const inline static std::string CHAR_GRAVE          {"`"};
const inline static std::string CHAR_SLASH          {"/"};
const inline static std::string CHAR_BSLASH         {"\\"};
const inline static std::string CHAR_SQ_BR_OPEN     {"["};
const inline static std::string CHAR_SQ_BR_CLOSE    {"]"};
const inline static std::string CHAR_PARENTH_OPEN   {"("};
const inline static std::string CHAR_PARENTH_CLOSE  {")"};
const inline static std::string CHAR_LESSER         {"<"};
const inline static std::string CHAR_GREATER        {">"};
const inline static std::string CHAR_STAR           {"*"};
const inline static std::string CHAR_QUESTION       {"?"};
const inline static std::string CHAR_COMMA          {","};
const inline static std::string CHAR_COLON          {":"};
const inline static std::string CHAR_SEMICOLON      {";"};
const inline static std::string CHAR_USCORE         {"_"};
const inline static std::string CHAR_EQUAL          {"="};
const inline static std::string CHAR_BR_OPEN        {"{"};
const inline static std::string CHAR_BR_CLOSE       {"}"};
const inline static std::string CHAR_CARET          {"^"};
const inline static std::string CHAR_PIPE           {"|"};
const inline static std::string CHAR_AMPERSAND      {"&"};

const inline static std::array<std::string_view, 4> WEB_LINK_STARTERS {
    "http://",
    "https://",
    "www.",
    "ftp://"
};
// https://stackoverflow.com/questions/1547899/which-characters-make-a-url-invalid
const inline static char URL_INVALID_CHARS[]{" \n\r\t\"<>\\^`{}"};

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
    "separator,ct_open_folder,*,ct_save,export_pdf,separator,"
    "find_in_allnodes,separator,handle_bull_list,handle_num_list,handle_todo_list,fmt_indent,fmt_unindent,"
    "separator,handle_image,handle_table,handle_codebox,handle_latex,handle_embfile,"
    "handle_link,handle_anchor,separator,fmt_clone,fmt_latest,fmt_rm,fmt_color_fg,"
    "fmt_color_bg,fmt_bold,fmt_italic,fmt_underline,fmt_strikethrough,"
    "fmt_h1,fmt_h2,fmt_h3,fmt_small,fmt_superscript,fmt_subscript,fmt_monospace"
};

const inline static gchar* LANG_DEFAULT{"default"};

const inline static std::vector<const gchar*> AVAILABLE_LANGS {
    LANG_DEFAULT, "ar", "bg", "cs", "de", "el", "en", "es", "fa", "fi", "fr", "hi_IN", "hr", "hu", "hy", "it",
    "ja", "kk_KZ", "kk_LA", "ko", "lt", "nl", "pl", "pt", "pt_BR", "ro", "ru", "sl", "sv", "tr", "uk", "zh_CN", "zh_TW"
};

const inline static int NODE_ICON_CODE_ID          {38};
const inline static int NODE_ICON_BULLET_ID        {25};
const inline static int NODE_ICON_NO_ICON_ID       {26};
const inline static int NODE_ICON_SEL_DEFAULT     {132};
const inline static int NODE_ICON_SIZE             {16};
const inline static int MAX_TOOLTIP_LINK_CHARS     {150};
const inline static int ADVISED_TABLE_LIGHT_HEAVY  {25};

// do not use _NODE_CUSTOM_ICONS directly, use wrapper CtStockIcon instead
const inline static std::vector<const gchar*> _NODE_CUSTOM_ICONS {
    nullptr,                // NEVER USED
    "ct_circle-green",      //  1
    "ct_circle-yellow",     //  2
    "ct_circle-red",        //  3
    "ct_circle-grey",       //  4
    "ct_add",               //  5
    "ct_remove",            //  6
    "ct_done",              //  7
    "ct_cancel",            //  8
    "ct_edit_delete",       //  9
    "ct_warning",           // 10
    "ct_star",              // 11
    "ct_info",              // 12
    "ct_help",              // 13
    "ct_home",              // 14
    "ct_index",             // 15
    "ct_mail",              // 16
    "ct_html",              // 17
    "ct_notes",             // 18
    "ct_timestamp",         // 19
    "ct_calendar",          // 20
    "ct_term",              // 21
    "ct_term-red",          // 22
    "ct_python",            // 23
    "ct_java",              // 24
    "ct_node_bullet",       // 25
    "ct_node_no_icon",      // 26
    CHERRY_BLACK,           // 27
    CHERRY_BLUE,            // 28
    CHERRY_CYAN,            // 29
    CHERRY_GREEN,           // 30
    CHERRY_GRAY,            // 31
    CHERRY_ORANGE,          // 32
    CHERRY_ORANGE_DARK,     // 33
    CHERRY_PURPLE,          // 34
    CHERRY_RED,             // 35
    CHERRY_SHERBERT,        // 36
    CHERRY_YELLOW,          // 37
    "ct_code",              // 38
    "ct_find",              // 39
    "ct_locked",            // 40
    "ct_unlocked",          // 41
    "ct_people",            // 42
    "ct_urgent",            // 43
    "ct_directory",         // 44
    "ct_leaf",              // 45
    "ct_xml",               // 46
    "ct_c",                 // 47
    "ct_cpp",               // 48
    "ct_perl",              // 49
    "ct_pin",               // 50
    "ct_anchor",            // 51
    "ct_edit",              // 52
    "ct_save",              // 53
    "ct_execute",           // 54
    "ct_preferences",       // 55
    "ct_clear",             // 56
    "ct_stop",              // 57
    "ct_close",             // 58
    "ct_quit-app",          // 59
    "ct_file",              // 60
    "ct_print",             // 61
    "ct_file_icon",         // 62
    "ct_link_handle",       // 63
    "ct_link_website",      // 64
    "ct_network",           // 65
    "ct_go-back",           // 66
    "ct_go-down",           // 67
    "ct_go-forward",        // 68
    "ct_go-up",             // 69
    "ct_go-jump",           // 70
    "ct_zoom-out",          // 71
    "ct_zoom-in",           // 72
    "ct_bg",                // 73
    "ct_cs",                // 74
    "ct_de",                // 75
    "ct_el",                // 76
    "ct_en",                // 77
    "ct_en_US",             // 78
    "ct_es",                // 79
    "ct_fi",                // 80
    "ct_fr",                // 81
    "ct_hy",                // 82
    "ct_it",                // 83
    "ct_ja",                // 84
    "ct_lt",                // 85
    "ct_nl",                // 86
    "ct_pl",                // 87
    "ct_pt_BR",             // 88
    "ct_ru",                // 89
    "ct_sl",                // 90
    "ct_sv",                // 91
    "ct_tr",                // 92
    "ct_uk",                // 93
    "ct_zh_CN",             // 94
    "ct_sports",            // 95
    "ct_briefcase",         // 96
    "ct_camera",            // 97
    "ct_chart",             // 98
    "ct_clapperboard",      // 99
    "ct_maths",             // 100
    "ct_games",             // 101
    "ct_globe",             // 102
    "ct_server",            // 103
    "ct_money",             // 104
    "ct_painting",          // 105
    "ct_puzzle",            // 106
    "ct_shopping",          // 107
    "ct_heart",             // 108
    "ct_smile",             // 109
    "ct_smile_cool",        // 110
    "ct_smile_surpr",       // 111
    "ct_skull",             // 112
    "ct_no_access",         // 113
    "ct_ruby",              // 114
    "ct_tux",               // 115
    "ct_gnome",             // 116
    "ct_debian",            // 117
    "ct_ubuntu",            // 118
    "ct_freebsd",           // 119
    "ct_win10",             // 120
    "ct_win2012",           // 121
    "ct_ko",                // 122
    "ct_kk_KZ",             // 123
    "ct_ro",                // 124
    "ct_hr",                // 125
    "ct_ghost",             // 126
    "ct_pt",                // 127
    "ct_zh_TW",             // 128
    "ct_hi_IN",             // 129
    "ct_ar",                // 130
    "ct_hu",                // 131
    "ct_heart_ukraine",     // 132
    "ct_microchip",         // 133
    "ct_ansible",           // 134
    "ct_aws",               // 135
    "ct_azure",             // 136
    "ct_docker",            // 137
    "ct_gcp",               // 138
    "ct_kubernetes",        // 139
    "ct_csharp",            // 140
    "ct_7zip",              // 141
    "ct_chat",              // 142
    "ct_db",                // 143
    "ct_dictionary",        // 144
    "ct_invest",            // 145
    "ct_keys",              // 146
    "ct_W-cloudy",          // 147
    "ct_W-few-clouds",      // 148
    "ct_W-fog",             // 149
    "ct_W-night-clear",     // 150
    "ct_W-night-few-clouds",// 151
    "ct_W-showers",         // 152
    "ct_W-snow",            // 153
    "ct_W-storm",           // 154
    "ct_W-sunny",           // 155
    "ct_apple",             // 156
    "ct_bike",              // 157
    "ct_bluetooth",         // 158
    "ct_building",          // 159
    "ct_bus",               // 160
    "ct_car",               // 161
    "ct_cellphone",         // 162
    "ct_cloud",             // 163
    "ct_computer",          // 164
    "ct_display",           // 165
    "ct_drive-harddisk",    // 166
    "ct_drive-usb",         // 167
    "ct_female",            // 168
    "ct_food",              // 169
    "ct_hamburger",         // 170
    "ct_lifebuoy",          // 171
    "ct_linuxmint",         // 172
    "ct_male",              // 173
    "ct_pizza",             // 174
    "ct_telephone",         // 175
    "ct_wifi",              // 176
    "ct_antenna",           // 177
    "ct_rust",              // 178
    "ct_fa",                // 179
    "ct_go",                // 180
    "ct_bug",               // 181
    "ct_github",            // 182
    "ct_gitlab",            // 183
    "ct_cmake",             // 184
    "ct_css",               // 185
    "ct_csv",               // 186
    "ct_diff",              // 187
    "ct_js",                // 188
    "ct_json",              // 189
    "ct_yaml",              // 190
    "ct_latex",             // 191
    "ct_lua",               // 192
    "ct_markdown",          // 193
    "ct_matlab",            // 194
    "ct_meson",             // 195
    "ct_php",               // 196
    "ct_scala",             // 197
    "ct_swift",             // 198
    "ct_ini",               // 199
    "ct_gtk",               // 200
    "ct_qt",                // 201
    "ct_bulb",              // 202
    "ct_airplane",          // 203
    "ct_alarm_clock",       // 204
    "ct_android",           // 205
    "ct_bat",               // 206
    "ct_bear",              // 207
    "ct_bell",              // 208
    "ct_bullseye",          // 209
    "ct_butterfly",         // 210
    "ct_cat",               // 211
    "ct_chick",             // 212
    "ct_coffee_beans",      // 213
    "ct_dog",               // 214
    "ct_dolphin",           // 215
    "ct_download",          // 216
    "ct_duck",              // 217
    "ct_fedora",            // 218
    "ct_fish",              // 219
    "ct_four_leaf_clover",  // 220
    "ct_fox",               // 221
    "ct_git",               // 222
    "ct_green_apple",       // 223
    "ct_hamster",           // 224
    "ct_horse",             // 225
    "ct_hot_drink",         // 226
    "ct_koala",             // 227
    "ct_lady_beetle",       // 228
    "ct_lion",              // 229
    "ct_map_marker",        // 230
    "ct_monkey",            // 231
    "ct_mushroom",          // 232
    "ct_owl",               // 233
    "ct_panda",             // 234
    "ct_pig",               // 235
    "ct_pool_8_ball",       // 236
    "ct_rabbit",            // 237
    "ct_rainbow",           // 238
    "ct_rocket",            // 239
    "ct_rooster",           // 240
    "ct_shield",            // 241
    "ct_ship",              // 242
    "ct_stackoverflow",     // 243
    "ct_zebra",             // 244
    "ct_postman",           // 245
};

const inline static std::vector<int> NODE_CUSTOM_ICONS_ORDERED {
    132/*heart_ukraine*/, 238/*rainbow*/, /*circle start*/1, 2, 3, 4/*circle end*/,
    206/*bat*/, 207/*bear*/, 210/*butterfly*/, 211/*cat*/, 212/*chick*/, 214/*dog*/, 215/*dolphin*/, 217/*duck*/,
    219/*fish*/, 221/*fox*/, 224/*hamster*/, 225/*horse*/, 227/*koala*/, 228/*lady_beetle*/, 229/*lion*/,
    231/*monkey*/, 233/*owl*/, 234/*panda*/, 235/*pig*/, 237/*rabbit*/, 240/*rooster*/, 244/*zebra*/,
    5/*add*/, 6/*remove*/, 7/*done*/, 8/*cancel*/, 9/*delete*/, 10/*warning*/, 43/*urgent*/, 113/*no access*/, 11/*star*/, 12/*info*/,
    13/*help*/, 171/*lifebuoy*/, 14/*home*/, 15/*index*/, 16/*mail*/, 18/*notes*/, 142/*chat*/, 19/*timestamp*/, 20/*calendar*/,
    204/*alarm_clock*/, 208/*bell*/,
    39/*find*/, 40/*locked*/, 41/*unlocked*/, 45/*leaf*/,
    50/*pin*/, 51/*anchor*/, 52/*edit*/, 53/*save*/, 54/*execute*/, 55/*preferences*/, 56/*clear*/, 57/*stop*/, 58/*close*/, 59/*quit app*/,
    60/*file*/, 141/*7zip*/, 44/*folder*/,
    61/*print*/, 62/*attachment*/, 63/*link*/, 64/*linkweb*/, 65/*network*/,
    66/*back*/, 67/*down*/, 68/*forward*/, 69/*up*/, 70/*jump to*/, 71/*zoom out*/, 72/*zoom in*/,
    95/*sports*/, 96/*briefcase*/, 159/*building*/, 98/*chart*/, 145/*invest*/, 104/*money*/,
    97/*camera*/, 99/*clapperboard*/, 100/*maths*/, 101/*games*/, 144/*dictionary*/,
    157/*bike*/, 161/*car*/, 160/*bus*/, 242/*ship*/, 203/*airplane*/, 239/*rocket*/,
    169/*food*/, 170/*hamburger*/, 174/*pizza*/, 232/*mushroom*/, 223/*green_apple*/, 213/*coffee beans*/, 226/*hot_drink*/,
    162/*cellphone*/, 175/*telephone*/,
    102/*globe*/, 230/*map_marker*/, 103/*server*/, 163/*cloud*/, 216/*download*/, 143/*db*/, 146/*keys*/, 241/*shield*/,
    202/*bulb*/, 176/*wifi*/, 177/*antenna*/, 158/*bluetooth*/,
    105/*painting*/, 106/*puzzle*/, 107/*shopping*/, 108/*heart*/,
    168/*female*/, 173/*male*/, 42/*people*/, 109/*smile*/, 110/*smile cool*/, 111/*smile surpr*/, 112/*skull*/, 126/*ghost*/,
    /*os start*/115, 116, 117, 118, 172/*linuxmint*/, 218/*fedora*/, 119/*freebsd*/, 205/*android*/, 120, 121, 156/*os end*/,
    133/*microchip*/, 164/*computer*/, 165/*display*/, 166/*drive hd*/, 167/*drive usb*/,
    /*coding start*/222/*git*/, 243/*stackoverflow*/, 181, 17, 21, 22, 23, 24, 178, 180, 38, 46, 47, 48, 49, 140, 114,
    184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201/*coding end*/,
    /*devops start*/134, 135, 136, 137, 138, 139, 182, 183, 245/*devops end*/,
    /*weather start*/147, 148, 149, 150, 151, 152, 153, 154, 155/*weather end*/,
    /*cherries start*/27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37/*cherries end*/,
    25/*bullet*/, 26/*noicon*/, 209/*bullseye*/, 220/*four_leaf_clover*/, 236/*pool_8_ball*/,
    /*flags start*/73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 122, 123, 124, 125, 127, 128, 129, 130, 131, 179/*flags end*/
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

const inline static std::vector<std::pair<const gchar*, const gchar*>> NODE_CODE_ICONS {
    std::make_pair("c",             "ct_c"),
    std::make_pair("c-sharp",       "ct_csharp"),
    std::make_pair("cmake",         "ct_cmake"),
    std::make_pair("cpp",           "ct_cpp"),
    std::make_pair("css",           "ct_css"),
    std::make_pair("csv",           "ct_csv"),
    std::make_pair("diff",          "ct_diff"),
    std::make_pair("dosbatch",      "ct_term-red"),
    std::make_pair("go",            "ct_go"),
    std::make_pair("gtk-doc",       "ct_gtk"),
    std::make_pair("gtkrc",         "ct_gtk"),
    std::make_pair("html",          "ct_html"),
    std::make_pair("ini",           "ct_ini"),
    std::make_pair("java",          "ct_java"),
    std::make_pair("js",            "ct_js"),
    std::make_pair("json",          "ct_json"),
    std::make_pair("latex",         "ct_latex"),
    std::make_pair("lua",           "ct_lua"),
    std::make_pair("markdown",      "ct_markdown"),
    std::make_pair("markdown-extra","ct_markdown"),
    std::make_pair("matlab",        "ct_matlab"),
    std::make_pair("meson",         "ct_meson"),
    std::make_pair("perl",          "ct_perl"),
    std::make_pair("php",           "ct_php"),
    std::make_pair("powershell",    "ct_term-red"),
    std::make_pair("python",        "ct_python"),
    std::make_pair("python3",       "ct_python"),
    std::make_pair("ruby",          "ct_ruby"),
    std::make_pair("rust",          "ct_rust"),
    std::make_pair("scala",         "ct_scala"),
    std::make_pair("sh",            "ct_term"),
    std::make_pair("sql",           "ct_db"),
    std::make_pair("swift",         "ct_swift"),
    std::make_pair("xml",           "ct_xml"),
    std::make_pair("yaml",          "ct_yaml"),
};

const inline static gchar* CODE_EXEC_TMP_SRC  {"<tmp_src_path>"};
const inline static gchar* CODE_EXEC_TMP_BIN  {"<tmp_bin_path>"};
const inline static gchar* CODE_EXEC_CODE_TXT {"<code_txt>"};
const inline static gchar* CODE_EXEC_COMMAND  {"<command>"};

const inline static std::vector<std::pair<const std::string, const std::string>> CODE_EXEC_TYPE_CMD_DEFAULT {
    std::make_pair("c",          std::string{"gcc -o "} + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("cpp",        std::string{"g++ -o "} + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("dosbatch",   std::string{"call "} + CODE_EXEC_TMP_SRC),
    std::make_pair("perl",       std::string{"perl "} + CODE_EXEC_TMP_SRC),
    std::make_pair("powershell", std::string{"powershell -File "} + CODE_EXEC_TMP_SRC),
    std::make_pair("python",     std::string{"python2 "} + CODE_EXEC_TMP_SRC),
    std::make_pair("python3",    std::string{"python3 "} + CODE_EXEC_TMP_SRC),
    std::make_pair("sh",         CODE_EXEC_CODE_TXT),
    std::make_pair("c-sharp",    std::string{"csc /out:"} + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("rust",       std::string{"rustc -o "} + CODE_EXEC_TMP_BIN + " " + CODE_EXEC_TMP_SRC + " && " + CODE_EXEC_TMP_BIN),
    std::make_pair("java",       std::string{"java "} + CODE_EXEC_TMP_SRC),
    std::make_pair("go",         std::string{"go run "} + CODE_EXEC_TMP_SRC),
};

const inline static std::vector<std::pair<const std::string, const std::string>> CODE_EXEC_TYPE_EXT_DEFAULT {
    std::make_pair("c",          "c"),
    std::make_pair("cpp",        "cpp"),
    std::make_pair("dosbatch",   "bat"),
    std::make_pair("perl",       "pl"),
    std::make_pair("powershell", "ps1"),
    std::make_pair("python",     "py"),
    std::make_pair("python3",    "py"),
    std::make_pair("sh",         "sh"),
    std::make_pair("c-sharp",    "cs"),
    std::make_pair("java",       "java"),
    std::make_pair("rust",       "rs"),
    std::make_pair("go",         "go"),
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

const inline static Glib::ustring TARGET_CTD_PLAIN_TEXT = "UTF8_STRING";
const inline static Glib::ustring TARGET_CTD_RICH_TEXT = "CTD_RICH";
const inline static Glib::ustring TARGET_CTD_TABLE = "CTD_TABLE";
const inline static Glib::ustring TARGET_CTD_CODEBOX = "CTD_CODEBOX";
const inline static Glib::ustring TARGET_WIN_HTML = "HTML Format";
const inline static std::vector<Glib::ustring> TARGETS_HTML = {"text/html", TARGET_WIN_HTML};
const inline static Glib::ustring TARGET_URI_LIST = "text/uri-list";
const inline static Glib::ustring TARGET_GTK_TEXT_BUFFER_CONTENTS = "GTK_TEXT_BUFFER_CONTENTS";
const inline static std::vector<Glib::ustring> TARGETS_PLAIN_TEXT = {"UTF8_STRING", "COMPOUND_TEXT", "STRING", "TEXT", "text/plain;charset=utf-8", "text/plain"};
const inline static std::vector<Glib::ustring> TARGETS_IMAGES = {"image/png", "image/jpeg", "image/bmp", "image/tiff", "image/x-MS-bmp", "image/x-bmp"};
const inline static Glib::ustring TARGET_WINDOWS_FILE_NAME = "FileName";
const inline static Glib::ustring TAG_UL_START = "<ul>";
const inline static Glib::ustring TAG_UL_END = "</ul>";
const inline static Glib::ustring TAG_OL_START = "<ol>";
const inline static Glib::ustring TAG_OL_END = "</ol>";
const inline static Glib::ustring TAG_LI_START = "<li>";
const inline static Glib::ustring TAG_LI_END = "</li>";

}; // struct CtConst
