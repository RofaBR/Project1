#include "../inc/bee_user.h"

void on_chat_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkWidget *chat_button = GTK_WIDGET(user_data);
    const char *button_label = gtk_button_get_label(GTK_BUTTON(chat_button));
    
    if (uses_chat != NULL && strcmp(uses_chat, button_label) == 0) {
        uses_chat = NULL;
        g_print("Chat not selected\n");
    } else {
        uses_chat = button_label;
        g_print("Chat label: %s\n", button_label);
    }
}

void on_button_create_chat_clicked(GtkButton *button, gpointer user_data) {
    (void)button;

    GtkWidget *main_box = GTK_WIDGET(user_data);

    // Создание поповера
    GtkWidget *chat_popover = gtk_popover_new(GTK_WIDGET(button));
    gtk_widget_set_name(chat_popover, "create_chat_popover");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(chat_popover), box);

    GtkWidget *private_chat_button = gtk_button_new_with_label("Create Private Chat");
    gtk_widget_set_name(private_chat_button, "private_chat_button");
    gtk_box_pack_start(GTK_BOX(box), private_chat_button, FALSE, FALSE, 0);

    g_signal_connect(private_chat_button, "clicked", G_CALLBACK(on_private_chat_clicked), main_box);

    gtk_widget_show_all(chat_popover);
    gtk_popover_popup(GTK_POPOVER(chat_popover));
}

void on_private_chat_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkWidget *main_box = GTK_WIDGET(user_data);

    // Ваша логика для создания приватного чата
    g_print("Private chat creation triggered.\n");


    // Пример: можно открыть диалог для ввода имени пользователя
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Create Private Chat",
        GTK_WINDOW(gtk_widget_get_toplevel(main_box)),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_ACCEPT,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL
    );
    gtk_widget_set_name(dialog, "dialog-search-person");
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter username...");
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_set_name(entry, "entry-search-person");

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *username = gtk_entry_get_text(GTK_ENTRY(entry));
        g_print("Creating private chat with: %s\n", username);
        cJSON *createChat_request = form_create_chat_with(username);
        prepare_and_send_json(createChat_request, main_data);
        // Добавьте здесь логику для создания чата
        
        GtkWidget *new_chat = gtk_button_new_with_label(username);
        gtk_box_pack_start(GTK_BOX(main_box), new_chat, FALSE, FALSE, 5);
        g_signal_connect(new_chat, "clicked", G_CALLBACK(on_chat_button_clicked), new_chat);
        gtk_widget_set_name(new_chat, "button-new-chat");

        gtk_widget_show_all(main_box);
    }

    gtk_widget_destroy(dialog);
}
