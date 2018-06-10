/*
 * treestore.cc
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

#include <glibmm/i18n.h>
#include <iostream>
#include <set>
#include "treestore.h"
#include "ct_doc_rw.h"
#include "ct_app.h"


TheTreeStore::TheTreeStore()
{
    mr_treestore = Gtk::TreeStore::create(m_columns);
}


TheTreeStore::~TheTreeStore()
{
}


void TheTreeStore::view_connect(Gtk::TreeView *p_treeview)
{
    p_treeview->set_model(mr_treestore);
}


void TheTreeStore::view_append_columns(Gtk::TreeView *p_treeview)
{
    Gtk::TreeView::Column* p_column = Gtk::manage(new Gtk::TreeView::Column(""));
    p_column->pack_start(m_columns.mr_col_pixbuf, /*expand=*/false);
    p_column->pack_start(m_columns.m_col_node_name);
    p_column->pack_start(m_columns.mr_col_pixbuf_aux, /*expand=*/false);
    p_treeview->append_column(*p_column);
}


bool TheTreeStore::read_nodes_from_filepath(Glib::ustring &filepath, Gtk::TreeIter *p_parent_iter)
{
    bool ret_ok = false;
    CherryTreeDocRead *p_ct_doc_read = nullptr;
    if (Glib::str_has_suffix(filepath, "ctd"))
    {
        p_ct_doc_read = new CherryTreeXMLRead(filepath);
    }
    else if (Glib::str_has_suffix(filepath, "ctb"))
    {
        p_ct_doc_read = new CherryTreeSQLiteRead(filepath);
    }
    if (p_ct_doc_read != nullptr)
    {
        p_ct_doc_read->m_signal_add_bookmark.connect(sigc::mem_fun(this, &TheTreeStore::on_request_add_bookmark));
        p_ct_doc_read->m_signal_append_node.connect(sigc::mem_fun(this, &TheTreeStore::on_request_append_node));
        p_ct_doc_read->tree_walk(p_parent_iter);
        delete p_ct_doc_read;
        ret_ok = true;
    }
    return ret_ok;
}


Glib::RefPtr<Gdk::Pixbuf> TheTreeStore::_get_node_icon(int node_depth, Glib::ustring &syntax, guint32 custom_icon_id)
{
    Glib::RefPtr<Gdk::Pixbuf> r_pixbuf;

    if (custom_icon_id != 0)
    {
        // custom_icon_id
        r_pixbuf = R_icontheme->load_icon(NODES_STOCKS.at(custom_icon_id), NODE_ICON_SIZE);
    }
    else if (NODE_ICON_TYPE_NONE == P_ct_config->m_nodes_icons)
    {
        // NODE_ICON_TYPE_NONE
        r_pixbuf = R_icontheme->load_icon(NODES_STOCKS.at(NODE_ICON_NO_ICON_ID), NODE_ICON_SIZE);
    }
    else if (1 == std::set<Glib::ustring>({RICH_TEXT_ID, PLAIN_TEXT_ID}).count(syntax))
    {
        // text node
        if (NODE_ICON_TYPE_CHERRY == P_ct_config->m_nodes_icons)
        {
            if (1 == NODES_ICONS.count(node_depth))
            {
                r_pixbuf = R_icontheme->load_icon(NODES_ICONS.at(node_depth), NODE_ICON_SIZE);
            }
            else
            {
                r_pixbuf = R_icontheme->load_icon(NODES_ICONS.at(-1), NODE_ICON_SIZE);
            }
        }
        else
        {
            // NODE_ICON_TYPE_CUSTOM
            r_pixbuf = R_icontheme->load_icon(NODES_STOCKS.at(P_ct_config->m_default_icon_text), NODE_ICON_SIZE);
        }
    }
    else
    {
        // code node
        r_pixbuf = R_icontheme->load_icon(get_stock_id_for_code_type(syntax), NODE_ICON_SIZE);
    }
    return r_pixbuf;
}


Gtk::TreeIter TheTreeStore::append_node(t_ct_node_data *p_node_data, Gtk::TreeIter *p_parent_iter)
{
    Gtk::TreeIter new_iter;
    //std::cout << p_node_data->name << std::endl;

    if (p_parent_iter == nullptr)
    {
        new_iter = mr_treestore->append();
    }
    else
    {
        new_iter = mr_treestore->append(static_cast<Gtk::TreeRow>(**p_parent_iter).children());
    }
    Gtk::TreeRow row = *new_iter;
    row[m_columns.mr_col_pixbuf] = _get_node_icon(mr_treestore->iter_depth(new_iter), p_node_data->syntax, p_node_data->custom_icon_id);
    row[m_columns.m_col_node_name] = p_node_data->name;
    //row[m_columns.m_col_text_buffer] = ;
    row[m_columns.m_col_node_unique_id] = p_node_data->node_id;
    row[m_columns.m_col_syntax_highlighting] = p_node_data->syntax;
    //row[m_columns.m_col_node_sequence] = ;
    row[m_columns.m_col_node_tags] = p_node_data->tags;
    row[m_columns.m_col_node_ro] = p_node_data->is_ro;
    //row[m_columns.mr_col_pixbuf_aux] = ;
    row[m_columns.m_col_custom_icon_id] = p_node_data->custom_icon_id;
    row[m_columns.m_col_weight] = _get_pango_weight(p_node_data->is_bold);
    //row[m_columns.m_col_foreground] = ;
    row[m_columns.m_col_ts_creation] = p_node_data->ts_creation;
    row[m_columns.m_col_ts_lastsave] = p_node_data->ts_lastsave;
    return new_iter;
}


void TheTreeStore::on_request_add_bookmark(gint64 node_id)
{
    m_bookmarks.push_back(node_id);
}


guint16 TheTreeStore::_get_pango_weight(bool is_bold)
{
    return is_bold ? PANGO_WEIGHT_HEAVY : PANGO_WEIGHT_NORMAL;
}


Gtk::TreeIter TheTreeStore::on_request_append_node(t_ct_node_data *p_node_data, Gtk::TreeIter *p_parent_iter)
{
    return append_node(p_node_data, p_parent_iter);
}
