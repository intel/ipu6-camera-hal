/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2015 - 2019 Intel Corporation.
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

#ifndef __IA_CSS_COMMON_IO_TYPES_H
#define __IA_CSS_COMMON_IO_TYPES_H

#include <type_support.h>
#include "ia_css_program_group_data_defs.h"
#include "ipu_spatial_terminal_descriptor.h"
#include "ia_css_psys_manifest_types.h"

#ifndef MAX_IO_DMA_CHANNELS
#define MAX_IO_DMA_CHANNELS 3
#endif

#ifdef PIPE_GENERATION
#define PIPEMEM(x) MEM(x)
#else
#define PIPEMEM(x)
#endif

typedef PIPEMEM(dmem) struct ia_css_terminal_descriptor_s *p_terminal_config;
typedef PIPEMEM(dmem) struct ia_css_common_dma_config *p_terminal_dma_config;
typedef PIPEMEM(dmem) struct ipu_spatial_terminal_descriptor_s *p_spatial_terminal_config;
typedef PIPEMEM(dmem) struct ipu_sequencer_info_descriptor_s *p_sequencer_info_config;
typedef PIPEMEM(dmem) struct ia_css_sliced_terminal_descriptor_s *p_sliced_terminal_config;

struct ia_css_resolution_descriptor_s {
	uint16_t			width;					/**< Logical dimensions */
	uint16_t			height;					/**< Logical dimensions */
};

/*
 * Should comply to enum ia_css_dimension_t in ./fw_abi_common_types/ia_css_terminal_defs.h
 */
#define COL_DIMENSION 0
#define ROW_DIMENSION 1
#define N_DATA_DIMENSION 2
#define N_MAX_NUM_SLICES_PER_FRAGMENT 32
#define N_MAX_NUM_SECTIONS_PER_SLICE 4

/*
 * Terminal type
 */
struct ia_css_terminal_descriptor_s {
	ia_css_frame_format_type_t	format;			/**< Indicates if this is a generic type or inbuild with variable size descriptor */
	uint32_t			plane_count;				/**< Number of data planes (pointers) */
	uint32_t			plane_offsets[IA_CSS_MAX_N_FRAME_PLANES];	/**< Plane offsets */
	uint32_t			stride;					/**< Physical size aspects */
	uint16_t			width;					/**< Logical dimensions */
	uint16_t			height;					/**< Logical dimensions */
	uint8_t				bpp;					/**< Bits per pixel */
	uint8_t				bpe;					/**< Bits per element */
	uint8_t				is_compressed;				/**< 1 if terminal uses compressed datatype, 0 otherwise */
	uint32_t			ts_offsets[IA_CSS_MAX_N_FRAME_PLANES];	/**< DEC400 compression tile status offsets */
	uint32_t			base_address;				/**< DDR base address */
	uint32_t			fragment_index[N_DATA_DIMENSION];	/**< Fragment start offset in pixels for each dimension */
	uint32_t			config_address;				/**< Dmem address of port configuration */
	ia_css_connection_type_t	connection_type;		/**< Connection {buffer, stream, ...} */
	uint8_t				link_id;	/* Link ID of the data terminal OTF, data barrier and stream2gen */
	uint16_t			stream2gen_buffer_size; /* Units of buffer_size are lines */
};

typedef struct ia_css_terminal_descriptor_s ia_css_terminal_descriptor_t;

/*
 * Sub-struct for Sliced terminal descriptor
 */
struct sliced_param_section_desc_s {
	uint32_t			mem_offset;		/**< Offset of the parameter allocation in memory */
	uint32_t			mem_size;		/**< Memory allocation size needs of this parameter */
};

/*
 * Sliced terminal descriptor
 */
struct ia_css_sliced_terminal_descriptor_s {
	uint32_t	base_address;  				/**< DDR base address of spatial parameter buffer */
	uint16_t	slice_count;				/**< Number of slices for the parameters for this fragment */
	struct sliced_param_section_desc_s section_param[N_MAX_NUM_SLICES_PER_FRAGMENT][N_MAX_NUM_SECTIONS_PER_SLICE]; /**< Section descriptors */
};

/* DMA based terminals */
struct ia_css_common_dma_config {
	uint32_t ddr_elems_per_word;
	uint32_t dma_channel[MAX_IO_DMA_CHANNELS];
	uint32_t here_crop_x; /* horizontal cropping in terms of elements on ISP side */
	uint32_t here_crop_y; /* vertical cropping in terms of elements on ISP side */
};

#endif /* __IA_CSS_COMMON_IO_TYPES_H */
