/*
 * ct_pref_dlg_misc.cc
 *
 * Copyright 2009-2026
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

#include "ct_pref_dlg.h"
#include "ct_main_win.h"

Gtk::Widget* CtPrefDlg::build_tab_misc()
{
    auto vbox_saving = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto hbox_autosave = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto checkbutton_autosave = Gtk::manage(new Gtk::CheckButton{_("Autosave Every")});
    Glib::RefPtr<Gtk::Adjustment> adjustment_autosave = Gtk::Adjustment::create(_pConfig->autosaveMinutes, 1, 1000, 1);
    auto spinbutton_autosave = Gtk::manage(new Gtk::SpinButton(adjustment_autosave));
    auto label_autosave = Gtk::manage(new Gtk::Label{_("Minutes")});
    hbox_autosave->pack_start(*checkbutton_autosave, false, false);
    hbox_autosave->pack_start(*spinbutton_autosave, false, false);
    hbox_autosave->pack_start(*label_autosave, false, false);
    auto checkbutton_autosave_on_quit = Gtk::manage(new Gtk::CheckButton{_("Autosave on Quit")});
    auto checkbutton_backup_before_saving = Gtk::manage(new Gtk::CheckButton{_("Create a Backup Copy Before Saving")});
    auto hbox_num_backups = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_num_backups = Gtk::manage(new Gtk::Label{_("Number of Backups to Keep")});
    Glib::RefPtr<Gtk::Adjustment> adjustment_num_backups = Gtk::Adjustment::create(_pConfig->backupNum, 1, 100, 1);
    auto spinbutton_num_backups = Gtk::manage(new Gtk::SpinButton{adjustment_num_backups});
    spinbutton_num_backups->set_sensitive(_pConfig->backupCopy);
    spinbutton_num_backups->set_value(_pConfig->backupNum);
    auto checkbutton_custom_backup_dir = Gtk::manage(new Gtk::CheckButton{_("Custom Backup Directory")});
    auto file_chooser_button_backup_dir = Gtk::manage(new Gtk::FileChooserButton{_("Custom Backup Directory"),
                                                                                 Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER});
    auto hbox_custom_backup_dir = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto checkbutton_mfname_on_disk = Gtk::manage(new Gtk::CheckButton{_("Multiple Files Storage, Use Embedded File Name On Disk")});

    hbox_num_backups->pack_start(*label_num_backups, false, false);
    hbox_num_backups->pack_start(*spinbutton_num_backups, false, false);
    hbox_custom_backup_dir->pack_start(*checkbutton_custom_backup_dir, false, false);
    hbox_custom_backup_dir->pack_start(*file_chooser_button_backup_dir);
    vbox_saving->pack_start(*hbox_autosave, false, false);
    vbox_saving->pack_start(*checkbutton_autosave_on_quit, false, false);
    vbox_saving->pack_start(*checkbutton_backup_before_saving, false, false);
    vbox_saving->pack_start(*hbox_num_backups, false, false);
    vbox_saving->pack_start(*hbox_custom_backup_dir, false, false);
    vbox_saving->pack_start(*checkbutton_mfname_on_disk, false, false);

    checkbutton_autosave->set_active(_pConfig->autosaveOn);
    spinbutton_autosave->set_value(_pConfig->autosaveMinutes);
    spinbutton_autosave->set_sensitive(_pConfig->autosaveOn);
    checkbutton_autosave_on_quit->set_active(_pConfig->autosaveOnQuit);
    checkbutton_backup_before_saving->set_active(_pConfig->backupCopy);
    checkbutton_custom_backup_dir->set_sensitive(_pConfig->backupCopy);
    checkbutton_custom_backup_dir->set_active(_pConfig->customBackupDirOn);
    file_chooser_button_backup_dir->set_filename(_pConfig->customBackupDir);
    file_chooser_button_backup_dir->set_sensitive(_pConfig->backupCopy and _pConfig->customBackupDirOn);
    checkbutton_mfname_on_disk->set_active(_pConfig->embfileMFNameOnDisk);

    Gtk::Frame* frame_saving = new_managed_frame_with_align(_("Saving"), vbox_saving);

    auto vbox_misc_misc = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto checkbutton_newer_version = Gtk::manage(new Gtk::CheckButton{_("Automatically Check for Newer Version")});
    auto checkbutton_reload_doc_last = Gtk::manage(new Gtk::CheckButton{_("Reload Document From Last Session")});
    auto checkbutton_start_dialog = Gtk::manage(new Gtk::CheckButton{_("Show Start Dialog When No Document Is Loaded")});
    auto checkbutton_mod_time_sentinel = Gtk::manage(new Gtk::CheckButton{_("Reload After External Update to CT* File")});
    auto checkbutton_debug_log = Gtk::manage(new Gtk::CheckButton{_("Enable Debug Log")});
    auto file_chooser_button_debug_log_dir = Gtk::manage(new Gtk::FileChooserButton{_("Debug Log Directory"),
                                                                                    Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER});
    auto hbox_debug_log = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    std::optional<fs::path> optLogdir = fs::get_cherrytree_logdir();
    checkbutton_debug_log->set_active(optLogdir.has_value());
    if (optLogdir.has_value()) {
        file_chooser_button_debug_log_dir->set_filename(optLogdir.value().string());
    }
    else {
        file_chooser_button_debug_log_dir->set_filename(fs::get_cherrytree_configdir().string());
        file_chooser_button_debug_log_dir->set_sensitive(false);
    }

    hbox_debug_log->pack_start(*checkbutton_debug_log, false, false);
    hbox_debug_log->pack_start(*file_chooser_button_debug_log_dir);
    vbox_misc_misc->pack_start(*checkbutton_newer_version, false, false);
    vbox_misc_misc->pack_start(*checkbutton_reload_doc_last, false, false);
    vbox_misc_misc->pack_start(*checkbutton_start_dialog, false, false);
    vbox_misc_misc->pack_start(*checkbutton_mod_time_sentinel, false, false);
    vbox_misc_misc->pack_start(*hbox_debug_log, false, false);

    checkbutton_newer_version->set_active(_pConfig->checkVersion);
    checkbutton_reload_doc_last->set_active(_pConfig->reloadDocLast);
    checkbutton_start_dialog->set_active(_pConfig->showStartDialog);
    checkbutton_mod_time_sentinel->set_active(_pConfig->modTimeSentinel);

    Gtk::Frame* frame_misc_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_misc);

    auto vbox_system_tray = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto checkbutton_systray = Gtk::manage(new Gtk::CheckButton{_("Enable System Tray Docking")});
    auto checkbutton_start_on_systray = Gtk::manage(new Gtk::CheckButton{_("Start Minimized in the System Tray")});
    vbox_system_tray->pack_start(*checkbutton_systray, false, false);
    vbox_system_tray->pack_start(*checkbutton_start_on_systray, false, false);

    Gtk::Frame* frame_system_tray = new_managed_frame_with_align(_("System Tray"), vbox_system_tray);

    checkbutton_systray->set_active(_pConfig->systrayOn);
    checkbutton_start_on_systray->set_active(_pConfig->startOnSystray);
    checkbutton_start_on_systray->set_sensitive(_pConfig->systrayOn);

#ifdef HAVE_NLS
    auto f_getButtonLabel = [this](const std::string langCode)->Glib::ustring{
        auto it = _mapCountryLanguages.find(langCode);
        return it != _mapCountryLanguages.end() ? it->second : _("System Default");
    };
    auto f_getStockId = [](const std::string langCode)->std::string{
        return langCode == CtConst::LANG_DEFAULT ? "ct_node_no_icon" : "ct_" + langCode;
    };
    const auto currLangId = CtMiscUtil::get_ct_language();
    auto button_country_language = Gtk::manage(new Gtk::Button{});
    button_country_language->set_label(f_getButtonLabel(currLangId));
    button_country_language->set_image(*_pCtMainWin->new_managed_image_from_stock(f_getStockId(currLangId), Gtk::ICON_SIZE_MENU));
    button_country_language->set_always_show_image(true);
    Gtk::Frame* frame_language = new_managed_frame_with_align(_("Language"), button_country_language);
#endif

    auto hbox_proxy_url_port = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 2/*spacing*/});
    auto hbox_proxy_user_passwd = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 2/*spacing*/});
    auto vbox_proxy = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 0/*spacing*/});

    auto label_proxy_url = Gtk::manage(new Gtk::Label{_("URL")});
    auto entry_proxy_url = Gtk::manage(new Gtk::Entry{});
    auto label_proxy_port = Gtk::manage(new Gtk::Label{_("Port")});
    auto entry_proxy_port = Gtk::manage(new Gtk::Entry{});
    hbox_proxy_url_port->pack_start(*label_proxy_url, false, false);
    hbox_proxy_url_port->pack_start(*entry_proxy_url, true, true);
    hbox_proxy_url_port->pack_start(*label_proxy_port, false, false);
    hbox_proxy_url_port->pack_start(*entry_proxy_port, false, false);

    if (not _pConfig->proxyUrlColonPort.empty()) {
        std::vector<std::string> proxyUrlAndPort = str::split(_pConfig->proxyUrlColonPort, ":");
        if (proxyUrlAndPort.size() > 0) {
            entry_proxy_url->set_text(proxyUrlAndPort[0]);
            if (proxyUrlAndPort.size() > 1) {
                entry_proxy_port->set_text(proxyUrlAndPort[1]);
            }
        }
    }

    auto label_proxy_user = Gtk::manage(new Gtk::Label{_("Username")});
    auto entry_proxy_user = Gtk::manage(new Gtk::Entry{});
    auto label_proxy_passwd = Gtk::manage(new Gtk::Label{_("Password")});
    auto entry_proxy_passwd = Gtk::manage(new Gtk::Entry{});
    entry_proxy_passwd->set_visibility(false);
    hbox_proxy_user_passwd->pack_start(*label_proxy_user, false, false);
    hbox_proxy_user_passwd->pack_start(*entry_proxy_user, true, true);
    hbox_proxy_user_passwd->pack_start(*label_proxy_passwd, false, false);
    hbox_proxy_user_passwd->pack_start(*entry_proxy_passwd, true, true);

    if (not _pConfig->proxyUsername.empty()) {
        entry_proxy_user->set_text(_pConfig->proxyUsername);
    }
    if (not _pConfig->proxyPassword.empty()) {
        entry_proxy_passwd->set_text(_pConfig->proxyPassword);
    }

    auto f_on_proxy_url_port_changed = [this, entry_proxy_url, entry_proxy_port](){
        const Glib::ustring url = str::trim(entry_proxy_url->get_text());
        const Glib::ustring port = str::trim(entry_proxy_port->get_text());
        if (port.empty()) {
            _pConfig->proxyUrlColonPort = url;
        }
        else {
            _pConfig->proxyUrlColonPort = url + ":" + port;
        }
    };
    entry_proxy_url->signal_changed().connect(f_on_proxy_url_port_changed);
    entry_proxy_port->signal_changed().connect(f_on_proxy_url_port_changed);
    entry_proxy_user->signal_changed().connect([this, entry_proxy_user](){
        _pConfig->proxyUsername = entry_proxy_user->get_text();
    });
    entry_proxy_passwd->signal_changed().connect([this, entry_proxy_passwd](){
        _pConfig->proxyPassword = entry_proxy_passwd->get_text();
    });

    vbox_proxy->pack_start(*hbox_proxy_url_port);
    vbox_proxy->pack_start(*hbox_proxy_user_passwd);
    Gtk::Frame* frame_proxy = new_managed_frame_with_align(_("Proxy"), vbox_proxy);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_saving, false, false);
    pMainBox->pack_start(*frame_misc_misc, false, false);
    pMainBox->pack_start(*frame_system_tray, false, false);
