/*
 * ct_pref_dlg.cc
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
#include <gtkmm/notebook.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <gdkmm/rgba.h>
#include <glib/gi18n.h>
#include <algorithm>
#include <gdk/gdkkeysyms.h>
#include "ct_misc_utils.h"
#include "ct_image.h"
#include "ct_dialogs.h"
#include "ct_codebox.h"
#include "ct_main_win.h"
#include <gspell/gspell.h>

CtPrefDlg::CtPrefDlg(CtMainWin* parent)
 : Gtk::Dialog(_("Preferences"), *parent, true)
{
    _restartReasons = 0;
    _pCtMainWin = parent;
    _pCtMenu = &_pCtMainWin->get_ct_menu();

    Gtk::Notebook* pNotebook = Gtk::manage(new Gtk::Notebook());
    pNotebook->set_tab_pos(Gtk::PositionType::POS_LEFT);
    pNotebook->append_page(*build_tab_text_n_code(),        _("Text and Code"));
    pNotebook->append_page(*build_tab_rich_text(),          _("Rich Text"));
    pNotebook->append_page(*build_tab_plain_text_n_code(),  _("Plain Text and Code"));
    pNotebook->append_page(*build_tab_special_characters(), _("Special Characters"));
    pNotebook->append_page(*build_tab_tree(),               _("Tree"));
    pNotebook->append_page(*build_tab_theme(),              _("Theme"));
    pNotebook->append_page(*build_tab_fonts(),              _("Fonts"));
    pNotebook->append_page(*build_tab_links(),              _("Links"));
    pNotebook->append_page(*build_tab_toolbar(),            _("Toolbar"));
    pNotebook->append_page(*build_tab_kb_shortcuts(),       _("Keyboard Shortcuts"));
    pNotebook->append_page(*build_tab_misc(),               _("Miscellaneous"));

    get_content_area()->pack_start(*pNotebook);
    get_content_area()->show_all();

    add_button(Gtk::Stock::CLOSE, 1);
}

Gtk::Frame* CtPrefDlg::new_managed_frame_with_align(const Glib::ustring& frameLabel, Gtk::Widget* pFrameChild)
{
    auto pFrame = Gtk::manage(new Gtk::Frame{Glib::ustring{"<b>"}+frameLabel+"</b>"});
    dynamic_cast<Gtk::Label*>(pFrame->get_label_widget())->set_use_markup(true);
    pFrame->set_shadow_type(Gtk::SHADOW_NONE);
    auto pAlign = Gtk::manage(new Gtk::Alignment());
    pAlign->set_padding(3, 6, 6, 6);
    pAlign->add(*pFrameChild);
    pFrame->add(*pAlign);
    return pFrame;
}

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

Gtk::Widget* CtPrefDlg::build_tab_special_characters()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::HBox* hbox_special_chars = Gtk::manage(new Gtk::HBox());
    hbox_special_chars->set_spacing(4);
    Gtk::VBox* vbox_special_chars = Gtk::manage(new Gtk::VBox());
    Gtk::Label* label_special_chars = Gtk::manage(new Gtk::Label(_("Special Characters")));
    Gtk::HBox* hbox_reset = Gtk::manage(new Gtk::HBox());
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset->set_tooltip_text(_("Reset to Default"));
    hbox_reset->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    hbox_reset->pack_start(*button_reset, false, false);
    hbox_reset->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    vbox_special_chars->pack_start(*Gtk::manage(new Gtk::Label()), false, false);
    vbox_special_chars->pack_start(*label_special_chars, false, false);
    vbox_special_chars->pack_start(*hbox_reset, false, false);
    vbox_special_chars->pack_start(*Gtk::manage(new Gtk::Label()), false, false);
    Gtk::Frame* frame_special_chars = Gtk::manage(new Gtk::Frame());
    frame_special_chars->set_size_request(-1, 80);
    frame_special_chars->set_shadow_type(Gtk::SHADOW_IN);
    Gtk::ScrolledWindow* scrolledwindow_special_chars = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow_special_chars->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    frame_special_chars->add(*scrolledwindow_special_chars);
    Gtk::TextView* textview_special_chars = Gtk::manage(new Gtk::TextView());
    textview_special_chars->get_buffer()->set_text(pConfig->specialChars.item());
    textview_special_chars->set_wrap_mode(Gtk::WRAP_CHAR);
    scrolledwindow_special_chars->add(*textview_special_chars);
    hbox_special_chars->pack_start(*vbox_special_chars, false, false);
    hbox_special_chars->pack_start(*frame_special_chars);

    Gtk::HBox* hbox_bullist_chars = Gtk::manage(new Gtk::HBox());
    hbox_bullist_chars->set_spacing(4);
    Gtk::Label* label_bullist_chars = Gtk::manage(new Gtk::Label(_("Chars for Bulleted List")));
    Gtk::Entry* entry_bullist_chars = Gtk::manage(new Gtk::Entry());
    entry_bullist_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
    entry_bullist_chars->set_text(pConfig->charsListbul.item());
    hbox_bullist_chars->pack_start(*label_bullist_chars, false, false);
    hbox_bullist_chars->pack_start(*entry_bullist_chars);

    Gtk::HBox* hbox_todolist_chars = Gtk::manage(new Gtk::HBox());
    hbox_todolist_chars->set_spacing(4);
    Gtk::Label* label_todolist_chars = Gtk::manage(new Gtk::Label(_("Chars for Todo List")));
    Gtk::Entry* entry_todolist_chars = Gtk::manage(new Gtk::Entry());
    entry_todolist_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
    entry_todolist_chars->set_text(pConfig->charsTodo.item());
    hbox_todolist_chars->pack_start(*label_todolist_chars, false, false);
    hbox_todolist_chars->pack_start(*entry_todolist_chars);

    Gtk::HBox* hbox_toc_chars = Gtk::manage(new Gtk::HBox());
    hbox_toc_chars->set_spacing(4);
    Gtk::Label* label_toc_chars = Gtk::manage(new Gtk::Label(_("Chars for Table Of Content")));
    Gtk::Entry* entry_toc_chars = Gtk::manage(new Gtk::Entry());
    entry_toc_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
    entry_toc_chars->set_text(pConfig->charsToc.item());
    hbox_toc_chars->pack_start(*label_toc_chars, false, false);
    hbox_toc_chars->pack_start(*entry_toc_chars);

    Gtk::HBox* hbox_dquote_chars = Gtk::manage(new Gtk::HBox());
    hbox_dquote_chars->set_spacing(4);
    Gtk::Label* label_dquote_chars = Gtk::manage(new Gtk::Label(_("Chars for Smart Double Quotes")));
    Gtk::Entry* entry_dquote_chars = Gtk::manage(new Gtk::Entry());
    entry_dquote_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
    entry_dquote_chars->set_text(pConfig->chars_smart_dquote.item());
    hbox_dquote_chars->pack_start(*label_dquote_chars, false, false);
    hbox_dquote_chars->pack_start(*entry_dquote_chars);

    Gtk::HBox* hbox_squote_chars = Gtk::manage(new Gtk::HBox());
    hbox_squote_chars->set_spacing(4);
    Gtk::Label* label_squote_chars = Gtk::manage(new Gtk::Label(_("Chars for Smart Single Quotes")));
    Gtk::Entry* entry_squote_chars = Gtk::manage(new Gtk::Entry());
    entry_squote_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
    entry_squote_chars->set_text(pConfig->chars_smart_squote.item());
    hbox_squote_chars->pack_start(*label_squote_chars, false, false);
    hbox_squote_chars->pack_start(*entry_squote_chars);

    Gtk::VBox* vbox_editor = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_auto_smart_quotes = Gtk::manage(new Gtk::CheckButton(_("Enable Smart Quotes Auto Replacement")));
    Gtk::CheckButton* checkbutton_enable_symbol_autoreplace = Gtk::manage(new Gtk::CheckButton(_("Enable Symbol Auto Replacement")));
    checkbutton_auto_smart_quotes->set_active(pConfig->autoSmartQuotes);
    checkbutton_enable_symbol_autoreplace->set_active(pConfig->enableSymbolAutoreplace);

    vbox_editor->pack_start(*hbox_special_chars, false, false);
    vbox_editor->pack_start(*hbox_bullist_chars, false, false);
    vbox_editor->pack_start(*hbox_todolist_chars, false, false);
    vbox_editor->pack_start(*hbox_toc_chars, false, false);
    vbox_editor->pack_start(*hbox_dquote_chars, false, false);
    vbox_editor->pack_start(*hbox_squote_chars, false, false);
    vbox_editor->pack_start(*checkbutton_auto_smart_quotes, false, false);
    vbox_editor->pack_start(*checkbutton_enable_symbol_autoreplace, false, false);

    Gtk::Frame* frame_editor = new_managed_frame_with_align(_("Text Editor"), vbox_editor);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_editor, false, false);

    textview_special_chars->get_buffer()->signal_changed().connect([this, pConfig, textview_special_chars](){
        Glib::ustring new_special_chars = textview_special_chars->get_buffer()->get_text();
        new_special_chars = str::replace(new_special_chars, CtConst::CHAR_NEWLINE, "");
        if ((pConfig->specialChars.item()) != new_special_chars)
        {
            pConfig->specialChars = new_special_chars;
            apply_for_each_window([](CtMainWin* win) { win->menu_set_items_special_chars(); });
        }
    });
    button_reset->signal_clicked().connect([this, textview_special_chars](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            textview_special_chars->get_buffer()->set_text(CtConst::SPECIAL_CHARS_DEFAULT);
        }
    });

    entry_bullist_chars->signal_changed().connect([pConfig, entry_bullist_chars](){
        if (entry_bullist_chars->get_text().size() >= CtConst::CHARS_LISTBUL_DEFAULT.size()) {
            pConfig->charsListbul = entry_bullist_chars->get_text();
            entry_bullist_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
        else {
            entry_bullist_chars->set_icon_from_icon_name("ct_urgent", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
    });
    entry_bullist_chars->signal_icon_release().connect([entry_bullist_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
        entry_bullist_chars->set_text(CtConst::CHARS_LISTBUL_DEFAULT);
    });
    entry_todolist_chars->signal_changed().connect([pConfig, entry_todolist_chars](){
        if (entry_todolist_chars->get_text().size() == CtConst::CHARS_TODO_DEFAULT.size()) {
            pConfig->charsTodo = entry_todolist_chars->get_text();
            entry_todolist_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
        else {
            entry_todolist_chars->set_icon_from_icon_name("ct_urgent", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
    });
    entry_todolist_chars->signal_icon_release().connect([entry_todolist_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
        entry_todolist_chars->set_text(CtConst::CHARS_TODO_DEFAULT);
    });
    entry_toc_chars->signal_changed().connect([pConfig, entry_toc_chars](){
        if (entry_toc_chars->get_text().size() >= CtConst::CHARS_TOC_DEFAULT.size()) {
            pConfig->charsToc = entry_toc_chars->get_text();
            entry_toc_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
        else {
            entry_toc_chars->set_icon_from_icon_name("ct_urgent", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
    });
    entry_toc_chars->signal_icon_release().connect([entry_toc_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
        entry_toc_chars->set_text(CtConst::CHARS_TOC_DEFAULT);
    });
    entry_dquote_chars->signal_changed().connect([pConfig, entry_dquote_chars](){
        if (entry_dquote_chars->get_text().size() == CtConst::CHARS_SMART_DQUOTE_DEFAULT.size()) {
            pConfig->chars_smart_dquote = entry_dquote_chars->get_text();
            entry_dquote_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
        else {
            entry_dquote_chars->set_icon_from_icon_name("ct_urgent", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
    });
    entry_dquote_chars->signal_icon_release().connect([entry_dquote_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
        entry_dquote_chars->set_text(CtConst::CHARS_SMART_DQUOTE_DEFAULT);
    });
    entry_squote_chars->signal_changed().connect([pConfig, entry_squote_chars](){
        if (entry_squote_chars->get_text().size() == CtConst::CHARS_SMART_SQUOTE_DEFAULT.size()) {
            pConfig->chars_smart_squote = entry_squote_chars->get_text();
            entry_squote_chars->set_icon_from_icon_name("ct_undo", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
        else {
            entry_squote_chars->set_icon_from_icon_name("ct_urgent", Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY);
        }
    });
    entry_squote_chars->signal_icon_release().connect([entry_squote_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
        entry_squote_chars->set_text(CtConst::CHARS_SMART_SQUOTE_DEFAULT);
    });

    checkbutton_auto_smart_quotes->signal_toggled().connect([pConfig, checkbutton_auto_smart_quotes](){
        pConfig->autoSmartQuotes = checkbutton_auto_smart_quotes->get_active();
    });
    checkbutton_enable_symbol_autoreplace->signal_toggled().connect([pConfig, checkbutton_enable_symbol_autoreplace](){
        pConfig->enableSymbolAutoreplace = checkbutton_enable_symbol_autoreplace->get_active();
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_rich_text()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_spell_check = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_spell_check = Gtk::manage(new Gtk::CheckButton(_("Enable Spell Check")));
    checkbutton_spell_check->set_active(pConfig->enableSpellCheck);
    Gtk::HBox* hbox_spell_check_lang = Gtk::manage(new Gtk::HBox());
    hbox_spell_check_lang->set_spacing(4);
    Gtk::Label* label_spell_check_lang = Gtk::manage(new Gtk::Label(_("Spell Check Language")));
    Gtk::ComboBoxText* combobox_spell_check_lang = Gtk::manage(new Gtk::ComboBoxText());
    for (const GList* l = gspell_language_get_available(); l != NULL; l = l->next) {
        auto pGspellLang = reinterpret_cast<const GspellLanguage*>(l->data);
        combobox_spell_check_lang->append(gspell_language_get_code(pGspellLang), gspell_language_get_name(pGspellLang));
    }
    combobox_spell_check_lang->set_active_id(pConfig->spellCheckLang);
    combobox_spell_check_lang->set_sensitive(pConfig->enableSpellCheck);

    hbox_spell_check_lang->pack_start(*label_spell_check_lang, false, false);
    hbox_spell_check_lang->pack_start(*combobox_spell_check_lang);
    vbox_spell_check->pack_start(*checkbutton_spell_check, false, false);
    vbox_spell_check->pack_start(*hbox_spell_check_lang, false, false);
    Gtk::Frame* frame_spell_check = new_managed_frame_with_align(_("Spell Check"), vbox_spell_check);

    Gtk::HBox* hbox_misc_text = Gtk::manage(new Gtk::HBox());
    hbox_misc_text->set_spacing(4);
    Gtk::CheckButton* checkbutton_rt_show_white_spaces = Gtk::manage(new Gtk::CheckButton(_("Show White Spaces")));
    checkbutton_rt_show_white_spaces->set_active(pConfig->rtShowWhiteSpaces);
    Gtk::CheckButton* checkbutton_rt_highl_curr_line = Gtk::manage(new Gtk::CheckButton(_("Highlight Current Line")));
    checkbutton_rt_highl_curr_line->set_active(pConfig->rtHighlCurrLine);
    Gtk::CheckButton* checkbutton_rt_highl_match_bra = Gtk::manage(new Gtk::CheckButton(_("Highlight Matching Brackets")));
    checkbutton_rt_highl_match_bra->set_active(pConfig->rtHighlMatchBra);
    Gtk::CheckButton* checkbutton_codebox_auto_resize = Gtk::manage(new Gtk::CheckButton(_("Expand CodeBoxes Automatically")));
    checkbutton_codebox_auto_resize->set_active(pConfig->codeboxAutoResize);

    Gtk::HBox* hbox_embfile_icon_size = Gtk::manage(new Gtk::HBox());
    hbox_embfile_icon_size->set_spacing(4);
    Gtk::Label* label_embfile_icon_size = Gtk::manage(new Gtk::Label(_("Embedded File Icon Size")));
    Glib::RefPtr<Gtk::Adjustment> adj_embfile_icon_size = Gtk::Adjustment::create(pConfig->embfileIconSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_embfile_icon_size = Gtk::manage(new Gtk::SpinButton(adj_embfile_icon_size));
    spinbutton_embfile_icon_size->set_value(pConfig->embfileIconSize);
    hbox_embfile_icon_size->pack_start(*label_embfile_icon_size, false, false);
    hbox_embfile_icon_size->pack_start(*spinbutton_embfile_icon_size, false, false);

    Gtk::HBox* hbox_embfile_max_size = Gtk::manage(new Gtk::HBox());
    hbox_embfile_max_size->set_spacing(4);
    Gtk::Label* label_embfile_max_size = Gtk::manage(new Gtk::Label(_("Embedded File Size Limit")));
    Glib::RefPtr<Gtk::Adjustment> adj_embfile_max_size = Gtk::Adjustment::create(pConfig->embfileIconSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_embfile_max_size = Gtk::manage(new Gtk::SpinButton(adj_embfile_max_size));
    spinbutton_embfile_max_size->set_value(pConfig->embfileMaxSize);
    hbox_embfile_max_size->pack_start(*label_embfile_max_size, false, false);
    hbox_embfile_max_size->pack_start(*spinbutton_embfile_max_size, false, false);

    Gtk::CheckButton* checkbutton_embfile_show_filename = Gtk::manage(new Gtk::CheckButton(_("Show File Name on Top of Embedded File Icon")));
    checkbutton_embfile_show_filename->set_active(pConfig->embfileShowFileName);
    Gtk::Label* label_limit_undoable_steps = Gtk::manage(new Gtk::Label(_("Limit of Undoable Steps Per Node")));
    Glib::RefPtr<Gtk::Adjustment> adj_limit_undoable_steps = Gtk::Adjustment::create(pConfig->limitUndoableSteps, 1, 10000, 1);
    Gtk::SpinButton* spinbutton_limit_undoable_steps = Gtk::manage(new Gtk::SpinButton(adj_limit_undoable_steps));
    spinbutton_limit_undoable_steps->set_value(pConfig->limitUndoableSteps);
    hbox_misc_text->pack_start(*label_limit_undoable_steps, false, false);
    hbox_misc_text->pack_start(*spinbutton_limit_undoable_steps, false, false);
    Gtk::CheckButton* checkbutton_triple_click_sel_paragraph = Gtk::manage(new Gtk::CheckButton(_("At Triple Click Select the Whole Paragraph")));
    checkbutton_triple_click_sel_paragraph->set_active(pConfig->tripleClickParagraph);
#ifdef MD_AUTO_REPLACEMENT
    Gtk::CheckButton* checkbutton_md_formatting = Gtk::manage(new Gtk::CheckButton(_("Enable Markdown Auto Replacement (Experimental)")));
    checkbutton_md_formatting->set_active(pConfig->enableMdFormatting);
#endif // MD_AUTO_REPLACEMENT

    Gtk::VBox* vbox_misc_text = Gtk::manage(new Gtk::VBox());
    vbox_misc_text->pack_start(*checkbutton_rt_show_white_spaces, false, false);
    vbox_misc_text->pack_start(*checkbutton_rt_highl_curr_line, false, false);
    vbox_misc_text->pack_start(*checkbutton_rt_highl_match_bra, false, false);
    vbox_misc_text->pack_start(*checkbutton_codebox_auto_resize, false, false);
    vbox_misc_text->pack_start(*hbox_embfile_icon_size, false, false);
    vbox_misc_text->pack_start(*hbox_embfile_max_size, false, false);
    vbox_misc_text->pack_start(*checkbutton_embfile_show_filename, false, false);
    vbox_misc_text->pack_start(*hbox_misc_text, false, false);
    vbox_misc_text->pack_start(*checkbutton_triple_click_sel_paragraph, false, false);
#ifdef MD_AUTO_REPLACEMENT
    vbox_misc_text->pack_start(*checkbutton_md_formatting, false, false);
#endif // MD_AUTO_REPLACEMENT
    Gtk::Frame* frame_misc_text = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_text);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_spell_check, false, false);
    pMainBox->pack_start(*frame_misc_text, false, false);

    checkbutton_spell_check->signal_toggled().connect([this, pConfig, checkbutton_spell_check, combobox_spell_check_lang](){
        pConfig->enableSpellCheck = checkbutton_spell_check->get_active();
        combobox_spell_check_lang->set_sensitive(pConfig->enableSpellCheck);
        apply_for_each_window([](CtMainWin* win) {
            win->get_text_view().set_spell_check(win->curr_tree_iter().get_node_is_rich_text());
            win->update_selected_node_statusbar_info();
        });
    });
    combobox_spell_check_lang->signal_changed().connect([this, pConfig, combobox_spell_check_lang](){
        pConfig->spellCheckLang = combobox_spell_check_lang->get_active_id();
        apply_for_each_window([](CtMainWin* win) {
            win->get_text_view().set_spell_check(win->curr_tree_iter().get_node_is_rich_text());
            win->update_selected_node_statusbar_info();
        });
    });
    checkbutton_rt_show_white_spaces->signal_toggled().connect([this, pConfig, checkbutton_rt_show_white_spaces](){
        pConfig->rtShowWhiteSpaces = checkbutton_rt_show_white_spaces->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('r'/*RichText*/);
        });
    });
    checkbutton_rt_highl_curr_line->signal_toggled().connect([this, pConfig, checkbutton_rt_highl_curr_line](){
        pConfig->rtHighlCurrLine = checkbutton_rt_highl_curr_line->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('r'/*RichText*/);
        });
    });
    checkbutton_rt_highl_match_bra->signal_toggled().connect([this, pConfig, checkbutton_rt_highl_match_bra](){
        pConfig->rtHighlMatchBra = checkbutton_rt_highl_match_bra->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->reapply_syntax_highlighting('r'/*RichText*/);
            win->reapply_syntax_highlighting('t'/*Table*/);
        });
    });
    checkbutton_codebox_auto_resize->signal_toggled().connect([this, pConfig, checkbutton_codebox_auto_resize](){
        pConfig->codeboxAutoResize = checkbutton_codebox_auto_resize->get_active();
        need_restart(RESTART_REASON::CODEBOX_AUTORESIZE);
    });
    spinbutton_embfile_icon_size->signal_value_changed().connect([this, pConfig, spinbutton_embfile_icon_size](){
        pConfig->embfileIconSize = spinbutton_embfile_icon_size->get_value_as_int();
        need_restart(RESTART_REASON::EMBFILE_SIZE);
    });
    spinbutton_embfile_max_size->signal_value_changed().connect([pConfig, spinbutton_embfile_max_size](){
        pConfig->embfileMaxSize = spinbutton_embfile_max_size->get_value_as_int();
    });
    checkbutton_embfile_show_filename->signal_toggled().connect([this, pConfig, checkbutton_embfile_show_filename](){
        pConfig->embfileShowFileName = checkbutton_embfile_show_filename->get_active();
        need_restart(RESTART_REASON::SHOW_EMBFILE_NAME);
    });
    spinbutton_limit_undoable_steps->signal_value_changed().connect([pConfig, spinbutton_limit_undoable_steps](){
        pConfig->limitUndoableSteps = spinbutton_limit_undoable_steps->get_value_as_int();
    });
    checkbutton_triple_click_sel_paragraph->signal_toggled().connect([pConfig, checkbutton_triple_click_sel_paragraph]{
        pConfig->tripleClickParagraph = checkbutton_triple_click_sel_paragraph->get_active();
    });
