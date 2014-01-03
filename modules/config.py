# -*- coding: UTF-8 -*-
#
#       config.py
#
#       Copyright 2009-2014 Giuseppe Penone <giuspen@gmail.com>
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

import os, sys, ConfigParser, gtk, pango, locale, subprocess, base64, webbrowser
import cons, support, pgsc_spellcheck
if cons.HAS_APPINDICATOR: import appindicator

ICONS_SIZE = {1: gtk.ICON_SIZE_MENU, 2: gtk.ICON_SIZE_SMALL_TOOLBAR, 3: gtk.ICON_SIZE_LARGE_TOOLBAR,
              4: gtk.ICON_SIZE_DND, 5: gtk.ICON_SIZE_DIALOG}

LINK_CUSTOM_ACTION_DEFAULT_WEB = "firefox %s &"
LINK_CUSTOM_ACTION_DEFAULT_FILE = "xdg-open %s &"
HORIZONTAL_RULE = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
COLOR_PALETTE_DEFAULT = ["#000000", "#ffffff", "#7f7f7f", "#ff0000", "#a020f0",
                         "#0000ff", "#add8e6", "#00ff00", "#ffff00", "#ffa500",
                         "#e6e6fa", "#a52a2a", "#8b6914", "#1e90ff", "#ffc0cb",
                         "#90ee90", "#1a1a1a", "#4d4d4d", "#bfbfbf", "#e5e5e5"]
SPECIAL_CHARS_DEFAULT = "“”„•☐☑☒…‰€©®™°↓↑→←↔↵⇓⇑⇒⇐⇔»«▼▲►◄≤≥≠±¹²³½¼⅛×÷∞ø∑√∫ΔδΠπΣΦΩωαβγεηλμ☺☻☼♥♀♂♪♫"

SPELL_CHECK_LANG_DEFAULT = locale.getdefaultlocale()[0]

