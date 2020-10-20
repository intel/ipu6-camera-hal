/*
 * GStreamer
 * Copyright (C) 2015-2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#define LOG_TAG "GstCameraSrcBufferPool"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "gst/allocators/gstdmabuf.h"
#include "gst/video/video.h"
#include "gst/video/gstvideometa.h"
#include "gst/video/gstvideopool.h"

#include "ICamera.h"
#include "ScopedAtrace.h"

#include "gstcameradeinterlace.h"
#include "gstcamerasrcbufferpool.h"
#include "gstcamerasrc.h"
#include <iostream>
#include <time.h>
#include <queue>
#include "utils.h"

using namespace icamera;
using std::queue;

GType
gst_camerasrc_meta_api_get_type (void)
{
  PERF_CAMERA_ATRACE();
  static volatile GType type;
  static const gchar *tags[] = { "memory", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("GstCamerasrcMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

const GstMetaInfo *
gst_camerasrc_meta_get_info (void)
{
  PERF_CAMERA_ATRACE();
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_camerasrc_meta_api_get_type (), "GstCamerasrcMeta",
        sizeof (GstCamerasrcMeta), (GstMetaInitFunction) NULL,
        (GstMetaFreeFunction) NULL, (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

/*
 * GstICGCAMBufferPool:
 */
#define gst_camerasrc_buffer_pool_parent_class parent_class
G_DEFINE_TYPE (GstCamerasrcBufferPool, gst_camerasrc_buffer_pool, GST_TYPE_BUFFER_POOL);

static gboolean gst_camerasrc_buffer_pool_set_config (GstBufferPool * bpool, GstStructure * config);
static gboolean gst_camerasrc_buffer_pool_start(GstBufferPool * bpool);
static gboolean gst_camerasrc_buffer_pool_stop(GstBufferPool *bpool);
static GstFlowReturn gst_camerasrc_buffer_pool_alloc_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params);
static void gst_camerasrc_buffer_pool_release_buffer (GstBufferPool * bpool, GstBuffer * buffer);
static GstFlowReturn gst_camerasrc_buffer_pool_acquire_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params);
static void gst_camerasrc_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer);
static void gst_camerasrc_free_weave_buffer (Gstcamerasrc *src, int stream_id);

static void
gst_camerasrc_buffer_pool_finalize (GObject * object)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL (object);
  GST_INFO("CameraId=%d, StreamId=%d.", pool->src->device_id, pool->stream_id);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_camerasrc_buffer_pool_class_init(GstCamerasrcBufferPoolClass *klass)
{
  PERF_CAMERA_ATRACE();
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_camerasrc_buffer_pool_finalize;

  bufferpool_class->start = gst_camerasrc_buffer_pool_start;
  bufferpool_class->stop = gst_camerasrc_buffer_pool_stop;
  bufferpool_class->set_config = gst_camerasrc_buffer_pool_set_config;
  bufferpool_class->alloc_buffer = gst_camerasrc_buffer_pool_alloc_buffer;
  bufferpool_class->acquire_buffer = gst_camerasrc_buffer_pool_acquire_buffer;
  bufferpool_class->release_buffer = gst_camerasrc_buffer_pool_release_buffer;
  bufferpool_class->free_buffer = gst_camerasrc_buffer_pool_free_buffer;
}

static void
gst_camerasrc_buffer_pool_init (GstCamerasrcBufferPool * pool)
{
  pool->number_allocated = 0;
  pool->acquire_buffer_index = 0;
  pool->alloc_done = FALSE;
}

GstBufferPool *
gst_camerasrc_buffer_pool_new (Gstcamerasrc *camerasrc,
      GstCaps *caps, int stream_id)
{
  PERF_CAMERA_ATRACE();
  GST_INFO("CameraId=%d StreamId=%d.", camerasrc->device_id, stream_id);
  int bpp = 0;

  GstCamerasrcBufferPool *pool = (GstCamerasrcBufferPool *) g_object_new (GST_TYPE_CAMERASRC_BUFFER_POOL, NULL);

  /* Get format bpp and actual frame size from HAL */
  int frame_size = get_frame_size(camerasrc->device_id, camerasrc->s[stream_id].format, camerasrc->s[stream_id].width,
                   camerasrc->s[stream_id].height, camerasrc->s[stream_id].field, &bpp);
  if (frame_size != camerasrc->s[stream_id].size) {
      GST_WARNING("CameraId=%d, StreamId=%d, hal_frame_size:%d is not equal to stream_size:%d, will be overwriten",
                       camerasrc->device_id, stream_id, frame_size, camerasrc->s[stream_id].size);
      camerasrc->s[stream_id].size = frame_size;
  }

  pool->src = camerasrc;
  /* each buffer pool has its own stream Id */
  pool->stream_id = stream_id;
  camerasrc->streams[stream_id].pool = GST_BUFFER_POOL(pool);

  GstStructure *s = gst_buffer_pool_get_config (GST_BUFFER_POOL_CAST (pool));
  gst_buffer_pool_config_set_params (s, caps, camerasrc->s[stream_id].size,
                   MIN_PROP_BUFFERCOUNT, MAX_PROP_BUFFERCOUNT);
  gst_buffer_pool_set_config (GST_BUFFER_POOL_CAST (pool), s);

  /* init buffer queue */
  camerasrc->streams[stream_id].buffer_queue = new queue<camera_buffer_t*>;

  GST_INFO("CameraId=%d, StreamId=%d Buffer pool config: min buffers=%d, max buffers=%d, buffer bpl=%d, bpp=%d, size=%d",
                   camerasrc->device_id, stream_id, MIN_PROP_BUFFERCOUNT, MAX_PROP_BUFFERCOUNT,
                   camerasrc->streams[stream_id].bpl, bpp, camerasrc->s[stream_id].size);
  return GST_BUFFER_POOL (pool);
}

