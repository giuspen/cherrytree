/*
 * ct_widgets.h
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
#include <libxml++/libxml++.h>

class CtAnchoredWidget : public Gtk::EventBox
{
public:
    CtAnchoredWidget(const int charOffset, const std::string& justification);
    virtual ~CtAnchoredWidget();

    void insertInTextBuffer(Glib::RefPtr<Gsv::Buffer> rTextBuffer);
    Glib::RefPtr<Gtk::TextChildAnchor> getTextChildAnchor() { return _rTextChildAnchor; }

    virtual void applyWidthHeight(const int parentTextWidth) = 0;
    virtual void to_xml(xmlpp::Element* /*p_node_parent*/, const int offset_adjustment) { _charOffset += offset_adjustment; }

    void updateOffset(int charOffset) { _charOffset = charOffset; }
    void updateJustification(std::string justification) { _justification = justification; }

    int getOffset() { return _charOffset; }
    const std::string& getJustification() { return _justification; }

protected:
    Gtk::Frame _frame;
    Gtk::Label _labelWidget;
    int _charOffset;
    std::string _justification;
    Glib::RefPtr<Gtk::TextChildAnchor> _rTextChildAnchor;
};


class CtTreeView : public Gtk::TreeView
{
public:
    CtTreeView();
    virtual ~CtTreeView();
    void set_cursor_safe(const Gtk::TreeIter& iter);

protected:
};


class CtTextView : public Gsv::View
{
public:
    CtTextView();
    virtual ~CtTextView();

    void setupForSyntax(const std::string& syntaxHighlighting);
    void set_pixels_inside_wrap(int space_around_lines, int relative_wrapped_space);
    void set_selection_at_offset_n_delta(int offset, int delta, Glib::RefPtr<Gtk::TextBuffer> text_buffer = Glib::RefPtr<Gtk::TextBuffer>());

public:
    static const double TEXT_SCROLL_MARGIN;

protected:
    void _setFontForSyntax(const std::string& syntaxHighlighting);
};
