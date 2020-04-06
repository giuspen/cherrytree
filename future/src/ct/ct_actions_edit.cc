/*
 * ct_actions_find.cc
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
#include <gtkmm/dialog.h>
#include "ct_clipboard.h"
#include "ct_list.h"
#include "ct_image.h"
#include <fstream>

// A Special character insert was Requested
void CtActions::insert_spec_char_action(gunichar ch)
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    proof.text_buffer->insert_at_cursor(Glib::ustring(1, ch));
}

// Step Back for the Current Node, if Possible
void CtActions::requested_step_back()
{
    if (!_pCtMainWin->curr_tree_iter()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto step_back = _pCtMainWin->get_state_machine().requested_state_previous(_pCtMainWin->curr_tree_iter().get_node_id());
    if (step_back)
        _pCtMainWin->load_buffer_from_state(step_back, _pCtMainWin->curr_tree_iter());
}

// Step Ahead for the Current Node, if Possible
void CtActions::requested_step_ahead()
{
    if (!_pCtMainWin->curr_tree_iter()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto step_ahead = _pCtMainWin->get_state_machine().requested_state_subsequent(_pCtMainWin->curr_tree_iter().get_node_id());
    if (step_ahead)
        _pCtMainWin->load_buffer_from_state(step_ahead, _pCtMainWin->curr_tree_iter());
}

// Insert/Edit Image
void CtActions::image_handle()
{
    if (not _node_sel_and_rich_text()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtDialogs::file_select_args args = {.pParentWin=_pCtMainWin, .curr_folder=_pCtMainWin->get_ct_config()->pickDirImg};
    std::string filename = CtDialogs::file_select_dialog(args);
    if (filename.empty()) return;
    _pCtMainWin->get_ct_config()->pickDirImg = Glib::path_get_dirname(filename);

    Glib::RefPtr<Gdk::Pixbuf> rPixbuf = Gdk::Pixbuf::create_from_file(filename);
    if (rPixbuf)
        _image_edit_dialog(rPixbuf, _curr_buffer()->get_insert()->get_iter(), nullptr);
    else
        CtDialogs::error_dialog(_("Image Format Not Recognized"), *_pCtMainWin);
}

// Insert Table
void CtActions::table_handle()
{
    if (!_node_sel_and_rich_text()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    int res = _table_dialog(_("Insert Table"), true);
    if (res == 0) return;

    int col_min = _pCtMainWin->get_ct_config()->tableColMin;
    int col_max = _pCtMainWin->get_ct_config()->tableColMin;
    std::list<std::vector<std::string>> rows;
    if (res == 1) {
        rows.push_back(std::vector<std::string>(_pCtMainWin->get_ct_config()->tableColumns, "click me"));
        std::vector<std::string> empty_row(_pCtMainWin->get_ct_config()->tableColumns, "");
        while (rows.size() < _pCtMainWin->get_ct_config()->tableRows)
            rows.push_back(empty_row);
    }
    if (res == 2) {
        CtDialogs::file_select_args args = {.pParentWin=_pCtMainWin, .curr_folder=_pCtMainWin->get_ct_config()->pickDirCsv,
                                           .filter_name=_("CSV File"), .filter_pattern={"*.csv"}};
        Glib::ustring filename = CtDialogs::file_select_dialog(args);
        if (filename.empty()) return;
        _pCtMainWin->get_ct_config()->pickDirCsv = Glib::path_get_dirname(filename);
        // todo: find good csv lib
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            if (line == "\r\n" || line == "\n\r" || line == "\n")
                continue;
            std::vector<std::string> splited_line = str::split(line, ",");
            for (std::string& word: splited_line)
                if (str::startswith(word, "\"") && str::endswith(word, "\""))
                    word = word.substr(1, word.size() - 2);
            rows.push_back(splited_line);
        }
        size_t col_num = 0;
        for (auto& row: rows)
            col_num = std::max(col_num, row.size());
        for (auto& row: rows)
            while (row.size() < col_num)
                row.push_back("");
        col_min = 40;
        col_max = 60;
    }

    CtTableMatrix tableMatrix;
    for(auto& row: rows)
    {
        tableMatrix.push_back(CtTableRow{});
        for (auto& cell: row)
            tableMatrix.back().push_back(new CtTableCell(_pCtMainWin, cell, CtConst::TABLE_CELL_TEXT_ID));
    }

    CtTable* pCtTable = new CtTable(_pCtMainWin, tableMatrix, col_min, col_max, true, _curr_buffer()->get_insert()->get_iter().get_offset(), "");
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pCtTable->insertInTextBuffer(gsv_buffer);

    getCtMainWin()->curr_tree_store().addAnchoredWidgets(getCtMainWin()->curr_tree_iter(),
        {pCtTable}, &getCtMainWin()->get_text_view());
    //pCtTable->get_text_view().grab_focus();
}

// Insert Code Box
void CtActions::codebox_handle()
{
    if (not _node_sel_and_rich_text()) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    Glib::ustring textContent, justification;
    Gtk::TextIter iter_sel_start, iter_sel_end;
    if (_curr_buffer()->get_has_selection())
    {
        _curr_buffer()->get_selection_bounds(iter_sel_start, iter_sel_end);
        textContent = iter_sel_start.get_text(iter_sel_end);
    }
    if (not CtDialogs::codeboxhandle_dialog(_pCtMainWin, _("Insert a CodeBox")))
        return;

    if (not textContent.empty())
        _curr_buffer()->erase(iter_sel_start, iter_sel_end);

    Gtk::TextIter iter_insert = _curr_buffer()->get_insert()->get_iter();

    CtCodebox* pCtCodebox = new CtCodebox(_pCtMainWin,
                                          textContent,
                                          _pCtMainWin->get_ct_config()->codeboxSynHighl,
                                          (int)_pCtMainWin->get_ct_config()->codeboxWidth,
                                          (int)_pCtMainWin->get_ct_config()->codeboxHeight,
                                          iter_insert.get_offset(),
                                          justification,
                                          _pCtMainWin->get_ct_config()->codeboxWidthPixels,
                                          _pCtMainWin->get_ct_config()->codeboxMatchBra,
                                          _pCtMainWin->get_ct_config()->codeboxLineNum);
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pCtCodebox->insertInTextBuffer(gsv_buffer);

    getCtMainWin()->curr_tree_store().addAnchoredWidgets(getCtMainWin()->curr_tree_iter(),
        {pCtCodebox}, &getCtMainWin()->get_text_view());
    pCtCodebox->get_text_view().grab_focus();
}

// Embedded File Insert
void CtActions::embfile_insert()
{
    if (!_node_sel_and_rich_text()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    auto iter_insert = _curr_buffer()->get_insert()->get_iter();

    CtDialogs::file_select_args args = {.pParentWin=_pCtMainWin, .curr_folder=_pCtMainWin->get_ct_config()->pickDirFile};
    std::string filepath = CtDialogs::file_select_dialog(args);
    if (filepath.empty()) return;

    _pCtMainWin->get_ct_config()->pickDirFile = Glib::path_get_dirname(filepath);
    if (CtFileSystem::getsize(filepath) > _pCtMainWin->get_ct_config()->embfileMaxSize*1024*1024)
    {
        CtDialogs::error_dialog(str::format(_("The Maximum Size for Embedded Files is %s MB"), _pCtMainWin->get_ct_config()->embfileMaxSize), *_pCtMainWin);
        return;
    }

    auto file = std::fstream(filepath, std::ios::in | std::ios::binary);
    std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
    file.close();

    auto blob = std::string(buffer.data(), buffer.size());
    std::string name = Glib::path_get_basename(filepath);
    CtAnchoredWidget* pAnchoredWidget = new CtImageEmbFile(_pCtMainWin, name, blob, std::time(nullptr), iter_insert.get_offset(), "");
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);

    getCtMainWin()->curr_tree_store().addAnchoredWidgets(getCtMainWin()->curr_tree_iter(),
        {pAnchoredWidget}, &getCtMainWin()->get_text_view());
}

// The Link Insert Button was Pressed
void CtActions::apply_tag_link()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_LINK);
}

// Insert an Anchor
void CtActions::anchor_handle()
{
    if (!_node_sel_and_rich_text()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    _anchor_edit_dialog(nullptr, _curr_buffer()->get_insert()->get_iter(), nullptr);
}

void CtActions::toc_insert()
{
    // todo:
}

// Insert Timestamp
void CtActions::timestamp_insert()
{
    if (not _is_curr_node_not_read_only_or_error()) return;

    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;

    time_t time = std::time(nullptr);
    std::string timestamp = str::time_format(_pCtMainWin->get_ct_config()->timestampFormat, time);
    proof.text_buffer->insert_at_cursor(timestamp);
}

// Insert a Horizontal Line
void CtActions::horizontal_rule_insert()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    proof.text_buffer->insert_at_cursor(_pCtMainWin->get_ct_config()->hRule + CtConst::CHAR_NEWLINE);
}

// Lowers the Case of the Selected Text/the Underlying Word
void CtActions::text_selection_lower_case()
{
    _text_selection_change_case('l');
}

// Uppers the Case of the Selected Text/the Underlying Word
void CtActions::text_selection_upper_case()
{
    _text_selection_change_case('u');
}

// Toggles the Case of the Selected Text/the Underlying Word
void CtActions::text_selection_toggle_case()
{
    _text_selection_change_case('t');
}

void CtActions::toggle_ena_dis_spellcheck()
{
    // todo:
}

// Copy as Plain Text
void CtActions::cut_as_plain_text()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtClipboard::force_plain_text();
    auto proof = _get_text_view_n_buffer_codebox_proof();
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "cut-clipboard");
}

// Copy as Plain Text
void CtActions::copy_as_plain_text()
{
    CtClipboard::force_plain_text();
    auto proof = _get_text_view_n_buffer_codebox_proof();
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "copy-clipboard");
}

// Paste as Plain Text
void CtActions::paste_as_plain_text()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    CtClipboard::force_plain_text();
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "paste-clipboard");
}

// Cut a Whole Row
void CtActions::text_row_cut()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    CtTextRange range = CtList(_pCtMainWin, proof.text_buffer).get_paragraph_iters();
    if (not range.iter_end.forward_char() and !range.iter_start.backward_char()) return;
    proof.text_buffer->select_range(range.iter_start, range.iter_end);
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "cut-clipboard");
}

// Copy a Whole Row
void CtActions::text_row_copy()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;

    CtTextRange range = CtList(_pCtMainWin, proof.text_buffer).get_paragraph_iters();
    if (not range.iter_end.forward_char() and !range.iter_start.backward_char()) return;
    proof.text_buffer->select_range(range.iter_start, range.iter_end);
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "copy-clipboard");
}

// Deletes the Whole Row
void CtActions::text_row_delete()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    CtTextRange range = CtList(_pCtMainWin, proof.text_buffer).get_paragraph_iters();
    if (not range.iter_end.forward_char() and !range.iter_start.backward_char()) return;
    proof.text_buffer->erase(range.iter_start, range.iter_end);
    _pCtMainWin->get_state_machine().update_state();
}

// Duplicates the Whole Row or a Selection
void CtActions::text_row_selection_duplicate()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    auto text_buffer = proof.text_buffer;
    if (proof.text_buffer->get_has_selection())
    {
        Gtk::TextIter iter_start, iter_end;
        text_buffer->get_selection_bounds(iter_start, iter_end);
        int sel_start_offset = iter_start.get_offset();
        int sel_end_offset = iter_end.get_offset();
        if (proof.from_codebox or proof.syntax_highl != CtConst::RICH_TEXT_ID)
        {
            Glib::ustring text_to_duplicate = text_buffer->get_text(iter_start, iter_end);
            if (text_to_duplicate.find(CtConst::CHAR_NEWLINE) != Glib::ustring::npos)
                text_to_duplicate = CtConst::CHAR_NEWLINE + text_to_duplicate;
            text_buffer->insert(iter_end, text_to_duplicate);
        }
        else
        {
            Glib::ustring rich_text = CtClipboard(_pCtMainWin).rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer, iter_start, iter_end);
            if (rich_text.find(CtConst::CHAR_NEWLINE) != Glib::ustring::npos)
            {
                text_buffer->insert(iter_end, CtConst::CHAR_NEWLINE);
                iter_end = proof.text_buffer->get_iter_at_offset(sel_end_offset+1);
                text_buffer->move_mark(proof.text_buffer->get_insert(), iter_end);
            }
            CtClipboard(_pCtMainWin).from_xml_string_to_buffer(text_buffer, rich_text);
        }
        text_buffer->select_range(text_buffer->get_iter_at_offset(sel_start_offset),
                                 text_buffer->get_iter_at_offset(sel_end_offset));
    }
    else
    {
        int cursor_offset = text_buffer->get_iter_at_mark(text_buffer->get_insert()).get_offset();
        CtTextRange range = CtList(_pCtMainWin, proof.text_buffer).get_paragraph_iters();
        if (range.iter_start.get_offset() == range.iter_end.get_offset())
        {
            Gtk::TextIter iter_start = text_buffer->get_iter_at_mark(text_buffer->get_insert());
            text_buffer->insert(iter_start, CtConst::CHAR_NEWLINE);
        }
        else
        {
            if (proof.from_codebox or proof.syntax_highl != CtConst::RICH_TEXT_ID)
            {
                Glib::ustring text_to_duplicate = text_buffer->get_text(range.iter_start, range.iter_end);
                text_buffer->insert(range.iter_end, CtConst::CHAR_NEWLINE + text_to_duplicate);
            }
            else
            {
                Glib::ustring rich_text = CtClipboard(_pCtMainWin).rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer, range.iter_start, range.iter_end);
                int sel_end_offset = range.iter_end.get_offset();
                text_buffer->insert(range.iter_end, CtConst::CHAR_NEWLINE);
                range.iter_end = text_buffer->get_iter_at_offset(sel_end_offset+1);
                text_buffer->move_mark(text_buffer->get_insert(), range.iter_end);
                CtClipboard(_pCtMainWin).from_xml_string_to_buffer(proof.text_buffer, rich_text);
                text_buffer->place_cursor(text_buffer->get_iter_at_offset(cursor_offset));
            }
        }
    }
    _pCtMainWin->get_state_machine().update_state();
}

// Moves Up the Current Row/Selected Rows
void CtActions::text_row_up()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_buffer;
    CtTextRange range = CtList(_pCtMainWin, text_buffer).get_paragraph_iters();
    range.iter_end.forward_char();
    bool missing_leading_newline = false;
    Gtk::TextIter destination_iter = range.iter_start;

    if (not destination_iter.backward_char()) return;
    if (not destination_iter.backward_char())
        missing_leading_newline = true;
    else
    {
        while (destination_iter.get_char() != CtConst::CHAR_NEWLINE[0])
            if (not destination_iter.backward_char())
            {
                missing_leading_newline = true;
                break;
            }
    }
    if (not missing_leading_newline) destination_iter.forward_char();
    int destination_offset = destination_iter.get_offset();
    int start_offset = range.iter_start.get_offset();
    int end_offset = range.iter_end.get_offset();
    //#print "iter_start %s %s '%s'" % (start_offset, ord(iter_start.get_char()), iter_start.get_char())
    //#print "iter_end %s %s '%s'" % (end_offset, ord(iter_end.get_char()), iter_end.get_char())
    //#print "destination_iter %s %s '%s'" % (destination_offset, ord(destination_iter.get_char()), destination_iter.get_char())
    Glib::ustring text_to_move = text_buffer->get_text(range.iter_start, range.iter_end);
    int diff_offsets = end_offset - start_offset;
    if (proof.from_codebox or proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (text_to_move.empty() or text_to_move[text_to_move.length()-1] != CtConst::CHAR_NEWLINE[0])
        {
            diff_offsets += 1;
            text_to_move += CtConst::CHAR_NEWLINE;
        }
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        text_buffer->insert(destination_iter, text_to_move);
        proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
    }
    else
    {
        Glib::ustring rich_text = CtClipboard(_pCtMainWin).rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(),
                                                                                         text_buffer, range.iter_start, range.iter_end, 'n', true /*exclude_iter_sel_end*/);
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (destination_offset > 0)
        {
            // clear the newline from any tag
            Gtk::TextIter clr_start_iter = text_buffer->get_iter_at_offset(destination_offset-1);
            text_buffer->remove_all_tags(clr_start_iter, destination_iter);
        }
        bool append_newline = false;
        if (text_to_move.empty() or text_to_move[text_to_move.length()-1] != CtConst::CHAR_NEWLINE[0])
        {
            diff_offsets += 1;
            append_newline = true;
        }
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        // trick of space to prevent subsequent text to take pasted text tag(s)
        text_buffer->insert_at_cursor(CtConst::CHAR_SPACE);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        // write moved line
        CtClipboard(_pCtMainWin).from_xml_string_to_buffer(text_buffer, rich_text);
        if (append_newline)
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        // clear space trick
        Gtk::TextIter cursor_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        text_buffer->erase(cursor_iter, text_buffer->get_iter_at_offset(cursor_iter.get_offset()+1));
        // selection
        proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
    }
    _pCtMainWin->get_state_machine().update_state();
}