static gboolean
gst_camerasrc_buffer_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL(bpool);
  Gstcamerasrc *camerasrc = pool->src;
  GST_INFO("CameraId=%d, StreamId=%d.", camerasrc->device_id, pool->stream_id);

  GstAllocator *allocator;
  GstAllocationParams params;
  GstCaps *caps;
  guint size, min_buffers, max_buffers;

  // parse the config and keep around
  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
        &max_buffers)) {
    GST_ERROR("CameraId=%d, StreamId=%d failed to parse buffer pool config.",
      camerasrc->device_id, pool->stream_id);
    return FALSE;
  }

  if (!gst_buffer_pool_config_get_allocator (config, &allocator, &params)) {
    GST_ERROR("CameraId=%d, StreamId=%d failed to get buffer pool allocator.",
      camerasrc->device_id, pool->stream_id);
    return FALSE;
  }

  if (pool->allocator)
    gst_object_unref (pool->allocator);
  pool->allocator = NULL;

  if (camerasrc->io_mode == GST_CAMERASRC_IO_MODE_DMA_EXPORT) {
    pool->allocator = gst_dmabuf_allocator_new ();
  } else {
    pool->allocator = allocator;
  }

  pool->params = params;
  pool->size = size;
  pool->number_of_buffers = camerasrc->number_of_buffers;

  return GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);
}

static void
gst_camerasrc_init_fps (Gstcamerasrc * src, int stream_id)
{
  src->streams[stream_id].fps_debug.buf_count = 0;
  src->streams[stream_id].fps_debug.last_buf_count = 0;
  src->streams[stream_id].fps_debug.sum_time = 0;
  src->streams[stream_id].fps_debug.tm_interval = 0;
  src->streams[stream_id].fps_debug.max_fps = 0;
  src->streams[stream_id].fps_debug.min_fps = 0;
  src->streams[stream_id].fps_debug.av_fps = 0;
  src->streams[stream_id].fps_debug.init_max_min_fps = true;
}

static gboolean
gst_camerasrc_buffer_pool_start (GstBufferPool * bpool)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL(bpool);
  Gstcamerasrc *camerasrc = pool->src;
  int stream_id = pool->stream_id;
  int count = 0;
  GST_INFO("CameraId=%d, StreamId=%d.", camerasrc->device_id, pool->stream_id);

  if (camerasrc->print_fps)
    gst_camerasrc_init_fps(camerasrc, stream_id);

  pool->buffers = g_new0 (GstBuffer *, pool->number_of_buffers);
  GST_INFO("CameraId=%d, StreamId=%d start pool %p, Thread ID=%ld, number of buffers in pool=%d.",
    camerasrc->device_id, pool->stream_id, pool, gettid(), pool->number_of_buffers);

  /* now, allocate the buffers */
  if (!GST_BUFFER_POOL_CLASS (parent_class)->start (bpool)) {
    return FALSE;
  }

  GST_CAMSRC_LOCK(camerasrc);
  GST_INFO("CameraId=%d, StreamId=%d pool is activated %p.",
    camerasrc->device_id, pool->stream_id, pool);

  /* interface:camera_device_start() can only be called once, hence the last stream that
   * calls this function should do that, otherwise device would start when other thread
   * is not ready */
  count = camerasrc->stream_start_count--;

  /* when count drops to 1, means that this's the last thread that calls this function */
  if (count == 1) {
    camera_device_start(camerasrc->device_id);
    GST_INFO("CameraId=%d StreamId=%d Stream count=%d, Bufferpool alloc done, ready to start streaming.",
      camerasrc->device_id, stream_id, count);
    camerasrc->start_streams = TRUE;
    GST_CAMSRC_BROADCAST(camerasrc);
  } else {
    while(!camerasrc->start_streams) {
      GST_INFO("CameraId=%d, StreamId=%d Stream count=%d, wait for stream start signal.",
        camerasrc->device_id, stream_id, count);
      GST_CAMSRC_WAIT(camerasrc);
    }
  }
  GST_CAMSRC_UNLOCK(camerasrc);

  return TRUE;
}

