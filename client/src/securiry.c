#include "../inc/client.h"

int generate_aes_key_iv(t_main *main) {
    if (!main) {
        fprintf(stderr, "Invalid main structure\n");
        return -1;
    }

    if (!RAND_bytes(main->keys.aes_key, AES_KEY_SIZE)) {
        fprintf(stderr, "Failed to generate AES key\n");
        return -1;
    }

    if (!RAND_bytes(main->keys.aes_iv, AES_IV_SIZE)) {
        fprintf(stderr, "Failed to generate AES IV\n");
        return -1;
    }

    //printf("AES key and IV generated successfully.\n");
    return 0;
}


int handshake(t_main *main) {
    if (!main || main->socket < 0) {
        fprintf(stderr, "Invalid main structure or socket\n");
        return -1;
    }

    if (mx_receiving_pubkey(main) != 0) {
        fprintf(stderr, "Failed to receive server public key\n");
        return -1;
    }

    if (generate_aes_key_iv(main) != 0) {
        fprintf(stderr, "Failed to generate AES keys\n");
        return -1;
    }
    if (mx_transfer_aes_key(main) != 0) {
        fprintf(stderr, "Failed to transfer AES keys to the server\n");
        return -1;
    }
    return 0;
}


int encrypt_aes_key(EVP_PKEY *pubkey, const unsigned char *aes_key, unsigned char *encrypted_key) {
    if (!pubkey || !aes_key || !encrypted_key) {
        fprintf(stderr, "Invalid argument(s) to encrypt_aes_key\n");
        return -1;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pubkey, NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create EVP_PKEY_CTX\n");
        return -1;
    }

    size_t encrypted_key_len;
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        fprintf(stderr, "EVP_PKEY_encrypt_init failed\n");
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_encrypt(ctx, NULL, &encrypted_key_len, aes_key, AES_KEY_SIZE) <= 0) {
        fprintf(stderr, "Failed to determine encrypted key length\n");
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_encrypt(ctx, encrypted_key, &encrypted_key_len, aes_key, AES_KEY_SIZE) <= 0) {
        fprintf(stderr, "Failed to encrypt AES key\n");
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    EVP_PKEY_CTX_free(ctx);
    return (int)encrypted_key_len;
}

int mx_receiving_pubkey(t_main *main) {
    if (!main || main->socket < 0) {
        fprintf(stderr, "Invalid arguments or socket\n");
        return -1;
    }

    char buffer[2048];
    ssize_t received = recv(main->socket, buffer, sizeof(buffer), 0);

    if (received <= 0) {
        perror("Failed to receive public key");
        return -1;
    }

    BIO *bio = BIO_new_mem_buf(buffer, received);
    if (!bio) {
        perror("Failed to create BIO");
        return -1;
    }

    main->keys.pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (main->keys.pkey == NULL) {
        ERR_print_errors_fp(stderr);
        BIO_free(bio);
        return -1;
    }

    BIO_free(bio);
    //printf("Public key received and parsed successfully.\n");
    return 0;
}

int mx_transfer_aes_key(t_main *main) {
    if (!main || !main->keys.pkey || main->keys.aes_key[0] == '\0' || main->keys.aes_iv[0] == '\0') {
        fprintf(stderr, "Invalid input: missing AES keys or public key\n");
        return -1;
    }

    cJSON *json_transfer_key = form_aes_key_transfer(main->keys.aes_key, main->keys.aes_iv, main->keys.pkey);
    if (!json_transfer_key) {
        fprintf(stderr, "Failed to create key transfer request JSON\n");
        return -1;  
    }

    char *json_str = cJSON_PrintUnformatted(json_transfer_key);
    if (!json_str) {
        fprintf(stderr, "Failed to serialize JSON\n");
        cJSON_Delete(json_transfer_key);
        return -1;
    }

    ssize_t sent_bytes = send(main->socket, json_str, strlen(json_str), 0);
    if (sent_bytes == -1) {
        perror("Failed to send key transfer request to server");
        free(json_str);
        cJSON_Delete(json_transfer_key);
        return -1;
    }

    if ((size_t)sent_bytes != strlen(json_str)) {
        fprintf(stderr, "Incomplete data sent to server\n");
        free(json_str);
        cJSON_Delete(json_transfer_key);
        return -1;
    }

    printf("AES key and IV successfully sent to the server.\n");

    free(json_str);
    cJSON_Delete(json_transfer_key);
    return 0;
}

