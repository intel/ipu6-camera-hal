/*
 * Copyright (C) 2017-2018 Intel Corporation.
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

#include "iutils/RWLock.h"
#include "CameraMetadata.h"

namespace icamera {

class Parameters;

/**
 * \class ParameterHelper
 *
 * There are two main purposes of this class:
 * 1. Provide some wrapper functions for the implementation of Parameters.
 * 2. Provide some interface for HAL to access and modify Parameters internal data.
 */
class ParameterHelper {
public:
    /**
     * \brief Merge and update dst parameter buffer with another parameter instance.
     *
     * \param[in] Parameters src: the source parameter.
     * \param[out] Parameters dst: the parameter to be updated.
     *
     * \return void
     */
    static void merge(const Parameters& src, Parameters* dst);

    /**
     * \brief Merge and update dst parameter buffer by using metadata.
     *
     * \param[in] CameraMetadata metadata: the source metadata.
     * \param[out] Parameters dst: the parameter to be updated.
     *
     * \return void
     */
    static void merge(const CameraMetadata& metadata, Parameters* dst);

    /**
     * \brief Copy metadata from parameter buffer.
     *
     * Some HAL V3 style parameters are not returned in the getParameter()
     * because it is not used inside the HAL.
     * Provide this function to assist upperlayer to return metadata
     * to applications.
     *
     * \param[in] Parameters source: the parameter to provide metadata.
     * \param[out] CameraMetadata metadata: the metadata to be updated.
     *
     * \return void
     */
    static void copyMetadata(const Parameters& source, CameraMetadata* metadata);

private:
    // The definitions and interfaces in this private section are only for Parameters internal
    // use, HAL other code shouldn't and cannot access them.
    friend class Parameters;

    /**
     * \class ParameterData
     *
     * \brief The definition of Parameters' internal data structure used to hide implementation
     *        details of Parameters.
     */
    class ParameterData {
    public:
        ParameterData() {}
        ~ParameterData() {}

        ParameterData(const ParameterData& other) : mMetadata(other.mMetadata) {}
        ParameterData& operator=(const ParameterData& other) {
            mMetadata = other.mMetadata;
            return *this;
        }

        CameraMetadata mMetadata; // The data structure to save all of the parameters.
        RWLock mRwLock;           // Read-write lock to make Parameters class thread-safe
    };

    // Customized wrappers of RWLock to make the implementation of Parameters much cleaner.
    class AutoRLock {
    public:
        AutoRLock(void* data) : mLock(getInternalData(data).mRwLock) { mLock.readLock(); }
        ~AutoRLock() { mLock.unlock(); }
    private:
        RWLock& mLock;
    };

    class AutoWLock {
    public:
        AutoWLock(void* data) : mLock(getInternalData(data).mRwLock) { mLock.writeLock(); }
        ~AutoWLock() { mLock.unlock(); }
    private:
        RWLock& mLock;
    };

    static ParameterData& getInternalData(void* data) {
        return *reinterpret_cast<ParameterData*>(data);
    }

    static void* createParameterData() {
        return new ParameterData();
    }

    static void* createParameterData(void* data) {
        return new ParameterData(getInternalData(data));
    }

    static void releaseParameterData(void* data) {
        delete &getInternalData(data);
    }

    static void deepCopy(void* srcData, void* dstData) {
        getInternalData(dstData) = getInternalData(srcData);
    }

    static CameraMetadata& getMetadata(void* data) {
        return getInternalData(data).mMetadata;
    }

    static icamera_metadata_ro_entry_t getMetadataEntry(void* data, uint32_t tag) {
        return const_cast<const CameraMetadata*>(&getMetadata(data))->find(tag);
    }
};

} // namespace icamera
