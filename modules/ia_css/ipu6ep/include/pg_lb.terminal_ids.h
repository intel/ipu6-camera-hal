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

#ifndef __PG_LB_TERMINAL_IDS_H
#define __PG_LB_TERMINAL_IDS_H

enum PG_LB_TERMINAL_ID {
	PG_LB_TERMINAL_ID_CACHED_PARAMETER_IN = 0,
	PG_LB_TERMINAL_ID_PROGRAM_INIT,
	PG_LB_TERMINAL_ID_PROGRAM_CONTROL_INIT,
	PG_LB_TERMINAL_ID_GET,
	PG_LB_TERMINAL_ID_PUT,
	PG_LB_TERMINAL_ID_GAMMASTAR_GRID,
	PG_LB_TERMINAL_ID_SPT_DVS_MV_OUT_L0,
	PG_LB_TERMINAL_ID_SPT_DVS_MV_OUT_L1,
	PG_LB_TERMINAL_ID_SPT_DVS_MV_OUT_L2,
	PG_LB_TERMINAL_ID_DVS_FE_IN_L0,
	PG_LB_TERMINAL_ID_DVS_FE_IN_L1,
	PG_LB_TERMINAL_ID_DVS_FE_IN_L2,
	PG_LB_TERMINAL_ID_DVS_FE_OUT_L0,
	PG_LB_TERMINAL_ID_DVS_FE_OUT_L1,
	PG_LB_TERMINAL_ID_DVS_FE_OUT_L2,
	PG_LB_TERMINAL_ID_N
};

#endif /* _PG_LB_TERMINAL_IDS_H */