def config_file_load(inst):
    """Load the Preferences from Config File"""
    if os.path.isfile(cons.CONFIG_PATH):
        config = ConfigParser.RawConfigParser()
        config.read(cons.CONFIG_PATH)

        section = "state"
        inst.file_dir = unicode(config.get(section, "file_dir"), cons.STR_UTF8, cons.STR_IGNORE) if config.has_option(section, "file_dir") else ""
        inst.file_name = unicode(config.get(section, "file_name"), cons.STR_UTF8, cons.STR_IGNORE) if config.has_option(section, "file_name") else ""
        inst.toolbar_visible = config.getboolean(section, "toolbar_visible") if config.has_option(section, "toolbar_visible") else True
        inst.win_is_maximized = config.getboolean(section, "win_is_maximized") if config.has_option(section, "win_is_maximized") else False
        # restore window size and position
        if config.has_option(section, "win_position_x") and config.has_option(section, "win_position_y"):
            inst.win_position = [config.getint(section, "win_position_x"), config.getint(section, "win_position_y")]
            inst.window.move(inst.win_position[0], inst.win_position[1])
        else: inst.win_position = [10, 10]
        if inst.win_is_maximized: inst.window.maximize()
        elif config.has_option(section, "win_size_w") and config.has_option(section, "win_size_h"):
            win_size = [config.getint(section, "win_size_w"), config.getint(section, "win_size_h")]
            inst.window.resize(win_size[0], win_size[1])
        inst.hpaned_pos = config.getint(section, "hpaned_pos") if config.has_option(section, "hpaned_pos") else 170
        if config.has_option(section, "node_path"):
            # restor the selected node
            str_path_list_of_str = config.get(section, "node_path")
            path_list_of_str = str_path_list_of_str.split()
            path_list_of_int = [int(element) for element in path_list_of_str]
            inst.node_path = tuple(path_list_of_int)
            inst.cursor_position = config.getint(section, "cursor_position") if config.has_option(section, "cursor_position") else 0
        else: inst.node_path = None
        inst.recent_docs = []
        if config.has_option(section, "recent_docs"):
            temp_recent_docs = config.get(section, "recent_docs").split(cons.CHAR_SPACE)
            for element in temp_recent_docs:
                if element: inst.recent_docs.append(unicode(base64.b64decode(element), cons.STR_UTF8, cons.STR_IGNORE))
        inst.pick_dir = config.get(section, "pick_dir") if config.has_option(section, "pick_dir") else ""
        inst.link_type = config.get(section, "link_type") if config.has_option(section, "link_type") else cons.LINK_TYPE_WEBS
        inst.show_node_name_label = config.getboolean(section, "show_node_name_label") if config.has_option(section, "show_node_name_label") else True
        if config.has_option(section, "toolbar_icon_size"):
            inst.toolbar_icon_size = config.getint(section, "toolbar_icon_size")
            if inst.toolbar_icon_size not in ICONS_SIZE: inst.toolbar_icon_size = 1
        else: inst.toolbar_icon_size = 1
        inst.curr_colors = {'f':gtk.gdk.color_parse(config.get(section, "fg")) if config.has_option(section, "fg") else None,
                            'b':gtk.gdk.color_parse(config.get(section, "bg")) if config.has_option(section, "bg") else None}

        section = "tree"
        inst.rest_exp_coll = config.getint(section, "rest_exp_coll") if config.has_option(section, "rest_exp_coll") else 0
        inst.expanded_collapsed_string = config.get(section, "expanded_collapsed_string") if config.has_option(section, "expanded_collapsed_string") else ""
        inst.nodes_icons = config.get(section, "nodes_icons") if config.has_option(section, "nodes_icons") else "c"
        inst.tree_right_side = config.getboolean(section, "tree_right_side") if config.has_option(section, "tree_right_side") else False
        inst.cherry_wrap_width = config.getint(section, "cherry_wrap_width") if config.has_option(section, "cherry_wrap_width") else 130

        section = "editor"
        inst.syntax_highlighting = config.get(section, "syntax_highlighting") if config.has_option(section, "syntax_highlighting") else cons.CUSTOM_COLORS_ID
        inst.style_scheme = config.get(section, "style_scheme") if config.has_option(section, "style_scheme") else cons.STYLE_SCHEME_DEFAULT
        inst.enable_spell_check = config.getboolean(section, "enable_spell_check") if config.has_option(section, "enable_spell_check") else False
        inst.spell_check_lang = config.get(section, "spell_check_lang") if config.has_option(section, "spell_check_lang") else SPELL_CHECK_LANG_DEFAULT
        inst.show_line_numbers = config.getboolean(section, "show_line_numbers") if config.has_option(section, "show_line_numbers") else False
        inst.spaces_instead_tabs = config.getboolean(section, "spaces_instead_tabs") if config.has_option(section, "spaces_instead_tabs") else True
        inst.tabs_width = config.getint(section, "tabs_width") if config.has_option(section, "tabs_width") else 4
        inst.anchor_size = config.getint(section, "anchor_size") if config.has_option(section, "anchor_size") else 16
        inst.line_wrapping = config.getboolean(section, "line_wrapping") if config.has_option(section, "line_wrapping") else True
        inst.wrapping_indent = config.getint(section, "wrapping_indent") if config.has_option(section, "wrapping_indent") else -14
        inst.auto_indent = config.getboolean(section, "auto_indent") if config.has_option(section, "auto_indent") else True
        inst.show_white_spaces = config.getboolean(section, "show_white_spaces") if config.has_option(section, "show_white_spaces") else True
        inst.highl_curr_line = config.getboolean(section, "highl_curr_line") if config.has_option(section, "highl_curr_line") else True
        inst.h_rule = config.get(section, "h_rule") if config.has_option(section, "h_rule") else HORIZONTAL_RULE
        inst.special_chars = unicode(config.get(section, "special_chars") if config.has_option(section, "special_chars") else SPECIAL_CHARS_DEFAULT, cons.STR_UTF8, cons.STR_IGNORE)
        inst.timestamp_format = config.get(section, "timestamp_format") if config.has_option(section, "timestamp_format") else "%Y/%m/%d - %H:%M"
        if config.has_option(section, "weblink_custom_action"):
            temp_str = config.get(section, "weblink_custom_action")
            inst.weblink_custom_action = [True, temp_str[4:]] if temp_str[:4] == "True" else [False, temp_str[5:]]
        else: inst.weblink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_WEB]
        if config.has_option(section, "filelink_custom_action"):
            temp_str = config.get(section, "filelink_custom_action")
            inst.filelink_custom_action = [True, temp_str[4:]] if temp_str[:4] == "True" else [False, temp_str[5:]]
        else: inst.filelink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]
        if config.has_option(section, "folderlink_custom_action"):
            temp_str = config.get(section, "folderlink_custom_action")
            inst.folderlink_custom_action = [True, temp_str[4:]] if temp_str[:4] == "True" else [False, temp_str[5:]]
        else: inst.folderlink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]

        section = "codebox"
        if config.has_option(section, "codebox_width"):
            inst.codebox_width = config.getfloat(section, "codebox_width")
        else: inst.codebox_width = 700
        if config.has_option(section, "codebox_height"):
            inst.codebox_height = config.getfloat(section, "codebox_height")
        else: inst.codebox_height = 100
        inst.codebox_width_pixels = config.getboolean(section, "codebox_width_pixels") if config.has_option(section, "codebox_width_pixels") else True

        section = "table"
        inst.table_rows = config.getint(section, "table_rows") if config.has_option(section, "table_rows") else 3
        inst.table_columns = config.getint(section, "table_columns") if config.has_option(section, "table_columns") else 3
        inst.table_column_mode = config.get(section, "table_column_mode") if config.has_option(section, "table_column_mode") else "rename"
        inst.table_col_min = config.getint(section, "table_col_min") if config.has_option(section, "table_col_min") else 40
        inst.table_col_max = config.getint(section, "table_col_max") if config.has_option(section, "table_col_max") else 60

        section = "fonts"
        inst.text_font = config.get(section, "text_font") if config.has_option(section, "text_font") else "Sans 9" # default text font
        inst.tree_font = config.get(section, "tree_font") if config.has_option(section, "tree_font") else "Sans 8" # default tree font
        inst.code_font = config.get(section, "code_font") if config.has_option(section, "code_font") else "Monospace 9" # default code font

        section = "colors"
        inst.rt_def_fg = config.get(section, "rt_def_fg") if config.has_option(section, "rt_def_fg") else cons.RICH_TEXT_DARK_FG
        inst.rt_def_bg = config.get(section, "rt_def_bg") if config.has_option(section, "rt_def_bg") else cons.RICH_TEXT_DARK_BG
        inst.tt_def_fg = config.get(section, "tt_def_fg") if config.has_option(section, "tt_def_fg") else cons.TREE_TEXT_LIGHT_FG
        inst.tt_def_bg = config.get(section, "tt_def_bg") if config.has_option(section, "tt_def_bg") else cons.TREE_TEXT_LIGHT_BG
        if config.has_option(section, "palette_list"):
            inst.palette_list = config.get(section, "palette_list").split(":")
        else: inst.palette_list = COLOR_PALETTE_DEFAULT
        inst.col_link_webs = config.get(section, "col_link_webs") if config.has_option(section, "col_link_webs") else cons.COLOR_48_LINK_WEBS
        inst.col_link_node = config.get(section, "col_link_node") if config.has_option(section, "col_link_node") else cons.COLOR_48_LINK_NODE
        inst.col_link_file = config.get(section, "col_link_file") if config.has_option(section, "col_link_file") else cons.COLOR_48_LINK_FILE
        inst.col_link_fold = config.get(section, "col_link_fold") if config.has_option(section, "col_link_fold") else cons.COLOR_48_LINK_FOLD

        section = "misc"
        inst.systray = config.getboolean(section, "systray") if config.has_option(section, "systray") else False
        inst.start_on_systray = config.getboolean(section, "start_on_systray") if config.has_option(section, "start_on_systray") else False
        inst.use_appind = config.getboolean(section, "use_appind") if config.has_option(section, "use_appind") else False
        if config.has_option(section, "autosave") and config.has_option(section, "autosave_val"):
            inst.autosave = [config.getboolean(section, "autosave"), config.getint(section, "autosave_val")]
        else: inst.autosave = [False, 5]
        inst.check_version = config.getboolean(section, "check_version") if config.has_option(section, "check_version") else False
        inst.reload_doc_last = config.getboolean(section, "reload_doc_last") if config.has_option(section, "reload_doc_last") else True
        inst.enable_mod_time_sentinel = config.getboolean(section, "mod_time_sent") if config.has_option(section, "mod_time_sent") else True
        inst.backup_copy = config.getboolean(section, "backup_copy") if config.has_option(section, "backup_copy") else True
        inst.autosave_on_quit = config.getboolean(section, "autosave_on_quit") if config.has_option(section, "autosave_on_quit") else False
        inst.limit_undoable_steps = config.getint(section, "limit_undoable_steps") if config.has_option(section, "limit_undoable_steps") else 20
    else:
        inst.file_dir = ""
        inst.file_name = ""
        inst.node_path = None
        inst.curr_colors = {'f':None, 'b':None}
        inst.syntax_highlighting = cons.CUSTOM_COLORS_ID
        inst.style_scheme = cons.STYLE_SCHEME_DEFAULT
        inst.tree_font = "Sans 8" # default tree font
        inst.text_font = "Sans 9" # default text font
        inst.code_font = "Monospace 9" # default code font
        inst.rt_def_fg = cons.RICH_TEXT_DARK_FG
        inst.rt_def_bg = cons.RICH_TEXT_DARK_BG
        inst.tt_def_fg = cons.TREE_TEXT_LIGHT_FG
        inst.tt_def_bg = cons.TREE_TEXT_LIGHT_BG
        inst.palette_list = COLOR_PALETTE_DEFAULT
        inst.col_link_webs = cons.COLOR_48_LINK_WEBS
        inst.col_link_node = cons.COLOR_48_LINK_NODE
        inst.col_link_file = cons.COLOR_48_LINK_FILE
        inst.col_link_fold = cons.COLOR_48_LINK_FOLD
        inst.h_rule = HORIZONTAL_RULE
        inst.special_chars = unicode(SPECIAL_CHARS_DEFAULT, cons.STR_UTF8, cons.STR_IGNORE)
        inst.enable_spell_check = False
        inst.spell_check_lang = SPELL_CHECK_LANG_DEFAULT
        inst.show_line_numbers = False
        inst.spaces_instead_tabs = True
        inst.tabs_width = 4
        inst.anchor_size = 16
        inst.line_wrapping = True
        inst.wrapping_indent = -14
        inst.auto_indent = True
        inst.systray = False
        inst.win_position = [10, 10]
        inst.autosave = [False, 5]
        inst.win_is_maximized = False
        inst.rest_exp_coll = 0
        inst.expanded_collapsed_string = ""
        inst.pick_dir = ""
        inst.link_type = cons.LINK_TYPE_WEBS
        inst.toolbar_icon_size = 1
        inst.table_rows = 3
        inst.table_columns = 3
        inst.table_column_mode = "rename"
        inst.table_col_min = 40
        inst.table_col_max = 60
        inst.limit_undoable_steps = 20
        inst.cherry_wrap_width = 130
        inst.start_on_systray = False
        inst.use_appind = False
        inst.weblink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_WEB]
        inst.filelink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]
        inst.folderlink_custom_action = [False, LINK_CUSTOM_ACTION_DEFAULT_FILE]
        inst.timestamp_format = "%Y/%m/%d - %H:%M"
        inst.codebox_width = 700
        inst.codebox_width_pixels = True
        inst.codebox_height = 100
        inst.check_version = False
        inst.reload_doc_last = True
        inst.enable_mod_time_sentinel = True
        inst.backup_copy = True
        inst.autosave_on_quit = False
        inst.tree_right_side = False
        inst.show_white_spaces = True
        inst.highl_curr_line = True
        inst.hpaned_pos = 170
        inst.show_node_name_label = True
        inst.nodes_icons = "c"
        inst.recent_docs = []
        inst.toolbar_visible = True

