#include "../inc/bee_user.h"

#include <time.h>

char* get_current_time(void) {
    time_t rawtime;
    struct tm *timeinfo;
    static char time_buffer[6]; 

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", timeinfo);
    return time_buffer;
}

void add_message_to_chat(const char *message, const char *file_path) {
    (void)file_path;
    if (message_area == NULL) {
        g_print("message_area is not initialized\n");
        return;
    }

    GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_top(message_box, 5);
    gtk_widget_set_margin_bottom(message_box, 5);

    gboolean is_user_message = (message != NULL);
    (void)is_user_message;
    gtk_widget_set_halign(message_box, GTK_ALIGN_END);

    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

    if (message && strlen(message) > 0) {
        GtkWidget *label = gtk_label_new(message);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0);
        gtk_widget_set_name(label, "message-label");
        gtk_box_pack_start(GTK_BOX(content_box), label, FALSE, FALSE, 2);
    }

    if (file_path && strlen(file_path) > 0) {
        GError *error = NULL;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(file_path, 200, 200, TRUE, &error);
        if (pixbuf) {
            GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
            gtk_box_pack_start(GTK_BOX(content_box), image, FALSE, FALSE, 2);
            g_object_unref(pixbuf);
        } else {
            g_print("Error loading image: %s\n", error->message);
            g_error_free(error);
        }
    }

    GtkWidget *time_label = gtk_label_new(get_current_time());
    gtk_widget_set_name(time_label, "time-label");
    gtk_label_set_xalign(GTK_LABEL(time_label), 1.0);
    gtk_box_pack_start(GTK_BOX(content_box), time_label, FALSE, FALSE, 2);

    gtk_box_pack_start(GTK_BOX(message_box), content_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(message_area), message_box, FALSE, FALSE, 5);

    gtk_widget_show_all(message_box);
}

void on_emoji_selected(GtkButton *button, gpointer user_data) {
    (void) user_data;  
    const char *emoji = gtk_button_get_label(button);  
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(chat_entry));

    gchar *new_text = g_strdup_printf("%s%s", current_text, emoji);
    gtk_entry_set_text(GTK_ENTRY(chat_entry), new_text);
    g_free(new_text);
}

void on_emoji_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; 
    GtkWidget *emoji_user_data = GTK_WIDGET(user_data);

    GtkWidget *popover, *emoji_grid;

    popover = gtk_widget_get_parent(emoji_user_data);

    if (GTK_IS_POPOVER(popover)) {
        gtk_widget_hide(popover);
        return;
    }

    popover = gtk_popover_new(emoji_user_data);
    gtk_widget_set_name(popover, "emoji_window");
    gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM); 

    emoji_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(popover), emoji_grid);

    const char *emojis[] = {
        "😊", "😂", "😢", "😍", "😎", "😡", "🥰", "😘", "😋", "🤗",
        "😃", "😄", "😅", "😜", "😏", "😇", "😱", "🥺", "😆", "👽",
        "🥳", "👻", "🤡", "👍", "👎", "💖", "💔", "❤️", "✨", "💫",
        "🌚", "🌻", "🌱", "🐱", "🐶", "🐝", "🦋", "🧚‍♀️", "👀", "🥴"
    };

    int num_emojis = sizeof(emojis) / sizeof(emojis[0]);

    for (int i = 0; i < num_emojis; i++) {
        GtkWidget *emoji_button = gtk_button_new_with_label(emojis[i]);
        g_signal_connect(emoji_button, "clicked", G_CALLBACK(on_emoji_selected), NULL);
        gtk_grid_attach(GTK_GRID(emoji_grid), emoji_button, i % 5, i / 5, 1, 1);
    }

    gtk_widget_show_all(popover);
}

void on_file_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkWidget *parent_window = GTK_WIDGET(user_data);

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Chose image",
        GTK_WINDOW(parent_window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        " Cancel", GTK_RESPONSE_CANCEL,
        " Open", GTK_RESPONSE_ACCEPT,
        NULL
    );

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    g_print("File chosen: %s\n", filename);
    
    if (uses_chat == NULL) {
        mx_printerr("Chat not chosen\n");

        GtkWidget *error_dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(error_dialog), "Error");
        gtk_widget_set_name(error_dialog, "dialog-search-person");
        gtk_window_set_transient_for(GTK_WINDOW(error_dialog), GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(chat_entry))));

        GtkWidget *error_label = gtk_label_new("Chat not chosen");
        gtk_widget_set_name(error_label, "error-label");

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(error_dialog));
        gtk_container_add(GTK_CONTAINER(content_area), error_label);
 
        gtk_widget_show_all(error_dialog);

        g_signal_connect(error_dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
        gtk_widget_grab_focus(chat_entry);

        g_free(filename);
    } else {
        // Proceed with adding a file message if chat is available
        add_message_to_chat(NULL, filename);
        g_free(filename);  // Don't forget to free the filename
    }
}


    gtk_widget_destroy(dialog);
}

void clear_message_area(void) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(message_area));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

// void on_chat_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
//     (void)box;
//     (void)user_data;
//     if (GTK_WIDGET(row) != current_chat) {
//         current_chat = GTK_WIDGET(row);
//         clear_message_area(); 
//     }
// }
