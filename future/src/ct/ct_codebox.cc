/*
 * ct_codebox.cc
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

#include "ct_codebox.h"
#include "ct_app.h"
#include "ct_list.h"


CtTextCell::CtTextCell(const Glib::ustring& textContent,
                       const Glib::ustring& syntaxHighlighting)
 : _syntaxHighlighting(syntaxHighlighting)
{
    _rTextBuffer = CtMiscUtil::getNewTextBuffer(syntaxHighlighting, textContent);
    _ctTextview.set_buffer(_rTextBuffer);
    _ctTextview.setupForSyntax(_syntaxHighlighting);
    // todo: anchor.sourceview.modify_font(pango.FontDescription(self.dad.code_font))
}

CtTextCell::~CtTextCell()
{
}

Glib::ustring CtTextCell::getTextContent() const
{
    Gtk::TextIter start_iter = _rTextBuffer->begin();
    Gtk::TextIter end_iter = _rTextBuffer->end();
    return start_iter.get_text(end_iter);
}


const Gsv::DrawSpacesFlags CtCodebox::DRAW_SPACES_FLAGS = Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE;

CtCodebox::CtCodebox(const Glib::ustring& textContent,
                     const Glib::ustring& syntaxHighlighting,
                     const int frameWidth,
                     const int frameHeight,
                     const int charOffset,
                     const std::string& justification)
 : CtAnchoredWidget(charOffset, justification),
   CtTextCell(textContent, syntaxHighlighting),
   _frameWidth(frameWidth),
   _frameHeight(frameHeight),
   _key_down(false)
{
    _scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindow.add(_ctTextview);
    _frame.add(_scrolledwindow);
    show_all();

    _ctTextview.signal_populate_popup().connect([this](Gtk::Menu* menu){
        if (!CtApp::P_ctActions->getCtMainWin()->user_active()) return;
        CtApp::P_ctActions->curr_codebox_anchor = this;
        CtApp::P_ctActions->getCtMainWin()->get_ct_menu().build_popup_menu(GTK_WIDGET(menu->gobj()), CtMenu::POPUP_MENU_TYPE::Codebox);
    });

    _ctTextview.signal_key_press_event().connect(sigc::mem_fun(*this, &CtCodebox::_onKeyPressEvent), false);
    /*&anchor.sourceview.connect('key_press_event', self.on_key_press_sourceview_codebox, anchor)
    anchor.sourceview.connect('key_release_event', self.on_key_release_sourceview_codebox, anchor)
    anchor.sourceview.connect('button-press-event', self.on_mouse_button_clicked_codebox, anchor)
    anchor.sourceview.connect("event-after", self.on_sourceview_event_after_codebox, anchor)
    anchor.sourceview.connect("motion-notify-event", self.on_sourceview_motion_notify_event_codebox)
    anchor.sourceview.connect("copy-clipboard", self.dad.clipboard_handler.copy, True)
    anchor.sourceview.connect("cut-clipboard", self.dad.clipboard_handler.cut, True)
            */
}

CtCodebox::~CtCodebox()
{
}

void CtCodebox::applyWidthHeight(const int parentTextWidth)
{
    int frameWidth = _widthInPixels ? _frameWidth : parentTextWidth*_frameWidth/100;
    _scrolledwindow.set_size_request(frameWidth, _frameHeight);
}

void CtCodebox::to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment)
{
    CtAnchoredWidget::to_xml(p_node_parent, offset_adjustment);
    xmlpp::Element* p_codebox_node = p_node_parent->add_child("codebox");
    p_codebox_node->set_attribute("char_offset", std::to_string(_charOffset));
    p_codebox_node->set_attribute(CtConst::TAG_JUSTIFICATION, _justification);
    p_codebox_node->set_attribute("frame_width", std::to_string(_frameWidth));
    p_codebox_node->set_attribute("frame_height", std::to_string(_frameHeight));
    p_codebox_node->set_attribute("width_in_pixels", std::to_string(_widthInPixels));
    p_codebox_node->set_attribute("syntax_highlighting", _syntaxHighlighting);
    p_codebox_node->set_attribute("highlight_brackets", std::to_string(_highlightBrackets));
    p_codebox_node->set_attribute("show_line_numbers", std::to_string(_showLineNumbers));
    p_codebox_node->add_child_text(getTextContent());
}

void CtCodebox::setShowLineNumbers(const bool showLineNumbers)
{
    _showLineNumbers = showLineNumbers;
    _ctTextview.set_show_line_numbers(_showLineNumbers);
}

void CtCodebox::setWidthHeight(int newWidth, int newHeight)
{
    if (newWidth) _frameWidth = newWidth;
    if (newHeight) _frameHeight = newHeight;
    applyWidthHeight(CtApp::P_ctActions->getCtMainWin()->get_allocation().get_width());
}

void CtCodebox::setHighlightBrackets(const bool highlightBrackets)
{
    _highlightBrackets = highlightBrackets;
    _rTextBuffer->set_highlight_matching_brackets(_highlightBrackets);
}

void CtCodebox::applyCursorPos(const int cursorPos)
{
    if (cursorPos > 0)
    {
        _rTextBuffer->place_cursor(_rTextBuffer->get_iter_at_offset(cursorPos));
    }
}

// Catches CodeBox Key Presses
bool CtCodebox::_onKeyPressEvent(GdkEventKey* event)
{
    _key_down = true;
    if (!CtApp::P_ctActions->getCtMainWin()->user_active())
        return false;
    if (event->state & Gdk::CONTROL_MASK)
    {
        CtApp::P_ctActions->curr_codebox_anchor = this;
        if (event->keyval == GDK_KEY_period)
        {
            if (event->state & Gdk::MOD1_MASK)
                CtApp::P_ctActions->codebox_decrease_width();
            else
                CtApp::P_ctActions->codebox_increase_width();
        }
        else if (event->keyval == GDK_KEY_comma)
        {
            if (event->state & Gdk::MOD1_MASK)
                CtApp::P_ctActions->codebox_decrease_height();
            else
                CtApp::P_ctActions->codebox_increase_height();
        }
        else if (event->keyval == GDK_KEY_space)
        {
            Gtk::TextIter text_iter = CtApp::P_ctActions->getCtMainWin()->get_text_view().get_buffer()->get_iter_at_child_anchor(getTextChildAnchor());
            text_iter.forward_char();
            CtApp::P_ctActions->getCtMainWin()->get_text_view().get_buffer()->place_cursor(text_iter);
            CtApp::P_ctActions->getCtMainWin()->get_text_view().grab_focus();
        }
    }
    if (event->keyval == GDK_KEY_ISO_Left_Tab)
    {
        auto text_buffer = _ctTextview.get_buffer();
        if (!text_buffer->get_has_selection())
        {
            Gtk::TextIter iter_insert = text_buffer->get_insert()->get_iter();
            CtListInfo list_info = CtList(text_buffer).get_paragraph_list_info(iter_insert);
            bool backward = event->state & Gdk::SHIFT_MASK;
            if (list_info)
            {
                if (backward && list_info.level)
                {
                    //support.on_sourceview_list_change_level(self.dad, iter_insert, list_info, text_buffer, False)
                    return true;
                }
                else if (!backward)
                {
                    //support.on_sourceview_list_change_level(self.dad, iter_insert, list_info, text_buffer, True)
                    return true;
                }
            }
        }
    }

    return false;
}

