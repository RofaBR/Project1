#include "../inc/bee_user.h"

t_bee_user *create_bee_user(const char *login, const char *username, const char *created_at, int logo_id) {
    t_bee_user *user = malloc(sizeof(t_bee_user));
    if (!user) {
        perror("Memory allocation failed for t_bee_user");
        return NULL;
    }

    user->login = strdup(login);
    user->username = strdup(username);
    user->created_at = strdup(created_at);
    user->logo_id = logo_id;

    if ((login && !user->login) || (username && !user->username) || (created_at && !user->created_at)) {
        fprintf(stderr, "Failed to allocate memory for user strings\n");
        free_bee_user(user);
        return NULL;
    }

    return user;
}

void free_bee_user(t_bee_user *user) {
    if (!user) return;

    free(user->login);
    free(user->username);
    free(user->created_at);
    free(user);
}

t_bee_group *create_bee_group(int id, const char *name, int is_private, int created_by, const char *creator_username, const char *created_at, const char *last_message_date) {
    t_bee_group *group = malloc(sizeof(t_bee_group));
    if (!group) {
        perror("Memory allocation failed for t_bee_group");
        return NULL;
    }

    group->id = id;
    group->name = name ? strdup(name) : NULL;
    group->is_private = is_private;
    group->created_by = created_by;
    group->creator_username = creator_username ? strdup(creator_username) : NULL;
    group->created_at = created_at ? strdup(created_at) : NULL;
    group->last_message_date = last_message_date ? strdup(last_message_date) : NULL;

    if ((name && !group->name) || (creator_username && !group->creator_username) ||
        (created_at && !group->created_at) || (last_message_date && !group->last_message_date)) {
        fprintf(stderr, "Failed to allocate memory for group strings\n");
        free_bee_group(group);
        return NULL;
    }

    return group;
}

void free_bee_group(t_bee_group *group) {
    if (!group) return;

    free(group->name);
    free(group->creator_username);
    free(group->created_at);
    free(group->last_message_date);
    free(group);
}
