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

#define LOG_TAG "PlatformData"

#include <sys/sysinfo.h>
#include <math.h>
#include <memory>

#include "iutils/CameraLog.h"

#include "PlatformData.h"
#include "CameraParser.h"
#include "PolicyParser.h"
#include "ParameterHelper.h"

#include "gc/GraphConfigManager.h"

using std::string;
using std::vector;

namespace icamera {
PlatformData *PlatformData::sInstance = nullptr;
Mutex  PlatformData::sLock;

PlatformData* PlatformData::getInstance()
{
    AutoMutex lock(sLock);
    if (sInstance == nullptr) {
        sInstance = new PlatformData();
    }

    return sInstance;
}

void PlatformData::releaseInstance()
{
    AutoMutex lock(sLock);
    LOG1("@%s", __func__);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

PlatformData::PlatformData()
{
    LOG1("@%s", __func__);
    MediaControl *mc = MediaControl::getInstance();
    if (mc) {
        mc->initEntities();
    }

    CameraParser CameraParser(mc, &mStaticCfg);
    PolicyParser PolicyParser(&mStaticCfg);
}

PlatformData::~PlatformData() {
    LOG1("@%s", __func__);

    releaseGraphConfigNodes();

    MediaControl::getInstance()->clearEntities();
    MediaControl::releaseInstance();

    for (size_t i = 0; i < mAiqInitData.size(); i++) {
        delete mAiqInitData[i];
    }

    mAiqInitData.clear();
}

bool PlatformData::isSensorAvailable() {
    LOG2("@%s", __func__);

    return !getInstance()->mStaticCfg.mCommonConfig.availableSensors.empty();
}

int PlatformData::init() {
    LOG2("@%s", __func__);

    getInstance()->parseGraphFromXmlFile();

    StaticCfg *staticCfg = &(getInstance()->mStaticCfg);
    for (size_t i = 0; i < staticCfg->mCameras.size(); i++) {
        AiqInitData* aiqInitData =
            new AiqInitData(staticCfg->mCameras[i].sensorName,
                            getCameraCfgPath(),
                            staticCfg->mCameras[i].mSupportedTuningConfig,
                            staticCfg->mCameras[i].mLardTagsConfig,
                            staticCfg->mCameras[i].mNvmDirectory,
                            staticCfg->mCameras[i].mMaxNvmDataSize);
        getInstance()->mAiqInitData.push_back(aiqInitData);
    }

    return OK;
}

/**
 * Read graph descriptor and settings from configuration files.
 *
 * The resulting graphs represend all possible graphs for given sensor, and
 * they are stored in capinfo structure.
 */
void PlatformData::parseGraphFromXmlFile() {
    std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>();

    // Assuming that PSL section from profiles is already parsed, and number
    // of cameras is known.
    graphConfig->addCustomKeyMap();
    for (size_t i = 0; i < getInstance()->mStaticCfg.mCameras.size(); ++i) {
        const string &fileName = getInstance()->mStaticCfg.mCameras[i].mGraphSettingsFile;
        if (fileName.empty()) {
            continue;
        }

        LOGXML("Using graph setting file:%s for camera:%zu", fileName.c_str(), i);
        int ret  = graphConfig->parse(i, fileName.c_str());
        CheckError(ret != OK, VOID_VALUE, "Could not read graph config file for camera %zu", i);
    }
}

void PlatformData::releaseGraphConfigNodes()
{
    std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>();
    graphConfig->releaseGraphNodes();
    for (uint8_t cameraId = 0; cameraId < mStaticCfg.mCameras.size(); cameraId++) {
        IGraphConfigManager::releaseInstance(cameraId);
    }
}

const char* PlatformData::getSensorName(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].sensorName.c_str();
}

const char* PlatformData::getSensorDescription(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].sensorDescription.c_str();
}

const char* PlatformData::getLensName(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mLensName.c_str();
}

int PlatformData::getLensHwType(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mLensHwType;
}

int PlatformData::getDVSType(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mDVSType;
}

bool PlatformData::getISYSCompression(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mISYSCompression;
}

bool PlatformData::getPSACompression(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mPSACompression;
}

bool PlatformData::getOFSCompression(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mOFSCompression;
}

int PlatformData::getCITMaxMargin(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mCITMaxMargin;
}

bool PlatformData::isEnableAIQ(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableAIQ;
}

bool PlatformData::isEnableLtmThread(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableLtmThread;
}

bool PlatformData::isFaceAeEnabled(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceAeEnabled;
}

int PlatformData::faceEngineRunningInterval(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningInterval;
}

int PlatformData::faceEngineRunningIntervalNoFace(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningIntervalNoFace;
}

bool PlatformData::isFaceEngineSyncRunning(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningSync;
}

bool PlatformData::isDvsSupported(int cameraId)
{
    camera_video_stabilization_list_t videoStabilizationList;
    Parameters* param = &getInstance()->mStaticCfg.mCameras[cameraId].mCapability;
    param->getSupportedVideoStabilizationMode(videoStabilizationList);

    bool supported = false;
    for (auto it : videoStabilizationList) {
        if (it == VIDEO_STABILIZATION_MODE_ON) {
            supported = true;
        }
    }

    LOG2("@%s, dvs supported:%d", __func__, supported);
    return supported;
}

