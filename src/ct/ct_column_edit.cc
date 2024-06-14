/*
 * ct_column_edit.cc
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

#include "ct_column_edit.h"
#include "ct_misc_utils.h"
#include "ct_filesystem.h"
#include "ct_const.h"

CtColumnEdit::CtColumnEdit(Gtk::TextView& textView)
 : _textView{textView}
{
}

Gdk::Point CtColumnEdit::_get_point(const Gtk::TextIter& textIter)
{
    return Gdk::Point{textIter.get_line_offset(), textIter.get_line()};
}

Gdk::Point CtColumnEdit::_get_cursor_column_mode_place()
{
    if (_marksEnd.size() > 0) {
        _pointEnd = _get_point(_marksEnd.back()->get_iter());
    }
    else if (_marksStart.size()) {
        _pointEnd = _get_point(_marksStart.back()->get_iter());
    }
    if (_marksStart.size()) {
        _pointStart = _get_point(_marksStart.front()->get_iter());
    }
    return Gdk::Point{_pointEnd.get_x(), _pointStart.get_y()};
}

Gdk::Point CtColumnEdit::_get_cursor_place()
{
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    Gtk::TextIter iterInsert = rTextBuffer->get_insert()->get_iter();
    return _get_point(iterInsert);
}

bool CtColumnEdit::_enforce_cursor_column_mode_place()
{
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    Gdk::Point cursorPlace = _get_cursor_column_mode_place();
    Gtk::TextIter currIter = rTextBuffer->get_iter_at_line_offset(cursorPlace.get_y(), cursorPlace.get_x());
    if (currIter) {
        rTextBuffer->place_cursor(currIter);
        return true;
    }
    return false;
}

void CtColumnEdit::_clear_marks(const bool alsoStart)
{
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    while (not _marksEnd.empty()) {
        _marksEnd.back()->set_visible(false);
        rTextBuffer->delete_mark(_marksEnd.back());
        _marksEnd.pop_back();
    }
    if (alsoStart) {
        while (not _marksStart.empty()) {
            _marksStart.back()->set_visible(false);
            rTextBuffer->delete_mark(_marksStart.back());
            _marksStart.pop_back();
        }
    }
}

void CtColumnEdit::_predit_to_edit_iter(Gtk::TextIter& iterStart, Gtk::TextIter& iterEnd, bool& firstLine)
{
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    if (firstLine) {
        firstLine = false;
        const unsigned delta_x = abs(iterEnd.get_line_offset() - iterStart.get_line_offset());
        _lastInsertedPoint.set_x(_lastInsertedPoint.get_x() - delta_x);
        _lastRemovedPoint.set_x(_lastRemovedPoint.get_x() - delta_x);
    }
    _myOwnInsertDelete = true;
    rTextBuffer->erase(iterStart, iterEnd);
    _myOwnInsertDelete = false;
}

void CtColumnEdit::_predit_to_edit()
{
    bool unexpected{false};
    if (CtColEditState::PrEdit == _state) {
        _state = CtColEditState::Edit;
        bool firstLine{true};
        size_t currEndIdx{0};
        for (Glib::RefPtr<Gtk::TextMark>& markStart : _marksStart) {
            if (markStart) {
                Gtk::TextIter iterStart = markStart->get_iter();
                if (iterStart) {
                    bool hasMatchingEnd{false};
                    if ( currEndIdx < _marksEnd.size() and
                         not _marksEnd.at(currEndIdx) )
                    {
                        unexpected = true;
                    }
                    else {
                        Gtk::TextIter iterEnd = _marksEnd.at(currEndIdx)->get_iter();
                        if (iterEnd) {
                            if (iterEnd.get_line() == iterStart.get_line()) {
                                hasMatchingEnd = true;
                                _predit_to_edit_iter(iterStart, iterEnd, firstLine);
                                ++currEndIdx;
                            }
                        }
                        else {
                            unexpected = true;
                        }
                    }
                    if (not hasMatchingEnd) {
                        Gtk::TextIter iterEnd = iterStart;
                        if (iterEnd.forward_to_line_end()){
                            _predit_to_edit_iter(iterStart, iterEnd, firstLine);
                        }
                    }
                }
                else {
                    unexpected = true;
                }
            }
            else {
                unexpected = true;
            }
            if (unexpected) {
                break;
            }
        }
    }
    else {
        unexpected = true;
    }
    if (unexpected) {
        column_mode_off();
    }
    else {
        _clear_marks(false/*alsoStart*/);
    }
}

