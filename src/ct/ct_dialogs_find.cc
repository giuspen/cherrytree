/*
 * ct_dialogs_find.cc
 *
 * Copyright 2009-2024
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

#include "ct_dialogs.h"
#include "ct_main_win.h"
#include "ct_actions.h"

void CtDialogs::dialog_search(CtMainWin* pCtMainWin,
                              const Glib::ustring& title,
                              CtSearchOptions& s_options,
                              CtSearchState& s_state,
                              bool multiple_nodes)
{
    auto pDialog = new Gtk::Dialog{title,
                                   *pCtMainWin,
                                   Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    s_state.searchfinddialog.reset(pDialog);

    auto button_cancel = pDialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    auto button_ok = pDialog->add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    pDialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    button_cancel->set_always_show_image(true);
    button_ok->set_always_show_image(true);
    button_cancel->grab_focus();
    button_ok->grab_default();
    pDialog->set_default_size(400, -1);

    auto search_entry = Gtk::manage(new Gtk::Entry{});
    search_entry->set_text(s_options.str_find);

    button_ok->set_sensitive(s_options.str_find.length() != 0);
    search_entry->signal_changed().connect([button_ok, search_entry](){
        button_ok->set_sensitive(search_entry->get_text().length() != 0);
    });
    auto search_frame = Gtk::manage(new Gtk::Frame{std::string("<b>")+_("Search for")+"</b>"});
    dynamic_cast<Gtk::Label*>(search_frame->get_label_widget())->set_use_markup(true);
    search_frame->set_shadow_type(Gtk::SHADOW_NONE);
    search_frame->add(*search_entry);

    Gtk::Frame* replace_frame{nullptr};
    Gtk::Entry* replace_entry{nullptr};
    if (s_state.replace_active) {
        replace_entry = Gtk::manage(new Gtk::Entry{});
        replace_entry->set_text(s_options.str_replace);
        replace_frame = Gtk::manage(new Gtk::Frame{std::string("<b>")+_("Replace with")+"</b>"});
        dynamic_cast<Gtk::Label*>(replace_frame->get_label_widget())->set_use_markup(true);
        replace_frame->set_shadow_type(Gtk::SHADOW_NONE);
        replace_frame->add(*replace_entry);
    }
    auto opt_vbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 1/*spacing*/});
    auto reg_exp_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 1/*spacing*/});
    auto four_1_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    four_1_hbox->set_homogeneous(true);
    auto four_2_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    four_2_hbox->set_homogeneous(true);
    auto four_3_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    four_3_hbox->set_homogeneous(true);
    auto bw_fw_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    bw_fw_hbox->set_homogeneous(true);
    auto three_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    three_hbox->set_homogeneous(true);
    auto three_vbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto match_case_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Match Case")});
    match_case_checkbutton->set_active(s_options.match_case);
    auto reg_exp_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Regular Expression")});
    reg_exp_checkbutton->set_active(s_options.reg_exp);
    auto reg_exp_help_button = Gtk::manage(new Gtk::Button{});
    reg_exp_help_button->set_image(*pCtMainWin->new_managed_image_from_stock("ct_help", Gtk::ICON_SIZE_BUTTON));
    reg_exp_help_button->set_tooltip_text(_("Online Manual"));
    reg_exp_hbox->pack_start(*reg_exp_checkbutton);
    reg_exp_hbox->pack_start(*reg_exp_help_button);
    auto accent_insensitive_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Accent Insensitive")});
    accent_insensitive_checkbutton->set_active(s_options.accent_insensitive);
    auto override_exclusions_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Override Exclusions")});
    override_exclusions_checkbutton->set_active(s_options.override_exclusions);
    auto whole_word_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Whole Word")});
    whole_word_checkbutton->set_active(s_options.whole_word);
    auto start_word_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Start Word")});
    start_word_checkbutton->set_active(s_options.start_word);
    auto fw_radiobutton = Gtk::manage(new Gtk::RadioButton{_("Forward")});
    fw_radiobutton->set_active(s_options.direction_fw);
    auto bw_radiobutton = Gtk::manage(new Gtk::RadioButton{_("Backward")});
    bw_radiobutton->join_group(*fw_radiobutton);
    bw_radiobutton->set_active(!s_options.direction_fw);
    auto all_radiobutton = Gtk::manage(new Gtk::RadioButton{_("All, List Matches")});
    all_radiobutton->set_active(s_options.all_firstsel_firstall == 0);
    auto first_from_radiobutton = Gtk::manage(new Gtk::RadioButton{_("First From Selection")});
    first_from_radiobutton->join_group(*all_radiobutton);
    first_from_radiobutton->set_active(s_options.all_firstsel_firstall == 1);
    auto first_all_radiobutton = Gtk::manage(new Gtk::RadioButton{_("First in All Range")});
    first_all_radiobutton->join_group(*all_radiobutton);
    first_all_radiobutton->set_active(s_options.all_firstsel_firstall == 2);

    Gtk::Frame* ts_frame{nullptr};
    Gtk::CheckButton* ts_node_created_after_checkbutton{nullptr};
    Gtk::CheckButton* ts_node_created_before_checkbutton{nullptr};
    Gtk::CheckButton* ts_node_modified_after_checkbutton{nullptr};
    Gtk::CheckButton* ts_node_modified_before_checkbutton{nullptr};
    if (multiple_nodes) {
        std::string ts_format{"%A, %d %B %Y, %H:%M"};
        ts_node_created_after_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Node Created After")});
        Glib::ustring ts_label = str::time_format(ts_format, s_options.ts_cre_after.time);
        auto ts_node_created_after_button = Gtk::manage(new Gtk::Button{ts_label});
        auto ts_node_created_after_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
        ts_node_created_after_hbox->set_homogeneous(true);
        ts_node_created_after_hbox->pack_start(*ts_node_created_after_checkbutton);
        ts_node_created_after_hbox->pack_start(*ts_node_created_after_button);
        ts_node_created_before_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Node Created Before")});
        ts_label = str::time_format(ts_format, s_options.ts_cre_before.time);
        auto ts_node_created_before_button = Gtk::manage(new Gtk::Button{ts_label});
        auto ts_node_created_before_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
        ts_node_created_before_hbox->set_homogeneous(true);
        ts_node_created_before_hbox->pack_start(*ts_node_created_before_checkbutton);
        ts_node_created_before_hbox->pack_start(*ts_node_created_before_button);
        ts_node_modified_after_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Node Modified After")});
        ts_label = str::time_format(ts_format, s_options.ts_mod_after.time);
        auto ts_node_modified_after_button = Gtk::manage(new Gtk::Button{ts_label});
        auto ts_node_modified_after_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
        ts_node_modified_after_hbox->set_homogeneous(true);
        ts_node_modified_after_hbox->pack_start(*ts_node_modified_after_checkbutton);
        ts_node_modified_after_hbox->pack_start(*ts_node_modified_after_button);
        ts_node_modified_before_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Node Modified Before")});
        ts_label = str::time_format(ts_format, s_options.ts_mod_before.time);
        auto ts_node_modified_before_button = Gtk::manage(new Gtk::Button{ts_label});
        auto ts_node_modified_before_hbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
        ts_node_modified_before_hbox->set_homogeneous(true);
        ts_node_modified_before_hbox->pack_start(*ts_node_modified_before_checkbutton);
        ts_node_modified_before_hbox->pack_start(*ts_node_modified_before_button);
        ts_node_created_after_checkbutton->set_active(s_options.ts_cre_after.on);
        ts_node_created_before_checkbutton->set_active(s_options.ts_cre_before.on);
        ts_node_modified_after_checkbutton->set_active(s_options.ts_mod_after.on);
        ts_node_modified_before_checkbutton->set_active(s_options.ts_mod_before.on);
        auto ts_node_vbox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
        ts_node_vbox->pack_start(*ts_node_created_after_hbox);
        ts_node_vbox->pack_start(*ts_node_created_before_hbox);
        ts_node_vbox->pack_start(*Gtk::manage(new Gtk::HSeparator{}));
        ts_node_vbox->pack_start(*ts_node_modified_after_hbox);
        ts_node_vbox->pack_start(*ts_node_modified_before_hbox);

        ts_frame = Gtk::manage(new Gtk::Frame{std::string("<b>")+_("Time filter")+"</b>"});
        dynamic_cast<Gtk::Label*>(ts_frame->get_label_widget())->set_use_markup(true);
        ts_frame->set_shadow_type(Gtk::SHADOW_NONE);
        ts_frame->add(*ts_node_vbox);

        auto on_ts_node_button_clicked = [pDialog, ts_format](Gtk::Button* button, const char* title, std::time_t* ts_value) {
            std::time_t new_time = CtDialogs::date_select_dialog(*pDialog, title, *ts_value);
            if (new_time == 0) return;
            *ts_value = new_time;
            button->set_label(str::time_format(ts_format, new_time));
        };
        ts_node_created_after_button->signal_clicked().connect(
            sigc::bind(on_ts_node_button_clicked,
                       ts_node_created_after_button,
                       _("Node Created After"),
                       &s_options.ts_cre_after.time));
        ts_node_created_before_button->signal_clicked().connect(
            sigc::bind(on_ts_node_button_clicked,
                       ts_node_created_before_button,
                       _("Node Created Before"),
                       &s_options.ts_cre_before.time));
        ts_node_modified_after_button->signal_clicked().connect(
            sigc::bind(on_ts_node_button_clicked,
                       ts_node_modified_after_button,
                       _("Node Modified After"),
                       &s_options.ts_mod_after.time));
        ts_node_modified_before_button->signal_clicked().connect(
            sigc::bind(on_ts_node_button_clicked,
                       ts_node_modified_before_button,
                       _("Node Modified Before"),
                       &s_options.ts_mod_before.time));
    }
    auto node_content_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Node Content")});
    node_content_checkbutton->set_active(s_options.node_content);
    auto node_name_n_tags_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Node Name and Tags")});
    node_name_n_tags_checkbutton->set_active(s_options.node_name_n_tags);
    auto hbox_node_content_name_n_tags = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 3/*spacing*/});
    hbox_node_content_name_n_tags->pack_start(*node_content_checkbutton);
    hbox_node_content_name_n_tags->pack_start(*node_name_n_tags_checkbutton);
    auto only_sel_n_subnodes_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Only Selected Node and Subnodes")});
    only_sel_n_subnodes_checkbutton->set_active(s_options.only_sel_n_subnodes);
    auto iter_dialog_checkbutton = Gtk::manage(new Gtk::CheckButton{_("Show Iterated Find/Replace Dialog")});
    iter_dialog_checkbutton->set_active(s_options.iterative_dialog);
    four_1_hbox->pack_start(*match_case_checkbutton);
    four_1_hbox->pack_start(*reg_exp_hbox);
    four_2_hbox->pack_start(*whole_word_checkbutton);
    four_2_hbox->pack_start(*start_word_checkbutton);
    four_3_hbox->pack_start(*accent_insensitive_checkbutton);
    four_3_hbox->pack_start(*override_exclusions_checkbutton);
    bw_fw_hbox->pack_start(*fw_radiobutton);
    bw_fw_hbox->pack_start(*bw_radiobutton);
    three_hbox->pack_start(*all_radiobutton);
    three_vbox->pack_start(*first_from_radiobutton);
    three_vbox->pack_start(*first_all_radiobutton);
    three_hbox->pack_start(*three_vbox);
    opt_vbox->pack_start(*four_1_hbox);
    opt_vbox->pack_start(*four_2_hbox);
    opt_vbox->pack_start(*four_3_hbox);
    opt_vbox->pack_start(*Gtk::manage(new Gtk::HSeparator{}));
    opt_vbox->pack_start(*bw_fw_hbox);
    opt_vbox->pack_start(*Gtk::manage(new Gtk::HSeparator{}));
    opt_vbox->pack_start(*three_hbox);
    opt_vbox->pack_start(*Gtk::manage(new Gtk::HSeparator{}));
    if (multiple_nodes) {
        opt_vbox->pack_start(*ts_frame);
        opt_vbox->pack_start(*Gtk::manage(new Gtk::HSeparator{}));
        opt_vbox->pack_start(*hbox_node_content_name_n_tags);
        opt_vbox->pack_start(*only_sel_n_subnodes_checkbutton);
    }
    opt_vbox->pack_start(*iter_dialog_checkbutton);
    auto opt_frame = Gtk::manage(new Gtk::Frame{Glib::ustring("<b>")+_("Search options")+"</b>"});
    dynamic_cast<Gtk::Label*>(opt_frame->get_label_widget())->set_use_markup(true);
    opt_frame->set_shadow_type(Gtk::SHADOW_NONE);
    opt_frame->add(*opt_vbox);
    auto content_area = pDialog->get_content_area();
    content_area->set_spacing(5);
    content_area->pack_start(*search_frame);
    if (s_state.replace_active) content_area->pack_start(*replace_frame);
    content_area->pack_start(*opt_frame);
    content_area->show_all();
    search_entry->grab_focus();

    reg_exp_help_button->signal_clicked().connect([](){
        fs::open_weblink("https://developer-old.gnome.org/glib/stable/glib-regex-syntax.html");
    });

    auto press_enter = [button_ok](GdkEventKey* pEventKey){
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            if (button_ok && button_ok->get_sensitive()) {
                button_ok->clicked();
                return true;
            }
        }
        return false;
    };
    pDialog->signal_key_press_event().connect(press_enter);
    search_entry->signal_key_press_event().connect(press_enter, false);
    if (replace_entry) {
        replace_entry->signal_key_press_event().connect(press_enter, false);
    }

    button_cancel->signal_clicked().connect([pDialog, &s_state](){
        pDialog->get_position(s_state.searchDialogPos[0], s_state.searchDialogPos[1]);
        pDialog->hide();
    });

    button_ok->signal_clicked().connect([pDialog,
                                         &s_state,
                                         &s_options,
                                         multiple_nodes,
                                         search_entry,
                                         replace_entry,
                                         match_case_checkbutton,
                                         reg_exp_checkbutton,
                                         accent_insensitive_checkbutton,
                                         override_exclusions_checkbutton,
                                         whole_word_checkbutton,
                                         start_word_checkbutton,
                                         fw_radiobutton,
                                         all_radiobutton,
                                         first_from_radiobutton,
                                         ts_node_created_after_checkbutton,
                                         ts_node_created_before_checkbutton,
                                         ts_node_modified_after_checkbutton,
                                         ts_node_modified_before_checkbutton,
                                         node_content_checkbutton,
                                         node_name_n_tags_checkbutton,
                                         only_sel_n_subnodes_checkbutton,
                                         iter_dialog_checkbutton,
                                         pCtMainWin](){
        pDialog->get_position(s_state.searchDialogPos[0], s_state.searchDialogPos[1]);
        pDialog->hide();

        s_options.str_find = search_entry->get_text();
        if (replace_entry) {
            s_options.str_replace = replace_entry->get_text();
        }
        s_options.match_case = match_case_checkbutton->get_active();
        s_options.reg_exp = reg_exp_checkbutton->get_active();
        s_options.accent_insensitive = accent_insensitive_checkbutton->get_active();
        s_options.override_exclusions = override_exclusions_checkbutton->get_active();
        s_options.whole_word = whole_word_checkbutton->get_active();
        s_options.start_word = start_word_checkbutton->get_active();
        s_options.direction_fw = fw_radiobutton->get_active();
        if (all_radiobutton->get_active())              s_options.all_firstsel_firstall = 0;
        else if (first_from_radiobutton->get_active())  s_options.all_firstsel_firstall = 1;
        else                                            s_options.all_firstsel_firstall = 2;
        s_options.ts_cre_after.on = multiple_nodes ? ts_node_created_after_checkbutton->get_active() : false;
        s_options.ts_cre_before.on = multiple_nodes ? ts_node_created_before_checkbutton->get_active() : false;
        s_options.ts_mod_after.on = multiple_nodes ? ts_node_modified_after_checkbutton->get_active() : false;
        s_options.ts_mod_before.on = multiple_nodes ? ts_node_modified_before_checkbutton->get_active() : false;
        s_options.node_content = node_content_checkbutton->get_active();
        s_options.node_name_n_tags = node_name_n_tags_checkbutton->get_active();
        s_options.only_sel_n_subnodes = only_sel_n_subnodes_checkbutton->get_active();
        s_options.iterative_dialog = iter_dialog_checkbutton->get_active();
        // special cases (#2190)
        if (s_options.reg_exp and s_options.str_find == ".*" ) {
            s_options.node_content = false;
            s_options.node_name_n_tags = true;
        }

        s_state.curr_find_pattern = s_options.str_find;
        if (multiple_nodes) {
            s_state.curr_find_type = CtCurrFindType::MultipleNodes;
            pCtMainWin->get_ct_actions()->find_in_multiple_nodes_ok_clicked();
        }
        else {
            s_state.curr_find_type = CtCurrFindType::SingleNode;
            pCtMainWin->get_ct_actions()->find_in_selected_node_ok_clicked();
        }
    });
    s_state.searchfinddialog->show();
    if (s_state.searchDialogPos[0] >= 0) {
        s_state.searchfinddialog->move(s_state.searchDialogPos[0], s_state.searchDialogPos[1]);
    }
}

