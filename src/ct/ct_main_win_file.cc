/*
 * ct_main_win_file.cc
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

#include "ct_main_win.h"
#include "ct_actions.h"
#include "ct_storage_control.h"

void CtMainWin::window_title_update(std::optional<bool> saveNeeded)
{
    Glib::ustring title;
    title.reserve(100);
    if (not saveNeeded.has_value()) {
        const Glib::ustring currTitle = get_title();
        saveNeeded = not currTitle.empty() and currTitle.at(0) == '*';
    }
    if (saveNeeded.value()) {
        title += "*";
    }
    if (not _uCtStorage->get_file_path().empty()) {
        title += _uCtStorage->get_file_name().string() + " - ";
        if (_pCtConfig->winTitleShowDocDir) {
            title += _uCtStorage->get_file_dir().string() + " - ";
        }
    }
    title += "CherryTree ";
    title += CtConst::CT_VERSION;
    if (not _pCtConfig->menubarVisible) {
        CtMenuAction* pAction = _uCtMenu->find_action("toggle_show_menubar");
        title += Glib::ustring{" - "} + _("Show Menubar:") + " " + (pAction ? pAction->get_shortcut(_pCtConfig) : std::string{"?"});
    }
    if (_pHeaderBar) {
        _pHeaderBar->set_title(title); // so it is shown on the taskbar button
        auto pCustomTitle = dynamic_cast<Gtk::Label*>(_pHeaderBar->get_custom_title());
        if (not pCustomTitle) {
            pCustomTitle = Gtk::manage(new Gtk::Label{});
            _pHeaderBar->set_custom_title(*pCustomTitle);
            pCustomTitle->property_ellipsize() = Pango::ELLIPSIZE_END;
            pCustomTitle->show();
        }
        pCustomTitle->set_markup(Glib::ustring{"<b><small>"}+title+"</small></b>");
        pCustomTitle->set_tooltip_text(title);
    }
    else {
        set_title(title);
    }
}

void CtMainWin::update_window_save_not_needed()
{
    window_title_update(false/*save_needed*/);
    _fileSaveNeeded = false;
    CtTreeIter treeIter = curr_tree_iter();
    if (treeIter) {
        Glib::RefPtr<Gsv::Buffer> rTextBuffer = treeIter.get_node_text_buffer();
        rTextBuffer->set_modified(false);
        std::list<CtAnchoredWidget*> anchoredWidgets = treeIter.get_anchored_widgets_fast();
        for (CtAnchoredWidget* pAnchoredWidget : anchoredWidgets) {
            pAnchoredWidget->set_modified_false();
        }
    }
}

void CtMainWin::update_window_save_needed(const CtSaveNeededUpdType update_type,
                                          const bool new_machine_state,
                                          const CtTreeIter* give_tree_iter)
{
    CtTreeIter treeIter = (nullptr != give_tree_iter ? *give_tree_iter : curr_tree_iter());
    if (treeIter.get_node_is_rich_text()) {
        treeIter.get_node_text_buffer()->set_modified(true); // support possible change inside anchored widget which doesn't toggle modified flag
    }
    if (false == _fileSaveNeeded) {
        window_title_update(true/*save_needed*/);
        _fileSaveNeeded = true;
    }
    switch (update_type) {
        case CtSaveNeededUpdType::None:
            break;
        case CtSaveNeededUpdType::nbuf: {
            treeIter.pending_edit_db_node_buff();
            g_autoptr(GDateTime) pGDateTime = g_date_time_new_now_local();
            const gint64 curr_time = g_date_time_to_unix(pGDateTime);
            treeIter.set_node_modification_time(curr_time);
            const gint64 node_id = treeIter.get_node_id();
            if ( (0 == _latestStatusbarUpdateTime.count(node_id)) or
                 (curr_time - _latestStatusbarUpdateTime.at(node_id) > 60) )
            {
                _latestStatusbarUpdateTime[node_id] = curr_time;
                update_selected_node_statusbar_info();
            }
        } break;
        case CtSaveNeededUpdType::npro: {
            treeIter.pending_edit_db_node_prop();
        } break;
        case CtSaveNeededUpdType::ndel: {
            const gint64 top_node_id = treeIter.get_node_id();
            std::vector<gint64> rm_node_ids = treeIter.get_children_node_ids();
            rm_node_ids.push_back(top_node_id);
            _uCtTreestore->pending_rm_db_nodes(rm_node_ids);
            for (auto node_id: rm_node_ids) {
                _ctStateMachine.delete_states(node_id);
            }
        } break;
        case CtSaveNeededUpdType::book: {
            _uCtTreestore->pending_edit_db_bookmarks();
        } break;
    }
    if (new_machine_state && treeIter) {
        _ctStateMachine.update_state(treeIter);
    }
}