bool PlatformData::psysAlignWithSof(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysAlignWithSof;
}

bool PlatformData::psysBundleWithAic(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysBundleWithAic;
}

bool PlatformData::swProcessingAlignWithIsp(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mSwProcessingAlignWithIsp;
}

bool PlatformData::isUsingSensorDigitalGain(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseSensorDigitalGain;
}

bool PlatformData::isUsingIspDigitalGain(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseIspDigitalGain;
}

bool PlatformData::isNeedToPreRegisterBuffer(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mNeedPreRegisterBuffers;
}

int PlatformData::getAutoSwitchType(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mAutoSwitchType;
}

bool PlatformData::isEnableFrameSyncCheck(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mFrameSyncCheckEnabled;
}

bool PlatformData::isEnableDefog(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableLtmDefog;
}

int PlatformData::getExposureNum(int cameraId, bool multiExposure)
{
    if (multiExposure) {
        return getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureNum;
    }

    int exposureNum = 1;

    return exposureNum;
}

bool PlatformData::isLtmEnabled(int cameraId)
{

    return getInstance()->mStaticCfg.mCameras[cameraId].mLtmEnabled;
}

int PlatformData::getSensorExposureType(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureType;
}

int PlatformData::getSensorGainType(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorGainType;
}

bool PlatformData::isSkipFrameOnSTR2MMIOErr(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mSkipFrameV4L2Error;
}

unsigned int PlatformData::getInitialSkipFrame(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mInitialSkipFrame;
}

unsigned int PlatformData::getMaxRawDataNum(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxRawDataNum;
}

bool PlatformData::getTopBottomReverse(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mTopBottomReverse;
}

bool PlatformData::isPsysContinueStats(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysContinueStats;
}

unsigned int PlatformData::getPreferredBufQSize(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mPreferredBufQSize;
}

unsigned int PlatformData::getPipeSwitchDelayFrame(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mPipeSwitchDelayFrame;
}

int PlatformData::getLtmGainLag(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mLtmGainLag;
}

int PlatformData::getMaxSensorDigitalGain(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxSensorDigitalGain;
}

SensorDgType PlatformData::sensorDigitalGainType(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorDgType;
}

int PlatformData::getDigitalGainLag(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mDigitalGainLag;
}

int PlatformData::getExposureLag(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mExposureLag;
}

int PlatformData::getAnalogGainLag(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mAnalogGainLag;
}

PolicyConfig* PlatformData::getExecutorPolicyConfig(int graphId)
{
    size_t i = 0;
    PlatformData::StaticCfg *cfg = &getInstance()->mStaticCfg;

    for (i = 0; i < cfg->mPolicyConfig.size(); i++) {
        if (graphId == cfg->mPolicyConfig[i].graphId) {
            return &(cfg->mPolicyConfig[i]);
        }
    }

    LOGW("Couldn't find the executor policy for graphId(%d), please check xml file", graphId);
    return nullptr;
}

int PlatformData::numberOfCameras()
{
    return getInstance()->mStaticCfg.mCameras.size();
}

int PlatformData::getXmlCameraNumber()
{
    return getInstance()->mStaticCfg.mCommonConfig.cameraNumber;
}

MediaCtlConf *PlatformData::getMediaCtlConf(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mCurrentMcConf;
}

int PlatformData::getCameraInfo(int cameraId, camera_info_t& info)
{
    // TODO correct the version info
    info.device_version = 1;
    info.facing = getInstance()->mStaticCfg.mCameras[cameraId].mFacing;
    info.orientation= getInstance()->mStaticCfg.mCameras[cameraId].mOrientation;
    info.name = getSensorName(cameraId);
    info.description = getSensorDescription(cameraId);
    info.capability = &getInstance()->mStaticCfg.mCameras[cameraId].mCapability;
    return OK;
}

bool PlatformData::isFeatureSupported(int cameraId, camera_features feature)
{
    camera_features_list_t features;
    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSupportedFeatures(features);

    if (features.empty()) {
        return false;
    }
    for (auto& item : features) {
        if (item == feature) {
            return true;
        }
    }
    return false;
}

bool PlatformData::isSupportedStream(int cameraId, const stream_t& conf)
{
    int width = conf.width;
    int height = conf.height;
    int format = conf.format;
    int field = conf.field;

    stream_array_t availableConfigs;
    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSupportedStreamConfig(availableConfigs);
    bool sameConfigFound = false;
    for (auto const& config : availableConfigs) {
        if (config.format == format && config.field == field
                && config.width == width && config.height == height) {
            sameConfigFound = true;
            break;
        }
    }

    return sameConfigFound;
}

void PlatformData::getSupportedISysSizes(int cameraId, vector <camera_resolution_t>& resolutions)
{
    resolutions = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysSizes;
}

bool PlatformData::getSupportedISysFormats(int cameraId, vector <int>& formats)
{
    formats = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysFormat;

    return true;
}

