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

    // Theme Editor
    auto grid_theme_editor = Gtk::manage(new Gtk::Grid{});

    Gtk::Frame* frame_theme_editor = new_managed_frame_with_align(_("Style Scheme Editor"), grid_theme_editor);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_tt_theme, false, false);
    pMainBox->pack_start(*frame_rt_theme, false, false);
    pMainBox->pack_start(*frame_pt_theme, false, false);
    pMainBox->pack_start(*frame_ta_theme, false, false);
    pMainBox->pack_start(*frame_theme_editor, false, false);

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
