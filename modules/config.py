# -*- coding: UTF-8 -*-
#
#       config.py
#
#       Copyright 2009-2020
#       Giuseppe Penone <giuspen@gmail.com>
#       Evgenii Gurianov <https://github.com/txe>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.

import os
import sys
import ConfigParser
import gtk
import pango
import locale
import subprocess
import webbrowser
import cgi
import cons
import menus
import support
import exports
import codeboxes
import pgsc_spellcheck
if cons.HAS_APPINDICATOR: import appindicator

ICONS_SIZE = {1: gtk.ICON_SIZE_MENU, 2: gtk.ICON_SIZE_SMALL_TOOLBAR, 3: gtk.ICON_SIZE_LARGE_TOOLBAR,
              4: gtk.ICON_SIZE_DND, 5: gtk.ICON_SIZE_DIALOG}
if cons.IS_WIN_OS:
    LINK_CUSTOM_ACTION_DEFAULT_WEB = "explorer %s &"
else:
    LINK_CUSTOM_ACTION_DEFAULT_WEB = "firefox %s &"
if cons.IS_WIN_OS:
    LINK_CUSTOM_ACTION_DEFAULT_FILE = "explorer %s &"
elif cons.IS_MAC_OS:
    LINK_CUSTOM_ACTION_DEFAULT_FILE = "open %s &"
else:
    LINK_CUSTOM_ACTION_DEFAULT_FILE = "xdg-open %s &"

CODE_EXEC_TMP_SRC = "<tmp_src_path>"
CODE_EXEC_TMP_BIN = "<tmp_bin_path>"
CODE_EXEC_COMMAND = "<command>"
CODE_EXEC_TYPE_CMD_DEFAULT = {
"c": "gcc -o %s %s && %s" % (CODE_EXEC_TMP_BIN, CODE_EXEC_TMP_SRC, CODE_EXEC_TMP_BIN),
"cpp": "g++ -o %s %s && %s" % (CODE_EXEC_TMP_BIN, CODE_EXEC_TMP_SRC, CODE_EXEC_TMP_BIN),
"dosbatch": "call %s" % CODE_EXEC_TMP_SRC,
"perl": "perl %s" % CODE_EXEC_TMP_SRC,
"powershell": "powershell -File %s" % CODE_EXEC_TMP_SRC,
"python": "python2 %s" % CODE_EXEC_TMP_SRC,
"python3": "python3 %s" % CODE_EXEC_TMP_SRC,
"sh": "sh %s" % CODE_EXEC_TMP_SRC,
}
CODE_EXEC_TYPE_EXT_DEFAULT = {
"c": "c",
"cpp": "cpp",
"dosbatch": "bat",
"perl": "pl",
"powershell": "ps1",
"python": "py",
"python3": "py",
"sh": "sh",
}
CODE_EXEC_TERM_RUN_DEFAULT = {
"linux" : "xterm -hold -geometry 180x45 -e \"%s\"" % CODE_EXEC_COMMAND,
"win" : "start cmd /k \"%s\"" % CODE_EXEC_COMMAND,
}
DEFAULT_MONOSPACE_BG = "#7f7f7f"
MAX_SIZE_EMBFILE_MB_DEFAULT = 10
HORIZONTAL_RULE = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
COLOR_PALETTE_DEFAULT = ["#000000", "#ffffff", "#7f7f7f", "#ff0000", "#a020f0",
                         "#0000ff", "#add8e6", "#00ff00", "#ffff00", "#ffa500",
                         "#e6e6fa", "#a52a2a", "#8b6914", "#1e90ff", "#ffc0cb",
                         "#90ee90", "#1a1a1a", "#4d4d4d", "#bfbfbf", "#e5e5e5"]
SPECIAL_CHARS_DEFAULT = unicode("“”„‘’•◇▪▸☐☑☒★…‰€©®™°↓↑→←↔↵⇓⇑⇒⇐⇔»«▼▲►◄≤≥≠≈±¹²³½¼⅛×÷∞ø∑√∫ΔδΠπΣσΦΩωαβγεηλμ☺☻☼♥♣♦✔♀♂♪♫✝", cons.STR_UTF8, cons.STR_IGNORE)
SELWORD_CHARS_DEFAULT = unicode(".-@", cons.STR_UTF8, cons.STR_IGNORE)
CHARS_LISTBUL_DEFAULT = unicode("•◇▪-→⇒", cons.STR_UTF8, cons.STR_IGNORE)
CHARS_TODO_DEFAULT = unicode("☐☑☒", cons.STR_UTF8, cons.STR_IGNORE)
CHARS_TOC_DEFAULT = unicode("▸•◇▪", cons.STR_UTF8, cons.STR_IGNORE)
CHARS_SMART_DQUOTE_DEFAULT = unicode("“”", cons.STR_UTF8, cons.STR_IGNORE)
CHARS_SMART_SQUOTE_DEFAULT = unicode("‘’", cons.STR_UTF8, cons.STR_IGNORE)
NODES_ON_NODE_NAME_HEADER_DEFAULT = 3
TIMESTAMP_FORMAT_DEFAULT = "%Y/%m/%d - %H:%M"
SEPARATOR_ASCII_REPR = "---------"
JOURNAL_DAY_FORMAT_DEFAULT = "%d %a"

try:
    SPELL_CHECK_LANG_DEFAULT = locale.getdefaultlocale()[0]
    assert SPELL_CHECK_LANG_DEFAULT is not None
except: SPELL_CHECK_LANG_DEFAULT = "en_US"


def get_code_exec_ext(dad, syntax_type):
    if syntax_type in dad.custom_codexec_ext.keys():
        ret_val = dad.custom_codexec_ext[syntax_type]
    elif syntax_type in CODE_EXEC_TYPE_EXT_DEFAULT.keys():
        ret_val = CODE_EXEC_TYPE_EXT_DEFAULT[syntax_type]
    else:
        ret_val = "txt"
    return ret_val

def get_code_exec_type_cmd(dad, syntax_type):
    if syntax_type in dad.custom_codexec_type.keys():
        ret_val = dad.custom_codexec_type[syntax_type]
    elif syntax_type in CODE_EXEC_TYPE_CMD_DEFAULT.keys():
        ret_val = CODE_EXEC_TYPE_CMD_DEFAULT[syntax_type]
    else:
        ret_val = None
    return ret_val

def get_code_exec_type_keys(dad):
    all_codexec_keys = dad.custom_codexec_type.keys()
    for key in CODE_EXEC_TYPE_CMD_DEFAULT.keys():
        if not key in all_codexec_keys:
            all_codexec_keys.append(key)
    return sorted(all_codexec_keys)

def get_code_exec_term_run(dad, op_sys=None):
    if not op_sys:
        op_sys = "linux" if not cons.IS_WIN_OS else "win"
    if dad.custom_codexec_term:
        ret_val = dad.custom_codexec_term
    else:
        ret_val = CODE_EXEC_TERM_RUN_DEFAULT[op_sys]
    return ret_val

def get_stock_id_for_code_type(key):
    return cons.CODE_ICONS[key] if key in cons.CODE_ICONS else cons.NODES_STOCKS[cons.NODE_ICON_CODE_ID]

def get_toolbar_entry_columns_from_key(dad, key):
    if key == cons.TAG_SEPARATOR: return [key, "", SEPARATOR_ASCII_REPR]
    if key == cons.CHAR_STAR: return [key, "gtk-open", _("Open a CherryTree Document")]
    for element in menus.get_entries(dad):
        if len(element) == 3: continue
        if key == element[0]: return [element[0], element[1], element[4]]
    return ["", "", ""]

def get_toolbar_icon_n_label_list(dad):
    icon_n_label_list = [[cons.TAG_SEPARATOR, "", SEPARATOR_ASCII_REPR]]
    for element in menus.get_entries(dad):
        if len(element) == 3: continue
        if element[0] in dad.toolbar_ui_list: continue
        if element[0] in menus.TOOLBAR_VEC_BLACKLIST: continue
        if element[0] == "ct_open_file" and cons.CHAR_STAR in dad.toolbar_ui_list: continue
        icon_n_label_list.append([element[0], element[1], element[4]])
    return icon_n_label_list

def get_toolbar_ui_str(dad):
    dad.toolbar_open_n_recent = -1
    toolbar_ui_str = "<ui><toolbar name='ToolBar'>"
    for i, toolbar_element in enumerate(dad.toolbar_ui_list):
        if toolbar_element == cons.CHAR_STAR: dad.toolbar_open_n_recent = i
        elif toolbar_element == cons.TAG_SEPARATOR: toolbar_ui_str += "<separator/>"
        elif toolbar_element == menus.TOOLBAR_SPLIT: pass
        else: toolbar_ui_str += "<toolitem action='%s'/>" % toolbar_element
    return toolbar_ui_str + "</toolbar></ui>"

def get_node_path_from_str(str_path_list_of_str):
    str_path_list_of_str = str(str_path_list_of_str)
    if str_path_list_of_str.startswith(cons.CHAR_PARENTH_OPEN)\
    and str_path_list_of_str.endswith(cons.CHAR_PARENTH_CLOSE):
        str_path_list_of_str = str_path_list_of_str[1:-1].replace(cons.CHAR_COMMA, "")
    path_list_of_str = str_path_list_of_str.split()
    path_list_of_int = [int(element) for element in path_list_of_str]
    return tuple(path_list_of_int)

def get_node_path_str_from_path(tree_path):
    path_list_of_str = []
    for element in tree_path:
        path_list_of_str.append( str(element) )
    return " ".join(path_list_of_str)

def get_pixels_inside_wrap(space_around_lines, relative_wrapped_space):
    return int(round(space_around_lines * (relative_wrapped_space / 100.0)))

