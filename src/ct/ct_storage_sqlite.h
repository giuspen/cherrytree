/*
 * ct_storage_sqlite.h
 *
 * Copyright 2009-2024
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

#pragma once

#include "ct_types.h"
#include "ct_filesystem.h"
#include <sqlite3.h>
#include <glibmm/refptr.h>
#include <gtksourceviewmm/buffer.h>
#include <gtkmm/treeiter.h>
#include <unordered_set>

class CtMainWin;
class CtAnchoredWidget;
class CtTreeIter;
class CtStorageCache;

class CtStorageSqlite : public CtStorageEntity
{
public:
    CtStorageSqlite(CtMainWin* pCtMainWin)
     : _pCtMainWin{pCtMainWin}
    {}
    ~CtStorageSqlite();

    void close_connect() override;
    void reopen_connect() override;
    void test_connection() override;
    void try_reopen() override;

    bool populate_treestore(const fs::path& file_path, Glib::ustring& error) override;
    bool save_treestore(const fs::path& file_path,
                        const CtStorageSyncPending& syncPending,
                        Glib::ustring& error,
                        const CtExporting export_type,
                        const std::map<gint64, gint64>* pExpoMasterReassign = nullptr,
                        const int start_offset = 0,
                        const int end_offset = -1) override;
    void vacuum() override;
    void import_nodes(const fs::path& path, const Gtk::TreeIter& parent_iter) override;

    Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64 node_id,
                                                      const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& widgets) const override;
private:
    void _open_db(const fs::path& path);
    void _close_db();
    bool _check_database_integrity();

    Gtk::TreeIter _node_from_db(const gint64 node_id,
                                const gint64 master_id,
                                const gint64 sequence,
                                Gtk::TreeIter parent_iter,
                                const gint64 new_id);

    /**
     * @brief Check that the database contains the required tables
     *
     * @param db
     */
    void                _fix_db_tables();
    /**
     * @brief Get a list of field names for a table
     * @warning Only hardcoded table names should be passed to this method
     * @param table_name
     * @return std::unordered_set<std::string>
     */
    std::unordered_set<std::string> _get_table_field_names(std::string_view table_name);

    void                _image_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const;
    void                _codebox_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const;
    void                _table_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const;

    void                _create_all_tables_in_db();
    void                _write_bookmarks_to_db(const std::list<gint64>& bookmarks);
    void                _write_node_to_db(const CtTreeIter* ct_tree_iter,
                                          const gint64 sequence,
                                          const gint64 node_father_id,
                                          const CtStorageNodeState& write_dict,
                                          const int start_offset,
                                          const int end_offset,
                                          CtStorageCache* storage_cache,
                                          const CtExporting export_type,
                                          const std::map<gint64, gint64>* pExpoMasterReassign);

    std::list<std::pair<gint64,gint64>> _get_children_node_ids_from_db(const gint64 father_id);
    void                _remove_db_node_with_children(const gint64 node_id);

    void                _exec_no_callback(const char* sqlCmd);
    void                _exec_bind_int64(const char* sqlCmd, const gint64 bind_int64);

public:
    static const char TABLE_NODE_CREATE[];
    static const char TABLE_NODE_INSERT[];
    static const char TABLE_NODE_DELETE[];
    static const char TABLE_CODEBOX_CREATE[];
    static const char TABLE_CODEBOX_INSERT[];
    static const char TABLE_CODEBOX_DELETE[];
    static const char TABLE_TABLE_CREATE[];
    static const char TABLE_TABLE_INSERT[];
    static const char TABLE_TABLE_DELETE[];
    static const char TABLE_IMAGE_CREATE[];
    static const char TABLE_IMAGE_INSERT[];
    static const char TABLE_IMAGE_DELETE[];
    static const char TABLE_CHILDREN_CREATE[];
    static const char TABLE_CHILDREN_INSERT[];
    static const char TABLE_CHILDREN_DELETE[];
    static const char TABLE_BOOKMARK_CREATE[];
    static const char TABLE_BOOKMARK_INSERT[];
    static const char TABLE_BOOKMARK_DELETE[];
    static const std::string ERR_SQLITE_PREPV2;
    static const std::string ERR_SQLITE_STEP;
    static const char* safe_sqlite3_column_text(sqlite3_stmt* stmt, int iCol);

private:
    CtMainWin*    _pCtMainWin;
    sqlite3*      _pDb{nullptr};
    fs::path      _file_path;
};