void CtDialogs::no_matches_dialog(CtMainWin* pCtMainWin,
                                  const Glib::ustring& title,
                                  const Glib::ustring& message)
{
    Gtk::Dialog dialog{title,
                       *pCtMainWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(300, -1);
    Gtk::Label message_label{message};
    message_label.set_use_markup(true);
    message_label.set_padding(3/*xpad*/, 5/*ypad*/);
    Gtk::Box vbox{Gtk::ORIENTATION_VERTICAL, 5/*spacing*/};
    vbox.pack_start(message_label);
    if (CtTreeIter::get_hit_exclusion_from_search()) {
        auto pHBoxExclusions = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 3/*spacing*/});
        Gtk::Image* pImageExclusions = pCtMainWin->new_managed_image_from_stock("ct_ghost", Gtk::ICON_SIZE_BUTTON);
        pImageExclusions->set_padding(3/*xpad*/, 0/*ypad*/);
        pHBoxExclusions->pack_start(*pImageExclusions, false, false);
        auto pLabelExclusions = Gtk::manage(new Gtk::Label{_("At least one node was skipped because of exclusions set in the node properties.\nIn order to clear all the exclusions, use the menu:\nSearch -> Clear All Exclusions From Search")});
        pLabelExclusions->set_xalign(0.0);
        pHBoxExclusions->pack_start(*pLabelExclusions);
        vbox.pack_start(*pHBoxExclusions, false, false);
    }
    Gtk::Box* pContentArea = dialog.get_content_area();
    pContentArea->pack_start(vbox);
    pContentArea->show_all();
    dialog.run();
}

