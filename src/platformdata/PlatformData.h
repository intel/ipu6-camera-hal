/*
 * Copyright (C) 2015-2024 Intel Corporation.
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

#include <limits.h>

#ifdef CAL_BUILD
#include <cros-camera/v4l2_device.h>
#else
#include <v4l2_device.h>
#endif

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "AiqInitData.h"
#include "CameraTypes.h"
#include "FaceType.h"
#include "ICamera.h"
#include "ia_types.h"
#include "IGraphConfig.h"
#include "MediaControl.h"
#include "Parameters.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

#define RESOLUTION_1_3MP_WIDTH 1280
#define RESOLUTION_1_3MP_HEIGHT 960
#define RESOLUTION_1080P_WIDTH 1920
#define RESOLUTION_1080P_HEIGHT 1080
#define RESOLUTION_720P_WIDTH 1280
#define RESOLUTION_720P_HEIGHT 720
#define RESOLUTION_VGA_WIDTH 640
#define RESOLUTION_VGA_HEIGHT 480

#define MAX_BUFFER_COUNT (10)
// VIRTUAL_CHANNEL_S
#define MAX_VC_GROUP_NUMBER 8
// VIRTUAL_CHANNEL_E
#define MAX_STREAM_NUMBER 5
#define MAX_WEIGHT_GRID_SIDE_LEN 1024

#define FACE_ENGINE_DEFAULT_RUNNING_INTERVAL 1

#define FACE_ENGINE_INTEL_PVL 0
#define FACE_ENGINE_GOOGLE_FACESSD 1

/* Max number of the RAW buffer number is 32.
 * Max number size of the pipeline depth is 6.
 * Max setting count should be larger than raw buffer number + pipeline depth.
 */
#define MAX_SETTING_COUNT 40

#ifdef CAL_BUILD
#define MAX_CAMERA_NUMBER 2
#define CAMERA_CACHE_DIR "/var/cache/camera/"
#define CAMERA_DEFAULT_CFG_PATH "/etc/camera/"
#define CAMERA_GRAPH_DESCRIPTOR_FILE "gcss/graph_descriptor.xml"
#define CAMERA_GRAPH_SETTINGS_DIR "gcss/"
#endif

#ifdef __ANDROID__
#define MAX_CAMERA_NUMBER 2
#define CAMERA_CACHE_DIR "./"
#define CAMERA_DEFAULT_CFG_PATH "/vendor/etc/"
#define CAMERA_GRAPH_DESCRIPTOR_FILE "graph_descriptor.xml"
#define CAMERA_GRAPH_SETTINGS_DIR ""
#endif

#ifdef LINUX_BUILD
#define MAX_CAMERA_NUMBER 100
// Temporarily using current path to save aiqd file for none CAL platforms.
#define CAMERA_CACHE_DIR "./"
#define CAMERA_DEFAULT_CFG_PATH "/etc/camera/"
#define CAMERA_GRAPH_DESCRIPTOR_FILE "gcss/graph_descriptor.xml"
#define CAMERA_GRAPH_SETTINGS_DIR "gcss/"
#endif

#define NVM_DATA_PATH "/sys/bus/i2c/devices/"

#define TNR7US_RESTART_THRESHOLD 5

class GraphConfigNodes;
class PlatformData {
 private:
    // Prevent to create multiple instances
    PlatformData();
    ~PlatformData();

 public:
    class StaticCfg {
     public:
        StaticCfg() { mCameras.clear(); }
        ~StaticCfg() {}  // not release resource by design

