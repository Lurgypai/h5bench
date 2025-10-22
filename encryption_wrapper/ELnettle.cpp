#include "ELnettle.h"

#include <cstring>
#include <stdexcept>

void ELnettle::prepare(Algorithm alg_) {
    alg = alg_;

    switch(alg) {
        case Algorithm::aes256:
            keyLen = AES256_KEY_SIZE;
            nonceLen = AES_BLOCK_SIZE;
            break;
        case Algorithm::chacha20:
            keyLen = CHACHA_KEY_SIZE;
            nonceLen = 12;
            break;
        default:
            throw std::runtime_error{"Nettle is not set up with support for the desired algorithm"};
            break;
    }
}

void ELnettle::setKey(void* data, size_t keyLen) {
    switch(alg) {
        case Algorithm::chacha20:
            chacha_set_key(&chachactx, static_cast<std::uint8_t*>(data));
            break;
        case Algorithm::aes256:
            aes256_set_encrypt_key(&aes256ctx, static_cast<std::uint8_t*>(data));
            aes256_set_decrypt_key(&aes256ctx, static_cast<std::uint8_t*>(data));
            break;
        default:
            break;
    }
}

void ELnettle::setNonce(void* data, size_t nonceLen) {
    switch(alg) {
        case Algorithm::chacha20:
            chacha_set_nonce(&chachactx, static_cast<std::uint8_t*>(data));
            break;
        default:
            break;
    }
}

void ELnettle::encrypt(void* source, size_t sourceSize, void* dest, size_t destSize) {
    switch(alg) {
        case Algorithm::aes256:
            aes256_encrypt(&aes256ctx, sourceSize,
                    static_cast<std::uint8_t*>(dest),
                    static_cast<std::uint8_t*>(source));
            break;
        case Algorithm::chacha20:
            chacha_crypt32(&chachactx, sourceSize, 
                    static_cast<std::uint8_t*>(dest),
                    static_cast<std::uint8_t*>(source));
            break;
        default:
            throw std::runtime_error{"Tried to use nettle with an unsupported algorithm"};
            break;
    }
}

void ELnettle::decrypt(void* source, size_t sourceSize, void* dest, size_t destSize) {
    switch(alg) {
        case Algorithm::aes256:
            aes256_decrypt(&aes256ctx, sourceSize,
                    static_cast<std::uint8_t*>(dest),
                    static_cast<std::uint8_t*>(source));
            break;
        case Algorithm::chacha20:
            chacha_crypt32(&chachactx, sourceSize, 
                    static_cast<std::uint8_t*>(dest),
                    static_cast<std::uint8_t*>(source));
            break;
        default:
            throw std::runtime_error{"Tried to use nettle with an unsupported algorithm"};
            break;
    }
}
void ELnettle::reset() {
    switch(alg) {
        default:
            break;
    }
};
