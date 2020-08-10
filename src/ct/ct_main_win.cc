/*
 * ct_main_win.cc
 *
 * Copyright 2009-2020
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

#include "ct_main_win.h"
#include "ct_config.h"
#include "ct_clipboard.h"
#include "ct_actions.h"
#include "ct_storage_control.h"
#include "ct_storage_xml.h"
#include "ct_list.h"
#include "ct_export2txt.h"
#include <glib-object.h>
#include "ct_logging.h"

CtMainWin::CtMainWin(bool             no_gui,
                     CtConfig*        pCtConfig,
                     CtTmp*           pCtTmp,
                     Gtk::IconTheme*  pGtkIconTheme,
                     Glib::RefPtr<Gtk::TextTagTable> rGtkTextTagTable,
                     Glib::RefPtr<Gtk::CssProvider> rGtkCssProvider,
                     Gsv::LanguageManager*    pGsvLanguageManager,
                     Gsv::StyleSchemeManager* pGsvStyleSchemeManager,
                     Gtk::StatusIcon*         pGtkStatusIcon)
 : Gtk::ApplicationWindow(),
   _no_gui(no_gui),
   _pCtConfig(pCtConfig),
   _pCtTmp(pCtTmp),
   _pGtkIconTheme(pGtkIconTheme),
   _rGtkTextTagTable(rGtkTextTagTable),
   _rGtkCssProvider(rGtkCssProvider),
   _pGsvLanguageManager(pGsvLanguageManager),
   _pGsvStyleSchemeManager(pGsvStyleSchemeManager),
   _pGtkStatusIcon(pGtkStatusIcon),
   _ctTextview(this),
   _ctStateMachine(this)
{
    if (not _no_gui) {
        set_icon(_pGtkIconTheme->load_icon(CtConst::APP_NAME, 48));
    }

    _uCtActions.reset(new CtActions(this));
    _uCtMenu.reset(new CtMenu(pCtConfig, _uCtActions.get()));
    _uCtPrint.reset(new CtPrint());
    _uCtStorage.reset(CtStorageControl::create_dummy_storage(this));

    _scrolledwindowTree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowTree.get_style_context()->add_class("ct-tree-scroll-panel");
    _scrolledwindowText.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindowText.add(_ctTextview);
    _vboxText.pack_start(_init_window_header(), false, false);
    _vboxText.pack_start(_scrolledwindowText);
    if (_pCtConfig->treeRightSide)
    {
        _hPaned.add1(_vboxText);
        _hPaned.add2(_scrolledwindowTree);
    }
    else
    {
        _hPaned.add1(_scrolledwindowTree);
        _hPaned.add2(_vboxText);
    }
    _hPaned.property_wide_handle() = true;

    _pMenuBar = _uCtMenu->build_menubar();
    _pMenuBar->set_name("MenuBar");
    _pBookmarksSubmenu = CtMenu::find_menu_item(_pMenuBar, "BookmarksMenu");
    _pRecentDocsSubmenu = CtMenu::find_menu_item(_pMenuBar, "RecentDocsMenu");
    _pSpecialCharsSubmenu = CtMenu::find_menu_item(_pMenuBar, "SpecialCharsMenu");
    _pMenuBar->show_all();
    gtk_window_add_accel_group(GTK_WINDOW(gobj()), _uCtMenu->default_accel_group());
    _pToolbars = _uCtMenu->build_toolbars(_pRecentDocsMenuToolButton);

    _vboxMain.pack_start(*_pMenuBar, false, false);
    for (auto pToolbar: _pToolbars)
        _vboxMain.pack_start(*pToolbar, false, false);
    _vboxMain.pack_start(_hPaned);
    _vboxMain.pack_start(_init_status_bar(), false, false);
    _vboxMain.show_all();
    add(_vboxMain);

    _reset_CtTreestore_CtTreeview();

    _ctTextview.get_style_context()->add_class("ct-view-panel");
    _ctTextview.set_sensitive(false);

    _ctTextview.signal_populate_popup().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_populate_popup));
    _ctTextview.signal_motion_notify_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_motion_notify_event));
    _ctTextview.signal_visibility_notify_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_visibility_notify_event));
    _ctTextview.signal_size_allocate().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_size_allocate));
    _ctTextview.signal_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_event));
    _ctTextview.signal_event_after().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_event_after));
    _ctTextview.signal_scroll_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_textview_scroll_event));

    _uCtPairCodeboxMainWin.reset(new CtPairCodeboxMainWin{nullptr, this});
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "cut-clipboard", G_CALLBACK(CtClipboard::on_cut_clipboard), _uCtPairCodeboxMainWin.get());
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "copy-clipboard", G_CALLBACK(CtClipboard::on_copy_clipboard), _uCtPairCodeboxMainWin.get());
    g_signal_connect(G_OBJECT(_ctTextview.gobj()), "paste-clipboard", G_CALLBACK(CtClipboard::on_paste_clipboard), _uCtPairCodeboxMainWin.get());

    signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_window_key_press_event), false);

    file_autosave_restart();

    _title_update(false/*saveNeeded*/);

    config_apply();

    menu_set_items_recent_documents();
    menu_set_items_special_chars();
    _uCtMenu->find_action("ct_vacuum")->signal_set_visible.emit(false);

    if (_no_gui) {
        set_visible(false);
    } else if (_pCtConfig->systrayOn && _pCtConfig->startOnSystray) {
        set_visible(false);
    } else {
        present();
    }

    // show status icon if it's needed and also check if systray exists
    if (!_no_gui && _pCtConfig->systrayOn) {
        _pGtkStatusIcon->set_visible(true);
        Glib::signal_idle().connect_once([&](){
            if (!_pGtkStatusIcon->is_embedded()) { // is_embedded works only in main event loop
                if (_pCtConfig->startOnSystray) {
                    spdlog::warn("Start on systray is enabled but system does not support system trays, starting normally");
                }
                _pGtkStatusIcon->set_visible(false);
                _pCtConfig->systrayOn = false;
                menu_set_visible_exit_app(false);
                present();
            }
        });
    }
}

CtMainWin::~CtMainWin()
{
    _autosave_timout_connection.disconnect();
    //std::cout << "~CtMainWin" << std::endl;
}

std::string CtMainWin::get_code_icon_name(std::string code_type)
{
    for (const auto& iconPair : CtConst::NODE_CODE_ICONS) {
        if (0 == strcmp(iconPair.first, code_type.c_str())) {
            return iconPair.second;
        }
    }
    return CtConst::NODE_CUSTOM_ICONS.at(CtConst::NODE_ICON_CODE_ID);
}

Gtk::Image* CtMainWin::new_image_from_stock(const std::string& stockImage, Gtk::BuiltinIconSize size)
{
    Gtk::Image* image = Gtk::manage(new Gtk::Image());
    image->set_from_icon_name(stockImage, size);
    return image;
}

void CtMainWin::apply_syntax_highlighting(Glib::RefPtr<Gsv::Buffer> text_buffer, const std::string& syntax)
{
    if (text_buffer->get_data(CtConst::STYLE_APPLIED_ID))
        return;
    if (CtConst::TABLE_CELL_TEXT_ID == syntax)
    {
        text_buffer->set_style_scheme(_pGsvStyleSchemeManager->get_scheme(CtConst::STYLE_SCHEME_LIGHT));
    }
    else if (CtConst::RICH_TEXT_ID == syntax)
    {
        // dark theme
        if (get_ct_config()->rtDefFg == CtConst::RICH_TEXT_DARK_FG && get_ct_config()->rtDefBg == CtConst::RICH_TEXT_DARK_BG)
            text_buffer->set_style_scheme(_pGsvStyleSchemeManager->get_scheme(CtConst::STYLE_SCHEME_DARK));
        else
            text_buffer->set_style_scheme(_pGsvStyleSchemeManager->get_scheme(CtConst::STYLE_SCHEME_LIGHT));
    }
    else
    {
        text_buffer->set_style_scheme(_pGsvStyleSchemeManager->get_scheme(_pCtConfig->styleSchemeId));
        if (CtConst::PLAIN_TEXT_ID == syntax)
        {
            text_buffer->set_highlight_syntax(false);
        }
        else
        {
            text_buffer->set_language(_pGsvLanguageManager->get_language(syntax));
            text_buffer->set_highlight_syntax(true);
        }
        text_buffer->set_highlight_matching_brackets(true);
    }
    text_buffer->set_data(CtConst::STYLE_APPLIED_ID, (void*)1);
}

