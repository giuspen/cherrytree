#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <glibmm/regex.h>
#include <regex>
#include "ct_image.h"
#include "ct_app.h"
#include "ct_dialogs.h"
#include "ct_doc_rw.h"
#include "src/fmt/fmt.h"


struct SearchOptions {
    struct time_search {
        std::time_t time;
        bool        on;
    };

    time_search ts_cre_after;
    time_search ts_cre_before;
    time_search ts_mod_after;
    time_search ts_mod_before;
    std::string search_replace_dict_find        = "";
    std::string search_replace_dict_replace     = "";
    bool        search_replace_dict_match_case  = false;
    bool        search_replace_dict_reg_exp     = false;
    bool        search_replace_dict_whole_word  = false;
    bool        search_replace_dict_start_word  = false;
    bool        search_replace_dict_fw          = true;
    int         search_replace_dict_a_ff_fa     = 0;
    bool        search_replace_dict_idialog     = true;
} s_options;

struct SearchState {
    bool         replace_active     = false;
    bool         replace_subsequent = false;
    std::string  curr_find_where    = "";
    std::string  curr_find_pattern  = "";
    bool         from_find_iterated = false;
    bool         from_find_back     = false;
    bool         newline_trick      = false;

    bool         first_useful_node  = false;
    int          counted_nodes      = 0;
    int          processed_nodes    = 0;
    int          latest_matches     = 0;

    int          matches_num;
    bool         user_active; //?
    bool         all_matches_first_in_node = false;

    int          latest_node_offset = -1;
    gint64       latest_node_offset_node_id = -1;

    Gtk::Dialog* iteratedfinddialog = nullptr;

    Glib::RefPtr<ct_dialogs::CtMatchDialogStore> match_store;

} s_state;

