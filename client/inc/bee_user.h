#ifndef BEE_USER_H
#define BEE_USER_H

#include "../inc/client.h"

typedef struct s_bee_user {
    int id;
    int logo_id;
    char *username;
    char *login;
    char  *created_at;
}              t_bee_user;

/*
typedef struct s_bee_message {
	int id;
	int sent_by;
	char* sender_username;
	char* text;
	int group_id;
	char* created_at;
}               t_bee_message;

typedef struct s_bee_group {
	int id;
	char* name;
	int is_private;
	int created_by;
	char* creator_username;
	char* created_at;
	char* last_message_date;
    GtkWidget *chat_box;
}               t_bee_group;
*/

t_bee_user *create_bee_user_from_json(cJSON *data);
void free_bee_user(t_bee_user *user);

#endif
