#include "../inc/server.h"

char exe_path[PATH_MAX];

/*
static void set_exe_path(void) {
    char full_path[PATH_MAX];

    ssize_t len = readlink("/proc/self/exe", full_path, sizeof(full_path) - 1);
    if (len == -1) {
        perror("Failed to get executable path");
        return;
    }

    full_path[len] = '\0';
    char *dir = dirname(full_path);

    snprintf(exe_path, PATH_MAX, "%s", dir);
}
*/

static void set_exe_path(void) {
    char full_path[PATH_MAX];
    uint32_t size = sizeof(full_path);
    if (_NSGetExecutablePath(full_path, &size) != 0) {        
	perror("Failed to get executable path");
        return;
    }
    char *dir = dirname(full_path);
    snprintf(exe_path, PATH_MAX, "%s", dir);
}

int start_server(t_server *server, const char *port) {

    struct addrinfo network_config;
    memset(&network_config, 0, sizeof(network_config));

    network_config.ai_family = AF_INET;
    network_config.ai_socktype = SOCK_STREAM;
    network_config.ai_flags = AI_PASSIVE;

    if(getaddrinfo(NULL, port, &network_config, &server->ai)) return -1;

    server->sd = socket(server->ai->ai_family, server->ai->ai_socktype, 0);
    if (server->sd == -1) {
        perror("Socket creation failed");
        free_server(server);
        return -1;
    }
    syslog(LOG_INFO, "Socket created successfully");

    if (bind(server->sd, server->ai->ai_addr, server->ai->ai_addrlen) == -1) {
        if (errno == EADDRINUSE) {
            syslog(LOG_ERR, "Port %s is already in use\n", port);
        } else {
            perror("Bind failed");
        }
        free_server(server);
        return -1;
    }
    syslog(LOG_INFO, "Bind successful on port %s", port);


    if (listen(server->sd, 10) == -1) {
        perror("Listen failed");
        free_server(server);
        return -1;
    }
    server->is_running = true;
    syslog(LOG_INFO, "Server listening on port %s", port);

    while (server->is_running) {
        struct sockaddr addr;
        socklen_t addrlen = sizeof(addr);

        int socket_fd = accept(server->sd, &addr, &addrlen);
        if (socket_fd == -1) {
            if (errno == EINTR) {
                syslog(LOG_WARNING, "Accept interrupted by signal");
                break;
            }
            syslog(LOG_ERR, "Accept failed: %s", strerror(errno));
            continue;
        }
        syslog(LOG_INFO, "New client connection accepted");
        t_client *client = create_new_client(socket_fd);

        char addrstr[INET_ADDRSTRLEN];
        struct sockaddr_in *client_addr = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &client_addr->sin_addr, addrstr, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Connected to client IP: %s", addrstr);

        if (pthread_create(&client->thread_id, NULL, handle_client, client) != 0) {
            perror("Thread creation failed");
            free_client(client);
            continue;
        }
    }

    syslog(LOG_INFO, "Shutting down server");
    free_server(server);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: ./uchat_server [port]\n");
        return -1;
    }

    set_exe_path();
    printf("Executable path: %s\n", exe_path);
    migration_up();

    const char *port = argv[1];

    t_server server;
    mx_daemon_start();
    set_signal(&server);

    if (start_server(&server, port) == -1) {
        fprintf(stderr, "failed to start server\n");
        return -1;
    }
}
