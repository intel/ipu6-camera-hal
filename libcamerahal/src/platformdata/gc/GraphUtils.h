/*
 * Copyright (C) 2018-2021 Intel Corporation
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

#include "IGraphConfig.h"
#include "IGraphConfigManager.h"
#include "ia_isp_bxt_types.h"

namespace icamera {
namespace GraphUtils {
void dumpConnections(const std::vector<IGraphType::PipelineConnection>& connections);
void dumpKernelInfo(const ia_isp_bxt_program_group& programGroup);
};  // namespace GraphUtils
}  // namespace icamera
