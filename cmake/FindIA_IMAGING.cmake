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

if(TARGET ia_aiq)
    message("libcamhal found ia_aiq target")
    return()
endif()

# Get include and lib paths for IA_IMAGING from pkgconfig

find_package(PkgConfig)
pkg_check_modules(IA_IMAGING${TARGET_SUFFIX} ia_imaging${TARGET_SUFFIX})
if(NOT IA_IMAGING${TARGET_SUFFIX}_FOUND)
    message(FATAL_ERROR "IA_IMAGING${TARGET_SUFFIX} not found")
endif()

set(IA_IMAGING${TARGET_SUFFIX}_LIBRARIES
    ia_cca${TARGET_SUFFIX}
    ia_aiq${TARGET_SUFFIX}
    ia_aiqb_parser${TARGET_SUFFIX}
    ia_cmc_parser${TARGET_SUFFIX}
    ia_exc${TARGET_SUFFIX}
    ia_mkn${TARGET_SUFFIX}
    ia_emd_decoder${TARGET_SUFFIX}
    ia_nvm${TARGET_SUFFIX}
    ia_log${TARGET_SUFFIX}
    ia_dvs${TARGET_SUFFIX}
    ia_coordinate${TARGET_SUFFIX}
    ia_ltm${TARGET_SUFFIX}
    ia_dvs${TARGET_SUFFIX}
    ia_isp_bxt${TARGET_SUFFIX}
    broxton_ia_pal${TARGET_SUFFIX}
# DOL_FEATURE_S
    ia_bcomp${TARGET_SUFFIX}
# DOL_FEATURE_E
)

if (NOT ENABLE_SANDBOXING)
    set(IA_IMAGING${TARGET_SUFFIX}_LIBRARIES
        ${IA_IMAGING${TARGET_SUFFIX}_LIBRARIES}
        ia_lard${TARGET_SUFFIX}
    )
endif() #ENABLE_SANDBOXING

if (USE_PG_LITE_PIPE)
    set(IA_IMAGING${TARGET_SUFFIX}_LIBRARIES
        ${IA_IMAGING${TARGET_SUFFIX}_LIBRARIES}
        ia_p2p${TARGET_SUFFIX}
    )
endif()