void CtActions::_find_init()
{
    s_state.match_store = ct_dialogs::CtMatchDialogStore::create();
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
    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = _ctMainWin->get_text_view().get_buffer();

    std::string entry_hint;
    std::string pattern;
    if (s_state.from_find_iterated == false) {
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
    bool forward = s_options.search_replace_dict_fw;
    if (s_state.from_find_back) {
        forward = !forward;
        s_state.from_find_back = false;
    }
    bool first_fromsel = s_options.search_replace_dict_a_ff_fa == 1;
    bool all_matches = s_options.search_replace_dict_a_ff_fa == 0;
    s_state.matches_num = 0;

    // searching start
    bool user_active_restore = s_state.user_active;
    s_state.user_active = false;

    if (all_matches) {
        s_state.match_store->clear();
        s_state.all_matches_first_in_node = true;
        while (_parse_node_content_iter(_ctMainWin->curr_tree_iter(), curr_buffer, pattern, forward, first_fromsel, all_matches, true))
            s_state.matches_num += 1;
    }
    else if (_parse_node_content_iter(_ctMainWin->curr_tree_iter(), curr_buffer, pattern, forward, first_fromsel, all_matches, true))
        s_state.matches_num = 1;
    if (s_state.matches_num == 0)
        ct_dialogs::info_dialog(fmt::format(_("The pattern '%s' was not found"), pattern), *_ctMainWin);
    else if (all_matches) {
        std::string title = std::to_string(s_state.matches_num) + CtConst::CHAR_SPACE + _("Matches");
        ct_dialogs::match_dialog(title, *_ctMainWin, s_state.match_store);
    }
    else if (s_options.search_replace_dict_idialog) {
        _iterated_find_dialog();
    }
    s_state.user_active = user_active_restore;
}

static int _count_nodes(const Gtk::TreeNodeChildren& children) {
    int count = 1;
    for(auto& child: children) { count += _count_nodes(child.children()); }
    return count;
}

void CtActions::_find_in_all_nodes(bool for_current_node)
{
    if (!_is_there_selected_node_or_error()) return;
    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = _ctMainWin->get_text_view().get_buffer();
    CtStatusBar& ctStatusBar = _ctMainWin->get_status_bar();

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

    CtTreeIter starting_tree_iter = _ctMainWin->curr_tree_iter();
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
        node_iter = _ctMainWin->curr_tree_iter();
    }
    else {
        s_state.first_useful_node = true; // all range will be parsed so no matter
        node_iter = forward ? _ctTreestore->get_iter_first() : _ctTreestore->get_tree_iter_last_sibling(_ctTreestore->get_store()->children());
    }
    s_state.matches_num = 0;
    if (all_matches) s_state.match_store->clear();

    std::string tree_expanded_collapsed_string = _ctTreestore->get_tree_expanded_collapsed_string(_ctMainWin->get_tree_view());
    // searching start
    bool user_active_restore = s_state.user_active;
    s_state.user_active = false;
    s_state.processed_nodes = 0;
    s_state.latest_matches = 0;
    s_state.counted_nodes = for_current_node ? _count_nodes(_ctMainWin->curr_tree_iter()->children()) : (_count_nodes(_ctTreestore->get_store()->children()) - 1);
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
        CtTreeIter ct_node_iter = _ctTreestore->to_ct_tree_iter(node_iter);
        while (_parse_given_node_content(ct_node_iter, pattern, forward, first_fromsel, all_matches)) {
            s_state.matches_num += 1;
            if (!all_matches ||  ctStatusBar.is_progress_stop()) break;
        }
        s_state.processed_nodes += 1;
        if (s_state.matches_num == 1 || !all_matches) break;
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
    std::cout << search_end_time - search_start_time << " sec" << std::endl;

    s_state.user_active = user_active_restore;
    _ctTreestore->set_tree_expanded_collapsed_string(tree_expanded_collapsed_string, _ctMainWin->get_tree_view(), CtApp::P_ctCfg->nodesBookmExp);
    if (!s_state.matches_num || all_matches) {
        _ctMainWin->get_tree_view().set_cursor_safe(starting_tree_iter);
        //todo: self.dad.objects_buffer_refresh()
        _ctMainWin->get_text_view().grab_focus();
        curr_buffer->place_cursor(curr_buffer->get_iter_at_offset(current_cursor_pos));
        _ctMainWin->get_text_view().scroll_to(curr_buffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);
    }
    if (!s_state.matches_num)
        ct_dialogs::info_dialog(fmt::format(_("The pattern '%s' was not found"), std::string(pattern)), *_ctMainWin);
    else {
        if (all_matches) {
            std::string title = std::to_string(s_state.matches_num) + CtConst::CHAR_SPACE + _("Matches");
            ct_dialogs::match_dialog(title, *_ctMainWin, s_state.match_store);
        } else {
            _ctMainWin->get_tree_view().set_cursor_safe(_ctMainWin->curr_tree_iter());
            if (s_options.search_replace_dict_idialog)
                _iterated_find_dialog();
        }
     }
    if (all_matches) {
        // todo: assert self.processed_nodes == self.dad.num_nodes or self.dad.progress_stop, "%s != %s" % (self.processed_nodes, self.dad.num_nodes)
        ctStatusBar.progressBar.hide();
        ctStatusBar.stopButton.hide();
        ctStatusBar.set_progress_stop(false);
    }
}

void CtActions::find_a_node()
{

}

void CtActions::find_again()
{

}

void CtActions::find_back()
{

}

void CtActions::replace_in_selected_node()
{

}

void CtActions::replace_in_all_nodes()
{

}

void CtActions::replace_in_sel_node_and_subnodes()
{

}

void CtActions::replace_in_nodes_names()
{

}

void CtActions::replace_again()
{

}

void CtActions::find_allmatchesdialog_restore()
{

}

