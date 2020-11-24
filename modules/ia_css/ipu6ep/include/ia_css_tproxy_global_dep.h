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

#ifndef __V6_IA_CSS_TPROXY_GLOBAL_DEP_H
#define __V6_IA_CSS_TPROXY_GLOBAL_DEP_H

enum ia_css_tproxy_dma_requestor_cell_id {
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_USE_DEFAULT = NCI_DMA_REQUESTOR_ID_0,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_SPC0_CELL = NCI_DMA_REQUESTOR_ID_0,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_SPP0_CELL = NCI_DMA_REQUESTOR_ID_1,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_ISP0_CELL = NCI_DMA_REQUESTOR_ID_2,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_BACKUP_ID_0 = NCI_DMA_REQUESTOR_ID_3,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_BACKUP_ID_1 = NCI_DMA_REQUESTOR_ID_4,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_BACKUP_ID_2 = NCI_DMA_REQUESTOR_ID_5,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_BACKUP_ID_3 = NCI_DMA_REQUESTOR_ID_6,
	IA_CSS_TPROXY_DMA_REQUESTOR_PSYS_BACKUP_ID_4 = NCI_DMA_REQUESTOR_ID_7,
	IA_CSS_TPROXY_DMA_NUM_REQUESTOR_PSYS_ID = NCI_DMA_PSYS_NUM_REQUESTOR_ID
};
#endif /* __V6_IA_CSS_TPROXY_GLOBAL_DEP_H */