Glib::ustring CtColumnEdit::copy() const
{
    Glib::ustring ret_txt;
    const size_t num_rows = _marksStart.size();
    if (_marksEnd.size() == num_rows) {
        for (size_t r = 0u; r < num_rows; ++r) {
            auto pMarkStart = _marksStart.at(r);
            auto pMarkEnd = _marksEnd.at(r);
            if (pMarkStart and pMarkEnd) {
                Gtk::TextIter iterStart = pMarkStart->get_iter();
                Gtk::TextIter iterEnd = pMarkEnd->get_iter();
                if (iterStart and iterEnd) {
                    if (r > 0u) {
                        ret_txt += CtConst::CHAR_NEWLINE;
                    }
                    ret_txt += iterStart.get_text(iterEnd);
                }
            }
        }
    }
    return ret_txt;
}

Glib::ustring CtColumnEdit::cut()
{
    Glib::ustring ret_txt = copy();
    _predit_to_edit();
    return ret_txt;
}

void CtColumnEdit::paste(const std::string& column_txt)
{
    const std::vector<std::string> vec_col_rows = str::split(column_txt, "\n");
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    Gtk::TextIter iterInsert = rTextBuffer->get_insert()->get_iter();
    const int insert_x{iterInsert.get_line_offset()};
    int insert_y{iterInsert.get_line()};
    for (const std::string& col_row : vec_col_rows) {
        if (col_row.empty()) {
            break;
        }
        //spdlog::debug("{}", col_row);
        iterInsert = rTextBuffer->get_iter_at_line_offset(insert_y, insert_x);
        if (not iterInsert or iterInsert.get_line_offset() != insert_x or iterInsert.get_line() != insert_y) {
            Gtk::TextIter iterLineCurrX = rTextBuffer->get_iter_at_line_offset(insert_y, 0);
            if (not iterLineCurrX or iterLineCurrX.get_line() != insert_y) {
                rTextBuffer->insert(rTextBuffer->end(), CtConst::CHAR_NEWLINE);
                iterLineCurrX = rTextBuffer->get_iter_at_line_offset(insert_y, 0);
                if (not iterLineCurrX or iterLineCurrX.get_line() != insert_y) {
                    spdlog::error("!! iter ({},0)", insert_y);
                    return;
                }
            }
            int curr_x{0};
            while (curr_x < insert_x) {
                ++curr_x;
                iterLineCurrX = rTextBuffer->get_iter_at_line_offset(insert_y, curr_x);
                if (not iterLineCurrX or iterLineCurrX.get_line_offset() != curr_x) {
                    rTextBuffer->insert(rTextBuffer->get_iter_at_line_offset(insert_y, 0), CtConst::CHAR_SPACE);
                    iterLineCurrX = rTextBuffer->get_iter_at_line_offset(insert_y, curr_x);
                    if (not iterLineCurrX or iterLineCurrX.get_line_offset() != curr_x) {
                        spdlog::error("!! iter ({},{})", insert_y, curr_x);
                        return;
                    }
                }
            }
            iterInsert = iterLineCurrX;
        }
        rTextBuffer->insert(iterInsert, col_row);
        ++insert_y;
    }
}

