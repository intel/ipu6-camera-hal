/*
 * Copyright (C) 2019-2020 Intel Corporation
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
#define LOG_TAG "ParserBase"

#include <memory>
#include <expat.h>
#include <string.h>

#include "ParserBase.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

ParserBase::ParserBase()
{
    LOGXML("@%s", __func__);
}

const char *ParserBase::skipWhiteSpace(const char *src)
{
    while (*src == '\n' || *src == '\t' || *src == ' ' || *src == '\v' || *src == '\r' || *src == '\f') {
        src++;
    }
    return src;
}

int ParserBase::parseXmlParameterToChar(const char *str, unsigned char *table)
{
    CheckError(str == nullptr, -1, "@%s, str is nullptr", __func__);

    int index = 0;
    char *savePtr, *tablePtr;
    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';

    tablePtr = strtok_r(src, ",", &savePtr);
    while (tablePtr) {
        table[index] = atoi(tablePtr);
        if (savePtr != nullptr)
            savePtr = const_cast<char *>(skipWhiteSpace(savePtr));
        index++;
        tablePtr = strtok_r(nullptr, ",", &savePtr);
    }
    return 0;
}

/* template function need the function ptr */
std::string ParserBase::convertCharToString(const char *str)
{
    return str;
}

void ParserBase::startElement(void *userData, const char *name, const char **atts)
{
    ParserBaseCallBack *profiles = reinterpret_cast<ParserBaseCallBack*>(userData);
    profiles->startParseElement(userData, name, atts);
}

void ParserBase::endElement(void *userData, const char *name)
{
    ParserBaseCallBack *profiles = reinterpret_cast<ParserBaseCallBack*>(userData);
    profiles->endParseElement(userData, name);
}

int ParserBase::parseXmlFile(const std::string &xmlFile)
{
    int ret = UNKNOWN_ERROR;
    int done;
    FILE *fp = nullptr;
    int bufSize = 4 * 1024;  // parse 4k data every time

    CheckError(xmlFile.empty(), UNKNOWN_ERROR, "xmlFile is empty");

    LOGXML("@%s, parsing profile: %s", __func__, xmlFile.c_str());

    fp = ::fopen(xmlFile.c_str(), "r");
    CheckError(nullptr == fp, UNKNOWN_ERROR, "@%s, line:%d, Can not open profile file %s in read mode, fp is nullptr",
          __func__, __LINE__, xmlFile.c_str());

    std::unique_ptr<char[]>pBuf(new char[bufSize]);
    XML_Parser parser = ::XML_ParserCreate(nullptr);
    if (nullptr == parser) {
        LOGE("@%s, line:%d, parser is nullptr", __func__, __LINE__);
        goto exit;
    }

    ::XML_SetUserData(parser, this);
    ::XML_SetElementHandler(parser, startElement, endElement);

    do {
        int len = (int)::fread(pBuf.get(), 1, bufSize, fp);
        if (!len) {
            if (ferror(fp)) {
                clearerr(fp);
                goto exit;
            }
        }
        done = len < bufSize;
        if (XML_Parse(parser, (const char *)pBuf.get(), len, done) == XML_STATUS_ERROR) {
            LOGE("@%s, line:%d, XML_Parse error", __func__, __LINE__);
            goto exit;
        }
    } while (!done);
    ret = OK;

exit:
    if (parser)
        ::XML_ParserFree(parser);
    if (fp)
    ::fclose(fp);

    return ret;
}

void ParserBase::getAvaliableXmlFile(const std::vector<const char *> &avaliableXmlFiles,
                                     std::string &xmlFile)
{
    struct stat st;
    for (auto xml : avaliableXmlFiles) {
        int ret = stat(xml, &st);
        if (ret == 0) {
            xmlFile = xml;
            return;
        }
    }
}

int ParserBase::getDataFromXmlFile(std::string fileName)
{
    LOGXML("@%s", __func__);
    CheckError(fileName.size() == 0, UNKNOWN_ERROR, "file name is null");

    std::string curFolderFileName = std::string("./") + fileName;
    std::string sysFolderFileName = PlatformData::getCameraCfgPath() + fileName;
    const std::vector <const char *> profiles = {
        curFolderFileName.c_str(),
        sysFolderFileName.c_str()
    };

    std::string chosenXmlFile;
    getAvaliableXmlFile(profiles, chosenXmlFile);
    CheckError(chosenXmlFile.empty(), UNKNOWN_ERROR, "%s is not found in: %s or %s",
          fileName.c_str(), curFolderFileName.c_str(), sysFolderFileName.c_str());

    return parseXmlFile(chosenXmlFile);
}

} //namespace icamera
