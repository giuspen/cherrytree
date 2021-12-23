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
    Gtk::VBox* vbox_spell_check = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_spell_check = Gtk::manage(new Gtk::CheckButton(_("Enable Spell Check")));
    checkbutton_spell_check->set_active(_pConfig->enableSpellCheck);
    checkbutton_spell_check->set_sensitive(gspell_language_get_available());
    Gtk::HBox* hbox_spell_check_lang = Gtk::manage(new Gtk::HBox());
    hbox_spell_check_lang->set_spacing(4);
    Gtk::Label* label_spell_check_lang = Gtk::manage(new Gtk::Label(_("Spell Check Language")));
    Gtk::ComboBoxText* combobox_spell_check_lang = Gtk::manage(new Gtk::ComboBoxText());
    for (const GList* l = gspell_language_get_available(); l != NULL; l = l->next) {
        auto pGspellLang = reinterpret_cast<const GspellLanguage*>(l->data);
        combobox_spell_check_lang->append(gspell_language_get_code(pGspellLang), gspell_language_get_name(pGspellLang));
    }
    combobox_spell_check_lang->set_active_id(_pConfig->spellCheckLang);
    combobox_spell_check_lang->set_sensitive(_pConfig->enableSpellCheck);

    hbox_spell_check_lang->pack_start(*label_spell_check_lang, false, false);
    hbox_spell_check_lang->pack_start(*combobox_spell_check_lang);
    vbox_spell_check->pack_start(*checkbutton_spell_check, false, false);
    vbox_spell_check->pack_start(*hbox_spell_check_lang, false, false);
    Gtk::Frame* frame_spell_check = new_managed_frame_with_align(_("Spell Check"), vbox_spell_check);

    Gtk::HBox* hbox_misc_text = Gtk::manage(new Gtk::HBox());
    hbox_misc_text->set_spacing(4);
    Gtk::CheckButton* checkbutton_rt_show_white_spaces = Gtk::manage(new Gtk::CheckButton(_("Show White Spaces")));
    checkbutton_rt_show_white_spaces->set_active(_pConfig->rtShowWhiteSpaces);
    Gtk::CheckButton* checkbutton_rt_highl_curr_line = Gtk::manage(new Gtk::CheckButton(_("Highlight Current Line")));
    checkbutton_rt_highl_curr_line->set_active(_pConfig->rtHighlCurrLine);
    Gtk::CheckButton* checkbutton_rt_highl_match_bra = Gtk::manage(new Gtk::CheckButton(_("Highlight Matching Brackets")));
    checkbutton_rt_highl_match_bra->set_active(_pConfig->rtHighlMatchBra);
    Gtk::CheckButton* checkbutton_codebox_auto_resize = Gtk::manage(new Gtk::CheckButton(_("Expand CodeBoxes Automatically")));
    checkbutton_codebox_auto_resize->set_active(_pConfig->codeboxAutoResize);

    Gtk::HBox* hbox_embfile_icon_size = Gtk::manage(new Gtk::HBox());
    hbox_embfile_icon_size->set_spacing(4);
    Gtk::Label* label_embfile_icon_size = Gtk::manage(new Gtk::Label(_("Embedded File Icon Size")));
    Glib::RefPtr<Gtk::Adjustment> adj_embfile_icon_size = Gtk::Adjustment::create(_pConfig->embfileIconSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_embfile_icon_size = Gtk::manage(new Gtk::SpinButton(adj_embfile_icon_size));
    spinbutton_embfile_icon_size->set_value(_pConfig->embfileIconSize);
    hbox_embfile_icon_size->pack_start(*label_embfile_icon_size, false, false);
    hbox_embfile_icon_size->pack_start(*spinbutton_embfile_icon_size, false, false);

    Gtk::HBox* hbox_embfile_max_size = Gtk::manage(new Gtk::HBox());
    hbox_embfile_max_size->set_spacing(4);
    Gtk::Label* label_embfile_max_size = Gtk::manage(new Gtk::Label(_("Embedded File Size Limit")));
    Glib::RefPtr<Gtk::Adjustment> adj_embfile_max_size = Gtk::Adjustment::create(_pConfig->embfileIconSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_embfile_max_size = Gtk::manage(new Gtk::SpinButton(adj_embfile_max_size));
    spinbutton_embfile_max_size->set_value(_pConfig->embfileMaxSize);
    hbox_embfile_max_size->pack_start(*label_embfile_max_size, false, false);
    hbox_embfile_max_size->pack_start(*spinbutton_embfile_max_size, false, false);

    Gtk::CheckButton* checkbutton_embfile_show_filename = Gtk::manage(new Gtk::CheckButton(_("Show File Name on Top of Embedded File Icon")));
    checkbutton_embfile_show_filename->set_active(_pConfig->embfileShowFileName);
    Gtk::Label* label_limit_undoable_steps = Gtk::manage(new Gtk::Label(_("Limit of Undoable Steps Per Node")));
    Glib::RefPtr<Gtk::Adjustment> adj_limit_undoable_steps = Gtk::Adjustment::create(_pConfig->limitUndoableSteps, 1, 10000, 1);
    Gtk::SpinButton* spinbutton_limit_undoable_steps = Gtk::manage(new Gtk::SpinButton(adj_limit_undoable_steps));
    spinbutton_limit_undoable_steps->set_value(_pConfig->limitUndoableSteps);
    hbox_misc_text->pack_start(*label_limit_undoable_steps, false, false);
    hbox_misc_text->pack_start(*spinbutton_limit_undoable_steps, false, false);
    Gtk::CheckButton* checkbutton_camelcase_autolink = Gtk::manage(new Gtk::CheckButton{_("Auto Link CamelCase Text to Node With Same Name")});
    checkbutton_camelcase_autolink->set_active(_pConfig->camelCaseAutoLink);
    Gtk::CheckButton* checkbutton_triple_click_sel_paragraph = Gtk::manage(new Gtk::CheckButton(_("At Triple Click Select the Whole Paragraph")));
    checkbutton_triple_click_sel_paragraph->set_active(_pConfig->tripleClickParagraph);
#ifdef MD_AUTO_REPLACEMENT
    Gtk::CheckButton* checkbutton_md_formatting = Gtk::manage(new Gtk::CheckButton(_("Enable Markdown Auto Replacement (Experimental)")));
    checkbutton_md_formatting->set_active(_pConfig->enableMdFormatting);
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
    vbox_misc_text->pack_start(*checkbutton_camelcase_autolink, false, false);
    vbox_misc_text->pack_start(*checkbutton_triple_click_sel_paragraph, false, false);
#ifdef MD_AUTO_REPLACEMENT
    vbox_misc_text->pack_start(*checkbutton_md_formatting, false, false);
#endif // MD_AUTO_REPLACEMENT
    Gtk::Frame* frame_misc_text = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc_text);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_spell_check, false, false);
    pMainBox->pack_start(*frame_misc_text, false, false);

    checkbutton_spell_check->signal_toggled().connect([this, checkbutton_spell_check, combobox_spell_check_lang](){
        _pConfig->enableSpellCheck = checkbutton_spell_check->get_active();
        combobox_spell_check_lang->set_sensitive(_pConfig->enableSpellCheck);
        apply_for_each_window([](CtMainWin* win) {
            win->get_text_view().set_spell_check(win->curr_tree_iter().get_node_is_text());
            win->update_selected_node_statusbar_info();
        });
    });
    combobox_spell_check_lang->signal_changed().connect([this, combobox_spell_check_lang](){
        _pConfig->spellCheckLang = combobox_spell_check_lang->get_active_id();
        apply_for_each_window([](CtMainWin* win) {
            win->get_text_view().set_spell_check(win->curr_tree_iter().get_node_is_text());
            win->update_selected_node_statusbar_info();
        });
    });
    checkbutton_rt_show_white_spaces->signal_toggled().connect([this, checkbutton_rt_show_white_spaces](){
        _pConfig->rtShowWhiteSpaces = checkbutton_rt_show_white_spaces->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('r'/*RichText*/);
        });
    });
    checkbutton_rt_highl_curr_line->signal_toggled().connect([this, checkbutton_rt_highl_curr_line](){
        _pConfig->rtHighlCurrLine = checkbutton_rt_highl_curr_line->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->resetup_for_syntax('r'/*RichText*/);
        });
    });
    checkbutton_rt_highl_match_bra->signal_toggled().connect([this, checkbutton_rt_highl_match_bra](){
        _pConfig->rtHighlMatchBra = checkbutton_rt_highl_match_bra->get_active();
        apply_for_each_window([](CtMainWin* win) {
            win->reapply_syntax_highlighting('r'/*RichText*/);
            win->reapply_syntax_highlighting('t'/*Table*/);
        });
    });
    checkbutton_codebox_auto_resize->signal_toggled().connect([this, checkbutton_codebox_auto_resize](){
        _pConfig->codeboxAutoResize = checkbutton_codebox_auto_resize->get_active();
        need_restart(RESTART_REASON::CODEBOX_AUTORESIZE);
    });
    spinbutton_embfile_icon_size->signal_value_changed().connect([this, spinbutton_embfile_icon_size](){
        _pConfig->embfileIconSize = spinbutton_embfile_icon_size->get_value_as_int();
        need_restart(RESTART_REASON::EMBFILE_SIZE);
    });
    spinbutton_embfile_max_size->signal_value_changed().connect([this, spinbutton_embfile_max_size](){
        _pConfig->embfileMaxSize = spinbutton_embfile_max_size->get_value_as_int();
    });
    checkbutton_embfile_show_filename->signal_toggled().connect([this, checkbutton_embfile_show_filename](){
        _pConfig->embfileShowFileName = checkbutton_embfile_show_filename->get_active();
        need_restart(RESTART_REASON::SHOW_EMBFILE_NAME);
    });
    spinbutton_limit_undoable_steps->signal_value_changed().connect([this, spinbutton_limit_undoable_steps](){
        _pConfig->limitUndoableSteps = spinbutton_limit_undoable_steps->get_value_as_int();
    });
    checkbutton_camelcase_autolink->signal_toggled().connect([this, checkbutton_camelcase_autolink]{
        _pConfig->camelCaseAutoLink = checkbutton_camelcase_autolink->get_active();
    });
    checkbutton_triple_click_sel_paragraph->signal_toggled().connect([this, checkbutton_triple_click_sel_paragraph]{
        _pConfig->tripleClickParagraph = checkbutton_triple_click_sel_paragraph->get_active();
    });
