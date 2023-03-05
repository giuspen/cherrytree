/*
 * ct_actions_others.cc
 *
 * Copyright 2009-2023
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

#include "ct_actions.h"
#include "ct_export2html.h"
#include "ct_pref_dlg.h"
#include "ct_clipboard.h"
#include "ct_storage_control.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <cstdlib>
#include "ct_logging.h"
#include "ct_list.h"
#ifndef _WIN32
#include <sys/wait.h> // WEXITSTATUS __FreeBSD__ (#1550)
#endif // !_WIN32
#if defined(HAVE_VTE)
#include <vte/vte.h>
#endif // HAVE_VTE

void CtActions::link_cut()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (!_link_check_around_cursor().empty()) {
        g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
    }
}

void CtActions::link_copy()
{
    if (!_link_check_around_cursor().empty()) {
        g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
    }
}

void CtActions::link_dismiss()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (!_link_check_around_cursor().empty()) {
        remove_text_formatting();
    }
}

void CtActions::link_delete()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (!_link_check_around_cursor().empty()) {
        _curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
        _pCtMainWin->get_text_view().grab_focus();
    }
}

void CtActions::anchor_cut()
{
    object_set_selection(curr_anchor_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

void CtActions::anchor_copy()
{
    object_set_selection(curr_anchor_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

void CtActions::anchor_delete()
{
    object_set_selection(curr_anchor_anchor);
    _curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_anchor_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::anchor_edit()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    Gtk::TextIter iter_insert = _curr_buffer()->get_iter_at_child_anchor(curr_anchor_anchor->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_insert;
    iter_bound.forward_char();
    _anchor_edit_dialog(curr_anchor_anchor, iter_insert, &iter_bound);
}

void CtActions::anchor_link_to_clipboard()
{
    CtClipboard{_pCtMainWin}.anchor_link_to_clipboard(_pCtMainWin->curr_tree_iter(), curr_anchor_anchor->get_anchor_name());
}

void CtActions::embfile_cut()
{
    object_set_selection(curr_file_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

void CtActions::embfile_copy()
{
    object_set_selection(curr_file_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

void CtActions::embfile_delete()
{
    object_set_selection(curr_file_anchor);
    _curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_file_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::embfile_save()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtConfig->pickDirFile;
    args.curr_file_name = curr_file_anchor->get_file_name();

    std::string filepath = CtDialogs::file_save_as_dialog(args);
    if (filepath.empty()) return;

    _pCtConfig->pickDirFile = Glib::path_get_dirname(filepath);
    g_file_set_contents(filepath.c_str(), curr_file_anchor->get_raw_blob().c_str(), (gssize)curr_file_anchor->get_raw_blob().size(), nullptr);
}

void CtActions::embfile_open()
{
    const size_t open_id = curr_file_anchor->get_unique_id();
    auto mapIter = _embfiles_opened.find(open_id);
    fs::path tmp_filepath;
    if (mapIter == _embfiles_opened.end()) {
        // the file was not opened yet
        const fs::path filename = std::to_string(_pCtMainWin->curr_tree_iter().get_node_id()) +
                                                 CtConst::CHAR_MINUS + std::to_string(open_id) +
                                                 CtConst::CHAR_MINUS + std::to_string(getpid())+
                                                 CtConst::CHAR_MINUS + curr_file_anchor->get_file_name().string();
        tmp_filepath = _pCtMainWin->get_ct_tmp()->getHiddenFilePath(filename);
        _embfiles_opened[open_id] = CtEmbFileOpened{
            .tmp_filepath = tmp_filepath,
            .mod_time = 0};
        mapIter = _embfiles_opened.find(open_id);
    }
    else {
        tmp_filepath = mapIter->second.tmp_filepath;
    }

    g_file_set_contents(tmp_filepath.c_str(), curr_file_anchor->get_raw_blob().c_str(), (gssize)curr_file_anchor->get_raw_blob().size(), nullptr);
    fs::open_filepath(tmp_filepath.c_str(), false, _pCtConfig);
    mapIter->second.mod_time = fs::getmtime(tmp_filepath);

    if (not _embfiles_timeout_connection) {
        _embfiles_timeout_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &CtActions::_on_embfiles_sentinel_timeout), 500);
    }
}

void CtActions::embfile_rename()
{
    if (not _is_curr_node_not_read_only_or_error()) return;

    Glib::ustring name = curr_file_anchor->get_file_name().string();
    Glib::ustring ret_name = CtDialogs::img_n_entry_dialog(*_pCtMainWin, _("Rename"), name, "");
    if (ret_name.empty()) return;

    curr_file_anchor->set_file_name(ret_name.c_str());
    curr_file_anchor->update_label_widget();
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::latex_save()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtConfig->pickDirImg;
    args.curr_file_name = "";
    args.filter_name = _("PNG Image");
    args.filter_pattern = {"*.png"};

    std::string filename = CtDialogs::file_save_as_dialog(args);
    if (filename.empty()) return;

    _pCtConfig->pickDirImg = Glib::path_get_dirname(filename);
    if (not str::endswith(filename, ".png")) filename += ".png";
    try {
       curr_latex_anchor->save(filename, "png");
    }
    catch (...) {
        CtDialogs::error_dialog(str::format(_("Write to %s Failed"), str::xml_escape(filename)), *_pCtMainWin);
    }
}

void CtActions::latex_edit()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    Gtk::TextIter iter_insert = _curr_buffer()->get_iter_at_child_anchor(curr_latex_anchor->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_insert;
    iter_bound.forward_char();
    _latex_edit_dialog(curr_latex_anchor->get_latex_text(), iter_insert, &iter_bound);
}

void CtActions::latex_cut()
{
    object_set_selection(curr_latex_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

void CtActions::latex_copy()
{
    object_set_selection(curr_latex_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

void CtActions::latex_delete()
{
    object_set_selection(curr_latex_anchor);
    _curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_latex_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::image_save()
{
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtConfig->pickDirImg;
    args.curr_file_name = "";
    args.filter_name = _("PNG Image");
    args.filter_pattern = {"*.png"};

    std::string filename = CtDialogs::file_save_as_dialog(args);
    if (filename.empty()) return;

    _pCtConfig->pickDirImg = Glib::path_get_dirname(filename);
    if (not str::endswith(filename, ".png")) filename += ".png";
    try {
       curr_image_anchor->save(filename, "png");
    }
    catch (...) {
        CtDialogs::error_dialog(str::format(_("Write to %s Failed"), str::xml_escape(filename)), *_pCtMainWin);
    }
}

void CtActions::image_edit()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    Gtk::TextIter iter_insert = _curr_buffer()->get_iter_at_child_anchor(curr_image_anchor->getTextChildAnchor());
    Gtk::TextIter iter_bound = iter_insert;
    iter_bound.forward_char();
    _image_edit_dialog(curr_image_anchor->get_pixbuf(), iter_insert, &iter_bound);
}

void CtActions::image_cut()
{
    object_set_selection(curr_image_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

void CtActions::image_copy()
{
    object_set_selection(curr_image_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

void CtActions::image_delete()
{
    object_set_selection(curr_image_anchor);
    _curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_image_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::image_link_edit()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    _link_entry = CtLinkEntry();
    if (curr_image_anchor->get_link().empty()) {
        _link_entry.type = CtConst::LINK_TYPE_WEBS; // default value
    }
    else if (not _links_entries_pre_dialog(curr_image_anchor->get_link(), _link_entry)) {
        return;
    }
    CtTreeIter sel_tree_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(_link_entry.node_id);
    if (not CtDialogs::link_handle_dialog(*_pCtMainWin, _("Insert/Edit Link"), sel_tree_iter, _link_entry)) {
        return;
    }
    Glib::ustring property_value = _links_entries_post_dialog(_link_entry);
    if (not property_value.empty()) {
        curr_image_anchor->set_link(property_value);
        curr_image_anchor->update_label_widget();
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
    }
}

void CtActions::image_link_dismiss()
{
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_image_anchor->set_link("");
    curr_image_anchor->update_label_widget();
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::toggle_show_hide_main_window()
{
    _pCtMainWin->signal_app_show_hide_main_win();
}

void CtActions::link_clicked(const Glib::ustring& tag_property_value, bool from_wheel)
{
    CtLinkEntry link_entry = CtMiscUtil::get_link_entry(tag_property_value);
    if (link_entry.type == CtConst::LINK_TYPE_WEBS) { // link to webpage
        Glib::ustring clean_weblink = str::replace(link_entry.webs, "amp;", "");
        if (_pCtConfig->weblinkCustomOn) {
            std::string cmd = fmt::sprintf(_pCtConfig->weblinkCustomAct, clean_weblink.raw());
            int retr = std::system(cmd.c_str());
            if (retr == -1) {
                // Internal std::system error
                spdlog::error("Error while executing: '{}'; Message: {}", cmd, std::strerror(errno));
                return;
            }
        }
        else {
            fs::open_weblink(clean_weblink);
        }
    }
    else if (link_entry.type == CtConst::LINK_TYPE_FILE) { // link to file
        fs::path filepath = fs::path{link_entry.file}.string_native();
        if (filepath.is_relative()) filepath = fs::canonical(filepath, _pCtMainWin->get_ct_storage()->get_file_path().parent_path());
        if (from_wheel) {
            filepath = filepath.parent_path();
            if (not fs::is_directory(filepath)) {
                CtDialogs::error_dialog(str::format(_("The Folder Link '%s' is Not Valid."), str::xml_escape(filepath.string())), *_pCtMainWin);
                return;
            }
            fs::open_folderpath(filepath, _pCtConfig);
        }
        else {
            if (not Glib::file_test(filepath.string(), Glib::FILE_TEST_IS_REGULAR)) {
                CtDialogs::error_dialog(str::format(_("The File Link '%s' is Not Valid."), str::xml_escape(filepath.string())), *_pCtMainWin);
                return;
            }
            fs::open_filepath(filepath, true, _pCtConfig);
        }
    }
    else if (link_entry.type == CtConst::LINK_TYPE_FOLD) { // link to folder
        fs::path folderpath = fs::path{link_entry.fold}.string_native();
        if (folderpath.is_relative()) folderpath = fs::canonical(folderpath, _pCtMainWin->get_ct_storage()->get_file_path().parent_path());
        if (from_wheel) {
            folderpath = Glib::path_get_dirname(folderpath.string());
        }
        if (not fs::is_directory(folderpath)) {
            CtDialogs::error_dialog(str::format(_("The Folder Link '%s' is Not Valid"), str::xml_escape(folderpath.string())), *_pCtMainWin);
            return;
        }
        fs::open_folderpath(folderpath, _pCtConfig);
    }
    else if (link_entry.type == CtConst::LINK_TYPE_NODE) { // link to a tree node
        CtTreeIter tree_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(link_entry.node_id);
        if (not tree_iter) {
            CtDialogs::error_dialog(str::format(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)"), std::to_string(link_entry.node_id)), *_pCtMainWin);
            return;
        }
        _pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
        _pCtMainWin->get_text_view().grab_focus();
        _pCtMainWin->get_text_view().get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::XTERM));
        _pCtMainWin->get_text_view().set_tooltip_text("");
        if (not link_entry.anch.empty()) {
            current_node_scroll_to_anchor(link_entry.anch);
        }
    }
    else {
        CtDialogs::error_dialog(str::format("Tag Name Not Recognized! (%s)", str::xml_escape(tag_property_value)), *_pCtMainWin);
    }
}

void CtActions::current_node_scroll_to_anchor(Glib::ustring anchor_name)
{
    if (not _is_there_selected_node_or_error()) return;

    CtImageAnchor* imageAnchor = nullptr;
    for (auto& widget : _pCtMainWin->curr_tree_iter().get_anchored_widgets_fast()) {
        if (auto anchor = dynamic_cast<CtImageAnchor*>(widget)) {
            if (anchor->get_anchor_name() == anchor_name) {
                imageAnchor = anchor;
            }
        }
    }
    if (not imageAnchor) {
        if (anchor_name.size() > (size_t)CtConst::MAX_TOOLTIP_LINK_CHARS) {
            anchor_name = anchor_name.substr(0, (size_t)CtConst::MAX_TOOLTIP_LINK_CHARS) + "...";
        }
        CtDialogs::warning_dialog(str::format(_("No anchor named '%s' found"), str::xml_escape(anchor_name)), *_pCtMainWin);
    }
    else {
        Gtk::TextIter iter_anchor = _curr_buffer()->get_iter_at_child_anchor(imageAnchor->getTextChildAnchor());
        _curr_buffer()->place_cursor(iter_anchor);
        _pCtMainWin->get_text_view().scroll_to(_curr_buffer()->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
    }
}

void CtActions::codebox_cut()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    object_set_selection(curr_codebox_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

void CtActions::codebox_copy()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    object_set_selection(curr_codebox_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

void CtActions::codebox_delete()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    object_set_selection(curr_codebox_anchor);
    _curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_codebox_anchor = nullptr;
   _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::codebox_delete_keeping_text()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    Glib::ustring content = curr_codebox_anchor->get_text_content();
    object_set_selection(curr_codebox_anchor);
    _curr_buffer()->erase_selection(true, _pCtMainWin->get_text_view().get_editable());
    curr_codebox_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
    _curr_buffer()->insert_at_cursor(content);
}

void CtActions::codebox_change_properties()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    _pCtConfig->codeboxWidth = curr_codebox_anchor->get_frame_width();
    _pCtConfig->codeboxWidthPixels = curr_codebox_anchor->get_width_in_pixels();
    _pCtConfig->codeboxHeight = curr_codebox_anchor->get_frame_height();
    _pCtConfig->codeboxLineNum = curr_codebox_anchor->get_show_line_numbers();
    _pCtConfig->codeboxMatchBra = curr_codebox_anchor->get_highlight_brackets();
    _pCtConfig->codeboxSynHighl = curr_codebox_anchor->get_syntax_highlighting();

    if (not CtDialogs::codeboxhandle_dialog(_pCtMainWin, _("Edit CodeBox"))) return;

    curr_codebox_anchor->set_syntax_highlighting(_pCtConfig->codeboxSynHighl,
                                                 _pCtMainWin->get_language_manager());
    curr_codebox_anchor->set_width_in_pixels(_pCtConfig->codeboxWidthPixels);
    curr_codebox_anchor->set_width_height((int)_pCtConfig->codeboxWidth, (int)_pCtConfig->codeboxHeight);
    curr_codebox_anchor->set_show_line_numbers(_pCtConfig->codeboxLineNum);
    curr_codebox_anchor->set_highlight_brackets(_pCtConfig->codeboxMatchBra);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::_exec_code(const bool is_all)
{
    if (not _is_there_selected_node_or_error()) return;

    std::string code_type;
    Glib::ustring code_val;
    auto proof = _get_text_view_n_buffer_codebox_proof();
    auto text_buffer = proof.text_view->get_buffer();
    if (CtConst::PLAIN_TEXT_ID == proof.syntax_highl ||
        CtConst::RICH_TEXT_ID == proof.syntax_highl ||
        CtConst::TABLE_CELL_TEXT_ID == proof.syntax_highl)
    {
        code_type = "sh";
    }
    else {
        code_type = proof.syntax_highl;
    }
    if (is_all) {
        code_val = text_buffer->begin().get_text(text_buffer->end());
    }
    else {
        if (text_buffer->get_has_selection()) {
            Gtk::TextIter iter_start, iter_end;
            text_buffer->get_selection_bounds(iter_start, iter_end);
            code_val = text_buffer->get_text(iter_start, iter_end);
        }
        else {
            CtTextRange range = CtList{_pCtConfig, text_buffer}.get_paragraph_iters();
            code_val = text_buffer->get_text(range.iter_start, range.iter_end);
        }
    }
    std::string binary_cmd = CtPrefDlg::get_code_exec_type_cmd(_pCtMainWin, code_type);
    if (binary_cmd.empty()) {
        CtDialogs::warning_dialog(str::format(_("You must associate a command to '%s'.\nDo so in the Preferences Dialog."), str::xml_escape(code_type)), *_pCtMainWin);
        return;
    }
    std::string code_type_ext = CtPrefDlg::get_code_exec_ext(_pCtMainWin, code_type);
    Glib::ustring code_exec_term = CtPrefDlg::get_code_exec_term_run(_pCtMainWin);

    fs::path filepath_src_tmp = _pCtMainWin->get_ct_tmp()->getHiddenFilePath("exec_code." + code_type_ext);
    fs::path filepath_bin_tmp = _pCtMainWin->get_ct_tmp()->getHiddenFilePath("exec_code.exe");
    const bool using_tmp_src{std::string::npos != binary_cmd.find(CtConst::CODE_EXEC_TMP_SRC)};
    if (using_tmp_src) {
        binary_cmd = str::replace(binary_cmd, CtConst::CODE_EXEC_TMP_SRC, filepath_src_tmp.string());
    }
    else {
        binary_cmd = str::replace(binary_cmd, CtConst::CODE_EXEC_CODE_TXT, code_val.raw());
    }
    binary_cmd = str::replace(binary_cmd, CtConst::CODE_EXEC_TMP_BIN, filepath_bin_tmp.string());

    if (_pCtConfig->codeExecConfirm and not CtDialogs::exec_code_confirm_dialog(*_pCtMainWin, code_type, code_val)) {
        return;
    }

#if !defined(HAVE_VTE)
    if (_pCtConfig->codeExecVte) {
        _pCtConfig->codeExecVte = false;
    }
#endif // !HAVE_VTE

    Glib::ustring terminal_cmd;
    if (not _pCtConfig->codeExecVte) {
        terminal_cmd = str::replace(code_exec_term, CtConst::CODE_EXEC_COMMAND, binary_cmd);
    }

#if !defined(_WIN32)
    if (not _pCtConfig->codeExecVte) {
        static bool xterm_verified{false};
        if (not xterm_verified and str::startswith(terminal_cmd, "xterm ")) {
            const int status = std::system("xterm -version");
            if (WEXITSTATUS(status) != 0) {
                CtDialogs::error_dialog(_("Install the package 'xterm' or configure a different terminal in the Preferences Dialog."), *_pCtMainWin);
                return;
            }
            xterm_verified = true;
        }
    }
#endif // !_WIN32

    if (using_tmp_src) {
        g_file_set_contents(filepath_src_tmp.c_str(), code_val.c_str(), (gssize)code_val.bytes(), nullptr);
    }

    if (_pCtConfig->codeExecVte) {
        binary_cmd += "\n";
        _pCtMainWin->exec_in_vte(binary_cmd);
    }
    else {
        (void)std::system(terminal_cmd.c_str());
    }
}

void CtActions::codebox_load_from_file()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtConfig->pickDirCbox;

    std::string filepath = CtDialogs::file_select_dialog(args);
    if (filepath.empty()) return;
    _pCtConfig->pickDirCbox = Glib::path_get_dirname(filepath);

    std::string buffer = Glib::file_get_contents(filepath);
    curr_codebox_anchor->get_buffer()->set_text(buffer);
}

void CtActions::codebox_save_to_file()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder=_pCtConfig->pickDirCbox;

    std::string filepath = CtDialogs::file_save_as_dialog(args);
    if (filepath.empty()) return;
    _pCtConfig->pickDirCbox = Glib::path_get_dirname(filepath);

    Glib::ustring text = curr_codebox_anchor->get_text_content();
    CtMiscUtil::text_file_set_contents_add_cr_on_win(filepath, text);
}

void CtActions::codebox_increase_width()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
    int prevFrameWidth = curr_codebox_anchor->get_frame_width();
    if (curr_codebox_anchor->get_width_in_pixels()) {
        if (_pCtConfig->codeboxAutoResize and prevFrameWidth < curr_codebox_anchor->get_text_view().get_allocated_width() ) {
            prevFrameWidth = curr_codebox_anchor->get_text_view().get_allocated_width();
        }
        curr_codebox_anchor->set_width_height(prevFrameWidth + CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX, 0);
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
    }
    else {
        if (prevFrameWidth + CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC < 100) {
            curr_codebox_anchor->set_width_height(prevFrameWidth + CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC, 0);
            _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
        }
    }
}

void CtActions::codebox_decrease_width()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
    if (curr_codebox_anchor->get_width_in_pixels()) {
        if (curr_codebox_anchor->get_frame_width() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX >= CtCodebox::CB_WIDTH_LIMIT_MIN) {
            curr_codebox_anchor->set_width_height(curr_codebox_anchor->get_frame_width() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX, 0);
            _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
        }
    }
    else {
        if (curr_codebox_anchor->get_frame_width() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC >= CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC) {
            curr_codebox_anchor->set_width_height(curr_codebox_anchor->get_frame_width() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PERC, 0);
            _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
        }
    }
}

void CtActions::codebox_increase_height()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
    int prevFrameHeight = curr_codebox_anchor->get_frame_height();
    if (_pCtConfig->codeboxAutoResize and prevFrameHeight < curr_codebox_anchor->get_text_view().get_allocated_height() ) {
        prevFrameHeight = curr_codebox_anchor->get_text_view().get_allocated_height();
    }
    curr_codebox_anchor->set_width_height(0, prevFrameHeight + CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
}

void CtActions::codebox_decrease_height()
{
    if (not _is_there_anch_widg_selection_or_error('c')) return;
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
    if (curr_codebox_anchor->get_frame_height() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX >= CtCodebox::CB_HEIGHT_LIMIT_MIN) {
        curr_codebox_anchor->set_width_height(0, curr_codebox_anchor->get_frame_height() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX);
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
    }
}

void CtActions::table_cut()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    object_set_selection(curr_table_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "cut-clipboard");
}

void CtActions::table_copy()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    object_set_selection(curr_table_anchor);
    g_signal_emit_by_name(G_OBJECT(_pCtMainWin->get_text_view().gobj()), "copy-clipboard");
}

void CtActions::table_delete()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    object_set_selection(curr_table_anchor);
    _curr_buffer()->erase_selection(true/*interactive*/, _pCtMainWin->get_text_view().get_editable());
    curr_table_anchor = nullptr;
    _pCtMainWin->get_text_view().grab_focus();
}

