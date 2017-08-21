/*
 * the_tree.h
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

#include <gtkmm.h>


class TheTree : public Gtk::TreeView
{
public:
    TheTree();
    virtual ~TheTree();
    bool read_nodes_from_filepath(Glib::ustring &filepath, Gtk::TreeIter *p_parent_iter=nullptr);
protected:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(m_col_icon_stock_id); add(m_col_node_name); add(m_col_text_buffer); add(m_col_node_unique_id);
            add(m_col_syntax_highlighting); add(m_col_node_sequence);
        }
        Gtk::TreeModelColumn<Glib::ustring>                 m_col_icon_stock_id;
        Gtk::TreeModelColumn<Glib::ustring>                 m_col_node_name;
        Gtk::TreeModelColumn<Glib::RefPtr<Gtk::TextBuffer>> m_col_text_buffer;
        Gtk::TreeModelColumn<gint64>                        m_col_node_unique_id;
        Gtk::TreeModelColumn<Glib::ustring>                 m_col_syntax_highlighting;
        Gtk::TreeModelColumn<guint16>                       m_col_node_sequence;
        Gtk::TreeModelColumn<Glib::ustring>                 m_col_node_tags;
        Gtk::TreeModelColumn<bool>                          m_col_node_ro;
        Gtk::TreeModelColumn<Glib::ustring>                 m_col_aux_icon_stock_id;
        Gtk::TreeModelColumn<guint16>                       m_col_custom_icon_id;
        Gtk::TreeModelColumn<guint16>                       m_col_weight;
        Gtk::TreeModelColumn<Glib::ustring>                 m_col_foreground;
        Gtk::TreeModelColumn<gint64>                        m_col_ts_creation;
        Gtk::TreeModelColumn<gint64>                        m_col_ts_lastsave;
    };
    ModelColumns m_columns;

    Glib::RefPtr<Gtk::TreeStore> mr_treestore;
    std::list<gint64>            m_bookmarks;
};
