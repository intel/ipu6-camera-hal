/*
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
 * Filename: ICamera.h
 *
 * ------------------------------------------------------------------------------
 * REVISION HISTORY
 *******************************************************************************
 *     Version        0.10       Initialize camera HAL API
 *******************************************************************************
 *     Version        0.20       Update for the device handle and stream handle
 *                               Update for the coding style of the type
 *******************************************************************************
 *     Version        0.21       Remove API camera_device_set_param and camera_device_get_param
 *                               Remove type define camera_stream_t
 *******************************************************************************
 *     Version        0.30       Remove API camera_device_del_stream
 *                               Rename API camera_device_add_stream -> camera_device_config_streams
 *                                   * All the streams are added at the same time
 *                               Rename camera_device_init -> camera_hal_init
 *                               Rename camera_device_deinit -> camera_hal_deinit
 *                               Remove the typedef for struct camera_buffer
 *******************************************************************************
 *     Version        0.31       Remove duplicated camera_frame_info_t
 *                               Cleanup the camera_buffer_t structure
 *******************************************************************************
 *     Version        0.40       Delete redundant query_frame_info
 *                               Add assist API: get_frame_size
 *******************************************************************************
 *     Version        0.41       Add API camera_device_allocate_memory to enable mmap memory mode
 *******************************************************************************
 *     Version        0.42       Merge all the common types to Parameters.h
 *******************************************************************************
  *    Version        0.43       Add sensor description in camera_info_t
 *******************************************************************************
 *     Version        0.44       Change output parameter of get_frame_size
 *******************************************************************************
 *     Version        0.45       Add two parameters(number of buffers, parameters) for camera_stream_qbuf
                                 Add one parameter(parameters) for camera_stream_dqbuf
*******************************************************************************
 *     Version        0.46       Add virtual channel camera number for camera_device_open
                                 Add virtual channel information in camera_info_t
 *******************************************************************************
 *     Version        0.50       Support specifying input format (aka ISYS output format).
 *******************************************************************************
 *     Version        0.51       Support specifying inputConfig inlucding inputformat/resolution
                                 Params int inputFmt changed to stream_t *inputConfig
 *******************************************************************************
 *     Version        0.60       Overload interface camera_stream_qbuf() with refined parameters
                                 old camera_stream_qbuf() will be removed soon
 *******************************************************************************
 *     Version        0.61       Add API camera_callback_register() to notify event to AAL
 *******************************************************************************
 *     Version        0.62       Add sequence in camera_get_parameters to fetch settings
 * ------------------------------------------------------------------------------
 *
 */

#pragma once

#include "Parameters.h"

#include <stdlib.h> // For including definition of NULL

