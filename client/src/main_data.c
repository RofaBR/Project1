#include "../inc/client.h"

t_main *mx_create_main_data(GtkApplication *app, const char *address, int port) {
    t_main *main = malloc(sizeof(t_main));
    if (!main) {
        perror("Failed to allocate memory for t_main");
        return NULL;
    }

    main->socket = -1;
    main->address = strdup(address);
    if (!main->address) {
        perror("Failed to allocate memory for address");
        free(main);
        return NULL;
    }

    main->port = port;
    main->rec_delay = 5;
    main->keys.pkey = NULL;
    main->app = app;
    main->buff = gtk_text_buffer_new(NULL);
    
    // if (pthread_mutex_init(&main->lock, NULL) != 0) {
    //     perror("Mutex init failed");
    //     free(main->address);
    //     free(main);
    //     return NULL;
    // }
    return main;
}

void mx_free_main_data(t_main *main) {
    if (!main) return;

    if (main->address) free(main->address);
    //if (main->server_response) cJSON_Delete(main->server_response);

    if (main->keys.pkey) {
        EVP_PKEY_free(main->keys.pkey);
        main->keys.pkey = NULL;
    }

    //pthread_mutex_destroy(&main->lock);
    free(main);
}
