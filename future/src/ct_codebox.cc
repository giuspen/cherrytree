/*
 * ct_codebox.cc
 * 
 * Copyright 2018 Giuseppe Penone <giuspen@gmail.com>
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

#include "ct_codebox.h"
#include "ct_app.h"

CtTextCell::CtTextCell(const Glib::ustring& textContent,
                       const Glib::ustring& syntaxHighlighting)
 : _syntaxHighlighting(syntaxHighlighting)
{
    _rTextBuffer = Gsv::Buffer::create(CtApp::R_textTagTable);
    _rTextBuffer->set_max_undo_levels(CtApp::P_ctCfg->limitUndoableSteps);
    _rTextBuffer->set_style_scheme(CtApp::R_styleSchemeManager->get_scheme(CtApp::P_ctCfg->styleSchemeId));
    if (CtConst::PLAIN_TEXT_ID == _syntaxHighlighting)
    {
        _rTextBuffer->set_highlight_syntax(false);
    }
    else
    {
        _rTextBuffer->set_language(CtApp::R_languageManager->get_language(_syntaxHighlighting));
        _rTextBuffer->set_highlight_syntax(true);
    }
    if (!textContent.empty())
    {
        _rTextBuffer->begin_not_undoable_action();
        _rTextBuffer->set_text(textContent);
        _rTextBuffer->end_not_undoable_action();
        _rTextBuffer->set_modified(false);
    }
    _ctTextview.set_highlight_current_line(CtApp::P_ctCfg->ptHighlCurrLine);
    if (CtApp::P_ctCfg->ptShowWhiteSpaces)
    {
        _ctTextview.set_draw_spaces(Gsv::DRAW_SPACES_ALL & ~Gsv::DRAW_SPACES_NEWLINE);
    }
    _ctTextview.setFontForSyntax(_syntaxHighlighting);
    _ctTextview.set_buffer(_rTextBuffer);
}

CtTextCell::~CtTextCell()
{
}


CtCodebox::CtCodebox(const Glib::ustring& textContent,
                     const Glib::ustring& syntaxHighlighting,
                     const int& frameWidth,
                     const int& frameHeight,
                     const int& charOffset,
                     const std::string& justification)
 : _frameWidth(frameWidth),
   _frameHeight(frameHeight),
   CtAnchoredWidget(charOffset, justification),
   CtTextCell(textContent, syntaxHighlighting)
{
    _scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _scrolledwindow.add(_ctTextview);
    _frame.add(_scrolledwindow);
    show_all();
}

CtCodebox::~CtCodebox()
{
}

void CtCodebox::applyWidthHeight(int parentTextWidth)
{
    int frameWidth = _widthInPixels ? _frameWidth : parentTextWidth*_frameWidth/100;
    _scrolledwindow.set_size_request(frameWidth, _frameHeight);
}

void CtCodebox::setShowLineNumbers(const bool& showLineNumbers)
{
    _ctTextview.set_show_line_numbers(showLineNumbers);
}

void CtCodebox::setHighlightBrackets(const bool& highlightBrackets)
{ 
    _rTextBuffer->set_highlight_matching_brackets(highlightBrackets);
}

void CtCodebox::applyCursorPos(const int& cursorPos)
{
    if (cursorPos > 0)
    {
        _rTextBuffer->place_cursor(_rTextBuffer->get_iter_at_offset(cursorPos));
    }
}
