/**
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

#ifndef __IPU_DEVICE_DEC400_PROPERTIES_FUNC_H
#define __IPU_DEVICE_DEC400_PROPERTIES_FUNC_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "ipu_device_dec400_devices.h"

#if (HAS_DEC400 == 1)
STORAGE_CLASS_INLINE unsigned int
ipu_device_dec400_num_devices(void)
{
	return NCI_DEC400_NUM_DEVICES;
}
STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_base_addr(nci_dec400_device_id dev_id)
{
	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	return ipu_device_dec400_properties[dev_id].regs_base_address;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_reg_config_addr(nci_dec400_device_id dev_id, uint8_t chan)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].reg_config_offset;
	addr += chan;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_reg_exConfig_addr(nci_dec400_device_id dev_id, uint8_t chan)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].reg_exConfig_offset;
	addr += chan;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_reg_buffer_base_addr(nci_dec400_device_id dev_id, uint8_t chan)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].reg_buffer_base_offset;
	addr += chan;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_reg_buffer_cache_addr(nci_dec400_device_id dev_id, uint8_t chan)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].reg_buffer_cache_offset;
	addr += chan;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_reg_buffer_end_addr(nci_dec400_device_id dev_id, uint8_t chan)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].reg_buffer_end_offset;
	addr += chan;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_reg_stride_addr(nci_dec400_device_id dev_id, uint8_t chan)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].reg_stride_offset;
	addr += chan;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].regs_base_address;
	return addr;
}
STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_reg_commit_addr(nci_dec400_device_id dev_id)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].reg_commit_offset;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_gp_reg_axi_id_addr(nci_dec400_device_id dev_id)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].gp_reg_axi_id_offset;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].gp_regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_gp_reg_path_en_addr(nci_dec400_device_id dev_id)
{
	uint32_t addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	addr = ipu_device_dec400_properties[dev_id].gp_reg_path_en_offset;
	addr <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	addr += ipu_device_dec400_properties[dev_id].gp_regs_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_irq_base_addr(void)
{
	return ipu_device_dec400_properties[0].irq_regs_base_address;
}

STORAGE_CLASS_INLINE uint32_t
ipu_device_dec400_get_irq_reg_addr(ipu_device_dec400_irq_register_offsets_t reg)
{
	uint32_t addr = ipu_device_dec400_get_irq_base_addr();

	assert(reg < NUM_DEC400_IRQ_REGISTERS);
	addr += (reg << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT);
	return addr;
}
#endif

#endif /* __IPU_DEVICE_DEC400_PROPERTIES_FUNC_H */
