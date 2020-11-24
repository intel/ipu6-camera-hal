/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __NCI_DEC400_INLINE_DEP_H
#define __NCI_DEC400_INLINE_DEP_H

#include <assert_support.h>
#include <type_support.h>

#include "nci_dec400.h"
#include "nci_dec400_struct.h"
#include "nci_dec400_regs_defs.h"
#include "nci_dec400_storage_class.h"
#include "nci_dec400_reg_access.h"
#include "nci_mmu_stream_id.h"

/* This is essentially the CIO_info that the DEC400 uses when writing the Tile Status information to DDR.
 * The stream ID value is taken from the MMU stream ID table and may be different for compression/decompression.
 * The main change vs. IPU5.5 is that the stream ID field has been extended to be 5 bits as the number of supported
 * streams has been extended from 16 to 32.
 */
union nci_dec400_register_axuser_cio_info {
	struct {
		uint16_t snoop: 1; /* [0:0] - 0: non snoopable. 1: snoopable */
		uint16_t flush: 1; /* [1:1] - without invalidate. 0: normal request. 1: request force a TS flush */
		uint16_t pass_through: 1; /* [2:2] - 0: request will not be compressed. 1: compression enabled */
		uint16_t zlw: 1; /* [3:3] 0: normal request. 1: mark trnx as zero length */
		uint16_t stream_id: 5; /* [8:4] stream ID for TS */
		uint16_t request_destination: 1; /* [9:9] 0: buttress. 1: external IOSF */
		uint16_t imr_base_address: 1; /* [10:10] 0: IMR base 1 (FW Read) 1: IMR Base 2 (FW write) */
		uint16_t bypass_io_mmu_translation: 1; /* [11:11] 0: no IMR 1: IMR req */
		uint16_t spare: 4; /* [12:15] */
	} data_bits;
	uint16_t data;
};

NCI_DEC400_STORAGE_CLASS_C
uint16_t nci_dec400_get_control_config_axuser(
	nci_dec400_device_id dev_id)
{
	union nci_dec400_register_axuser_cio_info axuser;

	axuser.data = 0;
	axuser.data_bits.flush = 1;

	if (dev_id == NCI_DEC400_ENCODER) {
		axuser.data_bits.stream_id = NCI_STREAM_ID_PSYS_T2E_TS;
	} else {
		axuser.data_bits.stream_id = NCI_STREAM_ID_PSYS_T2D_TS;
	}
	axuser.data_bits.request_destination = 1;

	return axuser.data;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_config_interrupt_ex_enable(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	bool enable)
{
	uint32_t intEnEx_address;
	uint32_t val;
	uint32_t val_mask;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(channel < IPU_DEVICE_DEC400_NUM_OF_CHANNELS);
	assert(channel > IPU_DEVICE_DEC400_INTR_EN_MAX_CHANNEL);

	val_mask = (1 << (channel - (IPU_DEVICE_DEC400_INTR_EN_MAX_CHANNEL + 1)));

	intEnEx_address = DEC400_REG_GCREG_AHBDECINTR_ENBL_EX;
	intEnEx_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	intEnEx_address += ipu_device_dec400_get_base_addr(dev_id);
	val = nci_dec400_reg_load(intEnEx_address);

	if (enable) {
		val |= val_mask;
	} else {
		val &= (~val_mask);
	}
	nci_dec400_reg_store(intEnEx_address, val);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_config_stream_debug(
	nci_dec400_device_id dev_id,
	uint8_t stream_id,
	uint8_t counter_id)
{
	uint32_t stream_debug_address;
	uint32_t base_address;
	union nci_dec400_register_debug_stream_counter_select stream_debug_cfg;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(stream_id < IPU_DEVICE_DEC400_NUM_OF_CHANNELS);

	base_address = ipu_device_dec400_get_base_addr(dev_id);

	stream_debug_address = DEC400_REG_GCREG_AHBDECDEBUG_STREAM_COUNTER_SELECT;
	stream_debug_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	stream_debug_address += base_address;

	stream_debug_cfg.data_bits.select_stream = stream_id;
	stream_debug_cfg.data_bits.select_counter = counter_id;
	stream_debug_cfg.data_bits.spare0 = 0;

	nci_dec400_reg_store(stream_debug_address, stream_debug_cfg.data);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_read_extended_debug_info(
	nci_dec400_device_id dev_id,
	struct nci_dec400_extended_debug_info_t *info)
{
	uint32_t debug_address;
	uint32_t base_address;
	uint32_t inc_addr;
	union nci_dec400_register_debug_stream_counter_select stream_debug_cfg;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(info != NULL);

	base_address = ipu_device_dec400_get_base_addr(dev_id);
	inc_addr = 1 << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;

	debug_address = DEC400_REG_GCREG_AHBDECDEBUG4;
	debug_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	debug_address += base_address;
	info->debug4 = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->debug5 = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->debug6 = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->debug7 = nci_dec400_reg_load(debug_address);

	debug_address = DEC400_REG_GCREG_AHBDECDEBUG_STREAM_TOTAL_WRITE_IN;
	debug_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	debug_address += base_address;
	info->total_writes_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_writes_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	stream_debug_cfg.data = nci_dec400_reg_load(debug_address);
	info->stream_id = stream_debug_cfg.data_bits.select_stream;
	info->counter_id = stream_debug_cfg.data_bits.select_counter;
}

#endif /* __NCI_DEC400_INLINE_DEP_H */
