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

/**
 * Warning this header is automatically generated.
 * Any manual change here will be lost.
 */

#pragma once

#include <stdint.h>

namespace icamera {

typedef enum
{
% for item in uuid_list:
${item}
% endfor
} camera_control_isp_ctrl_id;

% for item in struct_list:
/**
 * \struct ${item[0]}
 */
${item[1]}
% endfor
} // end of icamera
