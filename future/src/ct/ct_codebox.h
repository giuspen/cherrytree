/*
 * ct_codebox.h
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

#pragma once

#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include "ct_const.h"
#include "ct_widgets.h"


enum class CtPixTabCBox : int {Pixbuf=0, Table=1, CodeBox=2};

class CtTextCell
{
public:
    CtTextCell(const Glib::ustring& textContent,
               const Glib::ustring& syntaxHighlighting);
    virtual ~CtTextCell();

    Glib::ustring getTextContent() const;
    Glib::RefPtr<Gsv::Buffer> getBuffer() { return _rTextBuffer; }

protected:
    Glib::ustring _syntaxHighlighting;
    Glib::RefPtr<Gsv::Buffer> _rTextBuffer{nullptr};
    CtTextView _ctTextview;
};

class CtCodebox : public CtAnchoredWidget, public CtTextCell
{
public:
    static const Gsv::DrawSpacesFlags DRAW_SPACES_FLAGS;

    enum { CB_WIDTH_HEIGHT_STEP_PIX = 15,
           CB_WIDTH_HEIGHT_STEP_PERC = 9,
           CB_WIDTH_LIMIT_MIN = 40,
           CB_HEIGHT_LIMIT_MIN = 30
         };

public:
    CtCodebox(const Glib::ustring& textContent,
              const Glib::ustring& syntaxHighlighting,
              const int frameWidth,
              const int frameHeight,
              const int charOffset,
              const std::string& justification);
    virtual ~CtCodebox();

    virtual void applyWidthHeight(const int parentTextWidth);
    virtual void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment);

    void setWidthHeight(int newWidth, int newHeight);
    void setWidthInPixels(const bool widthInPixels) { _widthInPixels = widthInPixels; }
    void setHighlightBrackets(const bool highlightBrackets);
    void setShowLineNumbers(const bool showLineNumbers);
    void applyCursorPos(const int cursorPos);

    bool getWidthInPixels() { return _widthInPixels; }
    int  getFrameWidth() { return _frameWidth; }
    int  getFrameHeight() { return _frameHeight; }

private:
    bool _onKeyPressEvent(GdkEventKey* event);

protected:
    int _frameWidth;
    int _frameHeight;
    bool _widthInPixels{true};
    bool _highlightBrackets{true};
    bool _showLineNumbers{false};
    Gtk::ScrolledWindow _scrolledwindow;
    bool _key_down;
};
