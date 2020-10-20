/*
 * Copyright (C) 2013 The Android Open Source Project
 * Copyright (C) 2015-2020 Intel Corporation.
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

/*
 *
 * Filename: Parameters.h
 *
 * ------------------------------------------------------------------------------
 * REVISION HISTORY
 *     Version        0.1        Initialize camera parameters API
 *     Version        0.2        Merge all the types to this file
 *     Version        0.3        Add AE compensation related APIs.
 *     Version        0.31       Add manual color matrix APIs.
 *     Version        0.32       Add manual AE/AWB converge speed APIs.
 *     Version        0.33       Add timestamp variable in camera_buffer_t
 *     Version        0.34       Add AE window weight grid API
 *     Version        0.40       Add Data Structure for HAL 3.3
 *     Version        0.41       Add API getSupportedAeExposureTimeRange
 *                               Add API getSupportedAeGainRange
 *     Version        0.42       Add API updateDebugLevel
 *     Version        0.43       Add API set and get deinterlace mode
 *     Version        0.44       Add API set and get gps processing method
 *                               Add API set and get focal length
 *     Version        0.45       Add get supported static metadata APIs
 *     Version        0.50       Support low level ISP feature control
 *     Version        0.51       Support getting supported ISP control feature list
 *     Version        0.52       Add API set and get awb result
 *     Version        0.53       Add API to get/set enabled ISP control feature list
 *     Version        0.54       Add API to get/set fisheye dewarping mode
 *     Version        0.55       Add API to get/set LTM tuning data
 *     Version        0.56       Add API to get/set LDC/RSC/digital zoom ratio
 *     Version        0.58       Add API to get/set 3A state, and lens state.
 *     Version        0.59       Add API to get/set AE/AWB lock
 *     Version        0.61       Add API to support vertical and horizontal flip.
 *     Version        0.62       Add API to support 3A cadence.
 *     Version        0.63       Add API to enable/disable MONO Downscale feature.
 *     Version        0.64       Add callback message definition.
 *     Version        0.65       Add API to support OUTPUT/INPUT streams.
 *     Version        0.66       modifies callback message definition.
 *     Version        0.67       Add API to support lens.focusDistance and lens.focalLength
 *     Version        0.68       Add API to support shading map.
 *     Version        0.69       Add API to support statistics lens shading map control.
 *     Version        0.70       Add API to support tonemap.
 *     Version        0.71       Add API to support OPAQUE RAW usage for RAW reprocessing.
 *     Version        0.72       Add streamType into supported_stream_config_t.
 *     Version        0.73       Remove supported_stream_config_t structure.
 *     Version        0.74       Add API to support sensor iso.
 *     Version        0.75       Add API to support lens static info about apertures,
 *                               filter densities, min focus densities and hyperfocal distance.
 *     Version        0.76       Remove the marco for lsc grid size
 *     Version        0.77       Add API to support capture intent
 *     Version        0.78       Add API to support edge enhancement
 *
 *
 * ------------------------------------------------------------------------------
 */

#pragma once

#include <vector>
#include <set>
#include <stdint.h>

