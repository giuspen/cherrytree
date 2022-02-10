
#include <gtk/gtk.h>

// gcc -Wall window_position_get_set.c -o window_position_get_set `pkg-config --cflags --libs gtk+-3.0`

void on_button_clicked(GtkWidget* widget, gpointer window)
{
    gint x,y;
    gtk_window_get_position(GTK_WINDOW(window), &x, &y);
    g_print("gtk_window_get_position => (%d, %d)\n", x, y);
    gtk_window_move(GTK_WINDOW(window), x, y);
    g_print("gtk_window_move => (%d, %d)\n", x, y);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Click button to get position and move to position");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    GtkWidget* button = gtk_button_new_with_mnemonic("Get position and move to position");

    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), window);

    gtk_container_add(GTK_CONTAINER(window), button);

    gtk_widget_show_all(window);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();

    return 0;
}
