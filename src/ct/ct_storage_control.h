/*
 * ct_storage_control.h
 *
 * Copyright 2009-2023
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
#include <glibmm/miscutils.h>
#include <thread>

class CtMainWin;
class CtStorageControl
{
public:
    static CtStorageControl* create_dummy_storage(CtMainWin* pCtMainWin);
    static CtStorageControl* load_from(CtMainWin* pCtMainWin,
                                       const fs::path& file_path,
                                       Glib::ustring& error,
                                       Glib::ustring password = "");
    static CtStorageControl* save_as(CtMainWin* pCtMainWin,
                                     const fs::path& file_path,
                                     const Glib::ustring& password,
                                     Glib::ustring& error,
                                     const CtExporting exporting = CtExporting::NONE,
                                     const int start_offset = 0,
                                     const int end_offset = -1);
    static bool document_integrity_check_pass(CtMainWin* pCtMainWin,
                                              const fs::path& file_path,
                                              Glib::ustring& error);
    virtual ~CtStorageControl();

public:
    bool save(bool need_vacuum, Glib::ustring& error);

private:
    CtStorageControl(CtMainWin* pCtMainWin);

    static fs::path _extract_file(CtMainWin* pCtMainWin, const fs::path& file_path, Glib::ustring& password);
    static bool     _package_file(const fs::path& file_from, const fs::path& file_to, const Glib::ustring& password);

public:
    Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                      const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& widgets) const;

    const fs::path& get_file_path() { return _file_path; }
    time_t get_mod_time() { return _mod_time; }
    fs::path get_file_name() { return _file_path.empty() ? "" : _file_path.filename(); }
    fs::path get_file_dir()  { return _file_path.empty() ? "" : _file_path.parent_path(); }

    std::set<gint64> get_nodes_pending_rm() { return _syncPending.nodes_to_rm_set; }

    void pending_edit_db_node_prop(gint64 node_id);
    void pending_edit_db_node_buff(gint64 node_id);
    void pending_edit_db_node_hier(gint64 node_id);
    void pending_new_db_node(gint64 node_id);
    void pending_rm_db_nodes(const std::vector<gint64>& node_ids);
    void pending_edit_db_bookmarks();

    /**
     * @brief Add the nodes from an external CT file to the current tree
     * Operates on all CT files, uses the appropraite StorageEntity derivative and extracts
     * encrypted files
     * @param path: The path to the external CT file
     */
    void add_nodes_from_storage(const fs::path& path, Gtk::TreeIter parent_iter);

private:
    CtMainWin*                 const _pCtMainWin;
    CtConfig*                  const _pCtConfig;
    fs::path                         _file_path;
    time_t                           _mod_time{0};
    Glib::ustring                    _password;
    fs::path                         _extracted_file_path;
    std::unique_ptr<CtStorageEntity> _storage;
    CtStorageSyncPending             _syncPending;

private:
    struct CtBackupEncryptData {
        bool needBackup;
        bool needEncrypt;
        std::string main_backup;
        std::string file_path;
        std::string password;
        std::string extracted_copy;
    };
    std::unique_ptr<std::thread> _pThreadBackupEncrypt;
    ThreadSafeDEQueue<std::shared_ptr<CtBackupEncryptData>,9> _backupEncryptDEQueue;
    void _backupEncryptThread();
    bool _backupEncryptKeepGoing{true};
};

class CtImagePng;
class CtStorageCache
{
public:
    void generate_cache(CtMainWin* pCtMainWin, const CtStorageSyncPending* pending, bool xml);

    void parallel_fetch_pixbufers(const std::vector<CtImagePng*>& image_widgets, bool xml);
    bool get_cached_image(CtImagePng* image, std::string& cached_image);

private:
    std::map<CtImagePng*, std::string> _cached_images;
};
