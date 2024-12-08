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
        free(decoded_login);
        return;
    }

    char login[SHA256_DIGEST_LENGTH * 2 + 1];

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&login[i * 2], "%02x", decoded_login[i]);
    }
    login[SHA256_DIGEST_LENGTH * 2] = '\0';

    free(decoded_login);

    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "response_type", "create_group");

    t_user* user = db_user_read_by_login(login);

    if (!user) {
        cJSON_AddBoolToObject(json, "status", false);
        cJSON_AddStringToObject(json, "data", "User not found.");
        syslog(LOG_INFO, "User not found.");
    }
    else {
        if (user->id == client->id_db) {
            cJSON_AddBoolToObject(json, "status", false);
            cJSON_AddStringToObject(json, "data", "Why text yourself? That's called thoughts!");
            syslog(LOG_INFO, "Why text yourself? That's called thoughts!");
        }
        else {
            t_group* group = group_create("", client->id_db, 1);

            int group_id = db_group_create(group);

            if (group_id < 0) {
                cJSON_AddBoolToObject(json, "status", false);
                cJSON_AddStringToObject(json, "data", "Couldn't create chat.");
                syslog(LOG_INFO, "Couldn't create chat.");
            }
            else {
                if (db_user_add_to_group(client->id_db, group_id) < 0 || db_user_add_to_group(user->id, group_id) < 0) {
                    cJSON_AddBoolToObject(json, "status", false);
                    cJSON_AddStringToObject(json, "data", "Error creating chat.");
                    syslog(LOG_INFO, "Error creating chat.");
                    db_group_delete_by_id(group_id);
                }
                else {
                    free_group(&group);
                    group = db_group_read_by_id(group_id);
                    
                    cJSON* json_group = group_to_json(group);
                    if (!json_group) {
                        cJSON_AddBoolToObject(json, "status", false);
                        cJSON_AddStringToObject(json, "data", "Server error.");
                        syslog(LOG_INFO, "Server error.");
                    }
                    else {
                        cJSON_AddBoolToObject(json, "status", true);
                        cJSON_AddItemToObject(json, "data", json_group);

                        // SEND TO SENDER AND RECIPIENT
                        syslog(LOG_INFO, "otpravka do sendera");
                        send_to_client_by_id(json, client->id_db);
                        syslog(LOG_INFO, "otpravka do recivera");
                        send_to_client_by_id(json, user->id);

                        free_group(&group);
                        free_user(&user);

                        syslog(LOG_INFO, "Create group request received. Decoded login: %s", login);
                        return;
                    }
                }
            }
            free_group(&group);
        }

        free_user(&user);
    }

    syslog(LOG_INFO, "skill issue");
    prepare_and_send_json(json, client);

    syslog(LOG_INFO, "Create group request received. Decoded login: %s", login);
}


