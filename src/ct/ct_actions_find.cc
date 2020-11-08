/*
 * ct_actions_find.cc
 *
 * Copyright 2009-2020
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
    s_state.match_store = CtMatchDialogStore::create();
    std::time_t curr_time = std::time(nullptr);
    std::time_t yesterday_time = curr_time - 86400; //24*60*60
    s_options.ts_cre_after  = {yesterday_time, false};
    s_options.ts_mod_after  = {yesterday_time, false};
    s_options.ts_cre_before = {curr_time, false};
    s_options.ts_mod_before = {curr_time, false};
}

//"""Search for a pattern in the selected Node"""
void CtActions::find_in_selected_node()
{
    if (!_is_there_selected_node_or_error()) return;
    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = _pCtMainWin->get_text_view().get_buffer();

    std::string entry_hint;
    std::string pattern;
    if (!s_state.from_find_iterated) {
        s_state.latest_node_offset = -1;
        auto iter_insert = curr_buffer->get_iter_at_mark(curr_buffer->get_insert());
        auto iter_bound = curr_buffer->get_iter_at_mark(curr_buffer->get_selection_bound());
        auto entry_predefined_text = curr_buffer->get_text(iter_insert, iter_bound);
        if (entry_predefined_text.length())
            s_options.search_replace_dict_find = entry_predefined_text;
        std::string title = s_state.replace_active ? _("Replace in Current Node...") : _("Search in Current Node...");
        pattern = _dialog_search(title, s_state.replace_active, false, true);
        if (entry_predefined_text.length()) {
            curr_buffer->move_mark(curr_buffer->get_insert(), iter_insert);
            curr_buffer->move_mark(curr_buffer->get_selection_bound(), iter_bound);
        }
        if (pattern.empty()) return;
        s_state.curr_find_where = "in_selected_node";
        s_state.curr_find_pattern = pattern;
    }
    else
        pattern = s_state.curr_find_pattern;

    Glib::RefPtr<Glib::Regex> re_pattern = _create_re_pattern(pattern);
    if (!re_pattern) return;

    bool forward = s_options.search_replace_dict_fw;
    if (s_state.from_find_back) {
        forward = !forward;
        s_state.from_find_back = false;
    }
    bool first_fromsel = s_options.search_replace_dict_a_ff_fa == 1;
    bool all_matches = s_options.search_replace_dict_a_ff_fa == 0;
    s_state.matches_num = 0;

    // searching start
    auto on_scope_exit = scope_guard([&](void*) { _pCtMainWin->user_active() = true; });
    _pCtMainWin->user_active() = false;

    if (all_matches) {
        s_state.match_store->clear();
        s_state.match_store->saved_path.clear();
        s_state.all_matches_first_in_node = true;
        while (_parse_node_content_iter(_pCtMainWin->curr_tree_iter(), curr_buffer, re_pattern, forward, first_fromsel, all_matches, true))
            s_state.matches_num += 1;
        }
    else if (_parse_node_content_iter(_pCtMainWin->curr_tree_iter(), curr_buffer, re_pattern, forward, first_fromsel, all_matches, true))
        s_state.matches_num = 1;

    if (s_state.matches_num == 0)
        CtDialogs::info_dialog(str::format(_("The pattern '%s' was not found"), pattern), *_pCtMainWin);
    else if (all_matches) {
        s_state.match_dialog_title = std::to_string(s_state.matches_num) + CtConst::CHAR_SPACE + _("Matches");
        CtDialogs::match_dialog(s_state.match_dialog_title, _pCtMainWin, s_state.match_store);
    }
    else if (s_options.search_replace_dict_idialog) {
        _iterated_find_dialog();
    }
}

static int _count_nodes(const Gtk::TreeNodeChildren& children) {
    int count = 1;
    for(auto& child: children) { count += _count_nodes(child.children()); }
    return count;
}

void CtActions::_find_in_all_nodes(bool for_current_node)
{
    if (!_is_there_selected_node_or_error()) return;
    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = _pCtMainWin->get_text_view().get_buffer();
    CtStatusBar& ctStatusBar = _pCtMainWin->get_status_bar();

    Glib::ustring title;
    Glib::ustring pattern;
    if (!s_state.from_find_iterated) {
        s_state.latest_node_offset = -1;
        Gtk::TextIter iter_insert = curr_buffer->get_insert()->get_iter();
        Gtk::TextIter iter_bound = curr_buffer->get_selection_bound()->get_iter();
        Glib::ustring entry_predefined_text = curr_buffer->get_text(iter_insert, iter_bound);
        if (!entry_predefined_text.empty())
            s_options.search_replace_dict_find = entry_predefined_text;
        if (s_state.replace_active)
            title = for_current_node ? _("Replace in Selected Node and Subnodes") : _("Replace in All Nodes");
        else
            title = for_current_node ? _("Search in Selected Node and Subnodes") : _("Search in All Nodes");
        pattern = _dialog_search(title, s_state.replace_active, true, true);
        if (!entry_predefined_text.empty()) {
            curr_buffer->move_mark(curr_buffer->get_insert(), iter_insert);
            curr_buffer->move_mark(curr_buffer->get_selection_bound(), iter_bound);
        }
        if (!pattern.empty()) {
            s_state.curr_find_pattern = pattern;
            s_state.curr_find_where = for_current_node ? "in_sel_nod_n_sub" : "in_all_nodes";
        }
        else
            return;
    }
    else
        pattern = s_state.curr_find_pattern;

    Glib::RefPtr<Glib::Regex> re_pattern = _create_re_pattern(pattern);
    if (!re_pattern) return;

    CtTreeIter starting_tree_iter = _pCtMainWin->curr_tree_iter();
    Gtk::TreeIter node_iter;
    int current_cursor_pos = curr_buffer->property_cursor_position();
    bool forward = s_options.search_replace_dict_fw;
    if (s_state.from_find_back) {
        forward = !forward;
        s_state.from_find_back = false;
    }
    bool first_fromsel = s_options.search_replace_dict_a_ff_fa == 1;
    bool all_matches = s_options.search_replace_dict_a_ff_fa == 0;
    if (first_fromsel || for_current_node) {
        s_state.first_useful_node = false; // no one node content was parsed yet
        node_iter = _pCtMainWin->curr_tree_iter();
    }
    else {
        s_state.first_useful_node = true; // all range will be parsed so no matter
        node_iter = forward ? _pCtMainWin->get_tree_store().get_iter_first() : _pCtMainWin->get_tree_store().get_tree_iter_last_sibling(_pCtMainWin->get_tree_store().get_store()->children());
    }
    s_state.matches_num = 0;
    if (all_matches) {
        s_state.match_store->clear();
        s_state.match_store->saved_path.clear();
    }

    std::string tree_expanded_collapsed_string = _pCtMainWin->get_tree_store().treeview_get_tree_expanded_collapsed_string(_pCtMainWin->get_tree_view());
    // searching start
    bool user_active_restore = _pCtMainWin->user_active();
    _pCtMainWin->user_active() = false;
    s_state.processed_nodes = 0;
    s_state.latest_matches = 0;
    s_state.counted_nodes = for_current_node ? _count_nodes(_pCtMainWin->curr_tree_iter()->children()) : (_count_nodes(_pCtMainWin->get_tree_store().get_store()->children()) - 1);
    if (all_matches) {
        ctStatusBar.progressBar.set_text("0");
        ctStatusBar.progressBar.show();
        ctStatusBar.stopButton.show();
        ctStatusBar.set_progress_stop(false);
        while (gtk_events_pending()) gtk_main_iteration();
    }
    std::time_t search_start_time = std::time(nullptr);
    while (node_iter) {
        s_state.all_matches_first_in_node = true;
        CtTreeIter ct_node_iter = _pCtMainWin->get_tree_store().to_ct_tree_iter(node_iter);
        while (_parse_given_node_content(ct_node_iter, re_pattern, forward, first_fromsel, all_matches)) {
            s_state.matches_num += 1;
            if (!all_matches ||  ctStatusBar.is_progress_stop()) break;
        }
        s_state.processed_nodes += 1;
        if (s_state.matches_num == 1 && !all_matches) break;
        if (for_current_node && !s_state.from_find_iterated) break;
        Gtk::TreeIter last_top_node_iter = node_iter; // we need this if we start from a node that is not in top level
        if (forward) node_iter = ++node_iter;
        else         node_iter = --node_iter;
        if (!node_iter || for_current_node) break;
        // code that, in case we start from a node that is not top level, climbs towards the top
        while (!node_iter) {
            node_iter = last_top_node_iter->parent();
            if (node_iter) {
                last_top_node_iter = node_iter;
                // we do not check the parent on purpose, only the uncles in the proper direction
                if (forward) node_iter = ++node_iter;
                else         node_iter = --node_iter;
            }
            else break;
        }
        if (ctStatusBar.is_progress_stop()) break;
        if (all_matches)
            _update_all_matches_progress();
    }
    std::time_t search_end_time = std::time(nullptr);
    spdlog::debug("Search took {} sec", search_end_time - search_start_time);

    _pCtMainWin->user_active() = user_active_restore;
    _pCtMainWin->get_tree_store().treeview_set_tree_expanded_collapsed_string(tree_expanded_collapsed_string, _pCtMainWin->get_tree_view(), _pCtMainWin->get_ct_config()->nodesBookmExp);
    if (!s_state.matches_num || all_matches) {
        _pCtMainWin->get_tree_view().set_cursor_safe(starting_tree_iter);
        _pCtMainWin->get_text_view().grab_focus();
        curr_buffer->place_cursor(curr_buffer->get_iter_at_offset(current_cursor_pos));
        _pCtMainWin->get_text_view().scroll_to(curr_buffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
    }
    if (!s_state.matches_num)
        CtDialogs::info_dialog(str::format(_("The pattern '%s' was not found"), std::string(pattern)), *_pCtMainWin);
    else {
        if (all_matches) {
            s_state.match_dialog_title = std::to_string(s_state.matches_num) + CtConst::CHAR_SPACE + _("Matches");
            CtDialogs::match_dialog(s_state.match_dialog_title, _pCtMainWin, s_state.match_store);
        } else {
            _pCtMainWin->get_tree_view().set_cursor_safe(_pCtMainWin->curr_tree_iter());
            if (s_options.search_replace_dict_idialog)
                _iterated_find_dialog();
        }
     }
    if (all_matches) {
        ctStatusBar.progressBar.hide();
        ctStatusBar.stopButton.hide();
        ctStatusBar.set_progress_stop(false);
    }
}

void CtActions::find_a_node()
{
    if (!_is_tree_not_empty_or_error()) return;
    Glib::ustring pattern;
    if (!s_state.from_find_iterated) {
        std::string title = s_state.replace_active ? _("Replace in Node Names...") : _("Search For a Node Name...");
        pattern = _dialog_search(title, s_state.replace_active, true, false);
        if (pattern.empty()) return;
        s_state.curr_find_where = "a_node";
        s_state.curr_find_pattern = pattern;
    }
    else
        pattern = s_state.curr_find_pattern;

    Glib::RefPtr<Glib::Regex> re_pattern = _create_re_pattern(pattern);
    if (!re_pattern) return;

    bool forward = s_options.search_replace_dict_fw;
    if (s_state.from_find_back) {
        forward = !forward;
        s_state.from_find_back = false;
    }
    bool first_fromsel = s_options.search_replace_dict_a_ff_fa == 1;
    bool all_matches = s_options.search_replace_dict_a_ff_fa == 0;
    Gtk::TreeIter node_iter;
    if (first_fromsel) {
        node_iter = forward ? ++_pCtMainWin->curr_tree_iter() : --_pCtMainWin->curr_tree_iter();
        Gtk::TreeIter top_node_iter = _pCtMainWin->curr_tree_iter();
        while (!node_iter) {
            node_iter = top_node_iter;
            node_iter = forward ? ++node_iter : --node_iter;
            top_node_iter = top_node_iter->parent();
            if (!top_node_iter) break;
        }
    }
    else
        node_iter = forward ? _pCtMainWin->get_tree_store().get_iter_first() : _pCtMainWin->get_tree_store().get_tree_iter_last_sibling(_pCtMainWin->get_tree_store().get_store()->children());;

    s_state.matches_num = 0;
    if (all_matches) {
        s_state.match_store->clear();
        s_state.match_store->saved_path.clear();
    }
    // searching start
    while (node_iter) {
        if (_parse_node_name(_pCtMainWin->get_tree_store().to_ct_tree_iter(node_iter), re_pattern, forward, all_matches)) {
            s_state.matches_num += 1;
            if (!all_matches) break;
        }
        Gtk::TreeIter last_top_node_iter = node_iter; //  we need this if we start from a node that is not in top level
        node_iter = forward ? ++node_iter : --node_iter;
        //  code that, in case we start from a node that is not top level, climbs towards the top
        while (!node_iter) {
            node_iter = last_top_node_iter->parent();
            if (node_iter) {
                last_top_node_iter = node_iter;
                // we do not check the parent on purpose, only the uncles in the proper direction
                node_iter = forward ? ++node_iter : --node_iter;
            }
            else
                break;
        }
    }
    if (s_state.matches_num == 0)
        CtDialogs::info_dialog(str::format(_("The pattern '%s' was not found"), pattern.c_str()), *_pCtMainWin);
    else if (all_matches) {
        s_state.match_dialog_title = std::to_string(s_state.matches_num) + CtConst::CHAR_SPACE + _("Matches");
        CtDialogs::match_dialog(s_state.match_dialog_title, _pCtMainWin, s_state.match_store);
    }
    else if (s_options.search_replace_dict_idialog)
        _iterated_find_dialog();
    if (s_state.matches_num && s_state.replace_active)
        _pCtMainWin->update_window_save_needed();
}

// Continue the previous search (a_node/in_selected_node/in_all_nodes)
void CtActions::find_again()
{
    s_options.search_replace_dict_idialog = false;
    s_state.from_find_iterated = true;
    if (s_state.curr_find_where.empty()) CtDialogs::warning_dialog(_("No Previous Search Was Performed During This Session"), *_pCtMainWin);
    else if (s_state.curr_find_where == "in_selected_node") find_in_selected_node();
    else if (s_state.curr_find_where == "in_all_nodes")     _find_in_all_nodes(false);
    else if (s_state.curr_find_where == "in_sel_nod_n_sub") _find_in_all_nodes(true);
    else if (s_state.curr_find_where == "a_node")           find_a_node();
    s_state.from_find_iterated = false;
}

// Continue the previous search (a_node/in_selected_node/in_all_nodes) but in Opposite Direction
void CtActions::find_back()
{
    s_options.search_replace_dict_idialog = false;
    s_state.from_find_back = true;
    s_state.replace_active = false;
    find_again();
}

// Replace a pattern in the selected Node
void CtActions::replace_in_selected_node()
{
    if (!_is_there_selected_node_or_error()) return;
    s_state.replace_active = true;
    find_in_selected_node();
    s_state.replace_active = false;
}

// Replace the pattern in all the Tree Nodes
void CtActions::replace_in_all_nodes()
{
    if (!_is_tree_not_empty_or_error()) return;
    s_state.replace_active = true;
    _find_in_all_nodes(false);
    s_state.replace_active = false;
}

// Replace the pattern Selected Node and SubNodes
void CtActions::replace_in_sel_node_and_subnodes()
{
    if (!_is_tree_not_empty_or_error()) return;
    s_state.replace_active = true;
    _find_in_all_nodes(true);
    s_state.replace_active = false;
}

// Search for a pattern between all the Node's Names
void CtActions::replace_in_nodes_names()
{
    if (!_is_tree_not_empty_or_error()) return;
    s_state.replace_active = true;
    find_a_node();
    s_state.replace_active = false;
}

// Continue the previous replace (a_node/in_selected_node/in_all_nodes)
void CtActions::replace_again()
{
    s_state.replace_active = true;
    s_state.replace_subsequent = true;
    find_again();
    s_state.replace_active = false;
    s_state.replace_subsequent = false;
}

// Restore AllMatchesDialog
void CtActions::find_allmatchesdialog_restore()
{
    CtDialogs::match_dialog(s_state.match_dialog_title, _pCtMainWin, s_state.match_store);
}

// Opens the Search Dialog
std::string CtActions::_dialog_search(const std::string& title, bool replace_on, bool multiple_nodes, bool pattern_required)
{
    Gtk::Dialog dialog(title, *_pCtMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(*_pCtMainWin);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, -1);

    auto search_entry = Gtk::Entry();
    search_entry.set_text(s_options.search_replace_dict_find);

    auto button_ok = dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT);
    if (pattern_required) {
        button_ok->set_sensitive(s_options.search_replace_dict_find.length() != 0);
        search_entry.signal_changed().connect([&button_ok, &search_entry](){
            button_ok->set_sensitive(search_entry.get_text().length() != 0);
        });
    }
    auto search_frame = Gtk::Frame(std::string("<b>")+_("Search for")+"</b>");
    dynamic_cast<Gtk::Label*>(search_frame.get_label_widget())->set_use_markup(true);
    search_frame.set_shadow_type(Gtk::SHADOW_NONE);
    search_frame.add(search_entry);

    Gtk::Frame* replace_frame = nullptr;
    Gtk::Entry* replace_entry = nullptr;
    if (replace_on) {
        replace_entry = Gtk::manage(new Gtk::Entry());
        replace_entry->set_text(s_options.search_replace_dict_replace);
        replace_frame = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Replace with")+"</b>"));
        dynamic_cast<Gtk::Label*>(replace_frame->get_label_widget())->set_use_markup(true);
        replace_frame->set_shadow_type(Gtk::SHADOW_NONE);
        replace_frame->add(*replace_entry);
    }
    auto opt_vbox = Gtk::VBox();
    opt_vbox.set_spacing(1);
    auto four_1_hbox = Gtk::HBox();
    four_1_hbox.set_homogeneous(true);
    auto four_2_hbox = Gtk::HBox();
    four_2_hbox.set_homogeneous(true);
    auto bw_fw_hbox = Gtk::HBox();
    bw_fw_hbox.set_homogeneous(true);
    auto three_hbox = Gtk::HBox();
    three_hbox.set_homogeneous(true);
    auto three_vbox = Gtk::VBox();
    auto match_case_checkbutton = Gtk::CheckButton(_("Match Case"));
    match_case_checkbutton.set_active(s_options.search_replace_dict_match_case);
    auto reg_exp_checkbutton = Gtk::CheckButton(_("Regular Expression"));
    reg_exp_checkbutton.set_active(s_options.search_replace_dict_reg_exp);
    auto whole_word_checkbutton = Gtk::CheckButton(_("Whole Word"));
    whole_word_checkbutton.set_active(s_options.search_replace_dict_whole_word);
    auto start_word_checkbutton = Gtk::CheckButton(_("Start Word"));
    start_word_checkbutton.set_active(s_options.search_replace_dict_start_word);
    auto fw_radiobutton = Gtk::RadioButton(_("Forward"));
    fw_radiobutton.set_active(s_options.search_replace_dict_fw);
    auto bw_radiobutton = Gtk::RadioButton(_("Backward"));
    bw_radiobutton.join_group(fw_radiobutton);
    bw_radiobutton.set_active(!s_options.search_replace_dict_fw);
    auto all_radiobutton = Gtk::RadioButton(_("All, List Matches"));
    all_radiobutton.set_active(s_options.search_replace_dict_a_ff_fa == 0);
    auto first_from_radiobutton = Gtk::RadioButton(_("First From Selection"));
    first_from_radiobutton.join_group(all_radiobutton);
    first_from_radiobutton.set_active(s_options.search_replace_dict_a_ff_fa == 1);
    auto first_all_radiobutton = Gtk::RadioButton(_("First in All Range"));
    first_all_radiobutton.join_group(all_radiobutton);
    first_all_radiobutton.set_active(s_options.search_replace_dict_a_ff_fa == 2);

    Gtk::Frame* ts_frame = nullptr;
    Gtk::CheckButton* ts_node_created_after_checkbutton = nullptr;
    Gtk::CheckButton* ts_node_created_before_checkbutton = nullptr;
    Gtk::CheckButton* ts_node_modified_after_checkbutton = nullptr;
    Gtk::CheckButton* ts_node_modified_before_checkbutton = nullptr;
    if (multiple_nodes) {
        std::string ts_format = "%A, %d %B %Y, %H:%M";
        ts_node_created_after_checkbutton = Gtk::manage(new Gtk::CheckButton(_("Node Created After")));
        Glib::ustring ts_label = str::time_format(ts_format, s_options.ts_cre_after.time);
        auto ts_node_created_after_button = Gtk::manage(new Gtk::Button(ts_label));
        auto ts_node_created_after_hbox = Gtk::manage(new Gtk::HBox());
        ts_node_created_after_hbox->set_homogeneous(true);
        ts_node_created_after_hbox->pack_start(*ts_node_created_after_checkbutton);
        ts_node_created_after_hbox->pack_start(*ts_node_created_after_button);
        ts_node_created_before_checkbutton = Gtk::manage(new Gtk::CheckButton(_("Node Created Before")));
        ts_label = str::time_format(ts_format, s_options.ts_cre_before.time);
        auto ts_node_created_before_button = Gtk::manage(new Gtk::Button(ts_label));
        auto ts_node_created_before_hbox = Gtk::manage(new Gtk::HBox());
        ts_node_created_before_hbox->set_homogeneous(true);
        ts_node_created_before_hbox->pack_start(*ts_node_created_before_checkbutton);
        ts_node_created_before_hbox->pack_start(*ts_node_created_before_button);
        ts_node_modified_after_checkbutton = Gtk::manage(new Gtk::CheckButton(_("Node Modified After")));
        ts_label = str::time_format(ts_format, s_options.ts_mod_after.time);
        auto ts_node_modified_after_button = Gtk::manage(new Gtk::Button(ts_label));
        auto ts_node_modified_after_hbox = Gtk::manage(new Gtk::HBox());
        ts_node_modified_after_hbox->set_homogeneous(true);
        ts_node_modified_after_hbox->pack_start(*ts_node_modified_after_checkbutton);
        ts_node_modified_after_hbox->pack_start(*ts_node_modified_after_button);
        ts_node_modified_before_checkbutton = Gtk::manage(new Gtk::CheckButton(_("Node Modified Before")));
        ts_label = str::time_format(ts_format, s_options.ts_mod_before.time);
        auto ts_node_modified_before_button = Gtk::manage(new Gtk::Button(ts_label));
        auto ts_node_modified_before_hbox = Gtk::manage(new Gtk::HBox());
        ts_node_modified_before_hbox->set_homogeneous(true);
        ts_node_modified_before_hbox->pack_start(*ts_node_modified_before_checkbutton);
        ts_node_modified_before_hbox->pack_start(*ts_node_modified_before_button);
        ts_node_created_after_checkbutton->set_active(s_options.ts_cre_after.on);
        ts_node_created_before_checkbutton->set_active(s_options.ts_cre_before.on);
        ts_node_modified_after_checkbutton->set_active(s_options.ts_mod_after.on);
        ts_node_modified_before_checkbutton->set_active(s_options.ts_mod_before.on);
        auto ts_node_vbox = Gtk::manage(new Gtk::VBox());
        ts_node_vbox->pack_start(*ts_node_created_after_hbox);
        ts_node_vbox->pack_start(*ts_node_created_before_hbox);
        ts_node_vbox->pack_start(*Gtk::manage(new Gtk::HSeparator()));
        ts_node_vbox->pack_start(*ts_node_modified_after_hbox);
        ts_node_vbox->pack_start(*ts_node_modified_before_hbox);

        ts_frame = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Time filter")+"</b>"));
        dynamic_cast<Gtk::Label*>(ts_frame->get_label_widget())->set_use_markup(true);
        ts_frame->set_shadow_type(Gtk::SHADOW_NONE);
        ts_frame->add(*ts_node_vbox);

        auto on_ts_node_button_clicked = [&dialog, ts_format](Gtk::Button* button, const char* title, std::time_t& ts_value) {
            std::time_t new_time = CtDialogs::date_select_dialog(dialog, title, ts_value);
            if (new_time == 0) return;
             ts_value = new_time;
             button->set_label(str::time_format(ts_format, new_time));
        };
        ts_node_created_after_button->signal_clicked().connect(sigc::bind(on_ts_node_button_clicked, ts_node_created_after_button,
                                                                          _("Node Created After"), s_options.ts_cre_after.time));
        ts_node_created_before_button->signal_clicked().connect(sigc::bind(on_ts_node_button_clicked, ts_node_created_before_button,
                                                                           _("Node Created Before"), s_options.ts_cre_before.time));
        ts_node_modified_after_button->signal_clicked().connect(sigc::bind(on_ts_node_button_clicked, ts_node_modified_after_button,
                                                                           _("Node Modified After"), s_options.ts_mod_after.time));
        ts_node_modified_before_button->signal_clicked().connect(sigc::bind(on_ts_node_button_clicked, ts_node_modified_before_button,
                                                                            _("Node Modified Before"), s_options.ts_mod_before.time));
    }
    auto iter_dialog_checkbutton = Gtk::CheckButton(_("Show Iterated Find/Replace Dialog"));
    iter_dialog_checkbutton.set_active(s_options.search_replace_dict_idialog);
    four_1_hbox.pack_start(match_case_checkbutton);
    four_1_hbox.pack_start(reg_exp_checkbutton);
    four_2_hbox.pack_start(whole_word_checkbutton);
    four_2_hbox.pack_start(start_word_checkbutton);
    bw_fw_hbox.pack_start(fw_radiobutton);
    bw_fw_hbox.pack_start(bw_radiobutton);
    three_hbox.pack_start(all_radiobutton);
    three_vbox.pack_start(first_from_radiobutton);
    three_vbox.pack_start(first_all_radiobutton);
    three_hbox.pack_start(three_vbox);
    opt_vbox.pack_start(four_1_hbox);
    opt_vbox.pack_start(four_2_hbox);
    opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator()));
    opt_vbox.pack_start(bw_fw_hbox);
    opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator()));
    opt_vbox.pack_start(three_hbox);
    opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator()));
    if (multiple_nodes) {
        opt_vbox.pack_start(*ts_frame);
        opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator()));
    }
    opt_vbox.pack_start(iter_dialog_checkbutton);
    auto opt_frame = Gtk::Frame(std::string("<b>")+_("Search options")+"</b>");
    dynamic_cast<Gtk::Label*>(opt_frame.get_label_widget())->set_use_markup(true);
    opt_frame.set_shadow_type(Gtk::SHADOW_NONE);
    opt_frame.add(opt_vbox);
    auto content_area = dialog.get_content_area();
    content_area->set_spacing(5);
    content_area->pack_start(search_frame);
    if (replace_on) content_area->pack_start(*replace_frame);
    content_area->pack_start(opt_frame);
    content_area->show_all();
    search_entry.grab_focus();

    auto press_enter = [&dialog, &button_ok](GdkEventKey* key){
        if (key->keyval == GDK_KEY_Return)
            if (button_ok && button_ok->get_sensitive()) {
                dialog.response(Gtk::RESPONSE_ACCEPT);
                return true;
            }
        return false;
    };
    dialog.signal_key_press_event().connect(press_enter);
    search_entry.signal_key_press_event().connect(press_enter, false);

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return "";

    s_options.search_replace_dict_find = search_entry.get_text();
    if (replace_on)
        s_options.search_replace_dict_replace = replace_entry->get_text();
    s_options.search_replace_dict_match_case = match_case_checkbutton.get_active();
    s_options.search_replace_dict_reg_exp = reg_exp_checkbutton.get_active();
    s_options.search_replace_dict_whole_word = whole_word_checkbutton.get_active();
    s_options.search_replace_dict_start_word = start_word_checkbutton.get_active();
    s_options.search_replace_dict_fw = fw_radiobutton.get_active();
    if (all_radiobutton.get_active())              s_options.search_replace_dict_a_ff_fa = 0;
    else if (first_from_radiobutton.get_active())  s_options.search_replace_dict_a_ff_fa = 1;
    else                                           s_options.search_replace_dict_a_ff_fa = 2;
    s_options.ts_cre_after.on = multiple_nodes ? ts_node_created_after_checkbutton->get_active() : false;
    s_options.ts_cre_before.on = multiple_nodes ? ts_node_created_before_checkbutton->get_active() : false;
    s_options.ts_mod_after.on = multiple_nodes ? ts_node_modified_after_checkbutton->get_active() : false;
    s_options.ts_mod_before.on = multiple_nodes ? ts_node_modified_before_checkbutton->get_active() : false;
    s_options.search_replace_dict_idialog = iter_dialog_checkbutton.get_active();
    return s_options.search_replace_dict_find;
}


// Recursive function that searchs for the given pattern
bool CtActions::_parse_node_name(CtTreeIter node_iter, Glib::RefPtr<Glib::Regex> re_pattern, bool forward, bool all_matches)
{
    Glib::MatchInfo match;
    if (_is_node_within_time_filter(node_iter)) {
        Glib::ustring text_name = node_iter.get_node_name();
        if (!re_pattern->match(text_name, match)) {
            Glib::ustring text_tags = node_iter.get_node_tags();
            re_pattern->match(text_tags, match);
        }
    }
    if (match.matches()) {
        if (all_matches) {
            gint64 node_id = node_iter.get_node_id();
            Glib::ustring node_name = node_iter.get_node_name();
            Glib::ustring node_hier_name = CtMiscUtil::get_node_hierarchical_name(node_iter, " << ", false, false);
            Glib::ustring line_content = _get_first_line_content(node_iter.get_node_text_buffer());
            s_state.match_store->add_row(node_id, node_name, str::xml_escape(node_hier_name), 0, 0, 1, line_content);
        }
        if (s_state.replace_active && !node_iter.get_node_read_only()) {
            std::string replacer_text = s_options.search_replace_dict_replace;
            Glib::ustring text_name = node_iter.get_node_name();
            //str::replace(text_name, s_state.curr_find_pattern.c_str(), replacer_text.c_str());
            text_name = re_pattern->replace(text_name, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));
            node_iter.set_node_name(text_name);
            node_iter.pending_edit_db_node_prop();
        }
        if (!all_matches) {
            _pCtMainWin->get_tree_view().set_cursor_safe(node_iter);
            _pCtMainWin->get_text_view().grab_focus();
            return true;
        }
        else
            s_state.matches_num += 1;
    }

    // check for children
    if (!node_iter->children().empty()) {
        Gtk::TreeIter child_iter = forward ? node_iter->children().begin() : --node_iter->children().end();
        while (child_iter) {
            if (_parse_node_name(_pCtMainWin->get_tree_store().to_ct_tree_iter(child_iter), re_pattern, forward, all_matches)
                    && !all_matches)
                return true;
            child_iter = forward ? ++child_iter : --child_iter;
        }
    }
    return false;
}

// Returns True if pattern was found, False otherwise
bool CtActions::_parse_given_node_content(CtTreeIter node_iter, Glib::RefPtr<Glib::Regex> re_pattern, bool forward, bool first_fromsel, bool all_matches)
{
    auto text_buffer = node_iter.get_node_text_buffer();
    if (!s_state.first_useful_node) {
        // first_fromsel plus first_node not already parsed
        if (!_pCtMainWin->curr_tree_iter() || node_iter.get_node_id() == _pCtMainWin->curr_tree_iter().get_node_id()) {
            s_state.first_useful_node = true; // a first_node was parsed
            if (_parse_node_content_iter(node_iter, text_buffer, re_pattern, forward, first_fromsel, all_matches, true))
                return true; // first_node node, first_fromsel
        }
    } else {
        // not first_fromsel or first_fromsel with first_node already parsed
        if (_parse_node_content_iter(node_iter, text_buffer, re_pattern, forward, first_fromsel, all_matches, false))
            return true; // not first_node node
    }
    // check for children
    if (!node_iter->children().empty()) {
        Gtk::TreeIter child_iter = forward ? node_iter->children().begin() : --node_iter->children().end();
        while (child_iter && !_pCtMainWin->get_status_bar().is_progress_stop()) {
            s_state.all_matches_first_in_node = true;
            while (_parse_given_node_content(_pCtMainWin->get_tree_store().to_ct_tree_iter(child_iter), re_pattern, forward, first_fromsel, all_matches)) {
                s_state.matches_num += 1;
                if (!all_matches || _pCtMainWin->get_status_bar().is_progress_stop()) break;
            }
            if (s_state.matches_num == 1 && !all_matches) break;
            if (forward) child_iter = ++child_iter;
            else         child_iter = --child_iter;
            s_state.processed_nodes += 1;
            if (all_matches)
                _update_all_matches_progress();
        }
    }
    return false;
}

// Returns True if pattern was find, False otherwise
bool CtActions::_parse_node_content_iter(const CtTreeIter& tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, Glib::RefPtr<Glib::Regex> re_pattern,
                             bool forward, bool first_fromsel, bool all_matches, bool first_node)
{
    bool restore_modified;
    Gtk::TextIter start_iter;
    bool pattern_found;

    Gtk::TextIter buff_start_iter = text_buffer->begin();
    if (buff_start_iter.get_char() != g_utf8_get_char(CtConst::CHAR_NEWLINE)) {
        s_state.newline_trick = true;
        restore_modified = !text_buffer->get_modified();
        text_buffer->insert(buff_start_iter, CtConst::CHAR_NEWLINE);
    } else {
        s_state.newline_trick = false;
        restore_modified = false;
    }
    if ((first_fromsel && first_node) || (all_matches && !s_state.all_matches_first_in_node)) {
        gint64 node_id = tree_iter.get_node_id();
        start_iter = _get_inner_start_iter(text_buffer, forward, node_id);
    } else {
        start_iter = forward ? text_buffer->begin() : text_buffer->end();
        if (all_matches) s_state.all_matches_first_in_node = false;
    }

    pattern_found = false;
    if (_is_node_within_time_filter(tree_iter))
        pattern_found = _find_pattern(tree_iter, text_buffer, re_pattern, start_iter, forward, all_matches);

    if (s_state.newline_trick) {
        buff_start_iter = text_buffer->begin();
        Gtk::TextIter buff_step_iter = buff_start_iter;
        if (buff_step_iter.forward_char()) text_buffer->erase(buff_start_iter, buff_step_iter);
        if (restore_modified) text_buffer->set_modified(false);
    }
    if (s_state.replace_active && pattern_found)
        _pCtMainWin->update_window_save_needed(CtSaveNeededUpdType::nbuf, false/*new_machine_state*/, &tree_iter);
    return pattern_found;
}

