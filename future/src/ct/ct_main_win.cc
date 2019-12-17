/*
 * ct_main_win.cc
 *
 * Copyright 2017-2019 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_app.h"
#include "ct_p7za_iface.h"
#include "ct_clipboard.h"
#include <glib-object.h>


CtMainWin::CtMainWin(CtMenu* pCtMenu)
 : Gtk::ApplicationWindow(),
   _pCtMenu(pCtMenu)
{
    set_icon(CtApp::R_icontheme->load_icon(CtConst::APP_NAME, 48));
    _scrolledwindowTree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowText.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowTree.add(_ctTreeview);
    _scrolledwindowText.add(_ctTextview);
    _vboxText.pack_start(_init_window_header(), false, false);
    _vboxText.pack_start(_scrolledwindowText);
    if (CtApp::P_ctCfg->treeRightSide)
    {
        _hPaned.add1(_vboxText);
        _hPaned.add2(_scrolledwindowTree);
    }
    else
    {
        _hPaned.add1(_scrolledwindowTree);
        _hPaned.add2(_vboxText);
    }

    _pMenuBar = pCtMenu->build_menubar();
    _pMenuBar->set_name("MenuBar");
    _pBookmarksSubmenu = CtMenu::find_menu_item(_pMenuBar, "BookmarksMenu");
    _pSpecialCharsSubmenu = CtMenu::find_menu_item(_pMenuBar, "SpecialCharsMenu");
    _pMenuBar->show_all();
    gtk_window_add_accel_group (GTK_WINDOW(gobj()), pCtMenu->default_accel_group());
    _pToolbar = pCtMenu->build_toolbar();

    _vboxMain.pack_start(*_pMenuBar, false, false);
    _vboxMain.pack_start(*_pToolbar, false, false);
    _vboxMain.pack_start(_hPaned);
    _vboxMain.pack_start(_init_status_bar(), false, false);
    add(_vboxMain);

    _reset_CtTreestore();
    _uCtTreestore->view_append_columns(&_ctTreeview);

    _ctTreeview.signal_cursor_changed().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_cursor_changed));
    _ctTreeview.signal_button_release_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_button_release_event));
    _ctTreeview.signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_key_press_event), false);
    _ctTreeview.signal_popup_menu().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_popup_menu));

    _ctTreeview.get_style_context()->add_class("ct_node_view");
    _ctTextview.get_style_context()->add_class("ct_textview");

    _ctTextview.signal_populate_popup().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_populate_popup));
    _ctTextview.signal_motion_notify_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_motion_notify_event));
    _ctTextview.signal_visibility_notify_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_visibility_notify_event));
    _ctTextview.signal_size_allocate().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_size_allocate));
    _ctTextview.signal_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_event));
    _ctTextview.signal_event_after().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_event_after));
    _ctTextview.signal_scroll_event().connect([this](GdkEventScroll* event){
        if (event->state & GDK_CONTROL_MASK && (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN))
        {
            _ctTextview.zoom_text(event->direction == GDK_SCROLL_UP);
            return true;
        }
        return false;
    });
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "cut-clipboard", G_CALLBACK(CtClipboard::on_cut_clipboard), nullptr /*from_codebox*/);
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "copy-clipboard", G_CALLBACK(CtClipboard::on_copy_clipboard), nullptr /*from_codebox*/);
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "paste-clipboard", G_CALLBACK(CtClipboard::on_paste_clipboard), nullptr /*from_codebox*/);

    signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_window_key_press_event), false);

    _title_update(false/*saveNeeded*/);

    config_apply_before_show_all();
    show_all();
    config_apply_after_show_all();

    set_menu_items_special_chars();
}

CtMainWin::~CtMainWin()
{
    //printf("~CtMainWin\n");
}

void CtMainWin::_reset_CtTreestore()
{
    _prevTreeIter = CtTreeIter();
    _uCtTreestore.reset(new CtTreeStore);
    _uCtTreestore->view_connect(&_ctTreeview);
}

void CtMainWin::config_apply_before_show_all()
{
    move(CtApp::P_ctCfg->winRect[0], CtApp::P_ctCfg->winRect[1]);
    if (CtApp::P_ctCfg->winIsMaximised)
    {
        maximize();
    }
    else
    {
        set_default_size(CtApp::P_ctCfg->winRect[2], CtApp::P_ctCfg->winRect[3]);
    }
    _hPaned.property_position() = CtApp::P_ctCfg->hpanedPos;
}

