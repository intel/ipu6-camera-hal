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

#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace icamera {

typedef std::mutex Mutex;
typedef std::lock_guard<std::mutex> AutoMutex;
typedef std::unique_lock<std::mutex> ConditionLock;

enum {
    PRIORITY_LOWEST = 19,
    PRIORITY_BACKGROUND = 10,
    PRIORITY_NORMAL = 0,
    PRIORITY_FOREGROUND = -2,
    PRIORITY_DISPLAY = -4,
    PRIORITY_URGENT_DISPLAY = -8,
    PRIORITY_AUDIO = -16,
    PRIORITY_URGENT_AUDIO = -19,
    PRIORITY_HIGHEST = -20,
    PRIORITY_DEFAULT = 0,
    PRIORITY_MORE_FAVORABLE = -1,
    PRIORITY_LESS_FAVORABLE = 1,
};

class Condition {
public:
    Condition() {}
    ~Condition() {}

    /**
     * Wait on the condition variable. MUST be locked with ConditionLock before being called.
     *
     * \param[in] lock: An object of type ConditionLock, which must be locked by the current thread.
     */
    void wait(ConditionLock& lock) {
        mCondition.wait(lock);
    }

    /**
     * Wait on the condition variable with a period of time.
     *
     * \param[in] lock: An object of type ConditionLock, which must be locked by the current thread.
     * \param[in] reltime: The maximum time to spend waiting.
     *
     * \return TIMED_OUT if it's not notified to wake up within reltime, otherwise return OK.
     */
    int waitRelative(ConditionLock& lock, int64_t reltime);

    /**
     * Wake up one thread that is waiting on the condition variable.
     */
    void signal() { mCondition.notify_one(); }

    /**
     * Wake up all threads that are waiting on the condition variable.
     */
    void broadcast() { mCondition.notify_all(); }

private:
    Condition(const Condition& other) = delete;
    Condition& operator=(const Condition&) = delete;

    std::condition_variable mCondition;
};

/**
 * Thread is a wrapper class to std::thread
 *
 * Thread helps manager the thread's state and make the std::thread is easier to used.
 * Thread also hides the platform specific implementation details.
 */
class Thread {
public:
    Thread();
    virtual ~Thread();

    /**
     * Start the thread.
     */
    virtual int run(std::string name = ("nameless"), int priority = PRIORITY_DEFAULT);

    /**
     * Ask this object's thread to exit. This function is asynchronous, so when it
     * returns the thread might still be running.
     */
    virtual void requestExit();

    /**
     * Wait until this object's thread exits. Returns immediately if not yet running.
     *
     * join will not trigger the thread to exit, it just wait for the thread exits.
     * Do not call it from this object's thread, will return WOULD_BLOCK in that case.
     */
    int join();

    /**
     * Ask this object's thread to exit. This function is synchronous, so when it
     * returns the thread must exit already.
     * It has same effect with calling requestExit and join combined.
     *
     * Do not call from this object's thread, will return WOULD_BLOCK in that case.
     */
    int requestExitAndWait();

    /**
     * Indicates whether this thread is running or not.
     */
    bool isRunning() const;

    /**
     * Indicates whether this thread is going to exit or not.
     */
    bool isExiting() const;

    /**
     * Indicates whether this thread exited or not.
     */
    bool isExited() const;

private:

    /**
     * threadLoop is the function which is called by the thread.
     * The derived class MUST override this function. The thread starts its life here.
     *
     * There are two ways of using the thread object:
     * 1. loop: threadLoop will be called again as long as it returns true,
     *          and requestExit() wasn't called.
     * 2. once: If threadLoop() returns false, the thread will exit upon return.
     *
     * There are three ways of exiting the thread.
     * 1. threadLoop return false.
     * 2. requestExit is called.
     * 3. requestExitAndWait is called.
     */
    virtual bool threadLoop() { return false; }

private:
    Thread(const Thread& other) = delete;
    Thread& operator=(const Thread&) = delete;

    /**
     * The function which is used to create the std::thread.
     */
    static void _threadLoop(Thread* self);

    /**
     * Set thread's property such as thread's name or priority.
     */
    void setProperty();

private:
    enum {
        NOT_STARTED,
        RUNNING,
        EXITING,
        EXITED,
    } mState;

    // The max length for thread name is 15.
    static const int MAX_THREAD_NAME_LEN = 15;
    std::thread* mThread;
    std::string mName;
    std::thread::id mId;
    int mPriority;

    // A lock used to protect internal data and API accessing.
    mutable Mutex mLock;

    // To make sure the thread not dead before "run" not finish.
    Condition mStartCondition;

    // To make sure API like join be able to wait until thread exits.
    Condition mExitedCondition;
};

} // namespace icamera

