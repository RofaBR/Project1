#include "../inc/client.h"

void update_error_label(GtkWidget *error_label, const char *message) {
    if (message) {
        gtk_label_set_text(GTK_LABEL(error_label), message);
        gtk_widget_show(error_label);
    } else {
        gtk_label_set_text(GTK_LABEL(error_label), "");
        gtk_widget_hide(error_label);
    }
}

void on_create_account_button_clicked(GtkWidget *button, gpointer data) {
    t_main *main_data = (t_main *)g_object_get_data(G_OBJECT(button), "main_data");

    GtkWidget *vbox = GTK_WIDGET(data);

    GList *children = gtk_container_get_children(GTK_CONTAINER(vbox));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    GtkWidget *logo = create_image("img/logo.svg", 110, 100);
    gtk_box_pack_start(GTK_BOX(vbox), logo, FALSE, FALSE, 0);

    GtkWidget *create_account_label = gtk_label_new("Create Account");
    gtk_widget_set_name(create_account_label, "title-label");

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Enter username");
    gtk_widget_set_name(username_entry, "username_entry"); 
    gtk_entry_set_max_length(GTK_ENTRY(username_entry), 100); 

    GtkWidget *login_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(login_entry), "Enter login");
    gtk_widget_set_name(login_entry, "login_entry");
    gtk_entry_set_max_length(GTK_ENTRY(login_entry), 100); 

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Enter password"); 
    gtk_widget_set_name(password_entry, "password_entry");  
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_entry_set_max_length(GTK_ENTRY(password_entry), 100); 

    GtkWidget *confirm_password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(confirm_password_entry), "Enter password again");
    gtk_widget_set_name(confirm_password_entry, "confirm_password_entry");
    gtk_entry_set_visibility(GTK_ENTRY(confirm_password_entry), FALSE); 
    gtk_entry_set_max_length(GTK_ENTRY(confirm_password_entry), 100); 

    GtkWidget *error_label = gtk_label_new("");
    gtk_widget_set_name(error_label, "error-label");
    GtkStyleContext *context = gtk_widget_get_style_context(error_label);
    gtk_style_context_add_class(context, "error-label");
    g_object_set_data(G_OBJECT(main_data->buff), "error-label", error_label);
    
    GtkWidget *submit_create_account_button = gtk_button_new_with_label("Submit Account");
    gtk_widget_set_name(submit_create_account_button, "submit_create_account_button");
    GtkWidget *back_to_login_button = gtk_button_new_with_label("Back to Log In");
    gtk_widget_set_name(back_to_login_button, "back_to_login_button");
    g_object_set_data(G_OBJECT(back_to_login_button), "main_data", main_data);

    gtk_box_pack_start(GTK_BOX(vbox), create_account_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), username_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), login_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), password_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), confirm_password_entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), error_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), submit_create_account_button, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), back_to_login_button, FALSE, FALSE, 5);

    g_object_set_data(G_OBJECT(submit_create_account_button), "main_data", main_data);
    g_object_set_data(G_OBJECT(submit_create_account_button), "username_entry", username_entry);
    g_object_set_data(G_OBJECT(submit_create_account_button), "login_entry", login_entry);
    g_object_set_data(G_OBJECT(submit_create_account_button), "password_entry", password_entry);
    g_object_set_data(G_OBJECT(submit_create_account_button), "confirm_password_entry", confirm_password_entry);
    g_object_set_data(G_OBJECT(submit_create_account_button), "error_label", error_label);


    g_signal_connect(submit_create_account_button, "clicked", G_CALLBACK(on_submit_account_button_clicked), vbox);
    g_signal_connect(back_to_login_button, "clicked", G_CALLBACK(restore_login_form), vbox);

    gtk_widget_show_all(vbox);
}

void on_login_button_clicked(GtkWidget *button, gpointer data) {
    (void)data;
    t_main *main_data = (t_main *)g_object_get_data(G_OBJECT(button), "main_data");
    if (!main_data) {
        g_print("Error: main_data is NULL in on_login_button_clicked\n");
        return;
    }
    GtkWidget *username_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "username_entry"));
    GtkWidget *password_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "password_entry"));
    GtkWidget *error_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "error_label")); 

    const char *login = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));

    if (mx_strcmp(password, "") == 0 || mx_strcmp(login, "") == 0) {
        update_error_label(error_label, "Fill in all fields");
        return;
    }

    t_user* user = mx_create_log_user(login, password);
    
    cJSON *login_request = form_login_request(user->login, user->password);

    prepare_and_send_json(login_request, main_data);

    mx_free_user(user);
    update_error_label(error_label, NULL);
}

void on_submit_account_button_clicked(GtkWidget *button, gpointer data) {
    GtkWidget *vbox = GTK_WIDGET(data);
    (void)vbox;
    
    t_main *main_data = (t_main *)g_object_get_data(G_OBJECT(button), "main_data");
    if (!main_data) {
        g_print("Error: main_data is NULL in on_submit_account_button_clicked\n");
        return;
    }

    GtkWidget *username_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "username_entry"));
    GtkWidget *login_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "login_entry"));
    GtkWidget *password_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "password_entry"));
    GtkWidget *confirm_password_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "confirm_password_entry"));
    GtkWidget *error_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "error_label"));
    
    const char *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const char *login = gtk_entry_get_text(GTK_ENTRY(login_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
    const char *confirm_password = gtk_entry_get_text(GTK_ENTRY(confirm_password_entry));

    if (mx_strcmp(username, "") == 0 || mx_strcmp(password, "") == 0 || mx_strcmp(login, "") == 0) {
        update_error_label(error_label, "Fill in all fields");
        return;
    }

    if (mx_strlen(password) < 8) {
        update_error_label(error_label, "Password must be\nlonger than 8 characters");
        return;
    }
    
    if (mx_strcmp(password, confirm_password) != 0) {
        update_error_label(error_label, "Passwords don't match!");
        return;
    }

    t_user* user = mx_create_user(login, username, password);
	
    g_print("Account created successfully:\n");
    
    cJSON *register_request = form_register_request(user->login, user->name, user->password);

    prepare_and_send_json(register_request, main_data);

    mx_free_user(user);
    update_error_label(error_label, NULL);
}
