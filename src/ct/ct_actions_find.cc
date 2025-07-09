/*
 * ct_actions_find.cc
 *
 * Copyright 2009-2025
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
#include <gtkmm/dialog.h>
#include <glibmm/regex.h>
#include <regex>
#include "ct_image.h"
#include "ct_dialogs.h"
#include "ct_logging.h"

void CtActions::find_matches_store_reset()
{
    _s_state.match_store = CtMatchDialogStore::create(_pCtConfig->maxMatchesInPage);
    if (_s_state.pMatchStoreDialog) {
        delete _s_state.pMatchStoreDialog;
        _s_state.pMatchStoreDialog = nullptr;
    }
}

void CtActions::_find_init()
{
    _s_state.pMatchStoreDialog = nullptr;
    find_matches_store_reset();
    std::time_t curr_time = std::time(nullptr);
    std::time_t yesterday_time = curr_time - 86400; //24*60*60
    _s_options.ts_cre_after  = {yesterday_time, false};
    _s_options.ts_mod_after  = {yesterday_time, false};
    _s_options.ts_cre_before = {curr_time, false};
    _s_options.ts_mod_before = {curr_time, false};
}

void CtActions::find_in_selected_node()
{
    _s_state.replace_active = false;
    find_replace_in_selected_node();
}

void CtActions::find_replace_in_selected_node()
{
    if (not _is_there_selected_node_or_error()) return;

    if (not _s_state.from_find_iterated) {
        _s_state.latest_node_offset_match_start = -1;
        _s_state.latest_node_offset_match_end = -1;
        _s_state.latest_node_offset_node_id = -1;
        _s_state.find_iter_anchlist_size = 0u;
        text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
        Glib::ustring entry_predefined_text = CtTextIterUtil::get_selected_text(proof.text_view->get_buffer());
        if (entry_predefined_text.length()) {
            _s_options.str_find = entry_predefined_text;
        }
        Glib::ustring title = _s_state.replace_active ? _("Replace in Current Node") : _("Search in Current Node");
        CtDialogs::dialog_search(_pCtMainWin, title, _s_options, _s_state, false/*multiple_nodes*/);
    }
    else {
        find_in_selected_node_ok_clicked();
    }
}

void CtActions::find_in_selected_node_ok_clicked()
{
    Glib::RefPtr<Glib::Regex> re_pattern = _create_re_pattern(_s_state.curr_find_pattern);
    if (not re_pattern) return;

    bool forward = _s_options.direction_fw;
    if (_s_state.from_find_back) {
        forward = not forward;
        _s_state.from_find_back = false;
    }
    const bool first_fromsel = 1 == _s_options.all_firstsel_firstall or _s_state.from_find_iterated;
    const bool all_matches = 0 == _s_options.all_firstsel_firstall;
    _s_state.matches_num = 0;

    // searching start
    auto on_scope_exit = scope_guard([&](void*) { _pCtMainWin->user_active() = true; });
    _pCtMainWin->user_active() = false;

    if (all_matches) {
        _s_state.match_store->deep_clear();
        _s_state.all_matches_first_in_node = true;
    }
    CtTreeIter::clear_hit_exclusion_from_search();

    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = _pCtMainWin->get_text_view().get_buffer();

    while (_parse_node_content_iter(_pCtMainWin->curr_tree_iter(),
                                    curr_buffer,
                                    re_pattern,
                                    forward,
                                    first_fromsel,
                                    all_matches,
                                    true/*first_node*/))
    {
        ++_s_state.matches_num;
        if (not all_matches) break;
    }
    if (0 == _s_state.matches_num) {
        CtDialogs::no_matches_dialog(_pCtMainWin,
                                     "'" + _s_options.str_find + "'  -  0 " + _("Matches"),
                                     str::format(_("<b>The pattern '%s' was not found</b>"), str::xml_escape(_s_state.curr_find_pattern)));
    }
    else if (all_matches) {
        CtDialogs::match_dialog(_s_options.str_find, _pCtMainWin, _s_state);
    }
    else if (_s_options.iterative_dialog) {
        CtDialogs::iterated_find_dialog(_pCtMainWin, _s_state);
    }
}

static int _count_nodes(const Gtk::TreeNodeChildren& children)
{
    int count{1};
    for (auto& child : children) { count += _count_nodes(child.children()); }
    return count;
}

void CtActions::find_in_multiple_nodes()
{
    _s_state.replace_active = false;
    find_replace_in_multiple_nodes();
}

void CtActions::find_replace_in_multiple_nodes()
{
    if (not _is_there_selected_node_or_error()) return;

    if (not _s_state.from_find_iterated) {
        _s_state.latest_node_offset_match_start = -1;
        _s_state.latest_node_offset_match_end = -1;
        _s_state.latest_node_offset_node_id = -1;
        _s_state.find_iter_anchlist_size = 0u;
        if (_s_state.find_iterated_last_name_n_tags_id > 0) {
            _s_state.find_iterated_last_name_n_tags_id = 0;
            spdlog::debug("{} find_iterated_last_name_n_tags_id 0", __FUNCTION__);
        }
        text_view_n_buffer_codebox_proof proof = _get_text_view_n_buffer_codebox_proof();
        Glib::ustring entry_predefined_text = CtTextIterUtil::get_selected_text(proof.text_view->get_buffer());
        if (not entry_predefined_text.empty()) {
            _s_options.str_find = entry_predefined_text;
        }
        Glib::ustring title = _s_state.replace_active ? _("Replace in Multiple Nodes...") : _("Find in Multiple Nodes...");
        CtDialogs::dialog_search(_pCtMainWin, title, _s_options, _s_state, true/*multiple_nodes*/);
    }
    else {
        find_in_multiple_nodes_ok_clicked();
    }
}

