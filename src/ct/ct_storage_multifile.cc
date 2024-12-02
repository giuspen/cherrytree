/*
 * ct_storage_multifile.cc
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

#include "ct_storage_multifile.h"
#include "ct_storage_xml.h"
#include "ct_storage_control.h"
#include "ct_main_win.h"
#include "ct_logging.h"
#include <glib/gstdio.h>

/*static*/const std::string CtStorageMultiFile::SUBNODES_LST{"subnodes.lst"};
/*static*/const std::string CtStorageMultiFile::BOOKMARKS_LST{"bookmarks.lst"};
/*static*/const std::string CtStorageMultiFile::NODE_XML{"node.xml"};
/*static*/const std::string CtStorageMultiFile::BEFORE_SAVE{".before"};

bool CtStorageMultiFile::save_treestore(const fs::path& dir_path,
                                        const CtStorageSyncPending& syncPending,
                                        Glib::ustring& error,
                                        const CtExporting export_type,
                                        const std::map<gint64, gint64>* pExpoMasterReassign/*= nullptr*/,
                                        const int start_offset/*= 0*/,
                                        const int end_offset/*= -1*/)
{
    try {
        CtTreeStore& ct_tree_store = _pCtMainWin->get_tree_store();
        if (_dir_path.empty()) {
            // it's the first time (or an export), a new folder will be created
            if (g_mkdir(dir_path.c_str(), 0755) < 0) {
                error = Glib::ustring{"failed to create "} + dir_path.string();
                return false;
            }
            _dir_path = dir_path;

            if ( CtExporting::NONESAVEAS == export_type or
                 CtExporting::ALL_TREE == export_type )
            {
                // save bookmarks
                _write_bookmarks_to_disk(ct_tree_store.bookmarks_get());
            }
            CtStorageNodeState node_state;
            node_state.is_update_of_existing = false; // no need to delete the prev data
            node_state.prop = true;
            node_state.buff = true;
            node_state.hier = true;

            CtStorageCache storage_cache;
            storage_cache.generate_cache(_pCtMainWin, nullptr/*all nodes*/, false/*for_xml*/);

            std::list<gint64> subnodes_list;

            // save nodes
            if ( CtExporting::NONESAVEAS == export_type or
                 CtExporting::ALL_TREE == export_type )
            {
                CtTreeIter ct_tree_iter = ct_tree_store.get_ct_iter_first();
                while (ct_tree_iter) {
                    const gint64 node_id = ct_tree_iter.get_node_id();
                    subnodes_list.push_back(node_id);
                    if (not _nodes_to_multifile(&ct_tree_iter,
                                                dir_path / std::to_string(node_id),
                                                error,
                                                &storage_cache,
                                                node_state,
                                                export_type,
                                                pExpoMasterReassign,
                                                start_offset,
                                                end_offset))
                    {
                        return false;
                    }
                    ++ct_tree_iter;
                }
            }
            else {
                CtTreeIter ct_tree_iter = _pCtMainWin->curr_tree_iter();
                const gint64 node_id = ct_tree_iter.get_node_id();
                subnodes_list.push_back(node_id);
                if (not _nodes_to_multifile(&ct_tree_iter,
                                            dir_path / std::to_string(node_id),
                                            error,
                                            &storage_cache,
                                            node_state,
                                            export_type,
                                            pExpoMasterReassign,
                                            start_offset,
                                            end_offset))
                {
                    return false;
                }
            }

            // save subnodes
            Glib::file_set_contents(Glib::build_filename(dir_path.string(), SUBNODES_LST),
                                    str::join_numbers(subnodes_list, ","));
        }
        else {
            // or need just update some info
            CtStorageCache storage_cache;
            storage_cache.generate_cache(_pCtMainWin, &syncPending, false/*for_xml*/);

            // update bookmarks
            if (syncPending.bookmarks_to_write) {
                _write_bookmarks_to_disk(ct_tree_store.bookmarks_get());
            }
            // update changed nodes
            const std::list<std::pair<CtTreeIter, CtStorageNodeState>> nodes_to_write = CtStorageControl::get_sorted_by_level_nodes_to_write(
                &_pCtMainWin->get_tree_store(), syncPending.nodes_to_write_dict);
            bool any_hier{false};
            for (const auto& node_pair : nodes_to_write) {
                if (not _nodes_to_multifile(&node_pair.first,
                                            _get_node_dirpath(node_pair.first),
                                            error,
                                            &storage_cache,
                                            node_pair.second,
                                            export_type,
                                            pExpoMasterReassign,
                                            0,
                                            -1))
                {
                    return false;
                }
                if (not any_hier and node_pair.second.hier) {
                    any_hier = true;
                }
            }
            if (not syncPending.nodes_to_rm_set.empty()) {
                // remove nodes and their sub nodes
                _already_queued_for_removal.clear();
                for (const gint64 node_id : syncPending.nodes_to_rm_set) {
                    _remove_disk_node_with_children(node_id);
                }
                if (not any_hier) {
                    any_hier = true;
                }
            }
            if (any_hier) {
                // full tree hierarchy verification needed
                _verify_update_hierarchy(nullptr/*ct_tree_iter_parent*/, _dir_path);
            }
        }
        return true;
    }
    catch (std::exception& e) {
        error = e.what();
        return false;
    }
}

