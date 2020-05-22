/*
 * ct_storage_control.cc
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

#include "ct_dialogs.h"
#include "ct_storage_control.h"
#include "ct_storage_xml.h"
#include "ct_storage_sqlite.h"
#include "ct_p7za_iface.h"
#include "ct_main_win.h"
#include <glib/gstdio.h>


static CtStorageEntity* get_entity_by_type(CtMainWin* pCtMainWin, CtDocType file_type)
{
    if (file_type == CtDocType::SQLite)
        return new CtStorageSqlite(pCtMainWin);
    else
        return new CtStorageXml(pCtMainWin);
}

/*static*/ CtStorageControl* CtStorageControl::create_dummy_storage()
{
    return new CtStorageControl();
}

/*static*/ CtStorageControl* CtStorageControl::load_from(CtMainWin* pCtMainWin, const Glib::ustring &file_path, Glib::ustring& error)
{
    CtStorageEntity* storage{nullptr};
    Glib::ustring extracted_file_path = file_path;
    try
    {
        if (!Glib::file_test(file_path, Glib::FILE_TEST_IS_REGULAR))
            throw std::runtime_error("no file");

        // unpack file if need
        Glib::ustring password;
        if (CtMiscUtil::get_doc_encrypt(file_path) == CtDocEncrypt::True)
            extracted_file_path = _extract_file(pCtMainWin, file_path, password);

        // choose storage type
        storage = get_entity_by_type(pCtMainWin, CtMiscUtil::get_doc_type(file_path));

        // load from file
        if (!storage->populate_treestore(extracted_file_path, error))
            throw std::runtime_error(error);

        // it's ready
        CtStorageControl* doc = new CtStorageControl();
        doc->_pCtMainWin = pCtMainWin;
        doc->_file_path = file_path;
        doc->_password = password;
        doc->_extracted_file_path = extracted_file_path;
        doc->_need_backup = true;
        doc->_storage = storage;
        return doc;

    }
    catch (std::exception& e)
    {
        delete storage;
        if (extracted_file_path != file_path && Glib::file_test(extracted_file_path, Glib::FILE_TEST_IS_REGULAR))
            g_remove(extracted_file_path.c_str());

        std::cerr << e.what() << std::endl;
        error = e.what();
        return nullptr;
    }
}

/*static*/ CtStorageControl* CtStorageControl::save_as(CtMainWin* pCtMainWin, const Glib::ustring& file_path, const Glib::ustring& password, Glib::ustring& error)
{
    CtStorageEntity* storage{nullptr};
    Glib::ustring extracted_file_path = file_path;
    try
    {
        if (CtMiscUtil::get_doc_encrypt(file_path) == CtDocEncrypt::True)
            extracted_file_path = pCtMainWin->get_ct_tmp()->getHiddenFilePath(file_path);
        if (Glib::file_test(file_path, Glib::FILE_TEST_IS_REGULAR))
            g_remove(file_path.c_str());
        if (Glib::file_test(extracted_file_path, Glib::FILE_TEST_IS_REGULAR))
            g_remove(extracted_file_path.c_str());

        storage = get_entity_by_type(pCtMainWin, CtMiscUtil::get_doc_type(file_path));
        // will save all data because it's the first time
        CtStorageSyncPending fakePanding;
        if (!storage->save_treestore(extracted_file_path, fakePanding, error))
            throw std::runtime_error(error);

        // encrypt the file
        if (file_path != extracted_file_path)
        {
            storage->close_connect(); // temporary, because of sqlite keepig the file
            if (!_package_file(extracted_file_path, file_path, password))
                throw std::runtime_error("couldn't encrypt the file");
            storage->reopen_connect();
        }

        // it's ready
        CtStorageControl* doc = new CtStorageControl();
        doc->_pCtMainWin = pCtMainWin;
        doc->_file_path = file_path;
        doc->_password = password;
        doc->_extracted_file_path = extracted_file_path;
        doc->_need_backup = false; // no need for backup
        doc->_storage = storage;
        return doc;
    }
    catch (std::exception& e)
    {
        delete storage;
        if (Glib::file_test(file_path, Glib::FILE_TEST_IS_REGULAR))
            g_remove(file_path.c_str());
        if (Glib::file_test(extracted_file_path, Glib::FILE_TEST_IS_REGULAR))
            g_remove(extracted_file_path.c_str());

        std::cerr << e.what() << std::endl;
        error = e.what();
        return nullptr;
    }
}

