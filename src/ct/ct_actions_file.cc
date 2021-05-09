/*
 * ct_actions_file.cc
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

#include "ct_actions.h"
#include "ct_storage_control.h"
#include "ct_pref_dlg.h"

void CtActions::_file_save(bool need_vacuum)
{
    if (not _is_tree_not_empty_or_error())
        return;
    if (_pCtMainWin->get_ct_storage()->get_file_path().empty())
        file_save_as();
    else
        _pCtMainWin->file_save(need_vacuum);
}

void CtActions::file_new()
{
    _pCtMainWin->signal_app_new_instance();
}

// Save the file
void CtActions::file_save()
{
    _file_save(false);
}

// Save the file and vacuum the db
void CtActions::file_vacuum()
{
    _file_save(true);
}

// Save the file providing a new name
void CtActions::file_save_as()
{
    if (not _is_tree_not_empty_or_error())
    {
        return;
    }
    CtDialogs::storage_select_args storageSelArgs(_pCtMainWin);
    fs::path currDocFilepath = _pCtMainWin->get_ct_storage()->get_file_path();
    if (not currDocFilepath.empty())
    {
        storageSelArgs.ctDocType = fs::get_doc_type(currDocFilepath);
        storageSelArgs.ctDocEncrypt = fs::get_doc_encrypt(currDocFilepath);
    }
    if (not CtDialogs::choose_data_storage_dialog(storageSelArgs))
    {
        return;
    }
    CtDialogs::FileSelectArgs fileSelArgs{_pCtMainWin};
    if (not currDocFilepath.empty())
    {
        fileSelArgs.curr_folder = currDocFilepath.parent_path();
        fs::path suggested_basename = currDocFilepath.filename();
        fileSelArgs.curr_file_name = suggested_basename.stem().string() + CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
    }
    fileSelArgs.filter_name = _("CherryTree Document");
    std::string fileExtension = CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
    fileSelArgs.filter_pattern.push_back(std::string{CtConst::CHAR_STAR}+fileExtension);
    std::string filepath = CtDialogs::file_save_as_dialog(fileSelArgs);
    if (filepath.empty())
    {
        return;
    }

    CtMiscUtil::filepath_extension_fix(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt, filepath);
    _pCtMainWin->file_save_as(filepath, storageSelArgs.password);
}

void CtActions::file_open()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtMainWin->get_ct_storage()->get_file_dir();
    args.filter_name = _("CherryTree Document");
    args.filter_pattern.push_back("*.ctb"); // macos doesn't understand *.ct*
    args.filter_pattern.push_back("*.ctx");
    args.filter_pattern.push_back("*.ctd");
    args.filter_pattern.push_back("*.ctz");

    std::string filepath = CtDialogs::file_select_dialog(args);

    if (filepath.empty()) return;

    _pCtMainWin->file_open(filepath, "");
}

void CtActions::quit_or_hide_window()
{
    _pCtMainWin->signal_app_quit_or_hide_window(_pCtMainWin);
}

void CtActions::quit_window()
{
    _pCtMainWin->signal_app_quit_window(_pCtMainWin);
}

void CtActions::dialog_preferences()
{
    _pCtMainWin->get_text_view().synch_spell_check_change_from_gspell_right_click_menu();
    CtPrefDlg prefDlg(_pCtMainWin);
    prefDlg.show();
    prefDlg.run();
}

void CtActions::preferences_import()
{
    spdlog::debug(__FUNCTION__);
}

void CtActions::preferences_export()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    const time_t time = std::time(nullptr);
    args.curr_file_name = std::string{"config_"} + str::time_format("%Y.%m.%d_%H.%M.%S", time) + ".cfg";
    args.filter_name = _("Preferences File");
    args.filter_pattern = {"*.cfg"};
    const std::string filepath = CtDialogs::file_save_as_dialog(args);
    _pCtMainWin->config_update_data_from_curr_status();
    _pCtMainWin->get_ct_config()->write_to_file(filepath);
}

void CtActions::command_palette()
{
    std::string id = CtDialogs::dialog_palette(_pCtMainWin);
    if (CtMenuAction* action = _pCtMainWin->get_ct_menu().find_action(id))
        action->run_action();
}