void CtActions::find_in_multiple_nodes_ok_clicked()
{
    Glib::RefPtr<Glib::Regex> re_pattern = _create_re_pattern(_s_state.curr_find_pattern);
    if (not re_pattern) return;

    CtStatusBar& ctStatusBar = _pCtMainWin->get_status_bar();
    CtTreeStore& ctTreeStore = _pCtMainWin->get_tree_store();

    Gtk::TreeModel::iterator node_iter;
    bool forward = _s_options.direction_fw;
    if (_s_state.from_find_back) {
        forward = not forward;
        _s_state.from_find_back = false;
    }
    const bool first_fromsel = 1 == _s_options.all_firstsel_firstall or _s_state.from_find_iterated;
    const bool all_matches = 0 == _s_options.all_firstsel_firstall;
    if (first_fromsel or _s_options.only_sel_n_subnodes) {
        _s_state.first_useful_node = false; // no one node content was parsed yet
        node_iter = _pCtMainWin->curr_tree_iter();
    }
    else {
        _s_state.first_useful_node = true; // all range will be parsed so no matter
        node_iter = forward ? ctTreeStore.get_iter_first() : ctTreeStore.get_tree_iter_last_sibling(ctTreeStore.get_store()->children());
    }
    _s_state.matches_num = 0;
    if (all_matches) {
        _s_state.match_store->deep_clear();
    }
    CtTreeIter::clear_hit_exclusion_from_search();

    // searching start
    bool user_active_restore = _pCtMainWin->user_active();
    _pCtMainWin->user_active() = false;
    _s_state.processed_nodes = 0;
    _s_state.latest_matches = 0;
    _s_state.counted_nodes = _s_options.only_sel_n_subnodes ? _count_nodes(_pCtMainWin->curr_tree_iter()->children()) : (_count_nodes(ctTreeStore.get_store()->children()) - 1);
    if (all_matches) {
        ctStatusBar.progressBar.set_text("0");
        ctStatusBar.progressBar.show();
        ctStatusBar.stopButton.show();
        ctStatusBar.set_progress_stop(false);
        while (gtk_events_pending()) gtk_main_iteration();
    }
    std::time_t search_start_time = std::time(nullptr);
    while (node_iter) {
        _s_state.all_matches_first_in_node = true;
        CtTreeIter ct_node_iter = ctTreeStore.to_ct_tree_iter(node_iter);
        if (_s_options.node_content) {
            Glib::RefPtr<Gtk::TextBuffer> pTextBuffer = ct_node_iter.get_node_text_buffer();
            if (not pTextBuffer) {
                CtDialogs::error_dialog(str::format(_("Failed to retrieve the content of the node '%s'"), ct_node_iter.get_node_name().raw()), *_pCtMainWin);
                break;
            }
        }
        CtMatchType matchType{CtMatchType::None};
        auto f_matchTypeNotNone = [&](){
            matchType = _parse_given_node_content(ct_node_iter, re_pattern, forward, first_fromsel, all_matches, matchType);
            return CtMatchType::None != matchType;
        };
        while (f_matchTypeNotNone()) {
            ++_s_state.matches_num;
            if (not all_matches or ctStatusBar.is_progress_stop()) break;
        }
        ++_s_state.processed_nodes;
        if (_s_state.matches_num > 0 and not all_matches) break;
        if (_s_options.only_sel_n_subnodes and not _s_state.from_find_iterated) break;
        Gtk::TreeModel::iterator last_top_node_iter = node_iter; // we need this if we start from a node that is not in top level
        if (forward) { ++node_iter; }
        else         { --node_iter; }
        if (not node_iter and _s_options.only_sel_n_subnodes) break;
        // code that, in case we start from a node that is not top level, climbs towards the top
        while (not node_iter) {
            node_iter = last_top_node_iter->parent();
            if (node_iter) {
                last_top_node_iter = node_iter;
                // we do not check the parent on purpose, only the uncles in the proper direction
                if (forward) { ++node_iter; }
                else         { --node_iter; }
            }
            else break;
        }
        if (ctStatusBar.is_progress_stop()) break;
        if (all_matches) {
            _update_all_matches_progress();
        }
    }
    std::time_t search_end_time = std::time(nullptr);
    spdlog::debug("Search took {} sec", search_end_time - search_start_time);

    _pCtMainWin->user_active() = user_active_restore;
    if (0 == _s_state.matches_num) {
        CtDialogs::no_matches_dialog(_pCtMainWin,
                                     "'" + _s_options.str_find + "'  -  0 " + _("Matches"),
                                     str::format(_("<b>The pattern '%s' was not found</b>"), str::xml_escape(_s_state.curr_find_pattern)));
    }
    else {
        if (all_matches) {
            CtDialogs::match_dialog(_s_options.str_find, _pCtMainWin, _s_state);
        }
        else {
            if (_s_options.iterative_dialog) {
                CtDialogs::iterated_find_dialog(_pCtMainWin, _s_state);
            }
        }
    }
    if (all_matches) {
        ctStatusBar.progressBar.hide();
        ctStatusBar.stopButton.hide();
        ctStatusBar.set_progress_stop(false);
    }
}

// Continue the previous search (a_node/in_selected_node/in_all_nodes)
void CtActions::find_again_iter(const bool fromIterativeDialog)
{
    const bool restore_iterative_dialog = _s_options.iterative_dialog;
    _s_options.iterative_dialog = fromIterativeDialog;
    _s_state.from_find_iterated = true;
    if (0u != _s_state.find_iter_anchlist_size) {
        if (_s_state.from_find_back == _s_options.direction_fw) {
            if (_s_state.find_iter_anchlist_idx >= 1u) {
                --_s_state.find_iter_anchlist_idx;
                spdlog::debug("{}-- {}/{}", __FUNCTION__, _s_state.find_iter_anchlist_idx, _s_state.find_iter_anchlist_size);
            }
            else {
                spdlog::debug("{} LOLIM {}/{}", __FUNCTION__, _s_state.find_iter_anchlist_idx, _s_state.find_iter_anchlist_size);
                _s_state.find_iter_anchlist_size = 0u;
                Glib::RefPtr<Gtk::TextBuffer> text_buffer = _pCtMainWin->get_text_view().get_buffer();
                Gtk::TextIter min_iter = text_buffer->get_iter_at_offset(_s_state.latest_match_offsets.first);
                if (not min_iter.backward_char()) {
                    spdlog::debug("?? {} obj at offs 0", __FUNCTION__);
                    _s_state.find_back_exclude_obj_offs_zero = true;
                }
                else {
                    text_buffer->place_cursor(min_iter);
                }
            }
        }
        else {
            if (_s_state.find_iter_anchlist_idx < (_s_state.find_iter_anchlist_size - 1u)) {
                ++_s_state.find_iter_anchlist_idx;
                spdlog::debug("{}++ {}/{}", __FUNCTION__, _s_state.find_iter_anchlist_idx, _s_state.find_iter_anchlist_size);
            }
            else {
                spdlog::debug("{} HILIM {}/{}", __FUNCTION__, _s_state.find_iter_anchlist_idx, _s_state.find_iter_anchlist_size);
                _s_state.find_iter_anchlist_size = 0u;
                Glib::RefPtr<Gtk::TextBuffer> text_buffer = _pCtMainWin->get_text_view().get_buffer();
                Gtk::TextIter max_iter = text_buffer->get_iter_at_offset(_s_state.latest_match_offsets.second);
                text_buffer->place_cursor(max_iter);
            }
        }
    }
    switch (_s_state.curr_find_type) {
        case CtCurrFindType::SingleNode: {
            find_replace_in_selected_node();
        } break;
        case CtCurrFindType::MultipleNodes: {
            find_replace_in_multiple_nodes();
        } break;
        default:
            CtDialogs::warning_dialog(_("No Previous Search Was Performed During This Session."), *_pCtMainWin);
    }
    _s_state.from_find_iterated = false;
    _s_options.iterative_dialog = restore_iterative_dialog;
}

// Continue the previous search (a_node/in_selected_node/in_all_nodes) but in Opposite Direction
void CtActions::find_back_iter(const bool fromIterativeDialog)
{
    _s_state.from_find_back = true;
    _s_state.replace_active = false;
    find_again_iter(fromIterativeDialog);
}

// Replace a pattern in the selected Node
void CtActions::replace_in_selected_node()
{
    _s_state.replace_active = true;
    _s_state.replace_subsequent = false;
    find_replace_in_selected_node();
}

// Replace the pattern in all the Tree Nodes
void CtActions::replace_in_multiple_nodes()
{
    _s_state.replace_active = true;
    _s_state.replace_subsequent = false;
    find_replace_in_multiple_nodes();
}

// Continue the previous replace (a_node/in_selected_node/in_all_nodes)
void CtActions::replace_again()
{
    _s_state.replace_active = true;
    _s_state.replace_subsequent = true;
    find_again_iter(false/*fromIterativeDialog*/);
    _s_state.replace_active = false;
    _s_state.replace_subsequent = false;
}

// Restore AllMatchesDialog
void CtActions::find_allmatchesdialog_restore()
{
    CtDialogs::match_dialog(_s_options.str_find, _pCtMainWin, _s_state);
}

