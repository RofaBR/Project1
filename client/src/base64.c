#include "../inc/client.h"

char *base64_encode(const unsigned char *input, size_t input_len) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bio);

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO_write(b64, input, input_len);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &buffer_ptr);

    char *base64_str = (char *)malloc(buffer_ptr->length + 1);
    if (!base64_str) {
        BIO_free_all(b64);
        return NULL;
    }

    memcpy(base64_str, buffer_ptr->data, buffer_ptr->length);
    base64_str[buffer_ptr->length] = '\0';

    BIO_free_all(b64);
    return base64_str;
}

unsigned char *base64_decode(const char *input, size_t *output_len) {
    BIO *bio, *b64;
    int input_len = strlen(input);

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input, input_len);
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    unsigned char *buffer = (unsigned char *)malloc(input_len);
    if (!buffer) {
        BIO_free_all(bio);
        return NULL;
    }

    *output_len = BIO_read(bio, buffer, input_len);
    if (*output_len <= 0) {
        free(buffer);
        buffer = NULL;
    }

    BIO_free_all(bio);
    return buffer;
}

