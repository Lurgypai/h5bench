#pragma once

#include <string.h>

#include "EncryptionAlgorithm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum enc_library {
    gcrypt,
    nettle,
} enc_library;

int load_library(enc_library lib);
int enc_prepare(enc_algorithm alg);
int enc_encrypt(void* source, size_t source_size, void* dest, size_t dest_size); 
int enc_decrypt(void* source, size_t source_size, void* dest, size_t dest_size); 
int reset();
char* makeKey(size_t length);

#ifdef __cplusplus
}
#endif