Glib::RefPtr<Gsv::Buffer> CtMainWin::get_new_text_buffer(const Glib::ustring& textContent)
{
    Glib::RefPtr<Gsv::Buffer> rRetTextBuffer;
    rRetTextBuffer = Gsv::Buffer::create(_rGtkTextTagTable);
    rRetTextBuffer->set_max_undo_levels(_pCtConfig->limitUndoableSteps);

    if (not textContent.empty())
    {
        rRetTextBuffer->begin_not_undoable_action();
        rRetTextBuffer->set_text(textContent);
        rRetTextBuffer->end_not_undoable_action();
        rRetTextBuffer->set_modified(false);
    }
    return rRetTextBuffer;
}

const std::string CtMainWin::get_text_tag_name_exist_or_create(const std::string& propertyName, const std::string& propertyValue)
{
    const std::string tagName{propertyName + "_" + propertyValue};
    Glib::RefPtr<Gtk::TextTag> rTextTag = _rGtkTextTagTable->lookup(tagName);
    if (not rTextTag)
    {
        bool identified{true};
        rTextTag = Gtk::TextTag::create(tagName);
        if (CtConst::TAG_INDENT == propertyName)
        {
            rTextTag->property_left_margin() = CtConst::INDENT_MARGIN * std::stoi(propertyValue);
            rTextTag->property_indent() = 0;
        }
        else if (CtConst::TAG_WEIGHT == propertyName and CtConst::TAG_PROP_VAL_HEAVY == propertyValue)
        {
            rTextTag->property_weight() = PANGO_WEIGHT_HEAVY;
        }
        else if (CtConst::TAG_FOREGROUND == propertyName)
        {
            rTextTag->property_foreground() = propertyValue;
        }
        else if (CtConst::TAG_BACKGROUND == propertyName)
        {
            rTextTag->property_background() = propertyValue;
        }
        else if (CtConst::TAG_SCALE == propertyName)
        {
            if (CtConst::TAG_PROP_VAL_SMALL == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_SMALL;
            }
            else if (CtConst::TAG_PROP_VAL_H1 == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_XX_LARGE;
            }
            else if (CtConst::TAG_PROP_VAL_H2 == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_X_LARGE;
            }
            else if (CtConst::TAG_PROP_VAL_H3 == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_LARGE;
            }
            else if (CtConst::TAG_PROP_VAL_SUB == propertyValue or CtConst::TAG_PROP_VAL_SUP == propertyValue)
            {
                rTextTag->property_scale() = PANGO_SCALE_X_SMALL;
                int propRise = Pango::FontDescription(_pCtConfig->rtFont).get_size();
                if (CtConst::TAG_PROP_VAL_SUB == propertyValue)
                {
                    propRise /= -4;
                }
                else
                {
                    propRise /= 2;
                }
                rTextTag->property_rise() = propRise;
            }
            else
            {
                identified = false;
            }
        }
        else if (CtConst::TAG_STYLE == propertyName and CtConst::TAG_PROP_VAL_ITALIC == propertyValue)
        {
            rTextTag->property_style() = Pango::Style::STYLE_ITALIC;
        }
        else if (CtConst::TAG_UNDERLINE == propertyName and CtConst::TAG_PROP_VAL_SINGLE == propertyValue)
        {
            rTextTag->property_underline() = Pango::Underline::UNDERLINE_SINGLE;
        }
        else if (CtConst::TAG_JUSTIFICATION == propertyName)
        {
            if (CtConst::TAG_PROP_VAL_LEFT == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_LEFT;
            }
            else if (CtConst::TAG_PROP_VAL_RIGHT == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_RIGHT;
            }
            else if (CtConst::TAG_PROP_VAL_CENTER == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_CENTER;
            }
            else if (CtConst::TAG_PROP_VAL_FILL == propertyValue)
            {
                rTextTag->property_justification() = Gtk::Justification::JUSTIFY_FILL;
            }
            else
            {
                identified = false;
            }
        }
        else if (CtConst::TAG_FAMILY == propertyName and CtConst::TAG_PROP_VAL_MONOSPACE == propertyValue)
        {
            rTextTag->property_family() = CtConst::TAG_PROP_VAL_MONOSPACE;
            if (not _pCtConfig->monospaceBg.empty())
            {
                rTextTag->property_background() = _pCtConfig->monospaceBg;
            }
            if (not _pCtConfig->monospaceFont.empty())
            {
                rTextTag->property_font() = _pCtConfig->monospaceFont;
            }
        }
        else if (CtConst::TAG_STRIKETHROUGH == propertyName and CtConst::TAG_PROP_VAL_TRUE == propertyValue)
        {
            rTextTag->property_strikethrough() = true;
        }
        else if (CtConst::TAG_LINK == propertyName and propertyValue.size() > 4)
        {
            if (_pCtConfig->linksUnderline)
            {
                rTextTag->property_underline() = Pango::Underline::UNDERLINE_SINGLE;
            }
            Glib::ustring linkType = propertyValue.substr(0, 4);
            if (CtConst::LINK_TYPE_WEBS == linkType)
            {
                rTextTag->property_foreground() = _pCtConfig->colLinkWebs;
            }
            else if (CtConst::LINK_TYPE_NODE == linkType)
            {
                rTextTag->property_foreground() = _pCtConfig->colLinkNode;
            }
            else if (CtConst::LINK_TYPE_FILE == linkType)
            {
                rTextTag->property_foreground() = _pCtConfig->colLinkFile;
            }
            else if (CtConst::LINK_TYPE_FOLD == linkType)
            {
                rTextTag->property_foreground() = _pCtConfig->colLinkFold;
            }
            else
            {
                identified = false;
            }
        }
        else
        {
            identified = false;
        }
        if (not identified)
        {
           // spdlog::error("!! unsupported propertyName={} propertyValue={}", propertyName, propertyValue);
        }
        _rGtkTextTagTable->add(rTextTag);
    }
    return tagName;
}

// Get the tooltip for the underlying link
Glib::ustring CtMainWin::sourceview_hovering_link_get_tooltip(const Glib::ustring& link)
{
    Glib::ustring tooltip;
    auto vec = str::split(link, " ");
    if (vec.size() == 1) { // case when link has wrong format
        tooltip = str::replace(link, "amp;", "");
    } 
    else if (vec[0] == CtConst::LINK_TYPE_FILE or vec[0] == CtConst::LINK_TYPE_FOLD) {
        tooltip = Glib::Base64::decode(vec[1]);
    }
    else
    {
        if (vec[0] == CtConst::LINK_TYPE_NODE)
            tooltip = _uCtTreestore->get_node_name_from_node_id(std::stol(vec[1]));
        else
            tooltip = str::replace(vec[1], "amp;", "");
        if (vec.size() >= 3)
        {
            if (vec.size() == 3) tooltip += "#" + vec[2];
            else
                tooltip += "#" + link.substr(vec[0].length() + vec[1].length() + 2);
        }
    }
    return tooltip;
}

// Try to Select a Word Forward/Backward the Cursor
bool CtMainWin::apply_tag_try_automatic_bounds(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter iter_start)
{
    Gtk::TextIter iter_end = iter_start;
    auto curr_char = iter_end.get_char();
    auto re = Glib::Regex::create("\\w");
    // 1) select alphanumeric + special
    bool match = re->match(Glib::ustring(1, curr_char));
    if (not match and _pCtConfig->selwordChars.item().find(curr_char) == Glib::ustring::npos) {
        iter_start.backward_char();
        iter_end.backward_char();
        curr_char = iter_end.get_char();
        match = re->match(Glib::ustring(1, curr_char));
        if (not match and _pCtConfig->selwordChars.item().find(curr_char) == Glib::ustring::npos)
            return false;
    }
    while (match or _pCtConfig->selwordChars.item().find(curr_char) != Glib::ustring::npos) {
        if (not iter_end.forward_char()) break; // end of buffer
        curr_char = iter_end.get_char();
        match = re->match(Glib::ustring(1, curr_char));
    }
    iter_start.backward_char();
    curr_char = iter_start.get_char();
    match = re->match(Glib::ustring(1, curr_char));
    while (match or _pCtConfig->selwordChars.item().find(curr_char) != Glib::ustring::npos) {
        if (not iter_start.backward_char()) break; // start of buffer
        curr_char = iter_start.get_char();
        match = re->match(Glib::ustring(1, curr_char));
    }
    if (not match and _pCtConfig->selwordChars.item().find(curr_char) == Glib::ustring::npos)
        iter_start.forward_char();
    // 2) remove non alphanumeric from borders
    iter_end.backward_char();
    curr_char = iter_end.get_char();
    while (_pCtConfig->selwordChars.item().find(curr_char) != Glib::ustring::npos) {
        if (not iter_end.backward_char()) break; // start of buffer
        curr_char = iter_end.get_char();
    }
    iter_end.forward_char();
    curr_char = iter_start.get_char();
    while (_pCtConfig->selwordChars.item().find(curr_char) != Glib::ustring::npos) {
        if (not iter_start.forward_char()) break; // end of buffer
        curr_char = iter_start.get_char();
    }
    if (iter_end.compare(iter_start) > 0) {
        text_buffer->move_mark(text_buffer->get_insert(), iter_start);
        text_buffer->move_mark(text_buffer->get_selection_bound(), iter_end);
        return true;
    }
    return false;
}

