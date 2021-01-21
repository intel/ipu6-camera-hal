/*
 * Copyright (C) 2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IA_CSS_KERNEL_USER_PARAM_TYPES_H
#define __IA_CSS_KERNEL_USER_PARAM_TYPES_H

#include "type_support.h"

#define KERNEL_USER_PARAM_S_PADDING 4

#define SIZE_OF_KERNEL_USER_PARAM_S ( \
      (1 * IA_CSS_UINT64_T_BITS) \
    + (1 * IA_CSS_UINT32_T_BITS) \
    + (4 * IA_CSS_UINT16_T_BITS) \
    + (KERNEL_USER_PARAM_S_PADDING * IA_CSS_UINT8_T_BITS))

/** \brief A kernel user parameter object.
 * Always use ia_css_kernel_user_param_get_descriptor_size or
 * ia_css_kernel_user_param_get_descriptor_size_using_kernel_info to get the
 * appropriate size for the kernel user parameter object.
 *
 * Use an init function like ia_css_kernel_user_param_init to initialize
 * the kernel user parameter object.
 *
 * Never instantiate this struct in a different way.
 */
typedef struct ia_css_kernel_user_param_s {
    /* Parameter buffer handle attached to the terminal */
    union {
        void *p;
        uint64_t not_used;  /* ensures same struct layout on 64 and 32 bit compilations */
    } payload_buffer;
    /* Buffer size */
    uint32_t payload_buffer_size;
    /* Points to the variable array of ia_css_kernel_user_param_desc_s */
    uint16_t kernel_desc_offset;
    /* Number of kernels in program group */
    uint16_t kernel_count;
    /* Number of fragments in frame */
    uint16_t fragment_count;
    /* Fragment stride in bytes */
    uint16_t fragment_stride;
    /* padding */
    uint8_t padding[KERNEL_USER_PARAM_S_PADDING];
} ia_css_kernel_user_param_t;

typedef struct ia_css_kernel_user_param_kernel_desc_s {
    /* kernel identifier */
    uint32_t kernel_id;
    /* Points to the variable array of ia_css_kernel_user_param_cfg_desc_t */
    uint16_t cfg_desc_offset;
    /* Number of configurations for this kernel */
    uint16_t cfg_desc_count;
} ia_css_kernel_user_param_kernel_desc_t;

typedef struct ia_css_kernel_user_param_cfg_desc_s {
    /* Offset of the parameter allocation in memory */
    uint32_t mem_offset;
    /* Memory allocation size needs of this parameter */
    uint32_t mem_size;
} ia_css_kernel_user_param_cfg_desc_t;

/*!
 * Construction information to construct a kernel user param object.
 * This type is only used for ia_css_kernel_user_param_init and
 * ia_css_kernel_user_param_get_descriptor_size_using_kernel_info.
 */
typedef struct ia_css_kernel_user_param_kernel_info_s {
    uint32_t kernel_id;
    uint16_t section_count;
    uint32_t *section_sizes; /**< Array of size section_count. Each element
        encode the size in bytes or a kernel config (section). */
} ia_css_kernel_user_param_kernel_info_t;

#endif /* __IA_CSS_KERNEL_USER_PARAM_TYPES_H */
