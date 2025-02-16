
// gcc -Wall -O0 -g simplewindow.c -o simplewindow `pkg-config --cflags --libs gtk+-3.0`
// gdb simplewindow
// (gdb) r
// just change keyboard layout, this causes the crash
// (gdb) bt

// from https://zetcode.com/gui/gtk2/gtktextview/

#include <gtk/gtk.h>

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *view;
    GtkWidget *vbox;

    GtkTextBuffer *buffer;
    GtkTextIter iter;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_window_set_title(GTK_WINDOW(window), "mingw-w64-x86_64-gtk3 3.24.48-1 crash");

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    view = gtk_text_view_new();
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

    gtk_text_buffer_insert(buffer, &iter, " Just change keyboard layout with this window in focus\n This causes a crash in libgtk-3-0.dll\n With mingw-w64-x86_64-gtk3 3.24.48-1 (previous version 3.24.43-1 does not have this issue) ", -1);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