int PlatformData::getISysFormat(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc;
}

/**
 * The ISYS format is determined by the steps below:
 * 1. Try to use the specified format in media control config if it exists.
 * 2. If the given format is supported by ISYS, then use it.
 * 3. Use the first supported format if still could not find an appropriate one.
 */
void PlatformData::selectISysFormat(int cameraId, int format)
{
    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    if (mc != nullptr && mc->format != -1) {
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = mc->format;
    } else if (isISysSupportedFormat(cameraId, format)) {
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = format;
    } else {
        // Set the first one in support list to default Isys output.
        vector <int> supportedFormat =
            getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysFormat;
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = supportedFormat[0];
    }
}

/**
 * The media control config is determined by the steps below:
 * 1. Check if can get one from the given MC ID.
 * 2. And then, try to use ConfigMode to find matched one.
 * 3. Use stream config to get a corresponding mc id, and then get the config by id.
 * 4. Return nullptr if still could not find an appropriate one.
 */
void PlatformData::selectMcConf(int cameraId, stream_t stream, ConfigMode mode, int mcId)
{
    if (!isIsysEnabled(cameraId)) return;

    const StaticCfg::CameraInfo& pCam = getInstance()->mStaticCfg.mCameras[cameraId];

    MediaCtlConf* mcConfig = getMcConfByMcId(pCam, mcId);
    if (!mcConfig) {
        mcConfig = getMcConfByConfigMode(pCam, stream, mode);
    }

    if (!mcConfig) {
        mcConfig = getMcConfByStream(pCam, stream);
    }

    getInstance()->mStaticCfg.mCameras[cameraId].mCurrentMcConf = mcConfig;

    if (!mcConfig) {
        LOGE("No matching McConf: cameraId %d, configMode %d, mcId %d", cameraId, mode, mcId);
    }
}

/*
 * Find the MediaCtlConf based on the given MC id.
 */
MediaCtlConf* PlatformData::getMcConfByMcId(const StaticCfg::CameraInfo& cameraInfo, int mcId)
{
    if (mcId == -1) {
        return nullptr;
    }

    for (auto& mc : cameraInfo.mMediaCtlConfs) {
        if (mcId == mc.mcId) {
            return (MediaCtlConf*)&mc;
        }
    }

    return nullptr;
}

/*
 * Find the MediaCtlConf based on MC id in mStreamToMcMap.
 */
MediaCtlConf* PlatformData::getMcConfByStream(const StaticCfg::CameraInfo& cameraInfo,
                                              const stream_t& stream)
{
    int mcId = -1;
    for (auto& table : cameraInfo.mStreamToMcMap) {
        for(auto& config : table.second) {
            if (config.format == stream.format && config.field == stream.field
                    && config.width == stream.width && config.height == stream.height) {
                mcId = table.first;
                break;
            }
        }
        if (mcId != -1) {
            break;
        }
    }

    return getMcConfByMcId(cameraInfo, mcId);
}

/*
 * Find the MediaCtlConf based on operation mode and stream info.
 */
MediaCtlConf* PlatformData::getMcConfByConfigMode(const StaticCfg::CameraInfo& cameraInfo,
                                                  const stream_t& stream, ConfigMode mode)
{
    for (auto& mc : cameraInfo.mMediaCtlConfs) {
        for (auto& cfgMode : mc.configMode) {
            if (mode != cfgMode) continue;

            int outputWidth = mc.outputWidth;
            int outputHeight = mc.outputHeight;
            int stride = CameraUtils::getStride(mc.format, mc.outputWidth);
            bool sameStride = (stride == CameraUtils::getStride(mc.format, stream.width));
            /*
             * outputWidth and outputHeight is 0 means the ISYS output size
             * is dynamic, we don't need to check if it matches with stream config.
             */
            if ((outputWidth == 0 && outputHeight == 0 ) ||
                ((stream.width == outputWidth || sameStride)
                && stream.height == outputHeight)) {
                return (MediaCtlConf*)&mc;
            }
        }
    }

    return nullptr;
}

/*
 * Check if video node is enabled via camera Id and video node type.
 */
bool PlatformData::isVideoNodeEnabled(int cameraId, VideoNodeType type) {
    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    if (!mc) return false;

    for(auto const& nd : mc->videoNodes) {
        if (type == nd.videoNodeType) {
            return true;
        }
    }
    return false;
}

bool PlatformData::isISysSupportedFormat(int cameraId, int format)
{
    vector <int> supportedFormat;
    getSupportedISysFormats(cameraId, supportedFormat);

    for (auto const fmt : supportedFormat) {
        if (format == fmt)
            return true;
    }
    return false;
}

bool PlatformData::isISysSupportedResolution(int cameraId, camera_resolution_t resolution)
{
    vector <camera_resolution_t> res;
    getSupportedISysSizes(cameraId, res);

    for (auto const& size : res) {
        if (resolution.width == size.width && resolution.height== size.height)
            return true;
    }

    return false;
}

int PlatformData::getISysRawFormat(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mISysRawFormat;
}

