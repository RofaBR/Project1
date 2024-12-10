#ifndef CLIENT_H
#define CLIENT_H

#include "../../libs/libmx/inc/libmx.h"
#include "../../libs/JsonLib/cJSON.h"
#include "../inc/bee_user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#define AES_KEY_SIZE 32
#define AES_IV_SIZE 16 

typedef struct s_bee_group t_bee_group;

typedef struct s_keys {
	EVP_PKEY *pkey;
	unsigned char aes_key[AES_KEY_SIZE];
	unsigned char aes_iv[AES_IV_SIZE];
}				t_keys;

typedef struct s_packet {
	size_t len;
	bool status;
	char *data;
}			   t_packet;

typedef struct s_main {
	int socket;
	char *address;
	int rec_delay;
	int port;
	bool is_connected;
	bool is_closing;
	t_keys keys;

	GtkApplication *app;
	GtkTextBuffer *buff;

	GList *groups;
}				t_main;

typedef struct s_user {
    char* login;
    char* name;
    char* password;
} t_user;


extern t_main *main_data;
extern const char* uses_chat;
extern GtkWidget *chat_entry;
extern GtkWidget *message_area;


//func to communicate with the server
int mx_connect_to_server(t_main *main);
int mx_receiving_pubkey(t_main *main);
int mx_transfer_aes_key(t_main *main);
int handshake(t_main *main);
void process_response(t_packet *recieved_data);

//security func
int generate_aes_key_iv(t_main *main);

unsigned char *encrypt_json_with_aes(const unsigned char *aes_key, const unsigned char *iv, 
                                     cJSON *json, size_t *out_len);
int encrypt_aes_key(EVP_PKEY *pubkey, const unsigned char *aes_key, unsigned char *encrypted_key);

int decrypt_received_data(t_packet *data, const unsigned char *aes_key, const unsigned char *iv);

//UI func
void login_window(GtkApplication *app, gpointer user_data);

//main data func
t_main *mx_create_main_data(GtkApplication *app, const char *address, int port);
void mx_free_main_data(t_main *main);
void mx_add_group(t_main *main, t_bee_group *group);
void mx_free_group(t_bee_group *group);

//func json request
cJSON *form_login_request(const char *login, const char *password);
cJSON *form_register_request(const char *login, const char *username, const char *password);
cJSON *form_aes_key_transfer(const unsigned char *aes_key, const unsigned char *iv, EVP_PKEY *pubkey);
cJSON *form_create_chat_with(const char *userlogin);

//Base64
char *base64_encode(const unsigned char *input, size_t input_len);
unsigned char *base64_decode(const char *input, size_t *output_len);

//message utils func (for requests and receipts)
t_packet *create_message(const char *data, size_t data_len);
void free_message(t_packet *req);
void prepare_and_send_json(cJSON *json_payload, t_main *main);
void send_message(t_packet *reg, int socket);
t_packet *receive_message(int socket_fd);

//UI LOGIN REGISTER PAGE FUNC

void load_css(GtkWidget *window);
void load_css_for_screen(const char *css_file_path);
GtkWidget* create_image(const char *image_path, int width, int height);
t_user* mx_create_user(const char* login, const char* name, const char* password);
void mx_free_user(t_user* new_user);
t_user* mx_create_log_user(const char* login, const char* password);

//buttons
void on_create_account_button_clicked(GtkWidget *button, gpointer data);
void on_login_button_clicked(GtkWidget *button, gpointer data);
void on_submit_account_button_clicked(GtkWidget *button, gpointer data);
void restore_login_form(GtkWidget *button, gpointer data);

//response handler funcs
void handle_login_response(cJSON *json_payload);
void handle_register_response(cJSON *json_payload);
void handle_create_chat_response(cJSON *json_payload);

void update_error_label(GtkWidget *error_label, const char *message);
gboolean gtk_update_notification_label(gpointer user_data);
gboolean gtk_destroy_login_window(gpointer data);
gboolean check_connection(gpointer user_data);

gboolean gtk_create_main_window(gpointer user_data);
void on_button_create_chat_clicked(GtkButton *button, gpointer user_data);
void on_chat_button_clicked(GtkButton *button, gpointer user_data);
void on_private_chat_clicked(GtkButton *button, gpointer user_data);
void on_exit_button_clicked(GtkButton *button, gpointer user_data);
void render_chat(t_bee_group *group, GtkWidget *sidebar_box);
void render_all_chats(t_main *main_data, GtkWidget *sidebar_box);

//messange func
void on_emoji_button_clicked(GtkButton *button, gpointer user_data);
void on_file_button_clicked(GtkButton *button, gpointer user_data);
void add_message_to_chat(const char *message, const char *file_path);
void clear_message_area(void);
#endif
