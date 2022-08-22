#
#  Copyright (C) 2017-2021 Intel Corporation
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

set(IUTILS_SRCS
    ${IUTILS_DIR}/CameraLog.cpp
    ${IUTILS_DIR}/LogSink.cpp
    ${IUTILS_DIR}/ModuleTags.cpp
    ${IUTILS_DIR}/CameraDump.cpp
    ${IUTILS_DIR}/Trace.cpp
    ${IUTILS_DIR}/ScopedAtrace.cpp
    ${IUTILS_DIR}/Thread.cpp
    ${IUTILS_DIR}/Utils.cpp
    ${IUTILS_DIR}/SwImageConverter.cpp
# SUPPORT_MULTI_PROCESS_S
    ${IUTILS_DIR}/CameraShm.cpp
# SUPPORT_MULTI_PROCESS_E
    CACHE INTERNAL "iutils sources"
    )