static int
gst_camerasrc_alloc_weave_buffer(Gstcamerasrc *camerasrc, gint size)
{
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL_CAST(camerasrc);
  int stream_id = pool->stream_id;
  int ret = 0;

  if (camerasrc->interlace_field == GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE &&
        camerasrc->deinterlace_method == GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE) {
    GST_DEBUG("CameraId=%d, StreamId=%d allocate top and bottom buffers to do buffer weaving.",
      camerasrc->device_id, pool->stream_id);
    if (camerasrc->streams[stream_id].top == NULL) {
      camerasrc->streams[stream_id].top = (camera_buffer_t *)calloc(1, sizeof(camera_buffer_t));
      if (camerasrc->streams[stream_id].top == NULL) {
        GST_ERROR("CameraId=%d, StreamId=%d failed to alloc topfield buffer.",
          camerasrc->device_id, pool->stream_id);
        return GST_FLOW_ERROR;
      }

      ret = posix_memalign(&camerasrc->streams[stream_id].top->addr, getpagesize(), size);
      if (ret < 0) {
        GST_ERROR("CameraId=%d, StreamId=%d topfield buffer memalign error.",
          camerasrc->device_id, pool->stream_id);
        return GST_FLOW_ERROR;
      }
    }

    if (camerasrc->streams[stream_id].bottom == NULL) {
      camerasrc->streams[stream_id].bottom = (camera_buffer_t *)calloc(1, sizeof(camera_buffer_t));
      if (camerasrc->streams[stream_id].bottom == NULL) {
        GST_ERROR("CameraId=%d, StreamId=%d failed to alloc bottomfield buffer.",
          camerasrc->device_id, pool->stream_id);
        return GST_FLOW_ERROR;
      }

      ret = posix_memalign(&camerasrc->streams[stream_id].bottom->addr, getpagesize(), size);
      if (ret < 0) {
        GST_ERROR("CameraId=%d, StreamId=%d bottomfield buffer memalign error.",
          camerasrc->device_id, pool->stream_id);
        return GST_FLOW_ERROR;
      }
    }

    camerasrc->streams[stream_id].previous_sequence = 0;
  }

  return ret;
}

static int
gst_camerasrc_alloc_userptr(GstCamerasrcBufferPool *pool,
      GstBuffer **alloc_buffer, GstCamerasrcMeta **meta)
{
  Gstcamerasrc *src = pool->src;
  GST_DEBUG("CameraId=%d, StreamId=%d allocate userptr buffer.",
    src->device_id, pool->stream_id);

  *alloc_buffer = gst_buffer_new();
  *meta = GST_CAMERASRC_META_ADD(*alloc_buffer);
  (*meta)->buffer = (camera_buffer_t *)calloc(1, sizeof(camera_buffer_t));
  if ((*meta)->buffer == NULL)
    return GST_FLOW_ERROR;

  /* Allocate temp buffers: top(only top field), bottom(only bottom field)
   * in order to do deinterlace weaving and fill in metadata, also allocate buffer
   * to store the data of previous buffer to update top and bottom buffers
   * in case of inconsecutive buffer sequence */
  int ret = gst_camerasrc_alloc_weave_buffer(src, pool->size);
  if (ret < 0) {
    GST_ERROR("CameraId=%d, StreamId=%d failed to alloc weave buffers.",
      src->device_id, pool->stream_id);
    return GST_FLOW_ERROR;
  }

  (*meta)->buffer->s = src->s[pool->stream_id];
  (*meta)->buffer->s.memType = V4L2_MEMORY_USERPTR;
  (*meta)->buffer->flags = 0;
  ret = posix_memalign(&(*meta)->buffer->addr, getpagesize(), pool->size);

  if (ret < 0) {
    GST_ERROR("CameraId=%d, StreamId=%d userptr buffer memalign error.",
    src->device_id, pool->stream_id);
    return GST_FLOW_ERROR;
  }

  (*meta)->mem = (*meta)->buffer->addr;
  gst_buffer_append_memory (*alloc_buffer,
           gst_memory_new_wrapped (GST_MEMORY_FLAG_NO_SHARE,
           (*meta)->mem, pool->size, 0, pool->size, NULL, NULL));

  return GST_FLOW_OK;
}

static int
gst_camerasrc_alloc_mmap(GstCamerasrcBufferPool *pool,
      GstBuffer **alloc_buffer, GstCamerasrcMeta **meta)
{
  Gstcamerasrc *src = pool->src;
  GST_DEBUG("CameraId=%d, StreamId=%d allocate mmap buffer.",
    src->device_id, pool->stream_id);

  *alloc_buffer = gst_buffer_new();
  *meta = GST_CAMERASRC_META_ADD(*alloc_buffer);
  (*meta)->buffer = (camera_buffer_t *)calloc(1, sizeof(camera_buffer_t));
  if ((*meta)->buffer == NULL)
    return GST_FLOW_ERROR;

  (*meta)->buffer->s = src->s[pool->stream_id];
  (*meta)->buffer->s.memType = V4L2_MEMORY_MMAP;
  (*meta)->buffer->flags = 0;

  int ret = camera_device_allocate_memory(src->device_id, (*meta)->buffer);
  if (ret < 0) {
    GST_ERROR("CameraId=%d, StreamId=%d failed to alloc memory for mmap buffer.",
      src->device_id, pool->stream_id);
    return GST_FLOW_ERROR;
  }

  (*meta)->mem = (*meta)->buffer->addr;
  gst_buffer_append_memory (*alloc_buffer,
           gst_memory_new_wrapped (GST_MEMORY_FLAG_NO_SHARE, (*meta)->mem, pool->size, 0, pool->size, NULL, NULL));

  return GST_FLOW_OK;
}

static gboolean
gst_camerasrc_is_dma_buffer (GstBuffer *buf)
{
  GstMemory *mem;

  if (gst_buffer_n_memory (buf) < 1) {
    GST_ERROR("the amount of memory blocks is smaller than 1.");
    return FALSE;
  }

  mem = gst_buffer_peek_memory (buf, 0);
  if (!mem || !gst_is_dmabuf_memory (mem))
    return FALSE;

  return TRUE;
}

