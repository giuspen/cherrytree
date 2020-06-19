/*
 * ct_app.cc
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

#include <glib/gstdio.h>
#include "ct_app.h"
#include "ct_pref_dlg.h"
#include "ct_storage_control.h"
#include "config.h"
#include "ct_logging.h"



CtApp::CtApp() : Gtk::Application("com.giuspen.cherrytree", Gio::APPLICATION_HANDLES_OPEN)
{
    Gsv::init();

    std::string config_dir = Glib::build_filename(Glib::get_user_config_dir(), CtConst::APP_NAME);
    if (g_mkdir_with_parents(config_dir.c_str(), 0755) < 0)
        g_warning("Could not create config directory: %s", config_dir.c_str());

    _uCtCfg.reset(new CtConfig());
    //std::cout << _uCtCfg->specialChars.size() << "\t" << _uCtCfg->specialChars << std::endl;

    _rIcontheme = Gtk::IconTheme::get_default();
    _rIcontheme->add_resource_path("/icons/");
    //_print_gresource_icons();

    _uCtTmp.reset(new CtTmp());
    //std::cout << _uCtTmp->get_root_dirpath() << std::endl;

    _rTextTagTable = Gtk::TextTagTable::create();

    _rLanguageManager = Gsv::LanguageManager::create();
    std::vector<std::string> searchPath = _rLanguageManager->get_search_path();
    fs::path ctLanguagesSpecsPath = fs::get_cherrytree_datadir() / "language-specs";
    searchPath.push_back(ctLanguagesSpecsPath.string());
    _rLanguageManager->set_search_path(searchPath);

    _rStyleSchemeManager = Gsv::StyleSchemeManager::create();

    _rCssProvider = Gtk::CssProvider::create();

    _rStatusIcon = Gtk::StatusIcon::create(CtConst::APP_NAME);
    _rStatusIcon->set_visible(false);
    _rStatusIcon->set_name(CtConst::APP_NAME);
    _rStatusIcon->set_title(CtConst::APP_NAME);
    _rStatusIcon->set_tooltip_markup(_("CherryTree Hierarchical Note Taking"));
    _rStatusIcon->signal_button_press_event().connect([&](GdkEventButton* event) {
        if (event->button == 1) { _systray_show_hide_windows(); }
        return false;
    });
    _rStatusIcon->signal_popup_menu().connect([&](guint button, guint32 activate_time){
        Gtk::Menu* systrayMenu = Gtk::manage(new Gtk::Menu());
        auto item1 = CtMenu::create_menu_item(GTK_WIDGET(systrayMenu->gobj()), _("Show/Hide _CherryTree"), CtConst::APP_NAME, _("Toggle Show/Hide CherryTree"));
        item1->signal_activate().connect([&] {_systray_show_hide_windows();});
        auto item2 = CtMenu::create_menu_item(GTK_WIDGET(systrayMenu->gobj()), _("_Exit CherryTree"), "ct_quit-app", _("Exit from CherryTree"));
        item2->signal_activate().connect([&] { _systray_close_all(); });
        systrayMenu->show_all();
        systrayMenu->popup(button, activate_time);
    });

    _add_main_option_entries();
    signal_handle_local_options().connect(sigc::mem_fun(*this, &CtApp::_on_handle_local_options), false);
}

CtApp::~CtApp()
{
    //std::cout << "~CtApp()" << std::endl;
}

Glib::RefPtr<CtApp> CtApp::create()
{
    return Glib::RefPtr<CtApp>(new CtApp());
}

void CtApp::on_activate()
{
    if (get_windows().size() == 0)
    {
        CtMainWin* pAppWindow = _create_window(false);
        if (not CtApp::_uCtCfg->recentDocsFilepaths.empty())
        {
            Glib::RefPtr<Gio::File> r_file = Gio::File::create_for_path(CtApp::_uCtCfg->recentDocsFilepaths.front().string());
            if (r_file->query_exists())
            {
                if (not pAppWindow->file_open(r_file->get_path(), ""))
                {
                    g_warning("Couldn't open file: %s", r_file->get_path().c_str());
                }
            }
            else
            {
                g_message("Last doc not found: %s", CtApp::_uCtCfg->recentDocsFilepaths.front().c_str());
                CtApp::_uCtCfg->recentDocsFilepaths.move_or_push_back(CtApp::_uCtCfg->recentDocsFilepaths.front());
                pAppWindow->menu_set_items_recent_documents();
            }
        }
    }
}

void CtApp::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& /*hint*/)
{
    // do some export stuff from console and close app after
    if (_export_to_txt_dir != "" || _export_to_html_dir != "" || _export_to_pdf_file != "")
    {
        spdlog::debug("export arguments are detected");
        for (const Glib::RefPtr<Gio::File>& r_file : files)
        {
            spdlog::debug("file to export: {}", r_file->get_path());
            CtMainWin* win = _create_window(true); // start hidden
            if (win->file_open(r_file->get_path(), "")) {
                try
                {
                    if (_export_to_txt_dir != "") win->get_ct_actions()->export_to_txt_auto(_export_to_txt_dir, _export_overwrite);
                    if (_export_to_html_dir != "") win->get_ct_actions()->export_to_html_auto(_export_to_html_dir, _export_overwrite);
                    if (_export_to_pdf_file != "") win->get_ct_actions()->export_to_pdf_auto(_export_to_pdf_file, _export_overwrite);
                }
                catch (std::exception& e)
                {
                    spdlog::error("caught exception: {}", e.what());
                }
            }
            win->force_exit() = true;
            remove_window(*win);
        }
        spdlog::debug("export is done, closing app");
        // exit app
        return;
    }

    // ordinary app start with filepath argument
    for (const Glib::RefPtr<Gio::File>& r_file : files)
    {
        CtMainWin* pAppWindow = _get_window_by_path(r_file->get_path());
        if (nullptr == pAppWindow)
        {
            // there is not a window already running with that document
            pAppWindow = _create_window(false);
            if (not pAppWindow->file_open(r_file->get_path(), _node_to_focus))
            {
                g_warning("Couldn't open file: %s", r_file->get_path().c_str());
            }
        }
        // window can be hidden, so show it
        pAppWindow->present();
    }
}

