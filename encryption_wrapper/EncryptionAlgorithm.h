#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Algorithm {
    aes256,
    chacha20,
    camellia256,
    twofish
} Algorithm;

typedef Algorithm enc_algorithm;

#ifdef __cplusplus
}
#endif
