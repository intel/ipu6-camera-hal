/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (C) 2015-2020 Intel Corporation
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

#define LOG_TAG "CameraMetadata"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#include "CameraMetadata.h"

namespace icamera {

CameraMetadata::CameraMetadata() :
        mBuffer(NULL), mLocked(false) {
}

CameraMetadata::CameraMetadata(size_t entryCapacity, size_t dataCapacity) :
        mLocked(false)
{
    mBuffer = allocate_icamera_metadata(entryCapacity, dataCapacity);
}

CameraMetadata::CameraMetadata(const CameraMetadata &other) :
        mLocked(false) {
    mBuffer = clone_icamera_metadata(other.mBuffer);
}

CameraMetadata::CameraMetadata(icamera_metadata_t *buffer) :
        mBuffer(NULL), mLocked(false) {
    acquire(buffer);
}

CameraMetadata &CameraMetadata::operator=(const CameraMetadata &other) {
    return operator=(other.mBuffer);
}

CameraMetadata &CameraMetadata::operator=(const icamera_metadata_t *buffer) {
    if (mLocked) {
        LOGE("%s: Assignment to a locked CameraMetadata!", __func__);
        return *this;
    }

    if (buffer != mBuffer) {
        icamera_metadata_t *newBuffer = clone_icamera_metadata(buffer);
        clear();
        mBuffer = newBuffer;
    }
    return *this;
}

CameraMetadata::~CameraMetadata() {
    mLocked = false;
    clear();
}

const icamera_metadata_t* CameraMetadata::getAndLock() {
    mLocked = true;
    return mBuffer;
}

status_t CameraMetadata::unlock(const icamera_metadata_t *buffer) {
    if (!mLocked) {
        LOGE("%s: Can't unlock a non-locked CameraMetadata!", __func__);
        return INVALID_OPERATION;
    }
    if (buffer != mBuffer) {
        LOGE("%s: Can't unlock CameraMetadata with wrong pointer!",
                __func__);
        return BAD_VALUE;
    }
    mLocked = false;
    return OK;
}

icamera_metadata_t* CameraMetadata::release() {
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return NULL;
    }
    icamera_metadata_t *released = mBuffer;
    mBuffer = NULL;
    return released;
}

void CameraMetadata::clear() {
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return;
    }
    if (mBuffer) {
        free_icamera_metadata(mBuffer);
        mBuffer = NULL;
    }
}

void CameraMetadata::acquire(icamera_metadata_t *buffer) {
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return;
    }
    clear();
    mBuffer = buffer;

    if (validate_icamera_metadata_structure(mBuffer, /*size*/NULL) != OK) {
        LOGE("%s: Failed to validate metadata structure %p",
                 __func__, buffer);
    }
}

void CameraMetadata::acquire(CameraMetadata &other) {
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return;
    }
    acquire(other.release());
}

status_t CameraMetadata::append(const CameraMetadata &other) {
    return append(other.mBuffer);
}

status_t CameraMetadata::append(const icamera_metadata_t* other) {
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    size_t extraEntries = get_icamera_metadata_entry_count(other);
    size_t extraData = get_icamera_metadata_data_count(other);
    resizeIfNeeded(extraEntries, extraData);

    return append_icamera_metadata(mBuffer, other);
}

size_t CameraMetadata::entryCount() const {
    return (mBuffer == NULL) ? 0 :
            get_icamera_metadata_entry_count(mBuffer);
}

bool CameraMetadata::isEmpty() const {
    return entryCount() == 0;
}

status_t CameraMetadata::sort() {
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    return sort_icamera_metadata(mBuffer);
}

status_t CameraMetadata::checkType(uint32_t tag, uint8_t expectedType) {
    int tagType = get_icamera_metadata_tag_type(tag);
    if (tagType == -1 || tagType >= ICAMERA_NUM_TYPES) {
        LOGE("Update metadata entry: Unknown tag %d type=%d", tag, tagType);
        return INVALID_OPERATION;
    }
    if (tagType != expectedType) {
        LOGE("Mismatched tag type when updating entry %s (%d) of type %s; "
                "got type %s data instead ",
                get_icamera_metadata_tag_name(tag), tag,
                icamera_metadata_type_names[tagType],
                icamera_metadata_type_names[expectedType]);
        return INVALID_OPERATION;
    }
    return OK;
}

status_t CameraMetadata::update(uint32_t tag,
        const int32_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, ICAMERA_TYPE_INT32)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const uint8_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, ICAMERA_TYPE_BYTE)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const float *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, ICAMERA_TYPE_FLOAT)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const int64_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, ICAMERA_TYPE_INT64)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const double *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, ICAMERA_TYPE_DOUBLE)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const icamera_metadata_rational_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, ICAMERA_TYPE_RATIONAL)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const std::string &string) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, ICAMERA_TYPE_BYTE)) != OK) {
        return res;
    }
    // string.size() doesn't count the null termination character.
    return updateImpl(tag, (const void*)string.c_str(), string.size() + 1);
}

