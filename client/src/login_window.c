#include "../inc/client.h"

void login_window(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *center_box;
    GtkWidget *vbox;
    GtkWidget *logo, *title_label, *username_entry, *password_entry, *error_label;
    GtkWidget *login_button, *create_account_button;
    t_main *main_data = (t_main *)user_data;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Bee chat");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_name(window, "main-window");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(center_box), vbox);
    gtk_container_add(GTK_CONTAINER(window), center_box);

    logo = create_image("img/logo.svg", 110, 100);
    gtk_box_pack_start(GTK_BOX(vbox), logo, FALSE, FALSE, 0);

    title_label = gtk_label_new("Log In");
    gtk_widget_set_name(title_label, "title-label");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Enter your login");
    gtk_widget_set_name(username_entry, "username_entry");
    gtk_box_pack_start(GTK_BOX(vbox), username_entry, FALSE, FALSE, 5);

    password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Enter your password");
    gtk_widget_set_name(password_entry, "password_entry");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(password_entry), 8226);
    gtk_box_pack_start(GTK_BOX(vbox), password_entry, FALSE, FALSE, 5);

    error_label = gtk_label_new("");
    gtk_widget_set_name(error_label, "error-label");
    gtk_box_pack_start(GTK_BOX(vbox), error_label, FALSE, FALSE, 5);
    g_object_set_data(G_OBJECT(main_data->buff), "error-label", error_label);

    login_button = gtk_button_new_with_label("Enter Hive");
    gtk_widget_set_name(login_button, "login_button");

    g_object_set_data(G_OBJECT(login_button), "main_data", main_data);
    g_object_set_data(G_OBJECT(login_button), "username_entry", username_entry);
    g_object_set_data(G_OBJECT(login_button), "password_entry", password_entry);
    g_object_set_data(G_OBJECT(login_button), "error_label", error_label);

    gtk_box_pack_start(GTK_BOX(vbox), login_button, FALSE, FALSE, 5);
    g_object_set_data(G_OBJECT(main_data->buff), "login-window", window);
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), NULL);

    create_account_button = gtk_button_new_with_label("Become Member");
    gtk_widget_set_name(create_account_button, "create_account_button");

    g_object_set_data(G_OBJECT(create_account_button), "main_data", main_data);

    gtk_box_pack_start(GTK_BOX(vbox), create_account_button, FALSE, FALSE, 5);
    g_signal_connect(create_account_button, "clicked", G_CALLBACK(on_create_account_button_clicked), vbox);

    gtk_widget_show_all(window);
}

