cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# encryption library wrappers
project(encryption-wrapper)

find_package(GCrypt REQUIRED)
find_package(Nettle REQUIRED)

add_library(enc-lib "./EncryptionLibrary.cpp" "./ELgcrypt.cpp" "./ELnettle.cpp" "encryption_library.cpp")

target_link_libraries(enc-lib gcrypt)
target_link_libraries(enc-lib nettle)