namespace icamera {

/***************Start of Camera Basic Data Structure ****************************/
/**
 * Basic definition will be inherited by more complicated structure.
 * MUST be all "int" in this structure.
 */
typedef struct {
    int width;
    int height;
} camera_resolution_t;

/**
 * \struct stream_t: stream basic info
 *
 * \note
 *   MUST use int if new member added.
 */
typedef struct {
    int format;    /**< stream format refer to v4l2 definition https://linuxtv.org/downloads/v4l-dvb-apis/pixfmt.html */
    int width;     /**< image width */
    int height;    /**< image height */
    int field;     /**< refer to v4l2 definition https://linuxtv.org/downloads/v4l-dvb-apis/field-order.html#v4l2-field */

/*
* The buffer geometry introduction.
* The YUV image is formed with Y:Luma and UV:Chroma. And there are
* two kinds of styles for YUV format: planar and packed.
*
*   YUV420:NV12
*
*            YUV420(720x480) sampling
*
*       |<----width+padding=alignedBpl----->|
*     Y *-------*-------*-------*-------*....-----
*       |                               |   :  ^
*       |   # UV            #           |   :  |
*       |                               |   :  |
*       *-------*-------*-------*-------*....  |
*       |                               |   :  |
*       |   #               #           |   :  |
*       |                               |   :  |
*       *-------*-------*-------*-------*.... (height * 3 / 2)
*       |                               |   :  |
*       |   #               #           |   :  |
*       |                               |   :  |
*       *-------*-------*-------*-------*....  |
*       |                               |   :  |
*       |   #               #           |   :  |
*       |                               |   :  v
*       *-------*-------*-------*-------*....-----
*
*         The data stored in memory
*          ____________w___________ .....
*         |Y0|Y1                   |    :
*         |                        |    :
*         h                        h    :
*         |                        |    :
*         |                        |    :
*         |________________________|....:
*         |U|V|U|V                 |    :
*        h/2                      h/2   :
*         |____________w___________|....:
*
*       bpp = 12
*       bpl = width;
*       stride = align64(bpl):
*
*   YUV422:YUY2
*
*           YUV422(720x480) sampling
*
*       |<--(width*2)+padding=alignedBpl-->|
*   YUV *#----*#-----*#-----*#-----*#....-----
*       *#----*#-----*#-----*#-----*#....  |
*       *#----*#-----*#-----*#-----*#....  |
*       *#----*#-----*#-----*#-----*#....  |
*       *#----*#-----*#-----*#-----*#.... (height)
*       *#----*#-----*#-----*#-----*#....  |
*       *#----*#-----*#-----*#-----*#....  |
*       *#----*#-----*#-----*#-----*#....  |
*       *#----*#-----*#-----*#-----*#....  |
*       *#----*#-----*#-----*#-----*#....-----
*
*         The data stored in memory
*          ____________w___________ .....
*         |Y0|Cb|Y1|Cr             |    :
*         |                        |    :
*         |                        |    :
*         |                        |    :
*         h                        h    :
*         |                        |    :
*         |                        |    :
*         |                        |    :
*         |____________w___________|....:
*
*       bpp = 16
*       bpl = width * bpp / 8 = width * 2;
*       stride = align64(bpl):
*
*       Note: The stride defined in HAL is same as aligned bytes per line.
*/
    int stride;    /**< stride = aligned bytes per line */
    int size;      /**< real buffer size */

    int id;        /**< Id that is filled by HAL. */
    int memType;   /**< buffer memory type filled by app, refer to https://linuxtv.org/downloads/v4l-dvb-apis/io.html */

    /**
     * The maximum number of buffers the HAL device may need to have dequeued at
     * the same time. The HAL device may not have more buffers in-flight from
     * this stream than this value.
     */
    uint32_t max_buffers;

    int usage; /**<The usage of this stream defined in camera_stream_usage_t. */
    int streamType; /**<The stream type of this stream defined in camera_stream_type_t. */
} stream_t;

typedef std::vector<stream_t> stream_array_t;

/**
 * \struct stream_config_t: stream configuration info
 *
 * Contains all streams info in this configuration.
 */
typedef struct {
    int num_streams; /**< number of streams in this configuration */
    stream_t    *streams; /**< streams list */
    /**
     * The operation mode of the streams in this configuration. It should be one of the value
     * defined in camera_stream_configuration_mode_t.
     * The HAL uses this mode as an indicator to set the stream property (e.g.,
     * camera_stream->max_buffers) appropriately. For example, if the configuration is
     * CAMERA_STREAM_CONFIGURATION_CONSTRAINED_HIGH_SPEED_MODE, the HAL may want to set aside more
     * buffers for batch mode operation (see camera.control.availableHighSpeedVideoConfigurations
     * for batch mode definition).
     */
    uint32_t operation_mode;
} stream_config_t;

/**
 * \struct camera_buffer_flags_t: Specify a buffer's properties.
 *
 * The buffer's properties can be one of them or combined with some of them.
 */
typedef enum {
    BUFFER_FLAG_DMA_EXPORT = 1<<0,
    BUFFER_FLAG_INTERNAL = 1<<1,
    BUFFER_FLAG_SW_READ = 1<<2,
    BUFFER_FLAG_SW_WRITE = 1<<3,
} camera_buffer_flags_t;

/**
 * \struct camera_buffer_t: camera buffer info
 *
 * camera buffer is used to carry device frames. Application allocate buffer structure,
 * according to memory type to allocate memory and queue to device.
 */
typedef struct {
    stream_t s;   /**< stream info */
    void *addr;   /**< buffer addr for userptr and mmap memory mode */
    int index;    /**< buffer index, filled by HAL. it is used for qbuf and dqbuf in order */
    long sequence; /**< buffer sequence, filled by HAL, to record buffer dqueue sequence from device */
    int dmafd;    /**< buffer dmafd for DMA import and export mode */
    int flags;    /**< buffer flags, its type is camera_buffer_flags_t, used to specify buffer properties */
    uint64_t timestamp; /**< buffer timestamp, it's a time reference measured in nanosecond */
    int reserved; /**< reserved for future */
} camera_buffer_t;

/**
 * camera_stream_type_t:
 *
 * The type of the camera stream, which defines whether the camera HAL device
 * is the producer or the consumer for that stream, and how the buffers of that
 * stream relate to the other streams.
 */
typedef enum {
    /**
     * This stream is an output stream; the camera HAL device will be responsible to
     * fill the buffers of this stream with newly captured or reprocessed image data.
     */
    CAMERA_STREAM_OUTPUT = 0,

    /**
     * This stream is an input stream; the camera HAL device will be responsible
     * to read buffers from this stream and to send them through the camera
     * processing pipeline, as if the buffer was a newly captured image from
     * the imager.
     *
     * The pixel format for an input stream can be any format reported by
     * camera.scaler.availableInputOutputFormatsMap. The pixel format of the
     * output stream used to produce the reprocessing data may be any format
     * reported by camera.scaler.availableStreamConfigurations. The supported
     * inputoutput stream combinations depends on the camera device capabilities.
     * See camera.scaler.availableInputOutputFormatsMap for stream map details.
     *
     * This kind of stream is generally used to reprocess data into higher
     * quality images (that otherwise would cause a frame rate performance loss),
     * or to do off-line reprocessing.
     * The typical use cases are OPAQUE (typically ZSL) and YUV reprocessing.
     */
    CAMERA_STREAM_INPUT = 1,

    /**
     * This stream can be used for input and output. Typically, the stream is
     * used as an output stream, but occasionally one already-filled buffer may
     * be sent back to the HAL device for reprocessing.
     *
     * This kind of stream is generally meant for Zero Shutter Lag (ZSL)
     * features, where copying the captured image from the output buffer to the
     * reprocessing input buffer would be expensive.
     *
     * Note that the HAL will always be reprocessing data it produced.
     *
     */
    CAMERA_STREAM_BIDIRECTIONAL = 2,

    /**
     * Total number of framework-defined stream types
     */
    CAMERA_NUM_STREAM_TYPES

} camera_stream_type_t;

/**
 * camera_stream_usage_t:
 *
 * The type of the camera stream, which defines whether the camera HAL device
 * is the producer or the consumer for that stream, and how the buffers of that
 * stream relate to the other streams.
 */
typedef enum {
    /**
     * This stream is an output stream for preview;
     */
    CAMERA_STREAM_PREVIEW = 0,

    /**
     * This stream is an output stream for VIDEO CAPTURE;
     */
    CAMERA_STREAM_VIDEO_CAPTURE,

    /**
     * This stream is an output stream for STILL IMAGE CAPTURE;
     */
    CAMERA_STREAM_STILL_CAPTURE,

    /**
     * This stream is an output stream for Application processing which is accessed by CPU;
     */
    CAMERA_STREAM_APP,

    /**
     * This stream is an output stream for Opaque RAW reprocess.
     */
    CAMERA_STREAM_OPAQUE_RAW,
} camera_stream_usage_t;

/**
 * camera_stream_configuration_mode_t:
 *
 * This defines the general operation mode for the HAL (for a given stream configuration), where
 * modes besides NORMAL have different semantics, and usually the generality of the APIs are
 * limited in exchange for higher performance in some particular area.
 */
typedef enum {
    /**
     * Normal stream configuration operation mode.
     * This is the default camera operation mode, where all semantics of HAL APIs and metadata
     * controls apply.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_NORMAL = 0,

    /**
     * CONSTRAINED_HIGH_SPEED is the special constrained high speed operation mode for devices
     * that do not support high speed output in NORMAL mode.
     * To support this configuration mode, camera.control.availableHighSpeedVideoConfigurations
     * should be implemented and CONSTRAINED_HIGH_SPEED should be reported in
     * camera.request.availableCapabilities.
     * All streams in this configuration mode operate at high speed mode and have different
     * characteristics and limitations to achieve high speed output. The NORMAL mode can also be
     * used for high speed output, if the HAL supports high speed output while satisfying all the
     * semantics of HAL APIs and metadata controls. It is recommended for the HAL to support high
     * speed output in NORMAL mode (by advertising the high speed FPS ranges in
     * camera.control.aeAvailableTargetFpsRanges) if possible.
     *
     * This mode has below limitations/requirements:
     *
     *   1. The HAL must support up to 2 streams with sizes reported by
     *       camera.control.availableHighSpeedVideoConfigurations.
     *   2. In this mode, the HAL is expected to output up to 120fps or higher. It must
     *       support the targeted FPS range and resolution configurations reported by
     *       camera.control.availableHighSpeedVideoConfigurations.
     *   3. To achieve efficient high speed streaming, the HAL may have to aggregate multiple
     *       frames together and send the batch to camera device for processing there the request
     *       controls are same for all the frames in this batch (batch mode). The HAL must
     *       support the max batch size. And the max batch size requirements are defined by
     *       camera.control.availableHighSpeedVideoConfigurations.
     *   4. The HAL will override {aeMode, awbMode, afMode} to {ON, ON, CONTINUOUS_VIDEO}.
     *       All post-processing block mode controls must be overridden to be FAST. Therefore, no
     *       manual control of capture and post-processing parameters is possible. All other
     *       controls operate the same as when camera.control.mode == AUTO.
     *       This means that all other camera.control.* fields must continue to work, such as
     *           camera.control.aeTargetFpsRange
     *           camera.control.aeExposureCompensation
     *           camera.control.aeLock
     *           camera.control.awbLock
     *           camera.control.effectMode
     *           camera.control.aeRegions
     *           camera.control.afRegions
     *           camera.control.awbRegions
     *           camera.control.afTrigger
     *           camera.control.aePrecaptureTrigger
     *       Outside of camera.control.*, the following controls must work:
     *           camera.flash.mode (TORCH mode only, automatic flash for still capture will not
     *                                           work since aeMode is ON)
     *           camera.lens.opticalStabilizationMode (if it is supported)
     *           camera.scaler.cropRegion
     *           camera.statistics.faceDetectMode (if it is supported)
     *
     * TODO: The high speed mode is not completely supported yet.
     *       1) Now the HAL supports up to 60fps@1080p.
     *       2) The static metadata camera.control.availableHighSpeedVideoConfigurations should be
     *           implemented.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_CONSTRAINED_HIGH_SPEED = 1,

    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_AUTO is a configurable mode, but not a real
     * mode in HAL. The user uses this mode to allow the HAL selects appropriate config mode
     * internally, so it should NOT be regarded as specific ConfigMode, but operation mode only.
     *
     * TuningModes used in AUTO mode depends on ConfigMode the HAL selects.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_AUTO,
    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_HDR is used to select PSYS pipeline,
     * TuningMode and MediaCtlConfig HDR pipe.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_HDR,
    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_ULL is used to select PSYS pipeline,
     * TuningMode and MediaCtlConfig ULL pipe.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_ULL,
    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_HLC is used to select PSYS pipeline,
     * TuningMode and MediaCtlConfig HLC pipe.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_HLC,
    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_CUSTOM_AIC is used to select PSYS pipeline,
     * TuningMode and MediaCtlConfig CUSTOM_AIC pipe.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_CUSTOM_AIC,

    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_VIDEO_LL is used to select PSYS pipeline,
     * TuningMode and MediaCtlConfig Video LL pipe.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_VIDEO_LL,

    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_STILL_CAPTURE is used to select PSYS pipeline,
     * Create only still pipe
     * TuningMode and MediaCtlConfig still pipe.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_STILL_CAPTURE,

    /**
     * CAMERA_STREAM_CONFIGURATION_MODE_HDR2 is used to select PSYS pipeline,
     * TuningMode and MediaCtlConfig HDR2 pipe.
     */
    CAMERA_STREAM_CONFIGURATION_MODE_HDR2,

    CAMERA_STREAM_CONFIGURATION_MODE_END
} camera_stream_configuration_mode_t;

/***************End of Camera Basic Data Structure ****************************/

/*******************Start of Camera Parameters Definition**********************/
/**
 * \enum camera_features: camera supportted features.
 */
typedef enum {
    MANUAL_EXPOSURE,       /**< Allow user to controll exposure time and ISO manually */
    MANUAL_WHITE_BALANCE,  /**< Allow user to controll AWB mode, cct range, and gain */
    IMAGE_ENHANCEMENT,     /**< Sharpness, Brightness, Contrast, Hue, Saturation */
    NOISE_REDUCTION,       /**< Allow user to control NR mode and NR level */
    SCENE_MODE,            /**< Allow user to control scene mode */
    WEIGHT_GRID_MODE,      /**< Allow user to control custom weight grid mode */
    PER_FRAME_CONTROL,     /**< Allow user to control most of parameters for each frame */
    ISP_CONTROL,           /**< Allow user to control low level ISP features */
    INVALID_FEATURE
} camera_features;
typedef std::vector<camera_features> camera_features_list_t;

/**
 * \struct camera_range_t: Used to specify the range info for something like FPS.
 */
typedef struct {
    float min;
    float max;
} camera_range_t;
typedef std::vector<camera_range_t> camera_range_array_t;

/**
 * \enum camera_ae_mode_t: Used to control how AE works.
 */
typedef enum {
    AE_MODE_AUTO,   /**< */
    AE_MODE_MANUAL, /**< */
    AE_MODE_MAX     /**< Invalid AE mode, any new mode should be added before this */
} camera_ae_mode_t;

typedef enum {
    AE_STATE_NOT_CONVERGED,
    AE_STATE_CONVERGED
} camera_ae_state_t;

/**
 * \enum camera_antibanding_mode_t: Used to control antibanding mode.
 */
typedef enum {
    ANTIBANDING_MODE_AUTO, /**< Auto detect the flicker frequency. */
    ANTIBANDING_MODE_50HZ, /**< Specify the flicker frequency to 50Hz. */
    ANTIBANDING_MODE_60HZ, /**< Specify the flicker frequency to 60Hz. */
    ANTIBANDING_MODE_OFF,  /**< Do not try to remove the flicker. */
} camera_antibanding_mode_t;

/**
 * \enum camera_scene_mode_t: Used to control scene mode.
 *
 * Different scene mode may have different WB effects or different exposure behavior.
 */
typedef enum {
    SCENE_MODE_AUTO,
    SCENE_MODE_HDR,
    SCENE_MODE_ULL,
    SCENE_MODE_HLC,
    SCENE_MODE_NORMAL,
    SCENE_MODE_CUSTOM_AIC,
    SCENE_MODE_VIDEO_LL,
    SCENE_MODE_STILL_CAPTURE,
    SCENE_MODE_HDR2,
    SCENE_MODE_MAX
} camera_scene_mode_t;

/**
 * \struct camera_ae_exposure_time_range_t: Provide supported exposure time range info per scene mode.
 */
typedef struct {
    camera_scene_mode_t scene_mode;
    camera_range_t et_range; /**< The exposure time range whose unit is us. */
} camera_ae_exposure_time_range_t;

/**
 * \struct camera_ae_gain_range_t: Provide supported gain range info per scene mode.
 */
typedef struct {
    camera_scene_mode_t scene_mode;
    camera_range_t gain_range; /**< The available sensor gain range whose unit is db. */
} camera_ae_gain_range_t;

/*
 * \enum camera_weight_grid_mode_t: Use to select which customized weight grid should be used.
 */
typedef enum {
    WEIGHT_GRID_AUTO,
    CUSTOM_WEIGHT_GRID_1,
    CUSTOM_WEIGHT_GRID_2,
    CUSTOM_WEIGHT_GRID_3,
    CUSTOM_WEIGHT_GRID_4,
    CUSTOM_WEIGHT_GRID_5,
    CUSTOM_WEIGHT_GRID_6,
    CUSTOM_WEIGHT_GRID_7,
    CUSTOM_WEIGHT_GRID_8,
    CUSTOM_WEIGHT_GRID_9,
    CUSTOM_WEIGHT_GRID_10,
    CUSTOM_WEIGHT_GRID_MAX
} camera_weight_grid_mode_t;

/**
 * \enum camera_yuv_color_range_mode_t: Specify which YUV color range will be used.
 */
typedef enum {
    CAMERA_FULL_MODE_YUV_COLOR_RANGE,       /*!< Full range (0 - 255) YUV data. */
    CAMERA_REDUCED_MODE_YUV_COLOR_RANGE     /*!< Reduced range aka. BT.601 (16-235) YUV data range. */
} camera_yuv_color_range_mode_t;

/**
 * \enum camera_awb_mode_t: Used to control AWB working mode.
 */
typedef enum {
    AWB_MODE_AUTO,
    AWB_MODE_INCANDESCENT,
    AWB_MODE_FLUORESCENT,
    AWB_MODE_DAYLIGHT,
    AWB_MODE_FULL_OVERCAST,
    AWB_MODE_PARTLY_OVERCAST,
    AWB_MODE_SUNSET,
    AWB_MODE_VIDEO_CONFERENCE,
    AWB_MODE_MANUAL_CCT_RANGE,
    AWB_MODE_MANUAL_WHITE_POINT,
    AWB_MODE_MANUAL_GAIN,
    AWB_MODE_MANUAL_COLOR_TRANSFORM,
    AWB_MODE_MAX
} camera_awb_mode_t;

typedef enum {
    AWB_STATE_NOT_CONVERGED,
    AWB_STATE_CONVERGED
} camera_awb_state_t;

/**
 * \enum camera_af_mode_t: Used to control af working mode.
 *
 * OFF:
 * Af algo is disabled, len position is controlled by application if supported.
 *
 * AUTO:
 * In this mode, the lens does not move unless the af trigger is activated.
 * The af algo will update af state every frame, and lock lens position when action is
 * completed.
 * The af trigger can be activated repeatedly.
 * Cancelling af trigger resets the lens position to default.
 *
 * MACRO:
 * Similar to AUTO and focus on objects very close to the camera.
 *
 * CONTINUOUS_VIDEO:
 * In this mode, the af algo modifies the lens position continually to
 * attempt to provide a constantly-in-focus image stream.
 * When the af trigger is activated,  af algo locks the lens position
 * until a cancel AF trigger is received.
 *
 * CONTINUOUS_PICTURE:
 * Similar to CONTINUOUS_VIDEO, except:
 * When the af trigger is activated, af algo can finish the current scan
 * before locking the lens position.
 *
 * Please refer to camera_af_trigger_t about how to trigger auto focus.
 * Please refer to camera_af_state_t about how to get autofocus result.
 */
typedef enum {
    AF_MODE_OFF,
    AF_MODE_AUTO,
    AF_MODE_MACRO,
    AF_MODE_CONTINUOUS_VIDEO,
    AF_MODE_CONTINUOUS_PICTURE,
    AF_MODE_MAX,
} camera_af_mode_t;

/**
 * \enum camera_af_trigger_t: Used trigger/cancel autofocus
 *
 * When af algo is enabled and it is changed to START, the HAL will
 * trigger autofocus.
 * When it is changed to CANCEL, the HAL will cancel any active trigger.
 *
 * Generally, applications should set it to START or CANCEL for only a
 * single frame capture, and then return it to IDLE, to get ready for
 * the next action.
 */
typedef enum {
    AF_TRIGGER_IDLE,
    AF_TRIGGER_START,
    AF_TRIGGER_CANCEL,
} camera_af_trigger_t;

/**
 * \enum camera_af_state_t: Used to return af state.
 */
typedef enum {
    AF_STATE_IDLE,               /*!< Focus is idle */
    AF_STATE_LOCAL_SEARCH,       /*!< Focus is in local search state */
    AF_STATE_EXTENDED_SEARCH,    /*!< Focus is in extended search state */
    AF_STATE_SUCCESS,            /*!< Focus has succeeded */
    AF_STATE_FAIL                /*!< Focus has failed */
} camera_af_state_t;

/**
 * \enum camera_awb_mode_t: Used to control which preset effect will be used.
 */
typedef enum {
    CAM_EFFECT_NONE = 0,
    CAM_EFFECT_MONO,
    CAM_EFFECT_SEPIA,
    CAM_EFFECT_NEGATIVE,
    CAM_EFFECT_SKY_BLUE,
    CAM_EFFECT_GRASS_GREEN,
    CAM_EFFECT_SKIN_WHITEN_LOW,
    CAM_EFFECT_SKIN_WHITEN,
    CAM_EFFECT_SKIN_WHITEN_HIGH,
    CAM_EFFECT_VIVID,
} camera_effect_mode_t;

/**
 * \enum camera_test_pattern_mode_t: Use to control test pattern mode.
 */
typedef enum {
    TEST_PATTERN_OFF = 0,
    SOLID_COLOR,
    COLOR_BARS,
    COLOR_BARS_FADE_TO_GRAY,
    PN9,
    TEST_PATTERN_CUSTOM1,
} camera_test_pattern_mode_t;

/**
 * \enum camera_tonemap_mode_t: Use to control tonemap mode.
 */
typedef enum {
    TONEMAP_MODE_CONTRAST_CURVE,
    TONEMAP_MODE_FAST,
    TONEMAP_MODE_HIGH_QUALITY,
    TONEMAP_MODE_GAMMA_VALUE,
    TONEMAP_MODE_PRESET_CURVE,
} camera_tonemap_mode_t;

/**
 * \enum camera_tonemap_preset_curve_t: Use to control preset curve type.
 */
typedef enum {
    TONEMAP_PRESET_CURVE_SRGB,
    TONEMAP_PRESET_CURVE_REC709,
} camera_tonemap_preset_curve_t;

typedef struct {
    int32_t rSize;
    int32_t bSize;
    int32_t gSize;
    const float* rCurve;
    const float* bCurve;
    const float* gCurve;
} camera_tonemap_curves_t;

/**
 * \enum camera_msg_type_t: Use to indicate the type of message sent.
 */
typedef enum {
    CAMERA_EVENT_NONE = 0,
    CAMERA_ISP_BUF_READY,
    CAMERA_DEVICE_ERROR,
    CAMERA_IPC_ERROR,
} camera_msg_type_t;

/**
 * \struct Sensor RAW data info for ZSL.
 */
typedef struct {
    long sequence;
    uint64_t timestamp;
} sensor_raw_info_t;

/**
 * \struct isp_buffer_ready_t: Use to send isp buffer ready event data.
 */
typedef struct {
    uint32_t frameNumber;
    uint64_t timestamp;
    long sequence;
} isp_buffer_ready_t;

/**
 * \struct camera_msg_data_t: Use to specify msg data.
 */
typedef struct {
    camera_msg_type_t type;
    union {
        isp_buffer_ready_t buffer_ready;
    } data;
} camera_msg_data_t;

/**
 * \struct camera_callback_ops_t
 */
typedef struct camera_callback_ops {
    void (*notify)(const camera_callback_ops* cb, const camera_msg_data_t &data);
} camera_callback_ops_t;

/**
 * \struct camera_awb_gains_t: Used to specify AWB gain and AWB gain shift.
 */
typedef struct {
    int r_gain;
    int g_gain;
    int b_gain;
} camera_awb_gains_t;

/*!< camera_crop_region_t: Set crop region related parameters*/
typedef struct {
    int flag;
    int x;
    int y;
} camera_crop_region_t;

/**
 * \struct camera_color_transform_t: Specify the color transform matrix.
 */
typedef struct {
    float color_transform[3][3];
} camera_color_transform_t;

/**
 * \struct camera_color_gains_t: Specify the color correction gains.
 */
typedef struct {
    float color_gains_rggb[4];
} camera_color_gains_t;

/**
 * \enum camera_edge_mode_t: Specify the edge mode.
 */
typedef enum {
    EDGE_MODE_OFF,
    EDGE_MODE_FAST,
    EDGE_MODE_HIGH_QUALITY,
    EDGE_MODE_ZERO_SHUTTER_LAGE,
} camera_edge_mode_t;

/**
 * \enum camera_nr_mode_t: Specify the noise reduction mode.
 */
typedef enum {
    NR_MODE_OFF,
    NR_MODE_AUTO,
    NR_MODE_MANUAL_NORMAL,
    NR_MODE_MANUAL_EXPERT,
    NR_MODE_HIGH_QUALITY,
} camera_nr_mode_t;

/**
 * \struct camera_nr_level_t: Specify the noise reduction level.
 */
typedef struct {
    int overall;
    int spatial;
    int temporal;
} camera_nr_level_t;

/**
 * \enum camera_iris_mode_t: Specify the IRIS mode.
 */
typedef enum {
    IRIS_MODE_AUTO,
    IRIS_MODE_MANUAL,
    IRIS_MODE_CUSTOMIZED,
} camera_iris_mode_t;

/**
 * \enum camera_wdr_mode_t: Specify the WDR/HDR mode. (deprecated)
 */
typedef enum {
    WDR_MODE_AUTO,
    WDR_MODE_ON,
    WDR_MODE_OFF,
} camera_wdr_mode_t;

/**
 * \enum camera_blc_area_mode_t: Switch black area mode.
 */
typedef enum {
    BLC_AREA_MODE_OFF,
    BLC_AREA_MODE_ON,
} camera_blc_area_mode_t;

/**
 * \struct camera_window_t: Used to specify AE/AWB weighted regions.
 */
typedef struct {
    int left;
    int top;
    int right;
    int bottom;
    int weight;
} camera_window_t;
typedef std::vector<camera_window_t> camera_window_list_t;

/**
 * \struct camera_image_enhancement_t: Used to specify the image enhancement effect.
 */
typedef struct {
    int sharpness;
    int brightness;
    int contrast;
    int hue;
    int saturation;
} camera_image_enhancement_t;

/**
 * \struct camera_coordinate_t: The coordinate of a point in a specified coordinate system.
 */
typedef struct {
    int x;
    int y;
} camera_coordinate_t;

/**
 * \struct camera_coordinate_system_t: Used to specify the coordinate system.
 */
typedef struct {
    int left;   /*!< Left coordinate value in the coordinate system. */
    int top;    /*!< Top coordinate value in the coordinate system. */
    int right;  /*!< Right coordinate value in the coordinate system. */
    int bottom; /*!< Bottom coordinate value in the coordinate system. */
} camera_coordinate_system_t;

/**
 * \struct camera_rational_t: Used to present a rational.
 */
typedef struct {
    int numerator;
    int denominator;
} camera_rational_t;

/**
 * \struct camera_awb_result_t: Present AWB result.
 */
typedef struct {
    float r_per_g; /*!< Accurate White Point (R) for the image: relative value*/
    float b_per_g; /*!< Accurate White Point (B) for the image. relative value*/
} camera_awb_result_t;

/**
 * \enum camera_converge_speed_t: Used to control AE/AWB converge speed.
 */
typedef enum {
    CONVERGE_NORMAL,
    CONVERGE_MID,
    CONVERGE_LOW,
    CONVERGE_MAX
} camera_converge_speed_t;

/**
 * \enum camera_converge_speed_mode_t: Used to control AE/AWB converge speed mode.
 */
typedef enum {
    CONVERGE_SPEED_MODE_AIQ, /*!< Use AIQ Aglo to control converge speed. */
    CONVERGE_SPEED_MODE_HAL  /*!< Implement converge speed control in HAL. */
} camera_converge_speed_mode_t;

/**
 * \enum camera_ae_distribution_priority_t: Used to control exposure priority mode.
 */
typedef enum {
    DISTRIBUTION_AUTO,    /*!< The AIQ Aglo decides completely */
    DISTRIBUTION_SHUTTER, /*!< Shutter speed priority mode */
    DISTRIBUTION_ISO,     /*!< ISO priority mode */
    DISTRIBUTION_APERTURE /*!< Aperture priority mode */
} camera_ae_distribution_priority_t;

/**
 * \enum camera_deinterlace_mode_t: Used to control the deinterlace mode.
 */
typedef enum {
    DEINTERLACE_OFF,    /*!< Do not do any deinterlace */
    DEINTERLACE_WEAVING /*!< Weave the two frame buffers into one. */
} camera_deinterlace_mode_t;

/**
 * \enum camera_fisheye_dewarping_mode_t: Used to control the dewarping mode.
 */
typedef enum {
    FISHEYE_DEWARPING_OFF,
    FISHEYE_DEWARPING_REARVIEW,
    FISHEYE_DEWARPING_HITCHVIEW
} camera_fisheye_dewarping_mode_t;

/**
 * \enum camera_makernote_mode_t: Used to control makernote mode.
 */
typedef enum {
    MAKERNOTE_MODE_OFF,
    MAKERNOTE_MODE_JPEG,
    MAKERNOTE_MODE_RAW
} camera_makernote_mode_t;

/**
 * \enum camera_ldc_mode_t: Used to toggle lens distortion correction.
 */
typedef enum {
    LDC_MODE_OFF,
    LDC_MODE_ON
} camera_ldc_mode_t;

/**
 * \enum camera_rsc_mode_t: Used to toggle rolling shutter correction.
 */
typedef enum {
    RSC_MODE_OFF,
    RSC_MODE_ON
} camera_rsc_mode_t;

/**
 * \enum camera_flip_mode_t: Used to set output slip.
 */
typedef enum {
    FLIP_MODE_NONE = 0,
    FLIP_MODE_VFLIP,
    FLIP_MODE_HFLIP,
    FLIP_MODE_VHFLIP
} camera_flip_mode_t;

/**
 * \enum camera_mono_downscale_mode_t: Used to enable/disable MONO Downscale.
 */
typedef enum {
    MONO_DS_MODE_OFF,
    MONO_DS_MODE_ON
} camera_mono_downscale_mode_t;

/**
 * \enum camera_video_stabilization_mode_t: Used to control the video stabiliztion mode.
 */
typedef enum {
    VIDEO_STABILIZATION_MODE_OFF,
    VIDEO_STABILIZATION_MODE_ON
} camera_video_stabilization_mode_t;
typedef std::vector<camera_video_stabilization_mode_t> camera_video_stabilization_list_t;

/**
 * \enum camera_mount_type_t: camera mount type
 */
typedef enum {
    WALL_MOUNTED,
    CEILING_MOUNTED,
} camera_mount_type_t;

/**
* \enum camera_shading_mode_t: camera shading mode type
*/
typedef enum {
    SHADING_MODE_OFF,
    SHADING_MODE_FAST,
    SHADING_MODE_HIGH_QUALITY
} camera_shading_mode_t;

/**
* \enum camera_lens_shading_map_mode_type_t: camera lens shading map mode type
*/
typedef enum {
    LENS_SHADING_MAP_MODE_OFF,
    LENS_SHADING_MAP_MODE_ON
} camera_lens_shading_map_mode_type_t;

/**
 * \class Parameters
 *
 * \brief
 *   Manage parameter's data structure, and provide set and get parameters
 *
 * This class provides a thread safe management to internal parameter's data
 * structure, and helps client to easily set parameters to and get parameters
 * from camera device.
 *
 * \version 0.1
 *
 */
class Parameters {
public:
    Parameters();
    Parameters(const Parameters& other);
    Parameters& operator=(const Parameters& other);
    ~Parameters();
    /**
     * \brief Merge and update current parameter with other
     *
     * \param[in] Parameters other: parameter source
     *
     * \return void
     */
    void merge(const Parameters& other);

