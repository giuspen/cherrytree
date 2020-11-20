/*
 * ct_app.h
 *
 * Copyright 2009-2020
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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
    Glib::ustring _node_to_focus;
    std::string   _export_to_html_dir;
    std::string   _export_to_txt_dir;
    std::string   _export_to_pdf_file;
    bool          _export_overwrite{false};
    bool          _startup2{false};

protected:
    void on_startup2();
    void on_activate() override;
    void on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;
    void on_window_removed(Gtk::Window* window) override;

    void _add_main_option_entries();
    void _print_gresource_icons();

protected:
    CtMainWin*  _create_window(const bool no_gui = false);
    CtMainWin*  _get_window_by_path(const std::string& filepath);
    bool        _quit_or_hide_window(CtMainWin* pCtMainWin, bool from_delete);
    int         _on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& rOptions);

    void _systray_show_hide_windows();
    void _systray_close_all();
};
