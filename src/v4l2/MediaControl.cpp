/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2015-2021 Intel Corporation
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

#define LOG_TAG MediaControl

#include "MediaControl.h"

#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <stack>
#include <string>

#include "Parameters.h"
#include "PlatformData.h"
#include "SysCall.h"
#include "V4l2DeviceFactory.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

using std::string;
using std::vector;

namespace icamera {

struct MediaLink {
    MediaPad* source;
    MediaPad* sink;
    MediaLink* twin;
    uint32_t flags;
    uint32_t padding[3];
};

struct MediaPad {
    MediaEntity* entity;
    uint32_t index;
    uint32_t flags;
    uint32_t padding[3];
};

struct MediaEntity {
    media_entity_desc info;
    MediaPad* pads;
    MediaLink* links;
    unsigned int maxLinks;
    unsigned int numLinks;

    char devname[32];
};

static const string ivscName = "Intel IVSC CSI";
MediaControl* MediaControl::sInstance = nullptr;
Mutex MediaControl::sLock;

MediaControl* MediaControl::getMediaControlInstance() {
    MediaControl* mediaControlInstance = nullptr;

    for (int i = 0; i < MEDIA_DEVICE_MAX_NUM; i++) {
        std::string fileName = MEDIA_CTL_DEV_NAME;
        fileName.append(std::to_string(i));

        struct stat fileStat = {};
        int ret = stat(fileName.c_str(), &fileStat);
        if (ret != 0) {
            LOG1("%s: There is no file %s", __func__, fileName.c_str());
            continue;
        }

        SysCall* sc = SysCall::getInstance();
        int fd = sc->open(fileName.c_str(), O_RDWR);
        if (fd < 0) {
            LOG1("%s, Open media device(%s) failed: %s", __func__, fileName.c_str(),
                 strerror(errno));
            break;
        }

        media_device_info info;
        ret = sc->ioctl(fd, MEDIA_IOC_DEVICE_INFO, &info);
        if ((ret != -1) &&
            (0 == strncmp(info.driver, MEDIA_DRIVER_NAME, strlen(MEDIA_DRIVER_NAME)))) {
            mediaControlInstance = new MediaControl(fileName.c_str());
        }

        if (sc->close(fd) < 0) {
            LOGW("Failed to close media device %s:%s", fileName.c_str(), strerror(errno));
        }

        if (mediaControlInstance) {
            LOG1("%s: media device name:%s", __func__, fileName.c_str());
            break;
        }
    }

    return mediaControlInstance;
}

/*static*/ MediaControl* MediaControl::getInstance() {
    AutoMutex lock(sLock);
    if (!sInstance) {
        sInstance = getMediaControlInstance();
    }
    return sInstance;
}

void MediaControl::releaseInstance() {
    LOG1("%s", __func__);
    AutoMutex lock(sLock);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

MediaControl::MediaControl(const char* devName) : mDevName(devName) {
    LOG1("@%s device: %s", __func__, devName);
}

MediaControl::~MediaControl() {
    LOG1("@%s", __func__);
}

int MediaControl::initEntities() {
    LOG1("@%s", __func__);

    mEntities.reserve(100);

    int ret = enumInfo();
    if (ret != 0) {
        LOGE("Enum Info failed.");
        return -1;
    }

    return 0;
}

void MediaControl::clearEntities() {
    LOG1("@%s", __func__);

    auto entity = mEntities.begin();
    while (entity != mEntities.end()) {
        delete[] entity->pads;
        entity->pads = nullptr;
        delete[] entity->links;
        entity->links = nullptr;
        entity = mEntities.erase(entity);
    }
}

MediaEntity* MediaControl::getEntityByName(const char* name) {
    CheckAndLogError(!name, nullptr, "Invalid Entity name");

    for (auto& entity : mEntities) {
        if (strcmp(name, entity.info.name) == 0) {
            return &entity;
        }
    }

    return nullptr;
}

int MediaControl::getEntityIdByName(const char* name) {
    MediaEntity* entity = getEntityByName(name);
    if (!entity) {
        return -1;
    }

    return entity->info.id;
}

int MediaControl::resetAllLinks() {
    LOG1("@%s", __func__);

    for (auto& entity : mEntities) {
        for (uint32_t j = 0; j < entity.numLinks; j++) {
            MediaLink* link = &entity.links[j];

            if (link->flags & MEDIA_LNK_FL_IMMUTABLE ||
                link->source->entity->info.id != entity.info.id) {
                continue;
            }
            int ret = setupLink(link->source, link->sink, link->flags & ~MEDIA_LNK_FL_ENABLED);

            if (ret < 0) return ret;
        }
    }

    return 0;
}

// VIRTUAL_CHANNEL_S
int MediaControl::resetAllRoutes(int cameraId) {
    LOG1("<id%d> %s", cameraId, __func__);

    for (MediaEntity& entity : mEntities) {
        struct v4l2_subdev_route routes[entity.info.pads];
        uint32_t numRoutes = entity.info.pads;

        string subDeviceNodeName;
        subDeviceNodeName.clear();
        CameraUtils::getSubDeviceName(entity.info.name, subDeviceNodeName);
        if (subDeviceNodeName.find("/dev/") == std::string::npos) {
            continue;
        }

        V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, subDeviceNodeName);
        int ret = subDev->GetRouting(routes, &numRoutes);
        if (ret != 0) {
            continue;
        }

        for (uint32_t j = 0; j < numRoutes; j++) {
            routes[j].flags &= ~V4L2_SUBDEV_ROUTE_FL_ACTIVE;
        }

        ret = subDev->SetRouting(routes, numRoutes);
        if (ret < 0) {
            LOGW("@%s, setRouting ret:%d", __func__, ret);
        }
    }

    return OK;
}
// VIRTUAL_CHANNEL_E

int MediaControl::setupLink(MediaPad* source, MediaPad* sink, uint32_t flags) {
    MediaLink* link = nullptr;
    media_link_desc ulink;
    uint32_t i;
    int ret = 0;

    SysCall* sc = SysCall::getInstance();

    int fd = openDevice();
    if (fd < 0) goto done;

    for (i = 0; i < source->entity->numLinks; i++) {
        link = &source->entity->links[i];

        if (link->source->entity == source->entity && link->source->index == source->index &&
            link->sink->entity == sink->entity && link->sink->index == sink->index)
            break;
    }

    if (i == source->entity->numLinks) {
        LOGE("%s: Link not found", __func__);
        ret = -ENOENT;
        goto done;
    }

    /* source pad */
    memset(&ulink, 0, sizeof(media_link_desc));
    ulink.source.entity = source->entity->info.id;
    ulink.source.index = source->index;
    ulink.source.flags = MEDIA_PAD_FL_SOURCE;

    /* sink pad */
    ulink.sink.entity = sink->entity->info.id;
    ulink.sink.index = sink->index;
    ulink.sink.flags = MEDIA_PAD_FL_SINK;

    if (link) ulink.flags = flags | (link->flags & MEDIA_LNK_FL_IMMUTABLE);

    if (Log::isDumpMediaInfo()) dumpLinkDesc(&ulink, 1);

    ret = sc->ioctl(fd, MEDIA_IOC_SETUP_LINK, &ulink);
    if (ret == -1) {
        ret = -errno;
        LOGE("Unable to setup link (%s)", strerror(errno));
        goto done;
    }

    if (link) {
        link->flags = ulink.flags;
        link->twin->flags = ulink.flags;
    }

    ret = 0;

done:
    closeDevice(fd);
    return ret;
}

int MediaControl::setupLink(uint32_t srcEntity, uint32_t srcPad, uint32_t sinkEntity,
                            uint32_t sinkPad, bool enable) {
    LOG1("@%s srcEntity %d srcPad %d sinkEntity %d sinkPad %d enable %d", __func__, srcEntity,
         srcPad, sinkEntity, sinkPad, enable);

    for (auto& entity : mEntities) {
        for (uint32_t j = 0; j < entity.numLinks; j++) {
            MediaLink* link = &entity.links[j];

            if ((link->source->entity->info.id == srcEntity) && (link->source->index == srcPad) &&
                (link->sink->entity->info.id == sinkEntity) && (link->sink->index == sinkPad)) {
                if (enable)
                    link->flags |= MEDIA_LNK_FL_ENABLED;
                else
                    link->flags &= ~MEDIA_LNK_FL_ENABLED;

                return setupLink(link->source, link->sink, link->flags);
            }
        }
    }

    return -1;
}

int MediaControl::openDevice() {
    int fd;
    LOG1("@%s %s", __func__, mDevName.c_str());

    SysCall* sc = SysCall::getInstance();

    fd = sc->open(mDevName.c_str(), O_RDWR);
    if (fd < 0) {
        LOGE("Failed to open media device %s: %s", mDevName.c_str(), strerror(errno));
        return UNKNOWN_ERROR;
    }

    return fd;
}

void MediaControl::closeDevice(int fd) {
    LOG1("@%s, fd %d", __func__, fd);

    if (fd < 0) return;

    SysCall* sc = SysCall::getInstance();

    if (sc->close(fd) < 0) {
        LOGE("Failed to close media device %s: %s", mDevName.c_str(), strerror(errno));
    }
}

void MediaControl::dumpInfo(media_device_info& devInfo) {
    LOGI("Media controller API version %u.%u.%u\n\n", (devInfo.media_version << 16) & 0xff,
         (devInfo.media_version << 8) & 0xff, (devInfo.media_version << 0) & 0xff);

    LOGI("Media device information\n"
         "------------------------\n"
         "driver          %s\n"
         "model           %s\n"
         "serial          %s\n"
         "bus info        %s\n"
         "hw revision     0x%x\n"
         "driver version  %u.%u.%u\n\n",
         devInfo.driver, devInfo.model, devInfo.serial, devInfo.bus_info, devInfo.hw_revision,
         (devInfo.driver_version << 16) & 0xff, (devInfo.driver_version << 8) & 0xff,
         (devInfo.driver_version << 0) & 0xff);

    for (uint32_t i = 0; i < sizeof(devInfo.reserved) / sizeof(uint32_t); i++)
        LOGI("reserved[%u] %d", i, devInfo.reserved[i]);
}

int MediaControl::enumInfo() {
    SysCall* sc = SysCall::getInstance();

    if (mEntities.size() > 0) return 0;

    int fd = openDevice();
    if (fd < 0) {
        LOGE("Open device failed.");
        return fd;
    }

    media_device_info info;
    int ret = sc->ioctl(fd, MEDIA_IOC_DEVICE_INFO, &info);
    if (ret < 0) {
        LOGE("Unable to retrieve media device information for device %s (%s)", mDevName.c_str(),
             strerror(errno));
        goto done;
    }

    if (Log::isDumpMediaInfo()) dumpInfo(info);

    ret = enumEntities(fd, info);
    if (ret < 0) {
        LOGE("Unable to enumerate entities for device %s", mDevName.c_str());
        goto done;
    }

    LOG1("Found %lu entities, enumerating pads and links", mEntities.size());

    ret = enumLinks(fd);
    if (ret < 0) {
        LOGE("Unable to enumerate pads and linksfor device %s", mDevName.c_str());
        goto done;
    }

    ret = 0;

done:
    closeDevice(fd);
    return ret;
}

void MediaControl::dumpEntityDesc(media_entity_desc& desc, media_device_info& devInfo) {
    LOGI("id %d", desc.id);
    LOGI("name %s", desc.name);
    LOGI("type 0x%x", desc.type);
    LOGI("revision %d", desc.revision);
    LOGI("flags %d", desc.flags);
    LOGI("group_id %d", desc.group_id);
    LOGI("pads %d", desc.pads);
    LOGI("links %u", desc.links);

    for (uint32_t i = 0; i < sizeof(desc.reserved) / sizeof(uint32_t); i++)
        LOGI("reserved[%u] %d", i, devInfo.reserved[i]);
}

int MediaControl::enumEntities(int fd, media_device_info& devInfo) {
    MediaEntity entity;
    uint32_t id;
    int ret;
    SysCall* sc = SysCall::getInstance();

    for (id = 0, ret = 0;; id = entity.info.id) {
        memset(&entity, 0, sizeof(MediaEntity));
        entity.info.id = id | MEDIA_ENT_ID_FLAG_NEXT;

        ret = sc->ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &entity.info);
        if (ret < 0) {
            ret = errno != EINVAL ? -errno : 0;
            break;
        }

        if (Log::isDumpMediaInfo()) dumpEntityDesc(entity.info, devInfo);

        /* Number of links (for outbound links) plus number of pads (for
         * inbound links) is a good safe initial estimate of the total
         * number of links.
         */
        entity.maxLinks = entity.info.pads + entity.info.links;

        entity.pads = new MediaPad[entity.info.pads];
        entity.links = new MediaLink[entity.maxLinks];
        getDevnameFromSysfs(&entity);
        mEntities.push_back(entity);

        /* Note: carefully to move the follow setting. It must be behind of
         * push_back to mEntities:
         * 1. if entity is not pushed back to mEntities, getEntityById will
         * return NULL.
         * 2. we can't set entity.pads[i].entity to &entity direct. Because,
         * entity is stack variable, its scope is just this function.
         */
        for (uint32_t i = 0; i < entity.info.pads; ++i) {
            entity.pads[i].entity = getEntityById(entity.info.id);
        }
    }

