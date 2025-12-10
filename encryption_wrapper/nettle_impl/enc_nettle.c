#include "enc_nettle.h"

#include "nettle.h"

// FINISH IMPL:

static enc_algorithm alg;

static int nettle_prepare(enc_algorithm alg, size_t* key_len, size_t* nonce_len) {
    int cipher;
    int mode;

    switch(alg) {
        case aes256:
            cipher = GCRY_CIPHER_AES256;
            mode = GCRY_CIPHER_MODE_CBC;
            *nonce_len = gcry_cipher_get_algo_blklen(cipher);
            break;
        case chacha20:
            cipher = GCRY_CIPHER_CHACHA20;
            mode = GCRY_CIPHER_MODE_STREAM;
            *nonce_len = 12;
            break;
    }

    gcry_cipher_open(&handle, cipher, mode, 0);
    *key_len = gcry_cipher_get_algo_keylen(cipher);

    return 0;
}

static int nettle_set_key(char* key, size_t key_len) {
    gcry_cipher_setkey(handle, data, key_len);
    return 0;
}

static int nettle_set_nonce(char* nonce, size_t nonce_len) {
    gcry_cipher_setiv(handle, nonce, nonce_len);
    return 0;
}

static int nettle_encrypt(char* source, size_t souce_len, char* dest, size_t dest_len) {
    gcry_cipher_encrypt(handle, dest, dest_size, source, source_size);
    return 0;
}

static int nettle_decrypt(char* source, size_t souce_len, char* dest, size_t dest_len) {
    gcry_cipher_decrypt(handle, dest, dest_size, source, source_size);
    return 0;
}

static int nettle_reset() {
    gcry_cipher_reset(handle);
    return 0;
}

static char* make_random(size_t size) {
    char* data = malloc(size);
    gcry_randomize(data, size, GCRY_VERY_STRONG_RANDOM);
    return data;
}

char* nettle_make_key(size_t key_size) {
    return make_random(key_size);
}

char* nettle_make_nonce(size_t nonce_size) {
    return make_random(nonce_size);
}

enc_library_impl enc_get_nettle() {
    enc_library_impl ret = {
        .prepare = nettle_prepare,
        .set_key = nettle_set_key,
        .set_nonce = nettle_set_nonce,
        .encrypt = nettle_encrypt,
        .decrypt = nettle_decrypt,
        .reset = nettle_reset,
        .make_key = nettle_make_key,
        .make_nonce = nettle_make_nonce,
        .key_size = 0,
        .nonce_size = 0
    };
    return ret;
}
