/*
 * ct_pref_dlg_rich_text.cc
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