unsigned char *encrypt_json_with_aes(const unsigned char *aes_key, const unsigned char *iv, 
                                     cJSON *json, size_t *out_len) {
    if (!aes_key || !iv || !json || !out_len) {
        fprintf(stderr, "Invalid argument(s) to encrypt_json_with_aes\n");
        return NULL;
    }

    char *json_string = cJSON_PrintUnformatted(json);
    if (!json_string) {
        fprintf(stderr, "Failed to convert JSON to string\n");
        return NULL;
    }

    printf("JSON string to encrypt: %s\n", json_string);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create EVP_CIPHER_CTX\n");
        free(json_string);
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv) != 1) {
        fprintf(stderr, "EVP_EncryptInit_ex failed\n");
        EVP_CIPHER_CTX_free(ctx);
        free(json_string);
        return NULL;
    }

    size_t json_len = strlen(json_string);
    size_t block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());
    size_t encrypted_data_len = json_len + block_size;

    unsigned char *encrypted_data = (unsigned char *)malloc(encrypted_data_len);
    if (!encrypted_data) {
        fprintf(stderr, "Failed to allocate memory for encrypted data\n");
        EVP_CIPHER_CTX_free(ctx);
        free(json_string);
        return NULL;
    }

    int bytes_written = 0, final_bytes_written = 0;

    if (EVP_EncryptUpdate(ctx, encrypted_data, &bytes_written, 
                          (unsigned char *)json_string, json_len) != 1) {
        fprintf(stderr, "EVP_EncryptUpdate failed\n");
        EVP_CIPHER_CTX_free(ctx);
        free(json_string);
        free(encrypted_data);
        return NULL;
    }

    if (EVP_EncryptFinal_ex(ctx, encrypted_data + bytes_written, &final_bytes_written) != 1) {
        fprintf(stderr, "EVP_EncryptFinal_ex failed\n");
        EVP_CIPHER_CTX_free(ctx);
        free(json_string);
        free(encrypted_data);
        return NULL;
    }

    *out_len = bytes_written + final_bytes_written; 
    EVP_CIPHER_CTX_free(ctx);
    free(json_string);

    return encrypted_data;
}

int decrypt_received_data(t_packet *data, const unsigned char *aes_key, const unsigned char *iv) {
    if (!data || !aes_key || !iv) {
        fprintf(stderr, "Invalid argument(s) to decrypt_received_data\n");
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create EVP_CIPHER_CTX\n");
        return -1;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv) != 1) {
        fprintf(stderr, "EVP_DecryptInit_ex failed\n");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    size_t block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());
    size_t decrypted_data_len = data->len + block_size;
    unsigned char *decrypted_data = (unsigned char *)malloc(decrypted_data_len);
    if (!decrypted_data) {
        fprintf(stderr, "Failed to allocate memory for decrypted data\n");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    int bytes_written = 0, final_bytes_written = 0;
    if (EVP_DecryptUpdate(ctx, decrypted_data, &bytes_written, (unsigned char *)data->data, data->len) != 1) {
        fprintf(stderr, "EVP_DecryptUpdate failed\n");
        EVP_CIPHER_CTX_free(ctx);
        free(decrypted_data);
        return -1;
    }

    if (EVP_DecryptFinal_ex(ctx, decrypted_data + bytes_written, &final_bytes_written) != 1) {
        fprintf(stderr, "EVP_DecryptFinal_ex failed\n");
        EVP_CIPHER_CTX_free(ctx);
        free(decrypted_data);
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);

    size_t total_decrypted_len = bytes_written + final_bytes_written;

    free(data->data);
    data->data = (char *)malloc(total_decrypted_len + 1); 
    if (!data->data) {
        fprintf(stderr, "Failed to allocate memory for decrypted data in structure\n");
        free(decrypted_data);
        return -1;
    }

    memcpy(data->data, decrypted_data, total_decrypted_len);
    data->data[total_decrypted_len] = '\0';
    data->len = total_decrypted_len;

    free(decrypted_data);
    fprintf(stderr, "Data successfully decrypted and updated in the structure\n");

    return 0;
}


