#include "EncryptionLibrary.h"

#include <random>

std::string EncryptionLibrary::MakeKey(size_t length) {
    static std::random_device d;
    static std::default_random_engine e{d()};
    static std::uniform_int_distribution<unsigned char> dist{0, 255};
    std::string s;
    s.resize(length);
    for(auto& c : s) c = dist(e);
    return s;
}
