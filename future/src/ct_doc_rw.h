/*
 * ct_doc_rw.h
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

#pragma once

#include <libxml++/libxml++.h>
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
    char           foreground_rgb24[8];
    gint64         ts_creation;
    gint64         ts_lastsave;
};


class CherryTreeDocRead
{
public:
    CherryTreeDocRead(std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    virtual ~CherryTreeDocRead();
    virtual void tree_walk(Gtk::TreeIter *p_parent_iter=nullptr)=0;
protected:
    std::list<gint64> *mp_bookmarks;
    Glib::RefPtr<Gtk::TreeStore>  mr_treestore;
};


class CherryTreeXMLRead : public CherryTreeDocRead, public xmlpp::DomParser
{
public:
    CherryTreeXMLRead(Glib::ustring& filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    virtual ~CherryTreeXMLRead();
    void tree_walk(Gtk::TreeIter *p_parent_iter=nullptr);
private:
    void _xml_tree_walk_iter(xmlpp::Element *p_node_element, Gtk::TreeIter *p_parent_iter);
    t_node_properties _xml_get_node_properties(xmlpp::Element *p_node_element);
    Gtk::TreeIter _xml_node_process(xmlpp::Element *p_node_element, Gtk::TreeIter *p_parent_iter);
};


class CherryTreeSQLiteRead : public CherryTreeDocRead
{
public:
    CherryTreeSQLiteRead(Glib::ustring &filepath, std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore);
    virtual ~CherryTreeSQLiteRead();
    void tree_walk(Gtk::TreeIter *p_parent_iter=nullptr);
private:
    sqlite3 *mp_db;
    std::list<gint64> _sqlite3_get_children_node_id_from_father_id(gint64 father_id);
    void _sqlite3_tree_walk_iter(gint64 node_id, Gtk::TreeIter *p_parent_iter);
    t_node_properties _sqlite3_get_node_properties(gint64 node_id);
    Gtk::TreeIter _sqlite3_node_process(gint64 node_id, Gtk::TreeIter *p_parent_iter);
};
