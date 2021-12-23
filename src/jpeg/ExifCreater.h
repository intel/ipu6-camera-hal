/*
 * Copyright Samsung Electronics Co.,LTD.
 * Copyright (C) 2010 The Android Open Source Project
 * Copyright (C) 2016-2020 Intel Corporation
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
 *
 * JPEG DRIVER MODULE (JpegEncoder.h)
 * Author  : ge.lee       -- initial version
 * Date    : 03 June 2010
 * Purpose : This file implements the JPEG encoder APIs as needed by Camera HAL
 */

#pragma once

#include <stdint.h>
#include <sys/ioctl.h>

#include "Exif.h"
#include "iutils/Utils.h"

namespace icamera {

#define MAX_JPG_WIDTH 4352
#define MAX_JPG_HEIGHT 3264
#define MAX_JPG_RESOLUTION (MAX_JPG_WIDTH * MAX_JPG_HEIGHT)

#define MAX_JPG_THUMBNAIL_WIDTH 640
#define MAX_JPG_THUMBNAIL_HEIGHT 480
#define MAX_JPG_THUMBNAIL_RESOLUTION (MAX_JPG_THUMBNAIL_WIDTH * MAX_JPG_THUMBNAIL_HEIGHT)

#define MAX_RGB_WIDTH 800
#define MAX_RGB_HEIGHT 480
#define MAX_RGB_RESOLUTION (MAX_RGB_WIDTH * MAX_RGB_HEIGHT)

/*******************************************************************************/
/* define JPG & image memory */
/* memory area is 4k(PAGE_SIZE) aligned because of VirtualCopyEx() */
#define JPG_STREAM_BUF_SIZE ((MAX_JPG_RESOLUTION / PAGE_SIZE + 1) * PAGE_SIZE)
#define JPG_STREAM_THUMB_BUF_SIZE ((MAX_JPG_THUMBNAIL_RESOLUTION / PAGE_SIZE + 1) * PAGE_SIZE)
#define JPG_FRAME_BUF_SIZE (((MAX_JPG_RESOLUTION * 3) / PAGE_SIZE + 1) * PAGE_SIZE)
#define JPG_FRAME_THUMB_BUF_SIZE (((MAX_JPG_THUMBNAIL_RESOLUTION * 3) / PAGE_SIZE + 1) * PAGE_SIZE)
#define JPG_RGB_BUF_SIZE (((MAX_RGB_RESOLUTION * 4) / PAGE_SIZE + 1) * PAGE_SIZE)

#define JPG_TOTAL_BUF_SIZE                                                  \
    (JPG_STREAM_BUF_SIZE + JPG_STREAM_THUMB_BUF_SIZE + JPG_FRAME_BUF_SIZE + \
     JPG_FRAME_THUMB_BUF_SIZE + JPG_RGB_BUF_SIZE)

#define JPG_MAIN_START 0x00
#define JPG_THUMB_START JPG_STREAM_BUF_SIZE
#define IMG_MAIN_START (JPG_STREAM_BUF_SIZE + JPG_STREAM_THUMB_BUF_SIZE)
#define IMG_THUMB_START (IMG_MAIN_START + JPG_FRAME_BUF_SIZE)
/*******************************************************************************/

const char MAKERNOTE_ID[] = {0x49, 0x6e, 0x74, 0x65, 0x6c, 0x4d,
                             0x6b, 0x6e, 0x6f, 0x74, 0x65, 0x0 /* "IntelMknote\0" */};

const unsigned SIZEOF_LENGTH_FIELD = 2;
const unsigned SIZEOF_APP2_MARKER = 2;
const unsigned SIZEOF_APP2_OVERHEAD =
    sizeof(MAKERNOTE_ID) + SIZEOF_APP2_MARKER + SIZEOF_LENGTH_FIELD;
const bool ENABLE_APP2_MARKER = true;
typedef enum { EXIF_FAIL = -1, EXIF_SUCCESS = 0 } exif_status;

class ExifCreater {
 public:
    ExifCreater();
    virtual ~ExifCreater();

    exif_status setThumbData(const void* thumbBuf, unsigned int thumbSize);

    bool isThumbDataSet() const;

    exif_status makeExif(void* exifOut, exif_attribute_t* exifInfo, size_t* size);

 private:
    exif_status makeApp2(void* pStartApp2, size_t& size, exif_attribute_t* exifInfo,
                         bool writeId = true);
    void writeMarkerSizeToBuf(unsigned char* ptrTo, unsigned int size);
    /*
        Every IFD has 12Bytes.
        Tag ID, 2B; Type, 2B; Count, 4B; Value/Offset, 4B;
        If it is Value, please use the first two functions.
        If it is Offset, please use the last two functions
        and store the data in the rear.
    */
    void writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                      unsigned int count, uint32_t value);
    void writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                      unsigned int count, unsigned char* pValue);
    void writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                      unsigned int count, rational_t* pValue, unsigned int* offset,
                      unsigned char* start);
    void writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                      unsigned int count, unsigned char* pValue, unsigned int* offset,
                      unsigned char* start);
    void writeThumbData(unsigned char* pIfdStart, unsigned char* pNextIfdOffset,
                        unsigned int* LongerTagOffset, exif_attribute_t* exifInfo);

    unsigned char* m_thumbBuf;  // MAP: Added to set thumbnail from external data
    unsigned int m_thumbSize;   // MAP: Added to set thumbnail from external data
};

}  // namespace icamera
