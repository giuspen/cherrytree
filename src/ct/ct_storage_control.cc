/*
 * ct_storage_control.cc
 *
 * Copyright 2009-2022
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

#include "ct_dialogs.h"
#include "ct_storage_control.h"
#include "ct_storage_xml.h"
#include "ct_storage_sqlite.h"
#include "ct_p7za_iface.h"
#include "ct_main_win.h"
#include "ct_logging.h"
#include <glib/gstdio.h>

std::unique_ptr<CtStorageEntity> get_entity_by_type(CtMainWin* pCtMainWin, CtDocType file_type)
{
    if (file_type == CtDocType::SQLite) {
        return std::make_unique<CtStorageSqlite>(pCtMainWin);
    }
    return std::make_unique<CtStorageXml>(pCtMainWin);
}

/*static*/CtStorageControl* CtStorageControl::create_dummy_storage(CtMainWin* pCtMainWin)
{
    CtStorageControl* doc = new CtStorageControl{pCtMainWin};
    return doc;
}

/*static*/CtStorageControl* CtStorageControl::load_from(CtMainWin* pCtMainWin,
                                                        const fs::path& file_path,
                                                        Glib::ustring& error,
                                                        Glib::ustring password)
{
    std::unique_ptr<CtStorageEntity> storage;
    fs::path extracted_file_path = file_path;
    try {
        if (!fs::is_regular_file(file_path)) throw std::runtime_error("no file");

        // unpack file if need
        if (fs::get_doc_encrypt(file_path) == CtDocEncrypt::True) {
            extracted_file_path = _extract_file(pCtMainWin, file_path, password);
            if (extracted_file_path.empty()) {
                // user canceled operation
                return nullptr;
            }
        }

        // choose storage type
        storage = get_entity_by_type(pCtMainWin, fs::get_doc_type(file_path));

        // load from file
        if (!storage->populate_treestore(extracted_file_path, error)) throw std::runtime_error(error);

        // it's ready
        CtStorageControl* doc = new CtStorageControl(pCtMainWin);
        doc->_file_path = file_path;
        doc->_mod_time = fs::getmtime(file_path);
        doc->_password = password;
        doc->_extracted_file_path = extracted_file_path;
        doc->_storage.swap(storage);
        return doc;

    }
    catch (std::exception& e) {
        if (extracted_file_path != file_path && fs::is_regular_file(extracted_file_path))
            g_remove(extracted_file_path.c_str());

        spdlog::error(e.what());
        error = e.what();
        return nullptr;
    }
}

/*static*/CtStorageControl* CtStorageControl::save_as(CtMainWin* pCtMainWin,
                                                      const fs::path& file_path,
                                                      const Glib::ustring& password,
                                                      Glib::ustring& error,
                                                      const CtExporting exporting/*= CtExporting::NONE*/,
                                                      const int start_offset/*= 0*/,
                                                      const int end_offset/*= -1*/)
{
    auto on_scope_exit = scope_guard([&](void*) { pCtMainWin->get_status_bar().pop(); });
    pCtMainWin->get_status_bar().push(_("Writing to Disk..."));
    while (gtk_events_pending()) gtk_main_iteration();

    std::unique_ptr<CtStorageEntity> storage;
    fs::path extracted_file_path = file_path;
    try {
        if (fs::get_doc_encrypt(file_path) == CtDocEncrypt::True) {
            extracted_file_path = pCtMainWin->get_ct_tmp()->getHiddenFilePath(file_path);
        }
        if (fs::is_regular_file(file_path)) {
            fs::remove(file_path);
        }
        if (fs::is_regular_file(extracted_file_path)) {
            fs::remove(extracted_file_path);
        }

        storage = get_entity_by_type(pCtMainWin, fs::get_doc_type(file_path));
        // will save all data because it's the first time
        CtStorageSyncPending fakePending;
        if (not storage->save_treestore(extracted_file_path, fakePending, error, exporting, start_offset, end_offset)) {
            throw std::runtime_error(error);
        }
        // encrypt the file
        if (file_path != extracted_file_path)
        {
            storage->close_connect(); // temporary, because of sqlite keeping the file
            if (!_package_file(extracted_file_path, file_path, password))
                throw std::runtime_error("couldn't encrypt the file");
            storage->reopen_connect();
        }

        // it's ready
        CtStorageControl* doc = new CtStorageControl(pCtMainWin);
        doc->_file_path = file_path;
        doc->_mod_time = fs::getmtime(file_path);
        doc->_password = password;
        doc->_extracted_file_path = extracted_file_path;
        doc->_storage.swap(storage);
        return doc;
    }
    catch (std::exception& e) {
        if (fs::is_regular_file(file_path)) fs::remove(file_path);
        if (fs::is_regular_file(extracted_file_path)) fs::remove(extracted_file_path);

        spdlog::error(e.what());
        error = e.what();
        return nullptr;
    }
}

