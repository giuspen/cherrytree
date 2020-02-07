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
#include "ct_export2html.h"

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
    auto export_type = auto_path != "" ? CtDialogs::CtProcessNode::ALL_TREE
                                       : CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name,
                                                                                         &_export_options.new_node_page, nullptr);
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
        CtExportPrint(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options, -1, -1);
    }
    else if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE_AND_SUBNODES)
    {
        if (save_to_pdf)
        {
            pdf_filepath = _get_pdf_filepath(_pCtMainWin->get_curr_doc_file_name());
            if (pdf_filepath == "") return;
        }
        CtExportPrint(_pCtMainWin).node_and_subnodes_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options);
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
        CtExportPrint(_pCtMainWin).tree_export_print(pdf_filepath, _pCtMainWin->curr_tree_store().get_ct_iter_first(), _export_options);
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
        CtExportPrint(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options, iter_start.get_offset(), iter_end.get_offset());
    }
}

Glib::ustring CtActions::_get_pdf_filepath(Glib::ustring proposed_name)
{
    CtDialogs::file_select_args args = {.pParentWin=_pCtMainWin, .curr_folder=_pCtMainWin->get_ct_config()->pickDirExport, .curr_file_name=proposed_name + ".pdf",
                                       .filter_name=("PDF File"), .filter_pattern={"*.pdf"}};
    Glib::ustring filename = CtDialogs::file_save_as_dialog(args);
    if (filename != "")
    {
        if (!str::endswith(filename, ".pdf")) filename += ".pdf";
        _pCtMainWin->get_ct_config()->pickDirExport = Glib::path_get_dirname(filename);
    }
    return filename;
}

// Export to HTML
void CtActions::_export_to_html(Glib::ustring auto_path, bool auto_overwrite)
{
    if (!_is_there_selected_node_or_error()) return;
    auto export_type = auto_path != "" ? CtDialogs::CtProcessNode::ALL_TREE
                                       : CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name,
                                                                                         nullptr, &_export_options.index_in_page);
    if (export_type == CtDialogs::CtProcessNode::NONE) return;

    CtExport2Html export2html(_pCtMainWin);
    if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE)
    {
        Glib::ustring folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false))
            export2html.node_export_to_html(_pCtMainWin->curr_tree_iter(), _export_options, "", -1, -1);
    }
    else if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE_AND_SUBNODES)
    {
        Glib::ustring folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false))
            export2html.nodes_all_export_to_html(false, _export_options);
        // todo:
        // if self.filetype in ["b", "x"] and self.curr_tree_iter:
        //   self.state_machine.update_state()
        //   self.objects_buffer_refresh()
    }
    else if (export_type == CtDialogs::CtProcessNode::ALL_TREE)
    {
        Glib::ustring folder_name = _pCtMainWin->get_curr_doc_file_name();
        if (export2html.prepare_html_folder(auto_path, folder_name, auto_overwrite))
            export2html.nodes_all_export_to_html(true, _export_options);
        // todo:
        // if self.filetype in ["b", "x"] and self.curr_tree_iter:
        //     self.state_machine.update_state()
        //     self.objects_buffer_refresh()
    }
    else if (export_type == CtDialogs::CtProcessNode::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        Glib::ustring folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false))
            export2html.node_export_to_html(_pCtMainWin->curr_tree_iter(), _export_options, "", iter_start.get_offset(), iter_end.get_offset());
    }
}
