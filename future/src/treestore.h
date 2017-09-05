/*
 * treestore.h
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


struct t_ct_node_data
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


class TheTreeModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    TheTreeModelColumns()
    {
        add(mr_col_pixbuf); add(m_col_node_name); add(m_col_text_buffer); add(m_col_node_unique_id);
        add(m_col_syntax_highlighting); add(m_col_node_sequence); add(m_col_node_tags); add(m_col_node_ro);
        add(mr_col_pixbuf_aux); add(m_col_custom_icon_id); add(m_col_weight); add(m_col_foreground);
        add(m_col_ts_creation); add(m_col_ts_lastsave);
    }
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>     mr_col_pixbuf;
    Gtk::TreeModelColumn<Glib::ustring>                 m_col_node_name;
    Gtk::TreeModelColumn<Glib::RefPtr<Gtk::TextBuffer>> m_col_text_buffer;
    Gtk::TreeModelColumn<gint64>                        m_col_node_unique_id;
    Gtk::TreeModelColumn<Glib::ustring>                 m_col_syntax_highlighting;
    Gtk::TreeModelColumn<guint16>                       m_col_node_sequence;
    Gtk::TreeModelColumn<Glib::ustring>                 m_col_node_tags;
    Gtk::TreeModelColumn<bool>                          m_col_node_ro;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>     mr_col_pixbuf_aux;
    Gtk::TreeModelColumn<guint16>                       m_col_custom_icon_id;
    Gtk::TreeModelColumn<guint16>                       m_col_weight;
    Gtk::TreeModelColumn<Glib::ustring>                 m_col_foreground;
    Gtk::TreeModelColumn<gint64>                        m_col_ts_creation;
    Gtk::TreeModelColumn<gint64>                        m_col_ts_lastsave;
};


class TheTreeStore : public sigc::trackable
{
public:
    TheTreeStore();
    virtual ~TheTreeStore();

    void view_connect(Gtk::TreeView *p_treeview);

    void view_append_columns(Gtk::TreeView *p_treeview);

    bool read_nodes_from_filepath(Glib::ustring &filepath, Gtk::TreeIter *p_parent_iter=nullptr);

    Gtk::TreeIter append_node(t_ct_node_data *p_node_data, Gtk::TreeIter *p_parent_iter=nullptr);

    void on_request_add_bookmark(gint64 node_id);

    Gtk::TreeIter on_request_append_node(t_ct_node_data *p_node_data, Gtk::TreeIter *p_parent_iter);

protected:
    guint16 _get_pango_weight(bool is_bold);

    TheTreeModelColumns          m_columns;
    Glib::RefPtr<Gtk::TreeStore> mr_treestore;
    std::list<gint64>            m_bookmarks;
};
