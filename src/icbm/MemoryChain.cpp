/*
 * Copyright (C) 2023 Intel Corporation
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

#include "src/icbm/MemoryChain.h"

namespace icamera {

MemoryChainDescription::MemoryChainDescription(const ImageInfo& iii, const ImageInfo& iio)
        : mInInfo(iii),
          mOutInfo(iio) {}

void MemoryChainDescription::linkIn(const char* featureName, const char* inPortName,
                                    const char* outPortName) {
    iaic_memory inMemory = createMemoryDesc(mInInfo);
    iaic_memory outMemory = createMemoryDesc(mOutInfo);

    inMemory.p = mInInfo.bufAddr;
    outMemory.p = mOutInfo.bufAddr;

    inMemory.feature_name = featureName;
    inMemory.port_name = inPortName;

    outMemory.feature_name = featureName;
    outMemory.port_name = outPortName;

    inMemory.media_type = iaic_nv12;
    outMemory.media_type = iaic_nv12;

    mInChain.push_back(inMemory);
    mOutChain.push_back(outMemory);

    if (mInChain.size() > 1) mInChain[mInChain.size() - 2].next = &mInChain[mInChain.size() - 1];
    if (mOutChain.size() > 1)
        mOutChain[mOutChain.size() - 2].next = &mOutChain[mOutChain.size() - 1];
}

MemoryIOPort MemoryChainDescription::getIOPort() {
    if (mInChain.empty() || mOutChain.empty())
        return {nullptr, nullptr};
    else
        return {&mInChain[0], &mOutChain[0]};
}

iaic_memory MemoryChainDescription::createMemoryDesc(const ImageInfo& ii) {
    iaic_memory mem = {};

    mem.has_gfx = false;
    mem.size[0] = ii.size;
    mem.size[1] = ii.width;
    mem.size[2] = ii.height;
    mem.size[3] = ii.stride;

    return mem;
}
}  // namespace icamera
