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

if (SW_JPEG_ENCODE)
    set(JPEG_SRCS
        ${JPEG_DIR}/sw/SWJpegEncoder.cpp
        CACHE INTERNAL "jpeg sources"
       )
else ()
    set(JPEG_SRCS
        ${JPEG_DIR}/chrome/JpegEncoderCore.cpp
        CACHE INTERNAL "jpeg sources"
        )
endif()

set(JPEG_SRCS
    ${JPEG_SRCS}
    ${JPEG_DIR}/ExifCreater.cpp
    ${JPEG_DIR}/EXIFMetaData.cpp
    ${JPEG_DIR}/EXIFMaker.cpp
    ${JPEG_DIR}/JpegMaker.cpp
    CACHE INTERNAL "jpeg sources"
    )
