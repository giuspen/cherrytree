/*
 * ct_export2txt.h
 *
 * Copyright 2009-2022
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
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
#include "ct_dialogs.h"

class CtImageLatex;

class CtExport2Txt
{
public:
    CtExport2Txt(CtMainWin* pCtMainWin);

public:
    Glib::ustring node_export_to_txt(CtTreeIter tree_iter, fs::path filepath, CtExportOptions export_options, int sel_start, int sel_end);
    void          nodes_all_export_to_txt(bool all_tree, fs::path export_dir, fs::path single_txt_filepath, CtExportOptions export_options);
    Glib::ustring selection_export_to_txt(CtTreeIter tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, int sel_start, int sel_end, bool check_link_target);

    Glib::ustring get_table_plain(CtTableCommon* table_orig);
    Glib::ustring get_codebox_plain(CtCodebox* codebox);
    Glib::ustring get_latex_plain(CtImageLatex* latex);

private:
    Glib::ustring _plain_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer, bool check_link_target);
    Glib::ustring _tag_link_in_given_iter(Gtk::TextIter iter);

private:
    CtMainWin* _pCtMainWin;
};