bool CtStorageControl::save(bool need_vacuum, Glib::ustring &error)
{
    // backup system
    // before writing make a main backup as file.ext!
    // then write changes (and encrypt) into the original. If it's OK, then put the main backup to backup rotate
    // if it's not, copy file.ext! back;

    Glib::ustring main_backup = _file_path + "!";
    bool need_backup = _need_backup && _pCtMainWin->get_ct_config()->backupCopy and _pCtMainWin->get_ct_config()->backupNum > 0;
    try
    {
        if (_file_path == "")
            throw std::runtime_error("storage is not initialized");

        // sqlite could lost connection
        _storage->test_connection();

        if (need_backup)
        {
            // move is faster but the file is used by sqlite without encrypt
            if (_file_path == _extracted_file_path && CtMiscUtil::get_doc_type(_file_path) == CtDocType::SQLite)
            {
                _storage->close_connect();    // temporary, because of sqlite keepig the file
                if (!CtFileSystem::copy_file(_file_path, main_backup))
                    throw std::runtime_error(str::format(_("You Have No Write Access to %s"), Glib::path_get_dirname(_file_path)));
                _storage->reopen_connect();
            }
            else
            {
                if (!CtFileSystem::move_file(_file_path, main_backup))
                    throw std::runtime_error(str::format(_("You Have No Write Access to %s"), Glib::path_get_dirname(_file_path)));
            }
        }
        // save changes
        if (!_storage->save_treestore(_extracted_file_path, _syncPending, error))
            throw std::runtime_error(error);
        if (need_vacuum)
            _storage->vacuum();

        // encrypt the file
        if (_file_path != _extracted_file_path)
        {
            _storage->close_connect(); // temporary, because of sqlite keepig the file
            if (!_package_file(_extracted_file_path, _file_path, _password))
                throw std::runtime_error("couldn't encrypt the file");
            _storage->reopen_connect();
        }
        if (need_backup)
            _put_in_backup(main_backup);

        _syncPending.bookmarks_to_write = false;
        _syncPending.nodes_to_rm_set.clear();
        _syncPending.nodes_to_write_dict.clear();
        _need_backup = false;

        return true;
    }
    catch (std::exception& e)
    {
        // recover from backup
        try {
            _storage->close_connect();
            if (need_backup && Glib::file_test(main_backup, Glib::FILE_TEST_IS_REGULAR))
                CtFileSystem::move_file(main_backup, _file_path);
            _storage->reopen_connect();
        }
        catch (std::exception& e2) { std::cerr << e2.what() << std::endl; }

        std::cerr << e.what() << std::endl;
        error = e.what();
        return false;
    }
}

Glib::RefPtr<Gsv::Buffer> CtStorageControl::get_delayed_text_buffer(const gint64& node_id,
                                                                    const std::string& syntax,
                                                                    std::list<CtAnchoredWidget*>& widgets) const
{
    if (!_storage) {
        std::cerr << "!! storage is not initialized" << std::endl;
        return Glib::RefPtr<Gsv::Buffer>();
    }
    return _storage->get_delayed_text_buffer(node_id, syntax, widgets);
}

/*static*/ Glib::ustring CtStorageControl::_extract_file(CtMainWin* pCtMainWin, const Glib::ustring& file_path, Glib::ustring& password)
{
    Glib::ustring temp_dir = pCtMainWin->get_ct_tmp()->getHiddenDirPath(file_path);
    Glib::ustring temp_file_path = pCtMainWin->get_ct_tmp()->getHiddenFilePath(file_path);
    Glib::ustring title = str::format(_("Enter Password for %s"), Glib::path_get_basename(file_path));
    while (true)
    {
        CtDialogTextEntry dialogTextEntry(title, true/*forPassword*/, pCtMainWin);
        if (Gtk::RESPONSE_OK != dialogTextEntry.run())
            throw std::runtime_error("no password, user cancels operation");

        password = dialogTextEntry.get_entry_text();
        if (0 == CtP7zaIface::p7za_extract(file_path.c_str(), temp_dir.c_str(), password.c_str()))
            if (g_file_test(temp_file_path.c_str(), G_FILE_TEST_IS_REGULAR))
                return temp_file_path;
    }
}

/*static*/ bool CtStorageControl::_package_file(const Glib::ustring& file_from, const Glib::ustring& file_to, const Glib::ustring& password)
{
    return 0 == CtP7zaIface::p7za_archive(file_from.c_str(), file_to.c_str(), password.c_str())
            && Glib::file_test(file_to, Glib::FILE_TEST_IS_REGULAR);
}

void CtStorageControl::_put_in_backup(const Glib::ustring& main_backup)
{
    Glib::ustring tilda_filepath = _file_path + str::repeat(CtConst::CHAR_TILDE, _pCtMainWin->get_ct_config()->backupNum - 1);
    while (str::endswith(tilda_filepath, CtConst::CHAR_TILDE))
    {
        if (Glib::file_test(tilda_filepath, Glib::FILE_TEST_IS_REGULAR))
            if (!CtFileSystem::move_file(tilda_filepath, tilda_filepath + CtConst::CHAR_TILDE))
                throw std::runtime_error(str::format(_("You Have No Write Access to %s"), Glib::path_get_dirname(_file_path)));
        tilda_filepath = tilda_filepath.substr(0, tilda_filepath.size()-1);
    }
    if (!CtFileSystem::move_file(main_backup, _file_path + CtConst::CHAR_TILDE))
        throw std::runtime_error(str::format(_("You Have No Write Access to %s"), Glib::path_get_dirname(_file_path)));
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



void CtStorageControl::add_nodes_from_storage(const Glib::ustring& path) {
    if (!Glib::file_test(path, Glib::FILE_TEST_IS_REGULAR))
            throw std::runtime_error("no file");

        // unpack file if need
        Glib::ustring password;
        Glib::ustring extracted_file_path = path;
        if (CtMiscUtil::get_doc_encrypt(path) == CtDocEncrypt::True)
            extracted_file_path = _extract_file(_pCtMainWin, path, password);

        // load from file
        auto storage = get_entity_by_type(_pCtMainWin, CtMiscUtil::get_doc_type(extracted_file_path));
        storage->import_nodes(_pCtMainWin, extracted_file_path);
}