void CtStorageMultiFile::_verify_update_hierarchy(const CtTreeIter* ct_tree_iter_parent, const fs::path& dir_path)
{
    std::list<gint64> subnodes_list;

    CtTreeIter ct_tree_iter = ct_tree_iter_parent ? ct_tree_iter_parent->first_child() : _pCtMainWin->get_tree_store().get_ct_iter_first();

    while (ct_tree_iter) {
        const gint64 node_id = ct_tree_iter.get_node_id();
        subnodes_list.push_back(node_id);

        _verify_update_hierarchy(&ct_tree_iter, dir_path / std::to_string(node_id));

        ++ct_tree_iter;
    }

    const fs::path path_subnodes_lst = dir_path / SUBNODES_LST;
    const std::string new_subnodes_lst = str::join_numbers(subnodes_list, ",");
    const std::string old_subnodes_lst = fs::is_regular_file(path_subnodes_lst) ? Glib::file_get_contents(path_subnodes_lst.string()) : "";
    if (new_subnodes_lst != old_subnodes_lst) {
        if (new_subnodes_lst.empty()) {
            fs::remove(path_subnodes_lst);
        }
        else {
            Glib::file_set_contents(path_subnodes_lst.string(), new_subnodes_lst);
        }
        spdlog::debug("'{}'->'{}'", old_subnodes_lst, new_subnodes_lst);
    }
}

bool CtStorageMultiFile::_found_node_dirpath(const fs::path& node_id, const fs::path parent_path, fs::path& hierarchical_path) const
{
    const std::list<fs::path> dir_entries = fs::get_dir_entries(parent_path);
    for (const fs::path& curr_path : dir_entries) {
        if (fs::is_directory(curr_path)) {
            if (curr_path.filename() == node_id) {
                hierarchical_path = curr_path;
                return true;
            }
            if (_found_node_dirpath(node_id, curr_path, hierarchical_path)) {
                return true;
            }
        }
    }
    return false;
}

fs::path CtStorageMultiFile::_get_node_dirpath(const CtTreeIter& ct_tree_iter) const
{
    fs::path hierarchical_path{std::to_string(ct_tree_iter.get_node_id())};
    CtTreeIter father_iter = ct_tree_iter.parent();
    while (father_iter) {
        hierarchical_path = fs::path{std::to_string(father_iter.get_node_id())} / hierarchical_path;
        father_iter = father_iter.parent();
    }
    return _dir_path / hierarchical_path;
}