void CtActions::table_column_add()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->column_add(curr_table_anchor->current_column());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_column_delete()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->column_delete(curr_table_anchor->current_column());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_column_left()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->column_move_left(curr_table_anchor->current_column(), false/*from_move_right*/);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_column_right()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->column_move_right(curr_table_anchor->current_column());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_column_increase_width()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
    curr_table_anchor->set_col_width(curr_table_anchor->get_col_width() + CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_column_decrease_width()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return;
    if (curr_table_anchor->get_col_width() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX >= CtCodebox::CB_WIDTH_LIMIT_MIN) {
        curr_table_anchor->set_col_width(curr_table_anchor->get_col_width() - CtCodebox::CB_WIDTH_HEIGHT_STEP_PIX);
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
    }
}

void CtActions::table_row_add()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->row_add(curr_table_anchor->current_row());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_row_cut()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    table_row_copy();
    table_row_delete();
}

void CtActions::table_row_copy()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    auto table_state = std::dynamic_pointer_cast<CtAnchoredWidgetState_TableCommon>(curr_table_anchor->get_state());
    // remove rows after current
    while (table_state->rows.size() > curr_table_anchor->current_row() + 1)
        table_state->rows.pop_back();
    // remove rows between current and header
    while (table_state->rows.size() > 2)
        table_state->rows.erase(table_state->rows.begin() + 1);
    auto new_table = dynamic_cast<CtTableCommon*>(table_state->to_widget(_pCtMainWin));
    CtClipboard{_pCtMainWin}.table_row_to_clipboard(new_table);
    delete new_table;
}

