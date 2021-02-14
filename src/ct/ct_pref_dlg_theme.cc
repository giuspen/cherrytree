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

    // Rich Text Theme
    auto vbox_rt_theme = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});

    auto hbox_style_scheme_rt = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_style_scheme_rt = Gtk::manage(new Gtk::Label{_("Style Scheme")});
    auto combobox_style_scheme_rt = Gtk::manage(new Gtk::ComboBoxText{});
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids()) {
        combobox_style_scheme_rt->append(scheme);
    }
    combobox_style_scheme_rt->set_active_text(pConfig->rtStyleScheme);
    hbox_style_scheme_rt->pack_start(*label_style_scheme_rt, false, false);
    hbox_style_scheme_rt->pack_start(*combobox_style_scheme_rt, false, false);

    auto checkbutton_monospace_bg = Gtk::manage(new Gtk::CheckButton{_("Monospace Background")});
    std::string mono_color = pConfig->monospaceBg.empty() ? CtConst::DEFAULT_MONOSPACE_BG : pConfig->monospaceBg;
    auto colorbutton_monospace_bg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{mono_color}});
    auto hbox_monospace_bg = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    hbox_monospace_bg->pack_start(*checkbutton_monospace_bg, false, false);
    hbox_monospace_bg->pack_start(*colorbutton_monospace_bg, false, false);

    checkbutton_monospace_bg->set_active(!pConfig->monospaceBg.empty());
    colorbutton_monospace_bg->set_sensitive(!pConfig->monospaceBg.empty());

    vbox_rt_theme->pack_start(*hbox_style_scheme_rt, false, false);
    vbox_rt_theme->pack_start(*hbox_monospace_bg, false, false);
    Gtk::Frame* frame_rt_theme = new_managed_frame_with_align(_("Rich Text"), vbox_rt_theme);

    // Plain Text and Code Theme
    auto vbox_pt_theme = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});

    auto hbox_style_scheme_pt = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_style_scheme_pt = Gtk::manage(new Gtk::Label{_("Style Scheme")});
    auto combobox_style_scheme_pt = Gtk::manage(new Gtk::ComboBoxText());
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids()) {
        combobox_style_scheme_pt->append(scheme);
    }
    combobox_style_scheme_pt->set_active_text(pConfig->ptStyleScheme);
    hbox_style_scheme_pt->pack_start(*label_style_scheme_pt, false, false);
    hbox_style_scheme_pt->pack_start(*combobox_style_scheme_pt, false, false);

    vbox_pt_theme->pack_start(*hbox_style_scheme_pt, false, false);
    Gtk::Frame* frame_pt_theme = new_managed_frame_with_align(_("Plain Text and Code"), vbox_pt_theme);

    // Table Theme
    auto vbox_ta_theme = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});

    auto hbox_style_scheme_ta = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_style_scheme_ta = Gtk::manage(new Gtk::Label{_("Style Scheme")});
    auto combobox_style_scheme_ta = Gtk::manage(new Gtk::ComboBoxText());
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids()) {
        combobox_style_scheme_ta->append(scheme);
    }
    combobox_style_scheme_ta->set_active_text(pConfig->taStyleScheme);
    hbox_style_scheme_ta->pack_start(*label_style_scheme_ta, false, false);
    hbox_style_scheme_ta->pack_start(*combobox_style_scheme_ta, false, false);

    vbox_ta_theme->pack_start(*hbox_style_scheme_ta, false, false);
    Gtk::Frame* frame_ta_theme = new_managed_frame_with_align(_("Table"), vbox_ta_theme);

    // Theme Editor
    auto pGridThemeEditor = Gtk::manage(new Gtk::Grid{});
    pGridThemeEditor->set_row_homogeneous(true);
    pGridThemeEditor->set_column_spacing(4);

    auto pLabelTextFg = Gtk::manage(new Gtk::Label{_("Text Foreground")});
    auto pColorButtonTextFg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleTextFg}});
    auto pLabelTextBg = Gtk::manage(new Gtk::Label{_("Text Background")});
    auto pColorButtonTextBg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleTextBg}});
    auto pLabelSelectionFg = Gtk::manage(new Gtk::Label{_("Selection Foreground")});
    auto pColorButtonSelectionFg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleSelectionFg}});
    auto pLabelSelectionBg = Gtk::manage(new Gtk::Label{_("Selection Background")});
    auto pColorButtonSelectionBg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleSelectionBg}});
    auto pLabelCursor = Gtk::manage(new Gtk::Label{_("Cursor")});
    auto pColorButtonCursor = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleCursor}});
    auto pLabelCurrentLineBg = Gtk::manage(new Gtk::Label{_("Current Line Background")});
    auto pColorButtonCurrentLineBg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleCurrentLineBg}});
    auto pLabelLineNumbersFg = Gtk::manage(new Gtk::Label{_("Line Numbers Foreground")});
    auto pColorButtonLineNumbersFg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleLineNumbersFg}});
    auto pLabelLineNumbersBg = Gtk::manage(new Gtk::Label{_("Line Numbers Background")});
    auto pColorButtonLineNumbersBg = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA{pConfig->userStyleLineNumbersBg}});

    pGridThemeEditor->attach(*pLabelTextFg,              0, 0, 1, 1);
    pGridThemeEditor->attach(*pColorButtonTextFg,        1, 0, 1, 1);
    pGridThemeEditor->attach(*pLabelTextBg,              2, 0, 1, 1);
    pGridThemeEditor->attach(*pColorButtonTextBg,        3, 0, 1, 1);
    pGridThemeEditor->attach(*pLabelSelectionFg,         0, 1, 1, 1);
    pGridThemeEditor->attach(*pColorButtonSelectionFg,   1, 1, 1, 1);
    pGridThemeEditor->attach(*pLabelSelectionBg,         2, 1, 1, 1);
    pGridThemeEditor->attach(*pColorButtonSelectionBg,   3, 1, 1, 1);
    pGridThemeEditor->attach(*pLabelCursor,              0, 2, 1, 1);
    pGridThemeEditor->attach(*pColorButtonCursor,        1, 2, 1, 1);
    pGridThemeEditor->attach(*pLabelCurrentLineBg,       2, 2, 1, 1);
    pGridThemeEditor->attach(*pColorButtonCurrentLineBg, 3, 2, 1, 1);
    pGridThemeEditor->attach(*pLabelLineNumbersFg,       0, 3, 1, 1);
    pGridThemeEditor->attach(*pColorButtonLineNumbersFg, 1, 3, 1, 1);
    pGridThemeEditor->attach(*pLabelLineNumbersBg,       2, 3, 1, 1);
    pGridThemeEditor->attach(*pColorButtonLineNumbersBg, 3, 3, 1, 1);

    Gtk::Frame* frame_theme_editor = new_managed_frame_with_align(_("Style Scheme Editor"), pGridThemeEditor);

    auto f_onUserStyleChanged = [this,
                                 pConfig,
                                 combobox_style_scheme_rt,
                                 combobox_style_scheme_pt,
                                 combobox_style_scheme_ta](){
        pConfig->update_user_style();
        _pCtMainWin->get_style_scheme_manager()->force_rescan();
        if (combobox_style_scheme_rt->get_active_text() == "user-style") {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('r'/*RichText*/); });
        }
        if (combobox_style_scheme_pt->get_active_text() == "user-style") {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('p'/*PlainTextNCode*/); });
        }
        if (combobox_style_scheme_ta->get_active_text() == "user-style") {
            apply_for_each_window([](CtMainWin* win) { win->reapply_syntax_highlighting('t'/*Table*/); });
        }
    };
    pColorButtonTextFg->signal_color_set().connect([pColorButtonTextFg, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleTextFg = CtRgbUtil::rgb_any_to_24(pColorButtonTextFg->get_rgba());
        f_onUserStyleChanged();
    });
    pColorButtonTextBg->signal_color_set().connect([pColorButtonTextBg, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleTextBg = CtRgbUtil::rgb_any_to_24(pColorButtonTextBg->get_rgba());
        f_onUserStyleChanged();
    });
    pColorButtonSelectionFg->signal_color_set().connect([pColorButtonSelectionFg, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleSelectionFg = CtRgbUtil::rgb_any_to_24(pColorButtonSelectionFg->get_rgba());
        f_onUserStyleChanged();
    });
    pColorButtonSelectionBg->signal_color_set().connect([pColorButtonSelectionBg, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleSelectionBg = CtRgbUtil::rgb_any_to_24(pColorButtonSelectionBg->get_rgba());
        f_onUserStyleChanged();
    });
    pColorButtonCursor->signal_color_set().connect([pColorButtonCursor, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleCursor = CtRgbUtil::rgb_any_to_24(pColorButtonCursor->get_rgba());
        f_onUserStyleChanged();
    });
    pColorButtonCurrentLineBg->signal_color_set().connect([pColorButtonCurrentLineBg, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleCurrentLineBg = CtRgbUtil::rgb_any_to_24(pColorButtonCurrentLineBg->get_rgba());
        f_onUserStyleChanged();
    });
    pColorButtonLineNumbersFg->signal_color_set().connect([pColorButtonLineNumbersFg, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleLineNumbersFg = CtRgbUtil::rgb_any_to_24(pColorButtonLineNumbersFg->get_rgba());
        f_onUserStyleChanged();
    });
    pColorButtonLineNumbersBg->signal_color_set().connect([pColorButtonLineNumbersBg, pConfig, f_onUserStyleChanged](){
        pConfig->userStyleLineNumbersBg = CtRgbUtil::rgb_any_to_24(pColorButtonLineNumbersBg->get_rgba());
        f_onUserStyleChanged();
    });

    auto pVBoxMain = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    pVBoxMain->set_margin_left(6);
    pVBoxMain->set_margin_top(6);
    pVBoxMain->pack_start(*frame_tt_theme, false, false);
    pVBoxMain->pack_start(*frame_rt_theme, false, false);
    pVBoxMain->pack_start(*frame_pt_theme, false, false);
    pVBoxMain->pack_start(*frame_ta_theme, false, false);
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

    return pVBoxMain;
}
