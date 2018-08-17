/*
 * ct_doc_rw.h
 *
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
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
#include "treestore.h"


class CherryTreeDocRead
{
public:
    CherryTreeDocRead() {};
    virtual ~CherryTreeDocRead() {};
    virtual void tree_walk(const Gtk::TreeIter *p_parent_iter=nullptr)=0;
    sigc::signal<void, gint64> m_signal_add_bookmark;
    sigc::signal<Gtk::TreeIter, t_ct_node_data*, const Gtk::TreeIter*> m_signal_append_node;
};


class CherryTreeXMLRead : public CherryTreeDocRead, public xmlpp::DomParser
{
public:
    CherryTreeXMLRead(const char* filepath);
    virtual ~CherryTreeXMLRead();
    void tree_walk(const Gtk::TreeIter *p_parent_iter=nullptr);
private:
    void _xml_tree_walk_iter(xmlpp::Element *p_node_element, const Gtk::TreeIter *p_parent_iter);
    t_ct_node_data _xml_get_node_properties(xmlpp::Element *p_node_element);
    Gtk::TreeIter _xml_node_process(xmlpp::Element *p_node_element, const Gtk::TreeIter *p_parent_iter);
};


class CherryTreeSQLiteRead : public CherryTreeDocRead
{
public:
    CherryTreeSQLiteRead(const char* filepath);
    virtual ~CherryTreeSQLiteRead();
    void tree_walk(const Gtk::TreeIter *p_parent_iter=nullptr);
private:
    sqlite3 *mp_db;
    std::list<gint64> _sqlite3_get_children_node_id_from_father_id(gint64 father_id);
    void _sqlite3_tree_walk_iter(gint64 node_id, const Gtk::TreeIter *p_parent_iter);
    t_ct_node_data _sqlite3_get_node_properties(gint64 node_id);
    Gtk::TreeIter _sqlite3_node_process(gint64 node_id, const Gtk::TreeIter *p_parent_iter);
};
