#include "EncryptionLibrary.h"

#include <random>

std::size_t EncryptionLibrary::getKeySize() {
    return keyLen;
}

std::size_t EncryptionLibrary::getNonceSize() {
    return nonceLen;
}

static inline std::string genRandomData(std::size_t length) {
    static std::random_device d;
    static std::default_random_engine e{d()};
    static std::uniform_int_distribution<unsigned char> dist{0, 255};
    std::string s;
    s.resize(length);
    for(auto& c : s) c = dist(e);
    return s;
}

std::string EncryptionLibrary::makeKey() {
    return genRandomData(keyLen);
}

std::string EncryptionLibrary::makeNonce() {
    return genRandomData(nonceLen);
}
