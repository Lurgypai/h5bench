set(_NETTLE_ROOT_HINTS
    $ENV{NETTLE_ROOT_DIR}
)

FIND_PATH(NETTLE_INCLUDE_DIR
    NAMES
        nettle/md5.h
        nettle/ripemd160.h
        nettle/sha.h
    HINTS
        ${_NETTLE_ROOT_HINTS}
    PATH_SUFFIXES
        include
)


FIND_LIBRARY(NETTLE_LIBRARY
    NAMES
        nettle
        libnettle
    HINTS
        ${_NETTLE_ROOT_HINTS}
    PATH_SUFFIXES
        lib
        lib64
)

# handle the QUIETLY and REQUIRED arguments and set NETTLE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nettle DEFAULT_MSG NETTLE_LIBRARY NETTLE_INCLUDE_DIR)

IF(NETTLE_FOUND)
  SET(NETTLE_LIBRARIES ${NETTLE_LIBRARY})
ENDIF(NETTLE_FOUND)

if (NETTLE_LIBRARIES AND NETTLE_INCLUDE_DIR)
    add_library(Nettle::Nettle UNKNOWN IMPORTED)
    set_target_properties(Nettle::Nettle PROPERTIES
        IMPORTED_LOCATION "${NETTLE_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${NETTLE_INCLUDE_DIR}"
    )
endif()