/*static*/Glib::RefPtr<CtMatchDialogStore> CtMatchDialogStore::create(const size_t maxMatchesInPage)
{
    Glib::RefPtr<CtMatchDialogStore> rModel{new CtMatchDialogStore{maxMatchesInPage}};
    rModel->set_column_types(rModel->columns);
    return rModel;
}

void CtMatchDialogStore::deep_clear()
{
    clear();
    saved_path.clear();
    _page_idx = 0;
    _all_matches.clear();
}

CtMatchRowData* CtMatchDialogStore::add_row(gint64 node_id,
                                            const Glib::ustring& node_name,
                                            const Glib::ustring& node_hier_name,
                                            int start_offset,
                                            int end_offset,
                                            int line_num,
                                            const Glib::ustring& line_content)
{
    _all_matches.push_back(CtMatchRowData{
        .node_id = node_id,
        .node_name = node_name,
        .node_hier_name = node_hier_name,
        .start_offset = start_offset,
        .end_offset = end_offset,
        .line_num = line_num,
        .line_content = line_content
    });
    return &_all_matches.back();
}

void CtMatchDialogStore::load_current_page()
{
    if (get_iter("0")) {
        return; // already populated
    }
    const size_t iMax = (_page_idx + 1) * cMaxMatchesInPage;
    for (size_t i = _page_idx * cMaxMatchesInPage; i < iMax; ++i) {
        if (i >= _all_matches.size()) break;
        (void)_add_row(_all_matches.at(i));
    }
}