// Moves Down the Current Row/Selected Rows
void CtActions::text_row_down()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (not proof.text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_buffer;
    CtTextRange range = CtList(_pCtMainWin, text_buffer).get_paragraph_iters();
    if (not range.iter_end.forward_char()) return;
    int missing_leading_newline = false;
    Gtk::TextIter destination_iter = range.iter_end;
    while (destination_iter.get_char() != CtConst::CHAR_NEWLINE[0])
        if (not destination_iter.forward_char())
        {
            missing_leading_newline = true;
            break;
        }
    destination_iter.forward_char();
    int destination_offset = destination_iter.get_offset();
    int start_offset = range.iter_start.get_offset();
    int end_offset = range.iter_end.get_offset();
    //#print "iter_start %s %s '%s'" % (start_offset, ord(iter_start.get_char()), iter_start.get_char())
    //#print "iter_end %s %s '%s'" % (end_offset, ord(iter_end.get_char()), iter_end.get_char())
    //#print "destination_iter %s %s '%s'" % (destination_offset, ord(destination_iter.get_char()), destination_iter.get_char())
    Glib::ustring text_to_move = text_buffer->get_text(range.iter_start, range.iter_end);
    int diff_offsets = end_offset - start_offset;
    if (proof.from_codebox or proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_offset -= diff_offsets;
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (text_to_move.empty() or text_to_move[text_to_move.length() - 1] != CtConst::CHAR_NEWLINE[0])
        {
            diff_offsets += 1;
            text_to_move += CtConst::CHAR_NEWLINE;
        }
        if (missing_leading_newline)
        {
            diff_offsets += 1;
            text_to_move = CtConst::CHAR_NEWLINE + text_to_move;
        }
        text_buffer->insert(destination_iter, text_to_move);
        if (not missing_leading_newline)
            proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
        else
            proof.text_view->set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2);
    }
    else
    {
        Glib::ustring rich_text = CtClipboard(_pCtMainWin).rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer,
                                                                                         range.iter_start, range.iter_end, 'n', true /*exclude_iter_sel_end*/);
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_offset -= diff_offsets;
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (destination_offset > 0)
        {
            // clear the newline from any tag
            Gtk::TextIter clr_start_iter = text_buffer->get_iter_at_offset(destination_offset-1);
            text_buffer->remove_all_tags(clr_start_iter, destination_iter);
        }
        bool append_newline = false;
        if (text_to_move.empty() or text_to_move[text_to_move.length()-1] != CtConst::CHAR_NEWLINE[0])
        {
            diff_offsets += 1;
            append_newline = true;
        }
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        if (missing_leading_newline)
        {
            diff_offsets += 1;
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        }
        // trick of space to prevent subsequent text to take pasted text tag(s)
        text_buffer->insert_at_cursor(CtConst::CHAR_SPACE);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        text_buffer->move_mark(text_buffer->get_insert(), destination_iter);
        // write moved line
        CtClipboard(_pCtMainWin).from_xml_string_to_buffer(text_buffer, rich_text);
        if (append_newline)
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        // clear space trick
        Gtk::TextIter cursor_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        text_buffer->erase(cursor_iter, text_buffer->get_iter_at_offset(cursor_iter.get_offset()+1));
        // selection
        if (not missing_leading_newline)
            proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
        else
            proof.text_view->set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2);
    }
    _pCtMainWin->get_state_machine().update_state();
}

