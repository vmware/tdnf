# - Try to find libxml2
# Once done this will define
#  LIBXML2_INCLUDE_DIRS - The libxml2 include directories
#  LIBXML2_LIBRARIES - The libraries needed to use libxml2-devel

find_path(LIBXML2_INCLUDE_DIR libxml/parser.h)
find_library(LIBXML2_LIBRARY NAMES libxml2.so)

find_package_handle_standard_args(libxml2 DEFAULT_MSG
                                  LIBXML2_LIBRARY LIBXML2_INCLUDE_DIR)

set(LIBXML2_LIBRARIES ${LIBXML2_LIBRARY})
set(LIBXML2_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR})
