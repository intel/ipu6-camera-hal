#
#  Copyright (C) 2019 Intel Corporation
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

project(libcam_algo)

include_directories(${USR_INCLUDE_HEADER}/ia_imaging)
include_directories(${IUTILS_DIR})

set(LIBCAM_ALGO_SRCS
    ${IUTILS_DIR}/Utils.cpp
    ${IUTILS_DIR}/Trace.cpp
    ${IUTILS_DIR}/ScopedAtrace.cpp
    ${IUTILS_DIR}/Thread.cpp
    ${IUTILS_DIR}/CameraLog.cpp
    ${PLATFORMDATA_DIR}/gc/FormatUtils.cpp
    ${PLATFORMDATA_DIR}/gc/GraphUtils.cpp
    ${SANDBOXING_DIR}/IPCCommon.cpp
    ${SANDBOXING_DIR}/IPCIntelLard.cpp
    ${SANDBOXING_DIR}/IPCIntelFD.cpp
    ${SANDBOXING_DIR}/server/IntelFDServer.cpp
    ${SANDBOXING_DIR}/IPCGraphConfig.cpp
    ${SANDBOXING_DIR}/server/IntelAlgoServer.cpp
    ${SANDBOXING_DIR}/server/IntelLardServer.cpp
    ${SANDBOXING_DIR}/server/GraphConfigServer.cpp
    ${MODULES_DIR}/algowrapper/IntelLard.cpp
    ${MODULES_DIR}/algowrapper/IntelFaceDetection.cpp
    ${MODULES_DIR}/algowrapper/graph/GraphConfigImpl.cpp
    ${MODULES_DIR}/algowrapper/graph/GraphConfigPipe.cpp
    CACHE INTERNAL "libcam_algo sources"
    )

add_library(libcam_algo SHARED ${LIBCAM_ALGO_SRCS})

find_package(LIBCAB)
find_package(LIBMOJO)
find_package(IA_IMAGING)
find_package(LIBIACSS)

include_directories(${IA_IMAGING_INCLUDE_DIRS})
include_directories(${LIBCAB_INCLUDE_DIRS})
include_directories(${LIBMOJO_INCLUDE_DIRS})
include_directories(${SRC_ROOT_DIR}/fd/FaceBase.h)
include_directories(${LIBIACSS_INCLUDE_DIRS})

link_directories(${CMAKE_PREFIX_PATH}
                 ${LIBMOJO_LIBS}
                 ${LIBCAB_LIBS}
                 )

target_link_libraries(libcam_algo ${LIBCAB_LIBS})
target_link_libraries(libcam_algo ${LIBMOJO_LIBS})
target_link_libraries(libcam_algo ${CMAKE_PREFIX_PATH}/libia_lard.so)
target_link_libraries(libcam_algo ${IA_IMAGING_LIBS})
target_link_libraries(libcam_algo ${LIBIACSS_LIBS})

if (FACE_DETECTION)
    target_link_libraries(libcam_algo pvl_eye_detection pvl_face_detection pvl_mouth_detection)
endif()

install(TARGETS libcam_algo LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
set_target_properties(libcam_algo PROPERTIES OUTPUT_NAME "cam_algo")
