/*
 * ct_pref_dlg_theme.cc
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

Gtk::Widget* CtPrefDlg::build_tab_theme()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    // Tree Theme
    auto vbox_tt_theme = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});

    auto radiobutton_tt_col_light = Gtk::manage(new Gtk::RadioButton{_("Light Background, Dark Text")});
    auto radiobutton_tt_col_dark = Gtk::manage(new Gtk::RadioButton{_("Dark Background, Light Text")});
    radiobutton_tt_col_dark->join_group(*radiobutton_tt_col_light);
    auto radiobutton_tt_col_custom = Gtk::manage(new Gtk::RadioButton{_("Custom Background")});
    radiobutton_tt_col_custom->join_group(*radiobutton_tt_col_light);
    auto hbox_tt_col_custom = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto colorbutton_tree_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA{pConfig->ttDefBg}));
    auto label_tt_col_custom = Gtk::manage(new Gtk::Label{_("and Text")});
    auto colorbutton_tree_fg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->ttDefFg}});
    hbox_tt_col_custom->pack_start(*radiobutton_tt_col_custom, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_bg, false, false);
    hbox_tt_col_custom->pack_start(*label_tt_col_custom, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_fg, false, false);

    vbox_tt_theme->pack_start(*radiobutton_tt_col_light, false, false);
    vbox_tt_theme->pack_start(*radiobutton_tt_col_dark, false, false);
    vbox_tt_theme->pack_start(*hbox_tt_col_custom, false, false);
    Gtk::Frame* frame_tt_theme = new_managed_frame_with_align(_("Tree"), vbox_tt_theme);

    if (pConfig->ttDefFg == CtConst::TREE_TEXT_DARK_FG && pConfig->ttDefBg == CtConst::TREE_TEXT_DARK_BG) {
        radiobutton_tt_col_dark->set_active(true);
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
    }
    else if (pConfig->ttDefFg == CtConst::TREE_TEXT_LIGHT_FG && pConfig->ttDefBg == CtConst::TREE_TEXT_LIGHT_BG) {
        radiobutton_tt_col_light->set_active(true);
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
    }
    else {
        radiobutton_tt_col_custom->set_active(true);
    }

    // Style Schemes
    auto pGridStyleSchemes = Gtk::manage(new Gtk::Grid{});
    pGridStyleSchemes->set_row_homogeneous(true);
    pGridStyleSchemes->set_column_spacing(4);

    auto label_style_scheme_rt = Gtk::manage(new Gtk::Label{_("Rich Text")});
    auto combobox_style_scheme_rt = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids()) {
        combobox_style_scheme_rt->append(scheme);
    }
    combobox_style_scheme_rt->set_active_text(pConfig->rtStyleScheme);

    auto label_style_scheme_pt = Gtk::manage(new Gtk::Label{_("Plain Text")});
    auto combobox_style_scheme_pt = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids()) {
        combobox_style_scheme_pt->append(scheme);
    }
    combobox_style_scheme_pt->set_active_text(pConfig->ptStyleScheme);

    auto label_style_scheme_ta = Gtk::manage(new Gtk::Label{_("Table")});
    auto combobox_style_scheme_ta = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids()) {
        combobox_style_scheme_ta->append(scheme);
    }
    combobox_style_scheme_ta->set_active_text(pConfig->taStyleScheme);

    auto label_style_scheme_co = Gtk::manage(new Gtk::Label{_("Code")});
    auto combobox_style_scheme_co = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids()) {
        if (not Glib::str_has_prefix(scheme, "user-")) {
            combobox_style_scheme_co->append(scheme);
        }
    }
    combobox_style_scheme_co->set_active_text(pConfig->coStyleScheme);

    pGridStyleSchemes->attach(*label_style_scheme_rt,     0, 0, 1, 1);
    pGridStyleSchemes->attach(*combobox_style_scheme_rt,  1, 0, 1, 1);
    pGridStyleSchemes->attach(*label_style_scheme_pt,     2, 0, 1, 1);
    pGridStyleSchemes->attach(*combobox_style_scheme_pt,  3, 0, 1, 1);
    pGridStyleSchemes->attach(*label_style_scheme_ta,     0, 1, 1, 1);
    pGridStyleSchemes->attach(*combobox_style_scheme_ta,  1, 1, 1, 1);
    pGridStyleSchemes->attach(*label_style_scheme_co,     2, 1, 1, 1);
    pGridStyleSchemes->attach(*combobox_style_scheme_co,  3, 1, 1, 1);

    Gtk::Frame* frame_style_schemes = new_managed_frame_with_align(_("Style Schemes"), pGridStyleSchemes);

    // Theme Editor
    auto pNotebook = Gtk::manage(new Gtk::Notebook{});

    auto f_onUserStyleChanged = [this,
                                 pConfig,
                                 combobox_style_scheme_rt,
                                 combobox_style_scheme_pt,
                                 combobox_style_scheme_ta](const unsigned num){
        pConfig->update_user_style(num);
        _pCtMainWin->get_style_scheme_manager()->force_rescan();
        const std::string styleId = CtConfig::get_user_style_id(num);
        if (combobox_style_scheme_rt->get_active_text() == styleId) {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('r'/*RichText*/); });
        }
        if (combobox_style_scheme_pt->get_active_text() == styleId) {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
        }
        if (combobox_style_scheme_ta->get_active_text() == styleId) {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('t'/*Table*/); });
        }
    };

    Gtk::Grid* pGridThemeEditor[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelTextFg[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonTextFg[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelTextBg[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonTextBg[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelSelectionFg[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonSelectionFg[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelSelectionBg[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonSelectionBg[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelCursor[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonCursor[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelCurrentLineBg[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonCurrentLineBg[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelLineNumbersFg[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonLineNumbersFg[CtConst::NUM_USER_STYLES];
    Gtk::Label* pLabelLineNumbersBg[CtConst::NUM_USER_STYLES];
    Gtk::ColorButton* pColorButtonLineNumbersBg[CtConst::NUM_USER_STYLES];
    Gtk::Button* pButtonsResetToDefault[CtConst::NUM_USER_STYLES];
    Gtk::Box* pVBoxThemeEditor[CtConst::NUM_USER_STYLES];

    for (unsigned i = 0; i < CtConst::NUM_USER_STYLES; ++i) {
        pVBoxThemeEditor[i] = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL});
        pGridThemeEditor[i] = Gtk::manage(new Gtk::Grid{});
        pGridThemeEditor[i]->set_row_homogeneous(true);
        pGridThemeEditor[i]->set_column_spacing(4);
        pGridThemeEditor[i]->set_border_width(4);

        pLabelTextFg[i] = Gtk::manage(new Gtk::Label{_("Text Foreground")});
        pColorButtonTextFg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleTextFg[i]}});
        pLabelTextBg[i] = Gtk::manage(new Gtk::Label{_("Text Background")});
        pColorButtonTextBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleTextBg[i]}});
        pLabelSelectionFg[i] = Gtk::manage(new Gtk::Label{_("Selection Foreground")});
        pColorButtonSelectionFg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleSelectionFg[i]}});
        pLabelSelectionBg[i] = Gtk::manage(new Gtk::Label{_("Selection Background")});
        pColorButtonSelectionBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleSelectionBg[i]}});
        pLabelCursor[i] = Gtk::manage(new Gtk::Label{_("Cursor")});
        pColorButtonCursor[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleCursor[i]}});
        pLabelCurrentLineBg[i] = Gtk::manage(new Gtk::Label{_("Current Line Background")});
        pColorButtonCurrentLineBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleCurrentLineBg[i]}});
        pLabelLineNumbersFg[i] = Gtk::manage(new Gtk::Label{_("Line Numbers Foreground")});
        pColorButtonLineNumbersFg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleLineNumbersFg[i]}});
        pLabelLineNumbersBg[i] = Gtk::manage(new Gtk::Label{_("Line Numbers Background")});
        pColorButtonLineNumbersBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleLineNumbersBg[i]}});
        pButtonsResetToDefault[i] = Gtk::manage(new Gtk::Button{});
        pButtonsResetToDefault[i]->set_image(*_pCtMainWin->new_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
        pButtonsResetToDefault[i]->set_tooltip_text(_("Reset to Default"));

        pGridThemeEditor[i]->attach(*pLabelTextFg[i],              0, 0, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonTextFg[i],        1, 0, 1, 1);
        pGridThemeEditor[i]->attach(*pLabelTextBg[i],              2, 0, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonTextBg[i],        3, 0, 1, 1);
        pGridThemeEditor[i]->attach(*pLabelSelectionFg[i],         0, 1, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonSelectionFg[i],   1, 1, 1, 1);
        pGridThemeEditor[i]->attach(*pLabelSelectionBg[i],         2, 1, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonSelectionBg[i],   3, 1, 1, 1);
        pGridThemeEditor[i]->attach(*pLabelCursor[i],              0, 2, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonCursor[i],        1, 2, 1, 1);
        pGridThemeEditor[i]->attach(*pLabelCurrentLineBg[i],       2, 2, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonCurrentLineBg[i], 3, 2, 1, 1);
        pGridThemeEditor[i]->attach(*pLabelLineNumbersFg[i],       0, 3, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonLineNumbersFg[i], 1, 3, 1, 1);
        pGridThemeEditor[i]->attach(*pLabelLineNumbersBg[i],       2, 3, 1, 1);
        pGridThemeEditor[i]->attach(*pColorButtonLineNumbersBg[i], 3, 3, 1, 1);

        pVBoxThemeEditor[i]->pack_start(*pGridThemeEditor[i], false, false);
        pVBoxThemeEditor[i]->pack_start(*pButtonsResetToDefault[i], true, false);

        pNotebook->append_page(*(pVBoxThemeEditor[i]), CtConfig::get_user_style_id(i+1));

        pColorButtonTextFg[i]->signal_color_set().connect([pColorButtonTextFg, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonTextFg[i]->get_rgba());
            if (rgba != pConfig->userStyleTextFg[i]) {
                pConfig->userStyleTextFg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonTextBg[i]->signal_color_set().connect([pColorButtonTextBg, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonTextBg[i]->get_rgba());
            if (rgba != pConfig->userStyleTextBg[i]) {
                pConfig->userStyleTextBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonSelectionFg[i]->signal_color_set().connect([pColorButtonSelectionFg, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonSelectionFg[i]->get_rgba());
            if (rgba != pConfig->userStyleSelectionFg[i]) {
                pConfig->userStyleSelectionFg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonSelectionBg[i]->signal_color_set().connect([pColorButtonSelectionBg, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonSelectionBg[i]->get_rgba());
            if (rgba != pConfig->userStyleSelectionBg[i]) {
                pConfig->userStyleSelectionBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonCursor[i]->signal_color_set().connect([pColorButtonCursor, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonCursor[i]->get_rgba());
            if (rgba != pConfig->userStyleCursor[i]) {
                pConfig->userStyleCursor[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonCurrentLineBg[i]->signal_color_set().connect([pColorButtonCurrentLineBg, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonCurrentLineBg[i]->get_rgba());
            if (rgba != pConfig->userStyleCurrentLineBg[i]) {
                pConfig->userStyleCurrentLineBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonLineNumbersFg[i]->signal_color_set().connect([pColorButtonLineNumbersFg, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonLineNumbersFg[i]->get_rgba());
            if (rgba != pConfig->userStyleLineNumbersFg[i]) {
                pConfig->userStyleLineNumbersFg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonLineNumbersBg[i]->signal_color_set().connect([pColorButtonLineNumbersBg, i, pConfig, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_any_to_24(pColorButtonLineNumbersBg[i]->get_rgba());
            if (rgba != pConfig->userStyleLineNumbersBg[i]) {
                pConfig->userStyleLineNumbersBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pButtonsResetToDefault[i]->signal_clicked().connect([i, pConfig,
                                                             pColorButtonTextFg,
                                                             pColorButtonTextBg,
                                                             pColorButtonSelectionFg,
                                                             pColorButtonSelectionBg,
                                                             pColorButtonCursor,
                                                             pColorButtonCurrentLineBg,
                                                             pColorButtonLineNumbersFg,
                                                             pColorButtonLineNumbersBg,
                                                             f_onUserStyleChanged](){
            bool anyChange{false};
            if (pConfig->userStyleTextFg[i] != CtConst::USER_STYLE_TEXT_FG[i]) {
                pConfig->userStyleTextFg[i] = CtConst::USER_STYLE_TEXT_FG[i];
                pColorButtonTextFg[i]->set_rgba(Gdk::RGBA{pConfig->userStyleTextFg[i]});
                anyChange = true;
            }
            if (pConfig->userStyleTextBg[i] != CtConst::USER_STYLE_TEXT_BG[i]) {
                pConfig->userStyleTextBg[i] = CtConst::USER_STYLE_TEXT_BG[i];
                pColorButtonTextBg[i]->set_rgba(Gdk::RGBA{pConfig->userStyleTextBg[i]});
                anyChange = true;
            }
            if (pConfig->userStyleSelectionFg[i] != CtConst::USER_STYLE_SELECTION_FG[i]) {
                pConfig->userStyleSelectionFg[i] = CtConst::USER_STYLE_SELECTION_FG[i];
                pColorButtonSelectionFg[i]->set_rgba(Gdk::RGBA{pConfig->userStyleSelectionFg[i]});
                anyChange = true;
            }
            if (pConfig->userStyleSelectionBg[i] != CtConst::USER_STYLE_SELECTION_BG[i]) {
                pConfig->userStyleSelectionBg[i] = CtConst::USER_STYLE_SELECTION_BG[i];
                pColorButtonSelectionBg[i]->set_rgba(Gdk::RGBA{pConfig->userStyleSelectionBg[i]});
                anyChange = true;
            }
            if (pConfig->userStyleCursor[i] != CtConst::USER_STYLE_CURSOR[i]) {
                pConfig->userStyleCursor[i] = CtConst::USER_STYLE_CURSOR[i];
                pColorButtonCursor[i]->set_rgba(Gdk::RGBA{pConfig->userStyleCursor[i]});
                anyChange = true;
            }
            if (pConfig->userStyleCurrentLineBg[i] != CtConst::USER_STYLE_CURRENT_LINE_BG[i]) {
                pConfig->userStyleCurrentLineBg[i] = CtConst::USER_STYLE_CURRENT_LINE_BG[i];
                pColorButtonCurrentLineBg[i]->set_rgba(Gdk::RGBA{pConfig->userStyleCurrentLineBg[i]});
                anyChange = true;
            }
            if (pConfig->userStyleLineNumbersFg[i] != CtConst::USER_STYLE_LINE_NUMBERS_FG[i]) {
                pConfig->userStyleLineNumbersFg[i] = CtConst::USER_STYLE_LINE_NUMBERS_FG[i];
                pColorButtonLineNumbersFg[i]->set_rgba(Gdk::RGBA{pConfig->userStyleLineNumbersFg[i]});
                anyChange = true;
            }
            if (pConfig->userStyleLineNumbersBg[i] != CtConst::USER_STYLE_LINE_NUMBERS_BG[i]) {
                pConfig->userStyleLineNumbersBg[i] = CtConst::USER_STYLE_LINE_NUMBERS_BG[i];
                pColorButtonLineNumbersBg[i]->set_rgba(Gdk::RGBA{pConfig->userStyleLineNumbersBg[i]});
                anyChange = true;
            }
            if (anyChange) {
                f_onUserStyleChanged(i+1);
            }
            else {
                spdlog::debug("{} nothing to reset", CtConfig::get_user_style_id(i+1));
            }
        });
    }

    Gtk::Frame* frame_theme_editor = new_managed_frame_with_align(_("Style Scheme Editor"), pNotebook);

    auto pVBoxMain = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    pVBoxMain->set_margin_left(6);
    pVBoxMain->set_margin_top(6);
    pVBoxMain->pack_start(*frame_tt_theme, false, false);
    pVBoxMain->pack_start(*frame_style_schemes, false, false);
    pVBoxMain->pack_start(*frame_theme_editor, false, false);

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
        colorbutton_tree_fg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_LIGHT_FG});
        colorbutton_tree_bg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_LIGHT_BG});
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        update_tree_color();
    });
    radiobutton_tt_col_dark->signal_toggled().connect([radiobutton_tt_col_dark, colorbutton_tree_fg, colorbutton_tree_bg, update_tree_color](){
        if (!radiobutton_tt_col_dark->get_active()) return;
        colorbutton_tree_fg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_DARK_FG});
        colorbutton_tree_bg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_DARK_BG});
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

    combobox_style_scheme_pt->signal_changed().connect([this, pConfig, combobox_style_scheme_pt](){
        pConfig->ptStyleScheme = combobox_style_scheme_pt->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
    });

    combobox_style_scheme_ta->signal_changed().connect([this, pConfig, combobox_style_scheme_ta](){
        pConfig->taStyleScheme = combobox_style_scheme_ta->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('t'/*Table*/); });
    });

    combobox_style_scheme_co->signal_changed().connect([this, pConfig, combobox_style_scheme_co](){
        pConfig->coStyleScheme = combobox_style_scheme_co->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
    });

    return pVBoxMain;
}
