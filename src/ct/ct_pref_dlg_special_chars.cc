/*
 * ct_pref_dlg_special_chars.cc
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

    textview_special_chars->get_buffer()->signal_changed().connect([pConfig, textview_special_chars](){
        Glib::ustring new_special_chars = textview_special_chars->get_buffer()->get_text();
        new_special_chars = str::replace(new_special_chars, CtConst::CHAR_NEWLINE, "");
        if (pConfig->specialChars.item() != new_special_chars) {
            pConfig->specialChars = new_special_chars;
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
