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
// g++ ct_config.cc -o ct_config `pkg-config glibmm-2.4 --cflags --libs` -Wno-deprecated

#include <assert.h>
#include <iostream>
#include <glibmm.h>

class CTConfig
{
public:
    CTConfig();
    virtual ~CTConfig();

    Glib::ustring m_file_dir;
    Glib::ustring m_file_name;
    bool          m_toolbar_visible;
    bool          m_win_is_maximized;

protected:
    void _populate_with_defaults();
    void _populate_string_from_keyfile(Glib::ustring &group, const gchar *key, Glib::ustring *p_target);
    void _populate_bool_from_keyfile(Glib::ustring &group, const gchar *key, bool *p_target);
    void _populate_from_keyfile();
    bool _check_load_from_file();
    Glib::ustring m_filepath;
    Glib::KeyFile *mp_key_file;
};

CTConfig::CTConfig() : m_filepath(Glib::build_filename(Glib::get_user_config_dir(), "cherrytree", "config.cfg")),
                       mp_key_file(nullptr)
{
    _populate_with_defaults();
    std::cout << m_filepath << " " << (_check_load_from_file() ? "parsed":"missing") << std::endl;
}

CTConfig::~CTConfig()
{
    if (mp_key_file != nullptr)
    {
        delete mp_key_file;
    }
}

void CTConfig::_populate_with_defaults()
{
    m_toolbar_visible = true;
    m_win_is_maximized = false;
}

void CTConfig::_populate_string_from_keyfile(Glib::ustring &group, const gchar *key, Glib::ustring *p_target)
{
    if (mp_key_file->has_key(group, key))
    {
        try
        {
            *p_target = mp_key_file->get_string(group, key);
        }
        catch (Glib::KeyFileError kferror)
        {
            std::cerr << key << " error code " << kferror.code() << std::endl;
        }
    }
}

void CTConfig::_populate_bool_from_keyfile(Glib::ustring &group, const gchar *key, bool *p_target)
{
    if (mp_key_file->has_key(group, key))
    {
        try
        {
            *p_target = mp_key_file->get_boolean(group, key);
        }
        catch (Glib::KeyFileError kferror)
        {
            if (kferror.code() == Glib::KeyFileError::Code::INVALID_VALUE)
            {
                // booleans from python ConfigParser
                Glib::ustring bool_str = mp_key_file->get_string(group, key);
                *p_target = (bool_str == "True");
            }
            else
            {
                std::cerr << key << " error code " << kferror.code() << std::endl;
            }
        }
    }
}

void CTConfig::_populate_from_keyfile()
{
    Glib::ustring curr_group("state");
    _populate_string_from_keyfile(curr_group, "file_dir", &m_file_dir);
    _populate_string_from_keyfile(curr_group, "file_name", &m_file_name);
    _populate_bool_from_keyfile(curr_group, "toolbar_visible", &m_toolbar_visible);
    _populate_bool_from_keyfile(curr_group, "win_is_maximized", &m_win_is_maximized);
}

bool CTConfig::_check_load_from_file()
{
    if (Glib::file_test(m_filepath, Glib::FILE_TEST_EXISTS))
    {
        mp_key_file = new Glib::KeyFile();
        mp_key_file->load_from_file(m_filepath);
        _populate_from_keyfile();
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    CTConfig ct_config = CTConfig();
}
