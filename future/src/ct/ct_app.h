/*
 * ct_app.h
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

#pragma once

#include <iostream>
#include <unordered_map>

#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "ct_config.h"
#include "ct_main_win.h"
#include "ct_menu.h"
#include "ct_actions.h"
#include "ct_export2pdf.h"

class CtMenu;
class CtMainWin;
class CtActions;
class CtApp : public Gtk::Application
{
protected:
    CtApp();
    virtual ~CtApp() override;

public:
    static Glib::RefPtr<CtApp> create();

private:
    std::unique_ptr<CtConfig> _uCtCfg;
    std::unique_ptr<CtTmp> _uCtTmp;
    Glib::RefPtr<Gtk::IconTheme> _rIcontheme;
    Glib::RefPtr<Gtk::TextTagTable> _rTextTagTable;
    Glib::RefPtr<Gtk::CssProvider> _rCssProvider;
    Glib::RefPtr<Gsv::LanguageManager> _rLanguageManager;
    Glib::RefPtr<Gsv::StyleSchemeManager> _rStyleSchemeManager;
    Glib::RefPtr<Gtk::StatusIcon> _rStatusIcon;

protected:
    void on_activate() override;
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;
    void on_window_removed(Gtk::Window* window) override;

    void _printHelpMessage();
    void _printGresourceIcons();

public:

private:
    CtMainWin*  _create_window();
    CtMainWin*  _get_window_by_path(const std::string& filepath);
    bool        _quit_or_hide_window(CtMainWin* pCtMainWin, bool from_delete);

    void _systray_show_hide_windows();
    void _systray_close_all();
};
