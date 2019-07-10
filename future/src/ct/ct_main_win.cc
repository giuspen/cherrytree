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

CtTreeView::CtTreeView()
{
    set_headers_visible(false);
}

CtTreeView::~CtTreeView()
{
}

void CtTreeView::set_cursor_safe(const Gtk::TreeIter& iter)
{
    expand_to_path(get_model()->get_path(iter));
    set_cursor(get_model()->get_path(iter));
}

const double CtTextView::TEXT_SCROLL_MARGIN{0.3};

CtTextView::CtTextView()
{
    //set_sensitive(false);
    set_smart_home_end(Gsv::SMART_HOME_END_AFTER);
    set_left_margin(7);
    set_right_margin(7);
    set_insert_spaces_instead_of_tabs(CtApp::P_ctCfg->spacesInsteadTabs);
    set_tab_width((guint)CtApp::P_ctCfg->tabsWidth);
    if (CtApp::P_ctCfg->lineWrapping)
    {
        set_wrap_mode(Gtk::WrapMode::WRAP_WORD_CHAR);
    }
    else
    {
        set_wrap_mode(Gtk::WrapMode::WRAP_NONE);
    }
    for (const Gtk::TextWindowType& textWinType : std::list<Gtk::TextWindowType>{Gtk::TEXT_WINDOW_LEFT,
                                                                                 Gtk::TEXT_WINDOW_RIGHT,
                                                                                 Gtk::TEXT_WINDOW_TOP,
                                                                                 Gtk::TEXT_WINDOW_BOTTOM})
    {
        set_border_window_size(textWinType, 1);
    }
}

CtTextView::~CtTextView()
{
}

void CtTextView::setupForSyntax(const std::string& syntax)
{
    if (CtConst::RICH_TEXT_ID == syntax)
    {
        set_highlight_current_line(CtApp::P_ctCfg->rtHighlCurrLine);
        if (CtApp::P_ctCfg->rtShowWhiteSpaces)
        {
            set_draw_spaces(Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE);
        }
    }
    else
    {
        set_highlight_current_line(CtApp::P_ctCfg->ptHighlCurrLine);
        if (CtApp::P_ctCfg->ptShowWhiteSpaces)
        {
            set_draw_spaces(Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE);
        }
    }
    _setFontForSyntax(syntax);
}

void CtTextView::set_pixels_inside_wrap(int space_around_lines, int relative_wrapped_space)
{
    int pixels_around_wrap = (int)((double)space_around_lines * ((double)relative_wrapped_space / 100.0));
    Gtk::TextView::set_pixels_inside_wrap(pixels_around_wrap);
}

void CtTextView::set_selection_at_offset_n_delta(int offset, int delta, Glib::RefPtr<Gtk::TextBuffer> text_buffer /*=Glib::RefPtr<Gtk::TextBuffer>()*/)
{
    text_buffer = text_buffer ? text_buffer : get_buffer();
    Gtk::TextIter target = text_buffer->get_iter_at_offset(offset);
    if (target) {
        text_buffer->place_cursor(target);
        if (!target.forward_chars(delta)) {
            // #print "? bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
        }
        text_buffer->move_mark(text_buffer->get_selection_bound(), target);
    } else {
        // # print "! bad offset=%s, delta=%s on node %s" % (offset, delta, self.treestore[self.curr_tree_iter][1])
    }
}

void CtTextView::_setFontForSyntax(const std::string& syntaxHighlighting)
{
    Glib::RefPtr<Gtk::StyleContext> rStyleContext = get_style_context();
    std::string fontCss = CtFontUtil::getFontCssForSyntaxHighlighting(syntaxHighlighting);
    CtApp::R_cssProvider->load_from_data(fontCss);
    rStyleContext->add_provider(CtApp::R_cssProvider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}


CtMainWin::CtMainWin(CtMenu* pCtMenu) : Gtk::ApplicationWindow(), _ctMenu(pCtMenu)
{
    set_icon(CtApp::R_icontheme->load_icon(CtConst::APP_NAME, 48));
    _scrolledwindowTree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowText.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowTree.add(_ctTreeview);
    _scrolledwindowText.add(_ctTextview);
    _vboxText.pack_start(_initWindowHeader(), false, false);
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

    _pMenu = pCtMenu->build_menubar();
    _pMenu->set_name("MenuBar");
    _pBookmarksSubmenu = CtMenu::find_menu_item(_pMenu, "BookmarksMenu");
    _pSpecialCharsSubmenu = CtMenu::find_menu_item(_pMenu, "SpecialCharsMenu");
    _pMenu->show_all();
    gtk_window_add_accel_group (GTK_WINDOW(gobj()), pCtMenu->default_accel_group());
    _pNodePopup = pCtMenu->build_popup_menu_node();
    _pNodePopup->show_all();
    _pToolbar = pCtMenu->build_toolbar();

    _vboxMain.pack_start(*_pMenu, false, false);
    _vboxMain.pack_start(*_pToolbar, false, false);
    _vboxMain.pack_start(_hPaned);
    _vboxMain.pack_start(_initStatusBar(), false, false);
    add(_vboxMain);
    _ctTreestore.viewAppendColumns(&_ctTreeview);
    _ctTreestore.viewConnect(&_ctTreeview);
    _ctTreeview.signal_cursor_changed().connect(sigc::mem_fun(*this, &CtMainWin::_onTheTreeviewSignalCursorChanged));
    _ctTreeview.signal_button_release_event().connect(sigc::mem_fun(*this, &CtMainWin::_onTheTreeviewSignalButtonPressEvent));
    _ctTreeview.signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_onTheTreeviewSignalKeyPressEvent), false);
    _ctTreeview.signal_popup_menu().connect(sigc::mem_fun(*this, &CtMainWin::_onTheTreeviewSignalPopupMenu));

    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "cut-clipboard", G_CALLBACK(CtClipboard::on_cut_clipboard), nullptr /*from_codebox*/);
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "copy-clipboard", G_CALLBACK(CtClipboard::on_copy_clipboard), nullptr /*from_codebox*/);
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "paste-clipboard", G_CALLBACK(CtClipboard::on_paste_clipboard), nullptr /*from_codebox*/);

    signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_onTheWindowSignalKeyPressEvent), false);

    _titleUpdate(false/*saveNeeded*/);
    show_all();
    configApply(); // after show_all()
    set_menu_items_special_chars();
}