bool CtMainWin::get_file_save_needed()
{
    return _fileSaveNeeded or (curr_tree_iter() and curr_tree_iter().get_node_text_buffer()->get_modified());
}

bool CtMainWin::file_open(const fs::path& filepath, const std::string& node_to_focus, const std::string& anchor_to_focus, const Glib::ustring password)
{
    if (not fs::exists(filepath)) {
        g_autofree gchar* title = g_strdup_printf(_("The Path %s does Not Exist"), str::xml_escape(filepath.string()).c_str());
        CtDialogs::error_dialog(Glib::ustring{title}, *this);
        return false;
    }
    const CtDocType doc_type = fs::is_directory(filepath) ? CtDocType::MultiFile : fs::get_doc_type_from_file_ext(filepath);
    if (CtDocType::None == doc_type) {
        // not a cherrytree file but can try and insert plain text content into a new node
        if (file_insert_plain_text(filepath)) {
            return true;
        }
        CtDialogs::error_dialog(str::format(_("\"%s\" is Not a CherryTree File"), str::xml_escape(filepath.string())), *this);
        return false;
    }
    // check if there is a previous open file with unsaved changes
    if (not file_save_ask_user()) {
        return false;
    }

    fs::path prev_path = _uCtStorage->get_file_path();

    _ensure_curr_doc_in_recent_docs();
    reset(); // cannot reset after load_from because load_from fill tree store

    Glib::ustring error;
    CtStorageControl* new_storage = CtStorageControl::load_from(this, filepath, doc_type, error, password);
    if (not new_storage) {
        if (not error.empty()) {
            CtDialogs::error_dialog(str::format(_("Error Parsing the CherryTree Path:\n\"%s\""), str::xml_escape(error)), *this);
        }

        // trying to recover prevous document
        if (not prev_path.empty()) {
            file_open(prev_path, ""/*node*/, ""/*anchor*/); // it won't be in loop because storage is empty
        }
        return false; // show the given document is not loaded
    }

    _uCtStorage.reset(new_storage);

    window_title_update(false/*saveNeeded*/);
    menu_set_bookmark_menu_items();
    _uCtMenu->find_action("ct_vacuum")->signal_set_visible.emit(CtDocType::SQLite == doc_type);

    const auto iterDocsRestore{_pCtConfig->recentDocsRestore.find(filepath.string())};
    switch (_pCtConfig->restoreExpColl) {
        case CtRestoreExpColl::ALL_EXP: {
            _uCtTreeview->expand_all();
        } break;
        case CtRestoreExpColl::ALL_COLL: {
            _uCtTreeview->expand_all();
            _uCtTreestore->treeview_set_tree_expanded_collapsed_string("", *_uCtTreeview, _pCtConfig->nodesBookmExp);
        } break;
        default: {
            if (iterDocsRestore != _pCtConfig->recentDocsRestore.end()) {
                _uCtTreestore->treeview_set_tree_expanded_collapsed_string(iterDocsRestore->second.exp_coll_str, *_uCtTreeview, _pCtConfig->nodesBookmExp);
            }
        } break;
    }

    bool node_is_set = false;
    if (not node_to_focus.empty()) {
        CtTreeIter node = get_tree_store().get_node_from_node_name(node_to_focus);
        if (node) {
            _uCtTreeview->set_cursor_safe(node);
            _ctTextview.grab_focus();
            node_is_set = true;
            if (not anchor_to_focus.empty()) {
                _uCtActions->current_node_scroll_to_anchor(anchor_to_focus);
            }
        }
        else {
            CtDialogs::warning_dialog(str::format(_("No node named '%s' found."), str::xml_escape(node_to_focus)), *this);
        }
    }

    if ( not _no_gui and
         not node_is_set )
    {
        if (iterDocsRestore != _pCtConfig->recentDocsRestore.end()) {
            _uCtTreestore->treeview_set_tree_path_n_text_cursor(_uCtTreeview.get(),
                                                                iterDocsRestore->second.node_path,
                                                                iterDocsRestore->second.cursor_pos,
                                                                iterDocsRestore->second.v_adj_val);
        }
        else {
            _uCtTreestore->treeview_set_tree_path_n_text_cursor(_uCtTreeview.get(), "0", 0, 0);
        }
        _ctTextview.grab_focus();
    }

    if (iterDocsRestore != _pCtConfig->recentDocsRestore.end()) {
        auto nodes = CtStrUtil::gstring_split_to_int64(iterDocsRestore->second.visited_nodes.c_str(), ",");
        if (nodes.size() > 0) {
            _ctStateMachine.set_visited_nodes_list(nodes);
            window_header_update();
        }
    }

    _pCtConfig->recentDocsFilepaths.move_or_push_front(fs::canonical(filepath));
    menu_set_items_recent_documents();

    return true;
}

