#include "../inc/client.h"

gboolean gtk_update_notification_label(gpointer user_data) {
    int status = GPOINTER_TO_INT(user_data);

    GtkWidget *login_window = g_object_get_data(G_OBJECT(main_data->buff), "login-window");
    if (!login_window) {
        fprintf(stderr, "Login window not found\n");
        return G_SOURCE_REMOVE;
    }

    GtkWidget *notification_label = g_object_get_data(G_OBJECT(main_data->buff), "error-label");
    if (!notification_label) {
        fprintf(stderr, "Notification label not found\n");
        return G_SOURCE_REMOVE;
    }

    const gchar *message = NULL;
    GtkStyleContext *context = gtk_widget_get_style_context(notification_label);

    if (status == 1) {
        message = "Operation successful";
        gtk_style_context_remove_class(context, "error-label");
        gtk_style_context_add_class(context, "success-label");
    } else {
        message = "An error occurred";
        gtk_style_context_remove_class(context, "success-label");
        gtk_style_context_add_class(context, "error-label");
    }

    gtk_label_set_text(GTK_LABEL(notification_label), message);
    gtk_widget_show(notification_label);

    return G_SOURCE_REMOVE;
}

gboolean gtk_destroy_login_window(gpointer data) {
    (void)data;
    GtkWidget *login_window = g_object_get_data(G_OBJECT(main_data->buff), "login-window");
    if (login_window) {
        gtk_widget_destroy(login_window);
    }
    return G_SOURCE_REMOVE;
}

void handle_login_response(cJSON *json_payload) {
    cJSON *status = cJSON_GetObjectItemCaseSensitive(json_payload, "status");
    if (!cJSON_IsBool(status)) {
        fprintf(stderr, "Missing or invalid 'status' in JSON data\n");
        return;
    }

    int login_status = status->valueint ? 1 : 0;

    if (login_status) {
        printf("Login was successful\n");

        cJSON *data = cJSON_GetObjectItemCaseSensitive(json_payload, "data");
        if (!data) {
            fprintf(stderr, "Missing 'data' in JSON response\n");
            return;
        }

        t_bee_user user;
        memset(&user, 0, sizeof(t_bee_user));

        cJSON *login = cJSON_GetObjectItemCaseSensitive(data, "login");
        cJSON *username = cJSON_GetObjectItemCaseSensitive(data, "username");
        cJSON *created_at = cJSON_GetObjectItemCaseSensitive(data, "created_at");
        cJSON *logo_id = cJSON_GetObjectItemCaseSensitive(data, "logo_id");

        if (cJSON_IsString(login)) {
            size_t decoded_len;
            unsigned char *decoded = base64_decode(login->valuestring, &decoded_len);
            if (decoded) {
                user.login = strndup((char *)decoded, decoded_len);
                free(decoded);
            }
        }

        if (cJSON_IsString(username)) {
            size_t decoded_len;
            unsigned char *decoded = base64_decode(username->valuestring, &decoded_len);
            if (decoded) {
                user.username = strndup((char *)decoded, decoded_len);
                free(decoded);
            }
        }

        if (cJSON_IsString(created_at)) {
            size_t decoded_len;
            unsigned char *decoded = base64_decode(created_at->valuestring, &decoded_len);
            if (decoded) {
                user.created_at = strndup((char *)decoded, decoded_len);
                free(decoded);
            }
        }

        if (cJSON_IsNumber(logo_id)) {
            user.logo_id = logo_id->valueint;
        }

        printf("User login: %s, username: %s, created at: %s, logo_id: %d\n",
               user.login, user.username, user.created_at, user.logo_id);


        g_idle_add(gtk_create_main_window, NULL);
        g_idle_add(gtk_destroy_login_window, NULL);

        free(user.login);
        free(user.username);
        free(user.created_at);

    } else {
        printf("Login failed\n");

        cJSON *error_message = cJSON_GetObjectItemCaseSensitive(json_payload, "data");
        if (cJSON_IsString(error_message)) {
            printf("Error: %s\n", error_message->valuestring);
        }

        g_idle_add(gtk_update_notification_label, GINT_TO_POINTER(0));
    }
}

