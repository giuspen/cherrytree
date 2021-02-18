/*
 * ct_pref_dlg.cc
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

CtPrefDlg::CtPrefDlg(CtMainWin* parent)
 : Gtk::Dialog{_("Preferences"), *parent, true}
 , _mapCountryLanguages{
        {"bg", _("Bulgarian")},
        {"cs", _("Czech")},
        {"de", _("German")},
        {"el", _("Greek")},
        {"en", _("English")},
        {"es", _("Spanish")},
        {"fi", _("Finnish")},
        {"fr", _("French")},
        {"hy", _("Armenian")},
        {"it", _("Italian")},
        {"ja", _("Japanese")},
        {"lt", _("Lithuanian")},
        {"nl", _("Dutch")},
        {"pl", _("Polish")},
        {"pt_BR", _("Portuguese Brazil")},
        {"ru", _("Russian")},
        {"sl", _("Slovenian")},
        {"sv", _("Swedish")},
        {"tr", _("Turkish")},
        {"uk", _("Ukrainian")},
        {"zh_CN", _("Chinese Simplified")}}
{
    _restartReasons = 0;
    _pCtMainWin = parent;
    _pCtMenu = &_pCtMainWin->get_ct_menu();

    Gtk::Notebook* pNotebook = Gtk::manage(new Gtk::Notebook());
    pNotebook->set_tab_pos(Gtk::PositionType::POS_LEFT);
    pNotebook->append_page(*build_tab_text_n_code(),        _("Text and Code"));
    pNotebook->append_page(*build_tab_rich_text(),          _("Rich Text"));
    pNotebook->append_page(*build_tab_plain_text_n_code(),  _("Plain Text and Code"));
    pNotebook->append_page(*build_tab_special_characters(), _("Special Characters"));
    pNotebook->append_page(*build_tab_tree(),               _("Tree"));
    pNotebook->append_page(*build_tab_theme(),              _("Theme"));
    pNotebook->append_page(*build_tab_fonts(),              _("Fonts"));
    pNotebook->append_page(*build_tab_links(),              _("Links"));
    pNotebook->append_page(*build_tab_toolbar(),            _("Toolbar"));
    pNotebook->append_page(*build_tab_kb_shortcuts(),       _("Keyboard Shortcuts"));
    pNotebook->append_page(*build_tab_misc(),               _("Miscellaneous"));

    get_content_area()->pack_start(*pNotebook);
    get_content_area()->show_all();

    add_button(Gtk::Stock::CLOSE, 1);
}

Gtk::Frame* CtPrefDlg::new_managed_frame_with_align(const Glib::ustring& frameLabel, Gtk::Widget* pFrameChild)
{
    auto pFrame = Gtk::manage(new Gtk::Frame{Glib::ustring{"<b>"}+frameLabel+"</b>"});
    dynamic_cast<Gtk::Label*>(pFrame->get_label_widget())->set_use_markup(true);
    pFrame->set_shadow_type(Gtk::SHADOW_NONE);
    auto pAlign = Gtk::manage(new Gtk::Alignment());
    pAlign->set_padding(3, 6, 6, 6);
    pAlign->add(*pFrameChild);
    pFrame->add(*pAlign);
    return pFrame;
}

Gtk::Widget* CtPrefDlg::build_tab_fonts()
{
    CtConfig* pConfig = _pCtMainWin->get_ct_config();

    Gtk::Image* image_rt = _pCtMainWin->new_image_from_stock("ct_fonts", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_ms = _pCtMainWin->new_image_from_stock("ct_fmt-txt-monospace", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_pt = _pCtMainWin->new_image_from_stock("ct_fonts", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_code = _pCtMainWin->new_image_from_stock("ct_code", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_tree = _pCtMainWin->new_image_from_stock("ct_cherries", Gtk::ICON_SIZE_MENU);
    Gtk::Label* label_rt = Gtk::manage(new Gtk::Label(_("Rich Text")));
    label_rt->set_halign(Gtk::Align::ALIGN_END);
    Gtk::CheckButton* checkbutton_ms = Gtk::manage(new Gtk::CheckButton(_("Monospace")));
    checkbutton_ms->set_halign(Gtk::Align::ALIGN_END);
    checkbutton_ms->set_active(pConfig->msDedicatedFont);
    Gtk::Label* label_pt = Gtk::manage(new Gtk::Label(_("Plain Text")));
    label_pt->set_halign(Gtk::Align::ALIGN_END);
    Gtk::Label* label_code = Gtk::manage(new Gtk::Label(_("Code Font")));
    label_code->set_halign(Gtk::Align::ALIGN_END);
    Gtk::Label* label_tree = Gtk::manage(new Gtk::Label(_("Tree Font")));
    label_tree->set_halign(Gtk::Align::ALIGN_END);
    Gtk::FontButton* fontbutton_rt = Gtk::manage(new Gtk::FontButton(pConfig->rtFont));
    Gtk::FontButton* fontbutton_ms = Gtk::manage(new Gtk::FontButton(pConfig->monospaceFont));
    fontbutton_ms->set_sensitive(pConfig->msDedicatedFont);
    Gtk::FontButton* fontbutton_pt = Gtk::manage(new Gtk::FontButton(pConfig->ptFont));
    Gtk::FontButton* fontbutton_code = Gtk::manage(new Gtk::FontButton(pConfig->codeFont));
    Gtk::FontButton* fontbutton_tree = Gtk::manage(new Gtk::FontButton(pConfig->treeFont));
    Gtk::Grid* grid_fonts = Gtk::manage(new Gtk::Grid());
    grid_fonts->set_row_spacing(2);
    grid_fonts->set_column_spacing(4);
    grid_fonts->set_row_homogeneous(true);
    grid_fonts->attach(*image_rt,        0, 0, 1, 1);
    grid_fonts->attach(*image_ms,        0, 1, 1, 1);
    grid_fonts->attach(*image_pt,        0, 2, 1, 1);
    grid_fonts->attach(*image_code,      0, 3, 1, 1);
    grid_fonts->attach(*image_tree,      0, 4, 1, 1);
    grid_fonts->attach(*label_rt,        1, 0, 1, 1);
    grid_fonts->attach(*checkbutton_ms,  1, 1, 1, 1);
    grid_fonts->attach(*label_pt,        1, 2, 1, 1);
    grid_fonts->attach(*label_code,      1, 3, 1, 1);
    grid_fonts->attach(*label_tree,      1, 4, 1, 1);
    grid_fonts->attach(*fontbutton_rt,   2, 0, 1, 1);
    grid_fonts->attach(*fontbutton_ms,   2, 1, 1, 1);
    grid_fonts->attach(*fontbutton_pt,   2, 2, 1, 1);
    grid_fonts->attach(*fontbutton_code, 2, 3, 1, 1);
    grid_fonts->attach(*fontbutton_tree, 2, 4, 1, 1);
    Gtk::Frame* frame_fonts = new_managed_frame_with_align(_("Fonts"), grid_fonts);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_fonts, false, false);

    fontbutton_rt->signal_font_set().connect([this, pConfig, fontbutton_rt](){
        pConfig->rtFont = fontbutton_rt->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    checkbutton_ms->signal_toggled().connect([this, pConfig, checkbutton_ms, fontbutton_ms](){
        pConfig->msDedicatedFont = checkbutton_ms->get_active();
        fontbutton_ms->set_sensitive(pConfig->msDedicatedFont);
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_family() = pConfig->msDedicatedFont ? "" : CtConst::TAG_PROP_VAL_MONOSPACE;
            tag->property_font() = pConfig->msDedicatedFont ? pConfig->monospaceFont : "";
        }
    });
    fontbutton_ms->signal_font_set().connect([this, pConfig, fontbutton_ms](){
        pConfig->monospaceFont = fontbutton_ms->get_font_name();
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_font() = pConfig->monospaceFont;
        }
    });
    fontbutton_pt->signal_font_set().connect([this, pConfig, fontbutton_pt](){
        pConfig->ptFont = fontbutton_pt->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    fontbutton_code->signal_font_set().connect([this, pConfig, fontbutton_code](){
        pConfig->codeFont = fontbutton_code->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    fontbutton_tree->signal_font_set().connect([this, pConfig, fontbutton_tree](){
        pConfig->treeFont = fontbutton_tree->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
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

    Gtk::Frame* frame_links_actions = new_managed_frame_with_align(_("Custom Actions"), vbox_links_actions);

    checkbutton_custom_weblink_cmd->set_active(pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_sensitive(pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_text(pConfig->weblinkCustomAct);
    checkbutton_custom_filelink_cmd->set_active(pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_sensitive(pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_text(pConfig->filelinkCustomAct);
    checkbutton_custom_folderlink_cmd->set_active(pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_sensitive(pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_text(pConfig->folderlinkCustomAct);

    Gtk::Grid* grid_links_colors = Gtk::manage(new Gtk::Grid());
    grid_links_colors->set_row_spacing(2);
    grid_links_colors->set_column_spacing(15);
    grid_links_colors->set_row_homogeneous(true);

    Gtk::Label* label_col_link_webs = Gtk::manage(new Gtk::Label(_("To WebSite")));
    Gtk::ColorButton* colorbutton_col_link_webs = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkWebs)));
    Gtk::Label* label_col_link_node = Gtk::manage(new Gtk::Label(_("To Node")));
    Gtk::ColorButton* colorbutton_col_link_node = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkNode)));
    Gtk::Label* label_col_link_file = Gtk::manage(new Gtk::Label(_("To File")));
    Gtk::ColorButton* colorbutton_col_link_file = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkFile)));
    Gtk::Label* label_col_link_fold = Gtk::manage(new Gtk::Label(_("To Folder")));
    Gtk::ColorButton* colorbutton_col_link_fold = Gtk::manage(new Gtk::ColorButton(Gdk::RGBA(pConfig->colLinkFold)));

    grid_links_colors->attach(*label_col_link_webs, 0, 0, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_webs, 1, 0, 1, 1);

    grid_links_colors->attach(*label_col_link_node, 0, 1, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_node, 1, 1, 1, 1);

    grid_links_colors->attach(*label_col_link_file, 2, 0, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_file, 3, 0, 1, 1);

    grid_links_colors->attach(*label_col_link_fold, 2, 1, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_fold, 3, 1, 1, 1);

    Gtk::Frame* frame_links_colors = new_managed_frame_with_align(_("Colors"), grid_links_colors);

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

    Gtk::Frame* frame_links_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_links_misc);

    Gtk::VBox* pMainBox = Gtk::manage(new Gtk::VBox());
    pMainBox->set_spacing(3);
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
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

void CtPrefDlg::need_restart(RESTART_REASON reason, const gchar* msg /*= nullptr*/)
{
    if (!(_restartReasons & (int)reason)) {
        _restartReasons |= (int)reason;
        CtDialogs::info_dialog(msg ? msg : _("This Change will have Effect Only After Restarting CherryTree"), *this);
    }
}

void CtPrefDlg::apply_for_each_window(std::function<void(CtMainWin*)> callback)
{
    _pCtMainWin->signal_app_apply_for_each_window(callback);
}
