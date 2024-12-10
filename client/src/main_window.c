#include "../inc/bee_user.h"

const char* uses_chat = NULL;
GtkWidget *chat_entry;
GtkWidget *message_area;



void on_add_message_button_clicked(GtkWidget *button, gpointer user_data) {
    (void)button;
    GtkEntry *entry = GTK_ENTRY(user_data);
    
    if (uses_chat == NULL) {
        mx_printerr("Chat not chosen\n");
        gtk_entry_set_text(entry, "");

        GtkWidget *dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Error");
        gtk_widget_set_name(dialog, "dialog-search-person");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(entry))));

        GtkWidget *error = gtk_label_new("Chat not chosen");
        gtk_widget_set_name(error, "error-label");

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), error);
        gtk_widget_show_all(dialog);
        g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    } else {
        const char *message = gtk_entry_get_text(entry);
        if (message == NULL || mx_strlen(message) == 0) {
            mx_printerr("No message entered\n");
            gtk_entry_set_text(entry, "");
        } else {
            printf("Text entered: %s\n", message);
            add_message_to_chat(message, NULL);

            gtk_entry_set_text(entry, "");
        }
    }
}

gboolean check_connection(gpointer user_data) {
    t_main *main_data = (t_main *)user_data;
    if (!main_data->is_connected) {
        GtkWidget *main_window = g_object_get_data(G_OBJECT(main_data->buff), "main-window");
        if (main_window) {
            gtk_widget_destroy(main_window);
        }

        login_window(main_data->app, main_data);

        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

gboolean gtk_create_main_window(gpointer user_data) {
    (void)user_data;
    GtkWidget *main_window;
    GtkWidget *main_box;

    GtkWidget *vertical_box;

    GtkWidget *sidebar_box;

    GtkWidget *chat_box;

    GtkWidget *topbar_box;
    GtkWidget *exit_button;
    GtkWidget *exit_image;
    GtkWidget *chat_entry_box;

    main_window = gtk_application_window_new(GTK_APPLICATION(main_data->app));
    gtk_window_set_title(GTK_WINDOW(main_window), "BEE CHAT");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1280, 800);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE);

    GtkStyleContext *main_context = gtk_widget_get_style_context(main_window);
    gtk_style_context_add_class(main_context, "main-window");

    vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vertical_box);

    topbar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(topbar_box, "top_bar");
    gtk_widget_set_size_request(topbar_box, -1, 60);
    gtk_box_pack_start(GTK_BOX(vertical_box), topbar_box, FALSE, FALSE, 0);

    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical_box), main_box, TRUE, TRUE, 0);

    GtkWidget *sidebar_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(sidebar_scrolled, 300, -1);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sidebar_scrolled),
                                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(main_box), sidebar_scrolled, FALSE, TRUE, 0);

    sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(sidebar_scrolled), sidebar_box);
    GtkStyleContext *sidebar_context = gtk_widget_get_style_context(sidebar_box);
    gtk_style_context_add_class(sidebar_context, "side-box");

    exit_button = gtk_button_new();
    exit_image = create_image("img/exit_icon.png", 50, 50);
    gtk_button_set_image(GTK_BUTTON(exit_button), exit_image);
    gtk_widget_set_name(exit_button, "exit-button");
    gtk_widget_set_halign(exit_button, GTK_ALIGN_END);
    gtk_widget_set_valign(exit_button, GTK_ALIGN_CENTER);
    gtk_box_pack_end(GTK_BOX(topbar_box), exit_button, FALSE, FALSE, 0);
    gtk_widget_set_name(exit_button, "exit-button");
    gtk_widget_set_size_request(exit_button, 45, 45);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(on_exit_button_clicked), main_window);

    chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), chat_box, TRUE, TRUE, 0);
    GtkStyleContext *chat_context = gtk_widget_get_style_context(chat_box);
    gtk_style_context_add_class(chat_context, "chat-box");

    GtkWidget *message_scrolled = gtk_scrolled_window_new(NULL, NULL);
    message_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(message_scrolled, TRUE);
    gtk_widget_set_hexpand(message_scrolled, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_scrolled),
                                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(chat_box), message_scrolled, TRUE, TRUE, 0);

    message_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(message_scrolled), message_area);
    GtkStyleContext *message_area_context = gtk_widget_get_style_context(message_area);
    gtk_style_context_add_class(message_area_context, "message-area");

    chat_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(chat_entry_box, "chat_entry_box");
    gtk_box_pack_end(GTK_BOX(chat_box), chat_entry_box, FALSE, FALSE, 0);

    GtkWidget *button_create_chat = gtk_button_new();
    GtkWidget *image_create_chat = create_image("img/addrofa.svg", 20, 15);
    gtk_button_set_image(GTK_BUTTON(button_create_chat), image_create_chat);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), button_create_chat, FALSE, FALSE, 0);
    gtk_widget_set_name(button_create_chat, "create-chat-button");

    g_signal_connect(button_create_chat, "clicked", G_CALLBACK(on_button_create_chat_clicked), sidebar_box);

    GtkWidget *file_button = gtk_button_new();
    GtkWidget *image_load = create_image("img/uploadrofa.svg", 20, 15);
    gtk_button_set_image(GTK_BUTTON(file_button), image_load);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), file_button, FALSE, FALSE, 0);
    gtk_widget_set_name(file_button, "create-chat-button");
    gtk_widget_set_margin_start(file_button, 30);
    g_signal_connect(file_button, "clicked", G_CALLBACK(on_file_button_clicked), main_window);

    GtkWidget *emoji_button = gtk_button_new_with_label("😊");
    gtk_box_pack_start(GTK_BOX(chat_entry_box), emoji_button, FALSE, FALSE, 0);
    gtk_widget_set_name(emoji_button, "create-chat-button");
    gtk_widget_set_margin_start(emoji_button, 10);
    g_signal_connect(emoji_button, "clicked", G_CALLBACK(on_emoji_button_clicked), emoji_button);

    chat_entry = gtk_entry_new();
    gtk_widget_set_name(chat_entry, "chat_entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(chat_entry), "Type a message...");
    gtk_widget_set_size_request(chat_entry, 500, 40);
    gtk_widget_set_halign(chat_entry, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), chat_entry, FALSE, FALSE, 0);
    gtk_widget_set_margin_start(chat_entry, 150);

    GtkWidget *send_button = gtk_button_new();
    GtkWidget *image_send_button = create_image("img/sendrofa.svg", 20, 15);
    gtk_button_set_image(GTK_BUTTON(send_button), image_send_button);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), send_button, FALSE, FALSE, 0);
    gtk_widget_set_name(send_button, "send-button");
    gtk_widget_set_margin_start(send_button, 50);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_add_message_button_clicked), chat_entry);

    g_object_set_data(G_OBJECT(main_data->buff), "main-window", main_window);

    g_timeout_add(1000, check_connection, main_data);

    gtk_widget_show_all(main_window);

    return G_SOURCE_REMOVE;
}