stream_t PlatformData::getISysOutputByPort(int cameraId, Port port)
{
    stream_t config;
    CLEAR(config);

    MediaCtlConf *mc = PlatformData::getMediaCtlConf(cameraId);
    CheckError(!mc, config, "Invalid media control config.");

    for (const auto& output : mc->outputs) {
        if (output.port == port) {
            config.format  = output.v4l2Format;
            config.width   = output.width;
            config.height  = output.height;
            break;
        }
    }

    return config;
}

bool PlatformData::isAiqdEnabled(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableAiqd;
}

int PlatformData::getFormatByDevName(int cameraId, const string& devName, McFormat& format)
{
    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    CheckError(!mc, BAD_VALUE, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (auto &fmt : mc->formats) {
        if (fmt.formatType == FC_FORMAT && devName == fmt.entityName) {
            format = fmt;
            return OK;
        }
    }

    LOGE("Failed to find DevName for cameraId: %d, devname: %s", cameraId, devName.c_str());
    return BAD_VALUE;
}

int PlatformData::getVideoNodeNameByType(int cameraId, VideoNodeType videoNodeType, string& videoNodeName)
{
    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    CheckError(!mc, BAD_VALUE, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for(auto const& nd : mc->videoNodes) {
        if (videoNodeType == nd.videoNodeType) {
            videoNodeName = nd.name;
            return OK;
        }
    }

    LOGE("failed to find video note name for cameraId: %d", cameraId);
    return BAD_VALUE;
}

int PlatformData::getDevNameByType(int cameraId, VideoNodeType videoNodeType, string& devName)
{
    if (!isIsysEnabled(cameraId)) return OK;

    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    bool isSubDev = false;

    switch (videoNodeType) {
        case VIDEO_PIXEL_ARRAY:
        case VIDEO_PIXEL_BINNER:
        case VIDEO_PIXEL_SCALER:
        {
            isSubDev = true;
            // For sensor subdevices are fixed and sensor HW may be initialized before configure,
            // the first MediaCtlConf is used to find sensor subdevice name.
            PlatformData::StaticCfg::CameraInfo *pCam = &getInstance()->mStaticCfg.mCameras[cameraId];
            mc = &pCam->mMediaCtlConfs[0];
            break;
        }
        case VIDEO_ISYS_RECEIVER_BACKEND:
        case VIDEO_ISYS_RECEIVER:
        {
            isSubDev = true;
            break;
        }
        default:
            break;
    }

    CheckError(!mc, NAME_NOT_FOUND, "failed to get MediaCtlConf, videoNodeType %d", videoNodeType);

    for(auto& nd : mc->videoNodes) {
        if (videoNodeType == nd.videoNodeType) {
            string tmpDevName;
            CameraUtils::getDeviceName(nd.name.c_str(), tmpDevName, isSubDev);
            if (!tmpDevName.empty()) {
                devName = tmpDevName;
                LOG2("@%s, Found DevName. cameraId: %d, get video node: %s, devname: %s",
                      __func__, cameraId, nd.name.c_str(), devName.c_str());
                return OK;
            } else {
                // Use default device name if cannot find it
                if (isSubDev)
                    devName = "/dev/v4l-subdev1";
                else
                    devName = "/dev/video5";
                LOGE("Failed to find DevName for cameraId: %d, get video node: %s, devname: %s",
                      cameraId, nd.name.c_str(), devName.c_str());
                return NAME_NOT_FOUND;
            }
        }
    }

    LOG1("Failed to find devname for cameraId: %d, use default setting instead", cameraId);
    return NAME_NOT_FOUND;
}

/**
 * The ISYS best resolution is determined by the steps below:
 * 1. If the resolution is specified in MediaCtlConf, then use it.
 * 2. Try to find the exact matched one in ISYS supported resolutions.
 * 3. Try to find the same ratio resolution.
 * 4. If still couldn't get one, then use the biggest one.
 */
camera_resolution_t PlatformData::getISysBestResolution(int cameraId, int width,
                                                        int height, int field)
{
    LOG1("@%s, width:%d, height:%d", __func__, width, height);

    // Skip for interlace, we only support by-pass in interlaced mode
    if (field == V4L2_FIELD_ALTERNATE) {
        return {width, height};
    }

    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    // The isys output size is fixed if outputWidth/outputHeight != 0
    // So we use it to as the ISYS resolution.
    if (mc != nullptr && mc->outputWidth != 0 && mc->outputHeight != 0) {
        return {mc->outputWidth, mc->outputHeight};
    }

    const float RATIO_TOLERANCE = 0.05f; // Supported aspect ratios that are within RATIO_TOLERANCE
    const float kTargetRatio = (float)width / height;

    vector <camera_resolution_t> res;
    // The supported resolutions are saved in res with ascending order(small -> bigger)
    getSupportedISysSizes(cameraId, res);

    // Try to find out the same resolution in the supported isys resolution list
    // if it couldn't find out the same one, then use the bigger one which is the same ratio
    for (auto const& size : res) {
        if (width <= size.width && height <= size.height &&
            fabs((float)size.width/size.height - kTargetRatio) < RATIO_TOLERANCE) {
            LOG1("@%s: Found the best ISYS resoltoution (%d)x(%d)", __func__,
                 size.width, size.height);
            return {size.width, size.height};
        }
    }

    // If it still couldn't find one, then use the biggest one in the supported list.
    LOG1("@%s: ISYS resolution not found, used the biggest one: (%d)x(%d)",
         __func__, res.back().width, res.back().height);
    return {res.back().width, res.back().height};
}

bool PlatformData::isIsysEnabled(int cameraId)
{
    if (getInstance()->mStaticCfg.mCameras[cameraId].mMediaCtlConfs.empty()) {
        return false;
    }
    return true;
}

int PlatformData::calculateFrameParams(int cameraId, SensorFrameParams& sensorFrameParams)
{
    if (!isIsysEnabled(cameraId)) {
        LOG2("%s, no mc, just use default from xml", __func__);
        vector <camera_resolution_t> res;
        getSupportedISysSizes(cameraId, res);

        CheckError(res.empty(), BAD_VALUE, "Supported ISYS resolutions are not configured.");
        sensorFrameParams = {0, 0, static_cast<uint32_t>(res[0].width),
                             static_cast<uint32_t>(res[0].height), 1, 1, 1, 1};

        return OK;
    }

    CLEAR(sensorFrameParams);

    uint32_t width = 0;
    uint32_t horizontalOffset = 0;
    uint32_t horizontalBinNum = 1;
    uint32_t horizontalBinDenom = 1;
    uint32_t horizontalBin = 1;

    uint32_t height = 0;
    uint32_t verticalOffset = 0;
    uint32_t verticalBinNum = 1;
    uint32_t verticalBinDenom = 1;
    uint32_t verticalBin = 1;

    /**
     * For this function, it may be called without configuring stream
     * in some UT cases, the mc is nullptr at this moment. So we need to
     * get one default mc to calculate frame params.
     */
    MediaCtlConf *mc = PlatformData::getMediaCtlConf(cameraId);
    if (mc == nullptr) {
        PlatformData::StaticCfg::CameraInfo *pCam = &getInstance()->mStaticCfg.mCameras[cameraId];
        mc = &pCam->mMediaCtlConfs[0];
    }

    bool pixArraySizeFound = false;
    for (auto const& current : mc->formats) {
        if (!pixArraySizeFound && current.width > 0 && current.height > 0) {
            width = current.width;
            height = current.height;
            pixArraySizeFound = true;
            LOG2("%s: active pixel array H=%d, W=%d", __func__, height, width);
            //Setup initial sensor frame params.
            sensorFrameParams.horizontal_crop_offset += horizontalOffset;
            sensorFrameParams.vertical_crop_offset += verticalOffset;
            sensorFrameParams.cropped_image_width = width;
            sensorFrameParams.cropped_image_height = height;
            sensorFrameParams.horizontal_scaling_numerator = horizontalBinNum;
            sensorFrameParams.horizontal_scaling_denominator = horizontalBinDenom;
            sensorFrameParams.vertical_scaling_numerator = verticalBinNum;
            sensorFrameParams.vertical_scaling_denominator = verticalBinDenom;
        }

        if (current.formatType != FC_SELECTION) {
            continue;
        }

        if (current.selCmd == V4L2_SEL_TGT_CROP) {

            width = current.width * horizontalBin;
            horizontalOffset = current.left * horizontalBin;
            height = current.height * verticalBin;
            verticalOffset = current.top * verticalBin;

            LOG2("%s: crop (binning factor: hor/vert:%d,%d)"
                  , __func__, horizontalBin, verticalBin);

            LOG2("%s: crop left = %d, top = %d, width = %d height = %d",
                  __func__, horizontalOffset, verticalOffset, width, height);

        } else if (current.selCmd == V4L2_SEL_TGT_COMPOSE) {
            if (width == 0 || height == 0) {
                LOGE("Invalid XML configuration, no pixel array width/height when handling compose, skip.");
                return BAD_VALUE;
            }
            if (current.width == 0 || current.height == 0) {
                LOGW("%s: Invalid XML configuration for TGT_COMPOSE,"
                     "0 value detected in width or height", __func__);
                return BAD_VALUE;
            } else {
                LOG2("%s: Compose width %d/%d, height %d/%d", __func__, width, current.width,
                    height, current.height);
                // the scale factor should be float, so multiple numerator and denominator
                // with coefficient to indicate float factor
                const int SCALE_FACTOR_COEF = 10;
                horizontalBin = width / current.width;
                horizontalBinNum = width * SCALE_FACTOR_COEF / current.width;
                horizontalBinDenom = SCALE_FACTOR_COEF;
                verticalBin = height / current.height;
                verticalBinNum = height * SCALE_FACTOR_COEF / current.height;
                verticalBinDenom = SCALE_FACTOR_COEF;
            }

            LOG2("%s: COMPOSE horizontal bin factor=%d, (%d/%d)",
                  __func__, horizontalBin, horizontalBinNum, horizontalBinDenom);
            LOG2("%s: COMPOSE vertical bin factor=%d, (%d/%d)",
                  __func__, verticalBin, verticalBinNum, verticalBinDenom);
        } else {
            LOGW("%s: Target for selection is not CROP neither COMPOSE!", __func__);
            continue;
        }

        sensorFrameParams.horizontal_crop_offset += horizontalOffset;
        sensorFrameParams.vertical_crop_offset += verticalOffset;
        sensorFrameParams.cropped_image_width = width;
        sensorFrameParams.cropped_image_height = height;
        sensorFrameParams.horizontal_scaling_numerator = horizontalBinNum;
        sensorFrameParams.horizontal_scaling_denominator = horizontalBinDenom;
        sensorFrameParams.vertical_scaling_numerator = verticalBinNum;
        sensorFrameParams.vertical_scaling_denominator = verticalBinDenom;
    }

    return OK;

}

void PlatformData::getSupportedTuningConfig(int cameraId, vector <TuningConfig> &configs)
{
    configs = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig;
}

bool PlatformData::usePsys(int cameraId, int format)
{
    if (getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty()) {
        LOG1("@%s, the tuning config in xml does not exist", __func__);
        return false;
    }

    if (getInstance()->mStaticCfg.mCameras[cameraId].mPSysFormat.empty()) {
        LOG1("@%s, the psys supported format does not exist", __func__);
        return false;
    }

    for (auto &psys_fmt : getInstance()->mStaticCfg.mCameras[cameraId].mPSysFormat) {
        if (format == psys_fmt)
            return true;
    }

    LOGW("%s, No matched format found, but expected format:%s", __func__,
        CameraUtils::pixelCode2String(format));

    return false;
}

int PlatformData::getConfigModesByOperationMode(int cameraId, uint32_t operationMode, vector <ConfigMode> &configModes)
{
    if (operationMode == CAMERA_STREAM_CONFIGURATION_MODE_END) {
        LOG2("%s: operationMode was invalid operation mode", __func__);
        return INVALID_OPERATION;
    }

    CheckError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(), INVALID_OPERATION,
          "@%s, the tuning config in xml does not exist", __func__);

    if (operationMode == CAMERA_STREAM_CONFIGURATION_MODE_AUTO) {
        if (getInstance()->mStaticCfg.mCameras[cameraId].mConfigModesForAuto.empty()) {
            // Use the first config mode as default for auto
            configModes.push_back(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig[0].configMode);
            LOG2("%s: add config mode %d for operation mode %d", __func__, configModes[0], operationMode);
        } else {
            configModes = getInstance()->mStaticCfg.mCameras[cameraId].mConfigModesForAuto;
        }
    } else {
        for (auto &cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
            if (operationMode == (uint32_t)cfg.configMode) {
                configModes.push_back(cfg.configMode);
                LOG2("%s: add config mode %d for operation mode %d", __func__, cfg.configMode, operationMode);
            }
        }
    }

    if (configModes.size() > 0) return OK;
    LOGW("%s, configure number %zu, operationMode %x, cameraId %d", __func__,
            configModes.size(), operationMode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getTuningModeByConfigMode(int cameraId, ConfigMode configMode,
                                            TuningMode& tuningMode)
{
    CheckError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
          INVALID_OPERATION, "the tuning config in xml does not exist");

    for (auto &cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
        LOG2("%s, tuningMode %d, configMode %x", __func__, cfg.tuningMode, cfg.configMode);
        if (cfg.configMode == configMode) {
            tuningMode = cfg.tuningMode;
            return OK;
        }
    }

    LOGW("%s, configMode %x, cameraId %d, no tuningModes", __func__, configMode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getTuningConfigByConfigMode(int cameraId, ConfigMode mode, TuningConfig &config)
{
    CheckError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(), INVALID_OPERATION,
          "@%s, the tuning config in xml does not exist.", __func__);

    for (auto &cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
        if (cfg.configMode == mode) {
            config = cfg;
            return OK;
        }
    }

    LOGW("%s, configMode %x, cameraId %d, no TuningConfig", __func__, mode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getStreamIdByConfigMode(int cameraId, ConfigMode configMode)
{
    std::map<int, int> modeMap = getInstance()->mStaticCfg.mCameras[cameraId].mConfigModeToStreamId;
    return modeMap.find(configMode) == modeMap.end() ? -1 : modeMap[configMode];
}

int PlatformData::getMaxRequestsInflight(int cameraId)
{
    int inflight = getInstance()->mStaticCfg.mCameras[cameraId].mMaxRequestsInflight;
    if (inflight <= 0) {
        inflight = isEnableAIQ(cameraId) ? 4 : MAX_BUFFER_COUNT;
    }

    return inflight;
}

bool PlatformData::getGraphConfigNodes(int cameraId)
{
    return !(getInstance()->mStaticCfg.mCameras[cameraId].mGraphSettingsFile.empty());
}

GraphSettingType PlatformData::getGraphSettingsType(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mGraphSettingsType;
}

camera_yuv_color_range_mode_t PlatformData::getYuvColorRangeMode(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mYuvColorRangeMode;
}

ia_binary_data* PlatformData::getAiqd(int cameraId, TuningMode mode)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
               "@%s, bad cameraId:%d", __func__, cameraId);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    return aiqInitData->getAiqd(mode);
}

void PlatformData::saveAiqd(int cameraId, TuningMode tuningMode, const ia_binary_data& data)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
               "@%s, bad cameraId:%d", __func__, cameraId);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    aiqInitData->saveAiqd(tuningMode, data);
}

// load cpf when tuning file (.aiqb) is available
int PlatformData::getCpfAndCmc(int cameraId,
                               ia_binary_data* ispData,
                               ia_binary_data* aiqData,
                               ia_binary_data* otherData,
                               uintptr_t* cmcHandle,
                               TuningMode mode,
                               ia_cmc_t** cmcData)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()) ||
               cameraId >= MAX_CAMERA_NUMBER, BAD_VALUE, "@%s, bad cameraId:%d",
               __func__, cameraId);
    CheckError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
               INVALID_OPERATION, "@%s, the tuning config in xml does not exist", __func__);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    return aiqInitData->getCpfAndCmc(ispData, aiqData, otherData, cmcHandle, mode, cmcData);
}