static int
gst_camerasrc_alloc_dma_export(GstCamerasrcBufferPool *pool,
      GstBuffer **alloc_buffer, GstCamerasrcMeta **meta)
{
  Gstcamerasrc *src = pool->src;
  GST_DEBUG("CameraId=%d, StreamId=%d allocate DMA export buffer.",
    src->device_id, pool->stream_id);

  GstMemory *mem = NULL;
  *alloc_buffer = gst_buffer_new();
  *meta = GST_CAMERASRC_META_ADD(*alloc_buffer);
  (*meta)->buffer = (camera_buffer_t *)calloc(1, sizeof(camera_buffer_t));
  if ((*meta)->buffer == NULL)
    return GST_FLOW_ERROR;

  (*meta)->buffer->s = src->s[pool->stream_id];
  (*meta)->buffer->s.memType = V4L2_MEMORY_MMAP;
  (*meta)->buffer->flags = BUFFER_FLAG_DMA_EXPORT;
  int ret = camera_device_allocate_memory(src->device_id, (*meta)->buffer);
  if (ret < 0) {
    GST_ERROR("CameraId=%d, StreamId=%d failed to alloc memory for dma export buffer.",
      src->device_id, pool->stream_id);
    return GST_FLOW_ERROR;
  }

  int dmafd = dup ((*meta)->buffer->dmafd);
  if (dmafd < 0)
    goto err_get_fd;

  GST_DEBUG("CameraId=%d, StreamId=%d DMA export buffer fd=%d.",
    src->device_id, pool->stream_id, dmafd);

  mem = gst_dmabuf_allocator_alloc (pool->allocator, dmafd, pool->size);
  gst_buffer_append_memory (*alloc_buffer, mem);
  if (!gst_camerasrc_is_dma_buffer(*alloc_buffer))
    goto err_not_dmabuf;

  return GST_FLOW_OK;

err_get_fd:
  {
    GST_ERROR("CameraId=%d, StreamId=%d failed to get fd of DMA export buffer.",
      src->device_id, pool->stream_id);
    gst_buffer_unref (*alloc_buffer);
    return GST_FLOW_ERROR;
  }
err_not_dmabuf:
  {
    GST_ERROR("CameraId=%d, StreamId=%d not a dma buffer.",
      src->device_id, pool->stream_id);
    gst_buffer_unref (*alloc_buffer);
    return GST_FLOW_ERROR;
  }
}

static int
gst_camerasrc_alloc_dma_import(GstCamerasrcBufferPool *pool,
      GstBuffer **alloc_buffer, GstCamerasrcMeta **meta)
{
  Gstcamerasrc *src = pool->src;
  GST_DEBUG("CameraId=%d, StreamId=%d allocate DMA import buffer.",
    src->device_id, pool->stream_id);

  GstMemory *mem = NULL;
  int ret = gst_buffer_pool_acquire_buffer(src->streams[pool->stream_id].downstream_pool,
    alloc_buffer, NULL);
  if (ret != GST_FLOW_OK)
    goto err_acquire_buffer;

  *meta = GST_CAMERASRC_META_ADD(*alloc_buffer);

  (*meta)->buffer = (camera_buffer_t *)calloc(1, sizeof(camera_buffer_t));
  if ((*meta)->buffer == NULL) {
    return GST_FLOW_ERROR;
  }

  mem = gst_buffer_peek_memory(*alloc_buffer, 0);

  (*meta)->buffer->dmafd = dup(gst_dmabuf_memory_get_fd(mem));
  if ((*meta)->buffer->dmafd < 0)
    goto err_get_fd;

  if (!gst_camerasrc_is_dma_buffer(*alloc_buffer))
    goto err_not_dmabuf;

  GST_DEBUG("CameraId=%d, StreamId=%d DMA import buffer fd=%d.",
    src->device_id, pool->stream_id, (*meta)->buffer->dmafd);

  (*meta)->buffer->s = src->s[pool->stream_id];
  (*meta)->buffer->s.memType = V4L2_MEMORY_DMABUF;
  (*meta)->buffer->flags = 0;

  return GST_FLOW_OK;

  err_acquire_buffer:
  {
    GST_ERROR("CameraId=%d failed to acquire buffer from downstream pool.", src->device_id);
    gst_buffer_unref (*alloc_buffer);
    return GST_FLOW_ERROR;
  }
  err_get_fd:
  {
    GST_ERROR("CameraId=%d, StreamId=%d failed to get fd of DMA import buffer.",
      src->device_id, pool->stream_id);
    gst_buffer_unref (*alloc_buffer);
    return GST_FLOW_ERROR;
  }
  err_not_dmabuf:
  {
    GST_ERROR("CameraId=%d, StreamId=%d not a dma buffer.",
      src->device_id, pool->stream_id);
    gst_buffer_unref (*alloc_buffer);
    return GST_FLOW_ERROR;
  }
}

