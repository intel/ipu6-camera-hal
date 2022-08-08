/*
 * Copyright (C) 2016-2021 Intel Corporation.
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
 *\file TunningParser.h
 *
 * parser for the camera xml configuration file
 *
 * This file calls the libexpat ditectly. The libexpat is one xml parser.
 * It will parse the camera configuration out firstly.
 * Then other module can call the methods of it to get the real configuration.
 *
 */

#pragma once

#include "ParserBase.h"
#include "PlatformData.h"

namespace icamera {

/**
 * \class TunningParser
 *
 * This class is used to parse the camera configuration file.
 * The configuration file is xml format.
 * This class will use the expat lib to do the xml parser.
 */
class TunningParser : public ParserBase {
 public:
    explicit TunningParser(PlatformData::StaticCfg* cfg);
    ~TunningParser() {}

    void startParseElement(void* userData, const char* name, const char** atts);
    void endParseElement(void* userData, const char* name);

 private:
    PlatformData::StaticCfg* mStaticCfg;

    enum DataField { FIELD_INVALID = 0, FIELD_SENSOR, FIELD_COMMON } mCurrentDataField;
    PlatformData::StaticCfg::CameraInfo* mCurrentCam;

    void getCameraInfoByName(TunningParser* profiles, const char* name);
    void checkField(TunningParser* profiles, const char* name, const char** atts);

    void handleSensor(TunningParser* profiles, const char* name, const char** atts);
    void handleCommon(TunningParser* profiles, const char* name, const char** atts);

    void handleWeightGrid(TunningParser* profiles, const char* name, const char** atts);

 private:
    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(TunningParser);
};

}  // namespace icamera
