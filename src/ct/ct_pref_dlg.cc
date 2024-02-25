/*
 * ct_pref_dlg.cc
 *
 * Copyright 2009-2024
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
 : Gtk::Dialog{_("Preferences"), *parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT}
 , _pCtMainWin{parent}
 , _pCtMenu{&_pCtMainWin->get_ct_menu()}
 , _pConfig{_pCtMainWin->get_ct_config()}
 , _mapCountryLanguages{
    {"ar",      _("Arabic")},
    {"hy",      _("Armenian")},
    {"bg",      _("Bulgarian")},
    {"zh_CN",   _("Chinese Simplified")},
    {"zh_TW",   _("Chinese Traditional")},
    {"hr",      _("Croatian")},
    {"cs",      _("Czech")},
    {"nl",      _("Dutch")},
    {"en",      _("English")},
    {"fa",      _("Persian")},
    {"fi",      _("Finnish")},
    {"fr",      _("French")},
    {"de",      _("German")},
    {"el",      _("Greek")},
    {"hi_IN",   _("Hindi India")},
    {"hu",      _("Hungarian")},
    {"it",      _("Italian")},
    {"ja",      _("Japanese")},
    {"kk_KZ",   _("Kazakh")},
    {"kk_LA",   _("Kazakh (Latin)")},
    {"ko",      _("Korean")},
    {"lt",      _("Lithuanian")},
    {"pl",      _("Polish")},
    {"pt",      _("Portuguese")},
    {"pt_BR",   _("Portuguese Brazil")},
    {"ro",      _("Romanian")},
    {"ru",      _("Russian")},
    {"sl",      _("Slovenian")},
    {"es",      _("Spanish")},
    {"sv",      _("Swedish")},
    {"tr",      _("Turkish")},
    {"uk",      _("Ukrainian")}}
{
    auto pNotebook = Gtk::manage(new Gtk::Notebook{});
    pNotebook->set_tab_pos(Gtk::PositionType::POS_LEFT);
    pNotebook->append_page(*build_tab_text_n_code(),        _("Text and Code"));
    pNotebook->append_page(*build_tab_rich_text(),          _("Rich Text"));
    pNotebook->append_page(*build_tab_format(),             _("Format"));
    pNotebook->append_page(*build_tab_plain_text_n_code(),  _("Plain Text and Code"));
    pNotebook->append_page(*build_tab_special_characters(), _("Special Characters"));
    pNotebook->append_page(*build_tab_tree(),               _("Tree Explorer"));
    pNotebook->append_page(*build_tab_theme(),              _("Theme"));
    pNotebook->append_page(*build_tab_interface(),          _("Interface"));
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

Gtk::Widget* CtPrefDlg::build_tab_interface()
{
    Gtk::Image* image_rt = _pCtMainWin->new_managed_image_from_stock("ct_fonts", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_ms = _pCtMainWin->new_managed_image_from_stock("ct_fmt-txt-monospace", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_pt = _pCtMainWin->new_managed_image_from_stock("ct_fonts", Gtk::ICON_SIZE_MENU);
    Gtk::Image* image_code = _pCtMainWin->new_managed_image_from_stock("ct_code", Gtk::ICON_SIZE_MENU);
#if defined(HAVE_VTE)
    Gtk::Image* image_vte = _pCtMainWin->new_managed_image_from_stock("ct_term", Gtk::ICON_SIZE_MENU);
#endif // HAVE_VTE
    Gtk::Image* image_tree = _pCtMainWin->new_managed_image_from_stock("ct_cherries", Gtk::ICON_SIZE_MENU);
    auto label_rt = Gtk::manage(new Gtk::Label{_("Rich Text")});
    label_rt->set_halign(Gtk::Align::ALIGN_END);
    auto checkbutton_ms = Gtk::manage(new Gtk::CheckButton{_("Monospace")});
    checkbutton_ms->set_halign(Gtk::Align::ALIGN_END);
    checkbutton_ms->set_active(_pConfig->msDedicatedFont);
    auto label_pt = Gtk::manage(new Gtk::Label{_("Plain Text")});
    label_pt->set_halign(Gtk::Align::ALIGN_END);
    auto label_code = Gtk::manage(new Gtk::Label{_("Code Font")});
    label_code->set_halign(Gtk::Align::ALIGN_END);
#if defined(HAVE_VTE)
    auto label_vte = Gtk::manage(new Gtk::Label{_("Terminal Font")});
    label_vte->set_halign(Gtk::Align::ALIGN_END);
#endif // HAVE_VTE
    auto label_tree = Gtk::manage(new Gtk::Label{_("Tree Font")});
    label_tree->set_halign(Gtk::Align::ALIGN_END);
    auto fontbutton_rt = Gtk::manage(new Gtk::FontButton{_pConfig->rtFont});
    auto fontbutton_ms = Gtk::manage(new Gtk::FontButton{_pConfig->monospaceFont});
    fontbutton_ms->set_sensitive(_pConfig->msDedicatedFont);
    auto fontbutton_pt = Gtk::manage(new Gtk::FontButton{_pConfig->ptFont});
    auto fontbutton_code = Gtk::manage(new Gtk::FontButton{_pConfig->codeFont});
#if defined(HAVE_VTE)
    auto fontbutton_vte = Gtk::manage(new Gtk::FontButton{_pConfig->vteFont});
#endif // HAVE_VTE
    auto fontbutton_tree = Gtk::manage(new Gtk::FontButton{_pConfig->treeFont});
    auto button_reset_rt = Gtk::manage(new Gtk::Button{});
    button_reset_rt->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_rt->set_tooltip_text(_("Reset to Default"));
    auto button_reset_pt = Gtk::manage(new Gtk::Button{});
    button_reset_pt->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_pt->set_tooltip_text(_("Reset to Default"));
    auto button_reset_tree = Gtk::manage(new Gtk::Button{});
    button_reset_tree->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_tree->set_tooltip_text(_("Reset to Default"));
    auto button_reset_code = Gtk::manage(new Gtk::Button{});
    button_reset_code->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_code->set_tooltip_text(_("Reset to Default"));
#if defined(HAVE_VTE)
    auto button_reset_vte = Gtk::manage(new Gtk::Button{});
    button_reset_vte->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_vte->set_tooltip_text(_("Reset to Default"));
#endif // HAVE_VTE
    auto button_reset_ms = Gtk::manage(new Gtk::Button{});
    button_reset_ms->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
    button_reset_ms->set_tooltip_text(_("Reset to Default"));

    auto grid_fonts = Gtk::manage(new Gtk::Grid{});
    grid_fonts->set_row_spacing(2);
    grid_fonts->set_column_spacing(4);
    grid_fonts->set_row_homogeneous(true);
    grid_fonts->attach(*image_rt,          0, 0, 1, 1);
    grid_fonts->attach(*image_ms,          0, 1, 1, 1);
    grid_fonts->attach(*image_pt,          0, 2, 1, 1);
    grid_fonts->attach(*image_code,        0, 3, 1, 1);
    grid_fonts->attach(*image_tree,        0, 4, 1, 1);
#if defined(HAVE_VTE)
    grid_fonts->attach(*image_vte,         0, 5, 1, 1);
#endif // HAVE_VTE
    grid_fonts->attach(*label_rt,          1, 0, 1, 1);
    grid_fonts->attach(*checkbutton_ms,    1, 1, 1, 1);
    grid_fonts->attach(*label_pt,          1, 2, 1, 1);
    grid_fonts->attach(*label_code,        1, 3, 1, 1);
    grid_fonts->attach(*label_tree,        1, 4, 1, 1);
#if defined(HAVE_VTE)
    grid_fonts->attach(*label_vte,         1, 5, 1, 1);
#endif // HAVE_VTE
    grid_fonts->attach(*fontbutton_rt,     2, 0, 1, 1);
    grid_fonts->attach(*fontbutton_ms,     2, 1, 1, 1);
    grid_fonts->attach(*fontbutton_pt,     2, 2, 1, 1);
    grid_fonts->attach(*fontbutton_code,   2, 3, 1, 1);
    grid_fonts->attach(*fontbutton_tree,   2, 4, 1, 1);
#if defined(HAVE_VTE)
    grid_fonts->attach(*fontbutton_vte,    2, 5, 1, 1);
#endif // HAVE_VTE
    grid_fonts->attach(*button_reset_rt,   3, 0, 1, 1);
    grid_fonts->attach(*button_reset_ms,   3, 1, 1, 1);
    grid_fonts->attach(*button_reset_pt,   3, 2, 1, 1);
    grid_fonts->attach(*button_reset_code, 3, 3, 1, 1);
    grid_fonts->attach(*button_reset_tree, 3, 4, 1, 1);
#if defined(HAVE_VTE)
    grid_fonts->attach(*button_reset_vte,  3, 5, 1, 1);
#endif // HAVE_VTE
    Gtk::Frame* frame_fonts = new_managed_frame_with_align(_("Fonts"), grid_fonts);

    auto vbox_misc = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto checkbutton_word_count = Gtk::manage(new Gtk::CheckButton{_("Enable Word Count in Statusbar")});
    auto checkbutton_win_title_doc_dir = Gtk::manage(new Gtk::CheckButton{_("Show the Document Directory in the Window Title")});
    auto checkbutton_nn_header_full_path = Gtk::manage(new Gtk::CheckButton{_("Show the Full Path in the Node Name Header")});
    auto checkbutton_bookmarks_top_menu = Gtk::manage(new Gtk::CheckButton{_("Dedicated Bookmarks Menu in Menubar")});
    auto checkbutton_menubar_in_titlebar = Gtk::manage(new Gtk::CheckButton{_("Menubar in Titlebar")});

    checkbutton_word_count->set_active(_pConfig->wordCountOn);
    checkbutton_win_title_doc_dir->set_active(_pConfig->winTitleShowDocDir);
    checkbutton_nn_header_full_path->set_active(_pConfig->nodeNameHeaderShowFullPath);
    checkbutton_bookmarks_top_menu->set_active(_pConfig->bookmarksInTopMenu);
    checkbutton_menubar_in_titlebar->set_active(_pConfig->menubarInTitlebar);

    auto hbox_toolbar_icons_size = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_toolbar_icons_size = Gtk::manage(new Gtk::Label{_("Toolbar Icons Size")});
    Glib::RefPtr<Gtk::Adjustment> adjustment_toolbar_icons_size = Gtk::Adjustment::create(_pConfig->toolbarIconSize, 2, 5, 1);
    auto spinbutton_toolbar_icons_size = Gtk::manage(new Gtk::SpinButton{adjustment_toolbar_icons_size});
    hbox_toolbar_icons_size->pack_start(*label_toolbar_icons_size, false, false);
    hbox_toolbar_icons_size->pack_start(*spinbutton_toolbar_icons_size, false, false);

    auto hbox_scrollbar_overlay = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 6/*spacing*/});
    auto label_scrollbar_overlay = Gtk::manage(new Gtk::Label{_("Scrollbar Overlays Text Editor")});
    auto radiobutton_scrollbar_overlay_default = Gtk::manage(new Gtk::RadioButton{_("System Default")});
    auto radiobutton_scrollbar_overlay_on = Gtk::manage(new Gtk::RadioButton{_("Yes")});
    radiobutton_scrollbar_overlay_on->join_group(*radiobutton_scrollbar_overlay_default);
    auto radiobutton_scrollbar_overlay_off = Gtk::manage(new Gtk::RadioButton{_("No")});
    radiobutton_scrollbar_overlay_off->join_group(*radiobutton_scrollbar_overlay_default);
    hbox_scrollbar_overlay->pack_start(*label_scrollbar_overlay, false, false);
    hbox_scrollbar_overlay->pack_start(*radiobutton_scrollbar_overlay_default, false, false);
    hbox_scrollbar_overlay->pack_start(*radiobutton_scrollbar_overlay_on, false, false);
    hbox_scrollbar_overlay->pack_start(*radiobutton_scrollbar_overlay_off, false, false);
    radiobutton_scrollbar_overlay_default->set_active(2 == _pConfig->overlayScroll);
    radiobutton_scrollbar_overlay_on->set_active(1 == _pConfig->overlayScroll);
    radiobutton_scrollbar_overlay_off->set_active(0 ==_pConfig->overlayScroll);

    vbox_misc->pack_start(*checkbutton_word_count, false, false);
    vbox_misc->pack_start(*checkbutton_win_title_doc_dir, false, false);
    vbox_misc->pack_start(*checkbutton_nn_header_full_path, false, false);
    vbox_misc->pack_start(*checkbutton_bookmarks_top_menu, false, false);
    vbox_misc->pack_start(*checkbutton_menubar_in_titlebar, false, false);
    vbox_misc->pack_start(*hbox_toolbar_icons_size, false, false);
    vbox_misc->pack_start(*hbox_scrollbar_overlay, false, false);

    Gtk::Frame* frame_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_fonts, false, false);
    pMainBox->pack_start(*frame_misc, false, false);

    auto f_on_font_rt_set = [this, fontbutton_rt](){
        _pConfig->rtFont = fontbutton_rt->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    };
    fontbutton_rt->signal_font_set().connect(f_on_font_rt_set);
    checkbutton_ms->signal_toggled().connect([this, checkbutton_ms, fontbutton_ms](){
        _pConfig->msDedicatedFont = checkbutton_ms->get_active();
        fontbutton_ms->set_sensitive(_pConfig->msDedicatedFont);
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_family() = _pConfig->msDedicatedFont ? "" : CtConst::TAG_PROP_VAL_MONOSPACE;
            tag->property_font() = _pConfig->msDedicatedFont ? _pConfig->monospaceFont : "";
        }
    });
    auto f_on_font_ms_set = [this, fontbutton_ms](){
        _pConfig->monospaceFont = fontbutton_ms->get_font_name();
        if (_pConfig->msDedicatedFont) {
            if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
                tag->property_font() = _pConfig->monospaceFont;
            }
        }
    };
    fontbutton_ms->signal_font_set().connect(f_on_font_ms_set);
    auto f_on_font_pt_set = [this, fontbutton_pt](){
        _pConfig->ptFont = fontbutton_pt->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    };
    fontbutton_pt->signal_font_set().connect(f_on_font_pt_set);
    auto f_on_font_code_set = [this, fontbutton_code](){
        _pConfig->codeFont = fontbutton_code->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    };
    fontbutton_code->signal_font_set().connect(f_on_font_code_set);