#ifdef MD_AUTO_REPLACEMENT
    checkbutton_md_formatting->signal_toggled().connect([pConfig, checkbutton_md_formatting]{
        pConfig->enableMdFormatting = checkbutton_md_formatting->get_active();
    });
#endif // MD_AUTO_REPLACEMENT

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_plain_text_n_code()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_syntax = Gtk::manage(new Gtk::VBox());

    Gtk::CheckButton* checkbutton_pt_show_white_spaces = Gtk::manage(new Gtk::CheckButton(_("Show White Spaces")));
    checkbutton_pt_show_white_spaces->set_active(pConfig->ptShowWhiteSpaces);
    Gtk::CheckButton* checkbutton_pt_highl_curr_line = Gtk::manage(new Gtk::CheckButton(_("Highlight Current Line")));
    checkbutton_pt_highl_curr_line->set_active(pConfig->ptHighlCurrLine);
    Gtk::CheckButton* checkbutton_pt_highl_match_bra = Gtk::manage(new Gtk::CheckButton(_("Highlight Matching Brackets")));
    checkbutton_pt_highl_match_bra->set_active(pConfig->ptHighlMatchBra);

    vbox_syntax->pack_start(*checkbutton_pt_show_white_spaces, false, false);
    vbox_syntax->pack_start(*checkbutton_pt_highl_curr_line, false, false);
    vbox_syntax->pack_start(*checkbutton_pt_highl_match_bra, false, false);

    Gtk::Frame* frame_syntax = new_managed_frame_with_align(_("Text Editor"), vbox_syntax);

    Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(_commandModelColumns);
    _fill_custom_exec_commands_model(liststore);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(liststore));
    treeview->set_headers_visible(false);
    treeview->set_size_request(300, 200);

    Gtk::CellRendererPixbuf pixbuf_renderer;
    pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
    const int col_num_pixbuf = treeview->append_column("", pixbuf_renderer) - 1;
    treeview->get_column(col_num_pixbuf)->add_attribute(pixbuf_renderer, "icon-name", _shortcutModelColumns.icon);

    treeview->append_column("", _commandModelColumns.key);
    const int col_num_ext = treeview->append_column_editable("", _commandModelColumns.ext) - 1;
    const int col_num_desc = treeview->append_column_editable("", _commandModelColumns.desc) - 1;

    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow->add(*treeview);

    Gtk::Button* button_add = Gtk::manage(new Gtk::Button());
    button_add->set_image(*_pCtMainWin->new_image_from_stock("ct_add", Gtk::ICON_SIZE_BUTTON));
    button_add->set_tooltip_text(_("Add"));
    Gtk::Button* button_remove = Gtk::manage(new Gtk::Button());
    button_remove->set_image(*_pCtMainWin->new_image_from_stock("ct_remove", Gtk::ICON_SIZE_BUTTON));
    button_remove->set_tooltip_text(_("Remove Selected"));
    Gtk::Button* button_reset_cmds = Gtk::manage(new Gtk::Button());
    button_reset_cmds->set_image(*_pCtMainWin->new_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_cmds->set_tooltip_text(_("Reset to Default"));
    Gtk::VBox* vbox_buttons = Gtk::manage(new Gtk::VBox());
    vbox_buttons->pack_start(*button_add, false, false);
    vbox_buttons->pack_start(*button_remove, false, false);
    vbox_buttons->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    vbox_buttons->pack_start(*button_reset_cmds, false, false);

    Gtk::VBox* vbox_codexec = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_term_run = Gtk::manage(new Gtk::HBox());
    Gtk::Entry* entry_term_run = Gtk::manage(new Gtk::Entry());
    entry_term_run->set_text(get_code_exec_term_run(_pCtMainWin));
    Gtk::Button* button_reset_term = Gtk::manage(new Gtk::Button());
    button_reset_term->set_image(*_pCtMainWin->new_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_term->set_tooltip_text(_("Reset to Default"));
    hbox_term_run->pack_start(*entry_term_run, true, true);
    hbox_term_run->pack_start(*button_reset_term, false, false);
    Gtk::HBox* hbox_cmd_per_type = Gtk::manage(new Gtk::HBox());
    hbox_cmd_per_type->pack_start(*scrolledwindow, true, true);
    hbox_cmd_per_type->pack_start(*vbox_buttons, false, false);

    Gtk::Label* label = Gtk::manage(new Gtk::Label(Glib::ustring{"<b>"}+_("Command per Node/CodeBox Type")+"</b>"));
    label->set_use_markup(true);
    vbox_codexec->pack_start(*label, false, false);
    vbox_codexec->pack_start(*hbox_cmd_per_type, true, true);
    Gtk::Label* label2 = Gtk::manage(new Gtk::Label(Glib::ustring{"<b>"}+_("Terminal Command")+"</b>"));
    label2->set_use_markup(true);
    vbox_codexec->pack_start(*label2, false, false);
    vbox_codexec->pack_start(*hbox_term_run, false, false);

    Gtk::Frame* frame_codexec = new_managed_frame_with_align(_("Code Execution"), vbox_codexec);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_syntax, false, false);
    pMainBox->pack_start(*frame_codexec, true, true);

    checkbutton_pt_show_white_spaces->signal_toggled().connect([this, pConfig, checkbutton_pt_show_white_spaces](){
        pConfig->ptShowWhiteSpaces = checkbutton_pt_show_white_spaces->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('p'/*PlainTextNCode*/);
        });
    });
    checkbutton_pt_highl_curr_line->signal_toggled().connect([this, pConfig, checkbutton_pt_highl_curr_line](){
        pConfig->ptHighlCurrLine = checkbutton_pt_highl_curr_line->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('p'/*PlainTextNCode*/);
        });
    });
    checkbutton_pt_highl_match_bra->signal_toggled().connect([this, pConfig, checkbutton_pt_highl_match_bra](){
        pConfig->ptHighlMatchBra = checkbutton_pt_highl_match_bra->get_active();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
    });
    Gtk::CellRendererText* pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(treeview->get_column(col_num_desc)->get_cells()[0]);
    pCellRendererText->signal_edited().connect([this, pConfig, liststore](const Glib::ustring& path, const Glib::ustring& new_command){
        auto row = liststore->get_iter(path);
        row->set_value(_commandModelColumns.desc, new_command);
        pConfig->customCodexecType[row->get_value(_commandModelColumns.key)] = new_command;
    });
    pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(treeview->get_column(col_num_ext)->get_cells()[0]);
    pCellRendererText->signal_edited().connect([this, pConfig, liststore](const Glib::ustring& path, const Glib::ustring& new_ext){
        auto row = liststore->get_iter(path);
        row->set_value(_commandModelColumns.ext, new_ext);
        pConfig->customCodexecExt[row->get_value(_commandModelColumns.key)] = new_ext;
    });
    entry_term_run->signal_changed().connect([pConfig, entry_term_run](){
        pConfig->customCodexecTerm = entry_term_run->get_text();
    });
    button_add->signal_clicked().connect([this, treeview, liststore](){
        _add_new_command_in_model(treeview, liststore);
    });
    button_remove->signal_clicked().connect([this, treeview, liststore](){
        _remove_command_from_model(treeview, liststore);
    });
    auto button_remove_test_sensitive = [button_remove, treeview](){
        button_remove->set_sensitive(treeview->get_selection()->get_selected());
    };
    treeview->signal_cursor_changed().connect(button_remove_test_sensitive);
    button_remove_test_sensitive();
    button_reset_cmds->signal_clicked().connect([this, pConfig, liststore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->customCodexecType.clear();
            _fill_custom_exec_commands_model(liststore);
        }
    });
    button_reset_term->signal_clicked().connect([this, pConfig, entry_term_run](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->customCodexecTerm.clear();
            entry_term_run->set_text(get_code_exec_term_run(_pCtMainWin));
        }
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_theme()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    // Tree Theme
    Gtk::Box* vbox_tt_theme = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_tt_col_light = Gtk::manage(new Gtk::RadioButton(_("Light Background, Dark Text")));
    Gtk::RadioButton* radiobutton_tt_col_dark = Gtk::manage(new Gtk::RadioButton(_("Dark Background, Light Text")));
    radiobutton_tt_col_dark->join_group(*radiobutton_tt_col_light);
    Gtk::RadioButton* radiobutton_tt_col_custom = Gtk::manage(new Gtk::RadioButton(_("Custom Background")));
    radiobutton_tt_col_custom->join_group(*radiobutton_tt_col_light);
    Gtk::HBox* hbox_tt_col_custom = Gtk::manage(new Gtk::HBox());
    hbox_tt_col_custom->set_spacing(4);
    Gtk::ColorButton* colorbutton_tree_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->ttDefBg)));
    Gtk::Label* label_tt_col_custom = Gtk::manage(new Gtk::Label(_("and Text")));
    Gtk::ColorButton* colorbutton_tree_fg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->ttDefFg)));
    hbox_tt_col_custom->pack_start(*radiobutton_tt_col_custom, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_bg, false, false);
    hbox_tt_col_custom->pack_start(*label_tt_col_custom, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_fg, false, false);

    vbox_tt_theme->pack_start(*radiobutton_tt_col_light, false, false);
    vbox_tt_theme->pack_start(*radiobutton_tt_col_dark, false, false);
    vbox_tt_theme->pack_start(*hbox_tt_col_custom, false, false);
    Gtk::Frame* frame_tt_theme = new_managed_frame_with_align(_("Tree"), vbox_tt_theme);

    if (pConfig->ttDefFg == CtConst::TREE_TEXT_DARK_FG && pConfig->ttDefBg == CtConst::TREE_TEXT_DARK_BG)
    {
        radiobutton_tt_col_dark->set_active(true);
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
    }
    else if (pConfig->ttDefFg == CtConst::TREE_TEXT_LIGHT_FG && pConfig->ttDefBg == CtConst::TREE_TEXT_LIGHT_BG)
    {
        radiobutton_tt_col_light->set_active(true);
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
    }
    else
    {
        radiobutton_tt_col_custom->set_active(true);
    }

    // Rich Text Theme
    Gtk::VBox* vbox_rt_theme = Gtk::manage(new Gtk::VBox());

    Gtk::HBox* hbox_style_scheme_rt = Gtk::manage(new Gtk::HBox());
    hbox_style_scheme_rt->set_spacing(4);
    Gtk::Label* label_style_scheme_rt = Gtk::manage(new Gtk::Label(_("Style Scheme")));
    Gtk::ComboBoxText* combobox_style_scheme_rt = Gtk::manage(new Gtk::ComboBoxText());
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids())
        combobox_style_scheme_rt->append(scheme);
    combobox_style_scheme_rt->set_active_text(pConfig->rtStyleScheme);
    hbox_style_scheme_rt->pack_start(*label_style_scheme_rt, false, false);
    hbox_style_scheme_rt->pack_start(*combobox_style_scheme_rt, false, false);

    Gtk::CheckButton* checkbutton_monospace_bg = Gtk::manage(new Gtk::CheckButton(_("Monospace Background")));
    std::string mono_color = pConfig->monospaceBg.empty() ? CtConst::DEFAULT_MONOSPACE_BG : pConfig->monospaceBg;
    Gtk::ColorButton* colorbutton_monospace_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(mono_color)));
    Gtk::HBox* hbox_monospace_bg = Gtk::manage(new Gtk::HBox());
    hbox_monospace_bg->set_spacing(4);
    hbox_monospace_bg->pack_start(*checkbutton_monospace_bg, false, false);
    hbox_monospace_bg->pack_start(*colorbutton_monospace_bg, false, false);

    checkbutton_monospace_bg->set_active(!pConfig->monospaceBg.empty());
    colorbutton_monospace_bg->set_sensitive(!pConfig->monospaceBg.empty());

    vbox_rt_theme->pack_start(*hbox_style_scheme_rt, false, false);
    vbox_rt_theme->pack_start(*hbox_monospace_bg, false, false);
    Gtk::Frame* frame_rt_theme = new_managed_frame_with_align(_("Rich Text"), vbox_rt_theme);

    // Plain Text and Code Theme
    Gtk::VBox* vbox_pt_theme = Gtk::manage(new Gtk::VBox());

    Gtk::HBox* hbox_style_scheme_pt = Gtk::manage(new Gtk::HBox());
    hbox_style_scheme_pt->set_spacing(4);
    Gtk::Label* label_style_scheme_pt = Gtk::manage(new Gtk::Label(_("Style Scheme")));
    Gtk::ComboBoxText* combobox_style_scheme_pt = Gtk::manage(new Gtk::ComboBoxText());
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids())
        combobox_style_scheme_pt->append(scheme);
    combobox_style_scheme_pt->set_active_text(pConfig->ptStyleScheme);
    hbox_style_scheme_pt->pack_start(*label_style_scheme_pt, false, false);
    hbox_style_scheme_pt->pack_start(*combobox_style_scheme_pt, false, false);

    vbox_pt_theme->pack_start(*hbox_style_scheme_pt, false, false);
    Gtk::Frame* frame_pt_theme = new_managed_frame_with_align(_("Plain Text and Code"), vbox_pt_theme);

    // Table Theme
    Gtk::VBox* vbox_ta_theme = Gtk::manage(new Gtk::VBox());

    Gtk::HBox* hbox_style_scheme_ta = Gtk::manage(new Gtk::HBox());
    hbox_style_scheme_ta->set_spacing(4);
    Gtk::Label* label_style_scheme_ta = Gtk::manage(new Gtk::Label(_("Style Scheme")));
    Gtk::ComboBoxText* combobox_style_scheme_ta = Gtk::manage(new Gtk::ComboBoxText());
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids())
        combobox_style_scheme_ta->append(scheme);
    combobox_style_scheme_ta->set_active_text(pConfig->taStyleScheme);
    hbox_style_scheme_ta->pack_start(*label_style_scheme_ta, false, false);
    hbox_style_scheme_ta->pack_start(*combobox_style_scheme_ta, false, false);

    vbox_ta_theme->pack_start(*hbox_style_scheme_ta, false, false);
    Gtk::Frame* frame_ta_theme = new_managed_frame_with_align(_("Table"), vbox_ta_theme);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_tt_theme, false, false);
    pMainBox->pack_start(*frame_rt_theme, false, false);
    pMainBox->pack_start(*frame_pt_theme, false, false);
    pMainBox->pack_start(*frame_ta_theme, false, false);

    auto update_tree_color = [this, pConfig, colorbutton_tree_fg, colorbutton_tree_bg]() {
        pConfig->ttDefFg = CtRgbUtil::rgb_any_to_24(colorbutton_tree_fg->get_rgba());
        pConfig->ttDefBg = CtRgbUtil::rgb_any_to_24(colorbutton_tree_bg->get_rgba());
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    };

    colorbutton_tree_fg->signal_color_set().connect([update_tree_color, radiobutton_tt_col_custom](){
        if (!radiobutton_tt_col_custom->get_active()) return;
        update_tree_color();
    });
    colorbutton_tree_bg->signal_color_set().connect([update_tree_color, radiobutton_tt_col_custom](){
        if (!radiobutton_tt_col_custom->get_active()) return;
        update_tree_color();
    });
    radiobutton_tt_col_light->signal_toggled().connect([radiobutton_tt_col_light, colorbutton_tree_fg, colorbutton_tree_bg, update_tree_color](){
        if (!radiobutton_tt_col_light->get_active()) return;
        colorbutton_tree_fg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_LIGHT_FG));
        colorbutton_tree_bg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_LIGHT_BG));
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        update_tree_color();
    });
    radiobutton_tt_col_dark->signal_toggled().connect([radiobutton_tt_col_dark, colorbutton_tree_fg, colorbutton_tree_bg, update_tree_color](){
        if (!radiobutton_tt_col_dark->get_active()) return;
        colorbutton_tree_fg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_DARK_FG));
        colorbutton_tree_bg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_DARK_BG));
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        update_tree_color();
    });
    radiobutton_tt_col_custom->signal_toggled().connect([radiobutton_tt_col_custom, colorbutton_tree_fg, colorbutton_tree_bg](){
        if (!radiobutton_tt_col_custom->get_active()) return;
        colorbutton_tree_fg->set_sensitive(true);
        colorbutton_tree_bg->set_sensitive(true);
    });

    combobox_style_scheme_rt->signal_changed().connect([this, pConfig, combobox_style_scheme_rt](){
        pConfig->rtStyleScheme = combobox_style_scheme_rt->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('r'/*RichText*/); });
    });
    checkbutton_monospace_bg->signal_toggled().connect([this, pConfig, checkbutton_monospace_bg, colorbutton_monospace_bg](){
        pConfig->monospaceBg = checkbutton_monospace_bg->get_active() ?
            CtRgbUtil::rgb_any_to_24(colorbutton_monospace_bg->get_rgba()) : "";
        colorbutton_monospace_bg->set_sensitive(not pConfig->monospaceBg.empty());
        if (not pConfig->monospaceBg.empty()) {
            if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
                tag->property_background() = pConfig->monospaceBg;
            }
        }
        else {
            need_restart(RESTART_REASON::MONOSPACE);
        }
    });
    colorbutton_monospace_bg->signal_color_set().connect([this, pConfig, colorbutton_monospace_bg](){
        pConfig->monospaceBg = CtRgbUtil::rgb_any_to_24(colorbutton_monospace_bg->get_rgba());
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_background() = pConfig->monospaceBg;
        }
    });

    combobox_style_scheme_pt->signal_changed().connect([this, pConfig, combobox_style_scheme_pt](){
        pConfig->ptStyleScheme = combobox_style_scheme_pt->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
    });

    combobox_style_scheme_ta->signal_changed().connect([this, pConfig, combobox_style_scheme_ta](){
        pConfig->taStyleScheme = combobox_style_scheme_ta->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('t'/*Table*/); });
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_tree()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_nodes_icons = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_node_icon_cherry = Gtk::manage(new Gtk::RadioButton(_("Use Different Cherries per Level")));
    Gtk::RadioButton* radiobutton_node_icon_custom = Gtk::manage(new Gtk::RadioButton(_("Use Selected Icon")));
    radiobutton_node_icon_custom->join_group(*radiobutton_node_icon_cherry);
    Gtk::RadioButton* radiobutton_node_icon_none = Gtk::manage(new Gtk::RadioButton(_("No Icon")));
    radiobutton_node_icon_none->join_group(*radiobutton_node_icon_cherry);
    Gtk::CheckButton* checkbutton_aux_icon_hide = Gtk::manage(new Gtk::CheckButton(_("Hide Right Side Auxiliary Icon")));
    checkbutton_aux_icon_hide->set_active(pConfig->auxIconHide);

    Gtk::Button* c_icon_button = Gtk::manage(new Gtk::Button());
    c_icon_button->set_image(*_pCtMainWin->new_image_from_stock(CtConst::NODE_CUSTOM_ICONS.at(pConfig->defaultIconText), Gtk::ICON_SIZE_BUTTON));
    Gtk::HBox* c_icon_hbox = Gtk::manage(new Gtk::HBox());
    c_icon_hbox->set_spacing(2);
    c_icon_hbox->pack_start(*radiobutton_node_icon_custom, false, false);
    c_icon_hbox->pack_start(*c_icon_button, false, false);

    vbox_nodes_icons->pack_start(*radiobutton_node_icon_cherry, false, false);
    vbox_nodes_icons->pack_start(*c_icon_hbox, false, false);
    vbox_nodes_icons->pack_start(*radiobutton_node_icon_none, false, false);
    vbox_nodes_icons->pack_start(*checkbutton_aux_icon_hide, false, false);
    Gtk::Frame* frame_nodes_icons = new_managed_frame_with_align(_("Default Text Nodes Icons"), vbox_nodes_icons);

    radiobutton_node_icon_cherry->set_active(pConfig->nodesIcons == "c");
    radiobutton_node_icon_custom->set_active(pConfig->nodesIcons == "b");
    radiobutton_node_icon_none->set_active(pConfig->nodesIcons == "n");

    Gtk::VBox* vbox_nodes_startup = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_nodes_startup_restore = Gtk::manage(new Gtk::RadioButton(_("Restore Expanded/Collapsed Status")));
    Gtk::RadioButton* radiobutton_nodes_startup_expand = Gtk::manage(new Gtk::RadioButton(_("Expand all Nodes")));
    radiobutton_nodes_startup_expand->join_group(*radiobutton_nodes_startup_restore);
    Gtk::RadioButton* radiobutton_nodes_startup_collapse = Gtk::manage(new Gtk::RadioButton(_("Collapse all Nodes")));
    radiobutton_nodes_startup_collapse->join_group(*radiobutton_nodes_startup_restore);
    Gtk::CheckButton* checkbutton_nodes_bookm_exp = Gtk::manage(new Gtk::CheckButton(_("Nodes in Bookmarks Always Visible")));
    checkbutton_nodes_bookm_exp->set_active(pConfig->nodesBookmExp);
    checkbutton_nodes_bookm_exp->set_sensitive(pConfig->restoreExpColl != CtRestoreExpColl::ALL_EXP);

    vbox_nodes_startup->pack_start(*radiobutton_nodes_startup_restore, false, false);
    vbox_nodes_startup->pack_start(*radiobutton_nodes_startup_expand, false, false);
    vbox_nodes_startup->pack_start(*radiobutton_nodes_startup_collapse, false, false);
    vbox_nodes_startup->pack_start(*checkbutton_nodes_bookm_exp, false, false);
    Gtk::Frame* frame_nodes_startup = new_managed_frame_with_align(_("Nodes Status at Startup"), vbox_nodes_startup);

    radiobutton_nodes_startup_restore->set_active(pConfig->restoreExpColl == CtRestoreExpColl::FROM_STR);
    radiobutton_nodes_startup_expand->set_active(pConfig->restoreExpColl == CtRestoreExpColl::ALL_EXP);
    radiobutton_nodes_startup_collapse->set_active(pConfig->restoreExpColl == CtRestoreExpColl::ALL_COLL);

    Gtk::VBox* vbox_misc_tree = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_tree_nodes_names_width = Gtk::manage(new Gtk::HBox());
    hbox_tree_nodes_names_width->set_spacing(4);
    Gtk::CheckButton* checkbutton_tree_nodes_names_wrap_ena = Gtk::manage(new Gtk::CheckButton(_("Tree Nodes Names Wrapping Width")));
    checkbutton_tree_nodes_names_wrap_ena->set_active(pConfig->cherryWrapEnabled);
    Glib::RefPtr<Gtk::Adjustment> adj_tree_nodes_names_width = Gtk::Adjustment::create(pConfig->cherryWrapWidth, 10, 10000, 1);
    Gtk::SpinButton* spinbutton_tree_nodes_names_width = Gtk::manage(new Gtk::SpinButton(adj_tree_nodes_names_width));
    spinbutton_tree_nodes_names_width->set_value(pConfig->cherryWrapWidth);
    spinbutton_tree_nodes_names_width->set_sensitive(pConfig->cherryWrapEnabled);
    hbox_tree_nodes_names_width->pack_start(*checkbutton_tree_nodes_names_wrap_ena, false, false);
    hbox_tree_nodes_names_width->pack_start(*spinbutton_tree_nodes_names_width, false, false);
    Gtk::CheckButton* checkbutton_tree_right_side = Gtk::manage(new Gtk::CheckButton(_("Display Tree on the Right Side")));
    checkbutton_tree_right_side->set_active(pConfig->treeRightSide);
    Gtk::CheckButton* checkbutton_tree_click_focus_text = Gtk::manage(new Gtk::CheckButton(_("Move Focus to Text at Mouse Click")));
    checkbutton_tree_click_focus_text->set_active(pConfig->treeClickFocusText);
    Gtk::CheckButton* checkbutton_tree_click_expand = Gtk::manage(new Gtk::CheckButton(_("Expand Node at Mouse Click")));
    checkbutton_tree_click_expand->set_active(pConfig->treeClickExpand);
    Gtk::HBox* hbox_nodes_on_node_name_header = Gtk::manage(new Gtk::HBox());
    hbox_nodes_on_node_name_header->set_spacing(4);
    Gtk::Label* label_nodes_on_node_name_header = Gtk::manage(new Gtk::Label(_("Last Visited Nodes on Node Name Header")));
    Glib::RefPtr<Gtk::Adjustment> adj_nodes_on_node_name_header = Gtk::Adjustment::create(pConfig->nodesOnNodeNameHeader, 0, 100, 1);
    Gtk::SpinButton* spinbutton_nodes_on_node_name_header = Gtk::manage(new Gtk::SpinButton(adj_nodes_on_node_name_header));
    spinbutton_nodes_on_node_name_header->set_value(pConfig->nodesOnNodeNameHeader);
    hbox_nodes_on_node_name_header->pack_start(*label_nodes_on_node_name_header, false, false);
    hbox_nodes_on_node_name_header->pack_start(*spinbutton_nodes_on_node_name_header, false, false);

    vbox_misc_tree->pack_start(*hbox_tree_nodes_names_width, false, false);
    vbox_misc_tree->pack_start(*checkbutton_tree_right_side, false, false);
    vbox_misc_tree->pack_start(*checkbutton_tree_click_focus_text, false, false);
    vbox_misc_tree->pack_start(*checkbutton_tree_click_expand, false, false);
    vbox_misc_tree->pack_start(*hbox_nodes_on_node_name_header, false, false);
    Gtk::Frame* frame_misc_tree = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_tree);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_nodes_icons, false, false);
    pMainBox->pack_start(*frame_nodes_startup, false, false);
    pMainBox->pack_start(*frame_misc_tree, false, false);

    checkbutton_tree_nodes_names_wrap_ena->signal_toggled().connect([this,
                                                                     pConfig,
                                                                     checkbutton_tree_nodes_names_wrap_ena,
                                                                     spinbutton_tree_nodes_names_width](){
        pConfig->cherryWrapEnabled = checkbutton_tree_nodes_names_wrap_ena->get_active();
        spinbutton_tree_nodes_names_width->set_sensitive(pConfig->cherryWrapEnabled);
        need_restart(RESTART_REASON::TREE_NODE_WRAP);
    });
    spinbutton_tree_nodes_names_width->signal_value_changed().connect([this, pConfig, spinbutton_tree_nodes_names_width](){
        pConfig->cherryWrapWidth = spinbutton_tree_nodes_names_width->get_value_as_int();
        need_restart(RESTART_REASON::TREE_NODE_WRAP);
    });
    checkbutton_tree_right_side->signal_toggled().connect([this, pConfig, checkbutton_tree_right_side](){
        pConfig->treeRightSide = checkbutton_tree_right_side->get_active();
        apply_for_each_window([](CtMainWin* win) { win->config_switch_tree_side(); });
    });
    checkbutton_tree_click_focus_text->signal_toggled().connect([pConfig, checkbutton_tree_click_focus_text](){
        pConfig->treeClickFocusText = checkbutton_tree_click_focus_text->get_active();
    });
    checkbutton_tree_click_expand->signal_toggled().connect([pConfig, checkbutton_tree_click_expand](){
        pConfig->treeClickExpand = checkbutton_tree_click_expand->get_active();
    });
    spinbutton_nodes_on_node_name_header->signal_value_changed().connect([this, pConfig, spinbutton_nodes_on_node_name_header](){
        pConfig->nodesOnNodeNameHeader = spinbutton_nodes_on_node_name_header->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) { win->window_header_update(); });
    });

    radiobutton_node_icon_cherry->signal_toggled().connect([this, pConfig, radiobutton_node_icon_cherry](){
        if (!radiobutton_node_icon_cherry->get_active()) return;
        pConfig->nodesIcons = "c";
        apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false); });
    });
    radiobutton_node_icon_custom->signal_toggled().connect([this, pConfig, radiobutton_node_icon_custom](){
        if (!radiobutton_node_icon_custom->get_active()) return;
        pConfig->nodesIcons = "b";
        apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false); });
    });
    radiobutton_node_icon_none->signal_toggled().connect([this, pConfig, radiobutton_node_icon_none](){
        if (!radiobutton_node_icon_none->get_active()) return;
        pConfig->nodesIcons = "n";
        apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false); });
    });
    c_icon_button->signal_clicked().connect([this, pConfig, c_icon_button](){
        auto itemStore = CtChooseDialogListStore::create();
        for (int i = 1 /* skip 0 */; i < (int)CtConst::NODE_CUSTOM_ICONS.size(); ++i)
            itemStore->add_row(CtConst::NODE_CUSTOM_ICONS[i], std::to_string(i), "");
        auto res = CtDialogs::choose_item_dialog(*this, _("Select Node Icon"), itemStore);
        if (res) {
            pConfig->defaultIconText = std::stoi(res->get_value(itemStore->columns.key));
            c_icon_button->set_image(*_pCtMainWin->new_image_from_stock(res->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
            apply_for_each_window([](CtMainWin* win) { win->get_tree_store().update_nodes_icon(Gtk::TreeIter(), false);});
        }
    });
    radiobutton_nodes_startup_restore->signal_toggled().connect([pConfig, radiobutton_nodes_startup_restore, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_restore->get_active()) return;
        pConfig->restoreExpColl = CtRestoreExpColl::FROM_STR;
        checkbutton_nodes_bookm_exp->set_sensitive(true);
    });
    radiobutton_nodes_startup_expand->signal_toggled().connect([pConfig, radiobutton_nodes_startup_expand, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_expand->get_active()) return;
        pConfig->restoreExpColl = CtRestoreExpColl::ALL_EXP;
        checkbutton_nodes_bookm_exp->set_sensitive(false);
    });
    radiobutton_nodes_startup_collapse->signal_toggled().connect([pConfig, radiobutton_nodes_startup_collapse, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_collapse->get_active()) return;
        pConfig->restoreExpColl = CtRestoreExpColl::ALL_COLL;
        checkbutton_nodes_bookm_exp->set_sensitive(true);
    });
    checkbutton_nodes_bookm_exp->signal_toggled().connect([pConfig, checkbutton_nodes_bookm_exp](){
        pConfig->nodesBookmExp = checkbutton_nodes_bookm_exp->get_active();
    });
    checkbutton_aux_icon_hide->signal_toggled().connect([this, pConfig, checkbutton_aux_icon_hide](){
        pConfig->auxIconHide = checkbutton_aux_icon_hide->get_active();
        apply_for_each_window([pConfig](CtMainWin* win) { win->get_tree_view().get_column(CtTreeView::AUX_ICON_COL_NUM)->set_visible(!pConfig->auxIconHide); });
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_fonts()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::Image* image_rt = _pCtMainWin->new_image_from_stock("ct_fonts", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_ms = _pCtMainWin->new_image_from_stock("ct_fmt-txt-monospace", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_pt = _pCtMainWin->new_image_from_stock("ct_fonts", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_code = _pCtMainWin->new_image_from_stock("ct_code", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_tree = _pCtMainWin->new_image_from_stock("ct_cherries", Gtk::ICON_SIZE_MENU);
    Gtk::Label* label_rt = Gtk::manage(new Gtk::Label(_("Rich Text")));
    label_rt->set_halign(Gtk::Align::ALIGN_END);
    Gtk::CheckButton* checkbutton_ms = Gtk::manage(new Gtk::CheckButton(_("Monospace")));
    checkbutton_ms->set_halign(Gtk::Align::ALIGN_END);
    checkbutton_ms->set_active(pConfig->msDedicatedFont);
    Gtk::Label* label_pt = Gtk::manage(new Gtk::Label(_("Plain Text")));
    label_pt->set_halign(Gtk::Align::ALIGN_END);
    Gtk::Label* label_code = Gtk::manage(new Gtk::Label(_("Code Font")));
    label_code->set_halign(Gtk::Align::ALIGN_END);
    Gtk::Label* label_tree = Gtk::manage(new Gtk::Label(_("Tree Font")));
    label_tree->set_halign(Gtk::Align::ALIGN_END);
    Gtk::FontButton* fontbutton_rt = Gtk::manage(new Gtk::FontButton(pConfig->rtFont));
    Gtk::FontButton* fontbutton_ms = Gtk::manage(new Gtk::FontButton(pConfig->monospaceFont));
    fontbutton_ms->set_sensitive(pConfig->msDedicatedFont);
    Gtk::FontButton* fontbutton_pt = Gtk::manage(new Gtk::FontButton(pConfig->ptFont));
    Gtk::FontButton* fontbutton_code = Gtk::manage(new Gtk::FontButton(pConfig->codeFont));
    Gtk::FontButton* fontbutton_tree = Gtk::manage(new Gtk::FontButton(pConfig->treeFont));
    Gtk::Grid* grid_fonts = Gtk::manage(new Gtk::Grid());
    grid_fonts->set_row_spacing(2);
    grid_fonts->set_column_spacing(4);
    grid_fonts->set_row_homogeneous(true);
    grid_fonts->attach(*image_rt,        0, 0, 1, 1);
    grid_fonts->attach(*image_ms,        0, 1, 1, 1);
    grid_fonts->attach(*image_pt,        0, 2, 1, 1);
    grid_fonts->attach(*image_code,      0, 3, 1, 1);
    grid_fonts->attach(*image_tree,      0, 4, 1, 1);
    grid_fonts->attach(*label_rt,        1, 0, 1, 1);
    grid_fonts->attach(*checkbutton_ms,  1, 1, 1, 1);
    grid_fonts->attach(*label_pt,        1, 2, 1, 1);
    grid_fonts->attach(*label_code,      1, 3, 1, 1);
    grid_fonts->attach(*label_tree,      1, 4, 1, 1);
    grid_fonts->attach(*fontbutton_rt,   2, 0, 1, 1);
    grid_fonts->attach(*fontbutton_ms,   2, 1, 1, 1);
    grid_fonts->attach(*fontbutton_pt,   2, 2, 1, 1);
    grid_fonts->attach(*fontbutton_code, 2, 3, 1, 1);
    grid_fonts->attach(*fontbutton_tree, 2, 4, 1, 1);
    Gtk::Frame* frame_fonts = new_managed_frame_with_align(_("Fonts"), grid_fonts);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_fonts, false, false);

    fontbutton_rt->signal_font_set().connect([this, pConfig, fontbutton_rt](){
        pConfig->rtFont = fontbutton_rt->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    checkbutton_ms->signal_toggled().connect([this, pConfig, checkbutton_ms, fontbutton_ms](){
        pConfig->msDedicatedFont = checkbutton_ms->get_active();
        fontbutton_ms->set_sensitive(pConfig->msDedicatedFont);
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_family() = pConfig->msDedicatedFont ? "" : CtConst::TAG_PROP_VAL_MONOSPACE;
            tag->property_font() = pConfig->msDedicatedFont ? pConfig->monospaceFont : "";
        }
    });
    fontbutton_ms->signal_font_set().connect([this, pConfig, fontbutton_ms](){
        pConfig->monospaceFont = fontbutton_ms->get_font_name();
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_font() = pConfig->monospaceFont;
        }
    });
    fontbutton_pt->signal_font_set().connect([this, pConfig, fontbutton_pt](){
        pConfig->ptFont = fontbutton_pt->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    fontbutton_code->signal_font_set().connect([this, pConfig, fontbutton_code](){
        pConfig->codeFont = fontbutton_code->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    fontbutton_tree->signal_font_set().connect([this, pConfig, fontbutton_tree](){
        pConfig->treeFont = fontbutton_tree->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_links()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_links_actions = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_custom_weblink_cmd = Gtk::manage(new Gtk::CheckButton(_("Enable Custom Web Link Clicked Action")));
    Gtk::Entry* entry_custom_weblink_cmd = Gtk::manage(new Gtk::Entry());
    Gtk::CheckButton* checkbutton_custom_filelink_cmd = Gtk::manage(new Gtk::CheckButton(_("Enable Custom File Link Clicked Action")));
    Gtk::Entry* entry_custom_filelink_cmd = Gtk::manage(new Gtk::Entry());
    Gtk::CheckButton* checkbutton_custom_folderlink_cmd = Gtk::manage(new Gtk::CheckButton(_("Enable Custom Folder Link Clicked Action")));
    Gtk::Entry* entry_custom_folderlink_cmd = Gtk::manage(new Gtk::Entry());
    vbox_links_actions->pack_start(*checkbutton_custom_weblink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_weblink_cmd, false, false);
    vbox_links_actions->pack_start(*checkbutton_custom_filelink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_filelink_cmd, false, false);
    vbox_links_actions->pack_start(*checkbutton_custom_folderlink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_folderlink_cmd, false, false);

    Gtk::Frame* frame_links_actions = new_managed_frame_with_align(_("Custom Actions"), vbox_links_actions);

    checkbutton_custom_weblink_cmd->set_active(pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_sensitive(pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_text(pConfig->weblinkCustomAct);
    checkbutton_custom_filelink_cmd->set_active(pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_sensitive(pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_text(pConfig->filelinkCustomAct);
    checkbutton_custom_folderlink_cmd->set_active(pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_sensitive(pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_text(pConfig->folderlinkCustomAct);

    Gtk::Grid* grid_links_colors = Gtk::manage(new Gtk::Grid());
    grid_links_colors->set_row_spacing(2);
    grid_links_colors->set_column_spacing(15);
    grid_links_colors->set_row_homogeneous(true);

    Gtk::Label* label_col_link_webs = Gtk::manage(new Gtk::Label(_("To WebSite")));
    Gtk::ColorButton* colorbutton_col_link_webs = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkWebs)));
    Gtk::Label* label_col_link_node = Gtk::manage(new Gtk::Label(_("To Node")));
    Gtk::ColorButton* colorbutton_col_link_node = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkNode)));
    Gtk::Label* label_col_link_file = Gtk::manage(new Gtk::Label(_("To File")));
    Gtk::ColorButton* colorbutton_col_link_file = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkFile)));
    Gtk::Label* label_col_link_fold = Gtk::manage(new Gtk::Label(_("To Folder")));
    Gtk::ColorButton* colorbutton_col_link_fold = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkFold)));

    grid_links_colors->attach(*label_col_link_webs, 0, 0, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_webs, 1, 0, 1, 1);

    grid_links_colors->attach(*label_col_link_node, 0, 1, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_node, 1, 1, 1, 1);

    grid_links_colors->attach(*label_col_link_file, 2, 0, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_file, 3, 0, 1, 1);

    grid_links_colors->attach(*label_col_link_fold, 2, 1, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_fold, 3, 1, 1, 1);

    Gtk::Frame* frame_links_colors = new_managed_frame_with_align(_("Colors"), grid_links_colors);

    Gtk::VBox* vbox_links_misc = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_links_underline = Gtk::manage(new Gtk::CheckButton(_("Underline Links")));
    checkbutton_links_underline->set_active(pConfig->linksUnderline);
    Gtk::CheckButton* checkbutton_links_relative = Gtk::manage(new Gtk::CheckButton(_("Use Relative Paths for Files And Folders")));
    checkbutton_links_relative->set_active(pConfig->linksRelative);
    Gtk::HBox* hbox_anchor_size = Gtk::manage(new Gtk::HBox());
    hbox_anchor_size->set_spacing(4);
    Gtk::Label* label_anchor_size = Gtk::manage(new Gtk::Label(_("Anchor Size")));
    Glib::RefPtr<Gtk::Adjustment> adj_anchor_size = Gtk::Adjustment::create(pConfig->anchorSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_anchor_size = Gtk::manage(new Gtk::SpinButton(adj_anchor_size));
    spinbutton_anchor_size->set_value(pConfig->anchorSize);
    hbox_anchor_size->pack_start(*label_anchor_size, false, false);
    hbox_anchor_size->pack_start(*spinbutton_anchor_size, false, false);
    vbox_links_misc->pack_start(*checkbutton_links_underline, false, false);
    vbox_links_misc->pack_start(*checkbutton_links_relative, false, false);
    vbox_links_misc->pack_start(*hbox_anchor_size, false, false);

    Gtk::Frame* frame_links_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_links_misc);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_links_actions, false, false);
    pMainBox->pack_start(*frame_links_colors, false, false);
    pMainBox->pack_start(*frame_links_misc, false, false);

    checkbutton_custom_weblink_cmd->signal_toggled().connect([pConfig, checkbutton_custom_weblink_cmd, entry_custom_weblink_cmd](){
        pConfig->weblinkCustomOn = checkbutton_custom_weblink_cmd->get_active();
        entry_custom_weblink_cmd->set_sensitive(pConfig->weblinkCustomOn);
    });
    entry_custom_weblink_cmd->signal_changed().connect([pConfig, entry_custom_weblink_cmd](){
        pConfig->weblinkCustomAct = entry_custom_weblink_cmd->get_text();
    });
    checkbutton_custom_filelink_cmd->signal_toggled().connect([pConfig, checkbutton_custom_filelink_cmd, entry_custom_filelink_cmd](){
        pConfig->filelinkCustomOn = checkbutton_custom_filelink_cmd->get_active();
        entry_custom_filelink_cmd->set_sensitive(pConfig->filelinkCustomOn);
    });
    entry_custom_filelink_cmd->signal_changed().connect([pConfig, entry_custom_filelink_cmd](){
        pConfig->filelinkCustomAct = entry_custom_filelink_cmd->get_text();
    });
    checkbutton_custom_folderlink_cmd->signal_toggled().connect([pConfig, checkbutton_custom_folderlink_cmd, entry_custom_folderlink_cmd](){
        pConfig->folderlinkCustomOn = checkbutton_custom_folderlink_cmd->get_active();
        entry_custom_folderlink_cmd->set_sensitive(pConfig->folderlinkCustomOn);
    });
    entry_custom_folderlink_cmd->signal_changed().connect([pConfig, entry_custom_folderlink_cmd](){
        pConfig->folderlinkCustomAct = entry_custom_folderlink_cmd->get_text();
    });
    checkbutton_links_relative->signal_toggled().connect([pConfig, checkbutton_links_relative](){
        pConfig->linksRelative = checkbutton_links_relative->get_active();
    });
    checkbutton_links_underline->signal_toggled().connect([this, pConfig, checkbutton_links_underline](){
        pConfig->linksUnderline = checkbutton_links_underline->get_active();
        need_restart(RESTART_REASON::LINKS);
    });
    spinbutton_anchor_size->signal_value_changed().connect([this, pConfig, spinbutton_anchor_size](){
        pConfig->anchorSize = spinbutton_anchor_size->get_value_as_int();
        need_restart(RESTART_REASON::ANCHOR_SIZE);
    });
    colorbutton_col_link_webs->signal_color_set().connect([this, pConfig, colorbutton_col_link_webs](){
        pConfig->colLinkWebs = CtRgbUtil::rgb_to_string(colorbutton_col_link_webs->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_node->signal_color_set().connect([this, pConfig, colorbutton_col_link_node](){
        pConfig->colLinkNode = CtRgbUtil::rgb_to_string(colorbutton_col_link_node->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_file->signal_color_set().connect([this, pConfig, colorbutton_col_link_file](){
        pConfig->colLinkFile =  CtRgbUtil::rgb_to_string(colorbutton_col_link_file->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_fold->signal_color_set().connect([this, pConfig, colorbutton_col_link_fold](){
        pConfig->colLinkFold = CtRgbUtil::rgb_to_string(colorbutton_col_link_fold->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_toolbar()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(_toolbarModelColumns);
    fill_toolbar_model(liststore);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(liststore));
    treeview->set_headers_visible(false);
    treeview->set_reorderable(true);
    treeview->set_size_request(300, 300);
    treeview->get_selection()->select(Gtk::TreePath("0"));

    Gtk::CellRendererPixbuf pixbuf_renderer;
    pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
    const int col_num_pixbuf = treeview->append_column("", pixbuf_renderer) - 1;
    treeview->get_column(col_num_pixbuf)->add_attribute(pixbuf_renderer, "icon-name", _shortcutModelColumns.icon);

    treeview->append_column("", _toolbarModelColumns.desc);
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->add(*treeview);

    Gtk::Button* button_add = Gtk::manage(new Gtk::Button());
    button_add->set_image(*_pCtMainWin->new_image_from_stock("ct_add",  Gtk::ICON_SIZE_BUTTON));
    button_add->set_tooltip_text(_("Add"));
    Gtk::Button* button_remove = Gtk::manage(new Gtk::Button());
    button_remove->set_image(*_pCtMainWin->new_image_from_stock("ct_remove", Gtk::ICON_SIZE_BUTTON));
    button_remove->set_tooltip_text(_("Remove Selected"));
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset->set_tooltip_text(_("Reset to Default"));

    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());
    vbox->pack_start(*button_add, false, false);
    vbox->pack_start(*button_remove, false, false);
    vbox->pack_start(*Gtk::manage(new Gtk::Label()), true, true);
    vbox->pack_start(*button_reset, false, false);
    hbox->pack_start(*scrolledwindow, true, true);
    hbox->pack_start(*vbox, false, false);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*hbox);

    button_add->signal_clicked().connect([this, treeview, liststore](){
        if (add_new_item_in_toolbar_model(treeview, liststore)) {
            update_config_toolbar_from_model(liststore);
            apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
        }
    });
    button_remove->signal_clicked().connect([this, treeview, liststore](){
        liststore->erase(treeview->get_selection()->get_selected());
        update_config_toolbar_from_model(liststore);
        apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
    });
    button_reset->signal_clicked().connect([this, pConfig, liststore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->toolbarUiList = CtConst::TOOLBAR_VEC_DEFAULT;
            fill_toolbar_model(liststore);
            apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
        }
    });
    treeview->signal_key_press_event().connect([button_remove](GdkEventKey* key) -> bool {
        if (key->keyval == GDK_KEY_Delete) {
            button_remove->clicked();
            return true;
        }
        return false;
    });
    treeview->signal_drag_end().connect([this, liststore](const Glib::RefPtr<Gdk::DragContext>&){
        update_config_toolbar_from_model(liststore);
        apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true); });
    });
    auto button_remove_test_sensitive = [button_remove, treeview](){
        button_remove->set_sensitive(treeview->get_selection()->get_selected());
    };
    treeview->signal_cursor_changed().connect(button_remove_test_sensitive);
    button_remove_test_sensitive();

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_kb_shortcuts()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Glib::RefPtr<Gtk::TreeStore> treestore = Gtk::TreeStore::create(_shortcutModelColumns);
    fill_shortcut_model(treestore);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(treestore));
    treeview->set_headers_visible(false);
    treeview->set_reorderable(true);
    treeview->set_size_request(300, 300);
    treeview->set_reorderable(false);

    // icon column
    Gtk::CellRendererPixbuf pixbuf_renderer;
    pixbuf_renderer.property_stock_size() = Gtk::BuiltinIconSize::ICON_SIZE_LARGE_TOOLBAR;
    const int col_num_pixbuf = treeview->append_column("", pixbuf_renderer) - 1;
    treeview->get_column(col_num_pixbuf)->add_attribute(pixbuf_renderer, "icon-name", _shortcutModelColumns.icon);
    // shortcut column
    auto shortcut_cell_renderer = Gtk::manage(new Gtk::CellRendererText());
    shortcut_cell_renderer->property_xalign() = 1;
    auto shortcut_column = Gtk::manage(new Gtk::TreeViewColumn());
    shortcut_column->pack_start(*shortcut_cell_renderer, true);
    shortcut_column->set_cell_data_func(*shortcut_cell_renderer, [&](Gtk::CellRenderer* cell, const Gtk::TreeIter& iter){
        ((Gtk::CellRendererText*)cell)->property_markup() = "  " + str::xml_escape(CtStrUtil::get_accelerator_label(iter->get_value(_shortcutModelColumns.shortcut))) + "  ";
    });
    treeview->append_column(*shortcut_column);
    // desc
    treeview->append_column("", _shortcutModelColumns.desc);

    treeview->expand_all();
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->add(*treeview);

    Gtk::VBox* vbox_buttons = Gtk::manage(new Gtk::VBox());
    Gtk::Button* button_edit = Gtk::manage(new Gtk::Button());
    button_edit->set_image(*_pCtMainWin->new_image_from_stock("ct_edit",  Gtk::ICON_SIZE_BUTTON));
    button_edit->set_tooltip_text(_("Change Selected"));
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock("ct_undo",  Gtk::ICON_SIZE_BUTTON));
    button_reset->set_tooltip_text(_("Reset to Default"));
    vbox_buttons->pack_start(*button_edit, false, false);
    vbox_buttons->pack_start(*Gtk::manage(new Gtk::Label()), true, true);
    vbox_buttons->pack_start(*button_reset, false, false);
    Gtk::HBox* hbox_main = Gtk::manage(new Gtk::HBox());
    hbox_main->pack_start(*scrolledwindow, true, true);
    hbox_main->pack_start(*vbox_buttons, false, false);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->add(*hbox_main);

    button_edit->signal_clicked().connect([this, treeview](){
        if (edit_shortcut(treeview))
            need_restart(RESTART_REASON::SHORTCUT);
    });
    button_reset->signal_clicked().connect([this, pConfig, treestore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->customKbShortcuts.clear();
            fill_shortcut_model(treestore);
            need_restart(RESTART_REASON::SHORTCUT);
        }
    });
    treeview->signal_row_activated().connect([button_edit](const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*){
        button_edit->clicked();
    });
    auto button_edit_test_sensitive = [this, button_edit, treeview](){
        auto iter_sel = treeview->get_selection()->get_selected();
        button_edit->set_sensitive(iter_sel and not iter_sel->get_value(_shortcutModelColumns.key).empty());
    };
    treeview->signal_cursor_changed().connect(button_edit_test_sensitive);
    button_edit_test_sensitive();

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_misc()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_system_tray = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_systray = Gtk::manage(new Gtk::CheckButton(_("Enable System Tray Docking")));
    Gtk::CheckButton* checkbutton_start_on_systray = Gtk::manage(new Gtk::CheckButton(_("Start Minimized in the System Tray")));
    vbox_system_tray->pack_start(*checkbutton_systray, false, false);
    vbox_system_tray->pack_start(*checkbutton_start_on_systray, false, false);

    Gtk::Frame* frame_system_tray = new_managed_frame_with_align(_("System Tray"), vbox_system_tray);

    checkbutton_systray->set_active(pConfig->systrayOn);
    checkbutton_start_on_systray->set_active(pConfig->startOnSystray);
    checkbutton_start_on_systray->set_sensitive(pConfig->systrayOn);

    Gtk::VBox* vbox_saving = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_autosave = Gtk::manage(new Gtk::HBox());
    hbox_autosave->set_spacing(4);
    Gtk::CheckButton* checkbutton_autosave = Gtk::manage(new Gtk::CheckButton(_("Autosave Every")));
    Glib::RefPtr<Gtk::Adjustment> adjustment_autosave = Gtk::Adjustment::create(pConfig->autosaveVal, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_autosave = Gtk::manage(new Gtk::SpinButton(adjustment_autosave));
    Gtk::Label* label_autosave = Gtk::manage(new Gtk::Label(_("Minutes")));
    hbox_autosave->pack_start(*checkbutton_autosave, false, false);
    hbox_autosave->pack_start(*spinbutton_autosave, false, false);
    hbox_autosave->pack_start(*label_autosave, false, false);
    Gtk::CheckButton* checkbutton_autosave_on_quit = Gtk::manage(new Gtk::CheckButton(_("Autosave on Quit")));
    Gtk::CheckButton* checkbutton_backup_before_saving = Gtk::manage(new Gtk::CheckButton(_("Create a Backup Copy Before Saving")));
    Gtk::HBox* hbox_num_backups = Gtk::manage(new Gtk::HBox());
    hbox_num_backups->set_spacing(4);
    Gtk::Label* label_num_backups = Gtk::manage(new Gtk::Label(_("Number of Backups to Keep")));
    Glib::RefPtr<Gtk::Adjustment> adjustment_num_backups = Gtk::Adjustment::create(pConfig->backupNum, 1, 100, 1);
    Gtk::SpinButton* spinbutton_num_backups = Gtk::manage(new Gtk::SpinButton(adjustment_num_backups));
    spinbutton_num_backups->set_sensitive(pConfig->backupCopy);
    spinbutton_num_backups->set_value(pConfig->backupNum);
    Gtk::CheckButton* checkbutton_custom_backup_dir = Gtk::manage(new Gtk::CheckButton(_("Custom Backup Directory")));
    Gtk::Entry* entry_custom_backup_dir = Gtk::manage(new Gtk::Entry());
    entry_custom_backup_dir->property_editable() = false;
    Gtk::Button* button_custom_backup_dir = Gtk::manage(new Gtk::Button("..."));
    Gtk::HBox* hbox_custom_backup_dir = Gtk::manage(new Gtk::HBox());
    hbox_custom_backup_dir->set_spacing(4);

    hbox_num_backups->pack_start(*label_num_backups, false, false);
    hbox_num_backups->pack_start(*spinbutton_num_backups, false, false);
    hbox_custom_backup_dir->pack_start(*entry_custom_backup_dir);
    hbox_custom_backup_dir->pack_start(*button_custom_backup_dir, false, false);
    vbox_saving->pack_start(*hbox_autosave, false, false);
    vbox_saving->pack_start(*checkbutton_autosave_on_quit, false, false);
    vbox_saving->pack_start(*checkbutton_backup_before_saving, false, false);
    vbox_saving->pack_start(*hbox_num_backups, false, false);
    vbox_saving->pack_start(*checkbutton_custom_backup_dir, false, false);
    vbox_saving->pack_start(*hbox_custom_backup_dir, false, false);

    checkbutton_autosave->set_active(pConfig->autosaveOn);
    spinbutton_autosave->set_value(pConfig->autosaveVal);
    spinbutton_autosave->set_sensitive(pConfig->autosaveOn);
    checkbutton_autosave_on_quit->set_active(pConfig->autosaveOnQuit);
    checkbutton_backup_before_saving->set_active(pConfig->backupCopy);
    checkbutton_custom_backup_dir->set_sensitive(pConfig->backupCopy);
    checkbutton_custom_backup_dir->set_active(pConfig->customBackupDirOn);
    entry_custom_backup_dir->set_text(pConfig->customBackupDir);
    entry_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);
    button_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);

    Gtk::Frame* frame_saving = new_managed_frame_with_align(_("Saving"), vbox_saving);

    Gtk::VBox* vbox_misc_misc = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_newer_version = Gtk::manage(new Gtk::CheckButton(_("Automatically Check for Newer Version")));
    Gtk::CheckButton* checkbutton_word_count = Gtk::manage(new Gtk::CheckButton(_("Enable Word Count in Statusbar")));
    Gtk::CheckButton* checkbutton_reload_doc_last = Gtk::manage(new Gtk::CheckButton(_("Reload Document From Last Session")));
    Gtk::CheckButton* checkbutton_mod_time_sentinel = Gtk::manage(new Gtk::CheckButton(_("Reload After External Update to CT* File")));
    Gtk::CheckButton* checkbutton_win_title_doc_dir = Gtk::manage(new Gtk::CheckButton(_("Show the Document Directory in the Window Title")));
    vbox_misc_misc->pack_start(*checkbutton_newer_version, false, false);
    vbox_misc_misc->pack_start(*checkbutton_word_count, false, false);
    vbox_misc_misc->pack_start(*checkbutton_reload_doc_last, false, false);
    vbox_misc_misc->pack_start(*checkbutton_mod_time_sentinel, false, false);
    vbox_misc_misc->pack_start(*checkbutton_win_title_doc_dir, false, false);

    checkbutton_newer_version->set_active(pConfig->checkVersion);
    checkbutton_word_count->set_active(pConfig->wordCountOn);
    checkbutton_reload_doc_last->set_active(pConfig->reloadDocLast);
    checkbutton_mod_time_sentinel->set_active(pConfig->modTimeSentinel);
    checkbutton_win_title_doc_dir->set_active(pConfig->winTitleShowDocDir);

    Gtk::Frame* frame_misc_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_misc);

#ifdef HAVE_NLS
    Gtk::VBox* vbox_language = Gtk::manage(new Gtk::VBox());
    Gtk::ComboBoxText* combobox_country_language = Gtk::manage(new Gtk::ComboBoxText());
    combobox_country_language->append(CtConst::LANG_DEFAULT, "-");
    combobox_country_language->append("bg", _("Bulgarian"));
    combobox_country_language->append("cs", _("Czech"));
    combobox_country_language->append("de", _("German"));
    combobox_country_language->append("el", _("Greek"));
    combobox_country_language->append("en", _("English"));
    combobox_country_language->append("es", _("Spanish"));
    combobox_country_language->append("fi", _("Finnish"));
    combobox_country_language->append("fr", _("French"));
    combobox_country_language->append("hy", _("Armenian"));
    combobox_country_language->append("it", _("Italian"));
    combobox_country_language->append("ja", _("Japanese"));
    combobox_country_language->append("lt", _("Lithuanian"));
    combobox_country_language->append("nl", _("Dutch"));
    combobox_country_language->append("pl", _("Polish"));
    combobox_country_language->append("pt_BR", _("Portuguese Brazil"));
    combobox_country_language->append("ru", _("Russian"));
    combobox_country_language->append("sl", _("Slovenian"));
    combobox_country_language->append("sv", _("Swedish"));
    combobox_country_language->append("tr", _("Turkish"));
    combobox_country_language->append("uk", _("Ukrainian"));
    combobox_country_language->append("zh_CN", _("Chinese Simplified"));
    combobox_country_language->set_active_id(CtMiscUtil::get_ct_language());
    vbox_language->pack_start(*combobox_country_language, false, false);
    Gtk::Frame* frame_language = new_managed_frame_with_align(_("Language"), vbox_language);
#endif

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_system_tray, false, false);
    pMainBox->pack_start(*frame_saving, false, false);
    pMainBox->pack_start(*frame_misc_misc, false, false);
#ifdef HAVE_NLS
    pMainBox->pack_start(*frame_language, false, false);
#endif

    // cannot just turn on systray icon, we have to check if systray exists
    checkbutton_systray->signal_toggled().connect([this, pConfig, checkbutton_systray, checkbutton_start_on_systray](){
        if (checkbutton_systray->get_active()) {
            _pCtMainWin->get_status_icon()->set_visible(true);
            pConfig->systrayOn = CtDialogs::question_dialog(_("Has the System Tray appeared on the panel?"), *this);
            if (pConfig->systrayOn) {
                checkbutton_start_on_systray->set_sensitive(true);
                apply_for_each_window([](CtMainWin* win) { win->menu_set_visible_exit_app(true); });
            }
            else {
                CtDialogs::warning_dialog(_("Your system does not support the System Tray"), *_pCtMainWin);
                checkbutton_systray->set_active(false);
            }
        } else {
            pConfig->systrayOn = false;
            _pCtMainWin->get_status_icon()->set_visible(false);
            apply_for_each_window([](CtMainWin* win) { win->menu_set_visible_exit_app(false); });
            checkbutton_start_on_systray->set_sensitive(false);
        }
        checkbutton_systray->get_parent()->grab_focus();
    });
    checkbutton_start_on_systray->signal_toggled().connect([pConfig, checkbutton_start_on_systray](){
        pConfig->startOnSystray = checkbutton_start_on_systray->get_active();
    });
    checkbutton_autosave->signal_toggled().connect([this, pConfig, checkbutton_autosave, spinbutton_autosave](){
        pConfig->autosaveOn = checkbutton_autosave->get_active();
        _pCtMainWin->file_autosave_restart();
        spinbutton_autosave->set_sensitive(pConfig->autosaveOn);
    });
    spinbutton_autosave->signal_value_changed().connect([this, pConfig, spinbutton_autosave](){
        pConfig->autosaveVal = spinbutton_autosave->get_value_as_int();
        _pCtMainWin->file_autosave_restart();
    });
    checkbutton_autosave_on_quit->signal_toggled().connect([pConfig, checkbutton_autosave_on_quit](){
        pConfig->autosaveOnQuit = checkbutton_autosave_on_quit->get_active();
    });
    checkbutton_backup_before_saving->signal_toggled().connect([pConfig, checkbutton_backup_before_saving, spinbutton_num_backups, checkbutton_custom_backup_dir, entry_custom_backup_dir, button_custom_backup_dir](){
        pConfig->backupCopy = checkbutton_backup_before_saving->get_active();
        spinbutton_num_backups->set_sensitive(pConfig->backupCopy);
        checkbutton_custom_backup_dir->set_sensitive(pConfig->backupCopy);
        entry_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);
        button_custom_backup_dir->set_sensitive(pConfig->backupCopy && pConfig->customBackupDirOn);
    });
    spinbutton_num_backups->signal_value_changed().connect([pConfig, spinbutton_num_backups](){
        pConfig->backupNum = spinbutton_num_backups->get_value_as_int();
    });
    checkbutton_custom_backup_dir->signal_toggled().connect([pConfig, checkbutton_custom_backup_dir, entry_custom_backup_dir, button_custom_backup_dir](){
        pConfig->customBackupDirOn = checkbutton_custom_backup_dir->get_active();
        entry_custom_backup_dir->set_sensitive(checkbutton_custom_backup_dir->get_active());
        button_custom_backup_dir->set_sensitive(checkbutton_custom_backup_dir->get_active());
    });
    button_custom_backup_dir->signal_clicked().connect([this, pConfig, entry_custom_backup_dir](){
        auto dir_place = CtDialogs::folder_select_dialog(pConfig->customBackupDir, _pCtMainWin);
        if (dir_place.empty()) return;
        entry_custom_backup_dir->set_text(dir_place);
        pConfig->customBackupDir = dir_place;
    });
    checkbutton_reload_doc_last->signal_toggled().connect([pConfig, checkbutton_reload_doc_last](){
        pConfig->reloadDocLast = checkbutton_reload_doc_last->get_active();
    });
    checkbutton_mod_time_sentinel->signal_toggled().connect([this, pConfig, checkbutton_mod_time_sentinel](){
        pConfig->modTimeSentinel = checkbutton_mod_time_sentinel->get_active();
        _pCtMainWin->mod_time_sentinel_restart();
    });
    checkbutton_win_title_doc_dir->signal_toggled().connect([this, pConfig, checkbutton_win_title_doc_dir](){
        pConfig->winTitleShowDocDir = checkbutton_win_title_doc_dir->get_active();
        _pCtMainWin->window_title_update();
    });
    checkbutton_newer_version->signal_toggled().connect([pConfig, checkbutton_newer_version](){
        pConfig->checkVersion = checkbutton_newer_version->get_active();
    });
    checkbutton_word_count->signal_toggled().connect([this, pConfig, checkbutton_word_count](){
        pConfig->wordCountOn = checkbutton_word_count->get_active();
        apply_for_each_window([](CtMainWin* win) { win->update_selected_node_statusbar_info(); });
    });
#ifdef HAVE_NLS
    combobox_country_language->signal_changed().connect([this, combobox_country_language](){
        Glib::ustring new_lang = combobox_country_language->get_active_id();
        need_restart(RESTART_REASON::LANG, _("The New Language will be Available Only After Restarting CherryTree"));
        g_file_set_contents(fs::get_cherrytree_lang_filepath().c_str(),
                            new_lang.c_str(), (gssize)new_lang.bytes(), nullptr);
    });
#endif

    return pMainBox;
}

void CtPrefDlg::need_restart(RESTART_REASON reason, const gchar* msg /*= nullptr*/)
{
    if (!(_restartReasons & (int)reason)) {
        _restartReasons |= (int)reason;
        CtDialogs::info_dialog(msg ? msg : _("This Change will have Effect Only After Restarting CherryTree"), *this);
    }
}

std::string CtPrefDlg::get_code_exec_term_run(CtMainWin* pCtMainWin)
{
    if (!pCtMainWin->get_ct_config()->customCodexecTerm.empty())
    {
        return pCtMainWin->get_ct_config()->customCodexecTerm;
    }
    const std::string op_sys =
#ifdef _WIN32
        "win";
#else
        "linux";
#endif
    return CtConst::CODE_EXEC_TERM_RUN_DEFAULT.at(op_sys);
}

std::set<std::string> CtPrefDlg::_get_code_exec_type_keys()
{
    std::set<std::string> retSetCodexecTypeKeys;
    for (const auto& it : _pCtMainWin->get_ct_config()->customCodexecType) {
        retSetCodexecTypeKeys.insert(it.first);
    }
    for (const auto& it : CtConst::CODE_EXEC_TYPE_CMD_DEFAULT) {
        retSetCodexecTypeKeys.insert(it.first);
    }
    return retSetCodexecTypeKeys;
}

std::string CtPrefDlg::get_code_exec_ext(CtMainWin* pCtMainWin, const std::string code_type)
{
    for (const auto& it : pCtMainWin->get_ct_config()->customCodexecExt) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    for (const auto& it : CtConst::CODE_EXEC_TYPE_EXT_DEFAULT) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    return "txt";
}

std::string CtPrefDlg::get_code_exec_type_cmd(CtMainWin* pCtMainWin, const std::string code_type)
{
    for (const auto& it : pCtMainWin->get_ct_config()->customCodexecType) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    for (const auto& it : CtConst::CODE_EXEC_TYPE_CMD_DEFAULT) {
        if (it.first == code_type) {
            return it.second;
        }
    }
    return std::string{};
}

void CtPrefDlg::_fill_custom_exec_commands_model(Glib::RefPtr<Gtk::ListStore> rModel)
{
    rModel->clear();
    for (const auto& code_type : _get_code_exec_type_keys())
    {
        Gtk::TreeModel::Row row = *(rModel->append());
        row[_commandModelColumns.icon] = _pCtMainWin->get_code_icon_name(code_type);
        row[_commandModelColumns.key] = code_type;
        row[_commandModelColumns.ext] = CtPrefDlg::get_code_exec_ext(_pCtMainWin, code_type);
        row[_commandModelColumns.desc] = CtPrefDlg::get_code_exec_type_cmd(_pCtMainWin, code_type);
    }
}

void CtPrefDlg::_add_new_command_in_model(Gtk::TreeView* pTreeview, Glib::RefPtr<Gtk::ListStore> rModel)
{
    const std::set<std::string> all_codexec_keys = _get_code_exec_type_keys();
    auto itemStore = CtChooseDialogListStore::create();
    for (const auto& lang : _pCtMainWin->get_language_manager()->get_language_ids())
    {
        if (0 == all_codexec_keys.count(lang)) {
            itemStore->add_row(_pCtMainWin->get_code_icon_name(lang), "", lang);
        }
    }
    const Gtk::TreeIter treeIterChosen = CtDialogs::choose_item_dialog(*this, _("Select Element to Add"), itemStore);
    if (treeIterChosen) {
        const auto code_type = treeIterChosen->get_value(itemStore->columns.desc);
        Gtk::TreeIter newTreeIter;
        Gtk::TreeIter loopPrevTreeIter;
        for (const auto& currTreeIter : rModel->children()) {
            const int result = currTreeIter->get_value(_commandModelColumns.key).compare(code_type);
            if (result > 0) {
                newTreeIter = loopPrevTreeIter ? rModel->insert_after(loopPrevTreeIter) : rModel->prepend();
                break;
            }
            loopPrevTreeIter = currTreeIter;
        }
        if (not newTreeIter) {
            newTreeIter = rModel->append();
        }
        Gtk::TreeModel::Row row = *newTreeIter;
        row[_commandModelColumns.icon] = _pCtMainWin->get_code_icon_name(code_type);
        row[_commandModelColumns.key] = code_type;
        row[_commandModelColumns.ext] = CtPrefDlg::get_code_exec_ext(_pCtMainWin, code_type);
        row[_commandModelColumns.desc] = std::string{"REPLACE_ME "} + CtConst::CODE_EXEC_TMP_SRC;
        pTreeview->set_cursor(rModel->get_path(newTreeIter));
    }
}

void CtPrefDlg::_remove_command_from_model(Gtk::TreeView* pTreeview, Glib::RefPtr<Gtk::ListStore> rModel)
{
    Gtk::TreeIter sel_iter = pTreeview->get_selection()->get_selected();
    if (sel_iter) {
        const auto code_type = sel_iter->get_value(_commandModelColumns.key);
        auto& customCodexecType = _pCtMainWin->get_ct_config()->customCodexecType;
        auto& customCodexecExt = _pCtMainWin->get_ct_config()->customCodexecExt;
        if (customCodexecType.count(code_type)) {
            customCodexecType.erase(code_type);
        }
        if (customCodexecExt.count(code_type)) {
            customCodexecExt.erase(code_type);
        }
        rModel->erase(sel_iter);
    }
}

void CtPrefDlg::fill_toolbar_model(Glib::RefPtr<Gtk::ListStore> model)
{
    std::vector<std::string> vecToolbarElements = str::split(_pCtMainWin->get_ct_config()->toolbarUiList, ",");
    model->clear();
    for(const std::string& key: vecToolbarElements)
        add_new_item_in_toolbar_model(model->append(), key);
}

void CtPrefDlg::add_new_item_in_toolbar_model(Gtk::TreeIter row, const Glib::ustring& key)
{
    Glib::ustring icon, desc;
    if (key == CtConst::TAG_SEPARATOR)
    {
        desc = CtConst::TAG_SEPARATOR_ANSI_REPR;
    }
    else if (key == CtConst::TOOLBAR_SPLIT)
    {
        desc = _("Split Toolbar");
    }
    else if (key == CtConst::CHAR_STAR)
    {
        icon = "ct_open";
        desc = _("Open a CherryTree Document");

    }
    else if (CtMenuAction const* action = _pCtMenu->find_action(key))
    {
        icon = action->image;
        desc = action->desc;
    }

    row->set_value(_toolbarModelColumns.icon, icon);
    row->set_value(_toolbarModelColumns.key, key);
    row->set_value(_toolbarModelColumns.desc, desc);
}

bool CtPrefDlg::add_new_item_in_toolbar_model(Gtk::TreeView* treeview, Glib::RefPtr<Gtk::ListStore> model)
{
    auto itemStore = CtChooseDialogListStore::create();
    itemStore->add_row("", CtConst::TAG_SEPARATOR, CtConst::TAG_SEPARATOR_ANSI_REPR);
    itemStore->add_row("", CtConst::TOOLBAR_SPLIT, _("Split Toolbar"));
    for (const CtMenuAction& action: _pCtMenu->get_actions())
    {
        if (action.desc.empty()) continue; // skip stub menu entries
        if (action.id == "ct_open_file" && _pCtMainWin->get_ct_config()->toolbarUiList.find(CtConst::CHAR_STAR) != std::string::npos) continue;
        if (vec::exists(CtConst::TOOLBAR_VEC_BLACKLIST, action.id)) continue;
        Glib::ustring id = action.id == "ct_open_file" ? CtConst::CHAR_STAR : action.id;
        itemStore->add_row(action.image, id, action.desc);
    }

    auto chosen_row = CtDialogs::choose_item_dialog(*this, _("Select Element to Add"), itemStore);
    if (chosen_row) {
        auto selected_row = treeview->get_selection()->get_selected();
        auto new_row = selected_row ? model->insert_after(*selected_row) : model->append();
        add_new_item_in_toolbar_model(new_row, chosen_row->get_value(itemStore->columns.key));
        return true;
    }
    return false;
}

void CtPrefDlg::update_config_toolbar_from_model(Glib::RefPtr<Gtk::ListStore> model)
{
    std::vector<std::string> items;
    for (auto it: model->children())
        items.push_back(it.get_value(_toolbarModelColumns.key));
    _pCtMainWin->get_ct_config()->toolbarUiList = str::join(items, ",");
}

void CtPrefDlg::fill_shortcut_model(Glib::RefPtr<Gtk::TreeStore> model)
{
    model->clear();
    std::string category_name = "no name";
    Gtk::TreeModel::Row cat_row;
    for(const CtMenuAction& action: _pCtMenu->get_actions())
    {
        if (action.category.empty()) continue;
        if (action.category != category_name) {
            category_name = action.category;
            cat_row = *model->append();
            cat_row[_shortcutModelColumns.desc] = action.category;
        }
        auto row = *model->append(cat_row.children());
        row[_shortcutModelColumns.icon] = action.image;
        row[_shortcutModelColumns.key] = action.id;
        row[_shortcutModelColumns.desc] = action.desc;
        row[_shortcutModelColumns.shortcut] = action.get_shortcut(_pCtMainWin->get_ct_config());
    }
}

bool CtPrefDlg::edit_shortcut(Gtk::TreeView* treeview)
{
    auto tree_iter = treeview->get_selection()->get_selected();
    if (!tree_iter || tree_iter->get_value(_shortcutModelColumns.key).empty()) return false;
    std::string shortcut = tree_iter->get_value(_shortcutModelColumns.shortcut);
    std::string id = tree_iter->get_value(_shortcutModelColumns.key);
    if (edit_shortcut_dialog(shortcut)) {
        if (shortcut != "") {
            for(const CtMenuAction& action : _pCtMenu->get_actions())
                if (action.get_shortcut(_pCtMainWin->get_ct_config()) == shortcut && action.id != id) {
                    // todo: this is a shorter version from python code
                    std::string message = "<b>" + str::format(_("The Keyboard Shortcut '%s' is already in use"), CtStrUtil::get_accelerator_label(shortcut)) + "</b>\n\n";
                    message += str::format(_("The current associated action is '%s'"), str::replace(action.name, "_", "")) + "\n\n";
                    message += "<b>" + std::string(_("Do you want to steal the shortcut?")) + "</b>";
                    if (!CtDialogs::question_dialog(message, *this))
                        return false;
                    _pCtMainWin->get_ct_config()->customKbShortcuts[action.id] = "";
                }
        }
        _pCtMainWin->get_ct_config()->customKbShortcuts[id] = shortcut;
        return true;
    }
    return false;
}

bool CtPrefDlg::edit_shortcut_dialog(std::string& shortcut)
{
    std::string kb_shortcut_key = shortcut;
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_CONTROL.c_str(), "");
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_SHIFT.c_str(), "");
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_ALT.c_str(), "");
    kb_shortcut_key = str::replace(kb_shortcut_key, _pCtMenu->KB_META.c_str(), "");

    Gtk::Dialog dialog(_("Edit Keyboard Shortcut"), *this, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 100);
    Gtk::RadioButton* radiobutton_kb_none = Gtk::manage(new Gtk::RadioButton(_("No Keyboard Shortcut")));
    Gtk::RadioButton* radiobutton_kb_shortcut = Gtk::manage(new Gtk::RadioButton());
    radiobutton_kb_shortcut->join_group(*radiobutton_kb_none);
    Gtk::ToggleButton* ctrl_toggle = Gtk::manage(new Gtk::ToggleButton("control"));
    Gtk::ToggleButton* shift_toggle = Gtk::manage(new Gtk::ToggleButton("shift"));
    Gtk::ToggleButton* alt_toggle = Gtk::manage(new Gtk::ToggleButton("alt"));
    Gtk::ToggleButton* meta_toggle = Gtk::manage(new Gtk::ToggleButton("meta"));
    ctrl_toggle->set_size_request(70, 1);
    shift_toggle->set_size_request(70, 1);
    alt_toggle->set_size_request(70, 1);
    meta_toggle->set_size_request(70,1);
    Gtk::Entry* key_entry = Gtk::manage(new Gtk::Entry());
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    hbox->pack_start(*radiobutton_kb_shortcut);
    hbox->pack_start(*ctrl_toggle);
    hbox->pack_start(*shift_toggle);
    hbox->pack_start(*alt_toggle);
    hbox->pack_start(*meta_toggle);
    hbox->pack_start(*key_entry);
    hbox->set_spacing(5);
    vbox->pack_start(*radiobutton_kb_none);
    vbox->pack_start(*hbox);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(*vbox);

    auto b1 = Glib::Binding::bind_property(key_entry->property_sensitive(), ctrl_toggle->property_sensitive());
    auto b2 = Glib::Binding::bind_property(key_entry->property_sensitive(), shift_toggle->property_sensitive());
    auto b3 = Glib::Binding::bind_property(key_entry->property_sensitive(), alt_toggle->property_sensitive());
    auto b5 = Glib::Binding::bind_property(key_entry->property_sensitive(), meta_toggle->property_sensitive());
    auto b4 = Glib::Binding::bind_property(radiobutton_kb_shortcut->property_active(), key_entry->property_sensitive());

    key_entry->set_sensitive(!kb_shortcut_key.empty());
    key_entry->set_text(kb_shortcut_key);
    radiobutton_kb_none->set_active(kb_shortcut_key.empty());
    radiobutton_kb_shortcut->set_active(!kb_shortcut_key.empty());
    ctrl_toggle->set_active(shortcut.find(_pCtMenu->KB_CONTROL) != std::string::npos);
    shift_toggle->set_active(shortcut.find(_pCtMenu->KB_SHIFT) != std::string::npos);
    alt_toggle->set_active(shortcut.find(_pCtMenu->KB_ALT) != std::string::npos);
    meta_toggle->set_active(shortcut.find(_pCtMenu->KB_META) != std::string::npos);

    key_entry->signal_key_press_event().connect([key_entry](GdkEventKey* key)->bool{
        Glib::ustring keyname = gdk_keyval_name(key->keyval);
        if (keyname.size() == 1) keyname = keyname.uppercase();
        key_entry->set_text(keyname);
        key_entry->select_region(0, (int)keyname.size());
        return true;
    }, false);

    content_area->show_all();
    if (dialog.run() != Gtk::RESPONSE_ACCEPT)
        return false;

    shortcut = "";
    if (radiobutton_kb_shortcut->get_active() && !key_entry->get_text().empty()) {
        if (ctrl_toggle->get_active()) shortcut += _pCtMenu->KB_CONTROL;
        if (shift_toggle->get_active()) shortcut += _pCtMenu->KB_SHIFT;
        if (alt_toggle->get_active()) shortcut += _pCtMenu->KB_ALT;
        if (meta_toggle->get_active()) shortcut += _pCtMenu->KB_META;
        shortcut += key_entry->get_text();
    }
    return true;
}

void CtPrefDlg::apply_for_each_window(std::function<void(CtMainWin*)> callback)
{
    _pCtMainWin->signal_app_apply_for_each_window(callback);
}
