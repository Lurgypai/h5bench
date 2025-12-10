#pragma once

#include <stddef.h>
#include "enc_algorithm.h"

typedef struct enc_library_impl {
    int (*prepare)(enc_algorithm alg, size_t* key_len, size_t* nonce_len);
    int (*set_key)(char* key, size_t key_len);
    int (*set_nonce)(char* nonce, size_t key_len);
    int (*encrypt)(void* source, size_t source_size, void* dest, size_t dest_size); 
    int (*decrypt)(void* source, size_t source_size, void* dest, size_t dest_size); 
    int (*reset)();

    char* (*make_key)(size_t key_size);
    char* (*make_nonce)(size_t nonce_size);

    size_t key_size;
    size_t nonce_size;
} enc_library_impl;
