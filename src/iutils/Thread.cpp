/*
 * Copyright (C) 2017-2020 Intel Corporation.
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

#define LOG_TAG "Thread"

#include "Errors.h"
#include "Thread.h"
#include "CameraLog.h"

namespace icamera {

int Condition::waitRelative(ConditionLock& lock, int64_t reltime) {
    std::cv_status ret = mCondition.wait_for(lock, std::chrono::nanoseconds(reltime));
    return ret == std::cv_status::timeout ? TIMED_OUT : OK;
}

Thread::Thread() : mState(NOT_STARTED), mThread(nullptr), mPriority(PRIORITY_DEFAULT)
{
    LOG1("%s", __func__);
}

Thread::~Thread()
{
    LOG1("%s", __func__);

    requestExitAndWait();

    delete mThread;
}

int Thread::run(std::string name, int priority)
{
    LOG1("%s", __func__);

    AutoMutex lock(mLock);

    if (mState != NOT_STARTED && mState != EXITED) {
        LOGW("Cannot start thread(%s) in state(%d).", name.c_str(), mState);
        return INVALID_OPERATION;
    }

    // Thread can be restarted only if the previous one has exited.
    // Release the previous thread first if it's created already.
    delete mThread;

    mThread = new std::thread(_threadLoop, this);
    mThread->detach();
    mId = mThread->get_id();
    mName = name;
    mPriority = priority;
    mState = RUNNING;

    mStartCondition.signal();

    return OK;
}

void Thread::requestExit()
{
    LOG1("%s", __func__);

    AutoMutex lock(mLock);

    if (mState == RUNNING) {
        mState = EXITING;
    }
}

int Thread::requestExitAndWait()
{
    LOG1("%s", __func__);

    ConditionLock lock(mLock);

    // No need exit if it's not started.
    if (mState == NOT_STARTED) {
        return NO_INIT;
    }

    // The function cannot be called by same thread.
    if (std::this_thread::get_id() == mId) {
        LOGE("The thread itself cannot call its own requestExitAndWait function.");
        return WOULD_BLOCK;
    }

    while (mState != EXITED) {
        mState = EXITING;
        mExitedCondition.wait(lock);
    }

    return OK;
}

int Thread::join()
{
    LOG1("%s", __func__);

    ConditionLock lock(mLock);

    // No need join if it's not started.
    if (mState == NOT_STARTED) {
        return NO_INIT;
    }

    // The function cannot be called by same thread.
    if (std::this_thread::get_id() == mId) {
        LOGE("The thread itself cannot call its own join function.");
        return WOULD_BLOCK;
    }

    while (mState != EXITED) {
        mExitedCondition.wait(lock);
    }

    return OK;
}

bool Thread::isRunning() const
{
    AutoMutex lock(mLock);
    // A thread in EXITING also means it's still running, but it's going to exit.
    return mState == RUNNING || mState == EXITING;
}

bool Thread::isExiting() const
{
    AutoMutex lock(mLock);
    return mState == EXITING;
}

bool Thread::isExited() const
{
    AutoMutex lock(mLock);
    return mState == EXITED;
}

void Thread::_threadLoop(Thread* self)
{
    {
        // Wait for function "run" to finish.
        // If the thread is going to exit, then no need to wait anymore.
        ConditionLock lock(self->mLock);
        while (self->mState != RUNNING && self->mState != EXITING) {
            self->mStartCondition.wait(lock);
        }

        if (self->mState == EXITING) {
            self->mState = EXITED;
            self->mExitedCondition.broadcast();
            return;
        }

        self->setProperty();
    }

    while (true) {
        bool loopAgain = self->threadLoop();

        AutoMutex lock(self->mLock);
        if (!loopAgain || self->mState == EXITING) {
            self->mState = EXITED;
            self->mExitedCondition.broadcast();
            return;
        }
    }
}

// Platform specific implementation.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#include <sys/resource.h>

void Thread::setProperty()
{
    LOG1("%s, name:%s, priority:%d", __func__, mName.c_str(), mPriority);

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 12
    // Set thread's name
    std::string threadName = mName.empty() ? "NO_NAME" : mName;
    if (mName.size() > MAX_THREAD_NAME_LEN) {
        threadName = mName.substr(0, MAX_THREAD_NAME_LEN);
        LOG2("The thread name(%s) is too long, modify it to %s", mName.c_str(), threadName.c_str());
    }
    pthread_setname_np(pthread_self(), threadName.c_str());
#endif

    // Set thread's priority
    setpriority(PRIO_PROCESS, 0, mPriority);

    const int policy = SCHED_OTHER;
    int min = sched_get_priority_min(policy);
    int max = sched_get_priority_max(policy);
    LOG1("Priority range:(%d-%d)", min, max);

    if (mPriority < min) mPriority = min;
    if (mPriority > max) mPriority = max;

    sched_param param;
    param.sched_priority = mPriority;

    int ret = pthread_setschedparam(pthread_self(), policy, &param);
    LOG1("pthread_setschedparam ret:%d", ret);
}
#else
#warning "Setting thread's property is not implemented yet on this platform."
#endif

} // namespace icamera

