/*
 * ct_pref_dlg_theme.cc
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
#include <glib/gstdio.h>

Gtk::Widget* CtPrefDlg::build_tab_theme()
{
    // Tree Theme
        auto vbox_tt_theme = Gtk::manage(new Gtk::Box{
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
    #else
        Gtk::ORIENTATION_VERTICAL
    #endif
        });

    
#if GTKMM_MAJOR_VERSION >= 4
    auto radiobutton_tt_col_light = Gtk::manage(new Gtk::CheckButton{_("Light Background, Dark Text")});
    auto radiobutton_tt_col_dark = Gtk::manage(new Gtk::CheckButton{_("Dark Background, Light Text")});
    radiobutton_tt_col_dark->set_group(radiobutton_tt_col_light->get_group());
    auto radiobutton_tt_col_custom = Gtk::manage(new Gtk::CheckButton{_("Custom Background")});
    radiobutton_tt_col_custom->set_group(radiobutton_tt_col_light->get_group());
    auto hbox_tt_col_custom = Gtk::manage(new Gtk::Box{Gtk::Orientation::HORIZONTAL, 4/*spacing*/});
#else
    auto radiobutton_tt_col_light = Gtk::manage(new Gtk::RadioButton{_("Light Background, Dark Text")});
    auto radiobutton_tt_col_dark = Gtk::manage(new Gtk::RadioButton{_("Dark Background, Light Text")});
    radiobutton_tt_col_dark->join_group(*radiobutton_tt_col_light);
    auto radiobutton_tt_col_custom = Gtk::manage(new Gtk::RadioButton{_("Custom Background")});
    radiobutton_tt_col_custom->join_group(*radiobutton_tt_col_light);
    auto hbox_tt_col_custom = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
#endif
    auto colorbutton_tree_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA{_pConfig->ttDefBg}));
    auto label_tt_col_custom_txt = Gtk::manage(new Gtk::Label{_("Text")});
    auto label_tt_col_custom_sel_bg = Gtk::manage(new Gtk::Label{_("Selection Background")});
    auto label_tt_col_custom_sel_txt = Gtk::manage(new Gtk::Label{_("Text")});
    auto colorbutton_tree_fg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->ttDefFg}});
    auto colorbutton_tree_sel_bg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->ttSelBg}});
    auto colorbutton_tree_sel_fg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->ttSelFg}});
    
#if GTKMM_MAJOR_VERSION >= 4
    hbox_tt_col_custom->append(*radiobutton_tt_col_custom);
    hbox_tt_col_custom->append(*colorbutton_tree_bg);
    hbox_tt_col_custom->append(*label_tt_col_custom_txt);
    hbox_tt_col_custom->append(*colorbutton_tree_fg);
    hbox_tt_col_custom->append(*label_tt_col_custom_sel_bg);
    hbox_tt_col_custom->append(*colorbutton_tree_sel_bg);
    hbox_tt_col_custom->append(*label_tt_col_custom_sel_txt);
    hbox_tt_col_custom->append(*colorbutton_tree_sel_fg);
#else
    hbox_tt_col_custom->pack_start(*radiobutton_tt_col_custom, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_bg, false, false);
    hbox_tt_col_custom->pack_start(*label_tt_col_custom_txt, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_fg, false, false);
    hbox_tt_col_custom->pack_start(*label_tt_col_custom_sel_bg, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_sel_bg, false, false);
    hbox_tt_col_custom->pack_start(*label_tt_col_custom_sel_txt, false, false);
    hbox_tt_col_custom->pack_start(*colorbutton_tree_sel_fg, false, false);
#endif

    
#if GTKMM_MAJOR_VERSION >= 4
    vbox_tt_theme->append(*radiobutton_tt_col_light);
    vbox_tt_theme->append(*radiobutton_tt_col_dark);
    vbox_tt_theme->append(*hbox_tt_col_custom);
#else
    vbox_tt_theme->pack_start(*radiobutton_tt_col_light, false, false);
    vbox_tt_theme->pack_start(*radiobutton_tt_col_dark, false, false);
    vbox_tt_theme->pack_start(*hbox_tt_col_custom, false, false);
