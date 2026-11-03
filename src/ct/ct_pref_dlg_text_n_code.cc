/*
 * ct_pref_dlg_text_n_code.cc
 *
 * Copyright 2009-2026
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

#include "ct_pref_dlg.h"
#include "ct_main_win.h"

Gtk::Widget* CtPrefDlg::build_tab_text_n_code()
{
    auto add_to_box = [](Gtk::Box* box, Gtk::Widget& child, bool expand = false, bool fill = false) {
#if GTKMM_MAJOR_VERSION >= 4
        box->append(child);
        child.set_hexpand(expand);
#else
        box->pack_start(child, expand, fill);
#endif
    };

    auto hbox_tab_width = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
#else
        Gtk::ORIENTATION_HORIZONTAL
#endif
        , 4/*spacing*/});
    auto label_tab_width = Gtk::manage(new Gtk::Label{_("Tab Width")});
    Glib::RefPtr<Gtk::Adjustment> adj_tab_width = Gtk::Adjustment::create(_pConfig->tabsWidth, 1, 10000);
    auto spinbutton_tab_width = Gtk::manage(new Gtk::SpinButton{adj_tab_width});
    spinbutton_tab_width->set_value(_pConfig->tabsWidth);
    add_to_box(hbox_tab_width, *label_tab_width);
    add_to_box(hbox_tab_width, *spinbutton_tab_width);

    auto checkbutton_spaces_tabs = Gtk::manage(new Gtk::CheckButton{_("Insert Spaces Instead of Tabs")});
    checkbutton_spaces_tabs->set_active(_pConfig->spacesInsteadTabs);
    auto checkbutton_line_wrap = Gtk::manage(new Gtk::CheckButton{_("Use Line Wrapping")});
    checkbutton_line_wrap->set_active(_pConfig->lineWrapping);
    auto hbox_wrapping_indent = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
#else
        Gtk::ORIENTATION_HORIZONTAL
#endif
        , 4/*spacing*/});
    auto label_wrapping_indent = Gtk::manage(new Gtk::Label{_("Line Wrapping Indentation")});
    gtk_label_set_xalign(label_wrapping_indent->gobj(), 0.0);
    Glib::RefPtr<Gtk::Adjustment> adj_wrapping_indent = Gtk::Adjustment::create(_pConfig->wrappingIndent, -10000, 10000, 1);
    auto spinbutton_wrapping_indent = Gtk::manage(new Gtk::SpinButton{adj_wrapping_indent});
    spinbutton_wrapping_indent->set_value(_pConfig->wrappingIndent);
    add_to_box(hbox_wrapping_indent, *label_wrapping_indent);
    add_to_box(hbox_wrapping_indent, *spinbutton_wrapping_indent);
    auto checkbutton_auto_indent = Gtk::manage(new Gtk::CheckButton{_("Enable Automatic Indentation")});
    checkbutton_auto_indent->set_active(_pConfig->autoIndent);
    auto checkbutton_line_nums = Gtk::manage(new Gtk::CheckButton{_("Show Line Numbers")});
    checkbutton_line_nums->set_active(_pConfig->showLineNumbers);
    auto checkbutton_scroll_last_line = Gtk::manage(new Gtk::CheckButton{_("Scroll Beyond Last Line")});
    checkbutton_scroll_last_line->set_active(_pConfig->scrollBeyondLastLine);

#if GTKMM_MAJOR_VERSION >= 4
    auto hbox_cursor_blink = Gtk::manage(new Gtk::Box{Gtk::Orientation::HORIZONTAL, 6/*spacing*/});
    auto label_cursor_blink = Gtk::manage(new Gtk::Label{_("Cursor Blinking")});
    auto radiobutton_cursor_blink_default = Gtk::manage(new Gtk::CheckButton{_("System Default")});
    auto radiobutton_cursor_blink_on = Gtk::manage(new Gtk::CheckButton{_("On")});
    radiobutton_cursor_blink_on->set_group(*radiobutton_cursor_blink_default);
    auto radiobutton_cursor_blink_off = Gtk::manage(new Gtk::CheckButton{_("Off")});
    radiobutton_cursor_blink_off->set_group(*radiobutton_cursor_blink_default);
    hbox_cursor_blink->append(*label_cursor_blink);
    hbox_cursor_blink->append(*radiobutton_cursor_blink_default);
    hbox_cursor_blink->append(*radiobutton_cursor_blink_on);
    hbox_cursor_blink->append(*radiobutton_cursor_blink_off);