def config_file_load(dad):
    """Load the Preferences from Config File"""
    dad.custom_kb_shortcuts = {}
    dad.custom_codexec_type = {}
    dad.custom_codexec_ext = {}
    dad.custom_codexec_term = None
    dad.latest_tag = ["", ""]
    if os.path.isfile(cons.CONFIG_PATH):
        cfg = ConfigParser.RawConfigParser()
        try:
            cfg.read(cons.CONFIG_PATH)
        except ConfigParser.MissingSectionHeaderError:
            print "? ConfigParser.MissingSectionHeaderError"

        section = "state"
        dad.file_dir = unicode(cfg.get(section, "file_dir"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "file_dir") else ""
        dad.file_name = unicode(cfg.get(section, "file_name"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "file_name") else ""
        dad.toolbar_visible = cfg.getboolean(section, "toolbar_visible") if cfg.has_option(section, "toolbar_visible") else True
        dad.win_is_maximized = cfg.getboolean(section, "win_is_maximized") if cfg.has_option(section, "win_is_maximized") else False
        # restore window size and position
        if cfg.has_option(section, "win_position_x") and cfg.has_option(section, "win_position_y"):
            dad.win_position = [cfg.getint(section, "win_position_x"), cfg.getint(section, "win_position_y")]
            dad.window.move(dad.win_position[0], dad.win_position[1])
        else: dad.win_position = [10, 10]
        if dad.win_is_maximized: dad.window.maximize()
        elif cfg.has_option(section, "win_size_w") and cfg.has_option(section, "win_size_h"):
            win_size = [cfg.getint(section, "win_size_w"), cfg.getint(section, "win_size_h")]
            dad.window.resize(win_size[0], win_size[1])
        dad.hpaned_pos = cfg.getint(section, "hpaned_pos") if cfg.has_option(section, "hpaned_pos") else 170
        dad.tree_visible = cfg.getboolean(section, "tree_visible") if cfg.has_option(section, "tree_visible") else True
        if cfg.has_option(section, "node_path"):
            # restore the selected node
            dad.node_path = get_node_path_from_str(cfg.get(section, "node_path"))
            dad.cursor_position = cfg.getint(section, "cursor_position") if cfg.has_option(section, "cursor_position") else 0
        else: dad.node_path = None
        dad.recent_docs = []
        saved_from_gtkmm = False
        for i in range(cons.MAX_RECENT_DOCS):
            curr_key = "doc_%s" % i
            if cfg.has_option(section, curr_key):
                dad.recent_docs.append(unicode(cfg.get(section, curr_key), cons.STR_UTF8, cons.STR_IGNORE))
                # supporting saved from gtkmm
                if not dad.file_name or saved_from_gtkmm is True:
                    if i == 0:
                        dad.file_name = os.path.basename(dad.recent_docs[0])
                        dad.file_dir = os.path.dirname(dad.recent_docs[0])
                        if cfg.has_option(section, "nodep_0"):
                            saved_from_gtkmm = True
                            dad.node_path = get_node_path_from_str(cfg.get(section, "nodep_0").replace(":", " "))
                            dad.cursor_position = cfg.getint(section, "curs_0")
                            dad.expanded_collapsed_string = cfg.get(section, "expcol_0")
                    elif i in (1,2,3):
                        if cfg.has_option(section, "nodep_%s" % i):
                            setattr(dad, "expcollnam%s" % i, os.path.basename(dad.recent_docs[i]))
                            setattr(dad, "expcollstr%s" % i, cfg.get(section, "expcol_%s" % i))
                            setattr(dad, "expcollsel%s" % i, cfg.get(section, "nodep_%s" % i))
                            setattr(dad, "expcollcur%s" % i, cfg.get(section, "curs_%s" % i))
            else:
                break
        dad.pick_dir_import = cfg.get(section, "pick_dir_import") if cfg.has_option(section, "pick_dir_import") else ""
        dad.pick_dir_export = cfg.get(section, "pick_dir_export") if cfg.has_option(section, "pick_dir_export") else ""
        dad.pick_dir_file = cfg.get(section, "pick_dir_file") if cfg.has_option(section, "pick_dir_file") else ""
        dad.pick_dir_img = cfg.get(section, "pick_dir_img") if cfg.has_option(section, "pick_dir_img") else ""
        dad.pick_dir_csv = cfg.get(section, "pick_dir_csv") if cfg.has_option(section, "pick_dir_csv") else ""
        dad.pick_dir_cbox = cfg.get(section, "pick_dir_cbox") if cfg.has_option(section, "pick_dir_cbox") else ""
        dad.link_type = cfg.get(section, "link_type") if cfg.has_option(section, "link_type") else cons.LINK_TYPE_WEBS
        dad.show_node_name_header = cfg.getboolean(section, "show_node_name_header") if cfg.has_option(section, "show_node_name_header") else True
        dad.nodes_on_node_name_header = cfg.getint(section, "nodes_on_node_name_header") if cfg.has_option(section, "nodes_on_node_name_header") else NODES_ON_NODE_NAME_HEADER_DEFAULT
        if cfg.has_option(section, "toolbar_icon_size"):
            dad.toolbar_icon_size = cfg.getint(section, "toolbar_icon_size")
            if dad.toolbar_icon_size not in ICONS_SIZE: dad.toolbar_icon_size = 1
        else: dad.toolbar_icon_size = 1
        dad.curr_colors = {
            'f':gtk.gdk.color_parse(cfg.get(section, "fg")) if cfg.has_option(section, "fg") else None,
            'b':gtk.gdk.color_parse(cfg.get(section, "bg")) if cfg.has_option(section, "bg") else None,
            'n':gtk.gdk.color_parse(cfg.get(section, "nn")) if cfg.has_option(section, "nn") else None}

        section = "tree"
        dad.rest_exp_coll = cfg.getint(section, "rest_exp_coll") if cfg.has_option(section, "rest_exp_coll") else 0
        if not hasattr(dad, "expanded_collapsed_string"):
            dad.expanded_collapsed_string = cfg.get(section, "expanded_collapsed_string") if cfg.has_option(section, "expanded_collapsed_string") else ""
        if not hasattr(dad, "expcollnam1"):
            dad.expcollnam1 = unicode(cfg.get(section, "expcollnam1"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "expcollnam1") else ""
            dad.expcollstr1 = cfg.get(section, "expcollstr1") if cfg.has_option(section, "expcollstr1") else ""
            dad.expcollsel1 = cfg.get(section, "expcollsel1") if cfg.has_option(section, "expcollsel1") else ""
            dad.expcollcur1 = cfg.getint(section, "expcollcur1") if cfg.has_option(section, "expcollcur1") else 0
        if not hasattr(dad, "expcollnam2"):
            dad.expcollnam2 = unicode(cfg.get(section, "expcollnam2"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "expcollnam2") else ""
            dad.expcollstr2 = cfg.get(section, "expcollstr2") if cfg.has_option(section, "expcollstr2") else ""
            dad.expcollsel2 = cfg.get(section, "expcollsel2") if cfg.has_option(section, "expcollsel2") else ""
            dad.expcollcur2 = cfg.getint(section, "expcollcur2") if cfg.has_option(section, "expcollcur2") else 0
        if not hasattr(dad, "expcollnam3"):
            dad.expcollnam3 = unicode(cfg.get(section, "expcollnam3"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "expcollnam3") else ""
            dad.expcollstr3 = cfg.get(section, "expcollstr3") if cfg.has_option(section, "expcollstr3") else ""
            dad.expcollsel3 = cfg.get(section, "expcollsel3") if cfg.has_option(section, "expcollsel3") else ""
            dad.expcollcur3 = cfg.getint(section, "expcollcur3") if cfg.has_option(section, "expcollcur3") else 0
        dad.nodes_bookm_exp = cfg.getboolean(section, "nodes_bookm_exp") if cfg.has_option(section, "nodes_bookm_exp") else False
        dad.nodes_icons = cfg.get(section, "nodes_icons") if cfg.has_option(section, "nodes_icons") else "c"
        dad.aux_icon_hide = cfg.getboolean(section, "aux_icon_hide") if cfg.has_option(section, "aux_icon_hide") else False
        dad.default_icon_text = cfg.getint(section, "default_icon_text") if cfg.has_option(section, "default_icon_text") else cons.NODE_ICON_BULLET_ID
        dad.tree_right_side = cfg.getboolean(section, "tree_right_side") if cfg.has_option(section, "tree_right_side") else False
        dad.cherry_wrap_width = cfg.getint(section, "cherry_wrap_width") if cfg.has_option(section, "cherry_wrap_width") else 130
        dad.tree_click_focus_text = cfg.getboolean(section, "tree_click_focus_text") if cfg.has_option(section, "tree_click_focus_text") else False
        dad.tree_click_expand = cfg.getboolean(section, "tree_click_expand") if cfg.has_option(section, "tree_click_expand") else False

        section = "editor"
        dad.syntax_highlighting = cfg.get(section, "syntax_highlighting") if cfg.has_option(section, "syntax_highlighting") else cons.RICH_TEXT_ID
        dad.auto_syn_highl = cfg.get(section, "auto_syn_highl") if cfg.has_option(section, "auto_syn_highl") else "sh"
        dad.style_scheme = cfg.get(section, "style_scheme") if cfg.has_option(section, "style_scheme") else cons.STYLE_SCHEME_DARK
        dad.enable_spell_check = cfg.getboolean(section, "enable_spell_check") if cfg.has_option(section, "enable_spell_check") else False
        dad.spell_check_lang = cfg.get(section, "spell_check_lang") if cfg.has_option(section, "spell_check_lang") else SPELL_CHECK_LANG_DEFAULT
        dad.show_line_numbers = cfg.getboolean(section, "show_line_numbers") if cfg.has_option(section, "show_line_numbers") else False
        dad.spaces_instead_tabs = cfg.getboolean(section, "spaces_instead_tabs") if cfg.has_option(section, "spaces_instead_tabs") else True
        dad.tabs_width = cfg.getint(section, "tabs_width") if cfg.has_option(section, "tabs_width") else 4
        dad.anchor_size = cfg.getint(section, "anchor_size") if cfg.has_option(section, "anchor_size") else 16
        dad.embfile_size = cfg.getint(section, "embfile_size") if cfg.has_option(section, "embfile_size") else 48
        dad.embfile_show_filename = cfg.getboolean(section, "embfile_show_filename") if cfg.has_option(section, "embfile_show_filename") else True
        dad.embfile_max_size = cfg.getint(section, "embfile_max_size") if cfg.has_option(section, "embfile_max_size") else MAX_SIZE_EMBFILE_MB_DEFAULT
        dad.line_wrapping = cfg.getboolean(section, "line_wrapping") if cfg.has_option(section, "line_wrapping") else True
        dad.auto_smart_quotes = cfg.getboolean(section, "auto_smart_quotes") if cfg.has_option(section, "auto_smart_quotes") else True
        dad.triple_click_paragraph = cfg.getboolean(section, "triple_click_paragraph") if cfg.has_option(section, "triple_click_paragraph") else True
        dad.enable_symbol_autoreplace = cfg.getboolean(section, "enable_symbol_autoreplace") if cfg.has_option(section, "enable_symbol_autoreplace") else True
        dad.wrapping_indent = cfg.getint(section, "wrapping_indent") if cfg.has_option(section, "wrapping_indent") else -14
        dad.auto_indent = cfg.getboolean(section, "auto_indent") if cfg.has_option(section, "auto_indent") else True
        dad.rt_show_white_spaces = cfg.getboolean(section, "rt_show_white_spaces") if cfg.has_option(section, "rt_show_white_spaces") else False
        dad.pt_show_white_spaces = cfg.getboolean(section, "pt_show_white_spaces") if cfg.has_option(section, "pt_show_white_spaces") else True
        dad.rt_highl_curr_line = cfg.getboolean(section, "rt_highl_curr_line") if cfg.has_option(section, "rt_highl_curr_line") else True
        dad.pt_highl_curr_line = cfg.getboolean(section, "pt_highl_curr_line") if cfg.has_option(section, "pt_highl_curr_line") else True
        dad.space_around_lines = cfg.getint(section, "space_around_lines") if cfg.has_option(section, "space_around_lines") else 0
        dad.relative_wrapped_space = cfg.getint(section, "relative_wrapped_space") if cfg.has_option(section, "relative_wrapped_space") else 50
        dad.h_rule = cfg.get(section, "h_rule") if cfg.has_option(section, "h_rule") else HORIZONTAL_RULE
        dad.special_chars = unicode(cfg.get(section, "special_chars"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "special_chars") else SPECIAL_CHARS_DEFAULT
        dad.selword_chars = unicode(cfg.get(section, "selword_chars"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "selword_chars") else SELWORD_CHARS_DEFAULT
        dad.chars_listbul = unicode(cfg.get(section, "chars_listbul"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "chars_listbul") else CHARS_LISTBUL_DEFAULT
        dad.chars_todo = unicode(cfg.get(section, "chars_todo"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "chars_todo") else CHARS_TODO_DEFAULT
        dad.chars_toc = unicode(cfg.get(section, "chars_toc"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "chars_toc") else CHARS_TOC_DEFAULT
        dad.chars_smart_dquote = unicode(cfg.get(section, "chars_smart_dquote"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "chars_smart_dquote") else CHARS_SMART_DQUOTE_DEFAULT
        dad.chars_smart_squote = unicode(cfg.get(section, "chars_smart_squote"), cons.STR_UTF8, cons.STR_IGNORE) if cfg.has_option(section, "chars_smart_squote") else CHARS_SMART_SQUOTE_DEFAULT
        if cfg.has_option(section, "latest_tag_prop") and cfg.has_option(section, "latest_tag_val"):
            dad.latest_tag[0] = cfg.get(section, "latest_tag_prop")
            dad.latest_tag[1] = cfg.get(section, "latest_tag_val")
        dad.timestamp_format = cfg.get(section, "timestamp_format") if cfg.has_option(section, "timestamp_format") else TIMESTAMP_FORMAT_DEFAULT
        dad.links_underline = cfg.getboolean(section, "links_underline") if cfg.has_option(section, "links_underline") else True
        dad.links_relative = cfg.getboolean(section, "links_relative") if cfg.has_option(section, "links_relative") else False
        if cfg.has_option(section, "weblink_custom_action"):
            temp_str = cfg.get(section, "weblink_custom_action")
            dad.weblink_custom_action = [True, temp_str[4:]] if temp_str[:4] == "True" else [False, temp_str[5:]]
        else: dad.weblink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_WEB]
        if cfg.has_option(section, "filelink_custom_action"):
            temp_str = cfg.get(section, "filelink_custom_action")
            dad.filelink_custom_action = [True, temp_str[4:]] if temp_str[:4] == "True" else [False, temp_str[5:]]
        else: dad.filelink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]
        if cfg.has_option(section, "folderlink_custom_action"):
            temp_str = cfg.get(section, "folderlink_custom_action")
            dad.folderlink_custom_action = [True, temp_str[4:]] if temp_str[:4] == "True" else [False, temp_str[5:]]
        else: dad.folderlink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]

        section = "codebox"
        if cfg.has_option(section, "codebox_width"):
            dad.codebox_width = cfg.getfloat(section, "codebox_width")
        else: dad.codebox_width = 700
        if cfg.has_option(section, "codebox_height"):
            dad.codebox_height = cfg.getfloat(section, "codebox_height")
        else: dad.codebox_height = 100
        dad.codebox_width_pixels = cfg.getboolean(section, "codebox_width_pixels") if cfg.has_option(section, "codebox_width_pixels") else True
        dad.codebox_line_num = cfg.getboolean(section, "codebox_line_num") if cfg.has_option(section, "codebox_line_num") else False
        dad.codebox_match_bra = cfg.getboolean(section, "codebox_match_bra") if cfg.has_option(section, "codebox_match_bra") else True
        dad.codebox_syn_highl = cfg.get(section, "codebox_syn_highl") if cfg.has_option(section, "codebox_syn_highl") else cons.PLAIN_TEXT_ID
        dad.codebox_auto_resize = cfg.getboolean(section, "codebox_auto_resize") if cfg.has_option(section, "codebox_auto_resize") else False

        section = "table"
        dad.table_rows = cfg.getint(section, "table_rows") if cfg.has_option(section, "table_rows") else 3
        dad.table_columns = cfg.getint(section, "table_columns") if cfg.has_option(section, "table_columns") else 3
        dad.table_column_mode = cfg.get(section, "table_column_mode") if cfg.has_option(section, "table_column_mode") else "rename"
        dad.table_col_min = cfg.getint(section, "table_col_min") if cfg.has_option(section, "table_col_min") else 40
        dad.table_col_max = cfg.getint(section, "table_col_max") if cfg.has_option(section, "table_col_max") else 60

        section = "fonts"
        dad.rt_font = cfg.get(section, "rt_font") if cfg.has_option(section, "rt_font") else "Sans 9" # default rich text font
        dad.pt_font = cfg.get(section, "pt_font") if cfg.has_option(section, "pt_font") else "Sans 9" # default plain text font
        dad.tree_font = cfg.get(section, "tree_font") if cfg.has_option(section, "tree_font") else "Sans 8" # default tree font
        dad.code_font = cfg.get(section, "code_font") if cfg.has_option(section, "code_font") else "Monospace 9" # default code font

        section = "colors"
        dad.rt_def_fg = cfg.get(section, "rt_def_fg") if cfg.has_option(section, "rt_def_fg") else cons.RICH_TEXT_DARK_FG
        dad.rt_def_bg = cfg.get(section, "rt_def_bg") if cfg.has_option(section, "rt_def_bg") else cons.RICH_TEXT_DARK_BG
        dad.tt_def_fg = cfg.get(section, "tt_def_fg") if cfg.has_option(section, "tt_def_fg") else cons.TREE_TEXT_LIGHT_FG
        dad.tt_def_bg = cfg.get(section, "tt_def_bg") if cfg.has_option(section, "tt_def_bg") else cons.TREE_TEXT_LIGHT_BG
        dad.monospace_bg = cfg.get(section, "monospace_bg") if cfg.has_option(section, "monospace_bg") else DEFAULT_MONOSPACE_BG
        if cfg.has_option(section, "palette_list"):
            dad.palette_list = cfg.get(section, "palette_list").split(":")
        else: dad.palette_list = COLOR_PALETTE_DEFAULT
        dad.col_link_webs = cfg.get(section, "col_link_webs") if cfg.has_option(section, "col_link_webs") else cons.COLOR_48_LINK_WEBS
        dad.col_link_node = cfg.get(section, "col_link_node") if cfg.has_option(section, "col_link_node") else cons.COLOR_48_LINK_NODE
        dad.col_link_file = cfg.get(section, "col_link_file") if cfg.has_option(section, "col_link_file") else cons.COLOR_48_LINK_FILE
        dad.col_link_fold = cfg.get(section, "col_link_fold") if cfg.has_option(section, "col_link_fold") else cons.COLOR_48_LINK_FOLD

        section = "misc"
        dad.toolbar_ui_list = cfg.get(section, "toolbar_ui_list").split(cons.CHAR_COMMA) if cfg.has_option(section, "toolbar_ui_list") else menus.TOOLBAR_VEC_DEFAULT
        dad.systray = cfg.getboolean(section, "systray") if cfg.has_option(section, "systray") else False
        dad.start_on_systray = cfg.getboolean(section, "start_on_systray") if cfg.has_option(section, "start_on_systray") else False
        dad.use_appind = cfg.getboolean(section, "use_appind") if cfg.has_option(section, "use_appind") else False
        if cfg.has_option(section, "autosave") and cfg.has_option(section, "autosave_val"):
            dad.autosave = [cfg.getboolean(section, "autosave"), cfg.getint(section, "autosave_val")]
        else: dad.autosave = [False, 5]
        dad.check_version = cfg.getboolean(section, "check_version") if cfg.has_option(section, "check_version") else False
        dad.word_count = cfg.getboolean(section, "word_count") if cfg.has_option(section, "word_count") else False
        dad.reload_doc_last = cfg.getboolean(section, "reload_doc_last") if cfg.has_option(section, "reload_doc_last") else True
        dad.enable_mod_time_sentinel = cfg.getboolean(section, "mod_time_sent") if cfg.has_option(section, "mod_time_sent") else False
        dad.backup_copy = cfg.getboolean(section, "backup_copy") if cfg.has_option(section, "backup_copy") else True
        dad.backup_num = cfg.getint(section, "backup_num") if cfg.has_option(section, "backup_num") else 3
        dad.autosave_on_quit = cfg.getboolean(section, "autosave_on_quit") if cfg.has_option(section, "autosave_on_quit") else False
        dad.limit_undoable_steps = cfg.getint(section, "limit_undoable_steps") if cfg.has_option(section, "limit_undoable_steps") else 20
        dad.journal_day_format = cfg.get(section, "journal_day_format") if cfg.has_option(section, "journal_day_format") else JOURNAL_DAY_FORMAT_DEFAULT
        #print "read", cons.CONFIG_PATH, "('%s', '%s')" % (dad.file_name, dad.file_dir)
        section = "keyboard"
        if cfg.has_section(section):
            for option in cfg.options(section):
                value = cfg.get(section, option).strip()
                dad.custom_kb_shortcuts[option] = value if value else None
        section = "codexec_term"
        if cfg.has_section(section):
            if cfg.has_option(section, "custom"):
                dad.custom_codexec_term = cfg.get(section, "custom")
        section = "codexec_type"
        if cfg.has_section(section):
            for option in cfg.options(section):
                dad.custom_codexec_type[option] = cfg.get(section, option)
        section = "codexec_ext"
        if cfg.has_section(section):
            for option in cfg.options(section):
                dad.custom_codexec_ext[option] = cfg.get(section, option)
    else:
        dad.file_dir = ""
        dad.file_name = ""
        dad.node_path = None
        dad.curr_colors = {'f':None, 'b':None, 'n':None}
        dad.syntax_highlighting = cons.RICH_TEXT_ID
        dad.auto_syn_highl = "sh"
        dad.style_scheme = cons.STYLE_SCHEME_DARK
        dad.tree_font = "Sans 8" # default tree font
        dad.rt_font = "Sans 9" # default rich text font
        dad.pt_font = "Sans 9" # default plain text font
        dad.code_font = "Monospace 9" # default code font
        dad.rt_def_fg = cons.RICH_TEXT_DARK_FG
        dad.rt_def_bg = cons.RICH_TEXT_DARK_BG
        dad.tt_def_fg = cons.TREE_TEXT_LIGHT_FG
        dad.tt_def_bg = cons.TREE_TEXT_LIGHT_BG
        dad.palette_list = COLOR_PALETTE_DEFAULT
        dad.col_link_webs = cons.COLOR_48_LINK_WEBS
        dad.col_link_node = cons.COLOR_48_LINK_NODE
        dad.col_link_file = cons.COLOR_48_LINK_FILE
        dad.col_link_fold = cons.COLOR_48_LINK_FOLD
        dad.h_rule = HORIZONTAL_RULE
        dad.special_chars = SPECIAL_CHARS_DEFAULT
        dad.selword_chars = SELWORD_CHARS_DEFAULT
        dad.chars_listbul = CHARS_LISTBUL_DEFAULT
        dad.chars_todo = CHARS_TODO_DEFAULT
        dad.chars_toc = CHARS_TOC_DEFAULT
        dad.chars_smart_dquote = CHARS_SMART_DQUOTE_DEFAULT
        dad.chars_smart_squote = CHARS_SMART_SQUOTE_DEFAULT
        dad.enable_spell_check = False
        dad.spell_check_lang = SPELL_CHECK_LANG_DEFAULT
        dad.show_line_numbers = False
        dad.spaces_instead_tabs = True
        dad.tabs_width = 4
        dad.anchor_size = 16
        dad.embfile_size = 48
        dad.embfile_show_filename = True
        dad.embfile_max_size = MAX_SIZE_EMBFILE_MB_DEFAULT
        dad.line_wrapping = True
        dad.auto_smart_quotes = True
        dad.triple_click_paragraph = True
        dad.enable_symbol_autoreplace = True
        dad.wrapping_indent = -14
        dad.auto_indent = True
        dad.toolbar_ui_list = menus.TOOLBAR_VEC_DEFAULT
        dad.systray = False
        dad.win_position = [10, 10]
        dad.autosave = [False, 5]
        dad.win_is_maximized = False
        dad.rest_exp_coll = 0
        dad.expanded_collapsed_string = ""
        dad.expcollnam1 = ""
        dad.expcollnam2 = ""
        dad.expcollnam3 = ""
        dad.pick_dir_import = ""
        dad.pick_dir_export = ""
        dad.pick_dir_file = ""
        dad.pick_dir_img = ""
        dad.pick_dir_csv = ""
        dad.pick_dir_cbox = ""
        dad.link_type = cons.LINK_TYPE_WEBS
        dad.toolbar_icon_size = 1
        dad.table_rows = 3
        dad.table_columns = 3
        dad.table_column_mode = "rename"
        dad.table_col_min = 40
        dad.table_col_max = 60
        dad.limit_undoable_steps = 20
        dad.cherry_wrap_width = 130
        dad.tree_click_focus_text = False
        dad.tree_click_expand = False
        dad.start_on_systray = False
        dad.use_appind = False
        dad.monospace_bg = DEFAULT_MONOSPACE_BG
        dad.links_underline = True
        dad.links_relative = False
        dad.weblink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_WEB]
        dad.filelink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]
        dad.folderlink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]
        dad.timestamp_format = TIMESTAMP_FORMAT_DEFAULT
        dad.codebox_width = 700
        dad.codebox_height = 100
        dad.codebox_width_pixels = True
        dad.codebox_line_num = False
        dad.codebox_match_bra = True
        dad.codebox_syn_highl = cons.PLAIN_TEXT_ID
        dad.codebox_auto_resize = False
        dad.check_version = False
        dad.word_count = False
        dad.reload_doc_last = True
        dad.enable_mod_time_sentinel = False
        dad.backup_copy = True
        dad.backup_num = 3
        dad.autosave_on_quit = False
        dad.tree_right_side = False
        dad.aux_icon_hide = False
        dad.nodes_bookm_exp = False
        dad.rt_show_white_spaces = False
        dad.pt_show_white_spaces = True
        dad.rt_highl_curr_line = True
        dad.pt_highl_curr_line = True
        dad.space_around_lines = 0
        dad.relative_wrapped_space = 50
        dad.hpaned_pos = 170
        dad.tree_visible = True
        dad.show_node_name_header = True
        dad.nodes_on_node_name_header = NODES_ON_NODE_NAME_HEADER_DEFAULT
        dad.nodes_icons = "c"
        dad.default_icon_text = cons.NODE_ICON_BULLET_ID
        dad.recent_docs = []
        dad.toolbar_visible = True
        dad.journal_day_format = JOURNAL_DAY_FORMAT_DEFAULT
        print "missing", cons.CONFIG_PATH

def config_file_apply(dad):
    """Apply the Preferences from Config File"""
    dad.hpaned.set_property('position', dad.hpaned_pos)
    dad.scrolledwindow_tree.set_property(cons.STR_VISIBLE, dad.tree_visible)
    dad.header_node_name_hbox.set_property(cons.STR_VISIBLE, dad.show_node_name_header)
    dad.update_node_name_header_num_latest_visited()
    dad.set_treeview_font()
    dad.treeview_set_colors()
    if not pgsc_spellcheck.HAS_PYENCHANT:
        dad.enable_spell_check = False
    dad.sourceview.set_show_line_numbers(dad.show_line_numbers)
    dad.sourceview.set_insert_spaces_instead_of_tabs(dad.spaces_instead_tabs)
    dad.sourceview.set_tab_width(dad.tabs_width)
    dad.sourceview.set_indent(dad.wrapping_indent)
    dad.sourceview.set_pixels_above_lines(dad.space_around_lines)
    dad.sourceview.set_pixels_below_lines(dad.space_around_lines)
    dad.sourceview.set_pixels_inside_wrap(get_pixels_inside_wrap(dad.space_around_lines, dad.relative_wrapped_space))
    if dad.line_wrapping: dad.sourceview.set_wrap_mode(gtk.WRAP_WORD_CHAR)
    else: dad.sourceview.set_wrap_mode(gtk.WRAP_NONE)
    dad.renderer_text.set_property('wrap-width', dad.cherry_wrap_width)
    dad.aux_renderer_pixbuf.set_property("visible", not dad.aux_icon_hide)
    dad.ui.get_widget("/ToolBar").set_property(cons.STR_VISIBLE, dad.toolbar_visible)
    dad.ui.get_widget("/ToolBar").set_style(gtk.TOOLBAR_ICONS)
    dad.ui.get_widget("/ToolBar").set_property("icon-size", ICONS_SIZE[dad.toolbar_icon_size])
    if dad.autosave[0]: dad.autosave_timer_start()
    if dad.enable_mod_time_sentinel: dad.modification_time_sentinel_start()
    if dad.curr_tree_iter:
        node_id = dad.get_node_id_from_tree_iter(dad.curr_tree_iter)
        node_is_bookmarked = str(node_id) in dad.bookmarks
        dad.menu_tree_update_for_bookmarked_node(node_is_bookmarked)
    dad.progresstop.hide()
    dad.progressbar.hide()
    dad.header_node_name_icon_lock.hide()
    dad.header_node_name_icon_pin.hide()

def config_file_save(dad):
    """Save the Preferences to Config File"""
    cfg = ConfigParser.RawConfigParser()

    section = "state"
    cfg.add_section(section)
    cfg.set(section, "file_dir", dad.file_dir)
    cfg.set(section, "file_name", dad.file_name)
    cfg.set(section, "toolbar_visible", dad.toolbar_visible)
    cfg.set(section, "win_is_maximized", dad.win_is_maximized)
    dad.win_position = dad.window.get_position()
    cfg.set(section, "win_position_x", dad.win_position[0])
    cfg.set(section, "win_position_y", dad.win_position[1])
    if not dad.win_is_maximized:
        win_size = dad.window.get_size()
        cfg.set(section, "win_size_w", win_size[0])
        cfg.set(section, "win_size_h", win_size[1])
    cfg.set(section, "hpaned_pos", dad.hpaned.get_property('position'))
    cfg.set(section, "tree_visible", dad.tree_visible)
    if dad.curr_tree_iter:
        cfg.set(section, "node_path", get_node_path_str_from_path(dad.treestore.get_path(dad.curr_tree_iter)))
        cfg.set(section, "cursor_position", dad.curr_buffer.get_property(cons.STR_CURSOR_POSITION))
    if dad.recent_docs:
        for i in range(min(len(dad.recent_docs), cons.MAX_RECENT_DOCS)):
            cfg.set(section, "doc_%s" % i, dad.recent_docs[i])
    cfg.set(section, "pick_dir_import", dad.pick_dir_import)
    cfg.set(section, "pick_dir_export", dad.pick_dir_export)
    cfg.set(section, "pick_dir_file", dad.pick_dir_file)
    cfg.set(section, "pick_dir_img", dad.pick_dir_img)
    cfg.set(section, "pick_dir_csv", dad.pick_dir_csv)
    cfg.set(section, "pick_dir_cbox", dad.pick_dir_cbox)
    cfg.set(section, "link_type", dad.link_type)
    cfg.set(section, "show_node_name_header", dad.show_node_name_header)
    cfg.set(section, "nodes_on_node_name_header", dad.nodes_on_node_name_header)
    cfg.set(section, "toolbar_icon_size", dad.toolbar_icon_size)
    if dad.curr_colors['f']: cfg.set(section, "fg", dad.curr_colors['f'].to_string())
    if dad.curr_colors['b']: cfg.set(section, "bg", dad.curr_colors['b'].to_string())
    if dad.curr_colors['n']: cfg.set(section, "nn", dad.curr_colors['n'].to_string())

    section = "tree"
    cfg.add_section(section)
    cfg.set(section, "rest_exp_coll", dad.rest_exp_coll)
    if dad.rest_exp_coll == 0:
        get_tree_expanded_collapsed_string(dad)
        cfg.set(section, "expanded_collapsed_string", dad.expanded_collapsed_string)
    if dad.expcollnam1 and dad.expcollnam1 != dad.file_name:
        cfg.set(section, "expcollnam1", dad.expcollnam1)
        cfg.set(section, "expcollstr1", dad.expcollstr1)
        cfg.set(section, "expcollsel1", dad.expcollsel1)
        cfg.set(section, "expcollcur1", dad.expcollcur1)
    if dad.expcollnam2 and dad.expcollnam2 != dad.file_name:
        cfg.set(section, "expcollnam2", dad.expcollnam2)
        cfg.set(section, "expcollstr2", dad.expcollstr2)
        cfg.set(section, "expcollsel2", dad.expcollsel2)
        cfg.set(section, "expcollcur2", dad.expcollcur2)
    if dad.expcollnam3 and dad.expcollnam3 != dad.file_name:
        cfg.set(section, "expcollnam3", dad.expcollnam3)
        cfg.set(section, "expcollstr3", dad.expcollstr3)
        cfg.set(section, "expcollsel3", dad.expcollsel3)
        cfg.set(section, "expcollcur3", dad.expcollcur3)
    cfg.set(section, "nodes_bookm_exp", dad.nodes_bookm_exp)
    cfg.set(section, "nodes_icons", dad.nodes_icons)
    cfg.set(section, "aux_icon_hide", dad.aux_icon_hide)
    cfg.set(section, "default_icon_text", dad.default_icon_text)
    cfg.set(section, "tree_right_side", dad.tree_right_side)
    cfg.set(section, "cherry_wrap_width", dad.cherry_wrap_width)
    cfg.set(section, "tree_click_focus_text", dad.tree_click_focus_text)
    cfg.set(section, "tree_click_expand", dad.tree_click_expand)

    section = "editor"
    cfg.add_section(section)
    cfg.set(section, "syntax_highlighting", dad.syntax_highlighting)
    cfg.set(section, "auto_syn_highl", dad.auto_syn_highl)
    cfg.set(section, "style_scheme", dad.style_scheme)
    cfg.set(section, "spell_check_lang", dad.spell_check_lang)
    cfg.set(section, "enable_spell_check", dad.enable_spell_check)
    cfg.set(section, "show_line_numbers", dad.show_line_numbers)
    cfg.set(section, "spaces_instead_tabs", dad.spaces_instead_tabs)
    cfg.set(section, "tabs_width", dad.tabs_width)
    cfg.set(section, "anchor_size", dad.anchor_size)
    cfg.set(section, "embfile_size", dad.embfile_size)
    cfg.set(section, "embfile_show_filename", dad.embfile_show_filename)
    cfg.set(section, "embfile_max_size", dad.embfile_max_size)
    cfg.set(section, "line_wrapping", dad.line_wrapping)
    cfg.set(section, "auto_smart_quotes", dad.auto_smart_quotes)
    cfg.set(section, "triple_click_paragraph", dad.triple_click_paragraph)
    cfg.set(section, "enable_symbol_autoreplace", dad.enable_symbol_autoreplace)
    cfg.set(section, "wrapping_indent", dad.wrapping_indent)
    cfg.set(section, "auto_indent", dad.auto_indent)
    cfg.set(section, "rt_show_white_spaces", dad.rt_show_white_spaces)
    cfg.set(section, "pt_show_white_spaces", dad.pt_show_white_spaces)
    cfg.set(section, "rt_highl_curr_line", dad.rt_highl_curr_line)
    cfg.set(section, "pt_highl_curr_line", dad.pt_highl_curr_line)
    cfg.set(section, "space_around_lines", dad.space_around_lines)
    cfg.set(section, "relative_wrapped_space", dad.relative_wrapped_space)
    cfg.set(section, "h_rule", dad.h_rule)
    cfg.set(section, "special_chars", dad.special_chars)
    cfg.set(section, "selword_chars", dad.selword_chars)
    cfg.set(section, "chars_listbul", dad.chars_listbul)
    cfg.set(section, "chars_todo", dad.chars_todo)
    cfg.set(section, "chars_toc", dad.chars_toc)
    cfg.set(section, "chars_smart_dquote", dad.chars_smart_dquote)
    cfg.set(section, "chars_smart_squote", dad.chars_smart_squote)
    cfg.set(section, "latest_tag_prop", dad.latest_tag[0])
    cfg.set(section, "latest_tag_val", dad.latest_tag[1])
    cfg.set(section, "timestamp_format", dad.timestamp_format)
    cfg.set(section, "links_underline", dad.links_underline)
    cfg.set(section, "links_relative", dad.links_relative)
    cfg.set(section, "weblink_custom_action", str(dad.weblink_custom_action[0])+dad.weblink_custom_action[1])
    cfg.set(section, "filelink_custom_action", str(dad.filelink_custom_action[0])+dad.filelink_custom_action[1])
    cfg.set(section, "folderlink_custom_action", str(dad.folderlink_custom_action[0])+dad.folderlink_custom_action[1])

    section = "codebox"
    cfg.add_section(section)
    cfg.set(section, "codebox_width", dad.codebox_width)
    cfg.set(section, "codebox_height", dad.codebox_height)
    cfg.set(section, "codebox_width_pixels", dad.codebox_width_pixels)
    cfg.set(section, "codebox_line_num", dad.codebox_line_num)
    cfg.set(section, "codebox_match_bra", dad.codebox_match_bra)
    cfg.set(section, "codebox_syn_highl", dad.codebox_syn_highl)
    cfg.set(section, "codebox_auto_resize", dad.codebox_auto_resize)

    section = "table"
    cfg.add_section(section)
    cfg.set(section, "table_rows", dad.table_rows)
    cfg.set(section, "table_columns", dad.table_columns)
    cfg.set(section, "table_column_mode", dad.table_column_mode)
    cfg.set(section, "table_col_min", dad.table_col_min)
    cfg.set(section, "table_col_max", dad.table_col_max)

    section = "fonts"
    cfg.add_section(section)
    cfg.set(section, "rt_font", dad.rt_font)
    cfg.set(section, "pt_font", dad.pt_font)
    cfg.set(section, "tree_font", dad.tree_font)
    cfg.set(section, "code_font", dad.code_font)

    section = "colors"
    cfg.add_section(section)
    cfg.set(section, "rt_def_fg", dad.rt_def_fg)
    cfg.set(section, "rt_def_bg", dad.rt_def_bg)
    cfg.set(section, "tt_def_fg", dad.tt_def_fg)
    cfg.set(section, "tt_def_bg", dad.tt_def_bg)
    cfg.set(section, "monospace_bg", dad.monospace_bg)
    cfg.set(section, "palette_list", ":".join(dad.palette_list))
    cfg.set(section, "col_link_webs", dad.col_link_webs)
    cfg.set(section, "col_link_node", dad.col_link_node)
    cfg.set(section, "col_link_file", dad.col_link_file)
    cfg.set(section, "col_link_fold", dad.col_link_fold)

    section = "misc"
    cfg.add_section(section)
    cfg.set(section, "toolbar_ui_list", cons.CHAR_COMMA.join(dad.toolbar_ui_list))
    cfg.set(section, "systray", dad.systray)
    cfg.set(section, "start_on_systray", dad.start_on_systray)
    cfg.set(section, "use_appind", dad.use_appind)
    cfg.set(section, "autosave", dad.autosave[0])
    cfg.set(section, "autosave_val", dad.autosave[1])
    cfg.set(section, "check_version", dad.check_version)
    cfg.set(section, "word_count", dad.word_count)
    cfg.set(section, "reload_doc_last", dad.reload_doc_last)
    cfg.set(section, "mod_time_sent", dad.enable_mod_time_sentinel)
    cfg.set(section, "backup_copy", dad.backup_copy)
    cfg.set(section, "backup_num", dad.backup_num)
    cfg.set(section, "autosave_on_quit", dad.autosave_on_quit)
    cfg.set(section, "limit_undoable_steps", dad.limit_undoable_steps)
    cfg.set(section, "journal_day_format", dad.journal_day_format)

    section = "keyboard"
    cfg.add_section(section)
    for option in dad.custom_kb_shortcuts.keys():
        value = dad.custom_kb_shortcuts[option] if dad.custom_kb_shortcuts[option] else ""
        cfg.set(section, option, value)

    section = "codexec_term"
    cfg.add_section(section)
    if dad.custom_codexec_term:
        cfg.set(section, "custom", dad.custom_codexec_term)

    section = "codexec_type"
    cfg.add_section(section)
    for option in dad.custom_codexec_type.keys():
        cfg.set(section, option, dad.custom_codexec_type[option])

    section = "codexec_ext"
    cfg.add_section(section)
    for option in dad.custom_codexec_ext.keys():
        cfg.set(section, option, dad.custom_codexec_ext[option])

    with open(cons.CONFIG_PATH, 'wb') as fd:
        cfg.write(fd)
        #print "saved", cons.CONFIG_PATH, "('%s', '%s')" % (dad.file_name, dad.file_dir)

def get_tree_expanded_collapsed_string(dad):
    """Returns a String Containing the Info about Expanded and Collapsed Nodes"""
    expanded_collapsed_string = ""
    tree_iter = dad.treestore.get_iter_first()
    while tree_iter != None:
        expanded_collapsed_string += get_tree_expanded_collapsed_string_iter(tree_iter, dad)
        tree_iter = dad.treestore.iter_next(tree_iter)
    if len(expanded_collapsed_string) > 0: dad.expanded_collapsed_string = expanded_collapsed_string[1:]
    else: dad.expanded_collapsed_string = ""

def get_tree_expanded_collapsed_string_iter(tree_iter, dad):
    """Iter of the Info about Expanded and Collapsed Nodes"""
    expanded_collapsed_string = "_%s,%s" % (dad.treestore[tree_iter][3],
                                            dad.treeview.row_expanded(dad.treestore.get_path(tree_iter)))
    tree_iter = dad.treestore.iter_children(tree_iter)
    while tree_iter != None:
        expanded_collapsed_string += get_tree_expanded_collapsed_string_iter(tree_iter, dad)
        tree_iter = dad.treestore.iter_next(tree_iter)
    return expanded_collapsed_string

def set_tree_expanded_collapsed_string(dad, treeview=None):
    """Parses the String Containing the Info about Expanded and Collapsed Nodes"""
    if not treeview: treeview = dad.treeview
    treestore = treeview.get_model()
    treeview.collapse_all()
    expanded_collapsed_dict = {}
    expanded_collapsed_vector = dad.expanded_collapsed_string.split('_')
    for element in expanded_collapsed_vector:
        if "," in element:
            couple = element.split(',')
            expanded_collapsed_dict[couple[0]] = couple[1]
    tree_iter = treestore.get_iter_first()
    while tree_iter != None:
        set_tree_expanded_collapsed_string_iter(dad, tree_iter, expanded_collapsed_dict, treeview, treestore)
        tree_iter = treestore.iter_next(tree_iter)

def set_tree_expanded_collapsed_string_iter(dad, tree_iter, expanded_collapsed_dict, treeview, treestore):
    """Iter of the Expanded and Collapsed Nodes Parsing"""
    node_id = str(treestore[tree_iter][3])
    if node_id in expanded_collapsed_dict and expanded_collapsed_dict[node_id] == "True":
        treeview.expand_row(treestore.get_path(tree_iter), open_all=False)
    elif dad.nodes_bookm_exp and str(node_id) in dad.bookmarks:
        dad.treeview_expand_to_tree_iter(tree_iter)
    tree_iter = treestore.iter_children(tree_iter)
    while tree_iter != None:
        set_tree_expanded_collapsed_string_iter(dad, tree_iter, expanded_collapsed_dict, treeview, treestore)
        tree_iter = treestore.iter_next(tree_iter)

def set_tree_path_and_cursor_pos(dad):
    """Try to set node path and cursor pos"""
    if dad.node_path:
        try: node_iter_to_focus = dad.treestore.get_iter(dad.node_path)
        except: node_iter_to_focus = None
        if node_iter_to_focus:
            dad.treeview_safe_set_cursor(node_iter_to_focus)
            dad.sourceview.grab_focus()
            dad.curr_buffer.place_cursor(dad.curr_buffer.get_iter_at_offset(dad.cursor_position))
            dad.sourceview.scroll_to_mark(dad.curr_buffer.get_insert(), cons.SCROLL_MARGIN)
            if dad.tree_click_expand is True and dad.rest_exp_coll != 2:
                dad.treeview.expand_row(dad.node_path, open_all=False)
    else: node_iter_to_focus = None
    if not node_iter_to_focus:
        node_iter_to_focus = dad.treestore.get_iter_first()
        if node_iter_to_focus:
            dad.treeview.set_cursor(dad.treestore.get_path(node_iter_to_focus))
            dad.sourceview.grab_focus()

def preferences_tab_text_n_code(dad, vbox_all_nodes, pref_dialog):
    """Preferences Dialog, All Nodes Tab"""
    for child in vbox_all_nodes.get_children(): child.destroy()

    hbox_tab_width = gtk.HBox()
    hbox_tab_width.set_spacing(4)
    label_tab_width = gtk.Label(_("Tab Width"))
    adj_tab_width = gtk.Adjustment(value=dad.tabs_width, lower=1, upper=10000, step_incr=1)
    spinbutton_tab_width = gtk.SpinButton(adj_tab_width)
    spinbutton_tab_width.set_value(dad.tabs_width)
    hbox_tab_width.pack_start(label_tab_width, expand=False)
    hbox_tab_width.pack_start(spinbutton_tab_width, expand=False)
    checkbutton_spaces_tabs = gtk.CheckButton(label=_("Insert Spaces Instead of Tabs"))
    checkbutton_spaces_tabs.set_active(dad.spaces_instead_tabs)
    checkbutton_line_wrap = gtk.CheckButton(label=_("Use Line Wrapping"))
    checkbutton_line_wrap.set_active(dad.line_wrapping)
    hbox_wrapping_indent = gtk.HBox()
    hbox_wrapping_indent.set_spacing(4)
    label_wrapping_indent = gtk.Label(_("Line Wrapping Indentation"))
    adj_wrapping_indent = gtk.Adjustment(value=dad.wrapping_indent, lower=-10000, upper=10000, step_incr=1)
    spinbutton_wrapping_indent = gtk.SpinButton(adj_wrapping_indent)
    spinbutton_wrapping_indent.set_value(dad.wrapping_indent)
    hbox_wrapping_indent.pack_start(label_wrapping_indent, expand=False)
    hbox_wrapping_indent.pack_start(spinbutton_wrapping_indent, expand=False)
    checkbutton_auto_indent = gtk.CheckButton(label=_("Enable Automatic Indentation"))
    checkbutton_auto_indent.set_active(dad.auto_indent)
    checkbutton_line_nums = gtk.CheckButton(label=_("Show Line Numbers"))
    checkbutton_line_nums.set_active(dad.show_line_numbers)
    hbox_space_around_lines = gtk.HBox()
    hbox_space_around_lines.set_spacing(4)
    label_space_around_lines = gtk.Label(_("Vertical Space Around Lines"))
    adj_space_around_lines = gtk.Adjustment(value=dad.space_around_lines, lower=-0, upper=255, step_incr=1)
    spinbutton_space_around_lines = gtk.SpinButton(adj_space_around_lines)
    spinbutton_space_around_lines.set_value(dad.space_around_lines)
    hbox_space_around_lines.pack_start(label_space_around_lines, expand=False)
    hbox_space_around_lines.pack_start(spinbutton_space_around_lines, expand=False)
    hbox_relative_wrapped_space = gtk.HBox()
    hbox_relative_wrapped_space.set_spacing(4)
    label_relative_wrapped_space = gtk.Label(_("Vertical Space in Wrapped Lines"))
    adj_relative_wrapped_space = gtk.Adjustment(value=dad.relative_wrapped_space, lower=-0, upper=100, step_incr=1)
    spinbutton_relative_wrapped_space = gtk.SpinButton(adj_relative_wrapped_space)
    spinbutton_relative_wrapped_space.set_value(dad.relative_wrapped_space)
    hbox_relative_wrapped_space.pack_start(label_relative_wrapped_space, expand=False)
    hbox_relative_wrapped_space.pack_start(spinbutton_relative_wrapped_space, expand=False)
    hbox_relative_wrapped_space.pack_start(gtk.Label("%"), expand=False)

    vbox_text_editor = gtk.VBox()
    vbox_text_editor.pack_start(hbox_tab_width, expand=False)
    vbox_text_editor.pack_start(checkbutton_spaces_tabs, expand=False)
    vbox_text_editor.pack_start(checkbutton_line_wrap, expand=False)
    vbox_text_editor.pack_start(hbox_wrapping_indent, expand=False)
    vbox_text_editor.pack_start(checkbutton_auto_indent, expand=False)
    vbox_text_editor.pack_start(checkbutton_line_nums, expand=False)
    vbox_text_editor.pack_start(hbox_space_around_lines, expand=False)
    vbox_text_editor.pack_start(hbox_relative_wrapped_space, expand=False)
    frame_text_editor = gtk.Frame(label="<b>"+_("Text Editor")+"</b>")
    frame_text_editor.get_label_widget().set_use_markup(True)
    frame_text_editor.set_shadow_type(gtk.SHADOW_NONE)
    align_text_editor = gtk.Alignment()
    align_text_editor.set_padding(3, 6, 6, 6)
    align_text_editor.add(vbox_text_editor)
    frame_text_editor.add(align_text_editor)

    hbox_timestamp = gtk.HBox()
    hbox_timestamp.set_spacing(4)
    label_timestamp = gtk.Label(_("Timestamp Format"))
    entry_timestamp_format = gtk.Entry()
    entry_timestamp_format.set_text(dad.timestamp_format)
    button_strftime_help = gtk.Button()
    button_strftime_help.set_image(gtk.image_new_from_stock(gtk.STOCK_HELP, gtk.ICON_SIZE_BUTTON))
    hbox_timestamp.pack_start(label_timestamp, expand=False)
    hbox_timestamp.pack_start(entry_timestamp_format)
    hbox_timestamp.pack_start(button_strftime_help, expand=False)
    hbox_horizontal_rule = gtk.HBox()
    hbox_horizontal_rule.set_spacing(4)
    label_horizontal_rule = gtk.Label(_("Horizontal Rule"))
    entry_horizontal_rule = gtk.Entry()
    entry_horizontal_rule.set_text(dad.h_rule)
    hbox_horizontal_rule.pack_start(label_horizontal_rule, expand=False)
    hbox_horizontal_rule.pack_start(entry_horizontal_rule)
    hbox_special_chars = gtk.HBox()
    hbox_special_chars.set_spacing(4)
    vbox_special_chars = gtk.VBox()
    label_special_chars = gtk.Label(_("Special Characters"))
    hbox_reset = gtk.HBox()
    button_reset = gtk.Button()
    button_reset.set_image(gtk.image_new_from_stock(gtk.STOCK_UNDO, gtk.ICON_SIZE_BUTTON))
    button_reset.set_tooltip_text(_("Reset to Default"))
    hbox_reset.pack_start(gtk.Label(), expand=True)
    hbox_reset.pack_start(button_reset, expand=False)
    hbox_reset.pack_start(gtk.Label(), expand=True)
    vbox_special_chars.pack_start(gtk.Label(), expand=False)
    vbox_special_chars.pack_start(label_special_chars, expand=False)
    vbox_special_chars.pack_start(hbox_reset, expand=False)
    vbox_special_chars.pack_start(gtk.Label(), expand=False)
    frame_special_chars = gtk.Frame()
    frame_special_chars.set_size_request(-1, 80)
    frame_special_chars.set_shadow_type(gtk.SHADOW_IN)
    scrolledwindow_special_chars = gtk.ScrolledWindow()
    scrolledwindow_special_chars.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    frame_special_chars.add(scrolledwindow_special_chars)
    textbuffer_special_chars = gtk.TextBuffer()
    textbuffer_special_chars.set_text(dad.special_chars)
    textview_special_chars = gtk.TextView(buffer=textbuffer_special_chars)
    textview_special_chars.set_wrap_mode(gtk.WRAP_CHAR)
    scrolledwindow_special_chars.add(textview_special_chars)
    hbox_special_chars.pack_start(vbox_special_chars, expand=False)
    hbox_special_chars.pack_start(frame_special_chars)
    hbox_selword_chars = gtk.HBox()
    hbox_selword_chars.set_spacing(4)
    label_selword_chars = gtk.Label(_("Chars to Select at Double Click"))
    entry_selword_chars = gtk.Entry()
    entry_selword_chars.set_text(dad.selword_chars)
    hbox_selword_chars.pack_start(label_selword_chars, expand=False)
    hbox_selword_chars.pack_start(entry_selword_chars)

    vbox_misc_all = gtk.VBox()
    vbox_misc_all.set_spacing(2)
    vbox_misc_all.pack_start(hbox_timestamp)
    vbox_misc_all.pack_start(hbox_horizontal_rule)
    vbox_misc_all.pack_start(hbox_special_chars)
    vbox_misc_all.pack_start(hbox_selword_chars)
    frame_misc_all = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_all.get_label_widget().set_use_markup(True)
    frame_misc_all.set_shadow_type(gtk.SHADOW_NONE)
    align_misc_all = gtk.Alignment()
    align_misc_all.set_padding(3, 6, 6, 6)
    align_misc_all.add(vbox_misc_all)
    frame_misc_all.add(align_misc_all)

    vbox_all_nodes.pack_start(frame_text_editor, expand=False)
    vbox_all_nodes.pack_start(frame_misc_all, expand=False)

    def on_textbuffer_special_chars_changed(textbuffer, *args):
        new_special_chars = unicode(textbuffer.get_text(*textbuffer.get_bounds()).replace(cons.CHAR_NEWLINE, ""), cons.STR_UTF8, cons.STR_IGNORE)
        if dad.special_chars != new_special_chars:
            dad.special_chars = new_special_chars
            support.set_menu_items_special_chars(dad)
    textbuffer_special_chars.connect('changed', on_textbuffer_special_chars_changed)
    def on_button_reset_clicked(button):
        warning_label = "<b>"+_("Are you sure to Reset to Default?")+"</b>"
        response = support.dialog_question_warning(pref_dialog, warning_label)
        if response == gtk.RESPONSE_ACCEPT:
            textbuffer_special_chars.set_text(SPECIAL_CHARS_DEFAULT)
    button_reset.connect('clicked', on_button_reset_clicked)
    def on_spinbutton_tab_width_value_changed(spinbutton):
        dad.tabs_width = int(spinbutton.get_value())
        dad.sourceview.set_tab_width(dad.tabs_width)
    spinbutton_tab_width.connect('value-changed', on_spinbutton_tab_width_value_changed)
    def on_spinbutton_wrapping_indent_value_changed(spinbutton):
        dad.wrapping_indent = int(spinbutton.get_value())
        dad.sourceview.set_indent(dad.wrapping_indent)
    spinbutton_wrapping_indent.connect('value-changed', on_spinbutton_wrapping_indent_value_changed)
    def on_spinbutton_relative_wrapped_space_value_changed(spinbutton):
        dad.relative_wrapped_space = int(spinbutton.get_value())
        dad.sourceview.set_pixels_inside_wrap(get_pixels_inside_wrap(dad.space_around_lines, dad.relative_wrapped_space))
    spinbutton_relative_wrapped_space.connect('value-changed', on_spinbutton_relative_wrapped_space_value_changed)
    def on_spinbutton_space_around_lines_value_changed(spinbutton):
        dad.space_around_lines = int(spinbutton.get_value())
        dad.sourceview.set_pixels_above_lines(dad.space_around_lines)
        dad.sourceview.set_pixels_below_lines(dad.space_around_lines)
        on_spinbutton_relative_wrapped_space_value_changed(spinbutton_relative_wrapped_space)
    spinbutton_space_around_lines.connect('value-changed', on_spinbutton_space_around_lines_value_changed)
    def on_checkbutton_spaces_tabs_toggled(checkbutton):
        dad.spaces_instead_tabs = checkbutton.get_active()
        dad.sourceview.set_insert_spaces_instead_of_tabs(dad.spaces_instead_tabs)
    checkbutton_spaces_tabs.connect('toggled', on_checkbutton_spaces_tabs_toggled)
    def on_checkbutton_line_wrap_toggled(checkbutton):
        dad.line_wrapping = checkbutton.get_active()
        dad.sourceview.set_wrap_mode(gtk.WRAP_WORD_CHAR if dad.line_wrapping else gtk.WRAP_NONE)
    checkbutton_line_wrap.connect('toggled', on_checkbutton_line_wrap_toggled)
    def on_checkbutton_auto_indent_toggled(checkbutton):
        dad.auto_indent = checkbutton.get_active()
    checkbutton_auto_indent.connect('toggled', on_checkbutton_auto_indent_toggled)
    def on_checkbutton_line_nums_toggled(checkbutton):
        dad.show_line_numbers = checkbutton.get_active()
        dad.sourceview.set_show_line_numbers(dad.show_line_numbers)
    checkbutton_line_nums.connect('toggled', on_checkbutton_line_nums_toggled)
    def on_entry_timestamp_format_changed(entry):
        dad.timestamp_format = entry.get_text()
    entry_timestamp_format.connect('changed', on_entry_timestamp_format_changed)
    def on_button_strftime_help_clicked(menuitem, data=None):
        webbrowser.open("https://docs.python.org/2/library/time.html#time.strftime")
    button_strftime_help.connect('clicked', on_button_strftime_help_clicked)
    def on_entry_horizontal_rule_changed(entry):
        dad.h_rule = entry.get_text()
    entry_horizontal_rule.connect('changed', on_entry_horizontal_rule_changed)
    def on_entry_selword_chars_changed(entry):
        dad.selword_chars = entry.get_text()
    entry_selword_chars.connect('changed', on_entry_selword_chars_changed)

def preferences_tab_rich_text(dad, vbox_text_nodes, pref_dialog):
    """Preferences Dialog, Text Nodes Tab"""
    for child in vbox_text_nodes.get_children(): child.destroy()

    vbox_spell_check = gtk.VBox()
    checkbutton_spell_check = gtk.CheckButton(label=_("Enable Spell Check"))
    checkbutton_spell_check.set_active(dad.enable_spell_check)
    hbox_spell_check_lang = gtk.HBox()
    hbox_spell_check_lang.set_spacing(4)
    label_spell_check_lang = gtk.Label(_("Spell Check Language"))
    combobox_spell_check_lang = gtk.ComboBox()
    cell = gtk.CellRendererText()
    combobox_spell_check_lang.pack_start(cell, True)
    combobox_spell_check_lang.add_attribute(cell, 'text', 0)
    def set_checkbutton_spell_check_model():
        combobox_spell_check_lang.set_model(dad.spell_check_lang_liststore)
        combobox_spell_check_lang.set_active_iter(dad.get_combobox_iter_from_value(dad.spell_check_lang_liststore, 0, dad.spell_check_lang))
    if dad.spell_check_init: set_checkbutton_spell_check_model()
    hbox_spell_check_lang.pack_start(label_spell_check_lang, expand=False)
    hbox_spell_check_lang.pack_start(combobox_spell_check_lang)
    vbox_spell_check.pack_start(checkbutton_spell_check, expand=False)
    vbox_spell_check.pack_start(hbox_spell_check_lang, expand=False)
    frame_spell_check = gtk.Frame(label="<b>"+_("Spell Check")+"</b>")
    frame_spell_check.get_label_widget().set_use_markup(True)
    frame_spell_check.set_shadow_type(gtk.SHADOW_NONE)
    align_spell_check = gtk.Alignment()
    align_spell_check.set_padding(3, 6, 6, 6)
    align_spell_check.add(vbox_spell_check)
    frame_spell_check.add(align_spell_check)

    vbox_rt_theme = gtk.VBox()

    radiobutton_rt_col_light = gtk.RadioButton(label=_("Light Background, Dark Text"))
    radiobutton_rt_col_dark = gtk.RadioButton(label=_("Dark Background, Light Text"))
    radiobutton_rt_col_dark.set_group(radiobutton_rt_col_light)
    radiobutton_rt_col_custom = gtk.RadioButton(label=_("Custom Background"))
    radiobutton_rt_col_custom.set_group(radiobutton_rt_col_light)
    hbox_rt_col_custom = gtk.HBox()
    hbox_rt_col_custom.set_spacing(4)
    colorbutton_text_bg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.rt_def_bg))
    label_rt_col_custom = gtk.Label(_("and Text"))
    colorbutton_text_fg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.rt_def_fg))
    hbox_rt_col_custom.pack_start(radiobutton_rt_col_custom, expand=False)
    hbox_rt_col_custom.pack_start(colorbutton_text_bg, expand=False)
    hbox_rt_col_custom.pack_start(label_rt_col_custom, expand=False)
    hbox_rt_col_custom.pack_start(colorbutton_text_fg, expand=False)
    checkbutton_monospace_bg = gtk.CheckButton(_("Monospace Background"))
    mono_color = dad.monospace_bg if dad.monospace_bg else DEFAULT_MONOSPACE_BG
    colorbutton_monospace_bg = gtk.ColorButton(color=gtk.gdk.color_parse(mono_color))
    hbox_monospace_bg = gtk.HBox()
    hbox_monospace_bg.set_spacing(4)
    hbox_monospace_bg.pack_start(checkbutton_monospace_bg, expand=False)
    hbox_monospace_bg.pack_start(colorbutton_monospace_bg, expand=False)

    vbox_rt_theme.pack_start(radiobutton_rt_col_light, expand=False)
    vbox_rt_theme.pack_start(radiobutton_rt_col_dark, expand=False)
    vbox_rt_theme.pack_start(hbox_rt_col_custom, expand=False)
    vbox_rt_theme.pack_start(hbox_monospace_bg, expand=False)
    frame_rt_theme = gtk.Frame(label="<b>"+_("Theme")+"</b>")
    frame_rt_theme.get_label_widget().set_use_markup(True)
    frame_rt_theme.set_shadow_type(gtk.SHADOW_NONE)
    align_rt_theme = gtk.Alignment()
    align_rt_theme.set_padding(3, 6, 6, 6)
    align_rt_theme.add(vbox_rt_theme)
    frame_rt_theme.add(align_rt_theme)

    if dad.rt_def_fg == cons.RICH_TEXT_DARK_FG and dad.rt_def_bg == cons.RICH_TEXT_DARK_BG:
        radiobutton_rt_col_dark.set_active(True)
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
    elif dad.rt_def_fg == cons.RICH_TEXT_LIGHT_FG and dad.rt_def_bg == cons.RICH_TEXT_LIGHT_BG:
        radiobutton_rt_col_light.set_active(True)
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
    else: radiobutton_rt_col_custom.set_active(True)
    if dad.monospace_bg:
        checkbutton_monospace_bg.set_active(True)
        colorbutton_monospace_bg.set_sensitive(True)
    else:
        checkbutton_monospace_bg.set_active(False)
        colorbutton_monospace_bg.set_sensitive(False)

    hbox_misc_text = gtk.HBox()
    hbox_misc_text.set_spacing(4)
    checkbutton_rt_show_white_spaces = gtk.CheckButton(_("Show White Spaces"))
    checkbutton_rt_show_white_spaces.set_active(dad.rt_show_white_spaces)
    checkbutton_rt_highl_curr_line = gtk.CheckButton(_("Highlight Current Line"))
    checkbutton_rt_highl_curr_line.set_active(dad.rt_highl_curr_line)
    checkbutton_codebox_auto_resize = gtk.CheckButton(_("Expand CodeBoxes Automatically"))
    checkbutton_codebox_auto_resize.set_active(dad.codebox_auto_resize)
    hbox_embfile_size = gtk.HBox()
    hbox_embfile_size.set_spacing(4)
    label_embfile_size = gtk.Label(_("Embedded File Icon Size"))
    adj_embfile_size = gtk.Adjustment(value=dad.embfile_size, lower=1, upper=1000, step_incr=1)
    spinbutton_embfile_size = gtk.SpinButton(adj_embfile_size)
    spinbutton_embfile_size.set_value(dad.embfile_size)
    hbox_embfile_size.pack_start(label_embfile_size, expand=False)
    hbox_embfile_size.pack_start(spinbutton_embfile_size, expand=False)
    checkbutton_embfile_show_filename = gtk.CheckButton(_("Show File Name on Top of Embedded File Icon"))
    checkbutton_embfile_show_filename.set_active(dad.embfile_show_filename)
    label_limit_undoable_steps = gtk.Label(_("Limit of Undoable Steps Per Node"))
    adj_limit_undoable_steps = gtk.Adjustment(value=dad.limit_undoable_steps, lower=1, upper=10000, step_incr=1)
    spinbutton_limit_undoable_steps = gtk.SpinButton(adj_limit_undoable_steps)
    spinbutton_limit_undoable_steps.set_value(dad.limit_undoable_steps)
    hbox_misc_text.pack_start(label_limit_undoable_steps, expand=False)
    hbox_misc_text.pack_start(spinbutton_limit_undoable_steps, expand=False)

    vbox_misc_text = gtk.VBox()
    vbox_misc_text.pack_start(checkbutton_rt_show_white_spaces, expand=False)
    vbox_misc_text.pack_start(checkbutton_rt_highl_curr_line, expand=False)
    vbox_misc_text.pack_start(checkbutton_codebox_auto_resize, expand=False)
    vbox_misc_text.pack_start(hbox_embfile_size, expand=False)
    vbox_misc_text.pack_start(checkbutton_embfile_show_filename, expand=False)
    vbox_misc_text.pack_start(hbox_misc_text, expand=False)
    frame_misc_text = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_text.get_label_widget().set_use_markup(True)
    frame_misc_text.set_shadow_type(gtk.SHADOW_NONE)
    align_misc_text = gtk.Alignment()
    align_misc_text.set_padding(3, 6, 6, 6)
    align_misc_text.add(vbox_misc_text)
    frame_misc_text.add(align_misc_text)

    vbox_text_nodes.pack_start(frame_spell_check, expand=False)
    vbox_text_nodes.pack_start(frame_rt_theme, expand=False)
    vbox_text_nodes.pack_start(frame_misc_text, expand=False)
    def on_checkbutton_spell_check_toggled(checkbutton):
        dad.enable_spell_check = checkbutton.get_active()
        if dad.enable_spell_check:
            dad.spell_check_set_on()
            set_checkbutton_spell_check_model()
        else: dad.spell_check_set_off(True)
        combobox_spell_check_lang.set_sensitive(dad.enable_spell_check)
    checkbutton_spell_check.connect('toggled', on_checkbutton_spell_check_toggled)
    def on_combobox_spell_check_lang_changed(combobox):
        new_iter = combobox.get_active_iter()
        new_lang_code = dad.spell_check_lang_liststore[new_iter][0]
        if new_lang_code != dad.spell_check_lang: dad.spell_check_set_new_lang(new_lang_code)
    combobox_spell_check_lang.connect('changed', on_combobox_spell_check_lang_changed)
    def on_colorbutton_text_fg_color_set(colorbutton):
        dad.rt_def_fg = "#" + exports.rgb_any_to_24(colorbutton.get_color().to_string()[1:])
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.RICH_TEXT_ID:
            dad.widget_set_colors(dad.sourceview, dad.rt_def_fg, dad.rt_def_bg, False)
            support.rich_text_node_modify_codeboxes_color(dad.curr_buffer.get_start_iter(), dad)
    colorbutton_text_fg.connect('color-set', on_colorbutton_text_fg_color_set)
    def on_colorbutton_text_bg_color_set(colorbutton):
        dad.rt_def_bg = "#" + exports.rgb_any_to_24(colorbutton.get_color().to_string()[1:])
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.RICH_TEXT_ID:
            if dad.rt_highl_curr_line:
                dad.set_sourcebuffer_with_style_scheme()
                dad.sourceview.set_buffer(dad.curr_buffer)
                dad.objects_buffer_refresh()
            dad.widget_set_colors(dad.sourceview, dad.rt_def_fg, dad.rt_def_bg, False)
            support.rich_text_node_modify_codeboxes_color(dad.curr_buffer.get_start_iter(), dad)
    colorbutton_text_bg.connect('color-set', on_colorbutton_text_bg_color_set)
    def on_radiobutton_rt_col_light_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_text_fg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_LIGHT_FG))
        colorbutton_text_bg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_LIGHT_BG))
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
        on_colorbutton_text_fg_color_set(colorbutton_text_fg)
        on_colorbutton_text_bg_color_set(colorbutton_text_bg)
    radiobutton_rt_col_light.connect('toggled', on_radiobutton_rt_col_light_toggled)
    def on_radiobutton_rt_col_dark_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_text_fg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_DARK_FG))
        colorbutton_text_bg.set_color(gtk.gdk.color_parse(cons.RICH_TEXT_DARK_BG))
        colorbutton_text_fg.set_sensitive(False)
        colorbutton_text_bg.set_sensitive(False)
        on_colorbutton_text_fg_color_set(colorbutton_text_fg)
        on_colorbutton_text_bg_color_set(colorbutton_text_bg)
    radiobutton_rt_col_dark.connect('toggled', on_radiobutton_rt_col_dark_toggled)
    def on_radiobutton_rt_col_custom_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_text_fg.set_sensitive(True)
        colorbutton_text_bg.set_sensitive(True)
    radiobutton_rt_col_custom.connect('toggled', on_radiobutton_rt_col_custom_toggled)
    def on_checkbutton_monospace_bg_toggled(checkbutton):
        if checkbutton.get_active():
            dad.monospace_bg = "#" + exports.rgb_any_to_24(colorbutton_monospace_bg.get_color().to_string()[1:])
            colorbutton_monospace_bg.set_sensitive(True)
        else:
            dad.monospace_bg = ""
            colorbutton_monospace_bg.set_sensitive(False)
        if not pref_dialog.disp_dialog_after_restart:
            pref_dialog.disp_dialog_after_restart = True
            support.dialog_info_after_restart(pref_dialog)
    checkbutton_monospace_bg.connect('toggled', on_checkbutton_monospace_bg_toggled)
    def on_colorbutton_monospace_bg_color_set(colorbutton):
        dad.monospace_bg = "#" + exports.rgb_any_to_24(colorbutton.get_color().to_string()[1:])
        if not pref_dialog.disp_dialog_after_restart:
            pref_dialog.disp_dialog_after_restart = True
            support.dialog_info_after_restart(pref_dialog)
    colorbutton_monospace_bg.connect('color-set', on_colorbutton_monospace_bg_color_set)
    def on_checkbutton_rt_show_white_spaces_toggled(checkbutton):
        dad.rt_show_white_spaces = checkbutton.get_active()
        if dad.syntax_highlighting == cons.RICH_TEXT_ID:
            dad.sourceview.set_draw_spaces(codeboxes.DRAW_SPACES_FLAGS if dad.rt_show_white_spaces else 0)
    checkbutton_rt_show_white_spaces.connect('toggled', on_checkbutton_rt_show_white_spaces_toggled)
    def on_checkbutton_rt_highl_curr_line_toggled(checkbutton):
        dad.rt_highl_curr_line = checkbutton.get_active()
        if dad.syntax_highlighting == cons.RICH_TEXT_ID:
            dad.sourceview.set_highlight_current_line(dad.rt_highl_curr_line)
    checkbutton_rt_highl_curr_line.connect('toggled', on_checkbutton_rt_highl_curr_line_toggled)
    def on_checkbutton_codebox_auto_resize_toggled(checkbutton):
        dad.codebox_auto_resize = checkbutton.get_active()
    checkbutton_codebox_auto_resize.connect('toggled', on_checkbutton_codebox_auto_resize_toggled)
    def on_spinbutton_embfile_size_value_changed(spinbutton):
        dad.embfile_size = int(spinbutton_embfile_size.get_value())
        if not dad.embfile_size_mod:
            dad.embfile_size_mod = True
            support.dialog_info_after_restart(pref_dialog)
    spinbutton_embfile_size.connect('value-changed', on_spinbutton_embfile_size_value_changed)
    def on_checkbutton_embfile_show_filename_toggled(checkbutton):
        dad.embfile_show_filename = checkbutton.get_active()
        if not dad.embfile_show_filename_mod:
            dad.embfile_show_filename_mod = True
            support.dialog_info_after_restart(pref_dialog)
    checkbutton_embfile_show_filename.connect('toggled', on_checkbutton_embfile_show_filename_toggled)
    def on_spinbutton_limit_undoable_steps_value_changed(spinbutton):
        dad.limit_undoable_steps = int(spinbutton.get_value())
    spinbutton_limit_undoable_steps.connect('value-changed', on_spinbutton_limit_undoable_steps_value_changed)

    if not pgsc_spellcheck.HAS_PYENCHANT:
        checkbutton_spell_check.set_sensitive(False)
        combobox_spell_check_lang.set_sensitive(False)

