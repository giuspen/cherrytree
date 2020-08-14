
// g++ columnedit.cc -o columnedit `pkg-config gtkmm-3.0 --cflags --libs`

#include <gtkmm.h>
#include <utility>
#include <atomic>
#include <vector>
#include <mutex>

enum class CtColEditState { Off, Selection, PrEdit, Edit };

class CtColumnEdit
{
public:
    CtColumnEdit(Gtk::TextView& textView);

    void selection_update();
    void button_control_changed(const bool isDown) { _ctrlDown = isDown; }
    void button_alt_changed(const bool isDown) { _altDown = isDown; }
    void button_1_released();
    void text_inserted(const Gtk::TextIter& pos, const Glib::ustring& text);
    void text_removed(const Gtk::TextIter& range_start, const Gtk::TextIter& range_end);

private:
    Gdk::Point _get_point(const Gtk::TextIter& textIter);
    Gdk::Point _get_cursor_place();
    Gdk::Point _get_cursor_column_mode_place();
    void _clear_marks(const bool alsoStart = true);
    void _predit_to_edit();
    void _column_mode_off();
    void _edit_insert_delete(const bool isInsert);
    bool _enforce_cursor_column_mode_place();

    Glib::ustring _lastInsertedText;
    Gdk::Point _lastInsertedPoint;
    Glib::ustring _lastRemovedText;
    Gdk::Point _lastRemovedPoint;
    int _lastRemovedDeltaOffset;
    std::mutex _mutexLastInOut;

    Gtk::TextView& _textView;
    std::atomic<CtColEditState> _state{CtColEditState::Off};
    std::atomic<bool> _ctrlDown{false};
    std::atomic<bool> _altDown{false};
    std::atomic<bool> _myOwnInsertDelete{false};
    Gdk::Point _pointStart{-1,-1};
    Gdk::Point _pointEnd{-1,-1};
    std::vector<Glib::RefPtr<Gtk::TextMark>> _marksStart;
    std::vector<Glib::RefPtr<Gtk::TextMark>> _marksEnd;
};

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

