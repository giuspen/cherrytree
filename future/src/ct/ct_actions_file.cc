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
            if (false == _is_tree_not_empty_or_error())
            {
                
            }
            _pCtMainWin->curr_file_mod_time_update_value(true/*doEnable*/);
        }
        // todo
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
    if (!_is_tree_not_empty_or_error()) return;
    // todo: support all document types and destination path
    {
        const Glib::ustring filepath{"/tmp/test.ctb"};
        if (CtFileSystem::isfile(filepath))
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
        const Glib::ustring filepath{"/tmp/test.ctd"};
        ctXmlWrite.write_to_file(filepath);
        std::cout << "written " << filepath << std::endl;
    }
}
