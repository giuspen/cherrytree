/*
 * ct_pref_dlg_misc.cc
 *
 * Copyright 2009-2021
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
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_system_tray = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_systray = Gtk::manage(new Gtk::CheckButton(_("Enable System Tray Docking")));
    Gtk::CheckButton* checkbutton_start_on_systray = Gtk::manage(new Gtk::CheckButton(_("Start Minimized in the System Tray")));
    vbox_system_tray->pack_start(*checkbutton_systray, false, false);
    vbox_system_tray->pack_start(*checkbutton_start_on_systray, false, false);

    Gtk::Frame* frame_system_tray = new_managed_frame_with_align(_("System Tray"), vbox_system_tray);

    checkbutton_systray->set_active(pConfig->systrayOn);
    checkbutton_start_on_systray->set_active(pConfig->startOnSystray);
    checkbutton_start_on_systray->set_sensitive(pConfig->systrayOn);

    Gtk::VBox* vbox_saving = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_autosave = Gtk::manage(new Gtk::HBox());
    hbox_autosave->set_spacing(4);
    Gtk::CheckButton* checkbutton_autosave = Gtk::manage(new Gtk::CheckButton(_("Autosave Every")));
    Glib::RefPtr<Gtk::Adjustment> adjustment_autosave = Gtk::Adjustment::create(pConfig->autosaveVal, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_autosave = Gtk::manage(new Gtk::SpinButton(adjustment_autosave));
    Gtk::Label* label_autosave = Gtk::manage(new Gtk::Label(_("Minutes")));
    hbox_autosave->pack_start(*checkbutton_autosave, false, false);
    hbox_autosave->pack_start(*spinbutton_autosave, false, false);
    hbox_autosave->pack_start(*label_autosave, false, false);
    Gtk::CheckButton* checkbutton_autosave_on_quit = Gtk::manage(new Gtk::CheckButton(_("Autosave on Quit")));
    Gtk::CheckButton* checkbutton_backup_before_saving = Gtk::manage(new Gtk::CheckButton(_("Create a Backup Copy Before Saving")));
    Gtk::HBox* hbox_num_backups = Gtk::manage(new Gtk::HBox());
    hbox_num_backups->set_spacing(4);
    Gtk::Label* label_num_backups = Gtk::manage(new Gtk::Label(_("Number of Backups to Keep")));
    Glib::RefPtr<Gtk::Adjustment> adjustment_num_backups = Gtk::Adjustment::create(pConfig->backupNum, 1, 100, 1);
    Gtk::SpinButton* spinbutton_num_backups = Gtk::manage(new Gtk::SpinButton(adjustment_num_backups));
    spinbutton_num_backups->set_sensitive(pConfig->backupCopy);
    spinbutton_num_backups->set_value(pConfig->backupNum);
    Gtk::CheckButton* checkbutton_custom_backup_dir = Gtk::manage(new Gtk::CheckButton(_("Custom Backup Directory")));
    Gtk::Entry* entry_custom_backup_dir = Gtk::manage(new Gtk::Entry());
    entry_custom_backup_dir->property_editable() = false;
    Gtk::Button* button_custom_backup_dir = Gtk::manage(new Gtk::Button("..."));
    Gtk::HBox* hbox_custom_backup_dir = Gtk::manage(new Gtk::HBox());
    hbox_custom_backup_dir->set_spacing(4);

    hbox_num_backups->pack_start(*label_num_backups, false, false);
    hbox_num_backups->pack_start(*spinbutton_num_backups, false, false);
    hbox_custom_backup_dir->pack_start(*entry_custom_backup_dir);
    hbox_custom_backup_dir->pack_start(*button_custom_backup_dir, false, false);
    vbox_saving->pack_start(*hbox_autosave, false, false);
    vbox_saving->pack_start(*checkbutton_autosave_on_quit, false, false);
    vbox_saving->pack_start(*checkbutton_backup_before_saving, false, false);
    vbox_saving->pack_start(*hbox_num_backups, false, false);
    vbox_saving->pack_start(*checkbutton_custom_backup_dir, false, false);
    vbox_saving->pack_start(*hbox_custom_backup_dir, false, false);

    checkbutton_autosave->set_active(pConfig->autosaveOn);
    spinbutton_autosave->set_value(pConfig->autosaveVal);
    spinbutton_autosave->set_sensitive(pConfig->autosaveOn);
    checkbutton_autosave_on_quit->set_active(pConfig->autosaveOnQuit);
    checkbutton_backup_before_saving->set_active(pConfig->backupCopy);
    checkbutton_custom_backup_dir->set_sensitive(pConfig->backupCopy);
    checkbutton_custom_backup_dir->set_active(pConfig->customBackupDirOn);
    entry_custom_backup_dir->set_text(pConfig->customBackupDir);
    entry_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);
    button_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);

    Gtk::Frame* frame_saving = new_managed_frame_with_align(_("Saving"), vbox_saving);

    Gtk::VBox* vbox_misc_misc = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_newer_version = Gtk::manage(new Gtk::CheckButton(_("Automatically Check for Newer Version")));
    Gtk::CheckButton* checkbutton_word_count = Gtk::manage(new Gtk::CheckButton(_("Enable Word Count in Statusbar")));
    Gtk::CheckButton* checkbutton_reload_doc_last = Gtk::manage(new Gtk::CheckButton(_("Reload Document From Last Session")));
    Gtk::CheckButton* checkbutton_mod_time_sentinel = Gtk::manage(new Gtk::CheckButton(_("Reload After External Update to CT* File")));
    Gtk::CheckButton* checkbutton_win_title_doc_dir = Gtk::manage(new Gtk::CheckButton(_("Show the Document Directory in the Window Title")));
    vbox_misc_misc->pack_start(*checkbutton_newer_version, false, false);
    vbox_misc_misc->pack_start(*checkbutton_word_count, false, false);
    vbox_misc_misc->pack_start(*checkbutton_reload_doc_last, false, false);
    vbox_misc_misc->pack_start(*checkbutton_mod_time_sentinel, false, false);
    vbox_misc_misc->pack_start(*checkbutton_win_title_doc_dir, false, false);

    checkbutton_newer_version->set_active(pConfig->checkVersion);
    checkbutton_word_count->set_active(pConfig->wordCountOn);
    checkbutton_reload_doc_last->set_active(pConfig->reloadDocLast);
    checkbutton_mod_time_sentinel->set_active(pConfig->modTimeSentinel);
    checkbutton_win_title_doc_dir->set_active(pConfig->winTitleShowDocDir);

    Gtk::Frame* frame_misc_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_misc);

#ifdef HAVE_NLS
    Gtk::VBox* vbox_language = Gtk::manage(new Gtk::VBox());
    Gtk::ComboBoxText* combobox_country_language = Gtk::manage(new Gtk::ComboBoxText());
    combobox_country_language->append(CtConst::LANG_DEFAULT, "-");
    combobox_country_language->append("bg", _("Bulgarian"));
    combobox_country_language->append("cs", _("Czech"));
    combobox_country_language->append("de", _("German"));
    combobox_country_language->append("el", _("Greek"));
    combobox_country_language->append("en", _("English"));
    combobox_country_language->append("es", _("Spanish"));
    combobox_country_language->append("fi", _("Finnish"));
    combobox_country_language->append("fr", _("French"));
    combobox_country_language->append("hy", _("Armenian"));
    combobox_country_language->append("it", _("Italian"));
    combobox_country_language->append("ja", _("Japanese"));
    combobox_country_language->append("lt", _("Lithuanian"));
    combobox_country_language->append("nl", _("Dutch"));
    combobox_country_language->append("pl", _("Polish"));
    combobox_country_language->append("pt_BR", _("Portuguese Brazil"));
    combobox_country_language->append("ru", _("Russian"));
    combobox_country_language->append("sl", _("Slovenian"));
    combobox_country_language->append("sv", _("Swedish"));
    combobox_country_language->append("tr", _("Turkish"));
    combobox_country_language->append("uk", _("Ukrainian"));
    combobox_country_language->append("zh_CN", _("Chinese Simplified"));
    combobox_country_language->set_active_id(CtMiscUtil::get_ct_language());
    vbox_language->pack_start(*combobox_country_language, false, false);
    Gtk::Frame* frame_language = new_managed_frame_with_align(_("Language"), vbox_language);
#endif

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_system_tray, false, false);
    pMainBox->pack_start(*frame_saving, false, false);
    pMainBox->pack_start(*frame_misc_misc, false, false);
#ifdef HAVE_NLS
    pMainBox->pack_start(*frame_language, false, false);
#endif

    // cannot just turn on systray icon, we have to check if systray exists
    checkbutton_systray->signal_toggled().connect([this, pConfig, checkbutton_systray, checkbutton_start_on_systray](){
        if (checkbutton_systray->get_active()) {
            _pCtMainWin->get_status_icon()->set_visible(true);
            pConfig->systrayOn = CtDialogs::question_dialog(_("Has the System Tray appeared on the panel?"), *this);
            if (pConfig->systrayOn) {
                checkbutton_start_on_systray->set_sensitive(true);
                apply_for_each_window([](CtMainWin* win) { win->menu_set_visible_exit_app(true); });
            }
            else {
                CtDialogs::warning_dialog(_("Your system does not support the System Tray"), *_pCtMainWin);
                checkbutton_systray->set_active(false);
            }
        } else {
            pConfig->systrayOn = false;
            _pCtMainWin->get_status_icon()->set_visible(false);
            apply_for_each_window([](CtMainWin* win) { win->menu_set_visible_exit_app(false); });
            checkbutton_start_on_systray->set_sensitive(false);
        }
        checkbutton_systray->get_parent()->grab_focus();
    });
    checkbutton_start_on_systray->signal_toggled().connect([pConfig, checkbutton_start_on_systray](){
        pConfig->startOnSystray = checkbutton_start_on_systray->get_active();
    });
    checkbutton_autosave->signal_toggled().connect([this, pConfig, checkbutton_autosave, spinbutton_autosave](){
        pConfig->autosaveOn = checkbutton_autosave->get_active();
        _pCtMainWin->file_autosave_restart();
        spinbutton_autosave->set_sensitive(pConfig->autosaveOn);
    });
    spinbutton_autosave->signal_value_changed().connect([this, pConfig, spinbutton_autosave](){
        pConfig->autosaveVal = spinbutton_autosave->get_value_as_int();
        _pCtMainWin->file_autosave_restart();
    });
    checkbutton_autosave_on_quit->signal_toggled().connect([pConfig, checkbutton_autosave_on_quit](){
        pConfig->autosaveOnQuit = checkbutton_autosave_on_quit->get_active();
    });
    checkbutton_backup_before_saving->signal_toggled().connect([pConfig, checkbutton_backup_before_saving, spinbutton_num_backups, checkbutton_custom_backup_dir, entry_custom_backup_dir, button_custom_backup_dir](){
        pConfig->backupCopy = checkbutton_backup_before_saving->get_active();
        spinbutton_num_backups->set_sensitive(pConfig->backupCopy);
        checkbutton_custom_backup_dir->set_sensitive(pConfig->backupCopy);
        entry_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);
        button_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);
    });
    spinbutton_num_backups->signal_value_changed().connect([pConfig, spinbutton_num_backups](){
        pConfig->backupNum = spinbutton_num_backups->get_value_as_int();
    });
    checkbutton_custom_backup_dir->signal_toggled().connect([pConfig, checkbutton_custom_backup_dir, entry_custom_backup_dir, button_custom_backup_dir](){
        pConfig->customBackupDirOn = checkbutton_custom_backup_dir->get_active();
        entry_custom_backup_dir->set_sensitive(checkbutton_custom_backup_dir->get_active());
        button_custom_backup_dir->set_sensitive(checkbutton_custom_backup_dir->get_active());
    });
    button_custom_backup_dir->signal_clicked().connect([this, pConfig, entry_custom_backup_dir](){
        auto dir_place = CtDialogs::folder_select_dialog(pConfig->customBackupDir, _pCtMainWin);
        if (dir_place.empty()) return;
        entry_custom_backup_dir->set_text(dir_place);
        pConfig->customBackupDir = dir_place;
    });
    checkbutton_reload_doc_last->signal_toggled().connect([pConfig, checkbutton_reload_doc_last](){
        pConfig->reloadDocLast = checkbutton_reload_doc_last->get_active();
    });
    checkbutton_mod_time_sentinel->signal_toggled().connect([this, pConfig, checkbutton_mod_time_sentinel](){
        pConfig->modTimeSentinel = checkbutton_mod_time_sentinel->get_active();
        _pCtMainWin->mod_time_sentinel_restart();
    });
    checkbutton_win_title_doc_dir->signal_toggled().connect([this, pConfig, checkbutton_win_title_doc_dir](){
        pConfig->winTitleShowDocDir = checkbutton_win_title_doc_dir->get_active();
        _pCtMainWin->window_title_update();
    });
    checkbutton_newer_version->signal_toggled().connect([pConfig, checkbutton_newer_version](){
        pConfig->checkVersion = checkbutton_newer_version->get_active();
    });
    checkbutton_word_count->signal_toggled().connect([this, pConfig, checkbutton_word_count](){
        pConfig->wordCountOn = checkbutton_word_count->get_active();
        apply_for_each_window([](CtMainWin* win) { win->update_selected_node_statusbar_info(); });
    });
#ifdef HAVE_NLS
    combobox_country_language->signal_changed().connect([this, combobox_country_language](){
        Glib::ustring new_lang = combobox_country_language->get_active_id();
        need_restart(RESTART_REASON::LANG, _("The New Language will be Available Only After Restarting CherryTree"));
        g_file_set_contents(fs::get_cherrytree_lang_filepath().c_str(),
                            new_lang.c_str(), (gssize)new_lang.bytes(), nullptr);
    });
#endif

    return pMainBox;
}