// Opens the Search Dialog
std::string CtActions::_dialog_search(const std::string& title, bool replace_on, bool multiple_nodes, bool pattern_required)
{
    Gtk::Dialog dialog(title, *_ctMainWin, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(*_ctMainWin);
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
        std::string ts_label = str::time_format(ts_format, s_options.ts_cre_after.time);
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
            std::time_t new_time = ct_dialogs::date_select_dialog(dialog, title, ts_value);
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

// Returns True if pattern was found, False otherwise
bool CtActions::_parse_given_node_content(CtTreeIter node_iter, Glib::ustring pattern, bool forward, bool first_fromsel, bool all_matches)
{
    auto text_buffer = node_iter.get_node_text_buffer();
    if (!s_state.first_useful_node) {
        // first_fromsel plus first_node not already parsed
        if (!_ctMainWin->curr_tree_iter() || node_iter.get_node_id() == _ctMainWin->curr_tree_iter().get_node_id()) {
            s_state.first_useful_node = true; // a first_node was parsed
            if (_parse_node_content_iter(node_iter, text_buffer, pattern, forward, first_fromsel, all_matches, true))
                return true; // first_node node, first_fromsel
        }
    } else {
        // not first_fromsel or first_fromsel with first_node already parsed
        if (_parse_node_content_iter(node_iter, text_buffer, pattern, forward, first_fromsel, all_matches, false))
            return true; // not first_node node
    }
    // check for children
    if (!node_iter->children().empty()) {
        Gtk::TreeIter child_iter = forward ? node_iter->children().begin() : --node_iter->children().end();
        while (child_iter && !_ctMainWin->get_status_bar().is_progress_stop()) {
            s_state.all_matches_first_in_node = true;
            while (_parse_given_node_content(_ctTreestore->to_ct_tree_iter(child_iter), pattern, forward, first_fromsel, all_matches)) {
                s_state.matches_num += 1;
                if (!all_matches || _ctMainWin->get_status_bar().is_progress_stop()) break;
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
bool CtActions::_parse_node_content_iter(const CtTreeIter& tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, const std::string& pattern,
                             bool forward, bool first_fromsel, bool all_matches, bool first_node)
{
    bool restore_modified;
    Gtk::TextIter start_iter;
    bool pattern_found;

    Gtk::TextIter buff_start_iter = text_buffer->begin();
    if (buff_start_iter.get_char() != CtConst::CHAR_NEWLINE[0]) {
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
    if (_is_node_within_time_filter(tree_iter))
        pattern_found = _find_pattern(tree_iter, text_buffer, pattern, start_iter, forward, all_matches);
    else
        pattern_found = false;
    if (s_state.newline_trick) {
        buff_start_iter = text_buffer->begin();
        Gtk::TextIter buff_step_iter = buff_start_iter;
        if (buff_step_iter.forward_char()) text_buffer->erase(buff_start_iter, buff_step_iter);
        if (restore_modified) text_buffer->set_modified(false);
    }
    if (s_state.replace_active && pattern_found)
        _ctMainWin->update_window_save_needed("nbuf", tree_iter);
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
    std::time_t ts_cre = node_iter.get_node_creating_time();
    if (s_options.ts_cre_after.on && ts_cre < s_options.ts_cre_after.time)
        return false;
    if (s_options.ts_cre_before.on && ts_cre > s_options.ts_cre_before.time)
        return false;
    std::time_t ts_mod = node_iter.get_node_modification_time();
    if (s_options.ts_mod_after.on && ts_mod < s_options.ts_mod_after.time)
        return false;
    if (s_options.ts_mod_before.on && ts_mod > s_options.ts_mod_before.time)
        return false;
    return true;
}

// """Returns (start_iter, end_iter) or (None, None)"""
bool CtActions::_find_pattern(CtTreeIter tree_iter, Glib::RefPtr<Gtk::TextBuffer> text_buffer, std::string pattern,
                  Gtk::TextIter start_iter, bool forward, bool all_matches)
{
    /* Gtk::TextBuffer uses symbols positions
     * Glib::Regex uses byte positions
     */

    Glib::ustring text = text_buffer->get_text();
    if (!s_options.search_replace_dict_reg_exp) // NOT REGULAR EXPRESSION
    {
        pattern = Glib::Regex::escape_string(pattern); // backslashes all non alphanum chars => to not spoil re
        if (s_options.search_replace_dict_whole_word)  // WHOLE WORD
            pattern = "\\b" + pattern + "\\b";
        else if (s_options.search_replace_dict_start_word) // START WORD
            pattern = "\\b" + pattern;
    }
    Glib::RefPtr<Glib::Regex> re_pattern;
    if (s_options.search_replace_dict_match_case) // CASE SENSITIVE
        re_pattern = Glib::Regex::create(pattern, Glib::RegexCompileFlags::REGEX_MULTILINE);
    else
        re_pattern = Glib::Regex::create(pattern, Glib::RegexCompileFlags::REGEX_MULTILINE | Glib::RegexCompileFlags::REGEX_CASELESS);
    int start_offset = start_iter.get_offset();
    // # start_offset -= self.get_num_objs_before_offset(text_buffer, start_offset)
    std::array<int, 2> match_offsets = {-1, -1};
    if (forward) {
        Glib::MatchInfo match;
        if (re_pattern->match(text, str::symb_pos_to_byte_pos(text, start_offset), match))
            if (match.matches())
                match.fetch_pos(0, match_offsets[0], match_offsets[1]);
    } else {
        Glib::MatchInfo match;
        re_pattern->match(text, str::symb_pos_to_byte_pos(text, start_offset) /*as len*/, 0 /*as start position*/, match);
        while (match.matches()) {
            match.fetch_pos(0, match_offsets[0], match_offsets[1]);
            match.next();
        }
    }
    if (match_offsets[0] != -1) {
        match_offsets[0] = str::byte_pos_to_symb_pos(text, match_offsets[0]);
        match_offsets[1] = str::byte_pos_to_symb_pos(text, match_offsets[1]);
    }

    std::array<int,2> obj_match_offsets = {-1, -1};
    std::string obj_content;
    if (!s_state.replace_active) {
        obj_match_offsets = _check_pattern_in_object_between(text_buffer, re_pattern,
            start_iter.get_offset(), match_offsets[0], forward, obj_content);
    }
    if (obj_match_offsets[0] != -1) match_offsets = obj_match_offsets;
    if (match_offsets[0] == -1) return false;

    // match found!
    int num_objs = 0;
    if (obj_match_offsets[0] == -1)
        num_objs = _get_num_objs_before_offset(text_buffer, match_offsets[0]);
    int final_start_offset = match_offsets[0] + num_objs;
    int final_delta_offset = match_offsets[1] - match_offsets[0];
    // #print "IN", final_start_offset, final_delta_offset, self.dad.treestore[tree_iter][1]
    // #for count in range(final_delta_offset):
    // #    print count, text_buffer.get_iter_at_offset(final_start_offset+count).get_char()
    if (!_ctMainWin->curr_tree_iter() || _ctMainWin->curr_tree_iter().get_node_id() != tree_iter.get_node_id())
        _ctMainWin->get_tree_view().set_cursor_safe(tree_iter);
    _ctMainWin->get_text_view().set_selection_at_offset_n_delta(final_start_offset, final_delta_offset);
    // #print "OUT"
    auto mark_insert = text_buffer->get_insert();
    Gtk::TextIter iter_insert = text_buffer->get_iter_at_mark(mark_insert);
    if (all_matches) {
        int newline_trick_offset = s_state.newline_trick ? 1 : 0;
        gint64 node_id = tree_iter.get_node_id();
        int start_offset = match_offsets[0] + num_objs - newline_trick_offset;
        int end_offset = match_offsets[1] + num_objs - newline_trick_offset;
        std::string node_name = tree_iter.get_node_name();
        std::string node_hier_name = CtMiscUtil::get_node_hierarchical_name(tree_iter, " << ", false, false);
        std::string line_content = obj_match_offsets[0] != -1 ? obj_content : _get_line_content(text_buffer, iter_insert);
        int line_num = text_buffer->get_iter_at_offset(start_offset).get_line();
        if (!s_state.newline_trick) line_num += 1;
        s_state.match_store->add_row(node_id, node_name, str::xml_escape(node_hier_name), start_offset, end_offset, line_num, line_content);
        // #print line_num, self.matches_num
    } else {
        _ctMainWin->get_text_view().scroll_to(mark_insert, CtTextView::TEXT_SCROLL_MARGIN);
    }
    if (s_state.replace_active) {
        if (_ctMainWin->curr_tree_iter().get_node_read_only()) return false;
        std::string replacer_text = s_options.search_replace_dict_replace;
        text_buffer->delete_mark(text_buffer->get_selection_bound());
        text_buffer->insert_at_cursor(replacer_text);
        if (!all_matches)
            _ctMainWin->get_text_view().set_selection_at_offset_n_delta(match_offsets[0] + num_objs, replacer_text.size());
        // todo:
        //self.dad.state_machine.update_state();
        //self.dad.ctdb_handler.pending_edit_db_node_buff(self.dad.treestore[tree_iter][3], force_user_active=True)
    }
    return true;
}

//"""Search for the pattern in the given slice and direction"""
std::array<int, 2> CtActions::_check_pattern_in_object_between(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Glib::RefPtr<Glib::Regex> pattern,
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
    std::array<int, 2> sel_range = {start_offset, end_offset};
    if (!forward) std::swap(sel_range[0], sel_range[1]);
    /* todo:
    obj_vec = self.dad.state_machine.get_embedded_pixbufs_tables_codeboxes(text_buffer, sel_range=sel_range)
    if not obj_vec: return (None, None)
    if forward:
        for element in obj_vec:
            patt_in_obj = self.check_pattern_in_object(pattern, element)
            if patt_in_obj[0]:
                return (element[1][0], element[1][0]+1, patt_in_obj[1])
    else:
        for element in reversed(obj_vec):
            patt_in_obj = self.check_pattern_in_object(pattern, element)
            if patt_in_obj[0]:
                return (element[1][0], element[1][0]+1, patt_in_obj[1])
    */
    return {-1, -1};
}

//"""Returns the num of objects from buffer start to the given offset"""
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
    while (line_start.get_char() != CtConst::CHAR_NEWLINE[0])
        if (!line_start.backward_char())
            break;
    if (line_start.get_char() == CtConst::CHAR_NEWLINE[0])
        line_start.forward_char();
    while (line_end.get_char() != CtConst::CHAR_NEWLINE[0])
        if (!line_end.forward_char())
            break;
    return text_buffer->get_text(line_start, line_end);
}

// Iterated Find/Replace Dialog
void CtActions::_iterated_find_dialog()
{
    if (!s_state.iteratedfinddialog)
    {
        auto dialog = new Gtk::Dialog(_("Iterate Latest Find/Replace"), *_ctMainWin, Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
        auto button_close = dialog->add_button(_("Close"), 0);
        auto button_find_bw = dialog->add_button(_("Find Previous"), 4);
        auto button_find_fw = dialog->add_button(_("Find Next"), 1);
        auto button_replace = dialog->add_button(_("Replace"), 2);
        auto button_undo = dialog->add_button(_("Undo"), 3);
        dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
        button_close->set_image_from_icon_name(Gtk::Stock::CLOSE.id, Gtk::ICON_SIZE_BUTTON);
        button_find_bw->set_image_from_icon_name("find_back", Gtk::ICON_SIZE_BUTTON);
        button_find_fw->set_image_from_icon_name("find_again", Gtk::ICON_SIZE_BUTTON);
        button_replace->set_image_from_icon_name("find_replace", Gtk::ICON_SIZE_BUTTON);
        button_undo->set_image_from_icon_name(Gtk::Stock::UNDO.id, Gtk::ICON_SIZE_BUTTON);

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
           // todo:
           // self.dad.requested_step_back()
        });

        s_state.iteratedfinddialog = dialog;
    }
    s_state.iteratedfinddialog->show();
}

void CtActions::_update_all_matches_progress()
{
    double frac = double(s_state.processed_nodes)/double(s_state.counted_nodes);
    _ctMainWin->get_status_bar().progressBar.set_fraction(frac);
    if (s_state.matches_num != s_state.latest_matches) {
        s_state.latest_matches = s_state.matches_num;
        _ctMainWin->get_status_bar().progressBar.set_text(std::to_string(s_state.matches_num));
    }
}
