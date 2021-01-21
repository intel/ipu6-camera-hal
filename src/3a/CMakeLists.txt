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

set(3A_SRCS
    ${3A_DIR}/I3AControlFactory.cpp
    CACHE INTERNAL "3A sources"
    )

    set(3A_SRCS
        ${3A_SRCS}
        ${3A_DIR}/AiqUtils.cpp
        ${3A_DIR}/intel3a/Intel3AParameter.cpp
        ${3A_DIR}/AiqResult.cpp
        ${3A_DIR}/AiqResultStorage.cpp
        ${3A_DIR}/SensorManager.cpp
        ${3A_DIR}/LensManager.cpp
        ${3A_DIR}/AiqCore.cpp
        ${3A_DIR}/AiqEngine.cpp
        ${3A_DIR}/AiqStatistics.cpp
        ${3A_DIR}/AiqSetting.cpp
        ${3A_DIR}/AiqUnit.cpp
        ${3A_DIR}/MakerNote.cpp
        CACHE INTERNAL "3A sources"
        )

# INTEL_DVS_S
    set(3A_SRCS
        ${3A_SRCS}
        ${3A_DIR}/Dvs.cpp
        CACHE INTERNAL "3A sources"
        )
# INTEL_DVS_E
# LOCAL_TONEMAP_S
    set(3A_SRCS
        ${3A_SRCS}
        ${3A_DIR}/Ltm.cpp
        CACHE INTERNAL "3A sources"
        )
#LOCAL_TONEMAP_E

