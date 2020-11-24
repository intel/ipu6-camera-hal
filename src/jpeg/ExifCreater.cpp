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
 * JPEG DRIVER MODULE (ExifCreater.cpp)
 * Author  : ge.lee       -- initial version
 * Date    : 03 June 2010
 * Purpose : This file implements the JPEG encoder APIs as needed by Camera HAL
 */

#define LOG_TAG "ExifCreater"

#include "ExifCreater.h"

#include <fcntl.h>
#include <sys/mman.h>

#include "iutils/CameraLog.h"

static const char ExifAsciiPrefix[] = {0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0};

namespace icamera {

ExifCreater::ExifCreater() {
    m_thumbBuf = nullptr;
    m_thumbSize = 0;
}

ExifCreater::~ExifCreater() {}

exif_status ExifCreater::setThumbData(const void* thumbBuf, unsigned int thumbSize) {
    // TODO: Maybe we should take into account the rest of the EXIF data as well here,
    // not just the thumbnail size.
    if (thumbSize >= EXIF_SIZE_LIMITATION) {
        LOGE("ERROR: Too big thumb size %d (limit: %d)", thumbSize, EXIF_SIZE_LIMITATION);
        m_thumbBuf = nullptr;
        m_thumbSize = 0;
        return EXIF_FAIL;
    }

    m_thumbBuf = static_cast<unsigned char*>(const_cast<void*>(thumbBuf));
    m_thumbSize = thumbSize;
    return EXIF_SUCCESS;
}

bool ExifCreater::isThumbDataSet() const {
    return m_thumbBuf != nullptr;
}

// if exif tags size + thumbnail size is > 64K, it will disable thumbnail
exif_status ExifCreater::makeExif(void* exifOut, exif_attribute_t* exifInfo, size_t* size) {
    LOG1("makeExif start");

    unsigned char *pCur, *pApp1Start, *pIfdStart, *pGpsIfdPtr, *pNextIfdOffset;
    unsigned int tmp, LongerTagOffset = 0, LongerTagOffsetWithoutThumbnail;
    pApp1Start = pCur = static_cast<unsigned char*>(exifOut);

    // 2 Exif Identifier Code & TIFF Header
    pCur += 4;  // Skip 4 Byte for APP1 marker and length

    unsigned char ExifIdentifierCode[6] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
    MEMCPY_S(pCur, sizeof(ExifIdentifierCode), ExifIdentifierCode, sizeof(ExifIdentifierCode));
    pCur += 6;

    /* Byte Order - little endian, Offset of IFD - 0x00000008.H */
    unsigned char TiffHeader[8] = {0x49, 0x49, 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00};
    MEMCPY_S(pCur, sizeof(TiffHeader), TiffHeader, sizeof(TiffHeader));
    pIfdStart = pCur;
    pCur += 8;

    // 2 0th IFD TIFF Tags
    if (exifInfo->enableGps)
        tmp = NUM_0TH_IFD_TIFF;
    else
        tmp = NUM_0TH_IFD_TIFF - 1;

    MEMCPY_S(pCur, NUM_SIZE, (int8_t*)&tmp, NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffset += 8 + NUM_SIZE + tmp * IFD_SIZE + OFFSET_SIZE;

    writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG, 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG, 1, exifInfo->height);
    writeExifIfd(&pCur, EXIF_TAG_IMAGE_DESCRIPTION, EXIF_TYPE_ASCII,
                 strlen(reinterpret_cast<char*>(exifInfo->image_description)) + 1,
                 exifInfo->image_description, &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MAKE, EXIF_TYPE_ASCII, strlen((char*)exifInfo->maker) + 1,
                 exifInfo->maker, &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MODEL, EXIF_TYPE_ASCII, strlen((char*)exifInfo->model) + 1,
                 exifInfo->model, &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT, 1, exifInfo->orientation);
    writeExifIfd(&pCur, EXIF_TAG_X_RESOLUTION, EXIF_TYPE_RATIONAL, 1, &exifInfo->x_resolution,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_Y_RESOLUTION, EXIF_TYPE_RATIONAL, 1, &exifInfo->y_resolution,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_RESOLUTION_UNIT, EXIF_TYPE_SHORT, 1, exifInfo->resolution_unit);
    writeExifIfd(&pCur, EXIF_TAG_SOFTWARE, EXIF_TYPE_ASCII, strlen((char*)exifInfo->software) + 1,
                 exifInfo->software, &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME, EXIF_TYPE_ASCII, 20, exifInfo->date_time,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_YCBCR_POSITIONING, EXIF_TYPE_SHORT, 1,
                 exifInfo->ycbcr_positioning);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_IFD_POINTER, EXIF_TYPE_LONG, 1, LongerTagOffset);

