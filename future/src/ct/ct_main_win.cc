/*
 * ct_main_win.cc
 *
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

CtTreeView::CtTreeView()
{
    set_headers_visible(false);
}

CtTreeView::~CtTreeView()
{
}

void CtTreeView::setExpandedCollapsed(CtTreeStore& ctTreestore)
{
    collapse_all();
    std::vector<std::string> vecExpandedCollapsed;
    CtStrUtil::gstringSplit2string(CtApp::P_ctCfg->expandedCollapsedString.c_str(), vecExpandedCollapsed, "_");
    std::map<gint64,bool> mapExpandedCollapsed;
    for (const std::string& element : vecExpandedCollapsed)
    {
        std::vector<std::string> vecElem;
        CtStrUtil::gstringSplit2string(element.c_str(), vecElem, ",");
        if (2 == vecElem.size())
        {
            mapExpandedCollapsed[std::stoi(vecElem[0])] = CtStrUtil::isStrTrue(vecElem[1]);
        }
    }
    ctTreestore.setExpandedCollapsed(this, ctTreestore.getRootChildren(), mapExpandedCollapsed);
}


const double CtTextView::TEXT_SCROLL_MARGIN{0.3};

CtTextView::CtTextView()
{
    //set_sensitive(false);
    set_smart_home_end(Gsv::SMART_HOME_END_AFTER);
    set_left_margin(7);
    set_right_margin(7);
    set_insert_spaces_instead_of_tabs(CtApp::P_ctCfg->spacesInsteadTabs);
    set_tab_width(CtApp::P_ctCfg->tabsWidth);
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

void CtTextView::_setFontForSyntax(const std::string& syntaxHighlighting)
{
    Glib::RefPtr<Gtk::StyleContext> rStyleContext = get_style_context();
    std::string fontCss = CtFontUtil::getFontCssForSyntaxHighlighting(syntaxHighlighting);
    CtApp::R_cssProvider->load_from_data(fontCss);
    rStyleContext->add_provider(CtApp::R_cssProvider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}


CtMainWin::CtMainWin(GtkWidget* menu) : Gtk::ApplicationWindow()
{
    set_icon(CtApp::R_icontheme->load_icon("cherrytree", 48));
    _scrolledwindowTree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowText.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowTree.add(_ctTreeview);
    _scrolledwindowText.add(_ctTextview);
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
    _menu = Glib::wrap(GTK_MENU_BAR(menu));
    _menu->set_name("MenuBar");
    _menu->show_all();
    _vboxMain.pack_start(*_menu, false, false);
    _vboxMain.pack_start(_hPaned);
    add(_vboxMain);
    _ctTreestore.viewAppendColumns(&_ctTreeview);
    _ctTreestore.viewConnect(&_ctTreeview);
    _ctTreeview.signal_cursor_changed().connect(sigc::mem_fun(*this, &CtMainWin::_onTheTreeviewSignalCursorChanged));
    configApply();
    _titleUpdate(false/*saveNeeded*/);
    show_all();
}

CtMainWin::~CtMainWin()
{
    //printf("~CtMainWin\n");
}

void CtMainWin::configApply()
{
    _hPaned.property_position() = CtApp::P_ctCfg->hpanedPos;
    set_size_request(CtApp::P_ctCfg->winRect[2], CtApp::P_ctCfg->winRect[3]);
}

bool CtMainWin::readNodesFromGioFile(const Glib::RefPtr<Gio::File>& r_file, const bool isImport)
{
    bool retOk{false};
    std::string filepath{r_file->get_path()};
    CtDocEncrypt docEncrypt = CtMiscUtil::getDocEncrypt(filepath);
    const gchar* pFilepath{NULL};
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
    if (NULL != pFilepath)
    {
        retOk = _ctTreestore.readNodesFromFilepath(pFilepath, isImport);
    }
    if (retOk && !isImport)
    {
        _currFileName = Glib::path_get_basename(filepath);
        _currFileDir = Glib::path_get_dirname(filepath);
        _titleUpdate(false/*saveNeeded*/);

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
                _ctTreeview.setExpandedCollapsed(_ctTreestore);
            }
            _ctTreestore.setTreePathTextCursorFromConfig(&_ctTreeview, &_ctTextview);
        }
    }
    return retOk;
}

void CtMainWin::_onTheTreeviewSignalCursorChanged()
{
    Gtk::TreeIter treeIter = _ctTreeview.get_selection()->get_selected();
    _ctTreestore.applyTextBufferToCtTextView(treeIter, &_ctTextview);
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

void CtDialogTextEntry::_onEntryIconPress(Gtk::EntryIconPosition iconPosition, const GdkEventButton* event)
{
    _entry.set_text("");
}

Glib::ustring CtDialogTextEntry::getEntryText()
{
    return _entry.get_text();
}
