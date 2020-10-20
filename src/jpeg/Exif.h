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
 */

#pragma once

#include <math.h>

#define EXIF_LOG2(x) (log((double)(x)) / log(2.0))
#define APEX_FNUM_TO_APERTURE(x) (2 * (EXIF_LOG2((double)(x))))
#define APEX_EXPOSURE_TO_SHUTTER(x) (-1.0 * (EXIF_LOG2((double)(x))))
#define APEX_ISO_TO_FILMSENSITIVITY(x) ((int)(EXIF_LOG2((x) / 3.125) + 0.5))

#define NUM_SIZE 2
#define IFD_SIZE 12
#define OFFSET_SIZE 4

#define NUM_0TH_IFD_TIFF 14
#define NUM_0TH_IFD_EXIF 36
#define NUM_0TH_IFD_GPS 12
#define NUM_1TH_IFD_TIFF 9
// For QVGA: 320 * 240 * 1.5
#define EXIF_SIZE_LIMITATION 0x10000
// Limite the thumbnail size to 32k, to make sure the whole exif size does
// not exceed the exif size limitation. We guess the total size of all the
// other fields is smaller than 32k. (Currently the size is about 26k.)
#define THUMBNAIL_SIZE_LIMITATION 0x8000

/* Type */
#define EXIF_TYPE_BYTE 1
#define EXIF_TYPE_ASCII 2
#define EXIF_TYPE_SHORT 3
#define EXIF_TYPE_LONG 4
#define EXIF_TYPE_RATIONAL 5
#define EXIF_TYPE_UNDEFINED 7
#define EXIF_TYPE_SLONG 9
#define EXIF_TYPE_SRATIONAL 10

#define EXIF_FILE_SIZE 28800

/* 0th IFD TIFF Tags */
#define EXIF_TAG_IMAGE_WIDTH 0x0100
#define EXIF_TAG_IMAGE_HEIGHT 0x0101
#define EXIF_TAG_IMAGE_DESCRIPTION 0x010e
#define EXIF_TAG_MAKE 0x010f
#define EXIF_TAG_MODEL 0x0110
#define EXIF_TAG_ORIENTATION 0x0112
#define EXIF_TAG_X_RESOLUTION 0x011A
#define EXIF_TAG_Y_RESOLUTION 0x011B
#define EXIF_TAG_RESOLUTION_UNIT 0x0128
#define EXIF_TAG_SOFTWARE 0x0131
#define EXIF_TAG_DATE_TIME 0x0132
#define EXIF_TAG_YCBCR_POSITIONING 0x0213
#define EXIF_TAG_EXIF_IFD_POINTER 0x8769
#define EXIF_TAG_GPS_IFD_POINTER 0x8825