void CtColumnEdit::_edit_insert_delete(const bool isInsert)
{
    bool unexpected{false};
    if (CtColEditState::Edit == _state) {
        bool firstLine{true};
        Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
        for (Glib::RefPtr<Gtk::TextMark>& rMarkStart : _marksStart) {
            if (rMarkStart) {
                Gtk::TextIter iterStart = rMarkStart->get_iter();
                if (iterStart) {
                    std::lock_guard<std::mutex> lock(_mutexLastInOut);
                    if (firstLine) {
                        firstLine = false;
                        Gdk::Point cursorPlace = _get_cursor_place();
                        Gdk::Point iterStartPoint = _get_point(iterStart);
                        if (isInsert) {
                            Gdk::Point expCursor = iterStartPoint;
                            expCursor.set_x(expCursor.get_x() + _lastInsertedText.size());
                            if ( _lastInsertedPoint != iterStartPoint or
                                 cursorPlace != expCursor )
                            {
                                unexpected = true;
                                spdlog::debug("!! ins point {},{} vs {},{}",
                                    _lastInsertedPoint.get_x(), _lastInsertedPoint.get_y(),
                                    iterStartPoint.get_x(), iterStartPoint.get_y());
                                spdlog::debug("!! cursor {},{} vs {},{}",
                                    cursorPlace.get_x(), cursorPlace.get_y(),
                                    expCursor.get_x(), expCursor.get_y());
                            }
                            else if (iterStart.forward_chars(_lastInsertedText.size())) {
                                rTextBuffer->move_mark(rMarkStart, iterStart);
                            }
                            else {
                                unexpected = true;
                            }
                        }
                        else {
                            if ( _lastRemovedPoint != iterStartPoint or
                                 cursorPlace != iterStartPoint )
                            {
                                unexpected = true;
                                spdlog::debug("!! del point {},{} vs {},{}",
                                    _lastRemovedPoint.get_x(), _lastRemovedPoint.get_y(),
                                    iterStartPoint.get_x(), iterStartPoint.get_y());
                                spdlog::debug("!! cursor {},{} vs {},{}",
                                    cursorPlace.get_x(), cursorPlace.get_y(),
                                    iterStartPoint.get_x(), iterStartPoint.get_y());
                            }
                            else if (_lastRemovedText.find("\n") != Glib::ustring::npos) {
                                _myOwnInsertDelete = true;
                                rTextBuffer->insert_at_cursor(_lastRemovedText);
                                _myOwnInsertDelete = false;
                                if (_lastRemovedDeltaOffset > 0) {
                                    iterStart = rMarkStart->get_iter();
                                    if (iterStart and iterStart.forward_char()) {
                                        rTextBuffer->move_mark(rMarkStart, iterStart);
                                    }
                                }
                                if (not _enforce_cursor_column_mode_place()) {
                                    column_mode_off();
                                }
                            }
                        }
                    }
                    else {
                        if (isInsert) {
                            _myOwnInsertDelete = true;
                            rTextBuffer->insert(iterStart, _lastInsertedText);
                            _myOwnInsertDelete = false;
                            iterStart = rMarkStart->get_iter();
                            if (iterStart and iterStart.forward_chars(_lastInsertedText.size())) {
                                rTextBuffer->move_mark(rMarkStart, iterStart);
                            }
                            else {
                                unexpected = true;
                            }
                        }
                        else {
                            Gtk::TextIter iterEnd = iterStart;
                            if (_lastRemovedDeltaOffset < 0) {
                                // negative, delete front
                                iterEnd.forward_chars(-_lastRemovedDeltaOffset);
                            }
                            else {
                                // positive, backspace
                                iterStart.backward_chars(_lastRemovedDeltaOffset);
                            }
                            if (iterStart.get_text(iterEnd).find("\n") == Glib::ustring::npos) {
                                _myOwnInsertDelete = true;
                                rTextBuffer->erase(iterStart, iterEnd);
                                _myOwnInsertDelete = false;
                            }
                        }
                    }
                }
                else {
                    unexpected = true;
                }
            }
            else {
                unexpected = true;
            }
            if (unexpected) {
                break;
            }
        }
    }
    else {
        unexpected = true;
    }
    if (unexpected) {
        column_mode_off();
    }
}

