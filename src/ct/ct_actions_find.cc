/*
 * ct_actions_find.cc
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
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <glibmm/regex.h>
#include <regex>
#include "ct_image.h"
#include "ct_dialogs.h"
#include "ct_logging.h"

void CtActions::_find_init()
{
    _s_state.match_store = CtMatchDialogStore::create();
    std::time_t curr_time = std::time(nullptr);
    std::time_t yesterday_time = curr_time - 86400; //24*60*60
    _s_options.ts_cre_after  = {yesterday_time, false};
    _s_options.ts_mod_after  = {yesterday_time, false};
    _s_options.ts_cre_before = {curr_time, false};
    _s_options.ts_mod_before = {curr_time, false};
}

//"""Search for a pattern in the selected Node"""
void CtActions::find_in_selected_node()
{
    if (not _is_there_selected_node_or_error()) return;
    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = _pCtMainWin->get_text_view().get_buffer();

    std::string pattern;
    if (not _s_state.from_find_iterated) {
        _s_state.latest_node_offset = -1;
        auto iter_insert = curr_buffer->get_iter_at_mark(curr_buffer->get_insert());
        auto iter_bound = curr_buffer->get_iter_at_mark(curr_buffer->get_selection_bound());
        auto entry_predefined_text = curr_buffer->get_text(iter_insert, iter_bound);
        if (entry_predefined_text.length()) {
            _s_options.str_find = entry_predefined_text;
        }
        std::string title = _s_state.replace_active ? _("Replace in Current Node") : _("Search in Current Node");
        pattern = CtDialogs::dialog_search(_pCtMainWin, title, _s_options, _s_state.replace_active, false/*multiple_nodes*/);
        if (entry_predefined_text.length()) {
            curr_buffer->move_mark(curr_buffer->get_insert(), iter_insert);
            curr_buffer->move_mark(curr_buffer->get_selection_bound(), iter_bound);
        }
        if (pattern.empty()) return;
        _s_state.curr_find_pattern = pattern;
        _s_state.curr_find_type = CtCurrFindType::SingleNode;
    }
    else {
        pattern = _s_state.curr_find_pattern;
    }
    Glib::RefPtr<Glib::Regex> re_pattern = _create_re_pattern(pattern);
    if (not re_pattern) return;

    bool forward = _s_options.direction_fw;
    if (_s_state.from_find_back) {
        forward = not forward;
        _s_state.from_find_back = false;
    }
    bool first_fromsel = _s_options.all_firstsel_firstall == 1;
    bool all_matches = _s_options.all_firstsel_firstall == 0;
    _s_state.matches_num = 0;

    // searching start
    auto on_scope_exit = scope_guard([&](void*) { _pCtMainWin->user_active() = true; });
    _pCtMainWin->user_active() = false;

    if (all_matches) {
        _s_state.match_store->clear();
        _s_state.match_store->saved_path.clear();
        _s_state.all_matches_first_in_node = true;
    }
    while (_parse_node_content_iter(_pCtMainWin->curr_tree_iter(),
                                    curr_buffer,
                                    re_pattern,
                                    forward,
                                    first_fromsel,
                                    all_matches,
                                    true/*first_node*/))
    {
        _s_state.matches_num += 1;
        if (not all_matches) break;
    }
    if (0 == _s_state.matches_num) {
        CtDialogs::info_dialog(str::format(_("The pattern '%s' was not found"), str::xml_escape(pattern)), *_pCtMainWin);
    }
    else if (all_matches) {
        _s_state.match_dialog_title = "'" + _s_options.str_find + "'  -  " + std::to_string(_s_state.matches_num) + CtConst::CHAR_SPACE + _("Matches");
        CtDialogs::match_dialog(_s_state.match_dialog_title, _pCtMainWin, _s_state.match_store);
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
    if (not _is_there_selected_node_or_error()) return;
    CtTextView& ctTextView = _pCtMainWin->get_text_view();
    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = ctTextView.get_buffer();
    CtStatusBar& ctStatusBar = _pCtMainWin->get_status_bar();
    CtTreeStore& ctTreeStore = _pCtMainWin->get_tree_store();
    CtTreeView& ctTreeView = _pCtMainWin->get_tree_view();

    Glib::ustring title;
    Glib::ustring pattern;
    if (not _s_state.from_find_iterated) {
        _s_state.latest_node_offset = -1;
        Gtk::TextIter iter_insert = curr_buffer->get_insert()->get_iter();
        Gtk::TextIter iter_bound = curr_buffer->get_selection_bound()->get_iter();
        Glib::ustring entry_predefined_text = curr_buffer->get_text(iter_insert, iter_bound);
        if (not entry_predefined_text.empty()) {
            _s_options.str_find = entry_predefined_text;
        }
        title = _s_state.replace_active ? _("Replace in Multiple Nodes...") : _("Find in Multiple Nodes...");
        pattern = CtDialogs::dialog_search(_pCtMainWin, title, _s_options, _s_state.replace_active, true/*multiple_nodes*/);
        if (not entry_predefined_text.empty()) {
            curr_buffer->move_mark(curr_buffer->get_insert(), iter_insert);
            curr_buffer->move_mark(curr_buffer->get_selection_bound(), iter_bound);
        }
        if (not pattern.empty()) {
            _s_state.curr_find_pattern = pattern;
            _s_state.curr_find_type = CtCurrFindType::MultipleNodes;
        }
        else {
            return;
        }
    }
    else {
        pattern = _s_state.curr_find_pattern;
    }
    Glib::RefPtr<Glib::Regex> re_pattern = _create_re_pattern(pattern);
    if (not re_pattern) return;

    CtTreeIter starting_tree_iter = _pCtMainWin->curr_tree_iter();
    Gtk::TreeIter node_iter;
    int current_cursor_pos = curr_buffer->property_cursor_position();
    bool forward = _s_options.direction_fw;
    if (_s_state.from_find_back) {
        forward = not forward;
        _s_state.from_find_back = false;
    }
    bool first_fromsel = _s_options.all_firstsel_firstall == 1;
    bool all_matches = _s_options.all_firstsel_firstall == 0;
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
        _s_state.match_store->clear();
        _s_state.match_store->saved_path.clear();
    }

    std::string tree_expanded_collapsed_string = ctTreeStore.treeview_get_tree_expanded_collapsed_string(ctTreeView);
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
        while (_parse_given_node_content(ct_node_iter, re_pattern, forward, first_fromsel, all_matches)) {
            _s_state.matches_num += 1;
            if (not all_matches or ctStatusBar.is_progress_stop()) break;
        }
        _s_state.processed_nodes += 1;
        if (_s_state.matches_num == 1 and not all_matches) break;
        if (_s_options.only_sel_n_subnodes and not _s_state.from_find_iterated) break;
        Gtk::TreeIter last_top_node_iter = node_iter; // we need this if we start from a node that is not in top level
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
    auto last_iterated_node = _pCtMainWin->curr_tree_iter();
    ctTreeStore.treeview_set_tree_expanded_collapsed_string(tree_expanded_collapsed_string, ctTreeView, _pCtConfig->nodesBookmExp);
    if (not _s_state.matches_num or all_matches) {
        ctTreeView.set_cursor_safe(starting_tree_iter);
        ctTextView.grab_focus();
        curr_buffer->place_cursor(curr_buffer->get_iter_at_offset(current_cursor_pos));
        ctTextView.scroll_to(curr_buffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
    }
    if (not _s_state.matches_num) {
        CtDialogs::info_dialog(str::format(_("The pattern '%s' was not found"), str::xml_escape(pattern)), *_pCtMainWin);
    }
    else {
        if (all_matches) {
            _s_state.match_dialog_title = "'" + _s_options.str_find + "'  -  " + std::to_string(_s_state.matches_num) + CtConst::CHAR_SPACE + _("Matches");
            CtDialogs::match_dialog(_s_state.match_dialog_title, _pCtMainWin, _s_state.match_store);
        }
        else {
            ctTreeView.set_cursor_safe(last_iterated_node);
            ctTextView.set_selection_at_offset_n_delta(_s_state.latest_match_offsets.first,
                _s_state.latest_match_offsets.second - _s_state.latest_match_offsets.first);
            ctTextView.scroll_to(ctTextView.get_buffer()->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
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
    switch (_s_state.curr_find_type) {
        case CtCurrFindType::SingleNode: {
            find_in_selected_node();
        } break;
        case CtCurrFindType::MultipleNodes: {
            find_in_multiple_nodes();
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
    if (not _is_there_selected_node_or_error()) return;
    _s_state.replace_active = true;
    find_in_selected_node();
    _s_state.replace_active = false;
}

// Replace the pattern in all the Tree Nodes
void CtActions::replace_in_multiple_nodes()
{
    if (not _is_tree_not_empty_or_error()) return;
    _s_state.replace_active = true;
    find_in_multiple_nodes();
    _s_state.replace_active = false;
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
    CtDialogs::match_dialog(_s_state.match_dialog_title, _pCtMainWin, _s_state.match_store);
}

// Returns True if pattern was found, False otherwise
bool CtActions::_parse_given_node_content(CtTreeIter node_iter,
                                          Glib::RefPtr<Glib::Regex> re_pattern,
                                          bool forward,
                                          bool first_fromsel,
                                          bool all_matches)
{
    std::optional<bool> optFirstNode;
    if (not _s_state.first_useful_node) {
        // first_fromsel plus first_node not already parsed
        if (not _pCtMainWin->curr_tree_iter() or node_iter.get_node_id() == _pCtMainWin->curr_tree_iter().get_node_id()) {
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
                return true;
            }
        }
        if (_s_options.node_name_n_tags) {
            if (_parse_node_name_n_tags_iter(node_iter, re_pattern, all_matches) and not all_matches) {
                return false;
            }
        }
    }

    CtTreeStore& ctTreeStore = _pCtMainWin->get_tree_store();
    if (not node_iter.get_node_children_are_excluded_from_search() or _s_options.override_exclusions) {
        // check for children
        if (not node_iter->children().empty()) {
            Gtk::TreeIter child_iter = forward ? node_iter->children().begin() : --node_iter->children().end();
            while (child_iter and not _pCtMainWin->get_status_bar().is_progress_stop()) {
                _s_state.all_matches_first_in_node = true;
                CtTreeIter ct_node_iter = ctTreeStore.to_ct_tree_iter(child_iter);
                while (_parse_given_node_content(ct_node_iter, re_pattern, forward, first_fromsel, all_matches)) {
                    _s_state.matches_num += 1;
                    if (not all_matches or _pCtMainWin->get_status_bar().is_progress_stop()) break;
                }
                if (_s_state.matches_num == 1 and not all_matches) break;
                if (forward) child_iter = ++child_iter;
                else         child_iter = --child_iter;
                _s_state.processed_nodes += 1;
                if (all_matches) {
                    _update_all_matches_progress();
                }
            }
        }
    }
    return false;
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
            Glib::ustring node_hier_name = CtMiscUtil::get_node_hierarchical_name(node_iter, " << ", false, false);
            Glib::ustring line_content = _get_first_line_content(node_iter.get_node_text_buffer());
            const Glib::ustring text_tags = node_iter.get_node_tags();
            _s_state.match_store->add_row(node_id,
                                          text_tags.empty() ? node_name : node_name + "\n [" +  _("Tags") + _(": ") + text_tags + "]",
                                          str::xml_escape(node_hier_name),
                                          0, 0, 1, line_content);
        }
        if (_s_state.replace_active and not node_iter.get_node_read_only()) {
            std::string replacer_text = _s_options.str_replace;
            node_name = re_pattern->replace(node_name, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));
            node_iter.set_node_name(node_name);
            node_iter.pending_edit_db_node_prop();
        }
        if (not all_matches) {
            _pCtMainWin->get_tree_view().set_cursor_safe(node_iter);
            _pCtMainWin->get_text_view().grab_focus();
        }
        _s_state.matches_num += 1;
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

    bool restore_modified;
    Gtk::TextIter buff_start_iter = text_buffer->begin();
    if (buff_start_iter.get_char() != '\n') {
        _s_state.newline_trick = true;
        restore_modified = not text_buffer->get_modified();
        text_buffer->insert(buff_start_iter, CtConst::CHAR_NEWLINE);
    }
    else {
        _s_state.newline_trick = false;
        restore_modified = false;
    }
    Gtk::TextIter start_iter;
    if ((first_fromsel and first_node) or (all_matches and not _s_state.all_matches_first_in_node)) {
        gint64 node_id = tree_iter.get_node_id();
        start_iter = _get_inner_start_iter(text_buffer, forward, node_id);
    }
    else {
        start_iter = forward ? text_buffer->begin() : text_buffer->end();
        if (all_matches) _s_state.all_matches_first_in_node = false;
    }

    bool pattern_found = _find_pattern(tree_iter, text_buffer, re_pattern, start_iter, forward, all_matches);

    if (_s_state.newline_trick) {
        buff_start_iter = text_buffer->begin();
        Gtk::TextIter buff_step_iter = buff_start_iter;
        (void)buff_step_iter.forward_char();
        text_buffer->erase(buff_start_iter, buff_step_iter);
        if (restore_modified) text_buffer->set_modified(false);
    }
    if (_s_state.replace_active and pattern_found)
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, false/*new_machine_state*/, &tree_iter);
    return pattern_found;
}

// Get start_iter when not at beginning or end
Gtk::TextIter CtActions::_get_inner_start_iter(Glib::RefPtr<Gtk::TextBuffer> text_buffer, bool forward, const gint64& node_id)
{
    Gtk::TextIter min_iter, max_iter;
    if (text_buffer->get_has_selection()) {
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
    if (_s_state.latest_node_offset != -1 and
        _s_state.latest_node_offset_node_id == node_id and
        _s_state.latest_node_offset == start_iter.get_offset())
    {
        if (forward) start_iter.forward_char();
        else         start_iter.backward_char();
    }
    _s_state.latest_node_offset_node_id = node_id;
    _s_state.latest_node_offset = start_iter.get_offset();
    //print self.latest_node_offset["n"], offsets, self.latest_node_offset["o"]
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

// """Returns (start_iter, end_iter) or (None, None)"""
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

    int start_offset = start_iter.get_offset();
    // # start_offset -= self.get_num_objs_before_offset(text_buffer, start_offset)
    std::pair<int, int> match_offsets{-1, -1};
    if (forward) {
        Glib::MatchInfo match_info;
        if (re_pattern->match(text, str::symb_pos_to_byte_pos(text, start_offset)/*start_position*/, match_info)) {
            if (match_info.matches()) {
                match_info.fetch_pos(0, match_offsets.first, match_offsets.second);
            }
        }
    }
    else {
        Glib::MatchInfo match_info;
        re_pattern->match(text, str::symb_pos_to_byte_pos(text, start_offset)/*string_len*/, 0/*start_position*/, match_info);
        while (match_info.matches()) {
            match_info.fetch_pos(0, match_offsets.first, match_offsets.second);
            match_info.next();
        }
    }
    if (match_offsets.first != -1) {
        match_offsets.first = str::byte_pos_to_symb_pos(text, match_offsets.first);
        match_offsets.second = str::byte_pos_to_symb_pos(text, match_offsets.second);
    }

    std::pair<int,int> obj_match_offsets{-1, -1};
    std::string obj_content;
    if (not _s_state.replace_active) {
        obj_match_offsets = _check_pattern_in_object_between(tree_iter, text_buffer, re_pattern,
            start_iter.get_offset(), match_offsets.first, forward, obj_content);
    }
    if (obj_match_offsets.first != -1) match_offsets = obj_match_offsets;
    if (match_offsets.first == -1) return false;

    // match found!
    int num_objs = 0;
    if (obj_match_offsets.first == -1)
        num_objs = _get_num_objs_before_offset(text_buffer, match_offsets.first);
    int final_start_offset = match_offsets.first + num_objs;
    int final_delta_offset = match_offsets.second - match_offsets.first;

    if (not _pCtMainWin->curr_tree_iter() or _pCtMainWin->curr_tree_iter().get_node_id() != tree_iter.get_node_id()) {
        _pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
    }
    CtTextView& ctTextView = _pCtMainWin->get_text_view();
    ctTextView.set_selection_at_offset_n_delta(final_start_offset, final_delta_offset);

    auto mark_insert = text_buffer->get_insert();
    Gtk::TextIter iter_insert = text_buffer->get_iter_at_mark(mark_insert);
    const int newline_trick_offset = _s_state.newline_trick ? 1 : 0;
    _s_state.latest_match_offsets.first = match_offsets.first + num_objs - newline_trick_offset;
    _s_state.latest_match_offsets.second = match_offsets.second + num_objs - newline_trick_offset;
    Gtk::TreeIter iterAllMatchesRow;
    if (all_matches) {
        const gint64 node_id = tree_iter.get_node_id();
        const Glib::ustring node_name = tree_iter.get_node_name();
        const std::string node_hier_name = CtMiscUtil::get_node_hierarchical_name(tree_iter, " << ", false, false);
        const std::string line_content = obj_match_offsets.first != -1 ? obj_content : _get_line_content(text_buffer, iter_insert);
        int line_num = text_buffer->get_iter_at_offset(_s_state.latest_match_offsets.first).get_line();
        if (not _s_state.newline_trick) { line_num += 1; }
        const Glib::ustring text_tags = tree_iter.get_node_tags();
        iterAllMatchesRow = _s_state.match_store->add_row(node_id,
                                                          text_tags.empty() ? node_name : node_name + "\n [" +  _("Tags") + _(": ") + text_tags + "]",
                                                          str::xml_escape(node_hier_name),
                                                          _s_state.latest_match_offsets.first,
                                                          _s_state.latest_match_offsets.second,
                                                          line_num,
                                                          line_content);
    }
    else {
        ctTextView.scroll_to(mark_insert, CtTextView::TEXT_SCROLL_MARGIN);
    }
    if (_s_state.replace_active) {
        if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return false;
        Gtk::TextIter sel_start, sel_end;
        text_buffer->get_selection_bounds(sel_start, sel_end);

        Glib::ustring origin_text = sel_start.get_text(sel_end);
        Glib::ustring replacer_text = _s_options.str_replace; /* should be Glib::ustring to count symbols */

        // use re_pattern->replace for the cases with \n, maybe it even helps with groups
        if (_s_options.reg_exp)
            replacer_text = re_pattern->replace(origin_text, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));

        text_buffer->erase(sel_start, sel_end);
        text_buffer->insert_at_cursor(replacer_text);
        _s_state.latest_match_offsets.second = _s_state.latest_match_offsets.first + replacer_text.size();
        if (all_matches) {
            (*iterAllMatchesRow)[_s_state.match_store->columns.end_offset] = _s_state.latest_match_offsets.second;
        }
        else {
            ctTextView.set_selection_at_offset_n_delta(_s_state.latest_match_offsets.first,
                                                       static_cast<int>(replacer_text.size()));
        }
        _pCtMainWin->get_state_machine().update_state();
        tree_iter.pending_edit_db_node_buff();
    }
    return true;
}

// Search for the pattern in the given object
Glib::ustring CtActions::_check_pattern_in_object(Glib::RefPtr<Glib::Regex> pattern, CtAnchoredWidget* obj)
{
    if (CtImageEmbFile* image = dynamic_cast<CtImageEmbFile*>(obj)) {
        Glib::ustring text = image->get_file_name().string();
        if (_s_options.accent_insensitive) {
            text = str::diacritical_to_ascii(text);
        }
        if (pattern->match(text)) return text;
    }
    else if (CtImageAnchor* image = dynamic_cast<CtImageAnchor*>(obj)) {
        Glib::ustring text = image->get_anchor_name();
        if (_s_options.accent_insensitive) {
            text = str::diacritical_to_ascii(text);
        }
        if (pattern->match(text)) return text;
    }
    else if (auto table = dynamic_cast<CtTableCommon*>(obj)) {
        std::vector<std::vector<Glib::ustring>> rows;
        table->write_strings_matrix(rows);
        for (auto& row : rows) {
            for (Glib::ustring& col : row) {
                if (_s_options.accent_insensitive) {
                    col = str::diacritical_to_ascii(col);
                }
                if (pattern->match(col)) {
                    return "<table>";
                }
            }
        }
    }
    else if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(obj)) {
        Glib::ustring text = codebox->get_text_content();
        if (_s_options.accent_insensitive) {
            text = str::diacritical_to_ascii(text);
        }
        if (pattern->match(text)) return "<codebox>";
    }
    return "";
}

