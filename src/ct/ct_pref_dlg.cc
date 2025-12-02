/*
 * ct_pref_dlg.cc
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

#include <sigc++/sigc++.h>
#include "ct_pref_dlg.h"
#include "ct_main_win.h"
#include "ct_actions.h"

CtPrefDlg::CtPrefDlg(CtMainWin* parent)
#if GTKMM_MAJOR_VERSION >= 4
 : Gtk::Dialog{_("Preferences"), *parent, /*modal*/true, /*use_header_bar*/true}
#else
 : Gtk::Dialog{_("Preferences"), *parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT}
#endif
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
    {"sk",      _("Slovak")},
    {"sl",      _("Slovenian")},
    {"es",      _("Spanish")},
    {"sv",      _("Swedish")},
    {"tr",      _("Turkish")},
    {"uk",      _("Ukrainian")}}
{
    auto pNotebook = Gtk::manage(new Gtk::Notebook{});
#if GTKMM_MAJOR_VERSION >= 4
    pNotebook->set_tab_pos(Gtk::PositionType::LEFT);
#else
    pNotebook->set_tab_pos(Gtk::PositionType::POS_LEFT);
#endif
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

#if GTKMM_MAJOR_VERSION >= 4
    get_content_area()->append(*pNotebook);
#else
    get_content_area()->pack_start(*pNotebook);
    get_content_area()->show_all();
#endif

    (void)CtMiscUtil::dialog_add_button(this, _("OK"), (Gtk::ResponseType)1, "ct_close");
}

