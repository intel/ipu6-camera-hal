#
#  Copyright (C) 2024 Intel Corporation
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

# Get include and lib paths for LIBDRM from pkgconfig
include(FindPackageHandleStandardArgs)

find_package(PkgConfig)
pkg_check_modules(LIBDRM libdrm)
if(NOT LIBDRM_FOUND)
    message(SEND_ERROR "LIBDRM not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${LIBDRM_LIBRARY_DIRS})

# Libraries
find_library(DRM_LIB drm)
set(LIBDRM_LIBS ${DRM_LIB})

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(LIBDRM REQUIRED_VARS LIBDRM_INCLUDE_DIRS LIBDRM_LIBS)

if(NOT LIBDRM_FOUND)
        message(SEND_ERROR "LIBDRM not found")
endif()