def config_file_apply(inst):
    """Apply the Preferences from Config File"""
    inst.hpaned.set_property('position', inst.hpaned_pos)
    inst.header_node_name_label.set_property(cons.STR_VISIBLE, inst.show_node_name_label)
    inst.set_treeview_font()
    inst.treeview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(inst.tt_def_bg))
    inst.treeview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(inst.tt_def_fg))
    if not pgsc_spellcheck.HAS_PYENCHANT:
        inst.enable_spell_check = False
    if inst.enable_spell_check:
        inst.spell_check_set_on()
    inst.sourceview.set_show_line_numbers(inst.show_line_numbers)
    inst.sourceview.set_insert_spaces_instead_of_tabs(inst.spaces_instead_tabs)
    inst.sourceview.set_tab_width(inst.tabs_width)
    inst.sourceview.set_indent(inst.wrapping_indent)
    if inst.line_wrapping: inst.sourceview.set_wrap_mode(gtk.WRAP_WORD)
    else: inst.sourceview.set_wrap_mode(gtk.WRAP_NONE)
    inst.renderer_text.set_property('wrap-width', inst.cherry_wrap_width)
    inst.ui.get_widget("/ToolBar").set_property(cons.STR_VISIBLE, inst.toolbar_visible)
    inst.ui.get_widget("/ToolBar").set_style(gtk.TOOLBAR_ICONS)
    inst.ui.get_widget("/ToolBar").set_property("icon-size", ICONS_SIZE[inst.toolbar_icon_size])
    if inst.autosave[0]: inst.autosave_timer_start()
    if inst.enable_mod_time_sentinel: inst.modification_time_sentinel_start()