Gtk::Frame* CtPrefDlg::new_managed_frame_with_align(const Glib::ustring& frameLabel, Gtk::Widget* pFrameChild)
{
    auto pFrame = Gtk::manage(new Gtk::Frame{Glib::ustring{"<b>"}+frameLabel+"</b>"});
    dynamic_cast<Gtk::Label*>(pFrame->get_label_widget())->set_use_markup(true);
    CtMiscUtil::set_widget_margins(*pFrameChild, 3, 6, 6, 6);
#if GTKMM_MAJOR_VERSION >= 4
    pFrame->set_child(*pFrameChild);
#else
    pFrame->set_shadow_type(Gtk::SHADOW_NONE);
    pFrame->add(*pFrameChild);
#endif
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
        label_rt->set_halign(
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Align::END
    #else
        Gtk::Align::ALIGN_END
    #endif
        );
    auto checkbutton_ms = Gtk::manage(new Gtk::CheckButton{_("Monospace")});
        checkbutton_ms->set_halign(
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Align::END
    #else
        Gtk::Align::ALIGN_END
    #endif
        );
    checkbutton_ms->set_active(_pConfig->msDedicatedFont);
    auto label_pt = Gtk::manage(new Gtk::Label{_("Plain Text")});
        label_pt->set_halign(
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Align::END
    #else
        Gtk::Align::ALIGN_END
    #endif
        );
    auto label_code = Gtk::manage(new Gtk::Label{_("Code Font")});
        label_code->set_halign(
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Align::END
    #else
        Gtk::Align::ALIGN_END
    #endif
        );
#if defined(HAVE_VTE)
    auto label_vte = Gtk::manage(new Gtk::Label{_("Terminal Font")});
        label_vte->set_halign(
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Align::END
    #else
        Gtk::Align::ALIGN_END
    #endif
        );
#endif // HAVE_VTE
    auto label_tree = Gtk::manage(new Gtk::Label{_("Tree Font")});
        label_tree->set_halign(
    #if GTKMM_MAJOR_VERSION >= 4
        Gtk::Align::END
    #else
        Gtk::Align::ALIGN_END
    #endif
        );
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
#if GTKMM_MAJOR_VERSION < 4
    button_reset_rt->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
    button_reset_rt->set_tooltip_text(_("Reset to Default"));
    auto button_reset_pt = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_pt->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
    button_reset_pt->set_tooltip_text(_("Reset to Default"));
    auto button_reset_tree = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_tree->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
    button_reset_tree->set_tooltip_text(_("Reset to Default"));
    auto button_reset_code = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_code->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
    button_reset_code->set_tooltip_text(_("Reset to Default"));
#if defined(HAVE_VTE)
    auto button_reset_vte = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_vte->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
    button_reset_vte->set_tooltip_text(_("Reset to Default"));
#endif // HAVE_VTE
    auto button_reset_ms = Gtk::manage(new Gtk::Button{});
#if GTKMM_MAJOR_VERSION < 4
    button_reset_ms->set_image(*_pCtMainWin->new_managed_image_from_stock("ct_undo", Gtk::ICON_SIZE_BUTTON));
#endif
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
#if GTKMM_MAJOR_VERSION >= 4
    hbox_toolbar_icons_size->append(*label_toolbar_icons_size);
    hbox_toolbar_icons_size->append(*spinbutton_toolbar_icons_size);
#else
    hbox_toolbar_icons_size->pack_start(*label_toolbar_icons_size, false, false);
    hbox_toolbar_icons_size->pack_start(*spinbutton_toolbar_icons_size, false, false);
#endif

    auto hbox_nodes_on_node_name_header = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_nodes_on_node_name_header = Gtk::manage(new Gtk::Label{_("Last Visited Nodes on Node Name Header")});
    Glib::RefPtr<Gtk::Adjustment> adj_nodes_on_node_name_header = Gtk::Adjustment::create(_pConfig->nodesOnNodeNameHeader, 0, 100, 1);
    auto spinbutton_nodes_on_node_name_header = Gtk::manage(new Gtk::SpinButton{adj_nodes_on_node_name_header});
    spinbutton_nodes_on_node_name_header->set_value(_pConfig->nodesOnNodeNameHeader);
#if GTKMM_MAJOR_VERSION >= 4
    hbox_nodes_on_node_name_header->append(*label_nodes_on_node_name_header);
    hbox_nodes_on_node_name_header->append(*spinbutton_nodes_on_node_name_header);
#else
    hbox_nodes_on_node_name_header->pack_start(*label_nodes_on_node_name_header, false, false);
    hbox_nodes_on_node_name_header->pack_start(*spinbutton_nodes_on_node_name_header, false, false);
#endif

    auto hbox_scrollbar_min_size = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_scrollbar_min_size = Gtk::manage(new Gtk::Label{_("Scrollbar Slider Minimum Size (0 = System Default)")});
    Glib::RefPtr<Gtk::Adjustment> adjustment_scrollbar_min_size = Gtk::Adjustment::create(_pConfig->scrollSliderMin, 0, 1000, 1);
    auto spinbutton_scrollbar_min_size = Gtk::manage(new Gtk::SpinButton{adjustment_scrollbar_min_size});
#if GTKMM_MAJOR_VERSION >= 4
    hbox_scrollbar_min_size->append(*label_scrollbar_min_size);
    hbox_scrollbar_min_size->append(*spinbutton_scrollbar_min_size);
#else
    hbox_scrollbar_min_size->pack_start(*label_scrollbar_min_size, false, false);
    hbox_scrollbar_min_size->pack_start(*spinbutton_scrollbar_min_size, false, false);
#endif

    auto hbox_scrollbar_overlay = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 6/*spacing*/});
    auto label_scrollbar_overlay = Gtk::manage(new Gtk::Label{_("Scrollbar Overlays Text Editor")});
#if GTKMM_MAJOR_VERSION >= 4
    auto radiobutton_scrollbar_overlay_default = Gtk::manage(new Gtk::CheckButton{_("System Default")});
    auto radiobutton_scrollbar_overlay_on = Gtk::manage(new Gtk::CheckButton{_("Yes")});
    radiobutton_scrollbar_overlay_on->set_group(*radiobutton_scrollbar_overlay_default);
    auto radiobutton_scrollbar_overlay_off = Gtk::manage(new Gtk::CheckButton{_("No")});
    radiobutton_scrollbar_overlay_off->set_group(*radiobutton_scrollbar_overlay_default);
    hbox_scrollbar_overlay->append(*label_scrollbar_overlay);
    hbox_scrollbar_overlay->append(*radiobutton_scrollbar_overlay_default);
    hbox_scrollbar_overlay->append(*radiobutton_scrollbar_overlay_on);
    hbox_scrollbar_overlay->append(*radiobutton_scrollbar_overlay_off);
