/*
 * ct_actions_export.cc
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

#include "ct_actions.h"
#include "ct_export.h"

// Print Page Setup Operations
void CtActions::export_print_page_setup()
{
    _pCtMainWin->get_ct_print().run_page_setup_dialog(_pCtMainWin);
}

void CtActions::export_print()
{
    _export_print(false, "", false);
}

void CtActions::export_to_pdf()
{
    _export_print(true, "", false);
}

void CtActions::export_to_html()
{

}

void CtActions::export_to_txt_multiple()
{

}

void CtActions::export_to_txt_single()
{

}

void CtActions::export_to_ctd()
{

}

void CtActions::_export_print(bool save_to_pdf, Glib::ustring auto_path, bool auto_overwrite)
{
    if (!_is_there_selected_node_or_error()) return;
    auto export_type = auto_path != "" ? CtDialogs::CtProcessNode::ALL_TREE : CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_last_include_node_name, &_last_new_node_page, nullptr);
    if (export_type == CtDialogs::CtProcessNode::NONE) return;

    Glib::ustring pdf_filepath;
    if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE)
    {
        if (save_to_pdf)
        {
            pdf_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
            pdf_filepath = _get_pdf_filepath(pdf_filepath);
            if (pdf_filepath == "") return;
        }
        CtExportPrint(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _last_include_node_name, -1, -1);
    }
    else if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE_AND_SUBNODES)
    {
        if (save_to_pdf)
        {
            pdf_filepath = _get_pdf_filepath(_pCtMainWin->get_curr_doc_file_name());
            if (pdf_filepath == "") return;
        }
        CtExportPrint(_pCtMainWin).node_and_subnodes_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _last_include_node_name, _last_new_node_page);
    }
    else if (export_type == CtDialogs::CtProcessNode::ALL_TREE)
    {
        if (auto_path != "")
        {
            pdf_filepath = auto_path;
            // todo: if (!auto_overwrite && Gio::File:: os.path.isfile(self.print_handler.pdf_filepath):
            //    return
        }
        else if (save_to_pdf)
        {
            pdf_filepath = _get_pdf_filepath(_pCtMainWin->get_curr_doc_file_name());
            if (pdf_filepath == "") return;
        }
        CtExportPrint(_pCtMainWin).tree_export_print(pdf_filepath, _pCtMainWin->curr_tree_store().get_ct_iter_first(), _last_include_node_name, _last_new_node_page);
    }
    else if (export_type == CtDialogs::CtProcessNode::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        if (save_to_pdf)
        {
            pdf_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
            pdf_filepath = _get_pdf_filepath(pdf_filepath);
            if (pdf_filepath == "") return;
        }
        CtExportPrint(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _last_include_node_name, iter_start.get_offset(), iter_end.get_offset());
    }
}

Glib::ustring CtActions::_get_pdf_filepath(Glib::ustring proposed_name)
{
    CtDialogs::file_select_args args = {.pParentWin=_pCtMainWin, .curr_folder=_pCtMainWin->get_ct_config()->pickDirExport, .curr_file_name=proposed_name + ".pdf",
                                       .filter_name=("PDF File"), .filter_pattern={"*.pdf"}};
    Glib::ustring filename = CtDialogs::file_save_as_dialog(args);
    if (filename != "")
    {
        if (str::endswith(filename, ".pdf")) filename += ".pdf";
        _pCtMainWin->get_ct_config()->pickDirExport = Glib::path_get_dirname(filename);
    }
    return filename;
}
