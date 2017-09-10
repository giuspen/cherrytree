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

// https://developer.gnome.org/glibmm/stable/classGlib_1_1KeyFile.html
// https://developer.gnome.org/glibmm/stable/group__MiscUtils.html
// g++ ct_config.cc -o ct_config `pkg-config gtkmm-3.0 --cflags --libs` -Wno-deprecated

#include <assert.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <gtkmm.h>

enum class RestoreExpColl : int {FROM_STR=0, ALL_EXP=1, ALL_COLL=2};

struct t_ct_recent_docs_restore
{
    Glib::ustring   doc_name;
    Glib::ustring   exp_coll_str;
    Glib::ustring   node_path;
    int             cursor_pos;
};

const guint8  MAX_RECENT_DOCS = 10;
const guint8  MAX_RECENT_DOCS_RESTORE = 3;
const gchar   LINK_TYPE_WEBS[] = "webs";
const gchar   LINK_TYPE_FILE[] = "file";
const gchar   LINK_TYPE_FOLD[] = "fold";
const gchar   LINK_TYPE_NODE[] = "node";
const gchar   NODE_ICON_TYPE_CHERRY[] = "c";
const gchar   NODE_ICON_TYPE_CUSTOM[] = "b";
const gchar   NODE_ICON_TYPE_NONE[] = "n";
const gchar   CHERRY_RED[] = "cherry_red";
const gchar   CHERRY_BLUE[] = "cherry_blue";
const gchar   CHERRY_ORANGE[] = "cherry_orange";
const gchar   CHERRY_CYAN[] = "cherry_cyan";
const gchar   CHERRY_ORANGE_DARK[] = "cherry_orange_dark";
const gchar   CHERRY_SHERBERT[] = "cherry_sherbert";
const gchar   CHERRY_YELLOW[] = "cherry_yellow";
const gchar   CHERRY_GREEN[] = "cherry_green";
const gchar   CHERRY_PURPLE[] = "cherry_purple";
const gchar   CHERRY_BLACK[] = "cherry_black";
const gchar   CHERRY_GRAY[] = "cherry_gray";
const int NODE_ICON_CODE_ID = 38;
const int NODE_ICON_BULLET_ID = 25;
const int NODE_ICON_NO_ICON_ID = 26;
const std::map<int, Glib::ustring> NODES_STOCKS = {
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

protected:
    void _populate_with_defaults();
    bool _populate_string_from_keyfile(const gchar *key, Glib::ustring *p_target);
    bool _populate_bool_from_keyfile(const gchar *key, bool *p_target);
    bool _populate_int_from_keyfile(const gchar *key, int *p_target);
    void _populate_from_keyfile();
    bool _check_load_from_file();
    void _unexpected_keyfile_error(const gchar *key, const Glib::KeyFileError &kferror);

    Glib::ustring _m_filepath;
    Glib::KeyFile *_mp_key_file;
    Glib::ustring _m_current_group;
};

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

int main(int argc, char *argv[])
{
    CTConfig ct_config;
}