        /**
         * Camera feature info that is specific to camera id
         */
        class CameraInfo {
         public:
            CameraInfo()
                    : sensorName(""),
                      sensorDescription("unset"),
                      mLensName(""),
                      // VIRTUAL_CHANNEL_S
                      mVirtualChannel(false),
                      mVCNum(0),
                      mVCSeq(0),
                      mVCGroupId(-1),
                      // VIRTUAL_CHANNEL_E
                      mLensHwType(LENS_NONE_HW),
                      mSensorMode(SENSOR_MODE_UNKNOWN),
                      mEnablePdaf(false),
                      mSensorAwb(false),
                      mSensorAe(false),
                      mRunIspAlways(false),
                      // HDR_FEATURE_S
                      mHdrStatsInputBitDepth(0),
                      mHdrStatsOutputBitDepth(0),
                      mUseFixedHdrExposureInfo(true),
                      // HDR_FEATURE_E
                      mLtmEnabled(false),
                      mSensorExposureNum(2),
                      mSensorExposureType(SENSOR_EXPOSURE_SINGLE),
                      mSensorGainType(SENSOR_GAIN_NONE),
                      mLensCloseCode(0),
                      mEnableAIQ(false),
                      mAiqRunningInterval(1),
                      mStatsRunningRate(false),
                      mEnableMkn(true),
                      mIspTuningUpdate(false),
                      mSkipFrameV4L2Error(false),
                      mCITMaxMargin(0),
                      mYuvColorRangeMode(CAMERA_FULL_MODE_YUV_COLOR_RANGE),
                      mInitialSkipFrame(0),
                      mInitialPendingFrame(0),
                      mMaxRawDataNum(MAX_BUFFER_COUNT),
                      mTopBottomReverse(false),
                      mPsysContinueStats(false),
                      mMaxRequestsInflight(0),
                      mPreferredBufQSize(MAX_BUFFER_COUNT),
                      mDigitalGainLag(-1),
                      mExposureLag(MAX_BUFFER_COUNT),
                      mAnalogGainLag(0),
                      mLtmGainLag(0),
                      mEnableLtmThread(false),
                      mEnableLtmDefog(false),
                      mMaxSensorDigitalGain(0),
                      mSensorDgType(SENSOR_DG_TYPE_NONE),
                      mISysFourcc(V4L2_PIX_FMT_SGRBG8),
                      mISysRawFormat(V4L2_PIX_FMT_SGRBG10),
                      mUseCrlModule(true),
                      mOrientation(ORIENTATION_0),
                      mSensorOrientation(ORIENTATION_0),
                      mUseSensorDigitalGain(false),
                      mUseIspDigitalGain(false),
                      mNeedPreRegisterBuffers(false),
                      mMediaFormat(media_format_legacy),
                      // FRAME_SYNC_S
                      mFrameSyncCheckEnabled(false),
                      // FRAME_SYNC_E
                      mEnableAiqd(false),
                      mCurrentMcConf(nullptr),
                      mGraphSettingsType(COUPLED),
                      mDVSType(MORPH_TABLE),
                      mISYSCompression(false),
                      mPSACompression(false),
                      mOFSCompression(false),
                      mSchedulerEnabled(false),
                      mFaceAeEnabled(true),
                      mFaceEngineVendor(FACE_ENGINE_INTEL_PVL),
                      mFaceEngineRunningInterval(FACE_ENGINE_DEFAULT_RUNNING_INTERVAL),
                      mFaceEngineRunningIntervalNoFace(FACE_ENGINE_DEFAULT_RUNNING_INTERVAL),
                      mFaceEngineRunningSync(false),
                      mFaceEngineByIPU(false),
                      mMaxFaceDetectionNumber(MAX_FACES_DETECTABLE),
                      mPsysAlignWithSof(false),
                      mPsysBundleWithAic(false),
                      mSwProcessingAlignWithIsp(false),
                      mMaxNvmDataSize(0),
                      mNvmOverwrittenFileSize(0),
                      mTnrExtraFrameNum(0),
                      mDummyStillSink(false),
                      mGpuTnrEnabled(false),
                      mRemoveCacheFlushOutputBuffer(false),
                      mPLCEnable(false),
                      // PRIVACY_MODE_S
                      mSupportPrivacy(NO_PRIVACY_MODE),
                      mPrivacyModeThreshold(10),
                      mPrivacyModeFrameDelay(5),
                      // PRIVACY_MODE_E
                      mStillOnlyPipe(false),
                      mDisableBLCByAGain(false),
                      mDisableBLCAGainLow(-1),
                      mDisableBLCAGainHigh(-1),
                      mResetLinkRoute(true),
                      mReqWaitTimeout(0) {}

            std::vector<MediaCtlConf> mMediaCtlConfs;

            std::string sensorName;
            std::string sensorDescription;
            std::string mLensName;
            // VIRTUAL_CHANNEL_S
            bool mVirtualChannel;
            int mVCNum;
            int mVCSeq;
            int mVCGroupId;
            VcAggregator mVcAggregator;
            // VIRTUAL_CHANNEL_E
            int mLensHwType;
            std::map<TuningMode, SensitivityRange> mTuningModeToSensitivityMap;
            SensorMode mSensorMode;
            bool mEnablePdaf;
            bool mSensorAwb;
            bool mSensorAe;
            bool mRunIspAlways;
            // HDR_FEATURE_S
            int mHdrStatsInputBitDepth;
            int mHdrStatsOutputBitDepth;
            bool mUseFixedHdrExposureInfo;
            // HDR_FEATURE_E
            bool mLtmEnabled;
            int mSensorExposureNum;
            int mSensorExposureType;
            int mSensorGainType;
            int mLensCloseCode;
            bool mEnableAIQ;
            int mAiqRunningInterval;
            bool mStatsRunningRate;
            bool mEnableMkn;
            bool mIspTuningUpdate;
            // first: one algo type in imaging_algorithm_t, second: running rate
            std::unordered_map<int, float> mAlgoRunningRateMap;
            // DOL_FEATURE_S
            std::vector<int> mDolVbpOffset;
            // DOL_FEATURE_E
            bool mSkipFrameV4L2Error;
            int mCITMaxMargin;
            camera_yuv_color_range_mode_t mYuvColorRangeMode;
            unsigned int mInitialSkipFrame;
            unsigned int mInitialPendingFrame;
            unsigned int mMaxRawDataNum;
            bool mTopBottomReverse;
            bool mPsysContinueStats;
            int mMaxRequestsInflight;
            unsigned int mPreferredBufQSize;
            int mDigitalGainLag;
            int mExposureLag;
            int mAnalogGainLag;
            int mLtmGainLag;
            bool mEnableLtmThread;
            bool mEnableLtmDefog;
            int mMaxSensorDigitalGain;
            SensorDgType mSensorDgType;
            std::string mCustomAicLibraryName;
            std::string mCustom3ALibraryName;
            std::vector<camera_resolution_t> mSupportedISysSizes;  // ascending order request
            std::vector<int> mSupportedISysFormat;
            int mISysFourcc;     // the isys output format
            int mISysRawFormat;  // the isys raw format if scale enabled

            std::vector<int> mPSysFormat;  // the psys output format
            std::vector<TuningConfig> mSupportedTuningConfig;
            std::vector<LardTagConfig> mLardTagsConfig;
            std::vector<ConfigMode> mConfigModesForAuto;

            std::vector<std::string> mDisableHDRnetBoards;
            bool mUseCrlModule;
            int mOrientation;
            int mSensorOrientation;
            bool mUseSensorDigitalGain;
            bool mUseIspDigitalGain;
            bool mNeedPreRegisterBuffers;
            ia_media_format mMediaFormat;
            // FRAME_SYNC_S
            bool mFrameSyncCheckEnabled;
            // FRAME_SYNC_E
            bool mEnableAiqd;
            MediaCtlConf* mCurrentMcConf;
            std::map<int, stream_array_t> mStreamToMcMap;
            Parameters mCapability;