def preferences_tab_text(dad, vbox_text, pref_dialog):
    """Preferences Dialog, Plain Text and Rich Text Tab"""
    for child in vbox_text.get_children(): child.destroy()

    vbox_editor = gtk.VBox()
    checkbutton_auto_smart_quotes = gtk.CheckButton(_("Enable Smart Quotes Auto Replacement"))
    checkbutton_auto_smart_quotes.set_active(dad.auto_smart_quotes)

    checkbutton_enable_symbol_autoreplace = gtk.CheckButton(_("Enable Symbol Auto Replacement"))
    checkbutton_enable_symbol_autoreplace.set_active(dad.enable_symbol_autoreplace)

    vbox_editor.pack_start(checkbutton_auto_smart_quotes, expand=False)
    vbox_editor.pack_start(checkbutton_enable_symbol_autoreplace, expand=False)

    frame_editor = gtk.Frame(label="<b>"+_("Text Editor")+"</b>")
    frame_editor.get_label_widget().set_use_markup(True)
    frame_editor.set_shadow_type(gtk.SHADOW_NONE)
    align_editor = gtk.Alignment()
    align_editor.set_padding(3, 6, 6, 6)
    align_editor.add(vbox_editor)
    frame_editor.add(align_editor)

    vbox_text.pack_start(frame_editor, expand=False)
    def on_checkbutton_auto_smart_quotes_toggled(checkbutton):
        dad.auto_smart_quotes = checkbutton.get_active()
    def on_checkbutton_enable_symbol_autoreplace_toggled(checkbutton):
        dad.enable_symbol_autoreplace = checkbutton.get_active()
    checkbutton_auto_smart_quotes.connect('toggled', on_checkbutton_auto_smart_quotes_toggled)
    checkbutton_enable_symbol_autoreplace.connect('toggled', on_checkbutton_enable_symbol_autoreplace_toggled)