// Returns True if pattern was found, False otherwise
CtMatchType CtActions::_parse_given_node_content(CtTreeIter node_iter,
                                                 Glib::RefPtr<Glib::Regex> re_pattern,
                                                 bool forward,
                                                 bool first_fromsel,
                                                 bool all_matches,
                                                 CtMatchType thisNodeLastMatchType)
{
    const gint64 argNodeId = node_iter.get_node_id();
    std::optional<bool> optFirstNode;
    if (not _s_state.first_useful_node) {
        // first_fromsel plus first_node not already parsed
        CtTreeIter selTreeIter = _pCtMainWin->curr_tree_iter();
        if (not selTreeIter or argNodeId == selTreeIter.get_node_id()) {
            _s_state.first_useful_node = true; // a first_node was parsed
            optFirstNode = true;
        }
    }
    else {
        // not first_fromsel or first_fromsel with first_node already parsed
        optFirstNode = false;
    }
    if (optFirstNode.has_value() and (not node_iter.get_node_is_excluded_from_search() or _s_options.override_exclusions)) {
        if (_s_options.node_content) {
            if (_parse_node_content_iter(node_iter,
                                         node_iter.get_node_text_buffer(),
                                         re_pattern,
                                         forward,
                                         first_fromsel,
                                         all_matches,
                                         optFirstNode.value()))
            {
                return CtMatchType::Content;
            }
        }
        if (_s_options.node_name_n_tags and
            (not all_matches or CtMatchType::NameNTags != thisNodeLastMatchType))
        {
            if (_parse_node_name_n_tags_iter(node_iter, re_pattern, all_matches)) {
                if (_s_state.find_iterated_last_name_n_tags_id <= 0 or
                    _s_state.find_iterated_last_name_n_tags_id != argNodeId)
                {
                    _s_state.find_iterated_last_name_n_tags_id = argNodeId;
                    spdlog::debug("{} find_iterated_last_name_n_tags_id {}", __FUNCTION__, _s_state.find_iterated_last_name_n_tags_id);
                    return CtMatchType::NameNTags;
                }
                spdlog::debug("skipped name_n_tags {}", argNodeId);
            }
        }
        if (_s_state.find_iterated_last_name_n_tags_id > 0) {
            _s_state.find_iterated_last_name_n_tags_id = 0;
            spdlog::debug("{} find_iterated_last_name_n_tags_id 0", __FUNCTION__);
        }
    }

    CtTreeStore& ctTreeStore = _pCtMainWin->get_tree_store();
    if (not node_iter.get_node_children_are_excluded_from_search() or _s_options.override_exclusions) {
        // check for children
        if (not node_iter->children().empty()) {
            Gtk::TreeModel::iterator child_iter = forward ? node_iter->children().begin() : --node_iter->children().end();
            while (child_iter and not _pCtMainWin->get_status_bar().is_progress_stop()) {
                _s_state.all_matches_first_in_node = true;
                CtTreeIter ct_node_iter = ctTreeStore.to_ct_tree_iter(child_iter);
                if (_s_options.node_content) {
                    Glib::RefPtr<Gtk::TextBuffer> pTextBuffer = ct_node_iter.get_node_text_buffer();
                    if (not pTextBuffer) {
                        CtDialogs::error_dialog(str::format(_("Failed to retrieve the content of the node '%s'"), ct_node_iter.get_node_name().raw()), *_pCtMainWin);
                        break;
                    }
                }
                CtMatchType matchType{CtMatchType::None};
                auto f_matchTypeNotNone = [&](){
                    matchType = _parse_given_node_content(ct_node_iter, re_pattern, forward, first_fromsel, all_matches, matchType);
                    return CtMatchType::None != matchType;
                };
                while (f_matchTypeNotNone()) {
                    ++_s_state.matches_num;
                    if (not all_matches or _pCtMainWin->get_status_bar().is_progress_stop()) break;
                }
                if (_s_state.matches_num > 0 and not all_matches) break;
                if (forward) child_iter = ++child_iter;
                else         child_iter = --child_iter;
                _s_state.processed_nodes += 1;
                if (all_matches) {
                    _update_all_matches_progress();
                }
            }
        }
    }
    return CtMatchType::None;
}

// Returns True if pattern was found, False otherwise
bool CtActions::_parse_node_name_n_tags_iter(CtTreeIter& node_iter,
                                             Glib::RefPtr<Glib::Regex> re_pattern,
                                             const bool all_matches)
{
    if (not _is_node_within_time_filter(node_iter)) {
        return false;
    }

    Glib::ustring node_name = node_iter.get_node_name();
    if (_s_options.accent_insensitive) {
        node_name = str::diacritical_to_ascii(node_name);
    }
    Glib::MatchInfo match_info;
    if (not re_pattern->match(node_name, match_info)) {
        Glib::ustring text_tags = node_iter.get_node_tags();
        if (_s_options.accent_insensitive) {
            text_tags = str::diacritical_to_ascii(text_tags);
        }
        re_pattern->match(text_tags, match_info);
    }

    if (match_info.matches()) {
        if (all_matches) {
            gint64 node_id = node_iter.get_node_id();
            Glib::ustring node_hier_name = CtMiscUtil::get_node_hierarchical_name(node_iter, "  /  ", false/*for_filename*/, true/*root_to_leaf*/);
            Glib::ustring line_content = CtTextIterUtil::get_first_line_content(node_iter.get_node_text_buffer());
            const Glib::ustring text_tags = node_iter.get_node_tags();
            _s_state.match_store->add_row(node_id,
                                          text_tags.empty() ? node_name : node_name + "\n [" +  _("Tags") + _(": ") + text_tags + "]",
                                          str::xml_escape(node_hier_name),
                                          0, 0, 0/*line_num*/, line_content, CtAnchWidgType::None, 0, 0, 0);
        }
        if (_s_state.replace_active and not node_iter.get_node_read_only()) {
            std::string replacer_text = _s_options.str_replace;
            node_name = re_pattern->replace(node_name, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));
            node_iter.set_node_name(node_name);
            node_iter.pending_edit_db_node_prop();
        }
        if (not all_matches) {
            _pCtMainWin->get_tree_view().set_cursor_safe(node_iter);
            _pCtMainWin->get_text_view().mm().grab_focus();
        }
        return true;
    }
    return false;
}

// Returns True if pattern was found, False otherwise
bool CtActions::_parse_node_content_iter(const CtTreeIter& tree_iter,
                                         Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                         Glib::RefPtr<Glib::Regex> re_pattern,
                                         bool forward,
                                         bool first_fromsel,
                                         bool all_matches,
                                         bool first_node)
{
    if (not _is_node_within_time_filter(tree_iter)) {
        return false;
    }

    Gtk::TextIter start_iter;
    if ((first_fromsel and first_node) or (all_matches and not _s_state.all_matches_first_in_node)) {
        start_iter = _get_inner_start_iter(text_buffer, forward, all_matches);
    }
    else {
        start_iter = forward ? text_buffer->begin() : text_buffer->end();
        //spdlog::debug("fw={} nosel m={} M={} -> s={}", forward, text_buffer->begin().get_offset(), text_buffer->end().get_offset(), start_iter.get_offset());
        if (all_matches) _s_state.all_matches_first_in_node = false;
    }
    //spdlog::debug("parsing {} content from {} ffs={} 1st={}", tree_iter.get_node_id(), start_iter.get_offset(), first_fromsel, first_node);

    bool pattern_found = _find_pattern(tree_iter, text_buffer, re_pattern, start_iter, forward, all_matches);

    if (_s_state.replace_active and pattern_found)
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, false/*new_machine_state*/, &tree_iter);
    return pattern_found;
}

// Get start_iter when not at beginning or end
Gtk::TextIter CtActions::_get_inner_start_iter(Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                               const bool forward,
                                               const bool all_matches)
{
    Gtk::TextIter min_iter, max_iter;
    if (all_matches and _s_state.latest_match_offsets.first >= 0 and _s_state.latest_match_offsets.second >= 0) {
        min_iter = text_buffer->get_iter_at_offset(_s_state.latest_match_offsets.first);
        max_iter = text_buffer->get_iter_at_offset(_s_state.latest_match_offsets.second);
    }
    else if (text_buffer->get_has_selection()) {
        text_buffer->get_selection_bounds(min_iter, max_iter);
    }
    else {
        min_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        max_iter = min_iter;
    }
    Gtk::TextIter start_iter;
    if (not _s_state.replace_active or _s_state.replace_subsequent) {
        // it's a find or subsequent replace, so we want, given a selected word, to find for the subsequent one
        if (forward)    start_iter = max_iter;
        else            start_iter = min_iter;
    }
    else {
        // it's a first replace, so we want, given a selected word, to replace starting from this one
        if (forward)    start_iter = min_iter;
        else            start_iter = max_iter;
    }
    //spdlog::debug("fw={} m={} M={} -> s={}", forward, min_iter.get_offset(), max_iter.get_offset(), start_iter.get_offset());
    return start_iter;
}

//"""Returns True if the given node_iter is within the Time Filter"""
bool CtActions::_is_node_within_time_filter(const CtTreeIter& node_iter)
{
    gint64 ts_cre = node_iter.get_node_creating_time();
    if (_s_options.ts_cre_after.on and ts_cre < _s_options.ts_cre_after.time)
        return false;
    if (_s_options.ts_cre_before.on and ts_cre > _s_options.ts_cre_before.time)
        return false;
    gint64 ts_mod = node_iter.get_node_modification_time();
    if (_s_options.ts_mod_after.on and ts_mod < _s_options.ts_mod_after.time)
        return false;
    if (_s_options.ts_mod_before.on and ts_mod > _s_options.ts_mod_before.time)
        return false;
    return true;
}

