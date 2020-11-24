#
#  Copyright (C) 2017-2020 Intel Corporation
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

project(libcamhal)

option(USE_STATIC_GRAPH "Use static graph" OFF)

#---------------------------- Fold settings ----------------------------

# Source directories
set(SRC_ROOT_DIR        ${CMAKE_CURRENT_LIST_DIR}/src)

set(3A_DIR              ${SRC_ROOT_DIR}/3a)
set(CORE_DIR            ${SRC_ROOT_DIR}/core)
set(HAL_DIR             ${SRC_ROOT_DIR}/hal)
set(IIO_DIR             ${SRC_ROOT_DIR}/iio)
set(IUTILS_DIR          ${SRC_ROOT_DIR}/iutils)
set(METADATA_DIR        ${SRC_ROOT_DIR}/metadata)
set(PLATFORMDATA_DIR    ${SRC_ROOT_DIR}/platformdata)
set(V4L2_DIR            ${SRC_ROOT_DIR}/v4l2)
set(MODULES_DIR         ${CMAKE_CURRENT_LIST_DIR}/modules)
set(ALGOWRAPPER_DIR     ${MODULES_DIR}/algowrapper)
set(SANDBOXING_DIR      ${MODULES_DIR}/sandboxing)
set(IMAGE_PROCESS_DIR   ${SRC_ROOT_DIR}/image_process)

#AAL directories
if (CAL_BUILD)
    set(AAL_ROOT_DIR       ${CMAKE_CURRENT_LIST_DIR}/aal)
    set(JPEG_DIR           ${SRC_ROOT_DIR}/jpeg)
    set(MEMORY_DIR         ${MODULES_DIR}/memory)
endif() #CAL_BUILD

if (SUPPORT_LIVE_TUNING)
    #LiveTuning directories
    set(LIVE_TUNING_DIR         ${MODULES_DIR}/livetune)
endif() #SUPPORT_LIVE_TUNING

# ia_cipr is only used by psysprocessor
if (USE_PG_LITE_PIPE)
set(IA_CIPR_DIR         ${MODULES_DIR}/ia_cipr)
endif() #USE_PG_LITE_PIPE

# CMake module directory
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Install directories
# CMAKE_INSTALL_PREFIX is defined when executing cmake and supposed to be ./out/libcamhal
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}" CACHE PATH "Target location for install command." FORCE)
endif()

message(STATUS "install prefix: " ${CMAKE_INSTALL_PREFIX})

#------------------------- Compilation settings -------------------------

# -fPIC in xos way
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-faligned-new COMPILER_SUPPORTS_ALIGNED_NEW)
if (COMPILER_SUPPORTS_ALIGNED_NEW)
    add_definitions(-faligned-new)
endif()

add_compile_options(-std=c++11
                    -Wall -Werror
                    -fstack-protector
                    -fPIE -fPIC
                    -D_FORTIFY_SOURCE=2
                    -DDCHECK_ALWAYS_ON
                    -Wformat -Wformat-security
                    )
# code coverage.
include(code-coverage)

add_definitions(-D__STDC_FORMAT_MACROS
                -DHAVE_PTHREADS
                -DHAVE_LINUX_OS
                -DHAVE_IA_TYPES
                -DHAVE_PRCTL
                )

if (USE_PG_LITE_PIPE)
        add_definitions(-DUSE_PG_LITE_PIPE)
endif() #USE_PG_LITE_PIPE

if (CAL_BUILD)
    message("Chrome enabled ")
    add_definitions(-DCAL_BUILD)
    add_definitions(-DDUMP_IMAGE)
else()
    add_definitions(-DSUPPORT_MULTI_PROCESS)
    add_definitions(-DLINUX_BUILD)
endif() #CAL_BUILD

if (FACE_DETECTION)
    add_definitions(-DFACE_DETECTION)
endif() #FACE_DETECTION

# IPU6_FEATURE_S
if (IPU_VER MATCHES "ipu6")
    add_definitions(-DIPU_SYSVER_IPU6)
endif()
# IPU6_FEATURE_E

# IPU6_SE_FEATURE_S
if (IPU_VER MATCHES "ipu6sepla")
    add_definitions(-DIPU_SYSVER_ipu6v3)
endif()
# IPU6_SE_FEATURE_E

include_directories(include
                    include/api include/utils
                    ${CMAKE_CURRENT_LIST_DIR}
                    src
                    src/v4l2 src/core src/metadata
                    src/platformdata src/platformdata/gc
                    src/3a src/3a/intel3a src/3a/external
                    src/fd
                    src/core/psysprocessor
                    src/image_process
                    )

set(LIBCAMHAL_LD_FLAGS "-fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-z,relro -Wl,-z,now")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LIBCAMHAL_LD_FLAGS}")

add_subdirectory(src)
add_subdirectory(modules)