def preferences_tab_plain_text_n_code(dad, vbox_code_nodes, pref_dialog):
    """Preferences Dialog, Plain Text and Code Tab"""
    for child in vbox_code_nodes.get_children(): child.destroy()

    vbox_syntax = gtk.VBox()
    hbox_style_scheme = gtk.HBox()
    hbox_style_scheme.set_spacing(4)
    label_style_scheme = gtk.Label(_("Style Scheme"))
    combobox_style_scheme = gtk.ComboBox(model=dad.style_scheme_liststore)
    cell = gtk.CellRendererText()
    combobox_style_scheme.pack_start(cell, True)
    combobox_style_scheme.add_attribute(cell, 'text', 0)
    combobox_style_scheme.set_active_iter(dad.get_combobox_iter_from_value(dad.style_scheme_liststore, 0, dad.style_scheme))
    hbox_style_scheme.pack_start(label_style_scheme, expand=False)
    hbox_style_scheme.pack_start(combobox_style_scheme)
    checkbutton_pt_show_white_spaces = gtk.CheckButton(_("Show White Spaces"))
    checkbutton_pt_show_white_spaces.set_active(dad.pt_show_white_spaces)
    checkbutton_pt_highl_curr_line = gtk.CheckButton(_("Highlight Current Line"))
    checkbutton_pt_highl_curr_line.set_active(dad.pt_highl_curr_line)

    vbox_syntax.pack_start(hbox_style_scheme, expand=False)
    vbox_syntax.pack_start(checkbutton_pt_show_white_spaces, expand=False)
    vbox_syntax.pack_start(checkbutton_pt_highl_curr_line, expand=False)

    frame_syntax = gtk.Frame(label="<b>"+_("Text Editor")+"</b>")
    frame_syntax.get_label_widget().set_use_markup(True)
    frame_syntax.set_shadow_type(gtk.SHADOW_NONE)
    align_syntax = gtk.Alignment()
    align_syntax.set_padding(3, 6, 6, 6)
    align_syntax.add(vbox_syntax)
    frame_syntax.add(align_syntax)

    vbox_code_nodes.pack_start(frame_syntax, expand=False)
    def on_combobox_style_scheme_changed(combobox):
        new_iter = combobox_style_scheme.get_active_iter()
        new_style = dad.style_scheme_liststore[new_iter][0]
        if new_style != dad.style_scheme:
            dad.style_scheme = new_style
            support.dialog_info_after_restart(pref_dialog)
    combobox_style_scheme.connect('changed', on_combobox_style_scheme_changed)
    def on_checkbutton_pt_show_white_spaces_toggled(checkbutton):
        dad.pt_show_white_spaces = checkbutton.get_active()
        if dad.syntax_highlighting != cons.RICH_TEXT_ID:
            dad.sourceview.set_draw_spaces(codeboxes.DRAW_SPACES_FLAGS if dad.pt_show_white_spaces else 0)
    checkbutton_pt_show_white_spaces.connect('toggled', on_checkbutton_pt_show_white_spaces_toggled)
    def on_checkbutton_pt_highl_curr_line_toggled(checkbutton):
        dad.pt_highl_curr_line = checkbutton.get_active()
        if dad.syntax_highlighting != cons.RICH_TEXT_ID:
            dad.sourceview.set_highlight_current_line(dad.pt_highl_curr_line)
    checkbutton_pt_highl_curr_line.connect('toggled', on_checkbutton_pt_highl_curr_line_toggled)

    liststore = gtk.ListStore(str, str, str)
    treeview = gtk.TreeView(liststore)
    treeview.set_headers_visible(False)
    treeview.set_size_request(300, 200)
    renderer_pixbuf = gtk.CellRendererPixbuf()
    renderer_text_key = gtk.CellRendererText()
    renderer_text_val = gtk.CellRendererText()
    renderer_text_val.set_property('editable', True)
    def on_table_cell_edited(cell, path, new_text):
        if liststore[path][2] != new_text:
            liststore[path][2] = new_text
            key = liststore[path][1]
            dad.custom_codexec_type[key] = new_text
    renderer_text_val.connect('edited', on_table_cell_edited)
    column_key = gtk.TreeViewColumn()
    column_key.pack_start(renderer_pixbuf, False)
    column_key.pack_start(renderer_text_key, True)
    column_key.set_attributes(renderer_pixbuf, stock_id=0)
    column_key.set_attributes(renderer_text_key, text=1)
    column_val = gtk.TreeViewColumn("", renderer_text_val, text=2)
    treeview.append_column(column_key)
    treeview.append_column(column_val)
    treeviewselection = treeview.get_selection()
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    scrolledwindow.add(treeview)

    button_add = gtk.Button()
    button_add.set_image(gtk.image_new_from_stock(gtk.STOCK_ADD, gtk.ICON_SIZE_BUTTON))
    button_add.set_tooltip_text(_("Add"))
    button_reset_cmds = gtk.Button()
    button_reset_cmds.set_image(gtk.image_new_from_stock(gtk.STOCK_UNDO, gtk.ICON_SIZE_BUTTON))
    button_reset_cmds.set_tooltip_text(_("Reset to Default"))
    vbox_buttons = gtk.VBox()
    vbox_buttons.pack_start(button_add, expand=False)
    vbox_buttons.pack_start(gtk.Label(), expand=True)
    vbox_buttons.pack_start(button_reset_cmds, expand=False)

    vbox_codexec = gtk.VBox()
    hbox_term_run = gtk.HBox()
    entry_term_run = gtk.Entry()
    entry_term_run.set_text(get_code_exec_term_run(dad))
    button_reset_term = gtk.Button()
    button_reset_term.set_image(gtk.image_new_from_stock(gtk.STOCK_UNDO, gtk.ICON_SIZE_BUTTON))
    button_reset_term.set_tooltip_text(_("Reset to Default"))
    hbox_term_run.pack_start(entry_term_run, expand=True)
    hbox_term_run.pack_start(button_reset_term, expand=False)
    hbox_cmd_per_type = gtk.HBox()
    hbox_cmd_per_type.pack_start(scrolledwindow, expand=True)
    hbox_cmd_per_type.pack_start(vbox_buttons, expand=False)

    label = gtk.Label("<b>"+_("Command per Node/CodeBox Type")+"</b>")
    label.set_use_markup(True)
    vbox_codexec.pack_start(label, expand=False)
    vbox_codexec.pack_start(hbox_cmd_per_type, expand=True)
    label = gtk.Label("<b>"+_("Terminal Command")+"</b>")
    label.set_use_markup(True)
    vbox_codexec.pack_start(label, expand=False)
    vbox_codexec.pack_start(hbox_term_run, expand=False)

    frame_codexec = gtk.Frame(label="<b>"+_("Code Execution")+"</b>")
    frame_codexec.get_label_widget().set_use_markup(True)
    frame_codexec.set_shadow_type(gtk.SHADOW_NONE)
    align_codexec = gtk.Alignment()
    align_codexec.set_padding(3, 6, 6, 6)
    align_codexec.add(vbox_codexec)
    frame_codexec.add(align_codexec)

    def liststore_append_element(key, val=None):
        stock_id = get_stock_id_for_code_type(key)
        if not val:
            val = get_code_exec_type_cmd(dad, key)
        liststore.append((stock_id, key, val))

    def populate_liststore():
        liststore.clear()
        all_codexec_keys = get_code_exec_type_keys(dad)
        for key in all_codexec_keys:
            liststore_append_element(key)

    vbox_code_nodes.pack_start(frame_codexec, expand=True)
    def on_entry_term_run_changed(entry):
        dad.custom_codexec_term = entry.get_text()
    entry_term_run.connect('changed', on_entry_term_run_changed)
    def on_button_add_clicked(button):
        icon_n_key_list = []
        all_codexec_keys = get_code_exec_type_keys(dad)
        for key in dad.available_languages:
            if not key in all_codexec_keys:
                stock_id = get_stock_id_for_code_type(key)
                icon_n_key_list.append([key, stock_id, key])
        sel_key = support.dialog_choose_element_in_list(dad.window, _("Select Element to Add"), [], "", icon_n_key_list)
        if sel_key:
            default_type_command = "REPLACE_ME %s" % CODE_EXEC_TMP_SRC
            liststore_append_element(sel_key, default_type_command)
            dad.custom_codexec_type[sel_key] = default_type_command
    button_add.connect('clicked', on_button_add_clicked)
    def on_button_reset_cmds_clicked(button, type_str):
        warning_label = "<b>"+_("Are you sure to Reset to Default?")+"</b>"
        response = support.dialog_question_warning(pref_dialog, warning_label)
        if response == gtk.RESPONSE_ACCEPT:
            if type_str == "cmds":
                dad.custom_codexec_type.clear()
                populate_liststore()
            elif type_str == "term":
                dad.custom_codexec_term = None
                entry_term_run.set_text(get_code_exec_term_run(dad))
    button_reset_cmds.connect('clicked', on_button_reset_cmds_clicked, "cmds")
    button_reset_term.connect('clicked', on_button_reset_cmds_clicked, "term")

    populate_liststore()


