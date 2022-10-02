
// g++ vte.cc -o vte `pkg-config gtkmm-3.0 --cflags --libs` `pkg-config vte-2.91 --cflags --libs`
// https://cubicarch.wordpress.com/2012/06/10/little-vte-tutorial-cc/

#include <vte/vte.h>
#include <gtkmm.h>
#include <iostream>

// The following macro is copied from Geany https://github.com/geany/geany/blob/master/src/vte.c
static const gchar VTE_ADDITIONAL_WORDCHARS[] = "-,./?%&#:_";

static void _vteTerminalSpawnAsyncCallback(VteTerminal* terminal,
                                           GPid pid,
                                           GError* error,
                                           gpointer user_data)
{
    if (-1 != pid) {
        std::cout << "+VTE" << std::endl;
        char* sh_cmd = (char*)user_data;
        vte_terminal_feed_child(terminal, sh_cmd, strlen(sh_cmd));
    }
    else {
        std::cout << "!! VTE" << std::endl;
    }
    if (NULL != error) {
        std::cout << error->message << std::endl;
        g_clear_error(&error);
    }
}

int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    Gtk::Window  window;
    window.set_default_size(800, 400);

    GtkWidget* pTermWidget = vte_terminal_new();
    Gtk::Widget* pCtVte = Gtk::manage(Glib::wrap(pTermWidget));
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(pCtVte->gobj()), -1/*infinite*/);
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(pCtVte->gobj()), true);
    vte_terminal_set_word_char_exceptions(VTE_TERMINAL(pCtVte->gobj()), VTE_ADDITIONAL_WORDCHARS);

    char sh_cmd[] = "ls -la\n";

    g_autofree gchar* user_shell = vte_get_user_shell();
    char* startterm[2] = {0, 0};
    startterm[0] = user_shell ? user_shell : (char*)"/bin/sh";
    vte_terminal_spawn_async(VTE_TERMINAL(pCtVte->gobj()),
                             VTE_PTY_DEFAULT,
                             NULL/*working_directory*/,
                             startterm/*argv*/,
                             NULL/*envv*/,
                             G_SPAWN_DEFAULT/*spawn_flags_*/,
                             NULL/*child_setup*/,
                             NULL/*child_setup_data*/,
                             NULL/*child_setup_data_destroy*/,
                             -1/*timeout*/,
                             NULL/*cancellable*/,
                             &_vteTerminalSpawnAsyncCallback,
                             sh_cmd/*user_data*/);

    Gtk::Box hbox{Gtk::ORIENTATION_HORIZONTAL, 1/*spacing*/};
    Gtk::Box vbox{Gtk::ORIENTATION_VERTICAL, 0/*spacing*/};
    Gtk::Button button_copy_all{"Copy All"};
    Gtk::Button button_copy_sel{"Copy Sel"};
    Gtk::Button button_paste{"Paste"};

    hbox.pack_start(vbox, false, false);
    hbox.pack_start(*pCtVte, true, true);

    vbox.pack_start(button_copy_all, false, false);
    vbox.pack_start(button_copy_sel, false, false);
    vbox.pack_start(button_paste, false, false);

    button_copy_all.signal_clicked().connect([pTermWidget](){
        vte_terminal_select_all(VTE_TERMINAL(pTermWidget));
        vte_terminal_copy_clipboard_format(VTE_TERMINAL(pTermWidget), VTE_FORMAT_TEXT);
    });
    button_copy_sel.signal_clicked().connect([pTermWidget](){
        vte_terminal_copy_clipboard_format(VTE_TERMINAL(pTermWidget), VTE_FORMAT_TEXT);
    });
    button_paste.signal_clicked().connect([pTermWidget](){
        vte_terminal_paste_clipboard(VTE_TERMINAL(pTermWidget));
    });

    if (GTK_IS_SCROLLABLE(pTermWidget)) {
        GtkWidget* pScrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,
                gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(pTermWidget)));
        gtk_widget_set_can_focus(pScrollbar, FALSE);
        Gtk::Widget* pGtkmmScrollbar = Gtk::manage(Glib::wrap(pScrollbar));
        hbox.pack_start(*pGtkmmScrollbar, false, false);
    }
    else {
        std::cout << "!! GTK_IS_SCROLLABLE(pTermWidget)" << std::endl;
    }

    window.add(hbox);
    window.show_all();
    Gtk::Main::run(window);
    return EXIT_SUCCESS;
}
