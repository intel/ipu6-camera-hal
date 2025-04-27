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
    if (NOT BUILD_CAMHAL_PLUGIN)
        message("libcamhal found lib${IPU_VER} target")
        return()
    endif()
endif()

if(NOT DEFINED IPU_VER)
    set(libipu_ver libipu4)
else()
    set(libipu_ver lib${IPU_VER})
endif()

# Get include and lib paths for LIBIPU from pkgconfig

find_package(PkgConfig)
pkg_check_modules(LIBIPU${TARGET_SUFFIX} ${libipu_ver})
if(NOT LIBIPU${TARGET_SUFFIX}_FOUND)
    message(FATAL_ERROR "LIBIPU${TARGET_SUFFIX} not found")
endif()