#else
    auto radiobutton_scrollbar_overlay_default = Gtk::manage(new Gtk::RadioButton{_("System Default")});
    auto radiobutton_scrollbar_overlay_on = Gtk::manage(new Gtk::RadioButton{_("Yes")});
    radiobutton_scrollbar_overlay_on->join_group(*radiobutton_scrollbar_overlay_default);
    auto radiobutton_scrollbar_overlay_off = Gtk::manage(new Gtk::RadioButton{_("No")});
    radiobutton_scrollbar_overlay_off->join_group(*radiobutton_scrollbar_overlay_default);
    hbox_scrollbar_overlay->pack_start(*label_scrollbar_overlay, false, false);
    hbox_scrollbar_overlay->pack_start(*radiobutton_scrollbar_overlay_default, false, false);
    hbox_scrollbar_overlay->pack_start(*radiobutton_scrollbar_overlay_on, false, false);
    hbox_scrollbar_overlay->pack_start(*radiobutton_scrollbar_overlay_off, false, false);
#endif
    radiobutton_scrollbar_overlay_default->set_active(2 == _pConfig->overlayScroll);
    radiobutton_scrollbar_overlay_on->set_active(1 == _pConfig->overlayScroll);
    radiobutton_scrollbar_overlay_off->set_active(0 ==_pConfig->overlayScroll);

    auto hbox_tooltips_enable = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 3/*spacing*/});
    auto label_tooltips_enable = Gtk::manage(new Gtk::Label{_("Enable Tooltips When Hovering")});
    auto checkbutton_tooltips_enable_tree = Gtk::manage(new Gtk::CheckButton{_("Tree")});
    auto checkbutton_tooltips_enable_menus = Gtk::manage(new Gtk::CheckButton{_("Menus")});
    auto checkbutton_tooltips_enable_toolbar = Gtk::manage(new Gtk::CheckButton{_("Toolbar")});
#if GTKMM_MAJOR_VERSION >= 4
    hbox_tooltips_enable->append(*label_tooltips_enable);
    hbox_tooltips_enable->append(*checkbutton_tooltips_enable_tree);
    hbox_tooltips_enable->append(*checkbutton_tooltips_enable_menus);
    hbox_tooltips_enable->append(*checkbutton_tooltips_enable_toolbar);
#else
    hbox_tooltips_enable->pack_start(*label_tooltips_enable, false, false);
    hbox_tooltips_enable->pack_start(*checkbutton_tooltips_enable_tree, false, false);
    hbox_tooltips_enable->pack_start(*checkbutton_tooltips_enable_menus, false, false);
    hbox_tooltips_enable->pack_start(*checkbutton_tooltips_enable_toolbar, false, false);
#endif
    checkbutton_tooltips_enable_tree->set_active(_pConfig->treeTooltips);
    checkbutton_tooltips_enable_menus->set_active(_pConfig->menusTooltips);
    checkbutton_tooltips_enable_toolbar->set_active(_pConfig->toolbarTooltips);

    auto hbox_find_all_max_in_page = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_find_all_max_in_page = Gtk::manage(new Gtk::Label{_("Max Search Results per Page")});
    label_find_all_max_in_page->set_margin_start(2);
    Glib::RefPtr<Gtk::Adjustment> adjustment_find_all_max_in_page = Gtk::Adjustment::create(_pConfig->maxMatchesInPage, 0, 100000, 1);
    auto spinbutton_find_all_max_in_page = Gtk::manage(new Gtk::SpinButton{adjustment_find_all_max_in_page});