// Try to select the full paragraph
void CtMainWin::apply_tag_try_automatic_bounds_triple_click(Glib::RefPtr<Gtk::TextBuffer> text_buffer, Gtk::TextIter iter_start)
{
    Gtk::TextIter iter_end = iter_start;
    iter_end.forward_to_line_end();

    iter_end.forward_char();
    auto next_char = iter_end.get_char();
    while (next_char != '\n' && next_char != ' ')
    {
        // forward to the end of the line, if the next char
        // is not a new line or space then repeat
        iter_end.forward_to_line_end();
        if (!iter_end.forward_char()) break;
        next_char = iter_end.get_char();
    }

    // reverse to beginning of line to check for space indicating line
    // selected is the first line of a paragraph
    iter_start.backward_chars(iter_start.get_visible_line_offset());
    // reverse until either a new line or a space is found
    while (iter_start.get_char() != '\n' && iter_start.get_char() != ' ')
        if (!iter_start.backward_line()) break;

    if (iter_start.get_char() == '\n')
        iter_start.forward_chars(1);

    text_buffer->move_mark(text_buffer->get_insert(), iter_start);
    text_buffer->move_mark(text_buffer->get_selection_bound(), iter_end);
}

void CtMainWin::_reset_CtTreestore_CtTreeview()
{
    _prevTreeIter = CtTreeIter();
    _nodesCursorPos.clear();

    _scrolledwindowTree.remove();
    _uCtTreeview.reset(new CtTreeView);
    _scrolledwindowTree.add(*_uCtTreeview);
    _uCtTreeview->show();

    _uCtTreestore.reset(new CtTreeStore(this));
    _uCtTreestore->tree_view_connect(_uCtTreeview.get());
    _uCtTreeview->set_tree_node_name_wrap_width(get_ct_config()->cherryWrapEnabled, get_ct_config()->cherryWrapWidth);
    _uCtTreeview->get_column(CtTreeView::AUX_ICON_COL_NUM)->set_visible(!get_ct_config()->auxIconHide);

    _tree_just_auto_expanded = false;
    _uCtTreeview->signal_cursor_changed().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_cursor_changed));
    _uCtTreeview->signal_button_release_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_button_release_event));
    _uCtTreeview->signal_event_after().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_event_after));
    _uCtTreeview->signal_row_activated().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_row_activated));
    _uCtTreeview->signal_test_collapse_row().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_test_collapse_row));
    _uCtTreeview->signal_key_press_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_key_press_event), false);
    _uCtTreeview->signal_scroll_event().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_scroll_event));
    _uCtTreeview->signal_popup_menu().connect(sigc::mem_fun(*this, &CtMainWin::_on_treeview_popup_menu));

    //_uCtTreeview->set_reorderable(true); // tree store handles insert/removing rows
    _uCtTreeview->enable_model_drag_dest(Gdk::ACTION_MOVE);
    _uCtTreeview->enable_model_drag_source();

    _uCtTreeview->get_style_context()->add_class("ct-tree-panel");
    _uCtTreeview->set_margin_bottom(10);  // so horiz scroll doens't prevent to select the bottom element
}

void CtMainWin::config_apply()
{
    move(_pCtConfig->winRect[0], _pCtConfig->winRect[1]);
    set_default_size(_pCtConfig->winRect[2], _pCtConfig->winRect[3]);
    if (_pCtConfig->winIsMaximised)
    {
        maximize();
    }
    _hPaned.property_position() = _pCtConfig->hpanedPos;


    show_hide_tree_view(_pCtConfig->treeVisible);
    show_hide_win_header(_pCtConfig->showNodeNameHeader);
    _ctWinHeader.lockIcon.hide();
    _ctWinHeader.bookmarkIcon.hide();

    menu_rebuild_toolbars(false);

    _ctStatusBar.progressBar.hide();
    _ctStatusBar.stopButton.hide();

    menu_set_visible_exit_app(_pCtConfig->systrayOn);

    _ctTextview.set_show_line_numbers(get_ct_config()->showLineNumbers);
    _ctTextview.set_insert_spaces_instead_of_tabs(get_ct_config()->spacesInsteadTabs);
    _ctTextview.set_tab_width(get_ct_config()->tabsWidth);
    _ctTextview.set_indent(get_ct_config()->wrappingIndent);
    _ctTextview.set_pixels_above_lines(get_ct_config()->spaceAroundLines);
    _ctTextview.set_pixels_below_lines(get_ct_config()->spaceAroundLines);
    _ctTextview.set_pixels_inside_wrap(get_ct_config()->spaceAroundLines, get_ct_config()->relativeWrappedSpace);
    _ctTextview.set_wrap_mode(get_ct_config()->lineWrapping ? Gtk::WrapMode::WRAP_WORD_CHAR : Gtk::WrapMode::WRAP_NONE);

    update_theme();
}

void CtMainWin::config_update_data_from_curr_status()
{
    _pCtConfig->winIsMaximised = is_maximized();
    if (not _pCtConfig->winIsMaximised)
    {
        get_position(_pCtConfig->winRect[0], _pCtConfig->winRect[1]);
        get_size(_pCtConfig->winRect[2], _pCtConfig->winRect[3]);
    }
    _pCtConfig->hpanedPos = _hPaned.property_position();
    _ensure_curr_doc_in_recent_docs();
}

void CtMainWin::update_theme()
{ 
    auto font_to_string = [](Pango::FontDescription font, std::string /*fallbackFont*/)
    {
        // fallback font doesn't work on Win32 because of pango
        // add fallback font (to help with font on Win; on Linux, font works ok without explicit fallback
        // return " { font-family: \"" + font.get_family() + "\",\"" + fallbackFont +  "\";"
        //            "font-size: " + std::to_string(font.get_size()/Pango::SCALE) + "pt; } ";
        return " { font-family: " + font.get_family() +
                 "; font-size: " + std::to_string(font.get_size()/Pango::SCALE) + "pt; } ";
    };

    std::string rtFont = font_to_string(Pango::FontDescription(_pCtConfig->rtFont), _pCtConfig->fallbackFontFamily);
    std::string plFont = font_to_string(Pango::FontDescription(_pCtConfig->ptFont), _pCtConfig->fallbackFontFamily);
    std::string codeFont = font_to_string(Pango::FontDescription(_pCtConfig->codeFont), "monospace");
    std::string treeFont = font_to_string(Pango::FontDescription(_pCtConfig->treeFont), _pCtConfig->fallbackFontFamily);

    std::string font_css;
    font_css += ".ct-view-panel.ct-view-rich-text" + rtFont;
    font_css += ".ct-view-panel.ct-view-plain-text" + plFont;
    font_css += ".ct-view-panel.ct-view-code" + codeFont;
    font_css += ".ct-codebox.ct-view-rich-text" + rtFont;
    font_css += ".ct-codebox.ct-view-plain-text" + codeFont;
    font_css += ".ct-codebox.ct-view-code" + codeFont;
    font_css += ".ct-tree-panel" + treeFont;

    std::string theme_css;
    theme_css += ".ct-tree-panel { color: " + _pCtConfig->ttDefFg + "; background-color: " + _pCtConfig->ttDefBg + "; } ";
    theme_css += ".ct-tree-panel:selected { background: #5294e2;  } ";
    theme_css += ".ct-tree-scroll-panel { background-color: " + _pCtConfig->ttDefBg + "; } ";
    theme_css += ".ct-header-panel { background-color: " + _pCtConfig->ttDefBg + "; } ";
    theme_css += ".ct-header-panel button { margin: 2px; padding: 0 4px 0 4px; } ";
    theme_css += ".ct-status-bar bar { margin: 0px; } ";
    theme_css += ".ct-table-header-cell { font-weight: bold; } ";
    theme_css += ".ct-table grid { background: #cccccc; border-style:solid; border-width: 1px; border-color: gray; } ";
    theme_css += "toolbar { padding: 2px 2px 2px 2px; } ";
    theme_css += "toolbar button { padding: 0px; } ";

    if (_css_provider_theme)
    {
        Gtk::StyleContext::remove_provider_for_screen(get_screen(), _css_provider_theme);
    }
    _css_provider_theme = Gtk::CssProvider::create();
    _css_provider_theme->load_from_data(font_css + " " + theme_css);
    // _css_provider_theme->load_from_data(theme_css); second call of load_from_data erases css from the first call on mac
    get_style_context()->add_provider_for_screen(get_screen(), _css_provider_theme, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

Gtk::HBox& CtMainWin::_init_status_bar()
{
    _ctStatusBar.statusId = _ctStatusBar.statusBar.get_context_id("");
    _ctStatusBar.frame.set_shadow_type(Gtk::SHADOW_NONE);
    _ctStatusBar.frame.add(_ctStatusBar.progressBar);
    _ctStatusBar.stopButton.set_image_from_icon_name("ct_stop", Gtk::ICON_SIZE_MENU);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.statusBar, true, true);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.frame, false, true);
    _ctStatusBar.hbox.pack_start(_ctStatusBar.stopButton, false, true);

    _ctStatusBar.hbox.get_style_context()->add_class("ct-status-bar");
    // todo: move to css
    _ctStatusBar.frame.set_border_width(1);
    _ctStatusBar.statusBar.set_margin_top(0);
    _ctStatusBar.statusBar.set_margin_bottom(0);
    ((Gtk::Frame*)_ctStatusBar.statusBar.get_children()[0])->get_child()->set_margin_top(1);
    ((Gtk::Frame*)_ctStatusBar.statusBar.get_children()[0])->get_child()->set_margin_bottom(1);
    _ctStatusBar.hbox.set_border_width(0);

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
    _ctWinHeader.lockIcon.set_from_icon_name("ct_locked", Gtk::ICON_SIZE_MENU);
    _ctWinHeader.lockIcon.hide();
    _ctWinHeader.bookmarkIcon.set_from_icon_name("ct_pin", Gtk::ICON_SIZE_MENU);
    _ctWinHeader.bookmarkIcon.hide();
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.buttonBox, false, false);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.nameLabel, true, true);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.lockIcon, false, false);
    _ctWinHeader.headerBox.pack_start(_ctWinHeader.bookmarkIcon, false, false);
    _ctWinHeader.eventBox.add(_ctWinHeader.headerBox);
    _ctWinHeader.eventBox.get_style_context()->add_class("ct-header-panel");
    return _ctWinHeader.eventBox;
}

