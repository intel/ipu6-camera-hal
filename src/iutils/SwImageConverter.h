/*
 * Copyright (C) 2016-2018 Intel Corporation.
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

#pragma once

namespace icamera {

namespace SwImageConverter {
    void RGB2YUV(unsigned short R, unsigned short G, unsigned short B,
         unsigned char *Y, unsigned char *U, unsigned char *V);

    void YUV2RGB(unsigned char Y, unsigned char U, unsigned char V,
        unsigned short *R, unsigned short *G, unsigned short *B);

    void convertBayerBlock(unsigned int x, unsigned int y,
        unsigned int width, unsigned int height, unsigned short bayer_data[4],
        unsigned char *out_buf, unsigned int src_fmt, unsigned int dst_fmt);

    void convertYuvBlock(unsigned int x, unsigned int y,
        unsigned int width, unsigned int height, unsigned char *in_buf,
        unsigned char *out_buf, unsigned int src_fmt, unsigned int dst_fmt);

    //convert the buffer from the src_fmt to the dst_fmt
    int convertFormat(unsigned int width, unsigned int height,
        unsigned char *inBuf, unsigned int inLength, unsigned int srcFmt,
        unsigned char *outBuf, unsigned int outLength, unsigned int dstFmt);
}

} //namespace icamera
