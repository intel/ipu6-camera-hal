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
#include "subway_autogen.h"

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

extern void iaic_query_version(int* major, int* minor, int* patch);
extern void iaic_startup();
extern void iaic_shutdown();

extern void iaic_query_features(char* future_d, size_t* fsize);
extern void iaic_set_loglevel(iaic_log_level level);
extern void iaic_create_session(iaic_session uid, const char* feature, iaic_options opt);
extern void iaic_close_session(iaic_session uid, const char* feature);
extern void iaic_join_session(iaic_session uid, iaic_join_desc desc);
extern void iaic_disjoin_session(iaic_session uid, iaic_join_desc desc);
extern void iaic_pause_feature(iaic_session uid, const char* feature);
extern void iaic_resume_feature(iaic_session uid, const char* feature);
extern bool iaic_execute(iaic_session uid, iaic_memory in, iaic_memory out);
extern void iaic_set_data(iaic_session uid, iaic_memory& data);
extern void iaic_get_data(iaic_session uid, iaic_memory& data);
extern iaic_session_status iaic_get_status(iaic_session uid, const char* feature);

}

#endif