void CtMainWin::config_apply_after_show_all()
{
    show_hide_tree_view(CtApp::P_ctCfg->treeVisible);
    show_hide_win_header(CtApp::P_ctCfg->showNodeNameHeader);

    show_hide_toolbar(CtApp::P_ctCfg->toolbarVisible);
    _pToolbar->set_toolbar_style(Gtk::ToolbarStyle::TOOLBAR_ICONS);
    set_toolbar_icon_size(CtApp::P_ctCfg->toolbarIconSize);

    _ctStatusBar.progressBar.hide();
    _ctStatusBar.stopButton.hide();

    configure_theme();
}

void CtMainWin::configure_theme()
{
    auto font_to_string = [](Pango::FontDescription font)
    {
        return " { font-family: " + font.get_family() +
               "; font-size: " + std::to_string(font.get_size()/Pango::SCALE) +
               "pt; } ";
    };

    std::string rtFont = font_to_string(Pango::FontDescription(CtApp::P_ctCfg->rtFont));
    std::string plFont = font_to_string(Pango::FontDescription(CtApp::P_ctCfg->ptFont));
    std::string codeFont = font_to_string(Pango::FontDescription(CtApp::P_ctCfg->codeFont));
    std::string treeFont = font_to_string(Pango::FontDescription(CtApp::P_ctCfg->treeFont));

    auto screen = get_screen();
    if (_css_provider_theme_font)
    {
        Gtk::StyleContext::remove_provider_for_screen(screen, _css_provider_theme_font);
    }
    _css_provider_theme_font = Gtk::CssProvider::create();

    std::string font_css;
    font_css += ".ct_textview.rich-text" + rtFont;
    font_css += ".ct_textview.plain-text" + plFont;
    font_css += ".ct_textview.code" + codeFont;
    font_css += ".codebox.rich-text" + rtFont;
    font_css += ".codebox.plain-text" + codeFont;
    font_css += ".codebox.code" + codeFont;
    font_css += ".ct_node_view" + treeFont;

    _css_provider_theme_font->load_from_data(font_css);
    get_style_context()->add_provider_for_screen(screen, _css_provider_theme_font, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

#ifdef THEME_FIXES_REQUIRED
    // can do more complicated things than just changing colors
    if (_css_provider_theme)
    {
        Gtk::StyleContext::remove_provider_for_screen(screen, _css_provider_theme);
    }
    _css_provider_theme = Gtk::CssProvider::create();
    std::string theme_css;
    // todo: rich text fix light selected line with dark theme
    theme_css += ".ct_textview.rich-text > text { color: " + CtApp::P_ctCfg->rtDefFg + "; background-color: " + CtApp::P_ctCfg->rtDefBg + "; } ";
    // todo: tree selected node highlight no longer working
    theme_css += ".ct_node_view { color: " + CtApp::P_ctCfg->ttDefFg + "; background-color: " + CtApp::P_ctCfg->ttDefBg + "; } ";
    theme_css += ".ct_header { background-color: " + CtApp::P_ctCfg->ttDefBg + "; } ";

    _css_provider_theme->load_from_data(theme_css);
    get_style_context()->add_provider_for_screen(screen, _css_provider_theme, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#endif // THEME_FIXES_REQUIRED
}

Gtk::HBox& CtMainWin::_init_status_bar()
{
    _ctStatusBar.statusId = _ctStatusBar.statusBar.get_context_id("");
    _ctStatusBar.frame.set_shadow_type(Gtk::SHADOW_NONE);
    _ctStatusBar.frame.set_border_width(1);
    _ctStatusBar.frame.add(_ctStatusBar.progressBar);
    _ctStatusBar.stopButton.set_image_from_icon_name("gtk-stop", Gtk::ICON_SIZE_MENU);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.statusBar, true, true);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.frame, false, true);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.stopButton, false, true);
    _ctStatusBar.stopButton.signal_clicked().connect([this](){
        _ctStatusBar.set_progress_stop(true);
        _ctStatusBar.stopButton.hide();
    });
    _ctStatusBar.set_progress_stop(false);
    return _ctStatusBar.hbox;
}