// Get start_iter when not at beginning or end
Gtk::TextIter CtActions::_get_inner_start_iter(Glib::RefPtr<Gtk::TextBuffer> text_buffer, bool forward, const gint64& node_id)
{
    Gtk::TextIter start_iter, min_iter, max_iter;
    if (text_buffer->get_has_selection()) {
        text_buffer->get_selection_bounds(min_iter, max_iter);
    } else {
        min_iter = text_buffer->get_iter_at_mark(text_buffer->get_insert());
        max_iter = min_iter;
    }
    if (!s_state.replace_active || s_state.replace_subsequent) {
        // it's a find or subsequent replace, so we want, given a selected word, to find for the subsequent one
        if (forward)    start_iter = max_iter;
        else            start_iter = min_iter;
    } else {
        // it's a first replace, so we want, given a selected word, to replace starting from this one
        if (forward)    start_iter = min_iter;
        else            start_iter = max_iter;
    }
    if (s_state.latest_node_offset != -1
            && s_state.latest_node_offset_node_id == node_id
            && s_state.latest_node_offset == start_iter.get_offset())
    {
        if (forward) start_iter.forward_char();
        else         start_iter.backward_char();
    }
    s_state.latest_node_offset_node_id = node_id;
    s_state.latest_node_offset = start_iter.get_offset();
    //print self.latest_node_offset["n"], offsets, self.latest_node_offset["o"]
    return start_iter;
}

