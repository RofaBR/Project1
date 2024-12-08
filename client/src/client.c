#include "../inc/client.h"

t_main *main_data;

static void *connection(void *arg) {
    t_main *main = (t_main *)arg;

    while (!main->is_closing) {
        main->socket = mx_connect_to_server(main);
        if (main->socket == -1) {
            main->is_connected = false;
            printf("Failed to connect to server. Retrying in %d seconds...\n", main->rec_delay);
            sleep(main->rec_delay);
            continue;
        }

        printf("Connected to server.\n");
        main->is_connected = true;

        if (handshake(main) != 0) {
            fprintf(stderr, "Handshake failed. Retrying connection...\n");
            close(main->socket);
            main->is_connected = false;
            continue;
        }

        printf("Handshake succeeded. AES session established.\n");

        t_packet *received_data = NULL;
        while (!main->is_closing && (received_data = receive_message(main->socket)) != NULL) {
            if (decrypt_received_data(received_data, main->keys.aes_key, main->keys.aes_iv) == -1) {
                fprintf(stderr, "Failed to decrypt data\n");
                free_message(received_data);
                break;
            }

            process_response(received_data);

            free_message(received_data);
        }

        close(main->socket);
        main->socket = -1;
    }

    printf("Closing connection thread.\n");
    pthread_exit(NULL);
}

static int handle_command_line(GApplication *app, GApplicationCommandLine *command_line, gpointer user_data) {
    (void)user_data;

    int argc;
    char **argv = g_application_command_line_get_arguments(command_line, &argc);
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [IP address] [Port]\n", argv[0]);
        g_strfreev(argv);
        return 1;
    }

    const char *ip = argv[1];
    const char *port_str = argv[2];
    int port = atoi(port_str);

    main_data = mx_create_main_data(GTK_APPLICATION(app), ip, port);
    if (!main_data) {
        fprintf(stderr, "Failed to initialize main data\n");
        g_strfreev(argv);
        return 1;
    }

    g_strfreev(argv);
    g_application_activate(app);
    return 0;
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    (void)app;

    if (main_data) {
        pthread_t thread_id;
        if ((pthread_create(&thread_id, NULL, connection, main_data)) != 0) {
            fprintf(stderr, "Failed to create connection thread\n");
            mx_free_main_data(main_data);
            return;
        }
        load_css_for_screen("styles.css");
        login_window(main_data->app, main_data);
    }
}

int main(int argc, char **argv) {
    GtkApplication *beeChat;
    int status;
    char app_id[64];

    snprintf(app_id, sizeof(app_id), "bee.chat%d", getpid());
    printf("Starting application with ID: %s\n", app_id);

    beeChat = gtk_application_new(app_id, G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(beeChat, "activate", G_CALLBACK(activate), NULL);
    g_signal_connect(beeChat, "command-line", G_CALLBACK(handle_command_line), NULL);

    status = g_application_run(G_APPLICATION(beeChat), argc, argv);

    if (main_data) {
        main_data->is_closing = true;
    }
    mx_free_main_data(main_data);
    g_object_unref(beeChat);
    return status;
}
