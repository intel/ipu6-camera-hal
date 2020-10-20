/*
 * Copyright (C) 2017-2020 Intel Corporation.
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

#include "BufferQueue.h"

namespace icamera {

class ParameterGenerator;

/**
 * \class ProcessorManager
 *
 * \brief ProcessorManager helps to create and maintain the post processors.
 */
class ProcessorManager {
public:
    ProcessorManager(int cameraId);
    ~ProcessorManager();

    std::vector<BufferQueue*> createProcessors(int inputFmt,
                                               const std::map<Port, stream_t>& producerConfigs,
                                               const std::map<int, Port>& streamIdToPortMap,
                                               stream_config_t *streamList, const Parameters& param,
                                               ParameterGenerator* paramGenerator);
    int configureProcessors(const std::vector<ConfigMode>& configModes, BufferProducer* producer,
                            const Parameters& param);
    int deleteProcessors();

private:
    DISALLOW_COPY_AND_ASSIGN(ProcessorManager);
private:
    int mCameraId;

    enum PSysUsage {
        PSYS_NOT_USED = 0,
        PSYS_NORMAL,
        PSYS_WEAVING,
        PSYS_SCALE,
        PSYS_CSC,
        PSYS_SCALE_CSC,
        PSYS_WEAVING_SCALE,
        PSYS_WEAVING_SCALE_CSC,
        PSYS_FISHEYE,
        PSYS_MONO_DS,
    } mPsysUsage;

    struct ProcessorConfig {
        BufferQueue* mProcessor;
        std::map<Port, stream_t> mInputConfigs;
        std::map<Port, stream_t> mOutputConfigs;
    };

    std::vector<ProcessorConfig> mProcessors;
};

} // end of namespace icamera