#if GTKMM_MAJOR_VERSION >= 4
    hbox_find_all_max_in_page->append(*label_find_all_max_in_page);
    hbox_find_all_max_in_page->append(*spinbutton_find_all_max_in_page);
#else
    hbox_find_all_max_in_page->pack_start(*label_find_all_max_in_page, false, false);
    hbox_find_all_max_in_page->pack_start(*spinbutton_find_all_max_in_page, false, false);
#endif

#if GTKMM_MAJOR_VERSION >= 4
    vbox_misc->append(*checkbutton_word_count);
    vbox_misc->append(*checkbutton_win_title_doc_dir);
    vbox_misc->append(*checkbutton_nn_header_full_path);
    vbox_misc->append(*checkbutton_bookmarks_top_menu);
    vbox_misc->append(*checkbutton_menubar_in_titlebar);
    vbox_misc->append(*hbox_toolbar_icons_size);
    vbox_misc->append(*hbox_nodes_on_node_name_header);
    vbox_misc->append(*hbox_scrollbar_min_size);
    vbox_misc->append(*hbox_scrollbar_overlay);
    vbox_misc->append(*hbox_tooltips_enable);
    vbox_misc->append(*hbox_find_all_max_in_page);
#else
    vbox_misc->pack_start(*checkbutton_word_count, false, false);
    vbox_misc->pack_start(*checkbutton_win_title_doc_dir, false, false);
    vbox_misc->pack_start(*checkbutton_nn_header_full_path, false, false);
    vbox_misc->pack_start(*checkbutton_bookmarks_top_menu, false, false);
    vbox_misc->pack_start(*checkbutton_menubar_in_titlebar, false, false);
    vbox_misc->pack_start(*hbox_toolbar_icons_size, false, false);
    vbox_misc->pack_start(*hbox_nodes_on_node_name_header, false, false);
    vbox_misc->pack_start(*hbox_scrollbar_min_size, false, false);
    vbox_misc->pack_start(*hbox_scrollbar_overlay, false, false);
    vbox_misc->pack_start(*hbox_tooltips_enable, false, false);
    vbox_misc->pack_start(*hbox_find_all_max_in_page, false, false);
#endif

    Gtk::Frame* frame_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_misc);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_start(6);
    pMainBox->set_margin_top(6);
#if GTKMM_MAJOR_VERSION >= 4
    pMainBox->append(*frame_fonts);
    pMainBox->append(*frame_misc);
#else
    pMainBox->pack_start(*frame_fonts, false, false);
    pMainBox->pack_start(*frame_misc, false, false);
#endif

    auto f_on_font_rt_set = [this, fontbutton_rt](){
#if GTKMM_MAJOR_VERSION >= 4
        _pConfig->rtFont = fontbutton_rt->get_font();
#else
        _pConfig->rtFont = fontbutton_rt->get_font_name();
#endif
        _pConfig->rtResetFontSize = 0;
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    };
    fontbutton_rt->signal_font_set().connect(f_on_font_rt_set);
    checkbutton_ms->signal_toggled().connect([this, checkbutton_ms, fontbutton_ms](){
        _pConfig->msDedicatedFont = checkbutton_ms->get_active();
        _pConfig->msResetFontSize = 0;
        fontbutton_ms->set_sensitive(_pConfig->msDedicatedFont);
        if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
            tag->property_family() = _pConfig->msDedicatedFont ? "" : CtConst::TAG_PROP_VAL_MONOSPACE;
            tag->property_font() = _pConfig->msDedicatedFont ? _pConfig->monospaceFont : "";
        }
    });
    auto f_on_font_ms_set = [this, fontbutton_ms](){
#if GTKMM_MAJOR_VERSION >= 4
        _pConfig->monospaceFont = fontbutton_ms->get_font();
#else
        _pConfig->monospaceFont = fontbutton_ms->get_font_name();
#endif
        if (_pConfig->msDedicatedFont) {
            if (auto tag = _pCtMainWin->get_text_tag_table()->lookup(CtConst::TAG_ID_MONOSPACE)) {
                tag->property_font() = _pConfig->monospaceFont;
            }
        }
    };
    fontbutton_ms->signal_font_set().connect(f_on_font_ms_set);
    auto f_on_font_pt_set = [this, fontbutton_pt](){
#if GTKMM_MAJOR_VERSION >= 4
        _pConfig->ptFont = fontbutton_pt->get_font();
#else
        _pConfig->ptFont = fontbutton_pt->get_font_name();
#endif
        _pConfig->ptResetFontSize = 0;
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    };
    fontbutton_pt->signal_font_set().connect(f_on_font_pt_set);
    auto f_on_font_code_set = [this, fontbutton_code](){
#if GTKMM_MAJOR_VERSION >= 4
        _pConfig->codeFont = fontbutton_code->get_font();
#else
        _pConfig->codeFont = fontbutton_code->get_font_name();
#endif
        _pConfig->codeResetFontSize = 0;
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    };
    fontbutton_code->signal_font_set().connect(f_on_font_code_set);
