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

#pragma once

#include <map>
#include <list>
#include <memory>

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelMkn.h"
#else
#include "modules/algowrapper/IntelMkn.h"
#endif

#include "iutils/Utils.h"
#include "iutils/Thread.h"
#include "Parameters.h"

namespace icamera {

struct MakernoteData {
    int64_t sequence;
    uint64_t timestamp;
    unsigned int size;
    char section[MAKERNOTE_SECTION1_SIZE + MAKERNOTE_SECTION2_SIZE];

    MakernoteData() {
        sequence = -1;
        timestamp = 0;
        size = 0;
        CLEAR(section);
    }
};

/**
 * \class MakerNote
 *
 * This class encapsulates Intel Makernotes function, and provides interface
 * for enabling and acquiring Makenotes which is called by AiqEngine, Ltm
 * and AiqPlus.
 *
 */
class MakerNote {
 public:
    MakerNote();
    ~MakerNote();

    /**
     * \brief Save Makernote by ia_mkn_trg mode
     *
     * param[in] camera_makernote_mode_t: MAKERNOTE_MODE_JPEG is corresponding
     *           to ia_mkn_trg_section_1 for Normal Jpeg capture;
     *           MAKERNOTE_MODE_RAW is corresponding to ia_mkn_trg_section_2
     *           for Raw image capture.
     * param[in] int64_t sequence: the sequence in latest AiqResult
     *
     * return OK if get Makernote successfully, otherwise return ERROR.
     */
    int saveMakernoteData(camera_makernote_mode_t makernoteMode, int64_t sequence);

    /**
     * \brief Get ia_mkn (Makernote) handle.
     */
    ia_mkn *getMknHandle();

    /**
     * \brief Update timestamp of frame.
     *
     * param[in] sequence: the sequence in frame buffer;
     * param[in] timestamp: the timestamp in frame buffer.
     *
     */
    void updateTimestamp(int64_t sequence, uint64_t timestamp);

    /**
     * \brief Acquire MakerNote data.
     *
     * param[in] timestamp: the timestamp in frame buffer;
     * param[out] param: Makernote data will be saved in parameters as metadata.
     *
     */
    void acquireMakernoteData(uint64_t timestamp, Parameters *param);

 private:
    // Should > max request number in processing
    static const int MAX_MAKER_NOTE_LIST_SIZE = 32;

    enum MknState {
        UNINIT,
        INIT
    } mMknState;

    // Guard for MakerNote API
    Mutex mMknLock;
    ia_mkn *mMkn;

    IntelMkn mIntelMkn;

    std::list<std::shared_ptr<MakernoteData>> mMakernoteDataList;
};

}  // namespace icamera