            /* key: TuningMode,
               value: map (key: total gain,
                           value: map (key: hdr ratio,
                                       value: edge and noise settings)) */
            std::map<TuningMode,
                     std::map<float, std::map<float, EdgeNrSetting>>> mTotalGainHdrRatioToEdgeNrMap;
            std::string mGraphSettingsFile;
            GraphSettingType mGraphSettingsType;
            std::vector<MultiExpRange> mMultiExpRanges;
            std::vector<uint32_t> mSupportedIspControlFeatures;
            int mDVSType;
            bool mISYSCompression;
            bool mPSACompression;
            bool mOFSCompression;
            bool mSchedulerEnabled;
            bool mFaceAeEnabled;
            int mFaceEngineVendor;
            int mFaceEngineRunningInterval;
            int mFaceEngineRunningIntervalNoFace;
            int mFaceEngineRunningSync;
            bool mFaceEngineByIPU;
            unsigned int mMaxFaceDetectionNumber;
            bool mPsysAlignWithSof;
            bool mPsysBundleWithAic;
            bool mSwProcessingAlignWithIsp;

            /* key: camera_test_pattern_mode_t, value: sensor test pattern mode */
            std::unordered_map<int32_t, int32_t> mTestPatternMap;

            // This is for binding stream id to ConfigMode, since the stream id from kernel list of
            // a PG might be incorrect. To be removed after stream id mismatch issue fixed.
            std::map<int, int> mConfigModeToStreamId;
            std::vector<UserToPslOutputMap> mOutputMap;
            std::vector<camera_resolution_t> mPreferOutput;
            int mMaxNvmDataSize;
            std::string mNvmDirectory;
            int mNvmOverwrittenFileSize;
            std::string mNvmOverwrittenFile;  // overwrite NVM data
            std::string mCamModuleName;
            std::string mModuleId;
            std::string mSensorId;
            std::vector<std::string> mSupportModuleNames;
            /* key: camera module name, value: camera module info */
            std::unordered_map<std::string, CameraMetadata> mCameraModuleInfoMap;
            std::vector<IGraphType::ScalerInfo> mScalerInfo;
            int mTnrExtraFrameNum;
            bool mDummyStillSink;
            std::vector<camera_resolution_t> mTnrThresholdSizes;
            bool mGpuTnrEnabled;
            bool mRemoveCacheFlushOutputBuffer;
            bool mPLCEnable;
            // PRIVACY_MODE_S
            PrivacyModeType mSupportPrivacy;
            uint32_t mPrivacyModeThreshold;
            uint32_t mPrivacyModeFrameDelay;
            // PRIVACY_MODE_E
            bool mStillOnlyPipe;

            bool mDisableBLCByAGain;
            int mDisableBLCAGainLow;
            int mDisableBLCAGainHigh;
            bool mResetLinkRoute;
            /* mReqWaitTimeout is used to override dqbuf timeout (ns) */
            int64_t mReqWaitTimeout;
        };

        /**
         * Overwrite staticCfg with info from CMC
         */
        void getModuleInfoFromCmc(int cameraId);

        std::vector<CameraInfo> mCameras;
        std::vector<PolicyConfig> mPolicyConfig;
        CommonConfig mCommonConfig;
        std::string mBoardName;
    };

 private:
    StaticCfg mStaticCfg;

    std::vector<AiqInitData*> mAiqInitData;

 private:
    /**
     * Get access to the platform singleton.
     *
     * Note: this is implemented in PlatformFactory.cpp
     */
    static PlatformData* sInstance;
    static Mutex sLock;
    static PlatformData* getInstance();

    /**
     * Release GraphConfigNodes in StaticCfg::CameraInfo
     */
    void releaseGraphConfigNodes();

    /**
     * Get MediaCtlConf via MC ID.
     */
    static MediaCtlConf* getMcConfByMcId(const StaticCfg::CameraInfo& cameraInfo, int mcId);

    /**
     * Get MediaCtlConf via stream config.
     */
    static MediaCtlConf* getMcConfByStream(const StaticCfg::CameraInfo& cameraInfo,
                                           const stream_t& stream);

    /**
     * Get MediaCtlConf via ConfigMode.
     */
    static MediaCtlConf* getMcConfByConfigMode(const StaticCfg::CameraInfo& cameraInfo,
                                               const stream_t& stream, ConfigMode mode);

    /**
     * Check if video node is enabled via camera Id and video node type.
     */
    static bool isVideoNodeEnabled(int cameraId, VideoNodeType type);

 public:
    /**
     * releaseInstance
     * This function must be called when the hal is destroyed.
     */
    static void releaseInstance();

    /**
     * init PlatformData
     *
     * \return OK if init PlatformData successfully, otherwise return ERROR.
     */
    static int init();

    /**
     * Parse graph descriptor and settings from configuration files.
     */
    static void parseGraphFromXmlFile();

    /**
     * Query GraphSettings
     */
    static int queryGraphSettings(int cameraId, const stream_config_t* streamList);

    /**
     * get the camera numbers
     *
     * \return int: the camera numbers
     */
    static int numberOfCameras();

    /**
     * get the camera number in xml
     *
     * \return int: the camera numbers in xml
     */
    static int getXmlCameraNumber();

    /**
     * get module info of sensor
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param moduleId: the module id of sensor
     * \param sensorId: the sensor id of sensor
     *
     * \return int: return OK if has module info.
     */
    static int getModuleInfo(int cameraId, std::string& moduleId, std::string& sensorId);

    /**
     * get the sensor name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return char*: the sensor name string.
     */
    static const char* getSensorName(int cameraId);

    /**
     * set board name
     *
     * \param[in] boardName
     */
    static void setBoardName(const std::string& boardName);

    /**
     * get status if HDRnet tuning used
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[out] boardConfig: set true if configured
     * \return true if HDRnet tuning used, otherwise return false.
     */
    static bool isHDRnetTuningUsed(int cameraId, bool& boardConfig);