void handle_register_response(cJSON *json_payload) {
    cJSON *status = cJSON_GetObjectItemCaseSensitive(json_payload, "status");
    if (!cJSON_IsBool(status)) {
        fprintf(stderr, "Missing or invalid 'status' in JSON data\n");
        g_idle_add(gtk_update_notification_label, GINT_TO_POINTER(0));
        return;
    }

    int register_status = status->valueint ? 1 : 0;

    if (register_status) {
        printf("Registration was successful\n");
        g_idle_add(gtk_update_notification_label, GINT_TO_POINTER(1));
    } else {
        printf("Registration failed\n");

        cJSON *error_message = cJSON_GetObjectItemCaseSensitive(json_payload, "data");
        if (cJSON_IsString(error_message)) {
            printf("Error: %s\n", error_message->valuestring);
        }

        g_idle_add(gtk_update_notification_label, GINT_TO_POINTER(0));
    }
}

void handle_create_chat_response(cJSON *json_payload) {
    cJSON *status = cJSON_GetObjectItemCaseSensitive(json_payload, "status");
    if (!cJSON_IsBool(status)) {
        fprintf(stderr, "Missing or invalid 'status' in JSON data\n");
        return;
    }

    int creation_status = status->valueint ? 1 : 0;

    if (creation_status) {
        printf("Chat creation was successful\n");

        cJSON *data = cJSON_GetObjectItemCaseSensitive(json_payload, "data");
        if (!data) {
            fprintf(stderr, "Missing 'data' in JSON response\n");
            return;
        }

        t_bee_group *group = malloc(sizeof(t_bee_group));
        if (!group) {
            perror("Failed to allocate memory for new group");
            return;
        }

        memset(group, 0, sizeof(t_bee_group));

        cJSON *id = cJSON_GetObjectItemCaseSensitive(data, "id");
        cJSON *name = cJSON_GetObjectItemCaseSensitive(data, "name");
        cJSON *is_private = cJSON_GetObjectItemCaseSensitive(data, "is_private");
        cJSON *created_by = cJSON_GetObjectItemCaseSensitive(data, "created_by");
        cJSON *creator_username = cJSON_GetObjectItemCaseSensitive(data, "creator_username");
        cJSON *created_at = cJSON_GetObjectItemCaseSensitive(data, "created_at");
        cJSON *last_message_date = cJSON_GetObjectItemCaseSensitive(data, "last_message_date");

        if (cJSON_IsNumber(id)) {
            group->id = id->valueint;
        }

        if (cJSON_IsString(name)) {
            size_t decoded_len;
            unsigned char *decoded = base64_decode(name->valuestring, &decoded_len);
            if (decoded) {
                group->name = strndup((char *)decoded, decoded_len);
                free(decoded);
            }
        }

        if (cJSON_IsNumber(is_private)) {
            group->is_private = is_private->valueint;
        }

        if (cJSON_IsNumber(created_by)) {
            group->created_by = created_by->valueint;
        }

        if (cJSON_IsString(creator_username)) {
            size_t decoded_len;
            unsigned char *decoded = base64_decode(creator_username->valuestring, &decoded_len);
            if (decoded) {
                group->creator_username = strndup((char *)decoded, decoded_len);
                free(decoded);
            }
        }

        if (cJSON_IsString(created_at)) {
            size_t decoded_len;
            unsigned char *decoded = base64_decode(created_at->valuestring, &decoded_len);
            if (decoded) {
                group->created_at = strndup((char *)decoded, decoded_len);
                free(decoded);
            }
        }

        if (cJSON_IsString(last_message_date)) {
            size_t decoded_len;
            unsigned char *decoded = base64_decode(last_message_date->valuestring, &decoded_len);
            if (decoded) {
                group->last_message_date = strndup((char *)decoded, decoded_len);
                free(decoded);
            }
        }

        printf("Group ID: %d, Name: %s, Private: %d, Created By: %d, Creator Username: %s, Created At: %s, Last Message Date: %s\n",
               group->id, group->name, group->is_private, group->created_by, group->creator_username,
               group->created_at, group->last_message_date);

        mx_add_group(main_data, group);

    } else {
        printf("Chat creation failed\n");

        cJSON *error_message = cJSON_GetObjectItemCaseSensitive(json_payload, "data");
        if (cJSON_IsString(error_message)) {
            printf("Error: %s\n", error_message->valuestring);
        }
    }
}

