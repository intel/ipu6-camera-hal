/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2016-2020 Intel Corporation. All Rights Reserved.
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
#define LOG_TAG "SWJpegEncoder"

#include "SWJpegEncoder.h"

#include <string>

#include "ImageConverter.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#define RESOLUTION_1_3MP_WIDTH 1280
#define RESOLUTION_1_3MP_HEIGHT 960

namespace icamera {

SWJpegEncoder::SWJpegEncoder()
        : mJpegSize(-1),
          mTotalWidth(0),
          mTotalHeight(0),
          mDstBuf(nullptr),
          mCPUCoresNum(1) {
    LOG2("@%s, line:%d", __func__, __LINE__);
}

SWJpegEncoder::~SWJpegEncoder() {
    LOG2("@%s, line:%d", __func__, __LINE__);
}

std::unique_ptr<IJpegEncoder> IJpegEncoder::createJpegEncoder() {
    return std::unique_ptr<SWJpegEncoder>(new SWJpegEncoder());
}

/**
 * \param package: encode package for either thumbnail or main image
 * \return true if encoding succeeds
 * \return false if encoding fails
 */
bool SWJpegEncoder::doJpegEncode(EncodePackage* package) {
    CheckError(package == nullptr, false, "@%s, package is nullptr", __func__);

    int status = 0;
    nsecs_t startTime = CameraUtils::systemTime();

    LOG2("@%s: IN = {buf:%p, w:%u, h:%u, sz:%u, stride:%d, fmt:%s}", __func__, package->inputData,
         package->inputWidth, package->inputHeight, package->inputSize, package->inputStride,
         icamera::CameraUtils::format2string(package->inputFormat).c_str());

    LOG2("@%s: OUT = {buf:%p, w:%u, h:%u, sz:%u, q:%d}", __func__, package->outputData,
         package->outputWidth, package->outputHeight, package->outputSize, package->quality);

    if (package->inputWidth == 0 || package->inputHeight == 0 || package->inputFormat == 0) {
        ALOGE("Invalid input received!");
        mJpegSize = -1;
        goto exit;
    }

    mTotalWidth = package->inputWidth;
    mTotalHeight = package->inputHeight;

    /*
     * For encoding main buffer, need to skip the exif data and SOI header.
     * Because the SOI(jpeg maker) is written in the beginning of jpeg data when
     * do encode, so only skip the exif data size here, and the SOI will be moved
     * to the head of output buffer
     */
    mDstBuf = reinterpret_cast<unsigned char*>(package->outputData) + package->exifDataSize;

    if (useMultiThreadEncoding(package->inputWidth, package->inputHeight))
        status = swEncodeMultiThread(*package);
    else
        status = swEncode(*package);

    if (status < 0) goto exit;

    package->encodedDataSize = mJpegSize;

    LOG2("@%s encode, total consume:%ums, encoded jpeg size: %d", __func__,
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000), mJpegSize);

    return mJpegSize > 0 ? true : false;
exit:
    return false;
}

/**
 *  This function will decide if we need to enable the multi thread jpeg encoding.
 *  currently, we have two conditions to use the old single jpeg encoding.
 *  one is that the resolution is smaller than the 1.3M
 *  the other is that the CPU number is 1
 *
 *  \param width: the Jpeg width
 *  \param height: the Jpeg height
 *
 *  \return false if we don't need multi thread Jpeg encoding
 *  \return true if we need multi thread Jpeg encoding
 */
bool SWJpegEncoder::useMultiThreadEncoding(int width, int height) {
    LOG2("@%s, line:%d, width:%d, height:%d", __func__, __LINE__, width, height);
    bool ret = false;

    /* more conditions could be added to here by according to the request */
    if ((width < RESOLUTION_1_3MP_WIDTH && height < RESOLUTION_1_3MP_HEIGHT))
        ret = false;
    else if (width & 0xf)
        ret = false;
    else
        ret = true;

    LOG2("@%s, line:%d, ret:%d", __func__, __LINE__, ret);
    return ret;
}