void CtMatchDialogStore::load_next_page()
{
    if (not has_next_page()) return;
    clear();
    ++_page_idx;
    load_current_page();
}

void CtMatchDialogStore::load_prev_page()
{
    if (not has_prev_page()) return;
    clear();
    --_page_idx;
    load_current_page();
}

size_t CtMatchDialogStore::get_tot_matches()
{
    return _all_matches.size();
}

bool CtMatchDialogStore::is_multipage()
{
    return _all_matches.size() > cMaxMatchesInPage;
}

bool CtMatchDialogStore::has_next_page()
{
    return _all_matches.size() > cMaxMatchesInPage*(_page_idx + 1);
}

bool CtMatchDialogStore::has_prev_page()
{
    return _page_idx > 0;
}

std::string CtMatchDialogStore::get_this_page_range()
{
    const size_t match_idx_start = _page_idx * cMaxMatchesInPage;
    size_t match_idx_end = (_page_idx + 1) * cMaxMatchesInPage - 1;
    if (match_idx_end >= _all_matches.size()) match_idx_end = _all_matches.size() - 1;
    return fmt::format("{}..{}", match_idx_start + 1, match_idx_end + 1);
}

std::string CtMatchDialogStore::get_next_page_range()
{
    if (not has_next_page()) return "";
    const size_t match_idx_start = (_page_idx + 1) * cMaxMatchesInPage;
    size_t match_idx_end = (_page_idx + 2) * cMaxMatchesInPage - 1;
    if (match_idx_end >= _all_matches.size()) match_idx_end = _all_matches.size() - 1;
    return fmt::format("{}..{}", match_idx_start + 1, match_idx_end + 1);
}