    return ret;
}

int MediaControl::getDevnameFromSysfs(MediaEntity* entity) {
    char sysName[MAX_SYS_NAME] = {'\0'};
    char target[MAX_TARGET_NAME] = {'\0'};
    int ret;

    if (!entity) {
        LOGE("entity is null.");
        return -EINVAL;
    }

    ret = snprintf(sysName, MAX_SYS_NAME, "/sys/dev/char/%u:%u", entity->info.v4l.major,
                   entity->info.v4l.minor);
    if (ret <= 0) {
        LOGE("create sysName failed ret %d.", ret);
        return -EINVAL;
    }

    ret = readlink(sysName, target, MAX_TARGET_NAME);
    if (ret <= 0) {
        LOGE("readlink sysName %s failed ret %d.", sysName, ret);
        return -EINVAL;
    }

    char* d = strrchr(target, '/');
    if (!d) {
        LOGE("target is invalid %s.", target);
        return -EINVAL;
    }
    d++; /* skip '/' */

    char* t = strstr(d, "dvb");
    if (t && t == d) {
        t = strchr(t, '.');
        if (!t) {
            LOGE("target is invalid %s.", target);
            return -EINVAL;
        }
        *t = '/';
        d += 3; /* skip "dvb" */
        snprintf(entity->devname, sizeof(entity->devname), "/dev/dvb/adapter%s", d);
    } else {
        snprintf(entity->devname, sizeof(entity->devname), "/dev/%s", d);
    }

    return 0;
}