bool PlatformData::isCSIBackEndCapture(int cameraId)
{
    bool isCsiBECapture = false;
    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    CheckError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for(const auto& node : mc->videoNodes) {
        if (node.videoNodeType == VIDEO_GENERIC &&
                (node.name.find("BE capture") != string::npos ||
                 node.name.find("BE SOC capture") != string::npos)) {
            isCsiBECapture = true;
            break;
        }
    }

    return isCsiBECapture;
}

bool PlatformData::isCSIFrontEndCapture(int cameraId)
{
    bool isCsiFeCapture = false;
    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    CheckError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for(const auto& node : mc->videoNodes) {
        if (node.videoNodeType == VIDEO_GENERIC &&
                (node.name.find("CSI-2") != string::npos ||
                 node.name.find("TPG") != string::npos)) {
            isCsiFeCapture = true;
            break;
        }
    }
    return isCsiFeCapture;
}

bool PlatformData::isTPGReceiver(int cameraId)
{
    bool isTPGCapture = false;
    MediaCtlConf *mc = getMediaCtlConf(cameraId);
    CheckError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for(const auto& node : mc->videoNodes) {
        if (node.videoNodeType == VIDEO_ISYS_RECEIVER &&
                (node.name.find("TPG") != string::npos)) {
            isTPGCapture = true;
            break;
        }
    }
    return isTPGCapture;
}