/* 0th IFD Exif Private Tags */
#define EXIF_TAG_EXPOSURE_TIME 0x829A
#define EXIF_TAG_FNUMBER 0x829D
#define EXIF_TAG_EXPOSURE_PROGRAM 0x8822
#define EXIF_TAG_ISO_SPEED_RATING 0x8827
#define EXIF_TAG_EXIF_VERSION 0x9000
#define EXIF_TAG_DATE_TIME_ORG 0x9003
#define EXIF_TAG_DATE_TIME_DIGITIZE 0x9004
#define EXIF_TAG_COMPONENTS_CONFIGURATION 0x9101
#define EXIF_TAG_SHUTTER_SPEED 0x9201
#define EXIF_TAG_APERTURE 0x9202
#define EXIF_TAG_BRIGHTNESS 0x9203
#define EXIF_TAG_EXPOSURE_BIAS 0x9204
#define EXIF_TAG_MAX_APERTURE 0x9205
#define EXIF_TAG_SUBJECT_DISTANCE 0x9206
#define EXIF_TAG_METERING_MODE 0x9207
#define EXIF_TAG_LIGHT_SOURCE 0x9208
#define EXIF_TAG_FLASH 0x9209
#define EXIF_TAG_FOCAL_LENGTH 0x920A
#define EXIF_TAG_MAKER_NOTE 0x927C
#define EXIF_TAG_USER_COMMENT 0x9286
#define EXIF_TAG_SUBSEC_TIME 0x9290
#define EXIF_TAG_SUBSEC_TIME_ORIG 0x9291
#define EXIF_TAG_SUBSEC_TIME_DIG 0x9292
#define EXIF_TAG_FLASH_PIX_VERSION 0xA000
#define EXIF_TAG_COLOR_SPACE 0xA001
#define EXIF_TAG_PIXEL_X_DIMENSION 0xA002
#define EXIF_TAG_PIXEL_Y_DIMENSION 0xA003
#define EXIF_TAG_CUSTOM_RENDERED 0xA401
#define EXIF_TAG_EXPOSURE_MODE 0xA402
#define EXIF_TAG_WHITE_BALANCE 0xA403
#define EXIF_TAG_JPEG_ZOOM_RATIO 0XA404
#define EXIF_TAG_SCENCE_CAPTURE_TYPE 0xA406
#define EXIF_TAG_GAIN_CONTROL 0xA407
#define EXIF_TAG_CONTRAST 0xA408
#define EXIF_TAG_SATURATION 0xA409
#define EXIF_TAG_SHARPNESS 0xA40A

/* 0th IFD GPS Info Tags */
#define EXIF_TAG_GPS_VERSION_ID 0x0000
#define EXIF_TAG_GPS_LATITUDE_REF 0x0001
#define EXIF_TAG_GPS_LATITUDE 0x0002
#define EXIF_TAG_GPS_LONGITUDE_REF 0x0003
#define EXIF_TAG_GPS_LONGITUDE 0x0004
#define EXIF_TAG_GPS_ALTITUDE_REF 0x0005
#define EXIF_TAG_GPS_ALTITUDE 0x0006
#define EXIF_TAG_GPS_TIMESTAMP 0x0007
#define EXIF_TAG_GPS_IMG_DIRECTION_REF 0x0010
#define EXIF_TAG_GPS_IMG_DIRECTION 0x0011
#define EXIF_TAG_GPS_PROCESSING_METHOD 0x001B
#define EXIF_TAG_GPS_DATESTAMP 0x001D

/* 1th IFD TIFF Tags */
#define EXIF_TAG_COMPRESSION_SCHEME 0x0103
/*
#define EXIF_TAG_X_RESOLUTION                   0x011A
#define EXIF_TAG_Y_RESOLUTION                   0x011B
#define EXIF_TAG_RESOLUTION_UNIT                0x0128
*/
#define EXIF_TAG_JPEG_INTERCHANGE_FORMAT 0x0201
#define EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN 0x0202

typedef enum {
    EXIF_ORIENTATION_UP = 1,
    EXIF_ORIENTATION_90 = 6,
    EXIF_ORIENTATION_180 = 3,
    EXIF_ORIENTATION_270 = 8,
} ExifOrientationType;

typedef enum {
    EXIF_SCENE_STANDARD,
    EXIF_SCENE_LANDSCAPE,
    EXIF_SCENE_PORTRAIT,
    EXIF_SCENE_NIGHT,
} CamExifSceneCaptureType;

typedef enum {
    EXIF_METERING_UNKNOWN,
    EXIF_METERING_AVERAGE,
    EXIF_METERING_CENTER,
    EXIF_METERING_SPOT,
    EXIF_METERING_MULTISPOT,
    EXIF_METERING_PATTERN,
    EXIF_METERING_PARTIAL,
    EXIF_METERING_OTHER = 255,
} CamExifMeteringModeType;

typedef enum {
    EXIF_EXPOSURE_AUTO,
    EXIF_EXPOSURE_MANUAL,
    EXIF_EXPOSURE_AUTO_BRACKET,
} CamExifExposureModeType;

