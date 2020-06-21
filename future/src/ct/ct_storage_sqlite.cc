/*
 * ct_storage_sqlite.cc
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

#include "ct_storage_sqlite.h"
#include "ct_storage_xml.h"
#include "ct_storage_control.h"
#include "ct_main_win.h"
#include <unistd.h>
#include "ct_logging.h"


const char CtStorageSqlite::TABLE_NODE_CREATE[]{"CREATE TABLE node ("
"node_id INTEGER UNIQUE,"
"name TEXT,"
"txt TEXT,"
"syntax TEXT,"
"tags TEXT,"
"is_ro INTEGER,"
"is_richtxt INTEGER,"
"has_codebox INTEGER,"
"has_table INTEGER,"
"has_image INTEGER,"
"level INTEGER,"
"ts_creation INTEGER,"
"ts_lastsave INTEGER"
")"
};
const char CtStorageSqlite::TABLE_NODE_INSERT[]{"INSERT INTO node VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)"};
const char CtStorageSqlite::TABLE_NODE_DELETE[]{"DELETE FROM node WHERE node_id=?"};

const char CtStorageSqlite::TABLE_CODEBOX_CREATE[]{"CREATE TABLE codebox ("
"node_id INTEGER,"
"offset INTEGER,"
"justification TEXT,"
"txt TEXT,"
"syntax TEXT,"
"width INTEGER,"
"height INTEGER,"
"is_width_pix INTEGER,"
"do_highl_bra INTEGER,"
"do_show_linenum INTEGER"
")"
};
const char CtStorageSqlite::TABLE_CODEBOX_INSERT[]{"INSERT INTO codebox VALUES(?,?,?,?,?,?,?,?,?,?)"};
const char CtStorageSqlite::TABLE_CODEBOX_DELETE[]{"DELETE FROM codebox WHERE node_id=?"};

const char CtStorageSqlite::TABLE_TABLE_CREATE[]{"CREATE TABLE grid ("
"node_id INTEGER,"
"offset INTEGER,"
"justification TEXT,"
"txt TEXT,"
"col_min INTEGER,"
"col_max INTEGER"
")"
};
const char CtStorageSqlite::TABLE_TABLE_INSERT[]{"INSERT INTO grid VALUES(?,?,?,?,?,?)"};
const char CtStorageSqlite::TABLE_TABLE_DELETE[]{"DELETE FROM grid WHERE node_id=?"};

const char CtStorageSqlite::TABLE_IMAGE_CREATE[]{"CREATE TABLE image ("
"node_id INTEGER,"
"offset INTEGER,"
"justification TEXT,"
"anchor TEXT,"
"png BLOB,"
"filename TEXT,"
"link TEXT,"
"time INTEGER"
")"
};
const char CtStorageSqlite::TABLE_IMAGE_INSERT[]{"INSERT INTO image VALUES(?,?,?,?,?,?,?,?)"};
const char CtStorageSqlite::TABLE_IMAGE_DELETE[]{"DELETE FROM image WHERE node_id=?"};

const char CtStorageSqlite::TABLE_CHILDREN_CREATE[]{"CREATE TABLE children ("
"node_id INTEGER UNIQUE,"
"father_id INTEGER,"
"sequence INTEGER"
")"
};
const char CtStorageSqlite::TABLE_CHILDREN_INSERT[]{"INSERT INTO children (node_id, father_id, sequence) VALUES(?,?,?)"};
const char CtStorageSqlite::TABLE_CHILDREN_DELETE[]{"DELETE FROM children WHERE node_id=?"};

const char CtStorageSqlite::TABLE_BOOKMARK_CREATE[]{"CREATE TABLE bookmark ("
"node_id INTEGER UNIQUE,"
"sequence INTEGER"
")"
};
const char CtStorageSqlite::TABLE_BOOKMARK_INSERT[]{"INSERT INTO bookmark VALUES(?,?)"};
const char CtStorageSqlite::TABLE_BOOKMARK_DELETE[]{"DELETE FROM bookmark"};

const Glib::ustring CtStorageSqlite::ERR_SQLITE_PREPV2{"!! sqlite3_prepare_v2: "};
const Glib::ustring CtStorageSqlite::ERR_SQLITE_STEP{"!! sqlite3_step: "};

struct sqlite3_stmt_auto
{
    sqlite3_stmt_auto() = default;
    sqlite3_stmt_auto(sqlite3* db, const char* sql) { prepare(db, sql); }
    ~sqlite3_stmt_auto() { sqlite3_finalize(p_stmt); }

    bool prepare(sqlite3* db, const char* sql) { return sqlite3_prepare_v2(db, sql, -1, &p_stmt, nullptr) == SQLITE_OK; }
    operator sqlite3_stmt*() { return p_stmt; }
    bool is_bad() { return !p_stmt; } // it could be operator bool(), but this way it's more explicit in conditions

    sqlite3_stmt *p_stmt{nullptr};
};


std::optional<std::vector<std::string>> get_quick_check_issues(sqlite3* db) {
    if (!db) throw std::logic_error("get_quick_check_issues passed invalid database object");

    sqlite3_stmt_auto stmt(db, "PRAGMA quick_check");
    
    std::vector<std::string> rows;
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        std::string info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (info == "ok" && rows.empty()) {
            // First column and is OK, database is fine
            return std::nullopt;
        } else {
            rows.emplace_back(info);
        }
    }
    
    return rows;
}

bool CtStorageSqlite::_check_database_integrity() {
    auto corrupted_rows = get_quick_check_issues(_pDb);
    if (!corrupted_rows) return true;

    // Database is corrupted
    std::string log_msg = "\n=== Database problems report ===\n";
    for (const auto& corrupted_row : *corrupted_rows) {
        log_msg += fmt::format(": {}\n", corrupted_row);
    }
    spdlog::warn(log_msg);

    CtDialogs::warning_dialog(str::format(_("The database file %s is corrupt, see log for more details"), _file_path.string()), *_pCtMainWin);
    return false;
}

CtStorageSqlite::CtStorageSqlite(CtMainWin* pCtMainWin) : _pCtMainWin(pCtMainWin)
{

}

CtStorageSqlite::~CtStorageSqlite()
{
    _close_db();
}

void CtStorageSqlite::close_connect()
{
    _close_db();
}

void CtStorageSqlite::reopen_connect()
{
    _open_db(_file_path.c_str());
}

void CtStorageSqlite::test_connection()
{
    if (_file_path.empty()) return;

    auto test_readwrite = [&]() {
        try
        {
            _exec_no_callback("CREATE TABLE IF NOT EXISTS test_table (id)");
            _exec_no_callback("DROP TABLE IF EXISTS test_table");
            return true;
        } catch (std::exception&) { return false; }
    };

    if (_pDb && test_readwrite())
        return;

    _close_db();

    // todo: fix for win32
#ifndef _WIN32
    usleep(500 * 1000); // wait 0.5 sec, file can be block by sync program like Dropbox
#endif

    try {
        _open_db(_file_path);
    } catch(std::exception&) {
        throw std::runtime_error(str::format(_("%s write failed - file is missing. Reattach usb driver or shared resource"), _file_path));
    }
    if (!test_readwrite())
        throw std::runtime_error(str::format(_("%s write failed - is file blocked by a sync program?"), _file_path));
    if (!_check_database_integrity()) return;
}

bool CtStorageSqlite::populate_treestore(const fs::path& file_path, Glib::ustring& error)
{
    _close_db();
    try
    {
        // open db
        _open_db(file_path);
        _file_path = file_path;
        _fix_db_tables();

        if (!_check_database_integrity()) return false;


        // load bookmarks
        sqlite3_stmt_auto stmt(_pDb, "SELECT node_id FROM bookmark ORDER BY sequence ASC");
        if (stmt.is_bad())
            throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));
        while (sqlite3_step(stmt) == SQLITE_ROW)
               _pCtMainWin->get_tree_store().bookmarks_add(sqlite3_column_int64(stmt, 0));

        // load node tree
        std::function<void(guint node_id, Gtk::TreeIter)> nodes_from_db;
        nodes_from_db = [&](guint node_id, Gtk::TreeIter parent_iter) {
            Gtk::TreeIter new_iter = _node_from_db(node_id, parent_iter, -1);
            for (gint64 child_node_id : _get_children_node_ids_from_db(node_id))
                nodes_from_db(child_node_id, new_iter);
        };
        for (gint64 &top_node_id: _get_children_node_ids_from_db(0))
            nodes_from_db(top_node_id, Gtk::TreeIter());

        // keep db open for lazy node buffer loading
        return true;
    }
    catch (std::exception& e)
    {
        _close_db();
        error = e.what();
        return false;
    }
}

bool CtStorageSqlite::save_treestore(const fs::path& file_path, const CtStorageSyncPending& syncPending, Glib::ustring& error)
{
    try
    {
        // it's the first time, a new file will be created
        if (_pDb == nullptr)
        {
            _open_db(file_path);
            _file_path = file_path;

            _create_all_tables_in_db();
            _write_bookmarks_to_db(_pCtMainWin->get_tree_store().bookmarks_get());

            CtStorageNodeState node_state;
            node_state.upd = false; // no need to delete the prev data
            node_state.prop = true;
            node_state.buff = true;
            node_state.hier = true;

            CtStorageCache storage_cache;
            storage_cache.generate_cache(_pCtMainWin, nullptr /* all nodes */, false);

            // function to iterate through the tree
            std::function<void(CtTreeIter, const gint64, const gint64)> save_node_fun;
            save_node_fun = [&](CtTreeIter ct_tree_iter, const gint64 sequence, const gint64 father_id) {
                _write_node_to_db(&ct_tree_iter, sequence, father_id, node_state, 0, -1, &storage_cache);
                gint64 child_sequence{0};
                CtTreeIter ct_tree_iter_child = ct_tree_iter.first_child();
                while (ct_tree_iter_child) {
                    ++child_sequence;
                    save_node_fun(ct_tree_iter_child, child_sequence, ct_tree_iter.get_node_id());
                    ++ct_tree_iter_child;
                }
            };

            // saving nodes
            gint64 sequence{0};
            CtTreeIter ct_tree_iter = _pCtMainWin->get_tree_store().get_ct_iter_first();
            while (ct_tree_iter) {
                ++sequence;
                save_node_fun(ct_tree_iter, sequence, 0);
                ++ct_tree_iter;
            }

        }
        // or need just update some info
        else
        {
            CtStorageCache storage_cache;
            storage_cache.generate_cache(_pCtMainWin, &syncPending, false);

            // update bookmarks
            if (syncPending.bookmarks_to_write)
                _write_bookmarks_to_db(_pCtMainWin->get_tree_store().bookmarks_get());
            // update changed nodes
            for (const auto& node_pair : syncPending.nodes_to_write_dict)
            {
                CtTreeIter ct_tree_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(node_pair.first);
                CtTreeIter ct_tree_iter_parent = ct_tree_iter.parent();
                _write_node_to_db(&ct_tree_iter, ct_tree_iter.get_node_sequence(),
                                  ct_tree_iter_parent ? ct_tree_iter_parent.get_node_id() : 0, node_pair.second, 0, -1, &storage_cache);
            }
            // remove nodes and their sub nodes
            for (const auto node_id : syncPending.nodes_to_rm_set)
                _remove_db_node_with_children(node_id);
        }

        return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
        return false;
    }
}

