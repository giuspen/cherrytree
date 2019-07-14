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
#include <fstream>

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

// Cut Embedded File
void CtActions::embfile_cut()
{
    object_set_selection(curr_file_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

// Copy Embedded File
void CtActions::embfile_copy()
{
    object_set_selection(curr_file_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

// Delete Embedded File
void CtActions::embfile_delete()
{
    object_set_selection(curr_file_anchor);
    curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    _pCtMainWin->get_text_view().grab_focus();
}

// Embedded File Save Dialog
void CtActions::embfile_save()
{
    ct_dialogs::file_select_args args = {.parent=_pCtMainWin, .curr_folder=CtApp::P_ctCfg->pickDirFile, .curr_file_name=curr_file_anchor->getFileName()};
    Glib::ustring filepath = ct_dialogs::file_save_as_dialog(args);
    if (filepath.empty()) return;

    CtApp::P_ctCfg->pickDirFile = CtFileSystem::dirname(filepath);
    g_file_set_contents(filepath.c_str(), curr_file_anchor->getRawBlob().c_str(), (gssize)curr_file_anchor->getRawBlob().size(), nullptr);
}

// Embedded File Open
void CtActions::embfile_open()
{
    size_t open_id = (size_t)curr_file_anchor->get_data("open_id");
    if (open_id == 0)
    {
        open_id = _next_opened_emb_file_id;
        _next_opened_emb_file_id += 1;
        curr_file_anchor->set_data("open_id", (void*)open_id);
    }

    Glib::ustring filename = std::to_string(_pCtMainWin->curr_tree_iter().get_node_id()) +
            CtConst::CHAR_MINUS + std::to_string(open_id) +
            CtConst::CHAR_MINUS + std::to_string(getpid())+
            CtConst::CHAR_MINUS + curr_file_anchor->getFileName();
    Glib::ustring filepath = CtApp::P_ctTmp->getHiddenFilePath(filename);
    std::fstream file(filepath, std::ios::out | std::ios::binary);
    long size = (long)curr_file_anchor->getRawBlob().size();
    file.write(curr_file_anchor->getRawBlob().c_str(), size);
    file.close();

    std::cout << "embfile_open " << filepath << std::endl;

    CtFileSystem::external_filepath_open(filepath, false);
    _embfiles_opened[filepath] = CtFileSystem::getmtime(filepath);

    if (!_embfiles_timeout_connection)
        _embfiles_timeout_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &CtActions::_on_embfiles_sentinel_timeout), 500);
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

// Iteration of the Modification Time Sentinel
bool CtActions::_on_embfiles_sentinel_timeout()
{
    for(auto& item: _embfiles_opened)
    {
        const Glib::ustring& filepath = item.first;
        if (!CtFileSystem::isfile(filepath))
        {
            std::cout << "embdrop" << filepath;
            _embfiles_opened.erase(filepath);
            break;
        }
        if (item.second != CtFileSystem::getmtime(filepath))
        {
           _embfiles_opened[filepath] = CtFileSystem::getmtime(filepath);
           auto data_vec = str::split(CtFileSystem::basename(filepath), CtConst::CHAR_MINUS);
           gint64 node_id = std::stoll(data_vec[0]);
           size_t embfile_id = std::stol(data_vec[1]);

           CtTreeIter tree_iter = _pCtMainWin->get_tree_store().get_tree_iter_from_node_id(node_id);
           if (!tree_iter) continue;
           if (tree_iter.get_node_read_only())
           {
               ct_dialogs::warning_dialog(_("Cannot Edit Embedded File in Read Only Node"), *_pCtMainWin);
               continue;
           }
           _pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
           for (auto& widget: tree_iter.get_all_embedded_widgets())
           {
                if (CtImageEmbFile* embFile = dynamic_cast<CtImageEmbFile*>(widget))
                    if (((size_t)embFile->get_data("open_id")) == embfile_id)
                    {
                        auto file = std::fstream(filepath, std::ios::in | std::ios::binary);
                        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
                        file.close();
                        embFile->setRawBlob(buffer.data(), buffer.size());
                        embFile->setTime(std::time(nullptr));
                        embFile->updateTooltip();

                        _pCtMainWin->update_window_save_needed("nbuf");
                        _pCtMainWin->get_status_bar().update_status(_("Embedded File Automatically Updated:") + CtConst::CHAR_SPACE + embFile->getFileName());
                        break;
                    }
           }
        }
    }
    return true; // this way we keep the timer alive
}
