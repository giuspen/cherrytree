/*
 * ct_app.cc
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

#include <glib/gstdio.h>
#include "ct_app.h"
#include "ct_pref_dlg.h"
#include "ct_storage_control.h"
#include "config.h"
#include "ct_logging.h"
#include <iostream>

CtApp::CtApp(const Glib::ustring application_id_postfix)
 : Gtk::Application{Glib::ustring{"com.giuspen.cherrytree"} + application_id_postfix, Gio::APPLICATION_HANDLES_OPEN}
{
    Gsv::init();

    // action to call from second instance
    // user wanted to create a new window from command line
    add_action("new_window", [&]() {
        _new_window = true;
    });

    _add_main_option_entries();
    signal_handle_local_options().connect(sigc::mem_fun(*this, &CtApp::_on_handle_local_options), false);

    // This is normally called for us... but after the "handle_local_options" signal is emitted. If
    // we want to rely on actions for handling options, we need to call it here. This appears to
    // have no unwanted side-effect. It will also trigger the call to on_startup().
    register_application();
}

/*static*/Glib::RefPtr<CtApp> CtApp::create(const Glib::ustring application_id_postfix)
{
    return Glib::RefPtr<CtApp>(new CtApp{application_id_postfix});
}

static CtApp* _pCtApp{nullptr};
static void kill_callback_handler(int signum)
{
    spdlog::debug("{} {}", __FUNCTION__, signum);
    _pCtApp->close_all_windows(true/*fromKillCallback*/);
}

// small optimization: second instance doesn't need all UI initialization, so we call it on the real startup
void CtApp::_on_startup()
{
    if (_initDone) return;
    _initDone = true;

    const fs::path config_dir = fs::get_cherrytree_configdir();
    if (not fs::exists(config_dir)) {
        if (g_mkdir_with_parents(config_dir.c_str(), 0755) < 0) {
            spdlog::warn("Could not create config dir {}", config_dir.c_str());
        }
    }
    _uCtCfg.reset(new CtConfig{});
    //std::cout << _uCtCfg->specialChars.size() << "\t" << _uCtCfg->specialChars << std::endl;

    const fs::path user_dir_icons = config_dir / "icons";
    _rIcontheme = Gtk::IconTheme::get_default();
    _rIcontheme->append_search_path(user_dir_icons.string());
    _rIcontheme->add_resource_path("/icons/");
    //_print_gresource_icons();

    _uCtTmp.reset(new CtTmp{});
    //std::cout << _uCtTmp->get_root_dirpath() << std::endl;

    _rTextTagTable = Gtk::TextTagTable::create();

    _rLanguageManager = Gsv::LanguageManager::create();
    std::vector<std::string> langSearchPath = _rLanguageManager->get_search_path();
    fs::path ctLanguageSpecsData = fs::get_cherrytree_datadir() / CtConfig::ConfigLanguageSpecsDirname;
    langSearchPath.push_back(ctLanguageSpecsData.string());
    fs::path ctLanguageSpecsConfig = fs::get_cherrytree_config_language_specs_dirpath();
    langSearchPath.push_back(ctLanguageSpecsConfig.string());
    _rLanguageManager->set_search_path(langSearchPath);

    _rStyleSchemeManager = Gsv::StyleSchemeManager::create();
    std::vector<std::string> styleSearchPath = _rStyleSchemeManager->get_search_path();
    fs::path ctStylesData = fs::get_cherrytree_datadir() / CtConfig::ConfigStylesDirname;
    styleSearchPath.push_back(ctStylesData.string());
    fs::path ctStylesConfig = fs::get_cherrytree_config_styles_dirpath();
    styleSearchPath.push_back(ctStylesConfig.string());
    _rStyleSchemeManager->set_search_path(styleSearchPath);

    _rCssProvider = Gtk::CssProvider::create();

    _uCtStatusIcon.reset(new CtStatusIcon{*this});

    if (not _no_gui) {
        _pCtApp = this;
        signal(SIGTERM, kill_callback_handler); // kill/killall
        signal(SIGINT, kill_callback_handler);  // Ctrl+C
#ifndef _WIN32
        signal(SIGQUIT, kill_callback_handler); // Ctrl+Backslash
        signal(SIGHUP, kill_callback_handler);  // user’s terminal disconnected
#endif // not _WIN32
        // SIGKILL cannot be handled or ignored, and is therefore always fatal
    }
}