    // Belows are camera capability related parameters operations
    /**
     * \brief Get supported fps range list
     *
     * \param[out] camera_range_array_t& ranges
     *
     * \return 0 if fps range supported, otherwise non-0 value is returned.
     */
    int getSupportedFpsRange(camera_range_array_t& ranges) const;

    /**
     * \brief Get supported Stream Config list
     *
     * \param[out] stream_array_t& config
     *
     * \return 0 if Stream Configs supported, otherwise non-0 value is returned.
     */
    int getSupportedStreamConfig(stream_array_t& config) const;

    // Belows are camera capability related parameters operations
    /**
     * \brief Get supported sensor exposure time range (microsecond)
     *
     * \param[out] camera_range_t& range
     *
     * \return 0 if it is supported, otherwise non-0 value is returned.
     */
    int getSupportedSensorExposureTimeRange(camera_range_t& range) const;

    // Belows are camera capability related parameters operations
    /**
     * \brief Get supported sensor sensitivity time range
     *
     * \param[out] camera_range_t& range
     *
     * \return 0 if it is supported, otherwise non-0 value is returned.
     */
    int getSupportedSensorSensitivityRange(camera_range_t& range) const;

    /**
     * \brief Get supported feature list.
     *
     * Camera application MUST check if the feature is supported before trying to enable it.
     * Otherwise the behavior is undefined currently, HAL may just ignore the request.
     *
     * \param[out] camera_features_list_t& features: All supported feature will be filled in "features"
     *
     * \return: If no feature supported, features will be empty
     */
    int getSupportedFeatures(camera_features_list_t& features) const;

