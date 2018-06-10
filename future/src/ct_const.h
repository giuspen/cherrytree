/*
 * ct_const.h
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

#include <map>
#include <glibmm.h>

#define MAX_RECENT_DOCS             10
#define MAX_RECENT_DOCS_RESTORE      3
#define NODE_ICON_CODE_ID           38
#define NODE_ICON_BULLET_ID         25
#define NODE_ICON_NO_ICON_ID        26
#define NODE_ICON_SIZE              16

extern const gchar   LINK_TYPE_WEBS[];
extern const gchar   LINK_TYPE_FILE[];
extern const gchar   LINK_TYPE_FOLD[];
extern const gchar   LINK_TYPE_NODE[];
extern const gchar   NODE_ICON_TYPE_CHERRY[];
extern const gchar   NODE_ICON_TYPE_CUSTOM[];
extern const gchar   NODE_ICON_TYPE_NONE[];
extern const gchar   CHERRY_RED[];
extern const gchar   CHERRY_BLUE[];
extern const gchar   CHERRY_ORANGE[];
extern const gchar   CHERRY_CYAN[];
extern const gchar   CHERRY_ORANGE_DARK[];
extern const gchar   CHERRY_SHERBERT[];
extern const gchar   CHERRY_YELLOW[];
extern const gchar   CHERRY_GREEN[];
extern const gchar   CHERRY_PURPLE[];
extern const gchar   CHERRY_BLACK[];
extern const gchar   CHERRY_GRAY[];
extern const gchar   RICH_TEXT_ID[];
extern const gchar   PLAIN_TEXT_ID[];
extern const gchar   SYN_HIGHL_BASH[];
extern const gchar   STYLE_SCHEME_LIGHT[];
extern const gchar   STYLE_SCHEME_DARK[];
extern const gchar   STYLE_SCHEME_GRAY[];
extern const gchar   TIMESTAMP_FORMAT_DEFAULT[];
extern const gchar   SPECIAL_CHARS_DEFAULT[];
extern const gchar   SELWORD_CHARS_DEFAULT[];
extern const gchar   CHARS_LISTBUL_DEFAULT[];
extern const gchar   CHARS_TOC_DEFAULT[];
extern const gchar   COLOR_48_LINK_WEBS[];
extern const gchar   COLOR_48_LINK_NODE[];
extern const gchar   COLOR_48_LINK_FILE[];
extern const gchar   COLOR_48_LINK_FOLD[];
extern const gchar   COLOR_48_YELLOW[];
extern const gchar   COLOR_48_WHITE[];
extern const gchar   COLOR_48_BLACK[];
extern const gchar   COLOR_24_BLACK[];
extern const gchar   COLOR_24_WHITE[];
extern const gchar   COLOR_24_BLUEBG[];
extern const gchar   COLOR_24_LBLACK[];
extern const gchar   COLOR_24_GRAY[];
extern const gchar   DEFAULT_MONOSPACE_BG[];
extern const gchar  *RICH_TEXT_DARK_FG;
extern const gchar  *RICH_TEXT_DARK_BG;
extern const gchar  *RICH_TEXT_LIGHT_FG;
extern const gchar  *RICH_TEXT_LIGHT_BG;
extern const gchar  *TREE_TEXT_DARK_FG;
extern const gchar  *TREE_TEXT_DARK_BG;
extern const gchar  *TREE_TEXT_LIGHT_FG;
extern const gchar  *TREE_TEXT_LIGHT_BG;
extern const gchar   TOOLBAR_VEC_DEFAULT[];
extern const std::map<int, Glib::ustring> NODES_STOCKS;
extern const std::map<int, Glib::ustring> NODES_ICONS;
extern const std::map<Glib::ustring, Glib::ustring> CODE_ICONS;

Glib::ustring get_stock_id_for_code_type(Glib::ustring code_type);
