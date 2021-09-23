/*
 * ct_dialogs_find.cc
 *
 * Copyright 2009-2021
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

std::string CtDialogs::dialog_search(Gtk::Window* pParentWin,
                                     const std::string& title,
                                     CtSearchOptions& s_options,
                                     bool replace_on,
                                     bool multiple_nodes,
                                     bool pattern_required)
{
    Gtk::Dialog dialog{title,
                       *pParentWin,
                       Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    dialog.set_transient_for(*pParentWin);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, -1);

    Gtk::Entry search_entry{};
    search_entry.set_text(s_options.str_find);

    auto button_ok = dialog.get_widget_for_response(Gtk::RESPONSE_ACCEPT);
    if (pattern_required) {
        button_ok->set_sensitive(s_options.str_find.length() != 0);
        search_entry.signal_changed().connect([&button_ok, &search_entry](){
            button_ok->set_sensitive(search_entry.get_text().length() != 0);
        });
    }
    auto search_frame = Gtk::Frame{std::string("<b>")+_("Search for")+"</b>"};
    dynamic_cast<Gtk::Label*>(search_frame.get_label_widget())->set_use_markup(true);
    search_frame.set_shadow_type(Gtk::SHADOW_NONE);
    search_frame.add(search_entry);

    Gtk::Frame* replace_frame{nullptr};
    Gtk::Entry* replace_entry{nullptr};
    if (replace_on) {
        replace_entry = Gtk::manage(new Gtk::Entry{});
        replace_entry->set_text(s_options.str_replace);
        replace_frame = Gtk::manage(new Gtk::Frame{std::string("<b>")+_("Replace with")+"</b>"});
        dynamic_cast<Gtk::Label*>(replace_frame->get_label_widget())->set_use_markup(true);
        replace_frame->set_shadow_type(Gtk::SHADOW_NONE);
        replace_frame->add(*replace_entry);
    }
    Gtk::Box opt_vbox{Gtk::ORIENTATION_VERTICAL, 1/*spacing*/};
    Gtk::Box four_1_hbox{Gtk::ORIENTATION_HORIZONTAL};
    four_1_hbox.set_homogeneous(true);
    Gtk::Box four_2_hbox{Gtk::ORIENTATION_HORIZONTAL};
    four_2_hbox.set_homogeneous(true);
    Gtk::Box bw_fw_hbox{Gtk::ORIENTATION_HORIZONTAL};
    bw_fw_hbox.set_homogeneous(true);
    Gtk::Box three_hbox{Gtk::ORIENTATION_HORIZONTAL};
    three_hbox.set_homogeneous(true);
    Gtk::Box three_vbox{Gtk::ORIENTATION_VERTICAL};
    Gtk::CheckButton match_case_checkbutton{_("Match Case")};
    match_case_checkbutton.set_active(s_options.match_case);
    Gtk::CheckButton reg_exp_checkbutton{_("Regular Expression")};
    reg_exp_checkbutton.set_active(s_options.reg_exp);
    Gtk::CheckButton whole_word_checkbutton{_("Whole Word")};
    whole_word_checkbutton.set_active(s_options.whole_word);
    Gtk::CheckButton start_word_checkbutton{_("Start Word")};
    start_word_checkbutton.set_active(s_options.start_word);
    Gtk::RadioButton fw_radiobutton{_("Forward")};
    fw_radiobutton.set_active(s_options.direction_fw);
    Gtk::RadioButton bw_radiobutton{_("Backward")};
    bw_radiobutton.join_group(fw_radiobutton);
    bw_radiobutton.set_active(!s_options.direction_fw);
    Gtk::RadioButton all_radiobutton{_("All, List Matches")};
    all_radiobutton.set_active(s_options.all_firstsel_firstall == 0);
    Gtk::RadioButton first_from_radiobutton{_("First From Selection")};
    first_from_radiobutton.join_group(all_radiobutton);
    first_from_radiobutton.set_active(s_options.all_firstsel_firstall == 1);
    Gtk::RadioButton first_all_radiobutton{_("First in All Range")};
    first_all_radiobutton.join_group(all_radiobutton);
    first_all_radiobutton.set_active(s_options.all_firstsel_firstall == 2);

    Gtk::Frame* ts_frame{nullptr};
    Gtk::CheckButton* ts_node_created_after_checkbutton{nullptr};
    Gtk::CheckButton* ts_node_created_before_checkbutton{nullptr};
    Gtk::CheckButton* ts_node_modified_after_checkbutton{nullptr};
    Gtk::CheckButton* ts_node_modified_before_checkbutton{nullptr};
    if (multiple_nodes) {
        std::string ts_format = "%A, %d %B %Y, %H:%M";
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

        auto on_ts_node_button_clicked = [&dialog, ts_format](Gtk::Button* button, const char* title, std::time_t* ts_value) {
            std::time_t new_time = CtDialogs::date_select_dialog(dialog, title, *ts_value);
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
    Gtk::CheckButton only_sel_n_subnodes_checkbutton{_("Only Selected Node and Subnodes")};
    only_sel_n_subnodes_checkbutton.set_active(s_options.only_sel_n_subnodes);
    Gtk::CheckButton iter_dialog_checkbutton{_("Show Iterated Find/Replace Dialog")};
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
    opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator{}));
    opt_vbox.pack_start(bw_fw_hbox);
    opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator{}));
    opt_vbox.pack_start(three_hbox);
    opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator{}));
    if (multiple_nodes) {
        opt_vbox.pack_start(*ts_frame);
        opt_vbox.pack_start(*Gtk::manage(new Gtk::HSeparator{}));
        opt_vbox.pack_start(only_sel_n_subnodes_checkbutton);
    }
    opt_vbox.pack_start(iter_dialog_checkbutton);
    Gtk::Frame opt_frame{std::string("<b>")+_("Search options")+"</b>"};
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

    auto press_enter = [&dialog, &button_ok](GdkEventKey* pEventKey){
        if (GDK_KEY_Return == pEventKey->keyval or GDK_KEY_KP_Enter == pEventKey->keyval) {
            if (button_ok && button_ok->get_sensitive()) {
                dialog.response(Gtk::RESPONSE_ACCEPT);
                return true;
            }
        }
        return false;
    };
    dialog.signal_key_press_event().connect(press_enter);
    search_entry.signal_key_press_event().connect(press_enter, false);

    if (dialog.run() != Gtk::RESPONSE_ACCEPT) {
        return "";
    }
    s_options.str_find = search_entry.get_text();
    if (replace_on)
        s_options.str_replace = replace_entry->get_text();
    s_options.match_case = match_case_checkbutton.get_active();
    s_options.reg_exp = reg_exp_checkbutton.get_active();
    s_options.whole_word = whole_word_checkbutton.get_active();
    s_options.start_word = start_word_checkbutton.get_active();
    s_options.direction_fw = fw_radiobutton.get_active();
    if (all_radiobutton.get_active())              s_options.all_firstsel_firstall = 0;
    else if (first_from_radiobutton.get_active())  s_options.all_firstsel_firstall = 1;
    else                                           s_options.all_firstsel_firstall = 2;
    s_options.ts_cre_after.on = multiple_nodes ? ts_node_created_after_checkbutton->get_active() : false;
    s_options.ts_cre_before.on = multiple_nodes ? ts_node_created_before_checkbutton->get_active() : false;
    s_options.ts_mod_after.on = multiple_nodes ? ts_node_modified_after_checkbutton->get_active() : false;
    s_options.ts_mod_before.on = multiple_nodes ? ts_node_modified_before_checkbutton->get_active() : false;
    s_options.only_sel_n_subnodes = only_sel_n_subnodes_checkbutton.get_active();
    s_options.search_replace_dict_idialog = iter_dialog_checkbutton.get_active();
    return s_options.str_find;
}

