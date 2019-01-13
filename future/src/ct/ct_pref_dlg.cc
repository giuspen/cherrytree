#include "ct_pref_dlg.h"
#include <gtkmm/notebook.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <gdkmm/rgba.h>
#include <glib/gi18n.h>
#include <ct_app.h>

CtPrefDlg::CtPrefDlg(Gtk::Window& parent) : Gtk::Dialog (_("Preferences"), parent, true)
{
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

Gtk::Widget* CtPrefDlg::build_tab_text_n_code()
{
    CtConfig* config = CtApp::P_ctCfg;

    Gtk::HBox* hbox_tab_width = Gtk::manage(new Gtk::HBox());
    hbox_tab_width->set_spacing(4);
    Gtk::Label* label_tab_width = Gtk::manage(new Gtk::Label(_("Tab Width")));
    Glib::RefPtr<Gtk::Adjustment> adj_tab_width = Gtk::Adjustment::create(config->tabsWidth, 1, 10000);
    Gtk::SpinButton* spinbutton_tab_width = Gtk::manage(new Gtk::SpinButton(adj_tab_width));
    spinbutton_tab_width->set_value(config->tabsWidth);
    hbox_tab_width->pack_start(*label_tab_width, false, false);
    hbox_tab_width->pack_start(*spinbutton_tab_width, false, false);
    Gtk::CheckButton* checkbutton_spaces_tabs = Gtk::manage(new Gtk::CheckButton(_("Insert Spaces Instead of Tabs")));
    checkbutton_spaces_tabs->set_active(config->spacesInsteadTabs);
    Gtk::CheckButton* checkbutton_line_wrap = Gtk::manage(new Gtk::CheckButton(_("Use Line Wrapping")));
    checkbutton_line_wrap->set_active(config->lineWrapping);
    Gtk::HBox* hbox_wrapping_indent = Gtk::manage(new Gtk::HBox());
    hbox_wrapping_indent->set_spacing(4);
    Gtk::Label* label_wrapping_indent = Gtk::manage(new Gtk::Label(_("Line Wrapping Indentation")));
    Glib::RefPtr<Gtk::Adjustment> adj_wrapping_indent = Gtk::Adjustment::create(config->wrappingIndent, -10000, 10000, 1);
    Gtk::SpinButton* spinbutton_wrapping_indent = Gtk::manage(new Gtk::SpinButton(adj_wrapping_indent));
    spinbutton_wrapping_indent->set_value(config->wrappingIndent);
    hbox_wrapping_indent->pack_start(*label_wrapping_indent, false, false);
    hbox_wrapping_indent->pack_start(*spinbutton_wrapping_indent, false, false);
    Gtk::CheckButton* checkbutton_auto_indent = Gtk::manage(new Gtk::CheckButton(_("Enable Automatic Indentation")));
    checkbutton_auto_indent->set_active(config->autoIndent);
    Gtk::CheckButton* checkbutton_line_nums = Gtk::manage(new Gtk::CheckButton(_("Show Line Numbers")));
    checkbutton_line_nums->set_active(config->showLineNumbers);
    Gtk::HBox* hbox_space_around_lines = Gtk::manage(new Gtk::HBox());
    hbox_space_around_lines->set_spacing(4);
    Gtk::Label* label_space_around_lines = Gtk::manage(new Gtk::Label(_("Vertical Space Around Lines")));
    Glib::RefPtr<Gtk::Adjustment> adj_space_around_lines = Gtk::Adjustment::create(config->spaceAroundLines, -0, 255, 1);
    Gtk::SpinButton* spinbutton_space_around_lines = Gtk::manage(new Gtk::SpinButton(adj_space_around_lines));
    spinbutton_space_around_lines->set_value(config->spaceAroundLines);
    hbox_space_around_lines->pack_start(*label_space_around_lines, false, false);
    hbox_space_around_lines->pack_start(*spinbutton_space_around_lines, false, false);
    Gtk::HBox* hbox_relative_wrapped_space = Gtk::manage(new Gtk::HBox());
    hbox_relative_wrapped_space->set_spacing(4);
    Gtk::Label* label_relative_wrapped_space = Gtk::manage(new Gtk::Label(_("Vertical Space in Wrapped Lines")));
    Glib::RefPtr<Gtk::Adjustment> adj_relative_wrapped_space = Gtk::Adjustment::create(config->relativeWrappedSpace, -0, 100, 1);
    Gtk::SpinButton* spinbutton_relative_wrapped_space = Gtk::manage(new Gtk::SpinButton(adj_relative_wrapped_space));
    spinbutton_relative_wrapped_space->set_value(config->relativeWrappedSpace);
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
    entry_timestamp_format->set_text(config->timestampFormat);
    Gtk::Button* button_strftime_help = Gtk::manage(new Gtk::Button());
    button_strftime_help->set_image(*new_image_from_stock("gtk-help", Gtk::ICON_SIZE_BUTTON));
    hbox_timestamp->pack_start(*label_timestamp, false, false);
    hbox_timestamp->pack_start(*entry_timestamp_format, false, false);
    hbox_timestamp->pack_start(*button_strftime_help, false, false);
    Gtk::HBox* hbox_horizontal_rule = Gtk::manage(new Gtk::HBox());
    hbox_horizontal_rule->set_spacing(4);
    Gtk::Label* label_horizontal_rule = Gtk::manage(new Gtk::Label(_("Horizontal Rule")));
    Gtk::Entry* entry_horizontal_rule = Gtk::manage(new Gtk::Entry());
    entry_horizontal_rule->set_text(config->hRule);
    hbox_horizontal_rule->pack_start(*label_horizontal_rule, false, false);
    hbox_horizontal_rule->pack_start(*entry_horizontal_rule);
    Gtk::HBox* hbox_special_chars = Gtk::manage(new Gtk::HBox());
    hbox_special_chars->set_spacing(4);
    Gtk::VBox* vbox_special_chars = Gtk::manage(new Gtk::VBox());
    Gtk::Label* label_special_chars = Gtk::manage(new Gtk::Label(_("Special Characters")));
    Gtk::HBox* hbox_reset = Gtk::manage(new Gtk::HBox());
    Gtk::Button* button_reset = Gtk::manage(new Gtk::Button());
    button_reset->set_image(*new_image_from_stock("gtk-undo", Gtk::ICON_SIZE_BUTTON));
    button_reset->set_tooltip_text(_("Reset to Default"));
    hbox_reset->pack_start(*Gtk::manage(new Gtk::Label()), true, false); // todo: not sure about third arg
    hbox_reset->pack_start(*button_reset, false, false);
    hbox_reset->pack_start(*Gtk::manage(new Gtk::Label()), true, false); // todo: not sure about third arg
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
    textview_special_chars->get_buffer()->set_text(config->specialChars);
    textview_special_chars->set_wrap_mode(Gtk::WRAP_CHAR);
    scrolledwindow_special_chars->add(*textview_special_chars);
    hbox_special_chars->pack_start(*vbox_special_chars, false, false);
    hbox_special_chars->pack_start(*frame_special_chars);
    Gtk::HBox* hbox_selword_chars = Gtk::manage(new Gtk::HBox());
    hbox_selword_chars->set_spacing(4);
    Gtk::Label* label_selword_chars = Gtk::manage(new Gtk::Label(_("Chars to Select at Double Click")));
    Gtk::Entry* entry_selword_chars = Gtk::manage(new Gtk::Entry());
    entry_selword_chars->set_text(config->selwordChars);
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

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_text()
{
    CtConfig* config = CtApp::P_ctCfg;

    Gtk::VBox* vbox_editor = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_auto_smart_quotes = Gtk::manage(new Gtk::CheckButton(_("Enable Smart Quotes Auto Replacement")));
    checkbutton_auto_smart_quotes->set_active(config->autoSmartQuotes);

    vbox_editor->pack_start(*checkbutton_auto_smart_quotes, false, false);

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
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_rich_text()
{
    CtConfig* config = CtApp::P_ctCfg;

    Gtk::VBox* vbox_spell_check = Gtk::manage(new Gtk::VBox());
    Gtk::CheckButton* checkbutton_spell_check = Gtk::manage(new Gtk::CheckButton(_("Enable Spell Check")));
    checkbutton_spell_check->set_active(config->enableSpellCheck);
    Gtk::HBox* hbox_spell_check_lang = Gtk::manage(new Gtk::HBox());
    hbox_spell_check_lang->set_spacing(4);
    Gtk::Label* label_spell_check_lang = Gtk::manage(new Gtk::Label(_("Spell Check Language")));
    Gtk::ComboBox* combobox_spell_check_lang = Gtk::manage(new Gtk::ComboBox());
    Gtk::CellRendererText* cell = Gtk::manage(new Gtk::CellRendererText());
    combobox_spell_check_lang->pack_start(*cell, true);
    combobox_spell_check_lang->add_attribute(*cell, "text", 0);
    // todo: fix this
    //def set_checkbutton_spell_check_model():
    //    combobox_spell_check_lang->set_model(dad->spell_check_lang_liststore)
    //    combobox_spell_check_lang->set_active_iter(dad->get_combobox_iter_from_value(dad->spell_check_lang_liststore, 0, dad->spell_check_lang))
    //if dad->spell_check_init: set_checkbutton_spell_check_model()
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
    Gtk::ColorButton* colorbutton_text_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->rtDefBg)));
    Gtk::Label* label_rt_col_custom = Gtk::manage(new Gtk::Label(_("and Text")));
    Gtk::ColorButton* colorbutton_text_fg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->rtDefFg)));
    hbox_rt_col_custom->pack_start(*radiobutton_rt_col_custom, false, false);
    hbox_rt_col_custom->pack_start(*colorbutton_text_bg, false, false);
    hbox_rt_col_custom->pack_start(*label_rt_col_custom, false, false);
    hbox_rt_col_custom->pack_start(*colorbutton_text_fg, false, false);
    Gtk::CheckButton* checkbutton_monospace_bg = Gtk::manage(new Gtk::CheckButton(_("Monospace Background")));
    //mono_color = dad->monospace_bg if dad->monospace_bg else DEFAULT_MONOSPACE_BG
    Gtk::ColorButton* colorbutton_monospace_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->monospaceBg)));
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

    if (config->rtDefFg == CtConst::RICH_TEXT_DARK_FG && config->rtDefBg == CtConst::RICH_TEXT_DARK_BG)
    {
        radiobutton_rt_col_dark->set_active(true);
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
    }
    else if (config->rtDefFg == CtConst::RICH_TEXT_LIGHT_FG && config->rtDefBg == CtConst::RICH_TEXT_LIGHT_BG)
    {
        radiobutton_rt_col_light->set_active(true);
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
    }
    else
    {
        radiobutton_rt_col_custom->set_active(true);
    }
    if (!config->monospaceBg.empty())
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
    checkbutton_rt_show_white_spaces->set_active(config->rtShowWhiteSpaces);
    Gtk::CheckButton* checkbutton_rt_highl_curr_line = Gtk::manage(new Gtk::CheckButton(_("Highlight Current Line")));
    checkbutton_rt_highl_curr_line->set_active(config->rtHighlCurrLine);
    Gtk::CheckButton* checkbutton_codebox_auto_resize = Gtk::manage(new Gtk::CheckButton(_("Expand CodeBoxes Automatically")));
    checkbutton_codebox_auto_resize->set_active(config->codeboxAutoResize);
    Gtk::HBox* hbox_embfile_size = Gtk::manage(new Gtk::HBox());
    hbox_embfile_size->set_spacing(4);
    Gtk::Label* label_embfile_size = Gtk::manage(new Gtk::Label(_("Embedded File Icon Size")));
    Glib::RefPtr<Gtk::Adjustment> adj_embfile_size = Gtk::Adjustment::create(config->embfileSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_embfile_size = Gtk::manage(new Gtk::SpinButton(adj_embfile_size));
    spinbutton_embfile_size->set_value(config->embfileSize);
    hbox_embfile_size->pack_start(*label_embfile_size, false, false);
    hbox_embfile_size->pack_start(*spinbutton_embfile_size, false, false);
    Gtk::CheckButton* checkbutton_embfile_show_filename = Gtk::manage(new Gtk::CheckButton(_("Show File Name on Top of Embedded File Icon")));
    checkbutton_embfile_show_filename->set_active(config->embfileShowFileName);
    Gtk::Label* label_limit_undoable_steps = Gtk::manage(new Gtk::Label(_("Limit of Undoable Steps Per Node")));
    Glib::RefPtr<Gtk::Adjustment> adj_limit_undoable_steps = Gtk::Adjustment::create(config->limitUndoableSteps, 1, 10000, 1);
    Gtk::SpinButton* spinbutton_limit_undoable_steps = Gtk::manage(new Gtk::SpinButton(adj_limit_undoable_steps));
    spinbutton_limit_undoable_steps->set_value(config->limitUndoableSteps);
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
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_plain_text_n_code()
{
    CtConfig* config = CtApp::P_ctCfg;
    std::string style_scheme_liststore;

    Gtk::VBox* vbox_syntax = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_style_scheme = Gtk::manage(new Gtk::HBox());
    hbox_style_scheme->set_spacing(4);
    Gtk::Label* label_style_scheme = Gtk::manage(new Gtk::Label(_("Style Scheme")));
    Gtk::ComboBox* combobox_style_scheme = Gtk::manage(new Gtk::ComboBox(/*style_scheme_liststore*/));
    Gtk::CellRendererText* cell = Gtk::manage(new Gtk::CellRendererText());
    combobox_style_scheme->pack_start(*cell, true);
    combobox_style_scheme->add_attribute(*cell, "text", 0);
    //combobox_style_scheme->set_active_iter(dad->get_combobox_iter_from_value(dad->style_scheme_liststore, 0, dad->style_scheme))
    hbox_style_scheme->pack_start(*label_style_scheme, false, false);
    hbox_style_scheme->pack_start(*combobox_style_scheme);
    Gtk::CheckButton* checkbutton_pt_show_white_spaces = Gtk::manage(new Gtk::CheckButton(_("Show White Spaces")));
    checkbutton_pt_show_white_spaces->set_active(config->ptShowWhiteSpaces);
    Gtk::CheckButton* checkbutton_pt_highl_curr_line = Gtk::manage(new Gtk::CheckButton(_("Highlight Current Line")));
    checkbutton_pt_highl_curr_line->set_active(config->ptHighlCurrLine);

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


  /*  def on_combobox_style_scheme_changed(combobox):
        new_iter = combobox_style_scheme->get_active_iter()
        new_style = dad->style_scheme_liststore[new_iter][0]
        if new_style != dad->style_scheme:
            dad->style_scheme = new_style
            support->dialog_info_after_restart(pref_dialog)
    combobox_style_scheme->connect('changed', on_combobox_style_scheme_changed)
    def on_checkbutton_pt_show_white_spaces_toggled(checkbutton):
        dad->pt_show_white_spaces = checkbutton->get_active()
        if dad->syntax_highlighting != cons->RICH_TEXT_ID:
            dad->sourceview->set_draw_spaces(codeboxes->DRAW_SPACES_FLAGS if dad->pt_show_white_spaces else 0)
    checkbutton_pt_show_white_spaces->connect('toggled', on_checkbutton_pt_show_white_spaces_toggled)
    def on_checkbutton_pt_highl_curr_line_toggled(checkbutton):
        dad->pt_highl_curr_line = checkbutton->get_active()
        if dad->syntax_highlighting != cons->RICH_TEXT_ID:
            dad->sourceview->set_highlight_current_line(dad->pt_highl_curr_line)
    checkbutton_pt_highl_curr_line->connect('toggled', on_checkbutton_pt_highl_curr_line_toggled)
*/

    /*Gtk::ListStore* liststore = Gtk::manage(new Gtk::ListStore(str, str, str)
    treeview = Gtk::manage(new Gtk::TreeView(liststore)
    treeview->set_headers_visible(False)
    treeview->set_size_request(300, 200)
    renderer_pixbuf = Gtk::manage(new Gtk::CellRendererPixbuf()
    renderer_text_key = Gtk::manage(new Gtk::CellRendererText()
    renderer_text_val = Gtk::manage(new Gtk::CellRendererText()
    renderer_text_val->set_property('editable', true)
    def on_table_cell_edited(cell, path, new_text):
        if liststore[path][2] != new_text:
            liststore[path][2] = new_text
            key = liststore[path][1]
            dad->custom_codexec_type[key] = new_text
    renderer_text_val->connect('edited', on_table_cell_edited)
    column_key = Gtk::manage(new Gtk::TreeViewColumn()
    column_key->pack_start(renderer_pixbuf, False)
    column_key->pack_start(renderer_text_key, true)
    column_key->set_attributes(renderer_pixbuf, stock_id=0)
    column_key->set_attributes(renderer_text_key, text=1)
    column_val = Gtk::manage(new Gtk::TreeViewColumn("", renderer_text_val, text=2)
    treeview->append_column(column_key)
    treeview->append_column(column_val)
    treeviewselection = treeview->get_selection()
*/
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    ///scrolledwindow->add(treeview);

    Gtk::Button* button_add = Gtk::manage(new Gtk::Button());
    button_add->set_image(*new_image_from_stock("gtk-add", Gtk::ICON_SIZE_BUTTON));
    button_add->set_tooltip_text(_("Add"));
    Gtk::Button* button_reset_cmds = Gtk::manage(new Gtk::Button());
    button_reset_cmds->set_image(*new_image_from_stock("gtk-undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_cmds->set_tooltip_text(_("Reset to Default"));
    Gtk::VBox* vbox_buttons = Gtk::manage(new Gtk::VBox());
    vbox_buttons->pack_start(*button_add, false, false);
    vbox_buttons->pack_start(*Gtk::manage(new Gtk::Label()), true, false);
    vbox_buttons->pack_start(*button_reset_cmds, false, false);

    Gtk::VBox* vbox_codexec = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_term_run = Gtk::manage(new Gtk::HBox());
    Gtk::Entry* entry_term_run = Gtk::manage(new Gtk::Entry());
    //entry_term_run->set_text(get_code_exec_term_run(dad))
    Gtk::Button* button_reset_term = Gtk::manage(new Gtk::Button());
    button_reset_term->set_image(*new_image_from_stock("gtk-undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_term->set_tooltip_text(_("Reset to Default"));
    hbox_term_run->pack_start(*entry_term_run, true, false);
    hbox_term_run->pack_start(*button_reset_term, false, false);
    Gtk::HBox* hbox_cmd_per_type = Gtk::manage(new Gtk::HBox());
    hbox_cmd_per_type->pack_start(*scrolledwindow, true, false);
    hbox_cmd_per_type->pack_start(*vbox_buttons, false, false);

    Gtk::Label* label = Gtk::manage(new Gtk::Label(std::string("<b>")+_("Command per Node/CodeBox Type")+"</b>"));
    label->set_use_markup(true);
    vbox_codexec->pack_start(*label, false, false);
    vbox_codexec->pack_start(*hbox_cmd_per_type, true, false);
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
    pMainBox->pack_start(*frame_codexec, true, false);
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_tree_1()
{
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_tree_2()
{
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    return pMainBox;
}
Gtk::Widget* CtPrefDlg::build_tab_fonts()
{
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_links()
{
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_toolbar()
{
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_kb_shortcuts()
{
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_misc()
{
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    return pMainBox;
}

Gtk::Image* CtPrefDlg::new_image_from_stock(const std::string& id, Gtk::IconSize size)
{
    Gtk::Image* image = Gtk::manage(new Gtk::Image());
    image->set_from_icon_name(id, size);
    return image;
}
