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
#include "ct_export2html.h"
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
    curr_anchor_anchor = nullptr;
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
    curr_file_anchor = nullptr;
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

// Save to Disk the selected Image
void CtActions::image_save()
{
    ct_dialogs::file_select_args args = {.parent=_pCtMainWin, .curr_folder=CtApp::P_ctCfg->pickDirImg, .filter_name=_("PNG Image"), .filter_pattern={"*.png"}};
    Glib::ustring filename = ct_dialogs::file_save_as_dialog(args);
    if (filename.empty()) return;

    CtApp::P_ctCfg->pickDirImg = CtFileSystem::dirname(filename);
    if (!str::endswith(filename, ".png")) filename += ".png";
    try {
       curr_image_anchor->save(filename, "png");
    }
    catch (...) {
        ct_dialogs::error_dialog(str::format(_("Write to %s Failed"), std::string(filename)), *_pCtMainWin);
    }
}

// Edit the selected Image
void CtActions::image_edit()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    Gtk::TextIter iter_insert = curr_buffer()->get_iter_at_child_anchor(curr_image_anchor->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_insert;
    iter_bound.forward_char();
    _image_edit_dialog(curr_image_anchor->getPixBuf(), iter_insert, &iter_bound);
}

// Cut Image
void CtActions::image_cut()
{
    object_set_selection(curr_image_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

// Copy Image
void CtActions::image_copy()
{
    object_set_selection(curr_image_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

// Delete Image
void CtActions::image_delete()
{
    object_set_selection(curr_image_anchor);
    curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_image_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
}

// Edit the Link Associated to the Image
void CtActions::image_link_edit()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    _link_entry = ct_dialogs::CtLinkEntry();
    if  (curr_image_anchor->getLink().empty())
        _link_entry.type = CtConst::LINK_TYPE_WEBS; // default value
    else if (!_links_entries_pre_dialog(curr_image_anchor->getLink(), _link_entry))
       return;
    CtTreeIter sel_tree_iter = _pCtTreestore->get_node_from_node_id(_link_entry.node_id);
    if (!ct_dialogs::link_handle_dialog(*_pCtMainWin, _("Insert/Edit Link"), sel_tree_iter, _link_entry))
        return;
    Glib::ustring property_value = _links_entries_post_dialog(_link_entry);
    if (!property_value.empty())
    {
        curr_image_anchor->setLink(property_value);
        curr_image_anchor->updateLabelWidget();
        // todo: self.objects_buffer_refresh()
        _pCtMainWin->update_window_save_needed("nbuf", true);
    }
}

// Dismiss the Link Associated to the Image
void CtActions::image_link_dismiss()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    curr_image_anchor->setLink("");
    curr_image_anchor->updateLabelWidget();
    _pCtMainWin->update_window_save_needed("nbuf", true);
}

void CtActions::toggle_show_hide_main_window()
{
  // todo:
}

// Function Called at Every Link Click
void CtActions::link_clicked(const Glib::ustring& tag_property_value, bool from_wheel)
{
     auto vec = str::split(tag_property_value, " ");
     if (vec[0] == CtConst::LINK_TYPE_WEBS) // link to webpage
     {
         Glib::ustring clean_weblink = str::replace(vec[1], "amp;", "");
         if (CtApp::P_ctCfg->weblinkCustomOn)
         {
             // todo: subprocess.call(self.weblink_custom_action[1] % clean_weblink, shell=True)
         }
         else g_app_info_launch_default_for_uri(clean_weblink.c_str(), nullptr, nullptr); // todo: ?
     }
     else if (vec[0] == CtConst::LINK_TYPE_FILE) // link to file
     {
         Glib::ustring filepath = CtExport2Html::_link_process_filepath(vec[1]);
         if (!CtFileSystem::isfile(filepath))
         {
             ct_dialogs::error_dialog(str::format(_("The File Link '%s' is Not Valid"), std::string(filepath)), *_pCtMainWin);
             return;
         }
         if (from_wheel)
             filepath = CtFileSystem::dirname(CtFileSystem::abspath(filepath));
         CtFileSystem::external_filepath_open(filepath, true);
     }
     else if (vec[0] == CtConst::LINK_TYPE_FOLD) // link to folder
     {
         Glib::ustring folderpath = CtExport2Html::_link_process_folderpath(vec[1]);
         if (!CtFileSystem::isdir(folderpath))
         {
             ct_dialogs::error_dialog(str::format(_("The Folder Link '%s' is Not Valid"), std::string(folderpath)), *_pCtMainWin);
             return;
         }
         if (from_wheel)
             folderpath = CtFileSystem::dirname(CtFileSystem::abspath(folderpath));
         CtFileSystem::external_folderpath_open(folderpath);
     }
     else if (vec[0] == CtConst::LINK_TYPE_NODE) // link to a tree node
     {
         CtTreeIter tree_iter = _pCtTreestore->get_node_from_node_id(std::stol(vec[1]));
         if (!tree_iter)
         {
             ct_dialogs::error_dialog(str::format(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)"), std::string(vec[1])), *_pCtMainWin);
             return;
         }
         _pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
         _pCtMainWin->get_text_view().grab_focus();
         // todo: self.sourceview.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor(gtk.gdk.Cursor(gtk.gdk.XTERM))
         _pCtMainWin->get_text_view().set_tooltip_text("");
         if (vec.size() >= 3)
         {
             Glib::ustring anchor_name;
             if (vec.size() == 3) anchor_name = vec[2];
             else anchor_name = tag_property_value.substr(vec[0].size() + vec[1].size() + 2);

             CtImageAnchor* imageAnchor = nullptr;
             for (auto& widget: tree_iter.get_all_embedded_widgets())
                 if (CtImageAnchor* anchor = dynamic_cast<CtImageAnchor*>(widget))
                     if (anchor->getAnchorName() == anchor_name)
                         imageAnchor = anchor;
             if (!imageAnchor)
             {
                 if (anchor_name.size() > (size_t)CtConst::MAX_TOOLTIP_LINK_CHARS)
                     anchor_name = anchor_name.substr(0, (size_t)CtConst::MAX_TOOLTIP_LINK_CHARS) + "...";
                 ct_dialogs::warning_dialog(str::format(_("No anchor named '%s' found"), std::string(anchor_name)), *_pCtMainWin);
             }
             else
             {
                 Gtk::TextIter iter_anchor = curr_buffer()->get_iter_at_child_anchor(imageAnchor->getTextChildAnchor());
                 curr_buffer()->place_cursor(iter_anchor);
                 _pCtMainWin->get_text_view().scroll_to(curr_buffer()->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
             }
         }
     }
     else
         ct_dialogs::error_dialog(str::format("Tag Name Not Recognized! (%s)", std::string(vec[0])), *_pCtMainWin);
}

// Cut CodeBox
void CtActions::codebox_cut()
{
    object_set_selection(curr_codebox_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

// Copy CodeBox
void CtActions::codebox_copy()
{
    object_set_selection(curr_codebox_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

// Delete CodeBox
void CtActions::codebox_delete()
{
    object_set_selection(curr_codebox_anchor);
    curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_codebox_anchor = nullptr;
   _pCtMainWin->get_text_view().grab_focus();
}

// Delete CodeBox but keep the Text
void CtActions::codebox_delete_keeping_text()
{
    if (!_is_curr_node_not_read_only_or_error()) return;
    Glib::ustring content = curr_codebox_anchor->getTextContent();
    object_set_selection(curr_codebox_anchor);
    curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_codebox_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
    curr_buffer()->insert_at_cursor(content);
}

void CtActions::codebox_change_properties()
{

}

void CtActions::exec_code()
{

}

void CtActions::codebox_load_from_file()
{

}

void CtActions::codebox_save_to_file()
{

}

// Increase CodeBox Width
void CtActions::codebox_increase_width()
{
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
    if (curr_codebox_anchor->getWidthInPixels())
         curr_codebox_anchor->setWidthHeight(curr_codebox_anchor->getFrameWidth() + CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX, 0);
     else
         curr_codebox_anchor->setWidthHeight(curr_codebox_anchor->getFrameWidth() + CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC, 0);
}

// Decrease CodeBox Width
void CtActions::codebox_decrease_width()
{
     if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
     if (curr_codebox_anchor->getWidthInPixels())
     {
         if (curr_codebox_anchor->getFrameWidth() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX >= CtCodebox::CB_WIDTH_LIMIT_MIN)
             curr_codebox_anchor->setWidthHeight(curr_codebox_anchor->getFrameWidth() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX, 0);
     }
     else
     {
         if (curr_codebox_anchor->getFrameWidth() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC >= CtCodebox::CB_WIDTH_LIMIT_MIN)
             curr_codebox_anchor->setWidthHeight(curr_codebox_anchor->getFrameWidth() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC, 0);
     }
}

// Increase CodeBox Height
void CtActions::codebox_increase_height()
{
     if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
     curr_codebox_anchor->setWidthHeight(0, curr_codebox_anchor->getFrameHeight() + CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX);
}

// Decrease CodeBox Height
void CtActions::codebox_decrease_height()
{
     if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
     if (curr_codebox_anchor->getFrameHeight() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX >= CtCodebox::CB_HEIGHT_LIMIT_MIN)
         curr_codebox_anchor->setWidthHeight(0, curr_codebox_anchor->getFrameHeight() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX);
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

           CtTreeIter tree_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(node_id);
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
