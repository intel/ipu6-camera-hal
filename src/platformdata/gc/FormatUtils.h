/*
 * Copyright (C) 2016-2019 Intel Corporation.
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

namespace graphconfig {
namespace utils {

int32_t getV4L2Format(const int32_t commonPixelFormat);
const std::string format2string(int32_t format);
bool isPlanarFormat(int32_t v4l2Format);
bool isRaw(int32_t format);
bool isVectorizedRaw(int32_t format);
int32_t getBpl(int32_t format, int32_t width);
int32_t getBpp(int32_t format);
int32_t getBppFromCommon(int32_t format);

}  // namespace utils
}  // namespace graphconfig
