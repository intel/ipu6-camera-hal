#
#  Copyright (C) 2017-2023 Intel Corporation
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

set(CORE_SRCS
    ${CORE_DIR}/CaptureUnit.cpp
    ${CORE_DIR}/DeviceBase.cpp
    ${CORE_DIR}/CameraStream.cpp
    ${CORE_DIR}/CameraDevice.cpp
    ${CORE_DIR}/ProcessorManager.cpp
    ${CORE_DIR}/RequestThread.cpp
    ${CORE_DIR}/SwImageProcessor.cpp
    ${CORE_DIR}/BufferQueue.cpp
    ${CORE_DIR}/CameraBuffer.cpp
    ${CORE_DIR}/CameraEvent.cpp
    ${CORE_DIR}/LensHw.cpp
    ${CORE_DIR}/SensorHwCtrl.cpp
    ${CORE_DIR}/SofSource.cpp
    CACHE INTERNAL "core sources"
    )

# FRAME_SYNC_S
    set(CORE_SRCS
        ${CORE_SRCS}
        ${CORE_DIR}/SyncManager.cpp
        CACHE INTERNAL "core sources"
        )
# FRAME_SYNC_E

    set(CORE_SRCS
        ${CORE_SRCS}
        ${CORE_DIR}/psysprocessor/PolicyManager.cpp
        CACHE INTERNAL "core sources"
        )

# IPU_PROCESSING_S
    set(CORE_SRCS
        ${CORE_SRCS}
        ${CORE_DIR}/PSysProcessor.cpp
        ${CORE_DIR}/IspParamAdaptor.cpp
        ${CORE_DIR}/psysprocessor/PSysDAG.cpp
        CACHE INTERNAL "core sources"
       )

if (USE_PG_LITE_PIPE)
    set(CORE_SRCS
        ${CORE_SRCS}
        ${CORE_DIR}/psysprocessor/PipeLiteExecutor.cpp
        ${CORE_DIR}/psysprocessor/PGCommon.cpp
        ${CORE_DIR}/psysprocessor/PGUtils.cpp
        ${CORE_DIR}/psysprocessor/ShareReferBufferPool.cpp
        CACHE INTERNAL "core sources"
       )
else()
    set(CORE_SRCS
        ${CORE_SRCS}
        ${CORE_DIR}/psysprocessor/PipeExecutor.cpp
        ${CORE_DIR}/psysprocessor/PSysPipe.cpp
        CACHE INTERNAL "core sources"
       )
endif()
# IPU_PROCESSING_E

