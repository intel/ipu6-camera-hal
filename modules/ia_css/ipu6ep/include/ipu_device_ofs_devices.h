/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_OFS_DEVICES_H
#define __IPU_DEVICE_OFS_DEVICES_H

enum ipu_device_ofs_id {
	IPU_DEVICE_OFS0 = 0
};
#define IPU_DEVICE_OFS_NUM (IPU_DEVICE_OFS0 + 1)

/* OFS pins */
enum ipu_device_ofs_pin_id {
	IPU_DEVICE_OFS_PIN_MAIN = 0,/* Can be used for scaling */
	IPU_DEVICE_OFS_PIN_DISPLAY,/* Cannot be used for scaling */
	IPU_DEVICE_OFS_PIN_PP/* Can be used for scaling */
};

#define IPU_DEVICE_OFS_NUM_PINS (IPU_DEVICE_OFS_PIN_PP + 1)

#define IPU_DEVICE_OFS_PIN_TO_PID_OFFSET (1)    /* In OFS, PID is used to identify the pin but not
						 * device itself unlike other components.
						 * Process id can be mapped directly to the pin id
						 * The pid is 0 by default, therefore,
						 * pid should be > 0 for any active process and
						 * equal to 0 for inactive. This way there is no need
						 * to set pid for any inactive process or pin.
						 * In order to do that offset required to
						 * map pin id <-> pid (= pid offset +/- pin id).
						 * */
/* OFS OF devices in PSYS */
enum ipu_device_ofs_of_id {
	IPU_DEVICE_OFS_OF0 = 0,
	IPU_DEVICE_OFS_OF1,
	IPU_DEVICE_OFS_OF2
};
#define IPU_DEVICE_OFS_NUM_OF (IPU_DEVICE_OFS_OF2 + 1)

/* OFS SC devices in PSYS */
enum ipu_device_ofs_sc_id {
	IPU_DEVICE_OFS_SC0 = 0,
	IPU_DEVICE_OFS_SC1
};
#define IPU_DEVICE_OFS_NUM_SC (IPU_DEVICE_OFS_SC1 + 1)

/* OFS external buffer devices in PSYS */
enum ipu_device_ofs_buf_id {
	IPU_DEVICE_OFS_BUF0 = 0
};
#define IPU_DEVICE_OFS_NUM_BUFFER (IPU_DEVICE_OFS_BUF0 + 1)

#endif /* __IPU_DEVICE_OFS_DEVICES_H */
