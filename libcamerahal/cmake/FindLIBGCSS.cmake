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
pkg_check_modules(LIBGCSS libgcss)
if(NOT LIBGCSS_FOUND)
    message(FATAL_ERROR "LIBGCSS not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${LIBGCSS_LIBRARY_DIRS})

# Libraries
find_library(GCSS_LIB gcss)
set(LIBGCSS_LIBS ${GCSS_LIB})

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(LIBGCSS REQUIRED_VARS LIBGCSS_INCLUDE_DIRS LIBGCSS_LIBS)

if(NOT LIBGCSS_FOUND)
        message(FATAL_ERROR "LIBGCSS not found")
endif()

