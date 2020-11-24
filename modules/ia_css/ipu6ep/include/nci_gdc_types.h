/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __NCI_GDC_TYPES_H
#define __NCI_GDC_TYPES_H

#include "ipu_buf_cfg_types.h"
#include "ipu_event_cfg_types.h"
#include "ipu_dim_cfg_types.h"

/* WARNING: DON'T CHANGE THE ORDER OF THE FIELDS AS THEY REPRESENT THE REGISTER LAYOUT. */

/* some hardcoded values, needs to be cleaned */

#define NCI_GDC_NUM_CHANS      4   /* GDC has 4 channels */
#define GDC_CHANNELS_USED      3   /* PG is hardcoded to use 3 channels */
#define GDC_LOG2_OXDIM_DEFAULT 7 /* LOG2(GDC_LOG2_OXDIM_DEFAULT) */
#define GDC_LOG2_OYDIM_DEFAULT 5 /* LOG2(GDC_LOG2_OYDIM_DEFAULT) */
#define GDC_OXDIM_DEFAULT      (1 << GDC_LOG2_OXDIM_DEFAULT) /* default output block width */
#define GDC_OYDIM_DEFAULT      (1 << GDC_LOG2_OYDIM_DEFAULT) /* default output block height */
#define GDC_SCAN_MODE_STB      0 /* HRT_GDC_SCAN_STB: Scanning/sliding mode Slide to Bottom setting */
#define GDC_REG_DONT_CARE      0

struct nci_gdc_kup {
	uint32_t input_bpp;                 /* Bits per element at the input of GDC */
	uint32_t output_bpp;                /* Bits per element at the output of GDC.  */
	uint32_t operation_0;               /* 1st operation performed on input pixels.  */
	uint32_t operation_1;               /* 2nd  operation performed on input pixels. */
	uint32_t operation_2;               /* 3rd operation performed on input pixels. */
	uint32_t processing_mode;           /* Specify whether it is Tetragon, LDC/Homography, or WFOV mode */
	uint32_t output_block_width_exp;    /* GDC output block width expressed as an exponent of 2. */
	uint32_t output_block_height_exp;   /* GDC output block height expressed as an exponent of 2. */
};

enum nci_gdc_perf_mode {
	GDC_PERF_MODE_1x1y = 0,
	GDC_PERF_MODE_2x1y,
	GDC_PERF_MODE_1x2y,
	GDC_PERF_MODE_2x2y,
	GDC_PERF_MODE_ADAPTIVE
};

/**
 * Wrapper for output buffer parameters.
 */
struct gdc_output_buf_cfg {
	struct ipu_buf_cfg buf_config;
	struct ipu_buf_properties buf_properties;
};

/**
 * Wrapper for input buffer parameters.
 */
struct gdc_input_buf_cfg {
	uint32_t cmd_addr;              /* Address where the release/enqueue cmd needs to be sent */
	uint32_t token;                 /* Token to be sent as release/enqueue cmd */
};

/**
 * GDC mode settings
 */
struct gdc_mode_cfg {
	uint32_t scan_mode;             /* Scanning (sliding) mode */
	uint32_t proc_mode;             /* Processing (gdc) mode. TODO: fix P2P passes this over KUP */
	uint32_t src_packed;            /* Source packing mode used for 8bpp precision */
	uint32_t dst_packed;            /* Destination packing mode used for 8 bpp precision */
	uint32_t gro_rpwc;              /* GRO and Read-Post-Write-Check */
	uint32_t coord_ratio;           /* LDC/Homography/de-warping scale-down ratio */
};

/**
 * struct gdc_sectionA_cfg, as a blob, will be filled by PG Control Init code of the GDC.
 * It will be written by Program Load to the HW.
 * These parameters are needed for every channel.
 */
struct gdc_sectionA_cfg {
	uint32_t bpp_control;           /* input bpp, output bpp, input lsb align and output lsb align */
	struct ipu_dim_cfg output_dim;  /* Dimensions of the produced rectangle */
	struct ipu_dim_cfg input_dim;   /* Deprecated or don't care parameters: Dimensions of the input block */
	uint32_t dx;                    /* Deprecated or don't care parameters: Horizontal scaling factor */
	uint32_t dy;                    /* Deprecated or don't care parameters: Vertical scaling factor */
	uint32_t src_end;               /* Deprecated or don't care parameters: End address */
	uint32_t src_wrap;              /* Deprecated or don't care parameters: Wrap address */
	uint32_t src_stride;            /* MBR overwrite this, so don't care for GDC. TODO: confirm */
	uint32_t dummy;                 /* Reserved register */
	uint32_t perf_mode;                       /* Performance mode */
};