bool CtStorageControl::save(bool need_vacuum, Glib::ustring &error)
{
    _mod_time = 0;
    auto on_scope_exit = scope_guard([&](void*) {
        _pCtMainWin->get_status_bar().pop();
        _mod_time = fs::getmtime(_file_path);
    });
    _pCtMainWin->get_status_bar().push(_("Writing to Disk..."));
    while (gtk_events_pending()) gtk_main_iteration();

    // backup system
    // before writing make a main backup as file.ext!
    // then write changes (and encrypt) into the original. If it's OK, then put the main backup to backup rotate
    // if it's not, copy file.ext! back;

    const std::string str_timestamp = std::to_string(g_get_monotonic_time());
    fs::path main_backup = _file_path;
    main_backup += str_timestamp;
    const bool need_backup = _pCtConfig->backupCopy and _pCtConfig->backupNum > 0;
    const bool need_encrypt = _file_path != _extracted_file_path;
    try {
        if (_file_path.empty()) {
            throw std::runtime_error("storage is not initialized");
        }

        // sqlite could lose connection
        _storage->test_connection();

        if (need_backup) {
            if (fs::get_doc_type(_file_path) == CtDocType::SQLite and not need_encrypt) {
                _storage->close_connect();    // temporary, because of sqlite keepig the file
                if (not fs::copy_file(_file_path, main_backup)) {
                    throw std::runtime_error(str::format(_("You Have No Write Access to %s"), _file_path.parent_path().string()));
                }
                _storage->reopen_connect();
            }
            else {
                if (not fs::move_file(_file_path, main_backup)) {
                    throw std::runtime_error(str::format(_("You Have No Write Access to %s"), _file_path.parent_path().string()));
                }
            }
        }
        // save changes
        if (not _storage->save_treestore(_extracted_file_path, _syncPending, error)) {
            throw std::runtime_error(error);
        }
        if (need_vacuum) {
            _storage->vacuum();
        }
        if (need_backup or need_encrypt) {
            std::shared_ptr<CtBackupEncryptData> pBackupEncryptData = std::make_shared<CtBackupEncryptData>();
            pBackupEncryptData->needBackup = need_backup;
            pBackupEncryptData->needEncrypt = need_encrypt;
            pBackupEncryptData->main_backup = main_backup.string();
            if (need_encrypt) {
                pBackupEncryptData->extracted_copy = _extracted_file_path.string() + str_timestamp;
                _storage->close_connect();    // temporary, because of sqlite keepig the file
                if (not fs::copy_file(_extracted_file_path, pBackupEncryptData->extracted_copy)) {
                    throw std::runtime_error(str::format(_("You Have No Write Access to %s"), _extracted_file_path.parent_path().string()));
                }
                _storage->reopen_connect();
                pBackupEncryptData->file_path = _file_path.string();
                pBackupEncryptData->password = _password;
            }
            _backupEncryptDEQueue.push_back(pBackupEncryptData);
            if (not _pThreadBackupEncrypt) {
                _pThreadBackupEncrypt = std::make_unique<std::thread>(CtStorageControl::_staticBackupEncryptThread, this);
            }
        }
        _syncPending.fix_db_tables = false;
        _syncPending.bookmarks_to_write = false;
        _syncPending.nodes_to_rm_set.clear();
        _syncPending.nodes_to_write_dict.clear();

        return true;
    }
    catch (std::exception& e) {
        // recover from backup
        try {
            _storage->close_connect();
            if (need_backup && fs::is_regular_file(main_backup)) fs::move_file(main_backup, _file_path);
            _storage->reopen_connect();
        }
        catch (std::exception& e2) { spdlog::error(e2.what()); }

        spdlog::error(e.what());
        error = e.what();
        return false;
    }
}

