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

#ifndef __VIED_NCI_STRM_PACK_PRIVATE_TYPES_H
#define __VIED_NCI_STRM_PACK_PRIVATE_TYPES_H

typedef union strmpack_en {
#ifndef __HIVECC
	struct {
		unsigned int pack_en : 1; /* register_id: 1, parameter: Bypass */
		unsigned int spare0 : 31;
	}  fields;
#endif /* __HIVECC */
	unsigned int Dword;
} strmpack_en_t;

struct nci_strm_pack_private_cfg {
	strmpack_en_t pack_en;
};

#endif /* __VIED_NCI_STRM_PACK_PRIVATE_TYPES_H */
