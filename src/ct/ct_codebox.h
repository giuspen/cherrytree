/*
 * ct_codebox.h
 *
 * Copyright 2009-2023
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

#pragma once

#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include "ct_const.h"
#include "ct_widgets.h"
#include "ct_text_view.h"

class CtTextCell
{
public:
    CtTextCell(CtMainWin* pCtMainWin,
               const Glib::ustring& textContent,
               const std::string& syntaxHighlighting);
    virtual ~CtTextCell() {}

    Glib::ustring get_text_content() const;
    Glib::RefPtr<Gsv::Buffer> get_buffer() const { return _rTextBuffer; }
    CtTextView& get_text_view() { return _ctTextview; }
    const std::string& get_syntax_highlighting() const { return _syntaxHighlighting; }
    void set_syntax_highlighting(const std::string& syntaxHighlighting, Gsv::LanguageManager* pGsvLanguageManager);
    void set_text_buffer_modified_false() { _rTextBuffer->set_modified(false); }

protected:
    std::string _syntaxHighlighting;
    Glib::RefPtr<Gsv::Buffer> _rTextBuffer{nullptr};
    CtTextView _ctTextview;
    std::unique_ptr<CtPairCodeboxMainWin> _uCtPairCodeboxMainWin;
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
    CtCodebox(CtMainWin* pCtMainWin,
              const Glib::ustring& textContent,
              const std::string& syntaxHighlighting,
              const int frameWidth,
              const int frameHeight,
              const int charOffset,
              const std::string& justification,
              const bool widthInPixels,
              const bool highlightBrackets,
              const bool showLineNumbers);

    void apply_width_height(const int parentTextWidth) override;
    void apply_syntax_highlighting(const bool forceReApply) override;
    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache, const std::string& multifile_dir) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;
    void set_modified_false() override { set_text_buffer_modified_false(); }
    CtAnchWidgType get_type() const override { return CtAnchWidgType::CodeBox; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    void set_width_height(int newWidth, int newHeight);
    void set_width_in_pixels(const bool widthInPixels) { _widthInPixels = widthInPixels; }
    void set_highlight_brackets(const bool highlightBrackets);
    void set_show_line_numbers(const bool showLineNumbers);
    void apply_cursor_pos(const int cursorPos);

    bool get_width_in_pixels() const { return _widthInPixels; }
    int  get_frame_width() const {
        if (_widthInPixels and _ctTextview.get_allocated_width() > _frameWidth) {
            return _ctTextview.get_allocated_width();
        }
        return _frameWidth;
    }
    int  get_frame_height() const { return _frameHeight; }
    bool get_highlight_brackets() const { return _highlightBrackets; }
    bool get_show_line_numbers() const { return _showLineNumbers; }

private:
    bool _on_key_press_event(GdkEventKey* event);

private:
    int _frameWidth;
    int _frameHeight;
    bool _widthInPixels{true};
    bool _highlightBrackets{true};
    bool _showLineNumbers{false};
    Gtk::ScrolledWindow _scrolledwindow;
};