#else
    auto hbox_cursor_blink = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 6/*spacing*/});
    auto label_cursor_blink = Gtk::manage(new Gtk::Label{_("Cursor Blinking")});
    auto radiobutton_cursor_blink_default = Gtk::manage(new Gtk::RadioButton{_("System Default")});
    auto radiobutton_cursor_blink_on = Gtk::manage(new Gtk::RadioButton{_("On")});
    radiobutton_cursor_blink_on->join_group(*radiobutton_cursor_blink_default);
    auto radiobutton_cursor_blink_off = Gtk::manage(new Gtk::RadioButton{_("Off")});
    radiobutton_cursor_blink_off->join_group(*radiobutton_cursor_blink_default);
    hbox_cursor_blink->pack_start(*label_cursor_blink, false, false);
    hbox_cursor_blink->pack_start(*radiobutton_cursor_blink_default, false, false);
    hbox_cursor_blink->pack_start(*radiobutton_cursor_blink_on, false, false);
    hbox_cursor_blink->pack_start(*radiobutton_cursor_blink_off, false, false);
#endif
    radiobutton_cursor_blink_default->set_active(2 == _pConfig->cursorBlink);
    radiobutton_cursor_blink_on->set_active(1 == _pConfig->cursorBlink);
    radiobutton_cursor_blink_off->set_active(0 ==_pConfig->cursorBlink);

    auto hbox_margins = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
#else
        Gtk::ORIENTATION_HORIZONTAL
#endif
        , 4/*spacing*/});
    auto label_margins_left = Gtk::manage(new Gtk::Label{_("Text Margin: Left")});
    auto label_margins_right = Gtk::manage(new Gtk::Label{_("Right")});
    Glib::RefPtr<Gtk::Adjustment> adj_margins_left = Gtk::Adjustment::create(_pConfig->textMarginLeft, 0, 1000);
    Glib::RefPtr<Gtk::Adjustment> adj_margins_right = Gtk::Adjustment::create(_pConfig->textMarginRight, 0, 1000);
    auto spinbutton_margins_left = Gtk::manage(new Gtk::SpinButton{adj_margins_left});
    auto spinbutton_margins_right = Gtk::manage(new Gtk::SpinButton{adj_margins_right});
    add_to_box(hbox_margins, *label_margins_left);
    add_to_box(hbox_margins, *spinbutton_margins_left);
    add_to_box(hbox_margins, *label_margins_right);
    add_to_box(hbox_margins, *spinbutton_margins_right);

    auto hbox_space_around_lines = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
#else
        Gtk::ORIENTATION_HORIZONTAL
#endif
        , 4/*spacing*/});
    auto label_space_around_lines = Gtk::manage(new Gtk::Label{_("Vertical Space Around Lines")});
#if GTKMM_MAJOR_VERSION >= 4
    label_space_around_lines->set_halign(Gtk::Align::START);
#else
    label_space_around_lines->set_halign(Gtk::Align::ALIGN_START);
#endif
    gtk_label_set_xalign(label_space_around_lines->gobj(), 0.0);
    Glib::RefPtr<Gtk::Adjustment> adj_space_around_lines = Gtk::Adjustment::create(_pConfig->spaceAroundLines, -0, 255, 1);
    auto spinbutton_space_around_lines = Gtk::manage(new Gtk::SpinButton{adj_space_around_lines});
    spinbutton_space_around_lines->set_value(_pConfig->spaceAroundLines);
    add_to_box(hbox_space_around_lines, *label_space_around_lines);
    add_to_box(hbox_space_around_lines, *spinbutton_space_around_lines);
    auto hbox_relative_wrapped_space = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
#else
        Gtk::ORIENTATION_HORIZONTAL
