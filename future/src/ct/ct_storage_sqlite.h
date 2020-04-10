/*
 * ct_storage_sqlite.h
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
#include <sqlite3.h>
#include <glibmm/refptr.h>
#include <gtksourceviewmm/buffer.h>
#include <gtkmm/treeiter.h>

class CtMainWin;
class CtAnchoredWidget;
class CtTreeIter;

class CtStorageSqlite : public CtStorageEntity
{
public:
    CtStorageSqlite(CtMainWin* pCtMainWin);
    ~CtStorageSqlite();

    void close_connect() override;
    void reopen_connect() override;
    void test_connection() override;

    bool populate_treestore(const Glib::ustring& file_path, Glib::ustring& error) override;
    bool save_treestore(const Glib::ustring& file_path, const CtStorageSyncPending& syncPending, Glib::ustring& error) override;

    Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                      const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& widgets) const override;
private:
    void                _close_db();

    Gtk::TreeIter       _node_from_db(guint node_id, Gtk::TreeIter parent_iter);
    void                _image_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const;
    void                _codebox_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const;
    void                _table_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const;

    void                _create_all_tables_in_db();
    void                _write_bookmarks_to_db(const std::list<gint64>& bookmarks);
    bool                _write_node_to_db(CtTreeIter* ct_tree_iter,
                                          const gint64 sequence,
                                          const gint64 node_father_id,
                                          const CtStorageNodeState& write_dict,
                                          const int start_offset, const int end_offset);

    std::list<gint64>   _get_children_node_ids_from_db(gint64 father_id);
    void                _remove_db_node_with_children(const gint64 node_id);

    void                _exec_no_callback(const char* sqlCmd);
    void                _exec_bind_int64(const char* sqlCmd, const gint64 bind_int64);

private:
    CtMainWin*    _pCtMainWin;
    sqlite3*      _pDb{nullptr};
    Glib::ustring _file_path;
};
