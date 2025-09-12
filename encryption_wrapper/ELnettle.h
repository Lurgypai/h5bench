#pragma once

#include <cstdint>

#include <vector>

#include "EncryptionLibrary.h"
#include "nettle/aes.h"
#include "nettle/chacha.h"

class ELnettle : public EncryptionLibrary {
public:
    size_t prepare(Algorithm alg) override;
    void setKey(void* data, size_t keyLen) override;
    void encrypt(void* source, size_t sourceSize, void* dest, size_t destSize) override;
    void decrypt(void* source, size_t sourceSize, void* dest, size_t destSize) override;
    void reset() override;
private:
    Algorithm alg;
    aes256_ctx aes256ctx;
    chacha_ctx chachactx;
    // shouldn't store probably, fine for testing
    std::vector<std::uint8_t> key;

    // nonce for chacha
    std::vector<std::uint8_t> nonce;
};