/**
 * encode jpeg by calling the SWJpegEncoder which is the libjpeg wrapper
 * single thread.
 *
 * \param package: jpeg encode package
 * \return 0 if encoding was successful
 * \return -1 if encoding failed
 */
int SWJpegEncoder::swEncode(const EncodePackage& package) {
    LOG2("@%s, line:%d, use the libjpeg to do sw jpeg encoding", __func__, __LINE__);
    int status = 0;
    Codec encoder;

    encoder.init();
    encoder.setJpegQuality(package.quality);
    status = encoder.configEncoding(package.inputWidth, package.inputHeight, package.inputStride,
                                    static_cast<JSAMPLE*>(mDstBuf),
                                    (package.outputSize - package.exifDataSize));
    const void* uv_buf =
        static_cast<unsigned char*>(package.inputData) + package.inputStride * package.inputHeight;

    if (status) goto exit;

    status = encoder.doJpegEncoding(package.inputData, uv_buf, package.inputFormat);
    if (status) goto exit;

exit:
    if (status)
        mJpegSize = -1;
    else
        encoder.getJpegSize(&mJpegSize);

    encoder.deInit();

    return (status ? -1 : 0);
}

/**
 * encode jpeg by calling the SWJpegEncoder which is the libjpeg wrapper
 * multi thread.
 * the thread number depends on the CPU number.
 *
 * \param package: jpeg encode package
 * \return 0 if encoding was successful
 * \return -1 if encoding failed
 */
int SWJpegEncoder::swEncodeMultiThread(const EncodePackage& package) {
    LOG2("@%s, line:%d, use the libjpeg to do sw jpeg encoding", __func__, __LINE__);
    int status = 0;

    init(mCPUCoresNum);
    config(package);

    status = doJpegEncodingMultiThread();
    if (status) goto exit;

exit:
    mJpegSize = status ? -1 : mergeJpeg();
    deInit();

    return (status ? -1 : 0);
}

/**
 * Initialize for the multi thread jpeg encoding
 *
 * it will create n CodecWorkerThread by according to the thread number.
 */
void SWJpegEncoder::init(unsigned int threadNum) {
    unsigned int num = CLIP(threadNum, MAX_THREAD_NUM, MIN_THREAD_NUM);
    LOG2("@%s, line:%d, thread number, pass:%d, real:%d", __func__, __LINE__, threadNum, num);

    for (unsigned int i = 0; i < num; i++) {
        std::shared_ptr<CodecWorkerThread> codecWorkerThread(new CodecWorkerThread);
        mSwJpegEncoder.push_back(codecWorkerThread);
    }
}

/**
 * deInit for the multi thread jpeg encoding
 *
 * it will release all n CodecWorkerThread
 */
void SWJpegEncoder::deInit(void) {
    LOG2("@%s, line:%d", __func__, __LINE__);
    for (auto& encoder : mSwJpegEncoder) {
        encoder.reset();
    }

    mSwJpegEncoder.clear();
}

/**
 * configue every thread for multi thread jpeg
 *
 * \param package: jpeg encode package
 */
