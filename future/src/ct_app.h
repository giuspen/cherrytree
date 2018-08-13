/*
 * ct_app.h
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

#include <unordered_map>
#include <gtkmm.h>
#include "ct_config.h"
#include "main_win.h"


class CTTmp
{
public:
    CTTmp();
    virtual ~CTTmp();
    const gchar* getHiddenDirPath(const std::string& visiblePath);
    const gchar* getHiddenFilePath(const std::string& visiblePath);

protected:
    std::unordered_map<std::string,gchar*> _mapHiddenDirs;
    std::unordered_map<std::string,gchar*> _mapHiddenFiles;
};


class CTApplication: public Gtk::Application
{
protected:
    CTApplication();
    ~CTApplication();

public:
    static Glib::RefPtr<CTApplication> create();

    static CTConfig *P_ct_config;
    static Glib::RefPtr<Gtk::IconTheme> R_icontheme;
    static CTTmp* P_ctTmp;

protected:
    void on_activate() override;
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;

    void _print_help_message();
    void _print_gresource_icons();
    void _icontheme_populate();

private:
    MainWindow* create_appwindow();
};