void CtApp::on_activate()
{
    _on_startup();

    if (get_windows().size() == 0) {
        // start of main instance
        CtMainWin* pAppWindow = _create_window();
        if (CtApp::_uCtCfg->reloadDocLast && not CtApp::_uCtCfg->recentDocsFilepaths.empty()) {
            Glib::RefPtr<Gio::File> r_file = Gio::File::create_for_path(CtApp::_uCtCfg->recentDocsFilepaths.front().string());
            if (r_file->query_exists()) {
                const std::string canonicalPath = fs::canonical(r_file->get_path()).string();
                if (not pAppWindow->start_on_systray_is_active()) {
                    if (not pAppWindow->file_open(canonicalPath, "", _password)) {
                        spdlog::warn("%s Couldn't open file: %s", __FUNCTION__, canonicalPath);
                    }
                }
                else {
                    pAppWindow->start_on_systray_delayed_file_open_set(canonicalPath, "");
                }
            }
            else {
                spdlog::info("%s Last doc not found: {}", __FUNCTION__, CtApp::_uCtCfg->recentDocsFilepaths.front());
                CtApp::_uCtCfg->recentDocsFilepaths.move_or_push_back(CtApp::_uCtCfg->recentDocsFilepaths.front());
                pAppWindow->menu_set_items_recent_documents();
            }
        }
        if (CtApp::_uCtCfg->checkVersion) {
            pAppWindow->get_ct_actions()->check_for_newer_version();
        }
    }
    else {
        // start of the second instance
        if (_new_window) {
            _create_window()->present();
        }
        else {
            Gtk::Window* any_shown_win = nullptr;
            for (Gtk::Window* pWin : get_windows()) {
                if (pWin->get_visible()) {
                    any_shown_win = pWin;
                }
            }
            if (any_shown_win) {
                any_shown_win->present();
            }
            else {
                // all windows are hidden, show them
                // also it fixes an issue with a missing systray
                systray_show_hide_windows();
            }
        }
    }
    _new_window = false; // reset for future calls
}

void CtApp::on_open(const Gio::Application::type_vec_files& files, const Glib::ustring& /*hint*/)
{
    _on_startup();

    // do some export stuff from console and close app after
    if ( not _export_to_txt_dir.empty() or
         not _export_to_html_dir.empty() or
         not _export_to_pdf_dir.empty() )
    {
        _no_gui = true;
        spdlog::debug("export arguments are detected");
        for (const Glib::RefPtr<Gio::File>& r_file : files) {
            spdlog::debug("file to export: {}", r_file->get_path());
            CtMainWin* pWin = _create_window(true/*no_gui*/);
            const std::string canonicalPath = fs::canonical(r_file->get_path()).string();
            if (pWin->file_open(canonicalPath, "", _password)) {
                try {
                    if (not _export_to_txt_dir.empty()) {
                        pWin->get_ct_actions()->export_to_txt_auto(_export_to_txt_dir, _export_overwrite, _export_single_file);
                    }
                    if (not _export_to_html_dir.empty()) {
                        pWin->get_ct_actions()->export_to_html_auto(_export_to_html_dir, _export_overwrite, _export_single_file);
                    }
                    if (not _export_to_pdf_dir.empty()) {
                        pWin->get_ct_actions()->export_to_pdf_auto(_export_to_pdf_dir, _export_overwrite);
                    }
                }
                catch (std::exception& e) {
                    spdlog::error("caught exception: {}", e.what());
                }
            }
            pWin->force_exit() = true;
            remove_window(*pWin);
        }
        spdlog::debug("export is done, closing app");
        // exit app
        return;
    }

    // ordinary app start with filepath argument
    for (const Glib::RefPtr<Gio::File>& r_file : files) {
        CtMainWin* pAppWindow = _get_window_by_path(r_file->get_path());
        if (nullptr == pAppWindow) {
            // there is not a window already running with that document
            pAppWindow = _create_window();
            const std::string canonicalPath = fs::canonical(r_file->get_path()).string();
            if (not pAppWindow->start_on_systray_is_active()) {
                if (not pAppWindow->file_open(canonicalPath, _node_to_focus, _password)) {
                    spdlog::warn("%s Couldn't open file: {}", __FUNCTION__, canonicalPath);
                }
            }
            else {
                pAppWindow->start_on_systray_delayed_file_open_set(canonicalPath, _node_to_focus);
            }
            if (get_windows().size() == 1) {
                // start of main instance
                if (CtApp::_uCtCfg->checkVersion) {
                    pAppWindow->get_ct_actions()->check_for_newer_version();
                }
            }
        }
        // window can be hidden, so show it
        pAppWindow->present();
    }
    _new_window = false; // reset for future calls
}

