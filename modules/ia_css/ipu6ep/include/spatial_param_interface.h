/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2017 Intel Corporation
 * All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.Title to the Material remains with Intel
 * Corporation or its suppliers and licensors.The Material may contain trade
 * secrets and proprietary and confidential information of Intel Corporation
 * and its suppliers and licensors, and is protected by worldwide copyright
 * and trade secret laws and treaty provisions.No part of the Material may be
 * used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 *
 * Unless otherwise agreed by Intel in writing, you may not remove or alter
 * this notice or any other notice embedded in Materials by Intel or Intels
 * suppliers or licensors in any way.
 */

#ifndef __SPATIAL_PARAM_INTERFACE
#define __SPATIAL_PARAM_INTERFACE

typedef struct {
	uint32_t	max_slice_size;
	uint32_t	slice_0_dest_offset;
	uint32_t	next_slice_offset;
	uint32_t	elem_size;
} spatial_parameter_terminal_section_private_desc_t;

typedef struct {
	uint16_t	compute_units_p_elem;
	uint16_t	plane_count;
} spatial_parameter_terminal_terminal_private_desc_t;

typedef struct {
	const spatial_parameter_terminal_terminal_private_desc_t *terminal_in;
	const spatial_parameter_terminal_terminal_private_desc_t *terminal_out;
	const spatial_parameter_terminal_section_private_desc_t  *terminal_section_in;
	const spatial_parameter_terminal_section_private_desc_t  *terminal_section_out;
} interface_spatial_parameter_t;

#endif /* __SPATIAL_PARAM_INTERFACE */
