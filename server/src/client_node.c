#include "../inc/server.h"

void add_client_to_list(t_client *client) {
    pthread_mutex_lock(&client_list_mutex);

    t_client_node *new_node = malloc(sizeof(t_client_node));
    if (!new_node) {
        perror("Failed to allocate memory for new client node");
        pthread_mutex_unlock(&client_list_mutex);
        return;
    }
    new_node->client = client;
    new_node->next = client_list;
    client_list = new_node;

    pthread_mutex_unlock(&client_list_mutex);
}

void remove_client_from_list(t_client *client) {
    pthread_mutex_lock(&client_list_mutex);

    t_client_node **current = &client_list;
    while (*current) {
        if ((*current)->client == client) {
            t_client_node *to_remove = *current;
            *current = to_remove->next;
            free(to_remove);
            break;
        }
        current = &(*current)->next;
    }

    pthread_mutex_unlock(&client_list_mutex);
}
