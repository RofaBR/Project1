#include "../inc/bee_user.h"

t_bee_user *create_bee_user_from_json(cJSON *data) {
    if (!data || !cJSON_IsObject(data)) {
        fprintf(stderr, "Invalid JSON object for user data\n");
        return NULL;
    }

    t_bee_user *user = malloc(sizeof(t_bee_user));
    if (!user) {
        fprintf(stderr, "Memory allocation failed for t_bee_user\n");
        return NULL;
    }

    user->logo_id = cJSON_GetObjectItemCaseSensitive(data, "logo_id")->valueint;

    cJSON *username = cJSON_GetObjectItemCaseSensitive(data, "username");
    cJSON *login = cJSON_GetObjectItemCaseSensitive(data, "login");
    cJSON *created_at = cJSON_GetObjectItemCaseSensitive(data, "created_at");

    if (cJSON_IsString(username) && cJSON_IsString(login) && cJSON_IsString(created_at)) {
        user->username = strdup(username->valuestring);
        user->login = strdup(login->valuestring);
        user->created_at = strdup(created_at->valuestring);
    } else {
        fprintf(stderr, "Invalid or missing user fields in JSON\n");
        free(user);
        return NULL;
    }

    return user;
}

void free_bee_user(t_bee_user *user) {
    if (!user) return;

    free(user->username);
    free(user->login);
    free(user->created_at);
    free(user);
}

t_bee_group *create_bee_group_from_json(cJSON *data) {
    if (!data || !cJSON_IsObject(data)) {
        fprintf(stderr, "Invalid JSON object for user data\n");
        return NULL;
    }
}