Glib::RefPtr<Glib::Regex> CtActions::_create_re_pattern(Glib::ustring pattern)
{
    if (_s_options.accent_insensitive) {
        pattern = str::diacritical_to_ascii(pattern);
    }
    if (not _s_options.reg_exp) { // NOT REGULAR EXPRESSION
        pattern = Glib::Regex::escape_string(pattern);     // backslashes all non alphanum chars => to not spoil re
        if (_s_options.whole_word)      // WHOLE WORD
            pattern = "\\b" + pattern + "\\b";
        else if (_s_options.start_word) // START WORD
            pattern = "\\b" + pattern;
    }
    try {
        if (_s_options.match_case) // CASE SENSITIVE
            return Glib::Regex::create(pattern, Glib::RegexCompileFlags::REGEX_MULTILINE);
        else
            return Glib::Regex::create(pattern, Glib::RegexCompileFlags::REGEX_MULTILINE | Glib::RegexCompileFlags::REGEX_CASELESS);
    }
    catch (Glib::RegexError& e) {
        CtDialogs::error_dialog(str::xml_escape(e.what()), *_pCtMainWin);
        return Glib::RefPtr<Glib::Regex>();
    }
}

bool CtActions::_find_pattern(CtTreeIter tree_iter,
                              Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                              Glib::RefPtr<Glib::Regex> re_pattern,
                              Gtk::TextIter start_iter,
                              bool forward,
                              bool all_matches)
{
    // Gtk::TextBuffer uses symbols positions
    // Glib::Regex uses byte positions
    Glib::ustring text = text_buffer->get_text();
    if (_s_options.accent_insensitive) {
        text = str::diacritical_to_ascii(text);
    }

    const gint64 node_id = tree_iter.get_node_id();
    const int start_offset = start_iter.get_offset();
    const int num_objs_before_start = _get_num_objs_before_offset(text_buffer, start_offset);
    const int position_fw_start_or_bw_end = str::symb_pos_to_byte_pos(text, std::max(0, start_offset - num_objs_before_start));
    std::pair<int, int> match_offsets{-1, -1};
    if (forward) {
        Glib::MatchInfo match_info;
        (void)re_pattern->match(text, position_fw_start_or_bw_end, match_info);
        while (match_info.matches()) {
            std::pair<int,int> curr_pair;
            match_info.fetch_pos(0, curr_pair.first, curr_pair.second);
            if (curr_pair.first >= _s_state.latest_node_offset_match_end or
                node_id != _s_state.latest_node_offset_node_id)
            {
                match_offsets = curr_pair;
                _s_state.latest_node_offset_match_start = match_offsets.first;
                _s_state.latest_node_offset_match_end = match_offsets.second;
                //spdlog::debug("OK {}->{}", curr_pair.first, curr_pair.second);
                break;
            }
            //spdlog::debug("NOK {}->{}", curr_pair.first, curr_pair.second);
            match_info.next();
        }
    }
    else {
        Glib::MatchInfo match_info;
        (void)re_pattern->match(text, position_fw_start_or_bw_end, 0/*start_position*/, match_info);
        std::deque<std::pair<int,int>> match_deque;
        while (match_info.matches()) {
            std::pair<int,int> curr_pair;
            match_info.fetch_pos(0, curr_pair.first, curr_pair.second);
            match_deque.push_front(curr_pair);
            match_info.next();
        }
        for (const auto& curr_pair : match_deque) {
            if (curr_pair.second <= _s_state.latest_node_offset_match_start or
                node_id != _s_state.latest_node_offset_node_id)
            {
                match_offsets = curr_pair;
                _s_state.latest_node_offset_match_start = match_offsets.first;
                _s_state.latest_node_offset_match_end = match_offsets.second;
                break;
            }
        }
    }
    _s_state.latest_node_offset_node_id = node_id;
    if (match_offsets.first != -1) {
        match_offsets.first = str::byte_pos_to_symb_pos(text, match_offsets.first);
        match_offsets.second = str::byte_pos_to_symb_pos(text, match_offsets.second);
    }

    CtAnchMatchList anchMatchList;
    int obj_search_start_offs = start_iter.get_offset();
    int obj_search_end_offs = match_offsets.first != -1 ? _s_state.latest_node_offset_match_start : (forward ? text_buffer->end().get_offset() : 0);
    if (not forward) {
        std::swap(obj_search_start_offs, obj_search_end_offs);
    }
    if (tree_iter.get_node_is_rich_text() and
        _check_pattern_in_object_between(tree_iter,
                                         re_pattern,
                                         obj_search_start_offs,
                                         obj_search_end_offs,
                                         forward,
                                         all_matches,
                                         anchMatchList))
    {
        // find_iter_anchlist_idx is always 0 for all_matches, changes only in iterative find
        if (not all_matches) {
            if (0u == _s_state.find_iter_anchlist_size) {
                // first iteration in anch match list
                _s_state.find_iter_anchlist_idx = forward ? 0u : anchMatchList.size() - 1u;
            }
            else if (anchMatchList.size() != _s_state.find_iter_anchlist_size) {
                spdlog::debug("?? find_iter_anchlist_size {}->{}", _s_state.find_iter_anchlist_size, anchMatchList.size());
                _s_state.find_iter_anchlist_idx = forward ? 0u : anchMatchList.size() - 1u;
            }
            else if (_s_state.find_iter_anchlist_idx >= anchMatchList.size()) {
                spdlog::debug("?? after anchMatchList of {}", anchMatchList.size());
                _s_state.find_iter_anchlist_idx = forward ? 0u : anchMatchList.size() - 1u;
            }
            if (not forward and _s_state.find_back_exclude_obj_offs_zero) {
                anchMatchList.clear();
                _s_state.find_iter_anchlist_size = 0u;
                _s_state.find_back_exclude_obj_offs_zero = false;
                spdlog::debug("find_back_exclude_obj_offs_zero");
                return false;
            }
            _s_state.find_iter_anchlist_size = anchMatchList.size();
            spdlog::debug("anchMatchList {}->{} {}/{}",
                obj_search_start_offs, obj_search_end_offs,
                _s_state.find_iter_anchlist_idx, _s_state.find_iter_anchlist_size);
        }
        match_offsets.first = anchMatchList[0]->start_offset;
        match_offsets.second = match_offsets.first + 1;
    }
    else {
        if (not all_matches) {
            if (0u != _s_state.find_iter_anchlist_size) {
                _s_state.find_iter_anchlist_size = 0u;
            }
        }
    }
    if (match_offsets.first == -1) return false;

    // match found!
    const int num_objs = _get_num_objs_before_offset(text_buffer, match_offsets.first);
    if (0u == anchMatchList.size()) {
        _s_state.latest_match_offsets.first = match_offsets.first + num_objs;
        _s_state.latest_match_offsets.second = match_offsets.second + num_objs;
    }
    else {
        // the match offset comes from the anchored widget
        _s_state.latest_match_offsets.first = match_offsets.first;
        _s_state.latest_match_offsets.second = match_offsets.second;
        _s_state.latest_node_offset_match_start = match_offsets.first - num_objs;
        _s_state.latest_node_offset_match_end = match_offsets.second - num_objs;
    }
    auto f_match_replace_link = [this, &tree_iter, &re_pattern](Glib::RefPtr<Gtk::TextBuffer> pTextBuffer,
                                                                const int textIterOffset,
                                                                const int startOffset,
                                                                int& endOffset)->bool{
        if (tree_iter.get_node_read_only()) return false;
        Gtk::TextIter textIter = pTextBuffer->get_iter_at_offset(textIterOffset);
        if (not textIter) {
            spdlog::warn("!! f_match_replace_link unexp no TextIter offset {}", textIterOffset);
            return false;
        }
        Glib::ustring tag_property = CtMiscUtil::link_check_around_cursor(pTextBuffer, textIter);
        if (tag_property.empty()) {
            spdlog::warn("!! f_match_replace_link unexp no link_check_around_cursor offset {}", textIterOffset);
            return false;
        }
        CtLinkEntry link_entry = CtMiscUtil::get_link_entry_from_property(tag_property);
        if (CtLinkType::None == link_entry.type) {
            spdlog::warn("!! f_match_replace_link unexp CtLinkType::None tag_property '{}'", tag_property.c_str());
            return false;
        }
        Glib::ustring text_searchable = link_entry.get_target_searchable();
        const Glib::ustring pre_text = text_searchable.substr(0, startOffset);
        const Glib::ustring origin_text = text_searchable.substr(startOffset, endOffset - startOffset);
        const Glib::ustring post_text = text_searchable.substr(endOffset);
        Glib::ustring replacer_text = _s_options.str_replace; /* use Glib::ustring to count symbols */
        // use re_pattern->replace for the cases with \n, maybe it even helps with groups
        if (_s_options.reg_exp) {
            replacer_text = re_pattern->replace(origin_text, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));
        }
        const Glib::ustring out_text_searchable = pre_text + replacer_text + post_text;
        link_entry.set_target_searchable(out_text_searchable);
        const Glib::ustring property_value = CtMiscUtil::get_link_property_from_entry(link_entry);
        _pCtMainWin->get_ct_actions()->apply_tag(CtConst::TAG_LINK, property_value);
        endOffset = startOffset + replacer_text.size();
        _s_state.replace_subsequent = true;
        _pCtMainWin->get_state_machine().update_state(tree_iter);
        tree_iter.pending_edit_db_node_buff();
        return true;
    };
    auto f_match_replace_image_png = [this, &tree_iter, &re_pattern](CtImagePng* pImagePng,
                                                                     const int startOffset,
                                                                     int& endOffset)->bool{
        if (tree_iter.get_node_read_only()) return false;
        CtLinkEntry link_entry = CtMiscUtil::get_link_entry_from_property(pImagePng->get_link());
        Glib::ustring text_searchable = link_entry.get_target_searchable();
        const Glib::ustring pre_text = text_searchable.substr(0, startOffset);
        const Glib::ustring origin_text = text_searchable.substr(startOffset, endOffset - startOffset);
        const Glib::ustring post_text = text_searchable.substr(endOffset);
        Glib::ustring replacer_text = _s_options.str_replace; /* use Glib::ustring to count symbols */
        // use re_pattern->replace for the cases with \n, maybe it even helps with groups
        if (_s_options.reg_exp) {
            replacer_text = re_pattern->replace(origin_text, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));
        }
        const Glib::ustring out_text_searchable = pre_text + replacer_text + post_text;
        link_entry.set_target_searchable(out_text_searchable);
        pImagePng->set_link(CtMiscUtil::get_link_property_from_entry(link_entry));
        endOffset = startOffset + replacer_text.size();
        _s_state.replace_subsequent = true;
        _pCtMainWin->get_state_machine().update_state(tree_iter);
        tree_iter.pending_edit_db_node_buff();
        return true;
    };
    auto f_match_replace_light_table = [this, &tree_iter, &re_pattern](CtTableLight* pTableLight,
                                                                       const int cellIdx,
                                                                       const int startOffset,
                                                                       int& endOffset)->bool{
        if (tree_iter.get_node_read_only()) return false;
        const std::pair<size_t, size_t> rowIdxColIdx = pTableLight->get_row_idx_col_idx(cellIdx);
        const Glib::ustring in_cell_text = pTableLight->get_cell_text(rowIdxColIdx.first, rowIdxColIdx.second);
        const Glib::ustring pre_text = in_cell_text.substr(0, startOffset);
        const Glib::ustring origin_text = in_cell_text.substr(startOffset, endOffset - startOffset);
        const Glib::ustring post_text = in_cell_text.substr(endOffset);
        Glib::ustring replacer_text = _s_options.str_replace; /* use Glib::ustring to count symbols */
        // use re_pattern->replace for the cases with \n, maybe it even helps with groups
        if (_s_options.reg_exp) {
            replacer_text = re_pattern->replace(origin_text, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));
        }
        const Glib::ustring out_cell_text = pre_text + replacer_text + post_text;
        pTableLight->set_cell_text(rowIdxColIdx.first, rowIdxColIdx.second, out_cell_text);
        endOffset = startOffset + replacer_text.size();
        _s_state.replace_subsequent = true;
        _pCtMainWin->get_state_machine().update_state(tree_iter);
        tree_iter.pending_edit_db_node_buff();
        return true;
    };
    auto f_match_replace_text_buffer = [this, &tree_iter, &re_pattern](Glib::RefPtr<Gtk::TextBuffer> pTextBuffer,
                                                                       const int startOffset,
                                                                       int& endOffset)->bool{
        if (tree_iter.get_node_read_only()) return false;
        Gtk::TextIter sel_start = pTextBuffer->get_iter_at_offset(startOffset);
        Gtk::TextIter sel_end = pTextBuffer->get_iter_at_offset(endOffset);
        Glib::ustring origin_text = sel_start.get_text(sel_end);
        Glib::ustring replacer_text = _s_options.str_replace; /* use Glib::ustring to count symbols */
        // use re_pattern->replace for the cases with \n, maybe it even helps with groups
        if (_s_options.reg_exp) {
            replacer_text = re_pattern->replace(origin_text, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));
        }
        pTextBuffer->erase(sel_start, sel_end);
        pTextBuffer->insert(pTextBuffer->get_iter_at_offset(startOffset), replacer_text);
        endOffset = startOffset + replacer_text.size();
        _s_state.replace_subsequent = true;
        _pCtMainWin->get_state_machine().update_state(tree_iter);
        tree_iter.pending_edit_db_node_buff();
        return true;
    };
    if (all_matches) {
        const gint64 node_id = tree_iter.get_node_id();
        const Glib::ustring node_name = tree_iter.get_node_name();
        const std::string node_hier_name = CtMiscUtil::get_node_hierarchical_name(tree_iter, "  /  ", false/*for_filename*/, true/*root_to_leaf*/);
        const Glib::ustring esc_node_hier_name = str::xml_escape(node_hier_name);
        const Glib::ustring text_tags = tree_iter.get_node_tags();
        const Glib::ustring node_name_w_tags = text_tags.empty() ? node_name : node_name + "\n [" +  _("Tags") + _(": ") + text_tags + "]";
        if (0u == anchMatchList.size()) {
            const int line_num = text_buffer->get_iter_at_offset(_s_state.latest_match_offsets.first).get_line()/*0-based indexing*/ + 1;
            const Glib::ustring line_content = CtTextIterUtil::get_line_content(text_buffer, _s_state.latest_match_offsets.second);
            if (_s_state.replace_active) {
                if (not f_match_replace_text_buffer(text_buffer,
                                                    _s_state.latest_match_offsets.first,
                                                    _s_state.latest_match_offsets.second))
                {
                    return false;
                }
            }
            (void)_s_state.match_store->add_row(node_id,
                                                node_name_w_tags,
                                                esc_node_hier_name,
                                                _s_state.latest_match_offsets.first,
                                                _s_state.latest_match_offsets.second,
                                                line_num,
                                                line_content,
                                                CtAnchWidgType::None, 0, 0, 0);
        }
        else {
            int last_obj_offs{0};
            size_t last_cell_idx{0u};
            int accumulated_delta_offs{0};
            for (std::shared_ptr<CtAnchMatch>& pAnchMatch : anchMatchList) {
                _s_state.latest_match_offsets.first = pAnchMatch->start_offset;
                _s_state.latest_match_offsets.second = _s_state.latest_match_offsets.first + 1;
                const int line_num = text_buffer->get_iter_at_offset(_s_state.latest_match_offsets.first).get_line()/*0-based indexing*/ + 1;
                if ( _s_state.replace_active and
                     ( CtAnchWidgType::CodeBox == pAnchMatch->anch_type or
                       CtAnchWidgType::TableHeavy == pAnchMatch->anch_type or
                       CtAnchWidgType::TableLight == pAnchMatch->anch_type or
                       CtAnchWidgType::ImagePng == pAnchMatch->anch_type or
                       CtAnchWidgType::Link == pAnchMatch->anch_type ) )
                {
                    if ( last_obj_offs == pAnchMatch->start_offset and
                         last_cell_idx == pAnchMatch->anch_cell_idx )
                    {
                        // we have a subsequent replace in the same cell, we need to apply the accumulated delta
                        pAnchMatch->anch_offs_start += accumulated_delta_offs;
                        pAnchMatch->anch_offs_end += accumulated_delta_offs;
                    }
                    else {
                        last_obj_offs = pAnchMatch->start_offset;
                        last_cell_idx = pAnchMatch->anch_cell_idx;
                        accumulated_delta_offs = 0;
                    }
                    if (CtAnchWidgType::CodeBox == pAnchMatch->anch_type) {
                        if (auto pCodebox = dynamic_cast<CtCodebox*>(pAnchMatch->pAnchWidg)) {
                            const int prev_anch_offs_end = pAnchMatch->anch_offs_end;
                            if (not f_match_replace_text_buffer(pCodebox->get_text_view().get_buffer(),
                                                                pAnchMatch->anch_offs_start,
                                                                pAnchMatch->anch_offs_end))
                            {
                                return false;
                            }
                            accumulated_delta_offs += (pAnchMatch->anch_offs_end - prev_anch_offs_end);
                        }
                        else {
                            spdlog::warn("!! {} unexp no CtCodebox", __FUNCTION__);
                        }
                    }
                    else if (CtAnchWidgType::TableHeavy == pAnchMatch->anch_type) {
                        if (auto pTable = dynamic_cast<CtTableHeavy*>(pAnchMatch->pAnchWidg)) {
                            const std::pair<size_t, size_t> rowIdxColIdx = pTable->get_row_idx_col_idx(pAnchMatch->anch_cell_idx);
                            const int prev_anch_offs_end = pAnchMatch->anch_offs_end;
                            if (not f_match_replace_text_buffer(pTable->get_buffer(rowIdxColIdx.first, rowIdxColIdx.second),
                                                                pAnchMatch->anch_offs_start,
                                                                pAnchMatch->anch_offs_end))
                            {
                                return false;
                            }
                            accumulated_delta_offs += (pAnchMatch->anch_offs_end - prev_anch_offs_end);
                        }
                        else {
                            spdlog::warn("!! {} unexp no CtTableHeavy", __FUNCTION__);
                        }
                    }
                    else if (CtAnchWidgType::TableLight == pAnchMatch->anch_type) {
                        if (auto pTable = dynamic_cast<CtTableLight*>(pAnchMatch->pAnchWidg)) {
                            const int prev_anch_offs_end = pAnchMatch->anch_offs_end;
                            if (not f_match_replace_light_table(pTable,
                                                                pAnchMatch->anch_cell_idx,
                                                                pAnchMatch->anch_offs_start,
                                                                pAnchMatch->anch_offs_end))
                            {
                                return false;
                            }
                            accumulated_delta_offs += (pAnchMatch->anch_offs_end - prev_anch_offs_end);
                        }
                        else {
                            spdlog::warn("!! {} unexp no CtTableLight", __FUNCTION__);
                        }
                    }
                    else if (CtAnchWidgType::ImagePng == pAnchMatch->anch_type) {
                        if (auto pImagePng = dynamic_cast<CtImagePng*>(pAnchMatch->pAnchWidg)) {
                            const int prev_anch_offs_end = pAnchMatch->anch_offs_end;
                            if (not f_match_replace_image_png(pImagePng,
                                                              pAnchMatch->anch_offs_start,
                                                              pAnchMatch->anch_offs_end))
                            {
                                return false;
                            }
                            accumulated_delta_offs += (pAnchMatch->anch_offs_end - prev_anch_offs_end);
                        }
                        else {
                            spdlog::warn("!! {} unexp no CtImagePng", __FUNCTION__);
                        }
                    }
                    else if (CtAnchWidgType::Link == pAnchMatch->anch_type) {
                        // we cannot rely on pAnchMatch->pAnchWidg as that is immediately freed
                        const int prev_anch_offs_end = pAnchMatch->anch_offs_end;
                        if (not f_match_replace_link(text_buffer,
                                                     pAnchMatch->start_offset,
                                                     pAnchMatch->anch_offs_start,
                                                     pAnchMatch->anch_offs_end))
                        {
                            return false;
                        }
                        accumulated_delta_offs += (pAnchMatch->anch_offs_end - prev_anch_offs_end);
                    }
                }
                (void)_s_state.match_store->add_row(node_id,
                                                    node_name_w_tags,
                                                    esc_node_hier_name,
                                                    _s_state.latest_match_offsets.first,
                                                    _s_state.latest_match_offsets.second,
                                                    line_num,
                                                    pAnchMatch->line_content,
                                                    pAnchMatch->anch_type,
                                                    pAnchMatch->anch_cell_idx,
                                                    pAnchMatch->anch_offs_start,
                                                    pAnchMatch->anch_offs_end);
            }
        }
    }
    else {
        CtTreeIter curr_tree_iter = _pCtMainWin->curr_tree_iter();
        if (not curr_tree_iter or curr_tree_iter.get_node_id() != tree_iter.get_node_id()) {
            _pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
        }
        CtTextView& ct_text_view = _pCtMainWin->get_text_view();
        ct_text_view.set_selection_at_offset_n_delta(_s_state.latest_match_offsets.first, match_offsets.second - match_offsets.first);
        ct_text_view.mm().scroll_to(text_buffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
        if (anchMatchList.size() > 0u) {
            auto& pAnchMatch = anchMatchList[_s_state.find_iter_anchlist_idx];
            if (_s_state.replace_active) {
                if (CtAnchWidgType::CodeBox == pAnchMatch->anch_type) {
                    if (CtCodebox* pCodebox = dynamic_cast<CtCodebox*>(pAnchMatch->pAnchWidg)) {
                        if (not f_match_replace_text_buffer(pCodebox->get_text_view().get_buffer(),
                                                            pAnchMatch->anch_offs_start,
                                                            pAnchMatch->anch_offs_end))
                        {
                            return false;
                        }
                    }
                    else {
                        spdlog::warn("!! {} unexp no CtCodebox", __FUNCTION__);
                    }
                }
                else if (CtAnchWidgType::TableHeavy == pAnchMatch->anch_type) {
                    if (auto pTable = dynamic_cast<CtTableHeavy*>(pAnchMatch->pAnchWidg)) {
                        const std::pair<size_t, size_t> rowIdxColIdx = pTable->get_row_idx_col_idx(pAnchMatch->anch_cell_idx);
                        if (not f_match_replace_text_buffer(pTable->get_buffer(rowIdxColIdx.first, rowIdxColIdx.second),
                                                            pAnchMatch->anch_offs_start,
                                                            pAnchMatch->anch_offs_end))
                        {
                            return false;
                        }
                    }
                    else {
                        spdlog::warn("!! {} unexp no CtTableHeavy", __FUNCTION__);
                    }
                }
                else if (CtAnchWidgType::TableLight == pAnchMatch->anch_type) {
                    if (auto pTable = dynamic_cast<CtTableLight*>(pAnchMatch->pAnchWidg)) {
                        if (not f_match_replace_light_table(pTable,
                                                            pAnchMatch->anch_cell_idx,
                                                            pAnchMatch->anch_offs_start,
                                                            pAnchMatch->anch_offs_end))
                        {
                            return false;
                        }
                    }
                    else {
                        spdlog::warn("!! {} unexp no CtTableLight", __FUNCTION__);
                    }
                }
                else if (CtAnchWidgType::ImagePng == pAnchMatch->anch_type) {
                    if (auto pImagePng = dynamic_cast<CtImagePng*>(pAnchMatch->pAnchWidg)) {
                        if (not f_match_replace_image_png(pImagePng,
                                                          pAnchMatch->anch_offs_start,
                                                          pAnchMatch->anch_offs_end))
                        {
                            return false;
                        }
                    }
                    else {
                        spdlog::warn("!! {} unexp no CtImagePng", __FUNCTION__);
                    }
                }
                else if (CtAnchWidgType::Link == pAnchMatch->anch_type) {
                    // we cannot rely on pAnchMatch->pAnchWidg as that is immediately freed
                    if (not f_match_replace_link(text_buffer,
                                                 pAnchMatch->start_offset,
                                                 pAnchMatch->anch_offs_start,
                                                 pAnchMatch->anch_offs_end))
                    {
                        return false;
                    }
                }
            }
            CtActions::find_match_in_obj_focus(_s_state.latest_match_offsets.first,
                                               text_buffer,
                                               _pCtMainWin,
                                               tree_iter,
                                               pAnchMatch->anch_type,
                                               pAnchMatch->anch_cell_idx,
                                               pAnchMatch->anch_offs_start,
                                               pAnchMatch->anch_offs_end);
        }
        else if (_s_state.replace_active) {
            if (not f_match_replace_text_buffer(text_buffer,
                                                _s_state.latest_match_offsets.first,
                                                _s_state.latest_match_offsets.second))
            {
                return false;
            }
            ct_text_view.set_selection_at_offset_n_delta(_s_state.latest_match_offsets.first,
                _s_state.latest_match_offsets.second - _s_state.latest_match_offsets.first);
        }
    }
    return true;
}

