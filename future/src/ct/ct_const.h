/*
 * ct_extern const.h
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
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

namespace CtConst {

extern const gchar    CT_VERSION[];
extern const gchar    APP_NAME[];
extern const bool     IS_WIN_OS;
extern const int      NODE_ICON_CODE_ID;
extern const int      NODE_ICON_BULLET_ID;
extern const int      NODE_ICON_NO_ICON_ID;
extern const int      NODE_ICON_SIZE;
extern const int      MAX_TOOLTIP_LINK_CHARS;
extern const gchar    CTDOC_XML_NOENC[];
extern const gchar    CTDOC_XML_ENC[];
extern const gchar    CTDOC_SQLITE_NOENC[];
extern const gchar    CTDOC_SQLITE_ENC[];
extern const gchar    LINK_TYPE_WEBS[];
extern const gchar    LINK_TYPE_FILE[];
extern const gchar    LINK_TYPE_FOLD[];
extern const gchar    LINK_TYPE_NODE[];
extern const gchar    NODE_ICON_TYPE_CHERRY[];
extern const gchar    NODE_ICON_TYPE_CUSTOM[];
extern const gchar    NODE_ICON_TYPE_NONE[];
extern const gchar    CHERRY_RED[];
extern const gchar    CHERRY_BLUE[];
extern const gchar    CHERRY_ORANGE[];
extern const gchar    CHERRY_CYAN[];
extern const gchar    CHERRY_ORANGE_DARK[];
extern const gchar    CHERRY_SHERBERT[];
extern const gchar    CHERRY_YELLOW[];
extern const gchar    CHERRY_GREEN[];
extern const gchar    CHERRY_PURPLE[];
extern const gchar    CHERRY_BLACK[];
extern const gchar    CHERRY_GRAY[];
extern const gchar    RICH_TEXT_ID[];
extern const gchar    TABLE_CELL_TEXT_ID[];
extern const gchar    PLAIN_TEXT_ID[];
extern const gchar    SYN_HIGHL_BASH[];
extern const gchar    STYLE_SCHEME_LIGHT[];
extern const gchar    STYLE_SCHEME_DARK[];
extern const gchar    STYLE_SCHEME_GRAY[];
extern const gchar    TIMESTAMP_FORMAT_DEFAULT[];
extern const Glib::ustring SPECIAL_CHARS_DEFAULT;
extern const Glib::ustring SPECIAL_CHAR_ARROW_RIGHT;
extern const Glib::ustring SPECIAL_CHAR_ARROW_RIGHT2;
extern const Glib::ustring SPECIAL_CHAR_ARROW_LEFT;
extern const Glib::ustring SPECIAL_CHAR_ARROW_LEFT2;
extern const Glib::ustring SPECIAL_CHAR_ARROW_DOUBLE;
extern const Glib::ustring SPECIAL_CHAR_ARROW_DOUBLE2;
extern const Glib::ustring SPECIAL_CHAR_COPYRIGHT;
extern const Glib::ustring SPECIAL_CHAR_UNREGISTERED_TRADEMARK;
extern const Glib::ustring SPECIAL_CHAR_REGISTERED_TRADEMARK;
extern const Glib::ustring SELWORD_CHARS_DEFAULT;
extern const Glib::ustring CHARS_LISTBUL_DEFAULT;
extern const Glib::ustring CHARS_TOC_DEFAULT;
extern const Glib::ustring CHARS_TODO_DEFAULT;
extern const Glib::ustring CHARS_SMART_DQUOTE_DEFAULT;
extern const Glib::ustring CHARS_SMART_SQUOTE_DEFAULT;
extern const gchar    COLOR_48_LINK_WEBS[];
extern const gchar    COLOR_48_LINK_NODE[];
extern const gchar    COLOR_48_LINK_FILE[];
extern const gchar    COLOR_48_LINK_FOLD[];
extern const gchar    COLOR_48_YELLOW[];
extern const gchar    COLOR_48_WHITE[];
extern const gchar    COLOR_48_BLACK[];
extern const gchar    COLOR_24_BLACK[];
extern const gchar    COLOR_24_WHITE[];
extern const gchar    COLOR_24_BLUEBG[];
extern const gchar    COLOR_24_LBLACK[];
extern const gchar    COLOR_24_GRAY[];
extern const gchar    DEFAULT_MONOSPACE_BG[];
extern const gchar*   RICH_TEXT_DARK_FG;
extern const gchar*   RICH_TEXT_DARK_BG;
extern const gchar*   RICH_TEXT_LIGHT_FG;
extern const gchar*   RICH_TEXT_LIGHT_BG;
extern const gchar*   TREE_TEXT_DARK_FG;
extern const gchar*   TREE_TEXT_DARK_BG;
extern const gchar*   TREE_TEXT_LIGHT_FG;
extern const gchar*   TREE_TEXT_LIGHT_BG;
extern const gchar    GTKSPELLCHECK_TAG_NAME[];
extern const gchar    TAG_WEIGHT[];
extern const gchar    TAG_FOREGROUND[];
extern const gchar    TAG_BACKGROUND[];
extern const gchar    TAG_STYLE[];
extern const gchar    TAG_UNDERLINE[];
extern const gchar    TAG_STRIKETHROUGH[];
extern const gchar    TAG_SCALE[];
extern const gchar    TAG_FAMILY[];
extern const gchar    TAG_JUSTIFICATION[];
extern const gchar    TAG_LINK[];
extern const gchar    TAG_SEPARATOR[];
extern const gchar    TAG_SEPARATOR_ANSI_REPR[];
extern const gchar    TAG_PROP_VAL_HEAVY[];
extern const gchar    TAG_PROP_VAL_ITALIC[];
extern const gchar    TAG_PROP_VAL_MONOSPACE[];
extern const gchar    TAG_PROP_VAL_SINGLE[];
extern const gchar    TAG_PROP_VAL_SMALL[];
extern const gchar    TAG_PROP_VAL_TRUE[];
extern const gchar    TAG_PROP_VAL_H1[];
extern const gchar    TAG_PROP_VAL_H2[];
extern const gchar    TAG_PROP_VAL_H3[];
extern const gchar    TAG_PROP_VAL_H4[];
extern const gchar    TAG_PROP_VAL_H5[];
extern const gchar    TAG_PROP_VAL_H6[];
extern const gchar    TAG_PROP_VAL_SUP[];
extern const gchar    TAG_PROP_VAL_SUB[];
extern const gchar    TAG_PROP_VAL_LEFT[];
extern const gchar    TAG_PROP_VAL_CENTER[];
extern const gchar    TAG_PROP_VAL_RIGHT[];
extern const gchar    TAG_PROP_VAL_FILL[];
extern const gchar    STR_KEY_UP[];
extern const gchar    STR_KEY_DOWN[];
extern const gchar    STR_KEY_LEFT[];
extern const gchar    STR_KEY_RIGHT[];
extern const gchar    STR_STOCK_CT_IMP[];
extern const int      MAX_FILE_NAME_LEN;
extern const int      WHITE_SPACE_BETW_PIXB_AND_TEXT;
extern const int      GRID_SLIP_OFFSET;
extern const Glib::ustring  CHAR_SPACE;
extern const Glib::ustring  CHAR_NEWLINE;
extern const Glib::ustring  CHAR_NEWPAGE;
extern const Glib::ustring  CHAR_CR;
extern const Glib::ustring  CHAR_TAB;
extern const Glib::ustring  CHARS_LISTNUM;
extern const int            NUM_CHARS_LISTNUM;
extern const Glib::ustring  CHAR_TILDE;
extern const Glib::ustring  CHAR_MINUS;
extern const Glib::ustring  CHAR_DQUOTE;
extern const Glib::ustring  CHAR_SQUOTE;
extern const Glib::ustring  CHAR_GRAVE;
extern const Glib::ustring  CHAR_SLASH;
extern const Glib::ustring  CHAR_BSLASH;
extern const Glib::ustring  CHAR_SQ_BR_OPEN;
extern const Glib::ustring  CHAR_SQ_BR_CLOSE;
extern const Glib::ustring  CHAR_PARENTH_OPEN;
extern const Glib::ustring  CHAR_PARENTH_CLOSE;
extern const Glib::ustring  CHAR_LESSER;
extern const Glib::ustring  CHAR_GREATER;
extern const Glib::ustring  CHAR_STAR;
extern const Glib::ustring  CHAR_QUESTION;
extern const Glib::ustring  CHAR_COMMA;
extern const Glib::ustring  CHAR_COLON;
extern const Glib::ustring  CHAR_SEMICOLON;
extern const Glib::ustring  CHAR_USCORE;
extern const Glib::ustring  CHAR_EQUAL;
extern const Glib::ustring  CHAR_BR_OPEN;
extern const Glib::ustring  CHAR_BR_CLOSE;
extern const Glib::ustring  CHAR_CARET;
extern const Glib::ustring  CHAR_PIPE;
extern const Glib::ustring  CHAR_AMPERSAND;
extern const std::vector<Glib::ustring> WEB_LINK_STARTERS;
extern const std::unordered_set<const gchar*> TEXT_SYNTAXES;
extern const std::unordered_set<const gchar*> TAG_PROPERTIES;
extern const gchar    TOOLBAR_VEC_DEFAULT[];
extern const std::vector<std::string> TOOLBAR_VEC_BLACKLIST;
extern const gchar*   AVAILABLE_LANGS[21];
extern const std::unordered_map<int, std::string> NODES_STOCKS;
extern const std::unordered_map<int, std::string> NODES_ICONS;
extern const std::unordered_map<std::string, std::string> CODE_ICONS;

extern const std::string CODE_EXEC_TMP_SRC;
extern const std::string CODE_EXEC_TMP_BIN;
extern const std::string CODE_EXEC_COMMAND;
extern const std::unordered_map<std::string, std::string> CODE_EXEC_TYPE_CMD_DEFAULT;
extern const std::unordered_map<std::string, std::string> CODE_EXEC_TYPE_EXT_DEFAULT;
extern const std::unordered_map<std::string, std::string> CODE_EXEC_TERM_RUN_DEFAULT;

std::string getStockIdForCodeType(std::string code_type);

}; // namespace CtConst
