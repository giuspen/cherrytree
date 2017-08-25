/*
 * treestore.cc
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

#include <glibmm/i18n.h>
#include <iostream>
#include "treestore.h"
#include "ct_doc_rw.h"


TheTree::TheTree()
{
    mr_treestore = Gtk::TreeStore::create(m_columns);
}


TheTree::~TheTree()
{
}


bool TheTree::read_nodes_from_filepath(Glib::ustring &filepath, Gtk::TreeIter *p_parent_iter)
{
    bool ret_ok = false;
    CherryTreeDocRead *p_ct_doc_read = nullptr;
    if(Glib::str_has_suffix(filepath, "ctd"))
    {
        p_ct_doc_read = new CherryTreeXMLRead(filepath, &m_bookmarks, mr_treestore);
    }
    else if(Glib::str_has_suffix(filepath, "ctb"))
    {
        p_ct_doc_read = new CherryTreeSQLiteRead(filepath, &m_bookmarks, mr_treestore);
    }
    if(p_ct_doc_read != nullptr)
    {
        p_ct_doc_read->tree_walk(p_parent_iter);
        delete p_ct_doc_read;
        ret_ok = true;
    }
    return ret_ok;
}