bool CtMainWin::file_save_ask_user()
{
    if (_uCtActions->get_were_embfiles_opened()) {
        const Glib::ustring message = Glib::ustring{"<b>"} +
            _("Temporary Files were Created and Opened with External Applications.") +
            "</b>\n\n<b>" + _("Quit the External Applications Before Quit CherryTree.") +
            "</b>\n\n<b>" + _("Did you Quit the External Applications?") + "</b>";
        if (not CtDialogs::question_dialog(message, *this)) {
            return false;
        }
    }

    if (get_file_save_needed()) {
        const CtYesNoCancel yesNoCancel = [this]() {
            if (_pCtConfig->autosaveOnQuit && !_uCtStorage->get_file_path().empty())
                return CtYesNoCancel::Yes;
            set_visible(true);   // window could be hidden
            return CtDialogs::exit_save_dialog(*this);
        }();

        if (CtYesNoCancel::Cancel == yesNoCancel) {
            return false;
        }
        if (CtYesNoCancel::Yes == yesNoCancel) {
            _uCtActions->file_save();
            if (get_file_save_needed()) {
                // something went wrong in the save
                return false;
            }
        }
    }
    return true;
}

void CtMainWin::file_save(bool need_vacuum)
{
    if (_uCtStorage->get_file_path().empty())
        return;
    if (!get_file_save_needed())
        if (!need_vacuum)
            return;
    if (!get_tree_store().get_iter_first())
        return;

    Glib::ustring error;
    if (_uCtStorage->save(need_vacuum, error)) {
        update_window_save_not_needed();
        _ctStateMachine.update_state();
    }
    else {
        CtDialogs::error_dialog(str::xml_escape(error), *this);
    }
}

void CtMainWin::file_save_as(const std::string& new_filepath,
                             const CtDocType doc_type,
                             const Glib::ustring& password)
{
    Glib::ustring error;
    std::unique_ptr<CtStorageControl> new_storage{
        CtStorageControl::save_as(this,
                                  new_filepath,
                                  doc_type,
                                  password,
                                  error,
                                  CtExporting::NONESAVEAS)};
    if (not new_storage) {
        CtDialogs::error_dialog(str::xml_escape(error), *this);
        return;
    }

    // remember expanded nodes for new file
    CtRecentDocRestore doc_state_restore;
    doc_state_restore.visited_nodes = str::join_numbers(_ctStateMachine.get_visited_nodes_list(), ",");
    doc_state_restore.exp_coll_str = _uCtTreestore->treeview_get_tree_expanded_collapsed_string(*_uCtTreeview);
    if (const CtTreeIter curr_iter = curr_tree_iter()) {
        doc_state_restore.node_path = _uCtTreestore->get_path(curr_iter).to_string();
        doc_state_restore.cursor_pos = curr_iter.get_node_text_buffer()->property_cursor_position();
        doc_state_restore.v_adj_val = round(_scrolledwindowText.get_vadjustment()->get_value());
    }
    _pCtConfig->recentDocsFilepaths.move_or_push_front(fs::canonical(new_filepath));
    _pCtConfig->recentDocsRestore[new_filepath] = doc_state_restore;

    // it' a hack to recover expanded nodes for new file
    auto old_restore = _pCtConfig->restoreExpColl;
    auto on_scope_exit = scope_guard([&](void*) { _pCtConfig->restoreExpColl = old_restore; });
    _pCtConfig->restoreExpColl = CtRestoreExpColl::FROM_STR;

    // instead of setting all inner states, it's easier just to re-open file
    new_storage.reset();               // we don't need it
    update_window_save_not_needed();   // remove asking to save when we close the old file
    file_open(new_filepath, ""/*node*/, ""/*anchor*/, password);
}

