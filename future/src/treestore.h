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
#include <gtksourceviewmm.h>


struct CtNodeData
{
    gint64         nodeId{0};
    Glib::ustring  name;
    std::string    syntax;
    Glib::ustring  tags;
    bool           isRO{false};
    guint32        customIconId{0};
    bool           isBold{false};
    bool           fgOverride{false};
    char           foregroundRgb24[8];
    gint64         tsCreation{0};
    gint64         tsLastSave{0};
    Glib::RefPtr<Gsv::Buffer>  rTextBuffer{nullptr};
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

    void          viewConnect(Gtk::TreeView *pTreeView);
    void          viewAppendColumns(Gtk::TreeView *pTreeView);
    bool          readNodesFromFilepath(const char* filepath, const Gtk::TreeIter *pParentIter=nullptr);
    Gtk::TreeIter appendNode(CtNodeData *pNodeData, const Gtk::TreeIter *pParentIter=nullptr);
    void          onRequestAddBookmark(gint64 nodeId);
    Gtk::TreeIter onRequestAppendNode(CtNodeData *pNodeData, const Gtk::TreeIter *pParentIter);

    Glib::ustring getNodeName(Gtk::TreeIter treeIter);

protected:
    guint16                   _getPangoWeight(bool isBold);
    Glib::RefPtr<Gdk::Pixbuf> _getNodeIcon(int nodeDepth, std::string &syntax, guint32 customIconId);

    TheTreeModelColumns          _columns;
    Glib::RefPtr<Gtk::TreeStore> _rTreeStore;
    std::list<gint64>            _bookmarks;
};
