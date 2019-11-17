/*
 * ct_actions_file.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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
            if ( (false == _is_tree_not_empty_or_error()) &&
                 _file_write(doc_filepath, false/*firstWrite*/) )
            {
                _pCtMainWin->update_window_save_not_needed();
                // ? self.state_machine.update_state()
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
    if (false == _is_tree_not_empty_or_error())
    {
        return;
    }
    CtDialogs::storage_select_args storageSelArgs{ .pParentWin = _pCtMainWin };
    std::string currDocFilepath = _pCtMainWin->get_curr_doc_file_path();
    if (false == currDocFilepath.empty())
    {
        storageSelArgs.ctDocType = CtMiscUtil::get_doc_type(currDocFilepath);
        storageSelArgs.ctDocEncrypt = CtMiscUtil::get_doc_encrypt(currDocFilepath);
    }
    if (false == CtDialogs::choose_data_storage_dialog(storageSelArgs))
    {
        return;
    }
    CtDialogs::file_select_args fileSelArgs{ .pParentWin = _pCtMainWin };
    if (false == currDocFilepath.empty())
    {
        fileSelArgs.curr_folder = Glib::path_get_dirname(currDocFilepath);
        fileSelArgs.curr_file_name = Glib::path_get_basename(currDocFilepath);
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
    if (_file_write(filepath, true/*firstWrite*/))
    {
        _pCtMainWin->set_new_curr_doc(filepath);
        // support.add_recent_document(self, filepath)
        _pCtMainWin->update_window_save_not_needed();
        // ? self.state_machine.update_state()
    }
    _pCtMainWin->curr_file_mod_time_update_value(true/*doEnable*/);
#if 0
    // todo: support all document types and destination path
    {
        const std::string filepath{"/tmp/test.ctb"};
        if (Glib::file_test(filepath, Glib::FILE_TEST_IS_REGULAR))
        {
            g_remove(filepath.c_str());
        }
        CtSQLite ctSQLite(filepath.c_str());
        if (ctSQLite.write_db_full(_pCtTreestore->get_bookmarks(), _pCtTreestore->get_ct_iter_first()))
        {
            std::cout << "written " << filepath << std::endl;
        }
        else
        {
            std::cerr << "!! " << filepath << std::endl;
        }
    }
    {
        CtXmlWrite ctXmlWrite(CtConst::APP_NAME);
        ctXmlWrite.treestore_to_dom(_pCtTreestore->get_bookmarks(), _pCtTreestore->get_ct_iter_first());
        const std::string filepath{"/tmp/test.ctd"};
        ctXmlWrite.write_to_file(filepath);
        std::cout << "written " << filepath << std::endl;
    }
#endif // 0
}

bool CtActions::_file_write(const std::string& filepath, const bool firstWrite)
{
    // todo
    return false;
}
