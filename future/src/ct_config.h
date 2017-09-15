/*
 * ct_config.h
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

#pragma once

#include <unordered_map>
#include <glibmm.h>
#include "ct_const.h"


enum class RestoreExpColl : int {FROM_STR=0, ALL_EXP=1, ALL_COLL=2};

enum class TableColMode : int {RENAME=0, ADD=1, DELETE=2, RIGHT=3, LEFT=4};


struct t_ct_recent_docs_restore
{
    Glib::ustring   doc_name;
    Glib::ustring   exp_coll_str;
    Glib::ustring   node_path;
    int             cursor_pos;
};


class CTConfig
{
public:
    CTConfig();
    virtual ~CTConfig();

    // [state]
    Glib::ustring                               m_file_dir;
    Glib::ustring                               m_file_name;
    bool                                        m_toolbar_visible;
    bool                                        m_win_is_maximized;
    int                                         m_win_rect[4];
    int                                         m_hpaned_pos;
    Glib::ustring                               m_node_path;
    int                                         m_cursor_position;
    std::list<Glib::ustring>                    m_recent_docs;
    Glib::ustring                               m_pick_dir_import;
    Glib::ustring                               m_pick_dir_export;
    Glib::ustring                               m_pick_dir_file;
    Glib::ustring                               m_pick_dir_img;
    Glib::ustring                               m_pick_dir_csv;
    Glib::ustring                               m_pick_dir_cbox;
    Glib::ustring                               m_link_type;
    bool                                        m_show_node_name_header;
    int                                         m_nodes_on_node_name_header;
    int                                         m_toolbar_icon_size;
    std::unordered_map<gchar, Glib::ustring>    m_curr_colors;

    // [tree]
    RestoreExpColl                              m_rest_exp_coll;
    Glib::ustring                               m_expanded_collapsed_string;
    t_ct_recent_docs_restore                    m_recent_docs_restore[MAX_RECENT_DOCS_RESTORE];
    bool                                        m_nodes_bookm_exp;
    Glib::ustring                               m_nodes_icons;
    bool                                        m_aux_icon_hide;
    int                                         m_default_icon_text;
    bool                                        m_tree_right_side;
    int                                         m_cherry_wrap_width;
    bool                                        m_tree_click_focus_text;
    bool                                        m_tree_click_expand;

    // [editor]
    Glib::ustring                               m_syntax_highlighting;
    Glib::ustring                               m_auto_syn_highl;
    Glib::ustring                               m_style_scheme;
    bool                                        m_enable_spell_check;
    Glib::ustring                               m_spell_check_lang;
    bool                                        m_show_line_numbers;
    bool                                        m_spaces_instead_tabs;
    int                                         m_tabs_width;
    int                                         m_anchor_size;
    int                                         m_embfile_size;
    bool                                        m_embfile_show_filename;
    int                                         m_embfile_max_size;
    bool                                        m_line_wrapping;
    bool                                        m_auto_smart_quotes;
    int                                         m_wrapping_indent;
    bool                                        m_auto_indent;
    bool                                        m_rt_show_white_spaces;
    bool                                        m_pt_show_white_spaces;
    bool                                        m_rt_highl_curr_line;
    bool                                        m_pt_highl_curr_line;
    int                                         m_space_around_lines;
    int                                         m_relative_wrapped_space;
    Glib::ustring                               m_h_rule;
    Glib::ustring                               m_special_chars;
    Glib::ustring                               m_selword_chars;
    Glib::ustring                               m_chars_listbul;
    Glib::ustring                               m_chars_toc;
    Glib::ustring                               m_latest_tag_prop;
    Glib::ustring                               m_latest_tag_val;
    Glib::ustring                               m_timestamp_format;
    bool                                        m_links_underline;
    bool                                        m_links_relative;
    bool                                        m_weblink_custom_on;
    bool                                        m_filelink_custom_on;
    bool                                        m_folderlink_custom_on;
    Glib::ustring                               m_weblink_custom_act;
    Glib::ustring                               m_filelink_custom_act;
    Glib::ustring                               m_folderlink_custom_act;

    // [codebox]
    double                                      m_codebox_width;
    double                                      m_codebox_height;
    bool                                        m_codebox_width_pixels;
    bool                                        m_codebox_line_num;
    bool                                        m_codebox_match_bra;
    Glib::ustring                               m_codebox_syn_highl;
    bool                                        m_codebox_auto_resize;

    // [table]
    int                                         m_table_rows;
    int                                         m_table_columns;
    TableColMode                                m_table_col_mode;
    int                                         m_table_col_min;
    int                                         m_table_col_max;

    // [fonts]
    Glib::ustring                               m_text_font;
    Glib::ustring                               m_tree_font;
    Glib::ustring                               m_code_font;

    // [colors]
    Glib::ustring                               m_rt_def_fg;
    Glib::ustring                               m_rt_def_bg;
    Glib::ustring                               m_tt_def_fg;
    Glib::ustring                               m_tt_def_bg;
    Glib::ustring                               m_monospace_bg;
    Glib::ustring                               m_color_palette;
    Glib::ustring                               m_col_link_webs;
    Glib::ustring                               m_col_link_node;
    Glib::ustring                               m_col_link_file;
    Glib::ustring                               m_col_link_fold;

    // [misc]
    Glib::ustring                               m_toolbar_ui_list;
    bool                                        m_systray;
    bool                                        m_start_on_systray;
    bool                                        m_use_appind;
    bool                                        m_autosave_on;
    int                                         m_autosave_val;
    bool                                        m_check_version;
    bool                                        m_word_count;
    bool                                        m_reload_doc_last;
    bool                                        m_mod_time_sentinel;
    bool                                        m_backup_copy;
    int                                         m_backup_num;
    bool                                        m_autosave_on_quit;
    int                                         m_limit_undoable_steps;

    // [keyboard]
    std::map<Glib::ustring, Glib::ustring>     m_custom_kb_shortcuts;

    // [codexec_term]
    Glib::ustring                              m_custom_codexec_term;

    // [codexec_type]
    std::map<Glib::ustring, Glib::ustring>     m_custom_codexec_type;

    // [codexec_ext]
    std::map<Glib::ustring, Glib::ustring>     m_custom_codexec_ext;

protected:
    void _populate_with_defaults();
    bool _populate_string_from_keyfile(const gchar *key, Glib::ustring *p_target);
    bool _populate_bool_from_keyfile(const gchar *key, bool *p_target);
    bool _populate_int_from_keyfile(const gchar *key, int *p_target);
    bool _populate_double_from_keyfile(const gchar *key, double *p_target);
    void _populate_map_from_current_group(std::map<Glib::ustring, Glib::ustring> *p_target);
    void _populate_from_keyfile();
    bool _check_load_from_file();
    void _unexpected_keyfile_error(const gchar *key, const Glib::KeyFileError &kferror);

    Glib::ustring _m_filepath;
    Glib::KeyFile *_mp_key_file;
    Glib::ustring _m_current_group;
};
