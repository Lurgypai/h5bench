#pragma once
#include "EncryptionLibrary.h"
#include "gcrypt.h"

class ELgcrypt : public EncryptionLibrary {
public:
    ELgcrypt();
    void prepare(Algorithm alg) override;
    void setKey(void* data, size_t keyLen) override;
    void setNonce(void* data, size_t nonceLen) override;
    void encrypt(void* source, size_t sourceSize, void* dest, size_t destSize) override;
    void decrypt(void* source, size_t sourceSize, void* dest, size_t destSize) override;
    void reset() override;
private:
    gcry_cipher_hd_t handle;
};
