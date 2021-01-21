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

#ifndef __FRAME_SUPPORT_H
#define __FRAME_SUPPORT_H

#define INTERLEAVED_FRAME_TYPE_NUM_PLANES    1
#define YUV_SEMI_PLANAR_FRAME_TYPE_NUM_PLANES    2
#define YUV_PLANAR_FRAME_TYPE_NUM_PLANES    3
#define BAYER_PLANAR_FRAME_TYPE_NUM_PLANES    4

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
#endif    /* __FRAME_SUPPORT_H */
