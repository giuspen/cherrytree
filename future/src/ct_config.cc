/*
 * ct_config.cc
 * 
 * Copyright 2017 giuspen <giuspen@gmail.com>
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

#include <iostream>
#include <gtkmm.h>
#include "ct_config.h"


CTConfig::CTConfig() : _m_filepath(Glib::build_filename(Glib::get_user_config_dir(), "cherrytree", "config.cfg")),
                       _mp_key_file(nullptr)
{
    _populate_with_defaults();
    bool config_found = _check_load_from_file();
    std::cout << _m_filepath << " " << (config_found ? "parsed":"missing") << std::endl;
}


CTConfig::~CTConfig()
{
    if (_mp_key_file != nullptr)
    {
        delete _mp_key_file;
    }
}


void CTConfig::_populate_with_defaults()
{
    // [state]
    m_toolbar_visible = true;
    m_win_is_maximized = false;
    m_win_rect[0] = 10;
    m_win_rect[1] = 10;
    m_win_rect[2] = 963;
    m_win_rect[3] = 630;
    m_hpaned_pos = 170;
    m_link_type = LINK_TYPE_WEBS;
    m_show_node_name_header = true;
    m_nodes_on_node_name_header = 3;
    m_toolbar_icon_size = Gtk::BuiltinIconSize::ICON_SIZE_MENU;
    m_curr_colors['f'] = "";
    m_curr_colors['b'] = "";
    m_curr_colors['n'] = "";

    // [tree]
    m_rest_exp_coll = RestoreExpColl::FROM_STR;
    m_nodes_bookm_exp = false;
    m_nodes_icons = NODE_ICON_TYPE_CHERRY;
    m_aux_icon_hide = false;
    m_default_icon_text = NODE_ICON_BULLET_ID;
    m_tree_right_side = false;
    m_cherry_wrap_width = 130;
    m_tree_click_focus_text = false;
    m_tree_click_expand = false;

    // [editor]
    m_syntax_highlighting = RICH_TEXT_ID;
    m_auto_syn_highl = SYN_HIGHL_BASH;
    m_style_scheme = STYLE_SCHEME_DARK;
    m_enable_spell_check = false;
    m_show_line_numbers = false;
    m_spaces_instead_tabs = true;
    m_tabs_width = 4;
    m_anchor_size = 16;
    m_embfile_size = 48;
    m_embfile_show_filename = true;
    m_embfile_max_size = 10;
    m_line_wrapping = true;
    m_auto_smart_quotes = true;
    m_wrapping_indent = -14;
    m_auto_indent = true;
    m_rt_show_white_spaces = false;
    m_pt_show_white_spaces = true;
    m_rt_highl_curr_line = true;
    m_pt_highl_curr_line = true;
    m_space_around_lines = 0;
    m_relative_wrapped_space = 50;
    m_h_rule = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    m_special_chars = SPECIAL_CHARS_DEFAULT;
    m_selword_chars = SELWORD_CHARS_DEFAULT;
    m_chars_listbul = CHARS_LISTBUL_DEFAULT;
    m_chars_toc = CHARS_TOC_DEFAULT;
    m_timestamp_format = TIMESTAMP_FORMAT_DEFAULT;
    m_links_underline = true;
    m_links_relative = false;
    m_weblink_custom_on = false;
    m_filelink_custom_on = false;
    m_folderlink_custom_on = false;
    m_weblink_custom_act = "firefox %s &";
    m_filelink_custom_act = "xdg-open %s &";
    m_folderlink_custom_act = "xdg-open %s &";

    // [codebox]
    m_codebox_width = 700;
    m_codebox_height = 100;
    m_codebox_width_pixels = true;
    m_codebox_line_num = false;
    m_codebox_match_bra = true;
    m_codebox_syn_highl = PLAIN_TEXT_ID;
    m_codebox_auto_resize = true;

    // [table]
    m_table_rows = 3;
    m_table_columns = 3;
    m_table_col_mode = TableColMode::RENAME;
    m_table_col_min = 40;
    m_table_col_max = 60;

    // [fonts]
    m_text_font = "Sans 9";
    m_tree_font = "Sans 8";
    m_code_font = "Monospace 9";

    // [colors]
    m_rt_def_fg = RICH_TEXT_DARK_FG;
    m_rt_def_bg = RICH_TEXT_DARK_BG;
    m_tt_def_fg = TREE_TEXT_LIGHT_FG;
    m_tt_def_bg = TREE_TEXT_LIGHT_BG;
    m_monospace_bg = DEFAULT_MONOSPACE_BG;
    m_color_palette = 
        "#000000:#ffffff:#7f7f7f:#ff0000:#a020f0:"
        "#0000ff:#add8e6:#00ff00:#ffff00:#ffa500:"
        "#e6e6fa:#a52a2a:#8b6914:#1e90ff:#ffc0cb:"
        "#90ee90:#1a1a1a:#4d4d4d:#bfbfbf:#e5e5e5";
    m_col_link_webs = COLOR_48_LINK_WEBS;
    m_col_link_node = COLOR_48_LINK_NODE;
    m_col_link_file = COLOR_48_LINK_FILE;
    m_col_link_fold = COLOR_48_LINK_FOLD;

    // [misc]
    m_toolbar_ui_list = TOOLBAR_VEC_DEFAULT;
    m_systray = false;
    m_start_on_systray = false;
    m_use_appind = false;
    m_autosave_on = false;
    m_autosave_val = 5;
    m_check_version = false;
    m_word_count = false;
    m_reload_doc_last = true;
    m_mod_time_sentinel = true;
    m_backup_copy = true;
    m_backup_num = 3;
    m_autosave_on_quit = false;
    m_limit_undoable_steps = 20;
}


bool CTConfig::_populate_string_from_keyfile(const gchar *key, Glib::ustring *p_target)
{
    bool got_it = false;
    if (_mp_key_file->has_key(_m_current_group, key))
    {
        try
        {
            *p_target = _mp_key_file->get_value(_m_current_group, key);
            got_it = true;
        }
        catch (Glib::KeyFileError &kferror)
        {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return got_it;
}


bool CTConfig::_populate_bool_from_keyfile(const gchar *key, bool *p_target)
{
    bool got_it = false;
    if (_mp_key_file->has_key(_m_current_group, key))
    {
        try
        {
            *p_target = _mp_key_file->get_boolean(_m_current_group, key);
            got_it = true;
        }
        catch (Glib::KeyFileError &kferror)
        {
            if (kferror.code() == Glib::KeyFileError::Code::INVALID_VALUE)
            {
                // booleans from python ConfigParser
                Glib::ustring bool_str = _mp_key_file->get_value(_m_current_group, key);
                *p_target = (bool_str == "True");
                got_it = true;
            }
            else
            {
                _unexpected_keyfile_error(key, kferror);
            }
        }
    }
    return got_it;
}


bool CTConfig::_populate_int_from_keyfile(const gchar *key, int *p_target)
{
    bool got_it = false;
    if (_mp_key_file->has_key(_m_current_group, key))
    {
        try
        {
            *p_target = _mp_key_file->get_integer(_m_current_group, key);
            got_it = true;
        }
        catch (Glib::KeyFileError &kferror)
        {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return got_it;
}


bool CTConfig::_populate_double_from_keyfile(const gchar *key, double *p_target)
{
    bool got_it = false;
    if (_mp_key_file->has_key(_m_current_group, key))
    {
        try
        {
            *p_target = _mp_key_file->get_double(_m_current_group, key);
            got_it = true;
        }
        catch (Glib::KeyFileError &kferror)
        {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return got_it;
}


void CTConfig::_populate_map_from_current_group(std::map<Glib::ustring, Glib::ustring> *p_map)
{
    for (Glib::ustring key : _mp_key_file->get_keys(_m_current_group))
    {
        (*p_map)[key] = _mp_key_file->get_value(_m_current_group, key);
    }
}


void CTConfig::_unexpected_keyfile_error(const gchar *key, const Glib::KeyFileError &kferror)
{
    std::cerr << "!! " << key << " error code " << kferror.code() << std::endl;
}


void CTConfig::_populate_from_keyfile()
{
    guint8  i;
    #define MAX_TEMP_KEY_SIZE 16
    gchar   temp_key[MAX_TEMP_KEY_SIZE];
    // [state]
    _m_current_group = "state";
    _populate_string_from_keyfile("file_dir", &m_file_dir);
    _populate_string_from_keyfile("file_name", &m_file_name);
    _populate_bool_from_keyfile("toolbar_visible", &m_toolbar_visible);
    _populate_bool_from_keyfile("win_is_maximized", &m_win_is_maximized);
    _populate_int_from_keyfile("win_position_x", &m_win_rect[0]);
    _populate_int_from_keyfile("win_position_y", &m_win_rect[1]);
    _populate_int_from_keyfile("win_size_w", &m_win_rect[2]);
    _populate_int_from_keyfile("win_size_h", &m_win_rect[3]);
    _populate_int_from_keyfile("hpaned_pos", &m_hpaned_pos);
    if (_populate_string_from_keyfile("node_path", &m_node_path))
    {
        _populate_int_from_keyfile("cursor_position", &m_cursor_position);
    }
    for (i=0; i<MAX_RECENT_DOCS; i++)
    {
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "doc_%d", i);
        Glib::ustring recent_doc;
        if (!_populate_string_from_keyfile(temp_key, &recent_doc))
        {
            break;
        }
        m_recent_docs.push_back(recent_doc);
    }
    _populate_string_from_keyfile("pick_dir_import", &m_pick_dir_import);
    _populate_string_from_keyfile("pick_dir_export", &m_pick_dir_export);
    _populate_string_from_keyfile("pick_dir_file", &m_pick_dir_file);
    _populate_string_from_keyfile("pick_dir_img", &m_pick_dir_img);
    _populate_string_from_keyfile("pick_dir_csv", &m_pick_dir_csv);
    _populate_string_from_keyfile("pick_dir_cbox", &m_pick_dir_cbox);
    _populate_string_from_keyfile("link_type", &m_link_type);
    _populate_bool_from_keyfile("show_node_name_header", &m_show_node_name_header);
    _populate_int_from_keyfile("nodes_on_node_name_header", &m_nodes_on_node_name_header);
    _populate_int_from_keyfile("toolbar_icon_size", &m_toolbar_icon_size);
    _populate_string_from_keyfile("fg", &m_curr_colors['f']);
    _populate_string_from_keyfile("bg", &m_curr_colors['b']);
    _populate_string_from_keyfile("nn", &m_curr_colors['n']);

    // [tree]
    _m_current_group = "tree";
    int rest_exp_coll;
    if (_populate_int_from_keyfile("rest_exp_coll", &rest_exp_coll))
    {
        m_rest_exp_coll = static_cast<RestoreExpColl>(rest_exp_coll);
    }
    _populate_string_from_keyfile("expanded_collapsed_string", &m_expanded_collapsed_string);
    for (i=0; i<=MAX_RECENT_DOCS_RESTORE; i++)
    {
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollnam%d", i+1);
        if (!_populate_string_from_keyfile(temp_key, &m_recent_docs_restore[i].doc_name))
        {
            break;
        }
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollstr%d", i+1);
        _populate_string_from_keyfile(temp_key, &m_recent_docs_restore[i].exp_coll_str);
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollsel%d", i+1);
        _populate_string_from_keyfile(temp_key, &m_recent_docs_restore[i].node_path);
        snprintf(temp_key, MAX_TEMP_KEY_SIZE, "expcollcur%d", i+1);
        _populate_int_from_keyfile(temp_key, &m_recent_docs_restore[i].cursor_pos);
    }
    _populate_bool_from_keyfile("nodes_bookm_exp", &m_nodes_bookm_exp);
    _populate_string_from_keyfile("nodes_icons", &m_nodes_icons);
    _populate_bool_from_keyfile("aux_icon_hide", &m_aux_icon_hide);
    _populate_int_from_keyfile("default_icon_text", &m_default_icon_text);
    _populate_bool_from_keyfile("tree_right_side", &m_tree_right_side);
    _populate_int_from_keyfile("cherry_wrap_width", &m_cherry_wrap_width);
    _populate_bool_from_keyfile("tree_click_focus_text", &m_tree_click_focus_text);
    _populate_bool_from_keyfile("tree_click_expand", &m_tree_click_expand);

    // [editor]
    _m_current_group = "editor";
    _populate_string_from_keyfile("syntax_highlighting", &m_syntax_highlighting);
    _populate_string_from_keyfile("auto_syn_highl", &m_auto_syn_highl);
    _populate_string_from_keyfile("style_scheme", &m_style_scheme);
    if (_populate_bool_from_keyfile("enable_spell_check", &m_enable_spell_check))
    {
        _populate_string_from_keyfile("spell_check_lang", &m_spell_check_lang);
    }
    _populate_bool_from_keyfile("show_line_numbers", &m_show_line_numbers);
    _populate_bool_from_keyfile("spaces_instead_tabs", &m_spaces_instead_tabs);
    _populate_int_from_keyfile("tabs_width", &m_tabs_width);
    _populate_int_from_keyfile("anchor_size", &m_anchor_size);
    _populate_int_from_keyfile("embfile_size", &m_embfile_size);
    _populate_bool_from_keyfile("embfile_show_filename", &m_embfile_show_filename);
    _populate_int_from_keyfile("embfile_max_size", &m_embfile_max_size);
    _populate_bool_from_keyfile("line_wrapping", &m_line_wrapping);
    _populate_bool_from_keyfile("auto_smart_quotes", &m_auto_smart_quotes);
    _populate_int_from_keyfile("wrapping_indent", &m_wrapping_indent);
    _populate_bool_from_keyfile("auto_indent", &m_auto_indent);
    _populate_bool_from_keyfile("rt_show_white_spaces", &m_rt_show_white_spaces);
    _populate_bool_from_keyfile("pt_show_white_spaces", &m_pt_show_white_spaces);
    _populate_bool_from_keyfile("rt_highl_curr_line", &m_rt_highl_curr_line);
    _populate_bool_from_keyfile("pt_highl_curr_line", &m_pt_highl_curr_line);
    _populate_int_from_keyfile("space_around_lines", &m_space_around_lines);
    _populate_int_from_keyfile("relative_wrapped_space", &m_relative_wrapped_space);
    _populate_string_from_keyfile("h_rule", &m_h_rule);
    _populate_string_from_keyfile("special_chars", &m_special_chars);
    _populate_string_from_keyfile("selword_chars", &m_selword_chars);
    _populate_string_from_keyfile("chars_listbul", &m_chars_listbul);
    _populate_string_from_keyfile("chars_toc", &m_chars_toc);
    _populate_string_from_keyfile("latest_tag_prop", &m_latest_tag_prop);
    _populate_string_from_keyfile("latest_tag_val", &m_latest_tag_val);
    _populate_string_from_keyfile("timestamp_format", &m_timestamp_format);
    _populate_bool_from_keyfile("links_underline", &m_links_underline);
    _populate_bool_from_keyfile("links_relative", &m_links_relative);
    _populate_bool_from_keyfile("weblink_custom_on", &m_weblink_custom_on);
    _populate_bool_from_keyfile("filelink_custom_on", &m_filelink_custom_on);
    _populate_bool_from_keyfile("folderlink_custom_on", &m_folderlink_custom_on);
    _populate_string_from_keyfile("weblink_custom_act", &m_weblink_custom_act);
    _populate_string_from_keyfile("filelink_custom_act", &m_filelink_custom_act);
    _populate_string_from_keyfile("folderlink_custom_act", &m_folderlink_custom_act);

    // [codebox]
    _m_current_group = "codebox";
    _populate_double_from_keyfile("codebox_width", &m_codebox_width);
    _populate_double_from_keyfile("codebox_height", &m_codebox_height);
    _populate_bool_from_keyfile("codebox_width_pixels", &m_codebox_width_pixels);
    _populate_bool_from_keyfile("codebox_line_num", &m_codebox_line_num);
    _populate_bool_from_keyfile("codebox_match_bra", &m_codebox_match_bra);
    _populate_string_from_keyfile("codebox_syn_highl", &m_codebox_syn_highl);
    _populate_bool_from_keyfile("codebox_auto_resize", &m_codebox_auto_resize);

    // [table]
    _m_current_group = "table";
    _populate_int_from_keyfile("table_rows", &m_table_rows);
    _populate_int_from_keyfile("table_columns", &m_table_columns);
    int table_col_mode;
    if (_populate_int_from_keyfile("table_col_mode", &table_col_mode))
    {
        m_table_col_mode = static_cast<TableColMode>(table_col_mode);
    }
    _populate_int_from_keyfile("table_col_min", &m_table_col_min);
    _populate_int_from_keyfile("table_col_max", &m_table_col_max);

    // [fonts]
    _m_current_group = "fonts";
    _populate_string_from_keyfile("text_font", &m_text_font);
    _populate_string_from_keyfile("tree_font", &m_tree_font);
    _populate_string_from_keyfile("code_font", &m_code_font);

    // [colors]
    _m_current_group = "colors";
    _populate_string_from_keyfile("rt_def_fg", &m_rt_def_fg);
    _populate_string_from_keyfile("rt_def_bg", &m_rt_def_bg);
    _populate_string_from_keyfile("tt_def_fg", &m_tt_def_fg);
    _populate_string_from_keyfile("tt_def_bg", &m_tt_def_bg);
    _populate_string_from_keyfile("monospace_bg", &m_monospace_bg);
    _populate_string_from_keyfile("color_palette", &m_color_palette);
    _populate_string_from_keyfile("col_link_webs", &m_col_link_webs);
    _populate_string_from_keyfile("col_link_node", &m_col_link_node);
    _populate_string_from_keyfile("col_link_file", &m_col_link_file);
    _populate_string_from_keyfile("col_link_fold", &m_col_link_fold);

    // [misc]
    _m_current_group = "misc";
    _populate_string_from_keyfile("toolbar_ui_list", &m_toolbar_ui_list);
    _populate_bool_from_keyfile("systray", &m_systray);
    _populate_bool_from_keyfile("start_on_systray", &m_start_on_systray);
    _populate_bool_from_keyfile("use_appind", &m_use_appind);
    _populate_bool_from_keyfile("autosave_on", &m_autosave_on);
    _populate_int_from_keyfile("autosave_val", &m_autosave_val);
    _populate_bool_from_keyfile("check_version", &m_check_version);
    _populate_bool_from_keyfile("word_count", &m_word_count);
    _populate_bool_from_keyfile("reload_doc_last", &m_reload_doc_last);
    _populate_bool_from_keyfile("mod_time_sentinel", &m_mod_time_sentinel);
    _populate_bool_from_keyfile("backup_copy", &m_backup_copy);
    _populate_int_from_keyfile("backup_num", &m_backup_num);
    _populate_bool_from_keyfile("autosave_on_quit", &m_autosave_on_quit);
    _populate_int_from_keyfile("limit_undoable_steps", &m_limit_undoable_steps);

    // [keyboard]
    _m_current_group = "keyboard";
    _populate_map_from_current_group(&m_custom_kb_shortcuts);

    // [codexec_term]
    _m_current_group = "codexec_term";
    _populate_string_from_keyfile("custom_codexec_term", &m_custom_codexec_term);

    // [codexec_type]
    _m_current_group = "codexec_type";
    _populate_map_from_current_group(&m_custom_codexec_type);

    // [codexec_ext]
    _m_current_group = "codexec_ext";
    _populate_map_from_current_group(&m_custom_codexec_ext);
}


bool CTConfig::_check_load_from_file()
{
    if (Glib::file_test(_m_filepath, Glib::FILE_TEST_EXISTS))
    {
        _mp_key_file = new Glib::KeyFile();
        _mp_key_file->load_from_file(_m_filepath);
        _populate_from_keyfile();
        return true;
    }
    return false;
}
