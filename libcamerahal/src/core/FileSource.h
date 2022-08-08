/*
 * Copyright (C) 2017-2021 Intel Corporation.
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

#include "StreamSource.h"
#include "iutils/Thread.h"

namespace icamera {

/**
 * \class FileSource
 *
 * It's a buffer producer that's used to produce frame buffer via provided files
 * instead of from real sensor.
 *
 * There are three working mode:
 * 1. The first mode which only provides one same frame for all sequences.
 *    How to enable: export cameraInjectFile="FrameFileName"
 * 2. The second mode which can configure which file is used for any sequence or FPS.
 *    How to enable: export cameraInjectFile="ConfigFileName.xml"
 *    The value of cameraInjectFile MUST be ended with ".xml".
 * 3. The third mode which can inject files in sequence by specifying injection folder path.
 *    How to enable: export cameraInjectFile="Injection Folder"
 *    ("Injection Folder" is the specifyed injection folder path you want to run file injection)
 */
class FileSource : public StreamSource {
 public:
    explicit FileSource(int cameraId);
    ~FileSource();

    int init();
    void deinit();
    int configure(const std::map<Port, stream_t>& outputFrames,
                  const std::vector<ConfigMode>& configModes);
    int start();
    int stop();

    virtual int qbuf(Port port, const std::shared_ptr<CameraBuffer>& camBuffer);

    virtual void addFrameAvailableListener(BufferConsumer* listener);
    virtual void removeFrameAvailableListener(BufferConsumer* listener);
    void removeAllFrameAvailableListener();
    int allocateMemory(Port port, const std::shared_ptr<CameraBuffer>& camBuffer) { return OK; }

    // Overwrite EventSource APIs to avoid calling its parent's implementation.
    void registerListener(EventType eventType, EventListener* eventListener);
    void removeListener(EventType eventType, EventListener* eventListener);

 private:
    bool produce();
    int allocateSourceBuffer();
    void fillFrameBuffer(std::shared_ptr<CameraBuffer>& buffer);
    void fillFrameBuffer(std::string fileName, std::shared_ptr<CameraBuffer>& buffer);
    void notifyFrame(const std::shared_ptr<CameraBuffer>& buffer);
    void notifySofEvent();

 private:
    class ProduceThread : public Thread {
        FileSource* mFileSrc;

     public:
        explicit ProduceThread(FileSource* fileSource) : mFileSrc(fileSource) {}

        virtual bool threadLoop() { return mFileSrc->produce(); }
    };

    ProduceThread* mProduceThread;
    int mCameraId;
    bool mExitPending;

    float mFps;
    int64_t mSequence;
    std::string mInjectedFile;  // The injected file can be a actual frame or a XML config file.
    enum {
        USING_FRAME_FILE =
            0,  // If mInjectedFile is a frame file name, it means we're using frame file.
        USING_CONFIG_FILE,  // If mInjectedFile ends with ".xml", it means we're using config file.
        USING_INJECTION_PATH,  // If mInjectedFile is a directory, it means we're using injection
                               // path.
        UNKNOWN_INJECTED_WAY   // Error way
    } mInjectionWay;

    stream_t mStreamConfig;
    Port mOutputPort;

    std::vector<BufferConsumer*> mBufferConsumerList;
    std::map<std::string, std::shared_ptr<CameraBuffer>> mFrameFileBuffers;
    CameraBufQ mBufferQueue;
    Condition mBufferSignal;
    // Guard for FileSource Public API
    Mutex mLock;
};

/**
 * \class FileSourceProfile
 *
 * It's used to parse file source config file, and provide such fps and frame file name etc
 * information for FileSource to use.
 */
class FileSourceProfile {
 public:
    explicit FileSourceProfile(std::string configFile);
    ~FileSourceProfile() {}

    float getFps(int cameraId);
    std::string getFrameFile(int cameraId, int64_t sequence);
    int getFrameFiles(int cameraId, std::map<int, std::string>& framefiles);

    static void startElement(void* userData, const char* name, const char** atts);
    static void endElement(void* userData, const char* name);

    void checkField(const char* name, const char** atts);
    void parseXmlFile(const std::string& xmlFile);
    void mergeCommonConfig();

 private:
    struct CommonConfig {
        CommonConfig() : mFps(30.0), mFrameDir(".") {}
        float mFps;
        std::string mFrameDir;
    };

    struct FileSourceConfig {
        FileSourceConfig() : mFps(0) {}
        float mFps;
        std::string mFrameDir;
        std::map<int, std::string> mFrameFiles;
    };

    enum {
        FIELD_INVALID = 0,
        FIELD_SENSOR,
        FIELD_COMMON,
    } mCurrentDataField;

    std::string mCurrentSensor;
    CommonConfig mCommon;
    std::map<std::string, FileSourceConfig> mConfigs;

 private:
    DISALLOW_COPY_AND_ASSIGN(FileSourceProfile);
};

/**
 * \class FileSourceFromDir
 *
 * It's used to parse file source from injection file directory, and provide such frame file name
 * etc information for FileSource to use.
 */
class FileSourceFromDir {
 public:
    explicit FileSourceFromDir(const std::string& injectionPath);
    ~FileSourceFromDir();

    int getInjectionFileInfo(std::map<int, std::string>* frameFilesInfo);
    std::string getFrameFile(const std::map<int, std::string>& frameFilesInfo, int64_t sequence);
    void fillFrameBuffer(void* addr, size_t bufferSize, uint32_t sequence);

 private:
    std::string mInjectionPath;
    std::vector<std::string> mInjectionFiles;
};

}  // namespace icamera