void CtActions::table_row_paste()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->exit_cell_edit();
    CtClipboard{_pCtMainWin}.table_row_paste(curr_table_anchor);
    curr_table_anchor->grab_focus();
}

void CtActions::table_row_delete()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->row_delete(curr_table_anchor->current_row());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_row_up()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->row_move_up(curr_table_anchor->current_row(), false/*from_move_down*/);
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_row_down()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    curr_table_anchor->row_move_down(curr_table_anchor->current_row());
    _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
}

void CtActions::table_rows_sort_descending()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (curr_table_anchor->row_sort_desc()) {
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
    }
}

void CtActions::table_rows_sort_ascending()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    if (curr_table_anchor->row_sort_asc()) {
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
    }
}

void CtActions::table_edit_properties()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    if (not _is_curr_node_not_read_only_or_error()) return;
    _pCtConfig->tableColWidthDefault = curr_table_anchor->get_col_width_default();
    const bool was_light = curr_table_anchor->get_is_light();
    bool is_light{was_light};
    if (CtDialogs::TableHandleResp::Cancel == CtDialogs::table_handle_dialog(
        _pCtMainWin, _("Edit Table Properties"), false/*is_insert*/, is_light))
    {
        return;
    }
    curr_table_anchor->set_col_width_default(_pCtConfig->tableColWidthDefault);
    if (was_light != is_light) {
        std::shared_ptr<CtAnchoredWidgetState_TableCommon> pStateCommon = curr_table_anchor->get_state_common();
        table_delete();
        auto pCtTable = is_light ? static_cast<CtTableCommon*>(pStateCommon->to_widget_light(_pCtMainWin)) :
            static_cast<CtTableCommon*>(pStateCommon->to_widget_heavy(_pCtMainWin));
        Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(_curr_buffer());
        pCtTable->insertInTextBuffer(gsv_buffer);
        _pCtMainWin->get_tree_store().addAnchoredWidgets(_pCtMainWin->curr_tree_iter(),
            {pCtTable}, &_pCtMainWin->get_text_view());
        curr_table_anchor = pCtTable;
    }
    else {
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, true/*new_machine_state*/);
    }
}