    /**
     * get the sensor description
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return const char*: the sensor descrition string.
     */
    static const char* getSensorDescription(int cameraId);

    /**
     * get the Lens name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return char*: the lens name string.
     */
    static const char* getLensName(int cameraId);

    /**
     * get the Lens HW type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the Lens HW type
     */
    static int getLensHwType(int cameraId);

    /**
     * get sensor mode
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return SensorMode.
     */
    static SensorMode getSensorMode(int cameraId);

    /**
     * set sensor mode
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] sensorMode: SensorMode
     */
    static void setSensorMode(int cameraId, SensorMode sensorMode);

    /**
     * check if binning mode supported
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if binning mode supported, otherwise return false.
     */
    static bool isBinningModeSupport(int cameraId);

    /**
     * get sensitivity range by TuningMode
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[out] range: SensitivityRange
     * \return OK if found, otherwise return NAME_NOT_FOUND.
     */
    static int getSensitivityRangeByTuningMode(int cameraId, TuningMode mode,
                                               SensitivityRange& range);

    /**
     * check if PDAF is supported or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if PDAF is supported.
     */
    static bool isPdafEnabled(int cameraId);

    /**
     * get the sensor AWB
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return bool: the sensor AWB enable
     */
    static bool getSensorAwbEnable(int cameraId);

    /**
     * get the sensor AE
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return bool: the sensor AE enable
     */
    static bool getSensorAeEnable(int cameraId);

    /**
     * get the run ISP rate
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return bool: the run ISP rate
     */
    static bool getRunIspAlways(int cameraId);

    /**
     * get the DVS type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the DVS type
     */
    static int getDVSType(int cameraId);

    /**
     * get the ISYS compression flag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if ISYS compression is enabled
     */
    static bool getISYSCompression(int cameraId);

    /**
     * get the PSA compression flag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if PSA compression is enabled
     */
    static bool getPSACompression(int cameraId);

    /**
     * get the OFS compression flag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if OFS compression is enabled
     */
    static bool getOFSCompression(int cameraId);

    /**
     * get the max coarse integration time margin
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the value of max coarse integration time margin.
     */
    static int getCITMaxMargin(int cameraId);

    /**
     * Check AIQ is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if AIQ is enabled or not.
     */
    static bool isEnableAIQ(int cameraId);

    /**
     * get running rate of AIQ
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the running rate of AIQ
     */
    static int getAiqRunningInterval(int cameraId);

    /**
     * Check Mkn is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if Mkn is enabled or not.
     */
    static bool isEnableMkn(int cameraId);

    /**
     * get running rate of every Algorithms
     *
     * \param algo: one type of imaging_algorithm_t
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return float: the running rate of every Algorithms, otherwise return 0.0
     */
    static float getAlgoRunningRate(int algo, int cameraId);

    /**
     * if running rate is supported
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \\return if running rate is supported or not.
     */
    static bool isStatsRunningRateSupport(int cameraId);

    /**
     * Check if sensor digital gain is used or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if sensor gain is used or not.
     */
    static bool isUsingSensorDigitalGain(int cameraId);

    /**
     * Check if using isp digital gain or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if using isp gain or not.
     */
    static bool isUsingIspDigitalGain(int cameraId);

    /**
     * Check if need to pre-register buffers or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if pre-register buffers or not.
     */
    static bool isNeedToPreRegisterBuffer(int cameraId);

    /**
     * Check Defog(LTM) is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if Defog is enabled or not.
     */
    static bool isEnableDefog(int cameraId);

    // FRAME_SYNC_S
    /**
     * Check Frame Sync is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if Frame Sync is enabled or not.
     */
    static bool isEnableFrameSyncCheck(int cameraId);
    // FRAME_SYNC_E

    /**
     * Get exposure number
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param multiExposure: true or false
     * \return the value of exposure number according to different cases
     */
    static int getExposureNum(int cameraId, bool multiExposure);

    /**
     * Check LTM is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if LTM is enabled or not.
     */
    static bool isLtmEnabled(int cameraId);

    /**
     * Get media format
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of media format.
     */
    static ia_media_format getMediaFormat(int cameraId);

  // HDR_FEATURE_S
    /**
     * Update media format using isNarrow indicator
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param isNarrow: if the media format is narrow or not
     * \return if update sucessfully or not.
     */
    static bool updateMediaFormat(int cameraId, bool isNarrow);

    /**
     * Check HDR is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if HDR is enabled or not.
     */
    static bool isEnableHDR(int cameraId);

    /**
     * Get HDR stats input bit depth
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of HDR stats input bit depth
     */
    static int getHDRStatsInputBitDepth(int cameraId);

    /**
     * Get HDR stats output bit depth
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of HDR stats output bit depth
     */
    static int getHDRStatsOutputBitDepth(int cameraId);

    /**
     * Get if HDR exposure info is fixed or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if HDR exposure info is fixed
     */
    static int isUseFixedHDRExposureInfo(int cameraId);
    // HDR_FEATURE_E

    /**
     * Get if multi exposure cases or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] TuningMode: tuningMode
     * \return true if multi exposure case
     */
    static bool isMultiExposureCase(int cameraId, TuningMode tuningMode);

    /**
     * Get sensor exposure type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of sensor exposure type
     */
    static int getSensorExposureType(int cameraId);

    /**
     * Get sensor gain type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of sensor gain type
     */
    static int getSensorGainType(int cameraId);

    /**
     * Get sensor's initial skip frame number
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of initial skip frame number
     */
    static unsigned int getInitialSkipFrame(int cameraId);

    /**
     * Get initial pending frame number
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of initial pending frame number
     */
    static unsigned int getInitialPendingFrame(int cameraId);

