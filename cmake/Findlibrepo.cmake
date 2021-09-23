# - Try to find librepo
# Once done this will define
#  LIBREPO_FOUND - System has librepo
#  LIBREPO_INCLUDE_DIRS - The librepo include directories
#  LIBREPO_LIBRARIES - The libraries needed to use librepo

find_path(LIBREPO_INCLUDE_DIR librepo/metalink.h)
find_library(LIBREPO_LIBRARY NAMES librepo.so)

# handle the QUIETLY and REQUIRED arguments and set LIBREPO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(librepo DEFAULT_MSG
				LIBREPO_LIBRARY LIBREPO_INCLUDE_DIR)


set(LIBREPO_LIBRARIES ${LIBREPO_LIBRARY})
set(LIBREPO_INCLUDE_DIRS ${LIBREPO_INCLUDE_DIR})