    pGpsIfdPtr = pCur;
    if (exifInfo->enableGps) {
        pCur += IFD_SIZE;  // Skip a ifd size for gps IFD pointer
    }

    pNextIfdOffset = pCur;  // Skip a offset size for next IFD offset
    pCur += OFFSET_SIZE;

    // 2 0th IFD Exif Private Tags
    pCur = pIfdStart + LongerTagOffset;

    int drop_num = 0;
    if (exifInfo->exposure_time.den == 0) drop_num++;
    if (exifInfo->shutter_speed.den == 0) drop_num++;
    if (exifInfo->makerNoteDataSize == 0 || exifInfo->makernoteToApp2) {
        // skip the makernote IFD in APP1, when we don't have any,
        // or if we want it to APP2
        drop_num++;
    }
    tmp = NUM_0TH_IFD_EXIF - drop_num;
    MEMCPY_S(pCur, NUM_SIZE, &tmp, NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffset += NUM_SIZE + tmp * IFD_SIZE + OFFSET_SIZE;
    if (exifInfo->exposure_time.den != 0) {
        writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_TIME, EXIF_TYPE_RATIONAL, 1, &exifInfo->exposure_time,
                     &LongerTagOffset, pIfdStart);
    }
    writeExifIfd(&pCur, EXIF_TAG_FNUMBER, EXIF_TYPE_RATIONAL, 1, &exifInfo->fnumber,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_PROGRAM, EXIF_TYPE_SHORT, 1, exifInfo->exposure_program);
    writeExifIfd(&pCur, EXIF_TAG_ISO_SPEED_RATING, EXIF_TYPE_SHORT, 1, exifInfo->iso_speed_rating);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_VERSION, EXIF_TYPE_UNDEFINED, 4, exifInfo->exif_version);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_ORG, EXIF_TYPE_ASCII, 20, exifInfo->date_time,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_DIGITIZE, EXIF_TYPE_ASCII, 20, exifInfo->date_time,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_COMPONENTS_CONFIGURATION, EXIF_TYPE_UNDEFINED, 4,
                 exifInfo->components_configuration);
    if (exifInfo->shutter_speed.den != 0) {
        writeExifIfd(&pCur, EXIF_TAG_SHUTTER_SPEED, EXIF_TYPE_SRATIONAL, 1,
                     reinterpret_cast<rational_t*>(&exifInfo->shutter_speed), &LongerTagOffset,
                     pIfdStart);
    }
    writeExifIfd(&pCur, EXIF_TAG_APERTURE, EXIF_TYPE_RATIONAL, 1, &exifInfo->aperture,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_BRIGHTNESS, EXIF_TYPE_SRATIONAL, 1,
                 reinterpret_cast<rational_t*>(&exifInfo->brightness), &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_BIAS, EXIF_TYPE_SRATIONAL, 1,
                 reinterpret_cast<rational_t*>(&exifInfo->exposure_bias), &LongerTagOffset,
                 pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MAX_APERTURE, EXIF_TYPE_RATIONAL, 1, &exifInfo->max_aperture,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SUBJECT_DISTANCE, EXIF_TYPE_RATIONAL, 1,
                 &exifInfo->subject_distance, &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_METERING_MODE, EXIF_TYPE_SHORT, 1, exifInfo->metering_mode);
    writeExifIfd(&pCur, EXIF_TAG_LIGHT_SOURCE, EXIF_TYPE_SHORT, 1, exifInfo->light_source);
    writeExifIfd(&pCur, EXIF_TAG_FLASH, EXIF_TYPE_SHORT, 1, exifInfo->flash);
    writeExifIfd(&pCur, EXIF_TAG_FOCAL_LENGTH, EXIF_TYPE_RATIONAL, 1, &exifInfo->focal_length,
                 &LongerTagOffset, pIfdStart);
    char code[8] = {0x41, 0x53, 0x43, 0x49, 0x49, 0x00, 0x00, 0x00};
    size_t commentsLen = strlen((char*)exifInfo->user_comment) + 1;
    if (commentsLen > (sizeof(exifInfo->user_comment) - sizeof(code))) return EXIF_FAIL;
    memmove(exifInfo->user_comment + sizeof(code), exifInfo->user_comment, commentsLen);
    MEMCPY_S(exifInfo->user_comment, sizeof(exifInfo->user_comment), code, sizeof(code));
    writeExifIfd(&pCur, EXIF_TAG_USER_COMMENT, EXIF_TYPE_UNDEFINED, commentsLen + sizeof(code),
                 exifInfo->user_comment, &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SUBSEC_TIME, EXIF_TYPE_ASCII,
                 strlen((char*)exifInfo->subsec_time) + 1, exifInfo->subsec_time, &LongerTagOffset,
                 pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SUBSEC_TIME_ORIG, EXIF_TYPE_ASCII,
                 strlen((char*)exifInfo->subsec_time) + 1, exifInfo->subsec_time, &LongerTagOffset,
                 pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SUBSEC_TIME_DIG, EXIF_TYPE_ASCII,
                 strlen((char*)exifInfo->subsec_time) + 1, exifInfo->subsec_time, &LongerTagOffset,
                 pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_FLASH_PIX_VERSION, EXIF_TYPE_UNDEFINED, 4,
                 exifInfo->flashpix_version);
    writeExifIfd(&pCur, EXIF_TAG_COLOR_SPACE, EXIF_TYPE_SHORT, 1, exifInfo->color_space);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_X_DIMENSION, EXIF_TYPE_LONG, 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_Y_DIMENSION, EXIF_TYPE_LONG, 1, exifInfo->height);
    writeExifIfd(&pCur, EXIF_TAG_CUSTOM_RENDERED, EXIF_TYPE_SHORT, 1, exifInfo->custom_rendered);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_MODE, EXIF_TYPE_SHORT, 1, exifInfo->exposure_mode);
    writeExifIfd(&pCur, EXIF_TAG_WHITE_BALANCE, EXIF_TYPE_SHORT, 1, exifInfo->white_balance);
    writeExifIfd(&pCur, EXIF_TAG_JPEG_ZOOM_RATIO, EXIF_TYPE_RATIONAL, 1, &exifInfo->zoom_ratio,
                 &LongerTagOffset, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SCENCE_CAPTURE_TYPE, EXIF_TYPE_SHORT, 1,
                 exifInfo->scene_capture_type);
    writeExifIfd(&pCur, EXIF_TAG_GAIN_CONTROL, EXIF_TYPE_SHORT, 1, exifInfo->gain_control);
    writeExifIfd(&pCur, EXIF_TAG_CONTRAST, EXIF_TYPE_SHORT, 1, exifInfo->contrast);
    writeExifIfd(&pCur, EXIF_TAG_SATURATION, EXIF_TYPE_SHORT, 1, exifInfo->saturation);
    writeExifIfd(&pCur, EXIF_TAG_SHARPNESS, EXIF_TYPE_SHORT, 1, exifInfo->sharpness);

    // Save MakerNote data to APP1, unless we want it APP2
    if (exifInfo->makerNoteDataSize > 0 && !exifInfo->makernoteToApp2) {
        writeExifIfd(&pCur, EXIF_TAG_MAKER_NOTE, EXIF_TYPE_UNDEFINED, exifInfo->makerNoteDataSize,
                     (unsigned char*)exifInfo->makerNoteData, &LongerTagOffset, pIfdStart);
    }

    tmp = 0;
    MEMCPY_S(pCur, OFFSET_SIZE, (int8_t*)&tmp, OFFSET_SIZE);  // next IFD offset
    pCur += OFFSET_SIZE;

    // 2 0th IFD GPS Info Tags
    if (exifInfo->enableGps) {
        writeExifIfd(&pGpsIfdPtr, EXIF_TAG_GPS_IFD_POINTER, EXIF_TYPE_LONG, 1,
                     LongerTagOffset);  // GPS IFD pointer skipped on 0th IFD

        pCur = pIfdStart + LongerTagOffset;

        tmp = NUM_0TH_IFD_GPS;
        if ((exifInfo->enableGps & EXIF_GPS_LATITUDE) == 0) tmp -= 2;
        if ((exifInfo->enableGps & EXIF_GPS_LONGITUDE) == 0) tmp -= 2;
        if ((exifInfo->enableGps & EXIF_GPS_ALTITUDE) == 0) tmp -= 2;
        if ((exifInfo->enableGps & EXIF_GPS_TIMESTAMP) == 0) tmp -= 1;
        if ((exifInfo->enableGps & EXIF_GPS_PROCMETHOD) == 0) tmp -= 1;
        if ((exifInfo->enableGps & EXIF_GPS_IMG_DIRECTION) == 0) tmp -= 2;

        MEMCPY_S(pCur, NUM_SIZE, (int8_t*)&tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        LongerTagOffset += NUM_SIZE + tmp * IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_GPS_VERSION_ID, EXIF_TYPE_BYTE, 4, exifInfo->gps_version_id);
        if (exifInfo->enableGps & EXIF_GPS_LATITUDE) {
            writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE_REF, EXIF_TYPE_ASCII, 2,
                         exifInfo->gps_latitude_ref);
            writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE, EXIF_TYPE_RATIONAL, 3,
                         exifInfo->gps_latitude, &LongerTagOffset, pIfdStart);
        }

        if (exifInfo->enableGps & EXIF_GPS_LONGITUDE) {
            writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE_REF, EXIF_TYPE_ASCII, 2,
                         exifInfo->gps_longitude_ref);
            writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE, EXIF_TYPE_RATIONAL, 3,
                         exifInfo->gps_longitude, &LongerTagOffset, pIfdStart);
        }

        if (exifInfo->enableGps & EXIF_GPS_ALTITUDE) {
            writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE_REF, EXIF_TYPE_BYTE, 1,
                         exifInfo->gps_altitude_ref);
            writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE, EXIF_TYPE_RATIONAL, 1,
                         &exifInfo->gps_altitude, &LongerTagOffset, pIfdStart);
        }

        if (exifInfo->enableGps & EXIF_GPS_TIMESTAMP) {
            writeExifIfd(&pCur, EXIF_TAG_GPS_TIMESTAMP, EXIF_TYPE_RATIONAL, 3,
                         exifInfo->gps_timestamp, &LongerTagOffset, pIfdStart);
        }

        if (exifInfo->enableGps & EXIF_GPS_IMG_DIRECTION) {
            writeExifIfd(&pCur, EXIF_TAG_GPS_IMG_DIRECTION_REF, EXIF_TYPE_ASCII, 2,
                         exifInfo->gps_img_direction_ref);
            writeExifIfd(&pCur, EXIF_TAG_GPS_IMG_DIRECTION, EXIF_TYPE_RATIONAL, 1,
                         &exifInfo->gps_img_direction, &LongerTagOffset, pIfdStart);
        }

        if (exifInfo->enableGps & EXIF_GPS_PROCMETHOD) {
            tmp = strlen((char*)exifInfo->gps_processing_method);
            if (tmp > 0) {
                if (tmp > 100) {
                    tmp = 100;
                }
                unsigned char tmp_buf[100 + sizeof(ExifAsciiPrefix)];
                MEMCPY_S(tmp_buf, sizeof(tmp_buf), ExifAsciiPrefix, sizeof(ExifAsciiPrefix));
                MEMCPY_S(&tmp_buf[sizeof(ExifAsciiPrefix)], 100, exifInfo->gps_processing_method,
                         tmp);
                writeExifIfd(&pCur, EXIF_TAG_GPS_PROCESSING_METHOD, EXIF_TYPE_UNDEFINED,
                             tmp + sizeof(ExifAsciiPrefix), tmp_buf, &LongerTagOffset, pIfdStart);
            }
        }
        writeExifIfd(&pCur, EXIF_TAG_GPS_DATESTAMP, EXIF_TYPE_ASCII, 11, exifInfo->gps_datestamp,
                     &LongerTagOffset, pIfdStart);
        tmp = 0;
        MEMCPY_S(pCur, OFFSET_SIZE, (int8_t*)&tmp, OFFSET_SIZE);  // next IFD offset
        pCur += OFFSET_SIZE;
    }

    // backup LongerTagOffset, if the total exif size is > 64K, we will use it.
    LongerTagOffsetWithoutThumbnail = LongerTagOffset;
    if (LongerTagOffsetWithoutThumbnail >= EXIF_SIZE_LIMITATION) {
        LOGE("line:%d, in the makeExif, the size exceeds 64K", __LINE__);
        return EXIF_FAIL;
    }

    // 2 1th IFD TIFF Tags
    if (exifInfo->enableThumb && (m_thumbBuf != nullptr) && (m_thumbSize > 0)) {
        writeThumbData(pIfdStart, pNextIfdOffset, &LongerTagOffset, exifInfo);
    } else {
        tmp = 0;
        MEMCPY_S(pNextIfdOffset, OFFSET_SIZE, (int8_t*)&tmp,
                 OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD
    }

    // fill APP1 maker
    unsigned char App1Marker[2] = {0xff, 0xe1};
    MEMCPY_S(pApp1Start, 2, App1Marker, 2);
    pApp1Start += 2;

    // calc and fill the APP1 segment total size, 2 is length; 6 is ExifIdentifierCode
    *size = 2 + 6 + LongerTagOffset;

    writeMarkerSizeToBuf(pApp1Start, *size);

    unsigned app2StartOffset = *size;
    *size += 2;  // APP1 marker size

    exif_status status = EXIF_SUCCESS;

    if (exifInfo->makernoteToApp2) {
        LOG1("Makernote goes to APP2 segment.");
        status = makeApp2((pApp1Start + app2StartOffset), *size, exifInfo);
    }

    if (status != EXIF_SUCCESS) LOGW("Failed to create EXIF APP2 section");

    LOG1("makeExif End");

    return EXIF_SUCCESS;
}

