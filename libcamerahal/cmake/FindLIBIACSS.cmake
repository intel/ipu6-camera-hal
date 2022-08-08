#
#  Copyright (C) 2017 Intel Corporation
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

# Get include and lib paths for LIBIACSS from pkgconfig
include(FindPackageHandleStandardArgs)

find_package(PkgConfig)
pkg_check_modules(LIBIACSS libiacss)
if(NOT LIBIACSS_FOUND)
    message(FATAL_ERROR "LIBIACSS not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${LIBIACSS_LIBRARY_DIRS})

# Libraries
find_library(GCSS_LIB      gcss)
find_library(IA_CAMERA_LIB ia_camera)
find_library(IA_CIPF_LIB   ia_cipf)
set(LIBIACSS_LIBS
    ${GCSS_LIB}
    ${IA_CAMERA_LIB}
    ${IA_CIPF_LIB}
    )

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(LIBIACSS
                                  REQUIRED_VARS LIBIACSS_INCLUDE_DIRS LIBIACSS_LIBS)

if(NOT LIBIACSS_FOUND)
    message(FATAL_ERROR "LIBIACSS not found")
endif()

