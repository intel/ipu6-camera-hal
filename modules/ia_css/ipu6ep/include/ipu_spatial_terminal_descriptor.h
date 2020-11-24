/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2015 - 2016 Intel Corporation.
* All Rights Reserved.
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Intel Corporation
* or licensors. Title to the Material remains with Intel
* Corporation or its licensors. The Material contains trade
* secrets and proprietary and confidential information of Intel or its
* licensors. The Material is protected by worldwide copyright
* and trade secret laws and treaty provisions. No part of the Material may
* be used, copied, reproduced, modified, published, uploaded, posted,
* transmitted, distributed, or disclosed in any way without Intel's prior
* express written permission.
*
* No License under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or
* delivery of the Materials, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights
* must be express and approved by Intel in writing.
*/

#ifndef __IPU_SPATIAL_TERMINAL_DESCRIPTOR_H
#define __IPU_SPATIAL_TERMINAL_DESCRIPTOR_H

#include "ia_css_terminal_defs.h"

#define IA_CSS_MAX_N_FRAME_PLANES			(4)

struct ipu_frame_grid_param_section_descriptor_s {
	uint32_t			mem_offset;		/**< Offset of the parameter allocation in memory */
	uint32_t			mem_size;		/**< Memory allocation size needs of this parameter */
	uint32_t			stride;			/**< stride in bytes of each line of compute units for the specified memory space and region */
};

struct ipu_spatial_terminal_descriptor_s {
	uint32_t  base_address;                                            /**< DDR base address of spatial parameter buffer */
	uint16_t  frame_grid_dimension[IA_CSS_N_DATA_DIMENSION];           /**< Spatial param FRAME width/height, measured in compute units */
	uint16_t  fragment_grid_index[IA_CSS_N_DATA_DIMENSION];            /**< Spatial param FRAGMENT offset of the top-left compute unit, compared to the frame */
	uint16_t  fragment_grid_dimension[IA_CSS_N_DATA_DIMENSION];        /**< Spatial param FRAGMENT width/height, measured in compute units */

	struct ipu_frame_grid_param_section_descriptor_s param_planes[IA_CSS_MAX_N_FRAME_PLANES]; /**< Section/plane descriptors */
};

#endif /* __IPU_SPATIAL_TERMINAL_DESCRIPTOR_H */
