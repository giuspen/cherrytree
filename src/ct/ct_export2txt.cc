/*
 * ct_export2txt.cc
 *
 * Copyright 2009-2020
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

#include "ct_export2txt.h"
#include "ct_main_win.h"

CtExport2Txt::CtExport2Txt(CtMainWin* pCtMainWin)
 : _pCtMainWin(pCtMainWin)
{
}

// Export the Selected Node To Txt
Glib::ustring CtExport2Txt::node_export_to_txt(CtTreeIter tree_iter, fs::path filepath, CtExportOptions export_options, int sel_start, int sel_end)
{
    Glib::ustring plain_text;
    if (export_options.include_node_name) {
        for (int i = 0; i < 1+_pCtMainWin->get_tree_store().get_store()->iter_depth(tree_iter); ++i) {
            plain_text += "#";
        }
        plain_text += CtConst::CHAR_SPACE + tree_iter.get_node_name() + CtConst::CHAR_NEWLINE;
    }
    plain_text += selection_export_to_txt(tree_iter.get_node_text_buffer(), sel_start, sel_end, false);
    plain_text += str::repeat(CtConst::CHAR_NEWLINE, 2);
    if (filepath != "")
        g_file_set_contents(filepath.c_str(), plain_text.c_str(), (gssize)plain_text.bytes(), nullptr);
    return plain_text;
}

// Export All Nodes To Txt
void CtExport2Txt::nodes_all_export_to_txt(bool all_tree, fs::path export_dir, fs::path single_txt_filepath, CtExportOptions export_options)
{
    // function to iterate nodes
    Glib::ustring tree_plain_text;
    std::function<void(CtTreeIter)> traverseFunc;
    traverseFunc = [this, &traverseFunc, &export_options, &tree_plain_text, &export_dir](CtTreeIter tree_iter) {
        if (export_dir == "")
            tree_plain_text += node_export_to_txt(tree_iter, "", export_options, -1, -1);
        else
        {
            fs::path filepath = export_dir / (CtMiscUtil::get_node_hierarchical_name(tree_iter) + ".txt");
            node_export_to_txt(tree_iter, filepath, export_options, -1, -1);
        }
        for (auto& child: tree_iter->children())
            traverseFunc(_pCtMainWin->get_tree_store().to_ct_tree_iter(child));
    };
    // start to iterarte nodes
    CtTreeIter tree_iter = all_tree ? _pCtMainWin->get_tree_store().get_ct_iter_first() : _pCtMainWin->curr_tree_iter();
    for (;tree_iter; ++tree_iter)
    {
        traverseFunc(tree_iter);
        if (!all_tree) break;
    }

    if (single_txt_filepath != "")
        g_file_set_contents(single_txt_filepath.c_str(), tree_plain_text.c_str(), (gssize)tree_plain_text.bytes(), nullptr);
}

// Export the Buffer To Txt
Glib::ustring CtExport2Txt::selection_export_to_txt(Glib::RefPtr<Gtk::TextBuffer> text_buffer, int sel_start, int sel_end, bool check_link_target)
{
    Glib::ustring plain_text;
    std::list<CtAnchoredWidget*> widgets = _pCtMainWin->curr_tree_iter().get_anchored_widgets(sel_start, sel_end);

    int start_offset = sel_start >= 0 ? sel_start : 0;
    for (CtAnchoredWidget* widget: widgets)
    {
        int end_offset = widget->getOffset();
        Glib::ustring text_slot = _plain_process_slot(start_offset, end_offset, text_buffer, false);
        plain_text += text_slot;
        if (CtTable* ctTable = dynamic_cast<CtTable*>(widget)) plain_text += get_table_plain(ctTable);
        else if (CtCodebox* ctCodebox = dynamic_cast<CtCodebox*>(widget)) plain_text += get_codebox_plain(ctCodebox);
        start_offset = end_offset;
    }
    plain_text += _plain_process_slot(start_offset, sel_end, text_buffer, check_link_target && widgets.empty());
    return plain_text;
}

// Returns the plain Table
Glib::ustring CtExport2Txt::get_table_plain(CtTable* table_orig)
{
    Glib::ustring table_plain = CtConst::CHAR_NEWLINE;
    for (const auto& row: table_orig->get_table_matrix())
    {
        table_plain += CtConst::CHAR_PIPE;
        for (const auto& cell: row)
            table_plain += CtConst::CHAR_SPACE + cell->get_text_content() + CtConst::CHAR_SPACE + CtConst::CHAR_PIPE;
        table_plain += CtConst::CHAR_NEWLINE;
    }
    return table_plain;
}

// Returns the plain CodeBox
Glib::ustring CtExport2Txt::get_codebox_plain(CtCodebox* codebox)
{
    Glib::ustring codebox_plain = CtConst::CHAR_NEWLINE + _pCtMainWin->get_ct_config()->hRule + CtConst::CHAR_NEWLINE;
    codebox_plain += codebox->get_text_content();
    codebox_plain += CtConst::CHAR_NEWLINE + _pCtMainWin->get_ct_config()->hRule + CtConst::CHAR_NEWLINE;
    return codebox_plain;
}

// Process a Single plain Slot
Glib::ustring CtExport2Txt::_plain_process_slot(int start_offset, int end_offset, Glib::RefPtr<Gtk::TextBuffer> curr_buffer, bool check_link_target)
{
    if (end_offset == -1)
        end_offset = curr_buffer->end().get_offset();
    // begin operations
    Gtk::TextIter start_iter = curr_buffer->get_iter_at_offset(start_offset);
    Gtk::TextIter end_iter = curr_buffer->get_iter_at_offset(end_offset);
    // todo: check how check_link_target works
    if (!check_link_target)
        return curr_buffer->get_text(start_iter, end_iter);

    Glib::ustring start_link = _tag_link_in_given_iter(start_iter);
    Glib::ustring middle_link = _tag_link_in_given_iter(curr_buffer->get_iter_at_offset((start_offset+end_offset)/2-1));
    Glib::ustring end_link = _tag_link_in_given_iter(curr_buffer->get_iter_at_offset(end_offset-1));
    if (start_link !="" && start_link == middle_link && middle_link == end_link && !str::startswith(start_link, "node"))
    {
        return _pCtMainWin->sourceview_hovering_link_get_tooltip(start_link);
    }
    else
    {
        return curr_buffer->get_text(start_iter, end_iter);
    }
}

// Check for tag link in given_iter
Glib::ustring CtExport2Txt::_tag_link_in_given_iter(Gtk::TextIter iter)
{
    for (auto tag: iter.get_tags())
        if (str::startswith(tag->property_name().get_value(), "link_"))
            return tag->property_name().get_value().substr(5);
    return "";
}