int PlatformData::getSupportAeExposureTimeRange(int cameraId, camera_scene_mode_t sceneMode,
                                                camera_range_t& etRange)
{
    vector<camera_ae_exposure_time_range_t> ranges;
    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSupportedAeExposureTimeRange(ranges);

    if (ranges.empty())
        return NAME_NOT_FOUND;

    for (auto& item : ranges) {
        if (item.scene_mode == sceneMode) {
            etRange = item.et_range;
            return OK;
        }
    }
    return NAME_NOT_FOUND;
}

int PlatformData::getSupportAeGainRange(int cameraId, camera_scene_mode_t sceneMode,
                                        camera_range_t& gainRange)
{
    vector<camera_ae_gain_range_t> ranges;
    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSupportedAeGainRange(ranges);

    if(ranges.empty()) {
        return NAME_NOT_FOUND;
    }

    for (auto& item : ranges) {
        if (item.scene_mode == sceneMode) {
            gainRange = item.gain_range;
            return OK;
        }
    }
    return NAME_NOT_FOUND;
}

bool PlatformData::isUsingCrlModule(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseCrlModule;
}

vector<MultiExpRange> PlatformData::getMultiExpRanges(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mMultiExpRanges;
}

camera_resolution_t *PlatformData::getPslOutputForRotation(int width, int height, int cameraId)
{
    CheckError(getInstance()->mStaticCfg.mCameras[cameraId].mOutputMap.empty(), nullptr,
          "@%s, cameraId: %d, there isn't pslOutputMapForRotation field in xml.", __func__, cameraId);

    vector<UserToPslOutputMap> &outputMap = getInstance()->mStaticCfg.mCameras[cameraId].mOutputMap;
    for (auto & map : outputMap) {
        if (width == map.User.width && height == map.User.height) {
            LOG2("cameraId: %d, find the psl output resoltion(%d, %d) for %dx%d",
                  cameraId, map.Psl.width, map.Psl.height, map.User.width, map.User.height);
            return &map.Psl;
        }
    }

    return nullptr;
}

