#include "../inc/client.h"

t_main *mx_create_main_data(GtkApplication *app, const char *address, int port) {
    if (!address) {
        fprintf(stderr, "Error: address is NULL\n");
        return NULL;
    }
    if (!app) {
        fprintf(stderr, "Error: app is NULL\n");
        return NULL;
    }

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
    main->is_connected = false;
    main->is_closing = false;
    main->keys.pkey = NULL;
    main->app = app;
    main->buff = gtk_text_buffer_new(NULL);
    main->groups = NULL;

    return main;
}

void mx_free_main_data(t_main *main) {
    if (!main) return;

    if (main->address) free(main->address);

    if (main->keys.pkey) {
        EVP_PKEY_free(main->keys.pkey);
        main->keys.pkey = NULL;
    }

    if (main->groups) {
        g_list_free_full(main->groups, (GDestroyNotify)mx_free_group);
        main->groups = NULL;
    }

    if (main->buff) {
        g_object_unref(main->buff);
    }

    free(main);
}

void mx_free_group(t_bee_group *group) {
    if (!group) return;

    if (group->name) free(group->name);

    free(group);
}

void mx_add_group(t_main *main, t_bee_group *group) {
    if (!main || !group) return;
    main->groups = g_list_append(main->groups, group);
}