Glib::RefPtr<Gsv::Buffer> CtStorageControl::get_delayed_text_buffer(const gint64& node_id,
                                                                    const std::string& syntax,
                                                                    std::list<CtAnchoredWidget*>& widgets) const
{
    if (!_storage) {
        spdlog::error("!! storage is not initialized");
        return Glib::RefPtr<Gsv::Buffer>();
    }
    return _storage->get_delayed_text_buffer(node_id, syntax, widgets);
}

/*static*/ fs::path CtStorageControl::_extract_file(CtMainWin* pCtMainWin, const fs::path& file_path, Glib::ustring& password)
{
    fs::path temp_dir = pCtMainWin->get_ct_tmp()->getHiddenDirPath(file_path);
    fs::path temp_file_path = pCtMainWin->get_ct_tmp()->getHiddenFilePath(file_path);
    Glib::ustring title = str::format(_("Enter Password for %s"), file_path.filename().string());
    while (true) {
        if (password.empty()) {
            CtDialogTextEntry dialogTextEntry(title, true/*forPassword*/, pCtMainWin);
            auto on_scope_exit = scope_guard([pCtMainWin](void*) {
                pCtMainWin->set_systray_can_hide(true);
            });
            pCtMainWin->set_systray_can_hide(false);
            if (Gtk::RESPONSE_OK != dialogTextEntry.run()) {
                // no password, user cancels operation, return empty path
                return fs::path{};
            }
            password = dialogTextEntry.get_entry_text();
        }
        if (0 == CtP7zaIface::p7za_extract(file_path.c_str(), temp_dir.c_str(), password.c_str(), false)) {
            if (fs::is_regular_file(temp_file_path)) {
                return temp_file_path;
            }
            const std::list<fs::path> filesInTmpDir = fs::get_dir_entries(temp_file_path.parent_path());
            if (filesInTmpDir.size() == 1 and
                fs::get_doc_type(filesInTmpDir.front()) == fs::get_doc_type(temp_file_path) and
                fs::move_file(filesInTmpDir.front(), temp_file_path))
            {
                spdlog::debug("encrypt doc renamed {} -> {}", filesInTmpDir.front().filename(), temp_file_path.filename());
                return temp_file_path;
            }
        }
        else {
            spdlog::debug("!! CtP7zaIface::p7za_extract");
        }
        password.clear();
    }
}

/*static*/bool CtStorageControl::_package_file(const fs::path& file_from, const fs::path& file_to, const Glib::ustring& password)
{
    if (0 != CtP7zaIface::p7za_archive(file_from.c_str(), file_to.c_str(), password.c_str())) {
        spdlog::debug("!! p7za_archive {} -> {}", file_from.c_str(), file_to.c_str());
        return false;
    }
    if (not fs::is_regular_file(file_to)) {
        spdlog::debug("!! is_regular_file {}", file_to);
        return false;
    }
    return true;
}

CtStorageControl::CtStorageControl(CtMainWin* pCtMainWin)
 : _pCtMainWin{pCtMainWin}
 , _pCtConfig{pCtMainWin->get_ct_config()}
{
}

CtStorageControl::~CtStorageControl()
{
    if (_pThreadBackupEncrypt) {
        _backupEncryptKeepGoing = false;
        _backupEncryptDEQueue.push_back(nullptr);
        _pThreadBackupEncrypt->join();
    }
}