void CtStorageMultiFile::_remove_disk_node_with_children(const gint64 node_id)
{
    // the nodes must be passed to the BackupEncrypt thread from the leaves towards the root
    // so all can rotate in the backups
    std::function<void(const fs::path& curr_node_dirpath)> f_iterative_queue_nodes_for_removal;
    f_iterative_queue_nodes_for_removal = [&](const fs::path& curr_node_dirpath){
        const gint64 curr_node_id = CtStrUtil::gint64_from_gstring(curr_node_dirpath.filename().c_str());
        if (_already_queued_for_removal.find(curr_node_id) != _already_queued_for_removal.end()) {
            // already processed
            return;
        }
        // first process the subnodes
        for (const fs::path& node_dirpath : CtStorageMultiFile::get_child_nodes_dirs(curr_node_dirpath)) {
            f_iterative_queue_nodes_for_removal(node_dirpath);
        }
        // eventually process myself
        auto pBackupEncryptData = std::make_shared<CtBackupEncryptData>();
        pBackupEncryptData->backupType = CtBackupType::MultiFile;
        pBackupEncryptData->needEncrypt = false;
        pBackupEncryptData->file_path = _dir_path.string();
        pBackupEncryptData->main_backup = curr_node_dirpath.string();
        pBackupEncryptData->p_mod_time = nullptr;
        _pCtMainWin->get_ct_storage()->backupEncryptDEQueue.push_back(pBackupEncryptData);
        _already_queued_for_removal.insert(curr_node_id);
    };
    fs::path node_dirpath;
    if (_found_node_dirpath(std::to_string(node_id), _dir_path, node_dirpath) and not node_dirpath.empty()) {
        f_iterative_queue_nodes_for_removal(node_dirpath);
    }
}

void CtStorageMultiFile::_write_bookmarks_to_disk(const std::list<gint64>& bookmarks_list)
{
    const fs::path bookmarks_filepath = _dir_path / BOOKMARKS_LST;
    (void)fs::remove(bookmarks_filepath);
    if (not bookmarks_list.empty()) {
        Glib::file_set_contents(bookmarks_filepath.string(),
                                str::join_numbers(bookmarks_list, ","));
    }
}

void CtStorageMultiFile::_hier_try_move_node(const fs::path& dir_path_to)
{
    const fs::path dir_name = dir_path_to.filename();
    fs::path dir_path_from;
    std::function<void(const fs::path parent_path)> f_find_dir_from = [&](const fs::path parent_path){
        for (const fs::path& p : fs::get_dir_entries(parent_path)) {
            if (not dir_path_from.empty()) {
                return;
            }
            if (fs::is_directory(p)) {
                if (p.filename() == dir_name) {
                    dir_path_from = p;
                }
                else {
                    f_find_dir_from(p);
                }
            }
        }
    };
    f_find_dir_from(_dir_path);
    if (not dir_path_from.empty()) {
        spdlog::debug("{} -> {}", dir_path_from.string(), dir_path_to.string());
        fs::move_file(dir_path_from, dir_path_to);
    }
}