#endif
        , 4/*spacing*/});
    auto label_relative_wrapped_space = Gtk::manage(new Gtk::Label{_("Vertical Space in Wrapped Lines")});
    Glib::RefPtr<Gtk::Adjustment> adj_relative_wrapped_space = Gtk::Adjustment::create(_pConfig->relativeWrappedSpace, -0, 100, 1);
    auto spinbutton_relative_wrapped_space = Gtk::manage(new Gtk::SpinButton{adj_relative_wrapped_space});
    spinbutton_relative_wrapped_space->set_value(_pConfig->relativeWrappedSpace);
    add_to_box(hbox_relative_wrapped_space, *label_relative_wrapped_space);
    add_to_box(hbox_relative_wrapped_space, *spinbutton_relative_wrapped_space);
    add_to_box(hbox_relative_wrapped_space, *Gtk::manage(new Gtk::Label{"%"}));

    auto size_group_1 = Gtk::SizeGroup::create(
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::SizeGroup::Mode::HORIZONTAL
#else
        Gtk::SizeGroupMode::SIZE_GROUP_HORIZONTAL
#endif
        );
    size_group_1->add_widget(*label_wrapping_indent);
    size_group_1->add_widget(*label_space_around_lines);
    size_group_1->add_widget(*label_relative_wrapped_space);

    auto vbox_text_editor = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
#else
        Gtk::ORIENTATION_VERTICAL
#endif
        , 0/*spacing*/});
    add_to_box(vbox_text_editor, *hbox_tab_width);
    add_to_box(vbox_text_editor, *checkbutton_spaces_tabs);
    add_to_box(vbox_text_editor, *checkbutton_line_wrap);
    add_to_box(vbox_text_editor, *hbox_wrapping_indent);
    add_to_box(vbox_text_editor, *checkbutton_auto_indent);
    add_to_box(vbox_text_editor, *checkbutton_line_nums);
    add_to_box(vbox_text_editor, *checkbutton_scroll_last_line);
    add_to_box(vbox_text_editor, *hbox_space_around_lines);
    add_to_box(vbox_text_editor, *hbox_relative_wrapped_space);
    add_to_box(vbox_text_editor, *hbox_cursor_blink);
    add_to_box(vbox_text_editor, *hbox_margins);
    Gtk::Frame* frame_text_editor = new_managed_frame_with_align(_("Text Editor"), vbox_text_editor);

        auto hbox_timestamp = Gtk::manage(new Gtk::Box{
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
    #else
        Gtk::ORIENTATION_HORIZONTAL
    #endif
        , 4/*spacing*/});
    auto label_timestamp = Gtk::manage(new Gtk::Label{_("Timestamp Format")});
    gtk_label_set_xalign(label_timestamp->gobj(), 0.0);
    auto entry_timestamp_format = Gtk::manage(new Gtk::Entry{});
    entry_timestamp_format->set_text(_pConfig->timestampFormat);
    auto button_strftime_help = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_strftime_help->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_help", Gtk::ICON_SIZE_BUTTON));
#else
    button_strftime_help->set_icon_name("ct_help");
#endif
    button_strftime_help->set_tooltip_text(_("Online Manual"));
    auto button_reset_timestamp_format = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_timestamp_format->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#else
    button_reset_timestamp_format->set_icon_name("ct_undo");
#endif
    button_reset_timestamp_format->set_tooltip_text(_("Reset to Default"));
    
    add_to_box(hbox_timestamp, *label_timestamp);
    add_to_box(hbox_timestamp, *entry_timestamp_format, true, true);
    add_to_box(hbox_timestamp, *button_strftime_help);
    add_to_box(hbox_timestamp, *button_reset_timestamp_format);
    auto hbox_horizontal_rule = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
#else
        Gtk::ORIENTATION_HORIZONTAL
#endif
        , 4/*spacing*/});
    auto label_horizontal_rule = Gtk::manage(new Gtk::Label{_("Horizontal Rule")});
    gtk_label_set_xalign(label_horizontal_rule->gobj(), 0.0);
    auto entry_horizontal_rule = Gtk::manage(new Gtk::Entry{});
    entry_horizontal_rule->set_text(_pConfig->hRule);
    auto button_reset_horizontal_rule = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_horizontal_rule->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#else
    button_reset_horizontal_rule->set_icon_name("ct_undo");
