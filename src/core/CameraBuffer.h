/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

#include <memory>
#include <queue>
#include <vector>

#include <linux/videodev2.h>
#include <v4l2_device.h>

#include "api/Parameters.h"
#include "iutils/Utils.h"

namespace icamera {

typedef struct v4l2_buffer v4l2_buffer_t;

/* CameraBuffer is the core buffers for HAL. The buffer usage is described by the
 * BufferUsage. CameraBuffer are constructed based on usage */
enum BufferUsage {
    BUFFER_USAGE_GENERAL = 0,
    BUFFER_USAGE_PSYS_STATS,
    BUFFER_USAGE_PSYS_INPUT,
    BUFFER_USAGE_MIPI_CAPTURE,
    BUFFER_USAGE_METADATA,
    BUFFER_USAGE_PSYS_INTERNAL,
};

class CameraBuffer {
public:
    //assist function to create frame buffers
    static std::shared_ptr<CameraBuffer>
    create(int cameraId, int usage, int memory, unsigned int size, int index,
           int srcFmt = -1, int srcWidth=-1, int srcHeight=-1);

public:
    CameraBuffer(int cameraId, int usage, int memory, uint32_t size, int index, int format = -1);
    virtual ~CameraBuffer();

public:
    //user buffer information
    int getWidth() const { return mU->s.width; }
    int getHeight() const { return mU->s.height; }
    int getStride() const { return mU->s.stride; }
    int getFormat() const { return mU->s.format; }
    int getStreamType() const { return mU->s.streamType; }
    int getStreamUsage() const { return mU->s.usage; }
    int getStreamId() const { return mU->s.id; }
    int getFlags() const { return mU->flags; }

    //v4l2 buffer information
    uint32_t getIndex(void) const { return mV.Index(); }

    uint32_t getSequence(void) const { return mV.Sequence(); }
    void setSequence(uint32_t sequence) { mV.SetSequence(sequence); }

    uint32_t getField() const { return mV.Field(); }
    void setField(uint32_t field) { mV.SetField(field); }

    struct timeval getTimestamp(void) const  { return mV.Timestamp(); }
    void setTimestamp(struct timeval timestamp) { mV.SetTimestamp(timestamp); }

    int getFd(int planeIndex = 0);

    uint32_t getMemory(void) const { return mV.Memory(); }

    int numPlanes() { return mNumPlanes; }

     //For debug only v4l2 buffer information
    int getCsi2Port(void) const { return (mV.RequestFd() >> 4) & 0xf; }
    int getVirtualChannel(void) const { return mV.RequestFd() & 0xf; }

    /* u buffer is used to attach user private structure pointer
     * in CameraBuffer.
     *
     * Now, one of this usage is linking camera_buffer_t to CameraBuffer
     * together, so that we can get each pointer by other.
     * Notes: Please don't abuse this. It is only used in CameraDevice for user buffer
     */
    camera_buffer_t *getUserBuffer() { return mU; }
    //update the user  buffer with latest v4l2 buffer info from driver
    void    updateUserBuffer(void);
    //Update the v4l2 flags according to user buffer flag
    void    UpdateFlags(void);
    void    updateFlags(void);

    //Check if the specific flag in "mU->flags" is set or not
    bool isFlagsSet(int flag);
    //The ubuffer is from application
    void setUserBufferInfo(camera_buffer_t *ubuffer);
    void setUserBufferInfo(int format, int width, int height);
    void setUserBufferInfo(int format, int width, int height, void *usrPtr);

    uint32_t getBufferSize(int planeIndex = 0) { return mV.Length(planeIndex); }
    void setBufferSize(unsigned int size, int planeIndex = 0) { mV.SetLength(size, planeIndex); }

    unsigned int getBytesused(int planeIndex = 0) { return mV.BytesUsed(planeIndex); }
    void setBytesused(unsigned int bytes, int planeIndex = 0) { mV.SetBytesUsed(bytes, planeIndex); }

    void* getBufferAddr(int planeIndex = 0) { return getAddr(planeIndex); }
    void  setBufferAddr(void *addr, int planeIndex = 0) { return setAddr(addr, planeIndex); }

    void updateV4l2Buffer(const v4l2_buffer_t& v4l2buf);

    V4L2Buffer& getV4L2Buffer() { return mV; }

    int getUsage() const { return mBufferUsage; }

    void setSettingSequence(long sequence) { mSettingSequence = sequence; }
    long getSettingSequence() const { return mSettingSequence; }

    //Buffers are allocated the buffers by Camera
    int allocateMemory(V4L2VideoNode *vDevice = nullptr);

public:
    static void* mapDmaBufferAddr(int fd, unsigned int bufferSize);
    static void unmapDmaBufferAddr(void* addr, unsigned int bufferSize);

private:
    CameraBuffer(const CameraBuffer&);
    CameraBuffer& operator=(const CameraBuffer&);

    void freeMemory();
    int exportMmapDmabuf(V4L2VideoNode *vDevice);

    int allocateMmap(V4L2VideoNode* dev);
    int allocateUserPtr();
    void freeUserPtr();
    void freeMmap();
    void* getAddr(int plane = 0);
    void setAddr(void *userAddr, int plane = 0);
    void initBuffer(int memType, v4l2_buf_type bufType, uint32_t size, int idx, int num_plane);

    void setFd(int val, int plane);

protected:
    V4L2Buffer mV;
    int mNumPlanes;

private:
    //To tag whether the memory is allocated by CameraBuffer class. We need to free them
    bool mAllocatedMemory;

    camera_buffer_t *mU;
    int mBufferUsage;
    long mSettingSequence;

    void* mMmapAddrs[VIDEO_MAX_PLANES];
    int mDmaFd[VIDEO_MAX_PLANES];
};

typedef std::vector<std::shared_ptr<CameraBuffer> > CameraBufVector;
typedef std::queue<std::shared_ptr<CameraBuffer> > CameraBufQ;

}
