# - Try to find openssl
# Once done this will define
#  OPENSSL_INCLUDE_DIRS - The openssl include directories
#  OPENSSL_LIBRARIES - The libraries needed to use openssl-devel

find_path(OPENSSL_INCLUDE_DIR openssl/sha.h)
find_library(OPENSSL_LIBRARY NAMES ssl libssl.so)
find_library(CRYPTO_LIBRARY NAMES crypto libcrypto.so)

find_package_handle_standard_args(OpenSSL DEFAULT_MSG
                                  OPENSSL_LIBRARY CRYPTO_LIBRARY OPENSSL_INCLUDE_DIR)

set(OPENSSL_LIBRARIES ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY})
set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
