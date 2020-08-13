
// g++ columnedit.cc -o columnedit `pkg-config gtkmm-3.0 --cflags --libs`

#include <gtkmm.h>
#include <utility>
#include <atomic>

class CtColumnEdit
{
public:
    CtColumnEdit(Gtk::TextView& textView);

    void selection_update();

private:
    Gtk::TextView& _textView;
    std::atomic<bool> _ctrlDown{false};
    std::atomic<bool> _altDown{false};
    std::pair<int,int> _columnAndLineStart{-1,-1};
    std::pair<int,int> _columnAndLineEnd{-1,-1};
};

CtColumnEdit::CtColumnEdit(Gtk::TextView& textView)
 : _textView{textView}
{
}

void CtColumnEdit::selection_update()
{
    Glib::RefPtr<Gtk::TextBuffer> rTextBuffer = _textView.get_buffer();
    if (not rTextBuffer->get_has_selection()) {
        _columnAndLineStart.first = -1;
        return;
    }
    Gtk::TextIter startIter, endIter;
    rTextBuffer->get_selection_bounds(startIter, endIter);
    if ( startIter.get_line_offset() != _columnAndLineStart.first or
         startIter.get_line() != _columnAndLineStart.second or
         endIter.get_line_offset() != _columnAndLineEnd.first or
         endIter.get_line() != _columnAndLineEnd.second )
    {
        _columnAndLineStart.first = startIter.get_line_offset();
        _columnAndLineStart.second = startIter.get_line();
        _columnAndLineEnd.first = endIter.get_line_offset();
        _columnAndLineEnd.second = endIter.get_line();
        printf("%u,%u -> %u,%u\n", _columnAndLineStart.first, _columnAndLineStart.second, _columnAndLineEnd.first, _columnAndLineEnd.second);
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

    Gtk::Main::run(window);
    return EXIT_SUCCESS;
}
