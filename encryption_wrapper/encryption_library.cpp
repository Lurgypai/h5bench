#include "encryption_library.h"

#include "ELgcrypt.h"
#include "ELnettle.h"

#include <memory>
#include <random>
#include <cstdlib>

static std::unique_ptr<EncryptionLibrary> EncLib{};

int enc_load_library(enc_library lib) {
    switch(lib) {
        case gcrypt:
            EncLib = std::make_unique<ELgcrypt>();
            break;
        case nettle:
            EncLib = std::make_unique<ELnettle>();
            break;
    }

    return 0;
}

int enc_prepare(enc_algorithm alg) {
    EncLib->prepare(alg);
    return 0;
}

int enc_set_key(char* key, size_t key_len) {
    EncLib->setKey(key, key_len);
    return 0;
}

int enc_set_nonce(char* nonce, size_t nonce_len) {
    EncLib->setNonce(nonce, nonce_len);
}

int enc_encrypt(void* source, size_t source_size, void* dest, size_t dest_size) {
    EncLib->encrypt(source, source_size, dest, dest_size);
    return 0;
}

int enc_decrypt(void* source, size_t source_size, void* dest, size_t dest_size) {
    EncLib->decrypt(source, source_size, dest, dest_size);
    return 0;
}

int reset() {
    EncLib->reset();
    return 0;
}

size_t enc_get_key_size() {
    return EncLib->getKeySize();
}

size_t enc_get_nonce_size() {
    return EncLib->getNonceSize();
}

static char* make_random_data(size_t length) {
    static std::random_device d;
    static std::default_random_engine e{d()};
    static std::uniform_int_distribution<unsigned char> dist{0, 255};
    char* key = static_cast<char*>(malloc(length));
    for(int i = 0; i != length; ++i) {
        key[i] = dist(e);
    }
    return key;
}

char* enc_make_key() {
    return make_random_data(EncLib->getKeySize());
}

char* enc_make_nonce() {
    return make_random_data(EncLib->getNonceSize());
}
