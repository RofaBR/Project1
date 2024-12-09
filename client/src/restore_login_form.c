#include "../inc/client.h"

void restore_login_form(GtkWidget *button, gpointer data) {
    t_main *main_data = (t_main *)g_object_get_data(G_OBJECT(button), "main_data");
    GtkWidget *vbox = GTK_WIDGET(data);

    GList *children = gtk_container_get_children(GTK_CONTAINER(vbox));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    GtkWidget *logo = create_image("img/logo.svg", 110, 100);
    gtk_box_pack_start(GTK_BOX(vbox), logo, FALSE, FALSE, 0);

    GtkWidget *title_label = gtk_label_new("Log In");
    gtk_widget_set_name(title_label, "title-label");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Enter your login");
    gtk_widget_set_name(username_entry, "username_entry");

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Enter your password");
    gtk_widget_set_name(password_entry, "password_entry");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *error_label = gtk_label_new("");
    gtk_widget_set_name(error_label, "error-label");
    

    gtk_box_pack_start(GTK_BOX(vbox), username_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), password_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), error_label, FALSE, FALSE, 5);

    g_object_set_data(G_OBJECT(main_data->buff), "error-label", error_label);
    
    GtkWidget *login_button = gtk_button_new_with_label("Enter Hive");
    gtk_widget_set_name(login_button, "login_button");
    gtk_box_pack_start(GTK_BOX(vbox), login_button, FALSE, FALSE, 5);

    g_object_set_data(G_OBJECT(login_button), "main_data", main_data);
    g_object_set_data(G_OBJECT(login_button), "username_entry", username_entry);
    g_object_set_data(G_OBJECT(login_button), "password_entry", password_entry);
    g_object_set_data(G_OBJECT(login_button), "error_label", error_label);

    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), vbox);


    GtkWidget *create_account_button = gtk_button_new_with_label("Beecome membeer");
    gtk_widget_set_name(create_account_button, "create_account_button");
    gtk_box_pack_start(GTK_BOX(vbox), create_account_button, FALSE, FALSE, 5);

    g_object_set_data(G_OBJECT(create_account_button), "main_data", main_data);
    g_signal_connect(create_account_button, "clicked", G_CALLBACK(on_create_account_button_clicked), vbox);

    gtk_widget_show_all(vbox);
}