def preferences_tab_tree_2(dad, vbox_tree, pref_dialog):
    """Preferences Dialog, Tree Tab part 2"""
    for child in vbox_tree.get_children(): child.destroy()

    vbox_misc_tree = gtk.VBox()
    hbox_tree_nodes_names_width = gtk.HBox()
    hbox_tree_nodes_names_width.set_spacing(4)
    label_tree_nodes_names_width = gtk.Label(_("Tree Nodes Names Wrapping Width"))
    adj_tree_nodes_names_width = gtk.Adjustment(value=dad.cherry_wrap_width, lower=10, upper=10000, step_incr=1)
    spinbutton_tree_nodes_names_width = gtk.SpinButton(adj_tree_nodes_names_width)
    spinbutton_tree_nodes_names_width.set_value(dad.cherry_wrap_width)
    hbox_tree_nodes_names_width.pack_start(label_tree_nodes_names_width, expand=False)
    hbox_tree_nodes_names_width.pack_start(spinbutton_tree_nodes_names_width, expand=False)
    checkbutton_tree_right_side = gtk.CheckButton(_("Display Tree on the Right Side"))
    checkbutton_tree_right_side.set_active(dad.tree_right_side)
    checkbutton_tree_click_focus_text = gtk.CheckButton(_("Move Focus to Text at Mouse Click"))
    checkbutton_tree_click_focus_text.set_active(dad.tree_click_focus_text)
    checkbutton_tree_click_expand = gtk.CheckButton(_("Expand Node at Mouse Click"))
    checkbutton_tree_click_expand.set_active(dad.tree_click_expand)
    hbox_nodes_on_node_name_header = gtk.HBox()
    hbox_nodes_on_node_name_header.set_spacing(4)
    label_nodes_on_node_name_header = gtk.Label(_("Last Visited Nodes on Node Name Header"))
    adj_nodes_on_node_name_header = gtk.Adjustment(value=dad.nodes_on_node_name_header, lower=0, upper=100, step_incr=1)
    spinbutton_nodes_on_node_name_header = gtk.SpinButton(adj_nodes_on_node_name_header)
    spinbutton_nodes_on_node_name_header.set_value(dad.nodes_on_node_name_header)
    hbox_nodes_on_node_name_header.pack_start(label_nodes_on_node_name_header, expand=False)
    hbox_nodes_on_node_name_header.pack_start(spinbutton_nodes_on_node_name_header, expand=False)

    vbox_misc_tree.pack_start(hbox_tree_nodes_names_width, expand=False)
    vbox_misc_tree.pack_start(checkbutton_tree_right_side, expand=False)
    vbox_misc_tree.pack_start(checkbutton_tree_click_focus_text, expand=False)
    vbox_misc_tree.pack_start(checkbutton_tree_click_expand, expand=False)
    vbox_misc_tree.pack_start(hbox_nodes_on_node_name_header, expand=False)
    frame_misc_tree = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_tree.get_label_widget().set_use_markup(True)
    frame_misc_tree.set_shadow_type(gtk.SHADOW_NONE)
    align_misc_tree = gtk.Alignment()
    align_misc_tree.set_padding(3, 6, 6, 6)
    align_misc_tree.add(vbox_misc_tree)
    frame_misc_tree.add(align_misc_tree)

    vbox_tree.pack_start(frame_misc_tree, expand=False)
    def on_spinbutton_tree_nodes_names_width_value_changed(spinbutton):
        dad.cherry_wrap_width = int(spinbutton.get_value())
        dad.renderer_text.set_property('wrap-width', dad.cherry_wrap_width)
        dad.treeview_refresh()
    spinbutton_tree_nodes_names_width.connect('value-changed', on_spinbutton_tree_nodes_names_width_value_changed)
    def on_checkbutton_tree_right_side_toggled(checkbutton):
        dad.tree_right_side = checkbutton.get_active()
        tree_width = dad.scrolledwindow_tree.get_allocation().width
        text_width = dad.vbox_text.get_allocation().width
        dad.hpaned.remove(dad.scrolledwindow_tree)
        dad.hpaned.remove(dad.vbox_text)
        if dad.tree_right_side:
            dad.hpaned.add1(dad.vbox_text)
            dad.hpaned.add2(dad.scrolledwindow_tree)
            dad.hpaned.set_property('position', text_width)
        else:
            dad.hpaned.add1(dad.scrolledwindow_tree)
            dad.hpaned.add2(dad.vbox_text)
            dad.hpaned.set_property('position', tree_width)
    checkbutton_tree_right_side.connect('toggled', on_checkbutton_tree_right_side_toggled)
    def on_checkbutton_tree_click_focus_text_toggled(checkbutton):
        dad.tree_click_focus_text = checkbutton.get_active()
    checkbutton_tree_click_focus_text.connect('toggled', on_checkbutton_tree_click_focus_text_toggled)
    def on_checkbutton_tree_click_expand_toggled(checkbutton):
        dad.tree_click_expand = checkbutton.get_active()
    checkbutton_tree_click_expand.connect('toggled', on_checkbutton_tree_click_expand_toggled)
    def on_spinbutton_nodes_on_node_name_header_value_changed(spinbutton):
        dad.nodes_on_node_name_header = int(spinbutton.get_value())
        dad.update_node_name_header_num_latest_visited()
    spinbutton_nodes_on_node_name_header.connect('value-changed', on_spinbutton_nodes_on_node_name_header_value_changed)