#if defined(HAVE_VTE)
    auto f_on_font_vte_set = [this, fontbutton_vte](){
        _pConfig->vteFont = fontbutton_vte->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_vte_settings(); });
    };
    fontbutton_vte->signal_font_set().connect(f_on_font_vte_set);
#endif // HAVE_VTE
    auto f_on_font_tree_set = [this, fontbutton_tree](){
        _pConfig->treeFont = fontbutton_tree->get_font_name();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); win->window_header_update(); });
    };
    fontbutton_tree->signal_font_set().connect(f_on_font_tree_set);
    button_reset_rt->signal_clicked().connect([fontbutton_rt, f_on_font_rt_set](){
        fontbutton_rt->set_font_name(CtConst::FONT_RT_DEFAULT);
        f_on_font_rt_set();
    });
    button_reset_pt->signal_clicked().connect([fontbutton_pt, f_on_font_pt_set](){
        fontbutton_pt->set_font_name(CtConst::FONT_PT_DEFAULT);
        f_on_font_pt_set();
    });
    button_reset_tree->signal_clicked().connect([fontbutton_tree, f_on_font_tree_set](){
        fontbutton_tree->set_font_name(CtConst::FONT_TREE_DEFAULT);
        f_on_font_tree_set();
    });
    button_reset_code->signal_clicked().connect([fontbutton_code, f_on_font_code_set](){
        fontbutton_code->set_font_name(CtConst::FONT_CODE_DEFAULT);
        f_on_font_code_set();
    });