void MediaControl::dumpPadDesc(media_pad_desc* pads, const int padsCount, const char* name) {
    for (int i = 0; i < padsCount; i++) {
        LOGI("Dump %s Pad desc %d", name == nullptr ? "" : name, i);
        LOGI("entity: %d", pads[i].entity);
        LOGI("index: %d", pads[i].index);
        LOGI("flags: %d", pads[i].flags);
        LOGI("reserved[0]: %d", pads[i].reserved[0]);
        LOGI("reserved[1]: %d", pads[i].reserved[1]);
    }
}

void MediaControl::dumpLinkDesc(media_link_desc* links, const int linksCount) {
    for (int i = 0; i < linksCount; i++) {
        LOGI("Dump Link desc %d", i);
        MediaEntity* sourceEntity = getEntityById(links[i].source.entity);
        MediaEntity* sinkEntity = getEntityById(links[i].sink.entity);

        dumpPadDesc(&links[i].source, 1, sourceEntity->info.name);
        dumpPadDesc(&links[i].sink, 1, sinkEntity->info.name);
        LOGI("flags: %d", links[i].flags);
        LOGI("reserved[0]: %d", links[i].reserved[0]);
        LOGI("reserved[1]: %d", links[i].reserved[1]);
    }
}

int MediaControl::enumLinks(int fd) {
    int ret = 0;

    SysCall* sc = SysCall::getInstance();

    for (auto& entity : mEntities) {
        media_links_enum links;
        uint32_t i;

        links.entity = entity.info.id;
        links.pads = new media_pad_desc[entity.info.pads];
        memset(links.pads, 0, sizeof(struct media_pad_desc) * entity.info.pads);
        links.links = new media_link_desc[entity.info.links];
        memset(links.links, 0, sizeof(struct media_link_desc) * entity.info.links);

        if (sc->ioctl(fd, MEDIA_IOC_ENUM_LINKS, &links) < 0) {
            ret = -errno;
            LOG1("Unable to enumerate pads and links (%s).", strerror(errno));
            delete[] links.pads;
            delete[] links.links;
            return ret;
        }

        if (Log::isDumpMediaInfo()) {
            LOG1("entity %d", links.entity);
            dumpPadDesc(links.pads, entity.info.pads);
            dumpLinkDesc(links.links, entity.info.links);
        }

        for (i = 0; i < entity.info.pads; ++i) {
            entity.pads[i].entity = getEntityById(entity.info.id);
            entity.pads[i].index = links.pads[i].index;
            entity.pads[i].flags = links.pads[i].flags;
        }

        for (i = 0; i < entity.info.links; ++i) {
            media_link_desc* link = &links.links[i];
            MediaLink* fwdlink;
            MediaLink* backlink;
            MediaEntity* source;
            MediaEntity* sink;

            source = getEntityById(link->source.entity);
            sink = getEntityById(link->sink.entity);

            if (source == nullptr || sink == nullptr) {
                LOG1("WARNING entity %u link %u src %u/%u to %u/%u is invalid!", entity.info.id, i,
                     link->source.entity, link->source.index, link->sink.entity, link->sink.index);
                ret = -EINVAL;
            } else {
                fwdlink = entityAddLink(source);
                if (fwdlink) {
                    fwdlink->source = &source->pads[link->source.index];
                    fwdlink->sink = &sink->pads[link->sink.index];
                    fwdlink->flags = link->flags;
                }

                backlink = entityAddLink(sink);
                if (backlink) {
                    backlink->source = &source->pads[link->source.index];
                    backlink->sink = &sink->pads[link->sink.index];
                    backlink->flags = link->flags;
                }

                if (fwdlink) fwdlink->twin = backlink;
                if (backlink) backlink->twin = fwdlink;
            }
        }

        delete[] links.pads;
        delete[] links.links;
    }

    return ret;
}