def preferences_tab_tree_1(dad, vbox_tree, pref_dialog):
    """Preferences Dialog, Tree Tab part 1"""
    for child in vbox_tree.get_children(): child.destroy()

    vbox_tt_theme = gtk.VBox()

    radiobutton_tt_col_light = gtk.RadioButton(label=_("Light Background, Dark Text"))
    radiobutton_tt_col_dark = gtk.RadioButton(label=_("Dark Background, Light Text"))
    radiobutton_tt_col_dark.set_group(radiobutton_tt_col_light)
    radiobutton_tt_col_custom = gtk.RadioButton(label=_("Custom Background"))
    radiobutton_tt_col_custom.set_group(radiobutton_tt_col_light)
    hbox_tt_col_custom = gtk.HBox()
    hbox_tt_col_custom.set_spacing(4)
    colorbutton_tree_bg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.tt_def_bg))
    label_tt_col_custom = gtk.Label(_("and Text"))
    colorbutton_tree_fg = gtk.ColorButton(color=gtk.gdk.color_parse(dad.tt_def_fg))
    hbox_tt_col_custom.pack_start(radiobutton_tt_col_custom, expand=False)
    hbox_tt_col_custom.pack_start(colorbutton_tree_bg, expand=False)
    hbox_tt_col_custom.pack_start(label_tt_col_custom, expand=False)
    hbox_tt_col_custom.pack_start(colorbutton_tree_fg, expand=False)

    vbox_tt_theme.pack_start(radiobutton_tt_col_light, expand=False)
    vbox_tt_theme.pack_start(radiobutton_tt_col_dark, expand=False)
    vbox_tt_theme.pack_start(hbox_tt_col_custom, expand=False)
    frame_tt_theme = gtk.Frame(label="<b>"+_("Theme")+"</b>")
    frame_tt_theme.get_label_widget().set_use_markup(True)
    frame_tt_theme.set_shadow_type(gtk.SHADOW_NONE)
    align_tt_theme = gtk.Alignment()
    align_tt_theme.set_padding(3, 6, 6, 6)
    align_tt_theme.add(vbox_tt_theme)
    frame_tt_theme.add(align_tt_theme)

    if dad.tt_def_fg == cons.TREE_TEXT_DARK_FG and dad.tt_def_bg == cons.TREE_TEXT_DARK_BG:
        radiobutton_tt_col_dark.set_active(True)
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
    elif dad.tt_def_fg == cons.TREE_TEXT_LIGHT_FG and dad.tt_def_bg == cons.TREE_TEXT_LIGHT_BG:
        radiobutton_tt_col_light.set_active(True)
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
    else: radiobutton_tt_col_custom.set_active(True)

    vbox_nodes_icons = gtk.VBox()

    radiobutton_node_icon_cherry = gtk.RadioButton(label=_("Use Different Cherries per Level"))
    radiobutton_node_icon_custom = gtk.RadioButton(label=_("Use Selected Icon"))
    radiobutton_node_icon_custom.set_group(radiobutton_node_icon_cherry)
    radiobutton_node_icon_none = gtk.RadioButton(label=_("No Icon"))
    radiobutton_node_icon_none.set_group(radiobutton_node_icon_cherry)
    checkbutton_aux_icon_hide = gtk.CheckButton(_("Hide Right Side Auxiliary Icon"))
    checkbutton_aux_icon_hide.set_active(dad.aux_icon_hide)

    c_icon_button = gtk.Button()
    c_icon_button.set_image(gtk.image_new_from_stock(cons.NODES_STOCKS[dad.default_icon_text], gtk.ICON_SIZE_BUTTON))
    c_icon_hbox = gtk.HBox()
    c_icon_hbox.set_spacing(2)
    c_icon_hbox.pack_start(radiobutton_node_icon_custom, expand=False)
    c_icon_hbox.pack_start(c_icon_button, expand=False)

    vbox_nodes_icons.pack_start(radiobutton_node_icon_cherry, expand=False)
    vbox_nodes_icons.pack_start(c_icon_hbox, expand=False)
    vbox_nodes_icons.pack_start(radiobutton_node_icon_none, expand=False)
    vbox_nodes_icons.pack_start(checkbutton_aux_icon_hide, expand=False)
    frame_nodes_icons = gtk.Frame(label="<b>"+_("Default Text Nodes Icons")+"</b>")
    frame_nodes_icons.get_label_widget().set_use_markup(True)
    frame_nodes_icons.set_shadow_type(gtk.SHADOW_NONE)
    align_nodes_icons = gtk.Alignment()
    align_nodes_icons.set_padding(3, 6, 6, 6)
    align_nodes_icons.add(vbox_nodes_icons)
    frame_nodes_icons.add(align_nodes_icons)

    radiobutton_node_icon_cherry.set_active(dad.nodes_icons == "c")
    radiobutton_node_icon_custom.set_active(dad.nodes_icons == "b")
    radiobutton_node_icon_none.set_active(dad.nodes_icons == "n")

    vbox_nodes_startup = gtk.VBox()

    radiobutton_nodes_startup_restore = gtk.RadioButton(label=_("Restore Expanded/Collapsed Status"))
    radiobutton_nodes_startup_expand = gtk.RadioButton(label=_("Expand all Nodes"))
    radiobutton_nodes_startup_expand.set_group(radiobutton_nodes_startup_restore)
    radiobutton_nodes_startup_collapse = gtk.RadioButton(label=_("Collapse all Nodes"))
    radiobutton_nodes_startup_collapse.set_group(radiobutton_nodes_startup_restore)
    checkbutton_nodes_bookm_exp = gtk.CheckButton(_("Nodes in Bookmarks Always Visible"))
    checkbutton_nodes_bookm_exp.set_active(dad.nodes_bookm_exp)
    checkbutton_nodes_bookm_exp.set_sensitive(dad.rest_exp_coll != 1)

    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_restore, expand=False)
    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_expand, expand=False)
    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_collapse, expand=False)
    vbox_nodes_startup.pack_start(checkbutton_nodes_bookm_exp, expand=False)
    frame_nodes_startup = gtk.Frame(label="<b>"+_("Nodes Status at Startup")+"</b>")
    frame_nodes_startup.get_label_widget().set_use_markup(True)
    frame_nodes_startup.set_shadow_type(gtk.SHADOW_NONE)
    align_nodes_startup = gtk.Alignment()
    align_nodes_startup.set_padding(3, 6, 6, 6)
    align_nodes_startup.add(vbox_nodes_startup)
    frame_nodes_startup.add(align_nodes_startup)

    radiobutton_nodes_startup_restore.set_active(dad.rest_exp_coll == 0)
    radiobutton_nodes_startup_expand.set_active(dad.rest_exp_coll == 1)
    radiobutton_nodes_startup_collapse.set_active(dad.rest_exp_coll == 2)

    vbox_tree.pack_start(frame_tt_theme, expand=False)
    vbox_tree.pack_start(frame_nodes_icons, expand=False)
    vbox_tree.pack_start(frame_nodes_startup, expand=False)
    def on_colorbutton_tree_fg_color_set(colorbutton):
        dad.tt_def_fg = "#" + exports.rgb_any_to_24(colorbutton.get_color().to_string()[1:])
        dad.treeview_set_colors()
        if dad.curr_tree_iter: dad.update_node_name_header()
    colorbutton_tree_fg.connect('color-set', on_colorbutton_tree_fg_color_set)
    def on_colorbutton_tree_bg_color_set(colorbutton):
        dad.tt_def_bg = "#" + exports.rgb_any_to_24(colorbutton.get_color().to_string()[1:])
        dad.treeview_set_colors()
        if dad.curr_tree_iter: dad.update_node_name_header()
    colorbutton_tree_bg.connect('color-set', on_colorbutton_tree_bg_color_set)
    def on_radiobutton_tt_col_light_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_tree_fg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_LIGHT_FG))
        colorbutton_tree_bg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_LIGHT_BG))
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
        on_colorbutton_tree_fg_color_set(colorbutton_tree_fg)
        on_colorbutton_tree_bg_color_set(colorbutton_tree_bg)
    radiobutton_tt_col_light.connect('toggled', on_radiobutton_tt_col_light_toggled)
    def on_radiobutton_tt_col_dark_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_tree_fg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_DARK_FG))
        colorbutton_tree_bg.set_color(gtk.gdk.color_parse(cons.TREE_TEXT_DARK_BG))
        colorbutton_tree_fg.set_sensitive(False)
        colorbutton_tree_bg.set_sensitive(False)
        on_colorbutton_tree_fg_color_set(colorbutton_tree_fg)
        on_colorbutton_tree_bg_color_set(colorbutton_tree_bg)
    radiobutton_tt_col_dark.connect('toggled', on_radiobutton_tt_col_dark_toggled)
    def on_radiobutton_tt_col_custom_toggled(radiobutton):
        if not radiobutton.get_active(): return
        colorbutton_tree_fg.set_sensitive(True)
        colorbutton_tree_bg.set_sensitive(True)
    radiobutton_tt_col_custom.connect('toggled', on_radiobutton_tt_col_custom_toggled)
    def on_radiobutton_node_icon_cherry_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "c"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_cherry.connect('toggled', on_radiobutton_node_icon_cherry_toggled)
    def on_radiobutton_node_icon_custom_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "b"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_custom.connect('toggled', on_radiobutton_node_icon_custom_toggled)
    def on_radiobutton_node_icon_none_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "n"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_none.connect('toggled', on_radiobutton_node_icon_none_toggled)
    def on_c_icon_button_clicked(button):
        icon_n_label_list = []
        for key in cons.NODES_STOCKS_KEYS:
            icon_n_label_list.append([str(key), cons.NODES_STOCKS[key], ""])
        sel_key = support.dialog_choose_element_in_list(pref_dialog, _("Select Node Icon"), [], "", icon_n_label_list)
        if sel_key:
            dad.default_icon_text = int(sel_key)
            c_icon_button.set_image(gtk.image_new_from_stock(cons.NODES_STOCKS[dad.default_icon_text], gtk.ICON_SIZE_BUTTON))
            dad.treeview_refresh(change_icon=True)
    c_icon_button.connect('clicked', on_c_icon_button_clicked)
    def on_radiobutton_nodes_startup_restore_toggled(checkbutton):
        if checkbutton.get_active():
            dad.rest_exp_coll = 0
            checkbutton_nodes_bookm_exp.set_sensitive(True)
    radiobutton_nodes_startup_restore.connect('toggled', on_radiobutton_nodes_startup_restore_toggled)
    def on_radiobutton_nodes_startup_expand_toggled(checkbutton):
        if checkbutton.get_active():
            dad.rest_exp_coll = 1
            checkbutton_nodes_bookm_exp.set_sensitive(False)
    radiobutton_nodes_startup_expand.connect('toggled', on_radiobutton_nodes_startup_expand_toggled)
    def on_radiobutton_nodes_startup_collapse_toggled(checkbutton):
        if checkbutton.get_active():
            dad.rest_exp_coll = 2
            checkbutton_nodes_bookm_exp.set_sensitive(True)
    radiobutton_nodes_startup_collapse.connect('toggled', on_radiobutton_nodes_startup_collapse_toggled)
    def on_checkbutton_nodes_bookm_exp_toggled(checkbutton):
        dad.nodes_bookm_exp = checkbutton.get_active()
    checkbutton_nodes_bookm_exp.connect('toggled', on_checkbutton_nodes_bookm_exp_toggled)
    def on_checkbutton_aux_icon_hide_toggled(checkbutton):
        dad.aux_icon_hide = checkbutton.get_active()
        dad.aux_renderer_pixbuf.set_property("visible", not dad.aux_icon_hide)
        dad.treeview_refresh()
    checkbutton_aux_icon_hide.connect('toggled', on_checkbutton_aux_icon_hide_toggled)

def preferences_tab_fonts(dad, vbox_fonts, pref_dialog):
    """Preferences Dialog, Fonts Tab"""
    for child in vbox_fonts.get_children(): child.destroy()

    image_rt = gtk.Image()
    image_rt.set_from_stock(gtk.STOCK_SELECT_FONT, gtk.ICON_SIZE_MENU)
    image_pt = gtk.Image()
    image_pt.set_from_stock(gtk.STOCK_SELECT_FONT, gtk.ICON_SIZE_MENU)
    image_code = gtk.Image()
    image_code.set_from_stock("xml", gtk.ICON_SIZE_MENU)
    image_tree = gtk.Image()
    image_tree.set_from_stock("cherries", gtk.ICON_SIZE_MENU)
    label_rt = gtk.Label(_("Rich Text"))
    label_pt = gtk.Label(_("Plain Text"))
    label_code = gtk.Label(_("Code Font"))
    label_tree = gtk.Label(_("Tree Font"))
    fontbutton_rt = gtk.FontButton(fontname=dad.rt_font)
    fontbutton_pt = gtk.FontButton(fontname=dad.pt_font)
    fontbutton_code = gtk.FontButton(fontname=dad.code_font)
    fontbutton_tree = gtk.FontButton(fontname=dad.tree_font)
    table_fonts = gtk.Table(4, 3)
    table_fonts.set_row_spacings(2)
    table_fonts.set_col_spacings(4)
    table_fonts.attach(image_rt, 0, 1, 0, 1, 0, 0)
    table_fonts.attach(image_pt, 0, 1, 1, 2, 0, 0)
    table_fonts.attach(image_code, 0, 1, 2, 3, 0, 0)
    table_fonts.attach(image_tree, 0, 1, 3, 4, 0, 0)
    table_fonts.attach(label_rt, 1, 2, 0, 1, 0, 0)
    table_fonts.attach(label_pt, 1, 2, 1, 2, 0, 0)
    table_fonts.attach(label_code, 1, 2, 2, 3, 0, 0)
    table_fonts.attach(label_tree, 1, 2, 3, 4, 0, 0)
    table_fonts.attach(fontbutton_rt, 2, 3, 0, 1, yoptions=0)
    table_fonts.attach(fontbutton_pt, 2, 3, 1, 2, yoptions=0)
    table_fonts.attach(fontbutton_code, 2, 3, 2, 3, yoptions=0)
    table_fonts.attach(fontbutton_tree, 2, 3, 3, 4, yoptions=0)

    frame_fonts = gtk.Frame(label="<b>"+_("Fonts")+"</b>")
    frame_fonts.get_label_widget().set_use_markup(True)
    frame_fonts.set_shadow_type(gtk.SHADOW_NONE)
    align_fonts = gtk.Alignment()
    align_fonts.set_padding(3, 6, 6, 6)
    align_fonts.add(table_fonts)
    frame_fonts.add(align_fonts)

    vbox_fonts.pack_start(frame_fonts, expand=False)
    def on_fontbutton_rt_font_set(picker):
        dad.rt_font = picker.get_font_name()
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.RICH_TEXT_ID:
            dad.sourceview.modify_font(pango.FontDescription(dad.rt_font))
    fontbutton_rt.connect('font-set', on_fontbutton_rt_font_set)
    def on_fontbutton_pt_font_set(picker):
        dad.pt_font = picker.get_font_name()
        if not dad.curr_tree_iter: return
        if dad.syntax_highlighting == cons.PLAIN_TEXT_ID:
            dad.sourceview.modify_font(pango.FontDescription(dad.pt_font))
        elif dad.syntax_highlighting == cons.RICH_TEXT_ID:
            support.rich_text_node_modify_codeboxes_font(dad.curr_buffer.get_start_iter(), dad)
            support.rich_text_node_modify_tables_font(dad.curr_buffer.get_start_iter(), dad)
    fontbutton_pt.connect('font-set', on_fontbutton_pt_font_set)
    def on_fontbutton_code_font_set(picker):
        dad.code_font = picker.get_font_name()
        if not dad.curr_tree_iter: return
        if dad.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
            dad.sourceview.modify_font(pango.FontDescription(dad.code_font))
        elif dad.syntax_highlighting == cons.RICH_TEXT_ID:
            support.rich_text_node_modify_codeboxes_font(dad.curr_buffer.get_start_iter(), dad)
    fontbutton_code.connect('font-set', on_fontbutton_code_font_set)
    def on_fontbutton_tree_font_set(picker):
        dad.tree_font = picker.get_font_name()
        dad.set_treeview_font()
    fontbutton_tree.connect('font-set', on_fontbutton_tree_font_set)