static void
gst_camerasrc_set_meta(GstCamerasrcBufferPool *pool, GstBuffer *alloc_buffer)
{
  Gstcamerasrc *camerasrc = pool->src;
  gsize offset[GST_VIDEO_MAX_PLANES];
  gint n_planes, i, offs, stride[GST_VIDEO_MAX_PLANES];
  GstVideoFrameFlags videoFlags = GST_VIDEO_FRAME_FLAG_NONE;
  int stream_id = pool->stream_id;
  GstVideoFormat videoFmt =
      CameraSrcUtils::fourcc_2_gst_fmt(camerasrc->s[stream_id].format);
  if (videoFmt == GST_VIDEO_FORMAT_UNKNOWN) {
    GST_ERROR("CameraId=%d, StreamId=%d Unknown format.",
      camerasrc->device_id, pool->stream_id);
    return;
  }

  memset(offset,0,sizeof(offset));
  memset(stride,0,sizeof(stride));

  offs = 0;
  n_planes = GST_VIDEO_INFO_N_PLANES (&camerasrc->streams[stream_id].info);
  for (i = 0; i < n_planes; i++) {
      offset[i] = offs;
      stride[i] = camerasrc->streams[stream_id].bpl;
      offs = stride[i] * camerasrc->s[stream_id].height;
  }

  if (camerasrc->interlace_field == GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE)
      videoFlags = GST_VIDEO_FRAME_FLAG_INTERLACED;

  GST_DEBUG("CameraId=%d, StreamId=%d set Buffer meta: videoFlage: %d, videofmt: %d, \
          width: %d, heigh: %d, n_planes: %d, offset: %d, stride: %d.",
          camerasrc->device_id, pool->stream_id, videoFlags, videoFmt,
          camerasrc->s[pool->stream_id].width,
          camerasrc->s[pool->stream_id].height,
          n_planes, (int)offset[0], (int)stride[0]);

  /* add metadata to raw video buffers */
  gst_buffer_add_video_meta_full(alloc_buffer, videoFlags, videoFmt,
          camerasrc->s[stream_id].width, camerasrc->s[stream_id].height,
          n_planes, offset, stride);
}

static GstFlowReturn
gst_camerasrc_buffer_pool_alloc_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL(bpool);
  Gstcamerasrc *camerasrc = pool->src;
  GST_INFO("CameraId=%d, StreamId=%d io-mode=%d.",
    camerasrc->device_id, pool->stream_id, camerasrc->io_mode);

  GstBuffer *alloc_buffer = NULL;
  GstCamerasrcMeta *meta = NULL;

  switch (camerasrc->io_mode) {
    case GST_CAMERASRC_IO_MODE_USERPTR:
      if (gst_camerasrc_alloc_userptr(pool, &alloc_buffer, &meta) < 0)
        goto err_alloc_buffer;
      break;
    case GST_CAMERASRC_IO_MODE_MMAP:
      if (gst_camerasrc_alloc_mmap(pool, &alloc_buffer, &meta) < 0)
        goto err_alloc_buffer;
      break;
    case GST_CAMERASRC_IO_MODE_DMA_EXPORT:
      if (gst_camerasrc_alloc_dma_export(pool, &alloc_buffer, &meta) < 0)
        goto err_alloc_buffer;
      break;
    case GST_CAMERASRC_IO_MODE_DMA_IMPORT:
      if (gst_camerasrc_alloc_dma_import(pool, &alloc_buffer, &meta) < 0)
        goto err_alloc_buffer;
      break;
    default:
      break;
  }

  meta->index = pool->number_allocated;
  pool->buffers[meta->index] = alloc_buffer;
  pool->number_allocated++;

  //need to set meta to allocated buffer.
  gst_camerasrc_set_meta(pool, alloc_buffer);
  *buffer = alloc_buffer;
  GST_DEBUG("CameraId=%d, StreamId=%d alloc_buffer buffer %p\n",
    camerasrc->device_id, pool->stream_id, *buffer);

  /* finish buffer allocating */
  if (pool->number_allocated == pool->number_of_buffers)
    pool->alloc_done = TRUE;

  return GST_FLOW_OK;

  err_alloc_buffer:
  {
    return GST_FLOW_ERROR;
  }
}

/**
  *Use system time to update fps when dqbuf for every 2 seconds
  */
void gst_camerasrc_update_fps(Gstcamerasrc *camerasrc, int stream_id)
{
  PERF_CAMERA_ATRACE();
  camerasrc->streams[stream_id].fps_debug.buf_count++;

  /* Don't start counting fps after first few buffers due to they're not stable */
  if (camerasrc->streams[stream_id].fps_debug.buf_count == FPS_BUF_COUNT_START) {
      gettimeofday(&camerasrc->streams[stream_id].fps_debug.dqbuf_start_tm_count,NULL);
      camerasrc->streams[stream_id].fps_debug.dqbuf_tm_start = camerasrc->streams[stream_id].fps_debug.dqbuf_start_tm_count;
      camerasrc->streams[stream_id].fps_debug.last_buf_count = camerasrc->streams[stream_id].fps_debug.buf_count;
  } else if (camerasrc->streams[stream_id].fps_debug.buf_count > FPS_BUF_COUNT_START) {
      gettimeofday(&camerasrc->streams[stream_id].fps_debug.qbuf_tm_end,NULL);
      double duration = (camerasrc->streams[stream_id].fps_debug.qbuf_tm_end.tv_sec -
                         camerasrc->streams[stream_id].fps_debug.dqbuf_tm_start.tv_sec)*1000000 +
                         camerasrc->streams[stream_id].fps_debug.qbuf_tm_end.tv_usec -
                         camerasrc->streams[stream_id].fps_debug.dqbuf_tm_start.tv_usec;
      //calculate pipeline runtime
      camerasrc->streams[stream_id].fps_debug.sum_time += duration;
      camerasrc->streams[stream_id].fps_debug.tm_interval += duration;
      camerasrc->streams[stream_id].fps_debug.dqbuf_tm_start = camerasrc->streams[stream_id].fps_debug.qbuf_tm_end;

      if (camerasrc->streams[stream_id].fps_debug.tm_interval >= FPS_TIME_INTERVAL) {
          double interval_fps = (camerasrc->streams[stream_id].fps_debug.buf_count -
                             camerasrc->streams[stream_id].fps_debug.last_buf_count)/
                             (camerasrc->streams[stream_id].fps_debug.tm_interval/1000000);
          g_print("fps:%.4f   Camera name: %s Stream Id: %d\n",
            interval_fps, camerasrc->streams[stream_id].cam_info.name, stream_id);

          if (camerasrc->streams[stream_id].fps_debug.init_max_min_fps) {
              camerasrc->streams[stream_id].fps_debug.max_fps = interval_fps;
              camerasrc->streams[stream_id].fps_debug.min_fps = interval_fps;
              camerasrc->streams[stream_id].fps_debug.init_max_min_fps = false;
          }

          if (interval_fps >= camerasrc->streams[stream_id].fps_debug.max_fps) {
              camerasrc->streams[stream_id].fps_debug.max_fps = interval_fps;
          } else if (interval_fps < camerasrc->streams[stream_id].fps_debug.min_fps) {
              camerasrc->streams[stream_id].fps_debug.min_fps = interval_fps;
          }

          camerasrc->streams[stream_id].fps_debug.tm_interval = 0;
          camerasrc->streams[stream_id].fps_debug.last_buf_count = camerasrc->streams[stream_id].fps_debug.buf_count;
      }
  }
}

