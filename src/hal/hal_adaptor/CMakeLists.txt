#
#  Copyright (C) 2023-2024 Intel Corporation
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

# Source directories
set(ROOT_DIR        ${CMAKE_CURRENT_LIST_DIR}/../../../)
set(METADATA_DIR    ${ROOT_DIR}/src/metadata)
set(IUTILS_DIR      ${ROOT_DIR}/src/iutils)
set(HAL_ADAPTOR_DIR ${ROOT_DIR}/src/hal/hal_adaptor)

set(HAL_ADAPTOR_INCLUDEDIRS
    ${ROOT_DIR}/include
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
target_include_directories(hal_adaptor PRIVATE ${HAL_ADAPTOR_INCLUDEDIRS})
target_compile_definitions(hal_adaptor PRIVATE
                           -D__STDC_FORMAT_MACROS
                           -DHAVE_PTHREADS
                           -DHAVE_LINUX_OS
                           "-DCAMHAL_PLUGIN_DIR=\"${CMAKE_INSTALL_FULL_LIBDIR}/libcamhal/plugins/\""
                           )
set_target_properties(hal_adaptor PROPERTIES OUTPUT_NAME camhal)
set_target_properties(hal_adaptor PROPERTIES SOVERSION 0)
set_target_properties(hal_adaptor PROPERTIES VERSION "0.0.0")

#---------------------------- Link settings ----------------------------
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)
target_link_libraries(hal_adaptor ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(hal_adaptor ${CMAKE_DL_LIBS})
target_link_libraries(hal_adaptor rt)

# Install headers
install(DIRECTORY ${ROOT_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libcamhal)
if (SUPPORT_LIVE_TUNING)
    install(FILES ${ROOT_DIR}/modules/livetune/LiveTuning.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libcamhal/api)
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
configure_file(${PROJECT_SOURCE_DIR}/cmake/libcamhal.pc.cmakein
               ${PROJECT_SOURCE_DIR}/libcamhal.pc @ONLY)
install(FILES ${PROJECT_SOURCE_DIR}/libcamhal.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
