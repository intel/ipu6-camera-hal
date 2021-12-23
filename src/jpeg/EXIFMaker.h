/*
 * Copyright (C) 2016-2020 Intel Corporation
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

#include <string>

#include "EXIFMetaData.h"
#include "ExifCreater.h"
#include "iutils/Utils.h"

namespace icamera {

/**
 * \class EXIFMaker
 *
 */
class EXIFMaker {
 public:
    EXIFMaker();
    ~EXIFMaker();

    void readProperty();
    void initialize(int width, int height);
    bool isInitialized() { return mInitialized; }
    void initializeLocation(ExifMetaData* metadata);
    uint32_t getMakerNoteDataSize() const;
    void pictureTaken(ExifMetaData* exifmetadata);
    void enableFlash(bool enable, int8_t aeMode, int8_t flashMode);
    void setThumbnail(unsigned char* data, size_t size, int width, int height);
    bool isThumbnailSet() const;
    size_t makeExif(unsigned char* data);
    void setMaker(const char* data);
    void setModel(const char* data);
    void setSoftware(const char* data);
    void updateSensorInfo(const Parameters& params);
    void saveMakernote(const Parameters& params);
    void setSensorAeConfig(const Parameters& params);

 private:  // member variables
    ExifCreater mEncoder;
    exif_attribute_t mExifAttributes;
    size_t mExifSize;
    bool mInitialized;
    unsigned char* mMakernoteSection;
    std::string mProductName;
    std::string mManufacturerName;

 private:
    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(EXIFMaker);

 private:  // Methods
    void copyAttribute(uint8_t* dst, size_t dstSize, const char* src, size_t srcLength);

    void clear();
};

}  // namespace icamera