# Set source files
if (CAL_BUILD)
    if (SW_JPEG_ENCODE)
        add_definitions(-DSW_JPEG_ENCODE)
        include_directories(src/jpeg/sw)
    else ()
        include_directories(src/jpeg/chrome)
    endif()

    if (SW_POST_PROCESSING)
        include_directories(src/image_process/sw)
    else ()
        include_directories(src/image_process/chrome)
    endif()

    add_subdirectory(aal)

    set(LIBCAMHAL_SRCS
        ${LIBCAMHAL_SRCS}
        ${MEMORY_SRCS}
        ${AAL_SRCS}
        ${JPEG_SRCS}
        )
    include_directories(include include/api include/utils include/linux
                        ${USR_INCLUDE_HEADER}/android/system/core/include/
                        ${USR_INCLUDE_HEADER}/ia_imaging
                        ${USR_INCLUDE_HEADER}/lib${IPU_VER}
                        ${USR_INCLUDE_HEADER}/android/hardware/libhardware/include/
                        aal aal/chrome src/iutils src/jpeg
                        modules/memory modules/memory/chrome
                        )
endif() #CAL_BUILD

if (SUPPORT_LIVE_TUNING)
    set(LIBCAMHAL_SRCS ${LIVE_TUNING_SRCS})
    include_directories(modules/livetune)
endif() #SUPPORT_LIVE_TUNING

if (USE_PG_LITE_PIPE)
include_directories(modules/ia_css/${IPU_VER}/include)
endif() #USE_PG_LITE_PIPE

set(LIBCAMHAL_SRCS
    ${LIBCAMHAL_SRCS}
    ${3A_SRCS}
    ${CORE_SRCS}
    ${HAL_SRCS}
    ${IPS_CTRL_SRCS}
    ${IUTILS_SRCS}
    ${METADATA_SRCS}
    ${PLATFORMDATA_SRCS}
    ${V4L2_SRCS}
    ${ALGOWRAPPER_SRCS}
    ${IMAGE_PROCESS_SRCS}
    ${MODULES_DIR}/ia_cipr/src/Buffer.cpp
    ${MODULES_DIR}/ia_cipr/src/Context.cpp
    ${MODULES_DIR}/ia_cipr/src/Event.cpp
    ${MODULES_DIR}/ia_cipr/src/Command.cpp
    ${MODULES_DIR}/ia_cipr/src/Utils.cpp
    )

if(FACE_DETECTION)
set(LIBCAMHAL_SRCS
    ${LIBCAMHAL_SRCS}
    ${FACE_DETECTION_SRCS}
    )
endif() #FACE_DETECTION

if(ENABLE_SANDBOXING)
set(LIBCAMHAL_SRCS
    ${LIBCAMHAL_SRCS}
    ${SANDBOXING_CLIENT_SRCS}
    )
endif() #ENABLE_SANDBOXING

    set(LIBCAMHAL_SRCS ${LIBCAMHAL_SRCS} ${IIO_SRCS})

# Add libcamhal using the specified sources
add_library(camhal SHARED ${LIBCAMHAL_SRCS})
add_library(camhal_static STATIC ${LIBCAMHAL_SRCS})
set_target_properties(camhal_static PROPERTIES OUTPUT_NAME "camhal")

#---------------------------- Link settings ----------------------------
target_link_libraries(camhal ${CMAKE_DL_LIBS})
target_link_libraries(camhal_static ${CMAKE_DL_LIBS})

target_link_libraries(camhal expat rt)
target_link_libraries(camhal_static expat rt)

if (CAL_BUILD)
    target_link_libraries(camhal camera_v4l2_device)
    target_link_libraries(camhal_static camera_v4l2_device)
endif()

# Link libraries, the building will be interrupted if libs/includes not found

    # Link ia_imaging
    find_package(IA_IMAGING)
    include_directories(${IA_IMAGING_INCLUDE_DIRS})
    target_link_libraries(camhal ${IA_IMAGING_LIBS})
    target_link_libraries(camhal_static ${IA_IMAGING_LIBS})

    if (IPU_VER MATCHES "ipu7")
        # Link libgcss
        find_package(LIBGCSS)
        include_directories(${LIBGCSS_INCLUDE_DIRS})
        target_link_libraries(camhal ${LIBGCSS_LIBS})
        target_link_libraries(camhal_static ${LIBGCSS_LIBS})
    else()
        # Link libiacss
        find_package(LIBIACSS)
        include_directories(${LIBIACSS_INCLUDE_DIRS})
        target_link_libraries(camhal ${LIBIACSS_LIBS})
        target_link_libraries(camhal_static ${LIBIACSS_LIBS})
    endif()

    # Include libipu(4) headers
    find_package(LIBIPU)
    include_directories(${LIBIPU_INCLUDE_DIRS})