MediaLink* MediaControl::entityAddLink(MediaEntity* entity) {
    if (entity->numLinks >= entity->maxLinks) {
        uint32_t maxLinks = entity->maxLinks * 2;
        MediaLink* links = new MediaLink[maxLinks];

        MEMCPY_S(links, sizeof(MediaLink) * maxLinks, entity->links,
                 sizeof(MediaLink) * entity->maxLinks);
        delete[] entity->links;

        for (uint32_t i = 0; i < entity->numLinks; ++i) {
            links[i].twin->twin = &links[i];
        }

        entity->maxLinks = maxLinks;
        entity->links = links;
    }

    return &entity->links[entity->numLinks++];
}

MediaEntity* MediaControl::getEntityById(uint32_t id) {
    bool next = id & MEDIA_ENT_ID_FLAG_NEXT;

    id &= ~MEDIA_ENT_ID_FLAG_NEXT;

    for (uint32_t i = 0; i < mEntities.size(); i++) {
        if ((mEntities[i].info.id == id && !next) || (mEntities[0].info.id > id && next)) {
            return &mEntities[i];
        }
    }

    return nullptr;
}

const char* MediaControl::entitySubtype2String(unsigned type) {
    static const char* nodeTypes[] = {
        "Unknown", "V4L", "FB", "ALSA", "DVB",
    };
    static const char* subdevTypes[] = {
        "Unknown",
        "Sensor",
        "Flash",
        "Lens",
    };

    uint32_t subtype = type & MEDIA_ENT_SUBTYPE_MASK;

    switch (type & MEDIA_ENT_TYPE_MASK) {
        case MEDIA_ENT_T_DEVNODE:
            if (subtype >= ARRAY_SIZE(nodeTypes)) subtype = 0;
            return nodeTypes[subtype];

        case MEDIA_ENT_T_V4L2_SUBDEV:
            if (subtype >= ARRAY_SIZE(subdevTypes)) subtype = 0;
            return subdevTypes[subtype];
        default:
            return nodeTypes[0];
    }
}

