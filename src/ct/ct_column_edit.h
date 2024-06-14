/*
 * ct_column_edit.h
 *
 * Copyright 2009-2024
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
#include <utility>
#include <atomic>
#include <vector>
#include <mutex>

enum class CtColEditState { Off, Selection, PrEdit, Edit };

class CtColumnEdit
{
public:
    using StateOnOffCallback = std::function<void(const bool)>;
    using NewCursorRowColCallback = std::function<void(const int, const int)>;
    CtColumnEdit(Gtk::TextView& textView);

    CtColEditState get_state() const { return _state; }
    void register_on_off_callback(StateOnOffCallback stateOnOffCallback) { _stateOnOffCallback = stateOnOffCallback; }
    void register_new_cursor_row_col_callback(NewCursorRowColCallback newCursorRowColCallback) { _newCursorRowColCallback = newCursorRowColCallback; }
    Glib::ustring copy() const;
    Glib::ustring cut();
    void paste(const std::string& column_txt);

    void selection_update();
    void button_control_changed(const bool isDown) { _ctrlDown = isDown; }
    void button_alt_changed(const bool isDown) { _altDown = isDown; }
    void button_1_released();
    void text_inserted(const Gtk::TextIter& pos, const Glib::ustring& text);
    void text_removed(const Gtk::TextIter& range_start, const Gtk::TextIter& range_end);
    void column_mode_off();
    bool get_ctrl_down() { return _ctrlDown; }
    bool get_alt_down() { return _altDown; }
    bool get_own_insert_delete_active() { return _myOwnInsertDelete; }

private:
    Gdk::Point _get_point(const Gtk::TextIter& textIter);
    Gdk::Point _get_cursor_place();
    Gdk::Point _get_cursor_column_mode_place();
    void _clear_marks(const bool alsoStart = true);
    void _predit_to_edit();
    void _predit_to_edit_iter(Gtk::TextIter& iterStart, Gtk::TextIter& iterEnd, bool& firstLine);
    void _edit_insert_delete(const bool isInsert);
    bool _enforce_cursor_column_mode_place();

    Glib::ustring _lastInsertedText;
    Gdk::Point _lastInsertedPoint;
    Glib::ustring _lastRemovedText;
    Gdk::Point _lastRemovedPoint;
    int _lastRemovedDeltaOffset;
    std::mutex _mutexLastInOut;

    Gtk::TextView& _textView;
    StateOnOffCallback _stateOnOffCallback;
    NewCursorRowColCallback _newCursorRowColCallback;
    std::atomic<CtColEditState> _state{CtColEditState::Off};
    std::atomic<bool> _ctrlDown{false};
    std::atomic<bool> _altDown{false};
    std::atomic<bool> _myOwnInsertDelete{false};
    Gdk::Point _pointStart{-1,-1};
    Gdk::Point _pointEnd{-1,-1};
    std::vector<Glib::RefPtr<Gtk::TextMark>> _marksStart;
    std::vector<Glib::RefPtr<Gtk::TextMark>> _marksEnd;
};