// Remove trailing spaces/tabs
void CtActions::strip_trailing_spaces()
{
    Glib::RefPtr<Gtk::TextBuffer> text_buffer = _curr_buffer();
    int cleaned_lines = 0;
    bool removed_something = true;
    while (removed_something)
    {
        removed_something = false;
        Gtk::TextIter curr_iter = text_buffer->begin();
        int curr_state = 0;
        int start_offset = 0;
        while (curr_iter)
        {
            gunichar curr_char = curr_iter.get_char();
            if (curr_state == 0)
            {
                if (curr_char == CtConst::CHAR_SPACE[0] or curr_char ==  CtConst::CHAR_TAB[0])
                {
                    start_offset = curr_iter.get_offset();
                    curr_state = 1;
                }
            }
            else if (curr_state == 1)
            {
                if (curr_char == CtConst::CHAR_NEWLINE[0])
                {
                    text_buffer->erase(text_buffer->get_iter_at_offset(start_offset), curr_iter);
                    removed_something = true;
                    cleaned_lines += 1;
                    break;
                }
                else if (curr_char != CtConst::CHAR_SPACE[0] and curr_char !=  CtConst::CHAR_TAB[0])
                {
                    curr_state = 0;
                }
            }
            if (not curr_iter.forward_char())
            {
                if (curr_state == 1)
                {
                    text_buffer->erase(text_buffer->get_iter_at_offset(start_offset), curr_iter);
                    cleaned_lines += 1;
                }
                break;
            }
        }
    }

    CtDialogs::info_dialog(std::to_string(cleaned_lines) + " " + _("Lines Stripped"), *_pCtMainWin);
}

