#include "../inc/client.h"

void render_chat(t_bee_group *group, GtkWidget *sidebar_box) {
    GtkWidget *chat_button = gtk_button_new_with_label(group->name);
    gtk_box_pack_start(GTK_BOX(sidebar_box), chat_button, FALSE, FALSE, 5);

    g_object_set_data(G_OBJECT(chat_button), "group_data", group);

    g_signal_connect(chat_button, "clicked", G_CALLBACK(on_chat_button_clicked), chat_button);

    gtk_widget_set_name(chat_button, "chat-button");

    gtk_widget_show(chat_button);
}

void render_all_chats(t_main *main_data, GtkWidget *sidebar_box) {
    for (GList *node = main_data->groups; node != NULL; node = node->next) {
        t_bee_group *group = (t_bee_group *)node->data;
        render_chat(group, sidebar_box);
    }
}
