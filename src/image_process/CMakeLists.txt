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

set(IMAGE_PROCESS_SRCS
    ${IMAGE_PROCESS_DIR}/ImageConverter.cpp
    ${IMAGE_PROCESS_DIR}/ImageScalerCore.cpp
    CACHE INTERNAL "image_process sources"
    )

if (CAL_BUILD)
set(IMAGE_PROCESS_SRCS
    ${IMAGE_PROCESS_DIR}/PostProcessorBase.cpp
    ${IMAGE_PROCESS_DIR}/PostProcessorCore.cpp
    CACHE INTERNAL "image_process sources"
    )
if (SW_POST_PROCESSING)
    set(IMAGE_PROCESS_SRCS
        ${IMAGE_PROCESS_SRCS}
        ${IMAGE_PROCESS_DIR}/sw/SWPostProcessor.cpp
        CACHE INTERNAL "image_process sources"
       )
else()
    set(IMAGE_PROCESS_SRCS
        ${IMAGE_PROCESS_SRCS}
        ${IMAGE_PROCESS_DIR}/chrome/ImageProcessorCore.cpp
        CACHE INTERNAL "image_process sources"
        )
endif() #SW_POST_PROCESSING
endif() #CAL_BUILD
