# - Try to find GCrypt
# Once done this will define
#
#  GCRYPT_FOUND - system has GCrypt
#  GCRYPT_INCLUDE_DIRS - the GCrypt include directory
#  GCRYPT_LIBRARIES - Link these to use GCrypt
#  GCRYPT_DEFINITIONS - Compiler switches required for using GCrypt
#
#=============================================================================
#  Copyright (c) 2009-2012 Andreas Schneider <asn@cryptomilk.org>
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file Copyright.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
#=============================================================================
#

find_path(GPG_ERROR_INCLUDE_DIR
    NAMES
        gpg-error.h
        gpgrt.h
    PATHS
        ENV GPG_ERROR_ROOT_DIR
    PATH_SUFFIXES
        include
)

find_library(GPG_ERROR_LIBRARY
    NAMES
        gpg-error
        libgpg-error-0
        libgpg-error6-0
    PATHS
        ENV GPG_ERROR_ROOT_DIR
    PATH_SUFFIXES
        lib
)
set(GPG_ERROR_LIBRARIES ${GPG_ERROR_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GPGError
    REQUIRED_VARS
        GPG_ERROR_LIBRARIES
        GPG_ERROR_INCLUDE_DIR
)

if (GPG_ERROR_LIBRARIES AND GPG_ERROR_INCLUDE_DIR)
    add_library(GPGError::GPGError UNKNOWN IMPORTED)
    set_target_properties(GPGError::GPGError PROPERTIES
        IMPORTED_LOCATION "${GPG_ERROR_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${GPG_ERROR_INCLUDE_DIR}"
    )
endif()