void CtMainWin::window_header_update()
{
    // based on update_node_name_header
    std::string name = curr_tree_iter().get_node_name();
    std::string foreground = curr_tree_iter().get_node_foreground();
    foreground = foreground.empty() ? _pCtConfig->ttDefFg : foreground;
    _ctWinHeader.nameLabel.set_markup(
                "<b><span foreground=\"" + foreground + "\" size=\"xx-large\">"
                + str::xml_escape(name) + "</span></b>");

    // update last visited buttons
    if (get_ct_config()->nodesOnNodeNameHeader == 0)
    {
        for (auto button: _ctWinHeader.buttonBox.get_children())
            _ctWinHeader.buttonBox.remove(*button);
    }
    else
    {
        // add more buttons if that is needed
        while ((int)_ctWinHeader.buttonBox.get_children().size() < get_ct_config()->nodesOnNodeNameHeader)
        {
            Gtk::Button* button = Gtk::manage(new Gtk::Button(""));
            auto click = [this](Gtk::Button* button) {
                auto node_id = _ctWinHeader.button_to_node_id.find(button);
                if (node_id != _ctWinHeader.button_to_node_id.end())
                {
                    if (CtTreeIter tree_iter = get_tree_store().get_node_from_node_id(node_id->second))
                        _uCtTreeview->set_cursor_safe(tree_iter);
                    _ctTextview.grab_focus();
                }
            };
            button->signal_clicked().connect(sigc::bind(click, button));
            _ctWinHeader.buttonBox.add(*button);
        }

        // update button labels and node_ids
        gint64 curr_node = curr_tree_iter().get_node_id();
        int button_idx = 0;
        auto buttons = _ctWinHeader.buttonBox.get_children();
        auto nodes = get_state_machine().get_visited_nodes_list();
        _ctWinHeader.button_to_node_id.clear();
        for (auto iter = nodes.rbegin(); iter != nodes.rend(); ++iter)
        {
            if (*iter == curr_node) continue;
            if (CtTreeIter node = get_tree_store().get_node_from_node_id(*iter))
            {
                Glib::ustring name = "<small>" + str::xml_escape(node.get_node_name()) + "</small>";
                Glib::ustring tooltip = CtMiscUtil::get_node_hierarchical_name(node, "/", false);
                if (auto button = dynamic_cast<Gtk::Button*>(buttons[button_idx]))
                {
                    if (auto label = dynamic_cast<Gtk::Label*>(button->get_child()))
                    {
                        label->set_label(name);
                        label->set_use_markup(true);
                        label->set_ellipsize(Pango::ELLIPSIZE_END);
                    }
                    button->set_tooltip_text(tooltip);
                    button->show();
                    _ctWinHeader.button_to_node_id[button] = *iter;
                }
                ++button_idx;
                if (button_idx == (int)buttons.size())
                    break;
            }
        }
        for (int i = button_idx; i < (int)buttons.size(); ++i)
            buttons[i]->hide();
    }
}

void CtMainWin::window_header_update_lock_icon(bool show)
{
    show ? _ctWinHeader.lockIcon.show() : _ctWinHeader.lockIcon.hide();
}

void CtMainWin::window_header_update_bookmark_icon(bool show)
{
    show ? _ctWinHeader.bookmarkIcon.show() : _ctWinHeader.bookmarkIcon.hide();
}

void CtMainWin::menu_update_bookmark_menu_item(bool is_bookmarked)
{
    _uCtMenu->find_action("node_bookmark")->signal_set_visible.emit(not is_bookmarked);
    _uCtMenu->find_action("node_unbookmark")->signal_set_visible.emit(is_bookmarked);
}

void CtMainWin::menu_set_bookmark_menu_items()
{
    std::list<std::pair<gint64, std::string>> bookmarks;
    for (const gint64& node_id : _uCtTreestore->bookmarks_get())
    {
        bookmarks.push_back(std::make_pair(node_id, _uCtTreestore->get_node_name_from_node_id(node_id)));
    }

    sigc::slot<void, gint64> bookmark_action = [&](gint64 node_id) {
        Gtk::TreeIter tree_iter = _uCtTreestore->get_node_from_node_id(node_id);
        _uCtTreeview->set_cursor_safe(tree_iter);
    };
    _pBookmarksSubmenu->set_submenu(*_uCtMenu->build_bookmarks_menu(bookmarks, bookmark_action));
}

void CtMainWin::menu_set_items_recent_documents()
{
    sigc::slot<void, const std::string&> recent_doc_open_action = [&](const std::string& filepath)
    {
        if (Glib::file_test(filepath, Glib::FILE_TEST_IS_REGULAR))
        {
            if (file_open(filepath, ""))
            {
                _pCtConfig->recentDocsFilepaths.move_or_push_front(Glib::canonicalize_filename(filepath));
                menu_set_items_recent_documents();
            }
        }
        else
        {
            g_autofree gchar* title = g_strdup_printf(_("The Document %s was Not Found"), filepath.c_str());
            CtDialogs::error_dialog(Glib::ustring{title}, *this);
            _pCtConfig->recentDocsFilepaths.move_or_push_back(Glib::canonicalize_filename(filepath));
            menu_set_items_recent_documents();
        }
    };
    sigc::slot<void, const std::string&> recent_doc_rm_action = [&](const std::string& filepath)
    {
        _pCtConfig->recentDocsFilepaths.remove(filepath);
        menu_set_items_recent_documents();
    };
    if (_pRecentDocsSubmenu)
    {
        Gtk::Menu* pMenu = _pRecentDocsSubmenu->get_submenu();
        delete pMenu;
        _pRecentDocsSubmenu->set_submenu(*_uCtMenu->build_recent_docs_menu(_pCtConfig->recentDocsFilepaths,
                                                                           recent_doc_open_action,
                                                                           recent_doc_rm_action));
    }
    if (_pRecentDocsMenuToolButton)
    {
        _pRecentDocsMenuToolButton->set_arrow_tooltip_text(_("Open a Recent CherryTree Document"));
        Gtk::Menu* pMenu = _pRecentDocsMenuToolButton->get_menu();
        delete pMenu;
        _pRecentDocsMenuToolButton->set_menu(*_uCtMenu->build_recent_docs_menu(_pCtConfig->recentDocsFilepaths,
                                                                               recent_doc_open_action,
                                                                               recent_doc_rm_action));
    }
}