typedef enum {
    EXIF_WB_AUTO,
    EXIF_WB_MANUAL,
} CamExifWhiteBalanceType;

typedef enum {
    EXIF_LIGHT_SOURCE_UNKNOWN,
    EXIF_LIGHT_SOURCE_DAYLIGHT,
    EXIF_LIGHT_SOURCE_FLUORESCENT,
    EXIF_LIGHT_SOURCE_TUNGSTEN,
    EXIF_LIGHT_SOURCE_FLASH,
    EXIF_LIGHT_SOURCE_FINE_WEATHER = 9,
    EXIF_LIGHT_SOURCE_CLOUDY_WEATHER,
    EXIF_LIGHT_SOURCE_SHADE,
    EXIF_LIGHT_SOURCE_DAYLIGHT_FLUORESCENT,
    EXIF_LIGHT_SOURCE_DAY_WHITE_FLUORESCENT,
    EXIF_LIGHT_SOURCE_COOL_WHITE_FLUORESCENT,
    EXIF_LIGHT_SOURCE_WHITE_FLUORESCENT,
    EXIF_LIGHT_SOURCE_WARM_WHITE_FLUORESCENT,  // value 16 is used in EXIF V2.3, not for EXIF V2.2
    EXIF_LIGHT_SOURCE_STANDARD_LIGHT_A,
    EXIF_LIGHT_SOURCE_STANDARD_LIGHT_B,
    EXIF_LIGHT_SOURCE_STANDARD_LIGHT_C,
    EXIF_LIGHT_SOURCE_D55,
    EXIF_LIGHT_SOURCE_D65,
    EXIF_LIGHT_SOURCE_D75,
    EXIF_LIGHT_SOURCE_D50,
    EXIF_LIGHT_SOURCE_ISO_STUDIO_TUNGSTEN,
    EXIF_LIGHT_SOURCE_OTHER_LIGHT_SOURCE = 255,
} CamExifLightSourceType;

typedef enum {
    EXIF_EXPOSURE_PROGRAM_MANUAL = 1,
    EXIF_EXPOSURE_PROGRAM_NORMAL = 2,
    EXIF_EXPOSURE_PROGRAM_APERTURE_PRIORITY = 3,
    EXIF_EXPOSURE_PROGRAM_SHUTTER_PRIORITY = 4
} CamExifExposureProgramType;

typedef enum {
    EXIF_CONTRAST_NORMAL = 0,
    EXIF_CONTRAST_SOFT = 1,
    EXIF_CONTRAST_HARD = 2,
} CamExifContrastType;

typedef enum {
    EXIF_SATURATION_NORMAL = 0,
    EXIF_SATURATION_LOW = 1,
    EXIF_SATURATION_HIGH = 2,
} CamExifSaturationType;

typedef enum {
    EXIF_SHARPNESS_NORMAL = 0,
    EXIF_SHARPNESS_SOFT = 1,
    EXIF_SHARPNESS_HARD = 2,
} CamExifSharpnessType;

/* define the flag of enable gps info */
const uint8_t EXIF_GPS_LATITUDE = 0x01;
const uint8_t EXIF_GPS_LONGITUDE = 0x02;
const uint8_t EXIF_GPS_ALTITUDE = 0x04;
const uint8_t EXIF_GPS_TIMESTAMP = 0x08;
const uint8_t EXIF_GPS_PROCMETHOD = 0x10;
const uint8_t EXIF_GPS_IMG_DIRECTION = 0x20;

/* Values */
#define EXIF_DEF_IMAGE_DESCRIPTION "Jpeg"
#define EXIF_DEF_SOFTWARE "Android"
#define EXIF_DEF_EXIF_VERSION "0220"
#define EXIF_DEF_USERCOMMENTS "  "
#define EXIF_DEF_FLASHPIXVERSION "0100" /* Flashpix Format Version 1.0 */