    /**
     * \brief Get ae compensation range supported by camera device
     *
     * \param[out] camera_range_t& evRange
     *
     * \return 0 if ae compensation supported, non-0 or evRange equals [0, 0] means ae compensation not supported.
     */
    int getAeCompensationRange(camera_range_t& evRange) const;

    /**
     * \brief Get ae compensation step supported by camera device
     *
     * Smallest step by which the exposure compensation can be changed.
     * This is the unit for setAeCompensation. For example, if this key has
     * a value of `1/2`, then a setting of `-2` for setAeCompensation means
     * that the target EV offset for the auto-exposure routine is -1 EV.
     *
     * One unit of EV compensation changes the brightness of the captured image by a factor
     * of two. +1 EV doubles the image brightness, while -1 EV halves the image brightness.
     *
     * \param[out] camera_rational_t& evStep
     *
     * \return 0 if ae compensation supported, non-0 means ae compensation not supported.
     */
    int getAeCompensationStep(camera_rational_t& evStep) const;

    /**
     * \brief Get supported manual exposure time range
     *
     * Different sensors or same sensor in different settings may have different supported exposure
     * time range, so camera application needs to use this API to check if the user's settings is
     * in the supported range, if application pass an out of exposure time, HAL will clip it
     * according to this supported range.
     *
     * \param[out] vector<camera_ae_exposure_time_range_t>& etRanges
     *
     * \return 0 if exposure time range is filled by HAL.
     */
    int getSupportedAeExposureTimeRange(std::vector<camera_ae_exposure_time_range_t>& etRanges) const;