    /**
     * Get max raw data number
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of max raw data number
     */
    static unsigned int getMaxRawDataNum(int cameraId);

    /**
     * Get sensor's top bottom filed reverse option
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of top bottom filed reverse value
     */
    static bool getTopBottomReverse(int cameraId);

    /*
     * Check if Psys continuous stats is needed or not.
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if it is needed
     */
    static bool isPsysContinueStats(int cameraId);

    /**
     * Get preferred buffer queue size
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of preferred buffer queue size
     */
    static unsigned int getPreferredBufQSize(int cameraId);

    /**
     * Get Ltm Gain lag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of LTM gain lag
     */
    static int getLtmGainLag(int cameraId);

    /**
     * Check ltm thread is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if ltm thread is enabled or not.
     */
    static bool isEnableLtmThread(int cameraId);

    /**
     * Check if H-Scheduler is enabled
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if H-Scheduler is enabled
     */
    static bool isSchedulerEnabled(int cameraId);

    /**
     * Check face engine is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if face engine is enabled or not.
     */
    static bool isFaceDetectionSupported(int cameraId);

    /**
     * Check face AE is enabled or not, only for debug
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if face ae is enabled or not.
     */
    static bool isFaceAeEnabled(int cameraId);

    /**
     * get face engine's vendor
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the face engine's vendor.
     */
    static int faceEngineVendor(int cameraId);

    /**
     * get face engine's running interval
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the face engine running interval value.
     */
    static int faceEngineRunningInterval(int cameraId);

    /**
     * get face engine's running interval when face is not found
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the face engine running interval value when face is not found.
     */
    static int faceEngineRunningIntervalNoFace(int cameraId);

    /**
     * Check face detection runs synchronously or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if face detection runs synchronously or not.
     */
    static bool isFaceEngineSyncRunning(int cameraId);

    /**
     * Whether IPU has a single output to support face detection
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true or false.
     */
    static bool isIPUSupportFD(int cameraId);

    /**
     * get the max number of face detection
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the max number of face detection.
     */
    static unsigned int getMaxFaceDetectionNumber(int cameraId);

    /**
     * get dvs supported status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true: dvs supported; false: dvs not supported.
     */
    static bool isDvsSupported(int cameraId);

    /**
     * Check psys align with sof is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if psys align with sof is enabled or not.
     */
    static bool psysAlignWithSof(int cameraId);

    /**
     * Check running psys bundle with aic is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if running psys bundle with aic is enabled or not.
     */
    static bool psysBundleWithAic(int cameraId);

    /**
     * Check software processing align with isp is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if software processing align with isp is enabled or not.
     */
    static bool swProcessingAlignWithIsp(int cameraId);

    /**
     * Get the max digital gain of sensor
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of max digital gain
     */
    static int getMaxSensorDigitalGain(int cameraId);

    /**
     * Get sensor digital gain type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the sensor digital gain type
     */
    static SensorDgType sensorDigitalGainType(int cameraId);

    /**
     * Get sensor's digital gain lag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of digital gain lag
     */
    static int getDigitalGainLag(int cameraId);
    /**
     * Get sensor's exposure lag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of exposure lag
     */
    static int getExposureLag(int cameraId);

    /**
     * Get sensor's analog gain lag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of analog gain lag
     */
    static int getAnalogGainLag(int cameraId);

    /**
     * Get EdgeNrSetting based on total gain and hdr ratio
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] totalGain: total gain
     * \param[in] hdrRatio: hdr ratio
     * \param[in] mode: TuningMode
     * \param[out] setting: EdgeNrSetting setting
     *
     * \return OK if setting is available, otherwise return NAME_NOT_FOUND.
     */
    static int getEdgeNrSetting(int cameraId, float totalGain, float hdrRatio,
                                TuningMode mode, EdgeNrSetting& setting);

    /**
     * Get the executor policy config.
     *
     * \param[in] graphIds: the graph ids
     *
     * \return PolicyConfig* object if found, otherwise return nullptr.
     */
    static PolicyConfig* getExecutorPolicyConfig(const std::set<int>& graphIds);

    /**
     * According to stream info to select MC
     * this function will compare the format/resolutions/interlace to find the MediaCtlConf
     * and then store it into cameraInfo.
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param stream: the stream info
     * \param mode: the stream operation mode
     */
    static void selectMcConf(int cameraId, stream_t stream, ConfigMode mode, int mcId);

    /**
     * to get the current MediaCtlConf
     * after the media controller has been analyzed, the media controller information will be stored
     * in the mMediaCtlConfs.
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return MediaCtlConf*, if it doens't find one, this function will return nullptr.
     */
    static MediaCtlConf* getMediaCtlConf(int cameraId);

    /**
     * \brief Fill camera info and capability according to given camera id
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[out] camera_info_t info
     *
     * \return 0 if succeed, other value indicates failed.
     */
    static int getCameraInfo(int cameraId, camera_info_t& info);

    /**
     * \brief Check if the camera_features feature is supported
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] camera_features feature
     *
     * \return true if supported, otherwise return false.
     */
    static bool isFeatureSupported(int cameraId, camera_features feature);

    /**
     * \brief Check if the given stream config is supported
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] stream_t conf
     *
     * \return true if supported, otherwise return false.
     */
    static bool isSupportedStream(int cameraId, const stream_t& conf);

    /**
     * get the isys supported size list
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param sizes: the function will fill the isys supported size list to the sizes
     */
    static void getSupportedISysSizes(int cameraId, std::vector<camera_resolution_t>& resolutions);

    /**
     * get the isys supported format list
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param formats: the function will fill the isys supported format list to the formats
     * \return true if success, return false if it fails.
     */
    static bool getSupportedISysFormats(int cameraId, std::vector<int>& formats);