namespace icamera {

/**
 * \struct camera_info_t: Define each camera basic information
 */
typedef struct {
    int facing;
    int orientation;
    int device_version;
    const char* name; /**< Sensor name */
    const char* description; /**< Sensor description */
    const Parameters *capability; /**< camera capability */
} camera_info_t;

/**
 * \brief
 *   Get numbers of camera
 *
 * \note
 *   This allows user to get the numbers of camera without init or open camera device.
 *
 * \return
 *   >0 the numbers of camera
 * \return
 *   <= 0 failed to get camera numbers
 *
 * \par Sample code:
 *
 * \code
 *   int num = get_number_of_cameras();
 * \endcode
 **/
int get_number_of_cameras();

/**
 * \brief
 *   Get camera info including camera capability.
 *
 * \note
 *   It can be called before hal init
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[out]
 *   camera_info_t info: Camera info filled by libcamhal
 *
 * \return
 *   0 succeed to get camera info
 * \return
 *   <0 error code, failed to get camera info
 *
 * \par Sample code
 *
 * \code
 *   int camera_id = 0;
 *   camera_info_t info;
 *   get_camera_info(camera_id, info);
 *
 * \endcode
 *
 **/
int get_camera_info(int camera_id, camera_info_t& info);

/**
 * \brief
 *   Initialize camera HAL
 *
 * \return
 *   0 succeed to init camera HAL
 * \return
 *   <0 error code, failed to init camera HAL
 *
 * \par Sample code:
 *
 * \code
 *   int ret = camera_hal_init();
 * \endcode
 **/
int camera_hal_init();

/**
 * \brief
 *   De-initialize camera HAL
 *
 * \return
 *   0 succeed to deinit camera HAL
 * \return
 *   <0 error code, failed to deinit camera device
 *
 * \par Sample code:
 *
 * \code
 *   int ret = camera_hal_deinit();
 * \endcode
 **/
int camera_hal_deinit();

/**
 * \brief
 *   Register callback function
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[in]
 *   camera_callback_ops_t *callback: callback handle
 *
 **/
void camera_callback_register(int camera_id, const camera_callback_ops_t *callback);

/**
 * \brief
 *   Open camera device by camera ID
 *
 * \param[in]
 *   int camera_id: ID of the camera
 *
 * \return
 *   0 succeed to open camera device
 * \return
 *   <0 error code, failed to open camera device
 *
 * \par Sample code:
 *
 * \code
 *   int camera_id = 0;
 *   int ret = camera_device_open(camera_id);
 * \endcode
 **/
int camera_device_open(int camera_id);

/**
 * \brief
 *   Close camera device by camera ID
 *
 * \param[in]
 *   int camera_id: ID of the camera
 *
 * \par Sample code:
 *
 * \code
 *   int camera_id = 0;
 *   int ret = camera_device_open(camera_id);
 *   camera_device_close(camera_id);
 * \endcode
 **/
void camera_device_close(int camera_id);

/**
 * \brief
 *   Configure sensor input of camera device, it is not allowed to call this when camera is started.
 *   Optional call.
 *
 * \note
 *   1. To re-configure sensor input, camera device must be stopped first.
 *   2. The new sensor configuration will overwrite the previous config.
 *   3. The new "inputConfig" will be used for all the future operation until the device is closed.
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[in]
 *   int inputConfig: Specify which input format, resolution(the output of ISYS) should be used.
 *
 * \return
 *   0 succeed to configure streams
 * \return
 *   <0 error code, failed to configure stream
 *
 * \par Sample code:
 *
 * \code
 *   int camera_id = 0;
 *   stream_t input_config;
 *   CLEAR(input_config);
 *   input_config.format = V4L2_PIX_FMT_SGRBG8V32;
 *   ret = camera_device_config_sensor_input(camera_id, &input_config);
 * \endcode
 **/
int camera_device_config_sensor_input(int camera_id, const stream_t *inputConfig);

/**
 * \brief
 *   Configure streams to camera device, it is not allowed to call this when camera is started.
 *
 * \note
 *   1. To re-configure streams, camera device must be stopped first.
 *   2. The new streams configuration will overwrite the previous streams.
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[in]
 *   stream_config_t stream_list: stream configuration list, if success, stream id is filled in streams[]
 *
 * \return
 *   0 succeed to configure streams
 * \return
 *   <0 error code, failed to configure stream
 *
 * \par Sample code:
 *
 * \code
 *   int camera_id = 0;
 *   stream_config_t stream_list;
 *   stream_t streams[1];
 *   streams[0].format = V4L2_PIX_FMT_SGRBG8;
 *   streams[0].width = 1920;
 *   streams[0].height = 1080;
 *   streams[0].memType = V4L2_MEMORY_USERPTR;
 *   stream_list.num_streams = 1;
 *   stream_list.streams = streams;
 *   ret = camera_device_config_streams(camera_id, &stream_list);
 * \endcode
 **/
int camera_device_config_streams(int camera_id, stream_config_t *stream_list);

/**
 * \brief
 *   Start camera device
 *
 * \param[in]
 *   int camera_id: ID of the camera
 *
 * \return
 *   0 succeed to start device
 * \return
 *   <0 error code, failed to start device
 *
 * \par Sample code:
 *
 * \code
 *   int camera_id=0;
 *   stream_config_t stream_list;
 *   ...
 *   ret = camera_device_config_streams(camera_id, &stream_list);
 *   ret = camera_device_start(camera_id);
 *   ... ...
 *   ret = camera_device_stop(camera_id);
 * \endcode
 *
 **/
int camera_device_start(int camera_id);

/**
 * \brief
 *   Stop camera device.
 *
 * \param[in]
 *   int camera_id: ID of the camera
 *
 * \return
 *   0 succeed to stop device
 * \return
 *   <0 error code, failed to stop device
 *
 * \see camera_device_start()
 **/
int camera_device_stop(int camera_id);

/**
 * \brief
 *   Allocate memory for mmap & dma export io-mode
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[out]
 *   camera_buffer_t buffer: in Mmap mode, mmaped address is filled in addr;
 *   in DMA export mode, the dma fd is flled in dmafd.
 *
 * \return
 *   0 succeed to allocate memory
 * \return
 *   <0 error code, failed to allocate memory
 *
 * \par Sample code:
 *
 * \code
 *   camera_buffer_t *buffers = (camera_buffer_t *)malloc(sizeof(camera_buffer_t)*buffer_count);
 *   camera_buffer_t *buf = buffers;
 *   buf.s = stream;
 *   for (int i = 0; i < buffer_count; i++, buf++) {
 *     camera_device_allocate_memory(camera_id, buf):
 *   }
 *
 *   buf = buffers;
 *   for (int i = 0; i < buffer_count; i++, buf++) {
 *       camera_stream_qbuf(camera_id, stream_id, buf);
 *   }
 *
 *   camera_device_start(camera_id);
 * \endcode
 *
 */
int camera_device_allocate_memory(int camera_id, camera_buffer_t *buffer);

/**
 * \brief
 *   Queue a buffer to device (deprecated, will be removed soon.)
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[in]
 *   int stream_id: ID of stream
 * \param[in]
 *   camera_buffer_t buffer: buffer queued to device
 * \param[in]
 *   int num_buffers: indicates how many buffers need to be queued at the same time,
                      and these buffers MUST be for different streams.
                      And stream id in buffer MUST be filled correctly.
 * \param[in]
 *   Parameters settings: Settings used for this group of buffers.
 *
 * \return
 *   0 succeed to queue buffers
 * \return
 *   <0 error code, failed to queue buffers
 *
 * \see camera_stream_dqbuf();
 **/
int camera_stream_qbuf(int camera_id, int stream_id, camera_buffer_t *buffer,
                       int num_buffers = 1, const Parameters* settings = NULL);

/**
 * \brief
 *   Queue one or serveral buffers to the camera device
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[in]
 *   camera_buffer_t buffer: array of pointers to camera_buffer_t
 *   buffer[i]->s MUST be filled before calling this API.
 * \param[in]
 *   int num_buffers: indicates how many buffers are in the buffer pointer array,
 *                    and these buffers MUST be for different streams. Stream id is
 *                    filled and give back to app when camera_device_config_streams()
 *                    is called, HAL will do the mapping when parsing queued buffers
 *                    according to num_buffers.
 * \param[in]
 *   Parameters settings: Settings used for this group of buffers.
 *                        This is used for per-frame setting, which means the settings should be
 *                        applied for the group of buffers.
 *
 * \return
 *   0 succeed to queue buffers
 * \return
 *   <0 error code, failed to queue buffers
 *
 * \see camera_stream_qbuf();
 **/
int camera_stream_qbuf(int camera_id, camera_buffer_t **buffer,
                       int num_buffers = 1, const Parameters* settings = NULL);

/**
 * \brief
 *   Dequeue a buffer from device per stream id.
 *
 * \note
 *   It's a block function, that means the caller will be blocked until buffer is ready.
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[in]
 *   int stream_id: ID of stream
 * \param[out]
 *   camera_buffer_t buffer: buffer dqueued from device
 * \param[out]
 *   Parameters settings: Settings used for this buffer.
 *
 * \return
 *   0 succeed to dqueue buffer
 * \return
 *   <0 error code, failed to dqueue buffer
 *
 * \par Sample code
 *
 * \code
 *   const int buffer_count = 8;
 *   int bpp = 0;
 *   int buffer_size = get_frame_size(camera_id, V4L2_PIX_FMT_SGRBG8, 1920, 1080, V4L2_FIELD_ANY, &bpp);
 *   camera_buffer_t buffers[buffer_count];
 *   camera_buffer_t *buf = nullptr;
 *   for (int i = 0; i < buffer_count; i++) {
 *     buf = &buffers[i];
 *     posix_memalign(&buf->addr, getpagesize(), buffer_size);
 *     buf->s = stream; // stream here comes from parameter and result of camera_device_config_streams.
 *   }
 *
 *   for (int i = 0; i < buffer_count; i++) {
 *       buf = &buffers[i];
 *       camera_stream_qbuf(camera_id, stream_id, &buf);
 *   }
 *
 *   camera_device_start(camera_id);
 *
 *   for (int i = 0; i < buffer_count; i++) {
 *       camera_stream_dqbuf(camera_id, stream_id, &buf);
 *       // processing data with buf
 *   }
 * \endcode
 *
 **/
int camera_stream_dqbuf(int camera_id, int stream_id, camera_buffer_t **buffer,
                        Parameters* settings = NULL);

/**
 * \brief
 *   Set a set of parameters to the gaven camera device.
 *
 * \note
 *   It MUST be called after device opened, otherwise error will be returned.
 *   Which buffer the paramters takes effect in is not guaranteed.
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[in]
 *   Parameters param: A set of parameters.
 *
 * \return
 *   0 succeed to set camera parameters
 * \return
 *   <0 error code, failed to set camera parameters
 *
 * \par Sample code
 *
 * \code
 *   Parameters param;
 *    camera_ae_mode_t aeMode = AE_MODE_MANUAL;
 *    int64_t expTime = 10 * 1000;
 *    param.setAeMode(aeMode);
 *    param.setExposureTime(expTime);
 *    param.setXXX(); // Set other parameters...
 *
 *   int ret = camera_set_parameters(camera_id, param);
 *
 * \endcode
 *
 **/
int camera_set_parameters(int camera_id, const Parameters& param);

/**
 * \brief
 *   Get parameter from the gaven camera device.
 *
 * \note
 *   It MUST be called after device opened, otherwise error will be returned.
 *
 * \param[in]
 *   int camera_id: ID of the camera
 * \param[out]
 *   Parameters param:  parameters need to be filled in
 * \param[in]
 *   long sequence: sequence used to find target parameter and results, default is -1
 *
 * \return
 *   0 succeed to get camera parameters
 * \return
 *   <0 error code, failed to get camera parameters
 *
 * \par Sample code
 *
 * \code
 *   Parameters param;
 *   int ret = camera_get_parameters(camera_id, param);
 *   camera_ae_mode_t aeMode = AE_MODE_MANUAL;
 *   ret = param.getAeMode(aeMode);
 *
 * \endcode
 *
 **/
int camera_get_parameters(int camera_id, Parameters& param, long sequence = -1);

/**************************************Optional API ******************************
 * The API defined in this section is optional.
 */

/**
 * \brief
 *   Return the size information of a frame.
 *
 * \note
 *   It is used to assist the test cases to double confirm the final buffer size
 *
 * \param[in]
 *   int camera_id: The camera device index
 * \param[in]
 *   int format: The v4l2 format of the frame
 * \param[in]
 *   int width: The width of the frame
 * \param[in]
 *   int height: The height of the frame
 * \param[in]
 *   int field: The interlace field of the frame
 * \param[out]
 *   int bpp: The bpp of the format
 *
 * \return
 *   frame size.
 **/
int get_frame_size(int camera_id, int format, int width, int height, int field, int *bpp);

} // namespace icamera
