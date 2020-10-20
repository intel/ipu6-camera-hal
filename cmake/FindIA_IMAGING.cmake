#
#  Copyright (C) 2017-2019 Intel Corporation
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
pkg_check_modules(IA_IMAGING ia_imaging)
if(NOT IA_IMAGING_FOUND)
    message(FATAL_ERROR "IA_IMAGING not found")
endif()

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${IA_IMAGING_LIBRARY_DIRS})

# Libraries
find_library(IA_AIQ_LIB         ia_aiq)
find_library(IA_AIQB_PARSER_LIB ia_aiqb_parser)
find_library(IA_CMC_PARSER_LIB  ia_cmc_parser)
if (NOT ENABLE_SANDBOXING)
find_library(IA_LARD_LIB        ia_lard)
endif() #ENABLE_SANDBOXING
find_library(IA_EXC_LIB         ia_exc)
find_library(IA_MKN_LIB         ia_mkn)
find_library(IA_NVM_LIB         ia_nvm)
find_library(IA_LOG_LIB         ia_log)
find_library(IA_DVS_LIB         ia_dvs)
find_library(IA_COORDINATE_LIB  ia_coordinate)
find_library(IA_LTM_LIB         ia_ltm)
find_library(IA_DVS_LIB         ia_dvs)

set(ENABLE_LEGACY_AIC ON)

if (ENABLE_LEGACY_AIC)
    find_library(IA_ISP_BXT_LIB ia_isp_bxt)
    find_library(BXT_IA_PAL_LIB broxton_ia_pal)
endif()

set(IA_IMAGING_LIBS
    ${IA_AIQ_LIB}
    ${IA_AIQB_PARSER_LIB}
    ${IA_CMC_PARSER_LIB}
    ${IA_EXC_LIB}
    ${IA_MKN_LIB}
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

if (ENABLE_LEGACY_AIC)
    set(IA_IMAGING_LIBS ${IA_IMAGING_LIBS} ${IA_ISP_BXT_LIB} ${BXT_IA_PAL_LIB})
else()
    set(IA_IMAGING_LIBS ${IA_IMAGING_LIBS} ${IA_AIC_LIB})
endif()

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(IA_IMAGING
                                  REQUIRED_VARS IA_IMAGING_INCLUDE_DIRS IA_IMAGING_LIBS)

if(NOT IA_IMAGING_FOUND)
    message(FATAL_ERROR "IA_IMAGING not found")
endif()
