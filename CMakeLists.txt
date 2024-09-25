#
#  Copyright (C) 2017-2024 Intel Corporation
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

option(BUILD_CAMHAL_PLUGIN "Build libcamhal as plugins" OFF)
option(BUILD_CAMHAL_ADAPTOR "Build hal_adaptor as libcamhal" OFF)

#------------------------- Global settings -------------------------

#------------------------- folder settings -------------------------

# Source directories
set(SRC_ROOT_DIR        ${CMAKE_CURRENT_LIST_DIR}/src)

set(3A_DIR              ${SRC_ROOT_DIR}/3a)
set(CORE_DIR            ${SRC_ROOT_DIR}/core)
set(HAL_DIR             ${SRC_ROOT_DIR}/hal)
set(IIO_DIR             ${SRC_ROOT_DIR}/iio)
set(IUTILS_DIR          ${SRC_ROOT_DIR}/iutils)
set(SCHEDULER_DIR       ${SRC_ROOT_DIR}/scheduler)
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
    set(MODULE_IA_CSS_DIR   ${MODULES_DIR}/ia_css)
endif() #USE_PG_LITE_PIPE

# CMake module directory
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Install directories
# CMAKE_INSTALL_PREFIX is defined when executing cmake and supposed to be ./out/libcamhal
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}" CACHE PATH "Target location for install command." FORCE)
endif()

#------------------------- compile and link options -------------------------

include(GNUInstallDirs)
# code coverage.
include(code-coverage)

message(STATUS "install prefix: " ${CMAKE_INSTALL_PREFIX})

set (CMAKE_CXX_STANDARD 11)
add_compile_options(
    -Wall
    -Werror
    -fstack-protector
    -fPIE
    -fPIC
    -DDCHECK_ALWAYS_ON
    -Wformat
    -Wformat-security
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-faligned-new COMPILER_SUPPORTS_ALIGNED_NEW)
if (COMPILER_SUPPORTS_ALIGNED_NEW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
endif()

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

# check if _FORTIFY_SOURCE is default defined with -O
include(CheckFortifyLevel)
set(FORTIFY_SOURCE_VALUE)
check_fortify_source(FORTIFY_SOURCE_VALUE)
if(FORTIFY_SOURCE_VALUE)
    message(STATUS "compiler default _FORTIFY_SOURCE=${FORTIFY_SOURCE_VALUE}")
else()
    message(STATUS "_FORTIFY_SOURCE is not defined or could not be determined.")
    add_compile_options(-D_FORTIFY_SOURCE=2)
    message(STATUS "define _FORTIFY_SOURCE=2")
endif()

add_link_options(
    -fPIE
    -fPIC
    -Wformat
    -Wformat-security
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,--no-as-needed
    -Wl,--no-undefined
)

# -fPIC in xos way
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#------------------- common definitions, includes and link --------------------

add_definitions(-D__STDC_FORMAT_MACROS -DHAVE_PTHREADS -DHAVE_LINUX_OS -DHAVE_IA_TYPES -DHAVE_PRCTL)

include_directories(
    include
    include/api include/utils
    ${CMAKE_INSTALL_PREFIX}/include/ia_camera
    ${CMAKE_INSTALL_PREFIX}/include/
    ${CMAKE_CURRENT_LIST_DIR}
    src
    src/v4l2 src/core src/metadata
    src/platformdata src/platformdata/gc
    src/3a src/3a/intel3a src/3a/external
    src/fd src/scheduler
    src/core/psysprocessor
    src/image_process
)

set(LIBCAMHAL_LINK_LIBS ${LIBCAMHAL_LINK_LIBS} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} expat rt)

add_subdirectory(src)
add_subdirectory(modules)

if (USE_PG_LITE_PIPE)
    add_definitions(-DUSE_PG_LITE_PIPE)
endif() #USE_PG_LITE_PIPE

    set(LIBCAMHAL_SRCS ${LIBCAMHAL_SRCS} ${IIO_SRCS})

if (SUPPORT_LIVE_TUNING)
    include_directories(modules/livetune)
    set(LIBCAMHAL_SRCS ${LIBCAMHAL_SRCS} ${LIVE_TUNING_SRCS})
endif() #SUPPORT_LIVE_TUNING