//"""Returns True if the given node_iter is within the Time Filter"""
bool CtActions::_is_node_within_time_filter(const CtTreeIter& node_iter)
{
    gint64 ts_cre = node_iter.get_node_creating_time();
    if (s_options.ts_cre_after.on && ts_cre < s_options.ts_cre_after.time)
        return false;
    if (s_options.ts_cre_before.on && ts_cre > s_options.ts_cre_before.time)
        return false;
    gint64 ts_mod = node_iter.get_node_modification_time();
    if (s_options.ts_mod_after.on && ts_mod < s_options.ts_mod_after.time)
        return false;
    if (s_options.ts_mod_before.on && ts_mod > s_options.ts_mod_before.time)
        return false;
    return true;
}

Glib::RefPtr<Glib::Regex> CtActions::_create_re_pattern(Glib::ustring pattern)
{
    if (!s_options.search_replace_dict_reg_exp) // NOT REGULAR EXPRESSION
    {
        pattern = Glib::Regex::escape_string(pattern);     // backslashes all non alphanum chars => to not spoil re
        if (s_options.search_replace_dict_whole_word)      // WHOLE WORD
            pattern = "\\b" + pattern + "\\b";
        else if (s_options.search_replace_dict_start_word) // START WORD
            pattern = "\\b" + pattern;
    }
    try
    {
        if (s_options.search_replace_dict_match_case) // CASE SENSITIVE
            return Glib::Regex::create(pattern, Glib::RegexCompileFlags::REGEX_MULTILINE);
        else
            return Glib::Regex::create(pattern, Glib::RegexCompileFlags::REGEX_MULTILINE | Glib::RegexCompileFlags::REGEX_CASELESS);
    }
    catch (Glib::RegexError& e)
    {
        CtDialogs::error_dialog(e.what(), *_pCtMainWin);
        return Glib::RefPtr<Glib::Regex>();
    }
}