#if defined(HAVE_VTE)
    auto f_on_font_vte_set = [this, fontbutton_vte](){
#if GTKMM_MAJOR_VERSION >= 4
        _pConfig->vteFont = fontbutton_vte->get_font();
#else
        _pConfig->vteFont = fontbutton_vte->get_font_name();
#endif
        apply_for_each_window([](CtMainWin* win) { win->update_vte_settings(); });
    };
    fontbutton_vte->signal_font_set().connect(f_on_font_vte_set);
#endif // HAVE_VTE
    auto f_on_font_tree_set = [this, fontbutton_tree](){
#if GTKMM_MAJOR_VERSION >= 4
        _pConfig->treeFont = fontbutton_tree->get_font();
#else
        _pConfig->treeFont = fontbutton_tree->get_font_name();
#endif
        _pConfig->treeResetFontSize = 0;
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); win->window_header_update(); });
    };
    fontbutton_tree->signal_font_set().connect(f_on_font_tree_set);
    button_reset_rt->signal_clicked().connect([fontbutton_rt, f_on_font_rt_set](){
#if GTKMM_MAJOR_VERSION >= 4
        fontbutton_rt->set_font(CtConst::FONT_RT_DEFAULT);
#else
        fontbutton_rt->set_font_name(CtConst::FONT_RT_DEFAULT);
#endif
        f_on_font_rt_set();
    });
    button_reset_pt->signal_clicked().connect([fontbutton_pt, f_on_font_pt_set](){
#if GTKMM_MAJOR_VERSION >= 4
        fontbutton_pt->set_font(CtConst::FONT_PT_DEFAULT);
#else
        fontbutton_pt->set_font_name(CtConst::FONT_PT_DEFAULT);
#endif
        f_on_font_pt_set();
    });
    button_reset_tree->signal_clicked().connect([fontbutton_tree, f_on_font_tree_set](){
#if GTKMM_MAJOR_VERSION >= 4
        fontbutton_tree->set_font(CtConst::FONT_TREE_DEFAULT);
#else
        fontbutton_tree->set_font_name(CtConst::FONT_TREE_DEFAULT);
#endif
        f_on_font_tree_set();
    });
    button_reset_code->signal_clicked().connect([fontbutton_code, f_on_font_code_set](){
#if GTKMM_MAJOR_VERSION >= 4
        fontbutton_code->set_font(CtConst::FONT_CODE_DEFAULT);
#else
        fontbutton_code->set_font_name(CtConst::FONT_CODE_DEFAULT);
#endif
        f_on_font_code_set();
    });
#if defined(HAVE_VTE)
    button_reset_vte->signal_clicked().connect([fontbutton_vte, f_on_font_vte_set](){
#if GTKMM_MAJOR_VERSION >= 4
        fontbutton_vte->set_font(CtConst::FONT_VTE_DEFAULT);
#else
        fontbutton_vte->set_font_name(CtConst::FONT_VTE_DEFAULT);
#endif
        f_on_font_vte_set();
    });