if(FACE_DETECTION)
    add_definitions(-DFACE_DETECTION)
    set(LIBCAMHAL_SRCS ${LIBCAMHAL_SRCS} ${FACE_DETECTION_SRCS})
    set(LIBCAMHAL_LINK_LIBS
        ${LIBCAMHAL_LINK_LIBS}
        pvl_eye_detection
        pvl_face_detection
        pvl_mouth_detection
        ${WORK_DIR_TMP}/src/fd/libface_detection.a
    )
endif() #FACE_DETECTION

if(ENABLE_SANDBOXING)
    find_package(LIBMOJO)
    find_package(LIBCAMERA_IPC)
    find_package(LIBCAB)
    add_definitions(-DENABLE_SANDBOXING)
    set(LIBCAMHAL_SRCS ${LIBCAMHAL_SRCS} ${SANDBOXING_CLIENT_SRCS})
    include_directories(
        ${USR_INCLUDE_HEADER}/base-${BASEVER}
        ${LIBMOJO_INCLUDE_DIRS}
        ${LIBCAMERA_IPC_INCLUDE_DIRS}
        ${LIBCAB_INCLUDE_DIRS}
    )
    link_directories(
        ${CMAKE_PREFIX_PATH}
        ${LIBMOJO_LIBS}
        ${LIBCAMERA_IPC_LIBS}
        ${LIBCAB_LIBS}
    )
    set(LIBCAMHAL_LINK_LIBS ${LIBCAMHAL_LINK_LIBS} ${LIBCAB_LIBS} ${LIBMOJO_LIBS} ${LIBCAMERA_IPC_LIBS} ${CMAKE_PREFIX_PATH}/librt.a)
endif() #ENABLE_SANDBOXING

if (CAL_BUILD)
    message("Chrome enabled ")
    add_definitions(-DCAL_BUILD -DDUMP_IMAGE)
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

    set(LIBCAMHAL_SRCS ${LIBCAMHAL_SRCS} ${MEMORY_SRCS} ${AAL_SRCS} ${JPEG_SRCS})
    include_directories(include include/api include/utils include/linux
        ${USR_INCLUDE_HEADER}/android/system/core/include/
        ${USR_INCLUDE_HEADER}/ia_imaging
        ${USR_INCLUDE_HEADER}/usr/lib${IPU_VER}
        ${USR_INCLUDE_HEADER}/android/hardware/libhardware/include/
        aal aal/chrome src/iutils src/jpeg
        modules/memory modules/memory/chrome
    )
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

    set(LIBCAMHAL_LINK_LIBS ${LIBCAMHAL_LINK_LIBS} ${LIBCAMERA_CLIENT_LIBS} ${LIBCAMERA_COMMON_LIBS}
                          ${LIBCAMERA_METADATA_LIBS} ${LIBCAMERA_V4L2_DEVICE_LIBS}
                          ${LIBSYNC_LIBS} jpeg ${CMAKE_PREFIX_PATH}/libyuv.pic.a
                          ${LIBCBM_LIBS} camera_v4l2_device
                          )
endif() #CAL_BUILD

if (NOT CAL_BUILD)
    message("Linux build")
    add_definitions(-DSUPPORT_MULTI_PROCESS)
    add_definitions(-DLINUX_BUILD)
    include_directories(${MODULES_DIR}/v4l2)
    find_package(LIBDRM)
    if(LIBDRM_FOUND AND ("${LIBDRM_VERSION}" VERSION_GREATER_EQUAL 2.4.114))
        include_directories(${LIBDRM_INCLUDE_DIRS})
        set(LIBCAMHAL_LINK_LIBS ${LIBCAMHAL_LINK_LIBS} ${LIBDRM_LIBS})
        add_definitions(-DLIBDRM_SUPPORT_MMAP_OFFSET)
    endif()
endif()

set(LIBCAMHAL_SRCS
    ${LIBCAMHAL_SRCS}
    ${3A_SRCS}
    ${CORE_SRCS}
    ${HAL_SRCS}
    ${IPS_CTRL_SRCS}
    ${IUTILS_SRCS}
    ${METADATA_SRCS}
    ${PLATFORMDATA_SRCS}
    ${SCHEDULER_SRCS}
    ${V4L2_SRCS}
    ${ALGOWRAPPER_SRCS}
    ${IMAGE_PROCESS_SRCS}
    ${MODULES_DIR}/ia_cipr/src/Buffer.cpp
    ${MODULES_DIR}/ia_cipr/src/Context.cpp
    ${MODULES_DIR}/ia_cipr/src/Event.cpp
    ${MODULES_DIR}/ia_cipr/src/Command.cpp
    ${MODULES_DIR}/ia_cipr/src/Utils.cpp
)

