# - Try to find gpgme
# Once done this will define
#  GPGME_FOUND - System has gpgme
#  GPGME_INCLUDE_DIRS - The gpgme include directories
#  GPGME_LIBRARIES - The libraries needed to use gpgme

find_path(GPGME_INCLUDE_DIR gpgme.h)

find_library(GPGME_LIBRARY NAMES gpgme)

# handle the QUIETLY and REQUIRED arguments and set GPGME_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(gpgme DEFAULT_MSG
                                  GPGME_LIBRARY GPGME_INCLUDE_DIR)

mark_as_advanced(GPGME_INCLUDE_DIR GPGME_LIBRARY )

set(GPGME_LIBRARIES ${GPGME_LIBRARY} )
set(GPGME_INCLUDE_DIRS ${GPGME_INCLUDE_DIR} )

