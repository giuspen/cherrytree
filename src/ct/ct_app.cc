/*
 * ct_app.cc
 *
 * Copyright 2009-2025
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

#if GTKMM_MAJOR_VERSION >= 4
CtApp::CtApp(const Glib::ustring application_id_postfix, Gio::Application::Flags flags)
 : Gtk::Application{Glib::ustring{"net.giuspen.cherrytree"} + application_id_postfix, Gio::Application::Flags::HANDLES_OPEN | flags}
#else
CtApp::CtApp(const Glib::ustring application_id_postfix, Gio::ApplicationFlags flags)
 : Gtk::Application{Glib::ustring{"net.giuspen.cherrytree"} + application_id_postfix, Gio::APPLICATION_HANDLES_OPEN | flags}
#endif
 , _pCtConfig{CtConfig::GetCtConfig()}
{
#if GTK_SOURCE_MAJOR_VERSION >= 4
    gtk_source_init();
#endif

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

CtApp::~CtApp()
{
}

/*static*/Glib::RefPtr<CtApp> CtApp::create(const Glib::ustring application_id_postfix)
{
#if GTKMM_MAJOR_VERSION >= 4
    return Glib::make_refptr_for_instance<CtApp>(new CtApp{application_id_postfix});
#else
    return Glib::RefPtr<CtApp>(new CtApp{application_id_postfix});
#endif
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

#if defined(_WIN32)
    (void)fs::alter_TEXMFROOT_env_var();
    (void)fs::alter_PATH_env_var();
#endif // _WIN32

    const fs::path config_dir = fs::get_cherrytree_configdir();
    if (not fs::exists(config_dir)) {
        if (g_mkdir_with_parents(config_dir.c_str(), 0755) < 0) {
            spdlog::warn("Could not create config dir {}", config_dir.c_str());
        }
    }

    const fs::path user_dir_icons = config_dir / "icons";
#if GTKMM_MAJOR_VERSION >= 4
    _rIcontheme = Gtk::IconTheme::get_for_display(Gdk::Display::get_default());
    _rIcontheme->add_search_path(user_dir_icons.string());
#else
    _rIcontheme = Gtk::IconTheme::get_default();
    _rIcontheme->append_search_path(user_dir_icons.string());
#endif
    _rIcontheme->add_resource_path("/icons/");
    //_print_gresource_icons();

    _uCtTmp.reset(new CtTmp{});
    //std::cout << _uCtTmp->get_root_dirpath() << std::endl;

    _rTextTagTable = Gtk::TextTagTable::create();

    GtkSourceLanguageManager* pGtkSourceLanguageManager = gtk_source_language_manager_get_default();
    const gchar * const * pLMSearchPath = gtk_source_language_manager_get_search_path(pGtkSourceLanguageManager);
    std::vector<gchar const *> langSearchPath;
    for (auto pPath = pLMSearchPath; *pPath; ++pPath) {
        langSearchPath.push_back(*pPath);
    }
    fs::path ctLanguageSpecsData = fs::get_cherrytree_datadir() / CtConfig::ConfigLanguageSpecsDirname;
    langSearchPath.push_back(ctLanguageSpecsData.c_str());
    fs::path ctLanguageSpecsConfig = fs::get_cherrytree_config_language_specs_dirpath();
    langSearchPath.push_back(ctLanguageSpecsConfig.c_str());
    langSearchPath.push_back(nullptr);
    /* At the moment this function can be called only before the language files are loaded for the first time.
       In practice to set a custom search path for a GtkSourceLanguageManager, you have to call this function right after creating it. */
    _pGtkSourceLanguageManager = gtk_source_language_manager_new();
    gtk_source_language_manager_set_search_path(_pGtkSourceLanguageManager, (gchar **)langSearchPath.data());

    GtkSourceStyleSchemeManager* pGtkSourceStyleSchemeManager = gtk_source_style_scheme_manager_get_default();
    fs::path ctStylesData = fs::get_cherrytree_datadir() / CtConfig::ConfigStylesDirname;
    gtk_source_style_scheme_manager_append_search_path(pGtkSourceStyleSchemeManager, ctStylesData.c_str());
    fs::path ctStylesConfig = fs::get_cherrytree_config_styles_dirpath();
    gtk_source_style_scheme_manager_append_search_path(pGtkSourceStyleSchemeManager, ctStylesConfig.c_str());

    _rCssProvider = Gtk::CssProvider::create();

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    _uCtStatusIcon.reset(new CtStatusIcon{*this, _pCtConfig});
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */

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
        if (_pCtConfig->reloadDocLast && not _pCtConfig->recentDocsFilepaths.empty()) {
            Glib::RefPtr<Gio::File> r_file = Gio::File::create_for_path(_pCtConfig->recentDocsFilepaths.front().string());
            if (r_file->query_exists()) {
                const std::string canonicalPath = fs::canonical(r_file->get_path()).string();
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
                if (not pAppWindow->start_on_systray_is_active())
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
                {
                    if (not pAppWindow->file_open(canonicalPath, ""/*node*/, ""/*anchor*/, _password)) {
                        spdlog::warn("{} Couldn't open file: {}", __FUNCTION__, canonicalPath);
                    }
                }
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
                else {
                    pAppWindow->start_on_systray_delayed_file_open_set(canonicalPath, ""/*node*/, ""/*anchor*/);
                }
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
            }
            else {
                const fs::path last_doc_path{_pCtConfig->recentDocsFilepaths.front()};
                spdlog::info("{} Last doc not found: {}", __FUNCTION__, last_doc_path.string());
                _pCtConfig->recentDocsFilepaths.move_or_push_back(last_doc_path);
                pAppWindow->menu_set_items_recent_documents();
            }
        }
        if (_pCtConfig->checkVersion) {
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
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
                systray_show_hide_windows();
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
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
            if (pWin->file_open(canonicalPath, ""/*node*/, ""/*anchor*/, _password)) {
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
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
            if (not pAppWindow->start_on_systray_is_active())
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
            {
                if (not pAppWindow->file_open(canonicalPath, _node_to_focus, _anchor_to_focus, _password)) {
                    spdlog::warn("{} Couldn't open file: {}", __FUNCTION__, canonicalPath);
                }
            }
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
            else {
                pAppWindow->start_on_systray_delayed_file_open_set(canonicalPath, _node_to_focus, _anchor_to_focus);
            }
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
            if (get_windows().size() == 1) {
                // start of main instance
                if (_pCtConfig->checkVersion) {
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
    CtMainWin* pCtMainWin = new CtMainWin{no_gui,
                                          _pCtConfig,
                                          _uCtTmp.get(),
                                          _rIcontheme.get(),
                                          _rTextTagTable,
                                          _rCssProvider,
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
                                          _uCtStatusIcon.get(),
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
                                          _pGtkSourceLanguageManager};
    add_window(*pCtMainWin);

    pCtMainWin->connect_app_new_instance([this]() {
        auto win = _create_window();
        win->present(); // explicitly show it because it can be hidden by start in systray
    });
    pCtMainWin->connect_app_apply_for_each_window([this](std::function<void(CtMainWin*)> callback) {
        for (Gtk::Window* pWin : get_windows()) {
            if (CtMainWin* pCtMainWin = dynamic_cast<CtMainWin*>(pWin)) {
                callback(pCtMainWin);
            }
        }
    });
    pCtMainWin->connect_app_quit_or_hide_window([&](CtMainWin* win) {
        _quit_or_hide_window(win, false/*fromDelete*/, false/*fromKillCallback*/);
    });
    pCtMainWin->connect_app_quit_window([&](CtMainWin* win) {
    win->force_exit() = true;
        _quit_or_hide_window(win, false/*fromDelete*/, false/*fromKillCallback*/);
    });
#if GTKMM_MAJOR_VERSION >= 4
    pCtMainWin->signal_close_request().connect([this, pCtMainWin]() {
        bool good = _quit_or_hide_window(pCtMainWin, true/*fromDelete*/, false/*fromKillCallback*/);
        return !good;
    }, false);
#else
    pCtMainWin->signal_delete_event().connect([this, pCtMainWin](GdkEventAny*) {
        bool good = _quit_or_hide_window(pCtMainWin, true/*fromDelete*/, false/*fromKillCallback*/);
        return !good;
    });
#endif
#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
    pCtMainWin->signal_app_show_hide_main_win.connect([&]() {
        systray_show_hide_windows();
    });
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */
    pCtMainWin->connect_app_tree_node_copy([this, pCtMainWin]() {
        _pWinToCopyFrom = pCtMainWin;
        _nodeIdToCopyFrom = pCtMainWin->curr_tree_iter().get_node_id();
    });
    pCtMainWin->connect_app_tree_node_paste([this, pCtMainWin]() {
        Gtk::Window* pWinToCopyFromValidated{nullptr};
        if (_pWinToCopyFrom) {
            for (Gtk::Window* pWin : get_windows()) {
                if (pWin == _pWinToCopyFrom) {
                    pWinToCopyFromValidated = _pWinToCopyFrom;
                    break;
                }
            }
        }
        pCtMainWin->tree_node_paste_from_other_window(dynamic_cast<CtMainWin*>(pWinToCopyFromValidated), _nodeIdToCopyFrom);
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
        _pCtConfig->write_to_file();
    }

    if (not fromKillCallback) {
        _pCtConfig->move_from_tmp();
        if (_pCtConfig->systrayOn and not pCtMainWin->force_exit()) {
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

#if GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED)
void CtApp::systray_show_hide_windows()
{
    _uCtStatusIcon->ensure_menu_hidden();
    while (gtk_events_pending()) gtk_main_iteration();
    bool to_show{true};
    for (Gtk::Window* pWin : get_windows()) {
        // if any window is visible, we will hide
#ifdef _WIN32
        if (pWin->get_visible() and not static_cast<bool>(pWin->get_window()->get_state() & Gdk::WindowState::WINDOW_STATE_ICONIFIED)) {
#if 0
            Glib::RefPtr<Gdk::Window> pGdkWin = pWin->get_window();
            Gdk::WindowState gdkWinState = pGdkWin->get_state();
            spdlog::debug("WINDOW_STATE_WITHDRAWN {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_WITHDRAWN));
            spdlog::debug("WINDOW_STATE_ICONIFIED {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_ICONIFIED));
            spdlog::debug("WINDOW_STATE_MAXIMIZED {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_MAXIMIZED));
            spdlog::debug("WINDOW_STATE_STICKY {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_STICKY));
            spdlog::debug("WINDOW_STATE_FULLSCREEN {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_FULLSCREEN));
            spdlog::debug("WINDOW_STATE_ABOVE {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_ABOVE));
            spdlog::debug("WINDOW_STATE_BELOW {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_BELOW));
            spdlog::debug("WINDOW_STATE_FOCUSED {}", static_cast<bool>(gdkWinState & Gdk::WindowState::WINDOW_STATE_FOCUSED));
#endif /*0*/
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
#endif /* GTKMM_MAJOR_VERSION < 4 && !defined(GTKMM_DISABLE_DEPRECATED) */

void CtApp::close_all_windows(const bool fromKillCallback)
{
    static CtMainWin* pProcessingWin{nullptr};
    if (pProcessingWin) {
        spdlog::debug("{} pProcessingWin", __FUNCTION__);
        pProcessingWin->present();
        return;
    }
    for (Gtk::Window* pWin : get_windows()) {
        if (CtMainWin* win = dynamic_cast<CtMainWin*>(pWin)) {
            win->force_exit() = true;
            pProcessingWin = win;
            if (not _quit_or_hide_window(win, false/*fromDelete*/, fromKillCallback)) {
                break;
            }
        }
    }
    pProcessingWin = nullptr;
}

void CtApp::_add_main_option_entries()
{
#if GTKMM_MAJOR_VERSION >= 4
    add_main_option_entry(Gio::Application::OptionType::BOOL,     "version",            'V', _("Print CherryTree version"));
    add_main_option_entry(Gio::Application::OptionType::STRING,   "node",               'n', _("Node name to focus"));
    add_main_option_entry(Gio::Application::OptionType::STRING,   "anchor",             'a', _("Anchor name to scroll to in node"));
    add_main_option_entry(Gio::Application::OptionType::FILENAME, "export_to_html_dir", 'x', _("Export to HTML at specified directory path"));
    add_main_option_entry(Gio::Application::OptionType::FILENAME, "export_to_txt_dir",  't', _("Export to Text at specified directory path"));
    add_main_option_entry(Gio::Application::OptionType::FILENAME, "export_to_pdf_dir",  'p', _("Export to PDF at specified directory path"));
    add_main_option_entry(Gio::Application::OptionType::BOOL,     "export_overwrite",   'w', _("Overwrite if export path already exists"));
    add_main_option_entry(Gio::Application::OptionType::BOOL,     "export_single_file", 's', _("Export to a single file (for HTML or TXT)"));
    add_main_option_entry(Gio::Application::OptionType::STRING,   "password",           'P', _("Password to open document"));
    add_main_option_entry(Gio::Application::OptionType::BOOL,     "new_window",         'N', _("Create a new window"));
    add_main_option_entry(Gio::Application::OptionType::BOOL,     "secondary_session",  'S', _("Run in secondary session, independent from main session"));
#else
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "version",            'V', _("Print CherryTree version"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING,   "node",               'n', _("Node name to focus"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING,   "anchor",             'a', _("Anchor name to scroll to in node"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_html_dir", 'x', _("Export to HTML at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_txt_dir",  't', _("Export to Text at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_FILENAME, "export_to_pdf_dir",  'p', _("Export to PDF at specified directory path"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "export_overwrite",   'w', _("Overwrite if export path already exists"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "export_single_file", 's', _("Export to a single file (for HTML or TXT)"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_STRING,   "password",           'P', _("Password to open document"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "new_window",         'N', _("Create a new window"));
    add_main_option_entry(Gio::Application::OPTION_TYPE_BOOL,     "secondary_session",  'S', _("Run in secondary session, independent from main session"));
#endif
}

void CtApp::_print_gresource_icons()
{
#if GTKMM_MAJOR_VERSION >= 4
    for (const std::string& str_icon : Gio::Resource::enumerate_children_global("/icons/", Gio::Resource::LookupFlags::NONE))
    {
        spdlog::debug(str_icon);
    }
#else
    for (const std::string& str_icon : Gio::Resource::enumerate_children_global("/icons/", Gio::ResourceLookupFlags::RESOURCE_LOOKUP_FLAGS_NONE))
    {
        spdlog::debug(str_icon);
    }
#endif
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
    rOptions->lookup_value("anchor", _anchor_to_focus);
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
