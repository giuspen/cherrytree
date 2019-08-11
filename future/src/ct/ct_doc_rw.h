/*
 * ct_doc_rw.h
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

#include <libxml++/libxml++.h>
#include <sqlite3.h>
#include <gtkmm.h>
#include "ct_treestore.h"
#include "ct_table.h"

enum class CtXmlNodeType { None, RichText, EncodedPng, Table, CodeBox };
enum class CtExporting { No, All, NodeOnly, NodeAndSubnodes };

class CtDocRead
{
public:
    virtual bool read_populate_tree(const Gtk::TreeIter* pParentIter=nullptr)=0;
    sigc::signal<bool, gint64> signalAddBookmark;
    sigc::signal<Gtk::TreeIter, CtNodeData*, const Gtk::TreeIter*> signalAppendNode;
};

class CtXmlRead : public CtDocRead, public xmlpp::DomParser
{
public:
    CtXmlRead(const char* filepath, const char* textContent);
    virtual ~CtXmlRead();
    bool read_populate_tree(const Gtk::TreeIter* pParentIter=nullptr) override;
    Glib::RefPtr<Gsv::Buffer> getTextBuffer(const std::string& syntax,
                                            std::list<CtAnchoredWidget*>& anchoredWidgets,
                                            xmlpp::Element* pNodeElement=nullptr);

private:
    void _xmlTreeWalkIter(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter);
    Gtk::TreeIter _xmlNodeProcess(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter);

private:
    static CtXmlNodeType _xmlNodeGetTypeFromName(const Glib::ustring& xmlNodeName);

public:
    static void getTextBufferIter(Glib::RefPtr<Gsv::Buffer>& rTextBuffer, Gtk::TextIter* insertIter,
                                  std::list<CtAnchoredWidget*>& anchoredWidgets,
                                  xmlpp::Node *pNodeParent, int forceCharOffset = -1);
    static bool populateTableMatrixGetIsHeadFront(CtTableMatrix& tableMatrix, xmlpp::Element* pNodeElement);
};

class CtXmlWrite : public xmlpp::Document
{
public:
    CtXmlWrite(const char* root_name);
    virtual ~CtXmlWrite();
    void treestore_to_dom(const std::list<gint64>& bookmarks, CtTreeIter ct_tree_iter);
    void append_bookmarks(const std::list<gint64>& bookmarks);
    void append_dom_node(CtTreeIter& ct_tree_iter,
                         xmlpp::Element* p_node_parent=nullptr,
                         bool to_disk=true,
                         bool skip_children=true,
                         const std::pair<int,int>& offset_range=std::make_pair(-1,-1));
    void append_node_buffer(CtTreeIter& ct_tree_iter,
                            xmlpp::Element* p_node_node,
                            bool serialise_anchored_widgets,
                            const std::pair<int,int>& offset_range=std::make_pair(-1,-1));

public:
    static void rich_txt_serialize(xmlpp::Element* p_node_parent,
                                   Gtk::TextIter start_iter,
                                   Gtk::TextIter end_iter,
                                   std::map<const gchar*, std::string>& curr_attributes,
                                   gchar change_case='n');
};

class CtSQLite : public CtDocRead
{
public:
    CtSQLite(const char* filepath);
    virtual ~CtSQLite();
    bool getDbOpenOk() { return _dbOpenOk; }

    bool read_populate_tree(const Gtk::TreeIter* pParentIter=nullptr) override;
    bool write_db_full(const std::list<gint64>& bookmarks,
                       CtTreeIter ct_tree_iter,
                       const CtExporting exporting=CtExporting::No,
                       const std::pair<int,int>& offset_range=std::make_pair(-1,-1));

    Glib::RefPtr<Gsv::Buffer> getTextBuffer(const std::string& syntax,
                                            std::list<CtAnchoredWidget*>& anchoredWidgets,
                                            const gint64& nodeId) const;
    void pending_edit_db_bookmarks();
    void pending_edit_db_node_prop(const gint64 node_id);
    void pending_edit_db_node_buff(const gint64 node_id);
    void pending_edit_db_node_hier(const gint64 node_id);
    void pending_new_db_node(const gint64 node_id);

    struct CtNodeWriteDict
    {
        bool upd{false};
        bool prop{false};
        bool buff{false};
        bool hier{false};
        bool child{false};
    };
    struct CtSyncPending
    {
        std::unordered_map<gint64,CtNodeWriteDict> nodes_to_write_dict;
        std::set<gint64> nodes_to_rm_set;
        bool bookmarks_to_write{false};
    };

    static const char TABLE_NODE_CREATE[];
    static const char TABLE_NODE_INSERT[];
    static const char TABLE_NODE_DELETE[];
    static const char TABLE_CODEBOX_CREATE[];
    static const char TABLE_CODEBOX_INSERT[];
    static const char TABLE_CODEBOX_DELETE[];
    static const char TABLE_TABLE_CREATE[];
    static const char TABLE_TABLE_INSERT[];
    static const char TABLE_TABLE_DELETE[];
    static const char TABLE_IMAGE_CREATE[];
    static const char TABLE_IMAGE_INSERT[];
    static const char TABLE_IMAGE_DELETE[];
    static const char TABLE_CHILDREN_CREATE[];
    static const char TABLE_CHILDREN_INSERT[];
    static const char TABLE_CHILDREN_DELETE[];
    static const char TABLE_BOOKMARK_CREATE[];
    static const char TABLE_BOOKMARK_INSERT[];
    static const char TABLE_BOOKMARK_DELETE[];
    static const char ERR_SQLITE_PREPV2[];
    static const char ERR_SQLITE_STEP[];

protected:
    bool _get_children_node_ids_from_father_id(gint64 father_id, std::list<gint64>& ret_children);
    bool _sqlite3TreeWalkIter(gint64 nodeId, const Gtk::TreeIter* pParentIter);
    bool _sqlite3GetNodeProperties(gint64 nodeId, CtNodeData& nodeData);
    bool _sqlite3NodeProcess(gint64 nodeId, const Gtk::TreeIter* pParentIter, Gtk::TreeIter& newIter);
    void _getTextBufferAnchoredWidgets(Glib::RefPtr<Gsv::Buffer>& rTextBuffer,
                                       std::list<CtAnchoredWidget*>& anchoredWidgets,
                                       const gint64& nodeId,
                                       const bool& has_codebox,
                                       const bool& has_table,
                                       const bool& has_image) const;
    bool _exec_no_callback(const char* sqlCmd);
    bool _exec_bind_int64(const char* sqlCmd, const gint64 bind_int64);
    bool _remove_db_node_n_children(const gint64 node_id);
    bool _create_all_tables();
    bool _write_db_node(CtTreeIter ct_tree_iter,
                        const gint64 sequence,
                        const gint64 node_father_id,
                        const CtNodeWriteDict write_dict,
                        const CtExporting exporting=CtExporting::No,
                        const std::pair<int,int>& offset_range=std::make_pair(-1,-1));
    bool _write_db_bookmarks(const std::list<gint64>& bookmarks);

    sqlite3* _pDb{nullptr};
    bool     _dbOpenOk{false};
    CtSyncPending _syncPending;
};
