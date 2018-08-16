/*
 * main_win.cc
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

#include <glibmm/i18n.h>
#include <iostream>
#include "main_win.h"


TheTreeView::TheTreeView()
{
    set_headers_visible(false);
}

TheTreeView::~TheTreeView()
{
}


MainWindow::MainWindow() : Gtk::ApplicationWindow()
{
    _scrolledwindowTree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(_scrolledwindowTree);
    _theTreestore.view_append_columns(&_theTreeview);
    _theTreestore.view_connect(&_theTreeview);
    _scrolledwindowTree.add(_theTreeview);
    set_size_request(300, 400);
    show_all();
}

MainWindow::~MainWindow()
{
}

bool MainWindow::readNodesFromGioFile(const Glib::RefPtr<Gio::File>& r_file)
{
    std::string filepath{r_file->get_path()};
    if ( (Glib::str_has_suffix(filepath, ".ctz")) ||
         (Glib::str_has_suffix(filepath, ".ctx")) )
    {
        gchar* title = g_strdup_printf(_("Enter Password for %s"), Glib::path_get_basename(filepath).c_str());
        CTDialogTextEntry dialogTextEntry(title, true/*forPassword*/, this);
        Glib::ustring password = dialogTextEntry.getEntryText();
        g_free(title);
        if (!password.empty())
        {
            //CTApplication::P_ctTmp.
        }
    }


    return _theTreestore.read_nodes_from_filepath(filepath);
}


CTDialogTextEntry::CTDialogTextEntry(const char *title, const bool forPassword, Gtk::Window* pParent)
{
    set_title(title);
    set_transient_for(*pParent);
    set_modal();

    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    _entry.set_icon_from_stock(Gtk::Stock::CLEAR, Gtk::ENTRY_ICON_SECONDARY);
    _entry.set_size_request(350, -1);
    if (forPassword)
    {
        _entry.set_visibility(false);
    }
    get_vbox()->pack_start(_entry, true, true, 0);

    _entry.signal_key_press_event().connect(sigc::mem_fun(*this, &CTDialogTextEntry::on_entry_key_press_event), false);
    _entry.signal_icon_press().connect(sigc::mem_fun(*this, &CTDialogTextEntry::on_entry_icon_press));

    get_vbox()->show_all();
}

CTDialogTextEntry::~CTDialogTextEntry()
{
}

bool CTDialogTextEntry::on_entry_key_press_event(GdkEventKey *eventKey)
{
    if(GDK_KEY_Return == eventKey->keyval)
    {
        Gtk::Button *pButton = static_cast<Gtk::Button*>(get_widget_for_response(Gtk::RESPONSE_OK));
        pButton->clicked();
        return true;
    }
    return false;
}

void CTDialogTextEntry::on_entry_icon_press(Gtk::EntryIconPosition iconPosition, const GdkEventButton* event)
{
    _entry.set_text("");
}

Glib::ustring CTDialogTextEntry::getEntryText()
{
    return _entry.get_text();
}
