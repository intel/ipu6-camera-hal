/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2018 Intel Corporation.
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

#ifndef __VIED_NCI_DMA_TYPES_H
#define __VIED_NCI_DMA_TYPES_H

#include "type_support.h"
#include "storage_class.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_type_properties.h"
#include "vied_nci_dma_global_defs.h"
#include "vied_nci_dma_global_defs_dep.h"
#include "vied_nci_dma_types_dep.h"

#ifndef STRUCT_TYPE
#	if defined (__HIVECC) && defined(__INLINE_NCI_DMA__)
#		define STRUCT_TYPE register struct
#	else
#		define STRUCT_TYPE struct
#	endif
#endif

struct nci_dma_chan_res {
	uint16_t chan_id;
	uint16_t unit_id;
	uint16_t terminal_A_id;
	uint16_t terminal_B_id;
	uint16_t span_A_id;
	uint16_t span_B_id;
	uint16_t requestor_id;
	uint32_t chan_addr;
	uint32_t unit_addr;
	uint32_t terminal_A_addr;
	uint32_t terminal_B_addr;
	uint32_t span_A_addr;
	uint32_t span_B_addr;
	uint32_t requestor_addr;
};

struct nci_dma_request_desc {
	uint32_t descriptor_id_setup_1;
	uint32_t descriptor_id_setup_2;
};

struct nci_dma_chan {
	struct nci_dma_request_desc request_descr;
#if NCI_DMA_TERMINAL_REPLACEMENT_POLICY_BITS == 0
	uint32_t invalidate_instruction_terminal_A_B;
	uint32_t invalidate_instruction_span_A_B;
	uint32_t invalidate_instruction_unit;
#endif
	uint8_t dev_id;
	uint8_t chan_id;
	uint8_t requestor_id;
};

enum nci_dma_terminal {
	NCI_DMA_TERMINAL_A = 0,
	NCI_DMA_TERMINAL_B = 1,
};
#define N_NCI_DMA_TERMINAL 0x2

enum nci_dma_ack_mode {
	NCI_DMA_ACK_MODE_PASSIVE,
	NCI_DMA_ACK_MODE_ACTIVE
};
#define N_NCI_DMA_ACK_MODE 0x2

enum nci_dma_element_setup_fw {
	NCI_DMA_FW_ELEMENT_PRECISION_32BIT
};
#define N_NCI_DMA_ELEMENT_SETUP_FW 0x1

enum nci_dma_element_setup_int {
	NCI_DMA_INT_ELEMENT_PRECISION_8BIT,
	NCI_DMA_INT_ELEMENT_PRECISION_16BIT,
};
#define N_NCI_DMA_ELEMENT_SETUP_INT 0x2

/**
 * Structure to hold partial descriptor information.
 */
struct nci_dma_partial_terminal_desc_config {
	/*
	 * In GLV, the address from EXT and INT DMA are different
	 * Have to add cached_terminal_region_origin_A for EXT1R DMA
	 */
	uint32_t cached_terminal_region_origin_A;  /**< start address of a region in memory */
	uint32_t terminal_region_origin_A;  /**< start address of a region in memory */
	uint32_t terminal_region_origin_B;  /**< start address of a region in memory */
	uint32_t terminal_region_width;   /**< width of the region in elements @b -1 */
	uint32_t terminal_region_stride;  /**< stride of the region in bytes @b*/
};

/**
 * Structure to hold partial terminal descriptor information.
 * in case of __HIVECC && __INLINE_NCI_DMA__ it uses register struct
 */
typedef STRUCT_TYPE {

	/*
	 * This structure is used when reconfiguring terminals
	 * at block level
	 */
	uint32_t terminal_region_origin_A;  /**< start address of a region in memory */
	uint32_t terminal_region_origin_B;  /**< start address of a region in memory */
	uint32_t terminal_region_width;   /**< width of the region in elements @b -1 */
	uint32_t terminal_region_stride;  /**< stride of the region in bytes @b*/
} nci_dma_partial_terminal_desc_config_t;

/**
 * DMA4 device type supports global sets to set the maximum block width
 * in the global descriptor. The DMA channel descriptor has a register to
 * choose one of these global sets. We choose the set '0' (default) for uncompressed
 * transfers and set '1' for compressed transfers. The following enum makes this
 * definition more explicit.
 */
enum nci_dma_global_set {
	NCI_DMA_GLOBAL_SET_WITHOUT_COMPRESSION = 0,
	NCI_DMA_GLOBAL_SET_WITH_COMPRESSION = 1,
	NCI_DMA_GLOBAL_SET_8BPE_COMPRESSION = NCI_DMA_GLOBAL_SET_WITH_COMPRESSION,
	NCI_DMA_GLOBAL_SET_16BPE_COMPRESSION = 2
};

