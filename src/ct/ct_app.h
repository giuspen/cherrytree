/*
 * ct_app.h
 *
 * Copyright 2009-2022
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

#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "ct_config.h"
#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_export2pdf.h"
#include "ct_widgets.h"

class CtMainWin;
class CtApp : public Gtk::Application
{
protected:
    CtApp(const Glib::ustring application_id_postfix = Glib::ustring{});

public:
    static Glib::RefPtr<CtApp> create(const Glib::ustring application_id_postfix = Glib::ustring{});
    void                       close_all_windows(const bool userCanInteract = true);
    void                       systray_show_hide_windows();

protected:
    std::unique_ptr<CtConfig> _uCtCfg;
    std::unique_ptr<CtTmp> _uCtTmp;
    Glib::RefPtr<Gtk::IconTheme> _rIcontheme;
    Glib::RefPtr<Gtk::TextTagTable> _rTextTagTable;
    Glib::RefPtr<Gtk::CssProvider> _rCssProvider;
    Glib::RefPtr<Gsv::LanguageManager> _rLanguageManager;
    Glib::RefPtr<Gsv::StyleSchemeManager> _rStyleSchemeManager;
    std::unique_ptr<CtStatusIcon> _uCtStatusIcon;

protected:
    Glib::ustring _node_to_focus;
    std::string   _export_to_html_dir;
    std::string   _export_to_txt_dir;
    std::string   _export_to_pdf_dir;
    Glib::ustring _password;
    bool          _export_overwrite{false};
    bool          _export_single_file{false};
    bool          _new_window{false};
    bool          _initDone{false};
    bool          _no_gui{false};
    std::mutex    _quitOrHideWinMutex;

protected:
    void        on_activate() override;
    void        on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& hint) override;
    void        on_window_removed(Gtk::Window* window) override;

    void        _on_startup();
    void        _add_main_option_entries();
    void        _print_gresource_icons();

protected:
    CtMainWin*  _create_window(const bool no_gui = false);
    CtMainWin*  _get_window_by_path(const std::string& filepath);
    bool        _quit_or_hide_window(CtMainWin* pCtMainWin, const bool fromDelete, const bool fromKillCallback);
    int         _on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& rOptions);

private:
    Gtk::Window* _pWinToCopyFrom{nullptr};
    gint64       _nodeIdToCopyFrom;
};
