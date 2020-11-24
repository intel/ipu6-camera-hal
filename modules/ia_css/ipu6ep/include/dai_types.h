/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2013 Intel Corporation.
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

/**
 * @file
 * @brief General header files for type definitions
 */

#ifndef DAI_TYPES_H_
#define DAI_TYPES_H_

/** Types shared by DAI interfaces */

#if defined(__KERNEL__)
#include <linux/types.h>
#else
#include <stdint.h>
#endif

/** \brief An address within a DAI subsystem
 *
 * This will eventually replace the dai_memory_address_t and  dai_subsystem_address_t
 */
typedef uint32_t dai_address_t;

/** \brief Master port id */
typedef int dai_master_port_id_t;

/**
 * \brief Require the existence of a certain type
 *
 * This macro can be used in interface header files to ensure that
 * an implementation define type with a specified name exists.
 */
#define _DAI_REQUIRE_TYPE(T) enum { _DAI_SIZEOF_##T = sizeof(T) }

#endif  /* DAI_TYPES_H_ */