CtMainWin::~CtMainWin()
{
    //printf("~CtMainWin\n");
}

void CtMainWin::configApply()
{
    _hPaned.property_position() = CtApp::P_ctCfg->hpanedPos;
    set_size_request(CtApp::P_ctCfg->winRect[2], CtApp::P_ctCfg->winRect[3]);
    show_hide_tree_view(CtApp::P_ctCfg->treeVisible);
    show_hide_win_header(CtApp::P_ctCfg->showNodeNameHeader);

    show_hide_toolbar(CtApp::P_ctCfg->toolbarVisible);
    _pToolbar->set_toolbar_style(Gtk::ToolbarStyle::TOOLBAR_ICONS);
    set_toolbar_icon_size(CtApp::P_ctCfg->toolbarIconSize);

    _ctStatusBar.progressBar.hide();
    _ctStatusBar.stopButton.hide();
}

Gtk::HBox& CtMainWin::_initStatusBar()
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

Gtk::EventBox& CtMainWin::_initWindowHeader()
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
    return _ctWinHeader.eventBox;
}

void CtMainWin::window_header_update()
{
    // based on update_node_name_header
    std::string name = curr_tree_iter().get_node_name();
    _ctWinHeader.eventBox.override_background_color(Gdk::RGBA(CtApp::P_ctCfg->ttDefBg));
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

void CtMainWin::treeview_set_colors()
{
    std::string fg = curr_tree_iter().get_node_foreground();
    CtMiscUtil::widget_set_colors(_ctTreeview, CtApp::P_ctCfg->ttDefFg, CtApp::P_ctCfg->ttDefBg, false, fg);
}

void CtMainWin::menu_tree_update_for_bookmarked_node(bool is_bookmarked)
{
    _ctMenu->find_action("node_bookmark")->signal_set_visible.emit(!is_bookmarked);
    _ctMenu->find_action("node_unbookmark")->signal_set_visible.emit(is_bookmarked);
}

void CtMainWin::bookmark_action_select_node(gint64 node_id)
{
    Gtk::TreeIter tree_iter = _ctTreestore.get_tree_iter_from_node_id(node_id);
    get_tree_view().set_cursor_safe(tree_iter);
}

void CtMainWin::set_bookmarks_menu_items()
{
    std::list<std::tuple<gint64, std::string>> bookmarks;
    for (const gint64& node_id: _ctTreestore.get_bookmarks())
        bookmarks.push_back(std::make_tuple(node_id, _ctTreestore.get_node_name_from_node_id(node_id)));
    sigc::slot<void, gint64> bookmark_action = sigc::mem_fun(*this, &CtMainWin::bookmark_action_select_node);
    _pBookmarksSubmenu->set_submenu(*_ctMenu->build_bookmarks_menu(bookmarks, bookmark_action));
}

void CtMainWin::set_menu_items_special_chars()
{
    sigc::slot<void, gunichar> spec_char_action = sigc::mem_fun(*CtApp::P_ctActions, &CtActions::insert_spec_char_action);
    _pSpecialCharsSubmenu->set_submenu(*_ctMenu->build_special_chars_menu(CtApp::P_ctCfg->specialChars, spec_char_action));
}

bool CtMainWin::readNodesFromGioFile(const Glib::RefPtr<Gio::File>& r_file, const bool isImport)
{
    bool retOk{false};
    std::string filepath{r_file->get_path()};
    CtDocEncrypt docEncrypt = CtMiscUtil::getDocEncrypt(filepath);
    const gchar* pFilepath{nullptr};
    if (CtDocEncrypt::True == docEncrypt)
    {
        gchar* title = g_strdup_printf(_("Enter Password for %s"), Glib::path_get_basename(filepath).c_str());
        while (true)
        {
            CtDialogTextEntry dialogTextEntry(title, true/*forPassword*/, this);
            int response = dialogTextEntry.run();
            if (Gtk::RESPONSE_OK != response)
            {
                break;
            }
            Glib::ustring password = dialogTextEntry.getEntryText();
            if (0 == CtP7zaIface::p7za_extract(filepath.c_str(),
                                               CtApp::P_ctTmp->getHiddenDirPath(filepath),
                                               password.c_str()) &&
                g_file_test(CtApp::P_ctTmp->getHiddenFilePath(filepath), G_FILE_TEST_IS_REGULAR))
            {
                pFilepath = CtApp::P_ctTmp->getHiddenFilePath(filepath);
                break;
            }
        }
        g_free(title);
    }
    else if (CtDocEncrypt::False == docEncrypt)
    {
        pFilepath = filepath.c_str();
    }
    if (pFilepath)
    {
        retOk = _ctTreestore.readNodesFromFilepath(pFilepath, isImport);
    }
    if (retOk && !isImport)
    {
        _currFileName = Glib::path_get_basename(filepath);
        _currFileDir = Glib::path_get_dirname(filepath);
        _titleUpdate(false/*saveNeeded*/);
        set_bookmarks_menu_items();

        if ((_currFileName == CtApp::P_ctCfg->fileName) &&
            (_currFileDir == CtApp::P_ctCfg->fileDir))
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
                _ctTreestore.set_tree_expanded_collapsed_string(CtApp::P_ctCfg->expandedCollapsedString,
                                                                _ctTreeview,
                                                                CtApp::P_ctCfg->nodesBookmExp);
            }
            _ctTreestore.setTreePathTextCursorFromConfig(&_ctTreeview, &_ctTextview);
        }
    }
    return retOk;
}