void SWJpegEncoder::config(const EncodePackage& package) {
    LOG2("@%s, line:%d", __func__, __LINE__);
    std::shared_ptr<CodecWorkerThread> encThread;
    CodecWorkerThread::CodecConfig cfg;

    for (unsigned int i = 0; i < mSwJpegEncoder.size(); i++) {
        cfg.width = package.inputWidth;
        /*
            for example, there are 4 threads.
            the first 3 threads must align to 16 which is NV12_MCU_SIZE
            but for the last thread, it doesn't have this request.
        */
        cfg.height = ALIGN_16(package.inputHeight / mSwJpegEncoder.size());
        cfg.stride = package.inputStride;
        /*
         * For NV12 format, Y and UV data are independent, total size is width*height*1.5;
         * For YUYV format, Y and UV data are crossing, total size is width*height*2;
         * So the inBufY and inBufUV should be distinguished base on format.
         */
        cfg.fourcc = package.inputFormat;
        cfg.inBufY =
            (cfg.fourcc == V4L2_PIX_FMT_YUYV)
                ? static_cast<unsigned char*>(package.inputData) + cfg.stride * cfg.height * 2 * i
                : static_cast<unsigned char*>(package.inputData) + cfg.stride * cfg.height * i;
        cfg.inBufUV =
            (cfg.fourcc == V4L2_PIX_FMT_NV12 || cfg.fourcc == V4L2_PIX_FMT_NV21)
                ? (static_cast<unsigned char*>(package.inputData) +
                   package.inputStride * package.inputHeight + cfg.stride * cfg.height * i / 2)
                : nullptr;
        cfg.quality = package.quality;
        cfg.outBufSize = (package.outputSize - package.exifDataSize - DEST_BUF_OFFSET) /
                         package.inputHeight * cfg.height;
        cfg.outBuf = static_cast<unsigned char*>(mDstBuf) + DEST_BUF_OFFSET + cfg.outBufSize * i;
        /* update the last thread's height */
        if (i == mSwJpegEncoder.size() - 1) {
            cfg.height = package.inputHeight - cfg.height * (mSwJpegEncoder.size() - 1);
            cfg.outBufSize = package.outputSize - package.exifDataSize - DEST_BUF_OFFSET -
                             cfg.outBufSize * (mSwJpegEncoder.size() - 1);
        }

        encThread = mSwJpegEncoder[i];
        encThread->setConfig(cfg);
        LOG2("@%s, line:%d, the %d picture thread cfg", __func__, __LINE__, i);
        LOG2("@%s, line:%d, cfg.width:%d, cfg.height:%d", __func__, __LINE__, cfg.width,
             cfg.height);
        LOG2("@%s, line:%d, cfg.fourcc:%d, cfg.quality:%d", __func__, __LINE__, cfg.fourcc,
             cfg.quality);
        LOG2("@%s, line:%d, cfg.inBufY:%p, cfg.inBufUV:%p", __func__, __LINE__, cfg.inBufY,
             cfg.inBufUV);
        LOG2("@%s, line:%d, cfg.outBuf:%p, cfg.outBufSize:%d", __func__, __LINE__, cfg.outBuf,
             cfg.outBufSize);
    }
}

/**
 * the function will trigger the multi jpeg encoding
 *
 * \return 0 if encoding was successful
 * \return -1 if encoding failed
 */
int SWJpegEncoder::doJpegEncodingMultiThread(void) {
    LOG2("@%s, line:%d", __func__, __LINE__);
    std::shared_ptr<CodecWorkerThread> encThread;
    status_t status = OK;
    std::string threadName("CamHAL_SWEncodeMultiThread");

    /* run all threads */
    for (unsigned int i = 0; i < mSwJpegEncoder.size(); i++) {
        threadName = threadName + ":" + std::to_string(i);
        LOG2("@%s, new sw jpeg thread name:%s", __func__, threadName.c_str());
        encThread = mSwJpegEncoder[i];
        status = encThread->runThread(threadName.c_str());
        if (status != OK) {
            ALOGE("@%s, line:%d, start jpeg thread fail, thread name:%s", __func__, __LINE__,
                  threadName.c_str());
            return status;
        }
    }

    /* wait all threads to finish */
    for (unsigned int i = 0; i < mSwJpegEncoder.size(); i++) {
        LOG2("@%s, the %d sw jpeg encoder thread before join!", __func__, i);
        encThread = mSwJpegEncoder[i];
        encThread->waitThreadFinish();
        if (encThread->getJpegDataSize() == -1) status = UNKNOWN_ERROR;
    }

    return status;
}

/**
 * the function will merge all jpeg pictures which are generated in multi threads
 * to one jpeg picture
 *
 * \return int the merged jpeg size
 */
