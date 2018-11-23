/*
 * ct_treestore.h
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

class CtAnchoredWidget
{
public:
    void insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer, const int& charOffset, const Glib::ustring& justification);
    Glib::RefPtr<Gtk::TextChildAnchor> getTextChildAnchor() { return _rTextChildAnchor; }
protected:
    Glib::RefPtr<Gtk::TextChildAnchor> _rTextChildAnchor;
};

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
    std::list<CtAnchoredWidget*> anchoredWidgets;
};

class CtTreeModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    CtTreeModelColumns()
    {
        add(rColPixbuf); add(colNodeName); add(rColTextBuffer); add(colNodeUniqueId);
        add(colSyntaxHighlighting); add(colNodeSequence); add(colNodeTags); add(colNodeRO);
        add(rColPixbufAux); add(colCustomIconId); add(colWeight); add(colForeground);
        add(colTsCreation); add(colTsLastSave); add(colAnchoredWidgets);
    }
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>  rColPixbuf;
    Gtk::TreeModelColumn<Glib::ustring>              colNodeName;
    Gtk::TreeModelColumn<Glib::RefPtr<Gsv::Buffer>>  rColTextBuffer;
    Gtk::TreeModelColumn<gint64>                     colNodeUniqueId;
    Gtk::TreeModelColumn<std::string>                colSyntaxHighlighting;
    Gtk::TreeModelColumn<guint16>                    colNodeSequence;
    Gtk::TreeModelColumn<Glib::ustring>              colNodeTags;
    Gtk::TreeModelColumn<bool>                       colNodeRO;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>  rColPixbufAux;
    Gtk::TreeModelColumn<guint16>                    colCustomIconId;
    Gtk::TreeModelColumn<guint16>                    colWeight;
    Gtk::TreeModelColumn<Glib::ustring>              colForeground;
    Gtk::TreeModelColumn<gint64>                     colTsCreation;
    Gtk::TreeModelColumn<gint64>                     colTsLastSave;
    Gtk::TreeModelColumn<std::list<CtAnchoredWidget*>> colAnchoredWidgets;
};

class CtTreeStore : public sigc::trackable
{
public:
    CtTreeStore();
    virtual ~CtTreeStore();

    void          viewConnect(Gtk::TreeView* pTreeView);
    void          viewAppendColumns(Gtk::TreeView* pTreeView);
    bool          readNodesFromFilepath(const char* filepath, const Gtk::TreeIter* pParentIter=nullptr);
    Gtk::TreeIter appendNode(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter=nullptr);
    void          onRequestAddBookmark(gint64 nodeId);
    Gtk::TreeIter onRequestAppendNode(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter);

    Glib::ustring getNodeName(Gtk::TreeIter treeIter);
    std::string getNodeSyntaxHighlighting(Gtk::TreeIter treeIter);
    Glib::RefPtr<Gsv::Buffer> getNodeTextBuffer(Gtk::TreeIter treeIter);

protected:
    guint16                   _getPangoWeight(bool isBold);
    Glib::RefPtr<Gdk::Pixbuf> _getNodeIcon(int nodeDepth, std::string &syntax, guint32 customIconId);
    void                      _iterDeleteAnchoredWidgets(const Gtk::TreeModel::Children& children);

    CtTreeModelColumns           _columns;
    Glib::RefPtr<Gtk::TreeStore> _rTreeStore;
    std::list<gint64>            _bookmarks;
};
