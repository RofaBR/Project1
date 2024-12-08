#include "../inc/server.h"

void *handle_client(void *arg) {
    t_client *client = (t_client *)arg;

    syslog(LOG_INFO, "Client connected with ID: %ld", client->thread_id);

    if (handshake(client) != 0) {
        syslog(LOG_ERR, "Key exchange failed for client ID: %ld", client->thread_id);
        close(client->socket_fd);
        free(client);
        pthread_exit(NULL);
    }

    t_packet *received_data = NULL;

    while ((received_data = receive_packet(client->socket_fd)) != NULL) {
        if (decrypt_received_data(received_data, client->keys.aes_key, client->keys.aes_iv) == -1) {
            syslog(LOG_ERR, "Failed to decrypt data from client ID: %ld", client->thread_id);
            free_packet(received_data);
            break;
        }

        process_request(received_data, client);
        free_packet(received_data);
    }

    syslog(LOG_INFO, "Client ID: %ld disconnected", client->thread_id);
    remove_client_from_list(client);
    free_client(client);

    pthread_exit(NULL);
}

