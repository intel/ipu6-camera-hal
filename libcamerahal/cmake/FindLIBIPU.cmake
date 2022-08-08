#
#  Copyright (C) 2017-2019 Intel Corporation
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

if (TARGET ${IPU_VER})
    message("libcamhal found lib${IPU_VER} target")
    return()
endif()

# Get include and lib paths for LIBIPU from pkgconfig
include(FindPackageHandleStandardArgs)

# Include directory
find_package(PkgConfig)
if(NOT DEFINED IPU_VER)
set(libipu_ver libipu4)
else()
set(libipu_ver lib${IPU_VER})
endif()
pkg_check_modules(LIBIPU ${libipu_ver})
if(NOT LIBIPU_FOUND)
    message(FATAL_ERROR "LIBIPU not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${LIBIPU_LIBRARY_DIRS})

# Libraries
find_library(IPU_LIB NAMES ${libipu_ver}.a)

set(LIBIPU_LIBS ${IPU_LIB})

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(LIBIPU
                                  REQUIRED_VARS LIBIPU_INCLUDE_DIRS LIBIPU_LIBS)

if(NOT LIBIPU_FOUND)
    message(FATAL_ERROR "LIBIPU not found")
endif()