// Insert/Edit Image Dialog
void CtActions::_image_edit_dialog(Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
                                   Gtk::TextIter insert_iter,
                                   Gtk::TextIter* iter_bound)
{
    Glib::RefPtr<Gdk::Pixbuf> ret_pixbuf = CtDialogs::image_handle_dialog(*_pCtMainWin, _("Image Properties"), rPixbuf);
    if (not ret_pixbuf) return;
    Glib::ustring link = "";
    Glib::ustring image_justification;
    if (iter_bound) { // only in case of modify
        image_justification = CtTextIterUtil::get_text_iter_alignment(insert_iter, _pCtMainWin);
        int image_offset = insert_iter.get_offset();
        _curr_buffer()->erase(insert_iter, *iter_bound);
        insert_iter = _curr_buffer()->get_iter_at_offset(image_offset);
    }
    image_insert_png(insert_iter, ret_pixbuf, link, image_justification);
}

void CtActions::image_insert_png(Gtk::TextIter iter_insert, Glib::RefPtr<Gdk::Pixbuf> rPixbuf,
                                 const Glib::ustring& link, const Glib::ustring& image_justification)
{
    if (not rPixbuf) return;
    int charOffset = iter_insert.get_offset();
    CtAnchoredWidget* pAnchoredWidget = new CtImagePng(_pCtMainWin, rPixbuf, link, charOffset, image_justification);
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);

    getCtMainWin()->curr_tree_store().addAnchoredWidgets(getCtMainWin()->curr_tree_iter(),
        {pAnchoredWidget}, &getCtMainWin()->get_text_view());
}

