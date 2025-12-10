cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# encryption library wrappers
project(encryption-wrapper)

option(ENC_WRAPPER_ENABLE_GCRYPT "Enable gcrypt library support." ON)
option(ENC_WRAPPER_ENABLE_NETTLE "Enable nettle library support." ON)

set(ENC_WRAPPER_SRC "enc_wrapper.c")
set(ENC_WRAPPER_LIBS)

# ENABLE GCRYPT
if(ENC_WRAPPER_ENABLE_GCRYPT)
    find_package(GCrypt REQUIRED)
    find_package(GPGError REQUIRED)

    list(APPEND ENC_WRAPPER_SRC "gcrypt_impl/enc_gcrypt.c")
    list(APPEND ENC_WRAPPER_LIBS GCrypt::GCrypt GPGError::GPGError)
    message(STATUS "gcrypt enabled")
else()
    message(STATUS "gcrypt disabled")
endif()

# ENABLE NETTLE
if(ENC_WRAPPER_ENABLE_NETTLE)
    find_package(Nettle REQUIRED)

    list(APPEND ENC_WRAPPER_SRC "nettle_impl/enc_nettle.c")
    list(APPEND ENC_WRAPPER_LIBS Nettle::Nettle)
    message(STATUS "nettle enabled")
else()
    message(STATUS "nettle disabled")
endif()

#ADD LIBRARY
add_library(enc_wrapper ${ENC_WRAPPER_SRC})
if(ENC_WRAPPER_LIBS)
    target_link_libraries(enc_wrapper PUBLIC ${ENC_WRAPPER_LIBS})
endif()
