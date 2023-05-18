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

#include <vector>
#include <utility>

#include "src/icbm/ICBMTypes.h"
#include "src/icbm/OPIC2Api.h"

namespace icamera {
using MemoryChain = std::vector<iaic_memory>;
using MemoryIOPort = std::pair<iaic_memory*, iaic_memory*>;

class MemoryChainDescription {
 public:
    MemoryChainDescription(const ImageInfo& iii, const ImageInfo& iio);
    ~MemoryChainDescription() = default;
    MemoryChainDescription(MemoryChainDescription&& rhs) = default;
    MemoryChainDescription& operator=(const MemoryChainDescription& rhs) = delete;
    MemoryChainDescription(const MemoryChainDescription& rhs) = delete;

    void linkIn(const char* featureName, const char* inPortName, const char* outPortName);
    MemoryIOPort getIOPort();

 private:
    ImageInfo mInInfo;
    ImageInfo mOutInfo;

    MemoryChain mInChain;
    MemoryChain mOutChain;

    iaic_memory createMemoryDesc(const ImageInfo& ii);
};
}  // namespace icamera