if (CAL_BUILD)
    find_package(LIBCAMERA_CLIENT)
    find_package(LIBCAMERA_COMMON)
    find_package(LIBCAMERA_METADATA)
    find_package(LIBCAMERA_V4L2_DEVICE)
    find_package(LIBSYNC)
    find_package(LIBCBM)

    include_directories(${LIBCAMERA_CLIENT_INCLUDE_DIRS} ${LIBCAMERA_COMMON_INCLUDE_DIRS}
                        ${LIBCAMERA_METADATA_INCLUDE_DIRS} ${LIBCAMERA_V4L2_DEVICE_INCLUDE_DIRS}
                        ${LIBSYNC_INCLUDE_DIRS}
                        ${LIBCBM_INCLUDE_DIRS}
                        )
    link_directories(${CMAKE_PREFIX_PATH} ${LIBCAMERA_CLIENT_LIBS} ${LIBCAMERA_METADATA_LIBS}
                     ${LIBCAMERA_COMMON_LIBS} ${LIBSYNC_LIBS}
                     ${LIBCBM_LIBS}
                     )

    target_link_libraries(camhal ${LIBCAMERA_CLIENT_LIBS} ${LIBCAMERA_COMMON_LIBS}
                          ${LIBCAMERA_METADATA_LIBS} ${LIBCAMERA_V4L2_DEVICE_LIBS}
                          ${LIBSYNC_LIBS} jpeg ${CMAKE_PREFIX_PATH}/libyuv.pic.a
                          ${LIBCBM_LIBS}
                          )
    target_link_libraries(camhal_static ${LIBCAMERA_CLIENT_LIBS} ${LIBCAMERA_COMMON_LIBS}
                          ${LIBCAMERA_METADATA_LIBS} ${LIBCAMERA_V4L2_DEVICE_LIBS}
                          ${LIBSYNC_LIBS} jpeg ${CMAKE_PREFIX_PATH}/libyuv.pic.a
                          ${LIBCBM_LIBS}
                          )
endif() #CAL_BUILD

if (NOT CAL_BUILD)
    include_directories(${MODULES_DIR}/v4l2)
endif()

if (FACE_DETECTION)
    target_link_libraries(camhal pvl_eye_detection pvl_face_detection pvl_mouth_detection)
    target_link_libraries(camhal_static pvl_eye_detection pvl_face_detection pvl_mouth_detection)
    target_link_libraries(camhal ${WORK_DIR_TMP}/src/fd/libface_detection.a)
    target_link_libraries(camhal_static ${WORK_DIR_TMP}/src/fd/libface_detection.a)
endif()

if (ENABLE_SANDBOXING)
    add_definitions(-DENABLE_SANDBOXING)

    find_package(LIBMOJO)
    find_package(LIBCAMERA_IPC)
    find_package(LIBCAB)

    include_directories(${USR_INCLUDE_HEADER}/base-${BASEVER})
    include_directories(${LIBMOJO_INCLUDE_DIRS})
    include_directories(${LIBCAMERA_IPC_INCLUDE_DIRS})
    include_directories(${LIBCAB_INCLUDE_DIRS})

    link_directories(${CMAKE_PREFIX_PATH}
                     ${LIBMOJO_LIBS}
                     ${LIBCAMERA_IPC_LIBS}
                     ${LIBCAB_LIBS}
                     )

    target_link_libraries(camhal ${LIBCAB_LIBS})
    target_link_libraries(camhal ${LIBMOJO_LIBS})
    target_link_libraries(camhal ${LIBCAMERA_IPC_LIBS})
    target_link_libraries(camhal ${CMAKE_PREFIX_PATH}/librt.a)

    target_link_libraries(camhal_static ${LIBCAB_LIBS})
    target_link_libraries(camhal_static ${LIBMOJO_LIBS})
    target_link_libraries(camhal_static ${LIBCAMERA_IPC_LIBS})
    target_link_libraries(camhal_static ${CMAKE_PREFIX_PATH}/librt.a)
endif() #ENABLE_SANDBOXING

#--------------------------- Install settings ---------------------------
if (NOT CAL_BUILD)
# Install headers
install(DIRECTORY include/ DESTINATION include/libcamhal)
if (SUPPORT_LIVE_TUNING)
    install(FILES modules/livetune/LiveTuning.h DESTINATION include/libcamhal/api)
endif() #SUPPORT_LIVE_TUNING

# Install configure files
# By default ipu4 is used
if("${IPU_VER}" STREQUAL "ipu6")
    message("Install IPU6 configure files")
    install(DIRECTORY config/linux/ipu6/ DESTINATION share/defaults/etc/camera)
elseif("${IPU_VER}" STREQUAL "ipu7")
    message("Install IPU7 configure files")
    install(DIRECTORY config/linux/ipu7/ DESTINATION share/defaults/etc/camera)
else()
    message("Install IPU4 configure files")
    install(DIRECTORY config/linux/ipu4/ DESTINATION share/defaults/etc/camera)
endif()

# Install libraries
install(TARGETS camhal camhal_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

# Install package config file
configure_file(${PROJECT_SOURCE_DIR}/cmake/libcamhal.pc.cmakein
               ${PROJECT_SOURCE_DIR}/libcamhal.pc @ONLY)
install(FILES libcamhal.pc DESTINATION lib/pkgconfig)

endif() #NOT CAL_BUILD

set(CPACK_GENERATOR "RPM")
include(CPack)
