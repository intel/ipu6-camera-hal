/*
 * Copyright (C) 2021 Intel Corporation.
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

#ifndef LOG_SINK
#define LOG_SINK

namespace icamera {
struct LogItem {
    const char* logEntry;
    int level;
    const char* logTags;
};

class LogOutputSink {
 public:
    virtual ~LogOutputSink() = default;

    virtual const char* getName() const = 0;
    virtual void sendOffLog(LogItem logItem) = 0;

 protected:
    static void setLogTime(char* timeBuf);
};

#ifdef CAL_BUILD
class GLogSink : public LogOutputSink {
 public:
    const char* getName() const override;
    void sendOffLog(LogItem logItem) override;
};
#endif

#ifdef CAMERA_TRACE
class FtraceLogSink : public LogOutputSink {
 public:
    FtraceLogSink();

    const char* getName() const override;
    void sendOffLog(LogItem logItem) override;

 private:
    int mFtraceFD;
};
#endif

class StdconLogSink : public LogOutputSink {
 public:
    const char* getName() const override;
    void sendOffLog(LogItem logItem) override;
};

#ifdef CAMERA_SYS_LOG
class SysLogSink : public LogOutputSink {
 public:
    SysLogSink();
    ~SysLogSink();
    const char* getName() const override;
    void sendOffLog(LogItem logItem) override;
};
#endif

class FileLogSink : public LogOutputSink {
 public:
    FileLogSink();
    ~FileLogSink();
    const char* getName() const override;
    void sendOffLog(LogItem logItem) override;

 private:
    FILE* mFp;
};

}  // namespace icamera

#endif