Gtk::EventBox& CtMainWin::_init_window_header()
{
    _ctWinHeader.nameLabel.set_padding(10, 0);
    _ctWinHeader.nameLabel.set_ellipsize(Pango::EllipsizeMode::ELLIPSIZE_MIDDLE);
    _ctWinHeader.lockIcon.set_from_icon_name("locked", Gtk::ICON_SIZE_MENU);
    _ctWinHeader.lockIcon.hide();
    _ctWinHeader.bookmarkIcon.set_from_icon_name("pin", Gtk::ICON_SIZE_MENU);
    _ctWinHeader.bookmarkIcon.hide();
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.buttonBox, false, false);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.nameLabel, true, true);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.lockIcon, false, false);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.bookmarkIcon, false, false);
    _ctWinHeader.eventBox.add(_ctWinHeader.headerBox);
    _ctWinHeader.eventBox.get_style_context()->add_class("ct_header");
    return _ctWinHeader.eventBox;
}

void CtMainWin::window_header_update()
{
    // based on update_node_name_header
    std::string name = curr_tree_iter().get_node_name();
    std::string foreground = curr_tree_iter().get_node_foreground();
    foreground = foreground.empty() ? CtApp::P_ctCfg->ttDefFg : foreground;
    _ctWinHeader.nameLabel.set_markup(
                "<b><span foreground=\"" + foreground + "\" size=\"xx-large\">"
                + str::xml_escape(name) + "</span></b>");
    window_header_update_last_visited();
}

void CtMainWin::window_header_update_lock_icon(bool show)
{
    show ? _ctWinHeader.lockIcon.show() : _ctWinHeader.lockIcon.hide();
}

void CtMainWin::window_header_update_bookmark_icon(bool show)
{
    show ? _ctWinHeader.bookmarkIcon.show() : _ctWinHeader.bookmarkIcon.hide();
}

void CtMainWin::window_header_update_last_visited()
{
    // todo: update_node_name_header_latest_visited
}

void CtMainWin::window_header_update_num_last_visited()
{
    // todo: update_node_name_header_num_latest_visited
}

void CtMainWin::menu_tree_update_for_bookmarked_node(bool is_bookmarked)
{
    _pCtMenu->find_action("node_bookmark")->signal_set_visible.emit(!is_bookmarked);
    _pCtMenu->find_action("node_unbookmark")->signal_set_visible.emit(is_bookmarked);
}

void CtMainWin::bookmark_action_select_node(gint64 node_id)
{
    Gtk::TreeIter tree_iter = _uCtTreestore->get_node_from_node_id(node_id);
    get_tree_view().set_cursor_safe(tree_iter);
}

void CtMainWin::set_bookmarks_menu_items()
{
    std::list<std::tuple<gint64, std::string>> bookmarks;
    for (const gint64& node_id: _uCtTreestore->get_bookmarks())
        bookmarks.push_back(std::make_tuple(node_id, _uCtTreestore->get_node_name_from_node_id(node_id)));
    sigc::slot<void, gint64> bookmark_action = sigc::mem_fun(*this, &CtMainWin::bookmark_action_select_node);
    _pBookmarksSubmenu->set_submenu(*_pCtMenu->build_bookmarks_menu(bookmarks, bookmark_action));
}

void CtMainWin::set_menu_items_special_chars()
{
    sigc::slot<void, gunichar> spec_char_action = sigc::mem_fun(*CtApp::P_ctActions, &CtActions::insert_spec_char_action);
    _pSpecialCharsSubmenu->set_submenu(*_pCtMenu->build_special_chars_menu(CtApp::P_ctCfg->specialChars, spec_char_action));
}

void CtMainWin::filepath_open(const std::string& filepath, const bool force_reset)
{
    CtRecentDocRestore prevDocRestore;
    prevDocRestore.doc_name = get_curr_doc_file_name();
    prevDocRestore.exp_coll_str = _uCtTreestore->get_tree_expanded_collapsed_string(_ctTreeview);
    const CtTreeIter prevTreeIter = curr_tree_iter();
    if (prevTreeIter)
    {
        prevDocRestore.node_path = _uCtTreestore->get_path(prevTreeIter).to_string();
        const Glib::RefPtr<Gsv::Buffer> rTextBuffer = prevTreeIter.get_node_text_buffer();
        prevDocRestore.cursor_pos = rTextBuffer->property_cursor_position();
    }
    if (not reset(force_reset))
    {
        return;
    }
    const std::string new_file_name = Glib::path_get_basename(filepath);
    if (new_file_name != prevDocRestore.doc_name)
    {
        int i{0};
        for (; i<CtConst::MAX_RECENT_DOCS_RESTORE; i++)
        {
            if (new_file_name == CtApp::P_ctCfg->recentDocsRestore[i].doc_name)
            {
                CtApp::P_ctCfg->recentDocsRestore[i] = prevDocRestore;
                break;
            }
        }
        if (CtConst::MAX_RECENT_DOCS_RESTORE == i)
        {
            for (i=0; i<CtConst::MAX_RECENT_DOCS_RESTORE; i++)
            {
                if (CtApp::P_ctCfg->recentDocsRestore[i].doc_name.empty())
                {
                    CtApp::P_ctCfg->recentDocsRestore[i] = prevDocRestore;
                    break;
                }
            }
            if (CtConst::MAX_RECENT_DOCS_RESTORE == i)
            {
                for (i=CtConst::MAX_RECENT_DOCS_RESTORE-1; i>=0; i--)
                {
                    if (i > 0)
                    {
                        CtApp::P_ctCfg->recentDocsRestore[i] = CtApp::P_ctCfg->recentDocsRestore[i-1];
                    }
                    else
                    {
                        CtApp::P_ctCfg->recentDocsRestore[i] = prevDocRestore;
                    }
                }
            }
        }
    }
    //todo
}

