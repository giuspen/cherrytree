/*
 * treestore.h
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
        add(rColPixbuf); add(colNodeName); add(rColTextBuffer); add(colNodeUniqueId);
        add(colSyntaxHighlighting); add(colNodeSequence); add(colNodeTags); add(colNodeRO);
        add(rColPixbufAux); add(colCustomIconId); add(colWeight); add(colForeground);
        add(colTsCreation); add(colTsLastSave);
    }
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>     rColPixbuf;
    Gtk::TreeModelColumn<Glib::ustring>                 colNodeName;
    Gtk::TreeModelColumn<Glib::RefPtr<Gtk::TextBuffer>> rColTextBuffer;
    Gtk::TreeModelColumn<gint64>                        colNodeUniqueId;
    Gtk::TreeModelColumn<Glib::ustring>                 colSyntaxHighlighting;
    Gtk::TreeModelColumn<guint16>                       colNodeSequence;
    Gtk::TreeModelColumn<Glib::ustring>                 colNodeTags;
    Gtk::TreeModelColumn<bool>                          colNodeRO;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>     rColPixbufAux;
    Gtk::TreeModelColumn<guint16>                       colCustomIconId;
    Gtk::TreeModelColumn<guint16>                       colWeight;
    Gtk::TreeModelColumn<Glib::ustring>                 colForeground;
    Gtk::TreeModelColumn<gint64>                        colTsCreation;
    Gtk::TreeModelColumn<gint64>                        colTsLastSave;
};


class TheTreeStore : public sigc::trackable
{
public:
    TheTreeStore();
    virtual ~TheTreeStore();

    void          view_connect(Gtk::TreeView *pTreeView);
    void          view_append_columns(Gtk::TreeView *pTreeView);
    bool          readNodesFromFilepath(const char* filepath, const Gtk::TreeIter *pParentIter=nullptr);
    Gtk::TreeIter append_node(t_ct_node_data *p_node_data, const Gtk::TreeIter *p_parent_iter=nullptr);
    void          on_request_add_bookmark(gint64 node_id);
    Gtk::TreeIter on_request_append_node(t_ct_node_data *p_node_data, const Gtk::TreeIter *p_parent_iter);

    Glib::ustring getNodeName(Gtk::TreeIter treeIter);

protected:
    guint16                   _get_pango_weight(bool is_bold);
    Glib::RefPtr<Gdk::Pixbuf> _get_node_icon(int node_depth, Glib::ustring &syntax, guint32 custom_icon_id);

    TheTreeModelColumns          _columns;
    Glib::RefPtr<Gtk::TreeStore> _rTreeStore;
    std::list<gint64>            _bookmarks;
};
