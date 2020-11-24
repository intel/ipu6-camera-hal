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

#ifndef __NCI_DEC400_VPLESS_DEFS_H
#define __NCI_DEC400_VPLESS_DEFS_H

enum nci_dec400_fill_sections {
	NCI_DEC400_FILL_SECTIONS_0,
	NCI_DEC400_FILL_SECTIONS_CONFIG = NCI_DEC400_FILL_SECTIONS_0,
	NCI_DEC400_FILL_SECTIONS_1,
	NCI_DEC400_FILL_SECTIONS_EX_CONFIG = NCI_DEC400_FILL_SECTIONS_1,
	NCI_DEC400_FILL_SECTIONS_2,
	NCI_DEC400_FILL_SECTIONS_STRIDE = NCI_DEC400_FILL_SECTIONS_2,
	NCI_DEC400_FILL_SECTIONS_3,
	NCI_DEC400_FILL_SECTIONS_BUFFER_BASE = NCI_DEC400_FILL_SECTIONS_3,
	NCI_DEC400_FILL_SECTIONS_4,
	NCI_DEC400_FILL_SECTIONS_BUFFER_END = NCI_DEC400_FILL_SECTIONS_4,
	NCI_DEC400_FILL_SECTIONS_5,
	NCI_DEC400_FILL_SECTIONS_BUFFER_CACHE = NCI_DEC400_FILL_SECTIONS_5,
	NCI_DEC400_FILL_SECTIONS_6,
	NCI_DEC400_FILL_SECTIONS_COMMIT = NCI_DEC400_FILL_SECTIONS_6,
	NCI_DEC400_FILL_SECTIONS_NOF
};

#define NCI_DEC400_FILL_SECTIONS_CHANNEL 3
#define NCI_DEC400_FILL_SECTIONS_BUFFER 4

#define NCI_DEC400_FILL_LOAD_SECTION_NOF NCI_DEC400_FILL_SECTIONS_3
#define NCI_DEC400_FILL_CONNECT_SECTION_NOF (NCI_DEC400_FILL_SECTIONS_NOF - NCI_DEC400_FILL_LOAD_SECTION_NOF)

#endif /* __NCI_DEC400_VPLESS_DEFS_H */