void CtStorageSqlite::vacuum()
{
    spdlog::debug("VACUUM");
    _exec_no_callback("VACUUM");
    _exec_no_callback("REINDEX");
}

void CtStorageSqlite::_open_db(const fs::path& path)
{
    if (_pDb) return;
    if (sqlite3_open(path.c_str(), &_pDb) != SQLITE_OK)
    {
        std::string error = sqlite3_errmsg(_pDb);
        sqlite3_close(_pDb); // even after error, _pDb is initialized
        _pDb = nullptr;
        throw std::runtime_error(std::string("sqlite3_open: ") + error);
    }
}

void CtStorageSqlite::_close_db()
{
    if (!_pDb) return;
    sqlite3_close(_pDb);
    _pDb = nullptr;
    //_file_path = ""; we need file_path for reconnection
}

Gtk::TreeIter CtStorageSqlite::_node_from_db(gint64 node_id, Gtk::TreeIter parent_iter, gint64 new_id)
{
    sqlite3_stmt_auto stmt(_pDb, "SELECT name, syntax, tags, is_ro, is_richtxt, ts_creation, ts_lastsave FROM node WHERE node_id=?");
    if (stmt.is_bad())
        throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));

    sqlite3_bind_int64(stmt, 1, node_id);
    if (sqlite3_step(stmt) != SQLITE_ROW)
        throw std::runtime_error(std::string("CtDocSqliteStorage: missing node properties for id ") + std::to_string(node_id));

    CtNodeData nodeData;
    nodeData.nodeId = new_id == -1 ? node_id : new_id;
    nodeData.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    nodeData.syntax = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    nodeData.tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    gint64 readonly_n_custom_icon_id = sqlite3_column_int64(stmt, 3);
    nodeData.isRO = static_cast<bool>(readonly_n_custom_icon_id & 0x01);
    nodeData.customIconId = readonly_n_custom_icon_id >> 1;
    gint64 richtxt_bold_foreground = sqlite3_column_int64(stmt, 4);
    nodeData.isBold = static_cast<bool>((richtxt_bold_foreground >> 1) & 0x01);
    if (static_cast<bool>((richtxt_bold_foreground >> 2) & 0x01))
    {
        char foregroundRgb24[8];
        CtRgbUtil::set_rgb24str_from_rgb24int((richtxt_bold_foreground >> 3) & 0xffffff, foregroundRgb24);
        nodeData.foregroundRgb24 = foregroundRgb24;
    }
    nodeData.tsCreation = sqlite3_column_int64(stmt, 5);
    nodeData.tsLastSave = sqlite3_column_int64(stmt, 6);

    // buffer for imported node should be loaded now because file will be closed
    if (new_id != -1) {
        nodeData.rTextBuffer = get_delayed_text_buffer(node_id, nodeData.syntax, nodeData.anchoredWidgets);
    }

    return _pCtMainWin->get_tree_store().append_node(&nodeData, &parent_iter);
}