def config_file_save(inst):
    """Save the Preferences to Config File"""
    config = ConfigParser.RawConfigParser()

    section = "state"
    config.add_section(section)
    config.set(section, "file_dir", inst.file_dir)
    config.set(section, "file_name", inst.file_name)
    config.set(section, "toolbar_visible", inst.ui.get_widget("/ToolBar").get_property(cons.STR_VISIBLE))
    config.set(section, "win_is_maximized", inst.win_is_maximized)
    inst.win_position = inst.window.get_position()
    config.set(section, "win_position_x", inst.win_position[0])
    config.set(section, "win_position_y", inst.win_position[1])
    if not inst.win_is_maximized:
        win_size = inst.window.get_size()
        config.set(section, "win_size_w", win_size[0])
        config.set(section, "win_size_h", win_size[1])
    config.set(section, "hpaned_pos", inst.hpaned.get_property('position'))
    if inst.curr_tree_iter:
        path_list_of_str = []
        for element in inst.treestore.get_path(inst.curr_tree_iter):
            path_list_of_str.append( str(element) )
        config.set(section, "node_path", " ".join(path_list_of_str))
        config.set(section, "cursor_position", inst.curr_buffer.get_property(cons.STR_CURSOR_POSITION))
    if inst.recent_docs:
        temp_recent_docs = []
        for i, element in enumerate(inst.recent_docs):
            if i >= cons.MAX_RECENT_DOCS: break
            temp_recent_docs.append(base64.b64encode(element))
        str_recent_docs = cons.CHAR_SPACE.join(temp_recent_docs)
    else: str_recent_docs = ""
    config.set(section, "recent_docs", str_recent_docs)
    config.set(section, "pick_dir", inst.pick_dir)
    config.set(section, "link_type", inst.link_type)
    config.set(section, "show_node_name_label", inst.header_node_name_label.get_property(cons.STR_VISIBLE))
    config.set(section, "toolbar_icon_size", inst.toolbar_icon_size)
    if inst.curr_colors['f']: config.set(section, "fg", inst.curr_colors['f'].to_string())
    if inst.curr_colors['b']: config.set(section, "bg", inst.curr_colors['b'].to_string())

    section = "tree"
    config.add_section(section)
    config.set(section, "rest_exp_coll", inst.rest_exp_coll)
    if inst.rest_exp_coll == 0:
        get_tree_expanded_collapsed_string(inst)
        config.set(section, "expanded_collapsed_string", inst.expanded_collapsed_string)
    config.set(section, "nodes_icons", inst.nodes_icons)
    config.set(section, "tree_right_side", inst.tree_right_side)
    config.set(section, "cherry_wrap_width", inst.cherry_wrap_width)

    section = "editor"
    config.add_section(section)
    config.set(section, "syntax_highlighting", inst.syntax_highlighting)
    config.set(section, "style_scheme", inst.style_scheme)
    config.set(section, "spell_check_lang", inst.spell_check_lang)
    config.set(section, "enable_spell_check", inst.enable_spell_check)
    config.set(section, "show_line_numbers", inst.show_line_numbers)
    config.set(section, "spaces_instead_tabs", inst.spaces_instead_tabs)
    config.set(section, "tabs_width", inst.tabs_width)
    config.set(section, "anchor_size", inst.anchor_size)
    config.set(section, "line_wrapping", inst.line_wrapping)
    config.set(section, "wrapping_indent", inst.wrapping_indent)
    config.set(section, "auto_indent", inst.auto_indent)
    config.set(section, "show_white_spaces", inst.show_white_spaces)
    config.set(section, "highl_curr_line", inst.highl_curr_line)
    config.set(section, "h_rule", inst.h_rule)
    config.set(section, "special_chars", inst.special_chars)
    config.set(section, "timestamp_format", inst.timestamp_format)
    config.set(section, "weblink_custom_action", str(inst.weblink_custom_action[0])+inst.weblink_custom_action[1])
    config.set(section, "filelink_custom_action", str(inst.filelink_custom_action[0])+inst.filelink_custom_action[1])
    config.set(section, "folderlink_custom_action", str(inst.folderlink_custom_action[0])+inst.folderlink_custom_action[1])

    section = "codebox"
    config.add_section(section)
    config.set(section, "codebox_width", inst.codebox_width)
    config.set(section, "codebox_height", inst.codebox_height)
    config.set(section, "codebox_width_pixels", inst.codebox_width_pixels)

    section = "table"
    config.add_section(section)
    config.set(section, "table_rows", inst.table_rows)
    config.set(section, "table_columns", inst.table_columns)
    config.set(section, "table_column_mode", inst.table_column_mode)
    config.set(section, "table_col_min", inst.table_col_min)
    config.set(section, "table_col_max", inst.table_col_max)

    section = "fonts"
    config.add_section(section)
    config.set(section, "text_font", inst.text_font)
    config.set(section, "tree_font", inst.tree_font)
    config.set(section, "code_font", inst.code_font)

    section = "colors"
    config.add_section(section)
    config.set(section, "rt_def_fg", inst.rt_def_fg)
    config.set(section, "rt_def_bg", inst.rt_def_bg)
    config.set(section, "tt_def_fg", inst.tt_def_fg)
    config.set(section, "tt_def_bg", inst.tt_def_bg)
    config.set(section, "palette_list", ":".join(inst.palette_list))
    config.set(section, "col_link_webs", inst.col_link_webs)
    config.set(section, "col_link_node", inst.col_link_node)
    config.set(section, "col_link_file", inst.col_link_file)
    config.set(section, "col_link_fold", inst.col_link_fold)

    section = "misc"
    config.add_section(section)
    config.set(section, "systray", inst.systray)
    config.set(section, "start_on_systray", inst.start_on_systray)
    config.set(section, "use_appind", inst.use_appind)
    config.set(section, "autosave", inst.autosave[0])
    config.set(section, "autosave_val", inst.autosave[1])
    config.set(section, "check_version", inst.check_version)
    config.set(section, "reload_doc_last", inst.reload_doc_last)
    config.set(section, "mod_time_sent", inst.enable_mod_time_sentinel)
    config.set(section, "backup_copy", inst.backup_copy)
    config.set(section, "autosave_on_quit", inst.autosave_on_quit)
    config.set(section, "limit_undoable_steps", inst.limit_undoable_steps)

    with open(cons.CONFIG_PATH, 'wb') as configfile:
        config.write(configfile)

