/*
 * main.cc
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

#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <iostream>
#include "main_win.h"


class CTApplication: public Gtk::Application
{
protected:
    CTApplication();

public:
    static Glib::RefPtr<CTApplication> create();

protected:
    void on_activate() override;
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;

private:
    MainWindow* create_appwindow();
};


static void _print_help_message()
{
    std::cout << "Usage: " << GETTEXT_PACKAGE << " filepath[.ctd|.ctb]" << std::endl;
}


CTApplication::CTApplication() : Gtk::Application("com.giuspen.cherrytree", Gio::APPLICATION_HANDLES_OPEN)
{
}


Glib::RefPtr<CTApplication> CTApplication::create()
{
    return Glib::RefPtr<CTApplication>(new CTApplication());
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


int main(int argc, char *argv[])
{
    std::locale::global(std::locale("")); // Set the global C++ locale to the user-specified locale

    bindtextdomain(GETTEXT_PACKAGE, CHERRYTREE_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    auto p_app = CTApplication::create();

    return p_app->run(argc, argv);
}