status_t CameraMetadata::updateImpl(uint32_t tag, const void *data,
        size_t data_count) {
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    int type = get_icamera_metadata_tag_type(tag);
    if (type == -1) {
        LOGE("%s: Tag %d not found", __func__, tag);
        return BAD_VALUE;
    }
    size_t data_size = calculate_icamera_metadata_entry_data_size(type,
            data_count);

    res = resizeIfNeeded(1, data_size);

    if (res == OK) {
        icamera_metadata_entry_t entry;
        res = find_icamera_metadata_entry(mBuffer, tag, &entry);
        if (res == NAME_NOT_FOUND) {
            res = add_icamera_metadata_entry(mBuffer,
                    tag, data, data_count);
        } else if (res == OK) {
            res = update_icamera_metadata_entry(mBuffer,
                    entry.index, data, data_count, NULL);
        }
    }

    if (res != OK) {
        LOGE("%s: Unable to update metadata entry %s.%s (%x): %s (%d)",
                __func__, get_icamera_metadata_section_name(tag),
                get_icamera_metadata_tag_name(tag), tag, strerror(-res), res);
    }

    if(validate_icamera_metadata_structure(mBuffer, /*size*/NULL) != OK) {
             LOGE("%s: Failed to validate metadata structure after update %p",
             __func__, mBuffer);
    }

    return res;
}

bool CameraMetadata::exists(uint32_t tag) const {
    icamera_metadata_ro_entry entry;
    return find_icamera_metadata_ro_entry(mBuffer, tag, &entry) == 0;
}

icamera_metadata_entry_t CameraMetadata::find(uint32_t tag) {
    status_t res;
    icamera_metadata_entry entry;
    CLEAR(entry);
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        entry.count = 0;
        return entry;
    }
    res = find_icamera_metadata_entry(mBuffer, tag, &entry);
    if (res != OK) {
        entry.count = 0;
        entry.data.u8 = NULL;
    }
    return entry;
}

icamera_metadata_ro_entry_t CameraMetadata::find(uint32_t tag) const {
    status_t res;
    icamera_metadata_ro_entry entry;
    res = find_icamera_metadata_ro_entry(mBuffer, tag, &entry);
    if (res != OK) {
        entry.count = 0;
        entry.data.u8 = NULL;
    }
    return entry;
}

status_t CameraMetadata::erase(uint32_t tag) {
    icamera_metadata_entry_t entry;
    status_t res;
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return INVALID_OPERATION;
    }
    res = find_icamera_metadata_entry(mBuffer, tag, &entry);
    if (res == NAME_NOT_FOUND) {
        return OK;
    } else if (res != OK) {
        LOGE("%s: Error looking for entry %s.%s (%x): %s %d",
                __func__,
                get_icamera_metadata_section_name(tag),
                get_icamera_metadata_tag_name(tag), tag, strerror(-res), res);
        return res;
    }
    res = delete_icamera_metadata_entry(mBuffer, entry.index);
    if (res != OK) {
        LOGE("%s: Error deleting entry %s.%s (%x): %s %d",
                __func__,
                get_icamera_metadata_section_name(tag),
                get_icamera_metadata_tag_name(tag), tag, strerror(-res), res);
    }
    return res;
}

void CameraMetadata::dump(int fd, int verbosity, int indentation) const {
    dump_indented_icamera_metadata(mBuffer, fd, verbosity, indentation);
}

status_t CameraMetadata::resizeIfNeeded(size_t extraEntries, size_t extraData) {
    if (mBuffer == NULL) {
        mBuffer = allocate_icamera_metadata(extraEntries * 2, extraData * 2);
        if (mBuffer == NULL) {
            LOGE("%s: Can't allocate larger metadata buffer", __func__);
            return NO_MEMORY;
        }
    } else {
        size_t currentEntryCount = get_icamera_metadata_entry_count(mBuffer);
        size_t currentEntryCap = get_icamera_metadata_entry_capacity(mBuffer);
        size_t newEntryCount = currentEntryCount +
                extraEntries;
        newEntryCount = (newEntryCount > currentEntryCap) ?
                newEntryCount * 2 : currentEntryCap;

        size_t currentDataCount = get_icamera_metadata_data_count(mBuffer);
        size_t currentDataCap = get_icamera_metadata_data_capacity(mBuffer);
        size_t newDataCount = currentDataCount +
                extraData;
        newDataCount = (newDataCount > currentDataCap) ?
                newDataCount * 2 : currentDataCap;

        if (newEntryCount > currentEntryCap ||
                newDataCount > currentDataCap) {
            icamera_metadata_t *oldBuffer = mBuffer;
            mBuffer = allocate_icamera_metadata(newEntryCount,
                    newDataCount);
            if (mBuffer == NULL) {
                LOGE("%s: Can't allocate larger metadata buffer", __func__);
                return NO_MEMORY;
            }
            append_icamera_metadata(mBuffer, oldBuffer);
            free_icamera_metadata(oldBuffer);
        }
    }
    return OK;
}

void CameraMetadata::swap(CameraMetadata& other) {
    if (mLocked) {
        LOGE("%s: CameraMetadata is locked", __func__);
        return;
    } else if (other.mLocked) {
        LOGE("%s: Other CameraMetadata is locked", __func__);
        return;
    }

    icamera_metadata* thisBuf = mBuffer;
    icamera_metadata* otherBuf = other.mBuffer;

    other.mBuffer = thisBuf;
    mBuffer = otherBuf;
}

} // namespace icamera
