/*
 * Copyright (C) 2018-2020 Intel Corporation.
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
 *\File PolicyParser.h
 *
 * parser for the policy xml configuration file
 *
 * This file calls the libexpat ditectly. The libexpat is one xml parser.
 * It will parse the camera configuration out firstly.
 * Then other module can call the methods of it to get the real configuration.
 */

#pragma once

#include "iutils/Utils.h"

#include "CameraTypes.h"
#include "PlatformData.h"
#include "ParserBase.h"

namespace icamera {

/**
 * \class PolicyParser
 *
 * This class is used to parse the policy configuration file.
 * The configuration file is xml format.
 * This class will use the expat lib to do the xml parser.
 */
class PolicyParser : public ParserBase {
public:
    PolicyParser(PlatformData::StaticCfg *cfg);
    ~PolicyParser(){}

    void startParseElement(void *userData, const char *name, const char **atts);
    void endParseElement(void *userData, const char *name);

private:
    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(PolicyParser);

private:
    void checkField(PolicyParser *profiles, const char *name, const char **atts);
    void handlePolicyConfig(PolicyParser *profiles, const char *name, const char **atts);
    void handlePipeExecutor(PolicyParser *profiles, const char *name, const char **atts);
    void handleExclusivePGs(PolicyParser *profiles, const char *name, const char **atts);
    void handleBundles(PolicyParser *profiles, const char *name, const char **atts);
    void handleShareReferPair(PolicyParser *profiles, const char *name, const char **atts);

private:
    PlatformData::StaticCfg *mStaticCfg;

    enum DataField {
        FIELD_INVALID = 0,
        FIELD_GRAPH,
    } mCurrentDataField;
    PolicyConfig *pCurrentConf;
};

} // namespace icamera
