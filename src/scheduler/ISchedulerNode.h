/*
 * Copyright (C) 2022 Intel Corporation
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

namespace icamera {

/**
 * \Interface ISchedulerNode
 */
class ISchedulerNode {
 public:
    explicit ISchedulerNode(const char* name) : mName(name ? name : "unknown") {}
    virtual ~ISchedulerNode() {}

    virtual bool process(int64_t triggerId) = 0;

    const char* getName() const { return mName.c_str(); }

 private:
    std::string mName;
};

}  // namespace icamera
