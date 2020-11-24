/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2015 - 2016 Intel Corporation.
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

#ifndef __IPU_DEVICE_OFS_PROPERTIES_STRUCT_H
#define __IPU_DEVICE_OFS_PROPERTIES_STRUCT_H

#include "type_support.h"
#include "ipu_device_ofs_devices.h"

struct ipu_device_ofs_sc_prop_s {
	uint32_t base_address;
	uint32_t elements_per_vector;
	uint32_t num_polyphase_taps;
	uint32_t num_phases;
	uint32_t input_data_bpp;
	uint32_t output_data_bpp;
	uint32_t inter_data_bpp;
	uint32_t max_bits_per_coef;
	uint32_t of_to_scaler_rel_msg_q_addr;
	uint32_t scaler_to_of_rdy_msg_q_addr;
};

/* This data structure lists all properties of one OFS_OF */
/* To be used subsystem to store OFS_OF properties */
struct ipu_device_ofs_of_prop_s {
	/* The base address of the Output Formatter block of the subsystem we run on. */
	uint32_t base_address;
};

/* These properties are of OFS external Buffers */
struct ipu_device_ofs_buf_prop_s {
	uint32_t databus_base_addr;
};

struct ipu_device_ofs_prop_s {
	struct ipu_device_ofs_of_prop_s of[IPU_DEVICE_OFS_NUM_OF];
	struct ipu_device_ofs_sc_prop_s sc[IPU_DEVICE_OFS_NUM_SC];
	struct ipu_device_ofs_buf_prop_s buffer[IPU_DEVICE_OFS_NUM_BUFFER];
};
#endif /* __IPU_DEVICE_OFS_PROPERTIES_STRUCT_H */
