#include "../inc/bee_user.h"

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

        //add_message_to_chat(NULL, filename); 
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}