void CtDialogs::match_dialog(const Glib::ustring& title,
                             CtMainWin* pCtMainWin,
                             Glib::RefPtr<CtMatchDialogStore>& rModel)
{
    // cannot use static because dialog can be used in the several windows
    Gtk::Dialog* pMatchesDialog = new Gtk::Dialog{title, *pCtMainWin, Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT};
    pMatchesDialog->set_transient_for(*pCtMainWin);
    if (rModel->dlg_size[0] > 0) {
        pMatchesDialog->set_default_size(rModel->dlg_size[0], rModel->dlg_size[1]);
        pMatchesDialog->move(rModel->dlg_pos[0], rModel->dlg_pos[1]);
    }
    else {
        pMatchesDialog->set_default_size(700, 350);
        pMatchesDialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }
    CtMenuAction* pAction = pCtMainWin->get_ct_menu().find_action("toggle_show_allmatches_dlg");
    Glib::ustring label = CtStrUtil::get_accelerator_label(pAction->get_shortcut(pCtMainWin->get_ct_config()));
    Gtk::Button* pButtonHide = pMatchesDialog->add_button(str::format(_("Hide (Restore with '%s')"), label), Gtk::RESPONSE_CLOSE);
    pButtonHide->set_image_from_icon_name("ct_close", Gtk::ICON_SIZE_BUTTON);

    Gtk::TreeView* pTreeview = Gtk::manage(new Gtk::TreeView{rModel});
    pTreeview->append_column(_("Node Name"), rModel->columns.node_name);
    pTreeview->append_column(_("Line"), rModel->columns.line_num);
    pTreeview->append_column(_("Line Content"), rModel->columns.line_content);
    pTreeview->append_column("", rModel->columns.node_hier_name);
    pTreeview->get_column(3)->property_visible() = false;
    pTreeview->set_tooltip_column(3);
    Gtk::ScrolledWindow* pScrolledwindowAllmatches = Gtk::manage(new Gtk::ScrolledWindow{});
    pScrolledwindowAllmatches->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pScrolledwindowAllmatches->add(*pTreeview);
    Gtk::Box* pContentArea = pMatchesDialog->get_content_area();
    pContentArea->pack_start(*pScrolledwindowAllmatches);

    auto select_found_line = [pTreeview, rModel, pCtMainWin](){
        Gtk::TreeIter list_iter = pTreeview->get_selection()->get_selected();
        if (!list_iter) {
            return;
        }
        gint64 node_id = list_iter->get_value(rModel->columns.node_id);
        CtTreeIter tree_iter = pCtMainWin->get_tree_store().get_node_from_node_id(node_id);
        if (!tree_iter) {
            CtDialogs::error_dialog(str::format(_("The Link Refers to a Node that Does Not Exist Anymore (Id = %s)"), node_id),
                                    *pCtMainWin);
            rModel->erase(list_iter);
            return;
        }
        // remove previous selection because it can cause freezing in specific cases, see more in issue
        auto fake_iter = pCtMainWin->get_text_view().get_buffer()->get_iter_at_offset(-1);
        pCtMainWin->get_text_view().get_buffer()->place_cursor(fake_iter);

        pCtMainWin->get_tree_view().set_cursor_safe(tree_iter);
        auto rCurrBuffer = pCtMainWin->get_text_view().get_buffer();
        rCurrBuffer->select_range(rCurrBuffer->get_iter_at_offset(list_iter->get_value(rModel->columns.start_offset)),
                                  rCurrBuffer->get_iter_at_offset(list_iter->get_value(rModel->columns.end_offset)));
        pCtMainWin->get_text_view().scroll_to(rCurrBuffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);

        // pump events so UI's not going to freeze (#835)
        while (gdk_events_pending())
            gtk_main_iteration();
    };

    if (!rModel->saved_path.empty()) {
        auto iter = rModel->get_iter(rModel->saved_path);
        if (iter) {
            pTreeview->set_cursor(rModel->get_path(iter));
            pTreeview->scroll_to_row(rModel->get_path(iter), 0.5);
            select_found_line();
        }
    }

    pTreeview->signal_cursor_changed().connect(select_found_line);

    auto on_allmatchesdialog_delete_event = [pMatchesDialog, rModel, pTreeview](GdkEventAny* /*any_event*/)->bool{
        pMatchesDialog->get_position(rModel->dlg_pos[0], rModel->dlg_pos[1]);
        pMatchesDialog->get_size(rModel->dlg_size[0], rModel->dlg_size[1]);
        Gtk::TreeIter list_iter = pTreeview->get_selection()->get_selected();
        rModel->saved_path = list_iter ? pTreeview->get_model()->get_path(list_iter).to_string() : "";

        delete pMatchesDialog; // should delete ourselves
        return false;
    };
    pMatchesDialog->signal_delete_event().connect(on_allmatchesdialog_delete_event);
    pButtonHide->signal_clicked().connect([pMatchesDialog](){
        pMatchesDialog->close();
    });

    pMatchesDialog->show_all();
}

// Iterated Find/Replace Dialog
void CtDialogs::iterated_find_dialog(CtMainWin* pCtMainWin, CtSearchState& s_state)
{
    if (!s_state.iteratedfinddialog) {
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
