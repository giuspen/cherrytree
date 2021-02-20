/*
 * ct_pref_dlg_text_n_code.cc
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

#include "ct_pref_dlg.h"
#include "ct_main_win.h"

Gtk::Widget* CtPrefDlg::build_tab_text_n_code()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::HBox* hbox_tab_width = Gtk::manage(new Gtk::HBox());
    hbox_tab_width->set_spacing(4);
    Gtk::Label* label_tab_width = Gtk::manage(new Gtk::Label(_("Tab Width")));
    Glib::RefPtr<Gtk::Adjustment> adj_tab_width = Gtk::Adjustment::create(pConfig->tabsWidth, 1, 10000);
    Gtk::SpinButton* spinbutton_tab_width = Gtk::manage(new Gtk::SpinButton(adj_tab_width));
    spinbutton_tab_width->set_value(pConfig->tabsWidth);
    hbox_tab_width->pack_start(*label_tab_width, false, false);
    hbox_tab_width->pack_start(*spinbutton_tab_width, false, false);
    Gtk::CheckButton* checkbutton_spaces_tabs = Gtk::manage(new Gtk::CheckButton(_("Insert Spaces Instead of Tabs")));
    checkbutton_spaces_tabs->set_active(pConfig->spacesInsteadTabs);
    Gtk::CheckButton* checkbutton_line_wrap = Gtk::manage(new Gtk::CheckButton(_("Use Line Wrapping")));
    checkbutton_line_wrap->set_active(pConfig->lineWrapping);
    Gtk::HBox* hbox_wrapping_indent = Gtk::manage(new Gtk::HBox());
    hbox_wrapping_indent->set_spacing(4);
    Gtk::Label* label_wrapping_indent = Gtk::manage(new Gtk::Label(_("Line Wrapping Indentation")));
    gtk_label_set_xalign(label_wrapping_indent->gobj(), 0.0);
    Glib::RefPtr<Gtk::Adjustment> adj_wrapping_indent = Gtk::Adjustment::create(pConfig->wrappingIndent, -10000, 10000, 1);
    Gtk::SpinButton* spinbutton_wrapping_indent = Gtk::manage(new Gtk::SpinButton(adj_wrapping_indent));
    spinbutton_wrapping_indent->set_value(pConfig->wrappingIndent);
    hbox_wrapping_indent->pack_start(*label_wrapping_indent, false, false);
    hbox_wrapping_indent->pack_start(*spinbutton_wrapping_indent, false, false);
    Gtk::CheckButton* checkbutton_auto_indent = Gtk::manage(new Gtk::CheckButton(_("Enable Automatic Indentation")));
    checkbutton_auto_indent->set_active(pConfig->autoIndent);
    Gtk::CheckButton* checkbutton_line_nums = Gtk::manage(new Gtk::CheckButton(_("Show Line Numbers")));
    checkbutton_line_nums->set_active(pConfig->showLineNumbers);
    Gtk::CheckButton* checkbutton_scroll_last_line = Gtk::manage(new Gtk::CheckButton(_("Scroll Beyond Last Line")));
    checkbutton_scroll_last_line->set_active(pConfig->scrollBeyondLastLine);
    Gtk::HBox* hbox_space_around_lines = Gtk::manage(new Gtk::HBox());
    hbox_space_around_lines->set_spacing(4);
    Gtk::Label* label_space_around_lines = Gtk::manage(new Gtk::Label(_("Vertical Space Around Lines")));
    label_space_around_lines->set_halign(Gtk::Align::ALIGN_START);
    gtk_label_set_xalign(label_space_around_lines->gobj(), 0.0);
    Glib::RefPtr<Gtk::Adjustment> adj_space_around_lines = Gtk::Adjustment::create(pConfig->spaceAroundLines, -0, 255, 1);
    Gtk::SpinButton* spinbutton_space_around_lines = Gtk::manage(new Gtk::SpinButton(adj_space_around_lines));
    spinbutton_space_around_lines->set_value(pConfig->spaceAroundLines);
    hbox_space_around_lines->pack_start(*label_space_around_lines, false, false);
    hbox_space_around_lines->pack_start(*spinbutton_space_around_lines, false, false);
    Gtk::HBox* hbox_relative_wrapped_space = Gtk::manage(new Gtk::HBox());
    hbox_relative_wrapped_space->set_spacing(4);
    Gtk::Label* label_relative_wrapped_space = Gtk::manage(new Gtk::Label(_("Vertical Space in Wrapped Lines")));
    Glib::RefPtr<Gtk::Adjustment> adj_relative_wrapped_space = Gtk::Adjustment::create(pConfig->relativeWrappedSpace, -0, 100, 1);
    Gtk::SpinButton* spinbutton_relative_wrapped_space = Gtk::manage(new Gtk::SpinButton(adj_relative_wrapped_space));
    spinbutton_relative_wrapped_space->set_value(pConfig->relativeWrappedSpace);
    hbox_relative_wrapped_space->pack_start(*label_relative_wrapped_space, false, false);
    hbox_relative_wrapped_space->pack_start(*spinbutton_relative_wrapped_space, false, false);
    hbox_relative_wrapped_space->pack_start(*Gtk::manage(new Gtk::Label("%")), false, false);

    auto size_group_1 = Gtk::SizeGroup::create(Gtk::SizeGroupMode::SIZE_GROUP_HORIZONTAL);
    size_group_1->add_widget(*label_wrapping_indent);
    size_group_1->add_widget(*label_space_around_lines);
    size_group_1->add_widget(*label_relative_wrapped_space);

    Gtk::VBox* vbox_text_editor = Gtk::manage(new Gtk::VBox());
    vbox_text_editor->pack_start(*hbox_tab_width, false, false);
    vbox_text_editor->pack_start(*checkbutton_spaces_tabs, false, false);
    vbox_text_editor->pack_start(*checkbutton_line_wrap, false, false);
    vbox_text_editor->pack_start(*hbox_wrapping_indent, false, false);
    vbox_text_editor->pack_start(*checkbutton_auto_indent, false, false);
    vbox_text_editor->pack_start(*checkbutton_line_nums, false, false);
    vbox_text_editor->pack_start(*checkbutton_scroll_last_line, false, false);
    vbox_text_editor->pack_start(*hbox_space_around_lines, false, false);
    vbox_text_editor->pack_start(*hbox_relative_wrapped_space, false, false);
    Gtk::Frame* frame_text_editor = new_managed_frame_with_align(_("Text Editor"), vbox_text_editor);

    Gtk::HBox* hbox_timestamp = Gtk::manage(new Gtk::HBox());
    hbox_timestamp->set_spacing(4);
    Gtk::Label* label_timestamp = Gtk::manage(new Gtk::Label(_("Timestamp Format")));
    gtk_label_set_xalign(label_space_around_lines->gobj(), 0.0);
    Gtk::Entry* entry_timestamp_format = Gtk::manage(new Gtk::Entry());
    entry_timestamp_format->set_text(pConfig->timestampFormat);
    Gtk::Button* button_strftime_help = Gtk::manage(new Gtk::Button());
    button_strftime_help->set_tooltip_text(_("Online _Manual")); //added tooltip for button 
    button_strftime_help->set_image(*_pCtMainWin->new_image_from_stock("ct_help", Gtk::ICON_SIZE_BUTTON));
    hbox_timestamp->pack_start(*label_timestamp, false, false);
    hbox_timestamp->pack_start(*entry_timestamp_format, false, false);
    hbox_timestamp->pack_start(*button_strftime_help, false, false);
    Gtk::HBox* hbox_horizontal_rule = Gtk::manage(new Gtk::HBox());
    hbox_horizontal_rule->set_spacing(4);
    Gtk::Label* label_horizontal_rule = Gtk::manage(new Gtk::Label(_("Horizontal Rule")));
    gtk_label_set_xalign(label_horizontal_rule->gobj(), 0.0);
    Gtk::Entry* entry_horizontal_rule = Gtk::manage(new Gtk::Entry());
    entry_horizontal_rule->set_text(pConfig->hRule);
    hbox_horizontal_rule->pack_start(*label_horizontal_rule, false, false);
    hbox_horizontal_rule->pack_start(*entry_horizontal_rule);

    auto size_group_2 = Gtk::SizeGroup::create(Gtk::SizeGroupMode::SIZE_GROUP_HORIZONTAL);
    size_group_2->add_widget(*label_timestamp);
    size_group_2->add_widget(*label_horizontal_rule);

    Gtk::HBox* hbox_selword_chars = Gtk::manage(new Gtk::HBox());
    hbox_selword_chars->set_spacing(4);
    Gtk::Label* label_selword_chars = Gtk::manage(new Gtk::Label(_("Chars to Select at Double Click")));
    Gtk::Entry* entry_selword_chars = Gtk::manage(new Gtk::Entry());
    entry_selword_chars->set_text(pConfig->selwordChars.item());
    hbox_selword_chars->pack_start(*label_selword_chars, false, false);
    hbox_selword_chars->pack_start(*entry_selword_chars);

    Gtk::VBox* vbox_misc_all = Gtk::manage(new Gtk::VBox());
    vbox_misc_all->set_spacing(2);
    vbox_misc_all->pack_start(*hbox_timestamp);
    vbox_misc_all->pack_start(*hbox_horizontal_rule);
    vbox_misc_all->pack_start(*hbox_selword_chars);
    Gtk::Frame* frame_misc_all = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_all);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_text_editor, false, false);
    pMainBox->pack_start(*frame_misc_all, false, false);

    spinbutton_tab_width->signal_value_changed().connect([this, pConfig, spinbutton_tab_width](){
        pConfig->tabsWidth = spinbutton_tab_width->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) { win->get_text_view().set_tab_width((guint)win->get_ct_config()->tabsWidth); });
    });
    spinbutton_wrapping_indent->signal_value_changed().connect([this, pConfig, spinbutton_wrapping_indent](){
        pConfig->wrappingIndent = spinbutton_wrapping_indent->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) { win->get_text_view().set_indent(win->get_ct_config()->wrappingIndent); });
    });
    spinbutton_relative_wrapped_space->signal_value_changed().connect([this, pConfig, spinbutton_relative_wrapped_space](){
       pConfig->relativeWrappedSpace = spinbutton_relative_wrapped_space->get_value_as_int();
       apply_for_each_window([](CtMainWin* win) { win->get_text_view().set_pixels_inside_wrap(win->get_ct_config()->spaceAroundLines, win->get_ct_config()->relativeWrappedSpace);});
    });
    spinbutton_space_around_lines->signal_value_changed().connect([this, pConfig, spinbutton_space_around_lines](){
        pConfig->spaceAroundLines = spinbutton_space_around_lines->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) {
            win->get_text_view().set_pixels_above_lines(win->get_ct_config()->spaceAroundLines);
            win->get_text_view().set_pixels_below_lines(win->get_ct_config()->spaceAroundLines);
            win->get_text_view().set_pixels_inside_wrap(win->get_ct_config()->spaceAroundLines, win->get_ct_config()->relativeWrappedSpace);
        });
    });
    checkbutton_spaces_tabs->signal_toggled().connect([this, pConfig, checkbutton_spaces_tabs](){
        pConfig->spacesInsteadTabs = checkbutton_spaces_tabs->get_active();
        apply_for_each_window([](CtMainWin* win) { win->get_text_view().set_insert_spaces_instead_of_tabs(win->get_ct_config()->spacesInsteadTabs); });
    });
    checkbutton_line_wrap->signal_toggled().connect([this, pConfig, checkbutton_line_wrap](){
        pConfig->lineWrapping = checkbutton_line_wrap->get_active();
        apply_for_each_window([](CtMainWin* win) { win->get_text_view().set_wrap_mode(win->get_ct_config()->lineWrapping ? Gtk::WrapMode::WRAP_WORD_CHAR : Gtk::WrapMode::WRAP_NONE); });
    });
    checkbutton_auto_indent->signal_toggled().connect([pConfig, checkbutton_auto_indent](){
        pConfig->autoIndent = checkbutton_auto_indent->get_active();
    });
    checkbutton_line_nums->signal_toggled().connect([this, pConfig, checkbutton_line_nums](){
        pConfig->showLineNumbers = checkbutton_line_nums->get_active();
        apply_for_each_window([](CtMainWin* win) { win->get_text_view().set_show_line_numbers(win->get_ct_config()->showLineNumbers); });
    });
    checkbutton_scroll_last_line->signal_toggled().connect([this, pConfig, checkbutton_scroll_last_line](){
        pConfig->scrollBeyondLastLine = checkbutton_scroll_last_line->get_active();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    entry_timestamp_format->signal_changed().connect([pConfig, entry_timestamp_format](){
        pConfig->timestampFormat = entry_timestamp_format->get_text();
    });
    button_strftime_help->signal_clicked().connect([](){
        fs::open_weblink("https://linux.die.net/man/3/strftime");
    });
    entry_horizontal_rule->signal_changed().connect([pConfig, entry_horizontal_rule](){
        pConfig->hRule = entry_horizontal_rule->get_text();
    });
    entry_selword_chars->signal_changed().connect([pConfig, entry_selword_chars](){
        pConfig->selwordChars = entry_selword_chars->get_text();
    });

    return pMainBox;
}
