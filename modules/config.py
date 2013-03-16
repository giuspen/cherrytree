# -*- coding: UTF-8 -*-
#
#       config.py
#
#       Copyright 2009-2013 Giuseppe Penone <giuspen@gmail.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
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

import os, sys, ConfigParser, gtk, pango, subprocess, base64
import cons

ICONS_SIZE = {1: gtk.ICON_SIZE_MENU, 2: gtk.ICON_SIZE_SMALL_TOOLBAR, 3: gtk.ICON_SIZE_LARGE_TOOLBAR,
              4: gtk.ICON_SIZE_DND, 5: gtk.ICON_SIZE_DIALOG}

LINK_CUSTOM_ACTION_DEFAULT_WEB = "firefox %s &"
LINK_CUSTOM_ACTION_DEFAULT_FILE = "xdg-open %s &"
HORIZONTAL_RULE = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
COLOR_PALETTE_DEFAULT = ["#000000", "#FFFFFF", "#7F7F7F", "#FF0000", "#A020F0",
                         "#0000FF", "#ADD8E6", "#00FF00", "#FFFF00", "#FFA500",
                         "#E6E6FA", "#A52A2A", "#8B6914", "#1E90FF", "#FFC0CB",
                         "#90EE90", "#1A1A1A", "#4D4D4D", "#BFBFBF", "#E5E5E5"]

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
        inst.link_type = config.get(section, "link_type") if config.has_option(section, "link_type") else "webs"
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
        inst.show_line_numbers = config.getboolean(section, "show_line_numbers") if config.has_option(section, "show_line_numbers") else False
        inst.spaces_instead_tabs = config.getboolean(section, "spaces_instead_tabs") if config.has_option(section, "spaces_instead_tabs") else True
        inst.tabs_width = config.getint(section, "tabs_width") if config.has_option(section, "tabs_width") else 4
        inst.anchor_size = config.getint(section, "anchor_size") if config.has_option(section, "anchor_size") else 16
        inst.line_wrapping = config.getboolean(section, "line_wrapping") if config.has_option(section, "line_wrapping") else True
        inst.auto_indent = config.getboolean(section, "auto_indent") if config.has_option(section, "auto_indent") else True
        inst.show_white_spaces = config.getboolean(section, "show_white_spaces") if config.has_option(section, "show_white_spaces") else True
        inst.highl_curr_line = config.getboolean(section, "highl_curr_line") if config.has_option(section, "highl_curr_line") else True
        inst.h_rule = config.get(section, "h_rule") if config.has_option(section, "h_rule") else HORIZONTAL_RULE
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
            inst.glade.spinbutton_codebox_width.set_value(config.getfloat(section, "codebox_width"))
        else: inst.glade.spinbutton_codebox_width.set_value(700)
        if config.has_option(section, "codebox_height"):
            inst.glade.spinbutton_codebox_height.set_value(config.getfloat(section, "codebox_height"))
        else: inst.glade.spinbutton_codebox_height.set_value(100)
        if config.has_option(section, "codebox_width_pixels"):
            inst.glade.radiobutton_codebox_pixels.set_active(config.getboolean(section, "codebox_width_pixels"))
            inst.glade.radiobutton_codebox_percent.set_active(not config.getboolean(section, "codebox_width_pixels"))
        
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
        
        section = "misc"
        inst.systray = config.getboolean(section, "systray") if config.has_option(section, "systray") else False
        inst.start_on_systray = config.getboolean(section, "start_on_systray") if config.has_option(section, "start_on_systray") else False
        inst.use_appind = config.getboolean(section, "use_appind") if config.has_option(section, "use_appind") else False
        if config.has_option(section, "autosave") and config.has_option(section, "autosave_val"):
            inst.autosave = [config.getboolean(section, "autosave"), config.getint(section, "autosave_val")]
        else: inst.autosave = [False, 5]
        inst.check_version = config.getboolean(section, "check_version") if config.has_option(section, "check_version") else False
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
        inst.h_rule = HORIZONTAL_RULE
        inst.show_line_numbers = False
        inst.spaces_instead_tabs = True
        inst.tabs_width = 4
        inst.anchor_size = 16
        inst.line_wrapping = True
        inst.auto_indent = True
        inst.systray = False
        inst.win_position = [10, 10]
        inst.autosave = [False, 5]
        inst.win_is_maximized = False
        inst.rest_exp_coll = 0
        inst.expanded_collapsed_string = ""
        inst.pick_dir = ""
        inst.link_type = "webs"
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
        inst.glade.spinbutton_codebox_width.set_value(700)
        inst.glade.spinbutton_codebox_height.set_value(100)
        inst.check_version = False
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
    if inst.user_active:
        inst.user_active = False
        user_active_restore = True
    else: user_active_restore = False
    # treeview
    inst.hpaned.set_property('position', inst.hpaned_pos)
    inst.header_node_name_label.set_property(cons.STR_VISIBLE, inst.show_node_name_label)
    inst.set_treeview_font()
    inst.glade.fontbutton_tree.set_font_name(inst.tree_font)
    # sourceview
    inst.glade.fontbutton_text.set_font_name(inst.text_font)
    inst.glade.fontbutton_code.set_font_name(inst.code_font)
    inst.glade.colorbutton_text_fg.set_color(gtk.gdk.color_parse(inst.rt_def_fg))
    inst.glade.colorbutton_text_bg.set_color(gtk.gdk.color_parse(inst.rt_def_bg))
    inst.glade.colorbutton_tree_fg.set_color(gtk.gdk.color_parse(inst.tt_def_fg))
    inst.glade.colorbutton_tree_bg.set_color(gtk.gdk.color_parse(inst.tt_def_bg))
    if inst.rt_def_fg == cons.RICH_TEXT_DARK_FG and inst.rt_def_bg == cons.RICH_TEXT_DARK_BG:
        inst.glade.radiobutton_rt_col_dark.set_active(True)
        inst.glade.colorbutton_text_fg.set_sensitive(False)
        inst.glade.colorbutton_text_bg.set_sensitive(False)
    elif inst.rt_def_fg == cons.RICH_TEXT_LIGHT_FG and inst.rt_def_bg == cons.RICH_TEXT_LIGHT_BG:
        inst.glade.radiobutton_rt_col_light.set_active(True)
        inst.glade.colorbutton_text_fg.set_sensitive(False)
        inst.glade.colorbutton_text_bg.set_sensitive(False)
    else: inst.glade.radiobutton_rt_col_custom.set_active(True)
    if inst.tt_def_fg == cons.TREE_TEXT_DARK_FG and inst.tt_def_bg == cons.TREE_TEXT_DARK_BG:
        inst.glade.radiobutton_tt_col_dark.set_active(True)
        inst.glade.colorbutton_tree_fg.set_sensitive(False)
        inst.glade.colorbutton_tree_bg.set_sensitive(False)
    elif inst.tt_def_fg == cons.TREE_TEXT_LIGHT_FG and inst.tt_def_bg == cons.TREE_TEXT_LIGHT_BG:
        inst.glade.radiobutton_tt_col_light.set_active(True)
        inst.glade.colorbutton_tree_fg.set_sensitive(False)
        inst.glade.colorbutton_tree_bg.set_sensitive(False)
    else: inst.glade.radiobutton_tt_col_custom.set_active(True)
    inst.treeview.modify_base(gtk.STATE_NORMAL, gtk.gdk.color_parse(inst.tt_def_bg))
    inst.treeview.modify_text(gtk.STATE_NORMAL, gtk.gdk.color_parse(inst.tt_def_fg))
    inst.glade.entry_horizontal_rule.set_text(inst.h_rule)
    inst.sourceview.set_show_line_numbers(inst.show_line_numbers)
    inst.glade.checkbutton_line_nums.set_active(inst.show_line_numbers)
    inst.sourceview.set_insert_spaces_instead_of_tabs(inst.spaces_instead_tabs)
    inst.glade.checkbutton_spaces_tabs.set_active(inst.spaces_instead_tabs)
    inst.sourceview.set_tab_width(inst.tabs_width)
    inst.glade.spinbutton_tab_width.set_value(inst.tabs_width)
    inst.glade.spinbutton_anchor_size.set_value(inst.anchor_size)
    if inst.line_wrapping: inst.sourceview.set_wrap_mode(gtk.WRAP_WORD)
    else: inst.sourceview.set_wrap_mode(gtk.WRAP_NONE)
    inst.glade.checkbutton_line_wrap.set_active(inst.line_wrapping)
    inst.sourceview.set_auto_indent(inst.auto_indent)
    inst.glade.checkbutton_auto_indent.set_active(inst.auto_indent)
    inst.glade.checkbutton_systray.set_active(inst.systray)
    inst.glade.checkbutton_use_appind.set_active(inst.use_appind)
    inst.glade.spinbutton_autosave.set_value(inst.autosave[1])
    inst.glade.spinbutton_autosave.set_sensitive(inst.autosave[0])
    inst.glade.checkbutton_autosave.set_active(inst.autosave[0])
    inst.glade.radiobutton_nodes_startup_restore.set_active(inst.rest_exp_coll == 0)
    inst.glade.radiobutton_nodes_startup_expand.set_active(inst.rest_exp_coll == 1)
    inst.glade.radiobutton_nodes_startup_collapse.set_active(inst.rest_exp_coll == 2)
    inst.glade.checkbutton_newer_version.set_active(inst.check_version)
    inst.glade.checkbutton_mod_time_sentinel.set_active(inst.enable_mod_time_sentinel)
    inst.glade.checkbutton_backup_before_saving.set_active(inst.backup_copy)
    inst.glade.checkbutton_autosave_on_quit.set_active(inst.autosave_on_quit)
    inst.glade.checkbutton_tree_right_side.set_active(inst.tree_right_side)
    inst.glade.checkbutton_show_white_spaces.set_active(inst.show_white_spaces)
    inst.glade.checkbutton_highlight_current_line.set_active(inst.highl_curr_line)
    inst.glade.checkbutton_start_on_systray.set_active(inst.start_on_systray)
    inst.glade.checkbutton_start_on_systray.set_sensitive(inst.systray)
    # custom link clicked actions
    inst.glade.checkbutton_custom_weblink_cmd.set_active(inst.weblink_custom_action[0])
    inst.glade.entry_custom_weblink_cmd.set_sensitive(inst.weblink_custom_action[0])
    inst.glade.entry_custom_weblink_cmd.set_text(inst.weblink_custom_action[1])
    inst.glade.checkbutton_custom_filelink_cmd.set_active(inst.filelink_custom_action[0])
    inst.glade.entry_custom_filelink_cmd.set_sensitive(inst.filelink_custom_action[0])
    inst.glade.entry_custom_filelink_cmd.set_text(inst.filelink_custom_action[1])
    inst.glade.checkbutton_custom_folderlink_cmd.set_active(inst.folderlink_custom_action[0])
    inst.glade.entry_custom_folderlink_cmd.set_sensitive(inst.folderlink_custom_action[0])
    inst.glade.entry_custom_folderlink_cmd.set_text(inst.folderlink_custom_action[1])
    inst.glade.entry_timestamp_format.set_text(inst.timestamp_format)
    #
    inst.glade.radiobutton_link_website.set_active(inst.link_type == "webs")
    inst.glade.radiobutton_link_node_anchor.set_active(inst.link_type == "node")
    inst.glade.radiobutton_link_file.set_active(inst.link_type == "file")
    inst.glade.table_column_rename_radiobutton.set_active(inst.table_column_mode == "rename")
    inst.glade.table_column_delete_radiobutton.set_active(inst.table_column_mode == "delete")
    inst.glade.table_column_add_radiobutton.set_active(inst.table_column_mode == "add")
    inst.glade.table_column_left_radiobutton.set_active(inst.table_column_mode == cons.TAG_PROP_LEFT)
    inst.glade.table_column_right_radiobutton.set_active(inst.table_column_mode == cons.TAG_PROP_RIGHT)
    inst.glade.radiobutton_node_icon_cherry.set_active(inst.nodes_icons == "c")
    inst.glade.radiobutton_node_icon_bullet.set_active(inst.nodes_icons == "b")
    inst.glade.radiobutton_node_icon_none.set_active(inst.nodes_icons == "n")
    inst.glade.spinbutton_table_rows.set_value(inst.table_rows)
    inst.glade.spinbutton_table_columns.set_value(inst.table_columns)
    inst.glade.spinbutton_table_col_min.set_value(inst.table_col_min)
    inst.glade.spinbutton_table_col_max.set_value(inst.table_col_max)
    inst.glade.spinbutton_limit_undoable_steps.set_value(inst.limit_undoable_steps)
    inst.glade.spinbutton_tree_nodes_names_width.set_value(inst.cherry_wrap_width)
    inst.renderer_text.set_property('wrap-width', inst.cherry_wrap_width)
    inst.ui.get_widget("/ToolBar").set_property(cons.STR_VISIBLE, inst.toolbar_visible)
    inst.ui.get_widget("/ToolBar").set_style(gtk.TOOLBAR_ICONS)
    inst.ui.get_widget("/ToolBar").set_property("icon-size", ICONS_SIZE[inst.toolbar_icon_size])
    if inst.autosave[0]: inst.autosave_timer_start()
    if user_active_restore: inst.user_active = True

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
    config.set(section, "show_line_numbers", inst.show_line_numbers)
    config.set(section, "spaces_instead_tabs", inst.spaces_instead_tabs)
    config.set(section, "tabs_width", inst.tabs_width)
    config.set(section, "anchor_size", inst.anchor_size)
    config.set(section, "line_wrapping", inst.line_wrapping)
    config.set(section, "auto_indent", inst.auto_indent)
    config.set(section, "show_white_spaces", inst.show_white_spaces)
    config.set(section, "highl_curr_line", inst.highl_curr_line)
    config.set(section, "h_rule", inst.h_rule)
    config.set(section, "timestamp_format", inst.timestamp_format)
    config.set(section, "weblink_custom_action", str(inst.weblink_custom_action[0])+inst.weblink_custom_action[1])
    config.set(section, "filelink_custom_action", str(inst.filelink_custom_action[0])+inst.filelink_custom_action[1])
    config.set(section, "folderlink_custom_action", str(inst.folderlink_custom_action[0])+inst.folderlink_custom_action[1])
    
    section = "codebox"
    config.add_section(section)
    config.set(section, "codebox_width", inst.glade.spinbutton_codebox_width.get_value())
    config.set(section, "codebox_height", inst.glade.spinbutton_codebox_height.get_value())
    config.set(section, "codebox_width_pixels", inst.glade.radiobutton_codebox_pixels.get_active())
    
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
    
    section = "misc"
    config.add_section(section)
    config.set(section, "systray", inst.systray)
    config.set(section, "start_on_systray", inst.start_on_systray)
    config.set(section, "use_appind", inst.use_appind)
    config.set(section, "autosave", inst.autosave[0])
    config.set(section, "autosave_val", inst.autosave[1])
    config.set(section, "check_version", inst.check_version)
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