bool CtMainWin::reset(const bool force_reset)
{
    if (not force_reset and
        _uCtTreestore->get_iter_first() and
        not check_unsaved())
    {
        return false;
    }
    curr_file_mod_time_update_value(false/*doEnable*/);

    auto on_scope_exit = scope_guard([&](void*) { user_active() = true; });
    user_active() = false;

    _reset_CtTreestore();
    _latestStatusbarUpdateTime.clear();
    _set_new_curr_doc(Glib::RefPtr<Gio::File>{nullptr}, "");

    update_window_save_not_needed();
    //todo
    //self.state_machine.reset()
}

bool CtMainWin::check_unsaved()
{
    if (get_file_save_needed())
    {
        const CtYesNoCancel yesNoCancel = CtApp::P_ctCfg->autosaveOnQuit ? CtYesNoCancel::Yes : CtDialogs::exit_save_dialog(*this);
        if (CtYesNoCancel::Cancel == yesNoCancel)
        {
            return false;
        }
        if (CtYesNoCancel::Yes == yesNoCancel)
        {
            CtApp::P_ctActions->file_save();
            if (get_file_save_needed())
            {
                // something went wrong in the save
                return false;
            }
        }
    }
    return true;
}

void CtMainWin::_set_new_curr_doc(const Glib::RefPtr<Gio::File>& r_file, const std::string& password)
{
    _ctCurrFile.rFile = r_file;
    _ctCurrFile.password = password;
}

void CtMainWin::set_new_curr_doc(const std::string& filepath,
                                 const std::string& password,
                                 CtSQLite* const pCtSQLite)
{
    Glib::RefPtr<Gio::File> r_file = Gio::File::create_for_path(filepath);
    _set_new_curr_doc(r_file, password);
    _uCtTreestore->set_new_curr_sqlite_doc(pCtSQLite);
}

bool CtMainWin::read_nodes_from_gio_file(const Glib::RefPtr<Gio::File>& r_file, const bool isImport)
{
    bool retOk{false};
    const std::string filepath{r_file->get_path()};
    const CtDocEncrypt docEncrypt = CtMiscUtil::get_doc_encrypt(filepath);
    const gchar* pFilepath{nullptr};
    std::string password;
    if (CtDocEncrypt::True == docEncrypt)
    {
        g_autofree gchar* title = g_strdup_printf(_("Enter Password for %s"), Glib::path_get_basename(filepath).c_str());
        while (true)
        {
            CtDialogTextEntry dialogTextEntry(title, true/*forPassword*/, this);
            int response = dialogTextEntry.run();
            if (Gtk::RESPONSE_OK != response)
            {
                break;
            }
            password = dialogTextEntry.get_entry_text();
            if (0 == CtP7zaIface::p7za_extract(filepath.c_str(),
                                               CtApp::P_ctTmp->getHiddenDirPath(filepath),
                                               password.c_str()) &&
                g_file_test(CtApp::P_ctTmp->getHiddenFilePath(filepath), G_FILE_TEST_IS_REGULAR))
            {
                pFilepath = CtApp::P_ctTmp->getHiddenFilePath(filepath);
                break;
            }
        }
    }
    else if (CtDocEncrypt::False == docEncrypt)
    {
        pFilepath = filepath.c_str();
    }
    if (pFilepath)
    {
        retOk = _uCtTreestore->read_nodes_from_filepath(pFilepath, isImport);
    }
    if (retOk && not isImport)
    {
        _set_new_curr_doc(r_file, password);
        _title_update(false/*saveNeeded*/);
        set_bookmarks_menu_items();

        if ((get_curr_doc_file_name() == CtApp::P_ctCfg->fileName) &&
            (get_curr_doc_file_dir() == CtApp::P_ctCfg->fileDir))
        {
            if (CtRestoreExpColl::ALL_EXP == CtApp::P_ctCfg->restoreExpColl)
            {
                _ctTreeview.expand_all();
            }
            else
            {
                if (CtRestoreExpColl::ALL_COLL == CtApp::P_ctCfg->restoreExpColl)
                {
                    CtApp::P_ctCfg->expandedCollapsedString = "";
                }
                _uCtTreestore->set_tree_expanded_collapsed_string(CtApp::P_ctCfg->expandedCollapsedString,
                                                                _ctTreeview,
                                                                CtApp::P_ctCfg->nodesBookmExp);
            }
            _uCtTreestore->set_tree_path_n_text_cursor_from_config(&_ctTreeview, &_ctTextview);
        }
    }
    return retOk;
}