Glib::RefPtr<Gsv::Buffer> CtStorageSqlite::get_delayed_text_buffer(const gint64& node_id,
                                                                      const std::string& syntax,
                                                                      std::list<CtAnchoredWidget*>& widgets) const
{
    sqlite3_stmt_auto stmt(_pDb, "SELECT txt, has_codebox, has_table, has_image FROM node WHERE node_id=?");
    if (stmt.is_bad())
    {
        spdlog::error("{}: {}", ERR_SQLITE_PREPV2, sqlite3_errmsg(_pDb));
        return Glib::RefPtr<Gsv::Buffer>();
    }

    sqlite3_bind_int64(stmt, 1, node_id);
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        spdlog::error("!! missing node properties for id {}", node_id);
        return Glib::RefPtr<Gsv::Buffer>();
    }

    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer{nullptr};
    const char* textContent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (CtConst::RICH_TEXT_ID != syntax)
    {
        rRetTextBuffer = _pCtMainWin->get_new_text_buffer(textContent);
    }
    else
    {
        rRetTextBuffer = CtStorageXmlHelper(_pCtMainWin).create_buffer_no_widgets(syntax, textContent);
        if (!rRetTextBuffer)
        {
            spdlog::error("!! xml read: {}", textContent);
            return rRetTextBuffer;
        }
        if (sqlite3_column_int64(stmt, 1)) _codebox_from_db(node_id, widgets);
        if (sqlite3_column_int64(stmt, 2)) _table_from_db(node_id, widgets);
        if (sqlite3_column_int64(stmt, 3)) _image_from_db(node_id, widgets);

        widgets.sort([](CtAnchoredWidget* w1, CtAnchoredWidget* w2) { return w1->getOffset() < w2->getOffset(); });
        rRetTextBuffer->begin_not_undoable_action();
        for (auto widget: widgets)
            widget->insertInTextBuffer(rRetTextBuffer);
        rRetTextBuffer->end_not_undoable_action();
        rRetTextBuffer->set_modified(false);
    }
    return rRetTextBuffer;
}