void CtMainWin::menu_set_items_special_chars()
{
    sigc::slot<void, gunichar> spec_char_action = sigc::mem_fun(*_uCtActions, &CtActions::insert_spec_char_action);
    _pSpecialCharsSubmenu->set_submenu(*_uCtMenu->build_special_chars_menu(_pCtConfig->specialChars.item(), spec_char_action));
}

void CtMainWin::menu_set_visible_exit_app(bool visible)
{
    if (auto quit_label = CtMenu::get_accel_label(CtMenu::find_menu_item(_pMenuBar, "quit_app")))
    {
        quit_label->set_label(visible ? _("Hide") : _("Quit"));
        quit_label->set_tooltip_markup(visible ?  _("Hide the Window") : _("Quit the Application"));
    }
    CtMenu::find_menu_item(_pMenuBar, "exit_app")->set_visible(visible);
}

void CtMainWin::menu_rebuild_toolbars(bool new_toolbar)
{
    if (new_toolbar)
    {
        for (auto pToolbar: _pToolbars)
            _vboxMain.remove(*pToolbar);
        _pToolbars = _uCtMenu->build_toolbars(_pRecentDocsMenuToolButton);
        for (auto toolbar = _pToolbars.rbegin(); toolbar != _pToolbars.rend(); ++toolbar)
        {
            _vboxMain.pack_start(*(*toolbar), false, false);
            _vboxMain.reorder_child(*(*toolbar), 1);
        }
        menu_set_items_recent_documents();
        for (auto pToolbar: _pToolbars)
            pToolbar->show_all();
    }

    show_hide_toolbars(_pCtConfig->toolbarVisible);
    for (auto pToolbar: _pToolbars)
        pToolbar->set_toolbar_style(Gtk::ToolbarStyle::TOOLBAR_ICONS);
    set_toolbars_icon_size(_pCtConfig->toolbarIconSize);
}


void CtMainWin::config_switch_tree_side()
{
    auto tree_width = _scrolledwindowTree.get_width();
    auto text_width = _vboxText.get_width();

    _hPaned.remove(_scrolledwindowTree);
    _hPaned.remove(_vboxText);
    if (_pCtConfig->treeRightSide)
    {
        _hPaned.add1(_vboxText);
        _hPaned.add2(_scrolledwindowTree);
        _hPaned.property_position() = text_width;
    }
    else
    {
        _hPaned.add1(_scrolledwindowTree);
        _hPaned.add2(_vboxText);
        _hPaned.property_position() = tree_width;
    }
}

void CtMainWin::_ensure_curr_doc_in_recent_docs()
{
    fs::path currDocFilePath = _uCtStorage->get_file_path();
    if (not currDocFilePath.empty())
    {
        _pCtConfig->recentDocsFilepaths.move_or_push_front(fs::canonical(currDocFilePath));
        CtRecentDocRestore prevDocRestore;
        prevDocRestore.exp_coll_str = _uCtTreestore->treeview_get_tree_expanded_collapsed_string(*_uCtTreeview);
        const CtTreeIter prevTreeIter = curr_tree_iter();
        if (prevTreeIter)
        {
            prevDocRestore.node_path = _uCtTreestore->get_path(prevTreeIter).to_string();
            const Glib::RefPtr<Gsv::Buffer> rTextBuffer = prevTreeIter.get_node_text_buffer();
            prevDocRestore.cursor_pos = rTextBuffer->property_cursor_position();
        }
        _pCtConfig->recentDocsRestore[currDocFilePath.string()] = prevDocRestore;
    }
}

void CtMainWin::_zoom_tree(bool is_increase)
{
    Glib::RefPtr<Gtk::StyleContext> context = _uCtTreeview->get_style_context();
    Pango::FontDescription fontDesc = context->get_font(context->get_state());
    int size = fontDesc.get_size() / Pango::SCALE + (is_increase ? 1 : -1);
    if (size < 6) size = 6;
    fontDesc.set_size(size * Pango::SCALE);
    _uCtTreeview->override_font(fontDesc);
    _pCtConfig->treeFont = CtFontUtil::get_font_str(fontDesc);
}

bool CtMainWin::file_open(const fs::path& filepath, const std::string& node_to_focus, const Glib::ustring password)
{
    if (!fs::is_regular_file(filepath)) {
        CtDialogs::error_dialog("File does not exist", *this);
        return false;
    }
    if (fs::get_doc_type(filepath) == CtDocType::None) {
        // can't open file but can insert content into a new node
        if (file_insert_plain_text(filepath)) {
            return false; // that's right
        } else {
            CtDialogs::error_dialog(str::format(_("\"%s\" is Not a CherryTree Document"), filepath.string()), *this);
            return false;
        }
    }

    if (!file_save_ask_user())
        return false;

    fs::path prev_path = _uCtStorage->get_file_path();

    _ensure_curr_doc_in_recent_docs();
    reset(); // cannot reset after load_from because load_from fill tree store

    Glib::ustring error;
    auto new_storage = CtStorageControl::load_from(this, filepath, error, password);
    if (!new_storage) {
        if (not error.empty()) {
            CtDialogs::error_dialog(str::format(_("Error Parsing the CherryTree File:\n\"%s\""), error), *this);
        }

        // trying to recover prevous document
        if (!prev_path.empty())
            file_open(prev_path, ""); // it won't be in loop because storage is empty
        return false;                 // show the given document is not loaded
    }

    _uCtStorage.reset(new_storage);

    _title_update(false/*saveNeeded*/);
    menu_set_bookmark_menu_items();
    bool can_vacuum = fs::get_doc_type(_uCtStorage->get_file_path()) == CtDocType::SQLite;
    _uCtMenu->find_action("ct_vacuum")->signal_set_visible.emit(can_vacuum);

    const auto iterDocsRestore{_pCtConfig->recentDocsRestore.find(filepath.string())};
    switch (_pCtConfig->restoreExpColl)
    {
        case CtRestoreExpColl::ALL_EXP:
        {
            _uCtTreeview->expand_all();
        } break;
        case CtRestoreExpColl::ALL_COLL:
        {
            _uCtTreeview->expand_all();
            _uCtTreestore->treeview_set_tree_expanded_collapsed_string("", *_uCtTreeview, _pCtConfig->nodesBookmExp);
        } break;
        default:
        {
            if (iterDocsRestore != _pCtConfig->recentDocsRestore.end())
            {
                _uCtTreestore->treeview_set_tree_expanded_collapsed_string(iterDocsRestore->second.exp_coll_str, *_uCtTreeview, _pCtConfig->nodesBookmExp);
            }
        } break;
    }

    bool node_is_set = false;
    if (node_to_focus != "") {
        if (CtTreeIter node = get_tree_store().get_node_from_node_name(node_to_focus)) {
            get_tree_store().treeview_safe_set_cursor(_uCtTreeview.get(), node);
            _ctTextview.grab_focus();
            node_is_set = true;
        }
    }

    if ( not _no_gui and
         not node_is_set )
    {
        if (iterDocsRestore != _pCtConfig->recentDocsRestore.end()) {
            _uCtTreestore->treeview_set_tree_path_n_text_cursor(_uCtTreeview.get(),
                                                                iterDocsRestore->second.node_path,
                                                                iterDocsRestore->second.cursor_pos);
        }
        else {
            _uCtTreestore->treeview_set_tree_path_n_text_cursor(_uCtTreeview.get(), "0", 0);
        }
        _ctTextview.grab_focus();
    }

    get_ct_config()->recentDocsFilepaths.move_or_push_front(fs::canonical(filepath));
    menu_set_items_recent_documents();

    return true;
}

bool CtMainWin::file_save_ask_user()
{
    if (get_file_save_needed())
    {
        const CtYesNoCancel yesNoCancel = [this]() {
            if (_pCtConfig->autosaveOnQuit && !_uCtStorage->get_file_path().empty())
                return CtYesNoCancel::Yes;
            set_visible(true);   // window could be hidden
            return CtDialogs::exit_save_dialog(*this);
        }();

        if (CtYesNoCancel::Cancel == yesNoCancel)
        {
            return false;
        }
        if (CtYesNoCancel::Yes == yesNoCancel)
        {
            _uCtActions->file_save();
            if (get_file_save_needed())
            {
                // something went wrong in the save
                return false;
            }
        }
    }
    return true;
}

