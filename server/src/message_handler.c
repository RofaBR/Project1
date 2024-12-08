#include "../inc/server.h"

t_packet *receive_packet(int socket_fd) {
    size_t len;

    ssize_t rec_data = recv(socket_fd, &len, sizeof(len), 0);
    if (rec_data <= 0){
        return NULL;
    }

    t_packet *data = create_packet(NULL, len);
    if (!data) {
        return NULL;
    }

    rec_data = recv(socket_fd, data->data, data->len, 0);
    if (rec_data <= 0) {
        free_packet(data);
        return NULL;
    }

    return data;
}

t_packet *create_packet(const char *data, size_t data_len) {
    t_packet *receive = (t_packet *)malloc(sizeof(t_packet));
    if (!receive) {
        return NULL;
    }
    
    receive->len = data_len;
    receive->data = (char *)malloc(receive->len * sizeof(char));
    if (!receive->data) {
        free(receive);
        return NULL;
    }
    
    if (data) {
        memcpy(receive->data, data, data_len); 
    }

    return receive;
}

void free_packet(t_packet *receive) {
    if (receive) {
        if (receive->data) {
            free(receive->data);
            receive->data = NULL;
        }
        free(receive);
    }
}

void send_message(t_packet *req, int socket) {
    if (!req) {
        return;
    }

    if (send(socket, &req->len, sizeof(req->len), 0) == -1 || send(socket, req->data, req->len, 0) == -1)
        return;
}

void prepare_and_send_json(cJSON *json_payload, t_client *client) {
    if (!client || !json_payload) {
        syslog(LOG_ERR, "Invalid arguments to prepare_and_send_json");
        return;
    }

    size_t encrypted_data_len;
    unsigned char *encrypted_data = encrypt_json_with_aes(client->keys.aes_key, client->keys.aes_iv, json_payload, &encrypted_data_len);
    if (!encrypted_data) {
        syslog(LOG_ERR, "Failed to encrypt JSON object");
        cJSON_Delete(json_payload);
        return;
    }
    
    if (client->socket_fd > 0) {
        t_packet *message = create_packet((char *)encrypted_data, encrypted_data_len);
        if (message) {
            send_message(message, client->socket_fd);
            free_packet(message);
        }
    }

    cJSON_Delete(json_payload);
    free(encrypted_data);
}

void send_to_client_by_id(cJSON *json_payload, int receiver_user_id) {
    if (!json_payload) {
        syslog(LOG_ERR, "Invalid JSON payload in send_to_client_by_id");
        return;
    }

    cJSON *json_copy = cJSON_Duplicate(json_payload, 1);

    pthread_mutex_lock(&client_list_mutex);

    t_client_node *current = client_list;
    t_client *receiver_client = NULL;
    while (current) {
        t_client *client = current->client;
        if (client->id_db == receiver_user_id) {
            receiver_client = client;
            break;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&client_list_mutex);

    if (!receiver_client) {
        syslog(LOG_ERR, "Receiver user ID %d not found in client list", receiver_user_id);
        cJSON_Delete(json_copy);
        return;
    }
    
    prepare_and_send_json(json_copy, receiver_client);
}