#endif
    button_reset_horizontal_rule->set_tooltip_text(_("Reset to Default"));
    
    add_to_box(hbox_horizontal_rule, *label_horizontal_rule);
    add_to_box(hbox_horizontal_rule, *entry_horizontal_rule, true, true);
    add_to_box(hbox_horizontal_rule, *button_reset_horizontal_rule);

        auto size_group_2 = Gtk::SizeGroup::create(
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::SizeGroup::Mode::HORIZONTAL
    #else
        Gtk::SizeGroupMode::SIZE_GROUP_HORIZONTAL
    #endif
        );
    size_group_2->add_widget(*label_timestamp);
    size_group_2->add_widget(*label_horizontal_rule);

        auto hbox_selword_chars = Gtk::manage(new Gtk::Box{
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::HORIZONTAL
    #else
        Gtk::ORIENTATION_HORIZONTAL
    #endif
        , 4/*spacing*/});
    auto label_selword_chars = Gtk::manage(new Gtk::Label{_("Chars to Select at Double Click")});
    auto entry_selword_chars = Gtk::manage(new Gtk::Entry{});
    entry_selword_chars->set_text(_pConfig->selwordChars.item());
    auto button_reset_selword_chars = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_selword_chars->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#else
    button_reset_selword_chars->set_icon_name("ct_undo");