void CtStorageSqlite::_image_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const
{
    sqlite3_stmt_auto stmt(_pDb, "SELECT * FROM image WHERE node_id=? ORDER BY offset ASC");
    if (stmt.is_bad())
    {
        spdlog::error("{}: {}", ERR_SQLITE_PREPV2, sqlite3_errmsg(_pDb));
        return;
    }
    sqlite3_bind_int64(stmt, 1, nodeId);

    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        int charOffset = sqlite3_column_int64(stmt, 1);
        Glib::ustring justification = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (justification.empty()) justification = CtConst::TAG_PROP_VAL_LEFT;

        // image
        const Glib::ustring anchorName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        if (!anchorName.empty())
        {
            anchoredWidgets.push_back(new CtImageAnchor(_pCtMainWin, anchorName, charOffset, justification));
        }
        else
        {
            fs::path fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            const void* pBlob = sqlite3_column_blob(stmt, 4);
            const int blobSize = sqlite3_column_bytes(stmt, 4);
            const std::string rawBlob(reinterpret_cast<const char*>(pBlob), static_cast<size_t>(blobSize));
            if (!fileName.empty())
            {
                const double timeDouble = sqlite3_column_int64(stmt, 7);
                anchoredWidgets.push_back(new CtImageEmbFile(_pCtMainWin, fileName, rawBlob, timeDouble, charOffset, justification));
            }
            else
            {
                const Glib::ustring link = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
                anchoredWidgets.push_back(new CtImagePng(_pCtMainWin, rawBlob, link, charOffset, justification));
            }
        }

    }
}