int SWJpegEncoder::mergeJpeg(void) {
#define HEADER_TOTAL_LEN 623
#define HEADER_SOS_LEN 14
#define HEADER_EOI_LEN 2
#define HEADER_HEIGHT_POS 163
#define HEADER_WIDTH_POS 165
#define NV12_MCU_SIZE 16
    LOG2("@%s, line:%d", __func__, __LINE__);
    int size = HEADER_TOTAL_LEN - HEADER_SOS_LEN;
    CodecWorkerThread::CodecConfig cfg;
    nsecs_t startTime;
    std::shared_ptr<CodecWorkerThread> encThread = mSwJpegEncoder.at(0);
    if (encThread == nullptr) {
        ALOGE("encThread is nullptr");
        return -1;
    }
    encThread->getConfig(&cfg);

    /* Write the JPEG header */
    MEMCPY_S(mDstBuf, size, cfg.outBuf, size);

    /* Update the width and height info */
    mDstBuf[HEADER_HEIGHT_POS] = (mTotalHeight >> 8) & 0xFF;
    mDstBuf[HEADER_HEIGHT_POS + 1] = mTotalHeight & 0xFF;
    mDstBuf[HEADER_WIDTH_POS] = (mTotalWidth >> 8) & 0xFF;
    mDstBuf[HEADER_WIDTH_POS + 1] = mTotalWidth & 0xFF;

    /* Write the restarting interval */
    if (mSwJpegEncoder.size() > 1) {
        unsigned int MCUs = (cfg.height / NV12_MCU_SIZE) * (cfg.width / NV12_MCU_SIZE);
        mDstBuf[size++] = 0xFF;
        mDstBuf[size++] = 0xDD;
        mDstBuf[size++] = 0;
        mDstBuf[size++] = 4;
        mDstBuf[size++] = (MCUs >> 8) & 0xFF;
        mDstBuf[size++] = MCUs & 0xFF;
    }

    /* Write the SOS */
    MEMCPY_S(reinterpret_cast<void*>((unsigned long)mDstBuf + size), HEADER_SOS_LEN,
             reinterpret_cast<void*>((unsigned long)cfg.outBuf + HEADER_TOTAL_LEN - HEADER_SOS_LEN),
             HEADER_SOS_LEN);
    size += HEADER_SOS_LEN;

    /* Write coded segments */
    for (unsigned int i = 0; i < mSwJpegEncoder.size(); i++) {
        encThread = mSwJpegEncoder[i];
        startTime = CameraUtils::systemTime();
        encThread->getConfig(&cfg);
        memmove(reinterpret_cast<void*>((unsigned long)mDstBuf + size),
                reinterpret_cast<void*>((unsigned long)cfg.outBuf + HEADER_TOTAL_LEN),
                (encThread->getJpegDataSize() - HEADER_TOTAL_LEN - HEADER_EOI_LEN));
        LOG2("@%s, wr %d segments, size:%d, consume:%ums", __func__, i,
             (encThread->getJpegDataSize() - HEADER_TOTAL_LEN - HEADER_EOI_LEN),
             (unsigned)((CameraUtils::systemTime() - startTime) / 1000000));
        size += (encThread->getJpegDataSize() - HEADER_TOTAL_LEN - HEADER_EOI_LEN);

        if (i != (mSwJpegEncoder.size() - 1)) {
            mDstBuf[size++] = 0xFF;
            mDstBuf[size++] = (i & 0x7) | 0xD0;
        }
    }

    /* Write EOI */
    mDstBuf[size++] = 0xFF;
    mDstBuf[size++] = 0xD9;

    return size;
}

SWJpegEncoder::CodecWorkerThread::CodecWorkerThread() : mDataSize(-1) {
    LOG2("@%s, line:%d", __func__, __LINE__);
    CLEAR(mCfg);
}

SWJpegEncoder::CodecWorkerThread::~CodecWorkerThread() {
    LOG2("@%s, line:%d", __func__, __LINE__);
}

/**
 * run one thread for multi thread jpeg encoding
 *
 * \param name: the thread name
 */