void CtActions::image_insert_anchor(Gtk::TextIter iter_insert, const Glib::ustring &name, const Glib::ustring &image_justification)
{
    int charOffset = iter_insert.get_offset();
    CtAnchoredWidget* pAnchoredWidget = new CtImageAnchor(_pCtMainWin, name, charOffset, image_justification);
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);

    getCtMainWin()->curr_tree_store().addAnchoredWidgets(getCtMainWin()->curr_tree_iter(),
        {pAnchoredWidget}, &getCtMainWin()->get_text_view());
}

// Change the Case of the Selected Text/the Underlying Word"""
void CtActions::_text_selection_change_case(gchar change_type)
{
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    Glib::RefPtr<Gtk::TextBuffer> text_buffer = proof.text_buffer;
    if (not text_buffer) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (not text_buffer->get_has_selection() and not _pCtMainWin->apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter()))
    {
        CtDialogs::warning_dialog(_("No Text is Selected"), *_pCtMainWin);
        return;
    }

    Gtk::TextIter iter_start, iter_end;
    text_buffer->get_selection_bounds(iter_start, iter_end);
    Glib::ustring text_to_change_case, rich_text;
    if (proof.from_codebox or proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_to_change_case = text_buffer->get_text(iter_start, iter_end);
        if (change_type == 'l')         text_to_change_case = text_to_change_case.lowercase();
        else if (change_type == 'u')    text_to_change_case = text_to_change_case.uppercase();
        else if (change_type == 't')    text_to_change_case = str::swapcase(text_to_change_case);
    }
    else
    {
        rich_text = CtClipboard(_pCtMainWin).rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer, iter_start, iter_end, change_type);
    }

    int start_offset = iter_start.get_offset();
    int end_offset = iter_end.get_offset();
    text_buffer->erase(iter_start, iter_end);
    Gtk::TextIter iter_insert = text_buffer->get_iter_at_offset(start_offset);
    if (proof.from_codebox or proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_buffer->insert(iter_insert, text_to_change_case);
    }
    else
    {
        text_buffer->move_mark(text_buffer->get_insert(), iter_insert);
        CtClipboard(_pCtMainWin).from_xml_string_to_buffer(text_buffer, rich_text);
    }
    text_buffer->select_range(text_buffer->get_iter_at_offset(start_offset),
                              text_buffer->get_iter_at_offset(end_offset));
}

