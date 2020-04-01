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
#include "ct_doc_rw.h"
#include "ct_p7za_iface.h"
#include <glib/gstdio.h>

void CtActions::_file_save(const bool run_vacuum)
{
    const std::string doc_filepath = _pCtMainWin->get_curr_doc_file_path();
    if (doc_filepath.empty())
    {
        file_save_as();
    }
    else
    {
        if (_pCtMainWin->get_file_save_needed())
        {
            _pCtMainWin->curr_file_mod_time_update_value(false/*doEnable*/);
            if ( _is_tree_not_empty_or_error() and
                 _file_write(doc_filepath, _pCtMainWin->get_curr_doc_password(), false/*firstWrite*/, nullptr/*ppReturnCtSQLite*/, run_vacuum) )
            {
                _pCtMainWin->update_window_save_not_needed();
                _pCtMainWin->get_state_machine().update_state();
            }
            _pCtMainWin->curr_file_mod_time_update_value(true/*doEnable*/);
        }
    }
}

// Save the file
void CtActions::file_save()
{
    _file_save();
}

// Save the file and vacuum the db
void CtActions::file_vacuum()
{
    _file_save(true/*run_vacuum*/);
}

// Save the file providing a new name
void CtActions::file_save_as()
{
    if (not _is_tree_not_empty_or_error())
    {
        return;
    }
    CtDialogs::storage_select_args storageSelArgs{ .pParentWin = _pCtMainWin };
    std::string currDocFilepath = _pCtMainWin->get_curr_doc_file_path();
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
    _pCtMainWin->curr_file_mod_time_update_value(false/*doEnable*/);
    CtMiscUtil::filepath_extension_fix(storageSelArgs.ctDocType, storageSelArgs.ctDocEncrypt, filepath);
    CtSQLite* pReturnCtSQLite{nullptr};
    if (_file_write(filepath, storageSelArgs.password, true/*firstWrite*/, &pReturnCtSQLite))
    {
        _pCtMainWin->set_new_curr_doc(filepath, storageSelArgs.password, pReturnCtSQLite);
        // support.add_recent_document(self, filepath)
        _pCtMainWin->update_window_save_not_needed();
        _pCtMainWin->get_state_machine().update_state();
    }
    _pCtMainWin->curr_file_mod_time_update_value(true/*doEnable*/);
}

bool CtActions::_backups_handling(const std::string& filepath)
{
    bool retVal{true};
    if (_pCtMainWin->get_ct_config()->backupCopy and _pCtMainWin->get_ct_config()->backupNum > 0)
    {
        std::string tildesToBackup;
        int tildesLeft{_pCtMainWin->get_ct_config()->backupNum-1};
        while (tildesLeft > 0)
        {
            tildesToBackup += CtConst::CHAR_TILDE;
            tildesLeft--;
        }
        std::string filepathTildes = filepath + tildesToBackup;
        for (int b=_pCtMainWin->get_ct_config()->backupNum; b>0; b--)
        {
            Glib::RefPtr<Gio::File> rFileFrom = Gio::File::create_for_path(filepathTildes);
            if (rFileFrom->query_exists())
            {
                const std::string fileToFilepath{filepathTildes+CtConst::CHAR_TILDE};
                Glib::RefPtr<Gio::File> rFileTo = Gio::File::create_for_path(fileToFilepath);
                if (Glib::file_test(fileToFilepath, Glib::FILE_TEST_IS_REGULAR) && (0 != g_remove(fileToFilepath.c_str())))
                {
                    std::cerr << "!! g_remove" << std::endl;
                }
                if ( (b > 1) or
                     (CtDocType::XML == CtMiscUtil::get_doc_type(filepath)) or
                     (CtDocEncrypt::True == CtMiscUtil::get_doc_encrypt(filepath)) )
                {
                    // from and to are both backups or
                    // from is our document but xml which we don't keep open or
                    // from is our document sqlite but after encryption so not the open db
                    retVal = rFileFrom->move(rFileTo);
                }
                else
                {
                    // from is the sqlite document we have open, cannot move it
                    retVal = rFileFrom->copy(rFileTo);
                }
                if (not retVal)
                {
                    // write access issues
                    break;
                }
            }
            if (b > 1)
            {
                filepathTildes = filepathTildes.substr(0, filepathTildes.size()-1);
            }
        }
    }
    return retVal;
}