    /**
     * \brief Get supported manual sensor gain range
     *
     * Different sensors or same sensor in different settings may have different supported sensor
     * gain range, so camera application needs to use this API to check if the user's settings is
     * in the supported range, if application pass an out of range gain, HAL will clip it according
     * to this supported range.
     *
     * \param[out] vector<camera_ae_gain_range_t>& gainRanges
     *
     * \return 0 if exposure time range is filled by HAL.
     */
    int getSupportedAeGainRange(std::vector<camera_ae_gain_range_t>& gainRanges) const;

    // Belows are AE related parameters operations
    /**
     * \brief Set exposure mode(auto/manual).
     *
     * "auto" means 3a algorithm will control exposure time and gain automatically.
     * "manual" means user can control exposure time or gain, or both of them.
     * Under manual mode, if user only set one of exposure time or gain, then 3a algorithm
     * will help to calculate the other one.
     *
     * \param[in] camera_ae_mode_t aeMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeMode(camera_ae_mode_t aeMode);

    /**
     * \brief Get exposure mode
     *
     * \param[out] aeMode: Currently used ae mode will be set to aeMode if 0 is returned.
     *
     * \return 0 if exposure mode was set, otherwise non-0 value is returned.
     */
    int getAeMode(camera_ae_mode_t& aeMode) const;

    /**
     * \brief Set AE state.
     *
     * \param[in] camera_ae_state_t aeState
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeState(camera_ae_state_t aeState);

    /**
     * \brief Get AE state
     *
     * \param[out] aeState: Currently AE state will be set to aeState if 0 is returned.
     *
     * \return 0 if AE state was set, otherwise non-0 value is returned.
     */
    int getAeState(camera_ae_state_t& aeState) const;

    /**
     * \brief Set ae lock
     *
     * \param[in] bool lock
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeLock(bool lock);

    /**
     * \brief Get ae lock
     *
     * \param[out] bool lock
     *
     * \return 0 if lock was set, otherwise non-0 value is returned.
     */
    int getAeLock(bool& lock) const;

    /**
     * \brief Get supported video stabilization mode
     *
     * Camera application MUST check if the video stabilization mode is supported before trying
     * to enable it. Otherwise one error occuring, HAL may just ignore the request.
     *
     * \param[out] supportedModes: All supported video stabilization mode will be filled in "supportedModes"
     *
     * \return: If no mode supported, supportedModes will be empty
     */
    int getSupportedVideoStabilizationMode(camera_video_stabilization_list_t &supportedModes) const;

    /**
     * \brief Get supported ae mode
     *
     * Camera application MUST check if the ae mode is supported before trying to enable it.
     * Otherwise one error occuring, HAL may just ignore the request.
     *
     * \param[out] supportedAeModes: All supported ae mode will be filled in "supportedAeModes"
     *
     * \return: If no ae mode supported, supportedAeModes will be empty
     */
    int getSupportedAeMode(std::vector<camera_ae_mode_t> &supportedAeModes) const;

    /**
     * \brief Get supported awb mode
     *
     * Camera application MUST check if the awb mode is supported before trying to enable it.
     * Otherwise one error occuring, HAL may just ignore the request.
     *
     * \param[out] supportedAwbModes: All supported awb mode will be filled in "supportedAwbModes"
     *
     * \return: If no awb mode supported, supportedAwbModes will be empty
     */
    int getSupportedAwbMode(std::vector<camera_awb_mode_t> &supportedAwbModes) const;

    /**
     * \brief Get supported af mode
     *
     * Camera application MUST check if the af mode is supported before trying to enable it.
     * Otherwise one error occuring, HAL may just ignore the request.
     *
     * \param[out] supportedAfModes: All supported af mode will be filled in "supportedAfModes"
     *
     * \return: If no af mode supported, supportedAfModes will be empty
     */
    int getSupportedAfMode(std::vector<camera_af_mode_t> &supportedAfModes) const;

    /**
     * \brief Get supported scene mode
     *
     * Camera application MUST check if the scene mode is supported before trying to enable it.
     * Otherwise one error occuring, HAL may just ignore the request.
     *
     * \param[out] supportedSceneModes: All supported scene mode will be filled in "supportedSceneModes"
     *
     * \return: If no scene mode supported, supportedSceneModes will be empty
     */
    int getSupportedSceneMode(std::vector<camera_scene_mode_t> &supportedSceneModes) const;

    /**
     * \brief Get supported antibanding mode
     *
     * Camera application MUST check if the antibanding mode is supported before trying to enable it.
     * Otherwise one error occuring, HAL may just ignore the request.
     *
     * \param[out] supportedAntibindingModes: All supported scene mode will be filled in "supportedAntibindingModes"
     *
     * \return: If no antibanding mode supported, supportedAntibindingModes will be empty
     */
    int getSupportedAntibandingMode(std::vector<camera_antibanding_mode_t> &supportedAntibindingModes) const;

    /**
     * \brief Get if ae lock is available
     *
     * Camera application MUST check if ae lock is supported before trying to lock it.
     * Otherwise one error occuring, HAL may just ignore the request.
     *
     * \return: true if lock is supported, false if not
     */
    bool getAeLockAvailable() const;

    /**
     * \brief Get if awb lock is available
     *
     * Camera application MUST check if awb lock is supported before trying to lock it.
     * Otherwise one error occuring, HAL may just ignore the request.
     *
     * \return: true if lock is supported, false if not
     */
    bool getAwbLockAvailable() const;

    /**
     * \brief Set AE region
     *
     * Current only fisrt region can take effect when BLC mode is BLC_AREA_MODE_ON;
     * if BLC_AREA_MODE_OFF, AE region function will be diabled.
     *
     * \param[in] camera_window_list_t aeRegions
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeRegions(camera_window_list_t aeRegions);

    /**
     * \brief Get AE region
     *
     * \param[out] camera_window_list_t aeRegions
     *
     * \return 0 if aeRegions were set, otherwise non-0 value is returned.
     */
    int getAeRegions(camera_window_list_t& aeRegions) const;

    /**
     * \brief Set exposure time whose unit is microsecond(us).
     *
     * The exposure time only take effect when ae mode set to manual.
     *
     * \param[in] int64_t exposureTime
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setExposureTime(int64_t exposureTime);

    /**
     * \brief Get exposure time whose unit is microsecond(us).
     *
     * \param[out] int64_t& exposureTime: exposure time if be set in exposureTime if 0 is returned.
     *
     * \return 0 if exposure time was set, non-0 means no exposure time was set.
     */
    int getExposureTime(int64_t& exposureTime) const;

    /**
     * \brief Set sensor gain whose unit is db.
     * The sensor gain only take effect when ae mode set to manual.
     *
     * \param[in] float gain
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSensitivityGain(float gain);

    /**
     * \brief Get sensor gain whose unit is db.
     *
     * \param[out] float gain
     *
     * \return 0 if sensor gain was set, non-0 means no sensor gain was set.
     */
    int getSensitivityGain(float& gain) const;

    /**
     * \brief Set sensor ISO, will overwrite value of setSensitivityGain.
     * The sensor ISO only take effect when ae mode set to manual.
     *
     * \param[in] int32 iso
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSensitivityIso(int32_t iso);

    /**
     * \brief Get sensor iso.
     *
     * \param[out] int32 iso
     *
     * \return 0 if sensor iso was set, non-0 means no sensor iso was set.
     */
    int getSensitivityIso(int& iso) const;

    /**
     * \brief Set ae compensation whose unit is compensation step.
     *
     * The adjustment is measured as a count of steps, with the
     * step size defined ae compensation step and the allowed range by ae compensation range.
     *
     * For example, if the exposure value (EV) step is 0.333, '6'
     * will mean an exposure compensation of +2 EV; -3 will mean an
     * exposure compensation of -1 EV. One EV represents a doubling of image brightness.
     *
     * In the event of exposure compensation value being changed, camera device
     * may take several frames to reach the newly requested exposure target.
     *
     * \param[in] int ev
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeCompensation(int ev);

    /**
     * \brief Get ae compensation whose unit is compensation step.
     *
     * \param[out] int ev
     *
     * \return 0 if ae compensation was set, non-0 means no ae compensation was set.
     */
    int getAeCompensation(int& ev) const;

    /**
     * \brief Set frame rate
     *
     * \param[in] float fps
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFrameRate(float fps);

    /**
     * \brief Get frame rate
     *
     * \param[out] float& fps
     *
     * \return 0 if frame rate was set, otherwise non-0 value is returned.
     */
    int getFrameRate(float& fps) const;