enum nci_dma_element_setup_isl {
	NCI_DMA_ISL_ELEMENT_PRECISION_8BIT,
	NCI_DMA_ISL_ELEMENT_PRECISION_16BIT,
};
#define N_NCI_DMA_ELEMENT_SETUP_ISL 0x2

/* Do not change the order of the fields of this enum.
 * There is code that depends on it.
 */
enum nci_dma_span_order {
	NCI_DMA_SPAN_ORDER_ROW_FIRST,
	NCI_DMA_SPAN_ORDER_COLUMN_FIRST
};
#define N_NCI_DMA_SPAN_ORDER 0x2

/* Do not change the order of the fields of this enum.
 * There is code that depends on it.
 */
enum nci_dma_addressing_mode {
	NCI_DMA_ADDRESSING_MODE_BYTE_ADDRESS_BASED,
	NCI_DMA_ADDRESSING_MODE_COORDINATE_BASED
};
#define N_NCI_DMA_ADDRESSING_MODE 0x2

enum nci_dma_span_mode {
	NCI_DMA_SPAN_MODE_ROW_FIRST_BYTE_ADDRESS_BASED,
	NCI_DMA_SPAN_MODE_ROW_FIRST_COORDINATE_BASED,
	NCI_DMA_SPAN_MODE_COLUMN_FIRST_BYTE_ADDRESS_BASED,
	NCI_DMA_SPAN_MODE_COLUMN_FIRST_COORDINATE_BASED
};
#define N_NCI_DMA_SPAN_MODE 0x4

enum nci_dma_instruction_format {
	NCI_DMA_INSTRUCTION_FORMAT_EXECUTE,
	NCI_DMA_INSTRUCTION_FORMAT_INVALIDATE
};
#define N_NCI_DMA_INSTRUCTION_FORMAT 0x2

/* Do not change the order of the fields of this enum.
 * There is code that depends on it.
 */
enum nci_dma_transfer_direction {
	NCI_DMA_TRANSFER_DIRECTION_AB,
	NCI_DMA_TRANSFER_DIRECTION_BA
};
#define N_NCI_DMA_TRANSFER_DIRECTION 0x2

enum nci_dma_transfer_kind {
	NCI_DMA_TRANSFER_KIND_INIT,
	NCI_DMA_TRANSFER_KIND_MOVE
};
#define N_NCI_DMA_TRANSFER_KIND 0x2

enum nci_dma_command_type {
	NCI_DMA_MOVE_CMD_TYPE,
	NCI_DMA_FILL_A_CMD_TYPE,
	NCI_DMA_FILL_B_CMD_TYPE
};
#define N_NCI_DMA_COMMAND_TYPE 0x3

enum nci_dma_request_type {
	NCI_DMA_DEDICATED_REQUESTOR,
	NCI_DMA_SHARED_REQUESTOR
};
#define N_NCI_DMA_REQUEST_TYPE 0x2

struct nci_dma_global_desc {
	uint32_t unit_descriptor_base_addr;
	uint32_t span_descriptor_base_addr;
	uint32_t terminal_descriptor_base_addr;
	uint32_t channel_descriptor_base_addr;
	uint32_t max_block_height;
	uint32_t max_1d_block_width[NCI_DMA_MAX_GLOBAL_SETS];
	uint32_t max_2d_block_width[NCI_DMA_MAX_GLOBAL_SETS];
};

struct nci_dma_master_desc {
	uint32_t srmd_support;
	uint32_t burst_support;
	uint32_t max_stride;
};

/** Enum for burst support register values of the data masters
 *  Platforms 4 and 4P offer three different burst modes. These burst modes are not available on every DMA instance.
 *  There are 3 different modes: No burst support, 1D burst support, and 2D burst support. To see whether a particular
 *  instance supports a specific burst mode, consult the .zen file in the SDK.
 *
 *  Register value 0x2 is skipped/unused as specified in the DMA4 manual.
 *
 *  For more information on mode details, consult the DMA4 manual, Burst support register section.
 */
enum nci_dma_burst_support {
	NCI_DMA_BURST_NO_SUPPORT,		/**< No burst support. Single word transfers only */
	NCI_DMA_BURST_1D_SUPPORT,		/**< 1D burst support */
	/* skipped, value 0x2 is unused */
	NCI_DMA_BURST_2D_SUPPORT = 0x3	/**< 2D burst support */
};
#define N_NCI_DMA_BURST_SUPPORT 0x4

enum nci_dma_resource_type {
	NCI_DMA_RESOURCE_EXT,
	NCI_DMA_RESOURCE_INT,
	NCI_DMA_NUM_RESOURCE_TYPES
};
#define N_NCI_DMA_RESOURCE_TYPE 0x3