void ExifCreater::writeMarkerSizeToBuf(unsigned char* ptrTo, unsigned int size) {
    unsigned char size_mm[2] = {static_cast<unsigned char>((size >> 8) & 0xFF),
                                static_cast<unsigned char>(size & 0xFF)};

    MEMCPY_S(ptrTo, 2, size_mm, 2);
}

/**
 * makeApp2
 *
 * Write the makernote to APP2 segment. Use multiple APP2 segments if makernote
 * size is more than one segment (64 kb)
 *
 * \param pStartApp2 [IN] APP2 start address
 * \param exifInfo [IN] Data to be written
 * \param writeId [IN] Whether to write the Intel Makernote ID string.
 * \param size [OUT] Total size after APP2 is written
 */
exif_status ExifCreater::makeApp2(void* pStartApp2, size_t& size, exif_attribute_t* exifInfo,
                                  bool writeId) {
    LOG1("@%s", __func__);

    // APP2 marker will be written starting from the pos pointed to by
    // pStartApp2

    if (exifInfo->makerNoteDataSize <= 0) return EXIF_SUCCESS;

    int bytesLeftForSegment = EXIF_SIZE_LIMITATION;
    int bytesToWrite = exifInfo->makerNoteDataSize;

    unsigned char *pCur = nullptr, *pApp2Start = nullptr;
    unsigned char App2Marker[SIZEOF_APP2_MARKER] = {0xff, 0xe2};
    int writeCount = 0;
    unsigned char* toWrite = exifInfo->makerNoteData;

    pCur = static_cast<unsigned char*>(pStartApp2);

    // Write Makernote up to ~64kB, then split to a new
    // APP2 segment, if needed
    while (bytesToWrite > 0) {
        pApp2Start = pCur;
        pCur += 4;  // Skip 4 bytes for APP2 marker and length

        if (writeId) {
            MEMCPY_S(pCur, sizeof(MAKERNOTE_ID), MAKERNOTE_ID, sizeof(MAKERNOTE_ID));
            pCur += sizeof(MAKERNOTE_ID);
            size += sizeof(MAKERNOTE_ID);
            // ID overhead for one APP2 segment
            bytesLeftForSegment -= sizeof(MAKERNOTE_ID);
        }

        // Overhead for one APP2 segment:
        bytesLeftForSegment -= (sizeof(App2Marker) + SIZEOF_LENGTH_FIELD);

        if (bytesToWrite > bytesLeftForSegment) {
            // More data to write than what fits to one APP2 marker
            writeCount = bytesLeftForSegment;
        } else {
            // All data fits to one APP2 segment
            writeCount = bytesToWrite;
        }

        bytesToWrite -= writeCount;

        MEMCPY_S(pCur, writeCount, toWrite, writeCount);
        pCur += writeCount;
        toWrite += writeCount;
        size += writeCount;

        // Last, put the APP2 marker to the beginning of the segment
        MEMCPY_S(pApp2Start, sizeof(App2Marker), App2Marker, sizeof(App2Marker));
        pApp2Start += sizeof(App2Marker);

        // Length field goes after the APP2 marker
        int app2SegmentSize = writeCount + SIZEOF_LENGTH_FIELD;  // Raw data written + overhead
        if (writeId) app2SegmentSize += sizeof(MAKERNOTE_ID);

        writeMarkerSizeToBuf(pApp2Start, app2SegmentSize);

        // add the 2 bytes for both length field and APP2 marker, the caller has to know the total
        // size
        size += sizeof(App2Marker) + SIZEOF_LENGTH_FIELD;

        // Reset byte counts for another APP2 segment, if needed
        bytesLeftForSegment = EXIF_SIZE_LIMITATION;
    }

    return EXIF_SUCCESS;
}

