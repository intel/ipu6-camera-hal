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

#ifndef IC2API_FOR_CHROME_H
#define IC2API_FOR_CHROME_H

extern "C" {

#define KEYPOINTSCOUNT 40
struct FaceResult {
    int x;
    int y;
    int width;
    int height;
    float keypoints[KEYPOINTSCOUNT][2];
    bool valid;
};
}

#endif
