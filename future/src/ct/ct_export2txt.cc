/*
 * ct_export2txt.cc
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

#include "ct_export2txt.h"
#include "ct_main_win.h"

CtExport2Txt::CtExport2Txt(CtMainWin* pCtMainWin)
 : _pCtMainWin(pCtMainWin)
{
}

// Export the Selected Node To Txt
Glib::ustring CtExport2Txt::node_export_to_txt(Glib::RefPtr<Gtk::TextBuffer> text_buffer, std::pair<int, int> sel_range,
                                               bool check_link_target /*=false*/, Glib::ustring /*filepath*/ /*=""*/, CtTreeIter* tree_iter_for_node_name /*=nullptr*/)
{
    Glib::ustring plain_text;
    std::list<CtAnchoredWidget*> widgets = _pCtMainWin->curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(sel_range);

    int start_offset = sel_range.first >= 0 ? sel_range.first : 0;
    for (CtAnchoredWidget* widget: widgets)
    {
        int end_offset = widget->getOffset();
        Glib::ustring text_slot = _plain_process_slot(start_offset, end_offset, text_buffer, false);
        plain_text += text_slot;
        if (CtTable* ctTable = dynamic_cast<CtTable*>(widget)) plain_text += get_table_plain(ctTable);
        else if (CtCodebox* ctCodebox = dynamic_cast<CtCodebox*>(widget)) plain_text += get_codebox_plain(ctCodebox);
        start_offset = end_offset;
    }
    if (sel_range.second < 0)
        plain_text += _plain_process_slot(start_offset, -1, text_buffer, check_link_target && widgets.empty());
    else
        plain_text += _plain_process_slot(start_offset, sel_range.second, text_buffer, check_link_target && widgets.empty());

    if (tree_iter_for_node_name)
    {
        Glib::ustring node_name = tree_iter_for_node_name->get_node_name(); // todo: clean_text_to_utf8(self.dad.treestore[tree_iter_for_node_name][1])
        plain_text = node_name.uppercase() + CtConst::CHAR_NEWLINE + plain_text;
    }

    // todo:
    /*if filepath:
        file_descriptor = open(filepath, 'a')
        file_descriptor.write(plain_text + 2*cons.CHAR_NEWLINE)
        file_descriptor.close()
    */
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