def get_tree_expanded_collapsed_string(inst):
    """Returns a String Containing the Info about Expanded and Collapsed Nodes"""
    expanded_collapsed_string = ""
    tree_iter = inst.treestore.get_iter_first()
    while tree_iter != None:
        expanded_collapsed_string += get_tree_expanded_collapsed_string_iter(tree_iter, inst)
        tree_iter = inst.treestore.iter_next(tree_iter)
    if len(expanded_collapsed_string) > 0: inst.expanded_collapsed_string = expanded_collapsed_string[1:]
    else: inst.expanded_collapsed_string = ""

def get_tree_expanded_collapsed_string_iter(tree_iter, inst):
    """Iter of the Info about Expanded and Collapsed Nodes"""
    expanded_collapsed_string = "_%s,%s" % (inst.treestore[tree_iter][3],
                                            inst.treeview.row_expanded(inst.treestore.get_path(tree_iter)))
    tree_iter = inst.treestore.iter_children(tree_iter)
    while tree_iter != None:
        expanded_collapsed_string += get_tree_expanded_collapsed_string_iter(tree_iter, inst)
        tree_iter = inst.treestore.iter_next(tree_iter)
    return expanded_collapsed_string

def set_tree_expanded_collapsed_string(inst):
    """Parses the String Containing the Info about Expanded and Collapsed Nodes"""
    inst.treeview.collapse_all()
    if inst.expanded_collapsed_string == "": return
    expanded_collapsed_dict = {}
    expanded_collapsed_vector = inst.expanded_collapsed_string.split('_')
    for element in expanded_collapsed_vector:
        couple = element.split(',')
        expanded_collapsed_dict[couple[0]] = couple[1]
    tree_iter = inst.treestore.get_iter_first()
    while tree_iter != None:
        set_tree_expanded_collapsed_string_iter(tree_iter, expanded_collapsed_dict, inst)
        tree_iter = inst.treestore.iter_next(tree_iter)

def set_tree_expanded_collapsed_string_iter(tree_iter, expanded_collapsed_dict, inst):
    """Iter of the Expanded and Collapsed Nodes Parsing"""
    node_id = str(inst.treestore[tree_iter][3])
    if node_id in expanded_collapsed_dict and expanded_collapsed_dict[node_id] == "True":
        inst.treeview.expand_row(inst.treestore.get_path(tree_iter), open_all=False)
    tree_iter = inst.treestore.iter_children(tree_iter)
    while tree_iter != None:
        set_tree_expanded_collapsed_string_iter(tree_iter, expanded_collapsed_dict, inst)
        tree_iter = inst.treestore.iter_next(tree_iter)

def preferences_tab_all_nodes(dad, vbox_all_nodes):
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

    vbox_text_editor = gtk.VBox()
    vbox_text_editor.pack_start(hbox_tab_width, expand=False)
    vbox_text_editor.pack_start(checkbutton_spaces_tabs, expand=False)
    vbox_text_editor.pack_start(checkbutton_line_wrap, expand=False)
    vbox_text_editor.pack_start(hbox_wrapping_indent, expand=False)
    vbox_text_editor.pack_start(checkbutton_auto_indent, expand=False)
    vbox_text_editor.pack_start(checkbutton_line_nums, expand=False)
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
    button_strftime_help.set_image(gtk.image_new_from_stock("gtk-help", gtk.ICON_SIZE_BUTTON))
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
    label_special_chars = gtk.Label(_("Special Characters"))
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
    hbox_special_chars.pack_start(label_special_chars, expand=False)
    hbox_special_chars.pack_start(frame_special_chars)

    vbox_misc_all = gtk.VBox()
    vbox_misc_all.set_spacing(2)
    vbox_misc_all.pack_start(hbox_timestamp)
    vbox_misc_all.pack_start(hbox_horizontal_rule)
    vbox_misc_all.pack_start(hbox_special_chars)
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
    def on_spinbutton_tab_width_value_changed(spinbutton):
        dad.tabs_width = int(spinbutton.get_value())
        dad.sourceview.set_tab_width(dad.tabs_width)
    spinbutton_tab_width.connect('value-changed', on_spinbutton_tab_width_value_changed)
    def on_spinbutton_wrapping_indent_value_changed(spinbutton):
        dad.wrapping_indent = int(spinbutton.get_value())
        dad.sourceview.set_indent(dad.wrapping_indent)
    spinbutton_wrapping_indent.connect('value-changed', on_spinbutton_wrapping_indent_value_changed)
    def on_checkbutton_spaces_tabs_toggled(checkbutton):
        dad.spaces_instead_tabs = checkbutton.get_active()
        dad.sourceview.set_insert_spaces_instead_of_tabs(dad.spaces_instead_tabs)
    checkbutton_spaces_tabs.connect('toggled', on_checkbutton_spaces_tabs_toggled)
    def on_checkbutton_line_wrap_toggled(checkbutton):
        dad.line_wrapping = checkbutton.get_active()
        dad.sourceview.set_wrap_mode(gtk.WRAP_WORD if dad.line_wrapping else gtk.WRAP_NONE)
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
        lang_code = locale.getdefaultlocale()[0]
        if lang_code:
            page_lang = lang_code[0:2] if lang_code[0:2] in ["de", "es", "fr"] else ""
        else: page_lang = ""
        webbrowser.open("http://man.cx/strftime(3)/" + page_lang)
    button_strftime_help.connect('clicked', on_button_strftime_help_clicked)
    def on_entry_horizontal_rule_changed(entry):
        dad.h_rule = entry.get_text()
    entry_horizontal_rule.connect('changed', on_entry_horizontal_rule_changed)