void CtApp::on_window_removed(Gtk::Window* window)
{
    // override this function, so hidden windows won't be deleted from the window list
    // but destroy window when that is needed
    if (CtMainWin* win = dynamic_cast<CtMainWin*>(window)) {
        if (win->force_exit()) {
            Gtk::Application::on_window_removed(window);
            delete window;
        }
    }
}

CtMainWin* CtApp::_create_window(const bool no_gui)
{
    CtMainWin* pCtMainWin = new CtMainWin(no_gui,
                                          _uCtCfg.get(),
                                          _uCtTmp.get(),
                                          _rIcontheme.get(),
                                          _rTextTagTable,
                                          _rCssProvider,
                                          _rLanguageManager.get(),
                                          _rStyleSchemeManager.get(),
                                          _uCtStatusIcon.get());
    add_window(*pCtMainWin);

    pCtMainWin->signal_app_new_instance.connect([this]() {
        auto win = _create_window();
        win->present(); // explicitly show it because it can be hidden by start in systray
    });
    pCtMainWin->signal_app_apply_for_each_window.connect([this](std::function<void(CtMainWin*)> callback) {
        for (Gtk::Window* pWin : get_windows())
            if (CtMainWin* pCtMainWin = dynamic_cast<CtMainWin*>(pWin))
                callback(pCtMainWin);
    });
    pCtMainWin->signal_app_quit_or_hide_window.connect([&](CtMainWin* win) {
        _quit_or_hide_window(win, false/*fromDelete*/, false/*fromKillCallback*/);
    });
    pCtMainWin->signal_delete_event().connect([this, pCtMainWin](GdkEventAny*) {
        bool good = _quit_or_hide_window(pCtMainWin, true/*fromDelete*/, false/*fromKillCallback*/);
        return !good;
    });
    pCtMainWin->signal_app_quit_window.connect([&](CtMainWin* win) {
        win->force_exit() = true;
        _quit_or_hide_window(win, false/*fromDelete*/, false/*fromKillCallback*/);
    });
    pCtMainWin->signal_show_hide_main_win.connect([&]() {
        systray_show_hide_windows();
    });

    return pCtMainWin;
}

CtMainWin* CtApp::_get_window_by_path(const std::string& filepath)
{
    for (Gtk::Window* pWin : get_windows()) {
        CtMainWin* pCtMainWin = dynamic_cast<CtMainWin*>(pWin);
        if (filepath == pCtMainWin->get_ct_storage()->get_file_path()) {
            return pCtMainWin;
        }
    }
    return nullptr;
}

