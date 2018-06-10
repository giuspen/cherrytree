/*
 * main.cc
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

#include <glibmm/i18n.h>
#include <iostream>
#include "ct_app.h"


CTConfig *P_ct_config = nullptr;

Glib::RefPtr<Gtk::IconTheme> R_icontheme;


CTApplication::CTApplication() : Gtk::Application("com.giuspen.cherrytree", Gio::APPLICATION_HANDLES_OPEN)
{
    _config_read();
    _icontheme_populate();
}


CTApplication::~CTApplication()
{
    _config_teardown();
}


Glib::RefPtr<CTApplication> CTApplication::create()
{
    return Glib::RefPtr<CTApplication>(new CTApplication());
}


void CTApplication::_print_help_message()
{
    std::cout << "Usage: " << GETTEXT_PACKAGE << " filepath[.ctd|.ctb]" << std::endl;
}


void CTApplication::_print_gresource_icons()
{
    for (std::string &str_icon : Gio::Resource::enumerate_children_global("/icons/", Gio::ResourceLookupFlags::RESOURCE_LOOKUP_FLAGS_NONE))
    {
        std::cout << str_icon << std::endl;
    }
}


void CTApplication::_config_read()
{
    if (P_ct_config == nullptr)
    {
        P_ct_config = new CTConfig();
        //std::cout << P_ct_config->m_special_chars.size() << "\t" << P_ct_config->m_special_chars << std::endl;
    }
}


void CTApplication::_config_teardown()
{
    delete P_ct_config;
    P_ct_config = nullptr;
}


void CTApplication::_icontheme_populate()
{
    R_icontheme = Gtk::IconTheme::get_default();
    R_icontheme->add_resource_path("/icons/");
    //_print_gresource_icons();
}


MainWindow* CTApplication::create_appwindow()
{
    auto p_main_win = new MainWindow();

    add_window(*p_main_win);

    return p_main_win;
}


void CTApplication::on_activate()
{
    // app run without arguments
    auto p_appwindow = create_appwindow();
    p_appwindow->present();
}


void CTApplication::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& /* hint */)
{
    // app run with arguments
    MainWindow* p_appwindow = nullptr;
    auto windows_list = get_windows();
    if (windows_list.size() > 0)
    {
        p_appwindow = dynamic_cast<MainWindow*>(windows_list[0]);
    }

    if (!p_appwindow)
    {
        p_appwindow = create_appwindow();
    }

    for (const Glib::RefPtr<Gio::File>& r_file : files)
    {
        Glib::ustring filepath(r_file->get_path());
        if(r_file->query_exists())
        {
            if(!p_appwindow->read_nodes_from_filepath(filepath))
            {
                _print_help_message();
            }
        }
        else
        {
            std::cout << "!! Missing file " << filepath << std::endl;
        }
    }

    p_appwindow->present();
}
