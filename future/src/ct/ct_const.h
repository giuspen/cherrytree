/*
 * ct_extern const.h
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

#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <glibmm.h>

namespace CtConst {

extern const gchar    CT_VERSION[];
extern const gchar    APP_NAME[];
extern const bool     IS_WIN_OS;
extern const int      MAX_RECENT_DOCS;
extern const int      MAX_RECENT_DOCS_RESTORE;
extern const int      NODE_ICON_CODE_ID;
extern const int      NODE_ICON_BULLET_ID;
extern const int      NODE_ICON_NO_ICON_ID;
extern const int      NODE_ICON_SIZE;
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
extern const gchar    PLAIN_TEXT_ID[];
extern const gchar    SYN_HIGHL_BASH[];
extern const gchar    STYLE_SCHEME_LIGHT[];
extern const gchar    STYLE_SCHEME_DARK[];
extern const gchar    STYLE_SCHEME_GRAY[];
extern const gchar    TIMESTAMP_FORMAT_DEFAULT[];
extern const gchar    SPECIAL_CHARS_DEFAULT[];
extern const gchar    SELWORD_CHARS_DEFAULT[];
extern const gchar    CHARS_LISTBUL_DEFAULT[];
extern const gchar    CHARS_TOC_DEFAULT[];
extern const gchar    CHARS_TODO_DEFAULT[];
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
extern const gchar    CHAR_NEWLINE[];
extern const std::set<const gchar*> TEXT_SYNTAXES;
extern const std::set<const gchar*> TAG_PROPERTIES;
extern const gchar    TOOLBAR_VEC_DEFAULT[];
extern const gchar*   AVAILABLE_LANGS[20];
extern const std::unordered_map<int, Glib::ustring> NODES_STOCKS;
extern const std::unordered_map<int, Glib::ustring> NODES_ICONS;
extern const std::map<Glib::ustring, Glib::ustring> CODE_ICONS;

extern const Glib::ustring CODE_EXEC_TMP_SRC;
extern const Glib::ustring CODE_EXEC_TMP_BIN;
extern const Glib::ustring CODE_EXEC_COMMAND;
extern const std::map<Glib::ustring, Glib::ustring> CODE_EXEC_TYPE_CMD_DEFAULT;
extern const std::map<Glib::ustring, Glib::ustring> CODE_EXEC_TERM_RUN_DEFAULT;


Glib::ustring getStockIdForCodeType(Glib::ustring code_type);

}; // namespace CtConst