def preferences_tab_text_nodes(dad, vbox_text_nodes):
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

    vbox_rt_theme.pack_start(radiobutton_rt_col_light, expand=False)
    vbox_rt_theme.pack_start(radiobutton_rt_col_dark, expand=False)
    vbox_rt_theme.pack_start(hbox_rt_col_custom, expand=False)
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

    hbox_misc_text = gtk.HBox()
    hbox_misc_text.set_spacing(4)
    label_limit_undoable_steps = gtk.Label(_("Limit of Undoable Steps Per Node"))
    adj_limit_undoable_steps = gtk.Adjustment(value=dad.limit_undoable_steps, lower=1, upper=10000, step_incr=1)
    spinbutton_limit_undoable_steps = gtk.SpinButton(adj_limit_undoable_steps)
    spinbutton_limit_undoable_steps.set_value(dad.limit_undoable_steps)
    hbox_misc_text.pack_start(label_limit_undoable_steps, expand=False)
    hbox_misc_text.pack_start(spinbutton_limit_undoable_steps, expand=False)

    vbox_misc_text = gtk.VBox()
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
        else: dad.spell_check_set_off()
        combobox_spell_check_lang.set_sensitive(dad.enable_spell_check)
    checkbutton_spell_check.connect('toggled', on_checkbutton_spell_check_toggled)
    def on_combobox_spell_check_lang_changed(combobox):
        new_iter = combobox.get_active_iter()
        new_lang_code = dad.spell_check_lang_liststore[new_iter][0]
        if new_lang_code != dad.spell_check_lang: dad.spell_check_set_new_lang(new_lang_code)
    combobox_spell_check_lang.connect('changed', on_combobox_spell_check_lang_changed)
    def on_colorbutton_text_fg_color_set(colorbutton):
        dad.rt_def_fg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.rt_def_fg))
    colorbutton_text_fg.connect('color-set', on_colorbutton_text_fg_color_set)
    def on_colorbutton_text_bg_color_set(colorbutton):
        dad.rt_def_bg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.rt_def_bg))
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
    def on_spinbutton_limit_undoable_steps_value_changed(spinbutton):
        dad.limit_undoable_steps = int(spinbutton.get_value())
    spinbutton_limit_undoable_steps.connect('value-changed', on_spinbutton_limit_undoable_steps_value_changed)

    if not pgsc_spellcheck.HAS_PYENCHANT:
        checkbutton_spell_check.set_sensitive(False)
        combobox_spell_check_lang.set_sensitive(False)

def preferences_tab_code_nodes(dad, vbox_code_nodes):
    """Preferences Dialog, Code Nodes Tab"""
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
    checkbutton_show_white_spaces = gtk.CheckButton(_("Show White Spaces"))
    checkbutton_show_white_spaces.set_active(dad.show_white_spaces)
    checkbutton_highlight_current_line = gtk.CheckButton(_("Highlight Current Line"))
    checkbutton_highlight_current_line.set_active(dad.highl_curr_line)

    vbox_syntax.pack_start(hbox_style_scheme, expand=False)
    vbox_syntax.pack_start(checkbutton_show_white_spaces, expand=False)
    vbox_syntax.pack_start(checkbutton_highlight_current_line, expand=False)

    frame_syntax = gtk.Frame(label="<b>"+_("Automatic Syntax Highlighting")+"</b>")
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
            support.dialog_info_after_restart(dad.window)
    combobox_style_scheme.connect('changed', on_combobox_style_scheme_changed)
    def on_checkbutton_show_white_spaces_toggled(checkbutton):
        dad.show_white_spaces = checkbutton.get_active()
        support.dialog_info_after_restart(dad.window)
    checkbutton_show_white_spaces.connect('toggled', on_checkbutton_show_white_spaces_toggled)
    def on_checkbutton_highlight_current_line_toggled(checkbutton):
        dad.highl_curr_line = checkbutton.get_active()
        support.dialog_info_after_restart(dad.window)
    checkbutton_highlight_current_line.connect('toggled', on_checkbutton_highlight_current_line_toggled)