#endif // HAVE_VTE
    button_reset_ms->signal_clicked().connect([fontbutton_ms, f_on_font_ms_set](){
#if GTKMM_MAJOR_VERSION >= 4
        fontbutton_ms->set_font(CtConst::FONT_MS_DEFAULT);
#else
        fontbutton_ms->set_font_name(CtConst::FONT_MS_DEFAULT);
#endif
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
    spinbutton_nodes_on_node_name_header->signal_value_changed().connect([this, spinbutton_nodes_on_node_name_header](){
        _pConfig->nodesOnNodeNameHeader = spinbutton_nodes_on_node_name_header->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) { win->window_header_update(); });
    });
    spinbutton_scrollbar_min_size->signal_value_changed().connect([this, spinbutton_scrollbar_min_size](){
        _pConfig->scrollSliderMin = spinbutton_scrollbar_min_size->get_value_as_int();
        apply_for_each_window([](CtMainWin* win) { win->update_theme(); });
    });
    radiobutton_scrollbar_overlay_default->signal_toggled().connect([this, pRadiobutton_scrollbar_overlay_default=radiobutton_scrollbar_overlay_default](){
        if (not pRadiobutton_scrollbar_overlay_default->get_active()) return;
        _pConfig->overlayScroll = 2;
        need_restart(RESTART_REASON::OVERLAY_SCROLL);
    });
    radiobutton_scrollbar_overlay_on->signal_toggled().connect([this, pRadiobutton_scrollbar_overlay_on=radiobutton_scrollbar_overlay_on](){
        if (not pRadiobutton_scrollbar_overlay_on->get_active()) return;
        _pConfig->overlayScroll = 1;
        _pCtMainWin->getScrolledwindowText().set_overlay_scrolling(static_cast<bool>(_pConfig->overlayScroll));
    });
    radiobutton_scrollbar_overlay_off->signal_toggled().connect([this, pRadiobutton_scrollbar_overlay_off=radiobutton_scrollbar_overlay_off](){
        if (not pRadiobutton_scrollbar_overlay_off->get_active()) return;
        _pConfig->overlayScroll = 0;
        _pCtMainWin->getScrolledwindowText().set_overlay_scrolling(static_cast<bool>(_pConfig->overlayScroll));
    });
    checkbutton_tooltips_enable_tree->signal_toggled().connect([this, checkbutton_tooltips_enable_tree](){
        _pConfig->treeTooltips = checkbutton_tooltips_enable_tree->get_active();
        apply_for_each_window([this](CtMainWin* win) { win->get_tree_view().set_tooltips_enable(_pConfig->treeTooltips/*on*/); });
    });
    checkbutton_tooltips_enable_menus->signal_toggled().connect([this, checkbutton_tooltips_enable_menus](){
        _pConfig->menusTooltips = checkbutton_tooltips_enable_menus->get_active();
        need_restart(RESTART_REASON::MENUS_TOOLTIPS);
    });
    checkbutton_tooltips_enable_toolbar->signal_toggled().connect([this, checkbutton_tooltips_enable_toolbar](){
        _pConfig->toolbarTooltips = checkbutton_tooltips_enable_toolbar->get_active();
        _pCtMainWin->emit_app_apply_for_each_window([](CtMainWin* win) { win->menu_rebuild_toolbars(true/*new_toolbar*/); });
    });
    spinbutton_find_all_max_in_page->signal_value_changed().connect([this, spinbutton_find_all_max_in_page](){
        _pConfig->maxMatchesInPage = spinbutton_find_all_max_in_page->get_value_as_int();
        _pCtMainWin->get_ct_actions()->find_matches_store_reset();
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
#if GTKMM_MAJOR_VERSION >= 4
    vbox_links_actions->append(*checkbutton_custom_weblink_cmd);
    vbox_links_actions->append(*entry_custom_weblink_cmd);
    vbox_links_actions->append(*checkbutton_custom_filelink_cmd);
    vbox_links_actions->append(*entry_custom_filelink_cmd);
    vbox_links_actions->append(*checkbutton_custom_folderlink_cmd);
    vbox_links_actions->append(*entry_custom_folderlink_cmd);
#else
    vbox_links_actions->pack_start(*checkbutton_custom_weblink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_weblink_cmd, false, false);
    vbox_links_actions->pack_start(*checkbutton_custom_filelink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_filelink_cmd, false, false);
    vbox_links_actions->pack_start(*checkbutton_custom_folderlink_cmd, false, false);
    vbox_links_actions->pack_start(*entry_custom_folderlink_cmd, false, false);
#endif

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
    auto checkbutton_links_doubleclick = Gtk::manage(new Gtk::CheckButton{_("Double Click for Link Action")});
    checkbutton_links_doubleclick->set_active(_pConfig->doubleClickLink);
    auto checkbutton_links_underline = Gtk::manage(new Gtk::CheckButton{_("Underline Links")});
    checkbutton_links_underline->set_active(_pConfig->linksUnderline);
    auto checkbutton_links_relative = Gtk::manage(new Gtk::CheckButton{_("Use Relative Paths for Files And Folders")});
    checkbutton_links_relative->set_active(_pConfig->linksRelative);
    auto hbox_anchor_size = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_HORIZONTAL, 4/*spacing*/});
    auto label_anchor_size = Gtk::manage(new Gtk::Label{_("Anchor Size")});
    Glib::RefPtr<Gtk::Adjustment> adj_anchor_size = Gtk::Adjustment::create(_pConfig->anchorSize, 1, 1000, 1);
    auto spinbutton_anchor_size = Gtk::manage(new Gtk::SpinButton{adj_anchor_size});
    spinbutton_anchor_size->set_value(_pConfig->anchorSize);