// """Returns (start_iter, end_iter) or (None, None)"""
bool CtActions::_find_pattern(CtTreeIter tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, Glib::RefPtr<Glib::Regex> re_pattern,
                  Gtk::TextIter start_iter, bool forward, bool all_matches)
{
    /* Gtk::TextBuffer uses symbols positions
     * Glib::Regex uses byte positions
     */

    Glib::ustring text = text_buffer->get_text();

    int start_offset = start_iter.get_offset();
    // # start_offset -= self.get_num_objs_before_offset(text_buffer, start_offset)
    std::pair<int, int> match_offsets = {-1, -1};
    if (forward) {
        Glib::MatchInfo match;
        if (re_pattern->match(text, str::symb_pos_to_byte_pos(text, start_offset), match))
            if (match.matches())
                match.fetch_pos(0, match_offsets.first, match_offsets.second);
    } else {
        Glib::MatchInfo match;
        re_pattern->match(text, str::symb_pos_to_byte_pos(text, start_offset) /*as len*/, 0 /*as start position*/, match);
        while (match.matches()) {
            match.fetch_pos(0, match_offsets.first, match_offsets.second);
            match.next();
        }
    }
    if (match_offsets.first != -1) {
        match_offsets.first = str::byte_pos_to_symb_pos(text, match_offsets.first);
        match_offsets.second = str::byte_pos_to_symb_pos(text, match_offsets.second);
    }

    std::pair<int,int> obj_match_offsets = {-1, -1};
    std::string obj_content;
    if (!s_state.replace_active) {
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
    // #print "IN", final_start_offset, final_delta_offset, self.dad.treestore[tree_iter][1]
    // #for count in range(final_delta_offset):
    // #    print count, text_buffer.get_iter_at_offset(final_start_offset+count).get_char()
    if (!_pCtMainWin->curr_tree_iter() || _pCtMainWin->curr_tree_iter().get_node_id() != tree_iter.get_node_id())
        _pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
    _pCtMainWin->get_text_view().set_selection_at_offset_n_delta(final_start_offset, final_delta_offset);
    // #print "OUT"
    auto mark_insert = text_buffer->get_insert();
    Gtk::TextIter iter_insert = text_buffer->get_iter_at_mark(mark_insert);
    if (all_matches) {
        int newline_trick_offset = s_state.newline_trick ? 1 : 0;
        gint64 node_id = tree_iter.get_node_id();
        int start_offset = match_offsets.first + num_objs - newline_trick_offset;
        int end_offset = match_offsets.second + num_objs - newline_trick_offset;
        std::string node_name = tree_iter.get_node_name();
        std::string node_hier_name = CtMiscUtil::get_node_hierarchical_name(tree_iter, " << ", false, false);
        std::string line_content = obj_match_offsets.first != -1 ? obj_content : _get_line_content(text_buffer, iter_insert);
        int line_num = text_buffer->get_iter_at_offset(start_offset).get_line();
        if (!s_state.newline_trick) line_num += 1;
        s_state.match_store->add_row(node_id, node_name, str::xml_escape(node_hier_name), start_offset, end_offset, line_num, line_content);
        // #print line_num, self.matches_num
    } else {
        _pCtMainWin->get_text_view().scroll_to(mark_insert, CtTextView::TEXT_SCROLL_MARGIN);
    }
    if (s_state.replace_active) {
            if (_pCtMainWin->curr_tree_iter().get_node_read_only()) return false;
            Gtk::TextIter sel_start, sel_end;
            text_buffer->get_selection_bounds(sel_start, sel_end);

            Glib::ustring origin_text = sel_start.get_text(sel_end);
            Glib::ustring replacer_text = s_options.search_replace_dict_replace; /* should be Glib::ustring to count symbols */

            // use re_pattern->replace for the cases with \n, maybe it even helps with groups
            if (s_options.search_replace_dict_reg_exp)
                replacer_text = re_pattern->replace(origin_text, 0, replacer_text, static_cast<Glib::RegexMatchFlags>(0));

            text_buffer->erase(sel_start, sel_end);
            text_buffer->insert_at_cursor(replacer_text);
            if (!all_matches)
                _pCtMainWin->get_text_view().set_selection_at_offset_n_delta(match_offsets.first + num_objs, (int)replacer_text.size());
            _pCtMainWin->get_state_machine().update_state();
            tree_iter.pending_edit_db_node_buff();
        }
    return true;
}

// Search for the pattern in the given object
Glib::ustring CtActions::_check_pattern_in_object(Glib::RefPtr<Glib::Regex> pattern, CtAnchoredWidget* obj)
{
    if (CtImageEmbFile* image = dynamic_cast<CtImageEmbFile*>(obj))
    {
        if (pattern->match(image->get_file_name().string())) return image->get_file_name().string();
    }
    else if (CtImageAnchor* image = dynamic_cast<CtImageAnchor*>(obj))
    {
        if (pattern->match(image->get_anchor_name())) return image->get_anchor_name();
    }
    else if (CtTable* table = dynamic_cast<CtTable*>(obj))
    {
        for (auto& row: table->get_table_matrix())
            for (auto& col: row)
                if (pattern->match(col->get_text_content()))
                    return "<table>";
    }
    else if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(obj))
    {
        if (pattern->match(codebox->get_text_content())) return "<codebox>";
    }
    return "";
}