    /**
     * \brief Set antibanding mode
     *
     * \param[in] camera_antibanding_mode_t bandingMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAntiBandingMode(camera_antibanding_mode_t bandingMode);

    /**
     * \brief Get antibanding mode
     *
     * \param[out] camera_antibanding_mode_t& bandingMode
     *
     * \return 0 if antibanding mode was set, otherwise non-0 value is returned.
     */
    int getAntiBandingMode(camera_antibanding_mode_t& bandingMode) const;

    /**
     * \brief Set AE distribution priority.
     *
     * \param[in] camera_ae_distribution_priority_t priority: the AE distribution priority to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeDistributionPriority(camera_ae_distribution_priority_t priority);

    /**
     * \brief Get AE distribution priority.
     *
     * \param[out] camera_ae_distribution_priority_t priority: the AE distribution priority.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAeDistributionPriority(camera_ae_distribution_priority_t& priority) const;

    /**
     * \brief set exposure time range
     *
     * \param[in] camera_range_t: the exposure time range to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setExposureTimeRange(camera_range_t exposureTimeRange);

    /**
     * \brief get exposure time range
     *
     * \param[out] camera_range_t: the exposure time had been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getExposureTimeRange(camera_range_t& exposureTimeRange) const;

    /**
     * \brief set sensitivity gain range
     *
     * \param[in] camera_range_t: the sensitivity gain range(the unit is db) to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSensitivityGainRange(camera_range_t sensitivityGainRange);

    /**
     * \brief get sensitivity gain range
     *
     * \param[out] camera_range_t: the sensitivity gain(the unit is db) had been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getSensitivityGainRange(camera_range_t& sensitivityGainRange) const;

    /**
     * \brief Set weight grid mode
     *
     * \param[in] camera_weight_grid_mode_t weightGridMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setWeightGridMode(camera_weight_grid_mode_t weightGridMode);

    /**
     * \brief Get weight grid mode
     *
     * \param[out] camera_weight_grid_mode_t& weightGridMode
     *
     * \return 0 if weight grid mode was set, otherwise non-0 value is returned.
     */
    int getWeightGridMode(camera_weight_grid_mode_t& weightGridMode) const;

    /**
     * \brief Set BLC (backlight compensation) area mode
     *
     * \param[in] camera_blc_area_mode_t blcAreaMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setBlcAreaMode(camera_blc_area_mode_t blcAreaMode);

    /**
     * \brief Get BLC (backlight compensation) area mode
     *
     * \param[out] camera_blc_area_mode_t& blcAreaMode
     *
     * \return 0 if BLC area mode was set, otherwise non-0 value is returned.
     */
    int getBlcAreaMode(camera_blc_area_mode_t& blcAreaMode) const;

    int setFpsRange(camera_range_t fpsRange);
    int getFpsRange(camera_range_t& fpsRange) const;

    // Belows are AWB related parameters operations
    /**
     * \brief Set white balance mode
     *
     * White balance mode could be one of totally auto, preset cct range, customized cct range, customized
     * white area, customize gains.
     *
     * \param[in] camera_awb_mode_t awbMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbMode(camera_awb_mode_t awbMode);

    /**
     * \brief Get white balance mode currently used.
     *
     * \param[out] camera_awb_mode_t& awbMode
     *
     * \return 0 if awb mode was set, non-0 means no awb mode was set.
     */
    int getAwbMode(camera_awb_mode_t& awbMode) const;

    /**
     * \brief Set AWB state.
     *
     * \param[in] camera_awb_state_t awbState
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbState(camera_awb_state_t awbState);

    /**
     * \brief Get AWB state
     *
     * \param[out] awbState: Currently AWB state will be set to awbState if 0 is returned.
     *
     * \return 0 if AWB state was set, otherwise non-0 value is returned.
     */
    int getAwbState(camera_awb_state_t& awbState) const;

    /**
     * \brief Set awb lock
     *
     * \param[in] bool lock
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbLock(bool lock);

    /**
     * \brief Get awb lock
     *
     * \param[out] bool lock
     *
     * \return 0 if lock was set, otherwise non-0 value is returned.
     */
    int getAwbLock(bool& lock) const;

    /**
     * \brief Set customized cct range.
     *
     * Customized cct range only take effect when awb mode is set to AWB_MODE_MANUAL_CCT_RANGE
     *
     * \param[in] camera_range_t cct range, which specify min and max cct for 3a algorithm to use.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbCctRange(camera_range_t cct);

    /**
     * \brief Get customized cct range currently used.
     *
     * \param[out] camera_range_t& cct range
     *
     * \return 0 if cct range was set, non-0 means no cct range was set.
     */
    int getAwbCctRange(camera_range_t& cct) const;

    /**
     * \brief Set customized awb gains.
     *
     * Customized awb gains only take effect when awb mode is set to AWB_MODE_MANUAL_GAIN
     *
     * The range of each gain is (0, 255).
     *
     * \param[in] camera_awb_gains_t awb gains, which specify r,g,b gains for overriding awb result.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbGains(camera_awb_gains_t awbGains);

    /**
     * \brief Get customized awb gains currently used.
     *
     * \param[out] camera_awb_gains_t& awb gains
     *
     * \return 0 if awb gain was set, non-0 means no awb gain was set.
     */
    int getAwbGains(camera_awb_gains_t& awbGains) const;

    /**
     * \brief Set awb gain shift.
     *
     * Customized awb gain shift only take effect when awb mode is NOT set to AWB_MODE_MANUAL_GAIN
     *
     * The range of each gain shift is (0, 255).
     *
     * \param[in] camera_awb_gains_t awb gain shift, which specify r,g,b gains for updating awb result.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbGainShift(camera_awb_gains_t awbGainShift);

    /**
     * \brief Get customized awb gains shift currently used.
     *
     * \param[out] camera_awb_gains_t& awb gain shift
     *
     * \return 0 if awb gain shift was set, non-0 means no awb gain shift was set.
     */
    int getAwbGainShift(camera_awb_gains_t& awbGainShift) const;

    /**
     * \brief Set awb result.
     *
     * \param[in] data: The data used to override awb result.
     *
     * Note: data is allocated by the caller and NULL is for erasing the param.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbResult(void *data);

    /**
     * \brief Get awb result currently used.
     *
     * \param[out] data: the awb result pointer to user
     *
     * Note: data is allocated by the caller and it must never be NULL.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAwbResult(void *data) const;

    /**
     * \brief Set manual white point coordinate.
     *
     * Only take effect when awb mode is set to AWB_MODE_MANUAL_WHITE_POINT.
     * The coordinate system is based on frame which is currently displayed.
     *
     * \param[in] camera_coordinate_t white point
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbWhitePoint(camera_coordinate_t whitePoint);

    /**
     * \brief Get manual white point coordinate.
     *
     * \param[out] camera_coordinate_t& white point
     *
     * \return 0 if white point was set, non-0 means no white point was set.
     */
    int getAwbWhitePoint(camera_coordinate_t& whitePoint) const;

    /**
     * \brief Set customized color transform which is a 3x3 matrix.
     *
     *  Manual color transform only takes effect when awb mode set to AWB_MODE_MANUAL_COLOR_TRANSFORM.
     *
     * \param[in] camera_color_transform_t colorTransform: a 3x3 matrix for color convertion.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setColorTransform(camera_color_transform_t colorTransform);

    /**
     * \brief Get color transform matrix currently used.
     *
     * \param[out] camera_color_transform_t& color transform matrix
     *
     * \return 0 if color transform matrix was set, non-0 means no color transform matrix was set.
     */
    int getColorTransform(camera_color_transform_t& colorTransform) const;

    /**
     * \brief Set customized color correction gains which is a 4 array.
     *
     *  Manual color correction gains only takes effect when awb mode set to AWB_MODE_MANUAL_COLOR_TRANSFORM.
     *
     * \param[in] camera_color_gains_t colorGains: a 4 array for color correction gains.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setColorGains(camera_color_gains_t colorGains);
    /**
     * \brief Get color correction gains currently used.
     *
     * \param[out] camera_color_gains_t& color correction gains
     *
     * \return 0 if color correction gains was set, non-0 means no color correction gains was set.
     */
    int getColorGains(camera_color_gains_t& colorGains) const;

    int setAwbRegions(camera_window_list_t awbRegions);
    int getAwbRegions(camera_window_list_t& awbRegions) const;

