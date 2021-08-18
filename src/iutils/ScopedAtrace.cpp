/*
 * Copyright (C) 2015-2018 Intel Corporation.
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

#include "utils/ScopedAtrace.h"
#include "Trace.h"

namespace icamera {

const int ATRACE_LEN = 128;
int gScopedAtraceLevel = 0;

ScopedAtrace::ScopedAtrace(const int level, const char* func, const char* tag,
                           const char* note, long value, const char* note2,
                           int value2, const char* note3, int value3)
{
    mEnableAtraceEnd = false;
    if(gScopedAtraceLevel & level) {
        char buf[ATRACE_LEN];
        if (value < 0 || note == nullptr) {
            snprintf(buf, ATRACE_LEN, "<%s,%s>", func, tag);
            atrace_begin(ATRACE_TAG, buf);
        } else if (value2 < 0 || note2 == nullptr) {
            snprintf(buf, ATRACE_LEN, "<%s,%s>:%s(%ld)", func, tag, note, value);
            atrace_begin(ATRACE_TAG, buf);
        } else if (value3 < 0 || note3 == nullptr) {
            snprintf(buf, ATRACE_LEN, "<%s,%s>:%s(%ld) %s(%d)", func, tag, note,
                     value, note2, value2);
            atrace_begin(ATRACE_TAG, buf);
        } else {
            snprintf(buf, ATRACE_LEN, "<%s,%s>:%s(%ld) %s(%d) %s(%d)", func, tag,
                     note, value, note2, value2, note3, value3);
            atrace_begin(ATRACE_TAG, buf);
        }
        mEnableAtraceEnd = true;
    }
}

ScopedAtrace::~ScopedAtrace()
{
    if(mEnableAtraceEnd) {
        atrace_end(ATRACE_TAG);
    }
}

void ScopedAtrace::setTraceLevel(int level)
{
    gScopedAtraceLevel = level;
}

} // namespace icamera