#define EXIF_DEF_YCBCR_POSITIONING 1 /* centered */
#define EXIF_DEF_FNUMBER_NUM 26      /* 2.6 */
#define EXIF_DEF_FNUMBER_DEN 10
#define EXIF_DEF_EXPOSURE_PROGRAM 3 /* aperture priority */
#define EXIF_DEF_FOCAL_LEN_NUM 278  /* 2.78mm */
#define EXIF_DEF_FOCAL_LEN_DEN 100
#define EXIF_DEF_FLASH 0                 /* O: off, 1: on*/
#define EXIF_FLASH_FORCED_ON 1 << 3;     /* mode description */
#define EXIF_FLASH_FORCED_OFF 1 << 4;    /* mode description */
#define EXIF_FLASH_AUTO 1 << 3 | 1 << 4; /* mode description */
#define EXIF_FLASH_ON 1                  /* O: off, 1: on - fired or not*/
#define EXIF_DEF_COLOR_SPACE 1
#define EXIF_DEF_CUSTOM_RENDERED 0
#define EXIF_CUSTOM_RENDERED_HDR 1
#define EXIF_DEF_EXPOSURE_MODE EXIF_EXPOSURE_AUTO
#define EXIF_DEF_APEX_DEN 10
#define EXIF_DEF_APEX_NUM 25
#define EXIF_DEF_SUBJECT_DISTANCE_UNKNOWN 0

#define EXIF_DEF_COMPRESSION 6
#define EXIF_DEF_RESOLUTION_NUM 72
#define EXIF_DEF_RESOLUTION_DEN 1
#define EXIF_DEF_RESOLUTION_UNIT 2 /* inches */

typedef struct {
    uint32_t num;
    uint32_t den;
} rational_t;

typedef struct {
    int32_t num;
    int32_t den;
} srational_t;

typedef struct {
    bool enableThumb;

    uint8_t image_description[32];
    uint8_t flashpix_version[4];
    uint8_t components_configuration[4];
    uint8_t maker[32];
    uint8_t model[32];
    uint8_t software[32];
    uint8_t exif_version[4];
    uint8_t date_time[20];
    uint8_t subsec_time[8];
    uint8_t user_comment[150];

    uint32_t width;
    uint32_t height;
    uint32_t widthThumb;
    uint32_t heightThumb;

    uint16_t orientation;
    uint16_t ycbcr_positioning;
    uint16_t exposure_program;
    uint16_t iso_speed_rating;
    uint16_t metering_mode;
    uint16_t flash;
    uint16_t color_space;
    uint16_t custom_rendered;
    uint16_t exposure_mode;
    uint16_t white_balance;
    rational_t zoom_ratio;
    uint16_t scene_capture_type;
    uint16_t light_source;
    uint16_t gain_control;
    uint16_t contrast;
    uint16_t saturation;
    uint16_t sharpness;

    rational_t exposure_time;
    rational_t fnumber;
    rational_t aperture;
    rational_t max_aperture;
    rational_t focal_length;
    rational_t subject_distance;

    srational_t shutter_speed;
    srational_t brightness;
    srational_t exposure_bias;

    // bit 0~4 indicate whether Gps items latitude, longitude, altitude, timestamp,
    // datastamp exist or not.
    uint8_t enableGps;
    uint8_t gps_latitude_ref[2];
    uint8_t gps_longitude_ref[2];

    uint8_t gps_version_id[4];
    uint8_t gps_altitude_ref;

    rational_t gps_latitude[3];
    rational_t gps_longitude[3];
    rational_t gps_altitude;
    rational_t gps_timestamp[3];
    uint8_t gps_datestamp[11];
    uint8_t gps_processing_method[100];

    uint8_t gps_img_direction_ref[2];
    rational_t gps_img_direction;

    rational_t x_resolution;
    rational_t y_resolution;
    uint16_t resolution_unit;
    uint16_t compression_scheme;

    uint16_t makerNoteDataSize;
    unsigned char* makerNoteData;
    bool makernoteToApp2;
} exif_attribute_t;
