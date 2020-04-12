/*
 * ct_app.cc
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <glib/gstdio.h>
#include "ct_app.h"
#include "ct_pref_dlg.h"
#include "ct_storage_control.h"
#include "config.h"

CtApp::CtApp() : Gtk::Application("com.giuspen.cherrytree", Gio::APPLICATION_HANDLES_OPEN)
{
    Gsv::init();

    _uCtCfg.reset(new CtConfig());
    //std::cout << _uCtCfg->specialChars.size() << "\t" << _uCtCfg->specialChars << std::endl;

    _uCtActions.reset(new CtActions());

    _rIcontheme = Gtk::IconTheme::get_default();
    _rIcontheme->add_resource_path("/icons/");
    //_printGresourceIcons();

    _uCtTmp.reset(new CtTmp());
    //std::cout << _uCtTmp->get_root_dirpath() << std::endl;

    _rTextTagTable = Gtk::TextTagTable::create();

    _rLanguageManager = Gsv::LanguageManager::create();

    _rStyleSchemeManager = Gsv::StyleSchemeManager::create();

    _rCssProvider = Gtk::CssProvider::create();

    _uCtMenu.reset(new CtMenu(_uCtCfg.get()));
    _uCtMenu->init_actions(this, _uCtActions.get());

    _uCtPrint.reset(new CtPrint());
}

CtApp::~CtApp()
{
    //std::cout << "~CtApp()" << std::endl;
}

Glib::RefPtr<CtApp> CtApp::create()
{
    return Glib::RefPtr<CtApp>(new CtApp());
}

void CtApp::_printHelpMessage()
{
    std::cout << "Usage: " << GETTEXT_PACKAGE << " [filepath.ctd|.ctb|.ctz|.ctx]" << std::endl;
}

void CtApp::_printGresourceIcons()
{
    for (const std::string& str_icon : Gio::Resource::enumerate_children_global("/icons/", Gio::ResourceLookupFlags::RESOURCE_LOOKUP_FLAGS_NONE))
    {
        std::cout << str_icon << std::endl;
    }
}

void CtApp::file_new()
{
    _create_appwindow()->present();
}

CtMainWin* CtApp::_create_appwindow()
{
    CtMainWin* pCtMainWin = new CtMainWin(_uCtCfg.get(),
                                          _uCtActions.get(),
                                          _uCtTmp.get(),
                                          _uCtMenu.get(),
                                          _uCtPrint.get(),
                                          _rIcontheme.get(),
                                          _rTextTagTable,
                                          _rCssProvider,
                                          _rLanguageManager.get(),
                                          _rStyleSchemeManager.get());
    CtApp::_uCtActions->init(pCtMainWin);

    add_window(*pCtMainWin);

    pCtMainWin->signal_hide().connect(sigc::bind<CtMainWin*>(sigc::mem_fun(*this, &CtApp::_on_hide_window), pCtMainWin));
    return pCtMainWin;
}

CtMainWin* CtApp::_get_main_win(const std::string& filepath)
{
    // 1) look for exact filepath match
    for (Gtk::Window* pWin : get_windows())
    {
        CtMainWin* pCtMainWin = dynamic_cast<CtMainWin*>(pWin);
        if (filepath == pCtMainWin->get_ct_storage()->get_file_path())
        {
            return pCtMainWin;
        }
    }
    // 2) look for window with no loaded document
    for (Gtk::Window* pWin : get_windows())
    {
        CtMainWin* pCtMainWin = dynamic_cast<CtMainWin*>(pWin);
        if (pCtMainWin->get_ct_storage()->get_file_path().empty())
        {
            return pCtMainWin;
        }
    }
    // 3) if our filepath is empty, just get the first window
    if (filepath.empty() and get_windows().size() > 0)
    {
        return dynamic_cast<CtMainWin*>(get_windows().front());
    }
    return nullptr;
}

void CtApp::on_activate()
{
    // app run without arguments
    CtMainWin* pAppWindow = _get_main_win();
    if (nullptr == pAppWindow)
    {
        // there is not a window already running
        pAppWindow = _create_appwindow();
        if (not CtApp::_uCtCfg->recentDocsFilepaths.empty())
        {
            Glib::RefPtr<Gio::File> r_file = Gio::File::create_for_path(CtApp::_uCtCfg->recentDocsFilepaths.front());
            if (r_file->query_exists())
            {
                if (not pAppWindow->file_open(r_file->get_path(), false))
                {
                    _printHelpMessage();
                }
            }
            else
            {
                std::cout << "? not found " << CtApp::_uCtCfg->recentDocsFilepaths.front() << std::endl;
                CtApp::_uCtCfg->recentDocsFilepaths.move_or_push_back(CtApp::_uCtCfg->recentDocsFilepaths.front());
                pAppWindow->menu_set_items_recent_documents();
            }
        }
    }
    pAppWindow->present();
}

void CtApp::_on_hide_window(CtMainWin* pCtMainWin)
{
    pCtMainWin->config_update_data_from_curr_status();
    _uCtCfg->write_to_file();
    delete pCtMainWin;
}

void CtApp::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& /*hint*/)
{
    // app run with arguments
    for (const Glib::RefPtr<Gio::File>& r_file : files)
    {
        if (r_file->query_exists())
        {
            CtMainWin* pAppWindow = _get_main_win(r_file->get_path());
            if (nullptr == pAppWindow)
            {
                // there is not a window already running with that document
                pAppWindow = _create_appwindow();
                if (not pAppWindow->file_open(r_file->get_path(), false))
                {
                    _printHelpMessage();
                }
            }
            pAppWindow->present();
        }
        else
        {
            std::cout << "!! Missing file " << r_file->get_path() << std::endl;
            _printHelpMessage();
        }
    }
}

void CtApp::quit_application()
{
    quit();
}

void CtApp::dialog_preferences()
{
    CtPrefDlg prefDlg(_get_main_win());
    prefDlg.show();
    prefDlg.run();
    prefDlg.hide();
}
