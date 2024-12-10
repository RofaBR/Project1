#include "../inc/server.h"

int generate_rsa_keys(t_keys *keys) {
    if (!keys) {
        syslog(LOG_ERR, "Error: keys structure is NULL");
        return -1;
    }

    EVP_PKEY_CTX *ctx = NULL;
    keys->pkey = NULL;
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        syslog(LOG_ERR, "Error: EVP_PKEY_CTX_new_id failed");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        syslog(LOG_ERR, "Error: EVP_PKEY_keygen_init failed");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, OPENSSL_RSA_FIPS_MIN_MODULUS_BITS) <= 0) {
        syslog(LOG_ERR, "Error: EVP_PKEY_CTX_set_rsa_keygen_bits failed");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_keygen(ctx, &keys->pkey) <= 0) {
        syslog(LOG_ERR, "Error: EVP_PKEY_keygen failed");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    EVP_PKEY_CTX_free(ctx);
    return 0;
}

int handshake(t_client *client) {
    if (!client) {
        syslog(LOG_ERR, "Error: client is NULL");
        return -1;
    }

    if (generate_rsa_keys(&client->keys) != 0) {
        syslog(LOG_ERR, "Error: Failed to generate RSA keys for handshake");
        return -1;
    }

    unsigned char *pubkey_pem = NULL;
    size_t pubkey_len = 0;
    unsigned char *privkey_pem = NULL;
    size_t privkey_len = 0;

    if (rsa_keys_to_pem(client->keys.pkey, &pubkey_pem, &pubkey_len, &privkey_pem, &privkey_len) != 0) {
        syslog(LOG_ERR, "Error: Failed to export keys to PEM format");
        free_client_keys(client);
        return -1;
    }

    if (send(client->socket_fd, pubkey_pem, pubkey_len, 0) != (ssize_t)pubkey_len) {
        syslog(LOG_ERR, "Error: Failed to send public key to client");
        free(pubkey_pem);
        free(privkey_pem);
        free_client_keys(client);
        return -1;
    }
    free(pubkey_pem);

    unsigned char encrypted_aes_key[256];
    size_t encrypted_key_len = 0;
    unsigned char iv[AES_IV_SIZE];

    if (mx_receive_aes(client, encrypted_aes_key, &encrypted_key_len, iv) != 0) {
        syslog(LOG_ERR, "Error: Failed to receive encrypted AES key and IV from client");
        free(privkey_pem);
        free_client_keys(client);
        return -1;
    }

    unsigned char *decrypted_key = NULL;
    size_t decrypted_key_len = 0;
    EVP_PKEY *private_key = PEM_read_bio_PrivateKey(BIO_new_mem_buf(privkey_pem, privkey_len), NULL, NULL, NULL);
    free(privkey_pem);

    if (!private_key || rsa_decrypt_aes_key(private_key, encrypted_aes_key, encrypted_key_len, 
                                            &decrypted_key, &decrypted_key_len) != 0) {
        syslog(LOG_ERR, "Error: Failed to decrypt AES key");
        if (private_key) EVP_PKEY_free(private_key);
        free_client_keys(client);
        return -1;
    }
    EVP_PKEY_free(private_key);

    if (decrypted_key_len != AES_KEY_SIZE) {
        syslog(LOG_ERR, "Error: Invalid decrypted AES key length");
        free(decrypted_key);
        free_client_keys(client);
        return -1;
    }

    memcpy(client->keys.aes_key, decrypted_key, AES_KEY_SIZE);
    memcpy(client->keys.aes_iv, iv, AES_IV_SIZE);
    free(decrypted_key);

    syslog(LOG_INFO, "Handshake successful: AES key and IV stored for client");

    return 0;
}