void CtApp::on_window_removed(Gtk::Window* window)
{
    // override this function, so hidden windows won't be deleted from the window list
    // but destroy window when that is needed
    if (CtMainWin* win = dynamic_cast<CtMainWin*>(window))
        if (win->force_exit())
        {
            Gtk::Application::on_window_removed(window);
            delete window;
        }
}

CtMainWin* CtApp::_create_window(bool start_hidden)
{
    CtMainWin* pCtMainWin = new CtMainWin(start_hidden,
                                          _uCtCfg.get(),
                                          _uCtTmp.get(),
                                          _rIcontheme.get(),
                                          _rTextTagTable,
                                          _rCssProvider,
                                          _rLanguageManager.get(),
                                          _rStyleSchemeManager.get(),
                                          _rStatusIcon.get());
    add_window(*pCtMainWin);

    pCtMainWin->signal_app_new_instance.connect([this]() {
        _create_window(false);
    });
    pCtMainWin->signal_app_apply_for_each_window.connect([this](std::function<void(CtMainWin*)> callback) {
        for (Gtk::Window* pWin : get_windows())
            if (CtMainWin* pCtMainWin = dynamic_cast<CtMainWin*>(pWin))
                callback(pCtMainWin);
    });

    pCtMainWin->signal_app_quit_or_hide_window.connect([&](CtMainWin* win) {
        _quit_or_hide_window(win, false);
    });
    pCtMainWin->signal_delete_event().connect([this, pCtMainWin](GdkEventAny*) {
        bool good = _quit_or_hide_window(pCtMainWin, true);
        return !good;
    });
    pCtMainWin->signal_app_quit_window.connect([&](CtMainWin* win) {
        win->force_exit() = true;
        _quit_or_hide_window(win, false);
    });

    return pCtMainWin;
}

CtMainWin* CtApp::_get_window_by_path(const std::string& filepath)
{
    for (Gtk::Window* pWin : get_windows())
    {
        CtMainWin* pCtMainWin = dynamic_cast<CtMainWin*>(pWin);
        if (filepath == pCtMainWin->get_ct_storage()->get_file_path())
            return pCtMainWin;
    }
    return nullptr;
}

bool CtApp::_quit_or_hide_window(CtMainWin* pCtMainWin, bool from_delete)
{
    pCtMainWin->config_update_data_from_curr_status();
    _uCtCfg->write_to_file();

    if (_uCtCfg->systrayOn && !pCtMainWin->force_exit() /* if didn't come from quit_window */)
    {
        pCtMainWin->save_position();
        pCtMainWin->set_visible(false);
        return false; // to stop deleting window
    }
    else
    {
        // trying to save changes, it show window if needed
        if (!pCtMainWin->file_save_ask_user())
        {
            pCtMainWin->force_exit() = false;
            return false;  // to stop deleting windows
        }
        pCtMainWin->force_exit() = true; // this is for on_window_removed
        if (!from_delete)                // signal from remove, no need to remove again
            remove_window(*pCtMainWin);  // object will be destroyed in on_window_removed

        return true; // continue deleting window
    }
}

void CtApp::_systray_show_hide_windows()
{
    bool to_show = true;
    for (Gtk::Window* pWin : get_windows())
        if (pWin->has_toplevel_focus())
            to_show = false;
    for (Gtk::Window* pWin : get_windows())
    {
        CtMainWin* win = dynamic_cast<CtMainWin*>(pWin);
        if (to_show)
        {
            win->set_visible(true);
            win->present();
            win->restore_position();
        }
        else
        {
            win->save_position();
            win->set_visible(false);
        }
    }
}

void CtApp::_systray_close_all()
{
    for (Gtk::Window* pWin : get_windows())
        if (CtMainWin* win = dynamic_cast<CtMainWin*>(pWin))
        {
            win->force_exit() = true;
            if (!_quit_or_hide_window(win, false))
                break;
        }
}


void CtApp::_add_main_option_entries()
{
    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING,   "node",               'n', _("Node name to focus"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_html_dir", 'x', _("Export to HTML at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_txt_dir",  't', _("Export to Text at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_pdf_file", 'p', _("Export to PDF at specified file path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "export_overwrite",   'w', _("Overwrite if export path already exists"));
}

void CtApp::_print_gresource_icons()
{
    for (const std::string& str_icon : Gio::Resource::enumerate_children_global("/icons/", Gio::ResourceLookupFlags::RESOURCE_LOOKUP_FLAGS_NONE))
    {
        spdlog::debug(str_icon);
    }
}

int CtApp::_on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& rOptions)
{
    if (!rOptions) {
        spdlog::error("CtApp::on_handle_local_options: options is null!");
        return -1; // Keep going
    }

    rOptions->lookup_value("node", _node_to_focus);
    rOptions->lookup_value("export_to_html_dir", _export_to_html_dir);
    rOptions->lookup_value("export_to_txt_dir", _export_to_txt_dir);
    rOptions->lookup_value("export_to_pdf_file", _export_to_pdf_file);
    rOptions->lookup_value("export_overwrite", _export_overwrite);

    return -1; // Keep going
}