const char* MediaControl::padType2String(unsigned flag) {
    static const struct {
        __u32 flag;
        const char* name;
    } flags[] = {
        {MEDIA_PAD_FL_SINK, "Sink"},
        {MEDIA_PAD_FL_SOURCE, "Source"},
    };

    uint32_t i;

    for (i = 0; i < ARRAY_SIZE(flags); i++) {
        if (flags[i].flag & flag) return flags[i].name;
    }

    return "Unknown";
}

void MediaControl::setMediaMcCtl(int cameraId, vector<McCtl> ctls) {
    setSensorOrientation(cameraId);

    for (auto& ctl : ctls) {
        MediaEntity* entity = getEntityById(ctl.entity);
        V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, entity->devname);
        LOG1("set Ctl %s [%d] cmd %s [0x%08x] value %d", ctl.entityName.c_str(), ctl.entity,
             ctl.ctlName.c_str(), ctl.ctlCmd, ctl.ctlValue);
        if (subDev->SetControl(ctl.ctlCmd, ctl.ctlValue) != OK) {
            LOGW("set Ctl %s [%d] cmd %s [0x%08x] value %d failed.", ctl.entityName.c_str(),
                 ctl.entity, ctl.ctlName.c_str(), ctl.ctlCmd, ctl.ctlValue);
        }
    }
}

int MediaControl::setMediaMcLink(vector<McLink> links) {
    for (auto& link : links) {
        LOG1("setup Link %s [%d:%d] ==> %s [%dx%d] enable %d.", link.srcEntityName.c_str(),
             link.srcEntity, link.srcPad, link.sinkEntityName.c_str(), link.sinkEntity,
             link.sinkPad, link.enable);
        int ret =
            setupLink(link.srcEntity, link.srcPad, link.sinkEntity, link.sinkPad, link.enable);
        CheckAndLogError(ret < 0, ret, "setup Link %s [%d:%d] ==> %s [%dx%d] enable %d failed.",
                         link.srcEntityName.c_str(), link.srcEntity, link.srcPad,
                         link.sinkEntityName.c_str(), link.sinkEntity, link.sinkPad, link.enable);
    }
    return OK;
}

int MediaControl::setFormat(int cameraId, const McFormat* format, int targetWidth, int targetHeight,
                            int field) {
    PERF_CAMERA_ATRACE();
    int ret;
    v4l2_mbus_framefmt mbusfmt;
    MediaEntity* entity = getEntityById(format->entity);
    CheckAndLogError(!entity, BAD_VALUE, "Get entity fail for calling getEntityById");

    MediaPad* pad = &entity->pads[format->pad];
    V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, entity->devname);
    LOG1("SENSORCTRLINFO: width=%d, height=%d, code=0x%x", targetWidth, targetHeight,
         format->pixelCode);

    CLEAR(mbusfmt);
    if (format->width != 0 && format->height != 0) {
        mbusfmt.width = format->width;
        mbusfmt.height = format->height;
    } else if (format->type == RESOLUTION_TARGET) {
        mbusfmt.width = targetWidth;
        mbusfmt.height = targetHeight;
    }
    mbusfmt.field = field;

    if (format->pixelCode) {
        mbusfmt.code = format->pixelCode;
    } else {
        mbusfmt.code = CameraUtils::getMBusFormat(cameraId, PlatformData::getISysFormat(cameraId));
    }
    LOG1("set format %s [%d:%d/%d] [%dx%d] [%dx%d] %s ", format->entityName.c_str(), format->entity,
         format->pad, format->stream, mbusfmt.width, mbusfmt.height, targetWidth, targetHeight,
         CameraUtils::pixelCode2String(mbusfmt.code));

    struct v4l2_subdev_format fmt = {};
    fmt.pad = format->pad;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    fmt.format = mbusfmt;
    // VIRTUAL_CHANNEL_S
    fmt.stream = format->stream;
    // VIRTUAL_CHANNEL_E
    ret = subDev->SetFormat(fmt);
    CheckAndLogError(ret < 0, BAD_VALUE, "set format %s [%d:%d] [%dx%d] %s failed.",
                     format->entityName.c_str(), format->entity, format->pad, format->width,
                     format->height, CameraUtils::pixelCode2String(format->pixelCode));

    mbusfmt = fmt.format;

    /* If the pad is an output pad, automatically set the same format on
     * the remote subdev input pads, if any.
     */
    if (pad->flags & MEDIA_PAD_FL_SOURCE) {
        for (unsigned int i = 0; i < pad->entity->numLinks; ++i) {
            MediaLink* link = &pad->entity->links[i];

            if (!(link->flags & MEDIA_LNK_FL_ENABLED)) continue;

            if (link->source == pad && link->sink->entity->info.type == MEDIA_ENT_T_V4L2_SUBDEV) {
                auto subDev = V4l2DeviceFactory::getSubDev(cameraId, link->sink->entity->devname);

                struct v4l2_subdev_format tmt = {};
                tmt.format = mbusfmt;
                tmt.pad = link->sink->index;
                tmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
                subDev->SetFormat(tmt);
            }
        }
    }

    return 0;
}

