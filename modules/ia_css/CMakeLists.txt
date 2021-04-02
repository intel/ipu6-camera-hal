#
#  Copyright (C) 2021 Intel Corporation
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

if(NOT USE_PG_LITE_PIPE)
message(FATAL_ERROR "this folder is intended for PG_LITE_PIPE usage")
endif()

if("${IPU_VER}" STREQUAL "ipu6")
set(MODULE_IA_CSS_SRC
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_terminal.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_psys_program_group_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_psys_process_group.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_rbm.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_program_group_data.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_psys_process_group_cmd_impl.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_psys_process.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/vied_nci_psys_system.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_psys_terminal.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_program_group_param.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_terminal_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_psys_program_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_psys_terminal_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6/src/ia_css_kernel_bitmap.c
    PARENT_SCOPE
)
set(MODULE_IA_CSS_HEADER_INCLUDE_DIR ${MODULE_IA_CSS_DIR}/ipu6/include PARENT_SCOPE)

elseif("${IPU_VER}" STREQUAL "ipu6sepla")
set(MODULE_IA_CSS_SRC
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_terminal.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_psys_program_group_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_psys_process_group.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_rbm.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_program_group_data.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_psys_process_group_cmd_impl.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_psys_process.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/vied_nci_psys_system.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_psys_terminal.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_program_group_param.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_terminal_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_psys_program_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_psys_terminal_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6se/src/ia_css_kernel_bitmap.c
    PARENT_SCOPE
)

set(MODULE_IA_CSS_HEADER_INCLUDE_DIR ${MODULE_IA_CSS_DIR}/ipu6se/include PARENT_SCOPE)

elseif("${IPU_VER}" STREQUAL "ipu6ep")
set(MODULE_IA_CSS_SRC
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_terminal.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_psys_program_group_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_psys_process_group.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_rbm.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_program_group_data.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_psys_process_group_cmd_impl.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_psys_process.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/vied_nci_psys_system.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_psys_terminal.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_program_group_param.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_terminal_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_psys_program_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_psys_terminal_manifest.c
    ${MODULE_IA_CSS_DIR}/ipu6ep/src/ia_css_kernel_bitmap.c
    PARENT_SCOPE
)

set(MODULE_IA_CSS_HEADER_INCLUDE_DIR ${MODULE_IA_CSS_DIR}/ipu6ep/include PARENT_SCOPE)
else()
message(WARNING      "you are using cmake building system and you are not using ipu6")
endif()