status_t SWJpegEncoder::CodecWorkerThread::runThread(const char* name) {
    LOG2("@%s, line:%d", __func__, __LINE__);
    return this->run(name);
}

/**
 * wait one thread until it has finished
 *
 */
void SWJpegEncoder::CodecWorkerThread::waitThreadFinish(void) {
    LOG2("@%s, line:%d", __func__, __LINE__);
    this->join();
    this->requestExitAndWait();
}

/**
 * get jpeg size which is done in one thread
 *
 * \return int the coded jpeg size
 */
int SWJpegEncoder::CodecWorkerThread::getJpegDataSize(void) {
    LOG2("@%s, line:%d", __func__, __LINE__);
    return mDataSize;
}

/**
 * the thread exe function for one jpeg thread
 * when the encoding has been done, it will return false to terminate the thread
 *
 * \return false
 */
bool SWJpegEncoder::CodecWorkerThread::threadLoop() {
    LOG2("@%s, line:%d, in CodecWorkerThread", __func__, __LINE__);
    nsecs_t startTime = CameraUtils::systemTime();
    int ret = swEncode();
    LOG2("@%s one swEncode done!, consume:%ums, ret:%d", __func__,
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000), ret);

    return false;
}

/**
 * this function will call the SWJpegEncoder to encode one jpeg.
 * it's the main function of the threadLoop
 *
 * \return 0 if encoding was successful
 * \return -1 if encoding failed
 */
int SWJpegEncoder::CodecWorkerThread::swEncode(void) {
    LOG2("@%s, line:%d, in CodecWorkerThread", __func__, __LINE__);
    int status = 0;
    Codec encoder;

    encoder.init();
    encoder.setJpegQuality(mCfg.quality);
    status = encoder.configEncoding(mCfg.width, mCfg.height, mCfg.stride,
                                    static_cast<JSAMPLE*>(mCfg.outBuf), mCfg.outBufSize);
    if (status) goto exit;

    status = encoder.doJpegEncoding(mCfg.inBufY, mCfg.inBufUV, mCfg.fourcc);
    if (status) goto exit;

exit:
    if (status)
        mDataSize = -1;
    else
        encoder.getJpegSize(&mDataSize);

    encoder.deInit();

    return (status ? -1 : 0);
}

SWJpegEncoder::Codec::Codec() : mStride(-1), mJpegQuality(DEFAULT_JPEG_QUALITY) {
    LOG2("@%s", __func__);
    CLEAR(mCInfo);
    CLEAR(mJErr);
}

SWJpegEncoder::Codec::~Codec() {
    LOG2("@%s", __func__);
}

/**
 * Init the SW jpeg encoder
 *
 * It will init the libjpeg library
 */
void SWJpegEncoder::Codec::init(void) {
    LOG2("@%s", __func__);
    CLEAR(mCInfo);
    mCInfo.err = jpeg_std_error(&mJErr);
    jpeg_create_compress(&mCInfo);
}

/**
 * deInit the SW jpeg encoder
 *
 * It will deinit the libjpeg library
 */
void SWJpegEncoder::Codec::deInit(void) {
    LOG2("@%s", __func__);
    jpeg_destroy_compress(&mCInfo);
}

/**
 * Set the jpeg quality
 *
 * \param quality: one value from 0 to 100
 *
 */
void SWJpegEncoder::Codec::setJpegQuality(int quality) {
    LOG2("@%s, quality:%d", __func__, quality);
    mJpegQuality = CLIP(quality, 100, 1);
}

/**
 * Config the SW jpeg encoder.
 *
 * mainly, it will set the destination buffer manager, color space, quality.
 *
 * \param width: the width of the jpeg dimentions.
 * \param height: the height of the jpeg dimentions.
 * \param jpegBuf: the dest buffer to store the jpeg data
 * \param jpegBufSize: the size of jpegBuf buffer
 *
 * \return 0 if the configuration is right.
 * \return -1 if the configuration fails.
 */