bool CtStorageMultiFile::_nodes_to_multifile(const CtTreeIter* ct_tree_iter,
                                             const fs::path& dir_path,
                                             Glib::ustring& error,
                                             CtStorageCache* storage_cache,
                                             const CtStorageNodeState& node_state,
                                             const CtExporting export_type,
                                             const std::map<gint64, gint64>* pExpoMasterReassign/*= nullptr*/,
                                             const int start_offset/*= 0*/,
                                             const int end_offset/*=-1*/)
{
    if (CtExporting::NONESAVE == export_type and
        node_state.hier and
        node_state.is_update_of_existing and
        not fs::is_directory(dir_path))
    {
        _hier_try_move_node(dir_path);
    }
    if (not fs::is_directory(dir_path) and
        g_mkdir(dir_path.c_str(), 0755) < 0)
    {
        error = Glib::ustring{"!! mkdir "} + dir_path.string();
        return false;
    }
    if (node_state.buff or node_state.prop) {
        fs::path dir_before_save;
        if (CtExporting::NONESAVE == export_type) {
            // create folder of previous node.xml and widgets
            // (if binaries not changed, won't re-save but move over)
            dir_before_save = dir_path / BEFORE_SAVE;
            if (fs::is_directory(dir_before_save)) {
                (void)fs::remove_all(dir_before_save);
            }
            if (g_mkdir(dir_before_save.c_str(), 0755) < 0) {
                error = Glib::ustring{"!! mkdir "} + dir_before_save.string();
                return false;
            }
            for (const fs::path& file_from : fs::get_dir_entries(dir_path)) {
                if (fs::is_regular_file(file_from)) {
                    const fs::path name_from = file_from.filename();
                    if (name_from != SUBNODES_LST) {
                        const fs::path file_to = dir_before_save / name_from;
                        fs::move_file(file_from, file_to);
                    }
                }
            }
        }
        {
            xmlpp::Document xml_doc_node;
            xml_doc_node.create_root_node(CtConst::APP_NAME);

            (void)CtStorageXmlHelper{_pCtMainWin}.node_to_xml(
                ct_tree_iter,
                xml_doc_node.get_root_node(),
                dir_path.string()/*multifile_dir*/,
                storage_cache,
                export_type,
                pExpoMasterReassign,
                start_offset,
                end_offset
            );

            // write file
            const std::string xml_filepath = Glib::build_filename(dir_path.string(), NODE_XML);
            xml_doc_node.write_to_file_formatted(xml_filepath);

            // parse back
            try {
                std::unique_ptr<xmlpp::DomParser> parser = CtStorageXml::get_parser(xml_filepath);
            }
            catch (std::exception& ex) {
                error = fmt::format("parse {} after write: {}", xml_filepath, ex.what());
                if (CtExporting::NONESAVE == export_type) {
                    // restore from BEFORE_SAVE
                    for (const fs::path& file_from : fs::get_dir_entries(dir_before_save)) {
                        if (fs::is_regular_file(file_from)) {
                            const fs::path name_from = file_from.filename();
                            const fs::path file_to = dir_path / name_from;
                            fs::move_file(file_from, file_to);
                        }
                    }
                }
                return false;
            }
        }
        if (CtExporting::NONESAVE == export_type) {
            auto pBackupEncryptData = std::make_shared<CtBackupEncryptData>();
            pBackupEncryptData->backupType = CtBackupType::MultiFile;
            pBackupEncryptData->needEncrypt = false;
            pBackupEncryptData->file_path = _dir_path.string();
            pBackupEncryptData->main_backup = dir_before_save.string();
            pBackupEncryptData->p_mod_time = nullptr;
            _pCtMainWin->get_ct_storage()->backupEncryptDEQueue.push_back(pBackupEncryptData);
        }
    }
    // subnodes?
    if (CtExporting::NONESAVE != export_type and
        CtExporting::CURRENT_NODE != export_type and
        CtExporting::SELECTED_TEXT != export_type)
    {
        CtTreeIter ct_tree_iter_child = ct_tree_iter->first_child();
        if (ct_tree_iter_child) {

            std::list<gint64> subnodes_list;

            while (true) {
                const gint64 node_id = ct_tree_iter_child.get_node_id();
                subnodes_list.push_back(node_id);
                if (not _nodes_to_multifile(&ct_tree_iter_child,
                                            dir_path / std::to_string(node_id),
                                            error,
                                            storage_cache,
                                            node_state,
                                            export_type,
                                            pExpoMasterReassign,
                                            start_offset,
                                            end_offset))
                {
                    return false;
                }
                ++ct_tree_iter_child;
                if (not ct_tree_iter_child) {
                    break;
                }
            }

            // save subnodes
            Glib::file_set_contents(Glib::build_filename(dir_path.string(), SUBNODES_LST),
                                    str::join_numbers(subnodes_list, ","));
        }
    }
    return true;
}

