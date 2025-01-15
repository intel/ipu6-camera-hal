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

    if (ENABLE_SANDBOXING)
        add_subdirectory(sandboxing)
    else()
        add_subdirectory(algowrapper)
    endif() #ENABLE_SANDBOXING

if (USE_PG_LITE_PIPE)
    add_subdirectory(ia_cipr)
endif() #USE_PG_LITE_PIPE

if (SUPPORT_LIVE_TUNING)
    add_subdirectory(livetune)
endif() #SUPPORT_LIVE_TUNING
