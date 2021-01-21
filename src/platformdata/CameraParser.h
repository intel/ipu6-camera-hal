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

/**
 *\file CameraParser.h
 *
 * parser for the camera xml configuration file
 *
 * This file calls the libexpat ditectly. The libexpat is one xml parser.
 * It will parse the camera configuration out firstly.
 * Then other module can call the methods of it to get the real configuration.
 *
 */

#pragma once

#include <unordered_map>
#include "PlatformData.h"
#include "ParserBase.h"
#include "CameraMetadata.h"

namespace icamera {

/**
 * \class CameraParser
 *
 * This class is used to parse the camera configuration file.
 * The configuration file is xml format.
 * This class will use the expat lib to do the xml parser.
 */
class CameraParser : public ParserBase {
public:
    CameraParser(MediaControl *mc, PlatformData::StaticCfg *cfg);
    ~CameraParser();

    unsigned getSensorNum(void) {return mSensorNum;};

private:
    DISALLOW_COPY_AND_ASSIGN(CameraParser);

private:
    PlatformData::StaticCfg *mStaticCfg;

    enum DataField {
        FIELD_INVALID = 0,
        FIELD_SENSOR,
        FIELD_COMMON
    } mCurrentDataField;
    int mSensorNum;
    int mCurrentSensor;
    std::string mI2CBus;
    std::string mCsiPort;
    std::unordered_map<std::string, std::string> mAvailableSensor;
    PlatformData::StaticCfg::CameraInfo *pCurrentCam;
    bool mInMediaCtlCfg;
    bool mInStaticMetadata;
    MediaControl* mMC;
    CameraMetadata mMetadata;
    std::string mCameraModuleName;
    CameraMetadata mCameraModuleMetadata;

    long* mMetadataCache;
    static const int mMetadataCacheSize = 4096;

    std::unordered_map<std::string, enum icamera_metadata_tag> mGenericStaticMetadataToTag;

    void startParseElement(void *userData, const char *name, const char **atts);
    void endParseElement(void *userData, const char *name);

    static void parseSizesList(const char *sizesStr, std::vector <camera_resolution_t> &sizes);
    static int getSupportedFormat(const char* str, std::vector <int>& supportedFormat);
    static int parsePair(const char *str, int *first, int *second, char delim, char **endptr = nullptr);

    std::vector<std::string> getAvailableSensors(const std::string &ipuName,
                                                 const std::vector<std::string> &sensorsList);
    void getSensorDataFromXmlFile(void);
    void checkField(CameraParser *profiles, const char *name, const char **atts);

    void handleSensor(CameraParser *profiles, const char *name, const char **atts);
    void handleCommon(CameraParser *profiles, const char *name, const char **atts);

    void parseStreamConfig(const char* src, stream_array_t& configs);
    void parseSupportedFeatures(const char* src, camera_features_list_t& features);
    void parseSupportedIspControls(const char* src, std::vector<uint32_t>& features);
    int parseSupportedVideoStabilizationMode(const char* str, camera_video_stabilization_list_t &supportedModes);
    int parseSupportedAeMode(const char* str, std::vector <camera_ae_mode_t> &supportedModes);
    int parseSupportedAfMode(const char* str, std::vector <camera_af_mode_t> &supportedModes);
    int parseSupportedAntibandingMode(const char* str, std::vector <camera_antibanding_mode_t> &supportedModes);
    int parseSupportedAeParamRange(const char* src, std::vector<int>& scenes,
                                   std::vector<float>& minValues, std::vector<float>& maxValues);

// parse the media controller configuration in xml, the MediaControl MUST be run before the parser to run.
    void handleMediaCtlCfg(CameraParser *profiles, const char *name, const char **atts);
    void handleStaticMetaData(CameraParser *profiles, const char *name, const char **atts);
    void handleGenericStaticMetaData(const char* name, const char* src, CameraMetadata* metadata);
    void parseMediaCtlConfigElement(CameraParser *profiles, const char *name, const char **atts);
    void storeMcMappForConfig(int mcId, stream_t streamCfg);
    void parseLinkElement(CameraParser *profiles, const char *name, const char **atts);
    void parseRouteElement(CameraParser *profiles, const char *name, const char **atts);
    void parseControlElement(CameraParser *profiles, const char *name, const char **atts);
    void parseSelectionElement(CameraParser *profiles, const char *name, const char **atts);
    void parseFormatElement(CameraParser *profiles, const char *name, const char **atts);
    void parseVideoElement(CameraParser *profiles, const char *name, const char **atts);
    void parseOutputElement(CameraParser *profiles, const char *name, const char **atts);
    void parseMultiExpRange(const char* src);

    int parseSupportedTuningConfig(const char *str, std::vector <TuningConfig> &config);
    int parseLardTags(const char *str, std::vector <LardTagConfig> &lardTags);

    void dumpSensorInfo(void);

    void parseOutputMap(const char *str, std::vector<UserToPslOutputMap> &outputMap);

    std::string replaceStringInXml(CameraParser *profiles, const char *value);
};

} // namespace icamera
