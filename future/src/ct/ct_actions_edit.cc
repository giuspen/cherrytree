/*
 * ct_actions_find.cc
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

#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include "ct_clipboard.h"
#include "ct_list.h"
#include "ct_image.h"

// A Special character insert was Requested
void CtActions::insert_spec_char_action(gunichar ch)
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    proof.text_buffer->insert_at_cursor(Glib::ustring(1, ch));
}

void CtActions::requested_step_back()
{
    // todo
}

void CtActions::requested_step_ahead()
{
    // todo
}

// Insert/Edit Image
void CtActions::image_handle()
{
    if (!_node_sel_and_rich_text()) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    ct_dialogs::file_select_args args = {.parent=_pCtMainWin, .curr_folder=CtApp::P_ctCfg->pickDirImg};
    Glib::ustring filename = ct_dialogs::file_select_dialog(args);
    if (filename.empty()) return;
    // todo: self.pick_dir_img = os.path.dirname(filename)

    auto pixbuf = Gdk::Pixbuf::create_from_file(filename);
    if (pixbuf)
        _image_edit_dialog(pixbuf, curr_buffer()->get_insert()->get_iter(), nullptr);
    else
        ct_dialogs::error_dialog(_("Image Format Not Recognized"), *_pCtMainWin);
}

void CtActions::table_handle()
{
    // todo:
}

void CtActions::codebox_handle()
{
    // todo:
}

void CtActions::embfile_insert()
{
    // todo:
}

// The Link Insert Button was Pressed
void CtActions::apply_tag_link()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _apply_tag(CtConst::TAG_LINK);
}

void CtActions::anchor_handle()
{
    // todo:
}

void CtActions::toc_insert()
{
    // todo:
}

// Insert Timestamp
void CtActions::timestamp_insert()
{
    if (!_is_curr_node_not_read_only_or_error()) return;

    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;

    time_t time = std::time(nullptr);
    std::string timestamp = str::time_format(CtApp::P_ctCfg->timestampFormat, time);
    proof.text_buffer->insert_at_cursor(timestamp);
}

// Insert a Horizontal Line
void CtActions::horizontal_rule_insert()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    proof.text_buffer->insert_at_cursor(CtApp::P_ctCfg->hRule + CtConst::CHAR_NEWLINE);
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
    if (!_is_curr_node_not_read_only_or_error()) return;
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
    if (!proof.text_buffer) return;
    if (!_is_curr_node_not_read_only_or_error()) return;

    CtTextRange range = CtList(proof.text_buffer).get_paragraph_iters();
    if (!range.iter_end.forward_char() && !range.iter_start.backward_char()) return;
    proof.text_buffer->select_range(range.iter_start, range.iter_end);
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "cut-clipboard");
}

// Copy a Whole Row
void CtActions::text_row_copy()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;

    CtTextRange range = CtList(proof.text_buffer).get_paragraph_iters();
    if (!range.iter_end.forward_char() && !range.iter_start.backward_char()) return;
    proof.text_buffer->select_range(range.iter_start, range.iter_end);
    g_signal_emit_by_name(G_OBJECT(proof.text_view->gobj()), "copy-clipboard");
}

// Deletes the Whole Row
void CtActions::text_row_delete()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (!_is_curr_node_not_read_only_or_error()) return;

    CtTextRange range = CtList(proof.text_buffer).get_paragraph_iters();
    if (!range.iter_end.forward_char() && !range.iter_start.backward_char()) return;
    proof.text_buffer->erase(range.iter_start, range.iter_end);
    // todo: self.state_machine.update_state()
}

// Duplicates the Whole Row or a Selection
void CtActions::text_row_selection_duplicate()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    auto text_buffer = proof.text_buffer;
    if (proof.text_buffer->get_has_selection())
    {
        Gtk::TextIter iter_start, iter_end;
        text_buffer->get_selection_bounds(iter_start, iter_end);
        int sel_start_offset = iter_start.get_offset();
        int sel_end_offset = iter_end.get_offset();
        if (proof.from_codebox || proof.syntax_highl != CtConst::RICH_TEXT_ID)
        {
            Glib::ustring text_to_duplicate = text_buffer->get_text(iter_start, iter_end);
            if (text_to_duplicate.find(CtConst::CHAR_NEWLINE) != Glib::ustring::npos)
                text_to_duplicate = CtConst::CHAR_NEWLINE + text_to_duplicate;
            text_buffer->insert(iter_end, text_to_duplicate);
        }
        else
        {
            Glib::ustring rich_text = CtClipboard().rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer, iter_start, iter_end);
            if (rich_text.find(CtConst::CHAR_NEWLINE) != Glib::ustring::npos)
            {
                text_buffer->insert(iter_end, CtConst::CHAR_NEWLINE);
                iter_end = proof.text_buffer->get_iter_at_offset(sel_end_offset+1);
                text_buffer->move_mark(proof.text_buffer->get_insert(), iter_end);
            }
            CtClipboard().from_xml_string_to_buffer(text_buffer, rich_text);
        }
        text_buffer->select_range(text_buffer->get_iter_at_offset(sel_start_offset),
                                 text_buffer->get_iter_at_offset(sel_end_offset));
    }
    else
    {
        int cursor_offset = text_buffer->get_iter_at_mark(text_buffer->get_insert()).get_offset();
        CtTextRange range = CtList(proof.text_buffer).get_paragraph_iters();
        if (range.iter_start.get_offset() == range.iter_end.get_offset())
        {
            Gtk::TextIter iter_start = text_buffer->get_iter_at_mark(text_buffer->get_insert());
            text_buffer->insert(iter_start, CtConst::CHAR_NEWLINE);
        }
        else
        {
            if (proof.from_codebox || proof.syntax_highl != CtConst::RICH_TEXT_ID)
            {
                Glib::ustring text_to_duplicate = text_buffer->get_text(range.iter_start, range.iter_end);
                text_buffer->insert(range.iter_end, CtConst::CHAR_NEWLINE + text_to_duplicate);
            }
            else
            {
                Glib::ustring rich_text = CtClipboard().rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer, range.iter_start, range.iter_end);
                int sel_end_offset = range.iter_end.get_offset();
                text_buffer->insert(range.iter_end, CtConst::CHAR_NEWLINE);
                range.iter_end = text_buffer->get_iter_at_offset(sel_end_offset+1);
                text_buffer->move_mark(text_buffer->get_insert(), range.iter_end);
                CtClipboard().from_xml_string_to_buffer(proof.text_buffer, rich_text);
                text_buffer->place_cursor(text_buffer->get_iter_at_offset(cursor_offset));
            }
        }
    }
    // todo: self.state_machine.update_state()
}

// Moves Up the Current Row/Selected Rows
void CtActions::text_row_up()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (!_is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_buffer;
    CtTextRange range = CtList(text_buffer).get_paragraph_iters();
    int last_line_situation = false;
    range.iter_end.forward_char();
    bool missing_leading_newline = false;
    Gtk::TextIter destination_iter = range.iter_start;

    if (!destination_iter.backward_char()) return;
    if (!destination_iter.backward_char())
        missing_leading_newline = true;
    else
    {
        while (destination_iter.get_char() != CtConst::CHAR_NEWLINE[0])
            if (!destination_iter.backward_char())
            {
                missing_leading_newline = true;
                break;
            }
    }
    if (!missing_leading_newline) destination_iter.forward_char();
    int destination_offset = destination_iter.get_offset();
    int start_offset = range.iter_start.get_offset();
    int end_offset = range.iter_end.get_offset();
    //#print "iter_start %s %s '%s'" % (start_offset, ord(iter_start.get_char()), iter_start.get_char())
    //#print "iter_end %s %s '%s'" % (end_offset, ord(iter_end.get_char()), iter_end.get_char())
    //#print "destination_iter %s %s '%s'" % (destination_offset, ord(destination_iter.get_char()), destination_iter.get_char())
    Glib::ustring text_to_move = text_buffer->get_text(range.iter_start, range.iter_end);
    int diff_offsets = end_offset - start_offset;
    if (proof.from_codebox || proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (text_to_move.empty() || text_to_move[text_to_move.length()-1] != CtConst::CHAR_NEWLINE[0])
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
        Glib::ustring rich_text = CtClipboard().rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(),
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
        if (text_to_move.empty() || text_to_move[text_to_move.length()-1] != CtConst::CHAR_NEWLINE[0])
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
        CtClipboard().from_xml_string_to_buffer(text_buffer, rich_text);
        if (append_newline)
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        // clear space trick
        Gtk::TextIter cursor_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        text_buffer->erase(cursor_iter, text_buffer->get_iter_at_offset(cursor_iter.get_offset()+1));
        // selection
        proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
    }
    // todo: self.state_machine.update_state()
}

// Moves Down the Current Row/Selected Rows
void CtActions::text_row_down()
{
    auto proof = _get_text_view_n_buffer_codebox_proof();
    if (!proof.text_buffer) return;
    if (!_is_curr_node_not_read_only_or_error()) return;

    auto text_buffer = proof.text_buffer;
    CtTextRange range = CtList(text_buffer).get_paragraph_iters();
    if (!range.iter_end.forward_char()) return;
    int missing_leading_newline = false;
    Gtk::TextIter destination_iter = range.iter_end;
    while (destination_iter.get_char() != CtConst::CHAR_NEWLINE[0])
        if (!destination_iter.forward_char())
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
    if (proof.from_codebox || proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_buffer->erase(range.iter_start, range.iter_end);
        destination_offset -= diff_offsets;
        destination_iter = text_buffer->get_iter_at_offset(destination_offset);
        if (text_to_move.empty() || text_to_move[text_to_move.length() - 1] != CtConst::CHAR_NEWLINE[0])
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
        if (!missing_leading_newline)
            proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
        else
            proof.text_view->set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2);
    }
    else
    {
        Glib::ustring rich_text = CtClipboard().rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer,
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
        if (text_to_move.empty() || text_to_move[text_to_move.length()-1] != CtConst::CHAR_NEWLINE[0])
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
        CtClipboard().from_xml_string_to_buffer(text_buffer, rich_text);
        if (append_newline)
            text_buffer->insert_at_cursor(CtConst::CHAR_NEWLINE);
        // clear space trick
        Gtk::TextIter cursor_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        text_buffer->erase(cursor_iter, text_buffer->get_iter_at_offset(cursor_iter.get_offset()+1));
        // selection
        if (!missing_leading_newline)
            proof.text_view->set_selection_at_offset_n_delta(destination_offset, diff_offsets-1);
        else
            proof.text_view->set_selection_at_offset_n_delta(destination_offset+1, diff_offsets-2);
    }
    // self.state_machine.update_state()
}

// Remove trailing spaces/tabs
void CtActions::strip_trailing_spaces()
{
    Glib::RefPtr<Gtk::TextBuffer> text_buffer = curr_buffer();
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
                if (curr_char == CtConst::CHAR_SPACE[0] || curr_char ==  CtConst::CHAR_TAB[0])
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
                else if (curr_char != CtConst::CHAR_SPACE[0] && curr_char !=  CtConst::CHAR_TAB[0])
                {
                    curr_state = 0;
                }
            }
            if (!curr_iter.forward_char())
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

    ct_dialogs::info_dialog(std::to_string(cleaned_lines) + " " + _("Lines Stripped"), *_pCtMainWin);
}

// Insert/Edit Image Dialog
void CtActions::_image_edit_dialog(Glib::RefPtr<Gdk::Pixbuf> pixbuf, Gtk::TextIter insert_iter, Gtk::TextIter* iter_bound)
{
    Glib::RefPtr<Gdk::Pixbuf> ret_pixbuf = ct_dialogs::image_handle_dialog(*_pCtMainWin, _("Image Properties"), pixbuf);
    if (!ret_pixbuf) return;
    Glib::ustring link = "";
    Glib::ustring image_justification;
    if (iter_bound) { // only in case of modify
        image_justification = _get_iter_alignment(insert_iter);
        int image_offset = insert_iter.get_offset();
        curr_buffer()->erase(insert_iter, *iter_bound);
        insert_iter = curr_buffer()->get_iter_at_offset(image_offset);
    }
    image_insert(insert_iter, ret_pixbuf, link, image_justification);
}

// Get the Alignment Value of the given Iter
Glib::ustring CtActions::_get_iter_alignment(Gtk::TextIter text_iter)
{
    auto align_center = apply_tag_exist_or_create(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_CENTER);
    if (text_iter.has_tag(CtApp::R_textTagTable->lookup(align_center)))
        return CtConst::TAG_PROP_VAL_CENTER;
    auto align_fill = apply_tag_exist_or_create(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_FILL);
    if (text_iter.has_tag(CtApp::R_textTagTable->lookup(align_fill)))
        return CtConst::TAG_PROP_VAL_FILL;
    auto align_right = apply_tag_exist_or_create(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_RIGHT);
    if (text_iter.has_tag(CtApp::R_textTagTable->lookup(align_right)))
        return CtConst::TAG_PROP_VAL_RIGHT;
    return CtConst::TAG_PROP_VAL_LEFT;
}

void CtActions::image_insert(Gtk::TextIter iter_insert, Glib::RefPtr<Gdk::Pixbuf> pixbuf, Glib::ustring link,
                              Glib::ustring image_justification /*=""*/, Glib::RefPtr<Gtk::TextBuffer> text_buffer /*= Glib:RefPtr<Gtk::TextBuffer>()*/)
{
    if (!pixbuf) return;
    if (!text_buffer) text_buffer = curr_buffer();
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(text_buffer);

    int charOffset = iter_insert.get_offset();

    CtAnchoredWidget* pAnchoredWidget = new CtImagePng(pixbuf, link, charOffset, image_justification);
    pAnchoredWidget->insertInTextBuffer(gsv_buffer);

    getCtMainWin()->get_tree_store().addAnchoredWidgets(getCtMainWin()->curr_tree_iter(),
        {pAnchoredWidget}, &getCtMainWin()->get_text_view());
}

