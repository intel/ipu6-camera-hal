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

#ifndef __COMMON_YUV_IO_ISP_H
#define __COMMON_YUV_IO_ISP_H

#include "bufqueue.shared.h"
#include "storage_class.h"

STORAGE_CLASS_INLINE buf_blk_t
yuv_io_get_luma_buffer(bufqueue_t q);

STORAGE_CLASS_INLINE buf_blk_t
yuv_io_get_chroma_u_buffer(bufqueue_t q,
			   unsigned chroma_subsample_shift_x,
			   unsigned chroma_subsample_shift_y);
STORAGE_CLASS_INLINE buf_blk_t
yuv_io_get_chroma_v_buffer(bufqueue_t q,
			   unsigned chroma_subsample_shift_x,
			   unsigned chroma_subsample_shift_y);
STORAGE_CLASS_INLINE mem_ptr_t
yuv_io_get_luma_ptr(bufqueue_t q,
		    int buf_idx);
STORAGE_CLASS_INLINE mem_ptr_t
yuv_io_get_chroma_u_ptr(bufqueue_t q,
			unsigned chroma_subsample_shift_x,
			unsigned chroma_subsample_shift_y,
			int buf_idx);
STORAGE_CLASS_INLINE mem_ptr_t
yuv_io_get_chroma_v_ptr(bufqueue_t q,
			unsigned chroma_subsample_shift_x,
			unsigned chroma_subsample_shift_y,
			int buf_idx);

#ifndef PIPE_GENERATION
/* Don't include these files during genpipe code generation */
#include "common_yuv_io.isp.c"
#endif

#endif /* __COMMON_YUV_IO_ISP_H */