void CtMainWin::file_save(bool need_vacuum)
{
    if (_uCtStorage->get_file_path().empty())
        return;
    if (!get_file_save_needed())
        if (!need_vacuum)
            return;
    if (!get_tree_store().get_iter_first())
        return;

    Glib::ustring error;
    if (_uCtStorage->save(need_vacuum, error))
    {
        update_window_save_not_needed();
        get_state_machine().update_state();
    }
    else
    {
        CtDialogs::error_dialog(error, *this);
    }
}

void CtMainWin::file_save_as(const std::string& new_filepath, const Glib::ustring& password)
{
    Glib::ustring error;
    std::unique_ptr<CtStorageControl> new_storage(CtStorageControl::save_as(this, new_filepath, password, error));
    if (!new_storage)
    {
        CtDialogs::error_dialog(error, *this);
        return;
    }

    // remember expanded nodes for new file
    CtRecentDocRestore doc_state_restore;
    doc_state_restore.exp_coll_str = _uCtTreestore->treeview_get_tree_expanded_collapsed_string(*_uCtTreeview);
    if (const CtTreeIter curr_iter = curr_tree_iter())
    {
        doc_state_restore.node_path = _uCtTreestore->get_path(curr_iter).to_string();
        doc_state_restore.cursor_pos = curr_iter.get_node_text_buffer()->property_cursor_position();
    }
    _pCtConfig->recentDocsFilepaths.move_or_push_front(fs::canonical(new_filepath));
    _pCtConfig->recentDocsRestore[new_filepath] = doc_state_restore;

    // it' a hack to recover expanded nodes for new file
    auto old_restore = _pCtConfig->restoreExpColl;
    auto on_scope_exit = scope_guard([&](void*) { _pCtConfig->restoreExpColl = old_restore; });
    _pCtConfig->restoreExpColl = CtRestoreExpColl::FROM_STR;

    // instead of setting all inner states, it's easer just to re-open file
    new_storage.reset();               // we don't need it
    update_window_save_not_needed();   // remove asking to save when we close the old file
    file_open(new_filepath, "", password);
}

void CtMainWin::file_autosave_restart()
{
    bool was_connected = !_autosave_timout_connection.empty();
    _autosave_timout_connection.disconnect();
    if (!get_ct_config()->autosaveOn) {
        if (was_connected) spdlog::debug("autosave was stopped");
        return;
    }
    if (get_ct_config()->autosaveVal < 1) {
        CtDialogs::error_dialog("Wrong timeout for autosave", *this);
        return;
    }

    spdlog::debug("autosave is started");
    _autosave_timout_connection = Glib::signal_timeout().connect_seconds([this]() {        
        if (get_file_save_needed()) {
            spdlog::debug("autosave: time to save file");
            file_save(false);
        } else {
            spdlog::debug("autosave: no needs to save file");
        }
        return true;
    }, get_ct_config()->autosaveVal * 60);
}

bool CtMainWin::file_insert_plain_text(const fs::path& filepath)
{
    spdlog::debug("trying to insert text file as node: {}", filepath);

    gchar *text = nullptr;
    gsize length = 0;
    try {
        if (g_file_get_contents (filepath.c_str(), &text, &length, nullptr)) {
            Glib::ustring node_content(text, length);
            node_content = str::sanitize_bad_symbols(node_content);
            std::string name = filepath.filename().string();
            get_ct_actions()->_node_child_exist_or_create(Gtk::TreeIter(), name);
            get_text_view().get_buffer()->insert(get_text_view().get_buffer()->end(), node_content);
            g_free(text);
            return true;
        }
    }  catch (...) {

    }
    g_free(text);
    return false;
}

void CtMainWin::reset()
{
    auto on_scope_exit = scope_guard([&](void*) { user_active() = true; });
    user_active() = false;

    get_state_machine().reset();

    _uCtStorage.reset(CtStorageControl::create_dummy_storage(this));

    _reset_CtTreestore_CtTreeview();

    _latestStatusbarUpdateTime.clear();
    for (auto button: _ctWinHeader.buttonBox.get_children())
        button->hide();
    _ctWinHeader.nameLabel.set_markup("");
    window_header_update_lock_icon(false);
    window_header_update_bookmark_icon(false);
    menu_set_bookmark_menu_items();
    _uCtMenu->find_action("ct_vacuum")->signal_set_visible.emit(false);

    update_window_save_not_needed();
    _ctTextview.set_buffer(Glib::RefPtr<Gtk::TextBuffer>());
    _ctTextview.set_spell_check(false);
    if (not _no_gui) {
        _ctTextview.set_sensitive(false);
    }
}

bool CtMainWin::get_file_save_needed()
{
    return (_fileSaveNeeded or (curr_tree_iter() and curr_tree_iter().get_node_text_buffer()->get_modified()));
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
        if (get_ct_config()->enableSpellCheck && curr_tree_iter().get_node_is_rich_text())
        {
            statusbar_text += separator_text + _("Spell Check") + _(": ") + get_ct_config()->spellCheckLang;
        }
        if (get_ct_config()->wordCountOn)
        {
            statusbar_text += separator_text + _("Word Count") + _(": ") + std::to_string(CtTextIterUtil::get_words_count(get_text_view().get_buffer()));
        }
        if (treeIter.get_node_creating_time() > 0)
        {
            const std::string timestamp_creation = str::time_format(_pCtConfig->timestampFormat, treeIter.get_node_creating_time());
            statusbar_text += separator_text + _("Date Created") + _(": ") + timestamp_creation;
        }
        if (treeIter.get_node_modification_time() > 0)
        {
            const std::string timestamp_lastsave = str::time_format(_pCtConfig->timestampFormat, treeIter.get_node_modification_time());
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
        std::list<CtAnchoredWidget*> anchoredWidgets = treeIter.get_embedded_pixbufs_tables_codeboxes_fast();
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
            if ( (0 == _latestStatusbarUpdateTime.count(node_id)) or
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
            for (auto node_id: rm_node_ids)
                get_state_machine().delete_states(node_id);
        } break;
        case CtSaveNeededUpdType::book:
        {
            _uCtTreestore->pending_edit_db_bookmarks();
        } break;
    }
    if (new_machine_state && treeIter)
        get_state_machine().update_state(treeIter);
}

// Load Text Buffer from State Machine
void CtMainWin::load_buffer_from_state(std::shared_ptr<CtNodeState> state, CtTreeIter tree_iter)
{
    bool user_active_restore = user_active();
    user_active() = false;

    auto text_buffer = tree_iter.get_node_text_buffer();
    Glib::RefPtr<Gsv::Buffer> gsv_buffer = Glib::RefPtr<Gsv::Buffer>::cast_dynamic(text_buffer);

    text_buffer->begin_not_undoable_action();

    // erase is slow on empty buffer
    if (text_buffer->begin() != text_buffer->end())
        text_buffer->erase(text_buffer->begin(), text_buffer->end());
    tree_iter.remove_all_embedded_widgets();
    std::list<CtAnchoredWidget*> widgets;
    for (xmlpp::Node* text_node: state->buffer_xml.get_root_node()->get_children())
    {
        CtStorageXmlHelper(this).get_text_buffer_one_slot_from_xml(gsv_buffer, text_node, widgets, nullptr, -1);
    }

    // xml storage doesn't have widgets, so load them seperatrly
    for (auto widgetState: state->widgetStates)
        widgets.push_back(widgetState->to_widget(this));
    for (auto widget: widgets)
        widget->insertInTextBuffer(gsv_buffer);
    get_tree_store().addAnchoredWidgets(tree_iter, widgets, &get_text_view());

    text_buffer->end_not_undoable_action();
    text_buffer->set_modified(false);

    get_text_view().set_buffer(text_buffer);
    get_text_view().set_spell_check(curr_tree_iter().get_node_is_rich_text());
    text_buffer->place_cursor(text_buffer->get_iter_at_offset(state->cursor_pos));
    get_text_view().scroll_to(text_buffer->get_insert(), CtTextView::TEXT_SCROLL_MARGIN);

    user_active() = user_active_restore;

    update_window_save_needed(CtSaveNeededUpdType::nbuf, false, &tree_iter);
}

// Switch TextBuffer -> SourceBuffer or SourceBuffer -> TextBuffer
void CtMainWin::switch_buffer_text_source(Glib::RefPtr<Gsv::Buffer> text_buffer, CtTreeIter tree_iter, const std::string& new_syntax, const std::string& old_syntax)
{
    if (new_syntax == old_syntax)
        return;

    bool user_active_restore = user_active();
    user_active() = false;

    Glib::ustring node_text;
    if (old_syntax == CtConst::RICH_TEXT_ID)
    {
        node_text = CtExport2Txt(this).node_export_to_txt(tree_iter, "", {0}, -1, -1);
    }
    else
    {
        node_text = text_buffer->get_text();
    }

    auto new_buffer = get_new_text_buffer(node_text);
    tree_iter.set_node_text_buffer(new_buffer, new_syntax);
    _uCtTreestore->text_view_apply_textbuffer(tree_iter, &_ctTextview);

    user_active() = user_active_restore;
}

