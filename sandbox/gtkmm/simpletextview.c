
// gcc -Wall simpletextview.c -o simpletextview `pkg-config --cflags --libs gtk+-3.0`

// from https://zetcode.com/gui/gtk2/gtktextview/

#include <gtk/gtk.h>

gboolean on_key_pressed(GtkWidget *textview, GdkEventKey* event)
{
    if ((event->type == GDK_KEY_PRESS) && (event->state & GDK_CONTROL_MASK)) {
        g_print("!! GDK_CONTROL_MASK\n");
        return TRUE;
    }
    return FALSE;
}

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
    gtk_window_set_title(GTK_WINDOW(window), "GtkTextView");

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    view = gtk_text_view_new();
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

    gtk_text_buffer_insert(buffer, &iter, "If you push 'AltGr', it is detected as a 'Ctrl'\n\n", -1);
    
    gtk_text_buffer_insert(buffer, &iter, "Try to type anything requiring 'AltGr' and will be blocked as Ctrl down wrongly detected\n\n", -1);

    g_signal_connect(G_OBJECT(view), "key-press-event", G_CALLBACK(on_key_pressed), NULL);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    g_signal_connect(G_OBJECT(window), "destroy",
            G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
