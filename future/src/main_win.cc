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


TheTreeView::TheTreeView() : Gtk::TreeView()
{
    set_headers_visible(false);
}


TheTreeView::~TheTreeView()
{
}


MainWindow::MainWindow() : Gtk::ApplicationWindow()
{
    m_scrolledwindow_tree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(m_scrolledwindow_tree);
    m_treestore.view_append_columns(&m_treeview);
    m_treestore.view_connect(&m_treeview);
    m_scrolledwindow_tree.add(m_treeview);
    set_size_request(300, 400);
    show_all();
}


MainWindow::~MainWindow()
{
}


bool MainWindow::read_nodes_from_gio_file(const Glib::RefPtr<Gio::File>& r_file)
{
    std::string filepath{r_file->get_path()};
    if ( (Glib::str_has_suffix(filepath, ".ctz")) ||
         (Glib::str_has_suffix(filepath, ".ctx")) )
    {
        //CTApplication::P_ctTmp.
    }
    
    
    return m_treestore.read_nodes_from_filepath(filepath);
}