bool CtApp::_quit_or_hide_window(CtMainWin* pCtMainWin, const bool from_delete, const bool fromKillCallback)
{
    std::lock_guard<std::mutex> lock(_quitOrHideWinMutex);

    if (not _no_gui) {
        pCtMainWin->config_update_data_from_curr_status();
        _uCtCfg->write_to_file();
    }

    if (not fromKillCallback) {
        _uCtCfg->move_from_tmp();
        if (_uCtCfg->systrayOn and not pCtMainWin->force_exit()) {
            pCtMainWin->save_position();
            pCtMainWin->set_visible(false);
            return false; // stop deleting window
        }
        // trying to save changes, show window if possible/needed
        if (not pCtMainWin->file_save_ask_user()) {
            pCtMainWin->force_exit() = false;
            return false;  // stop deleting window
        }
    }
    else {
        // It's too dangerous to try and save the document while being killed
    }

    pCtMainWin->force_exit() = true; // this is for on_window_removed
    if (not from_delete) {           // signal from remove, no need to remove again
        remove_window(*pCtMainWin);  // object will be destroyed in on_window_removed
    }
    return true; // keep deleting window
}

void CtApp::systray_show_hide_windows()
{
    _uCtStatusIcon->ensure_menu_hidden();
    while (gtk_events_pending()) gtk_main_iteration();
    bool to_show{true};
    for (Gtk::Window* pWin : get_windows()) {
        // if any window is visible, we will hide
#ifdef _WIN32
        if (pWin->get_visible()) {
#else
        if (pWin->has_toplevel_focus()) {
#endif
            to_show = false;
            break;
        }
    }
    if (not to_show) {
        // check if any window should not be hidden
        for (Gtk::Window* pWin : get_windows()) {
            if (not dynamic_cast<CtMainWin*>(pWin)->get_systray_can_hide()) {
                to_show = true;
                break;
            }
        }
    }
    for (Gtk::Window* pWin : get_windows()) {
        CtMainWin* win = dynamic_cast<CtMainWin*>(pWin);
        if (to_show) {
            win->present();
            win->restore_position();
            win->set_visible(true);
            (void)win->start_on_systray_delayed_file_open_kick();
        }
        else {
            win->save_position();
            win->set_visible(false);
        }
    }
}

void CtApp::close_all_windows(const bool fromKillCallback)
{
    for (Gtk::Window* pWin : get_windows()) {
        if (CtMainWin* win = dynamic_cast<CtMainWin*>(pWin)) {
            win->force_exit() = true;
            if (not _quit_or_hide_window(win, false/*fromDelete*/, fromKillCallback)) {
                break;
            }
        }
    }
}

void CtApp::_add_main_option_entries()
{
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "version",            'V', _("Print CherryTree version"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING,   "node",               'n', _("Node name to focus"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_html_dir", 'x', _("Export to HTML at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_txt_dir",  't', _("Export to Text at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_pdf_dir",  'p', _("Export to PDF at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "export_overwrite",   'w', _("Overwrite if export path already exists"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "export_single_file", 's', _("Export to a single file (for HTML or TXT)"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING,   "password",           'P', _("Password to open document"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "new_window",         'N', _("Create a new window"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "secondary_session",  'S', _("Run in secondary session, independent from main session"));
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

    if (rOptions->contains("version")) {
        std::cout << "CherryTree " << CtConst::CT_VERSION << std::endl;
        return 0; // to exit app
    }

    bool new_window{false};

    rOptions->lookup_value("node", _node_to_focus);
    rOptions->lookup_value("export_to_html_dir", _export_to_html_dir);
    rOptions->lookup_value("export_to_txt_dir", _export_to_txt_dir);
    rOptions->lookup_value("export_to_pdf_dir", _export_to_pdf_dir);
    rOptions->lookup_value("export_overwrite", _export_overwrite);
    rOptions->lookup_value("export_single_file", _export_single_file);
    rOptions->lookup_value("password", _password);
    rOptions->lookup_value("new_window", new_window);

    if (new_window) {
        activate_action("new_window");
    }

    return -1; // Keep going
}