void CtStorageSqlite::_codebox_from_db(const gint64& nodeId ,std::list<CtAnchoredWidget*>& anchoredWidgets) const
{
    sqlite3_stmt_auto stmt(_pDb, "SELECT * FROM codebox WHERE node_id=? ORDER BY offset ASC");
    if (stmt.is_bad())
    {
        spdlog::error("{}: {}", ERR_SQLITE_PREPV2, sqlite3_errmsg(_pDb));
        return;
    }
    sqlite3_bind_int64(stmt, 1, nodeId);

    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        int charOffset = sqlite3_column_int64(stmt, 1);
        Glib::ustring justification = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (justification.empty()) justification = CtConst::TAG_PROP_VAL_LEFT;

        const Glib::ustring textContent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const Glib::ustring syntaxHighlighting = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const int frameWidth = sqlite3_column_int64(stmt, 5);
        const int frameHeight = sqlite3_column_int64(stmt, 6);
        const bool widthInPixels = sqlite3_column_int64(stmt, 7);
        const bool highlightBrackets = sqlite3_column_int64(stmt, 8);
        const bool showLineNumbers = sqlite3_column_int64(stmt, 9);

        anchoredWidgets.push_back(new CtCodebox(_pCtMainWin,
                                                textContent,
                                                syntaxHighlighting,
                                                frameWidth,
                                                frameHeight,
                                                charOffset,
                                                justification,
                                                widthInPixels,
                                                highlightBrackets,
                                                showLineNumbers));

    }
}

void CtStorageSqlite::_table_from_db(const gint64& nodeId, std::list<CtAnchoredWidget*>& anchoredWidgets) const
{
    sqlite3_stmt_auto stmt(_pDb, "SELECT * FROM grid WHERE node_id=? ORDER BY offset ASC");
    if (stmt.is_bad())
    {
        spdlog::error("{}: {}", ERR_SQLITE_PREPV2, sqlite3_errmsg(_pDb));
        return;
    }
    sqlite3_bind_int64(stmt, 1, nodeId);

    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        int charOffset = sqlite3_column_int64(stmt, 1);
        Glib::ustring justification = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (justification.empty()) justification = CtConst::TAG_PROP_VAL_LEFT;

        const char* textContent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const int colMin = sqlite3_column_int64(stmt, 4);
        const int colMax = sqlite3_column_int64(stmt, 5);

        CtTableMatrix tableMatrix;
        if (CtStorageXmlHelper(_pCtMainWin).populate_table_matrix(tableMatrix, textContent))
        {
            anchoredWidgets.push_back(new CtTable(_pCtMainWin, tableMatrix, colMin, colMax, charOffset, justification));
        }
        else
        {
            spdlog::error("!! table xml read: {}", textContent);
        }
    }
}

void CtStorageSqlite::_create_all_tables_in_db()
{
    _exec_no_callback(TABLE_NODE_CREATE);
    _exec_no_callback(TABLE_CODEBOX_CREATE);
    _exec_no_callback(TABLE_TABLE_CREATE);
    _exec_no_callback(TABLE_IMAGE_CREATE);
    _exec_no_callback(TABLE_CHILDREN_CREATE);
    _exec_no_callback(TABLE_BOOKMARK_CREATE);
}

