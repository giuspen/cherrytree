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
// g++ sqlite3.cc -o sqlite3 `pkg-config sqlite3 libxml++-2.6 gtkmm-3.0 --cflags --libs` -Wno-deprecated

#include <assert.h>
#include <iostream>
#include <sqlite3.h>
#include <libxml++/libxml++.h>
#include <gtkmm.h>


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
    Gtk::TreeIter _sqlite3_node_process(xmlpp::Element* p_node_element, Gtk::TreeIter parent_iter);
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

    std::list<gint64> children_node_id = _sqlite3_get_children_node_id_from_father_id(0);
    for(gint64 node_id : children_node_id)
    {
        
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


Gtk::TreeIter CherryTreeSQLiteRead::_sqlite3_node_process(xmlpp::Element *p_node_element, Gtk::TreeIter parent_iter)
{
    Gtk::TreeIter new_iter;

    

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