std::string CtMatchDialogStore::get_prev_page_range()
{
    if (not has_prev_page()) return "";
    const size_t match_idx_start = (_page_idx - 1) * cMaxMatchesInPage;
    const size_t match_idx_end = _page_idx * cMaxMatchesInPage - 1;
    return fmt::format("{}..{}", match_idx_start + 1, match_idx_end + 1);
}

Gtk::TreeIter CtMatchDialogStore::_add_row(const CtMatchRowData& row_data) {
    Gtk::TreeIter retIter = append();
    Gtk::TreeRow row = *retIter;
    row[columns.node_id] = row_data.node_id;
    row[columns.node_name] = row_data.node_name;
    row[columns.node_hier_name] = row_data.node_hier_name;
    row[columns.start_offset] = row_data.start_offset;
    row[columns.end_offset] = row_data.end_offset;
    row[columns.line_num] = row_data.line_num;
    row[columns.line_content] = row_data.line_content;
    return retIter;
}

void CtDialogs::match_dialog(const std::string& str_find,
                             CtMainWin* pCtMainWin,
                             CtSearchState& s_state)
{
    auto rModel = s_state.match_store;
    auto pMatchesDialog = new Gtk::Dialog{"", *pCtMainWin, Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    s_state.pMatchStoreDialog = pMatchesDialog;
    pMatchesDialog->set_transient_for(*pCtMainWin);
    if (rModel->dlg_size[0] > 0) {
        pMatchesDialog->set_default_size(rModel->dlg_size[0], rModel->dlg_size[1]);
        pMatchesDialog->move(rModel->dlg_pos[0], rModel->dlg_pos[1]);
    }
    else {
        pMatchesDialog->set_default_size(700, 350);
        pMatchesDialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }
    Gtk::Button* pButtonPrev = pMatchesDialog->add_button("", Gtk::RESPONSE_NO);
    Gtk::Button* pButtonNext = pMatchesDialog->add_button("", Gtk::RESPONSE_YES);
    pButtonPrev->set_image_from_icon_name("ct_go-back");
    pButtonPrev->set_always_show_image(true);
    pButtonNext->set_image_from_icon_name("ct_go-forward");
    pButtonNext->set_always_show_image(true);
    pButtonNext->set_image_position(Gtk::PositionType::POS_RIGHT);
    CtMenuAction* pAction = pCtMainWin->get_ct_menu().find_action("toggle_show_allmatches_dlg");
    Glib::ustring label = CtStrUtil::get_accelerator_label(pAction->get_shortcut(pCtMainWin->get_ct_config()));
    Gtk::Button* pButtonHide = pMatchesDialog->add_button(str::format(_("Hide (Restore with '%s')"), label), Gtk::RESPONSE_CLOSE);
    pButtonHide->set_image_from_icon_name("ct_close", Gtk::ICON_SIZE_BUTTON);

    rModel->load_current_page();
    auto pTreeview = Gtk::manage(new Gtk::TreeView{rModel});
    pTreeview->append_column(_("Node Name"), rModel->columns.node_name);
    pTreeview->append_column(_("Line"), rModel->columns.line_num);
    pTreeview->append_column(_("Line Content"), rModel->columns.line_content);
    pTreeview->append_column("", rModel->columns.node_hier_name);
    pTreeview->get_column(3)->property_visible() = false;
    pTreeview->set_tooltip_column(3);
    auto pScrolledBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pScrolledBox->pack_start(*pTreeview);
    if (CtTreeIter::get_hit_exclusion_from_search()) {
        auto pHBoxExclusions = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 3/*spacing*/});
        Gtk::Image* pImageExclusions = pCtMainWin->new_managed_image_from_stock("ct_ghost", Gtk::ICON_SIZE_BUTTON);
        pImageExclusions->set_padding(3/*xpad*/, 0/*ypad*/);
        pHBoxExclusions->pack_start(*pImageExclusions, false, false);
        auto pLabelExclusions = Gtk::manage(new Gtk::Label{_("At least one node was skipped because of exclusions set in the node properties.\nIn order to clear all the exclusions, use the menu:\nSearch -> Clear All Exclusions From Search")});
        pLabelExclusions->set_xalign(0.0);
        pHBoxExclusions->pack_start(*pLabelExclusions);
        pScrolledBox->pack_start(*pHBoxExclusions, false, false);
    }
    auto pScrolledwindowAllmatches = Gtk::manage(new Gtk::ScrolledWindow{});
    pScrolledwindowAllmatches->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pScrolledwindowAllmatches->add(*pScrolledBox);

    Gtk::Box* pContentArea = pMatchesDialog->get_content_area();
    pContentArea->pack_start(*pScrolledwindowAllmatches);

    auto select_found_line = [pTreeview, &s_state, pCtMainWin](){
        if (s_state.in_loading) {
            return;
        }
        Gtk::TreeIter list_iter = pTreeview->get_selection()->get_selected();
        if (not list_iter) {
            return;
        }
        gint64 node_id = list_iter->get_value(s_state.match_store->columns.node_id);
        CtTreeIter tree_iter = pCtMainWin->get_tree_store().get_node_from_node_id(node_id);
        if (not tree_iter) {
            CtDialogs::error_dialog(str::format(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)"), node_id),
                                    *pCtMainWin);
            s_state.match_store->erase(list_iter);
            return;
        }
        // remove previous selection because it can cause freezing in specific cases, see more in issue
        auto fake_iter = pCtMainWin->get_text_view().get_buffer()->get_iter_at_offset(-1);
        pCtMainWin->get_text_view().get_buffer()->place_cursor(fake_iter);

        pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
        auto rCurrBuffer = pCtMainWin->get_text_view().get_buffer();
        rCurrBuffer->select_range(rCurrBuffer->get_iter_at_offset(list_iter->get_value(s_state.match_store->columns.start_offset)),
                                  rCurrBuffer->get_iter_at_offset(list_iter->get_value(s_state.match_store->columns.end_offset)));
        pCtMainWin->get_text_view().scroll_to(rCurrBuffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);

        // pump events so UI's not going to freeze (#835)
        while (gdk_events_pending())
            gtk_main_iteration();
    };

    if (not rModel->saved_path.empty()) {
        auto iter = rModel->get_iter(rModel->saved_path);
        if (iter) {
            pTreeview->set_cursor(rModel->get_path(iter));
            pTreeview->scroll_to_row(rModel->get_path(iter), 0.5);
            select_found_line();
        }
    }

    pTreeview->signal_cursor_changed().connect(select_found_line);

    auto on_allmatchesdialog_delete_event = [pMatchesDialog, rModel, pTreeview, &s_state](GdkEventAny* /*any_event*/)->bool{
        pMatchesDialog->get_position(rModel->dlg_pos[0], rModel->dlg_pos[1]);
        pMatchesDialog->get_size(rModel->dlg_size[0], rModel->dlg_size[1]);
        Gtk::TreeIter list_iter = pTreeview->get_selection()->get_selected();
        rModel->saved_path = list_iter ? pTreeview->get_model()->get_path(list_iter).to_string() : "";

        s_state.pMatchStoreDialog = nullptr;
        delete pMatchesDialog; // should delete ourselves
        return false;
    };
    pMatchesDialog->signal_delete_event().connect(on_allmatchesdialog_delete_event);
    pButtonHide->signal_clicked().connect([pMatchesDialog](){
        pMatchesDialog->close();
    });
    auto f_reEval_multipage = [&str_find, rModel, pMatchesDialog, pButtonPrev, pButtonNext](){
        Glib::ustring title = "'" + str_find + "'  -  " + std::to_string(rModel->get_tot_matches()) + CtConst::CHAR_SPACE + _("Matches");
        if (rModel->is_multipage()) {
            title += "  -  " + rModel->get_this_page_range();
        }
        pMatchesDialog->set_title(title);
        if (rModel->has_prev_page()) {
            pButtonPrev->set_label(rModel->get_prev_page_range());
            pButtonPrev->set_visible(true);
        }
        else {
            pButtonPrev->set_visible(false);
        }
        if (rModel->has_next_page()) {
            pButtonNext->set_label(rModel->get_next_page_range());
            pButtonNext->set_visible(true);
        }
        else {
            pButtonNext->set_visible(false);
        }
    };

    pButtonPrev->signal_clicked().connect([&s_state, f_reEval_multipage](){
        s_state.in_loading = true;
        s_state.match_store->load_prev_page();
        s_state.in_loading = false;
        f_reEval_multipage();
    });
    pButtonNext->signal_clicked().connect([&s_state, f_reEval_multipage](){
        s_state.in_loading = true;
        s_state.match_store->load_next_page();
        s_state.in_loading = false;
        f_reEval_multipage();
    });

    pMatchesDialog->show_all();
    f_reEval_multipage();
}

