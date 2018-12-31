/*
 * ct_main_win.h
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
#include "ct_treestore.h"
#include "ct_misc_utils.h"

class CtTreeView : public Gtk::TreeView
{
public:
    CtTreeView();
    virtual ~CtTreeView();
    void setExpandedCollapsed(CtTreeStore& ctTreestore);
protected:
};

class CtTextView : public Gsv::View
{
public:
    CtTextView();
    virtual ~CtTextView();
    void setupForSyntax(const std::string& syntaxHighlighting);
protected:
    void _setFontForSyntax(const std::string& syntaxHighlighting);
};

class CtDialogTextEntry : public Gtk::Dialog
{
public:
    CtDialogTextEntry(const char* title, const bool forPassword, Gtk::Window* pParent);
    virtual ~CtDialogTextEntry();
    Glib::ustring getEntryText();

protected:
    bool _onEntryKeyPress(GdkEventKey* eventKey);
    void _onEntryIconPress(Gtk::EntryIconPosition iconPosition, const GdkEventButton* event);
    Gtk::Entry _entry;
};

class CtMainWin : public Gtk::ApplicationWindow
{
public:
    CtMainWin();
    virtual ~CtMainWin();

    bool readNodesFromGioFile(const Glib::RefPtr<Gio::File>& r_file, const bool isImport);
    void configApply();

protected:
    void                _onTheTreeviewSignalCursorChanged();
    void                _titleUpdate(bool saveNeeded);
    Gtk::VBox           _vboxMain;
    Gtk::VBox           _vboxText;
    Gtk::HPaned         _hPaned;
    Gtk::ScrolledWindow _scrolledwindowTree;
    Gtk::ScrolledWindow _scrolledwindowText;
    CtTreeStore         _ctTreestore;
    CtTreeView          _ctTreeview;
    CtTextView          _ctTextview;
    std::string         _currFileName;
    std::string         _currFileDir;
};