#------------------------- Target settings -------------------------

if (NOT IPU_VERSIONS)
    set(IPU_VERSIONS ${IPU_VER})
endif()

if (BUILD_CAMHAL_ADAPTOR)
    add_subdirectory(src/hal/hal_adaptor)
    if (NOT BUILD_CAMHAL_PLUGIN)
        message(NOTICE "Will not build libcamhal plugins")
        set(IPU_VERSIONS "")
    endif()
endif()

# Add different targets according to array IPU_VERSIONS
foreach(IPU_VER ${IPU_VERSIONS})

if (BUILD_CAMHAL_PLUGIN)
    set(TARGET_SUFFIX "-${IPU_VER}")
endif()

set(TARGET_INCLUDE "")
set(TARGET_DEFINITIONS "")
set(TARGET_SRCS "")
set(TARGET_LINK_LIBS "")

if (USE_PG_LITE_PIPE)
    add_subdirectory(modules/ia_css modules/ia_css${TARGET_SUFFIX})
    set(TARGET_INCLUDE ${TARGET_INCLUDE} ${MODULE_IA_CSS_HEADER_INCLUDE_DIR})
    set(TARGET_SRCS ${TARGET_SRCS} ${MODULE_IA_CSS_SRC})
endif() #USE_PG_LITE_PIPE

if (IPU_VER MATCHES "ipu6")
    message(STATUS "add definition -DIPU_SYSVER_IPU6 for ${IPU_VER}")
    set(TARGET_DEFINITIONS ${TARGET_DEFINITIONS} -DIPU_SYSVER_IPU6)
    if (IPU_VER STREQUAL "ipu6sepla")
        message(STATUS "add definition -DIPU_SYSVER_ipu6v3 for ${IPU_VER}")
        set(TARGET_DEFINITIONS ${TARGET_DEFINITIONS} -DIPU_SYSVER_ipu6v3)
    elseif (IPU_VER STREQUAL "ipu6ep")
        message(STATUS "add definition -DIPU_SYSVER_ipu6v5 for ${IPU_VER}")
        set(TARGET_DEFINITIONS ${TARGET_DEFINITIONS} -DIPU_SYSVER_ipu6v5)
    elseif (IPU_VER STREQUAL "ipu6epmtl")
        message(STATUS "add definition -DIPU_SYSVER_ipu6v6 for ${IPU_VER}")
        set(TARGET_DEFINITIONS ${TARGET_DEFINITIONS} -DIPU_SYSVER_ipu6v6)
    endif()
endif()

if (NOT CAL_BUILD)
    set(TARGET_DEFINITIONS ${TARGET_DEFINITIONS} "-DCAMERA_DEFAULT_CFG_PATH=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}/camera/${IPU_VER}/\"")
endif()

# Link libraries, the building will be interrupted if libs/includes not found
    # Link ia_imaging
    find_package(IA_IMAGING)
    set(TARGET_INCLUDE ${TARGET_INCLUDE} ${IA_IMAGING${TARGET_SUFFIX}_INCLUDE_DIRS})
    set(TARGET_LINK_LIBS ${TARGET_LINK_LIBS} ${IA_IMAGING${TARGET_SUFFIX}_LIBS})

    if(USE_PG_LITE_PIPE)
        find_package(LIBGCSS)
        set(TARGET_INCLUDE ${TARGET_INCLUDE} ${LIBGCSS${TARGET_SUFFIX}_INCLUDE_DIRS})
        set(TARGET_LINK_LIBS ${TARGET_LINK_LIBS} ${LIBGCSS${TARGET_SUFFIX}_LIBS})
        # Include libipu(4) headers
        find_package(LIBIPU)
        set(TARGET_INCLUDE ${TARGET_INCLUDE} ${LIBIPU${TARGET_SUFFIX}_INCLUDE_DIRS})
        set(TARGET_LINK_LIBS ${TARGET_LINK_LIBS} ${LIBIPU${TARGET_SUFFIX}_LIBS})
    else()
        # Link libiacss
        find_package(LIBIACSS)
        set(TARGET_INCLUDE ${TARGET_INCLUDE} ${LIBIACSS${TARGET_SUFFIX}_INCLUDE_DIRS})
        set(TARGET_LINK_LIBS ${TARGET_LINK_LIBS} ${LIBIACSS${TARGET_SUFFIX}_LIBS})
    endif()

#--------------------------- Add libcamhal target  ---------------------------

