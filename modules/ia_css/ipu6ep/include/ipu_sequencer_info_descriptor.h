/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2015 - 2017 Intel Corporation.
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

#ifndef __IPU_SEQUENCER_INFO_DESCRIPTOR_H
#define __IPU_SEQUENCER_INFO_DESCRIPTOR_H

#include "ipu_sequencer_info_descriptor_dep.h"
#include "ia_css_terminal_defs.h"
#include "ia_css_terminal_types.h"

/*
 * limits on command count
 */
#define N_COMMAND_COUNT     (4)

/*
 * Default limits on the number of command descriptor.
 */
#ifndef IA_CSS_MAX_N_COMMAND_DESC
#define IA_CSS_MAX_N_COMMAND_DESC   	(32)
#endif

/*
 * Sub-struct for Sequencer info descriptor
 */
struct ipu_kernel_fragment_sequencer_info_desc_s {
	uint16_t fragment_grid_slice_dimension[IA_CSS_N_DATA_DIMENSION];			/**< Slice dimensions */
	uint16_t fragment_grid_slice_count[IA_CSS_N_DATA_DIMENSION];				/**< Nof slices */
	uint16_t fragment_grid_point_decimation_factor[IA_CSS_N_DATA_DIMENSION];		/**< Grid point decimation factor */
	int16_t fragment_grid_overlay_pixel_topleft_index[IA_CSS_N_DATA_DIMENSION];	/**< Relative position of grid origin to pixel origin */
	int16_t fragment_grid_overlay_pixel_dimension[IA_CSS_N_DATA_DIMENSION];	/**< Size of active fragment region */
	uint16_t command_count;									/**< If >0 it overrides the standard fragment sequencer info */
	uint16_t command_index;									/**< To be used only if command_count>0, index to the array of (ia_css_kernel_fragment_sequencer_command_desc_s) */
};

/*
 * Sub-struct for Sequencer info descriptor
 */
struct ipu_kernel_fragment_sequencer_command_desc_s {	/**< 4 commands packed together to save memory space */
	uint16_t line_count[N_COMMAND_COUNT];		/**< Contains the "(command_index%4) == index" command desc */
};

/*
 * Sequencer info descriptor
 */
struct ipu_sequencer_info_descriptor_s {
	struct ipu_kernel_fragment_sequencer_info_desc_s kernel_fragment_sequencer_info_desc[IA_CSS_MAX_N_SEQUENCER_INFO];
	struct ipu_kernel_fragment_sequencer_command_desc_s kernel_fragment_sequencer_command_desc[IA_CSS_MAX_N_COMMAND_DESC]; /* This command desc array is shared by all sequencers via command_index */
};

void ia_css_connection_mgr_get_kernel_fragment_seq_info_desc(struct ipu_kernel_fragment_sequencer_info_desc_s* kernel_sequencer_info_desc,
										  const ia_css_program_terminal_t *prog_term);

#endif /* __IPU_SEQUENCER_INFO_DESCRIPTOR_H */