    /**
     * Format for the ISYS output
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the format for the isys output
     */
    static int getISysFormat(int cameraId);

    /**
     * Set ISYS output format
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param format: the isys output format
     */
    static void selectISysFormat(int cameraId, int format);

    /**
     * If ISYS supported format.
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     */
    static bool isISysSupportedFormat(int cameraId, int format);

    /**
     * if the resolution is supported by Isys
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param resolution: the requested resolution
     * \return true if the resolution is supported by isys, otherwise false
     */
    static bool isISysSupportedResolution(int cameraId, camera_resolution_t resolution);

    /**
     * Check if the frame needs to be skipped when STR2MMIO error occurs
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if corrupted frame needs to be skipped or not.
     */
    static bool isSkipFrameOnSTR2MMIOErr(int cameraId);

    /**
     * Format for the ISYS RAW output
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the RAW format if isys scale enabled
     */
    static int getISysRawFormat(int cameraId);

    /**
     * Get the config of the ISYS output per port
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the config of the ISYS output for the given port.
     */
    static stream_t getISysOutputByPort(int cameraId, Port port);

    // CSI_META_S
    /**
     * get CSI meta enabled status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if CSI meta is enabled, otherwise return false
     */
    static bool isCsiMetaEnabled(int cameraId);
    // CSI_META_E

    /**
     * get the format by device name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param devName: device name
     * \format: return param for format
     * \return the status
     */
    static int getFormatByDevName(int cameraId, const std::string& devName, McFormat& format);

    /**
     * get the video node name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param videoNodeType: value of enum VideoNodeType
     * \param videoNodeName: return param for the video node name
     * \return the status
     */
    static int getVideoNodeNameByType(int cameraId, VideoNodeType videoNodeType,
                                      std::string& videoNodeName);

    /**
     * get the hardware device name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param videoNodeType: value of enum VideoNodeType
     * \param devName: return param for the device name
     * \return the status
     */
    static int getDevNameByType(int cameraId, VideoNodeType videoNodeType, std::string& devName);

    // DOL_FEATURE_S
    /**
     * get fixed VBP from currently selected media configure
     * VBP is used to parse sensor output frame under DOL mode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return fixed vbp value
     */
    static int getFixedVbp(int cameraId);

    /**
     * whether need to handle VBP info from meta data
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configMode: the real configure mode
     * \return true if needed, else false
     */
    static bool needHandleVbpInMetaData(int cameraId, ConfigMode configMode);

    /**
     * whether need to pass down VBP info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configMode: the real configure mode
     * \return true if needed, else false
     */
    static bool needSetVbp(int cameraId, ConfigMode configMode);

    /**
     * Check DOL VBP offset
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param dolVbpOffset: return vbp offset for low and high bytes
     */
    static void getDolVbpOffset(int cameraId, std::vector<int>& dolVbpOffset);

    /**
     * get CSI BE SOC for Dol short exposure output enabled status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if video node is enabled, otherwise return false
     */
    static bool isDolShortEnabled(int cameraId);

    /**
     * get CSI BE SOC for Dol medium exposure output enabled status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if video node is enabled, otherwise return false
     */
    static bool isDolMediumEnabled(int cameraId);
    // DOL_FEATURE_E

    /**
     * Check if ISYS is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if ISYS is enabled, otherwise return false
     */
    static bool isIsysEnabled(int cameraId);

    static int calculateFrameParams(int cameraId, SensorFrameParams& sensorFrameParams);

    /**
     * Get the optmized resolutions that supported by input system
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param width:    The width of the request frame
     * \param height:   The height of the request frame
     * \param field:    The field of the request frame
     *
     * \return camera_resolution_t: The optimized resolution that used to configure the ISYS.
     */
    static camera_resolution_t getISysBestResolution(int cameraId, int width, int height,
                                                     int field);

    /**
     * to get if it support the format
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param format:the format will be checked if the psys supports.
     * \return true or false for the psys could be used or not.
     */
    static bool usePsys(int cameraId, int format);

    /**
     * to get supported psys dag config
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configs: the function will fill supported psys dag config list to the configs
     */
    static void getSupportedTuningConfig(int cameraId, std::vector<TuningConfig>& configs);

    /**
     * to get the ConfigMode by operation Mode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param operationMode: the operation mode
     * \param configModes: the function will fill available ConfigMode for this operation mode
     * \return OK if get ConfigMode, otherwise return INVALID_OPERATION
     */
    static int getConfigModesByOperationMode(int cameraId, uint32_t operationMode,
                                             std::vector<ConfigMode>& configModes);

    /**
     * to reorder tuning config
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configMode: type of ConfigMode
     */
    static void reorderSupportedTuningConfig(int cameraId, ConfigMode configMode);

    /**
     * to get the TuningMode by Config Mode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configMode: type of ConfigMode (except auto)
     * \param tuningMode: return related TuningMode
     * \return OK if get TuningMode, otherwise return INVALID_OPERATION
     */
    static int getTuningModeByConfigMode(int cameraId, ConfigMode configMode,
                                         TuningMode& tuningMode);

    /**
     * to get tuning config by ConfigMode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param mode: ConfigMode
     * \param config: return related TuningConfig
     * \return OK if get TuningConfig, otherwise return INVALID_OPERATION
     */
    static int getTuningConfigByConfigMode(int cameraId, ConfigMode mode, TuningConfig& config);

    /*
     * Get stream id by the given configMode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configMode: ConfigMode
     * \return the stream id if succeeds, otherwise return -1.
     */
    static int getStreamIdByConfigMode(int cameraId, ConfigMode configMode);

    /*
     * Get the max requests number in flight
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the max requests number in flight
     */
    static int getMaxRequestsInflight(int cameraId);

    /**
     * get yuv color range mode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the correponding camera_yuv_color_range_mode_t.
     */
    static camera_yuv_color_range_mode_t getYuvColorRangeMode(int cameraId);

