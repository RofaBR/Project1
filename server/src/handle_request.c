#include "../inc/server.h"

void handle_login_request(cJSON *json_payload, t_client *client) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in handle_login_request");
        return;
    }

    cJSON *login_item = cJSON_GetObjectItemCaseSensitive(json_payload, "userlogin");
    cJSON *password_item = cJSON_GetObjectItemCaseSensitive(json_payload, "password");

    if (!cJSON_IsString(login_item) || !login_item->valuestring ||
        !cJSON_IsString(password_item) || !password_item->valuestring) {
        syslog(LOG_ERR, "Missing or invalid fields in login request");
        return;
    }

    size_t decoded_len;
    unsigned char *decoded_login = base64_decode(login_item->valuestring, &decoded_len);
    unsigned char *decoded_password = base64_decode(password_item->valuestring, &decoded_len);

    if (!decoded_login || !decoded_password) {
        syslog(LOG_ERR, "Failed to decode Base64 fields in login request");
        free(decoded_login);
        free(decoded_password);
        return;
    }

    char user_login[SHA256_DIGEST_LENGTH * 2 + 1];
    char user_password[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&user_login[i * 2], "%02x", decoded_login[i]);
        sprintf(&user_password[i * 2], "%02x", decoded_password[i]);
    }
    user_login[SHA256_DIGEST_LENGTH * 2] = '\0';
    user_password[SHA256_DIGEST_LENGTH * 2] = '\0';

    free(decoded_login);
    free(decoded_password);

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "login");

    t_user *user = db_user_read_by_login(user_login);

    if (!user) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Couldn't find a user with this login.");
    } else {
        if (strcmp(user->password, user_password) == 0) {
            cJSON *json_user = user_to_json(user);
            if (!json_user) {
                cJSON_AddBoolToObject(json, "status", false);
                cJSON_AddStringToObject(json, "data", "Server error.");
            } else {
                cJSON_AddBoolToObject(json, "status", true);
                cJSON_AddItemToObject(json, "data", json_user);
                client->id_db = user->id;
            }
        } else {
            cJSON_AddBoolToObject(json, "status", false);
            cJSON_AddStringToObject(json, "data", "Wrong password.");
        }
        free_user(&user);
    }

    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Login request received. Decoded Login: %s, Decoded Password: %s", user_login, user_password);
}

void handle_register_request(cJSON *json_payload, t_client *client) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in handle_register_request");
        return;
    }

    cJSON *login_item = cJSON_GetObjectItemCaseSensitive(json_payload, "userlogin");
    cJSON *username_item = cJSON_GetObjectItemCaseSensitive(json_payload, "username");
    cJSON *password_item = cJSON_GetObjectItemCaseSensitive(json_payload, "password");

    if (!cJSON_IsString(login_item) || !login_item->valuestring ||
        !cJSON_IsString(username_item) || !username_item->valuestring ||
        !cJSON_IsString(password_item) || !password_item->valuestring) {
        syslog(LOG_ERR, "Missing or invalid fields in register request");
        return;
    }

    size_t decoded_login_len, decoded_username_len, decoded_password_len;
    unsigned char *decoded_login = base64_decode(login_item->valuestring, &decoded_login_len);
    unsigned char *decoded_username = base64_decode(username_item->valuestring, &decoded_username_len);
    unsigned char *decoded_password = base64_decode(password_item->valuestring, &decoded_password_len);

    if (!decoded_login || !decoded_username || !decoded_password) {
        syslog(LOG_ERR, "Failed to decode Base64 fields in register request");
        free(decoded_login);
        free(decoded_username);
        free(decoded_password);
        return;
    }

    char *username = malloc(decoded_username_len + 1);
    memcpy(username, decoded_username, decoded_username_len);
    username[decoded_username_len] = '\0';

    char user_login[SHA256_DIGEST_LENGTH * 2 + 1];
    char user_password[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&user_login[i * 2], "%02x", decoded_login[i]);
        sprintf(&user_password[i * 2], "%02x", decoded_password[i]);
    }
    user_login[SHA256_DIGEST_LENGTH * 2] = '\0';
    user_password[SHA256_DIGEST_LENGTH * 2] = '\0';

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "register");

    t_user *existing = db_user_read_by_login(user_login);

    if(existing) {
        free_user(&existing);
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "User already exists.");
    }
    else {
         t_user *user = user_create(username, user_login, user_password, 1);
         int user_id = db_user_create(user);

        if (user_id < 0) {
            cJSON_AddBoolToObject(json, "status", false);
            cJSON_AddStringToObject(json, "data", "Could not record user data.");
        } else {
            cJSON *json_user = user_to_json(user);
            if (!json_user) {
                cJSON_AddBoolToObject(json, "status", false);
                cJSON_AddStringToObject(json, "data", "Server error.");
            } else {
                client->id_db = user_id;
                cJSON_AddBoolToObject(json, "status", true);
                cJSON_AddItemToObject(json, "data", json_user);
            }
            free_user(&user);
        }
    }

    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Register request received. Decoded Login: %s, Username: %s, Password: %s", user_login, username, user_password);

    free(decoded_login);
    free(decoded_username);
    free(decoded_password);
}

