#include "enc_library.h"

#include <stdlib.h>

static enc_library_impl Enc_Library_Impl;

int enc_load_library(enc_library_impl impl) {
    Enc_Library_Impl = impl;
    return 0;
}

int enc_prepare(enc_algorithm alg) {
    (*Enc_Library_Impl.prepare)(alg, &Enc_Library_Impl.key_size, &Enc_Library_Impl.nonce_size);
    return 0;
}

int enc_set_key(char* key, size_t key_len) {
    (*Enc_Library_Impl.set_key)(key, key_len);
    return 0;
}

int enc_set_nonce(char* nonce, size_t nonce_len) {
    (*Enc_Library_Impl.set_nonce)(nonce, nonce_len);
}

int enc_encrypt(void* source, size_t source_size, void* dest, size_t dest_size) {
    (*Enc_Library_Impl.encrypt)(source, source_size, dest, dest_size);
    return 0;
}

int enc_decrypt(void* source, size_t source_size, void* dest, size_t dest_size) {
    (*Enc_Library_Impl.decrypt)(source, source_size, dest, dest_size);
    return 0;
}

int enc_reset() {
    (*Enc_Library_Impl.reset)();
    return 0;
}

size_t enc_get_key_size() {
    return Enc_Library_Impl.key_size;
}

size_t enc_get_nonce_size() {
    return Enc_Library_Impl.nonce_size;
}

char* enc_make_key() {
    return (*Enc_Library_Impl.make_key)(Enc_Library_Impl.key_size);
}

char* enc_make_nonce() {
    return (*Enc_Library_Impl.make_nonce)(Enc_Library_Impl.nonce_size);
}