/*static*/void CtActions::find_match_in_obj_focus(const int obj_offset,
                                                  Glib::RefPtr<Gtk::TextBuffer> pTextBuffer,
                                                  CtMainWin* pCtMainWin,
                                                  const CtTreeIter& tree_iter,
                                                  const CtAnchWidgType anch_type,
                                                  const size_t anch_cell_idx,
                                                  const int anch_offs_start,
                                                  const int anch_offs_end)
{
    //spdlog::debug("{} obj={} cell={} {}->{}", __FUNCTION__, obj_offset, anch_cell_idx, anch_offs_start, anch_offs_end);
    Gtk::TextIter anchor_iter = pTextBuffer->get_iter_at_offset(obj_offset);
    if (CtAnchWidgType::Link == anch_type) {
        std::optional<Glib::ustring> tag_name = CtTextIterUtil::iter_get_tag_startingwith(anchor_iter, CtConst::TAG_LINK_PREFIX);
        if (tag_name.has_value()) {
            Glib::RefPtr<Gtk::TextTag> pTextTag = pCtMainWin->get_text_tag_table()->lookup(tag_name.value());
            Gtk::TextIter textIterEndTmp{anchor_iter};
            (void)textIterEndTmp.forward_to_tag_toggle(pTextTag);
            Gtk::TextIter textIterStartTmp{anchor_iter};
            (void)textIterStartTmp.backward_to_tag_toggle(pTextTag);
            const int start_offset = textIterStartTmp.get_offset();
            const int end_offset = textIterEndTmp.get_offset();
            pCtMainWin->get_text_view().set_selection_at_offset_n_delta(start_offset, end_offset - start_offset, pTextBuffer);
        }
        else {
            spdlog::debug("? {} !tag_name", __FUNCTION__);
        }
        return;
    }
    Glib::RefPtr<Gtk::TextChildAnchor> pChildAnchor = anchor_iter.get_child_anchor();
    if (pChildAnchor) {
        CtAnchoredWidget* pCtAnchoredWidget = tree_iter.get_anchored_widget(pChildAnchor);
        if (pCtAnchoredWidget) {
            switch (anch_type) {
                case CtAnchWidgType::CodeBox: {
                    if (auto pCodebox = dynamic_cast<CtCodebox*>(pCtAnchoredWidget)) {
                        pCodebox->get_text_view().set_selection_at_offset_n_delta(anch_offs_start,
                            anch_offs_end - anch_offs_start);
                    }
                    else {
                        spdlog::debug("? {} !pCodebox", __FUNCTION__);
                    }
                } break;
                case CtAnchWidgType::TableHeavy: [[fallthrough]];
                case CtAnchWidgType::TableLight: {
                    if (auto pTable = dynamic_cast<CtTableCommon*>(pCtAnchoredWidget)) {
                        const size_t num_columns = pTable->get_num_columns();
                        const size_t rowIdx = anch_cell_idx / num_columns;
                        const size_t colIdx = anch_cell_idx % num_columns;
                        pTable->set_current_row_column(rowIdx, colIdx);
                        pTable->grab_focus();
                        pTable->set_selection_at_offset_n_delta(anch_offs_start,
                            anch_offs_end - anch_offs_start);
                    }
                    else {
                        spdlog::debug("? {} !pTable", __FUNCTION__);
                    }
                } break;
                default: break;
            }
        }
        else {
            spdlog::debug("? {} !pCtAnchoredWidget", __FUNCTION__);
        }
    }
    else {
        spdlog::debug("? {} !pChildAnchor", __FUNCTION__);
    }
}