    // Belows are convergence speed related parameters operations
    /**
     * \brief Set customized Ae converge speed.
     *
     * \param[in] camera_converge_speed_t speed: the converge speed to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeConvergeSpeed(camera_converge_speed_t speed);

    /**
     * \brief Get customized Ae converge speed.
     *
     * \param[out] camera_converge_speed_t& speed: the converge speed been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAeConvergeSpeed(camera_converge_speed_t& speed) const;

    /**
     * \brief Set customized Awb converge speed.
     *
     * \param[in] camera_converge_speed_t speed: the converge speed to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbConvergeSpeed(camera_converge_speed_t speed);

    /**
     * \brief Get customized Awb converge speed.
     *
     * \param[out] camera_converge_speed_t& speed: the converge speed been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAwbConvergeSpeed(camera_converge_speed_t& speed) const;

    /**
     * \brief Set customized Ae converge speed mode.
     *
     * \param[in] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeConvergeSpeedMode(camera_converge_speed_mode_t mode);

    /**
     * \brief Get customized Ae converge speed mode.
     *
     * \param[out] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAeConvergeSpeedMode(camera_converge_speed_mode_t& mode) const;

    /**
     * \brief Set customized Awb converge speed mode.
     *
     * \param[in] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbConvergeSpeedMode(camera_converge_speed_mode_t mode);

    /**
     * \brief Get customized Awb converge speed mode.
     *
     * \param[out] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAwbConvergeSpeedMode(camera_converge_speed_mode_t& mode) const;

    // Belows are ISP related parameters operations

    /**
     * \brief Set edge mode.
     *
     * \param[in] camera_edge_mode_t edgeMode: the edge mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setEdgeMode(camera_edge_mode_t edgeMode);

    /**
     * \brief Get edge mode.
     *
     * \param[out] camera_edge_mode_t mode: the edge mode to be get.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getEdgeMode(camera_edge_mode_t& edgeMode) const;

    int setNrMode(camera_nr_mode_t nrMode);
    int getNrMode(camera_nr_mode_t& nrMode) const;

    int setNrLevel(camera_nr_level_t level);
    int getNrLevel(camera_nr_level_t& level) const;

    /**
     * \brief Set YUV color range mode
     *
     * \param[in] camera_yuv_color_range_mode_t colorRange: the YUV color range mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setYuvColorRangeMode(camera_yuv_color_range_mode_t colorRange);

    /**
     * \brief Get YUV color range mode
     *
     * \param[out] camera_yuv_color_range_mode_t colorRange: the YUV color range mode.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getYuvColorRangeMode(camera_yuv_color_range_mode_t & colorRange) const;

    /**
     * \brief Set customized effects.
     *
     * One of sharpness, brightness, contrast, hue, saturation could be controlled by this API.
     * Valid range should be [-128, 127]
     *
     * \param[in] camera_image_enhancement_t effects
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setImageEnhancement(camera_image_enhancement_t effects);

    /**
     * \brief Get customized effects.
     *
     * \param[out] effects
     *
     * \return 0 if effects was set, non-0 return value means no effects was set.
     */
    int getImageEnhancement(camera_image_enhancement_t& effects) const;

    // Belows are other parameters operations
    int setIrisMode(camera_iris_mode_t irisMode);
    int getIrisMode(camera_iris_mode_t& irisMode);

    int setIrisLevel(int level);
    int getIrisLevel(int& level);

    /**
     * \brief Set WDR Level
     *
     * \param[in] uint8_t level
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setWdrLevel(uint8_t level);

    /**
     * \brief Get WDR level currently used.
     *
     * \param[out] uint8_t& level
     *
     * \return 0 if get WDR level, non-0 means error.
     */
    int getWdrLevel(uint8_t& level) const;

    /**
     * \brief Set effect scene mode
     *
     * \param[in] camera_scene_mode_t: scene mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setEffectSceneMode(camera_scene_mode_t sceneMode);

    /**
     * \brief Get effect scene mode based on runtime
     *
     * \param[out] camera_scene_mode_t&: scene mode
     *
     * \return 0 if get scene mode, non-0 means error.
     */
    int getEffectSceneMode(camera_scene_mode_t& sceneMode) const;

    /**
     * \brief Set scene mode
     *
     * \param[in] camera_scene_mode_t: scene mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSceneMode(camera_scene_mode_t sceneMode);

    /**
     * \brief Get scene mode current set by user.
     *
     * \param[out] camera_scene_mode_t&: scene mode
     *
     * \return 0 if get scene mode, non-0 means error.
     */
    int getSceneMode(camera_scene_mode_t& sceneMode) const;

    /**
     * \brief Set deinterlace mode
     *
     * \param[in] camera_deinterlace_mode_t deinterlaceMode
     *
     * Setting deinterlace mode only takes effect before camera_device_config_streams called
     * That's it cannot be changed after camera_device_config_streams.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setDeinterlaceMode(camera_deinterlace_mode_t deinterlaceMode);

    /**
     * \brief Get deinterlace mode
     *
     * \param[out] camera_deinterlace_mode_t& deinterlaceMode
     *
     * \return 0 if deinterlace mode was set, non-0 means no deinterlace mode was set.
     */
    int getDeinterlaceMode(camera_deinterlace_mode_t &deinterlaceMode) const;

    /**
     * \brief Set Makernote Data
     *
     * \param[in] const void* data: the pointer of data.
     * \param[in] unsigned int size: the size of the data.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setMakernoteData(const void* data, unsigned int size);

    /**
     * \brief Get Makernote Data
     *
     * \param[out] void* data: the pointer of destination buffer.
     * \param[in/out] in: the buffer size; out: the buffer used size.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getMakernoteData(void* data, unsigned int* size) const;

    /**
     * \brief Set Custom Aic Param
     *
     * \param[in] const void* data: the pointer of data.
     * \param[in] int length: the length of the data.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setCustomAicParam(const void* data, unsigned int length);

    /**
     * \brief Get Custom Aic Param
     *
     * \param[out] void* data: the pointer of destination buffer.
     * \param[in/out] in: the buffer size; out: the buffer used size.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getCustomAicParam(void* data, unsigned int* length) const;

    /**
     * \brief Set makernote mode
     *
     * \param[in] camera_makernote_mode_t mode
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int setMakernoteMode(camera_makernote_mode_t mode);

    /**
     * \brief get makernote mode
     *
     * \param[out] camera_makernote_mode_t &mode
     *
     * \return 0 if makernote mode was set, otherwise return non-0 value.
     */
    int getMakernoteMode(camera_makernote_mode_t &mode) const;

    /**
     * \brief Set digital zoom ratio
     *
     * \param[in] float ratio
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setDigitalZoomRatio(float ratio);

    /**
     * \brief Get digital zoom ratio
     *
     * \param[out] float& ratio
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getDigitalZoomRatio(float& ratio) const;

    /**
     * \brief Set lens distortion correction mode
     *
     * \param[in] camera_ldc_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setLdcMode(camera_ldc_mode_t mode);

    /**
     * \brief Get lens distortion correction mode
     *
     * \param[out] camera_ldc_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getLdcMode(camera_ldc_mode_t &mode) const;

    /**
     * \brief Set rolling shutter correction mode
     *
     * \param[in] camera_rsc_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setRscMode(camera_rsc_mode_t mode);

    /**
     * \brief Get rolling shutter correction mode
     *
     * \param[out] camera_rsc_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getRscMode(camera_rsc_mode_t &mode) const;

    /**
     * \brief flip mode
     *
     * \param[in] camera_flip_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFlipMode(camera_flip_mode_t mode);

    /**
     * \brief Get flip mode
     *
     * \param[out] camera_flip_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getFlipMode(camera_flip_mode_t &mode) const;

    /**
     * \brief set frame interval to run 3A
     *
     * \param[in] int cadence which is frame interval
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setRun3ACadence(int cadence);

    /**
     * \brief Get frame interval to run 3A
     *
     * \param[out] int& cadence which is frame interval
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getRun3ACadence(int &cadence) const;

    /**
     * \brief mono downscale mode
     *
     * \param[in] camera_mono_downscale_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setMonoDsMode(camera_mono_downscale_mode_t mode);

    /**
     * \brief Get mono downscale mode
     *
     * \param[out] camera_mono_downscale_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getMonoDsMode(camera_mono_downscale_mode_t &mode) const;

    /**
     * \brief Set Fisheye Dewarping Mode
     *
     * \param[in] camera_fisheye_dewarping_mode_t dewarpingMode
     *
     * Setting dewarping mode only takes effect before camera_device_config_streams called
     * That's it cannot be changed after camera_device_config_streams.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFisheyeDewarpingMode(camera_fisheye_dewarping_mode_t dewarpingMode);

    /**
     * \brief Get Fisheye Dewarping Mode
     *
     * \param[out] camera_fisheye_dewarping_mode_t &dewarpingMode
     *
     * \return 0 if dewarping mode was set, non-0 means no dewarping mode was set.
     */
    int getFisheyeDewarpingMode(camera_fisheye_dewarping_mode_t &dewarpingMode) const;

    // Belows are Jpeg related parameters operations
    int getJpegQuality(uint8_t *quality) const;
    int setJpegQuality(uint8_t quality);

    int getJpegThumbnailQuality(uint8_t *quality) const;
    int setJpegThumbnailQuality(uint8_t quality);

    int setJpegThumbnailSize(const camera_resolution_t& res);
    int getJpegThumbnailSize(camera_resolution_t& res) const;

    int getJpegRotation(int &rotation) const;
    int setJpegRotation(int  rotation);

    int setJpegGpsCoordinates(const double *coordinates);
    int getJpegGpsLatitude(double &latitude) const;
    int getJpegGpsLongitude(double &longitude) const;
    int getJpegGpsAltitude(double &altiude) const;

    int getJpegGpsTimeStamp(int64_t &timestamp) const;
    int setJpegGpsTimeStamp(int64_t  timestamp);

    int getJpegGpsProcessingMethod(int &processMethod) const;
    int setJpegGpsProcessingMethod(int  processMethod);

    int getJpegGpsProcessingMethod(int size, char* processMethod) const;
    int setJpegGpsProcessingMethod(const char* processMethod);

    int getImageEffect(camera_effect_mode_t &effect) const;
    int setImageEffect(camera_effect_mode_t  effect);

    int getVideoStabilizationMode(camera_video_stabilization_mode_t &mode) const;
    int setVideoStabilizationMode(camera_video_stabilization_mode_t mode);

    int getFocalLength(float &focal) const;
    int setFocalLength(float focal);

