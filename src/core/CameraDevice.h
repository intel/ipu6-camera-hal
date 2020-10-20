/*
 * Copyright (C) 2016-2020 Intel Corporation.
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

#include "CameraStream.h"
#include "RequestThread.h"
#include "StreamSource.h"
#include "AiqUnit.h"
#include "Parameters.h"
#include "ParameterGenerator.h"
#include "SensorHwCtrl.h"
#include "SofSource.h"
#include "LensHw.h"

#include "ProcessorManager.h"

#include "gc/IGraphConfigManager.h"

namespace icamera {

class RequestThread;

/**
  * CameraDevice : Create elements to construct the streaming pipeline
  * Each element must be a producer or a consumer or Both.
  *
  * These are the typical pipelines:
  *
  * For a single SOC YUV capture in by pass mode
  * StreamSource -> CameraStream
  *
  * For a single NV12 capture of TPG/Sensor using SwImageProcess
  * StreamSource -> SwImageProcessor -> CameraStream
  *
  * For a single NV12 capture of TPG/Sensor using PSYS processor
  * StreamSource -> PsysProcessor -> CameraStream
  *
  * For a dual streams NV12 video capture of TPG/Sensor using PSYS processor
  * StreamSource -> PsysProcessor | -> CameraStream (For video recording)
  *                              | -> CameraStream (for Preview)
  *
  * For a SDV(Snapshot during video) capture of TPG/Sensor uinsg PSYS processor
  * StreamSource | -> PsysProcessor | -> CameraStream (For video recording)
  *             |                  | -> CameraStream (for Preview)
  *             | -> PsysProcessor | -> CameraStream (For still YUV)
  *
  * The buffer notification between the Class is based on Interface defined
  * in BufferQueue. The upstream element use "onFrameAvailable" message to notfiy
  * downstream elements that the buffers are ready for further processing
  *
  * Following singleton instances are created and maintained by CameraDevice
  * 1. IGraphConfigManager
  * 2. AiqResultStorage
  */
class CameraDevice : public EventListener {

public:
    CameraDevice(int cameraId);
    ~CameraDevice();

    /**
     * \brief Camera device class init
     *
     * 1.Related classes init: StreamSource, SofSource, 3AControl, lensCtrl
     * 2.Register listener if enable AIQ
     * 3.Set the defualt parameters
     *
     * \return OK if succeed, other value indicates failed
     */
    int init();

    /**
     * \brief Camera device class deinit
     *
     * 1.Change the state
     * 2.Destory the listeners
     * 3.Delete the streams
     * 4.Deinit the Related classes.
     */
    void deinit();

    /**
     * \brief Camera device start
     *
     * 1. Start all streams
     * 2. Related classes start
     *
     * \return OK if succeed, other value indicates failed
     */
    int start();

    /**
     * \brief Camera device stop
     *
     * 1. Stop all related class module.
     * 2. Change the state
     */
    int stop();

    /**
     * \brief Allocate momery according to user buffer
     *
     * 1. Convert user buffer to CameraBuffer and push it into UserBufferQueue
     * 2. Calling CameraStream class to allocateMemory.
     *
     * \return OK if succeed, other value indicates failed
     */
    int allocateMemory(camera_buffer_t *ubuffer);

    /**
     * \brief dequeue buffer from cameraStream.
     *
     * 1. Dequeue one CameraBuffer from CameraStream
     * 2. Fill the user buffer base on CameraBuffer
     * 3. Fill the settings was used for this buffer if settings is not nullptr
     *
     * \return 0 if succeed, other value indicates failed
     */
    int dqbuf(int streamId, camera_buffer_t **ubuffer, Parameters* settings = nullptr);