/*static*/std::string CtStorageMultiFile::save_blob(const std::string& rawBlob,
                                                    const std::string& dir_path,
                                                    const std::string& file_ext)
{
    const std::string sha256sum = Glib::Checksum::compute_checksum(Glib::Checksum::ChecksumType::CHECKSUM_SHA256, rawBlob);
    const std::string sha256sum_ext = sha256sum + file_ext;
    const std::string filepath = Glib::build_filename(dir_path, sha256sum_ext);
    if (not Glib::file_test(filepath, Glib::FILE_TEST_IS_REGULAR)) {
        const std::string filepath_before = Glib::build_filename(dir_path, BEFORE_SAVE, sha256sum_ext);
        if (Glib::file_test(filepath_before, Glib::FILE_TEST_IS_REGULAR)) {
            fs::move_file(filepath_before, filepath);
        }
        else {
            Glib::file_set_contents(filepath, rawBlob);
        }
    }
    return sha256sum;
}

/*static*/bool CtStorageMultiFile::read_blob(const std::string& dir_path,
                                             const std::string& sha256sum,
                                             std::string& rawBlob)
{
    try {
        Glib::Dir gdir{dir_path};
        std::list<std::string> dir_entries{gdir.begin(), gdir.end()};
        for (const std::string& filename : dir_entries) {
            if (str::startswith(filename, sha256sum)) {
                rawBlob = Glib::file_get_contents(Glib::build_filename(dir_path, filename));
                return true;
            }
        }
    }
    catch (Glib::Error& error) {
        spdlog::error("{} {}", __FUNCTION__, error.what().raw());
    }
    return false;
}

/*static*/std::list<fs::path> CtStorageMultiFile::get_child_nodes_dirs(const fs::path& dir_path)
{
    std::list<fs::path> ret_list;
    const fs::path subnodes_filepath = dir_path / SUBNODES_LST;
    if (fs::is_regular_file(subnodes_filepath)) {
        const std::string subnodes_csv = Glib::file_get_contents(subnodes_filepath.string());
        for (const gint64 nodeId : CtStrUtil::gstring_split_to_int64(subnodes_csv.c_str(), ",")) {
            ret_list.push_back(dir_path / std::to_string(nodeId));
        }
    }
    return ret_list;
}