int rsa_keys_to_pem(EVP_PKEY *pkey, unsigned char **pubkey_pem, size_t *pubkey_len,
                    unsigned char **privkey_pem, size_t *privkey_len) {
    if (!pkey || !pubkey_pem || !pubkey_len || !privkey_pem || !privkey_len) {
        syslog(LOG_ERR, "Error: Invalid parameters for rsa_keys_to_pem");
        return -1;
    }

    BIO *pub_mem = BIO_new(BIO_s_mem());
    BIO *priv_mem = BIO_new(BIO_s_mem());
    if (!pub_mem || !priv_mem) {
        syslog(LOG_ERR, "Error: Failed to create BIOs for PEM export");
        if (pub_mem) BIO_free(pub_mem);
        if (priv_mem) BIO_free(priv_mem);
        return -1;
    }

    if (!PEM_write_bio_PUBKEY(pub_mem, pkey)) {
        syslog(LOG_ERR, "Error: Failed to write public key to PEM");
        BIO_free(pub_mem);
        BIO_free(priv_mem);
        return -1;
    }

    if (!PEM_write_bio_PrivateKey(priv_mem, pkey, NULL, NULL, 0, NULL, NULL)) {
        syslog(LOG_ERR, "Error: Failed to write private key to PEM");
        BIO_free(pub_mem);
        BIO_free(priv_mem);
        return -1;
    }

    char *pub_data;
    *pubkey_len = BIO_get_mem_data(pub_mem, &pub_data);
    *pubkey_pem = malloc(*pubkey_len);
    if (!*pubkey_pem) {
        syslog(LOG_ERR, "Error: Failed to allocate memory for public key PEM");
        BIO_free(pub_mem);
        BIO_free(priv_mem);
        return -1;
    }
    memcpy(*pubkey_pem, pub_data, *pubkey_len);

    char *priv_data;
    *privkey_len = BIO_get_mem_data(priv_mem, &priv_data);
    *privkey_pem = malloc(*privkey_len);
    if (!*privkey_pem) {
        syslog(LOG_ERR, "Error: Failed to allocate memory for private key PEM");
        free(*pubkey_pem);
        BIO_free(pub_mem);
        BIO_free(priv_mem);
        return -1;
    }
    memcpy(*privkey_pem, priv_data, *privkey_len);

    BIO_free(pub_mem);
    BIO_free(priv_mem);

    return 0;
}