if (BUILD_CAMHAL_PLUGIN)
    set(CAMHAL_TARGET ${IPU_VER})
    set(CAMHAL_STATIC_TARGET ${IPU_VER}_static)
else()
    set(CAMHAL_TARGET camhal)
    set(CAMHAL_STATIC_TARGET camhal_static)
endif()

# Add libcamhal using the specified sources
add_library(${CAMHAL_STATIC_TARGET} STATIC ${LIBCAMHAL_SRCS} ${TARGET_SRCS})

if (${CMAKE_VERSION} VERSION_LESS 3.11)
    add_library(${CAMHAL_TARGET} SHARED ${LIBCAMHAL_SRCS} ${TARGET_SRCS})
else()
    add_library(${CAMHAL_TARGET} SHARED $<TARGET_OBJECTS:${CAMHAL_STATIC_TARGET}>)
endif()

target_include_directories(${CAMHAL_STATIC_TARGET} PRIVATE ${TARGET_INCLUDE})
target_include_directories(${CAMHAL_TARGET} PRIVATE ${TARGET_INCLUDE})
target_compile_definitions(${CAMHAL_STATIC_TARGET} PRIVATE ${TARGET_DEFINITIONS})
target_compile_definitions(${CAMHAL_TARGET} PRIVATE ${TARGET_DEFINITIONS})
target_link_libraries(${CAMHAL_STATIC_TARGET} PRIVATE ${LIBCAMHAL_LINK_LIBS} ${TARGET_LINK_LIBS})
target_link_libraries(${CAMHAL_TARGET}  PRIVATE ${LIBCAMHAL_LINK_LIBS} ${TARGET_LINK_LIBS})

if (BUILD_CAMHAL_PLUGIN)
    set_target_properties(${CAMHAL_STATIC_TARGET} PROPERTIES PREFIX "")
    set_target_properties(${CAMHAL_TARGET} PROPERTIES PREFIX "")
    set_target_properties(${CAMHAL_STATIC_TARGET} PROPERTIES OUTPUT_NAME ${IPU_VER})
    set_target_properties(${CAMHAL_TARGET} PROPERTIES OUTPUT_NAME ${IPU_VER})
else()
    set_target_properties(${CAMHAL_STATIC_TARGET} PROPERTIES OUTPUT_NAME ${CAMHAL_TARGET})
    set_target_properties(${CAMHAL_TARGET} PROPERTIES SOVERSION 0)
    set_target_properties(${CAMHAL_TARGET} PROPERTIES VERSION "0.0.0")
    set_target_properties(${CAMHAL_TARGET} PROPERTIES OUTPUT_NAME ${CAMHAL_TARGET})
endif()

#--------------------------- Install settings ---------------------------
if (NOT CAL_BUILD)
    if (NOT BUILD_CAMHAL_PLUGIN)
        # Install headers
        install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libcamhal)
        if (SUPPORT_LIVE_TUNING)
            install(FILES modules/livetune/LiveTuning.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libcamhal/api)
        endif() #SUPPORT_LIVE_TUNING
    endif()

    # Install configure files, by default ipu4 is used
    message("Install camera configure files for ${IPU_VER}")
    install(DIRECTORY config/linux/${IPU_VER}/ DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/camera/${IPU_VER})

    # Install libraries
    set(CAMHAL_TARGETS ${CAMHAL_TARGET} ${CAMHAL_STATIC_TARGET})
    if (BUILD_CAMHAL_PLUGIN)
        set(CAMHAL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/libcamhal/plugins)
    else()
        set(CAMHAL_DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
    if (${CMAKE_VERSION} VERSION_LESS 3.11)
        install(TARGETS ${CAMHAL_TARGETS}
                LIBRARY DESTINATION ${CAMHAL_DESTINATION}
                ARCHIVE DESTINATION ${CAMHAL_DESTINATION})
    else()
        install(TARGETS ${CAMHAL_TARGETS} DESTINATION ${CAMHAL_DESTINATION})
    endif()
    if (NOT BUILD_CAMHAL_PLUGIN)
        # Install package config file
        configure_file(${PROJECT_SOURCE_DIR}/cmake/libcamhal.pc.cmakein
                       ${PROJECT_SOURCE_DIR}/libcamhal.pc @ONLY)
        install(FILES libcamhal.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    endif()

endif() #NOT CAL_BUILD

endforeach() #IPU_VERSIONS

set(CPACK_GENERATOR "RPM")
include(CPack)