def preferences_tab_links(dad, vbox_links, pref_dialog):
    """Preferences Dialog, Links Tab"""
    for child in vbox_links.get_children(): child.destroy()

    vbox_links_actions = gtk.VBox()
    checkbutton_custom_weblink_cmd = gtk.CheckButton(_("Enable Custom Web Link Clicked Action"))
    entry_custom_weblink_cmd = gtk.Entry()
    checkbutton_custom_filelink_cmd = gtk.CheckButton(_("Enable Custom File Link Clicked Action"))
    entry_custom_filelink_cmd = gtk.Entry()
    checkbutton_custom_folderlink_cmd = gtk.CheckButton(_("Enable Custom Folder Link Clicked Action"))
    entry_custom_folderlink_cmd = gtk.Entry()
    vbox_links_actions.pack_start(checkbutton_custom_weblink_cmd, expand=False)
    vbox_links_actions.pack_start(entry_custom_weblink_cmd, expand=False)
    vbox_links_actions.pack_start(checkbutton_custom_filelink_cmd, expand=False)
    vbox_links_actions.pack_start(entry_custom_filelink_cmd, expand=False)
    vbox_links_actions.pack_start(checkbutton_custom_folderlink_cmd, expand=False)
    vbox_links_actions.pack_start(entry_custom_folderlink_cmd, expand=False)

    frame_links_actions = gtk.Frame(label="<b>"+_("Custom Actions")+"</b>")
    frame_links_actions.get_label_widget().set_use_markup(True)
    frame_links_actions.set_shadow_type(gtk.SHADOW_NONE)
    align_links_actions = gtk.Alignment()
    align_links_actions.set_padding(3, 6, 6, 6)
    align_links_actions.add(vbox_links_actions)
    frame_links_actions.add(align_links_actions)

    checkbutton_custom_weblink_cmd.set_active(dad.weblink_custom_action[0])
    entry_custom_weblink_cmd.set_sensitive(dad.weblink_custom_action[0])
    entry_custom_weblink_cmd.set_text(dad.weblink_custom_action[1])
    checkbutton_custom_filelink_cmd.set_active(dad.filelink_custom_action[0])
    entry_custom_filelink_cmd.set_sensitive(dad.filelink_custom_action[0])
    entry_custom_filelink_cmd.set_text(dad.filelink_custom_action[1])
    checkbutton_custom_folderlink_cmd.set_active(dad.folderlink_custom_action[0])
    entry_custom_folderlink_cmd.set_sensitive(dad.folderlink_custom_action[0])
    entry_custom_folderlink_cmd.set_text(dad.folderlink_custom_action[1])

    table_links_colors = gtk.Table(2, 2)
    table_links_colors.set_row_spacings(2)
    table_links_colors.set_col_spacings(4)
    table_links_colors.set_homogeneous(True)

    hbox_col_link_webs = gtk.HBox()
    hbox_col_link_webs.set_spacing(4)
    label_col_link_webs = gtk.Label(_("To WebSite"))
    colorbutton_col_link_webs = gtk.ColorButton(color=gtk.gdk.color_parse(dad.col_link_webs))
    hbox_col_link_webs.pack_start(label_col_link_webs, expand=False)
    hbox_col_link_webs.pack_start(colorbutton_col_link_webs, expand=False)

    hbox_col_link_node = gtk.HBox()
    hbox_col_link_node.set_spacing(4)
    label_col_link_node = gtk.Label(_("To Node"))
    colorbutton_col_link_node = gtk.ColorButton(color=gtk.gdk.color_parse(dad.col_link_node))
    hbox_col_link_node.pack_start(label_col_link_node, expand=False)
    hbox_col_link_node.pack_start(colorbutton_col_link_node, expand=False)

    hbox_col_link_file = gtk.HBox()
    hbox_col_link_file.set_spacing(4)
    label_col_link_file = gtk.Label(_("To File"))
    colorbutton_col_link_file = gtk.ColorButton(color=gtk.gdk.color_parse(dad.col_link_file))
    hbox_col_link_file.pack_start(label_col_link_file, expand=False)
    hbox_col_link_file.pack_start(colorbutton_col_link_file, expand=False)

    hbox_col_link_fold = gtk.HBox()
    hbox_col_link_fold.set_spacing(4)
    label_col_link_fold = gtk.Label(_("To Folder"))
    colorbutton_col_link_fold = gtk.ColorButton(color=gtk.gdk.color_parse(dad.col_link_fold))
    hbox_col_link_fold.pack_start(label_col_link_fold, expand=False)
    hbox_col_link_fold.pack_start(colorbutton_col_link_fold, expand=False)

    table_links_colors.attach(hbox_col_link_webs, 0, 1, 0, 1, 0, 0)
    table_links_colors.attach(hbox_col_link_node, 0, 1, 1, 2, 0, 0)
    table_links_colors.attach(hbox_col_link_file, 1, 2, 0, 1, 0, 0)
    table_links_colors.attach(hbox_col_link_fold, 1, 2, 1, 2, 0, 0)

    frame_links_colors = gtk.Frame(label="<b>"+_("Colors")+"</b>")
    frame_links_colors.get_label_widget().set_use_markup(True)
    frame_links_colors.set_shadow_type(gtk.SHADOW_NONE)
    align_links_colors = gtk.Alignment()
    align_links_colors.set_padding(3, 6, 6, 6)
    align_links_colors.add(table_links_colors)
    frame_links_colors.add(align_links_colors)

    vbox_links_misc = gtk.VBox()
    checkbutton_links_underline = gtk.CheckButton(_("Underline Links"))
    checkbutton_links_underline.set_active(dad.links_underline)
    checkbutton_links_relative = gtk.CheckButton(_("Use Relative Paths for Files And Folders"))
    checkbutton_links_relative.set_active(dad.links_relative)
    hbox_anchor_size = gtk.HBox()
    hbox_anchor_size.set_spacing(4)
    label_anchor_size = gtk.Label(_("Anchor Size"))
    adj_anchor_size = gtk.Adjustment(value=dad.anchor_size, lower=1, upper=1000, step_incr=1)
    spinbutton_anchor_size = gtk.SpinButton(adj_anchor_size)
    spinbutton_anchor_size.set_value(dad.anchor_size)
    hbox_anchor_size.pack_start(label_anchor_size, expand=False)
    hbox_anchor_size.pack_start(spinbutton_anchor_size, expand=False)
    vbox_links_misc.pack_start(checkbutton_links_underline, expand=False)
    vbox_links_misc.pack_start(checkbutton_links_relative, expand=False)
    vbox_links_misc.pack_start(hbox_anchor_size, expand=False)

    frame_links_misc = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_links_misc.get_label_widget().set_use_markup(True)
    frame_links_misc.set_shadow_type(gtk.SHADOW_NONE)
    align_links_misc = gtk.Alignment()
    align_links_misc.set_padding(3, 6, 6, 6)
    align_links_misc.add(vbox_links_misc)
    frame_links_misc.add(align_links_misc)

    vbox_links.pack_start(frame_links_actions, expand=False)
    vbox_links.pack_start(frame_links_colors, expand=False)
    vbox_links.pack_start(frame_links_misc, expand=False)
    def on_checkbutton_custom_weblink_cmd_toggled(checkbutton):
        dad.weblink_custom_action[0] = checkbutton.get_active()
        entry_custom_weblink_cmd.set_sensitive(dad.weblink_custom_action[0])
    checkbutton_custom_weblink_cmd.connect('toggled', on_checkbutton_custom_weblink_cmd_toggled)
    def on_entry_custom_weblink_cmd_changed(entry):
        dad.weblink_custom_action[1] = entry.get_text()
    entry_custom_weblink_cmd.connect('changed', on_entry_custom_weblink_cmd_changed)
    def on_checkbutton_custom_filelink_cmd_toggled(checkbutton):
        dad.filelink_custom_action[0] = checkbutton.get_active()
        entry_custom_filelink_cmd.set_sensitive(dad.filelink_custom_action[0])
    checkbutton_custom_filelink_cmd.connect('toggled', on_checkbutton_custom_filelink_cmd_toggled)
    def on_entry_custom_filelink_cmd_changed(entry):
        dad.filelink_custom_action[1] = entry.get_text()
    entry_custom_filelink_cmd.connect('changed', on_entry_custom_filelink_cmd_changed)
    def on_checkbutton_custom_folderlink_cmd_toggled(checkbutton):
        dad.folderlink_custom_action[0] = checkbutton.get_active()
        entry_custom_folderlink_cmd.set_sensitive(dad.folderlink_custom_action[0])
    checkbutton_custom_folderlink_cmd.connect('toggled', on_checkbutton_custom_folderlink_cmd_toggled)
    def on_entry_custom_folderlink_cmd_changed(entry):
        dad.folderlink_custom_action[1] = entry.get_text()
    entry_custom_folderlink_cmd.connect('changed', on_entry_custom_folderlink_cmd_changed)
    def on_checkbutton_links_relative_toggled(checkbutton):
        dad.links_relative = checkbutton.get_active()
    checkbutton_links_relative.connect('toggled', on_checkbutton_links_relative_toggled)
    def on_checkbutton_links_underline_toggled(checkbutton):
        dad.links_underline = checkbutton.get_active()
        support.dialog_info_after_restart(pref_dialog)
    checkbutton_links_underline.connect('toggled', on_checkbutton_links_underline_toggled)
    def on_spinbutton_anchor_size_value_changed(spinbutton):
        dad.anchor_size = int(spinbutton_anchor_size.get_value())
        if not dad.anchor_size_mod:
            dad.anchor_size_mod = True
            support.dialog_info_after_restart(pref_dialog)
    spinbutton_anchor_size.connect('value-changed', on_spinbutton_anchor_size_value_changed)
    def on_colorbutton_col_link_webs_color_set(colorbutton):
        dad.col_link_webs = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(pref_dialog)
    colorbutton_col_link_webs.connect('color-set', on_colorbutton_col_link_webs_color_set)
    def on_colorbutton_col_link_node_color_set(colorbutton):
        dad.col_link_node = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(pref_dialog)
    colorbutton_col_link_node.connect('color-set', on_colorbutton_col_link_node_color_set)
    def on_colorbutton_col_link_file_color_set(colorbutton):
        dad.col_link_file = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(pref_dialog)
    colorbutton_col_link_file.connect('color-set', on_colorbutton_col_link_file_color_set)
    def on_colorbutton_col_link_fold_color_set(colorbutton):
        dad.col_link_fold = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(pref_dialog)
    colorbutton_col_link_fold.connect('color-set', on_colorbutton_col_link_fold_color_set)

