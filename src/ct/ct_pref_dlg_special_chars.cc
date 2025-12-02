/*
 * ct_pref_dlg_special_chars.cc
 *
 * Copyright 2009-2025
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
    auto hbox_special_chars = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto vbox_special_chars = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    Gtk::Label* label_special_chars = Gtk::manage(new Gtk::Label(_("Special Characters")));
    auto hbox_reset = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
#if GTKMM_MAJOR_VERSION < 4
    button_reset->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
    button_reset->set_tooltip_text(_("Reset to Default"));
#if GTKMM_MAJOR_VERSION >= 4
    hbox_reset->append(*Gtk::manage(new Gtk::Label()));
    hbox_reset->append(*button_reset);
    hbox_reset->append(*Gtk::manage(new Gtk::Label()));
    vbox_special_chars->append(*Gtk::manage(new Gtk::Label()));
    vbox_special_chars->append(*label_special_chars);
    vbox_special_chars->append(*hbox_reset);
    vbox_special_chars->append(*Gtk::manage(new Gtk::Label()));
#else
    hbox_reset->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    hbox_reset->pack_start(*button_reset, false, false);
    hbox_reset->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    vbox_special_chars->pack_start(*Gtk::manage(new Gtk::Label()), false, false);
    vbox_special_chars->pack_start(*label_special_chars, false, false);
    vbox_special_chars->pack_start(*hbox_reset, false, false);
    vbox_special_chars->pack_start(*Gtk::manage(new Gtk::Label()), false, false);
#endif
    Gtk::Frame* frame_special_chars = Gtk::manage(new Gtk::Frame());
    frame_special_chars->set_size_request(-1, 80);
    
    Gtk::ScrolledWindow* scrolledwindow_special_chars = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow_special_chars->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
#if GTKMM_MAJOR_VERSION >= 4
    frame_special_chars->set_child(*scrolledwindow_special_chars);
#else
    frame_special_chars->set_shadow_type(Gtk::SHADOW_IN);
    frame_special_chars->add(*scrolledwindow_special_chars);
#endif
    Gtk::TextView* textview_special_chars = Gtk::manage(new Gtk::TextView());
    textview_special_chars->get_buffer()->set_text(_pConfig->specialChars.item());
    
#if GTKMM_MAJOR_VERSION >= 4
    textview_special_chars->set_wrap_mode(Gtk::WrapMode::CHAR);
#else
    textview_special_chars->set_wrap_mode(Gtk::WRAP_CHAR);
#endif
#if GTKMM_MAJOR_VERSION >= 4
    scrolledwindow_special_chars->set_child(*textview_special_chars);
    hbox_special_chars->append(*vbox_special_chars);
    hbox_special_chars->append(*frame_special_chars);
#else
    scrolledwindow_special_chars->add(*textview_special_chars);
    hbox_special_chars->pack_start(*vbox_special_chars, false, false);
    hbox_special_chars->pack_start(*frame_special_chars);
#endif

    auto hbox_bullist_chars = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    Gtk::Label* label_bullist_chars = Gtk::manage(new Gtk::Label(_("Chars for Bulleted List")));
    Gtk::Entry* entry_bullist_chars = Gtk::manage(new Gtk::Entry());
        entry_bullist_chars->set_icon_from_icon_name("ct_undo",
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Entry::IconPosition::SECONDARY
    #else
        Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
    #endif
        );
    entry_bullist_chars->set_text(_pConfig->charsListbul.item());
#if GTKMM_MAJOR_VERSION >= 4
    hbox_bullist_chars->append(*label_bullist_chars);
    hbox_bullist_chars->append(*entry_bullist_chars);
#else
    hbox_bullist_chars->pack_start(*label_bullist_chars, false, false);
    hbox_bullist_chars->pack_start(*entry_bullist_chars);
#endif

    auto hbox_todolist_chars = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    Gtk::Label* label_todolist_chars = Gtk::manage(new Gtk::Label(_("Chars for Todo List")));
    Gtk::Entry* entry_todolist_chars = Gtk::manage(new Gtk::Entry());
        entry_todolist_chars->set_icon_from_icon_name("ct_undo",
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Entry::IconPosition::SECONDARY
    #else
        Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
    #endif
        );
    entry_todolist_chars->set_text(_pConfig->charsTodo.item());
#if GTKMM_MAJOR_VERSION >= 4
    hbox_todolist_chars->append(*label_todolist_chars);
    hbox_todolist_chars->append(*entry_todolist_chars);
#else
    hbox_todolist_chars->pack_start(*label_todolist_chars, false, false);
    hbox_todolist_chars->pack_start(*entry_todolist_chars);
#endif

    auto hbox_toc_chars = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    Gtk::Label* label_toc_chars = Gtk::manage(new Gtk::Label(_("Chars for Table Of Content")));
    Gtk::Entry* entry_toc_chars = Gtk::manage(new Gtk::Entry());
        entry_toc_chars->set_icon_from_icon_name("ct_undo",
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Entry::IconPosition::SECONDARY
    #else
        Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
    #endif
        );
    entry_toc_chars->set_text(_pConfig->charsToc.item());
#if GTKMM_MAJOR_VERSION >= 4
    hbox_toc_chars->append(*label_toc_chars);
    hbox_toc_chars->append(*entry_toc_chars);
#else
    hbox_toc_chars->pack_start(*label_toc_chars, false, false);
    hbox_toc_chars->pack_start(*entry_toc_chars);
#endif

    auto hbox_dquote_chars = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    Gtk::Label* label_dquote_chars = Gtk::manage(new Gtk::Label(_("Chars for Smart Double Quotes")));
    Gtk::Entry* entry_dquote_chars = Gtk::manage(new Gtk::Entry());
        entry_dquote_chars->set_icon_from_icon_name("ct_undo",
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Entry::IconPosition::SECONDARY
    #else
        Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
    #endif
        );
    entry_dquote_chars->set_text(_pConfig->chars_smart_dquote.item());
#if GTKMM_MAJOR_VERSION >= 4
    hbox_dquote_chars->append(*label_dquote_chars);
    hbox_dquote_chars->append(*entry_dquote_chars);
#else
    hbox_dquote_chars->pack_start(*label_dquote_chars, false, false);
    hbox_dquote_chars->pack_start(*entry_dquote_chars);
#endif

    auto hbox_squote_chars = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    Gtk::Label* label_squote_chars = Gtk::manage(new Gtk::Label(_("Chars for Smart Single Quotes")));
    Gtk::Entry* entry_squote_chars = Gtk::manage(new Gtk::Entry());
        entry_squote_chars->set_icon_from_icon_name("ct_undo",
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Entry::IconPosition::SECONDARY
    #else
        Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
    #endif
        );
    entry_squote_chars->set_text(_pConfig->chars_smart_squote.item());
#if GTKMM_MAJOR_VERSION >= 4
    hbox_squote_chars->append(*label_squote_chars);
    hbox_squote_chars->append(*entry_squote_chars);
#else
    hbox_squote_chars->pack_start(*label_squote_chars, false, false);
    hbox_squote_chars->pack_start(*entry_squote_chars);
#endif

    auto vbox_editor = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto checkbutton_auto_smart_quotes = Gtk::manage(new Gtk::CheckButton{_("Enable Smart Quotes Auto Replacement")});
    checkbutton_auto_smart_quotes->set_active(_pConfig->autoSmartQuotes);
    auto hbox_symbol_autoreplace = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto checkbutton_enable_symbol_autoreplace = Gtk::manage(new Gtk::CheckButton{_("Enable Symbol Auto Replacement")});
    checkbutton_enable_symbol_autoreplace->set_active(_pConfig->enableSymbolAutoreplace);
    auto button_symbol_autoreplace_help = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_symbol_autoreplace_help->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_help", Gtk::ICON_SIZE_BUTTON));
#endif
    button_symbol_autoreplace_help->set_tooltip_text(_("Supported Symbols Auto Replacements"));
#if GTKMM_MAJOR_VERSION >= 4
    hbox_symbol_autoreplace->append(*checkbutton_enable_symbol_autoreplace);
    hbox_symbol_autoreplace->append(*button_symbol_autoreplace_help);
#else
    hbox_symbol_autoreplace->pack_start(*checkbutton_enable_symbol_autoreplace, false, false);
    hbox_symbol_autoreplace->pack_start(*button_symbol_autoreplace_help, false, false);
#endif

#if GTKMM_MAJOR_VERSION >= 4
    vbox_editor->append(*hbox_special_chars);
    vbox_editor->append(*hbox_bullist_chars);
    vbox_editor->append(*hbox_todolist_chars);
    vbox_editor->append(*hbox_toc_chars);
    vbox_editor->append(*hbox_dquote_chars);
    vbox_editor->append(*hbox_squote_chars);
    vbox_editor->append(*checkbutton_auto_smart_quotes);
    vbox_editor->append(*hbox_symbol_autoreplace);
#else
    vbox_editor->pack_start(*hbox_special_chars, false, false);
    vbox_editor->pack_start(*hbox_bullist_chars, false, false);
    vbox_editor->pack_start(*hbox_todolist_chars, false, false);
    vbox_editor->pack_start(*hbox_toc_chars, false, false);
    vbox_editor->pack_start(*hbox_dquote_chars, false, false);
    vbox_editor->pack_start(*hbox_squote_chars, false, false);
    vbox_editor->pack_start(*checkbutton_auto_smart_quotes, false, false);
    vbox_editor->pack_start(*hbox_symbol_autoreplace, false, false);
#endif

    Gtk::Frame* frame_editor = new_managed_frame_with_align(_("Text Editor"), vbox_editor);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_start(6);
    pMainBox->set_margin_top(6);
#if GTKMM_MAJOR_VERSION >= 4
    pMainBox->append(*frame_editor);
#else
    pMainBox->pack_start(*frame_editor, false, false);
#endif

    textview_special_chars->get_buffer()->signal_changed().connect([this, textview_special_chars](){
        Glib::ustring new_special_chars = textview_special_chars->get_buffer()->get_text();
        new_special_chars = str::replace(new_special_chars, CtConst::CHAR_NEWLINE, "");
        if (_pConfig->specialChars.item() != new_special_chars) {
            _pConfig->specialChars = new_special_chars;
        }
    });
    button_reset->signal_clicked().connect([this, textview_special_chars](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            textview_special_chars->get_buffer()->set_text(CtConst::SPECIAL_CHARS_DEFAULT);
        }
    });

    entry_bullist_chars->signal_changed().connect([this, entry_bullist_chars](){
        if (entry_bullist_chars->get_text().size() >= CtConst::CHARS_LISTBUL_DEFAULT.size()) {
            _pConfig->charsListbul = entry_bullist_chars->get_text();
            entry_bullist_chars->set_icon_from_icon_name("ct_undo",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
        else {
            entry_bullist_chars->set_icon_from_icon_name("ct_urgent",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
    });
#if GTKMM_MAJOR_VERSION >= 4
    entry_bullist_chars->signal_icon_release().connect([entry_bullist_chars](Gtk::Entry::IconPosition /*icon_position*/){
#else
    entry_bullist_chars->signal_icon_release().connect([entry_bullist_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
#endif
        entry_bullist_chars->set_text(CtConst::CHARS_LISTBUL_DEFAULT);
    });
    entry_todolist_chars->signal_changed().connect([this, entry_todolist_chars](){
        if (entry_todolist_chars->get_text().size() == CtConst::CHARS_TODO_DEFAULT.size()) {
            _pConfig->charsTodo = entry_todolist_chars->get_text();
            entry_todolist_chars->set_icon_from_icon_name("ct_undo",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
        else {
            entry_todolist_chars->set_icon_from_icon_name("ct_urgent",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
    });
#if GTKMM_MAJOR_VERSION >= 4
    entry_todolist_chars->signal_icon_release().connect([entry_todolist_chars](Gtk::Entry::IconPosition /*icon_position*/){
#else
    entry_todolist_chars->signal_icon_release().connect([entry_todolist_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
#endif
        entry_todolist_chars->set_text(CtConst::CHARS_TODO_DEFAULT);
    });
    entry_toc_chars->signal_changed().connect([this, entry_toc_chars](){
        if (entry_toc_chars->get_text().size() >= CtConst::CHARS_TOC_DEFAULT.size()) {
            _pConfig->charsToc = entry_toc_chars->get_text();
            entry_toc_chars->set_icon_from_icon_name("ct_undo",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
        else {
            entry_toc_chars->set_icon_from_icon_name("ct_urgent",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
    });
#if GTKMM_MAJOR_VERSION >= 4
    entry_toc_chars->signal_icon_release().connect([entry_toc_chars](Gtk::Entry::IconPosition /*icon_position*/){
#else
    entry_toc_chars->signal_icon_release().connect([entry_toc_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
#endif
        entry_toc_chars->set_text(CtConst::CHARS_TOC_DEFAULT);
    });
    entry_dquote_chars->signal_changed().connect([this, entry_dquote_chars](){
        if (entry_dquote_chars->get_text().size() == CtConst::CHARS_SMART_DQUOTE_DEFAULT.size()) {
            _pConfig->chars_smart_dquote = entry_dquote_chars->get_text();
            entry_dquote_chars->set_icon_from_icon_name("ct_undo",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
        else {
            entry_dquote_chars->set_icon_from_icon_name("ct_urgent",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
    });
#if GTKMM_MAJOR_VERSION >= 4
    entry_dquote_chars->signal_icon_release().connect([entry_dquote_chars](Gtk::Entry::IconPosition /*icon_position*/){
#else
    entry_dquote_chars->signal_icon_release().connect([entry_dquote_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
#endif
        entry_dquote_chars->set_text(CtConst::CHARS_SMART_DQUOTE_DEFAULT);
    });
    entry_squote_chars->signal_changed().connect([this, entry_squote_chars](){
        if (entry_squote_chars->get_text().size() == CtConst::CHARS_SMART_SQUOTE_DEFAULT.size()) {
            _pConfig->chars_smart_squote = entry_squote_chars->get_text();
            entry_squote_chars->set_icon_from_icon_name("ct_undo",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
        else {
            entry_squote_chars->set_icon_from_icon_name("ct_urgent",
#if GTKMM_MAJOR_VERSION >= 4
                Gtk::Entry::IconPosition::SECONDARY
#else
                Gtk::EntryIconPosition::ENTRY_ICON_SECONDARY
#endif
            );
        }
    });
#if GTKMM_MAJOR_VERSION >= 4
    entry_squote_chars->signal_icon_release().connect([entry_squote_chars](Gtk::Entry::IconPosition /*icon_position*/){
#else
    entry_squote_chars->signal_icon_release().connect([entry_squote_chars](Gtk::EntryIconPosition /*icon_position*/, const GdkEventButton* /*event*/){
#endif
        entry_squote_chars->set_text(CtConst::CHARS_SMART_SQUOTE_DEFAULT);
    });

    checkbutton_auto_smart_quotes->signal_toggled().connect([this, checkbutton_auto_smart_quotes](){
        _pConfig->autoSmartQuotes = checkbutton_auto_smart_quotes->get_active();
    });
    checkbutton_enable_symbol_autoreplace->signal_toggled().connect([this, checkbutton_enable_symbol_autoreplace](){
        _pConfig->enableSymbolAutoreplace = checkbutton_enable_symbol_autoreplace->get_active();
    });
    button_symbol_autoreplace_help->signal_clicked().connect([this](){
        Glib::ustring helpMsg = Glib::ustring{"<b>"} + _("Supported Symbols Auto Replacements") + "</b>:\n" +
            str::xml_escape("-->    →\n"
                            "<--    ←\n"
                            "==>    ⇒\n"
                            "<==    ⇐\n"
                            "<->    ↔\n"
                            "<=>    ⇔\n"
                            "(c)    ©\n"
                            "(r)    ®\n"
                            "(tm)    ™\n") +
            Glib::ustring{"\n<b>"} + _("Only at the Start of the Line") + "</b>:\n" +
            str::xml_escape("*    •\n"
                            "[]    ☐\n"
                            "->    →\n"
                            "=>    ⇒\n"
                            "<>    ◇\n"
                            "::    ▪\n");
        CtDialogs::info_dialog(helpMsg, *this);
    });

    return pMainBox;
}
