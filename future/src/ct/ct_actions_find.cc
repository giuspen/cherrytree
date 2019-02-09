#include "ct_actions.h"
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include "ct_image.h"
#include "ct_app.h"
#include "ct_dialogs.h"
#include "ct_doc_rw.h"

std::string dialog_search(Gtk::Window& parent, const std::string& title, bool replace_on, bool multiple_nodes, bool pattern_required);

struct SearchOptions {
    struct time_search {
        std::time_t time;
        bool        on;
    };

    time_search ts_cre_after;
    time_search ts_cre_before;
    time_search ts_mod_after;
    time_search ts_mod_before;
    std::string search_replace_dict_find;
    std::string search_replace_dict_replace;
    bool        search_replace_dict_fw;
    int         search_replace_dict_a_ff_fa;
    bool        search_replace_dict_match_case;
    bool        search_replace_dict_reg_exp;
    bool        search_replace_dict_whole_word;
    bool        search_replace_dict_start_word;
    bool        search_replace_dict_idialog;
} s_options;

struct SearchState {
    bool        from_find_iterated;
    bool        replace_active;
    std::string curr_find_where;
    std::string curr_find_pattern;
    bool        from_find_back;
    int         matches_num;
    bool        user_active;

} s_state;

//"""Search for a pattern in the selected Node"""
void CtActions::find_in_selected_node()
{
    if (!_is_there_selected_node_or_error()) return;
    Glib::RefPtr<Gtk::TextBuffer> curr_buffer = _ctMainWin->get_text_view().get_buffer();

    std::string entry_hint;
    std::string pattern;
    if (s_state.from_find_iterated == false) {
        //self.latest_node_offset = {}
        auto iter_insert = curr_buffer->get_iter_at_mark(curr_buffer->get_insert());
        auto iter_bound = curr_buffer->get_iter_at_mark(curr_buffer->get_selection_bound());
        auto entry_predefined_text = curr_buffer->get_text(iter_insert, iter_bound);
        if (entry_predefined_text.length())
            s_options.search_replace_dict_find = entry_predefined_text;
        std::string title = s_state.replace_active ? _("Replace in Current Node...") : _("Search in Current Node...");
        pattern = dialog_search(*_ctMainWin, title, s_state.replace_active, false, true);
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
    /*
    if (all_matches) {
        find.allmatches_liststore.clear()
        find.all_matches_first_in_node = true
        while self.parse_node_content_iter(self.dad.curr_tree_iter, self.dad.curr_buffer, pattern, forward, first_fromsel, all_matches, True):
            self.matches_num += 1
    elif self.parse_node_content_iter(self.dad.curr_tree_iter, self.dad.curr_buffer, pattern, forward, first_fromsel, all_matches, True):
        self.matches_num = 1
    if self.matches_num == 0:
        support.dialog_info(_("The pattern '%s' was not found") % pattern, self.dad.window)
    elif all_matches:
        self.allmatches_title = str(self.matches_num) + cons.CHAR_SPACE + _("Matches")
        self.allmatchesdialog_show()
    elif self.search_replace_dict['idialog']:
        self.iterated_find_dialog()
    if user_active_restore: self.dad.user_active = True
    */
}

void CtActions::find_in_all_nodes()
{

}

void CtActions::find_in_sel_node_and_subnodes()
{

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

//"""Dialog to select a Date"""
std::time_t dialog_date_select(Gtk::Window& parent, const std::string& title, const std::time_t& curr_time)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);

    std::tm struct_time = *std::localtime(&curr_time);

    auto content_area = dialog.get_content_area();
    auto calendar = Gtk::Calendar();
    calendar.select_month(struct_time.tm_mon-1, struct_time.tm_year); // month 0-11
    calendar.select_day(struct_time.tm_mday); // day 1-31
    auto adj_h = Gtk::Adjustment::create(struct_time.tm_hour, 0, 23, 1);
    auto spinbutton_h = Gtk::SpinButton(adj_h);
    spinbutton_h.set_value(struct_time.tm_hour);
    auto adj_m = Gtk::Adjustment::create(struct_time.tm_min, 0, 59, 1);
    auto spinbutton_m = Gtk::SpinButton(adj_m);
    spinbutton_m.set_value(struct_time.tm_min);
    auto hbox = Gtk::HBox();
    hbox.pack_start(spinbutton_h);
    hbox.pack_start(spinbutton_m);
    content_area->pack_start(calendar);
    content_area->pack_start(hbox);

    dialog.signal_button_press_event().connect([&dialog](GdkEventButton* event)->bool {
        if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
            return false;
        dialog.response(Gtk::RESPONSE_ACCEPT);
        return true;
    });
    content_area->show_all();

    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return 0;

    guint new_year, new_month, new_day;
    calendar.get_date(new_year, new_month, new_day);

    std::tm tmtime = {0};
    tmtime.tm_year = new_year;
    tmtime.tm_mon = new_month + 1;
    tmtime.tm_mday = new_day;
    tmtime.tm_hour = spinbutton_h.get_value_as_int();
    tmtime.tm_min = spinbutton_m.get_value_as_int();

    std::time_t new_time = std::mktime(&tmtime);
    return new_time;
}

//    """Opens the Search Dialog"""
std::string dialog_search(Gtk::Window& parent, const std::string& title, bool replace_on, bool multiple_nodes, bool pattern_required)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
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

        auto on_ts_node_button_clicked = [&dialog, &ts_format](Gtk::Button* button, const char* title, std::time_t& ts_value) {
            std::time_t new_time = dialog_date_select(dialog, title, ts_value);
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