def preferences_tab_kb_shortcuts(dad, vbox_tool, pref_dialog):
    """Preferences Dialog, Keyboard Shortcuts Tab"""
    for child in vbox_tool.get_children(): child.destroy()

    # 0: action_name 1: stock_icon 2: kb_shortcut 3: description
    treestore = gtk.TreeStore(str, str, str, str)
    treeview = gtk.TreeView(treestore)
    treeview.set_headers_visible(False)
    treeview.set_reorderable(True)
    treeview.set_size_request(300, 300)
    renderer_pixbuf = gtk.CellRendererPixbuf()
    renderer_text_kb = gtk.CellRendererText()
    renderer_text_desc = gtk.CellRendererText()
    column = gtk.TreeViewColumn()
    column.pack_start(renderer_pixbuf, False)
    column.pack_start(renderer_text_kb, False)
    column.pack_start(renderer_text_desc, True)
    column.set_attributes(renderer_pixbuf, stock_id=1)
    column.set_attributes(renderer_text_kb, text=2)
    column.set_attributes(renderer_text_desc, text=3)
    treeview.append_column(column)
    treeviewselection = treeview.get_selection()
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.add(treeview)

    vbox_buttons = gtk.VBox()
    button_edit = gtk.Button()
    button_edit.set_image(gtk.image_new_from_stock(gtk.STOCK_EDIT, gtk.ICON_SIZE_BUTTON))
    button_edit.set_tooltip_text(_("Change Selected"))
    button_reset = gtk.Button()
    button_reset.set_image(gtk.image_new_from_stock(gtk.STOCK_UNDO, gtk.ICON_SIZE_BUTTON))
    button_reset.set_tooltip_text(_("Reset to Default"))
    vbox_buttons.pack_start(button_edit, expand=False)
    vbox_buttons.pack_start(gtk.Label(), expand=True)
    vbox_buttons.pack_start(button_reset, expand=False)
    hbox_main = gtk.HBox()
    hbox_main.pack_start(scrolledwindow, expand=True)
    hbox_main.pack_start(vbox_buttons, expand=False)
    vbox_tool.add(hbox_main)

    config_actions_sections = [
["file", _("File")],
["tree", _("Tree")],
["editor", _("Editor")],
["fmt", _("Format")],
["findrepl", _("Find/Replace")],
["view", _("View")],
["export", _("Export")],
["import", _("Import")],
]
    def reload_treestore():
        treestore.clear()
        for section in config_actions_sections:
            tree_iter = treestore.append(None, ["", "", "", section[1]])
            for name in menus.CONFIG_ACTIONS_DICT[section[0]]:
                subdict = dad.menudict[name]
                kb_shortcut = menus.get_menu_item_kb_shortcut(dad, name)
                treestore.append(tree_iter, [name, subdict["sk"], kb_shortcut, subdict["dn"]])
        treeview.expand_all()
    reload_treestore()

    def edit_selected_tree_iter():
        model, tree_iter = treeviewselection.get_selected()
        if not tree_iter: return
        action_name = model[tree_iter][0]
        kb_shortcut_full = model[tree_iter][2] if model[tree_iter][2] else ""
        kb_shortcut_key = kb_shortcut_full.replace(menus.KB_CONTROL, "").replace(menus.KB_SHIFT, "").replace(menus.KB_ALT, "")
        #print "'%s' -> '%s'" % (action_name, kb_shortcut)
        dialog = gtk.Dialog(title=_("Edit Keyboard Shortcut"),
            parent=dad.window,
            flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
            buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
            gtk.STOCK_OK, gtk.RESPONSE_ACCEPT) )
        dialog.set_position(gtk.WIN_POS_CENTER_ON_PARENT)
        dialog.set_default_size(400, 100)
        radiobutton_kb_none = gtk.RadioButton(label=_("No Keyboard Shortcut"))
        radiobutton_kb_shortcut = gtk.RadioButton()
        radiobutton_kb_shortcut.set_group(radiobutton_kb_none)
        ctrl_toggle = gtk.ToggleButton("control")
        shift_toggle = gtk.ToggleButton("shift")
        alt_toggle = gtk.ToggleButton("alt")
        for widget in [ctrl_toggle, shift_toggle, alt_toggle]:
            widget.set_size_request(70, -1)
        key_entry = gtk.Entry()
        key_entry.set_text(kb_shortcut_key)
        vbox = gtk.VBox()
        hbox = gtk.HBox()
        hbox.pack_start(radiobutton_kb_shortcut)
        hbox.pack_start(ctrl_toggle)
        hbox.pack_start(shift_toggle)
        hbox.pack_start(alt_toggle)
        hbox.pack_start(key_entry)
        hbox.set_spacing(5)
        vbox.pack_start(radiobutton_kb_none)
        vbox.pack_start(hbox)
        content_area = dialog.get_content_area()
        content_area.pack_start(vbox)
        def kb_shortcut_on_off(is_on):
            for widget in [ctrl_toggle, shift_toggle, alt_toggle, key_entry]:
                widget.set_sensitive(is_on)
        def on_radiobutton_kb_none_toggled(radiobutton):
            if radiobutton.get_active():
                kb_shortcut_on_off(False)
        radiobutton_kb_none.connect('toggled', on_radiobutton_kb_none_toggled)
        def on_radiobutton_kb_shortcut_toggled(radiobutton):
            if radiobutton.get_active():
                kb_shortcut_on_off(True)
        radiobutton_kb_shortcut.connect('toggled', on_radiobutton_kb_shortcut_toggled)
        def on_key_press_entry(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if len(keyname) == 1: out_txt = keyname.upper()
            else: out_txt = keyname
            key_entry.set_text(out_txt)
            key_entry.select_region(0, len(keyname))
            return True
        key_entry.connect("key_press_event", on_key_press_entry)
        def on_key_press_dialog(widget, event):
            keyname = gtk.gdk.keyval_name(event.keyval)
            if keyname == cons.STR_KEY_RETURN:
                try: dialog.get_widget_for_response(gtk.RESPONSE_ACCEPT).clicked()
                except: print cons.STR_PYGTK_222_REQUIRED
                return True
            return False
        dialog.connect("key_press_event", on_key_press_dialog)
        radiobutton_kb_none.set_active(not bool(kb_shortcut_full))
        radiobutton_kb_shortcut.set_active(bool(kb_shortcut_full))
        ctrl_toggle.set_active(menus.KB_CONTROL in kb_shortcut_full)
        shift_toggle.set_active(menus.KB_SHIFT in kb_shortcut_full)
        alt_toggle.set_active(menus.KB_ALT in kb_shortcut_full)
        kb_shortcut_on_off(bool(kb_shortcut_full))
        content_area.show_all()
        response = dialog.run()
        dialog.hide()
        if response == gtk.RESPONSE_ACCEPT:
            kb_shortcut_full = ""
            kb_shortcut_key = key_entry.get_text().strip()
            if radiobutton_kb_shortcut.get_active() and kb_shortcut_key:
                if ctrl_toggle.get_active(): kb_shortcut_full += menus.KB_CONTROL
                if shift_toggle.get_active(): kb_shortcut_full += menus.KB_SHIFT
                if alt_toggle.get_active(): kb_shortcut_full += menus.KB_ALT
                kb_shortcut_full += kb_shortcut_key
            #print kb_shortcut_full
            if kb_shortcut_full:
                in_use_name = menus.get_menu_item_name_from_shortcut(dad, kb_shortcut_full)
                if in_use_name and in_use_name != action_name:
                    warning_label = "<b>" + _("The Keyboard Shortcut '%s' is already in use") % cgi.escape(kb_shortcut_full) + "</b>\n\n" + _("The current associated action is '%s'") % dad.menudict[in_use_name]["dn"] + "\n\n<b>" + _("Do you want to steal the shortcut?") + "</b>"
                    response = support.dialog_question_warning(pref_dialog, warning_label)
                    if response == gtk.RESPONSE_ACCEPT:
                        dad.custom_kb_shortcuts[in_use_name] = None
                    else:
                        return
                dad.custom_kb_shortcuts[action_name] = kb_shortcut_full
            else:
                dad.custom_kb_shortcuts[action_name] = None
            reload_treestore()
            if not pref_dialog.disp_dialog_after_restart:
                pref_dialog.disp_dialog_after_restart = True
                support.dialog_info_after_restart(pref_dialog)
            menus.polish_overridden_keyboard_shortcuts(dad)

    def on_button_edit_clicked(button):
        edit_selected_tree_iter()
    button_edit.connect('clicked', on_button_edit_clicked)
    def on_button_reset_clicked(button):
        warning_label = "<b>"+_("Are you sure to Reset to Default?")+"</b>"
        response = support.dialog_question_warning(pref_dialog, warning_label)
        if response == gtk.RESPONSE_ACCEPT:
            dad.custom_kb_shortcuts = {}
            reload_treestore()
            support.dialog_info_after_restart(pref_dialog)
    button_reset.connect('clicked', on_button_reset_clicked)
    def on_button_press_treestore(widget, event):
        if event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
            edit_selected_tree_iter()
            return True
        return False
    treeview.connect('button-press-event', on_button_press_treestore)

def preferences_tab_toolbar(dad, vbox_tool, pref_dialog):
    """Preferences Dialog, Toolbar Tab"""
    for child in vbox_tool.get_children(): child.destroy()

    liststore = gtk.ListStore(str, str, str)
    treeview = gtk.TreeView(liststore)
    treeview.set_headers_visible(False)
    treeview.set_reorderable(True)
    treeview.set_size_request(300, 300)
    renderer_pixbuf = gtk.CellRendererPixbuf()
    renderer_text = gtk.CellRendererText()
    column = gtk.TreeViewColumn()
    column.pack_start(renderer_pixbuf, False)
    column.pack_start(renderer_text, True)
    column.set_attributes(renderer_pixbuf, stock_id=1)
    column.set_attributes(renderer_text, text=2)
    treeview.append_column(column)
    treeviewselection = treeview.get_selection()
    scrolledwindow = gtk.ScrolledWindow()
    scrolledwindow.add(treeview)

    button_add = gtk.Button()
    button_add.set_image(gtk.image_new_from_stock(gtk.STOCK_ADD, gtk.ICON_SIZE_BUTTON))
    button_add.set_tooltip_text(_("Add"))
    button_remove = gtk.Button()
    button_remove.set_image(gtk.image_new_from_stock(gtk.STOCK_REMOVE, gtk.ICON_SIZE_BUTTON))
    button_remove.set_tooltip_text(_("Remove Selected"))
    button_reset = gtk.Button()
    button_reset.set_image(gtk.image_new_from_stock(gtk.STOCK_UNDO, gtk.ICON_SIZE_BUTTON))
    button_reset.set_tooltip_text(_("Reset to Default"))

    hbox = gtk.HBox()
    vbox = gtk.VBox()
    vbox.pack_start(button_add, expand=False)
    vbox.pack_start(button_remove, expand=False)
    vbox.pack_start(gtk.Label(), expand=True)
    vbox.pack_start(button_reset, expand=False)
    hbox.pack_start(scrolledwindow, expand=True)
    hbox.pack_start(vbox, expand=False)

    vbox_tool.add(hbox)

    def populate_liststore():
        liststore.clear()
        for element in dad.toolbar_ui_list:
            liststore.append(get_toolbar_entry_columns_from_key(dad, element))
    def update_toolbar_ui_vec():
        dad.toolbar_ui_list = []
        tree_iter = liststore.get_iter_first()
        while tree_iter != None:
            dad.toolbar_ui_list.append(liststore[tree_iter][0])
            tree_iter = liststore.iter_next(tree_iter)
        if not pref_dialog.disp_dialog_after_restart:
            pref_dialog.disp_dialog_after_restart = True
            support.dialog_info_after_restart(pref_dialog)
    def on_button_add_clicked(button):
        icon_n_label_list = get_toolbar_icon_n_label_list(dad)
        sel_key = support.dialog_choose_element_in_list(pref_dialog, _("Select Element to Add"), [], "", icon_n_label_list)
        if sel_key:
            if sel_key == "ct_open_file": sel_key = cons.CHAR_STAR
            model, tree_iter = treeviewselection.get_selected()
            if tree_iter: liststore.insert_after(tree_iter, get_toolbar_entry_columns_from_key(dad, sel_key))
            else: liststore.append(get_toolbar_entry_columns_from_key(dad, sel_key))
            update_toolbar_ui_vec()
    button_add.connect('clicked', on_button_add_clicked)
    def on_button_remove_clicked(button):
        model, tree_iter = treeviewselection.get_selected()
        if tree_iter:
            model.remove(tree_iter)
            update_toolbar_ui_vec()
    button_remove.connect('clicked', on_button_remove_clicked)
    def on_button_reset_clicked(button):
        warning_label = "<b>"+_("Are you sure to Reset to Default?")+"</b>"
        response = support.dialog_question_warning(pref_dialog, warning_label)
        if response == gtk.RESPONSE_ACCEPT:
            dad.toolbar_ui_list = menus.TOOLBAR_VEC_DEFAULT
            populate_liststore()
            support.dialog_info_after_restart(pref_dialog)
    button_reset.connect('clicked', on_button_reset_clicked)
    def on_key_press_liststore(widget, event):
        keyname = gtk.gdk.keyval_name(event.keyval)
        if keyname == cons.STR_KEY_DELETE: on_button_remove_clicked()
    treeview.connect('key_press_event', on_key_press_liststore)
    def on_treeview_drag_end(*args):
        update_toolbar_ui_vec()
    treeview.connect('drag-end', on_treeview_drag_end)
    populate_liststore()

def preferences_tab_misc(dad, vbox_misc, pref_dialog):
    """Preferences Dialog, Misc Tab"""
    for child in vbox_misc.get_children(): child.destroy()

    vbox_system_tray = gtk.VBox()
    checkbutton_systray = gtk.CheckButton(_("Enable System Tray Docking"))
    checkbutton_start_on_systray = gtk.CheckButton(_("Start Minimized in the System Tray"))
    checkbutton_use_appind = gtk.CheckButton(_("Use AppIndicator for Docking"))
    vbox_system_tray.pack_start(checkbutton_systray, expand=False)
    vbox_system_tray.pack_start(checkbutton_start_on_systray, expand=False)
    vbox_system_tray.pack_start(checkbutton_use_appind, expand=False)

    frame_system_tray = gtk.Frame(label="<b>"+_("System Tray")+"</b>")
    frame_system_tray.get_label_widget().set_use_markup(True)
    frame_system_tray.set_shadow_type(gtk.SHADOW_NONE)
    align_system_tray = gtk.Alignment()
    align_system_tray.set_padding(3, 6, 6, 6)
    align_system_tray.add(vbox_system_tray)
    frame_system_tray.add(align_system_tray)

    checkbutton_systray.set_active(dad.systray)
    if cons.DISABLE_SYSTRAY: checkbutton_systray.set_sensitive(False)
    checkbutton_start_on_systray.set_active(dad.start_on_systray)
    checkbutton_start_on_systray.set_sensitive(dad.systray)
    checkbutton_use_appind.set_active(dad.use_appind)
    if not cons.HAS_APPINDICATOR or not cons.HAS_SYSTRAY: checkbutton_use_appind.set_sensitive(False)

    vbox_saving = gtk.VBox()
    hbox_autosave = gtk.HBox()
    hbox_autosave.set_spacing(4)
    checkbutton_autosave = gtk.CheckButton(_("Autosave Every"))
    adjustment_autosave = gtk.Adjustment(value=dad.autosave[1], lower=1, upper=1000, step_incr=1)
    spinbutton_autosave = gtk.SpinButton(adjustment_autosave)
    label_autosave = gtk.Label(_("Minutes"))
    hbox_autosave.pack_start(checkbutton_autosave, expand=False)
    hbox_autosave.pack_start(spinbutton_autosave, expand=False)
    hbox_autosave.pack_start(label_autosave, expand=False)
    checkbutton_autosave_on_quit = gtk.CheckButton(_("Autosave on Quit"))
    checkbutton_backup_before_saving = gtk.CheckButton(_("Create a Backup Copy Before Saving"))
    hbox_num_backups = gtk.HBox()
    hbox_num_backups.set_spacing(4)
    label_num_backups = gtk.Label(_("Number of Backups to Keep"))
    adjustment_num_backups = gtk.Adjustment(value=dad.backup_num, lower=1, upper=100, step_incr=1)
    spinbutton_num_backups = gtk.SpinButton(adjustment_num_backups)
    spinbutton_num_backups.set_sensitive(dad.backup_copy)
    spinbutton_num_backups.set_value(dad.backup_num)
    hbox_num_backups.pack_start(label_num_backups, expand=False)
    hbox_num_backups.pack_start(spinbutton_num_backups, expand=False)
    vbox_saving.pack_start(hbox_autosave, expand=False)
    vbox_saving.pack_start(checkbutton_autosave_on_quit, expand=False)
    vbox_saving.pack_start(checkbutton_backup_before_saving, expand=False)
    vbox_saving.pack_start(hbox_num_backups, expand=False)

    checkbutton_autosave.set_active(dad.autosave[0])
    spinbutton_autosave.set_value(dad.autosave[1])
    spinbutton_autosave.set_sensitive(dad.autosave[0])
    checkbutton_autosave_on_quit.set_active(dad.autosave_on_quit)
    checkbutton_backup_before_saving.set_active(dad.backup_copy)

    frame_saving = gtk.Frame(label="<b>"+_("Saving")+"</b>")
    frame_saving.get_label_widget().set_use_markup(True)
    frame_saving.set_shadow_type(gtk.SHADOW_NONE)
    align_saving = gtk.Alignment()
    align_saving.set_padding(3, 6, 6, 6)
    align_saving.add(vbox_saving)
    frame_saving.add(align_saving)

    vbox_misc_misc = gtk.VBox()
    checkbutton_newer_version = gtk.CheckButton(_("Automatically Check for Newer Version"))
    checkbutton_word_count = gtk.CheckButton(_("Enable Word Count in Statusbar"))
    checkbutton_reload_doc_last = gtk.CheckButton(_("Reload Document From Last Session"))
    checkbutton_mod_time_sentinel = gtk.CheckButton(_("Reload After External Update to CT* File"))
    vbox_misc_misc.pack_start(checkbutton_newer_version, expand=False)
    vbox_misc_misc.pack_start(checkbutton_word_count, expand=False)
    vbox_misc_misc.pack_start(checkbutton_reload_doc_last, expand=False)
    vbox_misc_misc.pack_start(checkbutton_mod_time_sentinel, expand=False)

    checkbutton_newer_version.set_active(dad.check_version)
    checkbutton_word_count.set_active(dad.word_count)
    checkbutton_reload_doc_last.set_active(dad.reload_doc_last)
    checkbutton_mod_time_sentinel.set_active(dad.enable_mod_time_sentinel)

    frame_misc_misc = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_misc.get_label_widget().set_use_markup(True)
    frame_misc_misc.set_shadow_type(gtk.SHADOW_NONE)
    align_misc_misc = gtk.Alignment()
    align_misc_misc.set_padding(3, 6, 6, 6)
    align_misc_misc.add(vbox_misc_misc)
    frame_misc_misc.add(align_misc_misc)

    vbox_language = gtk.VBox()
    combobox_country_language = gtk.ComboBox(model=dad.country_lang_liststore)
    vbox_language.pack_start(combobox_country_language)
    cell = gtk.CellRendererText()
    combobox_country_language.pack_start(cell, True)
    combobox_country_language.add_attribute(cell, 'text', 0)
    combobox_country_language.set_active_iter(dad.get_combobox_iter_from_value(dad.country_lang_liststore, 0, dad.country_lang))

    frame_language = gtk.Frame(label="<b>"+_("Language")+"</b>")
    frame_language.get_label_widget().set_use_markup(True)
    frame_language.set_shadow_type(gtk.SHADOW_NONE)
    align_language = gtk.Alignment()
    align_language.set_padding(3, 6, 6, 6)
    align_language.add(vbox_language)
    frame_language.add(align_language)

    vbox_misc.pack_start(frame_system_tray, expand=False)
    vbox_misc.pack_start(frame_saving, expand=False)
    vbox_misc.pack_start(frame_misc_misc, expand=False)
    vbox_misc.pack_start(frame_language, expand=False)
    def on_checkbutton_systray_toggled(checkbutton):
        dad.systray = checkbutton.get_active()
        if dad.systray:
            dad.ui.get_widget("/MenuBar/FileMenu/exit_app").set_property(cons.STR_VISIBLE, True)
            checkbutton_start_on_systray.set_sensitive(True)
        else:
            dad.ui.get_widget("/MenuBar/FileMenu/exit_app").set_property(cons.STR_VISIBLE, False)
            checkbutton_start_on_systray.set_sensitive(False)
        if dad.systray:
            if not dad.use_appind:
                if "status_icon" in dir(dad.boss): dad.boss.status_icon.set_property(cons.STR_VISIBLE, True)
                else: dad.status_icon_enable()
            else:
                if "ind" in dir(dad.boss): dad.boss.ind.set_status(appindicator.STATUS_ACTIVE)
                else: dad.status_icon_enable()
        else:
            if not dad.use_appind: dad.boss.status_icon.set_property(cons.STR_VISIBLE, False)
            else: dad.boss.ind.set_status(appindicator.STATUS_PASSIVE)
        dad.boss.systray_active = dad.systray
        if len(dad.boss.running_windows) > 1:
            for runn_win in dad.boss.running_windows:
                if runn_win.window == dad.window: continue
                runn_win.systray = dad.boss.systray_active
    checkbutton_systray.connect('toggled', on_checkbutton_systray_toggled)
    def on_checkbutton_start_on_systray_toggled(checkbutton):
        dad.start_on_systray = checkbutton.get_active()
    checkbutton_start_on_systray.connect('toggled', on_checkbutton_start_on_systray_toggled)
    def on_checkbutton_use_appind_toggled(checkbutton):
        if checkbutton_systray.get_active():
            former_active = True
            checkbutton_systray.set_active(False)
        else: former_active = False
        if checkbutton.get_active(): dad.use_appind = True
        else: dad.use_appind = False
        if former_active: checkbutton_systray.set_active(True)
        if len(dad.boss.running_windows) > 1:
            for runn_win in dad.boss.running_windows:
                if runn_win.window == dad.window: continue
                runn_win.use_appind = dad.use_appind
    checkbutton_use_appind.connect('toggled', on_checkbutton_use_appind_toggled)
    def on_checkbutton_autosave_toggled(checkbutton):
        dad.autosave[0] = checkbutton.get_active()
        if dad.autosave[0]:
            if dad.autosave_timer_id == None: dad.autosave_timer_start()
        else:
            if dad.autosave_timer_id != None: dad.autosave_timer_stop()
        spinbutton_autosave.set_sensitive(dad.autosave[0])
    checkbutton_autosave.connect('toggled', on_checkbutton_autosave_toggled)
    def on_spinbutton_autosave_value_changed(spinbutton):
        dad.autosave[1] = int(spinbutton.get_value())
        #print "new_autosave_value", dad.autosave[1]
        if dad.autosave_timer_id != None: dad.autosave_timer_stop()
        if dad.autosave[0] and dad.autosave_timer_id == None: dad.autosave_timer_start()
    spinbutton_autosave.connect('value-changed', on_spinbutton_autosave_value_changed)
    def on_checkbutton_backup_before_saving_toggled(checkbutton):
        dad.backup_copy = checkbutton.get_active()
        spinbutton_num_backups.set_sensitive(dad.backup_copy)
    checkbutton_backup_before_saving.connect('toggled', on_checkbutton_backup_before_saving_toggled)
    def on_spinbutton_num_backups_value_changed(spinbutton):
        dad.backup_num = int(spinbutton.get_value())
    spinbutton_num_backups.connect('value-changed', on_spinbutton_num_backups_value_changed)
    def on_checkbutton_autosave_on_quit_toggled(checkbutton):
        dad.autosave_on_quit = checkbutton.get_active()
    checkbutton_autosave_on_quit.connect('toggled', on_checkbutton_autosave_on_quit_toggled)
    def on_checkbutton_reload_doc_last_toggled(checkbutton):
        dad.reload_doc_last = checkbutton.get_active()
    checkbutton_reload_doc_last.connect('toggled', on_checkbutton_reload_doc_last_toggled)
    def on_checkbutton_mod_time_sentinel_toggled(checkbutton):
        dad.enable_mod_time_sentinel = checkbutton.get_active()
        if dad.enable_mod_time_sentinel:
            if dad.mod_time_sentinel_id == None:
                dad.modification_time_sentinel_start()
        else:
            if dad.mod_time_sentinel_id != None:
                dad.modification_time_sentinel_stop()
    checkbutton_mod_time_sentinel.connect('toggled', on_checkbutton_mod_time_sentinel_toggled)
    def on_checkbutton_newer_version_toggled(checkbutton):
        dad.check_version = checkbutton.get_active()
    checkbutton_newer_version.connect('toggled', on_checkbutton_newer_version_toggled)
    def on_checkbutton_word_count_toggled(checkbutton):
        dad.word_count = checkbutton.get_active()
        dad.update_selected_node_statusbar_info()
    checkbutton_word_count.connect('toggled', on_checkbutton_word_count_toggled)
    def on_combobox_country_language_changed(combobox):
        new_iter = combobox_country_language.get_active_iter()
        new_lang = dad.country_lang_liststore[new_iter][0]
        if new_lang != dad.country_lang:
            dad.country_lang = new_lang
            support.dialog_info(_("The New Language will be Available Only After Restarting CherryTree"), dad.window)
            lang_file_descriptor = file(cons.LANG_PATH, 'w')
            lang_file_descriptor.write(new_lang)
            lang_file_descriptor.close()
    combobox_country_language.connect('changed', on_combobox_country_language_changed)

def dialog_preferences(dad):
    """Preferences Dialog"""
    dialog = gtk.Dialog(title=_("Preferences"),
        parent=dad.window,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CLOSE, gtk.RESPONSE_ACCEPT))

    tabs_vbox_vec = []
    for tabs_idx in range(11):
        tabs_vbox_vec.append(gtk.VBox())
        tabs_vbox_vec[-1].set_spacing(3)

    notebook = gtk.Notebook()
    notebook.set_tab_pos(gtk.POS_LEFT)
    notebook.append_page(tabs_vbox_vec[0], gtk.Label(_("Text and Code")))
    notebook.append_page(tabs_vbox_vec[1], gtk.Label(_("Text")))
    notebook.append_page(tabs_vbox_vec[2], gtk.Label(_("Rich Text")))
    notebook.append_page(tabs_vbox_vec[3], gtk.Label(_("Plain Text and Code")))
    notebook.append_page(tabs_vbox_vec[4], gtk.Label(_("Tree")+" 1"))
    notebook.append_page(tabs_vbox_vec[5], gtk.Label(_("Tree")+" 2"))
    notebook.append_page(tabs_vbox_vec[6], gtk.Label(_("Fonts")))
    notebook.append_page(tabs_vbox_vec[7], gtk.Label(_("Links")))
    notebook.append_page(tabs_vbox_vec[8], gtk.Label(_("Toolbar")))
    notebook.append_page(tabs_vbox_vec[9], gtk.Label(_("Keyboard Shortcuts")))
    notebook.append_page(tabs_vbox_vec[10], gtk.Label(_("Miscellaneous")))

    tab_constructor = {
        0: preferences_tab_text_n_code,
        1: preferences_tab_text,
        2: preferences_tab_rich_text,
        3: preferences_tab_plain_text_n_code,
        4: preferences_tab_tree_1,
        5: preferences_tab_tree_2,
        6: preferences_tab_fonts,
        7: preferences_tab_links,
        8: preferences_tab_toolbar,
        9: preferences_tab_kb_shortcuts,
       10: preferences_tab_misc,
        }

    def on_notebook_switch_page(notebook, page, page_num):
        #print "new page", page_num
        tab_constructor[page_num](dad, tabs_vbox_vec[page_num], dialog)
        tabs_vbox_vec[page_num].show_all()
    notebook.connect('switch-page', on_notebook_switch_page)

    content_area = dialog.get_content_area()
    content_area.pack_start(notebook)
    content_area.show_all()
    notebook.set_current_page(dad.prefpage)
    dialog.disp_dialog_after_restart = False
    dialog.run()
    dad.prefpage = notebook.get_current_page()
    dialog.hide()
