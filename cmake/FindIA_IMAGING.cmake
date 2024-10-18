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

find_package(PkgConfig)
pkg_check_modules(IA_IMAGING${TARGET_SUFFIX} ia_imaging${TARGET_SUFFIX})
if(NOT IA_IMAGING${TARGET_SUFFIX}_FOUND)
    message(FATAL_ERROR "IA_IMAGING${TARGET_SUFFIX} not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${IA_IMAGING${TARGET_SUFFIX}_LIBRARY_DIRS})

# Libraries
find_library(IA_CCA${TARGET_SUFFIX}_LIB         ia_cca${TARGET_SUFFIX})
find_library(IA_AIQ${TARGET_SUFFIX}_LIB         ia_aiq${TARGET_SUFFIX})
find_library(IA_AIQB_PARSER${TARGET_SUFFIX}_LIB ia_aiqb_parser${TARGET_SUFFIX})
find_library(IA_CMC_PARSER${TARGET_SUFFIX}_LIB  ia_cmc_parser${TARGET_SUFFIX})
find_library(IA_EXC${TARGET_SUFFIX}_LIB         ia_exc${TARGET_SUFFIX})
find_library(IA_MKN${TARGET_SUFFIX}_LIB         ia_mkn${TARGET_SUFFIX})
# SENSOR_EMD_DECODER
find_library(IA_EMD${TARGET_SUFFIX}_LIB         ia_emd_decoder${TARGET_SUFFIX})
# DOL_FEATURE_S
find_library(IA_BCOMP${TARGET_SUFFIX}_LIB       ia_bcomp${TARGET_SUFFIX})
# DOL_FEATURE_E
find_library(IA_NVM${TARGET_SUFFIX}_LIB         ia_nvm${TARGET_SUFFIX})
find_library(IA_LOG${TARGET_SUFFIX}_LIB         ia_log${TARGET_SUFFIX})
find_library(IA_DVS${TARGET_SUFFIX}_LIB         ia_dvs${TARGET_SUFFIX})
find_library(IA_COORDINATE${TARGET_SUFFIX}_LIB  ia_coordinate${TARGET_SUFFIX})
find_library(IA_LTM${TARGET_SUFFIX}_LIB         ia_ltm${TARGET_SUFFIX})
find_library(IA_DVS${TARGET_SUFFIX}_LIB         ia_dvs${TARGET_SUFFIX})
find_library(IA_ISP_BXT${TARGET_SUFFIX}_LIB     ia_isp_bxt${TARGET_SUFFIX})
find_library(BXT_IA_PAL${TARGET_SUFFIX}_LIB     broxton_ia_pal${TARGET_SUFFIX})

set(IA_IMAGING${TARGET_SUFFIX}_LIBS
    ${IA_CCA${TARGET_SUFFIX}_LIB}
    ${IA_AIQ${TARGET_SUFFIX}_LIB}
    ${IA_AIQB_PARSER${TARGET_SUFFIX}_LIB}
    ${IA_CMC_PARSER${TARGET_SUFFIX}_LIB}
    ${IA_EXC${TARGET_SUFFIX}_LIB}
    ${IA_MKN${TARGET_SUFFIX}_LIB}
    ${IA_EMD${TARGET_SUFFIX}_LIB}
    ${IA_BCOMP${TARGET_SUFFIX}_LIB}
    ${IA_NVM${TARGET_SUFFIX}_LIB}
    ${IA_LOG${TARGET_SUFFIX}_LIB}
    ${IA_DVS${TARGET_SUFFIX}_LIB}
    ${IA_COORDINATE${TARGET_SUFFIX}_LIB}
    ${IA_LTM${TARGET_SUFFIX}_LIB}
    ${IA_DVS${TARGET_SUFFIX}_LIB}
    ${IA_OB${TARGET_SUFFIX}_LIB}
    ${IA_ISP_BXT${TARGET_SUFFIX}_LIB}
    ${BXT_IA_PAL${TARGET_SUFFIX}_LIB}
)

if (NOT ENABLE_SANDBOXING)
    find_library(IA_LARD${TARGET_SUFFIX}_LIB ia_lard${TARGET_SUFFIX})
    set(IA_IMAGING${TARGET_SUFFIX}_LIBS ${IA_IMAGING${TARGET_SUFFIX}_LIBS} ${IA_LARD${TARGET_SUFFIX}_LIB})
endif() #ENABLE_SANDBOXING

if (USE_PG_LITE_PIPE)
    find_library(IA_P2P${TARGET_SUFFIX}_LIB ia_p2p${TARGET_SUFFIX})
    set(IA_IMAGING${TARGET_SUFFIX}_LIBS ${IA_IMAGING${TARGET_SUFFIX}_LIBS} ${IA_P2P${TARGET_SUFFIX}_LIB})
endif()

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(IA_IMAGING${TARGET_SUFFIX}
                                  REQUIRED_VARS IA_IMAGING${TARGET_SUFFIX}_INCLUDE_DIRS IA_IMAGING${TARGET_SUFFIX}_LIBS)

if(NOT IA_IMAGING${TARGET_SUFFIX}_FOUND)
    message(FATAL_ERROR "IA_IMAGING${TARGET_SUFFIX} not found")
endif()
