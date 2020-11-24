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

#ifndef __IPU_RESOURCES_DEP_H
#define __IPU_RESOURCES_DEP_H

#include "ipu_device_dma_devices.h"
#include "vied_nci_psys_resource_model.h"
#include "dev_api_dfm_types.h"

extern const int32_t resource_model_dev_chn_2_nci_dma_dev[VIED_NCI_N_DEV_CHN_ID];

extern const uint16_t resource_model_dfm_dev_port_num_start[VIED_NCI_N_DEV_DFM_ID];

extern const enum ipu_device_dfm_id resource_model_dfm_dev_2_nci_dfm_dev[VIED_NCI_N_DEV_DFM_ID];

extern const int8_t resource_model_dev_chn_2_connect_section_count[VIED_NCI_N_DEV_CHN_ID];

extern const int8_t resource_model_dfm_dev_is_full_port_type[VIED_NCI_N_DEV_DFM_ID];

#ifdef __INLINE_IPU_RESOURCES__
#include "ipu_resources_dep.c"
#endif

#endif /* __IPU_RESOURCES_DEP_H */
