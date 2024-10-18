/*
 * Copyright (C) 2018-2023 Intel Corporation.
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

#include <list>
#include <map>
#include <memory>

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelCcaClient.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

#include "Parameters.h"
#include "iutils/Thread.h"
#include "iutils/Utils.h"

namespace icamera {

struct MakernoteData {
    int64_t sequence;
    uint64_t timestamp;
    cca::cca_mkn* mknData;

    MakernoteData() {
        sequence = -1;
        timestamp = 0;
        CLEAR(mknData);
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
     * \brief init Makernote
     * allocate memories for mMakernoteDataList by using IntelCca::allocMem()
     *
     * param[in] int cameraId
     * param[in] TuningMode tuningMode
     *
     * return OK if it is successful, fails for other values.
     */
    int init(int cameraId, TuningMode tuningMode);

    /**
     * \brief deinit Makernote
     * free memories for mMakernoteDataList by using IntelCca::freeMem()
     *
     * param[in] int cameraId
     * param[in] TuningMode tuningMode
     *
     * return OK if it is successful, fails for other values.
     */
    int deinit(int cameraId, TuningMode tuningMode);

    /**
     * \brief Save Makernote by ia_mkn_trg mode
     *
     * param[in] int cameraId: camera ID
     * param[in] camera_makernote_mode_t: MAKERNOTE_MODE_JPEG is corresponding
     *           to ia_mkn_trg_section_1 for Normal Jpeg capture;
     *           MAKERNOTE_MODE_RAW is corresponding to ia_mkn_trg_section_2
     *           for Raw image capture.
     * param[in] int64_t sequence: the sequence in latest AiqResult
     * param[in] TuningMode tuningMode: tuning mode
     *
     * return OK if get Makernote successfully, otherwise return ERROR.
     */
    int saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode, int64_t sequence,
                          TuningMode tuningMode);

    /**
     * \brief Get makernote buffer
     *
     * param[in] camera_makernote_mode_t: makernote mode
     * param[out] bool: return if in dump case
     *
     * return pointer of makernote buffer, otherwise return nullptr.
     */
    void* getMakernoteBuf(camera_makernote_mode_t makernoteMode, bool& dump);

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
    void acquireMakernoteData(uint64_t timestamp, Parameters* param);

 private:
    // Should > max request number in processing
    static const int MAX_MAKER_NOTE_LIST_SIZE = 48;

    enum MknState { UNINIT, INIT } mMknState;

    // Guard for MakerNote API
    Mutex mMknLock;
    std::list<MakernoteData> mMakernoteDataList;
};

}  // namespace icamera