void CtStorageSqlite::_write_bookmarks_to_db(const std::list<gint64>& bookmarks)
{
    _exec_no_callback(TABLE_BOOKMARK_DELETE);

    sqlite3_stmt_auto stmt(_pDb, TABLE_BOOKMARK_INSERT);
    if (stmt.is_bad())
        throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));

    gint64 sequence{0};
    for (gint64 bookmark : bookmarks)
    {
        ++sequence;
        sqlite3_bind_int64(stmt, 1, bookmark);
        sqlite3_bind_int64(stmt, 2, sequence);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw std::runtime_error(ERR_SQLITE_STEP + sqlite3_errmsg(_pDb));
        sqlite3_reset(stmt);
    }
}

void CtStorageSqlite::_write_node_to_db(CtTreeIter* ct_tree_iter,
                                        const gint64 sequence,
                                        const gint64 node_father_id,
                                        const CtStorageNodeState& node_state,
                                        const int start_offset, const int end_offset,
                                        CtStorageCache* storage_cache)
{
    const gint64 node_id = ct_tree_iter->get_node_id();
    // is_ro is packed with additional bitfield data
    gint64 is_ro = ct_tree_iter->get_node_read_only() ? 0x01 : 0x00;
    is_ro |= ct_tree_iter->get_node_custom_icon_id() << 1;
    // is_richtxt is packed with additional bitfield data
    gint64 is_richtxt = ct_tree_iter->get_node_is_rich_text() ? 0x01 : 0x00;
    if (ct_tree_iter->get_node_is_bold())
    {
        is_richtxt |= 0x02;
    }
    if (!ct_tree_iter->get_node_foreground().empty())
    {
        is_richtxt |= 0x04;
        is_richtxt |= CtRgbUtil::get_rgb24int_from_str_any(ct_tree_iter->get_node_foreground().c_str()+1) << 3;
    }

    bool remove_prev_widgets = node_state.upd && node_state.buff;
    bool remove_prev_node = node_state.upd && node_state.buff && node_state.prop;
    bool remove_prev_hier = node_state.upd && node_state.hier;

    // remove previous data in case full update (skip when add new or partial update
    if (remove_prev_widgets)
    {
        _exec_bind_int64(TABLE_CODEBOX_DELETE, node_id);
        _exec_bind_int64(TABLE_TABLE_DELETE, node_id);
        _exec_bind_int64(TABLE_IMAGE_DELETE, node_id);
    }
    if (remove_prev_node)
        _exec_bind_int64(TABLE_NODE_DELETE, node_id);
    if (remove_prev_hier)
        _exec_bind_int64(TABLE_CHILDREN_DELETE, node_id);

    bool has_codebox{false};
    bool has_table{false};
    bool has_image{false};

    // write hier
    if (node_state.hier)
    {
        sqlite3_stmt_auto stmt(_pDb, TABLE_CHILDREN_INSERT);
        if (stmt.is_bad())
            throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));
        sqlite3_bind_int64(stmt, 1, node_id);
        sqlite3_bind_int64(stmt, 2, node_father_id);
        sqlite3_bind_int64(stmt, 3, sequence);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw std::runtime_error(ERR_SQLITE_STEP + sqlite3_errmsg(_pDb));
    }

    // write widgets
    if (node_state.buff && (is_richtxt & 0x01))
    {
        for (CtAnchoredWidget* pAnchoredWidget : ct_tree_iter->get_embedded_pixbufs_tables_codeboxes(start_offset, end_offset))
        {
            if (!pAnchoredWidget->to_sqlite(_pDb, node_id, start_offset >= 0 ? -start_offset : 0, storage_cache))
                throw std::runtime_error("couldn't save widget");
            switch (pAnchoredWidget->get_type())
            {
                case CtAnchWidgType::CodeBox: has_codebox = true; break;
                case CtAnchWidgType::Table: has_table = true; break;
                default: has_image = true;
            }
        }
    }

    // write node
    if (node_state.buff)
    {
        // get buffer content
        std::string node_txt;
        if (is_richtxt & 0x01)
        {
            xmlpp::Document xml_doc;
            xml_doc.create_root_node("node");
            CtStorageXmlHelper::save_buffer_no_widgets_to_xml(xml_doc.get_root_node(), ct_tree_iter->get_node_text_buffer(), start_offset, end_offset, 'n');
            node_txt = xml_doc.write_to_string();
        }
        else
        {
            node_txt = ct_tree_iter->get_node_text_buffer()->get_text();
        }

        // full node rewrite
        if (node_state.buff && node_state.prop)
        {
            sqlite3_stmt_auto stmt(_pDb, TABLE_NODE_INSERT);
            if (stmt.is_bad())
                throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));

            const std::string node_name = ct_tree_iter->get_node_name();
            const std::string node_syntax = ct_tree_iter->get_node_syntax_highlighting();
            const std::string node_tags = ct_tree_iter->get_node_tags();
            sqlite3_bind_int64(stmt, 1, node_id);
            sqlite3_bind_text(stmt, 2, node_name.c_str(), node_name.size(), SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, node_txt.c_str(), node_txt.size(), SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, node_syntax.c_str(), node_syntax.size(), SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, node_tags.c_str(), node_tags.size(), SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 6, is_ro);
            sqlite3_bind_int64(stmt, 7, is_richtxt);
            sqlite3_bind_int64(stmt, 8, has_codebox);
            sqlite3_bind_int64(stmt, 9, has_table);
            sqlite3_bind_int64(stmt, 10, has_image);
            sqlite3_bind_int64(stmt, 11, 0); // todo: get rid of unused column 'level'
            sqlite3_bind_int64(stmt, 12, ct_tree_iter->get_node_creating_time());
            sqlite3_bind_int64(stmt, 13, ct_tree_iter->get_node_modification_time());
            if (sqlite3_step(stmt) != SQLITE_DONE)
                throw std::runtime_error(ERR_SQLITE_STEP + sqlite3_errmsg(_pDb));
        }
        // only node buff rewrite
        else if (node_state.buff)
        {
            sqlite3_stmt_auto stmt(_pDb, "UPDATE node SET txt=?, syntax=?, is_richtxt=?, has_codebox=?, has_table=?, has_image=?, ts_lastsave=? WHERE node_id=?");
            if (stmt.is_bad())
                throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));

            const std::string node_syntax = ct_tree_iter->get_node_syntax_highlighting();
            sqlite3_bind_text(stmt, 1, node_txt.c_str(), node_txt.size(), SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, node_syntax.c_str(), node_syntax.size(), SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 3, is_richtxt);
            sqlite3_bind_int64(stmt, 4, has_codebox);
            sqlite3_bind_int64(stmt, 5, has_table);
            sqlite3_bind_int64(stmt, 6, has_image);
            sqlite3_bind_int64(stmt, 7, ct_tree_iter->get_node_modification_time());
            sqlite3_bind_int64(stmt, 8, node_id);
            if (sqlite3_step(stmt) != SQLITE_DONE)
                throw std::runtime_error(ERR_SQLITE_STEP + sqlite3_errmsg(_pDb));
        }
        // only node prop rewrite
        else if (node_state.prop)
        {
            sqlite3_stmt_auto stmt(_pDb, "UPDATE node SET name=?, syntax=?, tags=?, is_ro=?, is_richtxt=? WHERE node_id=?");
            if (stmt.is_bad())
                throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));

            const std::string node_name = ct_tree_iter->get_node_name();
            const std::string node_syntax = ct_tree_iter->get_node_syntax_highlighting();
            const std::string node_tags = ct_tree_iter->get_node_tags();
            sqlite3_bind_text(stmt, 1, node_name.c_str(), node_name.size(), SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, node_syntax.c_str(), node_syntax.size(), SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, node_tags.c_str(), node_tags.size(), SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 4, is_ro);
            sqlite3_bind_int64(stmt, 5, is_richtxt);
            sqlite3_bind_int64(stmt, 6, node_id);
            if (sqlite3_step(stmt) != SQLITE_DONE)
                throw std::runtime_error(ERR_SQLITE_STEP + sqlite3_errmsg(_pDb));
        }
    }
}

