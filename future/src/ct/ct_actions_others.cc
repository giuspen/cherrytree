/*
 * ct_actions_others.cc
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
#include <gtkmm/stock.h>

// Cut Anchor
void CtActions::anchor_cut()
{
    object_set_selection(curr_anchor_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

// Copy Anchor
void CtActions::anchor_copy()
{
    object_set_selection(curr_anchor_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

// Delete Anchor
void CtActions::anchor_delete()
{
    object_set_selection(curr_anchor_anchor);
    curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    _pCtMainWin->get_text_view().grab_focus();
}

// Edit an Anchor
void CtActions::anchor_edit()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    Gtk::TextIter iter_insert = curr_buffer()->get_iter_at_child_anchor(curr_anchor_anchor->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_insert;
    iter_bound.forward_char();
    _anchor_edit_dialog(curr_anchor_anchor, iter_insert, &iter_bound);
}

void CtActions::embfile_cut()
{
  // todo:
}

void CtActions::embfile_copy()
{
  // todo:
}

void CtActions::embfile_delete()
{
  // todo:
}

void CtActions::embfile_save()
{
  // todo:
}

void CtActions::embfile_open()
{
  // todo:
}

void CtActions::image_save()
{
  // todo:
}

void CtActions::image_edit()
{
  // todo:
}

void CtActions::image_cut()
{
  // todo:
}

void CtActions::image_copy()
{
  // todo:
}

void CtActions::image_delete()
{
  // todo:
}

void CtActions::image_link_edit()
{
  // todo:
}

void CtActions::image_link_dismiss()
{
  // todo:
}

void CtActions::toggle_show_hide_main_window()
{
  // todo:
}

// Anchor Edit Dialog
void CtActions::_anchor_edit_dialog(CtImageAnchor* anchor, Gtk::TextIter insert_iter, Gtk::TextIter* iter_bound)
{
    Glib::ustring dialog_title = anchor->getAnchorName().empty() ? _("Insert Anchor") :  _("Edit Anchor");
    Glib::ustring ret_anchor_name = ct_dialogs::img_n_entry_dialog(*_pCtMainWin, dialog_title.c_str(), anchor->getAnchorName(), "anchor");
    if (ret_anchor_name.empty()) return;

    Glib::ustring image_justification;
    if (iter_bound) // only in case of modify
    {
        image_justification = _get_iter_alignment(insert_iter);
        int image_offset = insert_iter.get_offset();
        curr_buffer()->erase(insert_iter, *iter_bound);
        insert_iter = curr_buffer()->get_iter_at_offset(image_offset);
    }
    image_insert_anchor(insert_iter, ret_anchor_name, image_justification);
}
