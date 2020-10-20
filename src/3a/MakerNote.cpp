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

#define LOG_TAG "MakerNote"

#include <memory>

#include "src/3a/MakerNote.h"
#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

namespace icamera {

MakerNote::MakerNote() :
    mMknState(UNINIT),
    mMkn(nullptr) {
    LOG1("@%s", __func__);

    for (int i = 0; i < MAX_MAKER_NOTE_LIST_SIZE; i++) {
        std::shared_ptr<MakernoteData> data = std::shared_ptr<MakernoteData>(new MakernoteData());
        mMakernoteDataList.push_back(data);
    }

    mMkn = mIntelMkn.init(ia_mkn_cfg_compression,
                          MAKERNOTE_SECTION1_SIZE, MAKERNOTE_SECTION2_SIZE);
    CheckError(mMkn == nullptr, VOID_VALUE, "@%s, Failed to init mkn", __func__);

    int ret = mIntelMkn.enable(mMkn, true);
    CheckError(ret != ia_err_none, VOID_VALUE,
               "@%s, Failed to enable mkn ret %d", __func__, ret);

    mMknState = INIT;
}

MakerNote::~MakerNote() {
    LOG1("@%s", __func__);

    AutoMutex lock(mMknLock);
    mIntelMkn.deinit(mMkn);

    mMakernoteDataList.clear();
}

int MakerNote::saveMakernoteData(camera_makernote_mode_t makernoteMode, int64_t sequence) {
    LOG1("@%s", __func__);
    if (makernoteMode == MAKERNOTE_MODE_OFF) return OK;

    AutoMutex lock(mMknLock);
    CheckError(mMknState != INIT, BAD_VALUE, "@%s, mkn isn't initialized", __func__);

    ia_mkn_trg mknTrg = (makernoteMode == MAKERNOTE_MODE_JPEG ? ia_mkn_trg_section_1 :
                                                                ia_mkn_trg_section_2);
    ia_binary_data makerNote;
    int ret = mIntelMkn.prepare(mMkn, mknTrg, &makerNote);
    CheckError(ret != OK, UNKNOWN_ERROR, "@%s, Failed to prepare makernote", __func__);
    CheckError((makerNote.data == nullptr), UNKNOWN_ERROR,
               "@%s, invalid makernote data pointer", __func__);
    CheckError(makerNote.size == 0, UNKNOWN_ERROR, "@%s, 0 size makernote data saved", __func__);

    std::shared_ptr<MakernoteData> data = mMakernoteDataList.front();
    mMakernoteDataList.pop_front();

    MEMCPY_S(data->section, sizeof(char) * (MAKERNOTE_SECTION1_SIZE + MAKERNOTE_SECTION2_SIZE),
             makerNote.data, makerNote.size);

    data->size = makerNote.size;
    data->sequence = sequence;
    data->timestamp = 0;
    LOG2("@%s, saved makernote %d for sequence %ld", __func__, makernoteMode, sequence);

    mMakernoteDataList.push_back(data);
    return OK;
}

ia_mkn *MakerNote::getMknHandle() {
    LOG1("@%s", __func__);
    AutoMutex lock(mMknLock);
    CheckError(mMknState != INIT, nullptr, "@%s, mkn isn't initialized", __func__);

    return mMkn;
}

void MakerNote::updateTimestamp(int64_t sequence, uint64_t timestamp) {
    LOG1("@%s", __func__);
    AutoMutex lock(mMknLock);
    CheckError(mMknState != INIT, VOID_VALUE, "@%s, mkn isn't initialized", __func__);

    for (auto rit = mMakernoteDataList.rbegin(); rit != mMakernoteDataList.rend(); ++rit) {
        if ((*rit)->sequence == sequence) {
            LOG2("@%s, found sequence %ld for request sequence %ld, timestamp %ld", __func__,
                 (*rit)->sequence, sequence, timestamp);
            (*rit)->timestamp = timestamp;
            break;
        }
    }
}

void MakerNote::acquireMakernoteData(uint64_t timestamp, Parameters *param) {
    LOG1("@%s", __func__);
    AutoMutex lock(mMknLock);
    CheckError(mMknState != INIT, VOID_VALUE, "@%s, mkn isn't initialized", __func__);

    for (auto rit = mMakernoteDataList.rbegin(); rit != mMakernoteDataList.rend(); ++rit) {
        if ((*rit)->timestamp > 0 && timestamp >= (*rit)->timestamp) {
            LOG2("@%s, found timestamp %ld for request timestamp %ld", __func__,
                 (*rit)->timestamp, timestamp);
            param->setMakernoteData((*rit)->section, (*rit)->size);
            break;
        }
    }
}

}  // namespace icamera