#ifdef MD_AUTO_REPLACEMENT
    checkbutton_md_formatting->signal_toggled().connect([this, checkbutton_md_formatting]{
        _pConfig->enableMdFormatting = checkbutton_md_formatting->get_active();
    });
#endif // MD_AUTO_REPLACEMENT

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_format()
{
    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);

    auto pNotebookScalable = Gtk::manage(new Gtk::Notebook{});
    auto vbox_misc = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});

    const Glib::ustring tagPrefix = Glib::ustring{CtConst::TAG_SCALE} + CtConst::CHAR_USCORE;
    static const std::array<Glib::ustring,7> scalablesTagId{
        tagPrefix + CtConst::TAG_PROP_VAL_H1,
        tagPrefix + CtConst::TAG_PROP_VAL_H2,
        tagPrefix + CtConst::TAG_PROP_VAL_H3,
        tagPrefix + CtConst::TAG_PROP_VAL_H4,
        tagPrefix + CtConst::TAG_PROP_VAL_H5,
        tagPrefix + CtConst::TAG_PROP_VAL_H6,
        tagPrefix + CtConst::TAG_PROP_VAL_SMALL};

    for (unsigned i = 0; i < _pConfig->scalablesTags.size(); ++i) {
        CtScalableTag* pScalableCfg = _pConfig->scalablesTags.at(i);
        const Glib::ustring* pScalableTagId = &scalablesTagId.at(i);
        auto vboxTab = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
        vboxTab->set_margin_left(6);
        vboxTab->set_margin_top(6);

        Gtk::Image* pImageBold = _pCtMainWin->new_managed_image_from_stock("ct_fmt-txt-bold", Gtk::ICON_SIZE_MENU);
        Gtk::Image* pImageItalic = _pCtMainWin->new_managed_image_from_stock("ct_fmt-txt-italic", Gtk::ICON_SIZE_MENU);
        Gtk::Image* pImageUnderline = _pCtMainWin->new_managed_image_from_stock("ct_fmt-txt-underline", Gtk::ICON_SIZE_MENU);
        Gtk::Image* pImageFg = _pCtMainWin->new_managed_image_from_stock("ct_color_fg", Gtk::ICON_SIZE_MENU);
        Gtk::Image* pImageBg = _pCtMainWin->new_managed_image_from_stock("ct_color_bg", Gtk::ICON_SIZE_MENU);

        auto pLabel_scaleTab = Gtk::manage(new Gtk::Label{_("Scale")});
        auto pAdj_scaleTab = Gtk::Adjustment::create(pScalableCfg->scale, 0.1, 10.0, 0.1);
        auto pSpinButton_scaleTab = Gtk::manage(new Gtk::SpinButton{pAdj_scaleTab});
        pSpinButton_scaleTab->set_digits(3);
        auto pCheckButton_boldTab = Gtk::manage(new Gtk::CheckButton{_("Bold")});
        auto pCheckButton_italicTab = Gtk::manage(new Gtk::CheckButton{_("Italic")});
        auto pCheckButton_underlineTab = Gtk::manage(new Gtk::CheckButton{_("Underline")});
        auto hboxScaleBoItUnTab = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
        hboxScaleBoItUnTab->pack_start(*pLabel_scaleTab, false, false);
        hboxScaleBoItUnTab->pack_start(*pSpinButton_scaleTab, false, false);
        hboxScaleBoItUnTab->pack_start(*pImageBold, false, false);
        hboxScaleBoItUnTab->pack_start(*pCheckButton_boldTab, false, false);
        hboxScaleBoItUnTab->pack_start(*pImageItalic, false, false);
        hboxScaleBoItUnTab->pack_start(*pCheckButton_italicTab, false, false);
        hboxScaleBoItUnTab->pack_start(*pImageUnderline, false, false);
        hboxScaleBoItUnTab->pack_start(*pCheckButton_underlineTab, false, false);
        vboxTab->pack_start(*hboxScaleBoItUnTab, false, false);
        pCheckButton_boldTab->set_active(pScalableCfg->bold);
        pCheckButton_italicTab->set_active(pScalableCfg->italic);
        pCheckButton_underlineTab->set_active(pScalableCfg->underline);

        auto pCheckButton_fgTab = Gtk::manage(new Gtk::CheckButton{_("Text Color Foreground")});
        std::string fgTab_color = pScalableCfg->foreground.empty() ? CtConst::COLOR_24_LGRAY : pScalableCfg->foreground;
        auto pColorButton_fgTab = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{fgTab_color}});
        auto pCheckButton_bgTab = Gtk::manage(new Gtk::CheckButton{_("Text Color Background")});
        std::string bgTab_color = pScalableCfg->background.empty() ? CtConst::COLOR_24_LGRAY : pScalableCfg->background;
        auto pColorButton_bgTab = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{bgTab_color}});
        auto hboxFgBgTab = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
        hboxFgBgTab->pack_start(*pImageFg, false, false);
        hboxFgBgTab->pack_start(*pCheckButton_fgTab, false, false);
        hboxFgBgTab->pack_start(*pColorButton_fgTab, false, false);
        hboxFgBgTab->pack_start(*pImageBg, false, false);
        hboxFgBgTab->pack_start(*pCheckButton_bgTab, false, false);
        hboxFgBgTab->pack_start(*pColorButton_bgTab, false, false);
        vboxTab->pack_start(*hboxFgBgTab, false, false);
        pCheckButton_fgTab->set_active(not pScalableCfg->foreground.empty());
        pColorButton_fgTab->set_sensitive(not pScalableCfg->foreground.empty());
        pCheckButton_bgTab->set_active(not pScalableCfg->background.empty());
        pColorButton_bgTab->set_sensitive(not pScalableCfg->background.empty());

        Glib::ustring tabLabel = i != 6 ? Glib::ustring{"h"} + std::to_string(i+1) : _("Small");
        pNotebookScalable->append_page(*vboxTab, tabLabel);

        pSpinButton_scaleTab->signal_value_changed().connect([pSpinButton_scaleTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->scale = pSpinButton_scaleTab->get_value();
            if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
            }
        });
        pCheckButton_boldTab->signal_toggled().connect([pCheckButton_boldTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->bold = pCheckButton_boldTab->get_active();
            if (pScalableCfg->bold) {
                if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                    _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
                }
            }
            else {
                need_restart(RESTART_REASON::SCALABLE_TAGS);
            }
        });
        pCheckButton_italicTab->signal_toggled().connect([pCheckButton_italicTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->italic = pCheckButton_italicTab->get_active();
            if (pScalableCfg->italic) {
                if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                    _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
                }
            }
            else {
                need_restart(RESTART_REASON::SCALABLE_TAGS);
            }
        });
        pCheckButton_underlineTab->signal_toggled().connect([pCheckButton_underlineTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->underline = pCheckButton_underlineTab->get_active();
            if (pScalableCfg->underline) {
                if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                    _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
                }
            }
            else {
                need_restart(RESTART_REASON::SCALABLE_TAGS);
            }
        });
        pCheckButton_fgTab->signal_toggled().connect([pCheckButton_fgTab, pColorButton_fgTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->foreground = pCheckButton_fgTab->get_active() ?
                CtRgbUtil::rgb_any_to_24(pColorButton_fgTab->get_rgba()) : "";
            pColorButton_fgTab->set_sensitive(not pScalableCfg->foreground.empty());
            if (not pScalableCfg->foreground.empty()) {
                if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                    _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
                }
            }
            else {
                need_restart(RESTART_REASON::SCALABLE_TAGS);
            }
        });
        pColorButton_fgTab->signal_color_set().connect([pColorButton_fgTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->foreground = CtRgbUtil::rgb_any_to_24(pColorButton_fgTab->get_rgba());
            if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
            }
        });
        pCheckButton_bgTab->signal_toggled().connect([pCheckButton_bgTab, pColorButton_bgTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->background = pCheckButton_bgTab->get_active() ?
                CtRgbUtil::rgb_any_to_24(pColorButton_bgTab->get_rgba()) : "";
            pColorButton_bgTab->set_sensitive(not pScalableCfg->background.empty());
            if (not pScalableCfg->background.empty()) {
                if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                    _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
                }
            }
            else {
                need_restart(RESTART_REASON::SCALABLE_TAGS);
            }
        });
        pColorButton_bgTab->signal_color_set().connect([pColorButton_bgTab, pScalableCfg, pScalableTagId, this](){
            pScalableCfg->background = CtRgbUtil::rgb_any_to_24(pColorButton_bgTab->get_rgba());
            if (auto rTag = _pCtMainWin->get_text_tag_table()->lookup(*pScalableTagId)) {
                _pCtMainWin->apply_scalable_properties(rTag, pScalableCfg);
            }
        });
    }

    Gtk::Image* pImageMsFg = _pCtMainWin->new_managed_image_from_stock("ct_color_fg", Gtk::ICON_SIZE_MENU);
    auto checkbutton_monospace_fg = Gtk::manage(new Gtk::CheckButton{_("Text Color Foreground")});
    std::string mono_color_fg = _pConfig->monospaceFg.empty() ? CtConst::DEFAULT_MONOSPACE_FG : _pConfig->monospaceFg;
    auto colorbutton_monospace_fg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{mono_color_fg}});
    auto hbox_monospace_fg = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    hbox_monospace_fg->pack_start(*pImageMsFg, false, false);
    hbox_monospace_fg->pack_start(*checkbutton_monospace_fg, false, false);
    hbox_monospace_fg->pack_start(*colorbutton_monospace_fg, false, false);
    vbox_misc->pack_start(*hbox_monospace_fg, false, false);
    checkbutton_monospace_fg->set_active(not _pConfig->monospaceFg.empty());
    colorbutton_monospace_fg->set_sensitive(not _pConfig->monospaceFg.empty());

    Gtk::Image* pImageMsBg = _pCtMainWin->new_managed_image_from_stock("ct_color_bg", Gtk::ICON_SIZE_MENU);
    auto checkbutton_monospace_bg = Gtk::manage(new Gtk::CheckButton{_("Text Color Background")});
    std::string mono_color_bg = _pConfig->monospaceBg.empty() ? CtConst::DEFAULT_MONOSPACE_BG : _pConfig->monospaceBg;
    auto colorbutton_monospace_bg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{mono_color_bg}});
    auto hbox_monospace_bg = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    hbox_monospace_bg->pack_start(*pImageMsBg, false, false);
    hbox_monospace_bg->pack_start(*checkbutton_monospace_bg, false, false);
    hbox_monospace_bg->pack_start(*colorbutton_monospace_bg, false, false);
    vbox_misc->pack_start(*hbox_monospace_bg, false, false);
    checkbutton_monospace_bg->set_active(not _pConfig->monospaceBg.empty());
    colorbutton_monospace_bg->set_sensitive(not _pConfig->monospaceBg.empty());

    Gtk::Frame* pFrameScalable = new_managed_frame_with_align(_("Scalable Tags"), pNotebookScalable);
    Gtk::Frame* pFrameMs = new_managed_frame_with_align(_("Monospace"), vbox_misc);

    checkbutton_monospace_fg->signal_toggled().connect([this, checkbutton_monospace_fg, colorbutton_monospace_fg](){
        _pConfig->monospaceFg = checkbutton_monospace_fg->get_active() ?
            CtRgbUtil::rgb_any_to_24(colorbutton_monospace_fg->get_rgba()) : "";
        colorbutton_monospace_fg->set_sensitive(not _pConfig->monospaceFg.empty());
        if (not _pConfig->monospaceFg.empty()) {
            if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
                tag->property_foreground() = _pConfig->monospaceFg;
            }
        }
        else {
            need_restart(RESTART_REASON::MONOSPACE);
        }
    });
    colorbutton_monospace_fg->signal_color_set().connect([this, colorbutton_monospace_fg](){
        _pConfig->monospaceFg = CtRgbUtil::rgb_any_to_24(colorbutton_monospace_fg->get_rgba());
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_foreground() = _pConfig->monospaceFg;
        }
    });
    checkbutton_monospace_bg->signal_toggled().connect([this, checkbutton_monospace_bg, colorbutton_monospace_bg](){
        _pConfig->monospaceBg = checkbutton_monospace_bg->get_active() ?
            CtRgbUtil::rgb_any_to_24(colorbutton_monospace_bg->get_rgba()) : "";
        colorbutton_monospace_bg->set_sensitive(not _pConfig->monospaceBg.empty());
        if (not _pConfig->monospaceBg.empty()) {
            if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
                tag->property_background() = _pConfig->monospaceBg;
            }
        }
        else {
            need_restart(RESTART_REASON::MONOSPACE);
        }
    });
    colorbutton_monospace_bg->signal_color_set().connect([this, colorbutton_monospace_bg](){
        _pConfig->monospaceBg = CtRgbUtil::rgb_any_to_24(colorbutton_monospace_bg->get_rgba());
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_background() = _pConfig->monospaceBg;
        }
    });

    pMainBox->pack_start(*pFrameScalable, false, false);
    pMainBox->pack_start(*pFrameMs, false, false);

    return pMainBox;
}
