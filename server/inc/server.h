#ifndef SERVER_H
#define SERVER_H

#define DB_NAME "uchat.db"
#define AES_KEY_SIZE 32
#define AES_IV_SIZE 16 

#include "../../libs/libmx/inc/libmx.h"
#include "../../libs/JsonLib/cJSON.h"
#include "../../libs/Sqlite3Lib/sqlite3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <libgen.h>

extern char exe_path[PATH_MAX];

typedef struct s_keys t_keys;
typedef struct s_packet t_packet;
typedef struct s_client t_client;
typedef struct s_server t_server;
typedef struct s_client_node t_client_node;

typedef struct s_keys {
	EVP_PKEY *pkey;
	unsigned char aes_key[AES_KEY_SIZE];
	unsigned char aes_iv[AES_IV_SIZE];
}				t_keys;

typedef struct s_packet{
	size_t len;
	char *data;
}				t_packet;

typedef struct s_client {
	pthread_t thread_id;
	int socket_fd;
	t_keys keys;
	int id_db;
}              t_client;

typedef struct s_server {
	int sd;
	bool is_running;
	struct addrinfo *ai;
}			  t_server;

typedef struct s_client_node {
    t_client *client;
    struct s_client_node *next;
} 				t_client_node;

extern t_client_node *client_list;
extern pthread_mutex_t client_list_mutex;

//models
typedef struct s_user {
	int id;
	int logo_id;
	char* username;
	char* login;
	char* password;
	char* created_at;
} t_user;

typedef struct s_message {
	int id;
	int sent_by;
	char* sender_username;
	char* text;
	int group_id;
	char* created_at;
} t_message;

typedef struct s_group {
	int id;
	char* name;
	int is_private;
	int created_by;
	char* creator_username;
	char* created_at;
	char* last_message_date;
} t_group;

//Migrations
void users_migration_up(void);
void users_migration_down(void);
void groups_migration_up(void);
void groups_migration_down(void);
void messages_migration_up(void);
void messages_migration_down(void);
void users_groups_migration_up(void);
void users_groups_migration_down(void);
void migration_up(void);
void migration_down(void);

//CRUD Operations
int database_create(const char* insert, const char* into, const char* values);
t_list* database_read(const char* select, const char* from, const char* where);
bool database_update(const char* update, const char* set, const char* where);
bool database_delete(const char* from, const char* where);

//Users CRUD
int db_user_create(t_user* user);
bool db_user_delete(t_user* user);
bool db_user_delete_by_id(int id);
bool db_user_delete_by_named_field(const char* field, const char* value);
bool db_user_update(t_user* user);
t_user* db_user_read_by_id(int id);
t_user* db_user_read_by_login(const char *login);
t_list* db_user_read_all(void);

//Messages CRUD
int db_message_create(t_message* message);
bool db_message_delete(t_message* message);
bool db_message_delete_by_id(int id);
bool db_message_delete_by_named_field(const char* field, const char* value);
bool db_message_update(t_message* message);
t_message* db_message_read_by_id(int id);
t_list* db_message_read_all(void);
t_list* db_message_read_by_sender_id(int id);
t_list* db_message_read_by_group_id(int id);

//Groups CRUD
int db_group_create(t_group* group);
bool db_group_delete(t_group* group);
bool db_group_delete_by_id(int id);
bool db_group_delete_by_named_field(const char* field, const char* value);
bool db_group_update(t_group* group);
t_group* db_group_read_by_id(int id);
t_list* db_group_read_all(void);
bool db_private_group_exists(int idA, int idB);

//Users-Groups CRUD
t_list* db_user_read_by_group_id(int id);
t_list* db_group_read_by_user_id(int id);
int db_user_add_to_group(int user_id, int group_id);
bool db_user_remove_from_froup(int user_id, int group_id);

//Error handling
bool validate_database_operation(int rc, sqlite3* db, char* error);

//Model functions
t_user* user_create(const char* username, const char* login, const char* password, int logo_id);
t_user* user_from_data_list(t_list* list);
t_list* user_list_from_data_list(t_list* list);
void free_user(t_user** user);
void free_user_list(t_list* list);
cJSON *user_to_json(t_user* user);
cJSON* users_list_to_json_array(t_list* list);

t_message* message_create(int sent_by, const char* text, int group_id);
t_message* message_from_data_list(t_list* list);
t_list* message_list_from_data_list(t_list* list);
void free_message(t_message** message);
void free_message_list(t_list* list);
cJSON *message_to_json(t_message* msg);
cJSON *messages_list_to_json_array(t_list* list);

t_group* group_from_data_list(t_list* list);
t_group* group_create(const char* name, int created_by, int is_private);
t_list* group_list_from_data_list(t_list* list);
void free_group(t_group** group);
void free_group_list(t_list* list);
cJSON *group_to_json(t_group* group);
cJSON *groups_list_to_json_array(t_list* list) ;

//Server functions

//daemon func
void mx_daemon_start(void);
void mx_daemon_end(int signal, siginfo_t *info, void *context);
void set_signal(t_server *server);
int start_server(t_server *server, const char *port);

//Func to communicate with the client
void *handle_client(void *arg);
void handle_login_request(cJSON* json_payload, t_client *client);
void handle_register_request(cJSON *json_payload, t_client *client);
void handle_group_create_request(cJSON* json_payload, t_client* client);
t_client *create_new_client(int socket_fd);
void free_client(t_client *client);
void process_request(t_packet *receive_data, t_client *client);
void add_client_to_list(t_client *client);
void remove_client_from_list(t_client *client);

//security func
unsigned char *encrypt_json_with_aes(const unsigned char *aes_key, const unsigned char *iv, 
                                     cJSON *json, size_t *out_len);
int rsa_decrypt_aes_key(EVP_PKEY *private_key, const unsigned char *encrypted_aes_key, size_t encrypted_len, 
                        unsigned char **decrypted_key, size_t *decrypted_key_len);
int generate_rsa_keys(t_keys *keys);
int rsa_keys_to_pem(EVP_PKEY *pkey, unsigned char **pubkey_pem, size_t *pubkey_len,
                    unsigned char **privkey_pem, size_t *privkey_len);
int mx_receive_aes(t_client *client, unsigned char *encrypted_aes_key, size_t *encrypted_key_len, unsigned char *iv);
int handshake(t_client *client);
int decrypt_received_data(t_packet *data, const unsigned char *aes_key, const unsigned char *iv);

//base64
unsigned char *base64_decode(const char *input, size_t *output_len);
char *base64_encode(const unsigned char *input, size_t input_len);

//response utils func
t_packet *receive_packet(int socket_fd);
t_packet *create_packet(const char *data, size_t data_len);
void free_packet(t_packet *receive);
void send_message(t_packet *req, int socket);
void prepare_and_send_json(cJSON *json_payload, t_client *client);
void send_to_client_by_id(cJSON *json_payload, int receiver_user_id);

t_server *create_server(void);
void free_server(t_server *server);
void free_client_keys(t_client *client);

#endif 