bool CtActions::_file_write(const std::string& filepath,
                            const std::string& password,
                            const bool firstWrite,
                            CtSQLite** ppReturnCtSQLite,
                            const bool run_vacuum)
{
    if (not _backups_handling(filepath))
    {
        g_autofree gchar* title = g_strdup_printf(_("You Have No Write Access to %s"), Glib::path_get_dirname(filepath).c_str());
        CtDialogs::error_dialog(title, *_pCtMainWin);
        return false;
    }
    // self.statusbar.push(self.statusbar_context_id, _("Writing to Disk..."))
    while (gtk_events_pending()) gtk_main_iteration();
    bool retVal = _file_write_low_level(filepath, password, firstWrite, ppReturnCtSQLite, run_vacuum);
    // self.statusbar.pop(self.statusbar_context_id)
    return retVal;
}

bool CtActions::_file_write_low_level(const std::string& filepath,
                                      const std::string& password,
                                      const bool firstWrite,
                                      CtSQLite** ppReturnCtSQLite,
                                      const bool run_vacuum,
                                      const CtExporting exporting,
                                      const std::pair<int,int>& offset_range)
{
    bool retVal{false};
    const CtDocEncrypt docEncrypt = CtMiscUtil::get_doc_encrypt(filepath);
    const CtDocType docType = CtMiscUtil::get_doc_type(filepath);
    const char* filepath_tmp = (CtDocEncrypt::True == docEncrypt ? _pCtMainWin->get_ct_tmp()->getHiddenFilePath(filepath) : filepath.c_str());
    if (CtDocType::XML == docType)
    {
        // xml, full
        CtXmlWrite ctXmlWrite(CtConst::APP_NAME);
        ctXmlWrite.treestore_to_dom(_pCtMainWin->curr_tree_store().get_bookmarks(), _pCtMainWin->curr_tree_store().get_ct_iter_first());
        ctXmlWrite.write_to_file(filepath_tmp);
        std::cout << "W " << filepath_tmp << std::endl;
        retVal = true;
    }
    else if ( firstWrite or
              (CtExporting::No != exporting) )
    {
        // sqlite, full
        CtSQLite* pCtSQLite = new CtSQLite(_pCtMainWin, filepath_tmp);
        if (pCtSQLite->write_db_full(_pCtMainWin->curr_tree_store().get_bookmarks(),
                                     _pCtMainWin->curr_tree_store().get_ct_iter_first(),
                                     exporting,
                                     offset_range))
        {
            if (nullptr != ppReturnCtSQLite)
            {
                *ppReturnCtSQLite = pCtSQLite;
            }
            else
            {
                delete pCtSQLite;
            }
            std::cout << "W " << filepath_tmp << std::endl;
            retVal = true;
        }
        else
        {
            std::cerr << "!! W " << filepath_tmp << std::endl;
        }
    }
    else
    {
        // sqlite, update
        retVal = _pCtMainWin->curr_tree_store().pending_data_write(run_vacuum);
    }
    if ( retVal and
         (CtDocEncrypt::True == docEncrypt) )
    {
        retVal = ( (0 == CtP7zaIface::p7za_archive(filepath_tmp,
                                                   filepath.c_str(),
                                                   password.c_str())) and
                    Glib::file_test(filepath, Glib::FILE_TEST_IS_REGULAR) );
    }
    return retVal;
}

void CtActions::file_open()
{
    CtDialogs::file_select_args args = {
        .pParentWin=_pCtMainWin,
        .curr_folder=_pCtMainWin->get_curr_doc_file_dir()
    };
    args.filter_name = _("CherryTree Document");
    args.filter_pattern.push_back("*.ct*");
    std::string filepath = CtDialogs::file_select_dialog(args);
    if (not filepath.empty() and
        _pCtMainWin->filepath_open(filepath))
    {
        _pCtMainWin->get_ct_config()->recentDocsFilepaths.move_or_push_front(filepath);
        _pCtMainWin->set_menu_items_recent_documents();
    }
}

void CtActions::folder_cfg_open()
{
    CtFileSystem::external_folderpath_open(Glib::build_filename(Glib::get_user_config_dir(), CtConst::APP_NAME));
}

void CtActions::online_help()
{
    g_app_info_launch_default_for_uri("http://giuspen.com/cherrytreemanual/", nullptr, nullptr);
}