bool PlatformData::isTestPatternSupported(int cameraId)
{
    return !getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap.empty();
}

int32_t PlatformData::getSensorTestPattern(int cameraId, int32_t mode)
{
    CheckError(getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap.empty(), -1,
          "@%s, cameraId: %d, mTestPatternMap is empty!", __func__, cameraId);
    auto testPatternMap = getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap;

    if (testPatternMap.find(mode) == testPatternMap.end()) {
        LOGW("Test pattern %d wasn't found in configuration file, return -1", mode);
        return -1;
    }
    return testPatternMap[mode];
}

ia_binary_data *PlatformData::getNvm(int cameraId)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
               "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->getNvm();
}

camera_coordinate_system_t PlatformData::getActivePixelArray(int cameraId)
{
    camera_coordinate_system_t arraySize;
    CLEAR(arraySize);

    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSensorActiveArraySize(arraySize);

    return {arraySize.left, arraySize.top, arraySize.right, arraySize.bottom};
}

string PlatformData::getCameraCfgPath()
{
    char* p = getenv("CAMERA_CFG_PATH");

    return p? string(p) : string(CAMERA_DEFAULT_CFG_PATH);
}

string PlatformData::getGraphDescFilePath()
{
    return PlatformData::getCameraCfgPath() + string(CAMERA_GRAPH_DESCRIPTOR_FILE);
}

