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

class CtDocRead
{
public:
    CtDocRead() {};
    virtual ~CtDocRead() {};
    virtual void treeWalk(const Gtk::TreeIter* pParentIter=nullptr)=0;
    sigc::signal<bool, gint64> signalAddBookmark;
    sigc::signal<Gtk::TreeIter, CtNodeData*, const Gtk::TreeIter*> signalAppendNode;
};

enum class CtXmlNodeType { None, RichText, EncodedPng, Table, CodeBox };

class CtXmlRead : public CtDocRead, public xmlpp::DomParser
{
public:
    CtXmlRead(const char* filepath, const char* textContent);
    virtual ~CtXmlRead();
    virtual void treeWalk(const Gtk::TreeIter* pParentIter=nullptr);
    Glib::RefPtr<Gsv::Buffer> getTextBuffer(const std::string& syntax,
                                            std::list<CtAnchoredWidget*>& anchoredWidgets,
                                            xmlpp::Element* pNodeElement=nullptr);
    void populateTableMatrix(CtTableMatrix& tableMatrix, xmlpp::Element* pNodeElement=nullptr);

private:
    void _xmlTreeWalkIter(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter);
    Gtk::TreeIter _xmlNodeProcess(xmlpp::Element* pNodeElement, const Gtk::TreeIter* pParentIter);
    CtXmlNodeType _xmlNodeGetTypeFromName(const Glib::ustring& xmlNodeName);
    void _getTextBufferIter(Glib::RefPtr<Gsv::Buffer>& rTextBuffer,
                            std::list<CtAnchoredWidget*>& anchoredWidgets,
                            xmlpp::Node *pNodeParent);
};

class CtXmlWrite : public xmlpp::Document
{
public:
    CtXmlWrite(const char* filepath);
    virtual ~CtXmlWrite();
    void append_bookmarks(const std::list<gint64>& bookmarks);
    void append_dom_node(xmlpp::Element* p_node_parent=nullptr);
};

class CtSQLiteRead : public CtDocRead
{
public:
    CtSQLiteRead(const char* filepath);
    virtual ~CtSQLiteRead();
    virtual void treeWalk(const Gtk::TreeIter* pParentIter=nullptr);
    Glib::RefPtr<Gsv::Buffer> getTextBuffer(const std::string& syntax,
                                            std::list<CtAnchoredWidget*>& anchoredWidgets,
                                            const gint64& nodeId);
    void pending_new_db_node(gint64 node_id) { /* todo: */ }

private:
    sqlite3* _pDb;
    std::list<gint64> _sqlite3GetChildrenNodeIdFromFatherId(gint64 father_id);
    void _sqlite3TreeWalkIter(gint64 nodeId, const Gtk::TreeIter* pParentIter);
    CtNodeData _sqlite3GetNodeProperties(gint64 nodeId);
    Gtk::TreeIter _sqlite3NodeProcess(gint64 nodeId, const Gtk::TreeIter* pParentIter);
    void _getTextBufferAnchoredWidgets(Glib::RefPtr<Gsv::Buffer>& rTextBuffer,
                                       std::list<CtAnchoredWidget*>& anchoredWidgets,
                                       const gint64& nodeId,
                                       const bool& has_codebox,
                                       const bool& has_table,
                                       const bool& has_image);
};
