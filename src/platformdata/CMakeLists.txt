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

set(PLATFORMDATA_SRCS
    ${PLATFORMDATA_DIR}/ParserBase.cpp
    ${PLATFORMDATA_DIR}/PlatformData.cpp
    ${PLATFORMDATA_DIR}/CameraParser.cpp
    ${PLATFORMDATA_DIR}/PolicyParser.cpp
    CACHE INTERNAL "platformdata sources"
   )

    set(PLATFORMDATA_SRCS
        ${PLATFORMDATA_SRCS}
        ${PLATFORMDATA_DIR}/AiqInitData.cpp
        ${PLATFORMDATA_DIR}/gc/FormatUtils.cpp
        ${PLATFORMDATA_DIR}/gc/GraphUtils.cpp
        CACHE INTERNAL "platformdata sources"
        )
        set(PLATFORMDATA_SRCS
            ${PLATFORMDATA_SRCS}
            ${PLATFORMDATA_DIR}/gc/GraphConfigManager.cpp
            ${PLATFORMDATA_DIR}/gc/GraphConfig.cpp
            CACHE INTERNAL "platformdata sources"
        )

