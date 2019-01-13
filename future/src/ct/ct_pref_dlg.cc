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
    Gtk::Image* button_strftime_help_image = Gtk::manage(new Gtk::Image());
    button_strftime_help_image->set_from_icon_name("gtk-help", Gtk::ICON_SIZE_BUTTON);
    button_strftime_help->set_image(*button_strftime_help_image);
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
    Gtk::Image* button_reset_image = Gtk::manage(new Gtk::Image());
    button_reset_image->set_from_icon_name("gtk-undo", Gtk::ICON_SIZE_BUTTON);
    button_reset->set_image(*button_reset_image);
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
    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
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


/*
def dialog_preferences(dad):
    """Preferences Dialog"""
    dialog = gtk.Dialog(title=_("Preferences"),
        parent=dad.window,
        flags=gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT,
        buttons=(gtk.STOCK_CLOSE, gtk.RESPONSE_ACCEPT))

    tabs_vbox_vec = []
    for tabs_idx in range(11):
        tabs_vbox_vec.append(gtk.VBox())
        tabs_vbox_vec[-1].set_spacing(3)


    tab_constructor = {
        0: preferences_tab_text_n_code,
        1: preferences_tab_text,
        2: preferences_tab_rich_text,
        3: preferences_tab_plain_text_n_code,
        4: preferences_tab_tree_1,
        5: preferences_tab_tree_2,
        6: preferences_tab_fonts,
        7: preferences_tab_links,
        8: preferences_tab_toolbar,
        9: preferences_tab_kb_shortcuts,
       10: preferences_tab_misc,
        }

    def on_notebook_switch_page(notebook, page, page_num):
        #print "new page", page_num
        tab_constructor[page_num](dad, tabs_vbox_vec[page_num], dialog)
        tabs_vbox_vec[page_num].show_all()
    notebook.connect('switch-page', on_notebook_switch_page)

    content_area = dialog.get_content_area()
    content_area.pack_start(notebook)
    content_area.show_all()
    notebook.set_current_page(dad.prefpage)
    dialog.disp_dialog_after_restart = False
    dialog.run()
    dad.prefpage = notebook.get_current_page()
    dialog.hide()

 */
