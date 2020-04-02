/*
 * ct_pref_dlg.cc
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
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

CtPrefDlg::UniversalModelColumns::~UniversalModelColumns()
{
}

CtPrefDlg::CtPrefDlg(CtMainWin* parent)
 : Gtk::Dialog(_("Preferences"), *parent, true)
{
    _restartReasons = 0;
    _pCtMainWin = parent;
    _pCtMenu = &_pCtMainWin->get_ct_menu();

    Gtk::Notebook* pNotebook = Gtk::manage(new Gtk::Notebook());
    pNotebook->set_tab_pos(Gtk::PositionType::POS_LEFT);
    pNotebook->append_page(*build_tab_text_n_code(),       _("Text and Code"));
    pNotebook->append_page(*build_tab_text(),              _("Text"));
    pNotebook->append_page(*build_tab_rich_text(),         _("Rich Text"));
    pNotebook->append_page(*build_tab_plain_text_n_code(), _("Plain Text and Code"));
    pNotebook->append_page(*build_tab_tree_1(),            _("Tree") + std::string(" 1"));
    pNotebook->append_page(*build_tab_tree_2(),            _("Tree") + std::string(" 2"));
    pNotebook->append_page(*build_tab_fonts(),             _("Fonts"));
    pNotebook->append_page(*build_tab_links(),             _("Links"));
    pNotebook->append_page(*build_tab_toolbar(),           _("Toolbar"));
    pNotebook->append_page(*build_tab_kb_shortcuts(),      _("Keyboard Shortcuts"));
    pNotebook->append_page(*build_tab_misc(),              _("Miscellaneous"));

    get_content_area()->pack_start(*pNotebook);
    get_content_area()->show_all();

    add_button(Gtk::Stock::CLOSE, 1);
}

CtPrefDlg::~CtPrefDlg()
{
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
    Glib::RefPtr<Gtk::Adjustment> adj_wrapping_indent = Gtk::Adjustment::create(pConfig->wrappingIndent, -10000, 10000, 1);
    Gtk::SpinButton* spinbutton_wrapping_indent = Gtk::manage(new Gtk::SpinButton(adj_wrapping_indent));
    spinbutton_wrapping_indent->set_value(pConfig->wrappingIndent);
    hbox_wrapping_indent->pack_start(*label_wrapping_indent, false, false);
    hbox_wrapping_indent->pack_start(*spinbutton_wrapping_indent, false, false);
    Gtk::CheckButton* checkbutton_auto_indent = Gtk::manage(new Gtk::CheckButton(_("Enable Automatic Indentation")));
    checkbutton_auto_indent->set_active(pConfig->autoIndent);
    Gtk::CheckButton* checkbutton_line_nums = Gtk::manage(new Gtk::CheckButton(_("Show Line Numbers")));
    checkbutton_line_nums->set_active(pConfig->showLineNumbers);
    Gtk::HBox* hbox_space_around_lines = Gtk::manage(new Gtk::HBox());
    hbox_space_around_lines->set_spacing(4);
    Gtk::Label* label_space_around_lines = Gtk::manage(new Gtk::Label(_("Vertical Space Around Lines")));
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

    Gtk::VBox* vbox_text_editor = Gtk::manage(new Gtk::VBox());
    vbox_text_editor->pack_start(*hbox_tab_width, false, false);
    vbox_text_editor->pack_start(*checkbutton_spaces_tabs, false, false);
    vbox_text_editor->pack_start(*checkbutton_line_wrap, false, false);
    vbox_text_editor->pack_start(*hbox_wrapping_indent, false, false);
    vbox_text_editor->pack_start(*checkbutton_auto_indent, false, false);
    vbox_text_editor->pack_start(*checkbutton_line_nums, false, false);
    vbox_text_editor->pack_start(*hbox_space_around_lines, false, false);
    vbox_text_editor->pack_start(*hbox_relative_wrapped_space, false, false);
    Gtk::Frame* frame_text_editor = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Text Editor")+"</b>"));
    ((Gtk::Label*)frame_text_editor->get_label_widget())->set_use_markup(true);
    frame_text_editor->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_text_editor = Gtk::manage(new Gtk::Alignment());
    align_text_editor->set_padding(3, 6, 6, 6);
    align_text_editor->add(*vbox_text_editor);
    frame_text_editor->add(*align_text_editor);

    Gtk::HBox* hbox_timestamp = Gtk::manage(new Gtk::HBox());
    hbox_timestamp->set_spacing(4);
    Gtk::Label* label_timestamp = Gtk::manage(new Gtk::Label(_("Timestamp Format")));
    Gtk::Entry* entry_timestamp_format = Gtk::manage(new Gtk::Entry());
    entry_timestamp_format->set_text(pConfig->timestampFormat);
    Gtk::Button* button_strftime_help = Gtk::manage(new Gtk::Button());
    button_strftime_help->set_image(*_pCtMainWin->new_image_from_stock("help", Gtk::ICON_SIZE_BUTTON));
    hbox_timestamp->pack_start(*label_timestamp, false, false);
    hbox_timestamp->pack_start(*entry_timestamp_format, false, false);
    hbox_timestamp->pack_start(*button_strftime_help, false, false);
    Gtk::HBox* hbox_horizontal_rule = Gtk::manage(new Gtk::HBox());
    hbox_horizontal_rule->set_spacing(4);
    Gtk::Label* label_horizontal_rule = Gtk::manage(new Gtk::Label(_("Horizontal Rule")));
    Gtk::Entry* entry_horizontal_rule = Gtk::manage(new Gtk::Entry());
    entry_horizontal_rule->set_text(pConfig->hRule);
    hbox_horizontal_rule->pack_start(*label_horizontal_rule, false, false);
    hbox_horizontal_rule->pack_start(*entry_horizontal_rule);
    Gtk::HBox* hbox_special_chars = Gtk::manage(new Gtk::HBox());
    hbox_special_chars->set_spacing(4);
    Gtk::VBox* vbox_special_chars = Gtk::manage(new Gtk::VBox());
    Gtk::Label* label_special_chars = Gtk::manage(new Gtk::Label(_("Special Characters")));
    Gtk::HBox* hbox_reset = Gtk::manage(new Gtk::HBox());
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock("g-undo", Gtk::ICON_SIZE_BUTTON));
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
    textview_special_chars->get_buffer()->set_text(pConfig->specialChars);
    textview_special_chars->set_wrap_mode(Gtk::WRAP_CHAR);
    scrolledwindow_special_chars->add(*textview_special_chars);
    hbox_special_chars->pack_start(*vbox_special_chars, false, false);
    hbox_special_chars->pack_start(*frame_special_chars);
    Gtk::HBox* hbox_selword_chars = Gtk::manage(new Gtk::HBox());
    hbox_selword_chars->set_spacing(4);
    Gtk::Label* label_selword_chars = Gtk::manage(new Gtk::Label(_("Chars to Select at Double Click")));
    Gtk::Entry* entry_selword_chars = Gtk::manage(new Gtk::Entry());
    entry_selword_chars->set_text(pConfig->selwordChars);
    hbox_selword_chars->pack_start(*label_selword_chars, false, false);
    hbox_selword_chars->pack_start(*entry_selword_chars);

    Gtk::VBox* vbox_misc_all = Gtk::manage(new Gtk::VBox());
    vbox_misc_all->set_spacing(2);
    vbox_misc_all->pack_start(*hbox_timestamp);
    vbox_misc_all->pack_start(*hbox_horizontal_rule);
    vbox_misc_all->pack_start(*hbox_special_chars);
    vbox_misc_all->pack_start(*hbox_selword_chars);
    Gtk::Frame* frame_misc_all = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Miscellaneous")+"</b>"));
    ((Gtk::Label*)frame_misc_all->get_label_widget())->set_use_markup(true);
    frame_misc_all->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_misc_all = Gtk::manage(new Gtk::Alignment());
    align_misc_all->set_padding(3, 6, 6, 6);
    align_misc_all->add(*vbox_misc_all);
    frame_misc_all->add(*align_misc_all);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_text_editor, false, false);
    pMainBox->pack_start(*frame_misc_all, false, false);

    textview_special_chars->get_buffer()->signal_changed().connect([this, pConfig, textview_special_chars](){
        Glib::ustring new_special_chars = textview_special_chars->get_buffer()->get_text();
        str::replace(new_special_chars, CtConst::CHAR_NEWLINE, "");
        if (pConfig->specialChars != new_special_chars)
        {
            pConfig->specialChars = new_special_chars;
            _pCtMainWin->set_menu_items_special_chars();
        }
    });
    button_reset->signal_clicked().connect([this, textview_special_chars](){
        if (CtDialogs::question_dialog(reset_warning, *this))
            textview_special_chars->get_buffer()->set_text(CtConst::SPECIAL_CHARS_DEFAULT);
    });
    spinbutton_tab_width->signal_value_changed().connect([this, pConfig, spinbutton_tab_width](){
        pConfig->tabsWidth = spinbutton_tab_width->get_value_as_int();
        _pCtMainWin->get_text_view().set_tab_width((guint)pConfig->tabsWidth);
    });
    spinbutton_wrapping_indent->signal_value_changed().connect([this, pConfig, spinbutton_wrapping_indent](){
        pConfig->wrappingIndent = spinbutton_wrapping_indent->get_value_as_int();
        _pCtMainWin->get_text_view().set_indent(pConfig->wrappingIndent);
    });
    spinbutton_relative_wrapped_space->signal_value_changed().connect([this, pConfig, spinbutton_relative_wrapped_space](){
       pConfig->relativeWrappedSpace = spinbutton_relative_wrapped_space->get_value_as_int();
       _pCtMainWin->get_text_view().set_pixels_inside_wrap(pConfig->spaceAroundLines, pConfig->relativeWrappedSpace);
    });
    spinbutton_space_around_lines->signal_value_changed().connect([this, pConfig, spinbutton_space_around_lines](){
        pConfig->spaceAroundLines = spinbutton_space_around_lines->get_value_as_int();
        _pCtMainWin->get_text_view().set_pixels_above_lines(pConfig->spaceAroundLines);
        _pCtMainWin->get_text_view().set_pixels_below_lines(pConfig->spaceAroundLines);
        _pCtMainWin->get_text_view().set_pixels_inside_wrap(pConfig->spaceAroundLines, pConfig->relativeWrappedSpace);
    });
    checkbutton_spaces_tabs->signal_toggled().connect([this, pConfig, checkbutton_spaces_tabs](){
        pConfig->spacesInsteadTabs = checkbutton_spaces_tabs->get_active();
        _pCtMainWin->get_text_view().set_insert_spaces_instead_of_tabs(pConfig->spacesInsteadTabs);
    });
    checkbutton_line_wrap->signal_toggled().connect([this, pConfig, checkbutton_line_wrap](){
        pConfig->lineWrapping = checkbutton_line_wrap->get_active();
        _pCtMainWin->get_text_view().set_wrap_mode(pConfig->lineWrapping ? Gtk::WrapMode::WRAP_WORD_CHAR : Gtk::WrapMode::WRAP_NONE);
    });
    checkbutton_auto_indent->signal_toggled().connect([pConfig, checkbutton_auto_indent](){
        pConfig->autoIndent = checkbutton_auto_indent->get_active();
    });
    checkbutton_line_nums->signal_toggled().connect([this, pConfig, checkbutton_line_nums](){
        pConfig->showLineNumbers = checkbutton_line_nums->get_active();
        _pCtMainWin->get_text_view().set_show_line_numbers(pConfig->showLineNumbers);
    });
    entry_timestamp_format->signal_changed().connect([pConfig, entry_timestamp_format](){
        pConfig->timestampFormat = entry_timestamp_format->get_text();
    });
    button_strftime_help->signal_clicked().connect([](){
        if (0 != system("xdg-open https://docs.python.org/2/library/time.html#time.strftime")) g_print("? xdg-open");
    });
    entry_horizontal_rule->signal_changed().connect([pConfig, entry_horizontal_rule](){
        pConfig->hRule = entry_horizontal_rule->get_text();
    });
    entry_selword_chars->signal_changed().connect([pConfig, entry_selword_chars](){
        pConfig->selwordChars = entry_selword_chars->get_text();
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_text()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_editor = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_auto_smart_quotes = Gtk::manage(new Gtk::CheckButton(_("Enable Smart Quotes Auto Replacement")));
    Gtk::CheckButton* checkbutton_enable_symbol_autoreplace = Gtk::manage(new Gtk::CheckButton(_("Enable Symbol Auto Replacement")));
    checkbutton_auto_smart_quotes->set_active(pConfig->autoSmartQuotes);
    checkbutton_enable_symbol_autoreplace->set_active(pConfig->enableSymbolAutoreplace);

    vbox_editor->pack_start(*checkbutton_auto_smart_quotes, false, false);
    vbox_editor->pack_start(*checkbutton_enable_symbol_autoreplace, false, false);

    Gtk::Frame* frame_editor = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Text Editor")+"</b>"));
    ((Gtk::Label*)frame_editor->get_label_widget())->set_use_markup(true);
    frame_editor->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_editor = Gtk::manage(new Gtk::Alignment());
    align_editor->set_padding(3, 6, 6, 6);
    align_editor->add(*vbox_editor);
    frame_editor->add(*align_editor);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_editor, false, false);

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
    // todo:
    // for (auto& lang: some_spell_check_lang_list)
    //      combobox_spell_check_lang.append(lang);
    hbox_spell_check_lang->pack_start(*label_spell_check_lang, false, false);
    hbox_spell_check_lang->pack_start(*combobox_spell_check_lang);
    vbox_spell_check->pack_start(*checkbutton_spell_check, false, false);
    vbox_spell_check->pack_start(*hbox_spell_check_lang, false, false);
    Gtk::Frame* frame_spell_check = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Spell Check")+"</b>"));
    ((Gtk::Label*)frame_spell_check->get_label_widget())->set_use_markup(true);
    frame_spell_check->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_spell_check = Gtk::manage(new Gtk::Alignment());
    align_spell_check->set_padding(3, 6, 6, 6);
    align_spell_check->add(*vbox_spell_check);
    frame_spell_check->add(*align_spell_check);

    Gtk::VBox* vbox_rt_theme = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_rt_col_light = Gtk::manage(new Gtk::RadioButton(_("Light Background, Dark Text")));
    Gtk::RadioButton* radiobutton_rt_col_dark = Gtk::manage(new Gtk::RadioButton(_("Dark Background, Light Text")));
    radiobutton_rt_col_dark->join_group(*radiobutton_rt_col_light);
    Gtk::RadioButton* radiobutton_rt_col_custom = Gtk::manage(new Gtk::RadioButton(_("Custom Background")));
    radiobutton_rt_col_custom->join_group(*radiobutton_rt_col_light);
    Gtk::HBox* hbox_rt_col_custom = Gtk::manage(new Gtk::HBox());
    hbox_rt_col_custom->set_spacing(4);
    Gtk::ColorButton* colorbutton_text_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->rtDefBg)));
    Gtk::Label* label_rt_col_custom = Gtk::manage(new Gtk::Label(_("and Text")));
    Gtk::ColorButton* colorbutton_text_fg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->rtDefFg)));
    hbox_rt_col_custom->pack_start(*radiobutton_rt_col_custom, false, false);
    hbox_rt_col_custom->pack_start(*colorbutton_text_bg, false, false);
    hbox_rt_col_custom->pack_start(*label_rt_col_custom, false, false);
    hbox_rt_col_custom->pack_start(*colorbutton_text_fg, false, false);
    Gtk::CheckButton* checkbutton_monospace_bg = Gtk::manage(new Gtk::CheckButton(_("Monospace Background")));
    std::string mono_color = pConfig->monospaceBg.empty() ? CtConst::DEFAULT_MONOSPACE_BG : pConfig->monospaceBg;
    Gtk::ColorButton* colorbutton_monospace_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(mono_color)));
    Gtk::HBox* hbox_monospace_bg = Gtk::manage(new Gtk::HBox());
    hbox_monospace_bg->set_spacing(4);
    hbox_monospace_bg->pack_start(*checkbutton_monospace_bg, false, false);
    hbox_monospace_bg->pack_start(*colorbutton_monospace_bg, false, false);

    vbox_rt_theme->pack_start(*radiobutton_rt_col_light, false, false);
    vbox_rt_theme->pack_start(*radiobutton_rt_col_dark, false, false);
    vbox_rt_theme->pack_start(*hbox_rt_col_custom, false, false);
    vbox_rt_theme->pack_start(*hbox_monospace_bg, false, false);
    Gtk::Frame* frame_rt_theme = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Theme")+"</b>"));
    ((Gtk::Label*)frame_rt_theme->get_label_widget())->set_use_markup(true);
    frame_rt_theme->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_rt_theme = Gtk::manage(new Gtk::Alignment());
    align_rt_theme->set_padding(3, 6, 6, 6);
    align_rt_theme->add(*vbox_rt_theme);
    frame_rt_theme->add(*align_rt_theme);

    if (pConfig->rtDefFg == CtConst::RICH_TEXT_DARK_FG && pConfig->rtDefBg == CtConst::RICH_TEXT_DARK_BG)
    {
        radiobutton_rt_col_dark->set_active(true);
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
    }
    else if (pConfig->rtDefFg == CtConst::RICH_TEXT_LIGHT_FG && pConfig->rtDefBg == CtConst::RICH_TEXT_LIGHT_BG)
    {
        radiobutton_rt_col_light->set_active(true);
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
    }
    else
    {
        radiobutton_rt_col_custom->set_active(true);
    }
    if (!pConfig->monospaceBg.empty())
    {
        checkbutton_monospace_bg->set_active(true);
        colorbutton_monospace_bg->set_sensitive(true);
    }
    else
    {
        checkbutton_monospace_bg->set_active(false);
        colorbutton_monospace_bg->set_sensitive(false);
    }

    Gtk::HBox* hbox_misc_text = Gtk::manage(new Gtk::HBox());
    hbox_misc_text->set_spacing(4);
    Gtk::CheckButton* checkbutton_rt_show_white_spaces = Gtk::manage(new Gtk::CheckButton(_("Show White Spaces")));
    checkbutton_rt_show_white_spaces->set_active(pConfig->rtShowWhiteSpaces);
    Gtk::CheckButton* checkbutton_rt_highl_curr_line = Gtk::manage(new Gtk::CheckButton(_("Highlight Current Line")));
    checkbutton_rt_highl_curr_line->set_active(pConfig->rtHighlCurrLine);
    Gtk::CheckButton* checkbutton_codebox_auto_resize = Gtk::manage(new Gtk::CheckButton(_("Expand CodeBoxes Automatically")));
    checkbutton_codebox_auto_resize->set_active(pConfig->codeboxAutoResize);
    Gtk::HBox* hbox_embfile_size = Gtk::manage(new Gtk::HBox());
    hbox_embfile_size->set_spacing(4);
    Gtk::Label* label_embfile_size = Gtk::manage(new Gtk::Label(_("Embedded File Icon Size")));
    Glib::RefPtr<Gtk::Adjustment> adj_embfile_size = Gtk::Adjustment::create(pConfig->embfileSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_embfile_size = Gtk::manage(new Gtk::SpinButton(adj_embfile_size));
    spinbutton_embfile_size->set_value(pConfig->embfileSize);
    hbox_embfile_size->pack_start(*label_embfile_size, false, false);
    hbox_embfile_size->pack_start(*spinbutton_embfile_size, false, false);
    Gtk::CheckButton* checkbutton_embfile_show_filename = Gtk::manage(new Gtk::CheckButton(_("Show File Name on Top of Embedded File Icon")));
    checkbutton_embfile_show_filename->set_active(pConfig->embfileShowFileName);
    Gtk::Label* label_limit_undoable_steps = Gtk::manage(new Gtk::Label(_("Limit of Undoable Steps Per Node")));
    Glib::RefPtr<Gtk::Adjustment> adj_limit_undoable_steps = Gtk::Adjustment::create(pConfig->limitUndoableSteps, 1, 10000, 1);
    Gtk::SpinButton* spinbutton_limit_undoable_steps = Gtk::manage(new Gtk::SpinButton(adj_limit_undoable_steps));
    spinbutton_limit_undoable_steps->set_value(pConfig->limitUndoableSteps);
    hbox_misc_text->pack_start(*label_limit_undoable_steps, false, false);
    hbox_misc_text->pack_start(*spinbutton_limit_undoable_steps, false, false);

    Gtk::VBox* vbox_misc_text = Gtk::manage(new Gtk::VBox());
    vbox_misc_text->pack_start(*checkbutton_rt_show_white_spaces, false, false);
    vbox_misc_text->pack_start(*checkbutton_rt_highl_curr_line, false, false);
    vbox_misc_text->pack_start(*checkbutton_codebox_auto_resize, false, false);
    vbox_misc_text->pack_start(*hbox_embfile_size, false, false);
    vbox_misc_text->pack_start(*checkbutton_embfile_show_filename, false, false);
    vbox_misc_text->pack_start(*hbox_misc_text, false, false);
    Gtk::Frame* frame_misc_text = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Miscellaneous")+"</b>"));
    ((Gtk::Label*)frame_misc_text->get_label_widget())->set_use_markup(true);
    frame_misc_text->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_misc_text = Gtk::manage(new Gtk::Alignment());
    align_misc_text->set_padding(3, 6, 6, 6);
    align_misc_text->add(*vbox_misc_text);
    frame_misc_text->add(*align_misc_text);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_spell_check, false, false);
    pMainBox->pack_start(*frame_rt_theme, false, false);
    pMainBox->pack_start(*frame_misc_text, false, false);

    checkbutton_spell_check->signal_toggled().connect([pConfig, checkbutton_spell_check, combobox_spell_check_lang](){
        pConfig->enableSpellCheck = checkbutton_spell_check->get_active();
        //if dad.enable_spell_check:
        //    dad.spell_check_set_on()
        //    set_checkbutton_spell_check_model()
        //else: dad.spell_check_set_off(True)
        combobox_spell_check_lang->set_sensitive(pConfig->enableSpellCheck);
    });
    combobox_spell_check_lang->signal_changed().connect([/*pConfig, combobox_spell_check_lang*/](){
        //new_iter = combobox.get_active_iter()
        //new_lang_code = dad.spell_check_lang_liststore[new_iter][0]
        //if new_lang_code != dad.spell_check_lang: dad.spell_check_set_new_lang(new_lang_code)
    });
    colorbutton_text_fg->signal_color_set().connect([this, pConfig, colorbutton_text_fg](){
        pConfig->rtDefFg = CtRgbUtil::rgb_any_to_24(colorbutton_text_fg->get_rgba());
        _pCtMainWin->configure_theme();
    });
    colorbutton_text_bg->signal_color_set().connect([this, pConfig, colorbutton_text_bg](){
        pConfig->rtDefBg = CtRgbUtil::rgb_any_to_24(colorbutton_text_bg->get_rgba());
        _pCtMainWin->configure_theme();
    });
    radiobutton_rt_col_light->signal_toggled().connect([radiobutton_rt_col_light, colorbutton_text_fg, colorbutton_text_bg](){
        if (!radiobutton_rt_col_light->get_active()) return;
        colorbutton_text_fg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_LIGHT_FG));
        colorbutton_text_bg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_LIGHT_BG));
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
        g_signal_emit_by_name(colorbutton_text_fg->gobj(), "color-set");
        g_signal_emit_by_name(colorbutton_text_bg->gobj(), "color-set");
    });
    radiobutton_rt_col_dark->signal_toggled().connect([radiobutton_rt_col_dark, colorbutton_text_fg, colorbutton_text_bg](){
        if (!radiobutton_rt_col_dark->get_active()) return;
        colorbutton_text_fg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_DARK_FG));
        colorbutton_text_bg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_DARK_BG));
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
        g_signal_emit_by_name(colorbutton_text_fg->gobj(), "color-set");
        g_signal_emit_by_name(colorbutton_text_bg->gobj(), "color-set");
    });
    radiobutton_rt_col_custom->signal_toggled().connect([radiobutton_rt_col_custom, colorbutton_text_fg, colorbutton_text_bg](){
        if (!radiobutton_rt_col_custom->get_active()) return;
        colorbutton_text_fg->set_sensitive(true);
        colorbutton_text_bg->set_sensitive(true);
    });
    checkbutton_monospace_bg->signal_toggled().connect([this, pConfig, checkbutton_monospace_bg, colorbutton_monospace_bg](){
        if (checkbutton_monospace_bg->get_active())
        {
            pConfig->monospaceBg = CtRgbUtil::rgb_any_to_24(colorbutton_monospace_bg->get_rgba());
            colorbutton_monospace_bg->set_sensitive(true);
        } else {
            pConfig->monospaceBg = "";
            colorbutton_monospace_bg->set_sensitive(false);
        }
        need_restart(RESTART_REASON::MONOSPACE);
    });
    colorbutton_monospace_bg->signal_color_set().connect([this, pConfig, colorbutton_monospace_bg](){
        pConfig->monospaceBg = CtRgbUtil::rgb_any_to_24(colorbutton_monospace_bg->get_rgba());
        need_restart(RESTART_REASON::MONOSPACE);
    });
    checkbutton_rt_show_white_spaces->signal_toggled().connect([this, pConfig, checkbutton_rt_show_white_spaces](){
        pConfig->rtShowWhiteSpaces = checkbutton_rt_show_white_spaces->get_active();
        if (pConfig->syntaxHighlighting == CtConst::RICH_TEXT_ID)
            _pCtMainWin->get_text_view().set_draw_spaces(pConfig->rtShowWhiteSpaces ? CtCodebox::DRAW_SPACES_FLAGS : (Gsv::DrawSpacesFlags)0);
    });
    checkbutton_rt_highl_curr_line->signal_toggled().connect([this, pConfig, checkbutton_rt_highl_curr_line](){
        pConfig->rtHighlCurrLine = checkbutton_rt_highl_curr_line->get_active();
        if (pConfig->syntaxHighlighting == CtConst::RICH_TEXT_ID)
            _pCtMainWin->get_text_view().set_highlight_current_line(pConfig->rtHighlCurrLine);
    });
    checkbutton_codebox_auto_resize->signal_toggled().connect([pConfig, checkbutton_codebox_auto_resize](){
        pConfig->codeboxAutoResize = checkbutton_codebox_auto_resize->get_active();
    });
    spinbutton_embfile_size->signal_value_changed().connect([this, pConfig, spinbutton_embfile_size](){
        pConfig->embfileSize = spinbutton_embfile_size->get_value_as_int();
        need_restart(RESTART_REASON::EMBFILE_SIZE);
    });
    checkbutton_embfile_show_filename->signal_toggled().connect([this, pConfig, checkbutton_embfile_show_filename](){
        pConfig->embfileShowFileName = checkbutton_embfile_show_filename->get_active();
        need_restart(RESTART_REASON::SHOW_EMBFILE_NAME);
    });
    spinbutton_limit_undoable_steps->signal_value_changed().connect([pConfig, spinbutton_limit_undoable_steps](){
        pConfig->limitUndoableSteps = spinbutton_limit_undoable_steps->get_value_as_int();
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_plain_text_n_code()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_syntax = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_style_scheme = Gtk::manage(new Gtk::HBox());
    hbox_style_scheme->set_spacing(4);
    Gtk::Label* label_style_scheme = Gtk::manage(new Gtk::Label(_("Style Scheme")));
    Gtk::ComboBoxText* combobox_style_scheme = Gtk::manage(new Gtk::ComboBoxText());
    for (auto& scheme : _pCtMainWin->get_style_scheme_manager()->get_scheme_ids())
        combobox_style_scheme->append(scheme);
    combobox_style_scheme->set_active_text(pConfig->styleSchemeId);
    hbox_style_scheme->pack_start(*label_style_scheme, false, false);
    hbox_style_scheme->pack_start(*combobox_style_scheme, false, false);
    Gtk::CheckButton* checkbutton_pt_show_white_spaces = Gtk::manage(new Gtk::CheckButton(_("Show White Spaces")));
    checkbutton_pt_show_white_spaces->set_active(pConfig->ptShowWhiteSpaces);
    Gtk::CheckButton* checkbutton_pt_highl_curr_line = Gtk::manage(new Gtk::CheckButton(_("Highlight Current Line")));
    checkbutton_pt_highl_curr_line->set_active(pConfig->ptHighlCurrLine);

    vbox_syntax->pack_start(*hbox_style_scheme, false, false);
    vbox_syntax->pack_start(*checkbutton_pt_show_white_spaces, false, false);
    vbox_syntax->pack_start(*checkbutton_pt_highl_curr_line, false, false);

    Gtk::Frame* frame_syntax = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Text Editor")+"</b>"));
    ((Gtk::Label*)frame_syntax->get_label_widget())->set_use_markup(true);
    frame_syntax->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_syntax = Gtk::manage(new Gtk::Alignment());
    align_syntax->set_padding(3, 6, 6, 6);
    align_syntax->add(*vbox_syntax);
    frame_syntax->add(*align_syntax);

    Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(_commandModelColumns);
    fill_commands_model(liststore);
    Gtk::TreeView* treeview = Gtk::manage(new Gtk::TreeView(liststore));
    treeview->set_headers_visible(false);
    treeview->set_size_request(300, 200);
    treeview->append_column("", _commandModelColumns.icon);
    treeview->append_column("", _commandModelColumns.key);
    treeview->append_column_editable("", _commandModelColumns.desc);
    treeview->set_expander_column(*treeview->get_column(1));

    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow->add(*treeview);

    Gtk::Button* button_add = Gtk::manage(new Gtk::Button());
    button_add->set_image(*_pCtMainWin->new_image_from_stock("add", Gtk::ICON_SIZE_BUTTON));
    button_add->set_tooltip_text(_("Add"));
    Gtk::Button* button_reset_cmds = Gtk::manage(new Gtk::Button());
    button_reset_cmds->set_image(*_pCtMainWin->new_image_from_stock("g-undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_cmds->set_tooltip_text(_("Reset to Default"));
    Gtk::VBox* vbox_buttons = Gtk::manage(new Gtk::VBox());
    vbox_buttons->pack_start(*button_add, false, false);
    vbox_buttons->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    vbox_buttons->pack_start(*button_reset_cmds, false, false);

    Gtk::VBox* vbox_codexec = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_term_run = Gtk::manage(new Gtk::HBox());
    Gtk::Entry* entry_term_run = Gtk::manage(new Gtk::Entry());
    entry_term_run->set_text(get_code_exec_term_run(_pCtMainWin));
    Gtk::Button* button_reset_term = Gtk::manage(new Gtk::Button());
    button_reset_term->set_image(*_pCtMainWin->new_image_from_stock("g-undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_term->set_tooltip_text(_("Reset to Default"));
    hbox_term_run->pack_start(*entry_term_run, true, false);
    hbox_term_run->pack_start(*button_reset_term, false, false);
    Gtk::HBox* hbox_cmd_per_type = Gtk::manage(new Gtk::HBox());
    hbox_cmd_per_type->pack_start(*scrolledwindow, true, true);
    hbox_cmd_per_type->pack_start(*vbox_buttons, false, false);

    Gtk::Label* label = Gtk::manage(new Gtk::Label(std::string("<b>")+_("Command per Node/CodeBox Type")+"</b>"));
    label->set_use_markup(true);
    vbox_codexec->pack_start(*label, false, false);
    vbox_codexec->pack_start(*hbox_cmd_per_type, true, true);
    Gtk::Label* label2 = Gtk::manage(new Gtk::Label(std::string("<b>")+_("Terminal Command")+"</b>"));
    label2->set_use_markup(true);
    vbox_codexec->pack_start(*label2, false, false);
    vbox_codexec->pack_start(*hbox_term_run, false, false);

    Gtk::Frame* frame_codexec = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Code Execution")+"</b>"));
    ((Gtk::Label*)frame_codexec->get_label_widget())->set_use_markup(true);
    frame_codexec->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_codexec = Gtk::manage(new Gtk::Alignment());
    align_codexec->set_padding(3, 6, 6, 6);
    align_codexec->add(*vbox_codexec);
    frame_codexec->add(*align_codexec);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_syntax, false, false);
    pMainBox->pack_start(*frame_codexec, true, true);


    combobox_style_scheme->signal_changed().connect([this, pConfig, combobox_style_scheme](){
        pConfig->styleSchemeId = combobox_style_scheme->get_active_text();
        need_restart(RESTART_REASON::SCHEME);
    });
    checkbutton_pt_show_white_spaces->signal_toggled().connect([this, pConfig, checkbutton_pt_show_white_spaces](){
        pConfig->ptShowWhiteSpaces = checkbutton_pt_show_white_spaces->get_active();
        if (pConfig->syntaxHighlighting != CtConst::RICH_TEXT_ID)
            _pCtMainWin->get_text_view().set_draw_spaces(pConfig->ptShowWhiteSpaces ? CtCodebox::DRAW_SPACES_FLAGS : (Gsv::DrawSpacesFlags)0);
    });
    checkbutton_pt_highl_curr_line->signal_toggled().connect([this, pConfig, checkbutton_pt_highl_curr_line](){
        pConfig->ptHighlCurrLine = checkbutton_pt_highl_curr_line->get_active();
        if (pConfig->syntaxHighlighting != CtConst::RICH_TEXT_ID)
            _pCtMainWin->get_text_view().set_highlight_current_line(pConfig->ptHighlCurrLine);
    });
    ((Gtk::CellRendererText*)treeview->get_column(2)->get_cells()[0])->signal_edited().connect([this, pConfig, liststore](const Glib::ustring& path, const Glib::ustring& new_command){
        auto row = liststore->get_iter(path);
        // todo: the condition doesn't work because it already has the updated value (although docs say otherwise)
        // if (row->get_value(_commandModelColumns.command) == new_command) return;
        row->set_value(_commandModelColumns.desc, new_command);
        pConfig->customCodexecType[row->get_value(_commandModelColumns.key)] = new_command;
    });
    entry_term_run->signal_changed().connect([pConfig, entry_term_run](){
        pConfig->customCodexecTerm = entry_term_run->get_text();
    });
    button_add->signal_clicked().connect([this, liststore](){
        add_new_command_in_model(liststore);
    });
    button_reset_cmds->signal_clicked().connect([this, pConfig, liststore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->customCodexecType.clear();
            fill_commands_model(liststore);
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

Gtk::Widget* CtPrefDlg::build_tab_tree_1()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();
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
    Gtk::Frame* frame_tt_theme = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Theme")+"</b>"));
    ((Gtk::Label*)frame_tt_theme->get_label_widget())->set_use_markup(true);
    frame_tt_theme->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_tt_theme = Gtk::manage(new Gtk::Alignment());
    align_tt_theme->set_padding(3, 6, 6, 6);
    align_tt_theme->add(*vbox_tt_theme);
    frame_tt_theme->add(*align_tt_theme);

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

    Gtk::VBox* vbox_nodes_icons = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_node_icon_cherry = Gtk::manage(new Gtk::RadioButton(_("Use Different Cherries per Level")));
    Gtk::RadioButton* radiobutton_node_icon_custom = Gtk::manage(new Gtk::RadioButton(_("Use Selected Icon")));
    radiobutton_node_icon_custom->join_group(*radiobutton_node_icon_cherry);
    Gtk::RadioButton* radiobutton_node_icon_none = Gtk::manage(new Gtk::RadioButton(_("No Icon")));
    radiobutton_node_icon_none->join_group(*radiobutton_node_icon_cherry);
    Gtk::CheckButton* checkbutton_aux_icon_hide = Gtk::manage(new Gtk::CheckButton(_("Hide Right Side Auxiliary Icon")));
    checkbutton_aux_icon_hide->set_active(pConfig->auxIconHide);

    Gtk::Button* c_icon_button = Gtk::manage(new Gtk::Button());
    c_icon_button->set_image(*_pCtMainWin->new_image_from_stock(CtConst::NODES_STOCKS.at(pConfig->defaultIconText), Gtk::ICON_SIZE_BUTTON));
    Gtk::HBox* c_icon_hbox = Gtk::manage(new Gtk::HBox());
    c_icon_hbox->set_spacing(2);
    c_icon_hbox->pack_start(*radiobutton_node_icon_custom, false, false);
    c_icon_hbox->pack_start(*c_icon_button, false, false);

    vbox_nodes_icons->pack_start(*radiobutton_node_icon_cherry, false, false);
    vbox_nodes_icons->pack_start(*c_icon_hbox, false, false);
    vbox_nodes_icons->pack_start(*radiobutton_node_icon_none, false, false);
    vbox_nodes_icons->pack_start(*checkbutton_aux_icon_hide, false, false);
    Gtk::Frame* frame_nodes_icons = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Default Text Nodes Icons")+"</b>"));
    ((Gtk::Label*)frame_nodes_icons->get_label_widget())->set_use_markup(true);
    frame_nodes_icons->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_nodes_icons = Gtk::manage(new Gtk::Alignment());
    align_nodes_icons->set_padding(3, 6, 6, 6);
    align_nodes_icons->add(*vbox_nodes_icons);
    frame_nodes_icons->add(*align_nodes_icons);

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
    Gtk::Frame* frame_nodes_startup = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Nodes Status at Startup")+"</b>"));
    ((Gtk::Label*)frame_nodes_startup->get_label_widget())->set_use_markup(true);
    frame_nodes_startup->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_nodes_startup = Gtk::manage(new Gtk::Alignment());
    align_nodes_startup->set_padding(3, 6, 6, 6);
    align_nodes_startup->add(*vbox_nodes_startup);
    frame_nodes_startup->add(*align_nodes_startup);

    radiobutton_nodes_startup_restore->set_active(pConfig->restoreExpColl == CtRestoreExpColl::FROM_STR);
    radiobutton_nodes_startup_expand->set_active(pConfig->restoreExpColl == CtRestoreExpColl::ALL_EXP);
    radiobutton_nodes_startup_collapse->set_active(pConfig->restoreExpColl == CtRestoreExpColl::ALL_COLL);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_tt_theme, false, false);
    pMainBox->pack_start(*frame_nodes_icons, false, false);
    pMainBox->pack_start(*frame_nodes_startup, false, false);

    auto update_tree_color = [this, pConfig, colorbutton_tree_fg, colorbutton_tree_bg]() {
        pConfig->ttDefFg = CtRgbUtil::rgb_any_to_24(colorbutton_tree_fg->get_rgba());
        pConfig->ttDefBg = CtRgbUtil::rgb_any_to_24(colorbutton_tree_bg->get_rgba());
        _pCtMainWin->configure_theme();
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
    radiobutton_node_icon_cherry->signal_toggled().connect([this, pConfig, radiobutton_node_icon_cherry](){
        if (!radiobutton_node_icon_cherry->get_active()) return;
        pConfig->nodesIcons = "c";
        _pCtMainWin->curr_tree_store().refresh_node_icons(Gtk::TreeIter(), false);
    });
    radiobutton_node_icon_custom->signal_toggled().connect([this, pConfig, radiobutton_node_icon_custom](){
        if (!radiobutton_node_icon_custom->get_active()) return;
        pConfig->nodesIcons = "b";
        _pCtMainWin->curr_tree_store().refresh_node_icons(Gtk::TreeIter(), false);
    });
    radiobutton_node_icon_none->signal_toggled().connect([this, pConfig, radiobutton_node_icon_none](){
        if (!radiobutton_node_icon_none->get_active()) return;
        pConfig->nodesIcons = "n";
        _pCtMainWin->curr_tree_store().refresh_node_icons(Gtk::TreeIter(), false);
    });
    c_icon_button->signal_clicked().connect([this, pConfig, c_icon_button](){
        auto itemStore = CtChooseDialogListStore::create();
        for (auto& pair: CtConst::NODES_STOCKS)
            itemStore->add_row(pair.second, std::to_string(pair.first), "");
        auto res = CtDialogs::choose_item_dialog(*this, _("Select Node Icon"), itemStore);
        if (res) {
            pConfig->defaultIconText = std::stoi(res->get_value(itemStore->columns.key));
            c_icon_button->set_image(*_pCtMainWin->new_image_from_stock(res->get_value(itemStore->columns.stock_id), Gtk::ICON_SIZE_BUTTON));
            _pCtMainWin->curr_tree_store().refresh_node_icons(Gtk::TreeIter(), false);
        }
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
    checkbutton_aux_icon_hide->signal_toggled().connect([pConfig, checkbutton_aux_icon_hide](){
        pConfig->auxIconHide = checkbutton_aux_icon_hide->get_active();
        //dad.aux_renderer_pixbuf.set_property("visible", not dad.aux_icon_hide)
        //dad.treeview_refresh()
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_tree_2()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_misc_tree = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_tree_nodes_names_width = Gtk::manage(new Gtk::HBox());
    hbox_tree_nodes_names_width->set_spacing(4);
    Gtk::Label* label_tree_nodes_names_width = Gtk::manage(new Gtk::Label(_("Tree Nodes Names Wrapping Width")));
    Glib::RefPtr<Gtk::Adjustment> adj_tree_nodes_names_width = Gtk::Adjustment::create(pConfig->cherryWrapWidth, 10, 10000, 1);
    Gtk::SpinButton* spinbutton_tree_nodes_names_width = Gtk::manage(new Gtk::SpinButton(adj_tree_nodes_names_width));
    spinbutton_tree_nodes_names_width->set_value(pConfig->cherryWrapWidth);
    hbox_tree_nodes_names_width->pack_start(*label_tree_nodes_names_width, false, false);
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
    Gtk::Frame* frame_misc_tree = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Miscellaneous")+"</b>"));
    ((Gtk::Label*)frame_misc_tree->get_label_widget())->set_use_markup(true);
    frame_misc_tree->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_misc_tree = Gtk::manage(new Gtk::Alignment());
    align_misc_tree->set_padding(3, 6, 6, 6);
    align_misc_tree->add(*vbox_misc_tree);
    frame_misc_tree->add(*align_misc_tree);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_misc_tree, false, false);

    spinbutton_tree_nodes_names_width->signal_value_changed().connect([pConfig, spinbutton_tree_nodes_names_width](){
        pConfig->cherryWrapWidth = spinbutton_tree_nodes_names_width->get_value_as_int();
        //dad.renderer_text.set_property('wrap-width', dad.cherry_wrap_width)
        //dad.treeview_refresh()
    });
    checkbutton_tree_right_side->signal_toggled().connect([pConfig, checkbutton_tree_right_side](){
        pConfig->treeRightSide = checkbutton_tree_right_side->get_active();
        //tree_width = dad.scrolledwindow_tree.get_allocation().width
        //text_width = dad.vbox_text.get_allocation().width
        //dad.hpaned.remove(dad.scrolledwindow_tree)
        //dad.hpaned.remove(dad.vbox_text)
        //if dad.tree_right_side:
        //    dad.hpaned.add1(dad.vbox_text)
        //    dad.hpaned.add2(dad.scrolledwindow_tree)
        //    dad.hpaned.set_property('position', text_width)
        //else:
        //    dad.hpaned.add1(dad.scrolledwindow_tree)
        //    dad.hpaned.add2(dad.vbox_text)
        //    dad.hpaned.set_property('position', tree_width)
    });
    checkbutton_tree_click_focus_text->signal_toggled().connect([pConfig, checkbutton_tree_click_focus_text](){
        pConfig->treeClickFocusText = checkbutton_tree_click_focus_text->get_active();
    });
    checkbutton_tree_click_expand->signal_toggled().connect([pConfig, checkbutton_tree_click_expand](){
        pConfig->treeClickExpand = checkbutton_tree_click_expand->get_active();
    });
    spinbutton_nodes_on_node_name_header->signal_value_changed().connect([this, pConfig, spinbutton_nodes_on_node_name_header](){
        pConfig->nodesOnNodeNameHeader = spinbutton_nodes_on_node_name_header->get_value_as_int();
        _pCtMainWin->window_header_update_num_last_visited();
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_fonts()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::Image* image_rt = _pCtMainWin->new_image_from_stock(Gtk::Stock::SELECT_FONT.id, Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_pt = _pCtMainWin->new_image_from_stock(Gtk::Stock::SELECT_FONT.id, Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_code = _pCtMainWin->new_image_from_stock("xml", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_tree = _pCtMainWin->new_image_from_stock("cherries", Gtk::ICON_SIZE_MENU);
    Gtk::Label* label_rt = Gtk::manage(new Gtk::Label(_("Rich Text")));
    Gtk::Label* label_pt = Gtk::manage(new Gtk::Label(_("Plain Text")));
    Gtk::Label* label_code = Gtk::manage(new Gtk::Label(_("Code Font")));
    Gtk::Label* label_tree = Gtk::manage(new Gtk::Label(_("Tree Font")));
    Gtk::FontButton* fontbutton_rt = Gtk::manage(new Gtk::FontButton(pConfig->rtFont));
    Gtk::FontButton* fontbutton_pt = Gtk::manage(new Gtk::FontButton(pConfig->ptFont));
    Gtk::FontButton* fontbutton_code = Gtk::manage(new Gtk::FontButton(pConfig->codeFont));
    Gtk::FontButton* fontbutton_tree = Gtk::manage(new Gtk::FontButton(pConfig->treeFont));
    Gtk::Table* table_fonts = Gtk::manage(new Gtk::Table(4, 3));
    table_fonts->set_row_spacings(2);
    table_fonts->set_col_spacings(4);
    table_fonts->attach(*image_rt, 0, 1, 0, 1);//, 0, 0); // todo: fix expand param
    table_fonts->attach(*image_pt, 0, 1, 1, 2);//, 0, 0);
    table_fonts->attach(*image_code, 0, 1, 2, 3);//, 0, 0);
    table_fonts->attach(*image_tree, 0, 1, 3, 4);//, 0, 0);
    table_fonts->attach(*label_rt, 1, 2, 0, 1);// 0, 0);
    table_fonts->attach(*label_pt, 1, 2, 1, 2);//, 0, 0);
    table_fonts->attach(*label_code, 1, 2, 2, 3);//, 0, 0);
    table_fonts->attach(*label_tree, 1, 2, 3, 4);//, 0, 0);
    table_fonts->attach(*fontbutton_rt, 2, 3, 0, 1);//, yoptions=0);
    table_fonts->attach(*fontbutton_pt, 2, 3, 1, 2);//, yoptions=0);
    table_fonts->attach(*fontbutton_code, 2, 3, 2, 3);//, yoptions=0);
    table_fonts->attach(*fontbutton_tree, 2, 3, 3, 4);//, yoptions=0);

    Gtk::Frame* frame_fonts = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Fonts")+"</b>"));
    ((Gtk::Label*)frame_fonts->get_label_widget())->set_use_markup(true);
    frame_fonts->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_fonts = Gtk::manage(new Gtk::Alignment());
    align_fonts->set_padding(3, 6, 6, 6);
    align_fonts->add(*table_fonts);
    frame_fonts->add(*align_fonts);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_fonts, false, false);

    fontbutton_rt->signal_font_set().connect([this, pConfig, fontbutton_rt](){
        pConfig->rtFont = fontbutton_rt->get_font_name();
        _pCtMainWin->configure_theme();
        //if dad.curr_tree_iter and dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    dad.sourceview.modify_font(pango.FontDescription(dad.rt_font))
    });
    fontbutton_pt->signal_font_set().connect([this, pConfig, fontbutton_pt](){
        pConfig->ptFont = fontbutton_pt->get_font_name();
        _pCtMainWin->configure_theme();
        //if not dad.curr_tree_iter: return
        //if dad.syntax_highlighting == cons.PLAIN_TEXT_ID:
        //    dad.sourceview.modify_font(pango.FontDescription(dad.pt_font))
        //elif dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    support.rich_text_node_modify_codeboxes_font(dad.curr_buffer.get_start_iter(), dad)
        //    support.rich_text_node_modify_tables_font(dad.curr_buffer.get_start_iter(), dad)
    });
    fontbutton_code->signal_font_set().connect([this, pConfig, fontbutton_code](){
        pConfig->codeFont = fontbutton_code->get_font_name();
        _pCtMainWin->configure_theme();
        //if not dad.curr_tree_iter: return
        //if dad.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
        //    dad.sourceview.modify_font(pango.FontDescription(dad.code_font))
        //elif dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    support.rich_text_node_modify_codeboxes_font(dad.curr_buffer.get_start_iter(), dad)
    });
    fontbutton_tree->signal_font_set().connect([this, pConfig, fontbutton_tree](){
        pConfig->treeFont = fontbutton_tree->get_font_name();
        _pCtMainWin->configure_theme();
        //dad.set_treeview_font()
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

    Gtk::Frame* frame_links_actions = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Custom Actions")+"</b>"));
    ((Gtk::Label*)frame_links_actions->get_label_widget())->set_use_markup(true);
    frame_links_actions->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_links_actions = Gtk::manage(new Gtk::Alignment());
    align_links_actions->set_padding(3, 6, 6, 6);
    align_links_actions->add(*vbox_links_actions);
    frame_links_actions->add(*align_links_actions);

    checkbutton_custom_weblink_cmd->set_active(pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_sensitive(pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_text(pConfig->weblinkCustomAct);
    checkbutton_custom_filelink_cmd->set_active(pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_sensitive(pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_text(pConfig->filelinkCustomAct);
    checkbutton_custom_folderlink_cmd->set_active(pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_sensitive(pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_text(pConfig->folderlinkCustomAct);

    Gtk::Table* table_links_colors = Gtk::manage(new Gtk::Table(2, 2));
    table_links_colors->set_row_spacings(2);
    table_links_colors->set_col_spacings(4);
    table_links_colors->set_homogeneous(true);

    Gtk::HBox* hbox_col_link_webs = Gtk::manage(new Gtk::HBox());
    hbox_col_link_webs->set_spacing(4);
    Gtk::Label* label_col_link_webs = Gtk::manage(new Gtk::Label(_("To WebSite")));
    Gtk::ColorButton* colorbutton_col_link_webs = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkWebs)));
    hbox_col_link_webs->pack_start(*label_col_link_webs, false, false);
    hbox_col_link_webs->pack_start(*colorbutton_col_link_webs, false, false);

    Gtk::HBox* hbox_col_link_node = Gtk::manage(new Gtk::HBox());
    hbox_col_link_node->set_spacing(4);
    Gtk::Label* label_col_link_node = Gtk::manage(new Gtk::Label(_("To Node")));
    Gtk::ColorButton* colorbutton_col_link_node = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkNode)));
    hbox_col_link_node->pack_start(*label_col_link_node, false, false);
    hbox_col_link_node->pack_start(*colorbutton_col_link_node, false, false);

    Gtk::HBox* hbox_col_link_file = Gtk::manage(new Gtk::HBox());
    hbox_col_link_file->set_spacing(4);
    Gtk::Label* label_col_link_file = Gtk::manage(new Gtk::Label(_("To File")));
    Gtk::ColorButton* colorbutton_col_link_file = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkFile)));
    hbox_col_link_file->pack_start(*label_col_link_file, false, false);
    hbox_col_link_file->pack_start(*colorbutton_col_link_file, false, false);

    Gtk::HBox* hbox_col_link_fold = Gtk::manage(new Gtk::HBox());
    hbox_col_link_fold->set_spacing(4);
    Gtk::Label* label_col_link_fold = Gtk::manage(new Gtk::Label(_("To Folder")));
    Gtk::ColorButton* colorbutton_col_link_fold = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkFold)));
    hbox_col_link_fold->pack_start(*label_col_link_fold, false, false);
    hbox_col_link_fold->pack_start(*colorbutton_col_link_fold, false, false);

    table_links_colors->attach(*hbox_col_link_webs, 0, 1, 0, 1);
    table_links_colors->attach(*hbox_col_link_node, 0, 1, 1, 2);
    table_links_colors->attach(*hbox_col_link_file, 1, 2, 0, 1);
    table_links_colors->attach(*hbox_col_link_fold, 1, 2, 1, 2);

    Gtk::Frame* frame_links_colors = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Colors")+"</b>"));
    ((Gtk::Label*)frame_links_colors->get_label_widget())->set_use_markup(true);
    frame_links_colors->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_links_colors = Gtk::manage(new Gtk::Alignment());
    align_links_colors->set_padding(3, 6, 6, 6);
    align_links_colors->add(*table_links_colors);
    frame_links_colors->add(*align_links_colors);

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

    Gtk::Frame* frame_links_misc = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Miscellaneous")+"</b>"));
    ((Gtk::Label*)frame_links_misc->get_label_widget())->set_use_markup(true);
    frame_links_misc->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_links_misc = Gtk::manage(new Gtk::Alignment());
    align_links_misc->set_padding(3, 6, 6, 6);
    align_links_misc->add(*vbox_links_misc);
    frame_links_misc->add(*align_links_misc);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
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
    treeview->append_column("", _toolbarModelColumns.icon);
    treeview->append_column("", _toolbarModelColumns.desc);
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->add(*treeview);

    Gtk::Button* button_add = Gtk::manage(new Gtk::Button());
    button_add->set_image(*_pCtMainWin->new_image_from_stock(Gtk::Stock::ADD.id,  Gtk::ICON_SIZE_BUTTON));
    button_add->set_tooltip_text(_("Add"));
    Gtk::Button* button_remove = Gtk::manage(new Gtk::Button());
    button_remove->set_image(*_pCtMainWin->new_image_from_stock(Gtk::Stock::REMOVE.id, Gtk::ICON_SIZE_BUTTON));
    button_remove->set_tooltip_text(_("Remove Selected"));
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock(Gtk::Stock::UNDO.id, Gtk::ICON_SIZE_BUTTON));
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
        if (add_new_item_in_toolbar_model(treeview, liststore))
            need_restart(RESTART_REASON::TOOLBAR);
    });
    button_remove->signal_clicked().connect([this, treeview, liststore](){
        liststore->erase(treeview->get_selection()->get_selected());
        update_config_toolbar_from_model(liststore);
        need_restart(RESTART_REASON::TOOLBAR);
    });
    button_reset->signal_clicked().connect([this, pConfig, liststore](){
        if (CtDialogs::question_dialog(reset_warning, *this)) {
            pConfig->toolbarUiList = CtConst::TOOLBAR_VEC_DEFAULT;
            fill_toolbar_model(liststore);
            need_restart(RESTART_REASON::TOOLBAR);
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
        need_restart(RESTART_REASON::TOOLBAR);
    });

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
    treeview->append_column("", _shortcutModelColumns.icon);
    treeview->append_column("", _shortcutModelColumns.shortcut);
    treeview->append_column("", _shortcutModelColumns.desc);
    treeview->expand_all();
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->add(*treeview);

    Gtk::VBox* vbox_buttons = Gtk::manage(new Gtk::VBox());
    Gtk::Button* button_edit = Gtk::manage(new Gtk::Button());
    button_edit->set_image(*_pCtMainWin->new_image_from_stock(Gtk::Stock::ADD.id,  Gtk::ICON_SIZE_BUTTON));
    button_edit->set_tooltip_text(_("Change Selected"));
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*_pCtMainWin->new_image_from_stock(Gtk::Stock::UNDO.id,  Gtk::ICON_SIZE_BUTTON));
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

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_misc()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::VBox* vbox_system_tray = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_systray = Gtk::manage(new Gtk::CheckButton(_("Enable System Tray Docking")));
    Gtk::CheckButton* checkbutton_start_on_systray = Gtk::manage(new Gtk::CheckButton(_("Start Minimized in the System Tray")));
    Gtk::CheckButton* checkbutton_use_appind = Gtk::manage(new Gtk::CheckButton(_("Use AppIndicator for Docking")));
    vbox_system_tray->pack_start(*checkbutton_systray, false, false);
    vbox_system_tray->pack_start(*checkbutton_start_on_systray, false, false);
    vbox_system_tray->pack_start(*checkbutton_use_appind, false, false);

    Gtk::Frame* frame_system_tray = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("System Tray")+"</b>"));
    ((Gtk::Label*)frame_system_tray->get_label_widget())->set_use_markup(true);
    frame_system_tray->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_system_tray = Gtk::manage(new Gtk::Alignment());
    align_system_tray->set_padding(3, 6, 6, 6);
    align_system_tray->add(*vbox_system_tray);
    frame_system_tray->add(*align_system_tray);

    checkbutton_systray->set_active(pConfig->systrayOn);
    checkbutton_start_on_systray->set_active(pConfig->startOnSystray);
    checkbutton_start_on_systray->set_sensitive(pConfig->systrayOn);
    checkbutton_use_appind->set_active(pConfig->useAppInd);
    // todo:
    //if not cons->HAS_APPINDICATOR or not cons->HAS_SYSTRAY: checkbutton_use_appind->set_sensitive(False)

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
    hbox_num_backups->pack_start(*label_num_backups, false, false);
    hbox_num_backups->pack_start(*spinbutton_num_backups, false, false);
    vbox_saving->pack_start(*hbox_autosave, false, false);
    vbox_saving->pack_start(*checkbutton_autosave_on_quit, false, false);
    vbox_saving->pack_start(*checkbutton_backup_before_saving, false, false);
    vbox_saving->pack_start(*hbox_num_backups, false, false);

    checkbutton_autosave->set_active(pConfig->autosaveOn);
    spinbutton_autosave->set_value(pConfig->autosaveVal);
    spinbutton_autosave->set_sensitive(pConfig->autosaveOn);
    checkbutton_autosave_on_quit->set_active(pConfig->autosaveOnQuit);
    checkbutton_backup_before_saving->set_active(pConfig->backupCopy);

    Gtk::Frame* frame_saving = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Saving")+"</b>"));
    ((Gtk::Label*)frame_saving->get_label_widget())->set_use_markup(true);
    frame_saving->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_saving = Gtk::manage(new Gtk::Alignment());
    align_saving->set_padding(3, 6, 6, 6);
    align_saving->add(*vbox_saving);
    frame_saving->add(*align_saving);

    Gtk::VBox* vbox_misc_misc = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_newer_version = Gtk::manage(new Gtk::CheckButton(_("Automatically Check for Newer Version")));
    Gtk::CheckButton* checkbutton_word_count = Gtk::manage(new Gtk::CheckButton(_("Enable Word Count in Statusbar")));
    Gtk::CheckButton* checkbutton_reload_doc_last = Gtk::manage(new Gtk::CheckButton(_("Reload Document From Last Session")));
    Gtk::CheckButton* checkbutton_mod_time_sentinel = Gtk::manage(new Gtk::CheckButton(_("Reload After External Update to CT* File")));
    vbox_misc_misc->pack_start(*checkbutton_newer_version, false, false);
    vbox_misc_misc->pack_start(*checkbutton_word_count, false, false);
    vbox_misc_misc->pack_start(*checkbutton_reload_doc_last, false, false);
    vbox_misc_misc->pack_start(*checkbutton_mod_time_sentinel, false, false);

    checkbutton_newer_version->set_active(pConfig->checkVersion);
    checkbutton_word_count->set_active(pConfig->wordCountOn);
    checkbutton_reload_doc_last->set_active(pConfig->reloadDocLast);
    checkbutton_mod_time_sentinel->set_active(pConfig->modTimeSentinel);

    Gtk::Frame* frame_misc_misc = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Miscellaneous")+"</b>"));
    ((Gtk::Label*)frame_misc_misc->get_label_widget())->set_use_markup(true);
    frame_misc_misc->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_misc_misc = Gtk::manage(new Gtk::Alignment());
    align_misc_misc->set_padding(3, 6, 6, 6);
    align_misc_misc->add(*vbox_misc_misc);
    frame_misc_misc->add(*align_misc_misc);

    Gtk::VBox* vbox_language = Gtk::manage(new Gtk::VBox());
    Gtk::ComboBoxText* combobox_country_language = Gtk::manage(new Gtk::ComboBoxText());
    for (const gchar* lang: CtConst::AVAILABLE_LANGS)
        combobox_country_language->append(lang);
    // todo: country_lang (taken from s.path.isfile(cons.LANG_PATH))
    // combobox_country_language->set_active_text(pConfig->countryLang);
    vbox_language->pack_start(*combobox_country_language, false, false);
    Gtk::Frame* frame_language = Gtk::manage(new Gtk::Frame(std::string("<b>")+_("Language")+"</b>"));
    ((Gtk::Label*)frame_language->get_label_widget())->set_use_markup(true);
    frame_language->set_shadow_type(Gtk::SHADOW_NONE);
    Gtk::Alignment* align_language = Gtk::manage(new Gtk::Alignment());
    align_language->set_padding(3, 6, 6, 6);
    align_language->add(*vbox_language);
    frame_language->add(*align_language);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_system_tray, false, false);
    pMainBox->pack_start(*frame_saving, false, false);
    pMainBox->pack_start(*frame_misc_misc, false, false);
    pMainBox->pack_start(*frame_language, false, false);

    checkbutton_systray->signal_toggled().connect([pConfig, checkbutton_systray, checkbutton_start_on_systray](){
        pConfig->systrayOn = checkbutton_systray->get_active();
        if (pConfig->systrayOn) {
            //dad.ui.get_widget("/MenuBar/FileMenu/exit_app").set_property(cons.STR_VISIBLE, True)
            checkbutton_start_on_systray->set_sensitive(true);
        } else {
            //dad.ui.get_widget("/MenuBar/FileMenu/exit_app").set_property(cons.STR_VISIBLE, False)
            checkbutton_start_on_systray->set_sensitive(false);
        }
        //if dad.systray:
        //    if not dad.use_appind:
        //        if "status_icon" in dir(dad.boss): dad.boss.status_icon.set_property(cons.STR_VISIBLE, True)
        //        else: dad.status_icon_enable()
        //    else:
        //        if "ind" in dir(dad.boss): dad.boss.ind.set_status(appindicator.STATUS_ACTIVE)
        //        else: dad.status_icon_enable()
        //else:
        //    if not dad.use_appind: dad.boss.status_icon.set_property(cons.STR_VISIBLE, False)
        //    else: dad.boss.ind.set_status(appindicator.STATUS_PASSIVE)
        //dad.boss.systray_active = dad.systray
        //if len(dad.boss.running_windows) > 1:
        //    for runn_win in dad.boss.running_windows:
        //        if runn_win.window == dad.window: continue
        //        runn_win.systray = dad.boss.systray_active
    });
    checkbutton_start_on_systray->signal_toggled().connect([pConfig, checkbutton_start_on_systray](){
        pConfig->startOnSystray = checkbutton_start_on_systray->get_active();
    });
    checkbutton_use_appind->signal_toggled().connect([pConfig, checkbutton_use_appind, checkbutton_systray](){
        bool saved_systray_active = checkbutton_systray->get_active();
        if (saved_systray_active) checkbutton_systray->set_active(false); // todo: some hack?
        pConfig->useAppInd = checkbutton_use_appind->get_active();
        if (saved_systray_active) checkbutton_systray->set_active(true);
        //if len(dad.boss.running_windows) > 1:
        //    for runn_win in dad.boss.running_windows:
        //        if runn_win.window == dad.window: continue
        //        runn_win.use_appind = dad.use_appind
    });
    checkbutton_autosave->signal_toggled().connect([pConfig, checkbutton_autosave, spinbutton_autosave](){
        pConfig->autosaveOn = checkbutton_autosave->get_active();
        //if dad.autosave[0]:
        //    if dad.autosave_timer_id == None: dad.autosave_timer_start()
        //else:
        //    if dad.autosave_timer_id != None: dad.autosave_timer_stop()
        spinbutton_autosave->set_sensitive(pConfig->autosaveOn);
    });
    spinbutton_autosave->signal_value_changed().connect([pConfig, spinbutton_autosave](){
        pConfig->autosaveVal = spinbutton_autosave->get_value_as_int();
        //if dad.autosave_timer_id != None: dad.autosave_timer_stop()
        //if dad.autosave[0] and dad.autosave_timer_id == None: dad.autosave_timer_start()
    });
    spinbutton_num_backups->signal_value_changed().connect([pConfig, spinbutton_num_backups](){
        pConfig->backupNum = spinbutton_num_backups->get_value_as_int();
    });
    checkbutton_autosave_on_quit->signal_toggled().connect([pConfig, checkbutton_autosave_on_quit](){
        pConfig->autosaveOnQuit = checkbutton_autosave_on_quit->get_active();
    });
    checkbutton_reload_doc_last->signal_toggled().connect([pConfig, checkbutton_reload_doc_last](){
        pConfig->reloadDocLast = checkbutton_reload_doc_last->get_active();
    });
    checkbutton_mod_time_sentinel->signal_toggled().connect([pConfig, checkbutton_mod_time_sentinel](){
        pConfig->modTimeSentinel = checkbutton_mod_time_sentinel->get_active();
        if (pConfig->modTimeSentinel) {
            //if dad.mod_time_sentinel_id == None:
            //    dad.modification_time_sentinel_start()
        } else {
            //if dad.mod_time_sentinel_id != None:
            //    dad.modification_time_sentinel_stop()
        }
    });
    checkbutton_newer_version->signal_toggled().connect([pConfig, checkbutton_newer_version](){
        pConfig->checkVersion = checkbutton_newer_version->get_active();
    });
    checkbutton_word_count->signal_toggled().connect([&](){
        pConfig->wordCountOn = checkbutton_word_count->get_active();
        _pCtMainWin->update_selected_node_statusbar_info();
    });
    combobox_country_language->signal_changed().connect([this, /*pConfig, */combobox_country_language](){
        Glib::ustring new_lang = combobox_country_language->get_active_text();
        need_restart(RESTART_REASON::LANG, _("The New Language will be Available Only After Restarting CherryTree"));
        // pConfig->countryLang = new_lang;
        //dad.country_lang = new_lang
        //lang_file_descriptor = file(cons.LANG_PATH, 'w')
        //lang_file_descriptor.write(new_lang)
        //lang_file_descriptor.close()

    });

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
    std::string op_sys = CtConst::IS_WIN_OS ? "win" : "linux";
    if (!pCtMainWin->get_ct_config()->customCodexecTerm.empty())
        return pCtMainWin->get_ct_config()->customCodexecTerm;
    else
        return CtConst::CODE_EXEC_TERM_RUN_DEFAULT.at(op_sys);
}

void CtPrefDlg::fill_commands_model(Glib::RefPtr<Gtk::ListStore> model)
{
    std::set<std::string> used_code_exec_keys;
    for (auto& it: _pCtMainWin->get_ct_config()->customCodexecType)
        used_code_exec_keys.insert(it.first);
    for (const auto& it: CtConst::CODE_EXEC_TYPE_CMD_DEFAULT)
        used_code_exec_keys.insert(it.first);

    model->clear();
    for (auto& key: used_code_exec_keys)
    {
        std::string command;
        if (_pCtMainWin->get_ct_config()->customCodexecType.find(key) != _pCtMainWin->get_ct_config()->customCodexecType.end())
            command = _pCtMainWin->get_ct_config()->customCodexecType.at(key);
        else if (CtConst::CODE_EXEC_TYPE_CMD_DEFAULT.find(key) != CtConst::CODE_EXEC_TYPE_CMD_DEFAULT.end())
            command = CtConst::CODE_EXEC_TYPE_CMD_DEFAULT.at(key);

        Gtk::TreeModel::Row row = *(model->append());
        row[_commandModelColumns.icon] = _pCtMainWin->get_icon(CtConst::getStockIdForCodeType(key), CtConst::NODE_ICON_SIZE);
        row[_commandModelColumns.key] = key;
        row[_commandModelColumns.desc] = command;
    }
}

void CtPrefDlg::add_new_command_in_model(Glib::RefPtr<Gtk::ListStore> /*model*/)
{
    std::set<std::string> used_code_exec_keys;
    for (auto& it: _pCtMainWin->get_ct_config()->customCodexecType)
        used_code_exec_keys.insert(it.first);
    for (const auto& it: CtConst::CODE_EXEC_TYPE_CMD_DEFAULT)
        used_code_exec_keys.insert(it.first);
    // todo: add code based on this:
    /*
    def on_button_add_clicked(button):
        icon_n_key_list = []
        all_codexec_keys = get_code_exec_type_keys(dad)
        for key in dad.available_languages:
            if not key in all_codexec_keys:
                stock_id = get_stock_id_for_code_type(key)
                icon_n_key_list.append([key, stock_id, key])
        sel_key = support.dialog_choose_element_in_list(dad.window, _("Select Element to Add"), [], "", icon_n_key_list)
        if sel_key:
            default_type_command = "REPLACE_ME %s" % CODE_EXEC_TMP_SRC
            liststore_append_element(sel_key, default_type_command)
            dad.custom_codexec_type[sel_key] = default_type_command
     */
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
        icon = "";
        desc = CtConst::TAG_SEPARATOR_ANSI_REPR;
    }
    else if (key == CtConst::CHAR_STAR)
    {
        icon = "open";
        desc = _("Open a CherryTree Document");

    }
    else if (CtAction const* action = _pCtMenu->find_action(key))
    {
        icon = action->image;
        desc = action->desc;
    }

    if (icon != "") row->set_value(_toolbarModelColumns.icon, _pCtMainWin->get_icon(icon, CtConst::NODE_ICON_SIZE));
    row->set_value(_toolbarModelColumns.key, key);
    row->set_value(_toolbarModelColumns.desc, desc);
}

bool CtPrefDlg::add_new_item_in_toolbar_model(Gtk::TreeView* treeview, Glib::RefPtr<Gtk::ListStore> model)
{
    auto itemStore = CtChooseDialogListStore::create();
    itemStore->add_row("", CtConst::TAG_SEPARATOR, CtConst::TAG_SEPARATOR_ANSI_REPR);
    for (const CtAction& action: _pCtMenu->get_actions())
    {
        if (action.desc.empty()) continue; // skip stub menu entries
        if (action.id == "ct_open_file" && _pCtMainWin->get_ct_config()->toolbarUiList.find(CtConst::CHAR_STAR) != std::string::npos) continue;
        if (vec::exists(CtConst::TOOLBAR_VEC_BLACKLIST, action.id)) continue;
        itemStore->add_row(action.image, action.id, action.desc);
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
    for(const CtAction& action: _pCtMenu->get_actions())
    {
        if (action.category.empty()) continue;
        if (action.category != category_name) {
            category_name = action.category;
            cat_row = *model->append();
            cat_row[_shortcutModelColumns.desc] = action.category;
        }
        auto row = *model->append(cat_row.children());
        if (not action.image.empty())
        {
            row[_shortcutModelColumns.icon] = _pCtMainWin->get_icon(action.image, CtConst::NODE_ICON_SIZE);
        }
        row[_shortcutModelColumns.key] = action.id;
        row[_shortcutModelColumns.desc] = action.desc;
        row[_shortcutModelColumns.shortcut] = action.get_shortcut(_pCtMainWin->get_ct_config());
    }
}

bool CtPrefDlg::edit_shortcut(Gtk::TreeView* treeview)
{
    auto row = treeview->get_selection()->get_selected();
    if (!row || row->get_value(_shortcutModelColumns.key).empty()) return false;
    std::string shortcut = row->get_value(_shortcutModelColumns.shortcut);
    std::string id = row->get_value(_shortcutModelColumns.key);
    if (edit_shortcut_dialog(shortcut)) {
        if (shortcut != "") {
            for(const CtAction& action : _pCtMenu->get_actions())
                if (action.get_shortcut(_pCtMainWin->get_ct_config()) == shortcut && action.id != id) {
                    // todo: this is a shorter version from python code
                    if (!CtDialogs::question_dialog(std::string("<b>") + _("The Keyboard Shortcut '%s' is already in use") + "</b>", *this))
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
    str::replace(kb_shortcut_key, _pCtMenu->KB_CONTROL.c_str(), "");
    str::replace(kb_shortcut_key, _pCtMenu->KB_SHIFT.c_str(), "");
    str::replace(kb_shortcut_key, _pCtMenu->KB_ALT.c_str(), "");

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
    ctrl_toggle->set_size_request(70, 1);
    shift_toggle->set_size_request(70, 1);
    alt_toggle->set_size_request(70, 1);
    Gtk::Entry* key_entry = Gtk::manage(new Gtk::Entry());
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    hbox->pack_start(*radiobutton_kb_shortcut);
    hbox->pack_start(*ctrl_toggle);
    hbox->pack_start(*shift_toggle);
    hbox->pack_start(*alt_toggle);
    hbox->pack_start(*key_entry);
    hbox->set_spacing(5);
    vbox->pack_start(*radiobutton_kb_none);
    vbox->pack_start(*hbox);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(*vbox);

    auto b1 = Glib::Binding::bind_property(key_entry->property_sensitive(), ctrl_toggle->property_sensitive());
    auto b2 = Glib::Binding::bind_property(key_entry->property_sensitive(), shift_toggle->property_sensitive());
    auto b3 = Glib::Binding::bind_property(key_entry->property_sensitive(), alt_toggle->property_sensitive());
    auto b4 = Glib::Binding::bind_property(radiobutton_kb_shortcut->property_active(), key_entry->property_sensitive());

    key_entry->set_sensitive(!kb_shortcut_key.empty());
    key_entry->set_text(kb_shortcut_key);
    radiobutton_kb_none->set_active(kb_shortcut_key.empty());
    radiobutton_kb_shortcut->set_active(!kb_shortcut_key.empty());
    ctrl_toggle->set_active(shortcut.find(_pCtMenu->KB_CONTROL) != std::string::npos);
    shift_toggle->set_active(shortcut.find(_pCtMenu->KB_SHIFT) != std::string::npos);
    alt_toggle->set_active(shortcut.find(_pCtMenu->KB_ALT) != std::string::npos);

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
        shortcut += key_entry->get_text();
    }
    return true;
}