    /**
     * Get aiqd
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param tuningMode: mode
     * \return ia_binary_data
     */
    static ia_binary_data* getAiqd(int cameraId, TuningMode mode);

    /**
     * Save aiqd
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param tuningMode: mode
     * \param ia_binary_data: data
     */
    static void saveAiqd(int cameraId, TuningMode tuningMode, const ia_binary_data& data);

    /**
     * Get cpf
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER]
     * \param mode: tuning mode
     * \param aiqbData: cpf
     * \return OK if it is successful.
     */
    static int getCpf(int cameraId, TuningMode mode, ia_binary_data* aiqbData);

    /**
     * If dynamic graph config enabled
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if use graphConfig file.
     */
    static bool getGraphConfigNodes(int cameraId);

    /**
     * to get the type of graph settings
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the graph settings type: COUPLED or DISPERSED.
     */
    static GraphSettingType getGraphSettingsType(int cameraId);

    /**
     * if ISYS CSI Back End capture enabled
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if the current sensor is CSI Back End capture or not
     */
    static bool isCSIBackEndCapture(int cameraId);

    /**
     * if ISYS CSI Front End capture enabled
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if the current sensor is CSI Front End capture or not
     */
    static bool isCSIFrontEndCapture(int cameraId);

    /**
     * if AIQD enabled
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if AIQD is enabled or not
     */
    static bool isAiqdEnabled(int cameraId);

    /**
     * if image from tpg
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if frame is from tpg or not
     */
    static bool isTPGReceiver(int cameraId);

    static int getSupportAeExposureTimeRange(int cameraId, camera_scene_mode_t sceneMode,
                                             camera_range_t& etRange);
    static int getSupportAeGainRange(int cameraId, camera_scene_mode_t sceneMode,
                                     camera_range_t& gainRange);

    /**
     * if CrlModule is used
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if CrlModule driver is used, otherwise return false
     */
    static bool isUsingCrlModule(int cameraId);

    /**
     * to get the MultiExpRange of CameraInfo
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the MultiExpRange for current camera id.
     */
    static std::vector<MultiExpRange> getMultiExpRanges(int cameraId);

    // FILE_SOURCE_S
    /**
     * Get the injected file.
     *
     * \return the injected file or nullptr if cameraInjectFile isn't set.
     */
    static const char* getInjectedFile();

    /**
     * Check if FileSource is enabled.
     *
     * \return true cameraInjectFile is set, otherwise return false.
     */
    static bool isFileSourceEnabled();
    // FILE_SOURCE_E

    // VIRTUAL_CHANNEL_S
    /**
     * Get virtual channel sequence
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return sequence if Virtual channel supported, otherwise return -1.
     */
    static int getVirtualChannelSequence(int cameraId);

    /**
     * Get aggregator info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param aggregator: Struct VcAggregator
     * \return OK if found, otherwise return NO_ENTRY.
     */
    static int getVcAggregator(int cameraId, struct VcAggregator& aggregator);
    // VIRTUAL_CHANNEL_E

    /**
     * Get the psl output resolution
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param width:    The width of user requirement
     * \param height:   The height of user requirement
     * \return the psl output resolution if provides it in xml file, otherwise return nullptr.
     */
    static camera_resolution_t* getPslOutputForRotation(int width, int height, int cameraId);

    /**
     * Get preferred output size
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param width:    The width of user requirement
     * \param height:   The height of user requirement
     * \return the output resolution if provides it in xml file, otherwise return nullptr.
     */
    const static camera_resolution_t* getPreferOutput(int width, int height, int cameraId);

    /**
     * Check if test pattern is supported or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if mTestPatternMap is defined, otherwise return false.
     */
    static bool isTestPatternSupported(int cameraId);

    /**
     * get sensor test pattern
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param mode: camera_test_pattern_mode_t
     * \return corresponding sensor test pattern if provided in xml file, otherwise return -1.
     */
    static int32_t getSensorTestPattern(int cameraId, int32_t mode);

    /**
     * Get the nvm
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return ia_binary_data
     */
    static ia_binary_data* getNvm(int cameraId);

    /**
     * Get sensor active array size
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of camera_coordinate_system_t.
     */
    static camera_coordinate_system_t getActivePixelArray(int cameraId);

    /**
     * Get camera cfg path from environment variable
     *
     * \param void
     * \return the value of camera cfg path.
     */
    static std::string getCameraCfgPath();

    /**
     * Get camera graph descriptor file path
     *
     * \param void
     * \return the value of camera graph descriptor file path.
     */
    static std::string getGraphDescFilePath();

    /**
     * Get camera graph setting file path.
     *
     * \param void
     * \return the value of camera graph setting file path.
     */
    static std::string getGraphSettingFilePath();

    /*
     * Get sensor value for the digital gain.
     *
     * Since the calculation formula may be different between sensors,
     * so we need to get this value based on sensor digital gain type.
     * For imx274, the magnification = 2^x (x is the register value).
     *
     * Need to specify the sensorDgType, maxSensorDg and useIspDigitalGain in xml.
     */
    static int getSensorDigitalGain(int cameraId, float realDigitalGain);

    /*
     * Get the isp gain
     *
     * Separate real digital to sensorDg and ispDg, and the ispDg >= 1
     */
    static float getIspDigitalGain(int cameraId, float realDigitalGain);

    /**
     * \brief init Makernote
     * allocate memory resource for Makernote
     *
     * param[in] int cameraId: camera ID
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] TuningMode tuningMode
     *
     * return OK if it is successful, fails for other values.
     */
    static int initMakernote(int cameraId, TuningMode tuningMode);