void CtStorageControl::_backupEncryptThread()
{
    while (_backupEncryptKeepGoing) {
        std::shared_ptr<CtBackupEncryptData> pBackupEncryptData = _backupEncryptDEQueue.pop_front();
        if (not pBackupEncryptData) {
            break;
        }

        // encrypt the file
        if (pBackupEncryptData->needEncrypt) {
            const bool retValEncrypt = _package_file(pBackupEncryptData->extracted_copy, pBackupEncryptData->file_path, pBackupEncryptData->password);
            if (not fs::remove(pBackupEncryptData->extracted_copy)) {
                spdlog::debug("Failed to remove {}", pBackupEncryptData->extracted_copy);
            }
            if (not retValEncrypt) {
                _pCtMainWin->errorsDEQueue.push_back(_("Failed to encrypt the file"));
                _pCtMainWin->dispatcherErrorMsg.emit();
                return;
            }
        }

        if (not pBackupEncryptData->needBackup) {
            return;
        }

        // backups with tildas can be either in the same directory where the db places or in a custom backup dir
        // main_backup is always with the main db
        auto get_custom_backup_file = [&]() -> std::string {
            // backup path in custom dir: /custom_dir/full_file_path/filename.ext~
            std::string hash_dir = pBackupEncryptData->file_path;
            for (auto str : {"\\", "/", ":", "?"}) {
                hash_dir = str::replace(hash_dir, str, "_");
            }
            std::string new_backup_dir = Glib::build_filename(_pCtConfig->customBackupDir, hash_dir);
            Glib::RefPtr<Gio::File> dir_file = Gio::File::create_for_path(new_backup_dir);
            try {
                if (not dir_file->query_exists() and not dir_file->make_directory_with_parents()) {
                    spdlog::error("failed to create backup directory: {}", new_backup_dir);
                    return "";
                }
                return Glib::build_filename(new_backup_dir, Glib::path_get_basename(pBackupEncryptData->file_path)) + CtConst::CHAR_TILDE;
            }
            catch (Glib::Error& ex) {
                spdlog::error("failed to create backup directory: {}, \n{}", new_backup_dir, ex.what());
                return "";
            }
        };

        std::string new_backup_file = pBackupEncryptData->file_path + CtConst::CHAR_TILDE;
        if (_pCtConfig->customBackupDirOn and not _pCtConfig->customBackupDir.empty()) {
            std::string custom_backup_file = get_custom_backup_file();
            if (not custom_backup_file.empty()) {
                new_backup_file = custom_backup_file;
            }
        }

        // shift backups with tilda
        if (_pCtConfig->backupNum >= 2) {
            fs::path tilda_filepath = new_backup_file + std::string(_pCtConfig->backupNum - 2, '~');
            while (str::endswith(tilda_filepath.string(), CtConst::CHAR_TILDE)) {
                if (fs::is_regular_file(tilda_filepath)) {
                    if (not fs::move_file(tilda_filepath, tilda_filepath.string() + CtConst::CHAR_TILDE)) {
                        _pCtMainWin->errorsDEQueue.push_back(str::format(_("You Have No Write Access to %s"), fs::path{new_backup_file}.parent_path().string()));
                        _pCtMainWin->dispatcherErrorMsg.emit();
                        return;
                    }
                }
                tilda_filepath = tilda_filepath.string().substr(0, tilda_filepath.string().size()-1);
            }
        }

        if (not fs::move_file(pBackupEncryptData->main_backup, new_backup_file)) {
            _pCtMainWin->errorsDEQueue.push_back(str::format(_("You Have No Write Access to %s"), fs::path{new_backup_file}.parent_path().string()));
            _pCtMainWin->dispatcherErrorMsg.emit();
        }
    }
}

void CtStorageControl::pending_edit_db_node_prop(gint64 node_id)
{
    if (0 != _syncPending.nodes_to_write_dict.count(node_id))
    {
        _syncPending.nodes_to_write_dict[node_id].prop = true;
    }
    else
    {
        CtStorageNodeState node_state;
        node_state.upd = true;
        node_state.prop = true;
        _syncPending.nodes_to_write_dict[node_id] = node_state;
    }
}

void CtStorageControl::pending_edit_db_node_buff(gint64 node_id)
{
    if (0 != _syncPending.nodes_to_write_dict.count(node_id))
    {
        _syncPending.nodes_to_write_dict[node_id].buff = true;
    }
    else
    {
        CtStorageNodeState node_state;
        node_state.upd = true;
        node_state.buff = true;
        _syncPending.nodes_to_write_dict[node_id] = node_state;
    }
}

void CtStorageControl::pending_edit_db_node_hier(gint64 node_id)
{
    if (0 != _syncPending.nodes_to_write_dict.count(node_id))
    {
        _syncPending.nodes_to_write_dict[node_id].hier = true;
    }
    else
    {
        CtStorageNodeState node_state;
        node_state.upd = true;
        node_state.hier = true;
        _syncPending.nodes_to_write_dict[node_id] = node_state;
    }
}