#if GTKMM_MAJOR_VERSION >= 4
    hbox_anchor_size->append(*label_anchor_size);
    hbox_anchor_size->append(*spinbutton_anchor_size);
    vbox_links_misc->append(*checkbutton_links_doubleclick);
    vbox_links_misc->append(*checkbutton_links_underline);
    vbox_links_misc->append(*checkbutton_links_relative);
    vbox_links_misc->append(*hbox_anchor_size);
#else
    hbox_anchor_size->pack_start(*label_anchor_size, false, false);
    hbox_anchor_size->pack_start(*spinbutton_anchor_size, false, false);
    vbox_links_misc->pack_start(*checkbutton_links_doubleclick, false, false);
    vbox_links_misc->pack_start(*checkbutton_links_underline, false, false);
    vbox_links_misc->pack_start(*checkbutton_links_relative, false, false);
    vbox_links_misc->pack_start(*hbox_anchor_size, false, false);
#endif

    Gtk::Frame* frame_links_misc = new_managed_frame_with_align(_("Miscellaneous"), vbox_links_misc);

    auto pMainBox = Gtk::manage(new Gtk::Box{Gtk::ORIENTATION_VERTICAL, 3/*spacing*/});
    pMainBox->set_margin_start(6);
    pMainBox->set_margin_top(6);
#if GTKMM_MAJOR_VERSION >= 4
    pMainBox->append(*frame_links_actions);
    pMainBox->append(*frame_links_colors);
    pMainBox->append(*frame_links_misc);
#else
    pMainBox->pack_start(*frame_links_actions, false, false);
    pMainBox->pack_start(*frame_links_colors, false, false);
    pMainBox->pack_start(*frame_links_misc, false, false);
#endif

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
    checkbutton_links_doubleclick->signal_toggled().connect([this, checkbutton_links_doubleclick](){
        _pConfig->doubleClickLink = checkbutton_links_doubleclick->get_active();
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
    _pCtMainWin->emit_app_apply_for_each_window(callback);
}