void ExifCreater::writeThumbData(unsigned char* pIfdStart, unsigned char* pNextIfdOffset,
                                 unsigned int* LongerTagOffset, exif_attribute_t* exifInfo) {
    unsigned char* pCur;
    unsigned int tmp;

    // firstly calc the exif total size, if it's > 64K, we'll disable the thumbnail
    tmp = 4 + 6 + *LongerTagOffset;  // 4 is APP1 marker and length; 6 is ExifIdentifierCode
    tmp += NUM_SIZE + NUM_1TH_IFD_TIFF * IFD_SIZE + OFFSET_SIZE;
    tmp += sizeof(exifInfo->x_resolution) + sizeof(exifInfo->y_resolution);
    tmp += m_thumbSize;

    if (tmp > EXIF_SIZE_LIMITATION) {
        LOGD("line:%d, in makeExif, exif total size(%d) > 64K, we'll disable thumbnail.", __LINE__,
             tmp);
        m_thumbSize = 0;
        m_thumbBuf = nullptr;
        tmp = 0;
        MEMCPY_S(pNextIfdOffset, OFFSET_SIZE, (int8_t*)&tmp,
                 OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD
    } else {
        tmp = *LongerTagOffset;
        MEMCPY_S(pNextIfdOffset, OFFSET_SIZE, (int8_t*)&tmp,
                 OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD

        pCur = pIfdStart + *LongerTagOffset;

        tmp = NUM_1TH_IFD_TIFF;
        MEMCPY_S(pCur, NUM_SIZE, (int8_t*)&tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        *LongerTagOffset += NUM_SIZE + NUM_1TH_IFD_TIFF * IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG, 1, exifInfo->widthThumb);
        writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG, 1, exifInfo->heightThumb);
        writeExifIfd(&pCur, EXIF_TAG_COMPRESSION_SCHEME, EXIF_TYPE_SHORT, 1,
                     exifInfo->compression_scheme);
        writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT, 1, exifInfo->orientation);
        writeExifIfd(&pCur, EXIF_TAG_X_RESOLUTION, EXIF_TYPE_RATIONAL, 1, &exifInfo->x_resolution,
                     LongerTagOffset, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_Y_RESOLUTION, EXIF_TYPE_RATIONAL, 1, &exifInfo->y_resolution,
                     LongerTagOffset, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_RESOLUTION_UNIT, EXIF_TYPE_SHORT, 1,
                     exifInfo->resolution_unit);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT, EXIF_TYPE_LONG, 1, *LongerTagOffset);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN, EXIF_TYPE_LONG, 1, m_thumbSize);

        tmp = 0;
        MEMCPY_S(pCur, OFFSET_SIZE, (int8_t*)&tmp, OFFSET_SIZE);  // next IFD offset
        // pCur += OFFSET_SIZE;

        MEMCPY_S(pIfdStart + *LongerTagOffset, m_thumbSize, m_thumbBuf, m_thumbSize);
        *LongerTagOffset += m_thumbSize;
    }
}

