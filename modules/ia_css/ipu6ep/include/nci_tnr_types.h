/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __NCI_TNR_TYPES_H
#define __NCI_TNR_TYPES_H

#include "ipu_buf_cfg_types.h"
#include "ipu_event_cfg_types.h"
#include "ipu_dim_cfg_types.h"

/* Horizontal output block size in elements, independent of the format */
#define NCI_TNR_REF_OXDIM                      (128)
/* Vertical output block size in elements, independent of the format */
#define NCI_TNR_REF_OYDIM                      (32)
/* Down-scale factor from Luma to Chroma */
#define NCI_TNR_CHROMA_SUBSAMPLE_SHIFT         (1)
/* Horizontal spatial output block size */
#define NCI_TNR_RECSIM_BUF_WIDTH               (32)
/* Vertical spatial output block size */
#define NCI_TNR_RECSIM_BUF_HEIGHT              (8)
/* Number of DMA channels needed for writing the reference output (data + recursive similaritie) */
#define NCI_TNR_DMA_NUM_CHANS                  (3)
/* Number of buffers in Transfer VMEM for writing the reference output */
#define NCI_TNR_NUM_BUF                        (2)

#define NCI_TNR_REF_OUT_ACK_DATA               (1)

/* WARNING: DON'T CHANGE THE ORDER OF THE FIELDS AS THEY REPRESENT THE REGISTER LAYOUT. */

/**
 * Wrapper for input buffer parameters
 */
struct tnr_buf_cfg {
	struct ipu_buf_cfg buff_cfg;                  /* Consumer side buffer configuration */
	struct ipu_buf_properties buff_y;             /* Consumer side buffer properties of the Y-plane */
	struct ipu_buf_properties buff_u;             /* Consumer side buffer properties of the U-plane */
	uint32_t buff_v_addr;                         /* Consumer side buffer address of the V-plane */
	uint32_t reserved_0;
	uint32_t reserved_1;
};

/**
 * Wrapper for reference buffer parameters.
 */
struct tnr_ref_buf_cfg {
	uint32_t num_buf;                              /* Nof ref buffers, currently only supported value is 2 */
	struct ipu_buf_properties y;                   /* Luma plane */
	struct ipu_buf_properties uv;                  /* Chroma plane */
	struct ipu_buf_properties recsim;              /* Recursive similarity */

};

/**
 * Wrapper for reserved register IDs 66 - 85
 */
struct tnr_reserved_registers {
	uint32_t reserved_A;
	uint32_t reserved_B;
	uint32_t reserved_C;
	uint32_t reserved_D;
	uint32_t reserved_E;
	uint32_t reserved_F;
	uint32_t reserved_G;
	uint32_t reserved_H;
	uint32_t reserved_I;
	uint32_t reserved_J;
	uint32_t reserved_K;
	uint32_t reserved_L;
	uint32_t reserved_M;
	uint32_t reserved_N;
	uint32_t reserved_O;
	uint32_t reserved_P;
	uint32_t reserved_Q;
	uint32_t reserved_R;
	uint32_t reserved_S;
	uint32_t reserved_T;
};

/**
 * struct nci_tnr_cfg, as a blob, will be filled by PG Control Init code of the TNR.
 * It will be written by Program Load to the HW.
 */
struct nci_tnr_cfg {
	uint32_t ip_buff_ratio_converter;             /* Input buffer Ratio Converter, default is 3 (HAS) */
	struct tnr_buf_cfg ip_buff_cfg;               /* Input buffer parameters */
	struct tnr_buf_cfg op_buff_cfg;               /* Output buffer parameters */
	uint32_t ip_ref_mbr_TL_grid_start_addr;       /* Address where TNR (PME) stores top-left tetragon point */
	uint32_t ip_ref_mbr_BL_grid_start_addr;       /* Address where TNR (PME) stores bottom-left tetragon point */
	uint32_t reserved_0;                          /* Reserved register */
	uint32_t reserved_1;                          /* Reserved register */
	uint32_t reserved_2;                          /* Reserved register */
	uint32_t reserved_3;                          /* Reserved register */
	uint32_t ip_ref_mbr_cmd_addr;                 /* Address where the MBR cmd needs to be sent */
	struct tnr_ref_buf_cfg ip_ref_buf_cfg;        /* Input Reference buffer parameters */
	uint32_t dma_channel_descr_id;                /* Use this ID +0 (Y) or +1 (U/V) or +2 (rec.sim) */
	uint32_t dma_req_queue_size;                  /* Size of the DMA requestor queue */
	uint32_t cmd_addr;                            /* Address where the enqueue cmd needs to be sent */
	uint32_t cmd_slim_next;                       /* Token that will be sent to DMA */
	struct tnr_ref_buf_cfg op_ref_buf_cfg;        /* Output reference buffer parameters */
	struct tnr_reserved_registers reserved;       /* Reserved registers according to HAS */
	uint32_t read_post_write_check;               /* Enabled by default, only implemented at the last written block */
	struct ipu_event_cfg fragment;                /* Fragment completed command */
};

enum nci_tnr_sections {
	NCI_TNR_SECTION0 = 0,
	NCI_TNR_NUM_SECTIONS,
};

#endif /* __NCI_TNR_TYPES_H */