string PlatformData::getGraphSettingFilePath()
{
    return PlatformData::getCameraCfgPath() + string(CAMERA_GRAPH_SETTINGS_DIR);
}

int PlatformData::getSensorDigitalGain(int cameraId, float realDigitalGain)
{
    int sensorDg = 0;
    int maxSensorDg = PlatformData::getMaxSensorDigitalGain(cameraId);

    if (PlatformData::sensorDigitalGainType(cameraId) == SENSOR_DG_TYPE_2_X) {
        int index = 0;
        while (pow(2, index) <= realDigitalGain) {
            sensorDg = index;
            index++;
        }
        sensorDg = CLIP(sensorDg, maxSensorDg, 0);
    } else {
        LOGE("%s, don't support the sensor digital gain type: %d",
                __func__, PlatformData::sensorDigitalGainType(cameraId));
    }

    return sensorDg;
}

float PlatformData::getIspDigitalGain(int cameraId, float realDigitalGain)
{
    float ispDg = 1.0f;
    int sensorDg = getSensorDigitalGain(cameraId, realDigitalGain);

    if (PlatformData::sensorDigitalGainType(cameraId) == SENSOR_DG_TYPE_2_X) {
        ispDg = realDigitalGain / pow(2, sensorDg);
        ispDg = CLIP(ispDg, ispDg, 1.0);
    } else {
        LOGE("%s, don't support the sensor digital gain type: %d",
                __func__, PlatformData::sensorDigitalGainType(cameraId));
    }

    return ispDg;
}

int PlatformData::saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode,
                                    int64_t sequence)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), BAD_VALUE,
               "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->saveMakernoteData(makernoteMode, sequence);
}

void* PlatformData::getMknHandle(int cameraId)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
               "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->getMknHandle();
}

void PlatformData::updateMakernoteTimeStamp(int cameraId, int64_t sequence, uint64_t timestamp)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
               "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->updateMakernoteTimeStamp(sequence, timestamp);
}

void  PlatformData::acquireMakernoteData(int cameraId, uint64_t timestamp, Parameters *param)
{
    CheckError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
               "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->acquireMakernoteData(timestamp, param);
}

int PlatformData::getScalerInfo(int cameraId, int32_t streamId,
                                float *scalerWidth, float *scalerHeight)
{
    if (getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo.empty()) {
        *scalerWidth = 1.0;
        *scalerHeight = 1.0;
        return OK;
    }

    for (auto &scalerInfo : getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo) {
        LOG2("%s, streamId %d, scalerWidth %f, scalerHeight %f", __func__, scalerInfo.streamId,
             scalerInfo.scalerWidth, scalerInfo.scalerHeight);
        if (scalerInfo.streamId == streamId) {
            *scalerWidth = scalerInfo.scalerWidth;
            *scalerHeight = scalerInfo.scalerHeight;
            break;
        }
    }

    return OK;
}

void  PlatformData::setScalerInfo(int cameraId, std::vector<IGraphType::ScalerInfo> scalerInfo)
{
    for (auto &scalerInfoInput : scalerInfo) {
        bool flag = false;
        for (auto &scalerInfoTmp : getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo) {
            if (scalerInfoInput.streamId == scalerInfoTmp.streamId) {
                scalerInfoTmp.scalerWidth = scalerInfoInput.scalerWidth;
                scalerInfoTmp.scalerHeight = scalerInfoInput.scalerHeight;
                flag = true;
                break;
            }
        }
        if (!flag) {
            getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo.push_back(scalerInfoInput);
        }
    }
}

bool PlatformData::isGpuTnrEnabled()
{
    return getInstance()->mStaticCfg.mCommonConfig.isGpuTnrEnabled;
}

int PlatformData::getVideoStreamNum(int cameraId)
{
    return getInstance()->mStaticCfg.mCameras[cameraId].mVideoStreamNum;
}

bool PlatformData::isUsingGpuAlgo()
{
    bool enabled = false;
    enabled |= isGpuTnrEnabled();
    return enabled;
}

bool PlatformData::isStillTnrPrior()
{
    return getInstance()->mStaticCfg.mCommonConfig.isStillTnrPrior;
}
} // namespace icamera