def preferences_tab_tree(dad, vbox_tree):
    """Preferences Dialog, Tree Tab"""
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

    radiobutton_node_icon_cherry = gtk.RadioButton(label=_("Use Cherries as Nodes Icons"))
    radiobutton_node_icon_bullet = gtk.RadioButton(label=_("Use Bullets as Nodes Icons"))
    radiobutton_node_icon_bullet.set_group(radiobutton_node_icon_cherry)
    radiobutton_node_icon_none = gtk.RadioButton(label=_("Do Not Display Nodes Icons"))
    radiobutton_node_icon_none.set_group(radiobutton_node_icon_cherry)

    vbox_nodes_icons.pack_start(radiobutton_node_icon_cherry, expand=False)
    vbox_nodes_icons.pack_start(radiobutton_node_icon_bullet, expand=False)
    vbox_nodes_icons.pack_start(radiobutton_node_icon_none, expand=False)
    frame_nodes_icons = gtk.Frame(label="<b>"+_("Nodes Icons")+"</b>")
    frame_nodes_icons.get_label_widget().set_use_markup(True)
    frame_nodes_icons.set_shadow_type(gtk.SHADOW_NONE)
    align_nodes_icons = gtk.Alignment()
    align_nodes_icons.set_padding(3, 6, 6, 6)
    align_nodes_icons.add(vbox_nodes_icons)
    frame_nodes_icons.add(align_nodes_icons)

    radiobutton_node_icon_cherry.set_active(dad.nodes_icons == "c")
    radiobutton_node_icon_bullet.set_active(dad.nodes_icons == "b")
    radiobutton_node_icon_none.set_active(dad.nodes_icons == "n")

    vbox_nodes_startup = gtk.VBox()

    radiobutton_nodes_startup_restore = gtk.RadioButton(label=_("Restore Expanded/Collapsed Status"))
    radiobutton_nodes_startup_expand = gtk.RadioButton(label=_("Expand all Nodes"))
    radiobutton_nodes_startup_expand.set_group(radiobutton_nodes_startup_restore)
    radiobutton_nodes_startup_collapse = gtk.RadioButton(label=_("Collapse all Nodes"))
    radiobutton_nodes_startup_collapse.set_group(radiobutton_nodes_startup_restore)

    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_restore, expand=False)
    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_expand, expand=False)
    vbox_nodes_startup.pack_start(radiobutton_nodes_startup_collapse, expand=False)
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

    vbox_misc_tree.pack_start(hbox_tree_nodes_names_width, expand=False)
    vbox_misc_tree.pack_start(checkbutton_tree_right_side, expand=False)
    frame_misc_tree = gtk.Frame(label="<b>"+_("Miscellaneous")+"</b>")
    frame_misc_tree.get_label_widget().set_use_markup(True)
    frame_misc_tree.set_shadow_type(gtk.SHADOW_NONE)
    align_misc_tree = gtk.Alignment()
    align_misc_tree.set_padding(3, 6, 6, 6)
    align_misc_tree.add(vbox_misc_tree)
    frame_misc_tree.add(align_misc_tree)

    vbox_tree.pack_start(frame_tt_theme, expand=False)
    vbox_tree.pack_start(frame_nodes_icons, expand=False)
    vbox_tree.pack_start(frame_nodes_startup, expand=False)
    vbox_tree.pack_start(frame_misc_tree, expand=False)
    def on_colorbutton_tree_fg_color_set(colorbutton):
        dad.tt_def_fg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        dad.treeview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.tt_def_fg))
        if dad.curr_tree_iter: dad.update_node_name_header()
    colorbutton_tree_fg.connect('color-set', on_colorbutton_tree_fg_color_set)
    def on_colorbutton_tree_bg_color_set(colorbutton):
        dad.tt_def_bg = "#" + dad.html_handler.rgb_to_24(colorbutton.get_color().to_string()[1:])
        dad.treeview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(dad.tt_def_bg))
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
    def on_radiobutton_node_icon_bullet_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "b"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_bullet.connect('toggled', on_radiobutton_node_icon_bullet_toggled)
    def on_radiobutton_node_icon_none_toggled(radiobutton):
        if not radiobutton.get_active(): return
        dad.nodes_icons = "n"
        dad.treeview_refresh(change_icon=True)
    radiobutton_node_icon_none.connect('toggled', on_radiobutton_node_icon_none_toggled)
    def on_radiobutton_nodes_startup_restore_toggled(checkbutton):
        if checkbutton.get_active(): dad.rest_exp_coll = 0
    radiobutton_nodes_startup_restore.connect('toggled', on_radiobutton_nodes_startup_restore_toggled)
    def on_radiobutton_nodes_startup_expand_toggled(checkbutton):
        if checkbutton.get_active(): dad.rest_exp_coll = 1
    radiobutton_nodes_startup_expand.connect('toggled', on_radiobutton_nodes_startup_expand_toggled)
    def on_radiobutton_nodes_startup_collapse_toggled(checkbutton):
        if checkbutton.get_active(): dad.rest_exp_coll = 2
    radiobutton_nodes_startup_collapse.connect('toggled', on_radiobutton_nodes_startup_collapse_toggled)
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

def preferences_tab_fonts(dad, vbox_fonts):
    """Preferences Dialog, Fonts Tab"""
    for child in vbox_fonts.get_children(): child.destroy()

    image_text = gtk.Image()
    image_text.set_from_stock(gtk.STOCK_SELECT_FONT, gtk.ICON_SIZE_MENU)
    image_code = gtk.Image()
    image_code.set_from_stock(gtk.STOCK_SELECT_FONT, gtk.ICON_SIZE_MENU)
    image_tree = gtk.Image()
    image_tree.set_from_stock('cherries', gtk.ICON_SIZE_MENU)
    label_text = gtk.Label(_("Text Font"))
    label_code = gtk.Label(_("Code Font"))
    label_tree = gtk.Label(_("Tree Font"))
    fontbutton_text = gtk.FontButton(fontname=dad.text_font)
    fontbutton_code = gtk.FontButton(fontname=dad.code_font)
    fontbutton_tree = gtk.FontButton(fontname=dad.tree_font)
    table_fonts = gtk.Table(3, 3)
    table_fonts.set_row_spacings(2)
    table_fonts.set_col_spacings(4)
    table_fonts.attach(image_text, 0, 1, 0, 1, 0, 0)
    table_fonts.attach(image_code, 0, 1, 1, 2, 0, 0)
    table_fonts.attach(image_tree, 0, 1, 2, 3, 0, 0)
    table_fonts.attach(label_text, 1, 2, 0, 1, 0, 0)
    table_fonts.attach(label_code, 1, 2, 1, 2, 0, 0)
    table_fonts.attach(label_tree, 1, 2, 2, 3, 0, 0)
    table_fonts.attach(fontbutton_text, 2, 3, 0, 1, yoptions=0)
    table_fonts.attach(fontbutton_code, 2, 3, 1, 2, yoptions=0)
    table_fonts.attach(fontbutton_tree, 2, 3, 2, 3, yoptions=0)

    frame_fonts = gtk.Frame(label="<b>"+_("Fonts")+"</b>")
    frame_fonts.get_label_widget().set_use_markup(True)
    frame_fonts.set_shadow_type(gtk.SHADOW_NONE)
    align_fonts = gtk.Alignment()
    align_fonts.set_padding(3, 6, 6, 6)
    align_fonts.add(table_fonts)
    frame_fonts.add(align_fonts)

    vbox_fonts.pack_start(frame_fonts, expand=False)
    def on_fontbutton_text_font_set(picker):
        dad.text_font = picker.get_font_name()
        if dad.curr_tree_iter and dad.syntax_highlighting == cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_font(pango.FontDescription(dad.text_font))
    fontbutton_text.connect('font-set', on_fontbutton_text_font_set)
    def on_fontbutton_code_font_set(picker):
        dad.code_font = picker.get_font_name()
        if dad.curr_tree_iter and dad.syntax_highlighting != cons.CUSTOM_COLORS_ID:
            dad.sourceview.modify_font(pango.FontDescription(dad.code_font))
    fontbutton_code.connect('font-set', on_fontbutton_code_font_set)
    def on_fontbutton_tree_font_set(picker):
        dad.tree_font = picker.get_font_name()
        dad.set_treeview_font()
    fontbutton_tree.connect('font-set', on_fontbutton_tree_font_set)

