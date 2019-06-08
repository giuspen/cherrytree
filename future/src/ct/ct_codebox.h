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
#include "ct_main_win.h"

enum class CtPixTabCBox : int {Pixbuf=0, Table=1, CodeBox=2};

class CtAnchoredWidget : public Gtk::EventBox
{
public:
    CtAnchoredWidget(const int& charOffset, const std::string& justification);
    void insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer);
    Glib::RefPtr<Gtk::TextChildAnchor> getTextChildAnchor() { return _rTextChildAnchor; }
    virtual void applyWidthHeight(int parentTextWidth) {}
    void updateOffset(int charOffset) { _charOffset = charOffset; }
    void updateJustification(std::string justification) { _justification = justification; }

    int getOffset() { return _charOffset; }
    std::string getJustification() { return _justification; }

protected:
    Gtk::Frame _frame;
    Gtk::Label _labelWidget;
    int _charOffset;
    std::string _justification;
    Glib::RefPtr<Gtk::TextChildAnchor> _rTextChildAnchor;
};

class CtTextCell
{
public:
    CtTextCell(const Glib::ustring& textContent,
               const Glib::ustring& syntaxHighlighting);
    virtual ~CtTextCell();

protected:
    Glib::ustring _syntaxHighlighting;
    Glib::RefPtr<Gsv::Buffer> _rTextBuffer{nullptr};
    CtTextView _ctTextview;
};

class CtCodebox : public CtAnchoredWidget, public CtTextCell
{
public:
    static const Gsv::DrawSpacesFlags DRAW_SPACES_FLAGS;

public:
    CtCodebox(const Glib::ustring& textContent,
              const Glib::ustring& syntaxHighlighting,
              const int& frameWidth,
              const int& frameHeight,
              const int& charOffset,
              const std::string& justification);
    virtual ~CtCodebox();

    virtual void applyWidthHeight(int parentTextWidth);
    void setWidthInPixels(const bool& widthInPixels) { _widthInPixels = widthInPixels; }
    void setHighlightBrackets(const bool& highlightBrackets);
    void setShowLineNumbers(const bool& showLineNumbers);
    void applyCursorPos(const int& cursorPos);

protected:
    int _frameWidth;
    int _frameHeight;
    bool _widthInPixels{true};
    Gtk::ScrolledWindow _scrolledwindow;
};