/**
 * Dequeue a buffer from a stream
 */
static GstFlowReturn
gst_camerasrc_buffer_pool_acquire_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL(bpool);
  Gstcamerasrc *camerasrc = pool->src;
  int stream_id = pool->stream_id;
  GST_INFO("CameraId=%d, StreamId=%d Thread ID=%ld  .", camerasrc->device_id, pool->stream_id, gettid());

  GstBuffer *gbuffer = pool->buffers[pool->acquire_buffer_index%pool->number_allocated];
  GstCamerasrcMeta *meta = GST_CAMERASRC_META_GET(gbuffer);
  int sequence_diff = 0;
  gboolean do_weaving = true;
  const char *buffer_field;

  if (camerasrc->print_fps)
      gst_camerasrc_update_fps(camerasrc, stream_id);

  /* in PLAYING->PAUSED and PAUSED->NULL state, no need to dqbuf */
  if (camerasrc->running != GST_CAMERASRC_STATUS_RUNNING) {
    GST_INFO("CameraId=%d, StreamId=%d stop dqbuf.", camerasrc->device_id, pool->stream_id);
    return GST_FLOW_EOS;
  }

  int ret = camera_stream_dqbuf(camerasrc->device_id, stream_id, &meta->buffer);
  if (ret != 0) {
    GST_ERROR("CameraId=%d, StreamId=%d dqbuf failed ret %d.",
      camerasrc->device_id, pool->stream_id, ret);
    return GST_FLOW_ERROR;
  }

  GstClockTime timestamp = meta->buffer->timestamp;
  camerasrc->streams[stream_id].time_end = meta->buffer->timestamp;

  if (camerasrc->print_field)
    g_print("buffer field: %d    Camera Id: %d    buffer sequence: %ld\n",
      meta->buffer->s.field, camerasrc->device_id, meta->buffer->sequence);

  PRINT_FIELD(meta->buffer->s.field, buffer_field);
  GST_INFO("CameraId=%d, StreamId=%d DQ buffer done, GstBuffer=%p, UserBuffer=%p, \
    buffer index=%d, ts=%lu, buffer field=%s.",
    camerasrc->device_id, pool->stream_id, gbuffer, meta->buffer,
    meta->buffer->index, meta->buffer->timestamp, buffer_field);

  /* when sw_weaving is enabled, copy buffer data to both top and bottom
    * if it's the first buffer, or buffer sequence is inconsecutive */
  if (camerasrc->deinterlace_method == GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE)
  {
    sequence_diff = meta->buffer->sequence - camerasrc->streams[stream_id].previous_sequence;
    camerasrc->streams[stream_id].previous_sequence = meta->buffer->sequence;
  }
  if (camerasrc->first_frame || sequence_diff > 1) {
    gst_camerasrc_copy_field (camerasrc,
      meta->buffer,
      camerasrc->streams[stream_id].top);
    gst_camerasrc_copy_field(camerasrc,
      meta->buffer,
      camerasrc->streams[stream_id].bottom);

    do_weaving = false;
  }

  switch(meta->buffer->s.field) {
    case V4L2_FIELD_ANY:
        break;
    case V4L2_FIELD_TOP:
        GST_BUFFER_FLAG_SET (gbuffer, GST_VIDEO_BUFFER_FLAG_TFF);

        if (do_weaving) {
          gst_camerasrc_copy_field(camerasrc,
            meta->buffer,
            camerasrc->streams[stream_id].top);
        }
        break;
    case V4L2_FIELD_BOTTOM:
        GST_BUFFER_FLAG_UNSET (gbuffer, GST_VIDEO_BUFFER_FLAG_TFF);
        GST_BUFFER_FLAG_SET (gbuffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);

        if (do_weaving) {
          gst_camerasrc_copy_field(camerasrc,
            meta->buffer,
            camerasrc->streams[stream_id].bottom);
        }
        break;
    default:
        GST_BUFFER_FLAG_UNSET (gbuffer, GST_VIDEO_BUFFER_FLAG_TFF);
        GST_BUFFER_FLAG_UNSET (gbuffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);
        break;
  }

  ret = gst_camerasrc_deinterlace_frame(camerasrc, meta->buffer);
  if (ret != 0) {
    GST_ERROR("CameraId=%d, StreamId=%d deinterlace frame failed.",
      camerasrc->device_id, pool->stream_id);
    return GST_FLOW_ERROR;
  }

  camerasrc->first_frame = false;

  GST_BUFFER_TIMESTAMP(gbuffer) = timestamp;
  *buffer = gbuffer;
  pool->acquire_buffer_index++;
  GST_DEBUG("CameraId=%d, StreamId=%d acquire_buffer buffer %p.",
    camerasrc->device_id, pool->stream_id, *buffer);
  {
    PERF_CAMERA_ATRACE_PARAM1("sof.sequence", meta->buffer->sequence);
  }

  return GST_FLOW_OK;
}

