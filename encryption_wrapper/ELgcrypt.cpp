#include "ELgcrypt.h"

#include <cstring>

#include <stdexcept>

static void check_gcry(gcry_error_t err) {
    if(err != 0) {
        throw std::runtime_error{gcry_strerror(err)};
    }
}

ELgcrypt::ELgcrypt() {
    gcry_check_version(NULL);
    // gcry_control( GCRYCTL_DISABLE_SECMEM_WARN );
    // gcry_control( GCRYCTL_INIT_SECMEM, 16384, 0 );
}

void ELgcrypt::prepare(Algorithm alg) {
    int cipher;
    int mode;

    switch(alg) {
    case Algorithm::aes256:
        cipher = GCRY_CIPHER_AES256;
        mode = GCRY_CIPHER_MODE_CTR;
        nonceLen = gcry_cipher_get_algo_blklen(cipher);
        break;
    case Algorithm::chacha20:
        cipher = GCRY_CIPHER_CHACHA20;
        mode = GCRY_CIPHER_MODE_STREAM;
        nonceLen = 12;
        break;
    case Algorithm::camellia256:
        cipher = GCRY_CIPHER_CAMELLIA256;
        mode = GCRY_CIPHER_MODE_CBC;
        nonceLen = gcry_cipher_get_algo_blklen(cipher);
        break;
    case Algorithm::twofish:
        cipher = GCRY_CIPHER_TWOFISH;
        mode = GCRY_CIPHER_MODE_CBC;
        nonceLen = gcry_cipher_get_algo_blklen(cipher);
        break;
    default:
        break;
    }

    check_gcry(gcry_cipher_open(&handle, cipher, mode, 0));
    keyLen = gcry_cipher_get_algo_keylen(cipher);
}

void ELgcrypt::setKey(void* data, size_t keyLen) {
    check_gcry(gcry_cipher_setkey(handle, data, keyLen));
}

void ELgcrypt::setNonce(void* data, size_t nonceLen) {
    check_gcry(gcry_cipher_setiv(handle, data, nonceLen));
}

void ELgcrypt::encrypt(void* source, size_t sourceSize, void* dest, size_t destSize) {
    check_gcry(gcry_cipher_encrypt(handle, dest, destSize, source, sourceSize));
}

void ELgcrypt::decrypt(void* source, size_t sourceSize, void* dest, size_t destSize) {
    check_gcry(gcry_cipher_decrypt(handle, dest, destSize, source, sourceSize));
}

void ELgcrypt::reset() {
    check_gcry(gcry_cipher_reset(handle));
}