void CtColumnEdit::column_mode_off()
{
    if (CtColEditState::Off != _state) {
        _state = CtColEditState::Off;
        _clear_marks();
        _ctrlDown = false;
        _altDown = false;
        if (_stateOnOffCallback) _stateOnOffCallback(false);
    }
    else {
        if (_ctrlDown) _ctrlDown = false;
        if (_altDown) _altDown = false;
    }
}

void CtColumnEdit::text_inserted(const Gtk::TextIter& pos, const Glib::ustring& text)
{
    Glib::signal_idle().connect_once([&](){
        const Gdk::Point cursorPlace = _get_cursor_place();
        if (_newCursorRowColCallback) {
            _newCursorRowColCallback(cursorPlace.get_y()+1, cursorPlace.get_x());
        }
        else {
            spdlog::debug("{} {},{}", __FUNCTION__, cursorPlace.get_y()+1, cursorPlace.get_x());
        }
    });  
    if (CtColEditState::Off == _state or _myOwnInsertDelete) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(_mutexLastInOut);
        _lastInsertedText = text;
        _lastInsertedPoint = _get_point(pos);
    }
    Glib::signal_idle().connect_once([&](){
        if (CtColEditState::PrEdit == _state) {
            _predit_to_edit();
        }
        _edit_insert_delete(true/*isInsert*/);
    });
}

void CtColumnEdit::text_removed(const Gtk::TextIter& range_start, const Gtk::TextIter& range_end)
{
    Glib::signal_idle().connect_once([&](){
        const Gdk::Point cursorPlace = _get_cursor_place();
        if (_newCursorRowColCallback) {
            _newCursorRowColCallback(cursorPlace.get_y()+1, cursorPlace.get_x());
        }
        else {
            spdlog::debug("{} {},{}", __FUNCTION__, cursorPlace.get_y()+1, cursorPlace.get_x());
        }
    }); 
    if (CtColEditState::Off == _state or _myOwnInsertDelete) {
        return;
    }
    if (0 == _marksStart.size() or not _marksStart.front()) {
        column_mode_off();
        return;
    }
    Gtk::TextIter firstStartMarkIter = _marksStart.front()->get_iter();
    if (not firstStartMarkIter) {
        column_mode_off();
        return;
    }
    {
        std::lock_guard<std::mutex> lock(_mutexLastInOut);
        _lastRemovedText = range_start.get_text(range_end);
        _lastRemovedPoint = _get_point(range_start);
        _lastRemovedDeltaOffset = _lastRemovedText.size();
        if (range_end.get_line_offset() == firstStartMarkIter.get_line_offset()) {
            //spdlog::debug("positive, backspace");
        }
        else if (range_start.get_line_offset() == firstStartMarkIter.get_line_offset()) {
            //spdlog::debug("negative, delete front");
            _lastRemovedDeltaOffset *= -1;
        }
        else if (CtColEditState::PrEdit == _state and _marksEnd.size() > 0 and _marksEnd.front()) {
            Gtk::TextIter firstEndMarkIter = _marksEnd.front()->get_iter();
            if (not firstEndMarkIter) {
                column_mode_off();
                return;
            }
            if (range_end.get_line_offset() == firstEndMarkIter.get_line_offset()) {
                //spdlog::debug("positive, backspace");
            }
            else if (range_start.get_line_offset() == firstEndMarkIter.get_line_offset()) {
                //spdlog::debug("negative, delete front");
                _lastRemovedDeltaOffset *= -1;
            }
            else {
                spdlog::debug("!! unexp");
                column_mode_off();
                return;
            }
        }
        else {
            spdlog::debug("!! unexp");
            column_mode_off();
            return;
        }
    }
    Glib::signal_idle().connect_once([&](){
        if (CtColEditState::PrEdit == _state) {
            _predit_to_edit();
            std::lock_guard<std::mutex> lock(_mutexLastInOut);
            if (_lastRemovedDeltaOffset < 0) {
                Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
                _myOwnInsertDelete = true;
                rTextBuffer->insert_at_cursor(_lastRemovedText);
                _myOwnInsertDelete = false;
                if (not _enforce_cursor_column_mode_place()) {
                    column_mode_off();
                }
            }
        }
        else {
            _edit_insert_delete(false/*isInsert*/);
        }
    });
}

