#include "../inc/client.h"

cJSON *form_login_request(const char *login, const char *password) {
    cJSON *json_payload = cJSON_CreateObject();

    if (!json_payload) {    
        printf("Error creating JSON object\n");
        return NULL;
    }
    
    cJSON_AddStringToObject(json_payload, "request_type", "login");

    unsigned char hash_password[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)password, strlen(password), hash_password);

    unsigned char hash_login[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)login, strlen(login), hash_login);
    
    char *hash_password_b64 = base64_encode(hash_password, SHA256_DIGEST_LENGTH);
    char *hash_login_b64 = base64_encode(hash_login, SHA256_DIGEST_LENGTH);
    
    
    cJSON_AddStringToObject(json_payload, "userlogin", hash_login_b64);
    cJSON_AddStringToObject(json_payload, "password", hash_password_b64);

    free(hash_password_b64);
    free(hash_login_b64);

    return json_payload;
}

cJSON *form_register_request(const char *login, const char *username, const char *password) {
    cJSON *json_payload = cJSON_CreateObject();

    if (!json_payload) {    
        printf("Error creating JSON object\n");
        return NULL;
    }

    cJSON_AddStringToObject(json_payload, "request_type", "registration");

    unsigned char hash_login[SHA256_DIGEST_LENGTH];
    unsigned char hash_password[SHA256_DIGEST_LENGTH];

    SHA256((const unsigned char *)login, strlen(login), hash_login);
    SHA256((const unsigned char *)password, strlen(password), hash_password);

    char *hash_login_b64 = base64_encode(hash_login, SHA256_DIGEST_LENGTH);
    char *username_b64 = base64_encode((const unsigned char *)username, strlen(username));
    char *hash_password_b64 = base64_encode(hash_password, SHA256_DIGEST_LENGTH);
    
    cJSON_AddStringToObject(json_payload, "userlogin", hash_login_b64);
    cJSON_AddStringToObject(json_payload, "username", username_b64);
    cJSON_AddStringToObject(json_payload, "password", hash_password_b64);

    free(hash_login_b64);
    free(username_b64);
    free(hash_password_b64);
    
    return json_payload;
}

cJSON *form_create_chat_with(const char *userlogin) {
    cJSON *json_payload = cJSON_CreateObject();

    if (!json_payload) {    
        printf("Error creating JSON object\n");
        return NULL;
    }

    cJSON_AddStringToObject(json_payload, "request_type", "privateChatCreate");

    unsigned char hash_login[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)userlogin, strlen(userlogin), hash_login);
    char *hash_login_b64 = base64_encode(hash_login, SHA256_DIGEST_LENGTH);
    
    cJSON_AddStringToObject(json_payload, "userlogin", hash_login_b64);

    free(hash_login_b64);

    return json_payload;
}

cJSON *form_aes_key_transfer(const unsigned char *aes_key, const unsigned char *iv, EVP_PKEY *pubkey) {
    cJSON *json_payload = cJSON_CreateObject();
    if (!json_payload) {    
        printf("Error creating JSON object\n");
        return NULL;
    }

    unsigned char encrypted_aes_key[256]; 
    int encrypted_key_len = encrypt_aes_key(pubkey, aes_key, encrypted_aes_key);
    if (encrypted_key_len <= 0) {
        printf("Error encrypting AES key\n");
        cJSON_Delete(json_payload);
        return NULL;
    }

    char *encoded_aes_key = base64_encode(encrypted_aes_key, encrypted_key_len);
    if (!encoded_aes_key) {
        printf("Error encoding AES key to Base64\n");
        cJSON_Delete(json_payload);
        return NULL;
    }

    char *encoded_iv = base64_encode(iv, AES_IV_SIZE);
    if (!encoded_iv) {
        printf("Error encoding IV to Base64\n");
        free(encoded_aes_key);
        cJSON_Delete(json_payload);
        return NULL;
    }

    cJSON_AddStringToObject(json_payload, "aes_key", encoded_aes_key);
    cJSON_AddStringToObject(json_payload, "iv", encoded_iv);

    free(encoded_aes_key);
    free(encoded_iv);

    return json_payload;
}

