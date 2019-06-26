/*
 * ct_export2txt.h
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

#include <gtkmm/textiter.h>
#include "ct_treestore.h"
#include "ct_table.h"

class CtExport2Txt
{
public:
    CtExport2Txt();

public:
    Glib::ustring node_export_to_txt(Glib::RefPtr<Gtk::TextBuffer> text_buffer, std::pair<int, int> sel_range,
                                     bool check_link_target = false, Glib::ustring filepath = "", CtTreeIter* tree_iter_for_node_name = nullptr);

    Glib::ustring get_table_plain(CtTable* table_orig);
    Glib::ustring get_codebox_plain(CtCodebox* codebox);

private:
    Glib::ustring _plain_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer, bool check_link_target);
    Glib::ustring _tag_link_in_given_iter(Gtk::TextIter iter);
};