bool CtActions::_check_pattern_in_object(Glib::RefPtr<Glib::Regex> re_pattern,
                                         CtAnchoredWidget* pAnchWidg,
                                         CtAnchMatchList& anchMatchList)
{
    bool retVal{false};
    const CtAnchWidgType anchWidgType = pAnchWidg->get_type();
    switch (anchWidgType) {
        case CtAnchWidgType::ImageEmbFile: {
            if (CtImageEmbFile* pImageEmbFile = dynamic_cast<CtImageEmbFile*>(pAnchWidg)) {
                Glib::ustring text = pImageEmbFile->get_file_name().string();
                if (_s_options.accent_insensitive) {
                    text = str::diacritical_to_ascii(text);
                }
                if (re_pattern->match(text)) {
                    auto pAnchMatch = std::make_shared<CtAnchMatch>();
                    pAnchMatch->start_offset = pAnchWidg->getOffset();
                    pAnchMatch->line_content = text;
                    pAnchMatch->anch_type = anchWidgType;
                    pAnchMatch->pAnchWidg = pAnchWidg;
                    anchMatchList.push_back(pAnchMatch);
                    retVal = true;
                }
            }
            else {
                spdlog::warn("!! unexp no CtImageEmbFile");
            }
        } break;
        case CtAnchWidgType::ImageAnchor: {
            if (CtImageAnchor* pImageAnchor = dynamic_cast<CtImageAnchor*>(pAnchWidg)) {
                Glib::ustring text = pImageAnchor->get_anchor_name();
                if (_s_options.accent_insensitive) {
                    text = str::diacritical_to_ascii(text);
                }
                if (re_pattern->match(text)) {
                    auto pAnchMatch = std::make_shared<CtAnchMatch>();
                    pAnchMatch->start_offset = pAnchWidg->getOffset();
                    pAnchMatch->line_content = text;
                    pAnchMatch->anch_type = anchWidgType;
                    pAnchMatch->pAnchWidg = pAnchWidg;
                    anchMatchList.push_back(pAnchMatch);
                    retVal = true;
                }
            }
            else {
                spdlog::warn("!! unexp no CtImageAnchor");
            }
        } break;
        case CtAnchWidgType::ImagePng: {
            if (CtImagePng* pCtImagePng = dynamic_cast<CtImagePng*>(pAnchWidg)) {
                CtLinkEntry link_entry = CtMiscUtil::get_link_entry_from_property(pCtImagePng->get_link());
                if (CtLinkType::None != link_entry.type) {
                    Glib::ustring text = link_entry.get_target_searchable();
                    if (_s_options.accent_insensitive) {
                        text = str::diacritical_to_ascii(text);
                    }
                    Glib::MatchInfo match_info;
                    if (re_pattern->match(text, match_info)) {
                        CtAnchMatchList localAnchMatchList;
                        while (match_info.matches()) {
                            int match_start_offset, match_end_offset;
                            match_info.fetch_pos(0, match_start_offset, match_end_offset);
                            match_start_offset = str::byte_pos_to_symb_pos(text, match_start_offset);
                            match_end_offset = str::byte_pos_to_symb_pos(text, match_end_offset);
                            auto pAnchMatch = std::make_shared<CtAnchMatch>();
                            pAnchMatch->start_offset = pAnchWidg->getOffset();
                            pAnchMatch->line_content = text;
                            pAnchMatch->anch_type = anchWidgType;
                            pAnchMatch->anch_offs_start = match_start_offset;
                            pAnchMatch->anch_offs_end = match_end_offset;
                            pAnchMatch->pAnchWidg = pAnchWidg;
                            localAnchMatchList.push_back(pAnchMatch);
                            match_info.next();
                        }
                        for (auto& pAnchMatch : localAnchMatchList) {
                            anchMatchList.push_back(pAnchMatch);
                        }
                        retVal = true;
                    }
                }
            }
            else {
                spdlog::warn("!! unexp no CtImagePng");
            }
        } break;
        case CtAnchWidgType::Link: {
            if (CtAnchWidgLink* pAnchWidgLink = dynamic_cast<CtAnchWidgLink*>(pAnchWidg)) {
                Glib::ustring text = pAnchWidgLink->get_target_searchable();
                if (_s_options.accent_insensitive) {
                    text = str::diacritical_to_ascii(text);
                }
                Glib::MatchInfo match_info;
                if (re_pattern->match(text, match_info)) {
                    CtAnchMatchList localAnchMatchList;
                    while (match_info.matches()) {
                        int match_start_offset, match_end_offset;
                        match_info.fetch_pos(0, match_start_offset, match_end_offset);
                        match_start_offset = str::byte_pos_to_symb_pos(text, match_start_offset);
                        match_end_offset = str::byte_pos_to_symb_pos(text, match_end_offset);
                        auto pAnchMatch = std::make_shared<CtAnchMatch>();
                        pAnchMatch->start_offset = pAnchWidg->getOffset();
                        pAnchMatch->line_content = text;
                        pAnchMatch->anch_type = anchWidgType;
                        pAnchMatch->anch_offs_start = match_start_offset;
                        pAnchMatch->anch_offs_end = match_end_offset;
                        //pAnchMatch->pAnchWidg = pAnchWidg; THIS TEMPORARY ANCHOR-LIKE WIDGET WILL BE FREED IMMEDIATELY!
                        localAnchMatchList.push_back(pAnchMatch);
                        match_info.next();
                    }
                    for (auto& pAnchMatch : localAnchMatchList) {
                        anchMatchList.push_back(pAnchMatch);
                    }
                    retVal = true;
                }
            }
            else {
                spdlog::warn("!! unexp no CtAnchWidgLink");
            }
        } break;
        case CtAnchWidgType::CodeBox: {
            if (CtCodebox* pCodebox = dynamic_cast<CtCodebox*>(pAnchWidg)) {
                Glib::ustring text = pCodebox->get_text_content();
                if (_s_options.accent_insensitive) {
                    text = str::diacritical_to_ascii(text);
                }
                Glib::MatchInfo match_info;
                if (re_pattern->match(text, match_info)) {
                    CtAnchMatchList localAnchMatchList;
                    while (match_info.matches()) {
                        int match_start_offset, match_end_offset;
                        match_info.fetch_pos(0, match_start_offset, match_end_offset);
                        match_start_offset = str::byte_pos_to_symb_pos(text, match_start_offset);
                        match_end_offset = str::byte_pos_to_symb_pos(text, match_end_offset);
                        auto pAnchMatch = std::make_shared<CtAnchMatch>();
                        pAnchMatch->start_offset = pAnchWidg->getOffset();
                        pAnchMatch->line_content = CtTextIterUtil::get_line_content(pCodebox->get_buffer(), match_end_offset);
                        pAnchMatch->anch_type = anchWidgType;
                        pAnchMatch->anch_offs_start = match_start_offset;
                        pAnchMatch->anch_offs_end = match_end_offset;
                        pAnchMatch->pAnchWidg = pAnchWidg;
                        localAnchMatchList.push_back(pAnchMatch);
                        match_info.next();
                    }
                    for (auto& pAnchMatch : localAnchMatchList) {
                        anchMatchList.push_back(pAnchMatch);
                    }
                    retVal = true;
                }
            }
            else {
                spdlog::warn("!! {} unexp no CtCodebox", __FUNCTION__);
            }
        } break;
        case CtAnchWidgType::TableHeavy:
        case CtAnchWidgType::TableLight: {
            if (auto pTable = dynamic_cast<CtTableCommon*>(pAnchWidg)) {
                std::vector<std::vector<Glib::ustring>> rows;
                pTable->write_strings_matrix(rows);
                CtAnchMatchList localAnchMatchList;
                size_t rowIdx{0u};
                for (auto& row : rows) {
                    size_t colIdx{0u};
                    for (Glib::ustring& text : row) {
                        if (_s_options.accent_insensitive) {
                            text = str::diacritical_to_ascii(text);
                        }
                        Glib::MatchInfo match_info;
                        if (re_pattern->match(text, match_info)) {
                            while (match_info.matches()) {
                                int match_start_offset, match_end_offset;
                                match_info.fetch_pos(0, match_start_offset, match_end_offset);
                                match_start_offset = str::byte_pos_to_symb_pos(text, match_start_offset);
                                match_end_offset = str::byte_pos_to_symb_pos(text, match_end_offset);
                                auto pAnchMatch = std::make_shared<CtAnchMatch>();
                                pAnchMatch->start_offset = pAnchWidg->getOffset();
                                pAnchMatch->line_content = pTable->get_line_content(rowIdx, colIdx, match_end_offset);
                                pAnchMatch->anch_type = anchWidgType;
                                pAnchMatch->anch_offs_start = match_start_offset;
                                pAnchMatch->anch_offs_end = match_end_offset;
                                pAnchMatch->anch_cell_idx = pTable->get_num_columns()*rowIdx + colIdx;
                                pAnchMatch->pAnchWidg = pAnchWidg;
                                localAnchMatchList.push_back(pAnchMatch);
                                match_info.next();
                            }
                        }
                        ++colIdx;
                    }
                    ++rowIdx;
                }
                if (localAnchMatchList.size() > 0u) {
                    for (auto& pAnchMatch : localAnchMatchList) {
                        anchMatchList.push_back(pAnchMatch);
                    }
                    retVal = true;
                }
            }
            else {
                spdlog::warn("!! {} unexp no CtTableCommon", __FUNCTION__);
            }
        } break;
        default: break;
    }
    return retVal;
}

