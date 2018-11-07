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

CtCodebox::CtCodebox(const Glib::ustring& textContent, const Glib::ustring& syntaxHighlighting, const int& frameWidth, const int& frameHeight)
 : _textContent(textContent),
   _syntaxHighlighting(syntaxHighlighting),
   _frameWidth(frameWidth),
   _frameHeight(frameHeight)
{
    _rTextBuffer = Gsv::Buffer::create(CtApp::R_textTagTable);
    _rTextBuffer->set_max_undo_levels(CtApp::P_ctCfg->limitUndoableSteps);
    _rTextBuffer->set_style_scheme(CtApp::R_styleSchemeManager->get_scheme(CtApp::P_ctCfg->styleSchemeId));
    if (0 == _syntaxHighlighting.compare(CtConst::PLAIN_TEXT_ID))
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
}

void CtCodebox::setHighlightBrackets(const bool& highlightBrackets)
{ 
    _highlightBrackets = highlightBrackets;
    _rTextBuffer->set_highlight_matching_brackets(_highlightBrackets);
}

void CtCodebox::insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer, const int& charOffset, const Glib::ustring& justification)
{
    Gtk::TextIter textIter = rTextBuffer->get_iter_at_offset(charOffset);
    Glib::RefPtr<Gtk::TextChildAnchor> rTextChildAnchor = rTextBuffer->create_child_anchor(textIter);

    _rTextBuffer->place_cursor(_rTextBuffer->get_iter_at_offset(charOffset));
}