void CtMainWin::file_autosave_restart()
{
    const bool was_connected = not _autosave_timout_connection.empty();
    _autosave_timout_connection.disconnect();
    if (not _pCtConfig->autosaveOn) {
        if (was_connected) spdlog::debug("autosave was stopped");
        return;
    }
    if (_pCtConfig->autosaveVal < 1) {
        CtDialogs::error_dialog("Wrong timeout for autosave", *this);
        return;
    }

    spdlog::debug("autosave is started");
    _autosave_timout_connection = Glib::signal_timeout().connect_seconds([this]() {
        if (get_file_save_needed()) {
            spdlog::debug("autosave needed");
            _uCtActions->file_save();
        }
        else {
            spdlog::debug("autosave no need");
        }
        return true;
    }, _pCtConfig->autosaveVal * 60);
}

void CtMainWin::mod_time_sentinel_restart()
{
    const bool was_connected = not _mod_time_sentinel_timout_connection.empty();
    _mod_time_sentinel_timout_connection.disconnect();
    if (not _pCtConfig->modTimeSentinel) {
        if (was_connected) spdlog::debug("mod time sentinel was stopped");
        return;
    }

    spdlog::debug("mod time sentinel is started");
    _mod_time_sentinel_timout_connection = Glib::signal_timeout().connect_seconds([this]() {
        if (user_active() and _uCtStorage->get_mod_time() > 0) {
            const time_t currModTime = fs::getmtime(_uCtStorage->get_file_path());
            if (currModTime > _uCtStorage->get_mod_time()) {
                spdlog::debug("mod time was {} now {}", _uCtStorage->get_mod_time(), currModTime);
                fs::path file_path = _uCtStorage->get_file_path();
                if (file_open(file_path, ""/*node*/, ""/*anchor*/)) {
                    _ctStatusBar.update_status(_("The Document was Reloaded After External Update to CT* File."));
                }
            }
        }
        return true;
    }, 5/*sec*/);
}

bool CtMainWin::file_insert_plain_text(const fs::path& filepath)
{
    spdlog::debug("trying to insert text file as node: {}", filepath);
    try {
        std::string node_contents = Glib::file_get_contents(filepath.string());
        if (not node_contents.empty()) {
            CtStrUtil::convert_if_not_utf8(node_contents, true/*sanitise*/);
            std::string name = filepath.filename().string();
            _uCtActions->node_child_exist_or_create(Gtk::TreeIter{}, name);
            _ctTextview.get_buffer()->insert(_ctTextview.get_buffer()->end(), node_contents);
            return true;
        }
    }
    catch (std::exception& ex) {
        spdlog::error("{}, what: {}, file: {}", __FUNCTION__, ex.what(), filepath);
    }
    return false;
}

void CtMainWin::_ensure_curr_doc_in_recent_docs()
{
    fs::path currDocFilePath = _uCtStorage->get_file_path();
    if (not currDocFilePath.empty()) {
        _pCtConfig->recentDocsFilepaths.move_or_push_front(fs::canonical(currDocFilePath));
        CtRecentDocRestore prevDocRestore;
        prevDocRestore.visited_nodes = str::join_numbers(_ctStateMachine.get_visited_nodes_list(), ",");
        prevDocRestore.exp_coll_str = _uCtTreestore->treeview_get_tree_expanded_collapsed_string(*_uCtTreeview);
        const CtTreeIter prevTreeIter = curr_tree_iter();
        if (prevTreeIter) {
            prevDocRestore.node_path = _uCtTreestore->get_path(prevTreeIter).to_string();
            const Glib::RefPtr<Gsv::Buffer> rTextBuffer = prevTreeIter.get_node_text_buffer();
            prevDocRestore.cursor_pos = rTextBuffer->property_cursor_position();
            prevDocRestore.v_adj_val = round(_scrolledwindowText.get_vadjustment()->get_value());
        }
        _pCtConfig->recentDocsRestore[currDocFilePath.string()] = prevDocRestore;
    }
}