bool CtMainWin::get_file_save_needed()
{
    return (_fileSaveNeeded || (curr_tree_iter() && curr_tree_iter().get_node_text_buffer()->get_modified()));
}

void CtMainWin::curr_file_mod_time_update_value(const bool doEnable)
{
    if (doEnable && _ctCurrFile.rFile->query_exists())
    {
        Glib::RefPtr<Gio::FileInfo> rFileInfo = _ctCurrFile.rFile->query_info();
        if (rFileInfo)
        {
            Glib::TimeVal timeVal = rFileInfo->modification_time();
            if (timeVal.valid())
            {
                _ctCurrFile.modTime = timeVal.as_double();
            }
        }
    }
    else
    {
        _ctCurrFile.modTime = 0;
    }
}

void CtMainWin::update_selected_node_statusbar_info()
{
    CtTreeIter treeIter = curr_tree_iter();
    Glib::ustring statusbar_text;
    if (not treeIter)
    {
        statusbar_text = _("No Node is Selected");
    }
    else
    {
        const std::string separator_text{"  -  "};
        statusbar_text = Glib::ustring{_("Node Type")} + _(": ");
        const std::string syntaxHighl = treeIter.get_node_syntax_highlighting();
        if (CtConst::RICH_TEXT_ID == syntaxHighl)
        {
            statusbar_text += _("Rich Text");
        }
        else if (CtConst::PLAIN_TEXT_ID == syntaxHighl)
        {
            statusbar_text += _("Plain Text");
        }
        else
        {
            statusbar_text += syntaxHighl;
        }
        if (not treeIter.get_node_tags().empty())
        {
            statusbar_text += separator_text + _("Tags") + _(": ") + treeIter.get_node_tags();
        }
        // todo spellCheck
        //if self.enable_spell_check and self.syntax_highlighting == cons.RICH_TEXT_ID:
        //    statusbar_text += separator_text + _("Spell Check") + _(": ") + self.spell_check_lang
        // todo wordCount
        //if self.word_count:
        //    statusbar_text += separator_text + _("Word Count") + _(": ") + str(support.get_word_count(self))
        if (treeIter.get_node_creating_time() > 0)
        {
            const std::string timestamp_creation = str::time_format(CtApp::P_ctCfg->timestampFormat, treeIter.get_node_creating_time());
            statusbar_text += separator_text + _("Date Created") + _(": ") + timestamp_creation;
        }
        if (treeIter.get_node_modification_time() > 0)
        {
            const std::string timestamp_lastsave = str::time_format(CtApp::P_ctCfg->timestampFormat, treeIter.get_node_modification_time());
            statusbar_text += separator_text + _("Date Modified") + _(": ") + timestamp_lastsave;
        }
    }
    _ctStatusBar.update_status(statusbar_text);
}

void CtMainWin::update_window_save_not_needed()
{
    _title_update(false/*save_needed*/);
    _fileSaveNeeded = false;
    CtTreeIter treeIter = curr_tree_iter();
    if (treeIter)
    {
        Glib::RefPtr<Gsv::Buffer> rTextBuffer = treeIter.get_node_text_buffer();
        rTextBuffer->set_modified(false);
        std::list<CtAnchoredWidget*> anchoredWidgets = treeIter.get_all_embedded_widgets();
        for (CtAnchoredWidget* pAnchoredWidget : anchoredWidgets)
        {
            pAnchoredWidget->set_modified_false();
        }
    }
}

