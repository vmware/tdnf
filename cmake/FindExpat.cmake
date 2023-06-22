# - Try to find expat
# Once done this will define
#  EXPAT_INCLUDE_DIRS - The expat include directories
#  EXPAT_LIBRARIES - The libraries needed to use expat-devel

find_path(EXPAT_INCLUDE_DIR expat.h)
find_library(EXPAT_LIBRARY NAMES libexpat.so)

find_package_handle_standard_args(expat DEFAULT_MSG
                                  EXPAT_LIBRARY EXPAT_INCLUDE_DIR)

set(EXPAT_LIBRARIES ${EXPAT_LIBRARY})
set(EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIR})
