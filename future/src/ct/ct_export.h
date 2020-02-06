/*
 * ct_export.h
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_main_win.h"

class CtExportPrint
{
public:
    CtExportPrint(CtMainWin* pCtMainWin) { _pCtMainWin = pCtMainWin; }

    void node_export_print(const Glib::ustring& pdf_filepath, CtTreeIter tree_iter, bool include_node_name, int sel_start, int sel_end);
    void node_and_subnodes_export_print(const Glib::ustring& pdf_filepath, CtTreeIter tree_iter, bool include_node_name, bool new_node_in_new_page);
    void tree_export_print(const Glib::ustring& pdf_filepath, CtTreeIter tree_iter, bool include_node_name, bool new_node_in_new_page);

private:
    void _nodes_all_export_print_iter(CtTreeIter tree_iter, bool include_node_name, bool new_node_in_new_page,
                                      std::vector<Glib::ustring>& tree_pango_slots, std::list<CtAnchoredWidget*>& tree_widgets, Glib::ustring& text_font);
    void _add_node_name(Glib::ustring node_name, std::vector<Glib::ustring>& pango_slots);

private:
    CtMainWin* _pCtMainWin;
};


class CtExport2Pango
{
public:
    Glib::ustring pango_get_from_code_buffer(Glib::RefPtr<Gsv::Buffer> code_buffer, int sel_start, int sel_end);
    void pango_get_from_treestore_node(CtTreeIter node_iter, int sel_start, int sel_end, std::vector<Glib::ustring>& out_slots,
                                       bool excude_anchors, std::list<CtAnchoredWidget*>& out_widgets);
private:
    Glib::ustring _pango_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer);
    Glib::ustring _pango_text_serialize(Gtk::TextIter start_iter, Gtk::TextIter end_iter, const std::map<const gchar*, std::string>& curr_attributes);
};