std::list<gint64> CtStorageSqlite::_get_children_node_ids_from_db(gint64 father_id)
{
    sqlite3_stmt_auto stmt(_pDb, "SELECT node_id FROM children WHERE father_id=? ORDER BY sequence ASC");
    if (stmt.is_bad())
        throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));

    std::list<gint64> node_children;
    sqlite3_bind_int64(stmt, 1, father_id);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        node_children.push_back(sqlite3_column_int64(stmt, 0));

    return node_children;
}

void CtStorageSqlite::_remove_db_node_with_children(const gint64 node_id)
{
    _exec_bind_int64(TABLE_CODEBOX_DELETE, node_id);
    _exec_bind_int64(TABLE_TABLE_DELETE, node_id);
    _exec_bind_int64(TABLE_IMAGE_DELETE, node_id);
    _exec_bind_int64(TABLE_NODE_DELETE, node_id);
    _exec_bind_int64(TABLE_CHILDREN_DELETE, node_id);

    for (const gint64 child_node_id: _get_children_node_ids_from_db(node_id))
        _remove_db_node_with_children(child_node_id);
}

void CtStorageSqlite::_exec_no_callback(const char* sqlCmd)
{
    char *p_err_msg{nullptr};
    if (SQLITE_OK != sqlite3_exec(_pDb, sqlCmd, nullptr, nullptr, &p_err_msg))
    {
        std::string msg = std::string("!! sqlite3 '") + sqlCmd + "': " + p_err_msg;
        sqlite3_free(p_err_msg);
        throw std::runtime_error(msg);
    }
}