// Opens the Table Handle Dialog, pygtk: dialog_tablehandle
int CtActions::_table_dialog(Glib::ustring title, bool is_insert)
{
    Gtk::Dialog dialog(title, *_pCtMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(*_pCtMainWin);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);

    auto label_rows = Gtk::Label(_("Rows"));
    auto adj_rows = Gtk::Adjustment::create(_pCtMainWin->get_ct_config()->tableRows, 1, 10000, 1);
    auto spinbutton_rows = Gtk::SpinButton(adj_rows);
    spinbutton_rows.set_value(_pCtMainWin->get_ct_config()->tableRows);
    auto label_columns = Gtk::Label(_("Columns"));
    auto adj_columns = Gtk::Adjustment::create(_pCtMainWin->get_ct_config()->tableColumns, 1, 10000, 1);
    auto spinbutton_columns = Gtk::SpinButton(adj_columns);
    spinbutton_columns.set_value(_pCtMainWin->get_ct_config()->tableColumns);

    auto hbox_rows_cols = Gtk::HBox();
    hbox_rows_cols.pack_start(label_rows, false, false);
    hbox_rows_cols.pack_start(spinbutton_rows, false, false);
    hbox_rows_cols.pack_start(label_columns, false, false);
    hbox_rows_cols.pack_start(spinbutton_columns, false, false);
    hbox_rows_cols.set_spacing(5);
    auto size_align = Gtk::Alignment();
    size_align.set_padding(6, 6, 6, 6);
    size_align.add(hbox_rows_cols);

    auto size_frame = Gtk::Frame(std::string("<b>")+_("Table Size")+"</b>");
    dynamic_cast<Gtk::Label*>(size_frame.get_label_widget())->set_use_markup(true);
    size_frame.set_shadow_type(Gtk::SHADOW_NONE);
    size_frame.add(size_align);

    auto label_col_min = Gtk::Label(_("Min Width"));
    auto adj_col_min = Gtk::Adjustment::create(_pCtMainWin->get_ct_config()->tableColMin, 1, 10000, 1);
    auto spinbutton_col_min = Gtk::SpinButton(adj_col_min);
    spinbutton_col_min.set_value(_pCtMainWin->get_ct_config()->tableColMin);
    auto label_col_max = Gtk::Label(_("Max Width"));
    auto adj_col_max = Gtk::Adjustment::create(_pCtMainWin->get_ct_config()->tableColMax, 1, 10000, 1);
    auto spinbutton_col_max = Gtk::SpinButton(adj_col_max);
    spinbutton_col_max.set_value(_pCtMainWin->get_ct_config()->tableColMax);

    auto hbox_col_min_max = Gtk::HBox();
    hbox_col_min_max.pack_start(label_col_min, false, false);
    hbox_col_min_max.pack_start(spinbutton_col_min, false, false);
    hbox_col_min_max.pack_start(label_col_max, false, false);
    hbox_col_min_max.pack_start(spinbutton_col_max, false, false);
    hbox_col_min_max.set_spacing(5);
    auto col_min_max_align = Gtk::Alignment();
    col_min_max_align.set_padding(6, 6, 6, 6);
    col_min_max_align.add(hbox_col_min_max);

    auto col_min_max_frame = Gtk::Frame(std::string("<b>")+_("Column Properties")+"</b>");
    dynamic_cast<Gtk::Label*>(col_min_max_frame.get_label_widget())->set_use_markup(true);
    col_min_max_frame.set_shadow_type(Gtk::SHADOW_NONE);
    col_min_max_frame.add(col_min_max_align);

    auto checkbutton_table_ins_from_file = Gtk::CheckButton(_("Import from CSV File"));

    auto content_area = dialog.get_content_area();
    content_area->set_spacing(5);
    if (is_insert) content_area->pack_start(size_frame);
    content_area->pack_start(col_min_max_frame);
    if (is_insert) content_area->pack_start(checkbutton_table_ins_from_file);
    content_area->show_all();

    checkbutton_table_ins_from_file.signal_toggled().connect([&](){
        size_frame.set_sensitive(!checkbutton_table_ins_from_file.get_active());
        col_min_max_frame.set_sensitive(!checkbutton_table_ins_from_file.get_active());
    });
    spinbutton_col_min.signal_changed().connect([&] {
        if (spinbutton_col_min.get_value_as_int() > spinbutton_col_max.get_value_as_int())
            spinbutton_col_max.set_value(spinbutton_col_min.get_value_as_int());
    });
    spinbutton_col_max.signal_changed().connect([&] {
        if (spinbutton_col_max.get_value_as_int() < spinbutton_col_min.get_value_as_int())
            spinbutton_col_min.set_value(spinbutton_col_max.get_value_as_int());
    });

    dialog.run();
    if (dialog.run() == Gtk::RESPONSE_ACCEPT)
    {
        _pCtMainWin->get_ct_config()->tableRows = spinbutton_rows.get_value_as_int();
        _pCtMainWin->get_ct_config()->tableColumns = spinbutton_columns.get_value_as_int();
        _pCtMainWin->get_ct_config()->tableColMin = spinbutton_col_min.get_value_as_int();
        _pCtMainWin->get_ct_config()->tableColMax = spinbutton_col_max.get_value_as_int();
        if (checkbutton_table_ins_from_file.get_active())
            return 2;
        return 1;
    }
    return 0;
}
