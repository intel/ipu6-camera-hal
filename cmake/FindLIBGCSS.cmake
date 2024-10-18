#
#  Copyright (C) 2021 Intel Corporation
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# Get include and lib paths for LIBGCSS from pkgconfig
include(FindPackageHandleStandardArgs)

find_package(PkgConfig)
pkg_check_modules(LIBGCSS${TARGET_SUFFIX} libgcss${TARGET_SUFFIX})
if(NOT LIBGCSS${TARGET_SUFFIX}_FOUND)
    message(FATAL_ERROR "LIBGCSS${TARGET_SUFFIX} not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${LIBGCSS${TARGET_SUFFIX}_LIBRARY_DIRS})

# Libraries
find_library(GCSS${TARGET_SUFFIX}_LIB gcss${TARGET_SUFFIX})
set(LIBGCSS${TARGET_SUFFIX}_LIBS ${GCSS${TARGET_SUFFIX}_LIB})

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(LIBGCSS${TARGET_SUFFIX} REQUIRED_VARS LIBGCSS${TARGET_SUFFIX}_INCLUDE_DIRS LIBGCSS${TARGET_SUFFIX}_LIBS)

if(NOT LIBGCSS${TARGET_SUFFIX}_FOUND)
        message(FATAL_ERROR "LIBGCSS${TARGET_SUFFIX} not found")
endif()

