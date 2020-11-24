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

#ifndef __NCI_MBR_TYPES_H
#define __NCI_MBR_TYPES_H

#include "ipu_buf_cfg_types.h"
#include "ipu_event_cfg_types.h"
#include "ipu_dim_cfg_types.h"
#include "ipu_dma_cfg_types.h"

/* WARNING: DON'T CHANGE THE ORDER OF THE FIELDS AS THEY  REPRESENT THE REGISTER LAYOUT. */
#define MBR_REG_DONT_CARE      0

struct nci_mbr_kup {
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

enum nci_mbr_device_mode {
	NCI_MBR_DEVICE_MODE_GDC = 0,
	NCI_MBR_DEVICE_MODE_TNR
};

enum nci_mbr_ch {
	NCI_MBR_CH0 = 0,
	NCI_MBR_CH1,
	NCI_MBR_CH2,
	NCI_MBR_NUM_CHANS
};

enum nci_mbr_algo_type {
	MBR_MAX_SAVE = 0,
	MBR_FULL_GOB
};

enum nci_mbr_bpp {
	NCI_MBR_BPP_8 = 0,
	NCI_MBR_BPP_10,
	NCI_MBR_BPP_12,
	NCI_MBR_BPP_14,
	NCI_MBR_BPP_16
};

enum nci_mbr_filter_pad {
	NCI_MBR_FILTER_PAD_NND_BLI = 0,
	NCI_MBR_FILTER_PAD_4x4,
	NCI_MBR_FILTER_PAD_6x6,
};

enum nci_mbr_erosion_level {
	NCI_MBR_EROSION_DISABLE = 0,
	NCI_MBR_EROSION_LEVEL_1,
	NCI_MBR_EROSION_LEVEL_2,
};

struct mbr_buf_properties{
	uint32_t num_buf;
	struct ipu_buf_properties buf;
};

struct mbr_dim_cfg{
	uint32_t height;
	uint32_t width;
};

struct nci_mbr_global0_cfg {
	uint32_t activate_command;                /* Configure the processing modes and trigger MBR to start its operation */
	struct ipu_event_cfg fragment_ack;        /* Info about the ACK that MBR will send when the fragment id completed */
	uint32_t general_cntl_register;           /* Batch size for DMA_HBfX and DMA_i, and clock clock gating */
	uint32_t gdc_stride_reg_offset;           /* Address offset (register index) of the input buffer stride register */
	uint32_t gdc_tnr_cmd_addr;                /* Address of GDC or TNR command FIFO */
	uint32_t frame_init;                      /* Configure the mode and number of channels to be used (Resets MBR state machines) */
	uint32_t tetragon_array_stride;           /* Stride in bytes of the tetragon coordinates stored in DDR */
	uint32_t dma_lb_channel_id;               /* DMA_LB channel ID for the top command of coords fetch. Bottom command will use "channel_id+1" */
	uint32_t dma_lb_req_queue_size;           /* DMA_LB requestor bank queue depth (for coords fetch) */
	uint32_t dma_lb_req_queue_addr;           /* Address where the MBR submits DMA instructions for coordinate fetch */
	uint32_t dma_lb_inst_format;              /* The content of the instruction sent by MBR to DMA_LB */
	uint32_t ext_dma_inst_format;             /* The content of the instruction sent by MBR to DMA_HBfrX */
	uint32_t int_dma_inst_format;             /* The content of the instruction sent by MBR to DMA_i*/
	uint32_t start_blockID_X;                 /* (Only in LDC/Hom/DeW mode) horizontal offset in blocks of the first block to be processed */
	uint32_t start_blockID_Y;                 /* (Only in LDC/Hom/DeW mode) vertical offset in blocks of the first block to be processed */
	struct ipu_dim_cfg op_block_dim;          /* (only for autonomous modes) Dimensions of the output block */
	struct mbr_dim_cfg op_fragment_dim;       /* Output fragment/frame size */
	uint32_t dma_hb_req_queue_size;           /* DMA_HBfX requestor bank queue depth */
	uint32_t dma_int_req_queue_size;          /* DMA_i requestor bank queue depth */
};

struct nci_mbr_global1_cfg {
	struct ipu_dim_cfg input_image;           /* Dimensions of the input image */
};

struct nci_mbr_channel_cfg {
	struct ipu_dma_desc_id_cfg dma_hb;        /* DMA_HBfrX descriptor IDs */
	uint32_t dma_unit_desc_addr;              /* address of the first unit descriptor for DMA_HBfrX */
	uint32_t reserved_0;                      /* unused */
	uint32_t dma_span_desc_addr;              /* address of the first source span descriptor for DMA_HBfrX (span B) */
	uint32_t dma_unit_loc_dst_addr;           /* address of the first destination span descriptor for DMA_HBfrX (span A) */
	uint32_t reserved_1;                      /* unused */
	uint32_t reserved_2;                      /* unused */
	uint32_t dma_terminal_desc_addr;          /* address of the destination terminal descriptor of DMA_HBfrX for buffer 0 in GDC RF */
	uint32_t dma_terminal_desc_buf1_addr;     /* address of the destination terminal descriptor of DMA_HBfrX for buffer 1 in GDC RF */
	uint32_t coord_ratio;                     /* (LDC/Hom/DeW mode) final step down-scaling ratio */
	uint32_t dma_instruction_req_addr;        /* address of the instruction register of DMA_HBfrX requestor bank */
	uint32_t dma_ch_ack_mode_addr;            /* address of the ack_mode register in the channel descriptor of DMA_HBfrX */
	struct ipu_dma_desc_id_cfg dma_int;       /* DMA_i descriptor IDs */
	uint32_t dma_int_unit_width_addr;         /* address of the unit_width register in the unit descriptor for DMA_i */
	uint32_t dma_int_unit_height_addr;        /* address of the unit_height register in the unit descriptor for DMA_i */
	uint32_t dma_int_unit_loc_src_addr;       /* address of the unit_location register in the source span descriptor for DMA_i */
	uint32_t dma_int_unit_loc_dst_addr;       /* address of the unit_location register in the destination span descriptor for DMA_i */
	uint32_t reserved_3;                      /* unused */
	uint32_t dma_int_terminal_a_descr_addr;   /* address of the terminal A descriptor for DMA_i */
	uint32_t dma_int_terminal_b_descr_addr;   /* address of the terminal B descriptor for DMA_i */
	uint32_t reserved_4;                      /* unused */
	uint32_t reserved_5;                      /* unused */
	uint32_t int_dma_instruction_req_addr;    /* address of the instruction register of DMA_i requestor bank */
	uint32_t reserved_6;                      /* unused */
	uint32_t reserved_7;                      /* unused */
	struct mbr_buf_properties ip_buf;         /* input buffer properties */
	uint32_t data_ack_addr;                   /* address where the MBR sends the DATA ACK token */
	uint32_t data_ack_message;                /* unused according to MAS owner */
	uint32_t top_left_offset;                 /* unused according to MAS owner */
	uint32_t cmd;                             /* configuration of MBR BPP, algorithm type, and filter padding */
	uint32_t block_erosion;                   /* maximum erosion level allowed per GDC block */
};

struct nci_mbr_cfg {
	struct nci_mbr_global0_cfg global0_cfg;
	struct nci_mbr_global1_cfg global1_cfg;
	struct nci_mbr_channel_cfg ch_cfg[NCI_MBR_NUM_CHANS];
};

enum nci_mbr_sections {
	NCI_MBR_GLBL0_SECTION = 0,
	NCI_MBR_GLBL1_SECTION,
	NCI_MBR_CHAN0_SECTION,
	NCI_MBR_CHAN1_SECTION,
	NCI_MBR_CHAN2_SECTION,
	NCI_MBR_NUM_SECTIONS
};

#endif /* __NCI_MBR_TYPES_H */
