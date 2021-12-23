#
#  Copyright (C) 2019-2021 Intel Corporation
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

set(SANDBOXING_CLIENT_SRCS
    ${SANDBOXING_DIR}/client/IntelAlgoClient.cpp
    ${SANDBOXING_DIR}/client/IntelAlgoCommonClient.cpp
    ${SANDBOXING_DIR}/client/IntelLard.cpp
    ${SANDBOXING_DIR}/client/IntelFaceDetectionClient.cpp
    ${SANDBOXING_DIR}/client/GraphConfigImplClient.cpp
    ${SANDBOXING_DIR}/IPCCommon.cpp
    ${SANDBOXING_DIR}/IPCIntelLard.cpp
    ${SANDBOXING_DIR}/IPCIntelFD.cpp
    ${SANDBOXING_DIR}/IPCGraphConfig.cpp
    CACHE INTERNAL "sandboxing sources"
    )
