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

#include <vector>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <linux/v4l2-subdev.h>
#include <linux/media.h>
#include <expat.h>

#include <v4l2_device.h>

#include "CameraTypes.h"
#include "iutils/Thread.h"

#include "NodeInfo.h"

namespace icamera {

struct MediaEntity;
struct MediaPad;
struct MediaLink;

#define MEDIA_CTL_DEV_NAME "/dev/media"
#define MEDIA_DRIVER_NAME "intel-ipu"
#define MEDIA_DEVICE_MAX_NUM 256

enum {
    FC_FORMAT = 0,
    FC_SELECTION = 1,
};

enum ResolutionType {
    RESOLUTION_MAX = 0,
    RESOLUTION_COMPOSE,
    RESOLUTION_CROP,
    RESOLUTION_TARGET,

};

struct McFormat {
    int entity;
    int pad;
    int stream;
    int formatType;
    int selCmd;
    int top;
    int left;
    int width;
    int height;
    enum ResolutionType type;
    std::string entityName;
    unsigned int pixelCode;
    McFormat() {
        entity = 0;
        pad = 0;
        stream = 0;
        formatType = 0;
        selCmd = 0;
        top = 0;
        left = 0;
        width = 0;
        height = 0;
        type = RESOLUTION_MAX;
        pixelCode = 0;}
};

struct McOutput {
    Port port;
    unsigned int v4l2Format;
    int width;
    int height;
    McOutput() { port = INVALID_PORT; v4l2Format = 0; width = 0; height = 0; }
};

struct McCtl {
    int entity;
    int ctlCmd;
    int ctlValue;
    std::string ctlName;
    std::string entityName;
    McCtl() { entity = 0; ctlCmd = 0; ctlValue= 0; }
};

struct McLink {
    int srcEntity;
    int srcPad;
    int sinkEntity;
    int sinkPad;
    bool enable;
    std::string srcEntityName;
    std::string sinkEntityName;
    McLink() { srcEntity = 0; srcPad = 0; sinkEntity = 0; sinkPad = 0; enable = false; }
};

struct McRoute {
    int entity;
    uint32_t sinkPad;
    uint32_t sinkStream;
    uint32_t srcPad;
    uint32_t srcStream;
    uint32_t flag;
    std::string entityName;
    McRoute() { entity = 0; sinkPad = 0; srcPad = 0; sinkStream = 0; srcStream = 0; flag = 0; entityName.clear(); }
};

struct McVideoNode {
    std::string name;
    VideoNodeType videoNodeType;
    McVideoNode() { videoNodeType = VIDEO_GENERIC; }
};

struct MediaCtlConf {
    std::vector <McCtl> ctls;
    std::vector <McLink> links;
    std::vector <McRoute> routes;
    std::vector <McFormat> formats;
    std::vector <McOutput> outputs;
    std::vector <McVideoNode> videoNodes;
    int mcId;
    int outputWidth;
    int outputHeight;
    std::vector <ConfigMode> configMode;
    int format;
    /*
     * The outputWidth or outputHeight is 0 if there isn't this setting
     * in MediaCtlConf. It means the isys output size is dynamic, and
     * we don't use stream size to select MC.
     */
    MediaCtlConf() {
        mcId = -1;
        outputWidth = 0;
        outputHeight = 0;
        format = -1;
    }
};

/**
 * \class MediaController
 *
 * This class is used for discovering and configuring the internal topology
 * of a media device. Devices are modelled as an oriented graph of building
 * blocks called media entities. The media entities are connected to each other
 * through pads.
 *
 * Each media entity corresponds to a V4L2 subdevice. This class is also used
 * for configuring the V4L2 subdevices.
 */

class MediaControl {
public:
    /**
     * \brief Get the singleton instance of MediaControl
     */
    static MediaControl* getInstance();

    /**
     * \brief Release the singleton instance of MediaControl.
     */
    static void releaseInstance();

