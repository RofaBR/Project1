#include "../inc/client.h"

void load_css(GtkWidget *window) {
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;

    provider = gtk_css_provider_new();

    GError *error = NULL;
    if (!gtk_css_provider_load_from_path(provider, "styles.css", &error)) {
        g_warning("Failed to load CSS: %s", error->message);
        g_error_free(error);
    }

    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(gtk_widget_get_style_context(window), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

void load_css_for_screen(const char *css_file_path) {
    GtkCssProvider *css_provider;
    GFile *css_file;
    GError *error = NULL;

    css_provider = gtk_css_provider_new();

    css_file = g_file_new_for_path(css_file_path);

    if (!gtk_css_provider_load_from_file(css_provider, css_file, &error)) {
        g_warning("Failed to load CSS file: %s", error->message);
        g_error_free(error);
        g_object_unref(css_provider);
        g_object_unref(css_file);
        return;
    }

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    g_object_unref(css_provider);
    g_object_unref(css_file);
}