#if defined(HAVE_VTE)
    button_reset_vte->signal_clicked().connect([fontbutton_vte, f_on_font_vte_set](){
        fontbutton_vte->set_font_name(CtConst::FONT_VTE_DEFAULT);
        f_on_font_vte_set();
    });
#endif // HAVE_VTE
    button_reset_ms->signal_clicked().connect([fontbutton_ms, f_on_font_ms_set](){
        fontbutton_ms->set_font_name(CtConst::FONT_MS_DEFAULT);
        f_on_font_ms_set();
    });

    checkbutton_win_title_doc_dir->signal_toggled().connect([this, checkbutton_win_title_doc_dir](){
        _pConfig->winTitleShowDocDir = checkbutton_win_title_doc_dir->get_active();
        _pCtMainWin->window_title_update();
    });
    checkbutton_nn_header_full_path->signal_toggled().connect([this, checkbutton_nn_header_full_path](){
        _pConfig->nodeNameHeaderShowFullPath = checkbutton_nn_header_full_path->get_active();
        _pCtMainWin->window_header_update();
    });
    checkbutton_bookmarks_top_menu->signal_toggled().connect([this, checkbutton_bookmarks_top_menu](){
        _pConfig->bookmarksInTopMenu = checkbutton_bookmarks_top_menu->get_active();
        _pCtMainWin->menu_top_optional_bookmarks_enforce();
    });
    checkbutton_menubar_in_titlebar->signal_toggled().connect([this, checkbutton_menubar_in_titlebar](){
        _pConfig->menubarInTitlebar = checkbutton_menubar_in_titlebar->get_active();
        need_restart(RESTART_REASON::MENUBAR_IN_TITLEBAR);
    });
    checkbutton_word_count->signal_toggled().connect([this, checkbutton_word_count](){
        _pConfig->wordCountOn = checkbutton_word_count->get_active();
        apply_for_each_window([](CtMainWin* win) { win->update_selected_node_statusbar_info(); });
    });
    spinbutton_toolbar_icons_size->signal_value_changed().connect([this, spinbutton_toolbar_icons_size](){
        _pConfig->toolbarIconSize = spinbutton_toolbar_icons_size->get_value_as_int();
        apply_for_each_window([this](CtMainWin* win) { win->set_toolbars_icon_size(_pConfig->toolbarIconSize); });
    });

    radiobutton_scrollbar_overlay_default->signal_toggled().connect([this, radiobutton_scrollbar_overlay_default](){
        if (not radiobutton_scrollbar_overlay_default->get_active()) return;
        _pConfig->overlayScroll = 2;
        need_restart(RESTART_REASON::OVERLAY_SCROLL);
    });
    radiobutton_scrollbar_overlay_on->signal_toggled().connect([this, radiobutton_scrollbar_overlay_on](){
        if (not radiobutton_scrollbar_overlay_on->get_active()) return;
        _pConfig->overlayScroll = 1;
        _pCtMainWin->getScrolledwindowText().set_overlay_scrolling(static_cast<bool>(_pConfig->overlayScroll));
    });
    radiobutton_scrollbar_overlay_off->signal_toggled().connect([this, radiobutton_scrollbar_overlay_off](){
        if (not radiobutton_scrollbar_overlay_off->get_active()) return;
        _pConfig->overlayScroll = 0;
        _pCtMainWin->getScrolledwindowText().set_overlay_scrolling(static_cast<bool>(_pConfig->overlayScroll));
    });

    return pMainBox;
}

