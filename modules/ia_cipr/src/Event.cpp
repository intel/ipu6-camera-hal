/*
 * Copyright (C) 2020 Intel Corporation.
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

#define LOG_TAG "CIPR_EVENT"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

using icamera::CAMERA_DEBUG_LOG_DBG;
using icamera::CAMERA_DEBUG_LOG_ERR;
using icamera::CAMERA_DEBUG_LOG_VERBOSE;
using icamera::CAMERA_DEBUG_LOG_WARNING;

#include "modules/ia_cipr/include/Context.h"
#include "modules/ia_cipr/include/Event.h"
#include "modules/ia_cipr/include/Utils.h"

namespace icamera {
namespace CIPR {
Event::Event(const PSysEventConfig& eventConfig) {
    mInitialized = false;
    mEvent = reinterpret_cast<PSysEvent*>(CIPR::callocMemory(1, sizeof(*mEvent)));

    CheckError(!mEvent, VOID_VALUE, "%s: could not allocate mEvent", __func__);

    if (eventConfig.id != 0) {
        icamera::LOG2("ID-field of CIPR mEvent deprecated!");
    }

    mEvent->event.type = eventConfig.type;
    mEvent->event.user_token = eventConfig.commandToken;
    mEvent->event.issue_id = eventConfig.commandIssueID;
    mEvent->event.buffer_idx = 0;
    mEvent->event.error = eventConfig.error;
    mEvent->timeout = eventConfig.timeout;

    mInitialized = true;
}

Event::~Event() {
    if (!mInitialized) {
        return;
    }

    mInitialized = false;
    CIPR::freeMemory(mEvent);
}

Result Event::getConfig(PSysEventConfig* eventConfig) {
    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);
    CheckError(!eventConfig, Result::InvaildArg, "@%s, eventConfig is nullptr", __func__);

    eventConfig->type = mEvent->event.type;
    eventConfig->commandToken = mEvent->event.user_token;
    eventConfig->commandIssueID = mEvent->event.issue_id;
    eventConfig->bufferHandoverBitmap = UINT64_MAX; /* Not used */
    eventConfig->error = mEvent->event.error;
    eventConfig->timeout = mEvent->timeout;
    eventConfig->id = 0;

    return Result::OK;
}

Result Event::setConfig(const PSysEventConfig& eventConfig) {
    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);

    mEvent->event.type = eventConfig.type;
    mEvent->event.user_token = eventConfig.commandToken;
    mEvent->event.issue_id = eventConfig.commandIssueID;
    mEvent->event.buffer_idx = 0;
    mEvent->event.error = eventConfig.error;
    mEvent->timeout = eventConfig.timeout;

    return Result::OK;
}

Result Event::wait(Context* ctx) {
    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);
    CheckError(!ctx, Result::InvaildArg, "@%s, ctx is nullptr", __func__);

    auto poller = ctx->getPoller(POLLIN | POLLHUP | POLLERR, mEvent->timeout);
    int res = poller.poll();
    if (res == 1) {
        return ctx->doIoctl(static_cast<int>(IPU_IOC_DQEVENT), mEvent);
    } else if (res == 0) {
        return Result::TimeOut;
    }

    icamera::LOG2("%s: poll returned error: %s", __func__, strerror(res));
    return Result::GeneralError;
}
}  // namespace CIPR
}  // namespace icamera
