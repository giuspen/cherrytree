/*
 * ct_treestore.h
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include <set>

class CtAnchoredWidget;

struct CtNodeData
{
    gint64         nodeId{0};
    Glib::ustring  name;
    std::string    syntax;
    Glib::ustring  tags;
    bool           isRO{false};
    guint32        customIconId{0};
    bool           isBold{false};
    std::string    foregroundRgb24;
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
    Gtk::TreeModelColumn<std::string>                colForeground;
    Gtk::TreeModelColumn<gint64>                     colTsCreation;
    Gtk::TreeModelColumn<gint64>                     colTsLastSave;
    Gtk::TreeModelColumn<std::list<CtAnchoredWidget*>> colAnchoredWidgets;
};

class CtTreeIter : public Gtk::TreeIter
{
public:
    CtTreeIter(Gtk::TreeIter iter, const CtTreeModelColumns* _columns);

    CtTreeIter  parent();

    bool          get_node_read_only() const;
    void          set_node_read_only(bool val);
    gint64        get_node_id() const;
    Glib::ustring get_node_name() const;
    void          set_node_name(const Glib::ustring& node_name);
    Glib::ustring get_node_tags() const;
    std::string   get_node_foreground() const;
    std::string   get_node_syntax_highlighting() const;
    std::time_t   get_node_creating_time() const;
    std::time_t   get_node_modification_time() const;
    void          set_node_aux_icon(Glib::RefPtr<Gdk::Pixbuf> rPixbuf);

    Glib::RefPtr<Gsv::Buffer> get_node_text_buffer() const;

private:
    const CtTreeModelColumns* _columns;
};

class CtTextView;
class CtSQLiteRead;

class CtTreeStore : public sigc::trackable
{
friend class CtTreeIter;

public:
    CtTreeStore();
    virtual ~CtTreeStore();

    void          viewConnect(Gtk::TreeView* pTreeView);
    void          viewAppendColumns(Gtk::TreeView* pTreeView);
    bool          readNodesFromFilepath(const char* filepath, const bool isImport, const Gtk::TreeIter* pParentIter=nullptr);
    void          getNodeData(Gtk::TreeIter treeIter, CtNodeData& nodeData);
    void          updateNodeData(Gtk::TreeIter treeIter, const CtNodeData& nodeData);
    void          updateNodeAuxIcon(Gtk::TreeIter treeIter);
    Gtk::TreeIter appendNode(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter=nullptr);
    Gtk::TreeIter insertNode(CtNodeData* pNodeData, const Gtk::TreeIter& afterIter);

    bool          onRequestAddBookmark(gint64 nodeId);
    bool          onRequestRemoveBookmark(gint64 nodeId);
    Gtk::TreeIter onRequestAppendNode(CtNodeData* pNodeData, const Gtk::TreeIter* pParentIter);

    void applyTextBufferToCtTextView(const Gtk::TreeIter& treeIter, CtTextView* pTextView);
    const Gtk::TreeModel::Children getRootChildren() { return _rTreeStore->children(); }
    void expandToTreeRow(Gtk::TreeView* pTreeView, Gtk::TreeRow& row);
    void setTreePathTextCursorFromConfig(Gtk::TreeView* pTreeView, Gsv::View* pTextView);
    void treeviewSafeSetCursor(Gtk::TreeView* pTreeView, Gtk::TreeIter& iter);

    gint64                       node_id_get();
    void                         add_used_tags(const std::string& tags);
    const std::set<std::string>& get_used_tags() { return _usedTags; }
    bool                         is_node_bookmarked(const gint64& node_id);
    std::string                  get_node_name_from_node_id(const gint64& node_id);
    CtTreeIter                   get_tree_iter_from_node_id(const gint64& node_id);
    const std::list<gint64>&     get_bookmarks();
    void                         set_bookmarks(const std::list<gint64>& bookmarks);


    std::string get_tree_expanded_collapsed_string(Gtk::TreeView& treeView);
    void        set_tree_expanded_collapsed_string(const std::string& expanded_collapsed_string, Gtk::TreeView& treeView, bool nodes_bookm_exp);

    Glib::RefPtr<Gtk::TreeStore>    get_store();
    Gtk::TreeIter                   get_iter_first();
    Gtk::TreeIter                   get_tree_iter_last_sibling(const Gtk::TreeNodeChildren& children);
    Gtk::TreeIter                   get_tree_iter_prev_sibling(Gtk::TreeIter tree_iter);
    Gtk::TreePath                   get_path(Gtk::TreeIter tree_iter);
    CtTreeIter                      to_ct_tree_iter(Gtk::TreeIter tree_iter);

    void nodes_sequences_fix(Gtk::TreeIter father_iter, bool process_children) { /* todo: */ }
    CtSQLiteRead* ctdb_handler() { return _pCtSQLiteRead; }
    const CtTreeModelColumns& get_columns() { return _columns; }

protected:
    guint16                   _getPangoWeight(bool isBold);
    bool                      _getBold(guint16 pangoWeight);
    Glib::RefPtr<Gdk::Pixbuf> _getNodeIcon(int nodeDepth, const std::string &syntax, guint32 customIconId);
    void                      _iterDeleteAnchoredWidgets(const Gtk::TreeModel::Children& children);

    Glib::RefPtr<Gsv::Buffer> _getNodeTextBuffer(const Gtk::TreeIter& treeIter);

    CtTreeModelColumns             _columns;
    Glib::RefPtr<Gtk::TreeStore>   _rTreeStore;
    std::list<gint64>              _bookmarks;
    std::set<std::string>          _usedTags;
    std::map<gint64, std::string>  _nodes_names_dict; // for link tooltips
    CtSQLiteRead*                  _pCtSQLiteRead{nullptr};
};
