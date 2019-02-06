
// g++ codebox.cc -o codebox `pkg-config gtkmm-3.0 --cflags --libs`
// g++ codebox.cc -o codebox `pkg-config gtkmm-2.4 --cflags --libs`

#include <iostream>
#include <gtkmm.h>


int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    Gtk::Window  window;
    window.set_default_size(450, 450);
    Gtk::TextView textViewBase;
    Glib::RefPtr<Gtk::TextBuffer> rBufferBase = textViewBase.get_buffer();

    Gtk::TextView textViewNested;
    Gtk::ScrolledWindow scrolledWindowNested;
    scrolledWindowNested.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledWindowNested.add(textViewNested);
    scrolledWindowNested.set_size_request(300, 300);

    Gtk::Entry entryNested;

    rBufferBase->insert(rBufferBase->end(), "Anchored TextView below:\n==>");
    Glib::RefPtr<Gtk::TextChildAnchor> rAnchorTextView = rBufferBase->create_child_anchor(rBufferBase->end());
    rBufferBase->insert(rBufferBase->end(), "<==\nAnchored TextView above^\n\nAnchored Entry below:\n==>");
    Glib::RefPtr<Gtk::TextChildAnchor> rAnchorEntry = rBufferBase->create_child_anchor(rBufferBase->end());
    rBufferBase->insert(rBufferBase->end(), "<==\nAnchored Entry above^\n");

    textViewBase.add_child_at_anchor(scrolledWindowNested, rAnchorTextView);
    scrolledWindowNested.show_all();
    textViewBase.add_child_at_anchor(entryNested, rAnchorEntry);
    entryNested.show_all();

    window.add(textViewBase);
    window.show_all();
    Gtk::Main::run(window);
    return EXIT_SUCCESS;
}
