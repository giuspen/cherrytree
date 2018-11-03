/*
 * ct_codebox.h
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

#pragma once

#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include "ct_const.h"

class CtCodebox
{
public:
    CtCodebox(const Glib::ustring& textContent, const Glib::ustring& syntaxHighlighting, const int& frameWidth, const int& frameHeight);
    virtual ~CtCodebox() {}

    void insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer, const int& charOffset, const Glib::ustring& justification);

    void setWidthInPixels(const bool& widthInPixels) { _widthInPixels = widthInPixels; }
    void setHighlightBrackets(const bool& highlightBrackets) { _highlightBrackets = highlightBrackets; }
    void setShowLineNumbers(const bool& showLineNumbers) { _showLineNumbers = showLineNumbers; }

protected:
    Glib::ustring _textContent;
    Glib::ustring _syntaxHighlighting;
    int _frameWidth;
    int _frameHeight;
    bool _widthInPixels{true};
    bool _highlightBrackets{true};
    bool _showLineNumbers{false};
};
