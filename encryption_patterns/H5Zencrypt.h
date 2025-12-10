#pragma once
#include <hdf5.h>

#define ENCRYPTION_FILTER_ID = 131313

size_t encryption_filter(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[],
        size_t nbytes, size_t *buf_size, void** buf);

H5Z_class2_t encryption_filter_cls = {
    H5Z_CLASS_T_VERS,
    (H5Z_filter_t)ENCRYPTION_FILTER_ID,
    1,
    1,
    "Encryption Filter",
    NULL,
    NULL,
    encrypiton_filter
};

unsigned int encryption_filter_add_key(char * key, int id);
