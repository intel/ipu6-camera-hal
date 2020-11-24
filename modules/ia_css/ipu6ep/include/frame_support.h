/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2018 - 2018 Intel Corporation.
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

#ifndef __FRAME_SUPPORT_H
#define __FRAME_SUPPORT_H

#define INTERLEAVED_FRAME_TYPE_NUM_PLANES	1
#define YUV_SEMI_PLANAR_FRAME_TYPE_NUM_PLANES	2
#define YUV_PLANAR_FRAME_TYPE_NUM_PLANES	3
#define BAYER_PLANAR_FRAME_TYPE_NUM_PLANES	4

/* genpipe doesn't support B = A in enum definition */
#ifndef PIPE_GENERATION

enum image_iterator_planes {
	IMG_ITER_PLANE_0 = 0,
	IMG_ITER_PLANE_1,
	IMG_ITER_PLANE_2,
	IMG_ITER_PLANE_3,
	IMG_ITER_MAX_NUM_PLANES
};

enum interleaved_frames_types_planes {
	INTERLEAVED_FRAME_MAIN_PLANE = 0,
	INTERLEAVED_FRAME_NUM_PLANES
};

enum yuv_semi_planar_frame_types_planes {
	YUV_SEMI_PLANAR_FRAME_Y_PLANE = 0,
	YUV_SEMI_PLANAR_FRAME_UV_PLANE,
	YUV_SEMI_PLANAR_FRAME_NUM_PLANES
};

enum yuv_planar_frame_types_planes {
	YUV_PLANAR_FRAME_Y_PLANE = 0,
	YUV_PLANAR_FRAME_U_PLANE,
	YUV_PLANAR_FRAME_V_PLANE,
	YUV_PLANAR_FRAME_NUM_PLANES
};

enum bayer_planar_frame_types_planes {
	BAYER_PLANAR_FRAME_GR_PLANE = 0,
	BAYER_PLANAR_FRAME_R_PLANE,
	BAYER_PLANAR_FRAME_B_PLANE,
	BAYER_PLANAR_FRAME_GB_PLANE,
	BAYER_PLANAR_FRAME_NUM_PLANES
};

#endif /* PIPE_GENERATION */
#endif	/* __FRAME_SUPPORT_H */