int gst_camerasrc_get_buffer_usage_shifting(int flag)
{
  switch (flag) {
    case GST_CAMERASRC_BUFFER_USAGE_NONE:
      return 0;
    case GST_CAMERASRC_BUFFER_USAGE_READ:
      return BUFFER_FLAG_SW_READ;
    case GST_CAMERASRC_BUFFER_USAGE_WRITE:
      return BUFFER_FLAG_SW_WRITE;
    case GST_CAMERASRC_BUFFER_USAGE_DMA_EXPORT:
      return BUFFER_FLAG_DMA_EXPORT;
  }
  return 0;
}

/**
 * Queue buffer(s) into stream(s)
 */
static void
gst_camerasrc_buffer_pool_release_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL (bpool);
  Gstcamerasrc *camerasrc = pool->src;
  GstCamerasrcMeta *meta = GST_CAMERASRC_META_GET(buffer);
  int stream_id = pool->stream_id;

  meta->buffer->flags |= gst_camerasrc_get_buffer_usage_shifting(camerasrc->buffer_usage);

  g_mutex_lock(&camerasrc->qbuf_mutex);
  /* save buffer into queue */
  camerasrc->streams[stream_id].buffer_queue->push(meta->buffer);

  GST_INFO("CameraId=%d, StreamId=%d Ready to queue buffer, \
    number of buffer in queue=%ld, Buffer index=%d, Buffer flag=%d",
    camerasrc->device_id, stream_id,
    camerasrc->streams[stream_id].buffer_queue->size(),
    meta->buffer->index, meta->buffer->flags);

  /* in PLAYING->PAUSED and PAUSED->NULL state,
  * no need to check if queue has available buffer,
  * unlock qbuf_mutex immediately and quit function so pipeline can cease normally
  * this check is not needed before preallocate is done */
  if (camerasrc->start_streams) {
    if (camerasrc->running != GST_CAMERASRC_STATUS_RUNNING) {
      GST_INFO("CameraId=%d, StreamId=%d is exiting.", camerasrc->device_id, pool->stream_id);
      g_mutex_unlock(&camerasrc->qbuf_mutex);
      return;
    }
  }

  /* check if there's available buffer in queue */
  for (int i = 0; i < GST_CAMERASRC_MAX_STREAM_NUM; i++) {
    if (camerasrc->streams[i].activated) {
      if (camerasrc->streams[i].buffer_queue->empty()) {
        GST_INFO("CameraId=%d, StreamId=%d the other queue doesn't have available buffer",
          camerasrc->device_id, pool->stream_id);
        g_mutex_unlock(&camerasrc->qbuf_mutex);
        return;
      }
    }
  }

  /* acquire the first buffer in each queue and save into buffer_list array */
  for (int j = 0; j < GST_CAMERASRC_MAX_STREAM_NUM; j++) {
    if (camerasrc->streams[j].activated) {
      camerasrc->buffer_list[j] = camerasrc->streams[j].buffer_queue->front();
      camerasrc->buffer_list[j]->sequence = -1;
      camerasrc->buffer_list[j]->timestamp = 0;
    }
  }

  /* queue buffers from buffer_list here */
  int ret = camera_stream_qbuf(camerasrc->device_id, camerasrc->buffer_list, camerasrc->number_of_activepads);
  if (ret < 0) {
    GST_ERROR("CameraId=%d, StreamId=%d failed to qbuf back to stream.",
      camerasrc->device_id, stream_id);
    g_mutex_unlock(&camerasrc->qbuf_mutex);
    return;
  }
  GST_INFO("CameraId=%d, StreamId=%d Queue buffer succeed", camerasrc->device_id, stream_id);

  /* pop the buffer out of queue */
  for (int k = 0; k < GST_CAMERASRC_MAX_STREAM_NUM; k++) {
    if (camerasrc->streams[k].activated)
      camerasrc->streams[k].buffer_queue->pop();
  }
  g_mutex_unlock(&camerasrc->qbuf_mutex);

  {
    PERF_CAMERA_ATRACE_PARAM1("sof.sequence", meta->buffer->sequence);
  }
}