bool CtStorageMultiFile::populate_treestore(const fs::path& dir_path, Glib::ustring& error)
{
    try {
        if (not fs::is_directory(dir_path)) {
            error = Glib::ustring{"missing "} + dir_path.string();
            return false;
        }
        _dir_path = dir_path;

        CtTreeStore& ct_tree_store = _pCtMainWin->get_tree_store();

        // load bookmarks
        const fs::path bookmarks_filepath = _dir_path / BOOKMARKS_LST;
        if (fs::is_regular_file(bookmarks_filepath)) {
            const std::string bookmarks_csv = Glib::file_get_contents(bookmarks_filepath.string());
            for (const auto nodeId : CtStrUtil::gstring_split_to_int64(bookmarks_csv.c_str(), ",")) {
                if (not _isDryRun) {
                    ct_tree_store.bookmarks_add(nodeId);
                }
            }
        }

        // load node tree
        std::list<CtTreeIter> nodes_with_duplicated_id;
        std::list<CtTreeIter> nodes_shared_non_master;
        std::function<void(const fs::path&, const gint64, Gtk::TreeIter)> f_nodes_from_multifile;
        f_nodes_from_multifile = [&](const fs::path& nodedir, const gint64 sequence, Gtk::TreeIter parent_iter) {
            bool has_duplicated_id{false};
            bool is_shared_non_master{false};
            std::unique_ptr<xmlpp::DomParser> pParser;
            fs::path node_xml_path = nodedir / NODE_XML;
            bool parsingOk{false};
            try {
                pParser = CtStorageXml::get_parser(node_xml_path);
                parsingOk = true;
            }
            catch (std::exception& ex) {
                spdlog::error("parse {} : {} - trying first backup...", node_xml_path.string(), ex.what());
            }
            if (not parsingOk) {
                std::string first_backup_dir;
                CtStorageControl::get_first_backup_file_or_dir(first_backup_dir, _dir_path.string(), _pCtMainWin->get_ct_config());
                int missing_backup{0};
                for (int b = 0; b < 100; ++b) {
                    const fs::path curr_backup_dir = first_backup_dir + str::repeat(CtConst::CHAR_TILDE, b).raw();
                    if (fs::is_directory(curr_backup_dir)) {
                        missing_backup = 0;
                        spdlog::debug("backed up data, {} found", curr_backup_dir.string());
                        const fs::path backup_node_xml_path = curr_backup_dir / nodedir.filename() / NODE_XML;
                        try {
                            pParser = CtStorageXml::get_parser(backup_node_xml_path);
                            parsingOk = true;
                        }
                        catch (std::exception& ex) {
                            spdlog::error("parse {} : {} - trying backup {}...", node_xml_path.string(), ex.what(), b+2);
                        }
                        if (parsingOk) {
                            if (fs::exists(node_xml_path)) {
                                fs::move_file(node_xml_path, node_xml_path.parent_path() / (node_xml_path.stem() + std::string{"_BAD.xml"}));
                            }
                            spdlog::debug("parse backed up data ok, copying {} -> {}", backup_node_xml_path.string(), node_xml_path.string());
                            fs::copy_file(backup_node_xml_path, node_xml_path);
                            if (error.empty()) error += _("A Restore From Backup Was Necessary For:");
                            error += "\n\n" + node_xml_path.string();
                            break;
                        }
                    }
                    else {
                        spdlog::debug("?? backed up data, {} missing", curr_backup_dir.string());
                        if (++missing_backup > 3) break;
                    }
                }
            }
            xmlpp::Node* xml_node = pParser->get_document()->get_root_node()->get_first_child("node");
            auto xml_element = static_cast<xmlpp::Element*>(xml_node);
            Gtk::TreeIter new_iter = CtStorageXmlHelper{_pCtMainWin}.node_from_xml(
                xml_element,
                sequence,
                parent_iter,
                -1/*new_id*/,
                &has_duplicated_id,
                &is_shared_non_master,
                nullptr/*pImportedIdsRemap*/,
                _delayed_text_buffers,
                _isDryRun,
                nodedir.string());
            if (has_duplicated_id and not _isDryRun) {
                nodes_with_duplicated_id.push_back(ct_tree_store.to_ct_tree_iter(new_iter));
            }
            if (is_shared_non_master and not _isDryRun) {
                nodes_shared_non_master.push_back(ct_tree_store.to_ct_tree_iter(new_iter));
            }
            gint64 child_sequence{0};
            for (const fs::path& subnode_dirpath : CtStorageMultiFile::get_child_nodes_dirs(nodedir)) {
                f_nodes_from_multifile(subnode_dirpath, ++child_sequence, new_iter);
            }
        };
        gint64 sequence{0};
        for (const fs::path& node_dirpath : CtStorageMultiFile::get_child_nodes_dirs(_dir_path)) {
            f_nodes_from_multifile(node_dirpath, ++sequence, Gtk::TreeIter{});
        }
        // fix duplicated ids by allocating new ids
        // new ids can be allocated only after the whole tree is parsed
        for (CtTreeIter& ctTreeIter : nodes_with_duplicated_id) {
            ctTreeIter.set_node_id(ct_tree_store.node_id_get());
        }
        // populate shared non master nodes now that the master nodes
        // are in the tree
        for (CtTreeIter& ctTreeIter : nodes_shared_non_master) {
            CtNodeData nodeData{};
            ct_tree_store.get_node_data(ctTreeIter, nodeData, false/*loadTextBuffer*/);
            ct_tree_store.update_node_data(ctTreeIter, nodeData);
        }
        return true;
    }
    catch (std::exception& e) {
        error = e.what();
        return false;
    }
}