void CtMainWin::text_view_apply_cursor_position(CtTreeIter& treeIter, const int cursor_pos)
{
    Glib::RefPtr<Gsv::Buffer> rTextBuffer = treeIter.get_node_text_buffer();
    Gtk::TextIter textIter = rTextBuffer->get_iter_at_offset(cursor_pos);
    if (static_cast<bool>(textIter))
    {
        rTextBuffer->place_cursor(textIter);
        // if directly call `scroll_to`, it doesn't work maybe textview is not ready/visible or something else
        Glib::signal_idle().connect_once([&](){
            auto iter = _ctTextview.get_buffer()->get_insert()->get_iter();
            _ctTextview.scroll_to(iter, CtTextView::TEXT_SCROLL_MARGIN);
        });
    }
}

void CtMainWin::_on_treeview_cursor_changed()
{
    if (_prevTreeIter)
    {
        const gint64 prevNodeId = _prevTreeIter.get_node_id();
        if (prevNodeId == curr_tree_iter().get_node_id()) {
            return;
        }
        Glib::RefPtr<Gsv::Buffer> rTextBuffer = _prevTreeIter.get_node_text_buffer();
        if (rTextBuffer->get_modified())
        {
            _fileSaveNeeded = true;
            rTextBuffer->set_modified(false);
            get_state_machine().update_state(_prevTreeIter);
        }
        _nodesCursorPos[prevNodeId] = rTextBuffer->property_cursor_position();
    }
    CtTreeIter treeIter = curr_tree_iter();
    _uCtTreestore->text_view_apply_textbuffer(treeIter, &_ctTextview);
    auto mapIter = _nodesCursorPos.find(treeIter.get_node_id());
    if (mapIter != _nodesCursorPos.end() and mapIter->second > 0) {
        text_view_apply_cursor_position(treeIter, mapIter->second);
    }

    menu_update_bookmark_menu_item(_uCtTreestore->is_node_bookmarked(treeIter.get_node_id()));
    window_header_update();
    window_header_update_lock_icon(treeIter.get_node_read_only());
    window_header_update_bookmark_icon(_uCtTreestore->is_node_bookmarked(treeIter.get_node_id()));
    update_selected_node_statusbar_info();
    get_state_machine().node_selected_changed(treeIter.get_node_id());

    _prevTreeIter = treeIter;
}

bool CtMainWin::_on_treeview_button_release_event(GdkEventButton* event)
{
    if (event->button == 3)
    {
        _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(event->button, event->time);
        return true;
    }

    return false;
}

bool CtMainWin::_on_window_key_press_event(GdkEventKey* event)
{
    if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_Tab) {
            _uCtActions->toggle_tree_text();
            return true;
        }
    }
    return false;
}

void CtMainWin::_on_treeview_event_after(GdkEvent* event)
{
    if (event->type == GDK_BUTTON_PRESS and event->button.button == 1)
    {
        if (get_ct_config()->treeClickFocusText) {
            get_text_view().grab_focus();
        }
        if (get_ct_config()->treeClickExpand)
        {
            _tree_just_auto_expanded = false;
            Gtk::TreePath path_at_click;
            if (get_tree_view().get_path_at_pos((int)event->button.x, (int)event->button.y, path_at_click))
                if (!get_tree_view().row_expanded(path_at_click)) {
                    get_tree_view().expand_row(path_at_click, false);
                    _tree_just_auto_expanded = true;
                }
        }
    }
    if (event->type == GDK_BUTTON_PRESS && event->button.button == 2 /* wheel click */)
    {
        auto path = get_tree_store().get_path(curr_tree_iter());
        if (_uCtTreeview->row_expanded(path))
            _uCtTreeview->collapse_row(path);
        else
            _uCtTreeview->expand_row(path, true);
    }
    else if (event->type == GDK_2BUTTON_PRESS and event->button.button == 1)
    {
        // _on_treeview_row_activated works better for double-click
        // but it doesn't work with one click
        // in this case use real double click
        if (get_ct_config()->treeClickExpand)
        {
            Gtk::TreePath path_at_click;
            if (get_tree_view().get_path_at_pos((int)event->button.x, (int)event->button.y, path_at_click))
                if (path_at_click == get_tree_store().get_path(curr_tree_iter()))
                {
                    if (_uCtTreeview->row_expanded(path_at_click))
                        _uCtTreeview->collapse_row(path_at_click);
                    else
                        _uCtTreeview->expand_row(path_at_click, false);
                }
        }
    }
}

void CtMainWin::_on_treeview_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*)
{
    // _on_treeview_row_activated works better for double-click
    // but it doesn't work with one click
    // in this case use real double click
    if (get_ct_config()->treeClickExpand)
        return;
    if (_uCtTreeview->row_expanded(path))
        _uCtTreeview->collapse_row(path);
    else
        _uCtTreeview->expand_row(path, false);
}

bool CtMainWin::_on_treeview_test_collapse_row(const Gtk::TreeModel::iterator&,const Gtk::TreeModel::Path&)
{
    // to fix one click
    if (get_ct_config()->treeClickExpand)
        if (_tree_just_auto_expanded) {
            _tree_just_auto_expanded = false;
            return true;
        }
    return false;
}

bool CtMainWin::_on_treeview_key_press_event(GdkEventKey* event)
{

    if (not curr_tree_iter()) return false;
    if (event->state & GDK_SHIFT_MASK) {
        if (event->state & GDK_CONTROL_MASK && event->keyval == GDK_KEY_Right) {
            _uCtActions->node_change_father();
            return true;
        }
        else if (event->keyval == GDK_KEY_Up) {
            _uCtActions->node_up();
            return true;
        }
        else if (event->keyval == GDK_KEY_Down) {
            _uCtActions->node_down();
            return true;
        }
        else if (event->keyval == GDK_KEY_Left) {
            _uCtActions->node_left();
            return true;
        }
        else if (event->keyval == GDK_KEY_Right) {
            _uCtActions->node_right();
            return true;
        }
    }
    else if (event->state & GDK_MOD1_MASK) {

    }
    else if (event->state & GDK_CONTROL_MASK) {
        auto reduce = [](Gtk::TreeIter first, std::function<Gtk::TreeIter(Gtk::TreeIter)> operatr) -> Gtk::TreeIter
        {
            Gtk::TreeIter result;
            for (auto next = operatr(first); next; next = operatr(next))
                result = next;
            return result;
        };

        if (event->keyval == GDK_KEY_Up) {
            auto fist_sibling = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return --iter;});
            if (fist_sibling)
                get_tree_view().set_cursor_safe(fist_sibling);
            return true;
        }
        else if (event->keyval == GDK_KEY_Down) {
            auto last_sibling = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return ++iter;});
            if (last_sibling)
                get_tree_view().set_cursor_safe(last_sibling);
            return true;
        }
        else if (event->keyval == GDK_KEY_Left) {
            auto fist_parent = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return iter->parent();});
            if (fist_parent)
                get_tree_view().set_cursor_safe(fist_parent);
            return true;
        }
        else if (event->keyval == GDK_KEY_Right) {
            auto last_child = reduce(curr_tree_iter(), [](Gtk::TreeIter iter) { return iter->children().begin();});
            if (last_child)
                get_tree_view().set_cursor_safe(last_child);
            return true;
        } else {
            if (event->keyval == GDK_KEY_plus || event->keyval == GDK_KEY_KP_Add || event->keyval == GDK_KEY_equal) {
                _zoom_tree(true);
                return true;
            }
            else if (event->keyval == GDK_KEY_minus|| event->keyval == GDK_KEY_KP_Subtract) {
                _zoom_tree(false);
                return true;
            }
        }
    }
    else {
        if (event->keyval == GDK_KEY_Left) {
            if (_uCtTreeview->row_expanded(_uCtTreestore->get_path(curr_tree_iter())))
                _uCtTreeview->collapse_row(_uCtTreestore->get_path(curr_tree_iter()));
            else if (curr_tree_iter().parent())
                get_tree_view().set_cursor_safe(curr_tree_iter().parent());
            return true;
        }
        else if (event->keyval == GDK_KEY_Right) {
            get_tree_view().expand_row(get_tree_store().get_path(curr_tree_iter()), false);
            return true;
        }
        else if (event->keyval == GDK_KEY_Return) {
            auto path = get_tree_store().get_path(curr_tree_iter());
            if (_uCtTreeview->row_expanded(path))
                _uCtTreeview->collapse_row(path);
            else
                _uCtTreeview->expand_row(path, false);
            return true;
        }
        else if (event->keyval == GDK_KEY_Menu) {
            _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(0, event->time);
            return true;
        }
        else if (event->keyval == GDK_KEY_Tab) {
            _uCtActions->toggle_tree_text();
            return true;
        }
        else if (event->keyval == GDK_KEY_Delete) {
            _uCtActions->node_delete();
            return true;
        }
    }
    return false;
}

