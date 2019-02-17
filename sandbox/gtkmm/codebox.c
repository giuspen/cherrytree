
// gcc -Wall codebox.c -o codebox3 `pkg-config --cflags --libs gtk+-3.0`

#include<gtk/gtk.h>

static void get_focus(GtkWidget *button, GtkWidget *text_view_nested)
{
    g_print("Grab Focus Button\n");
    gtk_widget_grab_focus(text_view_nested);
}

gboolean grab_focus(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    g_print("Grab Focus Click\n");
    gtk_widget_grab_focus(widget);
    return TRUE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CodeBox");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 450);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget* text_view_base = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_base), GTK_WRAP_CHAR);
    gtk_widget_set_hexpand(text_view_base, TRUE);
    gtk_widget_set_vexpand(text_view_base, TRUE);

    GtkTextIter iter;
    GtkTextBuffer* r_buffer_base = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_base));
    gtk_text_buffer_get_end_iter(r_buffer_base, &iter);
    gtk_text_buffer_insert(r_buffer_base, &iter, "Anchored TextView below:\n==>", -1);

    GtkWidget* text_view_nested = gtk_text_view_new();
    gtk_widget_set_size_request(text_view_nested, 100, 50);
    gtk_widget_set_name(text_view_nested, "cyan_view");
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_nested), GTK_WRAP_CHAR);
    gtk_widget_set_hexpand(text_view_nested, TRUE);
    gtk_widget_set_vexpand(text_view_nested, TRUE);
    GtkTextBuffer* s_buffer_base = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_nested));
    gtk_text_buffer_get_end_iter(s_buffer_base, &iter);
    gtk_text_buffer_insert(s_buffer_base, &iter, "Nested buffer space.", -1);
    g_signal_connect(text_view_nested, "button-press-event", G_CALLBACK(grab_focus), NULL);

    GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_size_request(scroll, 100, 50);
    gtk_container_add(GTK_CONTAINER(scroll), text_view_nested);

    GtkWidget* entry_nested = gtk_entry_new();

    gtk_text_buffer_get_end_iter(r_buffer_base, &iter);
    GtkTextChildAnchor* r_anchor_text_view = gtk_text_buffer_create_child_anchor(r_buffer_base, &iter);
    gtk_text_buffer_get_end_iter(r_buffer_base, &iter);
    gtk_text_buffer_insert(r_buffer_base, &iter, "<==\nAnchored TextView above^\n\nAnchored Entry below:\n==>", -1);
    gtk_text_buffer_get_end_iter(r_buffer_base, &iter);
    GtkTextChildAnchor* r_anchor_entry = gtk_text_buffer_create_child_anchor(r_buffer_base, &iter);
    gtk_text_buffer_get_end_iter(r_buffer_base, &iter);
    gtk_text_buffer_insert(r_buffer_base, &iter, "<==\nAnchored Entry above^\n", -1);

    gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(text_view_base), scroll, r_anchor_text_view);
    gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(text_view_base), entry_nested, r_anchor_entry);

    GtkWidget* button = gtk_button_new_with_label("Get Focus");
    g_signal_connect(button, "clicked", G_CALLBACK(get_focus), text_view_nested);

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), text_view_base, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    GError* css_error = NULL;
    gchar* css_string1 = NULL;
    css_string1 = g_strdup("#cyan_view{background: cyan;}");
    GtkCssProvider* provider = gtk_css_provider_new();
    GdkDisplay* display = gdk_display_get_default();
    GdkScreen* screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, css_string1, -1, &css_error);

    if (css_error != NULL)
    {
        g_print("CSS loader error %s\n", css_error->message);
        g_error_free(css_error);
    }
    g_object_unref(provider);
    if (css_string1 != NULL) g_free(css_string1);

    gtk_main();

    return 0;
}
