/*
 * Copyright (C) 2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IPU_DEVICE_ACB_DEVICES_H
#define __IPU_DEVICE_ACB_DEVICES_H

enum ipu_device_acb_id {
	IPU_DEVICE_ACB_B2B_ID,
	IPU_DEVICE_ACB_RYNR_ID,
	IPU_DEVICE_ACB_DEMOSAIC_ID,
	IPU_DEVICE_ACB_ICA_ID,
	IPU_DEVICE_ACB_LSC_ID,
	IPU_DEVICE_ACB_DPC_ID,
	IPU_DEVICE_ACB_R2I_SIE_ID,
	IPU_DEVICE_ACB_R2I_DS_A_ID,
	IPU_DEVICE_ACB_R2I_DS_B_ID,
	IPU_DEVICE_ACB_AWB_ID,
	IPU_DEVICE_ACB_AF_ID,
	IPU_DEVICE_ACB_PAF_ID,
	IPU_DEVICE_ACB_AE_ID,
	IPU_DEVICE_ACB_NUM_ACB
};

#define IPU_DEVICE_ACB_NUM_ISA_ACB (IPU_DEVICE_ACB_NUM_ACB)

#endif /* __IPU_DEVICE_ACB_DEVICES_H */
