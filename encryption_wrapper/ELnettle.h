#pragma once

#include <cstdint>

#include <vector>

#include "EncryptionLibrary.h"
#include "nettle/aes.h"
#include "nettle/chacha.h"

class ELnettle : public EncryptionLibrary {
public:
    void prepare(Algorithm alg) override;
    void setKey(void* data, size_t keyLen) override;
    void setNonce(void* data, size_t nonceLen) override;
    void encrypt(void* source, size_t sourceSize, void* dest, size_t destSize) override;
    void decrypt(void* source, size_t sourceSize, void* dest, size_t destSize) override;
    // need to store only one ctx instead of two to clean up internals
    void reset() override;
private:
    Algorithm alg;
    aes256_ctx aes256ctx;
    chacha_ctx chachactx;
};
