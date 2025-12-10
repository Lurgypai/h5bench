#pragma once
#include "enc_algorithm.h"
#include "enc_library.h"

int enc_load_library(enc_library_impl impl);
int enc_prepare(enc_algorithm alg);
int enc_set_key(char* key, size_t key_len);
int enc_set_nonce(char* nonce, size_t key_len);
int enc_encrypt(void* source, size_t source_size, void* dest, size_t dest_size); 
int enc_decrypt(void* source, size_t source_size, void* dest, size_t dest_size); 
int enc_reset();
size_t enc_get_key_size();
size_t enc_get_nonce_size();
char* enc_make_key();
char* enc_make_nonce();