int MediaControl::setSelection(int cameraId, const McFormat* format, int targetWidth,
                               int targetHeight) {
    PERF_CAMERA_ATRACE();
    int ret = OK;

    MediaEntity* entity = getEntityById(format->entity);
    V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, entity->devname);
    LOG1("<id%d> @%s, targetWidth:%d, targetHeight:%d", cameraId, __func__, targetWidth,
         targetHeight);

    if (format->top != -1 && format->left != -1 && format->width != 0 && format->height != 0) {
        struct v4l2_subdev_selection selection = {};
        selection.pad = format->pad;
        selection.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        selection.target = format->selCmd;
        selection.flags = 0;
        selection.r.top = format->top;
        selection.r.left = format->left;
        selection.r.width = format->width;
        selection.r.height = format->height;

        ret = subDev->SetSelection(selection);
    } else if (format->selCmd == V4L2_SEL_TGT_CROP || format->selCmd == V4L2_SEL_TGT_COMPOSE) {
        struct v4l2_subdev_selection selection = {};
        selection.pad = format->pad;
        selection.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        selection.target = format->selCmd;
        selection.flags = 0;
        selection.r.top = 0;
        selection.r.left = 0;
        selection.r.width = targetWidth;
        selection.r.height = targetHeight;

        ret = subDev->SetSelection(selection);
    } else {
        ret = BAD_VALUE;
    }

    CheckAndLogError(ret < 0, BAD_VALUE,
                     "set selection %s [%d:%d] selCmd: %d [%d, %d] [%dx%d] failed",
                     format->entityName.c_str(), format->entity, format->pad, format->selCmd,
                     format->top, format->left, format->width, format->height);

    return OK;
}

int MediaControl::mediaCtlSetup(int cameraId, MediaCtlConf* mc, int width, int height, int field) {
    LOG1("<id%d> %s", cameraId, __func__);
    /* Setup controls in format Configuration */
    setMediaMcCtl(cameraId, mc->ctls);

    int ret = OK;
    // VIRTUAL_CHANNEL_S
    /* Set routing */
    for (auto& route : mc->routes) {
        LOG1("<id%d> route entity:%s, sinkPad:%d, srcPad:%d, sinkStream:%d, srcStream:%d, flag:%d",
             cameraId, route.entityName.c_str(), route.sinkPad, route.srcPad, route.sinkStream,
             route.srcStream, route.flag);

        string subDeviceNodeName;
        CameraUtils::getSubDeviceName(route.entityName.c_str(), subDeviceNodeName);
        V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, subDeviceNodeName);
        v4l2_subdev_route r = {route.sinkPad, route.sinkStream, route.srcPad, route.srcStream,
                               route.flag};
        ret = subDev->SetRouting(&r, 1);
        CheckAndLogError(ret != 0, ret, "setRouting fail, ret:%d", ret);
    }
    // VIRTUAL_CHANNEL_E

    /* Set format & selection in format Configuration */
    for (auto& fmt : mc->formats) {
        if (fmt.formatType == FC_FORMAT) {
            setFormat(cameraId, &fmt, width, height, field);
        } else if (fmt.formatType == FC_SELECTION) {
            setSelection(cameraId, &fmt, width, height);
        }
    }

    MediaEntity* ivsc = getEntityByName(ivscName.c_str());
    if (ivsc) {
        for (uint32_t i = 0; i < ivsc->numLinks; ++i) {
            if (ivsc->links[i].sink->entity == ivsc) {
                MediaEntity* sensor = ivsc->links[i].source->entity;
                int sensor_entity_id = sensor->info.id;
                LOG1("@%s, found %s -> %s", __func__,
                     sensor->info.name, ivscName.c_str());
                for (McLink& link : mc->links) {
                    if (link.srcEntity == sensor_entity_id) {
                        LOG1("@%s, skip %s, link %s -> %s",
                             __func__, link.srcEntityName.c_str(),
                             ivscName.c_str(), link.sinkEntityName.c_str());
                        link.srcEntity = ivsc->info.id;
                        link.srcEntityName = ivscName;
                        break;
                    }
                }
                break;
            }
        }
    }

    /* Set link in format Configuration */
    ret = setMediaMcLink(mc->links);
    CheckAndLogError(ret != OK, ret, "set MediaCtlConf McLink failed: ret = %d", ret);

    // DUMP_ENTITY_TOPOLOGY_S
    dumpEntityTopology();
    // DUMP_ENTITY_TOPOLOGY_E

    return OK;
}

int MediaControl::getVCMI2CAddr(const char* vcmName, string* vcmI2CAddr) {
    CheckAndLogError(!vcmI2CAddr, BAD_VALUE, "vcmI2CAddr is nullptr");
    CheckAndLogError(!vcmName, BAD_VALUE, "vcmName is nullptr");

    for (auto& entity : mEntities) {
        if (strncmp(entity.info.name, vcmName, strlen(vcmName)) == 0) {
            *vcmI2CAddr = entity.info.name;
            LOG1("%s, vcm addr name %s", __func__, entity.info.name);
            return OK;
        }
    }

    return NAME_NOT_FOUND;
}