void handle_group_create_request(cJSON* json_payload, t_client* client) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in handle_group_create_request");
        return;
    }

    cJSON* login_item = cJSON_GetObjectItemCaseSensitive(json_payload, "userlogin");
    if (!cJSON_IsString(login_item) || !login_item->valuestring) {
        syslog(LOG_ERR, "Missing or invalid fields in create group request");
        return;
    }

    size_t decoded_len;
    unsigned char* decoded_login = base64_decode(login_item->valuestring, &decoded_len);

    if (!decoded_login) {
        syslog(LOG_ERR, "Failed to decode Base64 fields in create group request");
        return;
    }

    char login[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(&login[i * 2], 3, "%02x", decoded_login[i]);
    }
    free(decoded_login);

    cJSON* json = cJSON_CreateObject();
    if (!json) {
        syslog(LOG_ERR, "Failed to create JSON object");
        return;
    }

    cJSON_AddStringToObject(json, "response_type", "create_group");
    syslog(LOG_INFO, "Create group request processed successfully. Decoded login: %s", login);
    t_user* user = db_user_read_by_login(login);
    if (!user) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "User not found.");
        prepare_and_send_json(json, client);
        return;
    }

    if (user->id == client->id_db) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Why text yourself? That's called thoughts!");
        free_user(&user);
        prepare_and_send_json(json, client);
        return;
    }

    if (db_private_group_exists(user->id, client->id_db)) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "You and this user are already chatting.");
        free_user(&user);
        prepare_and_send_json(json, client);
        return;
    }

    t_group* group = group_create("", client->id_db, 1);
    if (!group) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Couldn't create chat.");
        free_user(&user);
        prepare_and_send_json(json, client);
        return;
    }

    int group_id = db_group_create(group);
    if (group_id < 0) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Couldn't create chat.");
        free_group(&group);
        free_user(&user);
        prepare_and_send_json(json, client);
        return;
    }

    if (db_user_add_to_group(client->id_db, group_id) < 0 || db_user_add_to_group(user->id, group_id) < 0) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Error creating chat.");
        db_group_delete_by_id(group_id);
        free_group(&group);
        free_user(&user);
        prepare_and_send_json(json, client);
        return;
    }

    free_group(&group);
    group = db_group_read_by_id(group_id);
    if (!group) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Server error.");
        free_user(&user);
        prepare_and_send_json(json, client);
        return;
    }

    mx_strdel(&group->name);
    group->name = strdup(group->creator_username);

    cJSON* json_group = group_to_json(group);
    if (!json_group) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Server error.");
        free_group(&group);
        free_user(&user);
        prepare_and_send_json(json, client);
        return;
    }

    cJSON_AddBoolToObject(json, "status", true);
    cJSON_AddItemToObject(json, "data", json_group);

    send_to_client_by_id(json, user->id);

    mx_strdel(&group->name);
    group->name = strdup(user->username);

    json_group = group_to_json(group);
    if (!json_group) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Server error.");
    } else {
        cJSON_ReplaceItemInObject(json, "data", json_group);
        send_to_client_by_id(json, client->id_db);
    }

    free_group(&group);
    free_user(&user);
    cJSON_Delete(json);
}