// Search for the pattern in the given slice and direction
std::pair<int, int> CtActions::_check_pattern_in_object_between(CtTreeIter tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, Glib::RefPtr<Glib::Regex> pattern,
                                                                int start_offset, int end_offset, bool forward, std::string& obj_content)
{
    if (!forward) start_offset -= 1;
    if (end_offset < 0) {
        if (forward) {
            Gtk::TextIter start, end;
            text_buffer->get_bounds(start, end);
            end_offset = end.get_offset();
        } else
            end_offset = 0;
    }
    if (!forward) std::swap(start_offset, end_offset);

    std::list<CtAnchoredWidget*> obj_vec = tree_iter.get_embedded_pixbufs_tables_codeboxes(start_offset, end_offset);
    if (!forward)
        std::reverse(obj_vec.begin(), obj_vec.end());
    for (auto element: obj_vec)
    {
        obj_content = _check_pattern_in_object(pattern, element);
        if (!obj_content.empty())
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
        if (!curr_iter.forward_char())
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
    if (!line_start.backward_char()) return "";
    while (line_start.get_char() != g_utf8_get_char(CtConst::CHAR_NEWLINE))
        if (!line_start.backward_char())
            break;
    if (line_start.get_char() == g_utf8_get_char(CtConst::CHAR_NEWLINE))
        line_start.forward_char();
    while (line_end.get_char() != g_utf8_get_char(CtConst::CHAR_NEWLINE))
        if (!line_end.forward_char())
            break;
    return text_buffer->get_text(line_start, line_end);
}

// Returns the First Not Empty Line Content Given the Text Buffer
std::string CtActions::_get_first_line_content(Glib::RefPtr<Gtk::TextBuffer> text_buffer)
{
    Gtk::TextIter start_iter = text_buffer->get_iter_at_offset(0);
    while (start_iter.get_char() == g_utf8_get_char(CtConst::CHAR_NEWLINE))
        if (!start_iter.forward_char())
            return "";
    Gtk::TextIter end_iter = start_iter;
    while (end_iter.get_char() != g_utf8_get_char(CtConst::CHAR_NEWLINE))
        if (!end_iter.forward_char())
            break;
    return text_buffer->get_text(start_iter, end_iter);
}

// Iterated Find/Replace Dialog
void CtActions::_iterated_find_dialog()
{
    if (!s_state.iteratedfinddialog)
    {
        auto dialog = new Gtk::Dialog(_("Iterate Latest Find/Replace"), *_pCtMainWin, Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
        auto button_close = dialog->add_button(_("Close"), 0);
        auto button_find_bw = dialog->add_button(_("Find Previous"), 4);
        auto button_find_fw = dialog->add_button(_("Find Next"), 1);
        auto button_replace = dialog->add_button(_("Replace"), 2);
        auto button_undo = dialog->add_button(_("Undo"), 3);
        dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
        button_close->set_image_from_icon_name("ct_close", Gtk::ICON_SIZE_BUTTON);
        button_find_bw->set_image_from_icon_name("ct_find_back", Gtk::ICON_SIZE_BUTTON);
        button_find_fw->set_image_from_icon_name("ct_find_again", Gtk::ICON_SIZE_BUTTON);
        button_replace->set_image_from_icon_name("ct_find_replace", Gtk::ICON_SIZE_BUTTON);
        button_undo->set_image_from_icon_name("ct_undo", Gtk::ICON_SIZE_BUTTON);

        button_find_fw->grab_focus();
        button_find_fw->grab_default();

        button_close->signal_clicked().connect([dialog](){
            dialog->hide();
        });
        button_find_fw->signal_clicked().connect([this, dialog](){
            dialog->hide();
            s_state.replace_active = false;
            find_again();
        });
        button_find_bw->signal_clicked().connect([this, dialog](){
            dialog->hide();
            s_state.replace_active = false;
            find_back();
        });
        button_replace->signal_clicked().connect([this, dialog](){
           dialog->hide();
           s_state.replace_active = true;
           s_state.replace_subsequent = true;
           find_again();
           s_state.replace_subsequent = false;
        });
        button_undo->signal_clicked().connect([this](){
           requested_step_back();
        });

        s_state.iteratedfinddialog.reset(dialog);
    }
    s_state.iteratedfinddialog->show();
}

void CtActions::_update_all_matches_progress()
{
    double frac = double(s_state.processed_nodes)/double(s_state.counted_nodes);
    _pCtMainWin->get_status_bar().progressBar.set_fraction(frac);
    if (s_state.matches_num != s_state.latest_matches) {
        s_state.latest_matches = s_state.matches_num;
        _pCtMainWin->get_status_bar().progressBar.set_text(std::to_string(s_state.matches_num));
    }
    while (gtk_events_pending()) gtk_main_iteration();
}

