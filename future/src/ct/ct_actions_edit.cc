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

void CtActions::cut_as_plain_text()
{
    // todo:
}

void CtActions::copy_as_plain_text()
{
    // todo:
}

void CtActions::paste_as_plain_text()
{
    // todo:
}

void CtActions::text_row_cut()
{

}

void CtActions::text_row_copy()
{

}

void CtActions::text_row_delete()
{

}

void CtActions::text_row_selection_duplicate()
{

}

void CtActions::text_row_up()
{

}

void CtActions::text_row_down()
{

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
    // todo: what is that? ret_pixbuf.link = "";
    Glib::ustring image_justification;
    if (iter_bound) { // only in case of modify
        image_justification = _get_iter_alignment(insert_iter);
        int image_offset = insert_iter.get_offset();
        curr_buffer()->erase(insert_iter, *iter_bound);
        insert_iter = curr_buffer()->get_iter_at_offset(image_offset);
    }
    _image_insert(insert_iter, ret_pixbuf, image_justification);
}

// Get the Alignment Value of the given Iter
Glib::ustring CtActions::_get_iter_alignment(Gtk::TextIter text_iter)
{
    auto align_center = _apply_tag_exist_or_create(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_CENTER);
    if (text_iter.has_tag(CtApp::R_textTagTable->lookup(align_center)))
        return CtConst::TAG_PROP_VAL_CENTER;
    auto align_fill = _apply_tag_exist_or_create(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_FILL);
    if (text_iter.has_tag(CtApp::R_textTagTable->lookup(align_fill)))
        return CtConst::TAG_PROP_VAL_FILL;
    auto align_right = _apply_tag_exist_or_create(CtConst::TAG_JUSTIFICATION, CtConst::TAG_PROP_VAL_RIGHT);
    if (text_iter.has_tag(CtApp::R_textTagTable->lookup(align_right)))
        return CtConst::TAG_PROP_VAL_RIGHT;
    return CtConst::TAG_PROP_VAL_LEFT;
}

void CtActions::_image_insert(Gtk::TextIter iter_insert, Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                              Glib::ustring image_justification /*=""*/, Glib::RefPtr<Gtk::TextBuffer> text_buffer /*= Glib:RefPtr<Gtk::TextBuffer>()*/)
{
    if (!pixbuf) return;
    if (!text_buffer) text_buffer = curr_buffer();
    int image_offset = iter_insert.get_offset();
    /* todo:
    auto anchor = text_buffer->create_child_anchor(iter_insert);
    anchor.pixbuf = pixbuf
    anchor.eventbox = gtk.EventBox()
    anchor.eventbox.set_visible_window(False)
    anchor.frame = gtk.Frame()
    anchor.frame.set_shadow_type(gtk.SHADOW_NONE)
    pixbuf_attrs = dir(pixbuf)
    if not hasattr(anchor.pixbuf, "link"): anchor.pixbuf.link = ""
    if "anchor" in pixbuf_attrs:
        anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_anchor, anchor)
        anchor.eventbox.set_tooltip_text(pixbuf.anchor)
    elif "filename" in pixbuf_attrs:
        anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_file, anchor)
        self.embfile_set_tooltip(anchor)
        if self.embfile_show_filename:
            anchor_label = gtk.Label()
            anchor_label.set_markup("<b><small>"+pixbuf.filename+"</small></b>")
            anchor_label.modify_fg(gtk.STATE_NORMAL, gtk.gdk.color_parse(self.rt_def_fg))
            anchor.frame.set_label_widget(anchor_label)
    else:
        anchor.eventbox.connect("button-press-event", self.on_mouse_button_clicked_image, anchor)
        anchor.eventbox.connect("visibility-notify-event", self.on_image_visibility_notify_event)
        if anchor.pixbuf.link:
            self.image_link_apply_frame_label(anchor)
    anchor.image = gtk.Image()
    anchor.frame.add(anchor.image)
    anchor.eventbox.add(anchor.frame)
    anchor.image.set_from_pixbuf(anchor.pixbuf)
    self.sourceview.add_child_at_anchor(anchor.eventbox, anchor)
    anchor.eventbox.show_all()
    if image_justification:
        text_iter = text_buffer.get_iter_at_offset(image_offset)
        self.state_machine.apply_object_justification(text_iter, image_justification, text_buffer)
    elif self.user_active:
        # if I apply a justification, the state is already updated
        self.state_machine.update_state()
        */
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