void CtMainWin::update_window_save_needed(const CtSaveNeededUpdType update_type,
                                          const bool new_machine_state,
                                          const CtTreeIter* give_tree_iter)
{
    CtTreeIter treeIter = (nullptr != give_tree_iter ? *give_tree_iter : curr_tree_iter());
    if (treeIter.get_node_is_rich_text())
    {
        treeIter.get_node_text_buffer()->set_modified(true); // support possible change inside anchored widget which doesn't toggle modified flag
    }
    if (false == _fileSaveNeeded)
    {
        _title_update(true/*save_needed*/);
        _fileSaveNeeded = true;
    }
    switch (update_type)
    {
        case CtSaveNeededUpdType::None:
            break;
        case CtSaveNeededUpdType::nbuf:
        {
            treeIter.pending_edit_db_node_buff();
            g_autoptr(GDateTime) pGDateTime = g_date_time_new_now_local();
            const gint64 curr_time = g_date_time_to_unix(pGDateTime);
            treeIter.set_node_modification_time(curr_time);
            const gint64 node_id = treeIter.get_node_id();
            if ( (0 == _latestStatusbarUpdateTime.count(node_id)) ||
                 (curr_time - _latestStatusbarUpdateTime.at(node_id) > 60) )
            {
                _latestStatusbarUpdateTime[node_id] = curr_time;
                update_selected_node_statusbar_info();
            }
        } break;
        case CtSaveNeededUpdType::npro:
        {
            treeIter.pending_edit_db_node_prop();
        } break;
        case CtSaveNeededUpdType::ndel:
        {
            const gint64 top_node_id = treeIter.get_node_id();
            std::vector<gint64> rm_node_ids = treeIter.get_children_node_ids();
            rm_node_ids.push_back(top_node_id);
            _uCtTreestore->pending_rm_db_nodes(rm_node_ids);
            //todo
            //self.state_machine.delete_states(node_id) 
        } break;
        case CtSaveNeededUpdType::book:
        {
            _uCtTreestore->pending_edit_db_bookmarks();
        } break;
    }
    //todo
    //if new_state_machine and tree_iter: self.state_machine.update_state()
}

void CtMainWin::_on_treeview_cursor_changed()
{
    if (_prevTreeIter)
    {
        Glib::RefPtr<Gsv::Buffer> rTextBuffer = _prevTreeIter.get_node_text_buffer();
        if (rTextBuffer->get_modified())
        {
            _fileSaveNeeded = true;
            rTextBuffer->set_modified(false);
            //self.state_machine.update_state()
        }
    }
    CtTreeIter treeIter = curr_tree_iter();
    _uCtTreestore->apply_textbuffer_to_textview(treeIter, &_ctTextview);

    menu_tree_update_for_bookmarked_node(_uCtTreestore->is_node_bookmarked(treeIter.get_node_id()));
    window_header_update();
    window_header_update_lock_icon(treeIter.get_node_read_only());
    window_header_update_bookmark_icon(false);
    update_selected_node_statusbar_info();

    _prevTreeIter = treeIter;
}

bool CtMainWin::_on_treeview_button_release_event(GdkEventButton* event)
{
    if (event->button == 3)
    {
        _pCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(event->button, event->time);
        return true;
    }
    return false;
}

bool CtMainWin::_on_window_key_press_event(GdkEventKey* event)
{
    // todo:
    if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_Tab) {
            CtApp::P_ctActions->toggle_tree_text();
            return true;
        }
    }
    return false;
}

bool CtMainWin::_on_treeview_key_press_event(GdkEventKey* event)
{
    // todo:
    if (!curr_tree_iter()) return false;
    if (event->state & GDK_SHIFT_MASK) {
        if (event->keyval == GDK_KEY_Up) {
            CtApp::P_ctActions->node_up();
            return true;
        }
        if (event->keyval == GDK_KEY_Down) {
            CtApp::P_ctActions->node_down();
            return true;
        }
    }
    else if (event->state & GDK_MOD1_MASK) {

    }
    else if (event->state & GDK_CONTROL_MASK) {

    }
    else {
        if (event->keyval == GDK_KEY_Tab) {
            CtApp::P_ctActions->toggle_tree_text();
            return true;
        }
    }
    return false;
}

bool CtMainWin::_on_treeview_popup_menu()
{
    _pCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(0, 0);
    return true;
}