#endif
    button_reset_selword_chars->set_tooltip_text(_("Reset to Default"));
    add_to_box(hbox_selword_chars, *label_selword_chars);
    add_to_box(hbox_selword_chars, *entry_selword_chars, true, true);
    add_to_box(hbox_selword_chars, *button_reset_selword_chars);

    auto vbox_misc_all = Gtk::manage(new Gtk::Box{
#if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
#else
        Gtk::ORIENTATION_VERTICAL
#endif
        , 2/*spacing*/});
    add_to_box(vbox_misc_all, *hbox_timestamp, true, true);
    add_to_box(vbox_misc_all, *hbox_horizontal_rule, true, true);
    add_to_box(vbox_misc_all, *hbox_selword_chars, true, true);
    Gtk::Frame* frame_misc_all = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_all);

        auto pMainBox = Gtk::manage(new Gtk::Box{
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
    #else
        Gtk::ORIENTATION_VERTICAL
    #endif
        , 3/*spacing*/});
    pMainBox->set_margin_start(6);
    pMainBox->set_margin_top(6);
    add_to_box(pMainBox, *frame_text_editor);
    add_to_box(pMainBox, *frame_misc_all);

    spinbutton_tab_width->signal_value_changed().connect([this, spinbutton_tab_width](){
        _pConfig->tabsWidth = spinbutton_tab_width->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) {
            gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(win->get_text_view().gobj()), (guint)win->get_ct_config()->tabsWidth);
        });
    });
    spinbutton_margins_left->signal_value_changed().connect([this, spinbutton_margins_left](){
        _pConfig->textMarginLeft = spinbutton_margins_left->get_value_as_int();
        need_restart(RESTART_REASON::TEXT_MARGIN);
    });
    spinbutton_margins_right->signal_value_changed().connect([this, spinbutton_margins_right](){
        _pConfig->textMarginRight = spinbutton_margins_right->get_value_as_int();
        need_restart(RESTART_REASON::TEXT_MARGIN);
    });
    spinbutton_wrapping_indent->signal_value_changed().connect([this, spinbutton_wrapping_indent](){
        _pConfig->wrappingIndent = spinbutton_wrapping_indent->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) { win->get_text_view().mm().set_indent(win->get_ct_config()->wrappingIndent); });
    });
    spinbutton_relative_wrapped_space->signal_value_changed().connect([this, spinbutton_relative_wrapped_space](){
       _pConfig->relativeWrappedSpace = spinbutton_relative_wrapped_space->get_value_as_int();
       apply_for_each_window([](CtMainWin* win) { win->get_text_view().set_pixels_inside_wrap(win->get_ct_config()->spaceAroundLines, win->get_ct_config()->relativeWrappedSpace);});
    });
    spinbutton_space_around_lines->signal_value_changed().connect([this, spinbutton_space_around_lines](){
        _pConfig->spaceAroundLines = spinbutton_space_around_lines->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) {
            win->get_text_view().mm().set_pixels_above_lines(win->get_ct_config()->spaceAroundLines);
            win->get_text_view().mm().set_pixels_below_lines(win->get_ct_config()->spaceAroundLines);
            win->get_text_view().set_pixels_inside_wrap(win->get_ct_config()->spaceAroundLines, win->get_ct_config()->relativeWrappedSpace);
        });
    });
    checkbutton_spaces_tabs->signal_toggled().connect([this, checkbutton_spaces_tabs](){
        _pConfig->spacesInsteadTabs = checkbutton_spaces_tabs->get_active();
        apply_for_each_window([](CtMainWin* win) {
            gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(win->get_text_view().gobj()), win->get_ct_config()->spacesInsteadTabs);
        });
    });
    checkbutton_line_wrap->signal_toggled().connect([this, checkbutton_line_wrap](){
        _pConfig->lineWrapping = checkbutton_line_wrap->get_active();
        apply_for_each_window([](CtMainWin* win) {
#if GTKMM_MAJOR_VERSION >= 4
            win->get_text_view().mm().set_wrap_mode(win->get_ct_config()->lineWrapping ? Gtk::WrapMode::WORD_CHAR : Gtk::WrapMode::NONE);
#else
            win->get_text_view().mm().set_wrap_mode(win->get_ct_config()->lineWrapping ? Gtk::WrapMode::WRAP_WORD_CHAR : Gtk::WrapMode::WRAP_NONE);
#endif
        });
    });
    checkbutton_auto_indent->signal_toggled().connect([this, checkbutton_auto_indent](){
        _pConfig->autoIndent = checkbutton_auto_indent->get_active();
    });
    checkbutton_line_nums->signal_toggled().connect([this, checkbutton_line_nums](){
        _pConfig->showLineNumbers = checkbutton_line_nums->get_active();
        apply_for_each_window([](CtMainWin* win) {
            gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(win->get_text_view().gobj()), win->get_ct_config()->showLineNumbers);
        });
    });
    checkbutton_scroll_last_line->signal_toggled().connect([this, checkbutton_scroll_last_line](){
        _pConfig->scrollBeyondLastLine = checkbutton_scroll_last_line->get_active();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    entry_timestamp_format->signal_changed().connect([this, entry_timestamp_format](){
        _pConfig->timestampFormat = entry_timestamp_format->get_text();
    });
    button_strftime_help->signal_clicked().connect([](){
        fs::open_weblink("https://linux.die.net/man/3/strftime");
    });
    button_reset_timestamp_format->signal_clicked().connect([this, entry_timestamp_format](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            entry_timestamp_format->set_text(CtConst::TIMESTAMP_FORMAT_DEFAULT);
        }
    });
    entry_horizontal_rule->signal_changed().connect([this, entry_horizontal_rule](){
        _pConfig->hRule = entry_horizontal_rule->get_text();
    });
    button_reset_horizontal_rule->signal_clicked().connect([this, entry_horizontal_rule](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            entry_horizontal_rule->set_text(CtConst::HORIZONTAL_RULE_DEFAULT);
        }
    });
    entry_selword_chars->signal_changed().connect([this, entry_selword_chars](){
        _pConfig->selwordChars = entry_selword_chars->get_text();
    });
    button_reset_selword_chars->signal_clicked().connect([this, entry_selword_chars](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            entry_selword_chars->set_text(CtConst::SELWORD_CHARS_DEFAULT);
        }
    });

    radiobutton_cursor_blink_default->signal_toggled().connect([this, radiobutton_cursor_blink_default](){
        if (!radiobutton_cursor_blink_default->get_active()) return;
        _pConfig->cursorBlink = 2;
        need_restart(RESTART_REASON::CURSOR_BLINK);
    });
    radiobutton_cursor_blink_on->signal_toggled().connect([this, radiobutton_cursor_blink_on](){
        if (!radiobutton_cursor_blink_on->get_active()) return;
        _pConfig->cursorBlink = 1;
        Gtk::Settings::get_default()->property_gtk_cursor_blink() = true;
    });
    radiobutton_cursor_blink_off->signal_toggled().connect([this, radiobutton_cursor_blink_off](){
        if (!radiobutton_cursor_blink_off->get_active()) return;
        _pConfig->cursorBlink = 0;
        Gtk::Settings::get_default()->property_gtk_cursor_blink() = false;
    });

    return pMainBox;
}