void CtActions::table_export()
{
    if (not _is_there_anch_widg_selection_or_error('t')) return;
    CtDialogs::FileSelectArgs args{_pCtMainWin};
    args.curr_folder = _pCtConfig->pickDirCsv;
    args.curr_file_name = "";
    args.filter_name = _("CSV File");
    args.filter_pattern = {"*.csv"};

    Glib::ustring filename = CtDialogs::file_save_as_dialog(args);
    if (filename.empty()) return;
    if (!str::endswith(filename, ".csv")) filename += ".csv";
    _pCtConfig->pickDirCsv = Glib::path_get_dirname(filename);

    try {
        std::string result = curr_table_anchor->to_csv();
        Glib::file_set_contents(filename, result);
    }
    catch (std::exception& e) {
        spdlog::error("Exception caught while exporting table: {}", e.what());
        CtDialogs::error_dialog("Exception occured while exporting table, see log for details", *_pCtMainWin);
    }
}

void CtActions::_anchor_edit_dialog(CtImageAnchor* anchor, Gtk::TextIter insert_iter, Gtk::TextIter* iter_bound)
{
    Glib::ustring dialog_title = anchor == nullptr ? _("Insert Anchor") :  _("Edit Anchor");
    Glib::ustring name = anchor == nullptr ? "" : anchor->get_anchor_name();
    Glib::ustring ret_anchor_name = CtDialogs::img_n_entry_dialog(*_pCtMainWin, dialog_title, name, "ct_anchor");
    if (ret_anchor_name.empty()) return;

    Glib::ustring image_justification;
    if (iter_bound) { // only in case of modify
        image_justification = CtTextIterUtil::get_text_iter_alignment(insert_iter, _pCtMainWin);
        int image_offset = insert_iter.get_offset();
        _curr_buffer()->erase(insert_iter, *iter_bound);
        insert_iter = _curr_buffer()->get_iter_at_offset(image_offset);
    }
    image_insert_anchor(insert_iter, ret_anchor_name, image_justification);
}

