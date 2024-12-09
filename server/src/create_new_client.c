#include "../inc/server.h"

void free_client_keys(t_client *client) {
    if (!client) {
        syslog(LOG_ERR, "Error: client is NULL");
        return;
    }

    if (client->keys.pkey) {
        EVP_PKEY_free(client->keys.pkey);
        client->keys.pkey = NULL;
    }
}

t_client *create_new_client(int socket_fd) {
    t_client *client = malloc(sizeof(t_client));
    if (!client) {
        syslog(LOG_ERR, "Error: Failed to allocate memory for client");
        return NULL;
    }

    client->socket_fd = socket_fd;
    client->thread_id = 0;
    client->id_db = 0;

    client->keys.pkey = NULL;

    return client;
}

void free_client(t_client *client) {
    if (!client) {
        return;
    }
    syslog(LOG_INFO, "Freeing client resources: %p", (void *)client);
    if (client->socket_fd >= 0) {
        close(client->socket_fd);
    }

    free_client_keys(client);

    free(client);
}