    /**
     * \brief Enum entities and link, and reset all links
     *
     * \return 0 if succeed, other value indicates failed
     */
    int initEntities();

    /**
     * \brief Free all entities and links memory
     */
    void clearEntities();

    /**
     * \brief Get the entity by name
     *
     * \return entity id if succeed or -1 if error
     */
    int getEntityIdByName(const char *name);

    /**
     * \brief Get VCM I2C bus address
     *
     * \return 0 if succeed, other value indicates failed
     */
    int getVCMI2CAddr(const char* vcmName, std::string* vcmI2CAddr);

    /**
     * \brief Set up media controller pipe
     *
     * \param cameraId: the current camera id
     * \param mc: the MediaCtlConf got from platform data
     * \param sensorName: the sensor name get from platform data
     * \param width: The width of the request frame
     * \param height: The height of the request frame
     * \param format: The format of the request frame
     * \param field: The field of the request frame
     *
     * \return OK if succeed, other value indicates failed
     */
    int mediaCtlSetup(int cameraId, MediaCtlConf *mc, int width, int height, int field);

    /**
     * \brief Clear media controller pipe
     *
     *
     * \param cameraId: the current camera id
     * \param mc: the MediaCtlConf got from platform data
     */
    void mediaCtlClear(int cameraId, MediaCtlConf *mc);

    int resetAllLinks();

    int getLensName(std::string *lensName);
    bool checkAvailableSensor(const std::string &sensorEntityName,
                              const std::string &sinkEntityName);
    /**
     * Getting I2C bus address by the name of sensor entity and the name of sensor's sink entity.
     *
     * \sensorEntityName: the name of sensor entity.
     * \sinkEntityName: the name of sensor's sink entity.
     * \i2cBus: I2C bus address.
     * \return OK if succeed, other value indicates failed
     */
    int getI2CBusAddress(const std::string &sensorEntityName, const std::string &sinkEntityName,
                         std::string *i2cBus);
private:
    MediaControl& operator=(const MediaControl&);
    MediaControl(const char *devName);
    ~MediaControl();

    static MediaControl* getMediaControlInstance();
    int openDevice();
    void closeDevice(int fd);

    //enum MediaControl info.
    int enumInfo();
    int enumLinks(int fd);
    int enumEntities(int fd, media_device_info& devInfo);

    //get entity info.
    int getDevnameFromSysfs(MediaEntity *entity);
    MediaEntity *getEntityById(uint32_t id);
    MediaEntity *getEntityByName(const char *name);

    //set up entity link.

    MediaLink *entityAddLink(MediaEntity *entity);
    int setupLink(uint32_t srcEntity, uint32_t srcPad, uint32_t sinkEntity, uint32_t sinkPad, bool enable);
    int setupLink(MediaPad *source, MediaPad *sink, uint32_t flags);

    //set up MediaCtlConf info.
    void setMediaMcCtl(int cameraId, std::vector <McCtl> ctls);
    int setMediaMcLink(std::vector <McLink> links);
    int setFormat(int cameraId, const McFormat *format,
            int targetWidth, int targetHeight, int field);
    int setSelection(int cameraId, const McFormat *format,
            int targetWidth, int targetHeight);

    /* Dump functions */
    void dumpInfo(media_device_info& devInfo);
    void dumpEntityDesc(media_entity_desc& desc, media_device_info& devInfo);
    void dumpPadDesc(media_pad_desc *pads, const int padsCount = 1, const char *name = nullptr);
    void dumpLinkDesc(media_link_desc *links, const int linksCount = 1);
    const char *entitySubtype2String(unsigned type);
    const char *padType2String(unsigned flag);
    void dumpEntityTopology(bool dot = true);
    void dumpTopologyDot();
    void dumpTopologyText();

private:
    std::string mDevName;
    std::vector <MediaEntity> mEntities;

private:
    static MediaControl *sInstance;
    static Mutex sLock;
};

} //namespace icamera