int SWJpegEncoder::Codec::configEncoding(int width, int height, int stride, void* jpegBuf,
                                         int jpegBufSize) {
    LOG2("@%s", __func__);

    mStride = stride;
    mCInfo.input_components = 3;
    mCInfo.in_color_space = (J_COLOR_SPACE)SUPPORTED_FORMAT;
    mCInfo.image_width = width;
    mCInfo.image_height = height;

    if (setupJpegDestMgr(&mCInfo, static_cast<JSAMPLE*>(jpegBuf), jpegBufSize) < 0) {
        ALOGE("@%s, line:%d, setupJpegDestMgr fail", __func__, __LINE__);
        return -1;
    }

    jpeg_set_defaults(&mCInfo);
    jpeg_set_colorspace(&mCInfo, (J_COLOR_SPACE)SUPPORTED_FORMAT);
    jpeg_set_quality(&mCInfo, mJpegQuality, TRUE);
    mCInfo.raw_data_in = TRUE;
    mCInfo.dct_method = JDCT_ISLOW;
    mCInfo.comp_info[0].h_samp_factor = 2;
    mCInfo.comp_info[0].v_samp_factor = 2;
    mCInfo.comp_info[1].h_samp_factor = 1;
    mCInfo.comp_info[1].v_samp_factor = 1;
    mCInfo.comp_info[2].h_samp_factor = 1;
    mCInfo.comp_info[2].v_samp_factor = 1;
    jpeg_start_compress(&mCInfo, TRUE);

    return 0;
}

/**
 * Do the SW jpeg encoding.
 *
 * it will convert the YUV data to P411 and then do jpeg encoding.
 *
 * \param y_buf: the source buffer for Y data
 * \param uv_buf: the source buffer for UV data,
 * \it could be nullptr if fourcc is V4L2_PIX_FMT_YUYV
 * \return 0 if the encoding is successful.
 * \return -1 if the encoding fails.
 */
int SWJpegEncoder::Codec::doJpegEncoding(const void* y_buf, const void* uv_buf, int fourcc) {
    LOG2("@%s", __func__);

    unsigned char* srcY = nullptr;
    unsigned char* srcUV = nullptr;
    unsigned char* p411 = nullptr;
    JSAMPROW y[16], u[16], v[16];
    JSAMPARRAY data[3];
    int i, j, width, height;

    width = mCInfo.image_width;
    height = mCInfo.image_height;
    srcY = (unsigned char*)y_buf;
    srcUV = (unsigned char*)uv_buf;
    p411 = new unsigned char[width * height * 3 / 2];

    switch (fourcc) {
        case V4L2_PIX_FMT_YUYV:
            ImageConverter::YUY2ToP411(width, height, mStride, srcY, p411);
            break;
        case V4L2_PIX_FMT_NV12:
            ImageConverter::NV12ToP411Separate(width, height, mStride, srcY, srcUV, p411);
            break;
        case V4L2_PIX_FMT_NV21:
            ImageConverter::NV21ToP411Separate(width, height, mStride, srcY, srcUV, p411);
            break;
        default:
            ALOGE("%s Unsupported fourcc %d", __func__, fourcc);
            delete[] p411;
            return -1;
    }

    data[0] = y;
    data[1] = u;
    data[2] = v;
    for (i = 0; i < height; i += 16) {
        for (j = 0; j < 16 && (i + j) < height; j++) {
            y[j] = p411 + width * (j + i);
            if (j % 2 == 0) {
                u[j / 2] = p411 + width * height + width / 2 * ((j + i) / 2);
                v[j / 2] = p411 + width * height + width * height / 4 + width / 2 * ((j + i) / 2);
            }
        }
        jpeg_write_raw_data(&mCInfo, data, 16);
    }

    jpeg_finish_compress(&mCInfo);

    delete[] p411;
    p411 = nullptr;

    return 0;
}

/**
 * Get the jpeg size.
 *
 * \param jpegSize: get the real jpeg size, it will be -1, if encoding fails
 */
