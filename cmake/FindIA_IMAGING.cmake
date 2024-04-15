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
include(FindPackageHandleStandardArgs)

if(NOT DEFINED IPU_VER)
    set(IA_IMAGING_PKG_SUFFIX "-ipu4")
elseif(${IPU_VER} STREQUAL ipu6epmtl)
    set(IA_IMAGING_PKG_SUFFIX "")
else()
    set(IA_IMAGING_PKG_SUFFIX "-${IPU_VER}")
endif()

find_package(PkgConfig)
pkg_check_modules(IA_IMAGING ia_imaging${IA_IMAGING_PKG_SUFFIX})
if(NOT IA_IMAGING_FOUND)
    message(FATAL_ERROR "IA_IMAGING not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${IA_IMAGING_LIBRARY_DIRS})

# Libraries
find_library(IA_CCA_LIB         ia_cca${IA_IMAGING_PKG_SUFFIX})
find_library(IA_AIQ_LIB         ia_aiq${IA_IMAGING_PKG_SUFFIX})
find_library(IA_AIQB_PARSER_LIB ia_aiqb_parser${IA_IMAGING_PKG_SUFFIX})
find_library(IA_CMC_PARSER_LIB  ia_cmc_parser${IA_IMAGING_PKG_SUFFIX})
if (NOT ENABLE_SANDBOXING)
find_library(IA_LARD_LIB        ia_lard${IA_IMAGING_PKG_SUFFIX})
endif() #ENABLE_SANDBOXING
find_library(IA_EXC_LIB         ia_exc${IA_IMAGING_PKG_SUFFIX})
find_library(IA_MKN_LIB         ia_mkn${IA_IMAGING_PKG_SUFFIX})
# SENSOR_EMD_DECODER
find_library(IA_EMD_LIB         ia_emd_decoder${IA_IMAGING_PKG_SUFFIX})
# DOL_FEATURE_S
find_library(IA_BCOMP_LIB       ia_bcomp${IA_IMAGING_PKG_SUFFIX})
# DOL_FEATURE_E
find_library(IA_NVM_LIB         ia_nvm${IA_IMAGING_PKG_SUFFIX})
find_library(IA_LOG_LIB         ia_log${IA_IMAGING_PKG_SUFFIX})
find_library(IA_DVS_LIB         ia_dvs${IA_IMAGING_PKG_SUFFIX})
find_library(IA_COORDINATE_LIB  ia_coordinate${IA_IMAGING_PKG_SUFFIX})
find_library(IA_LTM_LIB         ia_ltm${IA_IMAGING_PKG_SUFFIX})
find_library(IA_DVS_LIB         ia_dvs${IA_IMAGING_PKG_SUFFIX})
find_library(IA_ISP_BXT_LIB     ia_isp_bxt${IA_IMAGING_PKG_SUFFIX})
find_library(BXT_IA_PAL_LIB     broxton_ia_pal${IA_IMAGING_PKG_SUFFIX})
if (USE_PG_LITE_PIPE)
find_library(P2P_LIB_NAME ia_p2p_${IPU_VER})
endif()

set(IA_IMAGING_LIBS
    ${IA_CCA_LIB}
    ${IA_AIQ_LIB}
    ${IA_AIQB_PARSER_LIB}
    ${IA_CMC_PARSER_LIB}
    ${IA_EXC_LIB}
    ${IA_MKN_LIB}
    ${IA_EMD_LIB}
    ${IA_BCOMP_LIB}
    ${IA_NVM_LIB}
    ${IA_LOG_LIB}
    ${IA_DVS_LIB}
    ${IA_COORDINATE_LIB}
    ${IA_LTM_LIB}
    ${IA_DVS_LIB}
    ${IA_OB_LIB}
)

if (NOT ENABLE_SANDBOXING)
set(IA_IMAGING_LIBS
    ${IA_IMAGING_LIBS}
    ${IA_LARD_LIB}
    )
endif() #ENABLE_SANDBOXING

if (USE_PG_LITE_PIPE)
set(IA_IMAGING_LIBS ${IA_IMAGING_LIBS} ${P2P_LIB_NAME})
endif()

set(IA_IMAGING_LIBS ${IA_IMAGING_LIBS} ${IA_ISP_BXT_LIB} ${BXT_IA_PAL_LIB})

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(IA_IMAGING
                                  REQUIRED_VARS IA_IMAGING_INCLUDE_DIRS IA_IMAGING_LIBS)

if(NOT IA_IMAGING_FOUND)
    message(FATAL_ERROR "IA_IMAGING not found")
endif()
