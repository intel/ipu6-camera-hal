/*
 * Copyright (C) 2015-2017 Intel Corporation.
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

#define LOG_TAG "CameraEvent"

#include "iutils/CameraLog.h"

#include "CameraEvent.h"

namespace icamera {

EventSource::EventSource()
{
    LOG1("@%s EventSource created", __func__);
}

EventSource::~EventSource()
{
    LOG1("@%s EventSource destructed", __func__);
}

void EventSource::registerListener(EventType eventType, EventListener* eventListener)
{
    LOG1("@%s eventType: %d, listener: %p", __func__, eventType, eventListener);

    CheckError(eventListener == nullptr, VOID_VALUE,
          "%s: event listener is nullptr, skip registration.", __func__);

    AutoMutex l(mListenersLock);

    std::set<EventListener*> listenersOfType;
    if (mListeners.find(eventType) != mListeners.end()) {
        listenersOfType = mListeners[eventType];
    }

    listenersOfType.insert(eventListener);
    mListeners[eventType] = listenersOfType;
}

void EventSource::removeListener(EventType eventType, EventListener* eventListener)
{
    LOG1("@%s eventType: %d, listener: %p", __func__, eventType, eventListener);
    AutoMutex l(mListenersLock);

    if (mListeners.find(eventType) == mListeners.end()) {
        LOG1("%s: no listener found for event type %d", __func__, eventType);
        return;
    }

    std::set<EventListener*> listenersOfType = mListeners[eventType];
    listenersOfType.erase(eventListener);
    mListeners[eventType] = listenersOfType;
}

void EventSource::notifyListeners(EventData eventData)
{
    LOG2("@%s eventType: %d", __func__, eventData.type);
    AutoMutex l(mListenersLock);

    if (mListeners.find(eventData.type) == mListeners.end()){
        LOG2("%s: no listener found for event type %d", __func__, eventData.type);
        return;
    }

    for (auto listener : mListeners[eventData.type]) {
        LOG2("%s: send event data to listener %p for event type %d", __func__, listener, eventData.type);
        listener->handleEvent(eventData);
    }
}

}
