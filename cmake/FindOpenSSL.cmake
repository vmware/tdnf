# - Try to find openssl
# Once done this will define
#  OPENSSL_INCLUDE_DIRS - The openssl include directories
#  OPENSSL_LIBRARIES - The libraries needed to use openssl-devel

find_path(OPENSSL_INCLUDE_DIR openssl/sha.h)
if(APPLE)
    # This *should* also work on Linux, but needs to be tested.
    # Keeping this guarded for now.
    find_library(OPENSSL_SSL_LIBRARY NAMES ssl)
    find_library(OPENSSL_CRYPTO_LIBRARY NAMES crypto)
    find_package_handle_standard_args(OpenSSL DEFAULT_MSG
                                      OPENSSL_SSL_LIBRARY OPENSSL_CRYPTO_LIBRARY OPENSSL_INCLUDE_DIR)
    set(OPENSSL_LIBRARY ${OPENSSL_SSL_LIBRARY})

else()
    find_library(OPENSSL_LIBRARY NAMES libssl.so)
    find_package_handle_standard_args(libssl DEFAULT_MSG
                                      OPENSSL_LIBRARY OPENSSL_INCLUDE_DIR)
endif()

if(APPLE)
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
else()
    set(OPENSSL_LIBRARIES ${OPENSSL_LIBRARY})
endif()
set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