static void
gst_camerasrc_free_weave_buffer (Gstcamerasrc *src, int stream_id)
{
  if (src->streams[stream_id].top->addr)
    free(src->streams[stream_id].top->addr);

  if (src->streams[stream_id].bottom->addr)
    free(src->streams[stream_id].bottom->addr);

  free(src->streams[stream_id].top);
  free(src->streams[stream_id].bottom);
  src->streams[stream_id].top = NULL;
  src->streams[stream_id].bottom = NULL;
}

static void
gst_camerasrc_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL (bpool);
  Gstcamerasrc *camerasrc = pool->src;
  GstCamerasrcMeta *meta = GST_CAMERASRC_META_GET(buffer);
  GST_INFO("CameraId=%d, StreamId=%d.", camerasrc->device_id, pool->stream_id);

  switch (camerasrc->io_mode) {
    case GST_CAMERASRC_IO_MODE_USERPTR:
      if (meta->buffer->addr) {
        free(meta->buffer->addr);
      }
      break;
    case GST_CAMERASRC_IO_MODE_MMAP:
      if (meta->mem)
        meta->mem = NULL;
      break;
    case GST_CAMERASRC_IO_MODE_DMA_EXPORT:
    case GST_CAMERASRC_IO_MODE_DMA_IMPORT:
      if (meta->buffer->dmafd >= 0)
        close(meta->buffer->dmafd);
      break;
    default:
      break;
  }

  free(meta->buffer);
  pool->buffers[meta->index] = NULL;
  GST_DEBUG("CameraId=%d, StreamId=%d free_buffer buffer %p.",
    camerasrc->device_id, pool->stream_id, buffer);

  if (buffer->pool == NULL && camerasrc->io_mode == GST_CAMERASRC_IO_MODE_DMA_IMPORT)
    buffer->pool = camerasrc->streams[pool->stream_id].downstream_pool;

  gst_buffer_unref (buffer);
}

static void
gst_camerasrc_print_framerate_analysis(Gstcamerasrc *camerasrc, int stream_id)
{
  double total_stream_buffers = camerasrc->streams[stream_id].fps_debug.buf_count-FPS_BUF_COUNT_START; //valid number of stream buffers
  double  total_stream_duration = camerasrc->streams[stream_id].fps_debug.sum_time/1000000; //valid time of counting stream buffers
  camerasrc->streams[stream_id].fps_debug.av_fps = total_stream_buffers/total_stream_duration;

  if (total_stream_duration < FPS_TIME_INTERVAL/1000000) {
     /* This case means that pipeline runtime is less than 2 seconds(we count fps every 2 seconds),
        * no updates from max_fps and min_fps, only average fps is available */
     g_print("\nTotal frame is: %g Camera name:%s(Id:%d) Stream Id:%d\nAverage fps is:%.4f\n",
                      camerasrc->streams[stream_id].fps_debug.buf_count,
                      camerasrc->streams[stream_id].cam_info.name,
                      camerasrc->device_id,
                      stream_id,
                      camerasrc->streams[stream_id].fps_debug.av_fps);
  } else {
     //This case means that pipeline runtime is longer than 2 seconds
     g_print("\nTotal frame is:%g  Camera name:%s(Id:%d) Stream Id:%d\n",
                      camerasrc->streams[stream_id].fps_debug.buf_count,
                      camerasrc->streams[stream_id].cam_info.name,
                      camerasrc->device_id,
                      stream_id);
     g_print("Max fps is:%.4f,Minimum fps is:%.4f,Average fps is:%.4f\n\n",
                      camerasrc->streams[stream_id].fps_debug.max_fps,
                      camerasrc->streams[stream_id].fps_debug.min_fps,
                      camerasrc->streams[stream_id].fps_debug.av_fps);
  }
}

static gboolean
gst_camerasrc_buffer_pool_stop(GstBufferPool *bpool)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL(bpool);
  Gstcamerasrc *camerasrc = pool->src;
  int stream_id = pool->stream_id;
  GST_INFO("CameraId=%d, StreamId=%d.", camerasrc->device_id, pool->stream_id);

  GST_CAMSRC_LOCK(camerasrc);
  if (camerasrc->camera_open) {
    camera_device_stop(camerasrc->device_id);
    camera_device_close(camerasrc->device_id);
    camerasrc->camera_open = false;
  }
  GST_CAMSRC_UNLOCK(camerasrc);

  /* Calculate max/min/average fps */
  if (camerasrc->print_fps)
     gst_camerasrc_print_framerate_analysis(camerasrc, stream_id);

  if (pool->allocator)
    gst_object_unref(pool->allocator);

  /* free topfield buffer and bottomfield buffer, both for temp storage */
  if (camerasrc->interlace_field == GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE &&
    camerasrc->deinterlace_method == GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE)
    gst_camerasrc_free_weave_buffer(camerasrc, stream_id);

  /* free the remaining buffers */
  for (int n = 0; n < pool->number_allocated; n++)
    gst_camerasrc_buffer_pool_free_buffer (bpool, pool->buffers[n]);

  pool->number_allocated = 0;
  g_free(pool->buffers);
  pool->buffers = NULL;

  if (camerasrc->streams[stream_id].downstream_pool)
    gst_object_unref(camerasrc->streams[stream_id].downstream_pool);
  else if (camerasrc->streams[stream_id].pool)
    gst_object_unref(camerasrc->streams[stream_id].pool);

  delete camerasrc->streams[stream_id].buffer_queue;
  camerasrc->streams[stream_id].buffer_queue = NULL;

  return TRUE;
}
