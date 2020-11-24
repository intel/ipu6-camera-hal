/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2013 - 2016 Intel Corporation.
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

/*===========================================================================*/
/** @brief <b> The NCI interfaces will not do any error checking and so will not
 * return an error code. We use only ASSERTS to do range checking. For more
 * information please read the NCI specification document </b>
 */
/*===========================================================================*/
#ifndef __VIED_NCI_EQ_DEVICE_H
#define __VIED_NCI_EQ_DEVICE_H

#include "vied_nci_eq_types.h"
#include "vied_nci_eq_storage_class.h"

/** Configures the event queue device
 *
 * Configures the event queue device with device specific configuration, queue
 * specific configuration (priority configurations will be added once the hardware
 * bug is solved, please refer to EQ NCI specification).
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance.
 * @param[in]	*dconf		nci_eq_device_config, event queue device
 * configuration data.
 * @param[in]	*qconf		nci_eq_queue_config, event queue queue
 * configuration data.
 *
 * @return 	void
 */
NCI_EQ_STORAGE_CLASS_H void nci_eq_device_configure(
	enum nci_eq_device_id eq_id,
	const struct nci_eq_device_config *dconf,
	const struct nci_eq_queue_config *qconf
	/* priority is not used in the current EQ NCI
	 * implementation, but is planned to be added in future.
	 *
	 * const struct nci_eq_priority_config* pconf */
);

/** Flush the event queue device
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 *
 * @return 	void
 */
NCI_EQ_STORAGE_CLASS_H void nci_eq_device_flush(
	enum nci_eq_device_id eq_id
);

#ifdef __INLINE_NCI_EQ__
#include "vied_nci_eq_device_inline.h"
#endif

#endif /* __VIED_NCI_EQ_DEVICE_H */
