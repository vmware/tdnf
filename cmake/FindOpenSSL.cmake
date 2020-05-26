# - Try to find openssl
# Once done this will define
#  OPENSSL_INCLUDE_DIRS - The openssl include directories
#  OPENSSL_LIBRARIES - The libraries needed to use openssl-devel

find_path(OPENSSL_INCLUDE_DIR openssl/sha.h)
find_library(OPENSSL_LIBRARY NAMES libssl.so)

find_package_handle_standard_args(libssl DEFAULT_MSG
                                  OPENSSL_LIBRARY OPENSSL_INCLUDE_DIR)

set(OPENSSL_LIBRARIES ${OPENSSL_LIBRARY})
set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