#ifdef HAVE_NLS
    pMainBox->pack_start(*frame_language, false, false);
#endif
    pMainBox->pack_start(*frame_proxy, false, false);

    checkbutton_autosave->signal_toggled().connect([this, checkbutton_autosave, spinbutton_autosave](){
        _pConfig->autosaveOn = checkbutton_autosave->get_active();
        _pCtMainWin->file_autosave_restart();
        spinbutton_autosave->set_sensitive(_pConfig->autosaveOn);
    });
    spinbutton_autosave->signal_value_changed().connect([this, spinbutton_autosave](){
        _pConfig->autosaveMinutes = spinbutton_autosave->get_value_as_int();
        _pCtMainWin->file_autosave_restart();
    });
    checkbutton_autosave_on_quit->signal_toggled().connect([this, checkbutton_autosave_on_quit](){
        _pConfig->autosaveOnQuit = checkbutton_autosave_on_quit->get_active();
    });
    checkbutton_backup_before_saving->signal_toggled().connect([this,
                                                                checkbutton_backup_before_saving,
                                                                spinbutton_num_backups,
                                                                checkbutton_custom_backup_dir,
                                                                file_chooser_button_backup_dir](){
        _pConfig->backupCopy = checkbutton_backup_before_saving->get_active();
        spinbutton_num_backups->set_sensitive(_pConfig->backupCopy);
        checkbutton_custom_backup_dir->set_sensitive(_pConfig->backupCopy);
        file_chooser_button_backup_dir->set_sensitive(_pConfig->backupCopy and _pConfig->customBackupDirOn);
    });
    spinbutton_num_backups->signal_value_changed().connect([this, spinbutton_num_backups](){
        _pConfig->backupNum = spinbutton_num_backups->get_value_as_int();
    });
    checkbutton_custom_backup_dir->signal_toggled().connect([this, checkbutton_custom_backup_dir, file_chooser_button_backup_dir](){
        _pConfig->customBackupDirOn = checkbutton_custom_backup_dir->get_active();
        file_chooser_button_backup_dir->set_sensitive(checkbutton_custom_backup_dir->get_active());
    });
    file_chooser_button_backup_dir->signal_file_set().connect([this, file_chooser_button_backup_dir](){
        _pConfig->customBackupDir = file_chooser_button_backup_dir->get_filename();
    });
    checkbutton_debug_log->signal_toggled().connect([this, checkbutton_debug_log, file_chooser_button_debug_log_dir](){
        if (checkbutton_debug_log->get_active()) {
            Glib::file_set_contents(fs::get_cherrytree_logcfg_filepath().string(), "");
        }
        else {
            fs::remove(fs::get_cherrytree_logcfg_filepath());
        }
        file_chooser_button_debug_log_dir->set_sensitive(checkbutton_debug_log->get_active());
        need_restart(RESTART_REASON::DEBUG_LOG);
    });
    file_chooser_button_debug_log_dir->signal_file_set().connect([this, file_chooser_button_debug_log_dir](){
        Glib::file_set_contents(fs::get_cherrytree_logcfg_filepath().string(),
                                file_chooser_button_debug_log_dir->get_filename());
        need_restart(RESTART_REASON::DEBUG_LOG);
    });
    checkbutton_mfname_on_disk->signal_toggled().connect([this, checkbutton_mfname_on_disk](){
        _pConfig->embfileMFNameOnDisk = checkbutton_mfname_on_disk->get_active();
    });
    checkbutton_reload_doc_last->signal_toggled().connect([this, checkbutton_reload_doc_last](){
        _pConfig->reloadDocLast = checkbutton_reload_doc_last->get_active();
    });
    checkbutton_start_dialog->signal_toggled().connect([this, checkbutton_start_dialog](){
        _pConfig->showStartDialog = checkbutton_start_dialog->get_active();
    });
    checkbutton_mod_time_sentinel->signal_toggled().connect([this, checkbutton_mod_time_sentinel](){
        _pConfig->modTimeSentinel = checkbutton_mod_time_sentinel->get_active();
        _pCtMainWin->mod_time_sentinel_restart();
    });
    checkbutton_newer_version->signal_toggled().connect([this, checkbutton_newer_version](){
        _pConfig->checkVersion = checkbutton_newer_version->get_active();
    });

    // cannot just turn on systray icon, we have to check if systray exists
    checkbutton_systray->signal_toggled().connect([this, checkbutton_systray, checkbutton_start_on_systray](){
        if (checkbutton_systray->get_active()) {
            _pCtMainWin->get_status_icon()->set_visible(true);
#if defined(_WIN32)
            _pConfig->systrayOn = true; // windows does support the systray
#else // !_WIN32
            _pConfig->systrayOn = CtDialogs::question_dialog(_("Has the System Tray appeared on the panel?"), *this);
#endif // !_WIN32
            if (_pConfig->systrayOn) {
                checkbutton_start_on_systray->set_sensitive(true);
                apply_for_each_window([](CtMainWin* win) { win->menu_set_visible_exit_app(true); });
            }
            else {
                CtDialogs::warning_dialog(_("Your system does not support the System Tray"), *this);
                checkbutton_systray->set_active(false);
            }
        }
        else {
            _pConfig->systrayOn = false;
            _pCtMainWin->get_status_icon()->set_visible(false);
            apply_for_each_window([](CtMainWin* win) { win->menu_set_visible_exit_app(false); });
            checkbutton_start_on_systray->set_sensitive(false);
        }
        checkbutton_systray->get_parent()->grab_focus();
    });
    checkbutton_start_on_systray->signal_toggled().connect([this, checkbutton_start_on_systray](){
        _pConfig->startOnSystray = checkbutton_start_on_systray->get_active();
    });

