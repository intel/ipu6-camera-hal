#
#  Copyright (C) 2023 Intel Corporation
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

cmake_minimum_required(VERSION 2.8)
project(hal_adaptor)

if (UPSTREAM_CODE)
    add_compile_definitions(IPU6_UPSTREAM=1)
else()
    add_compile_definitions(IPU6_UPSTREAM=0)
endif()

# Source directories
set(ROOT_DIR        ${CMAKE_CURRENT_LIST_DIR}/../../../)
set(METADATA_DIR    ${ROOT_DIR}/src/metadata)
set(IUTILS_DIR      ${ROOT_DIR}/src/iutils)
set(HAL_ADAPTOR_DIR ${ROOT_DIR}/src/hal/hal_adaptor)

include_directories(${ROOT_DIR}/include
                    ${ROOT_DIR}/include/api
                    ${ROOT_DIR}/include/utils
                    ${ROOT_DIR}/src/metadata
                    ${ROOT_DIR}/src/iutils
                    ${ROOT_DIR}/src/core
                    ${ROOT_DIR}/src/platformdata
                    ${ROOT_DIR}/src/isp_control
                    ${ROOT_DIR}/src
                    ${ROOT_DIR}/modules/v4l2
                    ${ROOT_DIR}
                    )

# Install directories
# CMAKE_INSTALL_PREFIX is defined when executing cmake.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}" CACHE PATH "Target location for install command." FORCE)
endif()
message(STATUS "install prefix: " ${CMAKE_INSTALL_PREFIX})

include(GNUInstallDirs)

# Set the aligned flag
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-faligned-new COMPILER_SUPPORTS_ALIGNED_NEW)
if (COMPILER_SUPPORTS_ALIGNED_NEW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
endif()

# Set the CXX to 11
set (CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -Werror
                    -fstack-protector
                    -fPIE -fPIC
                    -D_FORTIFY_SOURCE=2
                    -DDCHECK_ALWAYS_ON
                    -Wformat -Wformat-security
                    )

add_definitions(-D__STDC_FORMAT_MACROS
                -DHAVE_PTHREADS
                -DHAVE_LINUX_OS
                )

set(HAL_ADAPTOR_LD_FLAGS "-fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-z,relro -Wl,-z,now")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${HAL_ADAPTOR_LD_FLAGS}")

set(HAL_ADAPTOR_SRCS
    ${METADATA_DIR}/icamera_metadata_base.cpp
    ${METADATA_DIR}/CameraMetadata.cpp
    ${METADATA_DIR}/Parameters.cpp
    ${METADATA_DIR}/ParameterHelper.cpp
    ${IUTILS_DIR}/CameraLog.cpp
    ${IUTILS_DIR}/LogSink.cpp
    ${IUTILS_DIR}/ModuleTags.cpp
    ${IUTILS_DIR}/Trace.cpp
    ${IUTILS_DIR}/Utils.cpp
    ${IUTILS_DIR}/ScopedAtrace.cpp
    ${HAL_ADAPTOR_DIR}/HalAdaptor.cpp
    CACHE INTERNAL "hal adaptor sources"
)

add_library(hal_adaptor SHARED ${HAL_ADAPTOR_SRCS})

#---------------------------- Link settings ----------------------------
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)
target_link_libraries(hal_adaptor ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(hal_adaptor ${CMAKE_DL_LIBS})
target_link_libraries(hal_adaptor rt)

# Install headers
install(DIRECTORY ${ROOT_DIR}/include/ DESTINATION include/hal_adaptor)
if (SUPPORT_LIVE_TUNING)
    install(FILES ${ROOT_DIR}/modules/livetune/LiveTuning.h DESTINATION include/hal_adaptor/api)
endif()

# Install libraries
if (${CMAKE_VERSION} VERSION_LESS 3.11)
    install(TARGETS hal_adaptor
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
else()
    install(TARGETS hal_adaptor DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Install package config file
configure_file(${PROJECT_SOURCE_DIR}/hal_adaptor.pc.cmakein
               ${PROJECT_SOURCE_DIR}/hal_adaptor.pc @ONLY)
install(FILES hal_adaptor.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set(CPACK_GENERATOR "RPM")
include(CPack)