bool CtActions::_on_embfiles_sentinel_timeout()
{
    for (auto& item : _embfiles_opened) {
        const fs::path& tmp_filepath = item.second.tmp_filepath;
        if (not fs::is_regular_file(tmp_filepath)) {
            spdlog::debug("embdrop {}", tmp_filepath);
            _embfiles_opened.erase(item.first);
            break;
        }
        if (item.second.mod_time != fs::getmtime(tmp_filepath)) {
            item.second.mod_time = fs::getmtime(tmp_filepath);
            const auto data_vec = str::split(tmp_filepath.filename().string(), CtConst::CHAR_MINUS);
            const gint64 node_id = std::stoll(data_vec[0]);
            const size_t embfile_id = std::stol(data_vec[1]);

            CtTreeIter tree_iter = _pCtMainWin->get_tree_store().get_node_from_node_id(node_id);
            if (not tree_iter) {
                continue;
            }
            if (tree_iter.get_node_read_only()) {
                CtDialogs::warning_dialog(_("Cannot Edit Embedded File in Read Only Node."), *_pCtMainWin);
                continue;
            }
            _pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
            for (auto& widget : tree_iter.get_anchored_widgets_fast()) {
                if (auto embFile = dynamic_cast<CtImageEmbFile*>(widget)) {
                    if (embFile->get_unique_id() == embfile_id) {
                        std::string buffer = Glib::file_get_contents(tmp_filepath.string());
                        embFile->set_raw_blob(buffer);
                        embFile->set_time(std::time(nullptr));
                        embFile->update_tooltip();

                        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf);
                        _pCtMainWin->get_status_bar().update_status(_("Embedded File Automatically Updated:") + std::string(CtConst::CHAR_SPACE) + embFile->get_file_name().string());
                        break;
                    }
                }
            }
        }
    }
    return true; // this way we keep the timer alive
}

void CtActions::terminal_copy()
{
#if defined(HAVE_VTE)
    Gtk::Widget* pVte = _pCtMainWin->get_vte();
    if (not pVte) {
        return;
    }
    GtkWidget* pTermWidget = pVte->gobj();
    if (not vte_terminal_get_has_selection(VTE_TERMINAL(pTermWidget))) {
        vte_terminal_select_all(VTE_TERMINAL(pTermWidget));
    }
    vte_terminal_copy_clipboard_format(VTE_TERMINAL(pTermWidget), VTE_FORMAT_TEXT);
#endif // HAVE_VTE
}

void CtActions::terminal_paste()
{
#if defined(HAVE_VTE)
    Gtk::Widget* pVte = _pCtMainWin->get_vte();
    if (not pVte) {
        return;
    }
    GtkWidget* pTermWidget = pVte->gobj();
    vte_terminal_paste_clipboard(VTE_TERMINAL(pTermWidget));
#endif // HAVE_VTE
}

void CtActions::terminal_reset()
{
    _pCtMainWin->restart_vte(nullptr/*first_cmd_passed*/);
}