struct nci_dma_descriptor_addresses {
	uint32_t channel_descriptor_offset;
	uint32_t span_descriptor_offset;
	uint32_t terminal_descriptor_offset;
	uint32_t unit_descriptor_offset;
	uint32_t cmem_descriptor_base_address;
};

struct nci_dma_dev {
	enum nci_dma_device_id dev_id;
	enum nci_dma_bank_mode bank_mode;
	enum nci_dma_request_type request_type;
	struct nci_dma_descriptor_addresses descriptor_addresses;
	struct nci_dma_chan *chans;
	uint32_t next_one_instruction[N_NCI_DMA_COMMAND];

	/**
	 * These channel banks are reserved to be used exclusively in non-cache mode
	 * with cached span, unit and terminal descriptors.
	 */
	uint32_t num_of_reserved_channel_banks;
};

struct nci_dma_dev_config {
	enum nci_dma_bank_mode bank_mode;
	enum nci_dma_request_type request_type;
	struct nci_dma_descriptor_addresses descriptor_addresses;
	struct nci_dma_chan *chans;
	uint32_t num_of_reserved_channel_banks;
	/**
	 * Configuration for Global descriptor registers
	 * Not all registers under global descriptors are exposed, but the
	 * ones which we want to configure based on usecase.
	 * The rest of the registers are configured to default values set
	 * in DPI.
	 */
	uint32_t max_block_height;
	uint32_t max_1d_block_width[NCI_DMA_MAX_GLOBAL_SETS];
	uint32_t max_2d_block_width[NCI_DMA_MAX_GLOBAL_SETS];
	uint32_t read_req_addr;
	uint32_t read_req_enable;
	uint32_t read_req_sidpid;
};

struct nci_dma_transfer_config {
	struct nci_dma_channel_desc channel_desc;
	enum nci_dma_requestor_id requestor_id;
};

struct nci_dma_mixed_chan {
	/*Single Physical channel*/
	uint8_t physical_chan_id;
	struct nci_dma_chan physical_chan;

	/*Multiple cached channels*/
	uint8_t no_of_cached_descriptors;
	struct nci_dma_chan *cached_chans[NCI_DMA_MIXED_MAX_NO_OF_CACHED_DESCRIPTORS];
};

struct nci_dma_terminal_config {
	struct nci_dma_terminal_desc terminal_desc[NCI_DMA_MAX_NUM_PORTS];
	struct nci_dma_span_desc span_desc[NCI_DMA_MAX_NUM_PORTS];
	struct nci_dma_unit_desc unit_desc;
};

struct nci_dma_memcpy_info {
	uint32_t region_origin;
	uint32_t region_width;
	uint32_t region_stride;
	uint32_t cio_info_setup;
};

struct nci_dma_memcpy_config {
	struct nci_dma_memcpy_info memcpy_info[NCI_DMA_MAX_NUM_PORTS];
	struct nci_dma_unit_desc unit_desc;
};

struct nci_dma_memcpy_2d_info {
	uint32_t region_origin;
	uint32_t region_width;
	uint32_t region_stride;
	uint32_t x_coordinate;
	uint32_t cio_info_setup;
};

struct nci_dma_memcpy_2d_config {
	struct nci_dma_memcpy_2d_info memcpy_info[NCI_DMA_MAX_NUM_PORTS];
	struct nci_dma_unit_desc unit_desc;
};

struct nci_dma_memcpy_1d_config {
	/* partial terminal */
	uint32_t region_origin[NCI_DMA_MAX_NUM_PORTS];
	uint32_t region_width[NCI_DMA_MAX_NUM_PORTS];
	/* partial span */
	uint32_t x_coordinate[NCI_DMA_MAX_NUM_PORTS];
	/* unit */
	struct nci_dma_unit_desc unit_desc;
};

/**
 * Descriptor Info required by MBR
 */
struct nci_dma_chan_res_addr {
	uint32_t dma_unit_width_addr;
	uint32_t dma_unit_height_addr;
	uint32_t dma_unit_location_src_addr;
	uint32_t dma_unit_location_dst_addr;
	uint32_t dma_req_instruction_addr;
	uint32_t dma_req_setup_desc_1_addr;
	uint32_t dma_req_setup_desc_2_addr;
	uint32_t dma_channel_ack_mode_addr;
};

STORAGE_CLASS_INLINE uint32_t get_max_value(uint32_t bits)
{
	uint32_t max_value;

	if (32 == bits) {
		max_value = 0xFFFFFFFF;
	} else {
		max_value = (1 << bits) - 1;
	}
	return max_value;
}

#endif /* __VIED_NCI_DMA_TYPES_H */
