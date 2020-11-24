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
 * @brief Defines types for the DAI property interface
 */

#ifndef DAI_DEVICE_INFO_TYPES_H_
#define DAI_DEVICE_INFO_TYPES_H_

#if defined(__KERNEL__)
#include <linux/types.h>
#else
#include <stdlib.h>
#include <stdint.h>
#endif

typedef uint32_t dai_property_value_t;		//!< type definition for property value
typedef uint8_t  dai_property_value8_t;		//!< type definition for an 8 bit property value
typedef uint16_t dai_property_value16_t;	//!< type definition for a 16 bit property value
typedef uint32_t dai_property_value32_t;	//!< type definition for a 32 bit property value

#endif  /* DAI_DEVICE_INFO_TYPES_H_ */
