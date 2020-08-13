
// g++ columnedit.cc -o columnedit `pkg-config gtkmm-3.0 --cflags --libs`

#include <gtkmm.h>
#include <utility>
#include <atomic>
#include <list>

enum class CtColEditState { Off, Selection, Editing };

class CtColumnEdit
{
public:
    CtColumnEdit(Gtk::TextView& textView);

    void selection_update();
    void button_control_changed(const bool isDown) { _ctrlDown = isDown; }
    void button_alt_changed(const bool isDown) { _altDown = isDown; }

private:
    Gtk::TextView& _textView;
    std::atomic<CtColEditState> _state{CtColEditState::Off};
    std::atomic<bool> _ctrlDown{false};
    std::atomic<bool> _altDown{false};
    Gdk::Point _pointStart{-1,-1};
    Gdk::Point _pointEnd{-1,-1};
    std::list<Glib::RefPtr<Gtk::TextMark>> _marksStart;
    std::list<Glib::RefPtr<Gtk::TextMark>> _marksEnd;
};

CtColumnEdit::CtColumnEdit(Gtk::TextView& textView)
 : _textView{textView}
{
}

void CtColumnEdit::selection_update()
{
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    if (not rTextBuffer->get_has_selection()) {
        if (CtColEditState::Selection == _state) {
            _state = CtColEditState::Off;
            printf("colMode OFF\n");
        }
        _pointStart.set_x(-1);
        return;
    }
    Gtk::TextIter startIter, endIter;
    rTextBuffer->get_selection_bounds(startIter, endIter);
    if ( startIter.get_line_offset() != _pointStart.get_x() or
         startIter.get_line() != _pointStart.get_y() or
         endIter.get_line_offset() != _pointEnd.get_x() or
         endIter.get_line() != _pointEnd.get_y() )
    {
        if (CtColEditState::Off == _state and _ctrlDown and _altDown) {
            _state = CtColEditState::Selection;
            printf("colMode SEL\n");
        }
        if (CtColEditState::Selection == _state) {
            _pointStart.set_x(startIter.get_line_offset());
            _pointStart.set_y(startIter.get_line());
            _pointEnd.set_x(endIter.get_line_offset());
            _pointEnd.set_y(endIter.get_line());
            while (not _marksEnd.empty()) {
                rTextBuffer->delete_mark(_marksEnd.back());
                _marksEnd.pop_back();
            }
            while (not _marksStart.empty()) {
                rTextBuffer->delete_mark(_marksStart.back());
                _marksStart.pop_back();
            }
            if (_pointStart != _pointEnd) {
                std::list<Glib::RefPtr<Gtk::TextMark>>* pMarksList = &_marksStart;
                for (int x : std::list<int>{_pointStart.get_x(), _pointEnd.get_x()}) {
                    for (int y = _pointStart.get_y();y <= _pointEnd.get_y(); ++y) {
                        Gtk::TextIter currIter = rTextBuffer->get_iter_at_line_offset(y, x);
                        if (currIter) {
                            Glib::RefPtr<Gtk::TextMark> rMark = rTextBuffer->create_mark(currIter);
                            rMark->set_visible(true);
                            pMarksList->push_back(rMark);
                        }
                    }
                    pMarksList = &_marksEnd;
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
    textView.signal_event_after().connect([&](GdkEvent* pEvent){
        if (pEvent->type == GDK_KEY_PRESS) {
            if (pEvent->key.keyval == GDK_KEY_Control_L) {
                columnEdit.button_control_changed(true/*isDown*/);
            }
            else if (pEvent->key.keyval == GDK_KEY_Alt_L) {
                columnEdit.button_alt_changed(true/*isDown*/);
            }
        }
        else if (pEvent->type == GDK_KEY_RELEASE) {
            if (pEvent->key.keyval == GDK_KEY_Control_L) {
                columnEdit.button_control_changed(false/*isDown*/);
            }
            else if (pEvent->key.keyval == GDK_KEY_Alt_L) {
                columnEdit.button_alt_changed(false/*isDown*/);
            }
        }
    }, false);

    Gtk::Main::run(window);
    return EXIT_SUCCESS;
}