#endif
    Gtk::Frame* frame_tt_theme = new_managed_frame_with_align(_("Tree Explorer"), vbox_tt_theme);

    if (_pConfig->ttDefFg == CtConst::TREE_TEXT_DARK_FG and
        _pConfig->ttDefBg == CtConst::TREE_TEXT_DARK_BG and
        _pConfig->ttSelFg == CtConst::TREE_TEXT_DARK_BG and
        _pConfig->ttSelBg == CtConst::TREE_TEXT_SEL_BG)
    {
        radiobutton_tt_col_dark->set_active(true);
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        colorbutton_tree_sel_fg->set_sensitive(false);
        colorbutton_tree_sel_bg->set_sensitive(false);
    }
    else if (_pConfig->ttDefFg == CtConst::TREE_TEXT_LIGHT_FG and
             _pConfig->ttDefBg == CtConst::TREE_TEXT_LIGHT_BG and
             _pConfig->ttSelFg == CtConst::TREE_TEXT_LIGHT_BG and
             _pConfig->ttSelBg == CtConst::TREE_TEXT_SEL_BG)
    {
        radiobutton_tt_col_light->set_active(true);
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        colorbutton_tree_sel_fg->set_sensitive(false);
        colorbutton_tree_sel_bg->set_sensitive(false);
    }
    else {
        radiobutton_tt_col_custom->set_active(true);
    }

    auto _update_tree_color = [this,
                               colorbutton_tree_fg,
                               colorbutton_tree_bg,
                               colorbutton_tree_sel_fg,
                               colorbutton_tree_sel_bg]() {
        _pConfig->ttDefFg = CtRgbUtil::rgb_to_string_24(colorbutton_tree_fg->get_rgba());
        _pConfig->ttDefBg = CtRgbUtil::rgb_to_string_24(colorbutton_tree_bg->get_rgba());
        _pConfig->ttSelFg = CtRgbUtil::rgb_to_string_24(colorbutton_tree_sel_fg->get_rgba());
        _pConfig->ttSelBg = CtRgbUtil::rgb_to_string_24(colorbutton_tree_sel_bg->get_rgba());
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); win->window_header_update(); });
    };
    auto update_tree_color = [radiobutton_tt_col_custom,
                              _update_tree_color]() {
        if (radiobutton_tt_col_custom->get_active()) {
            _update_tree_color();
        }
    };
    colorbutton_tree_fg->signal_color_set().connect(update_tree_color);
    colorbutton_tree_bg->signal_color_set().connect(update_tree_color);
    colorbutton_tree_sel_fg->signal_color_set().connect(update_tree_color);
    colorbutton_tree_sel_bg->signal_color_set().connect(update_tree_color);

    radiobutton_tt_col_light->signal_toggled().connect([radiobutton_tt_col_light,
                                                        colorbutton_tree_fg,
                                                        colorbutton_tree_bg,
                                                        colorbutton_tree_sel_fg,
                                                        colorbutton_tree_sel_bg,
                                                        _update_tree_color](){
        if (not radiobutton_tt_col_light->get_active()) return;
        colorbutton_tree_fg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_LIGHT_FG});
        colorbutton_tree_bg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_LIGHT_BG});
        colorbutton_tree_sel_fg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_LIGHT_BG});
        colorbutton_tree_sel_bg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_SEL_BG});
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        colorbutton_tree_sel_fg->set_sensitive(false);
        colorbutton_tree_sel_bg->set_sensitive(false);
        _update_tree_color();
    });
    radiobutton_tt_col_dark->signal_toggled().connect([radiobutton_tt_col_dark,
                                                       colorbutton_tree_fg,
                                                       colorbutton_tree_bg,
                                                       colorbutton_tree_sel_fg,
                                                       colorbutton_tree_sel_bg,
                                                       _update_tree_color](){
        if (not radiobutton_tt_col_dark->get_active()) return;
        colorbutton_tree_fg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_DARK_FG});
        colorbutton_tree_bg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_DARK_BG});
        colorbutton_tree_sel_fg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_DARK_BG});
        colorbutton_tree_sel_bg->set_rgba(Gdk::RGBA{CtConst::TREE_TEXT_SEL_BG});
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        colorbutton_tree_sel_fg->set_sensitive(false);
        colorbutton_tree_sel_bg->set_sensitive(false);
        _update_tree_color();
    });
    radiobutton_tt_col_custom->signal_toggled().connect([radiobutton_tt_col_custom,
                                                         colorbutton_tree_fg,
                                                         colorbutton_tree_bg,
                                                         colorbutton_tree_sel_fg,
                                                         colorbutton_tree_sel_bg](){
        if (not radiobutton_tt_col_custom->get_active()) return;
        colorbutton_tree_fg->set_sensitive(true);
        colorbutton_tree_bg->set_sensitive(true);
        colorbutton_tree_sel_fg->set_sensitive(true);
        colorbutton_tree_sel_bg->set_sensitive(true);
    });

    // Style Schemes
    auto pGridStyleSchemes = Gtk::manage(new Gtk::Grid{});
    pGridStyleSchemes->set_row_homogeneous(true);
    pGridStyleSchemes->set_column_spacing(4);

    GtkSourceStyleSchemeManager* pGtkSourceStyleSchemeManager = gtk_source_style_scheme_manager_get_default();
    const gchar * const * pSSMSchemeIds = gtk_source_style_scheme_manager_get_scheme_ids(pGtkSourceStyleSchemeManager);

    auto label_style_scheme_rt = Gtk::manage(new Gtk::Label{_("Rich Text")});
    auto combobox_style_scheme_rt = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto pScheme = pSSMSchemeIds; *pScheme; ++pScheme) {
        combobox_style_scheme_rt->append(*pScheme);
    }
    combobox_style_scheme_rt->set_active_text(_pConfig->rtStyleScheme);

    auto label_style_scheme_pt = Gtk::manage(new Gtk::Label{_("Plain Text")});
    auto combobox_style_scheme_pt = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto pScheme = pSSMSchemeIds; *pScheme; ++pScheme) {
        combobox_style_scheme_pt->append(*pScheme);
    }
    combobox_style_scheme_pt->set_active_text(_pConfig->ptStyleScheme);

    auto label_style_scheme_ta = Gtk::manage(new Gtk::Label{_("Table")});
    auto combobox_style_scheme_ta = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto pScheme = pSSMSchemeIds; *pScheme; ++pScheme) {
        combobox_style_scheme_ta->append(*pScheme);
    }
    combobox_style_scheme_ta->set_active_text(_pConfig->taStyleScheme);

    auto label_style_scheme_co = Gtk::manage(new Gtk::Label{_("Code")});
    auto combobox_style_scheme_co = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto pScheme = pSSMSchemeIds; *pScheme; ++pScheme) {
        if (not g_str_has_prefix(*pScheme, "user-")) {
            combobox_style_scheme_co->append(*pScheme);
        }
    }
    combobox_style_scheme_co->set_active_text(_pConfig->coStyleScheme);

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
                                 pGtkSourceStyleSchemeManager,
                                 combobox_style_scheme_rt,
                                 combobox_style_scheme_pt,
                                 combobox_style_scheme_ta](const unsigned num){
        _pConfig->update_user_style(num);
        gtk_source_style_scheme_manager_force_rescan(pGtkSourceStyleSchemeManager);
        const std::string styleId = CtConfig::get_user_style_id(num);
        if (combobox_style_scheme_rt->get_active_text() == Glib::ustring{styleId}) {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('r'/*RichText*/); });
        }
        if (combobox_style_scheme_pt->get_active_text() == Glib::ustring{styleId}) {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
        }
        if (combobox_style_scheme_ta->get_active_text() == Glib::ustring{styleId}) {
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
        pVBoxThemeEditor[i] = Gtk::manage(new Gtk::Box{
    #if GTKMM_MAJOR_VERSION >= 4
            Gtk::Orientation::HORIZONTAL
    #else
            Gtk::ORIENTATION_HORIZONTAL
    #endif
        });
        pGridThemeEditor[i] = Gtk::manage(new Gtk::Grid{});
        pGridThemeEditor[i]->set_row_homogeneous(true);
        pGridThemeEditor[i]->set_column_spacing(4);
    #if GTKMM_MAJOR_VERSION >= 4
        pGridThemeEditor[i]->set_margin_start(4);
        pGridThemeEditor[i]->set_margin_end(4);
        pGridThemeEditor[i]->set_margin_top(4);
        pGridThemeEditor[i]->set_margin_bottom(4);
    #else
        pGridThemeEditor[i]->set_border_width(4);
    #endif

        pLabelTextFg[i] = Gtk::manage(new Gtk::Label{_("Text Foreground")});
        pColorButtonTextFg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleTextFg[i]}});
        pLabelTextBg[i] = Gtk::manage(new Gtk::Label{_("Text Background")});
        pColorButtonTextBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleTextBg[i]}});
        pLabelSelectionFg[i] = Gtk::manage(new Gtk::Label{_("Selection Foreground")});
        pColorButtonSelectionFg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleSelectionFg[i]}});
        pLabelSelectionBg[i] = Gtk::manage(new Gtk::Label{_("Selection Background")});
        pColorButtonSelectionBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleSelectionBg[i]}});
        pLabelCursor[i] = Gtk::manage(new Gtk::Label{_("Cursor")});
        pColorButtonCursor[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleCursor[i]}});
        pLabelCurrentLineBg[i] = Gtk::manage(new Gtk::Label{_("Current Line Background")});
        pColorButtonCurrentLineBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleCurrentLineBg[i]}});
        pLabelLineNumbersFg[i] = Gtk::manage(new Gtk::Label{_("Line Numbers Foreground")});
        pColorButtonLineNumbersFg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleLineNumbersFg[i]}});
        pLabelLineNumbersBg[i] = Gtk::manage(new Gtk::Label{_("Line Numbers Background")});
        pColorButtonLineNumbersBg[i] = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{_pConfig->userStyleLineNumbersBg[i]}});
        pButtonsResetToDefault[i] = Gtk::manage(new Gtk::Button{});
    #if GTKMM_MAJOR_VERSION < 4
        pButtonsResetToDefault[i]->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    #endif
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

        
    #if GTKMM_MAJOR_VERSION >= 4
        pVBoxThemeEditor[i]->append(*pGridThemeEditor[i]);
        pVBoxThemeEditor[i]->append(*pButtonsResetToDefault[i]);
    #else
        pVBoxThemeEditor[i]->pack_start(*pGridThemeEditor[i], false, false);
        pVBoxThemeEditor[i]->pack_start(*pButtonsResetToDefault[i], true, false);
    #endif

        pNotebook->append_page(*(pVBoxThemeEditor[i]), CtConfig::get_user_style_id(i+1));

        pColorButtonTextFg[i]->signal_color_set().connect([this, pColorButtonTextFg, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonTextFg[i]->get_rgba());
            if (rgba != _pConfig->userStyleTextFg[i]) {
                _pConfig->userStyleTextFg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonTextBg[i]->signal_color_set().connect([this, pColorButtonTextBg, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonTextBg[i]->get_rgba());
            if (rgba != _pConfig->userStyleTextBg[i]) {
                _pConfig->userStyleTextBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonSelectionFg[i]->signal_color_set().connect([this, pColorButtonSelectionFg, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonSelectionFg[i]->get_rgba());
            if (rgba != _pConfig->userStyleSelectionFg[i]) {
                _pConfig->userStyleSelectionFg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonSelectionBg[i]->signal_color_set().connect([this, pColorButtonSelectionBg, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonSelectionBg[i]->get_rgba());
            if (rgba != _pConfig->userStyleSelectionBg[i]) {
                _pConfig->userStyleSelectionBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonCursor[i]->signal_color_set().connect([this, pColorButtonCursor, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonCursor[i]->get_rgba());
            if (rgba != _pConfig->userStyleCursor[i]) {
                _pConfig->userStyleCursor[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonCurrentLineBg[i]->signal_color_set().connect([this, pColorButtonCurrentLineBg, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonCurrentLineBg[i]->get_rgba());
            if (rgba != _pConfig->userStyleCurrentLineBg[i]) {
                _pConfig->userStyleCurrentLineBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonLineNumbersFg[i]->signal_color_set().connect([this, pColorButtonLineNumbersFg, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonLineNumbersFg[i]->get_rgba());
            if (rgba != _pConfig->userStyleLineNumbersFg[i]) {
                _pConfig->userStyleLineNumbersFg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pColorButtonLineNumbersBg[i]->signal_color_set().connect([this, pColorButtonLineNumbersBg, i, f_onUserStyleChanged](){
            const std::string rgba = CtRgbUtil::rgb_to_string_24(pColorButtonLineNumbersBg[i]->get_rgba());
            if (rgba != _pConfig->userStyleLineNumbersBg[i]) {
                _pConfig->userStyleLineNumbersBg[i] = rgba;
                f_onUserStyleChanged(i+1);
            }
        });
        pButtonsResetToDefault[i]->signal_clicked().connect([this, i,
                                                             pColorButtonTextFg,
                                                             pColorButtonTextBg,
                                                             pColorButtonSelectionFg,
                                                             pColorButtonSelectionBg,
                                                             pColorButtonCursor,
                                                             pColorButtonCurrentLineBg,
                                                             pColorButtonLineNumbersFg,
                                                             pColorButtonLineNumbersBg,
                                                             f_onUserStyleChanged](){
            if (not CtDialogs::question_dialog(reset_warning, *this)) {
                return;
            }
            bool anyChange{false};
            if (_pConfig->userStyleTextFg[i] != CtConst::USER_STYLE_TEXT_FG[i]) {
                _pConfig->userStyleTextFg[i] = CtConst::USER_STYLE_TEXT_FG[i];
                pColorButtonTextFg[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleTextFg[i]});
                anyChange = true;
            }
            if (_pConfig->userStyleTextBg[i] != CtConst::USER_STYLE_TEXT_BG[i]) {
                _pConfig->userStyleTextBg[i] = CtConst::USER_STYLE_TEXT_BG[i];
                pColorButtonTextBg[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleTextBg[i]});
                anyChange = true;
            }
            if (_pConfig->userStyleSelectionFg[i] != CtConst::USER_STYLE_SELECTION_FG[i]) {
                _pConfig->userStyleSelectionFg[i] = CtConst::USER_STYLE_SELECTION_FG[i];
                pColorButtonSelectionFg[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleSelectionFg[i]});
                anyChange = true;
            }
            if (_pConfig->userStyleSelectionBg[i] != CtConst::USER_STYLE_SELECTION_BG[i]) {
                _pConfig->userStyleSelectionBg[i] = CtConst::USER_STYLE_SELECTION_BG[i];
                pColorButtonSelectionBg[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleSelectionBg[i]});
                anyChange = true;
            }
            if (_pConfig->userStyleCursor[i] != CtConst::USER_STYLE_CURSOR[i]) {
                _pConfig->userStyleCursor[i] = CtConst::USER_STYLE_CURSOR[i];
                pColorButtonCursor[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleCursor[i]});
                anyChange = true;
            }
            if (_pConfig->userStyleCurrentLineBg[i] != CtConst::USER_STYLE_CURRENT_LINE_BG[i]) {
                _pConfig->userStyleCurrentLineBg[i] = CtConst::USER_STYLE_CURRENT_LINE_BG[i];
                pColorButtonCurrentLineBg[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleCurrentLineBg[i]});
                anyChange = true;
            }
            if (_pConfig->userStyleLineNumbersFg[i] != CtConst::USER_STYLE_LINE_NUMBERS_FG[i]) {
                _pConfig->userStyleLineNumbersFg[i] = CtConst::USER_STYLE_LINE_NUMBERS_FG[i];
                pColorButtonLineNumbersFg[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleLineNumbersFg[i]});
                anyChange = true;
            }
            if (_pConfig->userStyleLineNumbersBg[i] != CtConst::USER_STYLE_LINE_NUMBERS_BG[i]) {
                _pConfig->userStyleLineNumbersBg[i] = CtConst::USER_STYLE_LINE_NUMBERS_BG[i];
                pColorButtonLineNumbersBg[i]->set_rgba(Gdk::RGBA{_pConfig->userStyleLineNumbersBg[i]});
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

    // Icon Theme
        auto pVBoxIconTheme = Gtk::manage(new Gtk::Box{
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
    #else
        Gtk::ORIENTATION_VERTICAL
    #endif
        });
    auto pButtonBreezeDarkIcons = Gtk::manage(new Gtk::Button{_("Set Dark Theme Icons")});
    auto pButtonBreezeLightIcons = Gtk::manage(new Gtk::Button{_("Set Light Theme Icons")});
    auto pButtonDefaultIcons = Gtk::manage(new Gtk::Button{_("Set Default Icons")});
#if GTKMM_MAJOR_VERSION >= 4
    pVBoxIconTheme->append(*pButtonBreezeDarkIcons);
    pVBoxIconTheme->append(*pButtonBreezeLightIcons);
    pVBoxIconTheme->append(*pButtonDefaultIcons);
#else
    pVBoxIconTheme->pack_start(*pButtonBreezeDarkIcons, false, false);
    pVBoxIconTheme->pack_start(*pButtonBreezeLightIcons, false, false);
    pVBoxIconTheme->pack_start(*pButtonDefaultIcons, false, false);
#endif

    auto f_removeConfigIconsAndCopyFrom = [](const char* folderName){
        fs::path ConfigIcons_dst = fs::get_cherrytree_config_icons_dirpath();
        if (fs::exists(ConfigIcons_dst)) {
            fs::remove_all(ConfigIcons_dst);
        }
        if (folderName) {
            g_mkdir(ConfigIcons_dst.c_str(), 0744);
            fs::path InstalledIcons_src = fs::get_cherrytree_datadir() / CtConfig::ConfigIconsDirname / folderName;
            for (const auto& filepath : fs::get_dir_entries(InstalledIcons_src)) {
                fs::copy_file(filepath, ConfigIcons_dst / filepath.filename());
            }
        }
        
    #if GTKMM_MAJOR_VERSION >= 4
        // GTK4 icon theme reload happens automatically; no explicit rescan API.
    #else
        Gtk::IconTheme::get_default()->rescan_if_needed();
    #endif
    };

    pButtonBreezeDarkIcons->signal_clicked().connect([f_removeConfigIconsAndCopyFrom](){
        f_removeConfigIconsAndCopyFrom("Breeze_Dark_icons");
    });
    pButtonBreezeLightIcons->signal_clicked().connect([f_removeConfigIconsAndCopyFrom](){
        f_removeConfigIconsAndCopyFrom("Breeze_Light_icons");
    });
    pButtonDefaultIcons->signal_clicked().connect([f_removeConfigIconsAndCopyFrom](){
        f_removeConfigIconsAndCopyFrom(nullptr);
    });

    Gtk::Frame* frame_icon_theme = new_managed_frame_with_align(_("Icon Theme"), pVBoxIconTheme);

        auto pVBoxMain = Gtk::manage(new Gtk::Box{
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Orientation::VERTICAL
    #else
        Gtk::ORIENTATION_VERTICAL
    #endif
        });
    pVBoxMain->set_margin_start(6);
    pVBoxMain->set_margin_top(6);
#if GTKMM_MAJOR_VERSION >= 4
    pVBoxMain->append(*frame_tt_theme);
    pVBoxMain->append(*frame_style_schemes);
    pVBoxMain->append(*frame_theme_editor);
    pVBoxMain->append(*frame_icon_theme);
#else
    pVBoxMain->pack_start(*frame_tt_theme, false, false);
    pVBoxMain->pack_start(*frame_style_schemes, false, false);
    pVBoxMain->pack_start(*frame_theme_editor, false, false);
    pVBoxMain->pack_start(*frame_icon_theme, false, false);
#endif

    combobox_style_scheme_rt->signal_changed().connect([this, combobox_style_scheme_rt](){
        _pConfig->rtStyleScheme = combobox_style_scheme_rt->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('r'/*RichText*/); });
    });

    combobox_style_scheme_pt->signal_changed().connect([this, combobox_style_scheme_pt](){
        _pConfig->ptStyleScheme = combobox_style_scheme_pt->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
    });

    combobox_style_scheme_ta->signal_changed().connect([this, combobox_style_scheme_ta](){
        _pConfig->taStyleScheme = combobox_style_scheme_ta->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('t'/*Table*/); });
    });

    combobox_style_scheme_co->signal_changed().connect([this, combobox_style_scheme_co](){
        _pConfig->coStyleScheme = combobox_style_scheme_co->get_active_text();
        apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
    });

    return pVBoxMain;
}