#ifdef HAVE_NLS
    button_country_language->signal_clicked().connect([this, button_country_language, f_getStockId, f_getButtonLabel](){

        struct customCompare {
            bool operator()(const std::pair<std::string, Glib::ustring>& l, const std::pair<std::string, Glib::ustring>& r) const {
                return l.second < r.second;
            }
        };
        std::set<std::pair<std::string, Glib::ustring>, customCompare> orderedSet{_mapCountryLanguages.begin(), _mapCountryLanguages.end()};

        Glib::RefPtr<CtChooseDialogListStore> rItemStore = CtChooseDialogListStore::create();
        const std::string currLang = CtMiscUtil::get_ct_language();
        rItemStore->add_row(f_getStockId(CtConst::LANG_DEFAULT), CtConst::LANG_DEFAULT, _("System Default"));
        unsigned pathSelectIdx{0};
        unsigned pathCurrIdx{1};
        for (const auto& currPair : orderedSet) {
            rItemStore->add_row(f_getStockId(currPair.first), currPair.first, currPair.second);
            if (currLang == currPair.first) {
                pathSelectIdx = pathCurrIdx;
            }
            ++pathCurrIdx;
        }
        Gtk::TreeModel::iterator res = CtDialogs::choose_item_dialog(*this,
                                                          _("Language"),
                                                          rItemStore,
                                                          nullptr/*single_column_name*/,
                                                          std::to_string(pathSelectIdx),
                                                          std::make_pair(200, _pConfig->winRect[3]));
        if (res) {
            const Glib::ustring selLangId = res->get_value(rItemStore->columns.key);
            button_country_language->set_label(f_getButtonLabel(selLangId));
            button_country_language->set_image(*_pCtMainWin->new_managed_image_from_stock(f_getStockId(selLangId), Gtk::ICON_SIZE_MENU));
            need_restart(RESTART_REASON::LANG);
            g_file_set_contents(fs::get_cherrytree_langcfg_filepath().c_str(),
                                selLangId.c_str(), (gssize)selLangId.bytes(), nullptr);
        }
    });
#endif

    return pMainBox;
}
