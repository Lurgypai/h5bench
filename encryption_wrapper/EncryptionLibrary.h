#pragma once

#include <string>

#include "EncryptionAlgorithm.h"

class EncryptionLibrary {
public:
    virtual ~EncryptionLibrary() {};
    // set and initialize the algorithm used, returns the length of the key needed
    virtual void prepare(Algorithm alg) = 0;
    virtual void setKey(void* data, size_t keyLen) = 0;
    virtual void setNonce(void* data, size_t nonceLen) = 0;
    // run encryption using set algorithm
    virtual void encrypt(void* source, size_t sourceSize, void* dest, size_t destSize) = 0;
    // run decryption using set algorithm
    virtual void decrypt(void* source, size_t sourceSize, void* dest, size_t destSize) = 0;
    // resets any encryption state that needs to be reset
    virtual void reset() = 0;

    size_t getNonceSize();
    size_t getKeySize();
    std::string makeKey();
    std::string makeNonce();
protected:
    size_t keyLen;
    size_t nonceLen;
};