/**
 * struct gdc_sectionB_cfg, as a blob, will be filled by PG Control Init code of the GDC.
 * It will be written by Program Load to the HW.
 * These parameters are needed for every channel
 */
struct gdc_sectionB_cfg {
	struct gdc_mode_cfg mode_config;          /* Mode configurations for the GDC */
	struct gdc_output_buf_cfg op_buf_cfg;     /* Producer side configuration of the GDC */
	struct gdc_input_buf_cfg ip_buf_cfg;      /* Consumer side configuration of the GDC */
	struct ipu_event_cfg frag_done_evt;       /* Fragment done ack configuration */
};

struct gdc_global_cfg {
	struct ipu_dim_cfg input_image;           /* Dimensions of the input image */
};

struct nci_gdc_cfg {
	struct gdc_global_cfg   glb;
	struct gdc_sectionA_cfg ch0_a;
	struct gdc_sectionB_cfg ch0_b;
	struct gdc_sectionA_cfg ch1_a;
	struct gdc_sectionB_cfg ch1_b;
	struct gdc_sectionA_cfg ch2_a;
	struct gdc_sectionB_cfg ch2_b;
};

enum nci_gdc_sections {
	NCI_GDC_GLOBAL_SECTION = 0,
	NCI_GDC_CHAN0_SECTION_A,
	NCI_GDC_CHAN0_SECTION_B,
	NCI_GDC_CHAN1_SECTION_A,
	NCI_GDC_CHAN1_SECTION_B,
	NCI_GDC_CHAN2_SECTION_A,
	NCI_GDC_CHAN2_SECTION_B,
	NCI_GDC_NUM_SECTIONS,
};

/* Cached in Sections: */
struct nci_gdc_chan_cached {
	uint32_t interp_type;
	uint32_t filter_size;
	uint32_t lut_sel;
};

struct nci_gdc_homography {
	uint32_t reg[171]; /* From GDC HAS: start_reg_ID -> 19  end_reg_ID -> 189 */
};

struct nci_gdc_datapath_lut {
	uint32_t lut[1024];	 /* From GDC HAS */
};

struct nci_gdc_ldc_dewarping_lut {
	uint32_t ldc_lut[256];       /* From GDC HAS */
};

struct nci_gdc_wfov {
	uint32_t gdc_modep;
	uint32_t rotation_matrix_const0;
	uint32_t rotation_matrix_const1;
	uint32_t rotation_matrix_const2;
	uint32_t rotation_matrix_const3;
	uint32_t rotation_matrix_const4;
	uint32_t rotation_matrix_const5;
	uint32_t rotation_matrix_const6;
	uint32_t rotation_matrix_const7;
	uint32_t rotation_matrix_const8;
	uint32_t projection_sf0;
	uint32_t projection_sf1;
	uint32_t projection_sf2;
	uint32_t projection_sf3;
	uint32_t projection_type;
	uint32_t projection_inv_f_pi;
	uint32_t ldc_mode;
	uint32_t ldc_normalization_sf;
	uint32_t ldc_max_a;
	uint32_t ldc_inv_max_a;
	uint32_t ldc_lut_shift_bits;
	uint32_t preaffine_matrix_scale_const0;
	uint32_t preaffine_matrix_scale_const1;
	uint32_t preaffine_matrix_scale_const2;
	uint32_t preaffine_matrix_scale_const3;
	uint32_t preaffine_matrix_translate_const0;
	uint32_t preaffine_matrix_translate_const1;
	uint32_t postaffine_matrix_scale_const0;
	uint32_t postaffine_matrix_scale_const1;
	uint32_t postaffine_matrix_scale_const2;
	uint32_t postaffine_matrix_scale_const3;
	uint32_t postaffine_matrix_translate_const0;
	uint32_t postaffine_matrix_translate_const1;
	uint32_t invalid_coord_mask_ch0;
	uint32_t invalid_coord_mask_ch1;
	uint32_t invalid_coord_mask_ch2;
	uint32_t invalid_coord_mask_ch3;
};

#endif /* __NCI_GDC_TYPES_H */
