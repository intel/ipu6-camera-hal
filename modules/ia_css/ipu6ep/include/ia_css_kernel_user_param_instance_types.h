/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2019 Intel Corporation.
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

#ifndef __IA_CSS_KERNEL_USER_PARAM_INSTANCE_TYPES_H
#define __IA_CSS_KERNEL_USER_PARAM_INSTANCE_TYPES_H

/** \brief A generic structure for encoding the fragment
 * sequencer information of a kernel.
 */
struct ia_css_kup_frag_seq_info_minimal {
	uint32_t fragment_width;			/* In pixels */
	uint32_t fragment_height;			/* In pixels */
};

/** \brief A generic structure for encoding the fragment
 * sequencer information of a kernel.
 */
struct ia_css_kup_frag_seq_info {
	uint32_t fragment_width;            /* In pixels */
	uint32_t fragment_height;           /* In pixels */
	int32_t fragment_start_x;           /* From pixel fragment to grid fragment. In pixels */
	int32_t fragment_start_y;           /* From pixel fragment to grid fragment. In pixels */
	uint32_t fragment_block_width;      /* In pixels */
	uint32_t fragment_block_height;     /* In pixels */
	uint32_t fragment_grid_width;       /* In number of block width */
	uint32_t fragment_grid_height;      /* In number of block height */
	uint32_t fragment_slice_width;      /* In number of block width */
	uint32_t fragment_slice_height;     /* In number of block height */
	uint32_t fragment_slice_count_x;    /* Number of slices x-axis */
	uint32_t fragment_slice_count_y;    /* Number of slices y-axis */
};

typedef uint8_t ia_css_dvs_y_bin_mode_t;

struct ia_css_isa_psa_connection_info_t {
	uint16_t width;			    /* In pixels */
	uint16_t height;		    /* In pixels */
	uint16_t is_yuv_format;		    /* 0 - BAYER_VECTORIZED, 1 - YYUVYY */
};

struct ia_css_kup_gdc_mbr {
	uint32_t input_bpp;                 /* Bits per element at the input of GDC */
	uint32_t output_bpp;                /* Bits per element at the output of GDC.  */
	uint32_t operation_0;               /* 1st operation performed on input pixels.  */
	uint32_t operation_1;               /* 2nd  operation performed on input pixels. */
	uint32_t operation_2;               /* 3rd operation performed on input pixels. */
	uint32_t processing_mode;           /* Specify whether it is Tetragon, LDC/Homography, or WFOV mode */
	uint32_t filter_padding;            /* Number of elements that the MBR adds based on GDC interpolation type */
	uint32_t erosion_level;             /* Maximum adaptiveness of GDC block size processing */
	uint32_t output_block_width_exp;    /* GDC output block width expressed as an exponent of 2. */
	uint32_t output_block_height_exp;   /* GDC output block height expressed as an exponent of 2. */
	uint32_t output_fragment_width;     /* Output fragment width of GDC (in elements - real dimension) */
	uint32_t output_fragment_height;    /* Output fragment height of GDC (in lines - real dimension) */
	uint32_t start_block_idx_x;         /* Horizontal index of the first block to be processed */
	uint32_t start_block_idx_y;         /* Vertical index of the first block to be processed */
};

struct ia_css_kup_ofs {
	uint32_t crop_top;              /* Cropper top pixel coordinate  */
	uint32_t crop_left;             /* Cropper left pixel coordinate */
	uint32_t input_width;           /* Output formatter input width  */
	uint32_t input_height;          /* Output formatter input height */
	uint32_t is_scaling;            /* 1 if scaler is enabled, 0 otherwise. This field applies only for DP and PP */
};

struct ia_css_kup_vert_padd {
	uint8_t top_num_of_lines;      /* Num of dummy lines to add at the top (max 6 bits) */
	uint8_t bottom_num_of_lines;   /* Num of dummy lines to add at the bottom (max 6 bits) */
};

/** \brief A structure for encoding LSC fragment
 * sequencer information of a kernel.
 */
struct ia_css_spt_in_kup_frag_seq_info {
	struct ia_css_kup_frag_seq_info frag_seq;
	uint32_t cfa_type; /* cfa type 0=1x1 1=2x2 2=4x4 */
};

/* For use in ipu7 Phase1 pss af_odr */
struct ia_css_complex_io_kup_info {
	uint32_t fragment_width;
	uint32_t fragment_height;
	uint32_t fragment_grid_width;
	uint32_t fragment_grid_height;
	int32_t x_output_offset; /* signed x offset needed for start of line padding */
	uint8_t y_output_offset;
	uint32_t cfa_type; /* Currently only '1', for bayer, is supported */
};

typedef struct ia_css_kup_frag_info {
	struct ia_css_kup_frag_seq_info_minimal in_res;
	struct ia_css_kup_frag_seq_info_minimal out_res;
	uint8_t x_output_offset;
	uint8_t y_output_offset;
} ia_css_kup_frag_info_t;

/* For use in ipu7 ifd and odr */
struct ia_css_kup_espa_control {
	bool luma_shift_left;
	uint8_t luma_shift_count;
	bool chroma_shift_left;
	uint8_t chroma_shift_count;
	uint8_t range_reduction_output_resolution;
	bool range_reduction_enable;
	bool luma_clipping_min_enable;
	bool luma_clipping_max_enable;
	bool chroma_clipping_min_enable;
	bool chroma_clipping_max_enable;
	bool espa_enable;
};

struct ia_css_kup_espa_luma_clipping {
	uint16_t clipping_min;
	uint16_t clipping_max;
};

struct ia_css_kup_espa_luma_offset {
	uint16_t offset_value;
};

struct ia_css_kup_espa_chroma_clipping {
	uint16_t clipping_min;
	uint16_t clipping_max;
};

struct ia_css_kup_espa_chroma_offset {
	uint16_t offset_value;
};

typedef struct ia_css_kup_block_info {
	uint8_t tile_mode;
} ia_css_kup_block_info_t;

typedef struct ia_css_lbff_io_kup_info {
	struct ia_css_kup_frag_info frag_info;
	struct ia_css_kup_espa_control espa_control;
	struct ia_css_kup_espa_luma_clipping espa_luma_clipping;
	struct ia_css_kup_espa_luma_offset espa_luma_offset;
	struct ia_css_kup_espa_chroma_clipping espa_chroma_clipping;
	struct ia_css_kup_espa_chroma_offset espa_chroma_offset;
	uint32_t cfa_type; /* Currently only '1', for bayer, is supported */
} ia_css_lbff_io_kup_info_t;

typedef struct ia_css_bbps_io_kup_info {
	struct ia_css_kup_frag_info frag_info;
	struct ia_css_kup_espa_control espa_control;
	struct ia_css_kup_espa_luma_clipping espa_luma_clipping;
	struct ia_css_kup_espa_luma_offset espa_luma_offset;
	struct ia_css_kup_espa_chroma_clipping espa_chroma_clipping;
	struct ia_css_kup_espa_chroma_offset espa_chroma_offset;
	struct ia_css_kup_block_info block_info;
	uint32_t cfa_type; /* Currently only '1', for bayer, is supported */
} ia_css_bbps_io_kup_info_t;

#endif /* __IA_CSS_KERNEL_USER_PARAM_INSTANCE_TYPES_H */