void MediaControl::mediaCtlClear(int cameraId, MediaCtlConf* mc) {
    LOG1("<id%d> %s", cameraId, __func__);

    // VIRTUAL_CHANNEL_S
    /* Clear routing */
    for (auto& route : mc->routes) {
        string subDeviceNodeName;
        CameraUtils::getSubDeviceName(route.entityName.c_str(), subDeviceNodeName);
        V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, subDeviceNodeName);
        v4l2_subdev_route r = {route.sinkPad, route.sinkStream, route.srcPad, route.srcStream,
                               route.flag & ~V4L2_SUBDEV_ROUTE_FL_ACTIVE};
        int ret = subDev->SetRouting(&r, 1);
        CheckAndLogError(ret != 0, VOID_VALUE, "Clear routing fail, ret:%d", ret);
    }
    // VIRTUAL_CHANNEL_E
}

// This function must be called after enumEntities().
int MediaControl::getLensName(string* lensName) {
    CheckAndLogError(!lensName, UNKNOWN_ERROR, "lensName is nullptr");

    for (auto& entity : mEntities) {
        if (entity.info.type == MEDIA_ENT_T_V4L2_SUBDEV_LENS) {
            *lensName = entity.info.name;
            return OK;
        }
    }

    return UNKNOWN_ERROR;
}

// This function must be called after enumEntities().
bool MediaControl::checkAvailableSensor(const std::string& sensorEntityName,
                                        const std::string& sinkEntityName) {
    LOG1("@%s, sensorEntityName:%s, sinkEntityName:%s", __func__, sensorEntityName.c_str(),
         sinkEntityName.c_str());

    std::string sensorEntityNameTmp = sensorEntityName;
    sensorEntityNameTmp.append(" ");
    size_t nameLen = sensorEntityNameTmp.length();
    for (auto& entity : mEntities) {
        int linksCount = entity.info.links;
        MediaLink* links = entity.links;
        for (int i = 0; i < linksCount; i++) {
            if (strcmp(links[i].sink->entity->info.name, sinkEntityName.c_str()) == 0 ||
                strcmp(links[i].sink->entity->info.name, ivscName.c_str()) == 0) {
                char* entityName = entity.info.name;
                if (strncmp(entityName, sensorEntityNameTmp.c_str(), nameLen) == 0) {
                    return true;
                }
            }
        }
    }

    return false;
}

// This function must be called after enumEntities().
int MediaControl::getI2CBusAddress(const string& sensorEntityName, const string& sinkEntityName,
                                   string* i2cBus) {
    LOG1("@%s, sensorEntityName:%s, sinkEntityName:%s", __func__, sensorEntityName.c_str(),
         sinkEntityName.c_str());
    CheckAndLogError(!i2cBus, UNKNOWN_ERROR, "i2cBus is nullptr");

    for (auto& entity : mEntities) {
        int linksCount = entity.info.links;
        MediaLink* links = entity.links;
        char* entityName = nullptr;
        size_t sensorEntityNameLen = sensorEntityName.length();
        for (int i = 0; i < linksCount; i++) {
            if (strcmp(links[i].sink->entity->info.name, sinkEntityName.c_str()) == 0) {
                entityName = entity.info.name;
                if (strcmp(entityName, ivscName.c_str()) == 0) {
                    return getI2CBusAddress(sensorEntityName, ivscName, i2cBus);
                }
                break;
            }
        }

        // entityName example: "imx319 10-0010", sensorEntityName example: "imx319"
        if (entityName && (strlen(entityName) > (sensorEntityNameLen + 1))) {
            *i2cBus = entityName + sensorEntityNameLen + 1;
            LOG1("i2cBus is %s", i2cBus->c_str());
            return OK;
        }
    }

    return UNKNOWN_ERROR;
}

