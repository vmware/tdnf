# - Try to find metalink
# Once done this will define
#  METALINK_FOUND - System has metalink
#  METALINK_INCLUDE_DIRS - The metalink include directories
#  METALINK_LIBRARIES - The libraries needed to use metalink

find_path(METALINK_INCLUDE_DIR metalink/metalink_parser.h)
find_library(METALINK_LIBRARY NAMES libmetalink.so)

# handle the QUIETLY and REQUIRED arguments and set METALINK_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libmetalink DEFAULT_MSG
                                  METALINK_LIBRARY METALINK_INCLUDE_DIR)


set(METALINK_LIBRARIES ${METALINK_LIBRARY})
set(METALINK_INCLUDE_DIRS ${METALINK_INCLUDE_DIR})