Gtk::Widget* CtPrefDlg::build_tab_links()
{
    auto vbox_links_actions = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto checkbutton_custom_weblink_cmd = Gtk::manage(new Gtk::CheckButton{_("Enable Custom Web Link Clicked Action")});
    auto entry_custom_weblink_cmd = Gtk::manage(new Gtk::Entry{});
    auto checkbutton_custom_filelink_cmd = Gtk::manage(new Gtk::CheckButton{_("Enable Custom File Link Clicked Action")});
    auto entry_custom_filelink_cmd = Gtk::manage(new Gtk::Entry{});
    auto checkbutton_custom_folderlink_cmd = Gtk::manage(new Gtk::CheckButton{_("Enable Custom Folder Link Clicked Action")});
    auto entry_custom_folderlink_cmd = Gtk::manage(new Gtk::Entry{});
    vbox_links_actions->pack_start(*checkbutton_custom_weblink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_weblink_cmd, false, false);
    vbox_links_actions->pack_start(*checkbutton_custom_filelink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_filelink_cmd, false, false);
    vbox_links_actions->pack_start(*checkbutton_custom_folderlink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_folderlink_cmd, false, false);

    Gtk::Frame* frame_links_actions = new_managed_frame_with_align(_("Custom Actions"), vbox_links_actions);

    checkbutton_custom_weblink_cmd->set_active(_pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_sensitive(_pConfig->weblinkCustomOn);
    entry_custom_weblink_cmd->set_text(_pConfig->weblinkCustomAct);
    checkbutton_custom_filelink_cmd->set_active(_pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_sensitive(_pConfig->filelinkCustomOn);
    entry_custom_filelink_cmd->set_text(_pConfig->filelinkCustomAct);
    checkbutton_custom_folderlink_cmd->set_active(_pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_sensitive(_pConfig->folderlinkCustomOn);
    entry_custom_folderlink_cmd->set_text(_pConfig->folderlinkCustomAct);

    auto grid_links_colors = Gtk::manage(new Gtk::Grid{});
    grid_links_colors->set_row_spacing(2);
    grid_links_colors->set_column_spacing(15);
    grid_links_colors->set_row_homogeneous(true);

    auto label_col_link_webs = Gtk::manage(new Gtk::Label{_("To WebSite")});
    auto colorbutton_col_link_webs = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA(_pConfig->colLinkWebs)});
    auto label_col_link_node = Gtk::manage(new Gtk::Label{_("To Node")});
    auto colorbutton_col_link_node = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA(_pConfig->colLinkNode)});
    auto label_col_link_file = Gtk::manage(new Gtk::Label{_("To File")});
    auto colorbutton_col_link_file = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA(_pConfig->colLinkFile)});
    auto label_col_link_fold = Gtk::manage(new Gtk::Label{_("To Folder")});
    auto colorbutton_col_link_fold = Gtk::manage(new Gtk::ColorButton{Gdk::RGBA(_pConfig->colLinkFold)});

    grid_links_colors->attach(*label_col_link_webs, 0, 0, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_webs, 1, 0, 1, 1);

    grid_links_colors->attach(*label_col_link_node, 0, 1, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_node, 1, 1, 1, 1);

    grid_links_colors->attach(*label_col_link_file, 2, 0, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_file, 3, 0, 1, 1);

    grid_links_colors->attach(*label_col_link_fold, 2, 1, 1, 1);
    grid_links_colors->attach(*colorbutton_col_link_fold, 3, 1, 1, 1);

    Gtk::Frame* frame_links_colors = new_managed_frame_with_align(_("Colors"), grid_links_colors);

    auto vbox_links_misc = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL});
    auto checkbutton_links_underline = Gtk::manage(new Gtk::CheckButton{_("Underline Links")});
    checkbutton_links_underline->set_active(_pConfig->linksUnderline);
    auto checkbutton_links_relative = Gtk::manage(new Gtk::CheckButton{_("Use Relative Paths for Files And Folders")});
    checkbutton_links_relative->set_active(_pConfig->linksRelative);
    auto hbox_anchor_size = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_anchor_size = Gtk::manage(new Gtk::Label{_("Anchor Size")});
    Glib::RefPtr<Gtk::Adjustment> adj_anchor_size = Gtk::Adjustment::create(_pConfig->anchorSize, 1, 1000, 1);
    auto spinbutton_anchor_size = Gtk::manage(new Gtk::SpinButton{adj_anchor_size});
    spinbutton_anchor_size->set_value(_pConfig->anchorSize);
    hbox_anchor_size->pack_start(*label_anchor_size, false, false);
    hbox_anchor_size->pack_start(*spinbutton_anchor_size, false, false);
    vbox_links_misc->pack_start(*checkbutton_links_underline, false, false);
    vbox_links_misc->pack_start(*checkbutton_links_relative, false, false);
    vbox_links_misc->pack_start(*hbox_anchor_size, false, false);

    Gtk::Frame* frame_links_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_links_misc);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_left(6);
    pMainBox->set_margin_top(6);
    pMainBox->pack_start(*frame_links_actions, false, false);
    pMainBox->pack_start(*frame_links_colors, false, false);
    pMainBox->pack_start(*frame_links_misc, false, false);

    checkbutton_custom_weblink_cmd->signal_toggled().connect([this, checkbutton_custom_weblink_cmd, entry_custom_weblink_cmd](){
        _pConfig->weblinkCustomOn = checkbutton_custom_weblink_cmd->get_active();
        entry_custom_weblink_cmd->set_sensitive(_pConfig->weblinkCustomOn);
    });
    entry_custom_weblink_cmd->signal_changed().connect([this, entry_custom_weblink_cmd](){
        _pConfig->weblinkCustomAct = entry_custom_weblink_cmd->get_text();
    });
    checkbutton_custom_filelink_cmd->signal_toggled().connect([this, checkbutton_custom_filelink_cmd, entry_custom_filelink_cmd](){
        _pConfig->filelinkCustomOn = checkbutton_custom_filelink_cmd->get_active();
        entry_custom_filelink_cmd->set_sensitive(_pConfig->filelinkCustomOn);
    });
    entry_custom_filelink_cmd->signal_changed().connect([this, entry_custom_filelink_cmd](){
        _pConfig->filelinkCustomAct = entry_custom_filelink_cmd->get_text();
    });
    checkbutton_custom_folderlink_cmd->signal_toggled().connect([this, checkbutton_custom_folderlink_cmd, entry_custom_folderlink_cmd](){
        _pConfig->folderlinkCustomOn = checkbutton_custom_folderlink_cmd->get_active();
        entry_custom_folderlink_cmd->set_sensitive(_pConfig->folderlinkCustomOn);
    });
    entry_custom_folderlink_cmd->signal_changed().connect([this, entry_custom_folderlink_cmd](){
        _pConfig->folderlinkCustomAct = entry_custom_folderlink_cmd->get_text();
    });
    checkbutton_links_relative->signal_toggled().connect([this, checkbutton_links_relative](){
        _pConfig->linksRelative = checkbutton_links_relative->get_active();
    });
    checkbutton_links_underline->signal_toggled().connect([this, checkbutton_links_underline](){
        _pConfig->linksUnderline = checkbutton_links_underline->get_active();
        need_restart(RESTART_REASON::LINKS);
    });
    spinbutton_anchor_size->signal_value_changed().connect([this, spinbutton_anchor_size](){
        _pConfig->anchorSize = spinbutton_anchor_size->get_value_as_int();
        need_restart(RESTART_REASON::ANCHOR_SIZE);
    });
    colorbutton_col_link_webs->signal_color_set().connect([this, colorbutton_col_link_webs](){
        _pConfig->colLinkWebs = CtRgbUtil::rgb_to_string_24(colorbutton_col_link_webs->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_node->signal_color_set().connect([this, colorbutton_col_link_node](){
        _pConfig->colLinkNode = CtRgbUtil::rgb_to_string_24(colorbutton_col_link_node->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_file->signal_color_set().connect([this, colorbutton_col_link_file](){
        _pConfig->colLinkFile =  CtRgbUtil::rgb_to_string_24(colorbutton_col_link_file->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });
    colorbutton_col_link_fold->signal_color_set().connect([this, colorbutton_col_link_fold](){
        _pConfig->colLinkFold = CtRgbUtil::rgb_to_string_24(colorbutton_col_link_fold->get_rgba());
        need_restart(RESTART_REASON::COLOR);
    });

    return pMainBox;
}

void CtPrefDlg::need_restart(RESTART_REASON reason, const gchar* msg /*= nullptr*/)
{
    if (!(_restartReasons & (int)reason)) {
        _restartReasons |= (int)reason;
        CtDialogs::info_dialog(msg ? msg : _("This Change will have Effect Only After Restarting CherryTree."), *this);
    }
}

void CtPrefDlg::apply_for_each_window(std::function<void(CtMainWin*)> callback)
{
    _pCtMainWin->signal_app_apply_for_each_window(callback);
}
