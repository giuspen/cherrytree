/*
 * sqlite3.cc
 *
 * Copyright 2017 giuspen <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// https://sqlite.org/cintro.html
// http://zetcode.com/db/sqlitec
// g++ sqlite3.cc -o sqlite3 `pkg-config sqlite3 gtkmm-3.0 --cflags --libs`

#include <assert.h>
#include <iostream>
#include <sqlite3.h>
#include <gtkmm.h>


struct t_node_properties
{
    gint64         node_id;
    Glib::ustring  name;
    Glib::ustring  syntax;
    Glib::ustring  tags;
    bool           is_ro;
    guint32        custom_icon_id;
    bool           is_bold;
    bool           fg_override;
    char           foreground[8];
    gint64         ts_creation;
    gint64         ts_lastsave;
};


gint64 gint64_from_gstring(gchar *in_gstring)
{
    return g_ascii_strtoll(in_gstring, NULL, 10);
}


class CherryTreeDocRead
{
public:
    CherryTreeDocRead(std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    virtual ~CherryTreeDocRead();
    virtual void tree_walk(Gtk::TreeIter parent_iter)=0;
protected:
    std::list<gint64> *mp_bookmarks;
    Glib::RefPtr<Gtk::TreeStore>  mr_treestore;
};


class CherryTreeSQLiteRead : public CherryTreeDocRead
{
public:
    CherryTreeSQLiteRead(Glib::ustring &filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    virtual ~CherryTreeSQLiteRead();
    void tree_walk(Gtk::TreeIter parent_iter);
private:
    sqlite3 *mp_db;
    std::list<gint64> _sqlite3_get_children_node_id_from_father_id(gint64 father_id);
    void _sqlite3_tree_walk_iter(gint64 node_id, Gtk::TreeIter parent_iter);
    t_node_properties _sqlite3_get_node_properties(gint64 node_id);
    Gtk::TreeIter _sqlite3_node_process(gint64 node_id, Gtk::TreeIter parent_iter);
};


CherryTreeDocRead::CherryTreeDocRead(std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore) : mp_bookmarks(p_bookmarks), mr_treestore(r_treestore)
{
}


CherryTreeDocRead::~CherryTreeDocRead()
{
}


CherryTreeSQLiteRead::CherryTreeSQLiteRead(Glib::ustring &filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore) : CherryTreeDocRead(p_bookmarks, r_treestore)
{
    int ret_code = sqlite3_open(filepath.c_str(), &mp_db);
    if(ret_code != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_open: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
}


CherryTreeSQLiteRead::~CherryTreeSQLiteRead()
{
    sqlite3_close(mp_db);
}


void CherryTreeSQLiteRead::tree_walk(Gtk::TreeIter parent_iter)
{
    sqlite3_stmt *p_stmt;
    if(sqlite3_prepare_v2(mp_db, "SELECT node_id FROM bookmark ORDER BY sequence ASC", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
    while(sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        gint64 node_id = sqlite3_column_int64(p_stmt, 0);
        mp_bookmarks->push_back(node_id);
    }
    sqlite3_finalize(p_stmt);

    std::list<gint64> top_nodes_ids = _sqlite3_get_children_node_id_from_father_id(0);
    for(gint64 top_node_id : top_nodes_ids)
    {
        _sqlite3_tree_walk_iter(top_node_id, parent_iter);
    }
}


void CherryTreeSQLiteRead::_sqlite3_tree_walk_iter(gint64 node_id, Gtk::TreeIter parent_iter)
{
    Gtk::TreeIter new_iter = _sqlite3_node_process(node_id, parent_iter);

    std::list<gint64> children_nodes_ids = _sqlite3_get_children_node_id_from_father_id(node_id);
    for(gint64 child_node_id : children_nodes_ids)
    {
        _sqlite3_tree_walk_iter(child_node_id, new_iter);
    }
}


std::list<gint64> CherryTreeSQLiteRead::_sqlite3_get_children_node_id_from_father_id(gint64 father_id)
{
    std::list<gint64> ret_children;
    sqlite3_stmt *p_stmt;
    if(sqlite3_prepare_v2(mp_db, "SELECT node_id FROM children WHERE father_id=? ORDER BY sequence ASC", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
    sqlite3_bind_int(p_stmt, 1, father_id);
    while(sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        gint64 node_id = sqlite3_column_int64(p_stmt, 0);
        ret_children.push_back(node_id);
    }
    sqlite3_finalize(p_stmt);
    return ret_children;
}


t_node_properties CherryTreeSQLiteRead::_sqlite3_get_node_properties(gint64 node_id)
{
    t_node_properties node_properties;
    node_properties.node_id = node_id;
    sqlite3_stmt *p_stmt;
    if(sqlite3_prepare_v2(mp_db, "SELECT name, syntax, tags, is_ro, is_richtxt, ts_creation, ts_lastsave FROM node WHERE node_id=?", -1, &p_stmt, 0) != SQLITE_OK)
    {
        std::cerr << "!! sqlite3_prepare_v2: " << sqlite3_errmsg(mp_db) << std::endl;
        exit(EXIT_FAILURE);
    }
    sqlite3_bind_int(p_stmt, 1, node_id);
    if(sqlite3_step(p_stmt) == SQLITE_ROW)
    {
        node_properties.name = (const char*)sqlite3_column_text(p_stmt, 0);
        node_properties.syntax = (const char*)sqlite3_column_text(p_stmt, 1);
        node_properties.tags = (const char*)sqlite3_column_text(p_stmt, 2);
        gint64 readonly_n_custom_icon_id = sqlite3_column_int64(p_stmt, 3);
        node_properties.is_ro = bool(readonly_n_custom_icon_id & 0x01);
        node_properties.custom_icon_id = readonly_n_custom_icon_id >> 1;
        gint64 richtxt_bold_foreground = sqlite3_column_int64(p_stmt, 4);
        node_properties.is_bold = bool((richtxt_bold_foreground >> 1) & 0x01);
        node_properties.fg_override = bool((richtxt_bold_foreground >> 2) & 0x01);
        //node_properties.foreground  ((richtxt_bold_foreground >> 3) & 0xffffff)
        node_properties.ts_creation = sqlite3_column_int64(p_stmt, 5);
        node_properties.ts_lastsave = sqlite3_column_int64(p_stmt, 6);
    }
    else
    {
        std::cerr << "!! missing node properties for id " << node_id << std::endl;
    }
    sqlite3_finalize(p_stmt);
    return node_properties;
}


Gtk::TreeIter CherryTreeSQLiteRead::_sqlite3_node_process(gint64 node_id, Gtk::TreeIter parent_iter)
{
    t_node_properties node_properties = _sqlite3_get_node_properties(node_id);
    Gtk::TreeIter new_iter;

    std::cout << node_properties.name << std::endl;

    return new_iter;
}


int main(int argc, char *argv[])
{
    std::locale::global(std::locale("")); // Set the global C++ locale to the user-specified locale
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " FILEPATH.CTB" << std::endl;
        return 1;
    }
    Glib::ustring filepath(argv[1]);
    assert(Glib::file_test(filepath, Glib::FILE_TEST_EXISTS));

    std::list<gint64> bookmarks;
    Glib::RefPtr<Gtk::TreeStore> r_treestore;
    Gtk::TreeIter parent_iter;

    CherryTreeSQLiteRead ct_sqlite_read(filepath, &bookmarks, r_treestore);
    ct_sqlite_read.tree_walk(parent_iter);
}