def preferences_tab_links(dad, vbox_links):
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
    hbox_anchor_size = gtk.HBox()
    hbox_anchor_size.set_spacing(4)
    label_anchor_size = gtk.Label(_("Anchor Size"))
    adj_anchor_size = gtk.Adjustment(value=dad.anchor_size, lower=1, upper=1000, step_incr=1)
    spinbutton_anchor_size = gtk.SpinButton(adj_anchor_size)
    spinbutton_anchor_size.set_value(dad.anchor_size)
    hbox_anchor_size.pack_start(label_anchor_size, expand=False)
    hbox_anchor_size.pack_start(spinbutton_anchor_size, expand=False)
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
    def on_spinbutton_anchor_size_value_changed(spinbutton):
        dad.anchor_size = int(spinbutton_anchor_size.get_value())
    spinbutton_anchor_size.connect('value-changed', on_spinbutton_anchor_size_value_changed)
    def on_colorbutton_col_link_webs_color_set(colorbutton):
        dad.col_link_webs = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(dad.window)
    colorbutton_col_link_webs.connect('color-set', on_colorbutton_col_link_webs_color_set)
    def on_colorbutton_col_link_node_color_set(colorbutton):
        dad.col_link_node = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(dad.window)
    colorbutton_col_link_node.connect('color-set', on_colorbutton_col_link_node_color_set)
    def on_colorbutton_col_link_file_color_set(colorbutton):
        dad.col_link_file = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(dad.window)
    colorbutton_col_link_file.connect('color-set', on_colorbutton_col_link_file_color_set)
    def on_colorbutton_col_link_fold_color_set(colorbutton):
        dad.col_link_fold = "#" + colorbutton.get_color().to_string()[1:]
        support.dialog_info_after_restart(dad.window)
    colorbutton_col_link_fold.connect('color-set', on_colorbutton_col_link_fold_color_set)

def preferences_tab_misc(dad, vbox_misc):
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
    vbox_saving.pack_start(hbox_autosave, expand=False)
    vbox_saving.pack_start(checkbutton_autosave_on_quit, expand=False)
    vbox_saving.pack_start(checkbutton_backup_before_saving, expand=False)

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
    checkbutton_reload_doc_last = gtk.CheckButton(_("Reload Document From Last Session"))
    checkbutton_mod_time_sentinel = gtk.CheckButton(_("Reload After External Update to CT* File"))
    vbox_misc_misc.pack_start(checkbutton_newer_version, expand=False)
    vbox_misc_misc.pack_start(checkbutton_reload_doc_last, expand=False)
    vbox_misc_misc.pack_start(checkbutton_mod_time_sentinel, expand=False)

    checkbutton_newer_version.set_active(dad.check_version)
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
            dad.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property(cons.STR_VISIBLE, True)
            checkbutton_start_on_systray.set_sensitive(True)
        else:
            dad.ui.get_widget("/MenuBar/FileMenu/ExitApp").set_property(cons.STR_VISIBLE, False)
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
    checkbutton_backup_before_saving.connect('toggled', on_checkbutton_backup_before_saving_toggled)
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
    for tabs_idx in range(7):
        tabs_vbox_vec.append(gtk.VBox())
        tabs_vbox_vec[-1].set_spacing(3)

    notebook = gtk.Notebook()
    notebook.set_tab_pos(gtk.POS_LEFT)
    notebook.append_page(tabs_vbox_vec[0], gtk.Label(_("All Nodes")))
    notebook.append_page(tabs_vbox_vec[1], gtk.Label(_("Text Nodes")))
    notebook.append_page(tabs_vbox_vec[2], gtk.Label(_("Code Nodes")))
    notebook.append_page(tabs_vbox_vec[3], gtk.Label(_("Tree")))
    notebook.append_page(tabs_vbox_vec[4], gtk.Label(_("Fonts")))
    notebook.append_page(tabs_vbox_vec[5], gtk.Label(_("Links")))
    notebook.append_page(tabs_vbox_vec[6], gtk.Label(_("Miscellaneous")))

    tab_constructor = {
        0: preferences_tab_all_nodes,
        1: preferences_tab_text_nodes,
        2: preferences_tab_code_nodes,
        3: preferences_tab_tree,
        4: preferences_tab_fonts,
        5: preferences_tab_links,
        6: preferences_tab_misc,
        }

    def on_notebook_switch_page(notebook, page, page_num):
        #print "new page", page_num
        tab_constructor[page_num](dad, tabs_vbox_vec[page_num])
        tabs_vbox_vec[page_num].show_all()
    notebook.connect('switch-page', on_notebook_switch_page)

    content_area = dialog.get_content_area()
    content_area.pack_start(notebook)
    content_area.show_all()
    notebook.set_current_page(dad.prefpage)
    dialog.run()
    dad.prefpage = notebook.get_current_page()
    dialog.hide()