    /**
     * \brief Get aperture value currently used
     *
     * \param[in] float& aperture
     *
     * \return 0 if aperture was set, non=0 means no aperture was set
     */
    int getAperture(float &aperture) const;
    /**
     * \brief Set aperture value
     *
     * \param[in] float aperture
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAperture(float aperture);

    /**
     * \brief Get focus distance value currently used
     *
     * \param[in] float& distance
     *
     * \return 0 if distance was set, non-0 means no focus distance was set
     */
    int getFocusDistance(float &distance) const;
    /**
     * \brief Set focus distance value
     *
     * \param[in] float distance
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFocusDistance(float distance);

    /**
     * \brief Get focus range value currently used
     *
     * \param[in] camera_range_t& focusRange
     *
     * \return 0 if focus range was set, non-0 means no focus range was set
     */
    int getFocusRange(camera_range_t& focusRange) const;
    /**
     * \brief Set focus range value
     *
     * \param[in] camera_range_t focusRange
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFocusRange(const camera_range_t& focusRange);

    /**
     * \brief Set af mode
     *
     * \param[in] camera_af_mode_t afMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfMode(camera_af_mode_t afMode);

    /**
     * \brief Get af mode currently used.
     *
     * \param[out] camera_af_mode_t& afMode
     *
     * \return 0 if af mode was set, non-0 means no af mode was set.
     */
    int getAfMode(camera_af_mode_t& afMode) const;

    /**
     * \brief Trigger or cancel auto focus
     *
     * \param[in] camera_af_trigger_t afTrigger
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfTrigger(camera_af_trigger_t afTrigger);

    /**
     * \brief Get auto focus trigger value
     *
     * \param[out] camera_af_trigger_t afTrigger
     *
     * \return 0 if af trigger was set, otherwise non-0 value is returned.
     */
    int getAfTrigger(camera_af_trigger_t& afTrigger) const;

    /**
     * \brief Set AF state.
     *
     * \param[in] camera_af_state_t afState
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfState(camera_af_state_t afState);

    /**
     * \brief Get AF state
     *
     * \param[out] afState: Currently AF state will be set to afState if 0 is returned.
     *
     * \return 0 if AF state was set, otherwise non-0 value is returned.
     */
    int getAfState(camera_af_state_t& afState) const;

    /**
     * \brief Set lens state.
     *
     * \param[in] if lens is moving
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setLensState(bool lensMoving);

    /**
     * \brief Get lens state
     *
     * \param[out] isMoving: if lens is moving currently
     *
     * \return 0 if lens state was set, otherwise non-0 value is returned.
     */
    int getLensState(bool& lensMoving) const;

    /**
     * \brief Get lens aperture.
     *
     * \param[out] aperture
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensAperture(float &aperture) const;

    /**
     * \brief Get lens filter density.
     *
     * \param[out] filter density
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensFilterDensity(float &filterDensity) const;

    /**
     * \brief Get lens min focus distance.
     *
     * \param[out] min focus distance
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensMinFocusDistance(float &minFocusDistance) const;

    /**
     * \brief Get lens hyperfocal distance.
     *
     * \param[out] hyperfocal distance
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensHyperfocalDistance(float &hyperfocalDistance) const;

    /**
     * \brief Set af region
     *
     * \param[in] camera_window_list_t afRegions
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfRegions(camera_window_list_t afRegions);

    /**
     * \brief Get af region
     *
     * \param[out] camera_window_list_t afRegions
     *
     * \return 0 if afRegions were set, otherwise non-0 value is returned.
     */
    int getAfRegions(camera_window_list_t& afRegions) const;

    /**
     * \brief Get camera sensor mount type
     *
     * \param[out] sensorMountType sensor mount type: WALL_MOUNT or CEILING_MOUNT
     *
     * \return 0 if sensorMountType was set, otherwise non-0 value is returned.
     */
    int getSensorMountType(camera_mount_type_t& sensorMountType) const;

    int updateDebugLevel();

    /**
     * \brief Set camera test pattern mode
     *
     * \param[in] mode: the camera device test pattern mode.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setTestPatternMode(camera_test_pattern_mode_t mode);

    /**
     * \brief Get camera test pattern mode
     *
     * \param[out] mode: the camera device test pattern mode.
     *
     * \return 0 if test pattern mode was set, otherwise non-0 value is returned.
     */
    int getTestPatternMode(camera_test_pattern_mode_t& mode) const;

    /**
     * \brief Set crop region
     *
     * \param[in] cropRegion  the crop region related parameters
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setCropRegion(camera_crop_region_t cropRegion);

    /**
     * \brief Get crop region
     *
     * \param[out] cropRegion  the crop related parameters
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getCropRegion(camera_crop_region_t& cropRegion) const;

    /**
    * \brief Set control scene mode
    *
    * \param[in] sceneModeValue the control scene mode related parameters
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setControlSceneMode(uint8_t sceneModeValue);

    /**
    * \brief Set face detect mode
    *
    * \param[in] faceDetectMode the face detect mode related parameters
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setFaceDetectMode(uint8_t faceDetectMode);

    /**
    * \brief Get face detect mode
    *
    * \param[out] faceDetectMode the face detect mode related parameters, 0:OFF 1:SIMPLE 2:FULL
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getFaceDetectMode(uint8_t& faceDetectMode) const;

    /**
    * \brief Set face id
    *
    * \param[in] int *faceIds, int faceNum
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setFaceIds(int *faceIds, int faceNum);

    /**
     * Get sensor active array size
     *
     * \param[out] camera_coordinate_system_t& arraySize
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getSensorActiveArraySize(camera_coordinate_system_t& arraySize) const;

    /**
    * \brief Set shading  mode
    *
    * \param[in] shadingMode the shading mode related parameters
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setShadingMode(camera_shading_mode_t shadingMode);

    /**
    * \brief Get shading  mode
    *
    * \param[out] shadingMode the shading mode related parameters, 0:OFF 1:FAST 2:HIGH_QUALITY
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getShadingMode(camera_shading_mode_t& shadingMode) const;

    /**
    * \brief Set statistics lens shading map mode
    *
    * \param[in] lensShadingMapMode the lens shading map mode related parameters
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setLensShadingMapMode(camera_lens_shading_map_mode_type_t lensShadingMapMode);

    /**
    * \brief Get statistics lens shading map mode
    *
    * \param[out] lensShadingMapMode the lens shading map mode related parameters, 0:OFF 1:ON
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getLensShadingMapMode(camera_lens_shading_map_mode_type_t &lensShadingMapMode) const;

    /**
    * \brief Set lens shading map
    *
    * \param[in] lensShadingMap the lens shading map
    * \param[in] lensShadingMapSize lensShadingMap's size
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setLensShadingMap(const float *lensShadingMap, size_t lensShadingMapSize);

    /**
    * \brief Get lens shading map
    *
    * \param[out] lensShadingMap the lens shading map
    * \param[out] lensShadingMapSize the lens shading map's size
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getLensShadingMap(float **lensShadingMap, size_t &lensShadingMapSize) const;

    /**
    * \brief Get lens shading map size
    *
    * \param[out] arraySize the lens shading map size related parameters
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getLensInfoShadingMapSize(camera_coordinate_t &shadingMapSize) const;

    /*
    * \brief Set tonemap mode
    *
    * \param[in] camera_tonemap_mode_t& mode
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setTonemapMode(camera_tonemap_mode_t mode);

    /**
    * \brief Get tonemap mode
    *
    * \param[out] camera_tonemap_mode_t& mode
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getTonemapMode(camera_tonemap_mode_t& mode) const;

    /**
    * \brief Get supported tonemap modes
    *
    * \param[out] vector<camera_tonemap_mode_t>& tonemapModes
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getSupportedTonemapMode(std::vector<camera_tonemap_mode_t>& tonemapModes) const;

    /**
    * \brief Set the type of tonemap preset curve
    *
    * \param[in] camera_tonemap_preset_curve_t type
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setTonemapPresetCurve(camera_tonemap_preset_curve_t type);

    /**
    * \brief Get tonemap gamma
    *
    * \param[out] camera_tonemap_preset_curve_t& type
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getTonemapPresetCurve(camera_tonemap_preset_curve_t& type) const;

    /**
    * \brief Set tonemap gamma
    *
    * \param[in] float gamma
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setTonemapGamma(float gamma);

    /**
    * \brief Get tonemap gamma
    *
    * \param[out] float& gamma
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getTonemapGamma(float& gamma) const;

    /**
    * \brief Get number of tonemap curve points
    *
    * \param[out] int32_t& number
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getTonemapMaxCurvePoints(int32_t& number) const;

    /**
    * \brief Set tonemap curves
    *
    * \param[in] const camera_tonemap_curves_t& curve
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int setTonemapCurves(const camera_tonemap_curves_t& curves);

    /**
    * \brief Get tonemap curves
    *
    * \param[out] camera_tonemap_curves_t& curve
    *
    * \return 0 if successfully, otherwise non-0 value is returned.
    */
    int getTonemapCurves(camera_tonemap_curves_t& curves) const;

    /**
     * \brief Set user request id
     *
     * \param[in] user request id
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setUserRequestId(int32_t userRequestId);

    /**
     * \brief Get user request id
     *
     * \param[out] user request id
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getUserRequestId(int32_t& userRequestId) const;

    /**
     * \brief Set capture intent
     *
     * \param[in] capture intent mode
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setCaptureIntent(uint8_t captureIntent);

    /**
     * \brief Get capture intent
     *
     * \param[out] capture intent mode
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getCaptureIntent(uint8_t& captureIntent) const;

private:
    friend class ParameterHelper;
    void* mData; // The internal data to save the all of the parameters.
}; // class Parameters
/*******************End of Camera Parameters Definition**********************/

}  // namespace icamera