// Extend the Default Right-Click Menu
void CtMainWin::_on_textview_populate_popup(Gtk::Menu* menu)
{
    if (curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID)
    {
        // todo:
        /*for (auto menuitem: menu->get_children())
            if (menu->)
            try:
                if menuitem.get_image().get_property("stock") == "gtk-paste":
                    menuitem.set_sensitive(True)
            except: pass
        */
        if (hovering_link_iter_offset() >= 0)
        {
            Gtk::TextIter target_iter = curr_buffer()->get_iter_at_offset(hovering_link_iter_offset());
            if (target_iter)
            {
                bool do_set_cursor = true;
                if (curr_buffer()->get_has_selection())
                {
                    Gtk::TextIter iter_sel_start, iter_sel_end;
                    curr_buffer()->get_selection_bounds(iter_sel_start, iter_sel_end);
                    if (hovering_link_iter_offset() >= iter_sel_start.get_offset()
                        && hovering_link_iter_offset() <= iter_sel_end.get_offset())
                    {
                        do_set_cursor = false;
                    }
                }
                if (do_set_cursor) curr_buffer()->place_cursor(target_iter);
            }
            CtApp::P_ctActions->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::Link);
        }
        else
            CtApp::P_ctActions->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::Text);
    }
    else
        CtApp::P_ctActions->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::Code);
}
// Update the cursor image if the pointer moved
bool CtMainWin::_on_textview_motion_notify_event(GdkEventMotion* event)
{
    if (!_ctTextview.get_cursor_visible())
        _ctTextview.set_cursor_visible(true);
    if (curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID
        && curr_tree_iter().get_node_syntax_highlighting() != CtConst::PLAIN_TEXT_ID)
    {
        // todo: it's ok to create cursor every time?
        get_text_view().get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::XTERM));
        return false;
    }
    int x, y;
    get_text_view().window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, (int)event->x, (int)event->y, x, y);
    get_text_view().cursor_and_tooltips_handler(x, y);
    return false;
}

// Update the cursor image if the window becomes visible (e.g. when a window covering it got iconified)
bool CtMainWin::_on_textview_visibility_notify_event(GdkEventVisibility*)
{
    if (curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID &&
            curr_tree_iter().get_node_syntax_highlighting() != CtConst::PLAIN_TEXT_ID)
    {
        get_text_view().get_window(Gtk::TEXT_WINDOW_TEXT)->set_cursor(Gdk::Cursor::create(Gdk::XTERM));
        return false;
    }
    int x,y, bx, by;
    Gdk::ModifierType mask;
    get_text_view().get_window(Gtk::TEXT_WINDOW_TEXT)->get_pointer(x, y, mask);
    get_text_view().window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, x, y, bx, by);
    get_text_view().cursor_and_tooltips_handler(bx, by);
    return false;
}

void CtMainWin::_on_textview_size_allocate(Gtk::Allocation& allocation)
{
    if (_prevTextviewWidth == 0)
        _prevTextviewWidth = allocation.get_width();
    else if (_prevTextviewWidth != allocation.get_width())
    {
        _prevTextviewWidth = allocation.get_width();
        auto widgets = curr_tree_iter().get_all_embedded_widgets();
        for (auto& widget: widgets)
            if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widget))
                if (!codebox->get_width_in_pixels())
                    codebox->apply_width_height(allocation.get_width());
    }
}

