/*
 * ct_storage_multifile.h
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
#include "ct_filesystem.h"
#include <glibmm/refptr.h>
#include <gtksourceviewmm/buffer.h>
#include <gtkmm/treeiter.h>
#include <libxml++/libxml++.h>

class CtMainWin;
class CtAnchoredWidget;
class CtTreeIter;
class CtStorageCache;

class CtStorageMultiFile : public CtStorageEntity
{
public:
    static const std::string SUBNODES_LST;
    static const std::string BOOKMARKS_LST;
    static const std::string NODE_XML;
    static const std::string BEFORE_SAVE;

    static std::string save_blob(const std::string& rawBlob,
                                 const std::string& dir_path,
                                 const std::string& file_ext);
    static bool read_blob(const std::string& dir_path,
                          const std::string& sha256sum,
                          std::string& rawBlob);

    static std::list<fs::path> get_child_nodes_dirs(const fs::path& dir_path);

    CtStorageMultiFile(CtMainWin* pCtMainWin)
     : _pCtMainWin{pCtMainWin}
    {}

    void close_connect() override {}
    void reopen_connect() override {}
    void test_connection() override {}
    void vacuum() override {}

    bool populate_treestore(const fs::path& file_path, Glib::ustring& error) override;
    bool save_treestore(const fs::path& dir_path,
                        const CtStorageSyncPending& syncPending,
                        Glib::ustring& error,
                        const CtExporting exporting,
                        const int start_offset = 0,
                        const int end_offset = -1) override;
    void import_nodes(const fs::path& file_path, const Gtk::TreeIter& parent_iter) override;

    Glib::RefPtr<Gsv::Buffer> get_delayed_text_buffer(const gint64& node_id,
                                                      const std::string& syntax,
                                                      std::list<CtAnchoredWidget*>& widgets) const override;

private:
    CtMainWin* const _pCtMainWin;
    fs::path         _dir_path;
    mutable CtDelayedTextBufferMap _delayed_text_buffers;

    fs::path _get_node_dirpath(const CtTreeIter& ct_tree_iter) const;
    bool _found_node_dirpath(const fs::path& node_id, const fs::path parent_path, fs::path& hierarchical_path) const;
    void _remove_disk_node_with_children(const gint64 node_id);
    void _verify_update_hierarchy(const CtTreeIter* ct_tree_iter_parent, const fs::path& dir_path);
    void _hier_try_move_node(const fs::path& dir_path);
    void _write_bookmarks_to_disk(const std::list<gint64>& bookmarks_list);
    bool _nodes_to_multifile(const CtTreeIter* ct_tree_iter,
                             const fs::path& parent_dir_path,
                             Glib::ustring& error,
                             CtStorageCache* storage_cache,
                             const CtStorageNodeState& node_state,
                             const CtExporting exporting,
                             const int start_offset = 0,
                             const int end_offset =-1);
};
