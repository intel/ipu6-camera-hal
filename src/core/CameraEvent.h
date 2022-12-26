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

#pragma once

#include <map>
#include <set>

#include "iutils/Thread.h"
#include "CameraEventType.h"

namespace icamera {

class EventListener
{
public:
    EventListener() {};
    virtual ~EventListener() {};
    virtual void handleEvent(EventData eventData) {};
};

class EventSource
{
private:
    std::map<EventType, std::set<EventListener*>> mListeners;

    // Guard for EventSource public API to protect mListeners.
    Mutex mListenersLock;
public:
    EventSource();
    virtual ~EventSource();
    virtual void registerListener(EventType eventType, EventListener* eventListener);
    virtual void removeListener(EventType eventType, EventListener* eventListener);
    virtual void notifyListeners(EventData eventData);
};

}
