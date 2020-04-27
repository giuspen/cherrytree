/*
 * ct_actions_file.cc
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

#include "ct_actions.h"
#include "ct_storage_control.h"
#include "ct_pref_dlg.h"
#include <glib/gstdio.h>

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
    CtDialogs::storage_select_args storageSelArgs{ .pParentWin = _pCtMainWin };
    std::string currDocFilepath = _pCtMainWin->get_ct_storage()->get_file_path();
    if (not currDocFilepath.empty())
    {
        storageSelArgs.ctDocType = CtMiscUtil::get_doc_type(currDocFilepath);
        storageSelArgs.ctDocEncrypt = CtMiscUtil::get_doc_encrypt(currDocFilepath);
    }
    if (not CtDialogs::choose_data_storage_dialog(storageSelArgs))
    {
        return;
    }
    CtDialogs::file_select_args fileSelArgs{ .pParentWin = _pCtMainWin };
    if (not currDocFilepath.empty())
    {
        fileSelArgs.curr_folder = Glib::path_get_dirname(currDocFilepath);
        const std::string suggested_basename = Glib::path_get_basename(currDocFilepath);
        fileSelArgs.curr_file_name = suggested_basename.substr(0, suggested_basename.size()-4)+CtMiscUtil::get_doc_extension(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt);
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
    CtDialogs::file_select_args args;
    args.pParentWin = _pCtMainWin;
    args.curr_folder = _pCtMainWin->get_ct_storage()->get_file_dir();
    args.filter_name = _("CherryTree Document");
    args.filter_pattern.push_back("*.ct*");
    std::string filepath = CtDialogs::file_select_dialog(args);

    if (filepath.empty()) return;

    _pCtMainWin->file_open(filepath, "");
}

void CtActions::folder_cfg_open()
{
    CtFileSystem::external_folderpath_open(Glib::build_filename(Glib::get_user_config_dir(), CtConst::APP_NAME));
}

void CtActions::online_help()
{
    g_app_info_launch_default_for_uri("http://giuspen.com/cherrytreemanual/", nullptr, nullptr);
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
    CtPrefDlg prefDlg(_pCtMainWin);
    prefDlg.show();
    prefDlg.run();
}

void CtActions::dialog_about()
{
    CtDialogs::dialog_about(*_pCtMainWin, _pCtMainWin->get_icon_theme()->load_icon(CtConst::APP_NAME, 128));
}

void CtActions::command_palette()
{
    std::string id = CtDialogs::dialog_pallete(_pCtMainWin);
    if (CtMenuAction* action = _pCtMainWin->get_ct_menu().find_action(id))
        action->run_action();
}