// Search for the pattern in the given slice and direction
std::pair<int, int> CtActions::_check_pattern_in_object_between(CtTreeIter tree_iter,
                                                                Glib::RefPtr<Gtk::TextBuffer> text_buffer,
                                                                Glib::RefPtr<Glib::Regex> pattern,
                                                                int start_offset,
                                                                int end_offset,
                                                                bool forward,
                                                                std::string& obj_content)
{
    if (not forward) start_offset -= 1;
    if (end_offset < 0) {
        if (forward) {
            Gtk::TextIter start, end;
            text_buffer->get_bounds(start, end);
            end_offset = end.get_offset();
        } else
            end_offset = 0;
    }
    if (not forward) std::swap(start_offset, end_offset);

    std::list<CtAnchoredWidget*> obj_vec = tree_iter.get_anchored_widgets(start_offset, end_offset);
    if (not forward)
        std::reverse(obj_vec.begin(), obj_vec.end());
    for (auto element : obj_vec) {
        obj_content = _check_pattern_in_object(pattern, element);
        if (not obj_content.empty())
            return {element->getOffset(), element->getOffset() + 1};
    }
    return {-1, -1};
}

// Returns the num of objects from buffer start to the given offset
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

// Returns the Line Content Given the Text Iter
std::string CtActions::_get_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter text_iter)
{
    auto line_start = text_iter;
    auto line_end = text_iter;
    if (not line_start.backward_char()) return "";
    while (line_start.get_char() != '\n')
        if (not line_start.backward_char())
            break;
    if (line_start.get_char() == '\n')
        line_start.forward_char();
    while (line_end.get_char() != '\n')
        if (not line_end.forward_char())
            break;
    return text_buffer->get_text(line_start, line_end);
}

// Returns the First Not Empty Line Content Given the Text Buffer
std::string CtActions::_get_first_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer)
{
    Gtk::TextIter start_iter = text_buffer->get_iter_at_offset(0);
    while (start_iter.get_char() == '\n')
        if (not start_iter.forward_char())
            return "";
    Gtk::TextIter end_iter = start_iter;
    while (end_iter.get_char() != '\n')
        if (not end_iter.forward_char())
            break;
    return text_buffer->get_text(start_iter, end_iter);
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
