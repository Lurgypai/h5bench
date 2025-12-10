#include "H5Zencryption.h"

#include <string.h>

struct key_map_ {
    size_t backing_size;
    size_t size;
    char ** keys;
};

static struct key_map_ key_map = {
    backing_size = 0;
    size = 0,
    keys = NULL,
};

size_t encryption_filter(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[],
        size_t nbytes, size_t *buf_size, void** buf) {

    if(cd_nelmts != 1) return 0;

    enc_set_key(key_map.keys[cd_values[0]], enc_get_key_size());

    size_t out_size = 0;
    void* out_buf = NULL;

    // read
    if(H5Z_FLAG_REVERSE) {
        // set nonce
        size_t nonce_size = enc_get_nonce_size();
        enc_set_nonce(*buf, nonce_size);
        // allocate out buffer
        out_size = *buf_size - nonce_size;
        out_buf = malloc(out_size);
        // decrypt
        enc_decrypt(*buf + nonce_size, out_size, out_buf, out_size);
    }
    // write
    else {
        // generate nonce
        size_t nonce_size = enc_get_nonce_size();
        char * nonce = enc_make_nonce();
        // allocate buffer with space for nonce and any necessary padding
        size_t out_size = *buf_size + nonce_size;
        void* outBuf = malloc(out_size);
        // do encrypt
        enc_encrypt(*buf, *buf_size, out_buf + nonce_size, *buf_size);
    }

    // swap buffer
    *buf_size = out_size;
    free(*buf);
    *buf = out_buf;

    return out_size;
}

unsigned int encryption_filter_add_key(char* key, unsigned int id) {
    ++key_map.size;
    if(key_map.size > key_map.backing_size) {
        key_map.backing_size = key_map.size;
        key_map.backing_size *= 2;
        char** new_keys = malloc(sizeof(char*) * key_map.backing_size);
        free(key_map.keys);
        key_map.keys = new_keys;
    }
    key_map.keys[key_map.size - 1] = key;

    return key_map.size - 1;
}
