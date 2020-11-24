/**
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

#ifndef __NCI_DEC400_STRUCT_H
#define __NCI_DEC400_STRUCT_H

#include "nci_dec400_defs.h"

struct nci_dec400_chip_info_t {
	uint32_t chip_rev;
	uint32_t chip_date;
	uint32_t chip_patch_revision;
	uint8_t engine_dec;
	uint8_t one_set_bus;
	uint8_t two_set_bus;
	uint8_t interface_ahb;
	uint8_t one_gpu;
	uint8_t bus64_bits;
	uint8_t bus128_bits;
	uint8_t dec;
	uint8_t dec_d;
	uint8_t dec_e;
	uint8_t grade_level;
	uint8_t product_type;
	uint16_t product_num;
};

union nci_dec400_config_stream_mode {
	enum nci_dec400_config_read_stream_mode read;
	enum nci_dec400_config_write_stream_mode write;
	uint32_t data;
};

struct nci_dec400_stream_cfg_t {
	uint16_t cbsr_width;
	enum nci_dec400_config_bit_depth bit_depth;
	bool tile_y;
	union nci_dec400_config_stream_mode stream_mode;
	bool ts_cache_mode;
	enum nci_dec400_config_cache_replacement pixel_cache_replacement;
	bool intel_p010;
	enum nci_dec400_config_cache_replacement ts_cache_replacement;
	bool compression_enable;
	enum nci_dec400_config_compression_format compression_format;
	enum nci_dec400_config_compression_align_mode compression_align_mode;
	enum nci_dec400_config_tile_align_mode tile_align_mode;
	enum nci_dec400_config_tile_mode tile_mode;
	uint32_t buffer_base;
	uint32_t cache_base;
	uint32_t buffer_end;
	uint32_t buffer_stride;
};

struct nci_dec400_control_cfg_t {
	bool disable_compression;
	bool disable_ram_clock_gating;
	bool disable_debug_registers;
	bool disable_hw_flush;
	bool clk_dis;
	bool enable_burst_split;
	bool enable_end_address_range_check;
	uint8_t tile_status_read_id;
	uint8_t tile_status_write_id;
	uint16_t axuser;
	uint32_t disable_module_clock_gating;
	uint32_t axqos;
};

struct nci_dec400_extended_debug_info_t {
	uint32_t stream_id;
	uint32_t counter_id;
	uint32_t total_writes_in;
	uint32_t total_writes_out;
	uint32_t debug4;
	uint32_t debug5;
	uint32_t debug6;
	uint32_t debug7;
};

struct nci_dec400_debug_info_t {
	uint32_t tile_status_debug;
	uint32_t encoder_debug;
	uint32_t decoder_debug;
	uint32_t total_reads_in;
	uint32_t total_writes_in;
	uint32_t total_read_bursts_in;
	uint32_t total_write_bursts_in;
	uint32_t total_reads_req_in;
	uint32_t total_writes_req_in;
	uint32_t total_read_lasts_in;
	uint32_t total_write_lasts_in;
	uint32_t total_reads_out;
	uint32_t total_writes_out;
	uint32_t total_read_bursts_out;
	uint32_t total_write_bursts_out;
	uint32_t total_reads_req_out;
	uint32_t total_writes_req_out;
	uint32_t total_read_lasts_out;
	uint32_t total_write_lasts_out;
	uint32_t debug0;
	uint32_t debug1;
	uint32_t debug2;
	uint32_t debug3;
};

struct nci_dec400_register {
	uint32_t val;
};

#endif /* __NCI_DEC400_STRUCT_H */
