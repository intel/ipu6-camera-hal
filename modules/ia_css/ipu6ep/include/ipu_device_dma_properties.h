/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2017 Intel Corporation.
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

#ifndef __IPU_DEVICE_DMA_PROPERTIES_H
#define __IPU_DEVICE_DMA_PROPERTIES_H

#include "storage_class.h"
#include "ipu_device_dma_type_properties.h"

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_num_devices(void);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_nativecio_2dblock(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_nativecio_raccept(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_nativecio_srmd(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_request_banks(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_units(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_unit_banks(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_spans(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_span_banks(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_terminals(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_terminal_banks(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_channels(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_channel_banks(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_macro_size(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_instructions_pending(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_operations_pending(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_unit_width(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_unit_height(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_span_width(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_span_height(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_region_width(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_region_height(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_completed_count(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_element_init_data_bits(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_global_sets(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_linear_burst_size(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_block_width(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_block_height(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_padding_amount(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_command_queue_depth(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_request_queue_depth(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_sub_sampling_factor(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_srmd_support(const unsigned int dev_id, const unsigned int master_bank);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_burst_support(const unsigned int dev_id, const unsigned int master_bank);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_region_stride(const unsigned int dev_id, const unsigned int master_bank);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_element_precisions(const unsigned int dev_id, const unsigned int master_bank);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_sub_sampling_factors(const unsigned int dev_id, const unsigned int master_bank);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_info_width(const unsigned int dev_id, const unsigned int master_bank);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_base_address(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_data_master_data_width(const unsigned int dev_id, const unsigned int master_bank);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_control_master_data_width(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_control_slave_data_width(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_bank_mode_register_available(const unsigned int dev_id);

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_region_stride_register_available(const unsigned int dev_id);

#include "ipu_device_dma_properties_func.h"

#endif /* __IPU_DEVICE_DMA_PROPERTIES_H */