bool CtMainWin::_on_treeview_popup_menu()
{
    _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Node)->popup(0, 0);
    return true;
}

bool CtMainWin::_on_treeview_scroll_event(GdkEventScroll* event)
{
    if (!(event->state & GDK_CONTROL_MASK))
        return false;
    if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
        _zoom_tree(event->direction == GDK_SCROLL_DOWN);
    if (event->direction == GDK_SCROLL_SMOOTH && event->delta_y != 0)
        _zoom_tree(event->delta_y < 0);
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
                if menuitem.get_image().get_property("stock") == "ct_edit_paste":
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
                        and hovering_link_iter_offset() <= iter_sel_end.get_offset())
                    {
                        do_set_cursor = false;
                    }
                }
                if (do_set_cursor) curr_buffer()->place_cursor(target_iter);
            }
            //for (auto iter : menu->get_children()) menu->remove(*iter);
            get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::Link);
        }
        else {
            //for (auto iter : menu->get_children()) menu->remove(*iter);
            get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::Text);
        }
    }
    else {
        //for (auto iter : menu->get_children()) menu->remove(*iter);
        _uCtActions->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::Code);
    }
}

// Update the cursor image if the pointer moved
bool CtMainWin::_on_textview_motion_notify_event(GdkEventMotion* event)
{
    if (not _ctTextview.get_cursor_visible())
        _ctTextview.set_cursor_visible(true);
    if (curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID
        and curr_tree_iter().get_node_syntax_highlighting() != CtConst::PLAIN_TEXT_ID)
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
    if (curr_tree_iter().get_node_syntax_highlighting() != CtConst::RICH_TEXT_ID and
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
        auto widgets = curr_tree_iter().get_embedded_pixbufs_tables_codeboxes_fast();
        for (auto& widget: widgets)
            if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widget))
                if (not codebox->get_width_in_pixels())
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
        if (event->key.keyval == GDK_KEY_ISO_Left_Tab and !curr_buffer->get_has_selection())
        {
            auto iter_insert = curr_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(this, curr_buffer).get_paragraph_list_info(iter_insert);
            if (list_info and list_info.level)
            {
                get_text_view().list_change_level(iter_insert, list_info, false);
                return true;
            }
        }
    }
    else if (event->key.state & Gdk::CONTROL_MASK and event->key.keyval == GDK_KEY_space)
    {
        auto iter_insert = curr_buffer->get_insert()->get_iter();
        auto widgets = curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(iter_insert.get_offset(), iter_insert.get_offset());
        if (not widgets.empty())
            if (CtCodebox* codebox = dynamic_cast<CtCodebox*>(widgets.front()))
            {
                codebox->get_text_view().grab_focus();
                return true;
            }
        CtListInfo list_info = CtList(this, curr_buffer).get_paragraph_list_info(iter_insert);
        if (list_info and list_info.type == CtListType::Todo)
            if (_uCtActions->_is_curr_node_not_read_only_or_error())
            {
                auto iter_start_list = curr_buffer->get_iter_at_offset(list_info.startoffs + 3*list_info.level);
                CtList(this, curr_buffer).todo_list_rotate_status(iter_start_list);
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
            if (not curr_buffer->get_has_selection()) return false;
            Gtk::TextIter iter_sel_start, iter_sel_end;
            curr_buffer->get_selection_bounds(iter_sel_start, iter_sel_end);
            int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
            if (num_chars != 1) return false;
            auto widgets = curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(iter_sel_start.get_offset(), iter_sel_start.get_offset());
            if (widgets.empty()) return false;
            if (CtImageAnchor* anchor = dynamic_cast<CtImageAnchor*>(widgets.front()))
            {
                _uCtActions->curr_anchor_anchor = anchor;
                _uCtActions->object_set_selection(anchor);
                _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Anchor)->popup(3, event->button.time);
            }
            else if (CtImagePng* image = dynamic_cast<CtImagePng*>(widgets.front()))
            {
                _uCtActions->curr_image_anchor = image;
                _uCtActions->object_set_selection(image);
                _uCtMenu->find_action("img_link_dismiss")->signal_set_visible.emit(not image->get_link().empty());
                _uCtMenu->get_popup_menu(CtMenu::POPUP_MENU_TYPE::Image)->popup(3, event->button.time);
            }
            return true;
        }
    }
    else if (event->key.keyval == GDK_KEY_Tab)
    {
        if (not curr_buffer->get_has_selection())
        {
            auto iter_insert = curr_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(this, curr_buffer).get_paragraph_list_info(iter_insert);
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
            const int num_chars = iter_sel_end.get_offset() - iter_sel_start.get_offset();
            if (num_chars != 1)
            {
                return false;
            }
            auto widgets = curr_tree_iter().get_embedded_pixbufs_tables_codeboxes(iter_sel_start.get_offset(), iter_sel_start.get_offset());
            if (widgets.empty())
            {
                return false;
            }
            if (dynamic_cast<CtTable*>(widgets.front()))
            {
                curr_buffer->place_cursor(iter_sel_end);
                get_text_view().grab_focus();
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else if (event->key.state & Gdk::CONTROL_MASK)
    {
        if (event->key.keyval == GDK_KEY_plus || event->key.keyval == GDK_KEY_KP_Add || event->key.keyval == GDK_KEY_equal) {
            _ctTextview.zoom_text(true, curr_tree_iter().get_node_syntax_highlighting());
            return true;
        }
        else if (event->key.keyval == GDK_KEY_minus|| event->key.keyval == GDK_KEY_KP_Subtract) {
            _ctTextview.zoom_text(false, curr_tree_iter().get_node_syntax_highlighting());
            return true;
        }
    }
    return false;
}

// Called after every event on the SourceView
void CtMainWin::_on_textview_event_after(GdkEvent* event)
{
    if (event->type == GDK_2BUTTON_PRESS and event->button.button == 1)
    {
        get_text_view().for_event_after_double_click_button1(event);
    }
    if (event->type == GDK_3BUTTON_PRESS and event->button.button == 1)
    {
        if (curr_tree_iter().get_node_is_rich_text() and _pCtConfig->tripleClickParagraph)
            get_text_view().for_event_after_triple_click_button1(event);
    }
    else if (event->type == GDK_BUTTON_PRESS or event->type == GDK_KEY_PRESS)
    {
        if (curr_tree_iter() and not curr_buffer()->get_modified())
        {
            get_state_machine().update_curr_state_cursor_pos(curr_tree_iter().get_node_id());
        }
        if (event->type == GDK_BUTTON_PRESS)
        {
            get_text_view().for_event_after_button_press(event);
        }
        if (event->type == GDK_KEY_PRESS)
        {
            get_text_view().for_event_after_key_press(event, curr_tree_iter().get_node_syntax_highlighting());
        }
    }
    else if (event->type == GDK_KEY_RELEASE)
    {
        if (event->key.keyval == GDK_KEY_Return or event->key.keyval == GDK_KEY_space)
        {
            if (_pCtConfig->wordCountOn)
            {
                update_selected_node_statusbar_info();
            }
        }
    }
}

bool CtMainWin::_on_textview_scroll_event(GdkEventScroll* event)
{
    if (!(event->state & GDK_CONTROL_MASK))
        return false;
    if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
        _ctTextview.zoom_text(event->direction == GDK_SCROLL_DOWN, curr_tree_iter().get_node_syntax_highlighting());
    if (event->direction == GDK_SCROLL_SMOOTH && event->delta_y != 0)
        _ctTextview.zoom_text(event->delta_y < 0, curr_tree_iter().get_node_syntax_highlighting());
    return true;
}

void CtMainWin::_title_update(const bool saveNeeded)
{
    Glib::ustring title;
    if (saveNeeded)
    {
        title += "*";
    }
    if (_uCtStorage->get_file_path() != "")
    {
        title += _uCtStorage->get_file_name().string() + " - " + _uCtStorage->get_file_dir().string() + " - ";
    }
    title += "CherryTree ";
    title += CtConst::CT_VERSION;
    set_title(title);
}