void CtColumnEdit::_predit_to_edit()
{
    bool unexpected{false};
    if (CtColEditState::PrEdit == _state) {
        _state = CtColEditState::Edit;
        printf("colMode EDIT\n");
        bool firstLine{true};
        Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
        size_t currEndIdx{0};
        for (Glib::RefPtr<Gtk::TextMark>& markStart : _marksStart) {
            if (markStart) {
                Gtk::TextIter iterStart = markStart->get_iter();
                if (iterStart) {
                    while (currEndIdx < _marksEnd.size()) {
                        if (_marksEnd.at(currEndIdx)) {
                            Gtk::TextIter iterEnd = _marksEnd.at(currEndIdx)->get_iter();
                            if (iterEnd) {
                                if (iterEnd.get_line() < iterStart.get_line()) {
                                    ++currEndIdx;
                                }
                                else {
                                    if (iterEnd.get_line() == iterStart.get_line()) {
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
                                    ++currEndIdx;
                                    break;
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
        _column_mode_off();
    }
    else {
        _clear_marks(false/*alsoStart*/);
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
                    _mutexLastInOut.lock();
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
                                printf("ins point %u,%u vs %u,%u\n",
                                    _lastInsertedPoint.get_x(), _lastInsertedPoint.get_y(),
                                    iterStartPoint.get_x(), iterStartPoint.get_y());
                                printf("cursor %u,%u vs %u,%u\n",
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
                                printf("del point %u,%u vs %u,%u\n",
                                    _lastRemovedPoint.get_x(), _lastRemovedPoint.get_y(),
                                    iterStartPoint.get_x(), iterStartPoint.get_y());
                                printf("cursor %u,%u vs %u,%u\n",
                                    cursorPlace.get_x(), cursorPlace.get_y(),
                                    iterStartPoint.get_x(), iterStartPoint.get_y());
                            }
                            else if (_lastRemovedText.find("\n") != Glib::ustring::npos) {
                                _myOwnInsertDelete = true;
                                rTextBuffer->insert_at_cursor(_lastRemovedText);
                                _myOwnInsertDelete = false;
                                if (not _enforce_cursor_column_mode_place()) {
                                    _column_mode_off();
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
                    _mutexLastInOut.unlock();
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
        _column_mode_off();
    }
}

void CtColumnEdit::_column_mode_off()
{
    _state = CtColEditState::Off;
    _clear_marks();
    _pointStart.set_x(-1);
    printf("colMode OFF\n");
}

void CtColumnEdit::text_inserted(const Gtk::TextIter& pos, const Glib::ustring& text)
{
    if (CtColEditState::Off == _state or _myOwnInsertDelete) {
        return;
    }
    _mutexLastInOut.lock();
    _lastInsertedText = text;
    _lastInsertedPoint = _get_point(pos);
    _mutexLastInOut.unlock();
    Glib::signal_idle().connect_once([&](){
        if (CtColEditState::PrEdit == _state) {
            _predit_to_edit();
        }
        _edit_insert_delete(true/*isInsert*/);
    });
}

void CtColumnEdit::text_removed(const Gtk::TextIter& range_start, const Gtk::TextIter& range_end)
{
    if (CtColEditState::Off == _state or _myOwnInsertDelete) {
        return;
    }
    if (0 == _marksStart.size() or not _marksStart.front()) {
        _column_mode_off();
        return;
    }
    Gtk::TextIter firstStartMarkIter = _marksStart.front()->get_iter();
    if (not firstStartMarkIter) {
        _column_mode_off();
        return;
    }
    _mutexLastInOut.lock();
    _lastRemovedText = range_start.get_text(range_end);
    _lastRemovedPoint = _get_point(range_start);
    _lastRemovedDeltaOffset = _lastRemovedText.size();
    if (range_end.get_line_offset() == firstStartMarkIter.get_line_offset()) {
        //printf("positive, backspace\n");
    }
    else if (range_start.get_line_offset() == firstStartMarkIter.get_line_offset()) {
        //printf("negative, delete front\n");
        _lastRemovedDeltaOffset *= -1;
    }
    else if (CtColEditState::PrEdit == _state and _marksEnd.size() > 0 and _marksEnd.front()) {
        Gtk::TextIter firstEndMarkIter = _marksEnd.front()->get_iter();
        if (not firstEndMarkIter) {
            _column_mode_off();
            return;
        }
        if (range_end.get_line_offset() == firstEndMarkIter.get_line_offset()) {
            //printf("positive, backspace\n");
        }
        else if (range_start.get_line_offset() == firstEndMarkIter.get_line_offset()) {
            //printf("negative, delete front\n");
            _lastRemovedDeltaOffset *= -1;
        }
        else {
            printf("unexp\n");
            _column_mode_off();
            return;
        }
    }
    else {
        printf("unexp\n");
        _column_mode_off();
        return;
    }
    _mutexLastInOut.unlock();
    Glib::signal_idle().connect_once([&](){
        if (CtColEditState::PrEdit == _state) {
            _predit_to_edit();
            _mutexLastInOut.lock();
            if (_lastRemovedDeltaOffset < 0) {
                Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
                _myOwnInsertDelete = true;
                rTextBuffer->insert_at_cursor(_lastRemovedText);
                _myOwnInsertDelete = false;
                if (not _enforce_cursor_column_mode_place()) {
                    _column_mode_off();
                }
            }
            _mutexLastInOut.unlock();
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
    printf("colMode PRE\n");
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
                printf("colMode EDIT\n");
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
        _column_mode_off();
    }
}

void CtColumnEdit::selection_update()
{
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    if (not rTextBuffer->get_has_selection()) {
        if ( CtColEditState::Off != _state and
             _get_cursor_place() != _get_cursor_column_mode_place() )
        {
            _column_mode_off();
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
            printf("colMode SEL\n");
        }
        if (CtColEditState::Selection == _state) {
            _pointStart = _get_point(startIter);
            _pointEnd = _get_point(endIter);
            _clear_marks();
            if (_pointStart != _pointEnd) {
                std::vector<Glib::RefPtr<Gtk::TextMark>>* pMarksVec = &_marksStart;
                for (int x : std::vector<int>{_pointStart.get_x(), _pointEnd.get_x()}) {
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
                    pMarksVec = &_marksEnd;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    Gtk::Window window;
    window.set_default_size(450, 450);
    Gtk::TextView textView;
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = textView.get_buffer();
    const char textContent[] =
        "This is the line number zero\n"
        "This is the line number one\n"
        "This is the line number two\n"
        "This is the line number three\n"
        "This is the line number four\n"
        "This is the line number five\n"
        "This is the line number six\n"
        "This is the line number seven\n"
        "This is the line number eight\n"
        "This is the line number nine\n"
        "This is the line number ten\n";

    rTextBuffer->insert(rTextBuffer->end(), textContent);
    rTextBuffer->insert(rTextBuffer->end(), "\n");
    rTextBuffer->insert(rTextBuffer->end(), textContent);

    window.add(textView);
    window.show_all();

    CtColumnEdit columnEdit{textView};

    rTextBuffer->signal_mark_set().connect([&](const Gtk::TextIter& iter, const Glib::RefPtr<Gtk::TextMark>& rMark){
        if (rMark->get_name() == "insert") {
            columnEdit.selection_update();
        }
    }, false);
    rTextBuffer->signal_insert().connect([&](const Gtk::TextIter& pos, const Glib::ustring& text, int bytes){
        columnEdit.text_inserted(pos, text);
    }, false);
    rTextBuffer->signal_erase().connect([&](const Gtk::TextIter& range_start, const Gtk::TextIter& range_end){
        columnEdit.text_removed(range_start, range_end);
    }, false);

    textView.signal_event_after().connect([&](GdkEvent* pEvent){
        switch (pEvent->type) {
            case GDK_KEY_PRESS: {
                if (pEvent->key.keyval == GDK_KEY_Control_L) {
                    columnEdit.button_control_changed(true/*isDown*/);
                }
                else if (pEvent->key.keyval == GDK_KEY_Alt_L) {
                    columnEdit.button_alt_changed(true/*isDown*/);
                }
            } break;
            case GDK_KEY_RELEASE: {
                if (pEvent->key.keyval == GDK_KEY_Control_L) {
                    columnEdit.button_control_changed(false/*isDown*/);
                }
                else if (pEvent->key.keyval == GDK_KEY_Alt_L) {
                    columnEdit.button_alt_changed(false/*isDown*/);
                }
            } break;
            case GDK_BUTTON_RELEASE: {
                if (pEvent->button.button == 1) {
                    columnEdit.button_1_released();
                }
            } break;
            default:
                break;
        }
    }, false);

    Gtk::Main::run(window);
    return EXIT_SUCCESS;
}
