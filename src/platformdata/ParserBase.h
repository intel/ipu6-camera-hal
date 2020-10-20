/*
 * Copyright (C) 2019-2020 Intel Corporation.
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
 *\file ParserBase.h
 *
 * parser for the camera xml configuration file, this is a basic class.
 *
 * This file calls the libexpat ditectly. The libexpat is one xml parser.
 * It will parse the camera configuration out firstly.
 * Then other module can call the methods of it to get the real configuration.
 *
 */

#pragma once

#include "PlatformData.h"

namespace icamera {

class ParserBaseCallBack {
public:
    virtual ~ParserBaseCallBack() {}
    virtual void startParseElement(void *userData, const char *name, const char **atts){};
    virtual void endParseElement(void *userData, const char *name){};
};

class ParserBase : public ParserBaseCallBack {
public:
    ParserBase();
    virtual ~ParserBase() {}

private:
    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(ParserBase);

protected:
    /**
     * Get an avaliable xml file
     *
     * Find the first avaliable xml file.
     *
     * \param[in] const vector<char *>& allAvaliableXmlFiles: all avaliable xml files list.
     * \param[out] string& xmlFile: to store a avaliable xml file
     */
    void getAvaliableXmlFile(const std::vector<const char *> &avaliableXmlFiles,
                             std::string &xmlFile);

    /**
     * Get camera configuration from xml file
     *
     * The function will read the xml configuration file firstly.
     * Then it will parse out the camera settings.
     * The camera setting is stored inside this CameraProfiles class.
     */
    int getDataFromXmlFile(std::string fileName);

    int parseXmlFile(const std::string &xmlFile);
    const char* skipWhiteSpace(const char *src);
    int parseXmlParameterToChar(const char *str, unsigned char *table);

    static void startElement(void *userData, const char *name, const char **atts);
    static void endElement(void *userData, const char *name);
    static std::string convertCharToString(const char *str);

    template<typename T>
    int parseXmlConvertStrings(const char *str, std::vector<T> &vectorT,
                               T (*parseXmlString)(const char *)) {
        CheckError(str == nullptr || parseXmlString == nullptr, -1, "@%s, input parameter is nullptr", __func__);

        int sz = strlen(str);
        char src[sz + 1];
        MEMCPY_S(src, sz, str, sz);
        src[sz] = '\0';

        char *savePtr = nullptr;
        char *cfgName = strtok_r(src, ",", &savePtr);
        while(cfgName) {
            vectorT.push_back(parseXmlString(cfgName));
            if (savePtr != nullptr)
                savePtr = const_cast<char*>(skipWhiteSpace(savePtr));
            cfgName = strtok_r(nullptr, ",", &savePtr);
        }

        return 0;
    }
};

} // namespace icamera
