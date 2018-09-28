/*
 * ct_doc_rw.h
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

#include <libxml++/libxml++.h>
#include <sqlite3.h>
#include <gtkmm.h>
#include "treestore.h"


class CherryTreeDocRead
{
public:
    CherryTreeDocRead() {};
    virtual ~CherryTreeDocRead() {};
    virtual void treeWalk(const Gtk::TreeIter *pParentIter=nullptr)=0;
    sigc::signal<void, gint64> signalAddBookmark;
    sigc::signal<Gtk::TreeIter, CtNodeData*, const Gtk::TreeIter*> signalAppendNode;
};


class CherryTreeXMLRead : public CherryTreeDocRead, public xmlpp::DomParser
{
public:
    CherryTreeXMLRead(const char* filepath);
    virtual ~CherryTreeXMLRead();
    void treeWalk(const Gtk::TreeIter *pParentIter=nullptr);
private:
    void _xmlTreeWalkIter(xmlpp::Element *pNodeElement, const Gtk::TreeIter *pParentIter);
    CtNodeData _xmlGetNodeProperties(xmlpp::Element *pNodeElement);
    Gtk::TreeIter _xmlNodeProcess(xmlpp::Element *pNodeElement, const Gtk::TreeIter *pParentIter);
};


class CherryTreeSQLiteRead : public CherryTreeDocRead
{
public:
    CherryTreeSQLiteRead(const char* filepath);
    virtual ~CherryTreeSQLiteRead();
    void treeWalk(const Gtk::TreeIter *pParentIter=nullptr);
private:
    sqlite3 *mp_db;
    std::list<gint64> _sqlite3GetChildrenNodeIdFromFatherId(gint64 father_id);
    void _sqlite3TreeWalkIter(gint64 nodeId, const Gtk::TreeIter *pParentIter);
    CtNodeData _sqlite3GetNodeProperties(gint64 nodeId);
    Gtk::TreeIter _sqlite3NodeProcess(gint64 nodeId, const Gtk::TreeIter *pParentIter);
};
