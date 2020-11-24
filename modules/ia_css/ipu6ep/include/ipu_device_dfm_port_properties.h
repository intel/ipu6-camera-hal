/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2017 Intel Corporation.
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

#ifndef __IPU_DEVICE_DFM_PORT_PROPERTIES_H
#define __IPU_DEVICE_DFM_PORT_PROPERTIES_H

#include "ipu_device_dfm_properties_defs.h"
#include "type_support.h"
#include "dai_ipu_top_devices.h"
#include "ipu_nci_dfm_struct.h" /* for ipu_device_dfm_seq_type_idx */
#include "storage_class.h"

#define IPU_DEVICE_DFM_NUM_OF_DEVICES       (NUM_DAI_IPU_TOP_DFM_INSTANCES)
#define IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS  (32)
#define IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR    (2)

#define IPU_DEVICE_DFM_EMPTY_PORT_START_ID      (0)
#define IPU_DEVICE_DFM_FULL_PORT_START_ID       (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS)

#define IPU_DEVICE_DFM_PORT_BEGIN_CMD_MASK  (0x3)
#define IPU_DEVICE_DFM_PORT_MIDDLE_CMD_MASK (0xC)
#define IPU_DEVICE_DFM_PORT_END_CMD_MASK    (0x30)

/* DFM command file next command token offset */
#define IPU_DEVICE_DFM_CF_NEXT_CMD_TOKEN_OFFSET	(2 * sizeof(uint32_t))

#define IPU_DEVICE_DFM_MAX_NUM_CMD_TOKENS (2)
#define IPU_DEVICE_DFM_MAX_NUM_SEQUENCE_TYPES (3)

/* The Capability of a device port in the DFM:-
 * Every port has different capability:
 * 1. how many sequence it can handle.
 * 2. How many command token can be send.
 * The SDK provides 16 bits numbers for each sequence
 * which encode the capability information encoded
 * for 8 ports. In the number, every 2 bits tells the
 * number of commands token a port can handle.
 * Following array code encode seq and commands token
 * a port can handle in 6 bit number.
 *            __________________________________________________
 * Bits      |   5    |   4   |   3    |   2   |   1    |   0   |
 *           +--------------------------------------------------+
 *           |NO. OF COMMANDS |NO. OF COMMANDS | NO. OF COMMANDS|
 * meaning   |   END SEQ      |   MIDDLE SEQ   |   BEGIN SEQ    |
 *           +--------------------------------------------------+
 */
#define IPU_DEVICE_DFM_PORT_BEGIN_NUM_CMD_BIT_POS   (0)
#define IPU_DEVICE_DFM_PORT_MIDDLE_NUM_CMD_BIT_POS  (2)
#define IPU_DEVICE_DFM_PORT_END_NUM_CMD_BIT_POS     (4)

/**
* @brief Get the number of instantiated port pairs for a given device instance.
* @param dev_id	Device id
* @return	number of instantiated port pairs for the given device instance
*/
uint32_t ipu_device_dfm_get_num_ports(const uint32_t dev_id);

/**
 * @brief Get the size of the sequence type (in bytes) for the given device port.
 * @param dev_id	Device id
 * @param port_num	Port number
 * @param seq		Sequence type
 * @return		Size of the sequence type (in bytes) for the given device port
 */
STORAGE_CLASS_INLINE uint8_t ipu_device_dfm_port_properties_get_cmd_seq_size(const uint32_t dev_id,
		const uint32_t port_num,
		const ipu_device_dfm_seq_type_idx seq);

/**
 * @brief Get the size of the command bank descriptor (in bytes) for the given device port.
 * @param dev_id	Device id
 * @param port_num	Port number
 * @return		Size of the command bank descriptor (in bytes) for the given device port
 */
STORAGE_CLASS_INLINE uint8_t ipu_device_dfm_port_properties_get_cmdbank_config_size(const uint32_t dev_id,
		const uint32_t port_num);

/**
 * @brief Retrieve whether a port is an empty port.
 * @param port_num	Number of the port
 * @return true if the port is an empty port
 */
STORAGE_CLASS_INLINE bool ipu_device_dfm_port_properties_is_empty_port(const uint32_t port_num);

/**
 * @brief Check whether a port is instantiated on a DFM device.
 *
 * @param dev_id	DFM device id
 * @param port_num	Port number
 *
 * @return true if the port is instantiated
 */
STORAGE_CLASS_INLINE bool ipu_device_dfm_port_properties_is_port_instantiated(
		const uint32_t dev_id,
		const uint32_t port_num);

#include "ipu_device_dfm_port_properties_func.h"

#endif /* __IPU_DEVICE_DFM_PORT_PROPERTIES_H */