// Change the Case of the Selected Text/the Underlying Word"""
void CtActions::_text_selection_change_case(gchar change_type)
{
    text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
    Glib::RefPtr<Gtk::TextBuffer> text_buffer = proof.text_buffer;
    if (!text_buffer) return;
    if (!_is_curr_node_not_read_only_or_error()) return;
    if (!text_buffer->get_has_selection() && !_apply_tag_try_automatic_bounds(text_buffer, text_buffer->get_insert()->get_iter()))
    {
        ct_dialogs::warning_dialog(_("No Text is Selected"), *_pCtMainWin);
        return;
    }

    Gtk::TextIter iter_start, iter_end;
    text_buffer->get_selection_bounds(iter_start, iter_end);
    Glib::ustring text_to_change_case, rich_text;
    if (proof.from_codebox || proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_to_change_case = text_buffer->get_text(iter_start, iter_end);
        if (change_type == 'l')         text_to_change_case = text_to_change_case.lowercase();
        else if (change_type == 'u')    text_to_change_case = text_to_change_case.uppercase();
        else if (change_type == 't')    text_to_change_case = str::swapcase(text_to_change_case);
    }
    else
    {
        rich_text = CtClipboard().rich_text_get_from_text_buffer_selection(_pCtMainWin->curr_tree_iter(), text_buffer, iter_start, iter_end, change_type);
    }

    int start_offset = iter_start.get_offset();
    int end_offset = iter_end.get_offset();
    text_buffer->erase(iter_start, iter_end);
    Gtk::TextIter iter_insert = text_buffer->get_iter_at_offset(start_offset);
    if (proof.from_codebox || proof.syntax_highl != CtConst::RICH_TEXT_ID)
    {
        text_buffer->insert(iter_insert, text_to_change_case);
    }
    else
    {
        text_buffer->move_mark(text_buffer->get_insert(), iter_insert);
        CtClipboard().from_xml_string_to_buffer(text_buffer, rich_text);
    }
    text_buffer->select_range(text_buffer->get_iter_at_offset(start_offset),
                              text_buffer->get_iter_at_offset(end_offset));
}
