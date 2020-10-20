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
#
#

set(ALGOWRAPPER_SRCS
    ${ALGOWRAPPER_DIR}/IntelLard.cpp
    ${ALGOWRAPPER_DIR}/IntelMkn.cpp
    ${ALGOWRAPPER_DIR}/IntelLtm.cpp
    ${ALGOWRAPPER_DIR}/IntelDvs.cpp
    ${ALGOWRAPPER_DIR}/IntelCmc.cpp
    ${ALGOWRAPPER_DIR}/IntelAiq.cpp
    CACHE INTERNAL "algo wrapper sources"
    )

    set(ALGOWRAPPER_SRCS
        ${ALGOWRAPPER_SRCS}
        ${ALGOWRAPPER_DIR}/graph/GraphConfigImpl.cpp
        ${ALGOWRAPPER_DIR}/graph/GraphConfigPipe.cpp
        CACHE INTERNAL "algo wrapper sources"
    )

    set(ALGOWRAPPER_SRCS
        ${ALGOWRAPPER_SRCS}
        ${ALGOWRAPPER_DIR}/IntelIspParamAdaptor.cpp
        CACHE INTERNAL "algo wrapper sources"
    )

if (USE_PG_LITE_PIPE)
    set(ALGOWRAPPER_SRCS
        ${ALGOWRAPPER_SRCS}
        ${ALGOWRAPPER_DIR}/IntelPGParam.cpp
        CACHE INTERNAL "algo wrapper sources" )
endif() #USE_PG_LITE_PIPE
