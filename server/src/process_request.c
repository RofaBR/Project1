#include "../inc/server.h"

static void debug_client_list(void) {
    pthread_mutex_lock(&client_list_mutex);
    syslog(LOG_INFO, "===== Client List State =====");
    t_client_node *current = client_list;

    while (current) {
        t_client *client = current->client;
        if (client) {
            syslog(LOG_INFO, "Client Thread ID: %lu, Socket FD: %d, DB ID: %d",
                   client->thread_id, client->socket_fd, client->id_db);

            syslog(LOG_INFO, "Client AES Key: %s, AES IV: %s",
                   client->keys.aes_key, client->keys.aes_iv);

            if (client->keys.pkey) {
                syslog(LOG_INFO, "Client has valid EVP_PKEY.");
            } else {
                syslog(LOG_WARNING, "Client EVP_PKEY is NULL.");
            }
        } else {
            syslog(LOG_WARNING, "Client node contains NULL client.");
        }

        current = current->next;
    }

    syslog(LOG_INFO, "===== End of Client List =====");
    pthread_mutex_unlock(&client_list_mutex);
}

void process_request(t_packet *receive_data, t_client *client) {
    if (!receive_data || !receive_data->data) {
        syslog(LOG_ERR, "Invalid t_receive structure in process_request");
        return;
    }

    cJSON *json_payload = cJSON_ParseWithLength(receive_data->data, receive_data->len);
    if (!json_payload) {
        syslog(LOG_ERR, "Failed to parse JSON from received data: %s", receive_data->data);
        return;
    }

    cJSON *request_type = cJSON_GetObjectItemCaseSensitive(json_payload, "request_type");
    if (!cJSON_IsString(request_type) || !request_type->valuestring) {
        syslog(LOG_ERR, "Missing or invalid 'request_type' in JSON data");
        cJSON_Delete(json_payload);
        return;
    }

    syslog(LOG_INFO, "Processing request of type: %s", request_type->valuestring);

    if (strcmp(request_type->valuestring, "login") == 0) {
        handle_login_request(json_payload, client);
    } else if (strcmp(request_type->valuestring, "registration") == 0) {
        handle_register_request(json_payload, client);
    } else if (strcmp(request_type->valuestring, "privateChatCreate") == 0) {
        //handle_group_create_request(json_payload, client);
    }

    cJSON_Delete(json_payload);
    debug_client_list();
}