void SWJpegEncoder::Codec::getJpegSize(int* jpegSize) {
    LOG2("@%s", __func__);

    JpegDestMgrPtr dest = (JpegDestMgrPtr)mCInfo.dest;

    *jpegSize = (false == dest->encodeSuccess) ? -1 : dest->codedSize;
}

/**
 * Setup the jpeg destination buffer manager
 *
 * it will convert the YUV data to P411 and then do jpeg encoding.
 *
 * \param cInfo: the compress pointer
 * \param jpegBuf: the buffer pointer for jpeg data
 * \param jpegBufSize: the jpegBuf buffer's size
 * \return 0 if it's successful.
 * \return -1 if it fails.
 */
int SWJpegEncoder::Codec::setupJpegDestMgr(j_compress_ptr cInfo, JSAMPLE* jpegBuf,
                                           int jpegBufSize) {
    LOG2("@%s", __func__);
    JpegDestMgrPtr dest;

    if (nullptr == jpegBuf || jpegBufSize <= 0) {
        ALOGE("@%s, line:%d, jpegBuf:%p, jpegBufSize:%d", __func__, __LINE__, jpegBuf, jpegBufSize);
        return -1;
    }

    if (cInfo->dest == nullptr) {
        cInfo->dest = (struct jpeg_destination_mgr*)(*cInfo->mem->alloc_small)(
            (j_common_ptr)cInfo, JPOOL_PERMANENT, sizeof(JpegDestMgr));
        CLEAR(*cInfo->dest);
    }
    dest = (JpegDestMgrPtr)cInfo->dest;

    dest->pub.init_destination = initDestination;
    dest->pub.empty_output_buffer = emptyOutputBuffer;
    dest->pub.term_destination = termDestination;
    dest->outJpegBuf = jpegBuf;
    dest->outJpegBufSize = jpegBufSize;

    return 0;
}

/**
 * Init the destination
 *
 * It's the first function which be called
 * among initDestination, emptyOutputBuffer and termDestination
 *
 * \param cInfo: the compress pointer
 */
void SWJpegEncoder::Codec::initDestination(j_compress_ptr cInfo) {
    LOG2("@%s", __func__);
    JpegDestMgrPtr dest = (JpegDestMgrPtr)cInfo->dest;

    dest->pub.next_output_byte = dest->outJpegBuf;
    dest->pub.free_in_buffer = dest->outJpegBufSize;
    dest->encodeSuccess = true;
}

/**
 * Empty the output buffer
 *
 * The function should not be called,
 * because we should allocate enough memory for the jpeg destination buffer
 * If we return FALSE, the libjpeg will terminate, so return TRUE always.
 * But when the function is called, the encoding failing will be recorded.
 *
 * \param cInfo: the compress pointer
 * \return TRUE if it is successful.
 * \return FALSE if something is wrong
 */
boolean SWJpegEncoder::Codec::emptyOutputBuffer(j_compress_ptr cInfo) {
    LOG2("@%s", __func__);
    ALOGE("@%s, line:%d, buffer overflow!", __func__, __LINE__);
    JpegDestMgrPtr dest = (JpegDestMgrPtr)cInfo->dest;

    /* re-cfg the buffer info */
    dest->pub.next_output_byte = dest->outJpegBuf;
    dest->pub.free_in_buffer = dest->outJpegBufSize;
    dest->encodeSuccess = false;

    return TRUE; /* if return FALSE, the total taking picture will fail */
}

/**
 * Terminate the destination
 *
 * The function will be called as the last function,
 * among initDestination, emptyOutputBuffer and termDestination.
 * We can get the encoded jpeg size from it.
 *
 * \param cInfo: the compress pointer
 */
void SWJpegEncoder::Codec::termDestination(j_compress_ptr cInfo) {
    LOG2("@%s", __func__);
    JpegDestMgrPtr dest = (JpegDestMgrPtr)cInfo->dest;

    dest->codedSize = dest->outJpegBufSize - dest->pub.free_in_buffer;
    LOG2("@%s, line:%d, codedSize:%d", __func__, __LINE__, dest->codedSize);
}

}  // namespace icamera
