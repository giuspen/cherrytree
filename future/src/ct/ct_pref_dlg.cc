#include "ct_pref_dlg.h"
#include <gtkmm/notebook.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <gdkmm/rgba.h>
#include <glib/gi18n.h>
#include "ct_app.h"
#include "ct_misc_utils.h"

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

    textview_special_chars->get_buffer()->signal_changed().connect([config, textview_special_chars](){
        Glib::ustring new_special_chars = textview_special_chars->get_buffer()->get_text();
        CtStrUtil::replaceInString(new_special_chars, CtConst::CHAR_NEWLINE, "");
        if (config->specialChars != new_special_chars)
        {
            config->specialChars = new_special_chars;
            //support.set_menu_items_special_chars();
        }
    });
    button_reset->signal_clicked().connect([this, textview_special_chars](){
        if (question_warning(std::string("<b>")+_("Are you sure to Reset to Default?")+"</b>"))
            textview_special_chars->get_buffer()->set_text(CtConst::SPECIAL_CHARS_DEFAULT);
    });
    spinbutton_tab_width->signal_value_changed().connect([config, spinbutton_tab_width](){
        config->tabsWidth = spinbutton_tab_width->get_value_as_int();
        //dad.sourceview.set_tab_width(dad.tabs_width)
    });
    spinbutton_wrapping_indent->signal_value_changed().connect([config, spinbutton_wrapping_indent](){
        config->wrappingIndent = spinbutton_wrapping_indent->get_value_as_int();
        //dad.sourceview.set_indent(dad.wrapping_indent)
    });
    spinbutton_relative_wrapped_space->signal_value_changed().connect([config, spinbutton_relative_wrapped_space](){
       config->relativeWrappedSpace = spinbutton_relative_wrapped_space->get_value_as_int();
       //dad.sourceview.set_pixels_inside_wrap(get_pixels_inside_wrap(dad.space_around_lines, dad.relative_wrapped_space))
    });
    spinbutton_space_around_lines->signal_value_changed().connect([config, spinbutton_space_around_lines](){
        config->spaceAroundLines = spinbutton_space_around_lines->get_value_as_int();
        //dad.sourceview.set_pixels_above_lines(dad.space_around_lines)
        //dad.sourceview.set_pixels_below_lines(dad.space_around_lines)
        //dad.sourceview.set_pixels_inside_wrap(get_pixels_inside_wrap(dad.space_around_lines, dad.relative_wrapped_space))
    });
    checkbutton_spaces_tabs->signal_toggled().connect([config, checkbutton_spaces_tabs](){
        config->spacesInsteadTabs = checkbutton_spaces_tabs->get_active();
        //dad.sourceview.set_insert_spaces_instead_of_tabs(dad.spaces_instead_tabs)
    });
    checkbutton_line_wrap->signal_toggled().connect([config, checkbutton_line_wrap](){
        config->lineWrapping = checkbutton_line_wrap->get_active();
        //dad.sourceview.set_wrap_mode(gtk.WRAP_WORD_CHAR if dad.line_wrapping else gtk.WRAP_NONE)
    });
    checkbutton_auto_indent->signal_toggled().connect([config, checkbutton_auto_indent](){
        config->autoIndent = checkbutton_auto_indent->get_active();
    });
    checkbutton_line_nums->signal_toggled().connect([config, checkbutton_line_nums](){
        config->showLineNumbers = checkbutton_line_nums->get_active();
        //dad.sourceview.set_show_line_numbers(dad.show_line_numbers)
    });
    entry_timestamp_format->signal_changed().connect([config, entry_timestamp_format](){
        config->timestampFormat = entry_timestamp_format->get_text();
    });
    button_strftime_help->signal_clicked().connect([](){
        //webbrowser.open("https://docs.python.org/2/library/time.html#time.strftime")
    });
    entry_horizontal_rule->signal_changed().connect([config, entry_horizontal_rule](){
        config->hRule = entry_horizontal_rule->get_text();
    });
    entry_selword_chars->signal_changed().connect([config, entry_selword_chars](){
        config->selwordChars = entry_selword_chars->get_text();
    });

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

    checkbutton_auto_smart_quotes->signal_toggled().connect([config, checkbutton_auto_smart_quotes](){
        config->autoSmartQuotes = checkbutton_auto_smart_quotes->get_active();
    });
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
    Gtk::ComboBoxText* combobox_spell_check_lang = Gtk::manage(new Gtk::ComboBoxText());
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
    Gtk::ColorButton* colorbutton_text_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->rtDefBg)));
    Gtk::Label* label_rt_col_custom = Gtk::manage(new Gtk::Label(_("and Text")));
    Gtk::ColorButton* colorbutton_text_fg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->rtDefFg)));
    hbox_rt_col_custom->pack_start(*radiobutton_rt_col_custom, false, false);
    hbox_rt_col_custom->pack_start(*colorbutton_text_bg, false, false);
    hbox_rt_col_custom->pack_start(*label_rt_col_custom, false, false);
    hbox_rt_col_custom->pack_start(*colorbutton_text_fg, false, false);
    Gtk::CheckButton* checkbutton_monospace_bg = Gtk::manage(new Gtk::CheckButton(_("Monospace Background")));
    std::string mono_color = config->monospaceBg.empty() ? CtConst::DEFAULT_MONOSPACE_BG : config->monospaceBg;
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

    checkbutton_spell_check->signal_toggled().connect([config, checkbutton_spell_check, combobox_spell_check_lang](){
        config->enableSpellCheck = checkbutton_spell_check->get_active();
        //if dad.enable_spell_check:
        //    dad.spell_check_set_on()
        //    set_checkbutton_spell_check_model()
        //else: dad.spell_check_set_off(True)
        combobox_spell_check_lang->set_sensitive(config->enableSpellCheck);
    });
    combobox_spell_check_lang->signal_changed().connect([config, combobox_spell_check_lang](){
        //new_iter = combobox.get_active_iter()
        //new_lang_code = dad.spell_check_lang_liststore[new_iter][0]
        //if new_lang_code != dad.spell_check_lang: dad.spell_check_set_new_lang(new_lang_code)
    });
    colorbutton_text_fg->signal_color_set().connect([this, config, colorbutton_text_fg](){
        config->rtDefFg = rgb_any_to_24(colorbutton_text_fg->get_rgba());
        //if dad.curr_tree_iter and dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    dad.widget_set_colors(dad.sourceview, dad.rt_def_fg, dad.rt_def_bg, False)
        //    support.rich_text_node_modify_codeboxes_color(dad.curr_buffer.get_start_iter(), dad)
    });
    colorbutton_text_bg->signal_color_set().connect([this, config, colorbutton_text_bg](){
        config->rtDefBg = rgb_any_to_24(colorbutton_text_bg->get_rgba());
        //if dad.curr_tree_iter and dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    if dad.rt_highl_curr_line:
        //        dad.set_sourcebuffer_with_style_scheme()
        //        dad.sourceview.set_buffer(dad.curr_buffer)
        //        dad.objects_buffer_refresh()
        //    dad.widget_set_colors(dad.sourceview, dad.rt_def_fg, dad.rt_def_bg, False)
        //    support.rich_text_node_modify_codeboxes_color(dad.curr_buffer.get_start_iter(), dad)
    });
    radiobutton_rt_col_light->signal_toggled().connect([radiobutton_rt_col_light, colorbutton_text_fg, colorbutton_text_bg](){
        if (!radiobutton_rt_col_light->get_active()) return;
        colorbutton_text_fg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_LIGHT_FG));
        colorbutton_text_bg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_LIGHT_BG));
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
        //on_colorbutton_text_fg_color_set(colorbutton_text_fg)
        //on_colorbutton_text_bg_color_set(colorbutton_text_bg)

    });
    radiobutton_rt_col_dark->signal_toggled().connect([radiobutton_rt_col_dark, colorbutton_text_fg, colorbutton_text_bg](){
        if (!radiobutton_rt_col_dark->get_active()) return;
        colorbutton_text_fg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_DARK_FG));
        colorbutton_text_bg->set_rgba(Gdk::RGBA(CtConst::RICH_TEXT_DARK_BG));
        colorbutton_text_fg->set_sensitive(false);
        colorbutton_text_bg->set_sensitive(false);
        //on_colorbutton_text_fg_color_set(colorbutton_text_fg)
        //on_colorbutton_text_bg_color_set(colorbutton_text_bg)
    });
    radiobutton_rt_col_custom->signal_toggled().connect([radiobutton_rt_col_custom, colorbutton_text_fg, colorbutton_text_bg](){
        if (!radiobutton_rt_col_custom->get_active()) return;
        colorbutton_text_fg->set_sensitive(true);
        colorbutton_text_bg->set_sensitive(true);
    });
    checkbutton_monospace_bg->signal_toggled().connect([this, config, checkbutton_monospace_bg, colorbutton_monospace_bg](){
        if (checkbutton_monospace_bg->get_active())
        {
            config->monospaceBg = rgb_any_to_24(colorbutton_monospace_bg->get_rgba());
            colorbutton_monospace_bg->set_sensitive(true);
        } else {
            config->monospaceBg = "";
            colorbutton_monospace_bg->set_sensitive(false);
        }
        need_restart(RESTART_REASON::MONOSPACE);
    });
    colorbutton_monospace_bg->signal_color_set().connect([this, config, colorbutton_monospace_bg](){
        config->monospaceBg = rgb_any_to_24(colorbutton_monospace_bg->get_rgba());
        need_restart(RESTART_REASON::MONOSPACE);
    });
    checkbutton_rt_show_white_spaces->signal_toggled().connect([config, checkbutton_rt_show_white_spaces](){
        config->rtShowWhiteSpaces = checkbutton_rt_show_white_spaces->get_active();
        //if dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    dad.sourceview.set_draw_spaces(codeboxes.DRAW_SPACES_FLAGS if dad.rt_show_white_spaces else 0)
    });
    checkbutton_rt_highl_curr_line->signal_toggled().connect([config, checkbutton_rt_highl_curr_line](){
        config->rtHighlCurrLine = checkbutton_rt_highl_curr_line->get_active();
        //if dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    dad.sourceview.set_highlight_current_line(dad.rt_highl_curr_line)
    });
    checkbutton_codebox_auto_resize->signal_toggled().connect([config, checkbutton_codebox_auto_resize](){
        config->codeboxAutoResize = checkbutton_codebox_auto_resize->get_active();
    });
    spinbutton_embfile_size->signal_value_changed().connect([this, config, spinbutton_embfile_size](){
        config->embfileSize = spinbutton_embfile_size->get_value_as_int();
        need_restart(RESTART_REASON::EMBFILE_SIZE);
    });
    checkbutton_embfile_show_filename->signal_toggled().connect([this, config, checkbutton_embfile_show_filename](){
        config->embfileShowFileName = checkbutton_embfile_show_filename->get_active();
        need_restart(RESTART_REASON::SHOW_EMBFILE_NAME);
    });
    spinbutton_limit_undoable_steps->signal_value_changed().connect([config, spinbutton_limit_undoable_steps](){
        config->limitUndoableSteps = spinbutton_limit_undoable_steps->get_value_as_int();
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_plain_text_n_code()
{
    CtConfig* config = CtApp::P_ctCfg;

    Gtk::VBox* vbox_syntax = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_style_scheme = Gtk::manage(new Gtk::HBox());
    hbox_style_scheme->set_spacing(4);
    Gtk::Label* label_style_scheme = Gtk::manage(new Gtk::Label(_("Style Scheme")));
    Gtk::ComboBoxText* combobox_style_scheme = Gtk::manage(new Gtk::ComboBoxText());
    for (auto& scheme: CtApp::R_styleSchemeManager->get_scheme_ids())
        combobox_style_scheme->append(scheme);
    combobox_style_scheme->set_active_text(config->styleSchemeId);
    hbox_style_scheme->pack_start(*label_style_scheme, false, false);
    hbox_style_scheme->pack_start(*combobox_style_scheme, false, false);
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


    combobox_style_scheme->signal_changed().connect([this, config, combobox_style_scheme](){
        config->styleSchemeId = combobox_style_scheme->get_active_text();
        need_restart(RESTART_REASON::SCHEME);
    });
    checkbutton_pt_show_white_spaces->signal_toggled().connect([config, checkbutton_pt_show_white_spaces](){
        config->ptShowWhiteSpaces = checkbutton_pt_show_white_spaces->get_active();
        //if dad->syntax_highlighting != cons->RICH_TEXT_ID:
        //    dad->sourceview->set_draw_spaces(codeboxes->DRAW_SPACES_FLAGS if dad->pt_show_white_spaces else 0)
    });
    checkbutton_pt_highl_curr_line->signal_toggled().connect([config, checkbutton_pt_highl_curr_line](){
        config->ptHighlCurrLine = checkbutton_pt_highl_curr_line->get_active();
        //if dad->syntax_highlighting != cons->RICH_TEXT_ID:
        //    dad->sourceview->set_highlight_current_line(dad->pt_highl_curr_line)
    });

    /*
   def liststore_append_element(key, val=None):
        stock_id = get_stock_id_for_code_type(key)
        if not val:
            val = get_code_exec_type_cmd(dad, key)
        liststore.append((stock_id, key, val))

    def populate_liststore():
        liststore.clear()
        all_codexec_keys = get_code_exec_type_keys(dad)
        for key in all_codexec_keys:
            liststore_append_element(key)

    vbox_code_nodes.pack_start(frame_codexec, expand=True)
    def on_entry_term_run_changed(entry):
        dad.custom_codexec_term = entry.get_text()
    entry_term_run.connect('changed', on_entry_term_run_changed)
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
    button_add.connect('clicked', on_button_add_clicked)
    def on_button_reset_cmds_clicked(button, type_str):
        warning_label = "<b>"+_("Are you sure to Reset to Default?")+"</b>"
        response = support.dialog_question_warning(pref_dialog, warning_label)
        if response == gtk.RESPONSE_ACCEPT:
            if type_str == "cmds":
                dad.custom_codexec_type.clear()
                populate_liststore()
            elif type_str == "term":
                dad.custom_codexec_term = None
                entry_term_run.set_text(get_code_exec_term_run(dad))
    button_reset_cmds.connect('clicked', on_button_reset_cmds_clicked, "cmds")
    button_reset_term.connect('clicked', on_button_reset_cmds_clicked, "term")

    populate_liststore()
    */
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_tree_1()
{
    CtConfig* config = CtApp::P_ctCfg;
    Gtk::Box* vbox_tt_theme = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_tt_col_light = Gtk::manage(new Gtk::RadioButton(_("Light Background, Dark Text")));
    Gtk::RadioButton* radiobutton_tt_col_dark = Gtk::manage(new Gtk::RadioButton(_("Dark Background, Light Text")));
    radiobutton_tt_col_dark->join_group(*radiobutton_tt_col_light);
    Gtk::RadioButton* radiobutton_tt_col_custom = Gtk::manage(new Gtk::RadioButton(_("Custom Background")));
    radiobutton_tt_col_custom->join_group(*radiobutton_tt_col_light);
    Gtk::HBox* hbox_tt_col_custom = Gtk::manage(new Gtk::HBox());
    hbox_tt_col_custom->set_spacing(4);
    Gtk::ColorButton* colorbutton_tree_bg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->ttDefBg)));
    Gtk::Label* label_tt_col_custom = Gtk::manage(new Gtk::Label(_("and Text")));
    Gtk::ColorButton* colorbutton_tree_fg = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->ttDefFg)));
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

    if (config->ttDefFg == CtConst::TREE_TEXT_DARK_FG && config->ttDefBg == CtConst::TREE_TEXT_DARK_BG)
    {
        radiobutton_tt_col_dark->set_active(true);
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
    }
    else if (config->ttDefFg == CtConst::TREE_TEXT_LIGHT_FG && config->ttDefBg == CtConst::TREE_TEXT_LIGHT_BG)
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
    checkbutton_aux_icon_hide->set_active(config->auxIconHide);

    Gtk::Button* c_icon_button = Gtk::manage(new Gtk::Button());
    c_icon_button->set_image(*new_image_from_stock(CtConst::NODES_STOCKS.at(config->defaultIconText), Gtk::ICON_SIZE_BUTTON));
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

    radiobutton_node_icon_cherry->set_active(config->nodesIcons == "c");
    radiobutton_node_icon_custom->set_active(config->nodesIcons == "b");
    radiobutton_node_icon_none->set_active(config->nodesIcons == "n");

    Gtk::VBox* vbox_nodes_startup = Gtk::manage(new Gtk::VBox());

    Gtk::RadioButton* radiobutton_nodes_startup_restore = Gtk::manage(new Gtk::RadioButton(_("Restore Expanded/Collapsed Status")));
    Gtk::RadioButton* radiobutton_nodes_startup_expand = Gtk::manage(new Gtk::RadioButton(_("Expand all Nodes")));
    radiobutton_nodes_startup_expand->join_group(*radiobutton_nodes_startup_restore);
    Gtk::RadioButton* radiobutton_nodes_startup_collapse = Gtk::manage(new Gtk::RadioButton(_("Collapse all Nodes")));
    radiobutton_nodes_startup_collapse->join_group(*radiobutton_nodes_startup_restore);
    Gtk::CheckButton* checkbutton_nodes_bookm_exp = Gtk::manage(new Gtk::CheckButton(_("Nodes in Bookmarks Always Visible")));
    checkbutton_nodes_bookm_exp->set_active(config->nodesBookmExp);
    checkbutton_nodes_bookm_exp->set_sensitive(config->restoreExpColl != CtRestoreExpColl::ALL_EXP);

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

    radiobutton_nodes_startup_restore->set_active(config->restoreExpColl == CtRestoreExpColl::FROM_STR);
    radiobutton_nodes_startup_expand->set_active(config->restoreExpColl == CtRestoreExpColl::ALL_EXP);
    radiobutton_nodes_startup_collapse->set_active(config->restoreExpColl == CtRestoreExpColl::ALL_COLL);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->pack_start(*frame_tt_theme, false, false);
    pMainBox->pack_start(*frame_nodes_icons, false, false);
    pMainBox->pack_start(*frame_nodes_startup, false, false);

    colorbutton_tree_fg->signal_color_set().connect([this, config, colorbutton_tree_fg](){
        config->ttDefFg = rgb_any_to_24(colorbutton_tree_fg->get_rgba());
        //dad.treeview_set_colors()
        //if dad.curr_tree_iter: dad.update_node_name_header()
    });
    colorbutton_tree_bg->signal_color_set().connect([this, config, colorbutton_tree_bg](){
        config->ttDefBg = rgb_any_to_24(colorbutton_tree_bg->get_rgba());
        //dad.treeview_set_colors()
        //if dad.curr_tree_iter: dad.update_node_name_header()
    });
    radiobutton_tt_col_light->signal_toggled().connect([radiobutton_tt_col_light, colorbutton_tree_fg, colorbutton_tree_bg](){
        if (!radiobutton_tt_col_light->get_active()) return;
        colorbutton_tree_fg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_LIGHT_FG));
        colorbutton_tree_bg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_LIGHT_BG));
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        //on_colorbutton_tree_fg_color_set(colorbutton_tree_fg)
        //on_colorbutton_tree_bg_color_set(colorbutton_tree_bg)
    });
    radiobutton_tt_col_dark->signal_toggled().connect([radiobutton_tt_col_dark, colorbutton_tree_fg, colorbutton_tree_bg](){
        if (radiobutton_tt_col_dark->get_active()) return;
        colorbutton_tree_fg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_DARK_FG));
        colorbutton_tree_bg->set_rgba(Gdk::RGBA(CtConst::TREE_TEXT_DARK_BG));
        colorbutton_tree_fg->set_sensitive(false);
        colorbutton_tree_bg->set_sensitive(false);
        //on_colorbutton_tree_fg_color_set(colorbutton_tree_fg)
        //on_colorbutton_tree_bg_color_set(colorbutton_tree_bg)
    });
    radiobutton_tt_col_custom->signal_toggled().connect([radiobutton_tt_col_custom, colorbutton_tree_fg, colorbutton_tree_bg](){
        if (!radiobutton_tt_col_custom->get_active()) return;
        colorbutton_tree_fg->set_sensitive(true);
        colorbutton_tree_bg->set_sensitive(true);
    });
    radiobutton_node_icon_cherry->signal_toggled().connect([config, radiobutton_node_icon_cherry](){
        if (!radiobutton_node_icon_cherry->get_active()) return;
        config->nodesIcons = "c";
        //dad.treeview_refresh(change_icon=True)
    });
    radiobutton_node_icon_custom->signal_toggled().connect([config, radiobutton_node_icon_custom](){
        if (!radiobutton_node_icon_custom->get_active()) return;
        config->nodesIcons = "b";
        //dad.treeview_refresh(change_icon=True)
    });
    radiobutton_node_icon_none->signal_toggled().connect([config, radiobutton_node_icon_none](){
        if (!radiobutton_node_icon_none->get_active()) return;
        config->nodesIcons = "n";
        //dad.treeview_refresh(change_icon=True)
    });
    c_icon_button->signal_clicked().connect([config, c_icon_button](){
        //icon_n_label_list = []
        //for key in cons.NODES_STOCKS_KEYS:
        //    icon_n_label_list.append([str(key), cons.NODES_STOCKS[key], ""])
        //sel_key = support.dialog_choose_element_in_list(pref_dialog, _("Select Node Icon"), [], "", icon_n_label_list)
        //if sel_key:
        //    dad.default_icon_text = int(sel_key)
        //    c_icon_button.set_image(gtk.image_new_from_stock(cons.NODES_STOCKS[dad.default_icon_text], gtk.ICON_SIZE_BUTTON))
        //    dad.treeview_refresh(change_icon=True)
    });
    radiobutton_nodes_startup_expand->signal_toggled().connect([config, radiobutton_nodes_startup_expand, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_expand->get_active()) return;
        config->restoreExpColl = CtRestoreExpColl::ALL_EXP;
        checkbutton_nodes_bookm_exp->set_sensitive(false);
    });
    radiobutton_nodes_startup_collapse->signal_toggled().connect([config, radiobutton_nodes_startup_collapse, checkbutton_nodes_bookm_exp](){
        if (!radiobutton_nodes_startup_collapse->get_active()) return;
        config->restoreExpColl = CtRestoreExpColl::ALL_COLL;
        checkbutton_nodes_bookm_exp->set_sensitive(true);
    });
    checkbutton_nodes_bookm_exp->signal_toggled().connect([config, checkbutton_nodes_bookm_exp](){
        config->nodesBookmExp = checkbutton_nodes_bookm_exp->get_active();
    });
    checkbutton_aux_icon_hide->signal_toggled().connect([config, checkbutton_aux_icon_hide](){
        config->auxIconHide = checkbutton_aux_icon_hide->get_active();
        //dad.aux_renderer_pixbuf.set_property("visible", not dad.aux_icon_hide)
        //dad.treeview_refresh()
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_tree_2()
{
    CtConfig* config = CtApp::P_ctCfg;

    Gtk::VBox* vbox_misc_tree = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_tree_nodes_names_width = Gtk::manage(new Gtk::HBox());
    hbox_tree_nodes_names_width->set_spacing(4);
    Gtk::Label* label_tree_nodes_names_width = Gtk::manage(new Gtk::Label(_("Tree Nodes Names Wrapping Width")));
    Glib::RefPtr<Gtk::Adjustment> adj_tree_nodes_names_width = Gtk::Adjustment::create(config->cherryWrapWidth, 10, 10000, 1);
    Gtk::SpinButton* spinbutton_tree_nodes_names_width = Gtk::manage(new Gtk::SpinButton(adj_tree_nodes_names_width));
    spinbutton_tree_nodes_names_width->set_value(config->cherryWrapWidth);
    hbox_tree_nodes_names_width->pack_start(*label_tree_nodes_names_width, false, false);
    hbox_tree_nodes_names_width->pack_start(*spinbutton_tree_nodes_names_width, false, false);
    Gtk::CheckButton* checkbutton_tree_right_side = Gtk::manage(new Gtk::CheckButton(_("Display Tree on the Right Side")));
    checkbutton_tree_right_side->set_active(config->treeRightSide);
    Gtk::CheckButton* checkbutton_tree_click_focus_text = Gtk::manage(new Gtk::CheckButton(_("Move Focus to Text at Mouse Click")));
    checkbutton_tree_click_focus_text->set_active(config->treeClickFocusText);
    Gtk::CheckButton* checkbutton_tree_click_expand = Gtk::manage(new Gtk::CheckButton(_("Expand Node at Mouse Click")));
    checkbutton_tree_click_expand->set_active(config->treeClickExpand);
    Gtk::HBox* hbox_nodes_on_node_name_header = Gtk::manage(new Gtk::HBox());
    hbox_nodes_on_node_name_header->set_spacing(4);
    Gtk::Label* label_nodes_on_node_name_header = Gtk::manage(new Gtk::Label(_("Last Visited Nodes on Node Name Header")));
    Glib::RefPtr<Gtk::Adjustment> adj_nodes_on_node_name_header = Gtk::Adjustment::create(config->nodesOnNodeNameHeader, 0, 100, 1);
    Gtk::SpinButton* spinbutton_nodes_on_node_name_header = Gtk::manage(new Gtk::SpinButton(adj_nodes_on_node_name_header));
    spinbutton_nodes_on_node_name_header->set_value(config->nodesOnNodeNameHeader);
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

    spinbutton_tree_nodes_names_width->signal_value_changed().connect([config, spinbutton_tree_nodes_names_width](){
        config->cherryWrapWidth = spinbutton_tree_nodes_names_width->get_value_as_int();
        //dad.renderer_text.set_property('wrap-width', dad.cherry_wrap_width)
        //dad.treeview_refresh()
    });
    checkbutton_tree_right_side->signal_toggled().connect([config, checkbutton_tree_right_side](){
        config->treeRightSide = checkbutton_tree_right_side->get_active();
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
    checkbutton_tree_click_focus_text->signal_toggled().connect([config, checkbutton_tree_click_focus_text](){
        config->treeClickFocusText = checkbutton_tree_click_focus_text->get_active();
    });
    checkbutton_tree_click_expand->signal_toggled().connect([config, checkbutton_tree_click_expand](){
        config->treeClickExpand = checkbutton_tree_click_expand->get_active();
    });
    spinbutton_nodes_on_node_name_header->signal_value_changed().connect([config, spinbutton_nodes_on_node_name_header](){
        config->nodesOnNodeNameHeader = spinbutton_nodes_on_node_name_header->get_value_as_int();
        //dad.update_node_name_header_num_latest_visited()
    });

    return pMainBox;
}
Gtk::Widget* CtPrefDlg::build_tab_fonts()
{
    CtConfig* config = CtApp::P_ctCfg;

    Gtk::Image* image_rt = new_image_from_stock(Gtk::Stock::SELECT_FONT.id, Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_pt = new_image_from_stock(Gtk::Stock::SELECT_FONT.id, Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_code = new_image_from_stock("xml", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_tree = new_image_from_stock("cherries", Gtk::ICON_SIZE_MENU);
    Gtk::Label* label_rt = Gtk::manage(new Gtk::Label(_("Rich Text")));
    Gtk::Label* label_pt = Gtk::manage(new Gtk::Label(_("Plain Text")));
    Gtk::Label* label_code = Gtk::manage(new Gtk::Label(_("Code Font")));
    Gtk::Label* label_tree = Gtk::manage(new Gtk::Label(_("Tree Font")));
    Gtk::FontButton* fontbutton_rt = Gtk::manage(new Gtk::FontButton(config->rtFont));
    Gtk::FontButton* fontbutton_pt = Gtk::manage(new Gtk::FontButton(config->ptFont));
    Gtk::FontButton* fontbutton_code = Gtk::manage(new Gtk::FontButton(config->codeFont));
    Gtk::FontButton* fontbutton_tree = Gtk::manage(new Gtk::FontButton(config->treeFont));
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

    fontbutton_rt->signal_font_set().connect([config, fontbutton_rt](){
        config->rtFont = fontbutton_rt->get_font_name();
        //if dad.curr_tree_iter and dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    dad.sourceview.modify_font(pango.FontDescription(dad.rt_font))
    });
    fontbutton_pt->signal_font_set().connect([config, fontbutton_pt](){
        config->ptFont = fontbutton_pt->get_font_name();
        //if not dad.curr_tree_iter: return
        //if dad.syntax_highlighting == cons.PLAIN_TEXT_ID:
        //    dad.sourceview.modify_font(pango.FontDescription(dad.pt_font))
        //elif dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    support.rich_text_node_modify_codeboxes_font(dad.curr_buffer.get_start_iter(), dad)
        //    support.rich_text_node_modify_tables_font(dad.curr_buffer.get_start_iter(), dad)
    });
    fontbutton_code->signal_font_set().connect([config, fontbutton_code](){
        config->codeFont = fontbutton_code->get_font_name();
        //if not dad.curr_tree_iter: return
        //if dad.syntax_highlighting not in [cons.RICH_TEXT_ID, cons.PLAIN_TEXT_ID]:
        //    dad.sourceview.modify_font(pango.FontDescription(dad.code_font))
        //elif dad.syntax_highlighting == cons.RICH_TEXT_ID:
        //    support.rich_text_node_modify_codeboxes_font(dad.curr_buffer.get_start_iter(), dad)
    });
    fontbutton_tree->signal_font_set().connect([config, fontbutton_tree](){
        config->treeFont = fontbutton_tree->get_font_name();
        //dad.set_treeview_font()
    });
    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_links()
{
    CtConfig* config = CtApp::P_ctCfg;

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

    checkbutton_custom_weblink_cmd->set_active(config->weblinkCustomOn);
    entry_custom_weblink_cmd->set_sensitive(config->weblinkCustomOn);
    entry_custom_weblink_cmd->set_text(config->weblinkCustomAct);
    checkbutton_custom_filelink_cmd->set_active(config->filelinkCustomOn);
    entry_custom_filelink_cmd->set_sensitive(config->filelinkCustomOn);
    entry_custom_filelink_cmd->set_text(config->filelinkCustomAct);
    checkbutton_custom_folderlink_cmd->set_active(config->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_sensitive(config->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_text(config->folderlinkCustomAct);

    Gtk::Table* table_links_colors = Gtk::manage(new Gtk::Table(2, 2));
    table_links_colors->set_row_spacings(2);
    table_links_colors->set_col_spacings(4);
    table_links_colors->set_homogeneous(true);

    Gtk::HBox* hbox_col_link_webs = Gtk::manage(new Gtk::HBox());
    hbox_col_link_webs->set_spacing(4);
    Gtk::Label* label_col_link_webs = Gtk::manage(new Gtk::Label(_("To WebSite")));
    Gtk::ColorButton* colorbutton_col_link_webs = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->colLinkWebs)));
    hbox_col_link_webs->pack_start(*label_col_link_webs, false, false);
    hbox_col_link_webs->pack_start(*colorbutton_col_link_webs, false, false);

    Gtk::HBox* hbox_col_link_node = Gtk::manage(new Gtk::HBox());
    hbox_col_link_node->set_spacing(4);
    Gtk::Label* label_col_link_node = Gtk::manage(new Gtk::Label(_("To Node")));
    Gtk::ColorButton* colorbutton_col_link_node = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->colLinkNode)));
    hbox_col_link_node->pack_start(*label_col_link_node, false, false);
    hbox_col_link_node->pack_start(*colorbutton_col_link_node, false, false);

    Gtk::HBox* hbox_col_link_file = Gtk::manage(new Gtk::HBox());
    hbox_col_link_file->set_spacing(4);
    Gtk::Label* label_col_link_file = Gtk::manage(new Gtk::Label(_("To File")));
    Gtk::ColorButton* colorbutton_col_link_file = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->colLinkFile)));
    hbox_col_link_file->pack_start(*label_col_link_file, false, false);
    hbox_col_link_file->pack_start(*colorbutton_col_link_file, false, false);

    Gtk::HBox* hbox_col_link_fold = Gtk::manage(new Gtk::HBox());
    hbox_col_link_fold->set_spacing(4);
    Gtk::Label* label_col_link_fold = Gtk::manage(new Gtk::Label(_("To Folder")));
    Gtk::ColorButton* colorbutton_col_link_fold = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(config->colLinkFold)));
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
    checkbutton_links_underline->set_active(config->linksUnderline);
    Gtk::CheckButton* checkbutton_links_relative = Gtk::manage(new Gtk::CheckButton(_("Use Relative Paths for Files And Folders")));
    checkbutton_links_relative->set_active(config->linksRelative);
    Gtk::HBox* hbox_anchor_size = Gtk::manage(new Gtk::HBox());
    hbox_anchor_size->set_spacing(4);
    Gtk::Label* label_anchor_size = Gtk::manage(new Gtk::Label(_("Anchor Size")));
    Glib::RefPtr<Gtk::Adjustment> adj_anchor_size = Gtk::Adjustment::create(config->anchorSize, 1, 1000, 1);
    Gtk::SpinButton* spinbutton_anchor_size = Gtk::manage(new Gtk::SpinButton(adj_anchor_size));
    spinbutton_anchor_size->set_value(config->anchorSize);
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

    checkbutton_custom_weblink_cmd->signal_toggled().connect([config, checkbutton_custom_weblink_cmd, entry_custom_weblink_cmd](){
        config->weblinkCustomOn = checkbutton_custom_weblink_cmd->get_active();
        entry_custom_weblink_cmd->set_sensitive(config->weblinkCustomOn);
    });
    entry_custom_weblink_cmd->signal_changed().connect([config, entry_custom_weblink_cmd](){
        config->weblinkCustomAct = entry_custom_weblink_cmd->get_text();
    });
    checkbutton_custom_filelink_cmd->signal_toggled().connect([config, checkbutton_custom_filelink_cmd, entry_custom_filelink_cmd](){
        config->filelinkCustomOn = checkbutton_custom_filelink_cmd->get_active();
        entry_custom_filelink_cmd->set_sensitive(config->filelinkCustomOn);
    });
    entry_custom_filelink_cmd->signal_changed().connect([config, entry_custom_filelink_cmd](){
        config->filelinkCustomAct = entry_custom_filelink_cmd->get_text();
    });
    checkbutton_custom_folderlink_cmd->signal_toggled().connect([config, checkbutton_custom_folderlink_cmd, entry_custom_folderlink_cmd](){
        config->folderlinkCustomOn = checkbutton_custom_folderlink_cmd->get_active();
        entry_custom_folderlink_cmd->set_sensitive(config->folderlinkCustomOn);
    });
    entry_custom_folderlink_cmd->signal_changed().connect([config, entry_custom_folderlink_cmd](){
        config->folderlinkCustomAct = entry_custom_folderlink_cmd->get_text();
    });
    checkbutton_links_relative->signal_toggled().connect([config, checkbutton_links_relative](){
        config->linksRelative = checkbutton_links_relative->get_active();
    });
    checkbutton_links_underline->signal_toggled().connect([this, config, checkbutton_links_underline](){
        config->linksUnderline = checkbutton_links_underline->get_active();
        need_restart(RESTART_REASON::LINKS);
    });
    spinbutton_anchor_size->signal_value_changed().connect([this, config, spinbutton_anchor_size](){
        config->anchorSize = spinbutton_anchor_size->get_value_as_int();
        need_restart(RESTART_REASON::ANCHOR_SIZE);
    });
    colorbutton_col_link_webs->signal_color_set().connect([this, config, colorbutton_col_link_webs](){
        config->colLinkWebs = rgb_to_string(colorbutton_col_link_webs->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_node->signal_color_set().connect([this, config, colorbutton_col_link_node](){
        config->colLinkNode = rgb_to_string(colorbutton_col_link_node->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_file->signal_color_set().connect([this, config, colorbutton_col_link_file](){
        config->colLinkFile =  rgb_to_string(colorbutton_col_link_file->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_fold->signal_color_set().connect([this, config, colorbutton_col_link_fold](){
        config->colLinkFold = rgb_to_string(colorbutton_col_link_fold->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });

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
    CtConfig* config = CtApp::P_ctCfg;

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

    checkbutton_systray->set_active(config->systrayOn);
    checkbutton_start_on_systray->set_active(config->startOnSystray);
    checkbutton_start_on_systray->set_sensitive(config->systrayOn);
    checkbutton_use_appind->set_active(config->useAppInd);
    //if not cons->HAS_APPINDICATOR or not cons->HAS_SYSTRAY: checkbutton_use_appind->set_sensitive(False)

    Gtk::VBox* vbox_saving = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox_autosave = Gtk::manage(new Gtk::HBox());
    hbox_autosave->set_spacing(4);
    Gtk::CheckButton* checkbutton_autosave = Gtk::manage(new Gtk::CheckButton(_("Autosave Every")));
    Glib::RefPtr<Gtk::Adjustment> adjustment_autosave = Gtk::Adjustment::create(config->autosaveVal, 1, 1000, 1);
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
    Glib::RefPtr<Gtk::Adjustment> adjustment_num_backups = Gtk::Adjustment::create(config->backupNum, 1, 100, 1);
    Gtk::SpinButton* spinbutton_num_backups = Gtk::manage(new Gtk::SpinButton(adjustment_num_backups));
    spinbutton_num_backups->set_sensitive(config->backupCopy);
    spinbutton_num_backups->set_value(config->backupNum);
    hbox_num_backups->pack_start(*label_num_backups, false, false);
    hbox_num_backups->pack_start(*spinbutton_num_backups, false, false);
    vbox_saving->pack_start(*hbox_autosave, false, false);
    vbox_saving->pack_start(*checkbutton_autosave_on_quit, false, false);
    vbox_saving->pack_start(*checkbutton_backup_before_saving, false, false);
    vbox_saving->pack_start(*hbox_num_backups, false, false);

    checkbutton_autosave->set_active(config->autosaveOn);
    spinbutton_autosave->set_value(config->autosaveVal);
    spinbutton_autosave->set_sensitive(config->autosaveOn);
    checkbutton_autosave_on_quit->set_active(config->autosaveOnQuit);
    checkbutton_backup_before_saving->set_active(config->backupCopy);

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

    checkbutton_newer_version->set_active(config->checkVersion);
    checkbutton_word_count->set_active(config->wordCountOn);
    checkbutton_reload_doc_last->set_active(config->reloadDocLast);
    checkbutton_mod_time_sentinel->set_active(config->modTimeSentinel);

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
    // todo: look for country_lang which is taken from s.path.isfile(cons.LANG_PATH)
    // combobox_country_language->set_active_text(config->countryLang);
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

    checkbutton_systray->signal_toggled().connect([config, checkbutton_systray, checkbutton_start_on_systray](){
        config->systrayOn = checkbutton_systray->get_active();
        if (config->systrayOn) {
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
    checkbutton_start_on_systray->signal_toggled().connect([config, checkbutton_start_on_systray](){
        config->startOnSystray = checkbutton_start_on_systray->get_active();
    });
    checkbutton_use_appind->signal_toggled().connect([config, checkbutton_use_appind, checkbutton_systray](){
        bool saved_systray_active = checkbutton_systray->get_active();
        if (saved_systray_active) checkbutton_systray->set_active(false); // todo: some hack?
        config->useAppInd = checkbutton_use_appind->get_active();
        if (saved_systray_active) checkbutton_systray->set_active(true);
        //if len(dad.boss.running_windows) > 1:
        //    for runn_win in dad.boss.running_windows:
        //        if runn_win.window == dad.window: continue
        //        runn_win.use_appind = dad.use_appind
    });
    checkbutton_autosave->signal_toggled().connect([config, checkbutton_autosave, spinbutton_autosave](){
        config->autosaveOn = checkbutton_autosave->get_active();
        //if dad.autosave[0]:
        //    if dad.autosave_timer_id == None: dad.autosave_timer_start()
        //else:
        //    if dad.autosave_timer_id != None: dad.autosave_timer_stop()
        spinbutton_autosave->set_sensitive(config->autosaveOn);
    });
    spinbutton_autosave->signal_value_changed().connect([config, spinbutton_autosave](){
        config->autosaveVal = spinbutton_autosave->get_value_as_int();
        //if dad.autosave_timer_id != None: dad.autosave_timer_stop()
        //if dad.autosave[0] and dad.autosave_timer_id == None: dad.autosave_timer_start()
    });
    spinbutton_num_backups->signal_value_changed().connect([config, spinbutton_num_backups](){
        config->backupNum = spinbutton_num_backups->get_value_as_int();
    });
    checkbutton_autosave_on_quit->signal_toggled().connect([config, checkbutton_autosave_on_quit](){
        config->autosaveOnQuit = checkbutton_autosave_on_quit->get_active();
    });
    checkbutton_reload_doc_last->signal_toggled().connect([config, checkbutton_reload_doc_last](){
        config->reloadDocLast = checkbutton_reload_doc_last->get_active();
    });
    checkbutton_mod_time_sentinel->signal_toggled().connect([config, checkbutton_mod_time_sentinel](){
        config->modTimeSentinel = checkbutton_mod_time_sentinel->get_active();
        if (config->modTimeSentinel) {
            //if dad.mod_time_sentinel_id == None:
            //    dad.modification_time_sentinel_start()
        } else {
            //if dad.mod_time_sentinel_id != None:
            //    dad.modification_time_sentinel_stop()
        }
    });
    checkbutton_newer_version->signal_toggled().connect([config, checkbutton_newer_version](){
        config->checkVersion = checkbutton_newer_version->get_active();
    });
    checkbutton_word_count->signal_toggled().connect([config, checkbutton_word_count](){
        config->wordCountOn = checkbutton_word_count->get_active();
        //dad.update_selected_node_statusbar_info()
    });
    combobox_country_language->signal_changed().connect([this, config, combobox_country_language](){
        Glib::ustring new_lang = combobox_country_language->get_active_text();
        need_restart(RESTART_REASON::LANG, _("The New Language will be Available Only After Restarting CherryTree"));
        // config->countryLang = new_lang;
        //dad.country_lang = new_lang
        //lang_file_descriptor = file(cons.LANG_PATH, 'w')
        //lang_file_descriptor.write(new_lang)
        //lang_file_descriptor.close()

    });

    return pMainBox;
}

Gtk::Image* CtPrefDlg::new_image_from_stock(const std::string& id, Gtk::IconSize size)
{
    Gtk::Image* image = Gtk::manage(new Gtk::Image());
    image->set_from_icon_name(id, size);
    return image;
}