bool CtMainWin::_on_textview_event(GdkEvent* event)
{
    if (event->type != GDK_KEY_PRESS)
        return false;

    auto curr_buffer = get_text_view().get_buffer();
    if (event->key.state & Gdk::SHIFT_MASK)
     {
        if (event->key.keyval == GDK_KEY_ISO_Left_Tab && !curr_buffer->get_has_selection())
        {
            auto iter_insert = curr_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(curr_buffer).get_paragraph_list_info(iter_insert);
            if (list_info && list_info.level)
            {
                get_text_view().list_change_level(iter_insert, list_info, false);
                return true;
            }
        }
    }
    else if (event->key.state & Gdk::CONTROL_MASK && event->key.keyval == GDK_KEY_space)
    {
        auto iter_insert = curr_buffer->get_insert()->get_iter();
        auto widgets = curr_tree_iter().get_embedded_pixbufs_tables_codeboxes({iter_insert.get_offset(), iter_insert.get_offset()});
        if (!widgets.empty())
            if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widgets.front()))
            {
                codebox->get_text_view().grab_focus();
                return true;
            }
        CtListInfo list_info = CtList(curr_buffer).get_paragraph_list_info(iter_insert);
        if (list_info && list_info.type == CtListType::Todo)
            if (CtApp::P_ctActions->_is_curr_node_not_read_only_or_error())
            {
                auto iter_start_list = curr_buffer->get_iter_at_offset(list_info.startoffs + 3*list_info.level);
                CtList(curr_buffer).todo_list_rotate_status(iter_start_list);
                return true;
            }
    }
    else if (event->key.keyval == GDK_KEY_Return)
    {
        auto iter_insert = curr_buffer->get_insert()->get_iter();
        if (iter_insert)
            cursor_key_press() = iter_insert.get_offset();
        else
            cursor_key_press() = -1;
        // print "self.cursor_key_press", self.cursor_key_press
    }
    else if (event->key.keyval == GDK_KEY_Menu)
    {
        if (curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID)
        {
            if (!curr_buffer->get_has_selection()) return false;
            Gtk::TextIter iter_sel_start, iter_sel_end;
            curr_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
            int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
            if (num_chars != 1) return false;
            auto widgets = curr_tree_iter().get_embedded_pixbufs_tables_codeboxes({iter_sel_start.get_offset(), iter_sel_start.get_offset()});
            if (widgets.empty()) return false;
            if (CtImageAnchor* anchor = dynamic_cast<CtImageAnchor*>(widgets.front()))
            {
                CtApp::P_ctActions->curr_anchor_anchor = anchor;
                CtApp::P_ctActions->object_set_selection(anchor);
                _pCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Anchor)->popup(3, event->button.time);
            }
            else if (CtImagePng* image = dynamic_cast<CtImagePng*>(widgets.front()))
            {
                CtApp::P_ctActions->curr_image_anchor = image;
                CtApp::P_ctActions->object_set_selection(image);
                _pCtMenu->find_action("img_link_dismiss")->signal_set_visible.emit(!image->get_link().empty());
                _pCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Image)->popup(3, event->button.time);
            }
            return true;
        }
    }
    else if (event->key.keyval == GDK_KEY_Tab)
    {
        if (!curr_buffer->get_has_selection())
        {
            auto iter_insert = curr_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(curr_buffer).get_paragraph_list_info(iter_insert);
            if (list_info)
             {
                get_text_view().list_change_level(iter_insert, list_info, true);
                return true;
            }
        }
        else if (curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID)
        {
            Gtk::TextIter iter_sel_start, iter_sel_end;
            curr_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
            int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
            if (num_chars != 1) return false;
            auto widgets = curr_tree_iter().get_embedded_pixbufs_tables_codeboxes({iter_sel_start.get_offset(), iter_sel_start.get_offset()});
            if (widgets.empty()) return false;
            if (dynamic_cast<CtTable*>(widgets.front()))
            {
                curr_buffer->place_cursor(iter_sel_end);
                get_text_view().grab_focus();
                return true;
            }
            else {
                return false;
            }
        }
    }
    return false;
}

// Called after every event on the SourceView
void CtMainWin::_on_textview_event_after(GdkEvent* event)
{
    if (event->type == GDK_2BUTTON_PRESS && event->button.button == 1)
        get_text_view().for_event_after_double_click_button1(event);
    else if (event->type == GDK_BUTTON_PRESS ||  event->type == GDK_KEY_PRESS)
    {
        if (curr_tree_iter() && curr_tree_iter().get_node_syntax_highlighting() == CtConst::RICH_TEXT_ID &&
            !curr_buffer()->get_modified())
        {
            // todo: self.state_machine.update_curr_state_cursor_pos(self.treestore[self.curr_tree_iter][3])
        }
        if (event->type == GDK_BUTTON_PRESS)
            get_text_view().for_event_after_button_press(event);
        if (event->type == GDK_KEY_PRESS)
            get_text_view().for_event_after_key_press(event, curr_tree_iter().get_node_syntax_highlighting());
    }
    else if (event->type == GDK_KEY_RELEASE)
    {
        if (event->key.keyval == GDK_KEY_Return || event->key.keyval == GDK_KEY_space)
        {
            if (CtApp::P_ctCfg->wordCountOn)
            {
                update_selected_node_statusbar_info();
            }
        }
    }
}

void CtMainWin::_title_update(const bool saveNeeded)
{
    Glib::ustring title;
    if (saveNeeded)
    {
        title += "*";
    }
    if (_ctCurrFile.rFile)
    {
        title += get_curr_doc_file_name() + " - " + get_curr_doc_file_dir() + " - ";
    }
    title += "CherryTree ";
    title += CtConst::CT_VERSION;
    set_title(title);
}
