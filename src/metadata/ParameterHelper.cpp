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

#define LOG_TAG "ParameterHelper"

#include "iutils/Utils.h"
#include "iutils/CameraLog.h"

#include "Parameters.h"
#include "ParameterHelper.h"

namespace icamera {

void ParameterHelper::merge(const Parameters& src, Parameters* dst)
{
    AutoRLock rl(src.mData);
    merge(getMetadata(src.mData), dst);
}

void ParameterHelper::merge(const CameraMetadata& metadata, Parameters* dst)
{
    if (metadata.isEmpty()) {
        // Nothing needs to be merged
        return;
    }

    AutoWLock wl(dst->mData);
    const icamera_metadata_t* src = const_cast<CameraMetadata*>(&metadata)->getAndLock();
    size_t count = metadata.entryCount();
    icamera_metadata_ro_entry_t entry;
    for (size_t i = 0; i < count; i++) {
        CLEAR(entry);
        if (get_icamera_metadata_ro_entry(src, i, &entry) != OK) {
            continue;
        }
        switch (entry.type) {
        case ICAMERA_TYPE_BYTE:
            getMetadata(dst->mData).update(entry.tag, entry.data.u8, entry.count);
            break;
        case ICAMERA_TYPE_INT32:
            getMetadata(dst->mData).update(entry.tag, entry.data.i32, entry.count);
            break;
        case ICAMERA_TYPE_FLOAT:
            getMetadata(dst->mData).update(entry.tag, entry.data.f, entry.count);
            break;
        case ICAMERA_TYPE_INT64:
            getMetadata(dst->mData).update(entry.tag, entry.data.i64, entry.count);
            break;
        case ICAMERA_TYPE_DOUBLE:
            getMetadata(dst->mData).update(entry.tag, entry.data.d, entry.count);
            break;
        case ICAMERA_TYPE_RATIONAL:
            getMetadata(dst->mData).update(entry.tag, entry.data.r, entry.count);
            break;
        default:
            LOGW("Invalid entry type, should never happen");
            break;
        }
    }
    const_cast<CameraMetadata*>(&metadata)->unlock(src);
}

void ParameterHelper::copyMetadata(const Parameters& source, CameraMetadata* metadata)
{
    CheckError((!metadata), VOID_VALUE, "null metadata to be updated!");

    AutoRLock rl(source.mData);
    *metadata = getMetadata(source.mData);
}

} // end of namespace icamera
