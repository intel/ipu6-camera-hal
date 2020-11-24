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

#ifndef __IPU_FABRICS_H
#define __IPU_FABRICS_H

enum ipu_fabrics_id {
	IPU_LB_FABRIC,
	IPU_BB_FABRIC,
	IPU_TRANSFER_FABRIC,
	IPU_CENTRAL_FABRIC,
	IPU_ISYS_FABRIC,
	IPU_ISL_FABRIC,
};

#define NUM_OF_IPU_FABRICS      (IPU_ISYS_FABRIC + 1)

#endif /* __IPU_FABRICS_H */