void CtColumnEdit::button_1_released()
{
    if ( CtColEditState::Selection != _state) {
        return;
    }
    bool unexpected{false};
    _state = CtColEditState::PrEdit;
    if ( _marksStart.size() > 0 and
         _marksEnd.size() > 0 and
         _marksStart.front() and
         _marksEnd.front() )
    {
        Gtk::TextIter iterLeftCol = _marksStart.front()->get_iter();
        Gtk::TextIter iterRightCol = _marksEnd.front()->get_iter();
        if ( iterLeftCol and
             iterRightCol and
             _enforce_cursor_column_mode_place() )
        {
            if (iterLeftCol.get_line_offset() == iterRightCol.get_line_offset()) {
                _state = CtColEditState::Edit;
                _clear_marks(false/*alsoStart*/);
            }
        }
        else {
            unexpected = true;
        }
    }
    else {
        unexpected = true;
    }
    if (unexpected) {
        column_mode_off();
    }
}

void CtColumnEdit::focus_in()
{
    const Gdk::Point cursorPlace = _get_cursor_place();
    if (_newCursorRowColCallback) {
        _newCursorRowColCallback(cursorPlace.get_y()+1, cursorPlace.get_x());
    }
    else {
        spdlog::debug("{} {},{}", __FUNCTION__, cursorPlace.get_y()+1, cursorPlace.get_x());
    }
}

void CtColumnEdit::selection_update()
{
    const Gdk::Point cursorPlace = _get_cursor_place();
    if (_newCursorRowColCallback) {
        _newCursorRowColCallback(cursorPlace.get_y()+1, cursorPlace.get_x());
    }
    else {
        spdlog::debug("{} {},{}", __FUNCTION__, cursorPlace.get_y()+1, cursorPlace.get_x());
    }
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    if (not rTextBuffer->get_has_selection()) {
        if ( CtColEditState::Off != _state and
             cursorPlace != _get_cursor_column_mode_place() )
        {
            column_mode_off();
        }
        return;
    }
    Gtk::TextIter startIter, endIter;
    rTextBuffer->get_selection_bounds(startIter, endIter);
    if ( _get_point(startIter) != _pointStart or
         _get_point(endIter) != _pointEnd )
    {
        if (CtColEditState::Off == _state and _ctrlDown and _altDown) {
            _state = CtColEditState::Selection;
            if (_stateOnOffCallback) _stateOnOffCallback(true);
        }
        if (CtColEditState::Selection == _state) {
            _pointStart = _get_point(startIter);
            _pointEnd = _get_point(endIter);
            _clear_marks();
            if (_pointStart != _pointEnd) {
                // in first iteration populate _marksStart
                std::vector<Glib::RefPtr<Gtk::TextMark>>* pMarksVec = &_marksStart;
                for (const int x : {_pointStart.get_x(), _pointEnd.get_x()}) {
                    for (int y = _pointStart.get_y(); y <= _pointEnd.get_y(); ++y) {
                        Gtk::TextIter currIter = rTextBuffer->get_iter_at_line_offset(y, x);
                        if ( currIter and
                             currIter.get_line_offset() == x and
                             currIter.get_line() == y )
                        {
                            Glib::RefPtr<Gtk::TextMark> rMark = rTextBuffer->create_mark(currIter);
                            rMark->set_visible(true);
                            pMarksVec->push_back(rMark);
                        }
                    }
                    // in second iteration populate _marksEnd
                    pMarksVec = &_marksEnd;
                }
            }
        }
    }
}