// Iterated Find/Replace Dialog
void CtDialogs::iterated_find_dialog(CtMainWin* pCtMainWin, CtSearchState& s_state)
{
    if (not s_state.iteratedfinddialog) {
        spdlog::debug("+iteratedfinddialog");
        auto pDialog = new Gtk::Dialog{_("Iterate Latest Find/Replace"),
                                       *pCtMainWin,
                                       Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
        auto button_close = pDialog->add_button(_("Close"), 0);
        auto button_find_bw = pDialog->add_button(_("Find Previous"), 4);
        auto button_find_fw = pDialog->add_button(_("Find Next"), 1);
        auto button_replace = pDialog->add_button(_("Replace"), 2);
        auto button_undo = pDialog->add_button(_("Undo"), 3);
        pDialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
        button_close->set_image_from_icon_name("ct_close", Gtk::ICON_SIZE_BUTTON);
        button_find_bw->set_image_from_icon_name("ct_find_back", Gtk::ICON_SIZE_BUTTON);
        button_find_fw->set_image_from_icon_name("ct_find_again", Gtk::ICON_SIZE_BUTTON);
        button_replace->set_image_from_icon_name("ct_find_replace", Gtk::ICON_SIZE_BUTTON);
        button_undo->set_image_from_icon_name("ct_undo", Gtk::ICON_SIZE_BUTTON);
        button_close->set_always_show_image(true);
        button_find_bw->set_always_show_image(true);
        button_find_fw->set_always_show_image(true);
        button_replace->set_always_show_image(true);
        button_undo->set_always_show_image(true);

        button_find_fw->grab_focus();
        button_find_fw->grab_default();

        button_close->signal_clicked().connect([pDialog, &s_state](){
            pDialog->get_position(s_state.iterDialogPos[0], s_state.iterDialogPos[1]);
            pDialog->hide();
        });
        button_find_fw->signal_clicked().connect([pDialog, &s_state, pCtMainWin](){
            pDialog->get_position(s_state.iterDialogPos[0], s_state.iterDialogPos[1]);
            pDialog->hide();
            s_state.replace_active = false;
            pCtMainWin->get_ct_actions()->find_again_iter(true/*fromIterativeDialog*/);
        });
        button_find_bw->signal_clicked().connect([pDialog, &s_state, pCtMainWin](){
            pDialog->get_position(s_state.iterDialogPos[0], s_state.iterDialogPos[1]);
            pDialog->hide();
            s_state.replace_active = false;
            pCtMainWin->get_ct_actions()->find_back_iter(true/*fromIterativeDialog*/);
        });
        button_replace->signal_clicked().connect([pDialog, &s_state, pCtMainWin](){
            pDialog->get_position(s_state.iterDialogPos[0], s_state.iterDialogPos[1]);
            pDialog->hide();
            s_state.replace_active = true;
            s_state.replace_subsequent = true;
            pCtMainWin->get_ct_actions()->find_again_iter(true/*fromIterativeDialog*/);
            s_state.replace_subsequent = false;
        });
        button_undo->signal_clicked().connect([pCtMainWin](){
            pCtMainWin->get_ct_actions()->requested_step_back();
        });

        s_state.iteratedfinddialog.reset(pDialog);
    }
    s_state.iteratedfinddialog->show();
    if (s_state.iterDialogPos[0] >= 0) {
        s_state.iteratedfinddialog->move(s_state.iterDialogPos[0], s_state.iterDialogPos[1]);
    }
}