    /**
     * \brief deinit Makernote
     * free memory resource for Makernote
     *
     * param[in] int cameraId: camera ID
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] TuningMode tuningMode
     *
     * return OK if it is successful, fails for other values.
     */
    static int deinitMakernote(int cameraId, TuningMode tuningMode);

    /**
     * \brief Save Makernote by ia_mkn_trg mode
     *
     * param[in] int cameraId: camera ID
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] camera_makernote_mode_t: MAKERNOTE_MODE_JPEG is corresponding
     *           to ia_mkn_trg_section_1 for Normal Jpeg capture;
     *           MAKERNOTE_MODE_RAW is corresponding to ia_mkn_trg_section_2
     *           for Raw image capture.
     * \param[in] int64_t sequence: the sequence in latest AiqResult
     * param[in] TuningMode tuningMode: tuning mode
     *
     * \return OK if get Makernote successfully, otherwise return ERROR.
     */
    static int saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode,
                                 int64_t sequence, TuningMode tuningMode);

    /**
     * \brief Get Makernote buffer
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] makernoteMode: makernote mode
     * \param[out] dump: return dump if in dump case
     *
     * \return the pointer of makernote buffer
     */
    static void* getMakernoteBuf(int cameraId, camera_makernote_mode_t makernoteMode,
                                 bool& dump);

    /**
     * \brief Update Makernote timestamp.
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] sequence: the sequence in frame buffer;
     * \param[in] timestamp: the frame timestamp corresponding sequence
     *
     */
    static void updateMakernoteTimeStamp(int cameraId, int64_t sequence, uint64_t timestamp);

    /**
     * \brief acquire Makernote data.
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] sequence: acquire MakerNote per timestamp
     * \param[out] param: Makernote data will be saved in Parameters as metadata.
     *
     */
    static void acquireMakernoteData(int cameraId, uint64_t timestamp, Parameters* param);

    /*
     * Get the scaler info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param streamId: hal stream id
     * \param sclscalerWidth and scalerHeight : return related scaler info
     * \return OK.
     */
    static int getScalerInfo(int cameraId, int32_t streamId, float* scalerWidth,
                             float* scalerHeight);

    /*
     * Set the scaler info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param scalerInfo related IGraphType::ScalerInfo
     */
    static void setScalerInfo(int cameraId, std::vector<IGraphType::ScalerInfo> scalerInfo);

    /**
     * Check gpu tnr is enabled or not
     *
     * \return true if tnr is enabled.
     */
    static bool isGpuTnrEnabled(int cameraId);

    /**
     * get the video stream number supported
     *
     * \return HAL video stream number.
     */
    static int getVideoStreamNum();

    /**
     * Check if support to update tuning data or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     */
    static bool supportUpdateTuning(int cameraId);

    /**
     * Check if support hardware jpeg encode or not
     */
    static bool supportHwJpegEncode();

    /**
     * get the max ISYS timeout value
     */
    static int getMaxIsysTimeout();

    /**
     * Check should connect gpu algo or not
     * should connect gpu algo service if any gpu algorithm is used
     * \return true if should connect gpu algo.
     */
    static bool isUsingGpuAlgo();

    /**
     * Check if still stream tnr is prior to video tnr
     */
    static bool isStillTnrPrior();

    /**
     * Check if update tnr7us params every frame
     */
    static bool isTnrParamForceUpdate();

    /**
     * the extra frame count for still stream
     */
    static int getTnrExtraFrameCount(int cameraId);

    /**
     * Check if global protection is enabled for tnr7us
     */
    static bool useTnrGlobalProtection();

    /*
     * Set the orientation Info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param sensor orientation
     */
    static void setSensorOrientation(int cameraId, int orientation);

    /*
     * Get the orientation Info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return sensor orientation
     */
    static int getSensorOrientation(int cameraId);

    /**
     * check if support dummy still stream
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if supported.
     */
    static bool isDummyStillSink(int cameraId);

    /**
     * get the tnr resolution list
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     */
    static void getTnrThresholdSizes(int cameraId, std::vector<camera_resolution_t>& resolutions);
    /*
     * check if removing cache flush output buffer
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if removing cache flush output buffer.
     */
    static bool removeCacheFlushOutputBuffer(int cameraId);

    /*
     * Get PLC Enable status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if supported.
     */
    static bool getPLCEnable(int cameraId);

    // PRIVACY_MODE_S
    /**
     * Check which privacy mode the camera supports
     *
     * \return privacy mode type.
     */
    static PrivacyModeType getSupportPrivacy(int cameraId);

    /**
     * Get the threshold of luminance in AE-based privacy mode.
     *
     * \return Threshold
     */
    static uint32_t getPrivacyModeThreshold(int cameraId);

    /**
     * Get the threshold of frame delay in AE-based privacy mode.
     *
     * \return Threshold
     */
    static uint32_t getPrivacyModeFrameDelay(int cameraId);
    // PRIVACY_MODE_E

    /**
     * Check support of still-only pipe is enabled or not
     *
     * \return true if is enabled.
     */
    static bool isStillOnlyPipeEnabled(int cameraId);

    /**
     * Check if BLC needs to be disabled in specific analog gain range.
     *
     * \return true if BLC needs to be disabled by analog gain.
     */
    static bool getDisableBLCByAGain(int cameraId, int& low, int& high);

    /**
     * Check if reset links and routes
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if reset links and routes
     */
    static bool isResetLinkRoute(int cameraId);

  /**
     * Get use defined timeout val for dqbuf
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return timeout interval for dqbuf in ns (2000000000 for 2s)
     */
    static int64_t getReqWaitTimeout(int cameraId);

    // LEVEL0_ICBM_S
    /**
     * Check GPU ICBM is enabled or not
     *
     * \return true if ICBM is enabled.
     */
    static bool isGPUICBMEnabled();
    // LEVEL0_ICBM_E
};
} /* namespace icamera */
