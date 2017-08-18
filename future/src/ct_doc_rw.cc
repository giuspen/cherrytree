/*
 * ct_doc_rw.cc
 * 
 * Copyright 2017 giuspen <giuspen@gmail.com>
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

#include "ct_doc_rw.h"


CherryTreeDocRead::CherryTreeDocRead(std::list<gint64> *p_bookmarks, Glib::RefPtr<Gtk::TreeStore> r_treestore) : mp_bookmarks(p_bookmarks), mr_treestore(r_treestore)
{
}


CherryTreeDocRead::~CherryTreeDocRead()
{
}