void CtMainWin::_onTheTreeviewSignalCursorChanged()
{
    CtTreeIter treeIter = curr_tree_iter();
    _ctTreestore.applyTextBufferToCtTextView(treeIter, &_ctTextview);

    menu_tree_update_for_bookmarked_node(_ctTreestore.is_node_bookmarked(treeIter.get_node_id()));
    treeview_set_colors();
    window_header_update();
    window_header_update_lock_icon(treeIter.get_node_read_only());
    window_header_update_bookmark_icon(false);
}

bool CtMainWin::_onTheTreeviewSignalButtonPressEvent(GdkEventButton* event)
{
    if (event->button == 3)
    {
        _pNodePopup->popup(event->button, event->time);
        return true;
    }
    return false;
}

bool CtMainWin::_onTheWindowSignalKeyPressEvent(GdkEventKey* event)
{
    if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_Tab) {
            CtApp::P_ctActions->toggle_tree_text();
            return true;
        }
    }
    return false;
}

bool CtMainWin::_onTheTreeviewSignalKeyPressEvent(GdkEventKey* event)
{
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

bool CtMainWin::_onTheTreeviewSignalPopupMenu()
{
    _pNodePopup->popup(0, 0);
    return true;
}

void CtMainWin::_titleUpdate(bool saveNeeded)
{
    Glib::ustring title;
    if (saveNeeded)
    {
        title += "*";
    }
    if (!_currFileName.empty())
    {
        title += _currFileName + " - " + _currFileDir + " - ";
    }
    title += "CherryTree ";
    title += CtConst::CT_VERSION;
    set_title(title);
}

CtDialogTextEntry::CtDialogTextEntry(const char* title, const bool forPassword, Gtk::Window* pParent)
{
    set_title(title);
    set_transient_for(*pParent);
    set_modal();

    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    _entry.set_icon_from_stock(Gtk::Stock::CLEAR, Gtk::ENTRY_ICON_SECONDARY);
    _entry.set_size_request(350, -1);
    if (forPassword)
    {
        _entry.set_visibility(false);
    }
    get_vbox()->pack_start(_entry, true, true, 0);

    _entry.signal_key_press_event().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_onEntryKeyPress), false);
    _entry.signal_icon_press().connect(sigc::mem_fun(*this, &CtDialogTextEntry::_onEntryIconPress));

    get_vbox()->show_all();
}

CtDialogTextEntry::~CtDialogTextEntry()
{
}

bool CtDialogTextEntry::_onEntryKeyPress(GdkEventKey *eventKey)
{
    if (GDK_KEY_Return == eventKey->keyval)
    {
        Gtk::Button *pButton = static_cast<Gtk::Button*>(get_widget_for_response(Gtk::RESPONSE_OK));
        pButton->clicked();
        return true;
    }
    return false;
}

void CtDialogTextEntry::_onEntryIconPress(Gtk::EntryIconPosition /*iconPosition*/, const GdkEventButton* /*event*/)
{
    _entry.set_text("");
}

Glib::ustring CtDialogTextEntry::getEntryText()
{
    return _entry.get_text();
}
