#
#  Copyright (C) 2019-2020 Intel Corporation
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
#

# Use -DFACE_DETECTION to enable the face detection feature
if (NOT ENABLE_SANDBOXING)
    set(FACE_DETECTION_SRCS
        ${SRC_ROOT_DIR}/fd/FaceDetection.cpp
        ${ALGOWRAPPER_DIR}/IntelFaceDetection.cpp
        CACHE INTERNAL "Face detect sources"
        )
#---------------------------- include the header ----------------------------
    include_directories(${USR_INCLUDE_HEADER}/ia_imaging
                        ${ALGOWRAPPER_DIR}/IntelFaceDetection.h
                        ${IUTILS_DIR}
                        include/utils)
#---------------------------- static library ----------------------------
    add_library(face_detection STATIC ${FACE_DETECTION_SRCS})
else ()
    set(FACE_DETECTION_SRCS
        ${SRC_ROOT_DIR}/fd/FaceDetection.cpp
        CACHE INTERNAL "Face detect sources"
        )
endif()

if (CAL_BUILD)
    find_package(LIBCAMERA_V4L2_DEVICE)
    include_directories(${LIBCAMERA_V42L_DEVICE_INCLUDE_DIRS})
else()
    include_directories(${MODULES_DIR}/v4l2)
endif()
