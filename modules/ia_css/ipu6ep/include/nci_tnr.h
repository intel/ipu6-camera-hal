/*
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

#ifndef __NCI_TNR_H
#define __NCI_TNR_H

#include "nci_tnr_types.h"
#include "nci_tnr_storage_class.h"

NCI_TNR_STORAGE_CLASS_H
void nci_tnr_print_config(
	void *buffer);

NCI_TNR_STORAGE_CLASS_H
uint32_t nci_tnr_get_nof_sections(void);

NCI_TNR_STORAGE_CLASS_H
uint32_t nci_tnr_get_sizeof_section(enum nci_tnr_sections section);

NCI_TNR_STORAGE_CLASS_H
uint32_t nci_tnr_get_sizeof_blob(void);

NCI_TNR_STORAGE_CLASS_H
void nci_tnr_print_config(void *buffer);

/* host or cell specific interface */
#include "nci_tnr_local.h"

#ifdef __INLINE_NCI_TNR__
#include "nci_tnr_inline.h"
#endif

#endif /* __NCI_TNR_H */