/*
void handle_message_edit_request(cJSON *json_payload, t_client *client) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in handle_message_edit_request");
        return;
    }

    cJSON *id_item = cJSON_GetObjectItemCaseSensitive(json_payload, "message_id");
    cJSON *text_item = cJSON_GetObjectItemCaseSensitive(json_payload, "text");

    if (!cJSON_IsNumber(id_item) || !id_item->valueint ||
        !cJSON_IsString(text_item) || !text_item->valuestring) {
        syslog(LOG_ERR, "Missing or invalid fields in edit message request");
        return;
    }

    int id = id_item->valueint;

    size_t decoded_len;
    unsigned char *decoded_text = base64_decode(text_item->valuestring, &decoded_len);

    if (!decoded_text) {
        syslog(LOG_ERR, "Failed to decode Base64 fields in edit message request");
        free(decoded_text);
        return;
    }
      
    char text[SHA256_DIGEST_LENGTH * 2 + 1];

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&text[i * 2], "%02x", decoded_text[i]);
    }
    text[SHA256_DIGEST_LENGTH * 2] = '\0';

    free(decoded_text);

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "edit_message");

    t_message* message = db_message_read_by_id(id);

    if (!message) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Message not found.");
    } else {
        mx_strdel(&message->text);
        message->text = mx_strdup(text);

        if (db_message_update(message)) {
            t_list* users = db_user_read_by_group_id(message->group_id);

            cJSON *json_msg = message_to_json(message);
            if (!json_msg) {
                cJSON_AddBoolToObject(json, "status", false);
                cJSON_AddStringToObject(json, "data", "Server error.");
            } else {
                cJSON_AddBoolToObject(json, "status", true);
                cJSON_AddItemToObject(json, "data", json_msg);
                
                // SEND TO ALL CLIENTS FROM THE LIST
                if(users) free_user_list(users);
                free_message(&message);

                syslog(LOG_INFO, "Edit message request received. Decoded id: %d, Decoded text: %s", id, text);

                return;
            }
             
            if(users) free_user_list(users);

        } else {
            cJSON_AddBoolToObject(json, "status", false);
            cJSON_AddStringToObject(json, "data", "Update failed.");
        }
        free_message(&message);
    }

    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Edit message request received. Decoded id: %d, Decoded text: %s", id, text);
}


void handle_message_delete_request(cJSON *json_payload, t_client *client) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in handle_message_delete_request");
        return;
    }

    cJSON *id_item = cJSON_GetObjectItemCaseSensitive(json_payload, "message_id");

    if (!cJSON_IsNumber(id_item) || !id_item->valueint) {
        syslog(LOG_ERR, "Missing or invalid fields in delete message request");
        return;
    }

    int id = id_item->valueint;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "delete_message");

    t_message* message = db_message_read_by_id(id);

    if (!message) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Message not found.");
    } else {
        if (db_message_delete_by_id(message->id)) {
            t_list* users = db_user_read_by_group_id(message->group_id);

            cJSON *json_data = cJSON_CreateObject();
            cJSON_AddNumberToObject(json_data, "message_id", (const double)user->message->id);
            cJSON_AddNumberToObject(json_data, "group_id", (const double)user->message->group_id);
            
            // SEND TO ALL CLIENTS FROM THE LIST
             
            if(users) free_user_list(users);
            free_message(&message);

            syslog(LOG_INFO, "Delete message request received. Decoded id: %d", id);

            return;

        } else {
            cJSON_AddBoolToObject(json, "status", false);
            cJSON_AddStringToObject(json, "data", "Delete failed.");
        }
        free_message(&message);
    }

    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Delete message request received. Decoded id: %d", id);
}

void handle_message_send_request(cJSON *json_payload, t_client *client) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in handle_message_send_request");
        return;
    }

    cJSON *group_id_item = cJSON_GetObjectItemCaseSensitive(json_payload, "group_id");
    cJSON *text_item = cJSON_GetObjectItemCaseSensitive(json_payload, "text");

    if (!cJSON_IsNumber(group_id_item) || !group_id_item->valueint ||
        !cJSON_IsString(text_item) || !text_item->valuestring) {
        syslog(LOG_ERR, "Missing or invalid fields in send message request");
        return;
    }

    int group_id = group_id_item->valueint;

    size_t decoded_len;
    unsigned char *decoded_text = base64_decode(text_item->valuestring, &decoded_len);

    if (!decoded_text) {
        syslog(LOG_ERR, "Failed to decode Base64 fields in send message request");
        free(decoded_text);
        return;
    }
      
    char text[SHA256_DIGEST_LENGTH * 2 + 1];

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&text[i * 2], "%02x", decoded_text[i]);
    }
    text[SHA256_DIGEST_LENGTH * 2] = '\0';

    free(decoded_text);

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "send_message");

    t_message* message = message_create(client->id_db, text, group_id);
    int msg =_id = db_message_create(message);
    free(&message);

    message = db_message_read_by_id(msg_id);

    if (!message) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "Could not send the message.");
    } else {
            t_list* users = db_user_read_by_group_id(message->group_id);

            cJSON *json_msg = message_to_json(message);
            if (!json_msg) {
                cJSON_AddBoolToObject(json, "status", false);
                cJSON_AddStringToObject(json, "data", "Server error.");
            } else {
                cJSON_AddBoolToObject(json, "status", true);
                cJSON_AddItemToObject(json, "data", json_msg);
                
                // SEND TO ALL CLIENTS FROM THE LIST
                if(users) free_user_list(users);
                free_message(&message);
            }
             
            if(users) free_user_list(users);
        }
        free_message(&message);
    }

    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Send message request received. Decoded group id: %d, Decoded text: %s", group_id, text);
}

void handle_get_groups_request(cJSON *json_payload, t_client *client) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "get_groups");

    t_list* groups = db_group_read_by_user_id(client->id_db);

    if (!groups) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "No groups found.");
    } else {
        cJSON *array = cJSON_CreateArray();

        t_list* current = groups;
        while(current) {
            t_group* g = (t_group*)current->data;
            cJSON* json_group = group_to_json(g);
            cJSON_AddItemToArray(array, json_group);

            current = current->next;
        }

        free_group_list(groups);

        cJSON_AddBoolToObject(json, "status", true);
        cJSON_AddItemToObject(json, "groups", array);
    }

    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Get groups request received.");
}

void handle_get_messages_request(cJSON *json_payload, t_client *client) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in handle_get_messages_request");
        return;
    }

    cJSON *group_id_item = cJSON_GetObjectItemCaseSensitive(json_payload, "group_id");
    if (!cJSON_IsNumber(group_id_item) || !group_id_item->valueint) {
        syslog(LOG_ERR, "Missing or invalid fields in get messages request");
        return;
    }

    int group_id = group_id_item->valueint;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "get_messages");

    t_list* messages = db_message_read_by_group_id(group_id);

    if (!messages) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "No messages found.");
    } else {
        cJSON *array = cJSON_CreateArray();

        t_list* current = messages;
        while(current) {
            t_message* m = (t_message*)current->data;
            cJSON* json_msg = message_to_json(m);
            cJSON_AddItemToArray(array, json_msg);

            current = current->next;
        }

        free_message_list(messages);

        cJSON_AddBoolToObject(json, "status", true);
        cJSON_AddItemToObject(json, "messages", array);
    }

    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Get messages request received.");
}
*/