int rsa_decrypt_aes_key(EVP_PKEY *private_key, const unsigned char *encrypted_aes_key, size_t encrypted_len, 
                        unsigned char **decrypted_key, size_t *decrypted_key_len) {
    if (!private_key || !encrypted_aes_key || !decrypted_key || !decrypted_key_len) {
        syslog(LOG_ERR, "Error: Invalid parameters for rsa_decrypt_aes_key");
        return -1;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(private_key, NULL);
    if (!ctx) {
        syslog(LOG_ERR, "Error: Failed to create EVP_PKEY_CTX");
        return -1;
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        syslog(LOG_ERR, "Error: EVP_PKEY_decrypt_init failed");
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    size_t out_len = 0;
    if (EVP_PKEY_decrypt(ctx, NULL, &out_len, encrypted_aes_key, encrypted_len) <= 0) {
        syslog(LOG_ERR, "Error: EVP_PKEY_decrypt failed to determine output length");
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    unsigned char *out = malloc(out_len);
    if (!out) {
        syslog(LOG_ERR, "Error: Failed to allocate memory for decrypted key");
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_decrypt(ctx, out, &out_len, encrypted_aes_key, encrypted_len) <= 0) {
        syslog(LOG_ERR, "Error: EVP_PKEY_decrypt failed to decrypt AES key");
        free(out);
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    *decrypted_key = out;
    *decrypted_key_len = out_len;

    EVP_PKEY_CTX_free(ctx);
    return 0;
}

int mx_receive_aes(t_client *client, unsigned char *encrypted_aes_key, size_t *encrypted_key_len, unsigned char *iv) {
    if (!client || !encrypted_aes_key || !encrypted_key_len || !iv) {
        syslog(LOG_ERR, "Error: Invalid arguments passed to mx_receive_aes");
        return -1;
    }

    char buffer[1024];
    ssize_t received = recv(client->socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        syslog(LOG_ERR, "Failed to receive data from Client id: %ld", client->thread_id);
        return -1;
    }
    buffer[received] = '\0';

    cJSON *json_payload = cJSON_Parse(buffer);
    if (!json_payload) {
        syslog(LOG_ERR, "Failed to parse JSON from Client id: %ld", client->thread_id);
        return -1;
    }

    cJSON *aes_key_b64_json = cJSON_GetObjectItem(json_payload, "aes_key");
    cJSON *iv_b64_json = cJSON_GetObjectItem(json_payload, "iv");
    if (!aes_key_b64_json || !iv_b64_json || 
        !cJSON_IsString(aes_key_b64_json) || !cJSON_IsString(iv_b64_json)) {
        syslog(LOG_ERR, "Invalid JSON structure from Client id: %ld", client->thread_id);
        cJSON_Delete(json_payload);
        return -1;
    }

    unsigned char *decoded_aes_key = base64_decode(aes_key_b64_json->valuestring, encrypted_key_len);
    if (!decoded_aes_key || *encrypted_key_len == 0 || *encrypted_key_len > 256) {
        syslog(LOG_ERR, "Failed to decode encrypted AES key from Client id: %ld", client->thread_id);
        cJSON_Delete(json_payload);
        if (decoded_aes_key) free(decoded_aes_key);
        return -1;
    }
    memcpy(encrypted_aes_key, decoded_aes_key, *encrypted_key_len);
    free(decoded_aes_key);

    size_t iv_len = 0;
    unsigned char *decoded_iv = base64_decode(iv_b64_json->valuestring, &iv_len);
    if (!decoded_iv || iv_len != AES_IV_SIZE) {
        syslog(LOG_ERR, "Failed to decode IV from Client id: %ld", client->thread_id);
        cJSON_Delete(json_payload);
        if (decoded_iv) free(decoded_iv);
        return -1;
    }
    memcpy(iv, decoded_iv, AES_IV_SIZE);
    free(decoded_iv);

    cJSON_Delete(json_payload);
    return 0;
}

unsigned char *encrypt_json_with_aes(const unsigned char *aes_key, const unsigned char *iv, 
                                     cJSON *json, size_t *out_len) {
    if (!aes_key || !iv || !json || !out_len) {
        syslog(LOG_ERR, "Invalid argument(s) to encrypt_json_with_aes");
        return NULL;
    }

    char *json_string = cJSON_PrintUnformatted(json);
    if (!json_string) {
        syslog(LOG_ERR, "Failed to convert JSON to string");
        return NULL;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        syslog(LOG_ERR, "Failed to create EVP_CIPHER_CTX");
        free(json_string);
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv) != 1) {
        syslog(LOG_ERR, "EVP_EncryptInit_ex failed");
        EVP_CIPHER_CTX_free(ctx);
        free(json_string);
        return NULL;
    }

    size_t json_len = strlen(json_string);
    size_t block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());
    size_t encrypted_data_len = json_len + block_size;

    unsigned char *encrypted_data = (unsigned char *)malloc(encrypted_data_len);
    if (!encrypted_data) {
        syslog(LOG_ERR, "Failed to allocate memory for encrypted data");
        EVP_CIPHER_CTX_free(ctx);
        free(json_string);
        return NULL;
    }

    int bytes_written = 0, final_bytes_written = 0;

    if (EVP_EncryptUpdate(ctx, encrypted_data, &bytes_written, 
                          (unsigned char *)json_string, json_len) != 1) {
        syslog(LOG_ERR, "EVP_EncryptUpdate failed");
        EVP_CIPHER_CTX_free(ctx);
        free(json_string);
        free(encrypted_data);
        return NULL;
    }

    if (EVP_EncryptFinal_ex(ctx, encrypted_data + bytes_written, &final_bytes_written) != 1) {
        syslog(LOG_ERR, "EVP_EncryptFinal_ex failed");
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
        syslog(LOG_ERR, "Invalid argument(s) to decrypt_received_data");
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        syslog(LOG_ERR, "Failed to create EVP_CIPHER_CTX");
        return -1;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv) != 1) {
        syslog(LOG_ERR, "EVP_DecryptInit_ex failed");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    size_t block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());
    size_t decrypted_data_len = data->len + block_size;
    unsigned char *decrypted_data = (unsigned char *)malloc(decrypted_data_len);
    if (!decrypted_data) {
        syslog(LOG_ERR, "Failed to allocate memory for decrypted data");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    int bytes_written = 0, final_bytes_written = 0;
    if (EVP_DecryptUpdate(ctx, decrypted_data, &bytes_written, (unsigned char *)data->data, data->len) != 1) {
        syslog(LOG_ERR, "EVP_DecryptUpdate failed");
        EVP_CIPHER_CTX_free(ctx);
        free(decrypted_data);
        return -1;
    }

    if (EVP_DecryptFinal_ex(ctx, decrypted_data + bytes_written, &final_bytes_written) != 1) {
        syslog(LOG_ERR, "EVP_DecryptFinal_ex failed");
        EVP_CIPHER_CTX_free(ctx);
        free(decrypted_data);
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);

    size_t total_decrypted_len = bytes_written + final_bytes_written;

    free(data->data);
    data->data = (char *)malloc(total_decrypted_len + 1); 
    if (!data->data) {
        syslog(LOG_ERR, "Failed to allocate memory for decrypted data in structure");
        free(decrypted_data);
        return -1;
    }

    memcpy(data->data, decrypted_data, total_decrypted_len);
    data->data[total_decrypted_len] = '\0';
    data->len = total_decrypted_len;

    free(decrypted_data);
    syslog(LOG_INFO, "Data successfully decrypted and updated in the structure");

    return 0;
}