    /**
     * \brief Queue one buffer to CameraStream
     *
     * 1. Get the CameraBuffer base on the ubuffer
     * 2. Calling CameraStream to queue one CameraBuffer
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int qbuf(camera_buffer_t **ubuffer, int bufferNum = 1, const Parameters* settings = nullptr);

    /**
     * \brief Configure the device sensor input
     *
     *
     * \param inputConfig: the Output format/resolution of the isys
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int configureInput(const stream_t *inputConfig);

    /**
     * \brief Configure the streams, devices and post processor.
     *
     * Configure the streams according to the streamList info
     * Extra processor is needed if the format isn't supported in Psys output
     *
     * \param streamList: all the streams info
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int configure(stream_config_t *streamList);

    /**
     * \brief Set the parameters
     *
     * Merge the param to internal parameters and set them to 3A
     * and processor.
     *
     * \param param: the parameters need to set
     *
     * \return OK if succeed, other value indicates failed
     */
    int setParameters(const Parameters& param);

    /**
     * \brief Get the parameters
     *
     * Get the internal parameters list
     *
     * \return OK if succeed, other value indicates failed
     */
    int getParameters(Parameters& param, long sequence);

    void handleEvent(EventData eventData);

    void callbackRegister(const camera_callback_ops_t* callback);
private:
    DISALLOW_COPY_AND_ASSIGN(CameraDevice);

    /**
     * \brief Reconfigure the streams, devices and post processor.
     *
     * \param streamList: all the streams info
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int reconfigure(stream_config_t *streamList);

    int startLocked();
    int stopLocked();
    int initDefaultParameters();
    std::shared_ptr<CameraBuffer> userBufferToCameraBuffer(Port port,
                                                                camera_buffer_t *ubuffer);

    StreamSource* createBufferProducer();
    std::map<Port, stream_t> selectProducerConfig(const stream_config_t *streamList, int mcId);
    bool isProcessorNeeded(const stream_config_t *streamList, const stream_t &producerConfig);
    bool isStillDuringVideo(const stream_config_t *streamList);
    int analyzeStream(stream_config_t *streamList);
    int createStreams(stream_config_t *streamList);
    int bindStreams(stream_config_t *streamList);
    void deleteStreams();

    /**
     * Bind all event listeners with their source.
     */
    void bindListeners();

    /**
     * Unbind all event listeners from their source.
     */
    void unbindListeners();

    long fetchCaptureSettings(const Parameters * params);

    // The second phase of qbuf(), done in RequestThread
    int handleQueueBuffer(int bufferNum, camera_buffer_t **ubuffer, long sequence);

    // No lock protect for internal usage, should be protected by mDeviceLock from outside
    int configureL(stream_config_t *streamList, bool clean = true);
    int setParametersL(const Parameters& param);

    int registerBuffer(camera_buffer_t **ubuffer, int bufferNum);

private:
    enum {
        DEVICE_UNINIT = 0,
        DEVICE_INIT,
        DEVICE_CONFIGURE, //means stream configured
        DEVICE_START,
        DEVICE_STOP,
        DEVICE_BUFFER_READY, //At least one buffer is queued to ISP
    } mState;

    // Guard for CameraDevice public API
    Mutex mDeviceLock;

    static const nsecs_t kWaitDuration = 5000000000; //5000ms

    // Pipeline elements
    CameraStream* mStreams[MAX_STREAM_NUMBER];
    std::map<int, Port> mStreamIdToPortMap;
    std::vector<int> mSortedStreamIds; // Used to save sorted stream ids with descending order.
    StreamSource *mProducer;

    ProcessorManager* mProcessorManager;
    std::vector<BufferQueue*> mProcessors;

    ParameterGenerator *mParamGenerator;

    LensHw *mLensCtrl;
    SensorHwCtrl *mSensorCtrl;
    SofSource* mSofSource;
    AiqUnitBase *m3AControl;

    //Internal used variable
    int     mCameraId;
    int     mStreamNum;
    Parameters mParameter;
    //The latest result parameters
    Parameters mResultParameter;
    bool mPerframeControlSupport;

    RequestThread* mRequestThread;
    IGraphConfigManager *mGCM;
    stream_t mInputConfig;
    camera_callback_ops_t *mCallback;
};

} //namespace icamera