void CtStorageSqlite::_exec_bind_int64(const char* sqlCmd, const gint64 bind_int64)
{
    sqlite3_stmt_auto stmt(_pDb, sqlCmd);
    if (stmt.is_bad())
        throw std::runtime_error(ERR_SQLITE_PREPV2 + sqlite3_errmsg(_pDb));
    sqlite3_bind_int64(stmt, 1, bind_int64);
    if (sqlite3_step(stmt) != SQLITE_DONE)
        throw std::runtime_error(ERR_SQLITE_STEP + sqlite3_errmsg(_pDb));
}

void CtStorageSqlite::import_nodes(const fs::path& path)
{
    _open_db(path); // storage is temp so can just open db
    if (!_check_database_integrity()) return; 
    // _fix_db_tables(); how to do it withough saving changes
    
    std::function<void(gint64, Gtk::TreeIter)> add_node_func;
    add_node_func = [this, &add_node_func](gint64 nodeId, Gtk::TreeIter parent_iter) {
        auto node_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(_node_from_db(nodeId, parent_iter, _pCtMainWin->get_tree_store().node_id_get()));
        node_iter.pending_new_db_node();
        for (auto child_id : _get_children_node_ids_from_db(nodeId)) {
            add_node_func(child_id, node_iter);
        }
    };

    for (auto node_id : _get_children_node_ids_from_db(0))
        add_node_func(node_id, Gtk::TreeIter());

    _close_db();
}

std::unordered_set<std::string> CtStorageSqlite::_get_table_field_names(std::string_view table_name) 
{
    // Note, possible SQL injection - Table names passed to this should be hardcoded 
    auto fields_info_pragma = fmt::format("PRAGMA table_info({})", table_name);
    sqlite3_stmt_auto stmt(_pDb, fields_info_pragma.c_str());

    std::unordered_set<std::string> fields;
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        // Name is the second column
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        fields.emplace(std::move(name));
    }

    return fields;
}

void CtStorageSqlite::_fix_db_tables()
{
    const static std::vector<std::vector<std::string>> tables = {
        {"node", "ts_creation", "INTEGER", "ts_lastsave", "INTEGER"}, {"image", "filename", "TEXT", "link", "TEXT", "time", "TEXT"}
    };
    
    try {
        for (const auto& table : tables) {
            auto& table_name = table[0];
            auto node_fields = _get_table_field_names(table_name);
            for (auto field = table.begin() + 1; field != table.end(); field += 2) {
                if (node_fields.find(*field) == node_fields.end()) {
                    auto sql = fmt::format("ALTER TABLE {} ADD COLUMN {} {}", table_name, *field, *(field + 1));
                    _exec_no_callback(sql.c_str());
                }
                // Stop us going off the end
                if ((field + 1) == table.end()) break;
            }
        }

    } catch(std::runtime_error& e) {
            throw std::runtime_error(fmt::format("Error while adding ts_creation and ts_lastsave to node table: {}", e.what()));
        }
}