bool CtActions::_check_pattern_in_object_between(CtTreeIter tree_iter,
                                                 Glib::RefPtr<Glib::Regex> re_pattern,
                                                 int start_offset,
                                                 int end_offset,
                                                 const bool forward,
                                                 const bool all_matches,
                                                 CtAnchMatchList& anchMatchList)
{
    bool retVal{false};
    std::list<CtAnchoredWidget*> obj_vec = tree_iter.get_anchored_widgets(start_offset, end_offset, true/*also_links*/);
    if (not forward) {
        std::reverse(obj_vec.begin(), obj_vec.end());
    }
    for (CtAnchoredWidget* pAnchWidg : obj_vec) {
        if (_check_pattern_in_object(re_pattern, pAnchWidg, anchMatchList)) {
            if (not retVal) {
                retVal = true;
            }
            if (not all_matches) {
                break;
            }
        }
    }
    // cleanup for get_anchored_widgets with also_links!
    for (CtAnchoredWidget* pAnchWidg : obj_vec) {
        if (CtAnchWidgType::Link == pAnchWidg->get_type()) {
            delete pAnchWidg;
        }
    }
    return retVal;
}

int CtActions::_get_num_objs_before_offset(Glib::RefPtr<Gtk::TextBuffer> text_buffer, int max_offset)
{
    int num_objs = 0;
    int local_limit_offset = max_offset;
    Gtk::TextIter curr_iter = text_buffer->get_iter_at_offset(0);
    int curr_offset = curr_iter.get_offset();
    while (curr_offset <= local_limit_offset) {
        auto anchor = curr_iter.get_child_anchor();
        if (anchor) {
            num_objs += 1;
            local_limit_offset += 1;
        }
        if (not curr_iter.forward_char())
            break;
        int next_offset = curr_iter.get_offset();
        if (next_offset == curr_offset)
            break;
        curr_offset = next_offset;
    }
    return num_objs;
}

void CtActions::_update_all_matches_progress()
{
    double frac = double(_s_state.processed_nodes)/double(_s_state.counted_nodes);
    _pCtMainWin->get_status_bar().progressBar.set_fraction(frac);
    if (_s_state.matches_num != _s_state.latest_matches) {
        _s_state.latest_matches = _s_state.matches_num;
        _pCtMainWin->get_status_bar().progressBar.set_text(std::to_string(_s_state.matches_num));
    }
    while (gtk_events_pending()) gtk_main_iteration();
}