void ExifCreater::writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                               unsigned int count, uint32_t value) {
    MEMCPY_S(*pCur, 2, (int8_t*)&tag, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 2, (int8_t*)&type, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 4, (int8_t*)&count, 4);
    *pCur += 4;
    MEMCPY_S(*pCur, 4, (int8_t*)&value, 4);
    *pCur += 4;
}

void ExifCreater::writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                               unsigned int count, unsigned char* pValue) {
    char buf[4] = {
        0,
    };

    MEMCPY_S(buf, count, pValue, count);
    MEMCPY_S(*pCur, 2, (int8_t*)&tag, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 2, (int8_t*)&type, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 4, (int8_t*)&count, 4);
    *pCur += 4;
    MEMCPY_S(*pCur, 4, (int8_t*)buf, 4);
    *pCur += 4;
}

void ExifCreater::writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                               unsigned int count, unsigned char* pValue, unsigned int* offset,
                               unsigned char* start) {
    MEMCPY_S(*pCur, 2, (int8_t*)&tag, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 2, (int8_t*)&type, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 4, (int8_t*)&count, 4);
    *pCur += 4;
    MEMCPY_S(*pCur, 4, (int8_t*)offset, 4);
    *pCur += 4;
    MEMCPY_S(start + *offset, count, pValue, count);
    *offset += count;
}

void ExifCreater::writeExifIfd(unsigned char** pCur, unsigned short tag, unsigned short type,
                               unsigned int count, rational_t* pValue, unsigned int* offset,
                               unsigned char* start) {
    MEMCPY_S(*pCur, 2, (int8_t*)&tag, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 2, (int8_t*)&type, 2);
    *pCur += 2;
    MEMCPY_S(*pCur, 4, (int8_t*)&count, 4);
    *pCur += 4;
    MEMCPY_S(*pCur, 4, (int8_t*)offset, 4);
    *pCur += 4;
    MEMCPY_S(start + *offset, 8 * count, (int8_t*)pValue, 8 * count);
    *offset += 8 * count;
}

}  // namespace icamera
