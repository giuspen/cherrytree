/*
 * main_win.h
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
#include "treestore.h"
#include "ct_misc_utils.h"


class TheTreeView : public Gtk::TreeView
{
public:
    TheTreeView();
    virtual ~TheTreeView();
};


class TheTextView : public Gsv::View
{
public:
    TheTextView();
    virtual ~TheTextView();
};


class CTDialogTextEntry : public Gtk::Dialog
{
public:
    CTDialogTextEntry(const char *title, const bool forPassword, Gtk::Window* pParent);
    virtual ~CTDialogTextEntry();
    Glib::ustring getEntryText();

protected:
    bool on_entry_key_press_event(GdkEventKey *event_key);
    void on_entry_icon_press(Gtk::EntryIconPosition icon_position, const GdkEventButton* event);
    Gtk::Entry _entry;
};


class MainWindow : public Gtk::ApplicationWindow
{
public:
    MainWindow();
    virtual ~MainWindow();

    bool readNodesFromGioFile(const Glib::RefPtr<Gio::File>& r_file);
    void configApply();

protected:
    void                _on_theTreeview_signal_cursor_changed();
    void                _titleUpdate(bool saveNeeded);
    Gtk::VBox           _vboxMain;
    Gtk::VBox           _vboxText;
    Gtk::HPaned         _hPaned;
    Gtk::ScrolledWindow _scrolledwindowTree;
    Gtk::ScrolledWindow _scrolledwindowText;
    TheTreeStore        _theTreestore;
    TheTreeView         _theTreeview;
    TheTextView         _theTextview;
    std::string         _currFileName;
    std::string         _currFileDir;
};
