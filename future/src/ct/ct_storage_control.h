/*
 * ct_storage_control.h
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

#pragma once
#include "ct_types.h"

class CtMainWin;
class CtStorageControl
{
public:
    static CtStorageControl* create_dummy_storage();
    static CtStorageControl* load_from(CtMainWin* pCtMainWin, const Glib::ustring& file_path, Glib::ustring& error);
    static CtStorageControl* save_as(CtMainWin* pCtMainWin, const Glib::ustring& file_path, const Glib::ustring& password, Glib::ustring& error);

public:
    bool save(Glib::ustring& error);
    void vacuum();

 private:
    CtStorageControl() = default;

    static Glib::ustring _extract_file(CtMainWin* pCtMainWin, const Glib::ustring& file_path, Glib::ustring& password);
    static bool          _package_file(const Glib::ustring& file_from, const Glib::ustring& file_to, const Glib::ustring& password);

    void _put_in_backup(const Glib::ustring& main_backup);

public:
    Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                      const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& widgets) const;

    Glib::ustring get_file_path() { return _file_path; }
    Glib::ustring get_file_name() { return _file_path.empty() ? "" : Glib::path_get_basename(_file_path); }
    Glib::ustring get_file_dir()  { return _file_path.empty() ? "" : Glib::path_get_dirname(_file_path); }

    std::set<gint64> get_nodes_pending_rm() { return _syncPending.nodes_to_rm_set; }

    void pending_edit_db_node_prop(gint64 node_id);
    void pending_edit_db_node_buff(gint64 node_id);
    void pending_edit_db_node_hier(gint64 node_id);
    void pending_new_db_node(gint64 node_id);
    void pending_rm_db_nodes(const std::vector<gint64>& node_ids);
    void pending_edit_db_bookmarks();
private:

private:
    CtMainWin*           _pCtMainWin{nullptr};
    Glib::ustring        _file_path;
    Glib::ustring        _password;
    Glib::ustring        _extracted_file_path;
    bool                 _need_backup{true};   // create a backup once, on the first saving
    CtStorageEntity*     _storage{nullptr};
    CtStorageSyncPending _syncPending;
};