void CtStorageControl::pending_new_db_node(gint64 node_id)
{
    CtStorageNodeState node_state;
    node_state.upd = false;
    node_state.prop = true;
    node_state.buff = true;
    node_state.hier = true;
    _syncPending.nodes_to_write_dict[node_id] = node_state;
}

void CtStorageControl::pending_rm_db_nodes(const std::vector<gint64>& node_ids)
{
    for (const gint64 node_id : node_ids)
    {
        if (0 != _syncPending.nodes_to_write_dict.count(node_id))
        {
            // no need to write changes to a node that got to be removed
            _syncPending.nodes_to_write_dict.erase(node_id);
        }
        _syncPending.nodes_to_rm_set.insert(node_id);
    }
}

void CtStorageControl::pending_edit_db_bookmarks()
{
    _syncPending.bookmarks_to_write = true;
}

void CtStorageControl::add_nodes_from_storage(const fs::path& path, Gtk::TreeIter parent_iter)
{
    if (!fs::is_regular_file(path)) {
        throw std::runtime_error(fmt::format("File: {} - is not a regular file", path));
    }

    Glib::ustring password;
    fs::path extracted_file_path = path;
    if (fs::get_doc_encrypt(path) == CtDocEncrypt::True) {
        extracted_file_path = _extract_file(_pCtMainWin, path, password);
        if (extracted_file_path.empty()) {
            // user canceled operation
            return;
        }
    }

    std::unique_ptr<CtStorageEntity> storage = get_entity_by_type(_pCtMainWin, fs::get_doc_type(extracted_file_path));
    storage->import_nodes(extracted_file_path, parent_iter);

    _pCtMainWin->get_tree_store().nodes_sequences_fix(parent_iter, false);
    _pCtMainWin->update_window_save_needed();
}

void CtStorageCache::generate_cache(CtMainWin* pCtMainWin, const CtStorageSyncPending* pending, bool xml)
{
    std::vector<CtImagePng*> image_list;

    auto& store = pCtMainWin->get_tree_store();
    if (pending == nullptr) // all nodes
    {
        store.get_store()->foreach([&](const Gtk::TreePath&, const Gtk::TreeIter& iter)->bool
        {
            for (auto widget: store.to_ct_tree_iter(iter).get_anchored_widgets_fast())
                if (widget->get_type() == CtAnchWidgType::ImagePng) // important to check type
                    if (auto image = dynamic_cast<CtImagePng*>(widget))
                        image_list.emplace_back(image);
            return false; /* false for continue */
        });
    }
    else
    {
        for (const auto& node_pair : pending->nodes_to_write_dict)
        {
            CtTreeIter ct_tree_iter = store.get_node_from_node_id(node_pair.first);
            if (node_pair.second.buff && ct_tree_iter.get_node_is_rich_text())
            {
                for (auto widget: ct_tree_iter.get_anchored_widgets_fast())
                    if (widget->get_type() == CtAnchWidgType::ImagePng) // important to check type
                        if (auto image = dynamic_cast<CtImagePng*>(widget))
                            image_list.emplace_back(image);
            }
        }
    }

    parallel_fetch_pixbufers(image_list, xml);
}

void CtStorageCache::parallel_fetch_pixbufers(const std::vector<CtImagePng*>& image_widgets, bool xml)
{
    _cached_images.clear();

    // auto start = std::chrono::steady_clock::now();

    std::vector<std::pair<CtImagePng*, std::string>> image_pair(image_widgets.size());
    for (size_t i = 0; i < image_widgets.size(); ++i)
        image_pair[i].first = image_widgets[i];

    // replacement for tbb::parallel_for
    CtMiscUtil::parallel_for(0, image_pair.size(), [&](size_t index) {
        auto& pair = image_pair[index];
        pair.second = pair.first->get_raw_blob();
        if (xml) pair.second = Glib::Base64::encode(pair.second);
    });

    for (auto& pair: image_pair)
        _cached_images.emplace(pair);

    //auto end = std::chrono::steady_clock::now();
    //std::chrono::duration<double> elapsed_seconds = end-start;
    //spdlog::debug("parallel_fetch_pixbufers amount: , {} sec.", elapsed_seconds.count());
}

bool CtStorageCache::get_cached_image(CtImagePng* image, std::string& cached_image)
{
    auto it = _cached_images.find(image);
    if (it == _cached_images.end()) return false;
    cached_image = it->second;
    return true;
}