void CtStorageMultiFile::import_nodes(const fs::path& dir_path, const Gtk::TreeIter& parent_iter)
{
    CtTreeStore& ct_tree_store = _pCtMainWin->get_tree_store();

    std::list<CtTreeIter> nodes_shared_non_master;
    std::map<gint64,gint64> imported_ids_remap;
    std::function<void(const fs::path&, const gint64 sequence, Gtk::TreeIter)> f_nodes_from_multifile;
    f_nodes_from_multifile = [&](const fs::path& nodedir, const gint64 sequence, Gtk::TreeIter parent_iter) {
        std::unique_ptr<xmlpp::DomParser> parser = CtStorageXml::get_parser(nodedir / NODE_XML);
        xmlpp::Node* xml_node = parser->get_document()->get_root_node()->get_first_child("node");
        auto xml_element = static_cast<xmlpp::Element*>(xml_node);
        bool is_shared_non_master{false};
        Gtk::TreeIter new_iter = CtStorageXmlHelper{_pCtMainWin}.node_from_xml(
            xml_element,
            sequence,
            parent_iter,
            ct_tree_store.node_id_get(),
            nullptr/*pHasDuplicatedId*/,
            &is_shared_non_master,
            &imported_ids_remap,
            _delayed_text_buffers,
            _isDryRun,
            nodedir.string());
        CtTreeIter new_ct_iter = ct_tree_store.to_ct_tree_iter(new_iter);
        new_ct_iter.pending_new_db_node();
        if (is_shared_non_master) {
            nodes_shared_non_master.push_back(ct_tree_store.to_ct_tree_iter(new_iter));
        }
        gint64 child_sequence{0};
        for (const fs::path& subnode_dirpath : CtStorageMultiFile::get_child_nodes_dirs(nodedir)) {
            f_nodes_from_multifile(subnode_dirpath, ++child_sequence, new_iter);
        }
    };
    gint64 sequence{0};
    for (const fs::path& node_dirpath : CtStorageMultiFile::get_child_nodes_dirs(dir_path)) {
        f_nodes_from_multifile(node_dirpath, ++sequence, ct_tree_store.to_ct_tree_iter(parent_iter));
    }
    // populate shared non master nodes now that the master nodes
    // are in the tree
    for (CtTreeIter& ctTreeIter : nodes_shared_non_master) {
        // the shared node master id is remapped after the import
        const gint64 origMasterId = ctTreeIter.get_node_shared_master_id();
        const auto it = imported_ids_remap.find(origMasterId);
        if (imported_ids_remap.end() == it) {
            spdlog::error("!! unexp missing master id {} from remap", origMasterId);
        }
        else {
            ctTreeIter.set_node_shared_master_id(it->second);
            CtNodeData nodeData{};
            ct_tree_store.get_node_data(ctTreeIter, nodeData, false/*loadTextBuffer*/);
            ct_tree_store.update_node_data(ctTreeIter, nodeData);
        }
    }
}

Glib::RefPtr<Gtk::TextBuffer> CtStorageMultiFile::get_delayed_text_buffer(const gint64 node_id,
                                                                          const std::string& syntax,
                                                                          std::list<CtAnchoredWidget*>& widgets) const
{
    if (_delayed_text_buffers.count(node_id) == 0) {
        spdlog::error("!! {} node_id {}", __FUNCTION__, node_id);
        return Glib::RefPtr<Gtk::TextBuffer>{};
    }
    std::shared_ptr<xmlpp::Document> node_buffer = _delayed_text_buffers[node_id];
    auto xml_element = dynamic_cast<xmlpp::Element*>(node_buffer->get_root_node()->get_first_child());
    const fs::path multifile_dir = _get_node_dirpath(_pCtMainWin->get_tree_store().get_node_from_node_id(node_id));
    auto ret_buffer = CtStorageXmlHelper{_pCtMainWin}.create_buffer_and_widgets_from_xml(xml_element, syntax, widgets, nullptr, -1, multifile_dir.string());
    if (ret_buffer) {
        _delayed_text_buffers.erase(node_id);
    }
    return ret_buffer;
}
