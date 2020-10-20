#
#  Copyright (C) 2018-2019 Intel Corporation
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

add_subdirectory(3a)
add_subdirectory(core)
add_subdirectory(hal)
add_subdirectory(iutils)
add_subdirectory(metadata)
add_subdirectory(platformdata)
add_subdirectory(v4l2)
add_subdirectory(image_process)
if (CAL_BUILD)
    add_subdirectory(jpeg)
endif() #CAL_BUILD
if(FACE_DETECTION)
    add_subdirectory(fd)
endif() #FACE_DETECTION