// DUMP_ENTITY_TOPOLOGY_S
void MediaControl::dumpTopologyDot() {
    printf("digraph board {\n");
    printf("\trankdir=TB\n");

    for (auto& entity : mEntities) {
        const media_entity_desc* info = &entity.info;
        const char* devname = (entity.devname[0] ? entity.devname : nullptr);
        uint32_t numLinks = entity.numLinks;
        uint32_t npads;
        UNUSED(npads);

        switch (info->type & MEDIA_ENT_TYPE_MASK) {
            case MEDIA_ENT_T_DEVNODE:
                // Although printf actually can print NULL pointer, but make check
                // to make KW happy.
                if (devname)
                    printf("\tn%08x [label=\"%s\\n%s\", shape=box, style=filled, "
                           "fillcolor=yellow]\n",
                           info->id, info->name, devname);
                break;

            case MEDIA_ENT_T_V4L2_SUBDEV:
                printf("\tn%08x [label=\"{{", info->id);

                for (int i = 0, npads = 0; i < info->pads; ++i) {
                    MediaPad* pad = entity.pads + i;

                    if (!(pad->flags & MEDIA_PAD_FL_SINK)) continue;

                    printf("%s<port%d> %d", npads ? " | " : "", i, i);
                    npads++;
                }

                printf("} | %s", info->name);
                if (devname) printf("\\n%s", devname);
                printf(" | {");

                for (int i = 0, npads = 0; i < info->pads; ++i) {
                    MediaPad* pad = entity.pads + i;

                    if (!(pad->flags & MEDIA_PAD_FL_SOURCE)) continue;

                    printf("%s<port%d> %d", npads ? " | " : "", i, i);
                    npads++;
                }

                printf("}}\", shape=Mrecord, style=filled, fillcolor=green]\n");
                break;

            default:
                continue;
        }

        for (uint32_t i = 0; i < numLinks; i++) {
            MediaLink* link = entity.links + i;
            MediaPad* source = link->source;
            MediaPad* sink = link->sink;

            /*Only print the forward links of the entity*/
            if (source->entity != &entity) continue;

            printf("\tn%08x", source->entity->info.id);
            if ((source->entity->info.type & MEDIA_ENT_TYPE_MASK) == MEDIA_ENT_T_V4L2_SUBDEV)
                printf(":port%u", source->index);
            printf(" -> ");
            printf("n%08x", sink->entity->info.id);
            if ((sink->entity->info.type & MEDIA_ENT_TYPE_MASK) == MEDIA_ENT_T_V4L2_SUBDEV)
                printf(":port%u", sink->index);

            if (link->flags & MEDIA_LNK_FL_IMMUTABLE)
                printf(" [style=bold]");
            else if (!(link->flags & MEDIA_LNK_FL_ENABLED))
                printf(" [style=dashed]");
            printf("\n");
        }
    }

    printf("}\n");
}

void MediaControl::dumpTopologyText() {
    static const struct {
        __u32 flag;
        const char* name;
    } link_flags[] = {
        {MEDIA_LNK_FL_ENABLED, "ENABLED"},
        {MEDIA_LNK_FL_IMMUTABLE, "IMMUTABLE"},
        {MEDIA_LNK_FL_DYNAMIC, "DYNAMIC"},
    };

    printf("Device topology\n");

    for (auto& entity : mEntities) {
        const media_entity_desc* info = &entity.info;
        const char* devname = (entity.devname[0] ? entity.devname : nullptr);
        uint32_t numLinks = entity.numLinks;

        uint32_t padding = printf("- entity %u: ", info->id);
        printf("%s (%u pad%s, %u link%s)\n", info->name, info->pads, info->pads > 1 ? "s" : "",
               numLinks, numLinks > 1 ? "s" : "");
        printf("%*ctype %s subtype %s flags %x\n", padding, ' ', padType2String(info->type),
               entitySubtype2String(info->type), info->flags);
        if (devname) printf("%*cdevice node name %s\n", padding, ' ', devname);

        for (int i = 0; i < info->pads; i++) {
            MediaPad* pad = entity.pads + i;

            printf("\tpad%d: %s\n", i, padType2String(pad->flags));

            /*
             *if ((info->type & MEDIA_ENT_TYPE_MASK) == MEDIA_ENT_T_V4L2_SUBDEV)
             *v4l2_subdev_print_format(entity, i, V4L2_SUBDEV_FORMAT_ACTIVE);
             */
            for (uint32_t j = 0; j < numLinks; j++) {
                MediaLink* link = entity.links + j;
                MediaPad* source = link->source;
                MediaPad* sink = link->sink;
                bool first = true;

                if (source->entity == &entity && source->index == j)
                    printf("\t\t-> \"%s\":%u [", sink->entity->info.name, sink->index);
                else if (sink->entity == &entity && sink->index == j)
                    printf("\t\t<- \"%s\":%u [", source->entity->info.name, source->index);
                else
                    continue;

                for (uint32_t k = 0; k < ARRAY_SIZE(link_flags); k++) {
                    if (!(link->flags & link_flags[k].flag)) continue;
                    if (!first) printf(",");
                    printf("%s", link_flags[k].name);
                    first = false;
                }

                printf("]\n");
            }
        }
        printf("\n");
    }
}

void MediaControl::dumpEntityTopology(bool dot) {
    if (Log::isDumpMediaTopo()) {
        if (dot)
            dumpTopologyDot();
        else
            dumpTopologyText();
    }
}
// DUMP_ENTITY_TOPOLOGY_E

void MediaControl::setSensorOrientation(int cameraId) {
    int orientation = icamera::PlatformData::getSensorOrientation(cameraId);

    if (orientation != ORIENTATION_180) {
        LOG1("@%s, orientation %d do not supported currently", __func__, orientation);
        return;
    }

    std::string subDevName;
    PlatformData::getDevNameByType(cameraId, VIDEO_PIXEL_ARRAY, subDevName);
    LOG1("@%s, sub-dev name is %s", __func__, subDevName.c_str());
    V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, subDevName);
    if ((subDev->SetControl(V4L2_CID_HFLIP, 1) == OK) &&
        (subDev->SetControl(V4L2_CID_VFLIP, 1) == OK)) {
        LOG1("@%s, IOCTL V4L2_CID_HFLIP/VFLIP OK", __func__);
    } else {
        LOGE("Cannot set sensor orientation to %d.", orientation);
    }
}

}  // namespace icamera
