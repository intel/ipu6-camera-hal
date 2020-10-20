/*
 * GStreamer
 * Copyright (C) 2015-2018 Intel Corporation
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gstcambasesrc.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>
#include <gst/video/video.h>

using namespace std;

GST_DEBUG_CATEGORY_STATIC (gst_cam_base_src_debug);
#define GST_CAT_DEFAULT gst_cam_base_src_debug

#define GST_LIVE_GET_LOCK(elem)               (&GST_CAM_BASE_SRC_CAST(elem)->live_lock)
#define GST_LIVE_LOCK(elem)                   g_mutex_lock(GST_LIVE_GET_LOCK(elem))
#define GST_LIVE_UNLOCK(elem)                 g_mutex_unlock(GST_LIVE_GET_LOCK(elem))
#define GST_LIVE_GET_COND(elem)               (&GST_CAM_BASE_SRC_CAST(elem)->live_cond)
#define GST_LIVE_WAIT(elem)                   g_cond_wait (GST_LIVE_GET_COND (elem), GST_LIVE_GET_LOCK (elem))
#define GST_LIVE_WAIT_UNTIL(elem, end_time)   g_cond_timed_wait (GST_LIVE_GET_COND (elem), GST_LIVE_GET_LOCK (elem), end_time)
#define GST_LIVE_SIGNAL(elem)                 g_cond_signal (GST_LIVE_GET_COND (elem));

#define GST_VID_LIVE_GET_LOCK(elem, id)       (&GST_CAM_BASE_SRC_CAST(elem)->mux[id].vid_live_lock)
#define GST_VID_LIVE_LOCK(elem, id)           g_mutex_lock(GST_VID_LIVE_GET_LOCK(elem, id))
#define GST_VID_LIVE_UNLOCK(elem, id)         g_mutex_unlock(GST_VID_LIVE_GET_LOCK(elem, id))

#define GST_ASYNC_GET_COND(elem)              (&GST_CAM_BASE_SRC_CAST(elem)->priv->async_cond)
#define GST_ASYNC_WAIT(elem)                  g_cond_wait (GST_ASYNC_GET_COND (elem), GST_OBJECT_GET_LOCK (elem))
#define GST_ASYNC_SIGNAL(elem)                g_cond_signal (GST_ASYNC_GET_COND (elem));

#define CLEAR_PENDING_EOS(bsrc) \
  G_STMT_START { \
    g_atomic_int_set (&bsrc->priv->has_pending_eos, FALSE); \
    gst_event_replace (&bsrc->priv->pending_eos, NULL); \
  } G_STMT_END
#define SEEK_TYPE_IS_RELATIVE(t) (((t) != GST_SEEK_TYPE_NONE) && ((t) != GST_SEEK_TYPE_SET))

struct _GstCamMuxSrcPrivate {
  /* if video stream should be started */
  gboolean vid_stream_start_pending;

  GstClockTime vid_latency;
  GstClockTimeDiff vid_ts_offset;

  /* buffer pool params of video pad */
  GstBufferPool *vid_pool;
  GstAllocator *vid_allocator;
  GstAllocationParams vid_params;
};

struct _GstCamBaseSrcPrivate {
  gboolean discont;
  gboolean flushing;

  GstFlowReturn start_result;
  gboolean async;

  /* if a stream-start event should be sent */
  gboolean stream_start_pending;

  /* if segment should be sent and a
   * seqnum if it was originated by a seek */
  gboolean segment_pending;
  guint32 segment_seqnum;

  /* if EOS is pending (atomic) */
  GstEvent *pending_eos;
  gint has_pending_eos;

  /* if the eos was caused by a forced eos from the application */
  gboolean forced_eos;

  /* if the src pad got EOS, request pads should also stop immediately */
  gboolean receive_eos;

  /* startup latency is the time it takes between going to PLAYING and producing
   * the first BUFFER with running_time 0. This value is included in the latency
   * reporting. */
  GstClockTime latency;

  /* timestamp offset, this is the offset add to the values of gst_times for
   * pseudo live sources */
  GstClockTimeDiff ts_offset;

  gboolean do_timestamp;
  volatile gint dynamic_size;
  volatile gint automatic_eos;

  /* stream sequence number */
  guint32 seqnum;

  /* pending events (TAG, CUSTOM_BOTH, CUSTOM_DOWNSTREAM) to be
   * pushed in the data stream */
  GList *pending_events;
  volatile gint have_events;

  /* QoS *//* with LOCK */
  gboolean qos_enabled;
  gdouble proportion;
  GstClockTime earliest_time;

  /* buffer pool params of src pad */
  GstBufferPool *pool;
  GstAllocator *allocator;
  GstAllocationParams params;

  GCond async_cond;

  map<string, int> request_stream_map;

  GstCamMuxSrcPrivate muxPriv[GST_CAMERASRC_MAX_STREAM_NUM - 1];
};

enum
{
  PROP_0,
  PROP_BLOCKSIZE,
  PROP_NUM_BUFFERS,
  PROP_TYPEFIND,
  PROP_DO_TIMESTAMP,
};

static GstElementClass *parent_class = NULL;

static gint GstCamBaseSrc_private_offset = 0;

static void gst_cam_base_src_class_init (GstCamBaseSrcClass *klass);
static void gst_cam_base_src_init (GstCamBaseSrc *src, GstCamBaseSrcClass *klass);
static void gst_cam_base_src_finalize (GObject * object);

static GstCaps *gst_cam_base_src_default_get_caps (GstCamBaseSrc *bsrc, GstCaps *filter);
static GstCaps *gst_cam_base_src_default_fixate (GstCamBaseSrc * bsrc, GstCaps * caps);
static GstCaps *gst_cam_base_src_fixate (GstCamBaseSrc * src, GstCaps * caps);
static gboolean gst_cam_base_src_is_random_access (GstCamBaseSrc * src);
static gboolean gst_cam_base_src_activate_mode (GstPad * pad, GstObject * parent,
    GstPadMode mode, gboolean active);
static void gst_cam_base_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_cam_base_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static gboolean gst_cam_base_src_event (GstPad * pad, GstObject * parent, GstEvent * event);
static gboolean gst_cam_base_src_send_event (GstElement * elem, GstEvent * event);
static gboolean gst_cam_base_src_default_event (GstCamBaseSrc * src, GstEvent * event);
static gboolean gst_cam_base_src_query (GstPad * pad, GstObject * parent, GstQuery * query);
static gboolean gst_cam_base_src_activate_pool (GstCamBaseSrc * basesrc, gboolean active);
static gboolean gst_cam_base_src_default_negotiate (GstCamBaseSrc * basesrc, GstPad *pad);
static gboolean gst_cam_base_src_default_do_seek (GstCamBaseSrc * src, GstSegment * segment);
static gboolean gst_cam_base_src_default_query (GstCamBaseSrc *src, GstQuery *query);
static gboolean gst_cam_base_src_default_prepare_seek_segment (GstCamBaseSrc * src, GstEvent * event,
    GstSegment * seeksegment);
static GstFlowReturn gst_cam_base_src_default_create (GstCamBaseSrc * basesrc,
    GstPad *pad, guint64 offset, guint size, GstBuffer ** buf);
static GstFlowReturn gst_cam_base_src_default_alloc (GstCamBaseSrc * basesrc,
    GstPad *pad, guint64 offset, guint size, GstBuffer ** buf);
static gboolean gst_cam_base_src_default_decide_allocation (GstCamBaseSrc * basesrc,
    GstQuery * query, GstPad * pad);
static gboolean gst_cam_base_src_set_flushing (GstCamBaseSrc * basesrc,
    gboolean flushing, gboolean live_play, gboolean * playing);
static gboolean gst_cam_base_src_start (GstCamBaseSrc * basesrc);
static gboolean gst_cam_base_src_stop (GstCamBaseSrc * basesrc);
static GstStateChangeReturn gst_cam_base_src_change_state (GstElement * element, GstStateChange transition);
static void gst_cam_base_src_loop (GstPad * pad);
static GstFlowReturn gst_cam_base_src_getrange (GstPad * pad, GstObject * parent,
    guint64 offset, guint length, GstBuffer ** buf);
static GstFlowReturn gst_cam_base_src_get_range (GstCamBaseSrc * src, GstPad *pad, guint64 offset,
    guint length, GstBuffer ** buf);
static gboolean gst_cam_base_src_seekable (GstCamBaseSrc * src);
static gboolean gst_cam_base_src_negotiate (GstCamBaseSrc * basesrc, GstPad *pad);
static gboolean gst_cam_base_src_update_length (GstCamBaseSrc * src, guint64 offset, guint * length, gboolean force);

/* debug symbols */
static gboolean gst_cam_base_src_activate_push (GstPad * pad, GstObject * parent, gboolean active);
static gboolean gst_cam_base_src_set_allocation (GstCamBaseSrc * basesrc, GstPad *pad,
    GstBufferPool * pool, GstAllocator * allocator, GstAllocationParams * params);
static gboolean gst_cam_base_src_perform_seek (GstCamBaseSrc * src, GstEvent * event, gboolean unlock);
static gboolean gst_cam_base_src_do_seek (GstCamBaseSrc * src, GstSegment * segment);
static gboolean gst_cam_base_src_send_stream_start (GstCamBaseSrc * src);
static gboolean gst_cam_base_src_prepare_allocation (GstCamBaseSrc * basesrc, GstCaps * caps, GstPad * pad);
static GstClockReturn gst_cam_base_src_do_sync (GstCamBaseSrc * basesrc, GstBuffer * buffer);
static GstClockReturn gst_cam_base_src_wait (GstCamBaseSrc * basesrc, GstClock * clock, GstClockTime time);
static void gst_cam_base_src_update_qos (GstCamBaseSrc * src,
    gdouble proportion, GstClockTimeDiff diff, GstClockTime timestamp);
static gboolean gst_cam_base_src_set_playing (GstCamBaseSrc * basesrc, gboolean live_play);

/* requisite functions for operating requested pads and streams */
static GstPad *gst_cam_base_src_add_video_pad(GstCamBaseSrc *basesrc, GstCamBaseSrcClass *klass,
    GstPad *pad, guint stream_id, const char *padname);
static gboolean gst_cam_base_src_video_activate_push (GstPad * pad, GstObject * parent, guint stream_id, gboolean active);
static GstClockReturn gst_cam_base_src_do_video_sync (GstCamBaseSrc * basesrc, guint stream_id, GstBuffer * buffer);
static gboolean gst_cam_base_src_video_query (GstPad *pad, GstObject *parent, GstQuery *query);
static GstFlowReturn gst_cam_base_src_video_get_range (GstCamBaseSrc * src, GstPad *pad, guint64 offset,
    guint length, GstBuffer ** buf);
static gboolean gst_cam_base_src_activate_video_pool (GstCamBaseSrc * basesrc, gboolean active);
static void gst_cam_base_src_video_loop (GstPad * pad);
static gboolean gst_cam_base_src_send_video_stream_start (GstCamBaseSrc *src, GstPad *pad);

/* Call init functions*/
GType
gst_cam_base_src_get_type (void)
{
  static volatile gsize cam_base_src_type = 0;

  if (g_once_init_enter(&cam_base_src_type)) {
    GType _type;
    static const GTypeInfo cam_base_src_info = {
        sizeof(GstCamBaseSrcClass),
        NULL,
        NULL,
        (GClassInitFunc)gst_cam_base_src_class_init,
        NULL,
        NULL,
        sizeof(GstCamBaseSrc),
        0,
        (GInstanceInitFunc) gst_cam_base_src_init,
        NULL
    };

    _type = g_type_register_static(GST_TYPE_ELEMENT, "GstCamBaseSrc",
        &cam_base_src_info, G_TYPE_FLAG_ABSTRACT);

    GstCamBaseSrc_private_offset =
        g_type_add_instance_private (_type, sizeof (GstCamBaseSrcPrivate));

    g_once_init_leave(&cam_base_src_type, _type);
  }
  return cam_base_src_type;
}

/* Main entrance*/
static void
gst_cam_base_src_class_init(GstCamBaseSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gstelement_class = GST_ELEMENT_CLASS(klass);

  GST_DEBUG_CATEGORY_INIT(gst_cam_base_src_debug, "CamBaseSrc", 0, "CamBaseSrc element");

  if (GstCamBaseSrc_private_offset != 0)
    g_type_class_adjust_private_offset (klass, &GstCamBaseSrc_private_offset);

  parent_class = (GstElementClass *)g_type_class_peek_parent (klass);

  gobject_class->finalize = gst_cam_base_src_finalize;
  gobject_class->set_property = gst_cam_base_src_set_property;
  gobject_class->get_property = gst_cam_base_src_get_property;

  g_object_class_install_property (gobject_class, PROP_BLOCKSIZE,
      g_param_spec_uint ("blocksize", "Block size",
          "Size in bytes to read per buffer (-1 = default)", 0, G_MAXUINT,
          DEFAULT_BLOCKSIZE, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_NUM_BUFFERS,
      g_param_spec_int ("num-buffers", "num-buffers",
          "Number of buffers to output before sending EOS (-1 = unlimited)",
          -1, G_MAXINT, DEFAULT_NUM_BUFFERS, (GParamFlags)(G_PARAM_READWRITE |
          G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_TYPEFIND,
      g_param_spec_boolean ("typefind", "Typefind",
          "Run typefind before negotiating", DEFAULT_TYPEFIND,
          (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_DO_TIMESTAMP,
      g_param_spec_boolean ("do-timestamp", "Do timestamp",
          "Apply current stream time to buffers", DEFAULT_DO_TIMESTAMP,
          (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gst_cam_base_src_change_state);
  gstelement_class->send_event = GST_DEBUG_FUNCPTR (gst_cam_base_src_send_event);

  klass->get_caps = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_get_caps);
  klass->negotiate = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_negotiate);
  klass->fixate = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_fixate);
  klass->prepare_seek_segment =
      GST_DEBUG_FUNCPTR (gst_cam_base_src_default_prepare_seek_segment);
  klass->do_seek = GST_DEBUG_FUNCPTR (gst_cam_base_src_default_do_seek);
  klass->query = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_query);
  klass->event = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_event);
  klass->create = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_create);
  klass->alloc = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_alloc);
  klass->decide_allocation = GST_DEBUG_FUNCPTR(gst_cam_base_src_default_decide_allocation);

  /* extra interface for multi-stream feature to add more source pads */
  klass->add_video_pad = GST_DEBUG_FUNCPTR(gst_cam_base_src_add_video_pad);

  /* Registering debug symbols for function pointers */
  GST_DEBUG_REGISTER_FUNCPTR (gst_cam_base_src_activate_mode);
  GST_DEBUG_REGISTER_FUNCPTR (gst_cam_base_src_event);
  GST_DEBUG_REGISTER_FUNCPTR (gst_cam_base_src_query);
  GST_DEBUG_REGISTER_FUNCPTR (gst_cam_base_src_video_query);
  GST_DEBUG_REGISTER_FUNCPTR (gst_cam_base_src_getrange);
  GST_DEBUG_REGISTER_FUNCPTR (gst_cam_base_src_fixate);
}

static void
gst_cam_base_src_add_src_pad(GstCamBaseSrc *basesrc, GstCamBaseSrcClass *klass)
{
  GstPad *pad;
  GstPadTemplate *pad_template;

  /* add template to structure*/
  pad_template =
          gst_element_class_get_pad_template (GST_ELEMENT_CLASS (klass), GST_CAM_BASE_SRC_PAD_NAME);
  g_return_if_fail (pad_template != NULL);

  GST_DEBUG_OBJECT (basesrc, "creating src pad");
  pad = gst_pad_new_from_template (pad_template, GST_CAM_BASE_SRC_PAD_NAME);

  GST_DEBUG_OBJECT (basesrc, "setting functions on src pad");
  gst_pad_set_activatemode_function (pad, gst_cam_base_src_activate_mode);
  gst_pad_set_event_function (pad, gst_cam_base_src_event);
  gst_pad_set_query_function (pad, gst_cam_base_src_query);
  gst_pad_set_getrange_function (pad, gst_cam_base_src_getrange);

  // hold pointer to pad
  basesrc->srcpad = pad;
  GST_DEBUG_OBJECT (basesrc, "adding src pad");
  gst_element_add_pad (GST_ELEMENT (basesrc), pad);
}

/* video source pad is a request pad from subclass, its stream id starts from "0" since
* In GstCamBaseSrc, the arraies of GstCamMuxSrc and GstCamMuxSrcPrivate are to describe request pads,
* and the stream id should start from "0" since "src" pad is not considered.
*/
static GstPad *
gst_cam_base_src_add_video_pad(GstCamBaseSrc *basesrc, GstCamBaseSrcClass *klass,
                               GstPad *pad, guint stream_id, const char *padname)
{
  GST_DEBUG_OBJECT (basesrc, "creating video pad");

  basesrc->priv->request_stream_map[padname] = stream_id;

  basesrc->priv->muxPriv[stream_id].vid_stream_start_pending = FALSE;
  g_mutex_init(&basesrc->mux[stream_id].vid_live_lock);

  GST_DEBUG_OBJECT (basesrc, "setting functions on video pad");
  gst_pad_set_activatemode_function (pad, gst_cam_base_src_activate_mode);
  gst_pad_set_event_function (pad, gst_cam_base_src_event);
  gst_pad_set_query_function (pad, gst_cam_base_src_video_query);
  gst_pad_set_getrange_function (pad, gst_cam_base_src_getrange);

  // hold pointer to pad
  basesrc->mux[stream_id].videopad = pad;
  GST_DEBUG_OBJECT (basesrc, "adding video pad");
  gst_element_add_pad (GST_ELEMENT (basesrc), pad);

  return pad;
}

static inline gpointer
gst_cam_base_src_get_instance_private (GstCamBaseSrc *self)
{
  return (G_STRUCT_MEMBER_P (self, GstCamBaseSrc_private_offset));
}

static void
gst_cam_base_src_init(GstCamBaseSrc *basesrc, GstCamBaseSrcClass *klass)
{
    GST_INFO("DBG: @%s number of added templates=%d\n", __func__,
                     GST_ELEMENT_CLASS(klass)->numpadtemplates);
    basesrc->priv = (GstCamBaseSrcPrivate *) gst_cam_base_src_get_instance_private (basesrc);
    g_return_if_fail (GST_IS_CAM_BASE_SRC (basesrc));

    basesrc->is_live = FALSE;
    g_mutex_init (&basesrc->live_lock);
    g_cond_init (&basesrc->live_cond);
    basesrc->num_buffers = DEFAULT_NUM_BUFFERS;
    basesrc->num_buffers_left = -1;
    basesrc->priv->automatic_eos = TRUE;
    basesrc->priv->stream_start_pending = FALSE;
    basesrc->priv->receive_eos = FALSE;
    basesrc->priv->request_stream_map.clear();
    basesrc->can_activate_push = TRUE;

    gst_cam_base_src_add_src_pad(basesrc, klass);

    basesrc->blocksize = DEFAULT_BLOCKSIZE;
    basesrc->clock_id = NULL;
    //we operate in BYTES by default
    gst_cam_base_src_set_format (basesrc, GST_FORMAT_BYTES, GST_CAM_BASE_SRC_PAD_NAME);
    basesrc->typefind = DEFAULT_TYPEFIND;
    basesrc->priv->do_timestamp = DEFAULT_DO_TIMESTAMP;
    g_atomic_int_set (&basesrc->priv->have_events, FALSE);

    g_cond_init (&basesrc->priv->async_cond);
    basesrc->priv->start_result = GST_FLOW_FLUSHING;
    GST_OBJECT_FLAG_UNSET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTED);
    GST_OBJECT_FLAG_UNSET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTING);
    GST_OBJECT_FLAG_SET (basesrc, GST_ELEMENT_FLAG_SOURCE);

    GST_DEBUG_OBJECT (basesrc, "init done");
}

static void
gst_cam_base_src_finalize (GObject * object)
{
  GstCamBaseSrc *basesrc;
  GstEvent **event_p;

  basesrc = GST_CAM_BASE_SRC (object);

  g_mutex_clear (&basesrc->live_lock);
  g_cond_clear (&basesrc->live_cond);
  g_cond_clear (&basesrc->priv->async_cond);

  event_p = &basesrc->pending_seek;
  gst_event_replace (event_p, NULL);

  if (basesrc->priv->pending_events) {
    g_list_foreach (basesrc->priv->pending_events, (GFunc) gst_event_unref,
        NULL);
    g_list_free (basesrc->priv->pending_events);
  }

  basesrc->priv->request_stream_map.clear();

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * gst_cam_base_src_wait_playing:
 * @src: the src
 *
 * If the #GstCamBaseSrcClass.create() method performs its own synchronisation
 * against the clock it must unblock when going from PLAYING to the PAUSED state
 * and call this method before continuing to produce the remaining data.
 *
 * This function will block until a state change to PLAYING happens (in which
 * case this function returns %GST_FLOW_OK) or the processing must be stopped due
 * to a state change to READY or a FLUSH event (in which case this function
 * returns %GST_FLOW_FLUSHING).
 *
 * Returns: %GST_FLOW_OK if @src is PLAYING and processing can
 * continue. Any other return value should be returned from the create vmethod.
 */
GstFlowReturn
gst_cam_base_src_wait_playing (GstCamBaseSrc * src)
{
  g_return_val_if_fail (GST_IS_CAM_BASE_SRC (src), GST_FLOW_ERROR);

  do {
    /* block until the state changes, or we get a flush, or something */
    GST_DEBUG_OBJECT (src, "live source waiting for running state");
    GST_LIVE_WAIT (src);
    GST_DEBUG_OBJECT (src, "live source unlocked");
    if (src->priv->flushing)
      goto flushing;
  } while (G_UNLIKELY (!src->live_running));

  return GST_FLOW_OK;

  /* ERRORS */
flushing:
  {
    GST_DEBUG_OBJECT (src, "we are flushing");
    return GST_FLOW_FLUSHING;
  }
}

/**
 * gst_cam_base_src_set_live:
 * @src: base source instance
 * @live: new live-mode
 *
 * If the element listens to a live source, @live should
 * be set to %TRUE.
 *
 * A live source will not produce data in the PAUSED state and
 * will therefore not be able to participate in the PREROLL phase
 * of a pipeline. To signal this fact to the application and the
 * pipeline, the state change return value of the live source will
 * be GST_STATE_CHANGE_NO_PREROLL.
 */
void
gst_cam_base_src_set_live (GstCamBaseSrc * src, gboolean live)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));

  GST_OBJECT_LOCK (src);
  src->is_live = live;
  GST_OBJECT_UNLOCK (src);
}

/**
 * gst_cam_base_src_is_live:
 * @src: base source instance
 *
 * Check if an element is in live mode.
 *
 * Returns: %TRUE if element is in live mode.
 */
gboolean
gst_cam_base_src_is_live (GstCamBaseSrc * src)
{
  gboolean result;

  g_return_val_if_fail (GST_IS_CAM_BASE_SRC (src), FALSE);

  GST_OBJECT_LOCK (src);
  result = src->is_live;
  GST_OBJECT_UNLOCK (src);

  return result;
}

/**
 * gst_cam_base_src_set_format:
 * @src: base source instance
 * @format: the format to use
 * @name: the name of the pad to set format
 *
 * Sets the default format of the source. This will be the format used
 * for sending SEGMENT events and for performing seeks.
 *
 * If a format of GST_FORMAT_BYTES is set, the element will be able to
 * operate in pull mode if the #GstCamBaseSrcClass.is_seekable() returns %TRUE.
 *
 * This function must only be called in states < %GST_STATE_PAUSED.
 */
void
gst_cam_base_src_set_format (GstCamBaseSrc * src,
  GstFormat format, const char *padname)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));
  g_return_if_fail (GST_STATE (src) <= GST_STATE_READY);

  GST_OBJECT_LOCK (src);
  if (!strcmp(padname, GST_CAM_BASE_SRC_PAD_NAME))
    gst_segment_init (&src->segment, format);
  else {
    /* for request pads only */
    gst_segment_init (&src->mux[src->priv->request_stream_map[padname]].vid_segment, format);
  }
  GST_OBJECT_UNLOCK (src);
}

/**
 * gst_cam_base_src_set_dynamic_size:
 * @src: base source instance
 * @dynamic: new dynamic size mode
 *
 * If not @dynamic, size is only updated when needed, such as when trying to
 * read past current tracked size.  Otherwise, size is checked for upon each
 * read.
 */
void
gst_cam_base_src_set_dynamic_size (GstCamBaseSrc * src, gboolean dynamic)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));

  g_atomic_int_set (&src->priv->dynamic_size, dynamic);
}

/**
 * gst_cam_base_src_set_automatic_eos:
 * @src: base source instance
 * @automatic_eos: automatic eos
 *
 * If @automatic_eos is %TRUE, @src will automatically go EOS if a buffer
 * after the total size is returned. By default this is %TRUE but sources
 * that can't return an authoritative size and only know that they're EOS
 * when trying to read more should set this to %FALSE.
 *
 * Since: 1.4
 */
void
gst_cam_base_src_set_automatic_eos (GstCamBaseSrc * src, gboolean automatic_eos)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));

  g_atomic_int_set (&src->priv->automatic_eos, automatic_eos);
}

/**
 * gst_cam_base_src_set_async:
 * @src: base source instance
 * @async: new async mode
 *
 * Configure async behaviour in @src, no state change will block. The open,
 * close, start, stop, play and pause virtual methods will be executed in a
 * different thread and are thus allowed to perform blocking operations. Any
 * blocking operation should be unblocked with the unlock vmethod.
 */
void
gst_cam_base_src_set_async (GstCamBaseSrc * src, gboolean async)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));

  GST_OBJECT_LOCK (src);
  src->priv->async = async;
  GST_OBJECT_UNLOCK (src);
}

gboolean
gst_cam_base_src_is_async (GstCamBaseSrc * src)
{
  gboolean res;

  g_return_val_if_fail (GST_IS_CAM_BASE_SRC (src), FALSE);

  GST_OBJECT_LOCK (src);
  res = src->priv->async;
  GST_OBJECT_UNLOCK (src);

  return res;
}

/**
 * gst_cam_base_src_query_latency:
 * @src: the source
 * @live: (out) (allow-none): if the source is live
 * @min_latency: (out) (allow-none): the min latency of the source
 * @max_latency: (out) (allow-none): the max latency of the source
 *
 * Query the source for the latency parameters. @live will be %TRUE when @src is
 * configured as a live source. @min_latency and @max_latency will be set
 * to the difference between the running time and the timestamp of the first
 * buffer.
 *
 * This function is mostly used by subclasses.
 *
 * Returns: %TRUE if the query succeeded.
 */
gboolean
gst_cam_base_src_query_latency (GstCamBaseSrc * src, gboolean * live,
    GstClockTime * min_latency, GstClockTime * max_latency)
{
  GstClockTime min;

  g_return_val_if_fail (GST_IS_CAM_BASE_SRC (src), FALSE);

  GST_OBJECT_LOCK (src);
  if (live)
    *live = src->is_live;

  /* if we have a startup latency, report this one, else report 0. Subclasses
   * are supposed to override the query function if they want something
   * else. */
  if ((gint64)src->priv->latency != -1)
    min = src->priv->latency;
  else
    min = 0;

  if (min_latency)
    *min_latency = min;
  if (max_latency)
    *max_latency = min;

  GST_LOG_OBJECT (src, "latency: live %d, min %" GST_TIME_FORMAT
      ", max %" GST_TIME_FORMAT, src->is_live, GST_TIME_ARGS (min),
      GST_TIME_ARGS (min));
  GST_OBJECT_UNLOCK (src);

  return TRUE;
}

/**
 * gst_cam_base_src_set_blocksize:
 * @src: the source
 * @blocksize: the new blocksize in bytes
 *
 * Set the number of bytes that @src will push out with each buffer. When
 * @blocksize is set to -1, a default length will be used.
 */
void
gst_cam_base_src_set_blocksize (GstCamBaseSrc * src, guint blocksize)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));

  GST_OBJECT_LOCK (src);
  src->blocksize = blocksize;
  GST_OBJECT_UNLOCK (src);
}

/**
 * gst_cam_base_src_get_blocksize:
 * @src: the source
 *
 * Get the number of bytes that @src will push out with each buffer.
 *
 * Returns: the number of bytes pushed with each buffer.
 */
guint
gst_cam_base_src_get_blocksize (GstCamBaseSrc * src)
{
  gint res;

  g_return_val_if_fail (GST_IS_CAM_BASE_SRC (src), 0);

  GST_OBJECT_LOCK (src);
  res = src->blocksize;
  GST_OBJECT_UNLOCK (src);

  return res;
}

/**
 * gst_cam_base_src_set_do_timestamp:
 * @src: the source
 * @timestamp: enable or disable timestamping
 *
 * Configure @src to automatically timestamp outgoing buffers based on the
 * current running_time of the pipeline. This property is mostly useful for live
 * sources.
 */
void
gst_cam_base_src_set_do_timestamp (GstCamBaseSrc * src, gboolean timestamp)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));

  GST_OBJECT_LOCK (src);
  src->priv->do_timestamp = timestamp;
  if (timestamp && src->segment.format != GST_FORMAT_TIME)
    gst_segment_init (&src->segment, GST_FORMAT_TIME);
  GST_OBJECT_UNLOCK (src);
}

/**
 * gst_cam_base_src_get_do_timestamp:
 * @src: the source
 *
 * Query if @src timestamps outgoing buffers based on the current running_time.
 *
 * Returns: %TRUE if the base class will automatically timestamp outgoing buffers.
 */
gboolean
gst_cam_base_src_get_do_timestamp (GstCamBaseSrc * src)
{
  gboolean res;

  g_return_val_if_fail (GST_IS_CAM_BASE_SRC (src), FALSE);

  GST_OBJECT_LOCK (src);
  res = src->priv->do_timestamp;
  GST_OBJECT_UNLOCK (src);

  return res;
}

static gboolean
gst_cam_base_src_send_stream_start (GstCamBaseSrc * src)
{
  gboolean ret = TRUE;

  if (src->priv->stream_start_pending) {
    gchar *stream_id;
    gchar *name = gst_pad_get_name (src->srcpad);
    GstEvent *event;

    stream_id =
        gst_pad_create_stream_id (src->srcpad, GST_ELEMENT_CAST (src), name);

    GST_DEBUG_OBJECT (src, "%s pad: Pushing STREAM_START", name);
    event = gst_event_new_stream_start (stream_id);
    gst_event_set_group_id (event, gst_util_group_id_next ());

    ret = gst_pad_push_event (src->srcpad, event);
    src->priv->stream_start_pending = FALSE;
    g_free (stream_id);
    g_free (name);
  }

  return ret;
}

static gboolean
gst_cam_base_src_send_video_stream_start(GstCamBaseSrc *src, GstPad *pad)
{
  gboolean ret = TRUE;
  guint stream_id = CameraSrcUtils::get_stream_id_by_pad(src->priv->request_stream_map, pad);

  /* start video stream */
  if (src->priv->muxPriv[stream_id].vid_stream_start_pending) {
    gchar *vid_stream_id;
    gchar *name = gst_pad_get_name (src->mux[stream_id].videopad);
    GstEvent *event;

    vid_stream_id = gst_pad_create_stream_id (src->mux[stream_id].videopad, GST_ELEMENT_CAST(src), name);
    event = gst_event_new_stream_start(vid_stream_id);
    gst_event_set_group_id(event, gst_util_group_id_next());
    ret = gst_pad_push_event(src->mux[stream_id].videopad, event);
    src->priv->muxPriv[stream_id].vid_stream_start_pending = FALSE;
    g_free(vid_stream_id);
    g_free (name);
  }

  return ret;
}

/**
 * gst_cam_base_src_set_caps:
 * @src: a #GstCamBaseSrc
 * @pad: a #Gstpad that need to work on
 * @caps: (transfer none): a #GstCaps
 *
 * Set new caps on the basesrc source pad.
 *
 * Returns: %TRUE if the caps could be set
 */
gboolean
gst_cam_base_src_set_caps (GstCamBaseSrc * src, GstPad * pad, GstCaps * caps)
{
  GstCamBaseSrcClass *bclass;
  gboolean res = TRUE;
  GstCaps *current_caps = NULL;
  const char *format;
  int width, height;
  gchar *padname = gst_pad_get_name(pad);

  CameraSrcUtils::get_stream_info_by_caps(caps, &format, &width, &height);
  GST_INFO("pad:%s, format:%s width:%d height:%d\n",
    padname, format, width, height);

  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  /* src pad is the main data pad, we won't call set_caps until all
  * requested pads are done configuring
  */
  if (pad == src->srcpad) {
    gst_cam_base_src_send_stream_start (src);
  } else {
    gst_cam_base_src_send_video_stream_start(src, pad);
  }

  current_caps = gst_pad_get_current_caps (pad);
  if (current_caps && gst_caps_is_equal (current_caps, caps)) {
    GST_DEBUG_OBJECT (src, "New caps equal to old ones: %" GST_PTR_FORMAT,
        caps);
    res = TRUE;
  } else {
    if (bclass->set_caps)
      res = bclass->set_caps (src, pad, caps);

    if (res) {
      res = gst_pad_push_event (pad, gst_event_new_caps (caps));
    }
  }

  if (current_caps)
    gst_caps_unref (current_caps);

  g_free (padname);

  return res;
}

static GstCaps *
gst_cam_base_src_default_get_caps (GstCamBaseSrc *bsrc, GstCaps *filter)
{
  GstCaps *caps = NULL;
  GstPadTemplate *pad_template;
  GstCamBaseSrcClass *bclass;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (bsrc);

  pad_template =
      gst_element_class_get_pad_template (GST_ELEMENT_CLASS (bclass), GST_CAM_BASE_SRC_PAD_NAME);

  if (pad_template != NULL) {
    caps = gst_pad_template_get_caps (pad_template);

    if (filter) {
      GstCaps *intersection;

      intersection =
          gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
      gst_caps_unref (caps);
      caps = intersection;
    }
  }
  return caps;
}

static GstCaps *
gst_cam_base_src_default_fixate (GstCamBaseSrc * bsrc, GstCaps * caps)
{
  return gst_caps_fixate (caps);
}

static GstCaps *
gst_cam_base_src_fixate (GstCamBaseSrc * bsrc, GstCaps * caps)
{
  GstCamBaseSrcClass *bclass;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (bsrc);

  if (bclass->fixate)
    caps = bclass->fixate (bsrc, caps);

  return caps;
}

static gboolean
gst_cam_base_src_default_query (GstCamBaseSrc *src, GstQuery *query)
{
  gboolean res;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_POSITION:
    {
      GstFormat format;
      gst_query_parse_position (query, &format, NULL);

      GST_DEBUG_OBJECT (src, "position query in format %s",
          gst_format_get_name (format));

      switch (format) {
        case GST_FORMAT_PERCENT:
        {
          gint64 percent;
          gint64 position;
          gint64 duration;

          GST_OBJECT_LOCK (src);
          position = src->segment.position;
          duration = src->segment.duration;
          GST_OBJECT_UNLOCK (src);

          if (position != -1 && duration != -1) {
            if (position < duration)
              percent = gst_util_uint64_scale (GST_FORMAT_PERCENT_MAX, position,
                  duration);
            else
              percent = GST_FORMAT_PERCENT_MAX;
          } else
            percent = -1;

          gst_query_set_position (query, GST_FORMAT_PERCENT, percent);
          res = TRUE;
          break;
        }
        default:
        {
          gint64 position;
          GstFormat seg_format;

          GST_OBJECT_LOCK (src);
          position =
              gst_segment_to_stream_time (&src->segment, src->segment.format,
              src->segment.position);
          seg_format = src->segment.format;
          GST_OBJECT_UNLOCK (src);

          if (position != -1) {
            /* convert to requested format */
            res =
                gst_pad_query_convert (src->srcpad, seg_format,
                position, format, &position);
          } else
            res = TRUE;

          if (res)
            gst_query_set_position (query, format, position);

          break;
        }
      }
      break;
    }
    case GST_QUERY_DURATION:
    {
      GstFormat format;

      gst_query_parse_duration (query, &format, NULL);

      GST_DEBUG_OBJECT (src, "duration query in format %s",
          gst_format_get_name (format));

      switch (format) {
        case GST_FORMAT_PERCENT:
          gst_query_set_duration (query, GST_FORMAT_PERCENT,
              GST_FORMAT_PERCENT_MAX);
          res = TRUE;
          break;
        default:
        {
          gint64 duration;
          GstFormat seg_format;
          guint length = 0;

          /* may have to refresh duration */
          gst_cam_base_src_update_length (src, 0, &length,
              g_atomic_int_get (&src->priv->dynamic_size));

          /* this is the duration as configured by the subclass. */
          GST_OBJECT_LOCK (src);
          duration = src->segment.duration;
          seg_format = src->segment.format;
          GST_OBJECT_UNLOCK (src);

          GST_LOG_OBJECT (src, "duration %" G_GINT64_FORMAT ", format %s",
              duration, gst_format_get_name (seg_format));

          if (duration != -1) {
            /* convert to requested format, if this fails, we have a duration
             * but we cannot answer the query, we must return FALSE. */
            res =
                gst_pad_query_convert (src->srcpad, seg_format,
                duration, format, &duration);
          } else {
            /* The subclass did not configure a duration, we assume that the
             * media has an unknown duration then and we return TRUE to report
             * this. Note that this is not the same as returning FALSE, which
             * means that we cannot report the duration at all. */
            res = TRUE;
          }

          if (res)
            gst_query_set_duration (query, format, duration);

          break;
        }
      }
      break;
    }
    case GST_QUERY_SEEKING:
    {
      GstFormat format, seg_format;
      gint64 duration;

      GST_OBJECT_LOCK (src);
      duration = src->segment.duration;
      seg_format = src->segment.format;
      GST_OBJECT_UNLOCK (src);

      gst_query_parse_seeking (query, &format, NULL, NULL, NULL);
      if (format == seg_format) {
        gst_query_set_seeking (query, seg_format,
            gst_cam_base_src_seekable (src), 0, duration);
        res = TRUE;
      } else {
        /* FIXME 2.0: return TRUE + seekable=FALSE for SEEKING query here */
        /* Don't reply to the query to make up for demuxers which don't
         * handle the SEEKING query yet. Players like Totem will fall back
         * to the duration when the SEEKING query isn't answered. */
        res = FALSE;
      }
      break;
    }
    case GST_QUERY_SEGMENT:
    {
      GstFormat format;
      gint64 start, stop;

      GST_OBJECT_LOCK (src);

      format = src->segment.format;

      start =
          gst_segment_to_stream_time (&src->segment, format,
          src->segment.start);
      if ((stop = src->segment.stop) == -1)
        stop = src->segment.duration;
      else
        stop = gst_segment_to_stream_time (&src->segment, format, stop);

      gst_query_set_segment (query, src->segment.rate, format, start, stop);

      GST_OBJECT_UNLOCK (src);
      res = TRUE;
      break;
    }
    case GST_QUERY_FORMATS:
    {
      gst_query_set_formats (query, 3, GST_FORMAT_DEFAULT,
          GST_FORMAT_BYTES, GST_FORMAT_PERCENT);
      res = TRUE;
      break;
    }
    case GST_QUERY_CONVERT:
    {
      GstFormat src_fmt, dest_fmt;
      gint64 src_val, dest_val;

      gst_query_parse_convert (query, &src_fmt, &src_val, &dest_fmt, &dest_val);

      /* we can only convert between equal formats... */
      if (src_fmt == dest_fmt) {
        dest_val = src_val;
        res = TRUE;
      } else
        res = FALSE;

      gst_query_set_convert (query, src_fmt, src_val, dest_fmt, dest_val);
      break;
    }
    case GST_QUERY_LATENCY:
    {
      GstClockTime min, max;
      gboolean live;

      /* Subclasses should override and implement something useful */
      res = gst_cam_base_src_query_latency (src, &live, &min, &max);

      GST_LOG_OBJECT (src, "report latency: live %d, min %" GST_TIME_FORMAT
          ", max %" GST_TIME_FORMAT, live, GST_TIME_ARGS (min),
          GST_TIME_ARGS (max));

      gst_query_set_latency (query, live, min, max);
      break;
    }
    case GST_QUERY_JITTER:
    case GST_QUERY_RATE:
      res = FALSE;
      break;
    case GST_QUERY_BUFFERING:
    {
      GstFormat format, seg_format;
      gint64 start, stop, estimated;

      gst_query_parse_buffering_range (query, &format, NULL, NULL, NULL);

      GST_DEBUG_OBJECT (src, "buffering query in format %s",
          gst_format_get_name (format));

      GST_OBJECT_LOCK (src);
      if (src->random_access) {
        estimated = 0;
        start = 0;
        if (format == GST_FORMAT_PERCENT)
          stop = GST_FORMAT_PERCENT_MAX;
        else
          stop = src->segment.duration;
      } else {
        estimated = -1;
        start = -1;
        stop = -1;
      }
      seg_format = src->segment.format;
      GST_OBJECT_UNLOCK (src);

      /* convert to required format. When the conversion fails, we can't answer
       * the query. When the value is unknown, we can don't perform conversion
       * but report TRUE. */
      if (format != GST_FORMAT_PERCENT && stop != -1) {
        res = gst_pad_query_convert (src->srcpad, seg_format,
            stop, format, &stop);
      } else {
        res = TRUE;
      }
      if (res && format != GST_FORMAT_PERCENT && start != -1)
        res = gst_pad_query_convert (src->srcpad, seg_format,
            start, format, &start);

      gst_query_set_buffering_range (query, format, start, stop, estimated);
      break;
    }
    case GST_QUERY_SCHEDULING:
    {
      gboolean random_access;

      random_access = gst_cam_base_src_is_random_access (src);

      /* we can operate in getrange mode if the native format is bytes
       * and we are seekable, this condition is set in the random_access
       * flag and is set in the _start() method. */
      gst_query_set_scheduling (query, GST_SCHEDULING_FLAG_SEEKABLE, 1, -1, 0);
      if (random_access)
        gst_query_add_scheduling_mode (query, GST_PAD_MODE_PULL);
      gst_query_add_scheduling_mode (query, GST_PAD_MODE_PUSH);

      res = TRUE;
      break;
    }
    case GST_QUERY_CAPS:
    {
      GstCamBaseSrcClass *bclass;
      GstCaps *caps, *filter;

      bclass = GST_CAM_BASE_SRC_GET_CLASS (src);
      if (bclass->get_caps) {
        gst_query_parse_caps (query, &filter);
        if ((caps = bclass->get_caps (src, filter))) {
          gst_query_set_caps_result (query, caps);
          gst_caps_unref (caps);
          res = TRUE;
        } else {
          res = FALSE;
        }
      } else
        res = FALSE;
      break;
    }
    case GST_QUERY_URI:
    {
      if (GST_IS_URI_HANDLER (src)) {
        gchar *uri = gst_uri_handler_get_uri (GST_URI_HANDLER (src));

        if (uri != NULL) {
          gst_query_set_uri (query, uri);
          g_free (uri);
          res = TRUE;
        } else {
          res = FALSE;
        }
      } else {
        res = FALSE;
      }
      break;
    }
    default:
      res = FALSE;
      break;
  }
  GST_DEBUG_OBJECT (src, "query %s returns %d", GST_QUERY_TYPE_NAME (query),
      res);

  return res;
}

static gboolean
gst_cam_base_src_query (GstPad * pad, GstObject * parent,
    GstQuery * query)
{
  GstCamBaseSrc *src;
  GstCamBaseSrcClass *bclass;
  gboolean result = FALSE;

  src = GST_CAM_BASE_SRC (parent);
  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  if (bclass->query)
    result = bclass->query (src, query);

  return result;
}

static gboolean
gst_cam_base_src_video_query (GstPad *pad, GstObject *parent,
    GstQuery *query)
{
  gboolean result = FALSE;

  switch (GST_QUERY_TYPE(query)) {
    default:
      result = gst_pad_query_default (pad, parent, query);
      break;
  }
  return result;
}

static gboolean
gst_cam_base_src_default_do_seek (GstCamBaseSrc * src, GstSegment * segment)
{
  gboolean res = TRUE;

  /* update our offset if the start/stop position was updated */
  if (segment->format == GST_FORMAT_BYTES) {
    segment->time = segment->start;
  } else if (segment->start == 0) {
    /* seek to start, we can implement a default for this. */
    segment->time = 0;
  } else {
    res = FALSE;
    GST_INFO_OBJECT (src, "Can't do a default seek");
  }

  return res;
}

static gboolean
gst_cam_base_src_do_seek (GstCamBaseSrc * src, GstSegment * segment)
{
  GstCamBaseSrcClass *bclass;
  gboolean result = FALSE;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  GST_INFO_OBJECT (src, "seeking: %" GST_SEGMENT_FORMAT, segment);

  if (bclass->do_seek)
    result = bclass->do_seek (src, segment);

  return result;
}

static gboolean
gst_cam_base_src_default_prepare_seek_segment (GstCamBaseSrc * src, GstEvent * event,
    GstSegment * segment)
{
  /* By default, we try one of 2 things:
   *   - For absolute seek positions, convert the requested position to our
   *     configured processing format and place it in the output segment \
   *   - For relative seek positions, convert our current (input) values to the
   *     seek format, adjust by the relative seek offset and then convert back to
   *     the processing format
   */
  GstSeekType start_type, stop_type;
  gint64 start, stop;
  GstSeekFlags flags;
  GstFormat seek_format, dest_format;
  gdouble rate;
  gboolean update;
  gboolean res = TRUE;

  gst_event_parse_seek (event, &rate, &seek_format, &flags,
      &start_type, &start, &stop_type, &stop);
  dest_format = segment->format;

  if (seek_format == dest_format) {
    gst_segment_do_seek (segment, rate, seek_format, flags,
        start_type, start, stop_type, stop, &update);
    return TRUE;
  }

  if (start_type != GST_SEEK_TYPE_NONE) {
    /* FIXME: Handle seek_end by converting the input segment vals */
    res =
        gst_pad_query_convert (src->srcpad, seek_format, start, dest_format,
        &start);
    start_type = GST_SEEK_TYPE_SET;
  }

  if (res && stop_type != GST_SEEK_TYPE_NONE) {
    /* FIXME: Handle seek_end by converting the input segment vals */
    res =
        gst_pad_query_convert (src->srcpad, seek_format, stop, dest_format,
        &stop);
    stop_type = GST_SEEK_TYPE_SET;
  }

  /* And finally, configure our output segment in the desired format */
  gst_segment_do_seek (segment, rate, dest_format, flags, start_type, start,
      stop_type, stop, &update);

  if (!res)
    goto no_format;

  return res;

no_format:
  {
    GST_DEBUG_OBJECT (src, "undefined format given, seek aborted.");
    return FALSE;
  }
}

static gboolean
gst_cam_base_src_prepare_seek_segment (GstCamBaseSrc * src, GstEvent * event,
    GstSegment * seeksegment)
{
  GstCamBaseSrcClass *bclass;
  gboolean result = FALSE;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  if (bclass->prepare_seek_segment)
    result = bclass->prepare_seek_segment (src, event, seeksegment);

  return result;
}

static GstFlowReturn
gst_cam_base_src_default_alloc (GstCamBaseSrc * src, GstPad *pad,
    guint64 offset, guint size, GstBuffer ** buffer)
{
  GstFlowReturn ret;
  GstCamBaseSrcPrivate *priv = src->priv;
  GstBufferPool *pool = NULL;
  GstAllocator *allocator = NULL;
  GstAllocationParams params;

  /* distinguish which pad is using, then assign pointers to
   * bufferpool, allocator and params */
  if (pad == src->srcpad) {
    pool = priv->pool;
    allocator = priv->allocator;
    params = priv->params;
  } else {
    guint stream_id = CameraSrcUtils::get_stream_id_by_pad(priv->request_stream_map, pad);
    pool = priv->muxPriv[stream_id].vid_pool;
    allocator = priv->muxPriv[stream_id].vid_allocator;
    params = priv->muxPriv[stream_id].vid_params;
  }

  if (pool) {
    ret = gst_buffer_pool_acquire_buffer (pool, buffer, NULL);
  } else if ((gint64)size > 0) {
    *buffer = gst_buffer_new_allocate (allocator, size, &params);
    if (G_UNLIKELY (*buffer == NULL))
      goto alloc_failed;

    ret = GST_FLOW_OK;
  } else {
    GST_WARNING_OBJECT (src, "Not trying to alloc %u bytes. Blocksize not set?",
        size);
    ret = GST_FLOW_ERROR;
  }
  return ret;

  /* ERRORS */
alloc_failed:
  {
    GST_ERROR_OBJECT (src, "Failed to allocate %u bytes", size);
    return GST_FLOW_ERROR;
  }
}

static GstFlowReturn
gst_cam_base_src_default_create (GstCamBaseSrc * src, GstPad *pad,
    guint64 offset, guint size, GstBuffer ** buffer)
{
  GstCamBaseSrcClass *bclass;
  GstFlowReturn ret;
  GstBuffer *res_buf;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  if (G_UNLIKELY (!bclass->alloc))
    goto no_function;
  if (G_UNLIKELY (!bclass->fill))
    goto no_function;

  if (*buffer == NULL) {
    /* downstream did not provide us with a buffer to fill, allocate one
     * ourselves */
    ret = bclass->alloc (src, pad, offset, size, &res_buf);
    if (G_UNLIKELY (ret != GST_FLOW_OK))
      goto alloc_failed;
  } else {
    res_buf = *buffer;
  }

  if (G_LIKELY (size > 0)) {
    /* only call fill when there is a size */
    ret = bclass->fill (src, pad, offset, size, res_buf);
    if (G_UNLIKELY (ret != GST_FLOW_OK))
      goto not_ok;
  }

  *buffer = res_buf;

  return GST_FLOW_OK;

  /* ERRORS */
no_function:
  {
    GST_DEBUG_OBJECT (src, "no fill or alloc function");
    return GST_FLOW_NOT_SUPPORTED;
  }
alloc_failed:
  {
    GST_DEBUG_OBJECT (src, "Failed to allocate buffer of %u bytes", size);
    return ret;
  }
not_ok:
  {
    GST_DEBUG_OBJECT (src, "fill returned %d (%s)", ret,
        gst_flow_get_name (ret));
    if (*buffer == NULL)
      gst_buffer_unref (res_buf);
    return ret;
  }
}

/* this code implements the seeking. It is a good example
 * handling all cases.
 *
 * A seek updates the currently configured segment.start
 * and segment.stop values based on the SEEK_TYPE. If the
 * segment.start value is updated, a seek to this new position
 * should be performed.
 *
 * The seek can only be executed when we are not currently
 * streaming any data, to make sure that this is the case, we
 * acquire the STREAM_LOCK which is taken when we are in the
 * _loop() function or when a getrange() is called. Normally
 * we will not receive a seek if we are operating in pull mode
 * though. When we operate as a live source we might block on the live
 * cond, which does not release the STREAM_LOCK. Therefore we will try
 * to grab the LIVE_LOCK instead of the STREAM_LOCK to make sure it is
 * safe to perform the seek.
 *
 * When we are in the loop() function, we might be in the middle
 * of pushing a buffer, which might block in a sink. To make sure
 * that the push gets unblocked we push out a FLUSH_START event.
 * Our loop function will get a FLUSHING return value from
 * the push and will pause, effectively releasing the STREAM_LOCK.
 *
 * For a non-flushing seek, we pause the task, which might eventually
 * release the STREAM_LOCK. We say eventually because when the sink
 * blocks on the sample we might wait a very long time until the sink
 * unblocks the sample. In any case we acquire the STREAM_LOCK and
 * can continue the seek. A non-flushing seek is normally done in a
 * running pipeline to perform seamless playback, this means that the sink is
 * PLAYING and will return from its chain function.
 * In the case of a non-flushing seek we need to make sure that the
 * data we output after the seek is continuous with the previous data,
 * this is because a non-flushing seek does not reset the running-time
 * to 0. We do this by closing the currently running segment, ie. sending
 * a new_segment event with the stop position set to the last processed
 * position.
 *
 * After updating the segment.start/stop values, we prepare for
 * streaming again. We push out a FLUSH_STOP to make the peer pad
 * accept data again and we start our task again.
 *
 * A segment seek posts a message on the bus saying that the playback
 * of the segment started. We store the segment flag internally because
 * when we reach the segment.stop we have to post a segment.done
 * instead of EOS when doing a segment seek.
 */
static gboolean
gst_cam_base_src_perform_seek (GstCamBaseSrc * src, GstEvent * event, gboolean unlock)
{
  gboolean res = TRUE, tres;
  gdouble rate;
  GstFormat seek_format, dest_format;
  GstSeekFlags flags;
  GstSeekType start_type, stop_type;
  gint64 start, stop;
  gboolean flush, playing;
  gboolean update;
  gboolean relative_seek = FALSE;
  gboolean seekseg_configured = FALSE;
  GstSegment seeksegment;
  guint32 seqnum;
  GstEvent *tevent;

  GST_DEBUG_OBJECT (src, "doing seek: %" GST_PTR_FORMAT, event);

  GST_OBJECT_LOCK (src);
  dest_format = src->segment.format;
  GST_OBJECT_UNLOCK (src);

  if (event) {
    gst_event_parse_seek (event, &rate, &seek_format, &flags,
        &start_type, &start, &stop_type, &stop);

    relative_seek = SEEK_TYPE_IS_RELATIVE (start_type) ||
        SEEK_TYPE_IS_RELATIVE (stop_type);

    if (dest_format != seek_format && !relative_seek) {
      /* If we have an ABSOLUTE position (SEEK_SET only), we can convert it
       * here before taking the stream lock, otherwise we must convert it later,
       * once we have the stream lock and can read the last configures segment
       * start and stop positions */
      gst_segment_init (&seeksegment, dest_format);

      if (!gst_cam_base_src_prepare_seek_segment (src, event, &seeksegment))
        goto prepare_failed;

      seekseg_configured = TRUE;
    }

    flush = flags & GST_SEEK_FLAG_FLUSH;
    seqnum = gst_event_get_seqnum (event);
  } else {
    flush = FALSE;
    /* get next seqnum */
    seqnum = gst_util_seqnum_next ();
  }

  /* send flush start */
  if (flush) {
    tevent = gst_event_new_flush_start ();
    gst_event_set_seqnum (tevent, seqnum);
    gst_pad_push_event (src->srcpad, tevent);
  } else
    gst_pad_pause_task (src->srcpad);

  /* unblock streaming thread. */
  if (unlock)
    gst_cam_base_src_set_flushing (src, TRUE, FALSE, &playing);

  /* grab streaming lock, this should eventually be possible, either
   * because the task is paused, our streaming thread stopped
   * or because our peer is flushing. */
  GST_PAD_STREAM_LOCK (src->srcpad);
  if (G_UNLIKELY (src->priv->seqnum == seqnum)) {
    /* we have seen this event before, issue a warning for now */
    GST_WARNING_OBJECT (src, "duplicate event found %" G_GUINT32_FORMAT,
        seqnum);
  } else {
    src->priv->seqnum = seqnum;
    GST_DEBUG_OBJECT (src, "seek with seqnum %" G_GUINT32_FORMAT, seqnum);
  }

  if (unlock)
    gst_cam_base_src_set_flushing (src, FALSE, playing, NULL);

  /* If we configured the seeksegment above, don't overwrite it now. Otherwise
   * copy the current segment info into the temp segment that we can actually
   * attempt the seek with. We only update the real segment if the seek succeeds. */
  if (!seekseg_configured) {
    MEMCPY_S (&seeksegment, sizeof (GstSegment), &src->segment, sizeof (GstSegment));

    /* now configure the final seek segment */
    if (event) {
      if (seeksegment.format != seek_format) {
        /* OK, here's where we give the subclass a chance to convert the relative
         * seek into an absolute one in the processing format. We set up any
         * absolute seek above, before taking the stream lock. */
        if (!gst_cam_base_src_prepare_seek_segment (src, event, &seeksegment)) {
          GST_DEBUG_OBJECT (src, "Preparing the seek failed after flushing. "
              "Aborting seek");
          res = FALSE;
        }
      } else {
        /* The seek format matches our processing format, no need to ask the
         * the subclass to configure the segment. */
        gst_segment_do_seek (&seeksegment, rate, seek_format, flags,
            start_type, start, stop_type, stop, &update);
      }
    }
    /* Else, no seek event passed, so we're just (re)starting the
       current segment. */
  }

  if (res) {
    GST_DEBUG_OBJECT (src, "segment configured from %" G_GINT64_FORMAT
        " to %" G_GINT64_FORMAT ", position %" G_GINT64_FORMAT,
        seeksegment.start, seeksegment.stop, seeksegment.position);

    /* do the seek, segment.position contains the new position. */
    res = gst_cam_base_src_do_seek (src, &seeksegment);
  }

  /* and prepare to continue streaming */
  if (flush) {
    tevent = gst_event_new_flush_stop (TRUE);
    gst_event_set_seqnum (tevent, seqnum);
    /* send flush stop, peer will accept data and events again. We
     * are not yet providing data as we still have the STREAM_LOCK. */
    gst_pad_push_event (src->srcpad, tevent);
  }

  /* The subclass must have converted the segment to the processing format
   * by now */
  if (res && seeksegment.format != dest_format) {
    GST_DEBUG_OBJECT (src, "Subclass failed to prepare a seek segment "
        "in the correct format. Aborting seek.");
    res = FALSE;
  }

  /* if the seek was successful, we update our real segment and push
   * out the new segment. */
  if (res) {
    GST_OBJECT_LOCK (src);
    MEMCPY_S (&src->segment, sizeof (GstSegment), &seeksegment, sizeof (GstSegment));
    GST_OBJECT_UNLOCK (src);

    if (seeksegment.flags & GST_SEGMENT_FLAG_SEGMENT) {
      GstMessage *message;

      message = gst_message_new_segment_start (GST_OBJECT (src),
          seeksegment.format, seeksegment.position);
      gst_message_set_seqnum (message, seqnum);

      gst_element_post_message (GST_ELEMENT (src), message);
    }

    src->priv->segment_pending = TRUE;
    src->priv->segment_seqnum = seqnum;
  }

  src->priv->discont = TRUE;
  src->running = TRUE;
  /* and restart the task in case it got paused explicitly or by
   * the FLUSH_START event we pushed out. */
  tres = gst_pad_start_task (src->srcpad, (GstTaskFunction) gst_cam_base_src_loop,
      src->srcpad, NULL);
  if (res && !tres)
    res = FALSE;

  /* and release the lock again so we can continue streaming */
  GST_PAD_STREAM_UNLOCK (src->srcpad);

  return res;

  /* ERROR */
prepare_failed:
  GST_DEBUG_OBJECT (src, "Preparing the seek failed before flushing. "
      "Aborting seek");
  return FALSE;
}

/* all events send to this element directly. This is mainly done from the
 * application.
 */
static gboolean
gst_cam_base_src_send_event (GstElement * element, GstEvent * event)
{
  GstCamBaseSrc *src;
  gboolean result = FALSE;
  GstCamBaseSrcClass *bclass;

  src = GST_CAM_BASE_SRC (element);
  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  GST_DEBUG_OBJECT (src, "handling event %p %" GST_PTR_FORMAT, event, event);

  switch (GST_EVENT_TYPE (event)) {
      /* bidirectional events */
    case GST_EVENT_FLUSH_START:
      GST_DEBUG_OBJECT (src, "pushing flush-start event downstream");
      result = gst_pad_push_event (src->srcpad, event);
      /* also unblock the create function */
      gst_cam_base_src_activate_pool (src, FALSE);
      /* unlock any subclasses, we need to do this before grabbing the
       * LIVE_LOCK since we hold this lock before going into ::create. We pass an
       * unlock to the params because of backwards compat (see seek handler)*/
      if (bclass->unlock)
        bclass->unlock (src);

      /* the live lock is released when we are blocked, waiting for playing or
       * when we sync to the clock. */
      GST_LIVE_LOCK (src);
      src->priv->flushing = TRUE;
      /* clear pending EOS if any */
      if (g_atomic_int_get (&src->priv->has_pending_eos)) {
        GST_OBJECT_LOCK (src);
        CLEAR_PENDING_EOS (src);
        src->priv->forced_eos = FALSE;
        GST_OBJECT_UNLOCK (src);
      }
      if (bclass->unlock_stop)
        bclass->unlock_stop (src);
      if (src->clock_id)
        gst_clock_id_unschedule (src->clock_id);
      GST_DEBUG_OBJECT (src, "signal");
      GST_LIVE_SIGNAL (src);
      GST_LIVE_UNLOCK (src);
      event = NULL;
      break;
    case GST_EVENT_FLUSH_STOP:
    {
      gboolean start;

      GST_LIVE_LOCK (src);
      src->priv->segment_pending = TRUE;
      src->priv->flushing = FALSE;
      GST_DEBUG_OBJECT (src, "pushing flush-stop event downstream");
      result = gst_pad_push_event (src->srcpad, event);

      gst_cam_base_src_activate_pool (src, TRUE);

      GST_OBJECT_LOCK (src->srcpad);
      start = (GST_PAD_MODE (src->srcpad) == GST_PAD_MODE_PUSH);
      GST_OBJECT_UNLOCK (src->srcpad);

      if (src->is_live) {
        if (!src->live_running)
          start = FALSE;
      }

      if (start)
        gst_pad_start_task (src->srcpad, (GstTaskFunction) gst_cam_base_src_loop,
            src->srcpad, NULL);
      GST_LIVE_UNLOCK (src);
      event = NULL;
      break;
    }

      /* downstream serialized events */
    case GST_EVENT_EOS:
    {
      /* queue EOS and make sure the task or pull function performs the EOS
       * actions.
       *
       * We have two possibilities:
       *
       *  - Before we are to enter the _create function, we check the has_pending_eos
       *    first and do EOS instead of entering it.
       *  - If we are in the _create function or we did not manage to set the
       *    flag fast enough and we are about to enter the _create function,
       *    we unlock it so that we exit with FLUSHING immediately. We then
       *    check the EOS flag and do the EOS logic.
       */
      GST_OBJECT_LOCK (src);
      g_atomic_int_set (&src->priv->has_pending_eos, TRUE);
      if (src->priv->pending_eos)
        gst_event_unref (src->priv->pending_eos);
      src->priv->pending_eos = event;
      event = NULL;
      GST_OBJECT_UNLOCK (src);

      GST_DEBUG_OBJECT (src, "EOS marked, calling unlock");

      /* unlock the _create function so that we can check the has_pending_eos flag
       * and we can do EOS. This will eventually release the LIVE_LOCK again so
       * that we can grab it and stop the unlock again. We don't take the stream
       * lock so that this operation is guaranteed to never block. */
      gst_cam_base_src_activate_pool (src, FALSE);
      if (bclass->unlock)
        bclass->unlock (src);

      GST_DEBUG_OBJECT (src, "unlock called, waiting for LIVE_LOCK");

      GST_LIVE_LOCK (src);
      GST_DEBUG_OBJECT (src, "LIVE_LOCK acquired, calling unlock_stop");
      /* now stop the unlock of the streaming thread again. Grabbing the live
       * lock is enough because that protects the create function. */
      if (bclass->unlock_stop)
        bclass->unlock_stop (src);
      gst_cam_base_src_activate_pool (src, TRUE);
      GST_LIVE_UNLOCK (src);

      result = TRUE;
      break;
    }
    case GST_EVENT_SEGMENT:
      /* sending random SEGMENT downstream can break sync. */
      break;
    case GST_EVENT_TAG:
    case GST_EVENT_CUSTOM_DOWNSTREAM:
    case GST_EVENT_CUSTOM_BOTH:
      /* Insert TAG, CUSTOM_DOWNSTREAM, CUSTOM_BOTH in the dataflow */
      GST_OBJECT_LOCK (src);
      src->priv->pending_events =
          g_list_append (src->priv->pending_events, event);
      g_atomic_int_set (&src->priv->have_events, TRUE);
      GST_OBJECT_UNLOCK (src);
      event = NULL;
      result = TRUE;
      break;
    case GST_EVENT_BUFFERSIZE:
      /* does not seem to make much sense currently */
      break;

      /* upstream events */
    case GST_EVENT_QOS:
      /* elements should override send_event and do something */
      break;
    case GST_EVENT_SEEK:
    {
      gboolean started;

      GST_OBJECT_LOCK (src->srcpad);
      if (GST_PAD_MODE (src->srcpad) == GST_PAD_MODE_PULL)
        goto wrong_mode;
      started = GST_PAD_MODE (src->srcpad) == GST_PAD_MODE_PUSH;
      GST_OBJECT_UNLOCK (src->srcpad);

      if (started) {
        GST_DEBUG_OBJECT (src, "performing seek");
        /* when we are running in push mode, we can execute the
         * seek right now. */
        result = gst_cam_base_src_perform_seek (src, event, TRUE);
      } else {
        GstEvent **event_p;

        /* else we store the event and execute the seek when we
         * get activated */
        GST_OBJECT_LOCK (src);
        GST_DEBUG_OBJECT (src, "queueing seek");
        event_p = &src->pending_seek;
        gst_event_replace ((GstEvent **) event_p, event);
        GST_OBJECT_UNLOCK (src);
        /* assume the seek will work */
        result = TRUE;
      }
      break;
    }
    case GST_EVENT_NAVIGATION:
      /* could make sense for elements that do something with navigation events
       * but then they would need to override the send_event function */
      break;
    case GST_EVENT_LATENCY:
      /* does not seem to make sense currently */
      break;

      /* custom events */
    case GST_EVENT_CUSTOM_UPSTREAM:
      /* override send_event if you want this */
      break;
    case GST_EVENT_CUSTOM_DOWNSTREAM_OOB:
    case GST_EVENT_CUSTOM_BOTH_OOB:
      /* insert a random custom event into the pipeline */
      GST_DEBUG_OBJECT (src, "pushing custom OOB event downstream");
      result = gst_pad_push_event (src->srcpad, event);
      /* we gave away the ref to the event in the push */
      event = NULL;
      break;
    default:
      break;
  }
done:
  /* if we still have a ref to the event, unref it now */
  if (event)
    gst_event_unref (event);

  return result;

  /* ERRORS */
wrong_mode:
  {
    GST_DEBUG_OBJECT (src, "cannot perform seek when operating in pull mode");
    GST_OBJECT_UNLOCK (src->srcpad);
    result = FALSE;
    goto done;
  }
}

static gboolean
gst_cam_base_src_seekable (GstCamBaseSrc * src)
{
  GstCamBaseSrcClass *bclass;
  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);
  if (bclass->is_seekable)
    return bclass->is_seekable (src);
  else
    return FALSE;
}

static void
gst_cam_base_src_update_qos (GstCamBaseSrc * src,
    gdouble proportion, GstClockTimeDiff diff, GstClockTime timestamp)
{
  GST_CAT_DEBUG_OBJECT (_gst_debug_category_new ("GST_QOS", 0, "QoS"), src,
      "qos: proportion: %lf, diff %" G_GINT64_FORMAT ", timestamp %"
      GST_TIME_FORMAT, proportion, diff, GST_TIME_ARGS (timestamp));

  GST_OBJECT_LOCK (src);
  src->priv->proportion = proportion;
  src->priv->earliest_time = timestamp + diff;
  GST_OBJECT_UNLOCK (src);
}

static gboolean gst_cam_base_src_default_event (GstCamBaseSrc * src, GstEvent * event)
{
  gboolean result;

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
      /* is normally called when in push mode */
      if (!gst_cam_base_src_seekable (src))
        goto not_seekable;

      result = gst_cam_base_src_perform_seek (src, event, TRUE);
      break;
    case GST_EVENT_FLUSH_START:
      /* cancel any blocking getrange, is normally called
       * when in pull mode. */
      result = gst_cam_base_src_set_flushing (src, TRUE, FALSE, NULL);
      break;
    case GST_EVENT_FLUSH_STOP:
      result = gst_cam_base_src_set_flushing (src, FALSE, TRUE, NULL);
      break;
    case GST_EVENT_QOS:
    {
      gdouble proportion;
      GstClockTimeDiff diff;
      GstClockTime timestamp;

      gst_event_parse_qos (event, NULL, &proportion, &diff, &timestamp);
      gst_cam_base_src_update_qos (src, proportion, diff, timestamp);
      result = TRUE;
      break;
    }
    case GST_EVENT_RECONFIGURE:
      result = TRUE;
      break;
    case GST_EVENT_LATENCY:
      result = TRUE;
      break;
    default:
      result = FALSE;
      break;
  }
  return result;

  /* ERRORS */
not_seekable:
  {
    GST_DEBUG_OBJECT (src, "is not seekable");
    return FALSE;
  }
}

static gboolean
gst_cam_base_src_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  GstCamBaseSrc *src;
  GstCamBaseSrcClass *bclass;
  gboolean result = FALSE;

  src = GST_CAM_BASE_SRC (parent);
  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  if (bclass->event) {
    if (!(result = bclass->event (src, event)))
      goto subclass_failed;
  }

done:
  gst_event_unref (event);

  return result;

  /* ERRORS */
subclass_failed:
  {
    GST_DEBUG_OBJECT (src, "subclass refused event");
    goto done;
  }
}

static void
gst_cam_base_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstCamBaseSrc *src;

  src = GST_CAM_BASE_SRC (object);

  switch (prop_id) {
    case PROP_BLOCKSIZE:
      gst_cam_base_src_set_blocksize (src, g_value_get_uint (value));
      break;
    case PROP_NUM_BUFFERS:
      src->num_buffers = g_value_get_int (value);
      break;
    case PROP_TYPEFIND:
      src->typefind = g_value_get_boolean (value);
      break;
    case PROP_DO_TIMESTAMP:
      gst_cam_base_src_set_do_timestamp (src, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_cam_base_src_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstCamBaseSrc *src;

  src = GST_CAM_BASE_SRC (object);

  switch (prop_id) {
    case PROP_BLOCKSIZE:
      g_value_set_uint (value, gst_cam_base_src_get_blocksize (src));
      break;
    case PROP_NUM_BUFFERS:
      g_value_set_int (value, src->num_buffers);
      break;
    case PROP_TYPEFIND:
      g_value_set_boolean (value, src->typefind);
      break;
    case PROP_DO_TIMESTAMP:
      g_value_set_boolean (value, gst_cam_base_src_get_do_timestamp (src));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* with STREAM_LOCK and LOCK */
static GstClockReturn
gst_cam_base_src_wait (GstCamBaseSrc * basesrc, GstClock * clock, GstClockTime time)
{
  GstClockReturn ret;
  GstClockID id;

  id = gst_clock_new_single_shot_id (clock, time);

  basesrc->clock_id = id;
  /* release the live lock while waiting */
  GST_LIVE_UNLOCK (basesrc);

  ret = gst_clock_id_wait (id, NULL);

  GST_LIVE_LOCK (basesrc);
  gst_clock_id_unref (id);
  basesrc->clock_id = NULL;

  return ret;
}

/* perform synchronisation on a buffer.
 * with STREAM_LOCK.
 */
static GstClockReturn
gst_cam_base_src_do_sync (GstCamBaseSrc * basesrc, GstBuffer * buffer)
{
  GstClockReturn result;
  GstClockTime start, end;
  GstCamBaseSrcClass *bclass;
  GstClockTime base_time;
  GstClock *clock;
  GstClockTime now = GST_CLOCK_TIME_NONE, pts, dts;
  gboolean do_timestamp, first, is_live;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);

  start = end = -1;
  if (bclass->get_times)
    bclass->get_times (basesrc, buffer, &start, &end);

  /* get buffer timestamp */
  dts = GST_BUFFER_DTS (buffer);
  pts = GST_BUFFER_PTS (buffer);

  /* grab the lock to prepare for clocking and calculate the startup
   * latency. */
  GST_OBJECT_LOCK (basesrc);

  is_live = basesrc->is_live;
  /* check for the first buffer */
  first = ((gint64)basesrc->priv->latency == -1);

  if (first) {
    GST_DEBUG_OBJECT (basesrc, "src pad: no latency needed, live %d, sync %d",
        is_live, (gint64)start != -1);
    basesrc->priv->latency = 0;
  }

  /* get clock, if no clock, we can't sync or do timestamps */
  if ((clock = GST_ELEMENT_CLOCK (basesrc)) == NULL)
    goto no_clock;
  else
    gst_object_ref (clock);

  base_time = GST_ELEMENT_CAST (basesrc)->base_time;

  do_timestamp = basesrc->priv->do_timestamp;
  GST_OBJECT_UNLOCK (basesrc);

  /* first buffer, calculate the timestamp offset */
  if (first) {
    GstClockTime running_time;

    now = gst_clock_get_time (clock);
    running_time = now - base_time;

    GST_LOG_OBJECT (basesrc,
        "src pad: startup PTS: %" GST_TIME_FORMAT ", DTS %" GST_TIME_FORMAT
        ", running_time %" GST_TIME_FORMAT, GST_TIME_ARGS (pts),
        GST_TIME_ARGS (dts), GST_TIME_ARGS (running_time));

    basesrc->priv->ts_offset = 0;
    GST_LOG_OBJECT (basesrc, "src pad: no timestamp offset needed");

    if (!GST_CLOCK_TIME_IS_VALID (dts)) {
      if (do_timestamp) {
        dts = running_time;
      } else if (!GST_CLOCK_TIME_IS_VALID (pts)) {
        if (GST_CLOCK_TIME_IS_VALID (basesrc->segment.start)) {
          dts = basesrc->segment.start;
        } else {
          dts = 0;
        }
      }
      GST_BUFFER_DTS (buffer) = dts;

      GST_LOG_OBJECT (basesrc, "src pad: created DTS %" GST_TIME_FORMAT,
          GST_TIME_ARGS (dts));
    }
  } else {
    /* not the first buffer, the timestamp is the diff between the clock and
     * base_time */
    if (do_timestamp && !GST_CLOCK_TIME_IS_VALID (dts)) {
      now = gst_clock_get_time (clock);

      dts = now - base_time;
      GST_BUFFER_DTS (buffer) = dts;

      GST_LOG_OBJECT (basesrc, "src pad: created DTS %" GST_TIME_FORMAT,
          GST_TIME_ARGS (dts));
    }
  }
  if (!GST_CLOCK_TIME_IS_VALID (pts)) {
    if (!GST_BUFFER_FLAG_IS_SET (buffer, GST_BUFFER_FLAG_DELTA_UNIT))
      pts = dts;

    GST_BUFFER_PTS (buffer) = dts;

    GST_LOG_OBJECT (basesrc, "src pad: created PTS %" GST_TIME_FORMAT,
        GST_TIME_ARGS (pts));
  }

  /* if we don't have a buffer timestamp, we don't sync */
  if (!GST_CLOCK_TIME_IS_VALID (start))
    goto no_sync;

  if (is_live) {
    /* for pseudo live sources, add our ts_offset to the timestamp */
    if (GST_CLOCK_TIME_IS_VALID (pts))
      GST_BUFFER_PTS (buffer) += basesrc->priv->ts_offset;
    if (GST_CLOCK_TIME_IS_VALID (dts))
      GST_BUFFER_DTS (buffer) += basesrc->priv->ts_offset;
    start += basesrc->priv->ts_offset;
  }

  GST_LOG_OBJECT (basesrc,
      "src pad: waiting for clock, base time %" GST_TIME_FORMAT
      ", stream_start %" GST_TIME_FORMAT,
      GST_TIME_ARGS (base_time), GST_TIME_ARGS (start));

  result = gst_cam_base_src_wait (basesrc, clock, start + base_time);

  gst_object_unref (clock);

  GST_LOG_OBJECT (basesrc, "src pad: clock entry done: %d", result);

  return result;

  /* special cases */
no_clock:
  {
    GST_DEBUG_OBJECT (basesrc, "src pad: we have no clock");
    GST_OBJECT_UNLOCK (basesrc);
    return GST_CLOCK_OK;
  }
no_sync:
  {
    GST_DEBUG_OBJECT (basesrc, "src pad: no sync needed");
    gst_object_unref (clock);
    return GST_CLOCK_OK;
  }
}

static GstClockReturn
gst_cam_base_src_do_video_sync(GstCamBaseSrc * basesrc, guint stream_id, GstBuffer * buffer)
{
  GstClockTime start, end;
  GstClockTime base_time;
  GstClock *clock;
  GstClockTime now = GST_CLOCK_TIME_NONE, pts, dts;
  gboolean do_timestamp, first, is_live;

  start = end = -1;

  /* get buffer timestamp */
  dts = GST_BUFFER_DTS (buffer);
  pts = GST_BUFFER_PTS (buffer);

  /* grab the lock to prepare for clocking and calculate the startup
   * latency. */
  GST_OBJECT_LOCK (basesrc);

  is_live = basesrc->is_live;
  /* check for the first buffer */
  first = ((gint64)basesrc->priv->muxPriv[stream_id].vid_latency == -1);

  if (first) {
    GST_DEBUG_OBJECT (basesrc, "video pad: no latency needed, live %d, sync %d",
      is_live, (gint64)start != -1);
    basesrc->priv->muxPriv[stream_id].vid_latency = 0;
  }

  /* get clock, if no clock, we can't sync or do timestamps */
  if ((clock = GST_ELEMENT_CLOCK (basesrc)) == NULL)
    goto no_clock;
  else
    gst_object_ref (clock);

  base_time = GST_ELEMENT_CAST (basesrc)->base_time;

  do_timestamp = basesrc->priv->do_timestamp;
  GST_OBJECT_UNLOCK (basesrc);

  /* first buffer, calculate the timestamp offset */
  if (first) {
    GstClockTime running_time;

    now = gst_clock_get_time (clock);
    running_time = now - base_time;

    GST_LOG_OBJECT (basesrc,
             "video pad: startup PTS: %" GST_TIME_FORMAT ", DTS %" GST_TIME_FORMAT
             ", running_time %" GST_TIME_FORMAT, GST_TIME_ARGS (pts),
             GST_TIME_ARGS (dts), GST_TIME_ARGS (running_time));

    basesrc->priv->muxPriv[stream_id].vid_ts_offset = 0;
    GST_LOG_OBJECT (basesrc, "video pad: no timestamp offset needed");

    if (!GST_CLOCK_TIME_IS_VALID (dts)) {
      if (do_timestamp) {
        dts = running_time;
      } else if (!GST_CLOCK_TIME_IS_VALID (pts)) {
        dts = 0;
      }
      GST_BUFFER_DTS (buffer) = dts;

      GST_LOG_OBJECT (basesrc, "video pad: created DTS %" GST_TIME_FORMAT,
        GST_TIME_ARGS (dts));
    }
  } else {
    /* not the first buffer, the timestamp is the diff between the clock and
     * base_time */
    if (do_timestamp && !GST_CLOCK_TIME_IS_VALID (dts)) {
      now = gst_clock_get_time (clock);

      dts = now - base_time;
      GST_BUFFER_DTS (buffer) = dts;

      GST_LOG_OBJECT (basesrc, "video pad: created DTS %" GST_TIME_FORMAT,
        GST_TIME_ARGS (dts));
    }
  }
  if (!GST_CLOCK_TIME_IS_VALID (pts)) {
    if (!GST_BUFFER_FLAG_IS_SET (buffer, GST_BUFFER_FLAG_DELTA_UNIT))
      pts = dts;

    GST_BUFFER_PTS (buffer) = dts;
    GST_LOG_OBJECT (basesrc, "video pad: created PTS %" GST_TIME_FORMAT,
      GST_TIME_ARGS (pts));
  }

  /* if we don't have a buffer timestamp, we don't sync */
  if (!GST_CLOCK_TIME_IS_VALID (start))
    goto no_sync;

  return GST_CLOCK_OK;

  /* special cases */
  no_clock:
  {
    GST_DEBUG_OBJECT (basesrc, "video pad: we have no clock");
    GST_OBJECT_UNLOCK (basesrc);
    return GST_CLOCK_OK;
  }
  no_sync:
  {
    GST_DEBUG_OBJECT (basesrc, "video pad: no sync needed");
    gst_object_unref (clock);
    return GST_CLOCK_OK;
  }
}

/* Called with STREAM_LOCK and LIVE_LOCK */
static gboolean
gst_cam_base_src_update_length (GstCamBaseSrc * src, guint64 offset, guint * length,
    gboolean force)
{
  guint64 size, maxsize;
  GstCamBaseSrcClass *bclass;
  gint64 stop;

  /* only operate if we are working with bytes */
  if (src->segment.format != GST_FORMAT_BYTES)
    return TRUE;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

  stop = src->segment.stop;
  /* get total file size */
  size = src->segment.duration;

  /* when not doing automatic EOS, just use the stop position. We don't use
   * the size to check for EOS */
  if (!g_atomic_int_get (&src->priv->automatic_eos))
    maxsize = stop;
  /* Otherwise, the max amount of bytes to read is the total
   * size or up to the segment.stop if present. */
  else if (stop != -1)
    maxsize = ((gint64)size != -1) ? MIN ((gint64)size, stop) : stop;
  else
    maxsize = size;


  GST_DEBUG_OBJECT (src,
      "reading offset %" G_GUINT64_FORMAT ", length %u, size %" G_GINT64_FORMAT
      ", segment.stop %" G_GINT64_FORMAT ", maxsize %" G_GINT64_FORMAT, offset,
      *length, size, stop, maxsize);

  /* check size if we have one */
  if ((gint64)maxsize != -1) {
    /* if we run past the end, check if the file became bigger and
     * retry.  Mind wrap when checking. */
    if (G_UNLIKELY (offset >= maxsize || offset + *length >= maxsize || force)) {
      /* see if length of the file changed */
      if (bclass->get_size)
        if (!bclass->get_size (src, &size))
          size = -1;

      /* make sure we don't exceed the configured segment stop
       * if it was set */
      if (stop != -1)
        maxsize = MIN ((gint64)size, stop);
      else
        maxsize = size;

      /* if we are at or past the end, EOS */
      if (G_UNLIKELY (offset >= maxsize))
        goto unexpected_length;

      /* else we can clip to the end */
      if (G_UNLIKELY (offset + *length >= maxsize))
        *length = maxsize - offset;

    }
  }

  /* keep track of current duration. segment is in bytes, we checked
   * that above. */
  GST_OBJECT_LOCK (src);
  src->segment.duration = size;
  GST_OBJECT_UNLOCK (src);

  return TRUE;

  /* ERRORS */
unexpected_length:
  {
    GST_WARNING_OBJECT (src, "processing at or past EOS");
    return FALSE;
  }
}

/* must be called with LIVE_LOCK */
static GstFlowReturn
gst_cam_base_src_get_range (GstCamBaseSrc * src, GstPad *pad, guint64 offset,
    guint length, GstBuffer ** buf)
{
  GstFlowReturn ret;
  GstCamBaseSrcClass *bclass;
  GstClockReturn status;
  GstBuffer *res_buf;
  GstBuffer *in_buf;
  gchar *padname = gst_pad_get_name(pad);

  bclass = GST_CAM_BASE_SRC_GET_CLASS (src);

again:
  if (src->is_live) {
    if (G_UNLIKELY (!src->live_running)) {
      ret = gst_cam_base_src_wait_playing (src);
      if (ret != GST_FLOW_OK)
        goto stopped;
    }
  }

  if (G_UNLIKELY (!GST_CAM_BASE_SRC_IS_STARTED (src)
          && !GST_CAM_BASE_SRC_IS_STARTING (src)))
    goto not_started;

  if (G_UNLIKELY (!bclass->create))
    goto no_function;

  if (G_UNLIKELY (!gst_cam_base_src_update_length (src, offset, &length, FALSE)))
    goto unexpected_length;

  /* track position */
  GST_OBJECT_LOCK (src);
  if (src->segment.format == GST_FORMAT_BYTES)
    src->segment.position = offset;
  GST_OBJECT_UNLOCK (src);

  /* normally we don't count buffers */
  if (G_UNLIKELY (src->num_buffers_left >= 0)) {
    if (src->num_buffers_left == 0)
      goto reached_num_buffers;
    else
      src->num_buffers_left--;
  }

  /* don't enter the create function if a pending EOS event was set. For the
   * logic of the has_pending_eos, check the event function of this class. */
  if (G_UNLIKELY (g_atomic_int_get (&src->priv->has_pending_eos))) {
    src->priv->forced_eos = TRUE;
    goto eos;
  }

  GST_DEBUG_OBJECT (src,
      "%s pad: calling create offset %" G_GUINT64_FORMAT " length %u, time %"
      G_GINT64_FORMAT, padname, offset, length, src->segment.time);

  res_buf = in_buf = *buf;

  ret = bclass->create (src, pad, offset, length, &res_buf);

  /* The create function could be unlocked because we have a pending EOS. It's
   * possible that we have a valid buffer from create that we need to
   * discard when the create function returned _OK. */
  if (G_UNLIKELY (g_atomic_int_get (&src->priv->has_pending_eos))) {
    src->priv->forced_eos = TRUE;
    goto eos;
  }

  if (G_UNLIKELY (ret != GST_FLOW_OK))
    goto not_ok;

  /* fallback in case the create function didn't fill a provided buffer */
  if (in_buf != NULL && res_buf != in_buf) {
    GstMapInfo info;
    gsize copied_size;

    GST_CAT_DEBUG_OBJECT (NULL, src, "%s pad: create function didn't "
        "fill the provided buffer, copying", padname);

    if (!gst_buffer_map (in_buf, &info, GST_MAP_WRITE))
      goto map_failed;

    copied_size = gst_buffer_extract (res_buf, 0, info.data, info.size);
    gst_buffer_unmap (in_buf, &info);
    gst_buffer_set_size (in_buf, copied_size);

    gst_buffer_copy_into (in_buf, res_buf, GST_BUFFER_COPY_ALL, 0, -1);

    gst_buffer_unref (res_buf);
    res_buf = in_buf;
  }

  /* no timestamp set and we are at offset 0, we can timestamp with 0 */
  if (offset == 0 && src->segment.time == 0
      && (gint64)GST_BUFFER_DTS (res_buf) == -1 && !src->is_live) {
    GST_DEBUG_OBJECT (src, "%s pad: setting first timestamp to 0", padname);
    res_buf = gst_buffer_make_writable (res_buf);
    GST_BUFFER_DTS (res_buf) = 0;
  }

  /* now sync before pushing the buffer */
  status = gst_cam_base_src_do_sync (src, res_buf);

  /* waiting for the clock could have made us flushing */
  if (G_UNLIKELY (src->priv->flushing))
    goto flushing;

  switch (status) {
    case GST_CLOCK_EARLY:
      /* the buffer is too late. We currently don't drop the buffer. */
      GST_DEBUG_OBJECT (src, "buffer too late!, returning anyway");
      break;
    case GST_CLOCK_OK:
      /* buffer synchronised properly */
      GST_DEBUG_OBJECT (src, "buffer ok");
      break;
    case GST_CLOCK_UNSCHEDULED:
      if (!src->live_running) {
        /* We return FLUSHING when we are not running to stop the dataflow also
         * get rid of the produced buffer. */
        GST_DEBUG_OBJECT (src,
            "clock was unscheduled (%d), returning FLUSHING", status);
        ret = GST_FLOW_FLUSHING;
      } else {
        /* If we are running when this happens, we quickly switched between
         * pause and playing. We try to produce a new buffer */
        GST_DEBUG_OBJECT (src,
            "clock was unscheduled (%d), but we are running", status);
        goto again;
      }
      break;
    default:
      /* all other result values are unexpected and errors */
      GST_ELEMENT_ERROR (src, CORE, CLOCK,
          ("Internal clock error."),
          ("clock returned unexpected return value %d", status));
      ret = GST_FLOW_ERROR;
      break;
  }
  if (G_LIKELY (ret == GST_FLOW_OK))
    *buf = res_buf;

  g_free (padname);

  return ret;

  /* ERROR */
stopped:
  {
    GST_DEBUG_OBJECT (src, "%s pad: wait_playing returned %d (%s)", padname,
        ret, gst_flow_get_name (ret));
    g_free (padname);
    return ret;
  }
not_ok:
  {
    GST_DEBUG_OBJECT (src, "%s pad: create returned %d (%s)", padname, ret,
        gst_flow_get_name (ret));
    g_free (padname);
    return ret;
  }
map_failed:
  {
    GST_ELEMENT_ERROR (src, RESOURCE, BUSY,
        ("%s pad: failed to map buffer.", padname),
        ("failed to map result buffer in WRITE mode"));
    g_free (padname);
    return GST_FLOW_ERROR;
  }
not_started:
  {
    GST_DEBUG_OBJECT (src, "%s pad: getrange but not started", padname);
    g_free (padname);
    return GST_FLOW_FLUSHING;
  }
no_function:
  {
    GST_DEBUG_OBJECT (src, "%s pad: no create function", padname);
    g_free (padname);
    return GST_FLOW_NOT_SUPPORTED;
  }
unexpected_length:
  {
    GST_DEBUG_OBJECT (src, "%s pad: unexpected length %u (offset=%" G_GUINT64_FORMAT
        ", size=%" G_GINT64_FORMAT ")", padname, length, offset, src->segment.duration);
    g_free (padname);
    return GST_FLOW_EOS;
  }
reached_num_buffers:
  {
    GST_DEBUG_OBJECT (src, "%s pad: sent all buffers", padname);
    g_free (padname);
    return GST_FLOW_EOS;
  }
flushing:
  {
    GST_DEBUG_OBJECT (src, "%s pad: we are flushing", padname);
    g_free (padname);
    return GST_FLOW_FLUSHING;
  }
eos:
  {
    GST_DEBUG_OBJECT (src, "%s pad: we are EOS", padname);
    g_free (padname);
    return GST_FLOW_EOS;
  }
}

static GstFlowReturn
gst_cam_base_src_video_get_range (GstCamBaseSrc * src, GstPad *pad, guint64 offset,
    guint length, GstBuffer ** buf)
{
  GstFlowReturn ret;
  GstCamBaseSrcClass *bclass;
  GstClockReturn status;
  GstBuffer *res_buf, *in_buf;
  gchar *padname = gst_pad_get_name(pad);
  guint stream_id = CameraSrcUtils::get_stream_id_by_pad(src->priv->request_stream_map, pad);

  bclass = GST_CAM_BASE_SRC_GET_CLASS(src);

  if (G_UNLIKELY (!GST_CAM_BASE_SRC_IS_STARTED (src)
          && !GST_CAM_BASE_SRC_IS_STARTING (src)))
    goto not_started;

  if (G_UNLIKELY (src->mux[stream_id].num_video_buffers_left >= 0)) {
    if (src->mux[stream_id].num_video_buffers_left == 0)
      goto reached_num_buffers;
    else
      src->mux[stream_id].num_video_buffers_left--;
  }

  if (G_UNLIKELY (g_atomic_int_get (&src->priv->has_pending_eos))) {
    src->priv->forced_eos = TRUE;
    goto eos;
  }

  GST_DEBUG_OBJECT (src,
      "%s pad: calling create offset %" G_GUINT64_FORMAT " length %u, time %"
      G_GINT64_FORMAT, padname, offset, length, src->mux[stream_id].vid_segment.time);

  res_buf = in_buf = *buf;

  ret = bclass->create (src, pad, offset, length, &res_buf);
  if (G_UNLIKELY (ret != GST_FLOW_OK))
    goto not_ok;

  if (G_UNLIKELY (g_atomic_int_get (&src->priv->has_pending_eos))) {
    src->priv->forced_eos = TRUE;
    goto eos;
  }

  if (offset == 0 && src->mux[stream_id].vid_segment.time == 0
      && (gint64)GST_BUFFER_DTS (res_buf) == -1 && !src->is_live) {
    GST_DEBUG_OBJECT (src, "%s pad: setting first timestamp to 0", padname);
    res_buf = gst_buffer_make_writable (res_buf);
    GST_BUFFER_DTS (res_buf) = 0;
  }

  status = gst_cam_base_src_do_video_sync(src, stream_id, res_buf);

  if (G_UNLIKELY (src->priv->flushing))
    goto flushing;

  switch (status) {
    case GST_CLOCK_UNSCHEDULED:
      if (!src->live_running) {
        ret = GST_FLOW_FLUSHING;
      }
      break;
    case GST_CLOCK_EARLY:
    case GST_CLOCK_OK:
      break;
    default:
      ret = GST_FLOW_ERROR;
      break;
  }
  if (G_LIKELY(ret == GST_FLOW_OK))
    *buf = res_buf;

  g_free (padname);

  return ret;

not_ok:
  {
    GST_DEBUG_OBJECT (src, "%s pad: create returned %d (%s)", padname,
        ret, gst_flow_get_name (ret));
    g_free (padname);
    return ret;
  }
not_started:
  {
    GST_DEBUG_OBJECT (src, "%s pad: getrange but not started", padname);
    g_free (padname);
    return GST_FLOW_FLUSHING;
  }
reached_num_buffers:
  {
    GST_DEBUG_OBJECT (src, "%s pad: sent all buffers", padname);
    g_free (padname);
    return GST_FLOW_EOS;
  }
flushing:
  {
    GST_DEBUG_OBJECT (src, "%s pad: we are flushing", padname);
    g_free (padname);
    return GST_FLOW_FLUSHING;
  }
eos:
  {
    GST_DEBUG_OBJECT (src, "%s pad: we are EOS", padname);
    g_free (padname);
    return GST_FLOW_EOS;
  }
}

static GstFlowReturn
gst_cam_base_src_getrange (GstPad * pad, GstObject * parent, guint64 offset,
    guint length, GstBuffer ** buf)
{
  GstCamBaseSrc *src;
  GstFlowReturn res = GST_FLOW_OK;

  src = GST_CAM_BASE_SRC_CAST (parent);

  GST_LIVE_LOCK (src);
  if (G_UNLIKELY (src->priv->flushing))
    goto flushing;

  if (pad == src->srcpad)
    res = gst_cam_base_src_get_range (src, pad, offset, length, buf);
  else
    res = gst_cam_base_src_video_get_range (src, pad, offset, length, buf);

done:
  GST_LIVE_UNLOCK (src);

  return res;

  /* ERRORS */
flushing:
  {
    GST_DEBUG_OBJECT (src, "we are flushing");
    res = GST_FLOW_FLUSHING;
    goto done;
  }
}

static gboolean
gst_cam_base_src_is_random_access (GstCamBaseSrc * src)
{
  /* we need to start the basesrc to check random access */
  if (!GST_CAM_BASE_SRC_IS_STARTED (src)) {
    GST_LOG_OBJECT (src, "doing start/stop to check get_range support");
    if (G_LIKELY (gst_cam_base_src_start (src))) {
      if (gst_cam_base_src_start_wait (src) != GST_FLOW_OK)
        goto start_failed;
      gst_cam_base_src_stop (src);
    }
  }

  return src->random_access;

  /* ERRORS */
start_failed:
  {
    GST_DEBUG_OBJECT (src, "failed to start");
    return FALSE;
  }
}

static void
gst_cam_base_src_loop (GstPad * pad)
{
  GstCamBaseSrc *src;
  GstBuffer *buf = NULL;
  GstFlowReturn ret;
  gint64 position;
  gboolean eos;
  guint blocksize;
  GList *pending_events = NULL, *tmp;
  gchar *padname = gst_pad_get_name(pad);

  eos = FALSE;

  src = GST_CAM_BASE_SRC (GST_OBJECT_PARENT (pad));

  /* Just leave immediately if we're flushing */
  GST_LIVE_LOCK (src);
  if (G_UNLIKELY (src->priv->flushing || GST_PAD_IS_FLUSHING (pad)))
    goto flushing;
  GST_LIVE_UNLOCK (src);

  gst_cam_base_src_send_stream_start (src);

  /* The stream-start event could've caused something to flush us */
  GST_LIVE_LOCK (src);
  if (G_UNLIKELY (src->priv->flushing || GST_PAD_IS_FLUSHING (pad)))
    goto flushing;
  GST_LIVE_UNLOCK (src);

  /* check if we need to renegotiate */
  if (gst_pad_check_reconfigure (pad)) {
    if (!gst_cam_base_src_negotiate (src, pad)) {
      gst_pad_mark_reconfigure (pad);
      if (GST_PAD_IS_FLUSHING (pad)) {
        GST_LIVE_LOCK (src);
        goto flushing;
      } else {
        goto negotiate_failed;
      }
    }
  }

  GST_LIVE_LOCK (src);

  if (G_UNLIKELY (src->priv->flushing || GST_PAD_IS_FLUSHING (pad)))
    goto flushing;

  blocksize = src->blocksize;

  /* if we operate in bytes, we can calculate an offset */
  if (src->segment.format == GST_FORMAT_BYTES) {
    position = src->segment.position;
    /* for negative rates, start with subtracting the blocksize */
    if (src->segment.rate < 0.0) {
      /* we cannot go below segment.start */
      if (position > (gint64)(src->segment.start + blocksize))
        position -= blocksize;
      else {
        /* last block, remainder up to segment.start */
        blocksize = position - src->segment.start;
        position = src->segment.start;
      }
    }
  } else
    position = -1;

  GST_LOG_OBJECT (src, "%s pad: next_ts %" GST_TIME_FORMAT " size %u",
      padname, GST_TIME_ARGS (position), blocksize);

  ret = gst_cam_base_src_get_range (src, pad, position, blocksize, &buf);
  if (G_UNLIKELY (ret != GST_FLOW_OK)) {
    GST_INFO_OBJECT (src, "%s pad: pausing after gst_cam_base_src_get_range() = %s",
        padname, gst_flow_get_name (ret));
    GST_LIVE_UNLOCK (src);
    goto pause;
  }
  /* this should not happen */
  if (G_UNLIKELY (buf == NULL))
    goto null_buffer;

  /* push events to close/start our segment before we push the buffer. */
  if (G_UNLIKELY (src->priv->segment_pending)) {
    GstEvent *seg_event = gst_event_new_segment (&src->segment);

    gst_event_set_seqnum (seg_event, src->priv->segment_seqnum);
    src->priv->segment_seqnum = gst_util_seqnum_next ();
    gst_pad_push_event (pad, seg_event);
    src->priv->segment_pending = FALSE;
  }

  if (g_atomic_int_get (&src->priv->have_events)) {
    GST_OBJECT_LOCK (src);
    /* take the events */
    pending_events = src->priv->pending_events;
    src->priv->pending_events = NULL;
    g_atomic_int_set (&src->priv->have_events, FALSE);
    GST_OBJECT_UNLOCK (src);
  }

  /* Push out pending events if any */
  if (G_UNLIKELY (pending_events != NULL)) {
    for (tmp = pending_events; tmp; tmp = g_list_next (tmp)) {
      GstEvent *ev = (GstEvent *) tmp->data;
      gst_pad_push_event (pad, ev);
    }
    g_list_free (pending_events);
  }

  /* figure out the new position */
  switch (src->segment.format) {
    case GST_FORMAT_BYTES:
    {
      guint bufsize = gst_buffer_get_size (buf);

      /* we subtracted above for negative rates */
      if (src->segment.rate >= 0.0)
        position += bufsize;
      break;
    }
    case GST_FORMAT_TIME:
    {
      GstClockTime start, duration;

      start = GST_BUFFER_TIMESTAMP (buf);
      duration = GST_BUFFER_DURATION (buf);

      if (GST_CLOCK_TIME_IS_VALID (start))
        position = start;
      else
        position = src->segment.position;

      if (GST_CLOCK_TIME_IS_VALID (duration)) {
        if (src->segment.rate >= 0.0)
          position += duration;
        else if (position > (gint64)duration)
          position -= duration;
        else
          position = 0;
      }
      break;
    }
    case GST_FORMAT_DEFAULT:
      if (src->segment.rate >= 0.0)
        position = GST_BUFFER_OFFSET_END (buf);
      else
        position = GST_BUFFER_OFFSET (buf);
      break;
    default:
      position = -1;
      break;
  }
  if (position != -1) {
    if (src->segment.rate >= 0.0) {
      /* positive rate, check if we reached the stop */
      if ((gint64)src->segment.stop != -1) {
        if (position >= (gint64)src->segment.stop) {
          eos = TRUE;
          position = src->segment.stop;
        }
      }
    } else {
      /* negative rate, check if we reached the start. start is always set to
       * something different from -1 */
      if (position <= (gint64)src->segment.start) {
        eos = TRUE;
        position = src->segment.start;
      }
      /* when going reverse, all buffers are DISCONT */
      src->priv->discont = TRUE;
    }
    GST_OBJECT_LOCK (src);
    src->segment.position = position;
    GST_OBJECT_UNLOCK (src);
  }

  if (G_UNLIKELY (src->priv->discont)) {
    GST_INFO_OBJECT (src, "marking pending DISCONT");
    buf = gst_buffer_make_writable (buf);
    GST_BUFFER_FLAG_SET (buf, GST_BUFFER_FLAG_DISCONT);
    src->priv->discont = FALSE;
  }
  GST_LIVE_UNLOCK (src);

  ret = gst_pad_push (pad, buf);
  if (G_UNLIKELY (ret != GST_FLOW_OK)) {
    if (ret == GST_FLOW_NOT_NEGOTIATED) {
      goto not_negotiated;
    }
    GST_INFO_OBJECT (src, "%s pad: pausing after gst_pad_push() = %s",
        padname, gst_flow_get_name (ret));
    goto pause;
  }

  /* Segment pending means that a new segment was configured
   * during this loop run */
  if (G_UNLIKELY (eos && !src->priv->segment_pending)) {
    GST_INFO_OBJECT (src, "%s pad: pausing after end of segment", padname);
    ret = GST_FLOW_EOS;
    goto pause;
  }

done:
  g_free (padname);
  return;

  /* special cases */
not_negotiated:
  {
    if (gst_pad_needs_reconfigure (pad)) {
      GST_DEBUG_OBJECT (src, "%s pad: Retrying to renegotiate", padname);
      g_free (padname);
      return;
    }
    /* fallthrough when push returns NOT_NEGOTIATED and we don't have
     * a pending negotiation request on our srcpad */
  }
negotiate_failed:
  {
    GST_DEBUG_OBJECT (src, "%s pad: not negotiated", padname);
    ret = GST_FLOW_NOT_NEGOTIATED;
    goto pause;
  }
flushing:
  {
    GST_DEBUG_OBJECT (src, "%s pad: we are flushing", padname);
    GST_LIVE_UNLOCK (src);
    ret = GST_FLOW_FLUSHING;
    goto pause;
  }
pause:
  {
    const gchar *reason = gst_flow_get_name (ret);
    GstEvent *event;

    GST_DEBUG_OBJECT (src, "%s pad: pausing task, reason %s", padname, reason);
    src->running = FALSE;
    gst_pad_pause_task (pad);
    if (ret == GST_FLOW_EOS) {
      gboolean flag_segment;
      GstFormat format;
      gint64 position;

      flag_segment = (src->segment.flags & GST_SEGMENT_FLAG_SEGMENT) != 0;
      format = src->segment.format;
      position = src->segment.position;

      /* perform EOS logic */
      if (src->priv->forced_eos) {
        g_assert (g_atomic_int_get (&src->priv->has_pending_eos));
        GST_OBJECT_LOCK (src);
        event = src->priv->pending_eos;
        src->priv->pending_eos = NULL;
        GST_OBJECT_UNLOCK (src);
      } else if (flag_segment) {
        GstMessage *message;

        message = gst_message_new_segment_done (GST_OBJECT_CAST (src),
            format, position);
        gst_message_set_seqnum (message, src->priv->seqnum);
        gst_element_post_message (GST_ELEMENT_CAST (src), message);
        event = gst_event_new_segment_done (format, position);
        gst_event_set_seqnum (event, src->priv->seqnum);
      } else {
        event = gst_event_new_eos ();
        gst_event_set_seqnum (event, src->priv->seqnum);
        src->priv->receive_eos = TRUE;
      }

      gst_pad_push_event (pad, event);
      src->priv->forced_eos = FALSE;
    } else if (ret == GST_FLOW_NOT_LINKED || ret <= GST_FLOW_EOS) {
      event = gst_event_new_eos ();
      gst_event_set_seqnum (event, src->priv->seqnum);
      /* for fatal errors we post an error message, post the error
       * first so the app knows about the error first.
       * Also don't do this for FLUSHING because it happens
       * due to flushing and posting an error message because of
       * that is the wrong thing to do, e.g. when we're doing
       * a flushing seek. */
      GST_ELEMENT_ERROR (src, STREAM, FAILED,
          ("%s pad: Internal data flow error.", padname),("streaming task paused, reason %s (%d)",
          reason, ret));
      gst_pad_push_event (pad, event);
    }
    goto done;
  }
null_buffer:
  {
    GST_ELEMENT_ERROR (src, STREAM, FAILED,
        ("%s pad: Internal data flow error in.", padname), ("element returned NULL buffer"));
    GST_LIVE_UNLOCK (src);
    goto done;
  }
}

static void gst_cam_base_src_video_loop (GstPad * pad)
{
  GstBuffer *buf = NULL;
  gint64 position = 0;
  GstFlowReturn ret = GST_FLOW_OK;
  guint blocksize;
  gchar *padname = gst_pad_get_name(pad);
  GstCamBaseSrc *src = GST_CAM_BASE_SRC(GST_OBJECT_PARENT(pad));
  guint stream_id = CameraSrcUtils::get_stream_id_by_pad(src->priv->request_stream_map, pad);

  /* Just leave immediately if we're flushing */
  GST_VID_LIVE_LOCK (src, stream_id);
  if (G_UNLIKELY (src->priv->flushing || GST_PAD_IS_FLUSHING (pad)))
    goto flushing;
  GST_VID_LIVE_UNLOCK (src, stream_id);

  gst_cam_base_src_send_video_stream_start(src, pad);

  /* The stream-start event could've caused something to flush us */
  GST_VID_LIVE_LOCK (src, stream_id);
  if (G_UNLIKELY (src->priv->flushing || GST_PAD_IS_FLUSHING (pad)))
    goto flushing;
  GST_VID_LIVE_UNLOCK (src, stream_id);

  /* check if we need to renegotiate */
  if (gst_pad_check_reconfigure (pad)) {
    if (!gst_cam_base_src_negotiate (src, pad)) {
      gst_pad_mark_reconfigure (pad);
      if (GST_PAD_IS_FLUSHING(pad)) {
        GST_VID_LIVE_LOCK(src, stream_id);
        goto flushing;
      } else {
        goto negotiate_failed;
      }
    }
  }

  GST_VID_LIVE_LOCK(src, stream_id);
  blocksize = src->blocksize;

  /* we only consider the scenario of GST_FORMAT_TIME */
  if (src->mux[stream_id].vid_segment.format == GST_FORMAT_TIME)
    position = -1;

  GST_LOG_OBJECT (src, "%s pad: next_ts %" GST_TIME_FORMAT " size %u",
      padname, GST_TIME_ARGS (position), blocksize);

  ret = gst_cam_base_src_video_get_range (src, pad, position, blocksize, &buf);
  if (G_UNLIKELY (ret != GST_FLOW_OK)) {
    GST_INFO_OBJECT (src, "%s pad: pausing after gst_cam_base_src_video_get_range() = %s",
        padname, gst_flow_get_name (ret));
    GST_VID_LIVE_UNLOCK (src, stream_id);
    goto pause;
  }

  if (G_UNLIKELY (buf == NULL))
    goto null_buffer;

  if (src->mux[stream_id].vid_segment.format == GST_FORMAT_TIME) {
    GstClockTime start, duration;
    start = GST_BUFFER_TIMESTAMP(buf);
    duration = GST_BUFFER_DURATION(buf);
    if (GST_CLOCK_TIME_IS_VALID (start))
      position = start;

    if (GST_CLOCK_TIME_IS_VALID (duration)) {
      if (src->mux[stream_id].vid_segment.rate >= 0.0)
        position += duration;
      else if (position > (gint64)duration)
        position -= duration;
      else
        position = 0;
    }
  }
  GST_VID_LIVE_UNLOCK(src, stream_id);

  ret = gst_pad_push(pad, buf);
  if (G_UNLIKELY (ret != GST_FLOW_OK)) {
    if (ret == GST_FLOW_NOT_NEGOTIATED) {
      goto not_negotiated;
    }
    GST_INFO_OBJECT (src, "%s pad: is pausing after gst_pad_push() = %s",
        padname, gst_flow_get_name (ret));
    goto pause;
  }

  if (src->priv->receive_eos) {
    ret = GST_FLOW_EOS;
    goto pause;
  }

done:
  g_free (padname);
  return;

flushing:
  {
    GST_DEBUG_OBJECT(src, "%s pad: is flushing", padname);
    GST_VID_LIVE_UNLOCK(src, stream_id);
    ret = GST_FLOW_FLUSHING;
    goto pause;
  }
not_negotiated:
  {
    if (gst_pad_needs_reconfigure(pad)) {
      GST_DEBUG_OBJECT(src, "%s pad: Retry to renegotiate", padname);
      goto done;
    }
  }
negotiate_failed:
  {
    GST_DEBUG_OBJECT(src, "%s pad: not negotiated", padname);
    goto pause;
  }
pause:
  {
    const gchar *reason = gst_flow_get_name(ret);
    GstEvent *event;

    GST_DEBUG_OBJECT (src, "%s pad: pausing task, reason %s", padname, reason);
    src->running = FALSE;
    gst_pad_pause_task (pad);
    if (ret == GST_FLOW_EOS) {
      gboolean flag_segment;
      GstFormat format;
      gint64 position;

      flag_segment = (src->mux[stream_id].vid_segment.flags & GST_SEGMENT_FLAG_SEGMENT) != 0;
      format = src->mux[stream_id].vid_segment.format;
      position = src->mux[stream_id].vid_segment.position;

      /* perform EOS logic */
      if (src->priv->forced_eos) {
        g_assert (g_atomic_int_get (&src->priv->has_pending_eos));
        GST_OBJECT_LOCK (src);
        event = src->priv->pending_eos;
        src->priv->pending_eos = NULL;
        GST_OBJECT_UNLOCK (src);
      } else if (flag_segment) {
        GstMessage *message;

        message = gst_message_new_segment_done (GST_OBJECT_CAST (src),
            format, position);
        gst_message_set_seqnum (message, src->priv->seqnum);
        gst_element_post_message (GST_ELEMENT_CAST (src), message);
        event = gst_event_new_segment_done (format, position);
        gst_event_set_seqnum (event, src->priv->seqnum);
      } else {
        event = gst_event_new_eos();
        gst_event_set_seqnum (event, src->priv->seqnum);
        src->priv->receive_eos = TRUE;
      }
      gst_pad_push_event (pad, event);
      src->priv->forced_eos = FALSE;
    } else if (ret == GST_FLOW_NOT_LINKED || ret <= GST_FLOW_EOS) {
      event = gst_event_new_eos();
      gst_event_set_seqnum(event, src->priv->seqnum);
      GST_ELEMENT_ERROR (src, STREAM, FAILED,
        ("%s pad: Internal data flow error.", padname),
        ("streaming task paused, reason %s (%d)",
        reason, ret));
        gst_pad_push_event (pad, event);
    }

    goto done;
  }
null_buffer:
  {
    GST_ELEMENT_ERROR(src, STREAM, FAILED,
      ("%s pad: Internal data flow error.", padname),
      ("element return NULL buffer"));
    GST_VID_LIVE_UNLOCK(src, stream_id);

    goto done;
  }
}

static gboolean
gst_cam_base_src_set_allocation (GstCamBaseSrc * basesrc, GstPad *pad,
    GstBufferPool * pool, GstAllocator * allocator, GstAllocationParams * params)
{
  GstAllocator *oldalloc = NULL;
  GstBufferPool *oldpool = NULL;
  GstCamBaseSrcPrivate *priv = basesrc->priv;

  if (pool) {
    GST_DEBUG_OBJECT (basesrc, "activate pool %p", pool);
    if (!gst_buffer_pool_set_active (pool, TRUE))
      goto activate_failed;
  }

  GST_OBJECT_LOCK (basesrc);

  if (pad == basesrc->srcpad) {
    oldalloc = priv->allocator;
    oldpool = priv->pool;
    priv->allocator = allocator;
    priv->pool = pool;

    if (priv->pool)
      gst_object_ref (priv->pool);
    if (priv->allocator)
      gst_object_ref(priv->allocator);
    if (params)
       priv->params = *params;
    else
       gst_allocation_params_init(&priv->params);
  } else {
    guint stream_id = CameraSrcUtils::get_stream_id_by_pad(priv->request_stream_map, pad);
    oldalloc = priv->muxPriv[stream_id].vid_allocator;
    oldpool = priv->muxPriv[stream_id].vid_pool;
    priv->muxPriv[stream_id].vid_allocator = allocator;
    priv->muxPriv[stream_id].vid_pool = pool;

    if (priv->muxPriv[stream_id].vid_pool)
      gst_object_ref(priv->muxPriv[stream_id].vid_pool);
    if (priv->muxPriv[stream_id].vid_allocator)
      gst_object_ref(priv->muxPriv[stream_id].vid_allocator);
    if (params)
      priv->muxPriv[stream_id].vid_params = *params;
    else
      gst_allocation_params_init(&priv->muxPriv[stream_id].vid_params);
  }

  GST_OBJECT_UNLOCK (basesrc);

  if (oldpool) {
    /* only deactivate if the pool is not the one we're using */
    if (oldpool != pool && !basesrc->live_running) {
      GST_DEBUG_OBJECT (basesrc, "deactivate old pool %p", oldpool);
      gst_buffer_pool_set_active (oldpool, FALSE);
    }
    gst_object_unref (oldpool);
  }

  if (oldalloc) {
    gst_object_unref (oldalloc);
  }
  return TRUE;

  /* ERRORS */
activate_failed:
  {
    GST_ERROR_OBJECT (basesrc, "failed to activate bufferpool.");
    return FALSE;
  }
}

static gboolean
gst_cam_base_src_activate_pool (GstCamBaseSrc * basesrc, gboolean active)
{
  GstCamBaseSrcPrivate *priv = basesrc->priv;
  GstBufferPool *pool;
  gboolean res = TRUE;

  GST_OBJECT_LOCK (basesrc);
  if ((pool = priv->pool))
    pool = (GstBufferPool*)gst_object_ref (pool);
  GST_OBJECT_UNLOCK (basesrc);

  if (pool) {
    res = gst_buffer_pool_set_active (pool, active);
    gst_object_unref (pool);
  }
  return res;
}

static gboolean
gst_cam_base_src_activate_video_pool (GstCamBaseSrc * basesrc, gboolean active)
{
  GstCamBaseSrcPrivate *priv = basesrc->priv;
  gboolean res = TRUE;
  if (priv->request_stream_map.empty())
    return res;

  for (auto & it : priv->request_stream_map) {
    GstBufferPool *pool;

    GST_OBJECT_LOCK (basesrc);
    if ((pool = priv->muxPriv[it.second].vid_pool))
      pool = (GstBufferPool*)gst_object_ref (pool);
    GST_OBJECT_UNLOCK (basesrc);

    if (pool) {
      res &= gst_buffer_pool_set_active (pool, active);
      if (!res)
        GST_ERROR_OBJECT(basesrc, "Failed to activate pool for request pad %d", it.second);
      gst_object_unref (pool);
    }
  }

  return res;
}

static gboolean
gst_cam_base_src_default_decide_allocation (GstCamBaseSrc * basesrc,
    GstQuery * query, GstPad * pad)
{
  GstCaps *outcaps;
  GstBufferPool *pool;
  guint size, min, max;
  GstAllocator *allocator;
  GstAllocationParams params;
  GstStructure *config;
  gboolean update_allocator;

  gst_query_parse_allocation (query, &outcaps, NULL);

  /* we got configuration from our peer or the decide_allocation method,
   * parse them */
  if (gst_query_get_n_allocation_params (query) > 0) {
    /* try the allocator */
    gst_query_parse_nth_allocation_param (query, 0, &allocator, &params);
    update_allocator = TRUE;
  } else {
    allocator = NULL;
    gst_allocation_params_init (&params);
    update_allocator = FALSE;
  }

  if (gst_query_get_n_allocation_pools (query) > 0) {
    gst_query_parse_nth_allocation_pool (query, 0, &pool, &size, &min, &max);

    if (pool == NULL) {
      /* no pool, we can make our own */
      GST_DEBUG_OBJECT (basesrc, "no pool, making new pool");
      pool = gst_buffer_pool_new ();
    }
  } else {
    pool = NULL;
    size = min = max = 0;
  }

  /* now configure */
  if (pool) {
    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_set_params (config, outcaps, size, min, max);
    gst_buffer_pool_config_set_allocator (config, allocator, &params);

    /* buffer pool may have to do some changes */
    if (!gst_buffer_pool_set_config (pool, config)) {
      config = gst_buffer_pool_get_config (pool);

      /* If change are not acceptable, fallback to generic pool */
      if (!gst_buffer_pool_config_validate_params (config, outcaps, size, min,
              max)) {
        GST_DEBUG_OBJECT (basesrc, "unsupported pool, making new pool");

        gst_object_unref (pool);
        pool = gst_buffer_pool_new ();
        gst_buffer_pool_config_set_params (config, outcaps, size, min, max);
        gst_buffer_pool_config_set_allocator (config, allocator, &params);
      }

      if (!gst_buffer_pool_set_config (pool, config))
        goto config_failed;
    }
  }

  if (update_allocator)
    gst_query_set_nth_allocation_param (query, 0, allocator, &params);
  else
    gst_query_add_allocation_param (query, allocator, &params);
  if (allocator)
    gst_object_unref (allocator);

  if (pool) {
    gst_query_set_nth_allocation_pool (query, 0, pool, size, min, max);
    gst_object_unref (pool);
  }

  return TRUE;

config_failed:
  GST_ELEMENT_ERROR (basesrc, RESOURCE, SETTINGS,
      ("Failed to configure the buffer pool"),
      ("Configuration is most likely invalid, please report this issue."));
  gst_object_unref (pool);
  return FALSE;
}

static gboolean
gst_cam_base_src_prepare_allocation (GstCamBaseSrc * basesrc, GstCaps * caps, GstPad * pad)
{
  GstCamBaseSrcClass *bclass;
  gboolean result = TRUE;
  GstQuery *query;
  GstBufferPool *pool = NULL;
  GstAllocator *allocator = NULL;
  GstAllocationParams params;
  gchar *padname = gst_pad_get_name(pad);

  bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);

  /* make query and let peer pad answer, we don't really care if it worked or
   * not, if it failed, the allocation query would contain defaults and the
   * subclass would then set better values if needed */
  query = gst_query_new_allocation (caps, TRUE);
  if (!gst_pad_peer_query (pad, query)) {
    /* not a problem, just debug a little */
    GST_DEBUG_OBJECT (basesrc, "%s pad: peer ALLOCATION query failed",
      padname);
  }

  g_assert (bclass->decide_allocation != NULL);
  result = bclass->decide_allocation (basesrc, query, pad);

  GST_DEBUG_OBJECT (basesrc, "%s pad: ALLOCATION (%d) params: %" GST_PTR_FORMAT,
    padname, result, query);

  if (!result)
    goto no_decide_allocation;

  /* we got configuration from our peer or the decide_allocation method,
   * parse them */
  if (gst_query_get_n_allocation_params (query) > 0) {
    gst_query_parse_nth_allocation_param (query, 0, &allocator, &params);
  } else {
    allocator = NULL;
    gst_allocation_params_init (&params);
  }

  if (gst_query_get_n_allocation_pools (query) > 0)
    gst_query_parse_nth_allocation_pool (query, 0, &pool, NULL, NULL, NULL);

  result = gst_cam_base_src_set_allocation (basesrc, pad, pool, allocator, &params);

  if (allocator)
    gst_object_unref (allocator);
  if (pool)
    gst_object_unref (pool);

  gst_query_unref (query);

  g_free (padname);

  return result;

  /* Errors */
no_decide_allocation:
  {
    GST_WARNING_OBJECT (basesrc, "Subclass failed to decide allocation");
    gst_query_unref (query);

    return result;
  }
}

/* default negotiation code.
 *
 * Take intersection between src and sink pads, take first
 * caps and fixate.
 */
static gboolean
gst_cam_base_src_default_negotiate (GstCamBaseSrc * basesrc, GstPad *pad)
{
  GstCaps *thiscaps;
  GstCaps *caps = NULL;
  GstCaps *peercaps = NULL;
  gboolean result = FALSE;

  /* first see what is possible on our source pad */
  thiscaps = gst_pad_query_caps (pad, NULL);
  GST_DEBUG_OBJECT (basesrc, "caps of src: %" GST_PTR_FORMAT, thiscaps);
  /* nothing or anything is allowed, we're done */
  if (thiscaps == NULL || gst_caps_is_any (thiscaps))
    goto no_nego_needed;

  if (G_UNLIKELY (gst_caps_is_empty (thiscaps)))
    goto no_caps;

  /* get the peer caps */
  peercaps = gst_pad_peer_query_caps (pad, thiscaps);
  GST_DEBUG_OBJECT (basesrc, "caps of peer: %" GST_PTR_FORMAT, peercaps);
  if (peercaps) {
    /* The result is already a subset of our caps */
    caps = peercaps;
    gst_caps_unref (thiscaps);
  } else {
    /* no peer, work with our own caps then */
    caps = thiscaps;
  }
  if (caps && !gst_caps_is_empty (caps)) {
    /* now fixate */
    GST_DEBUG_OBJECT (basesrc, "have caps: %" GST_PTR_FORMAT, caps);
    if (gst_caps_is_any (caps)) {
      GST_DEBUG_OBJECT (basesrc, "any caps, we stop");
      /* hmm, still anything, so element can do anything and
       * nego is not needed */
      result = TRUE;
    } else {
      caps = gst_cam_base_src_fixate (basesrc, caps);
      GST_DEBUG_OBJECT (basesrc, "fixated to: %" GST_PTR_FORMAT, caps);
      if (gst_caps_is_fixed (caps)) {
        /* yay, fixed caps, use those then, it's possible that the subclass does
         * not accept this caps after all and we have to fail. */
        result = gst_cam_base_src_set_caps (basesrc, pad, caps);
      }
    }
    gst_caps_unref (caps);
  } else {
    if (caps)
      gst_caps_unref (caps);
    GST_DEBUG_OBJECT (basesrc, "no common caps");
  }
  return result;

no_nego_needed:
  {
    GST_DEBUG_OBJECT (basesrc, "no negotiation needed");
    if (thiscaps)
      gst_caps_unref (thiscaps);
    return TRUE;
  }
no_caps:
  {
    GST_ELEMENT_ERROR (basesrc, STREAM, FORMAT,
        ("No supported formats found"),
        ("This element did not produce valid caps"));
    if (thiscaps)
      gst_caps_unref (thiscaps);
    return TRUE;
  }
}

static gboolean
gst_cam_base_src_negotiate (GstCamBaseSrc * basesrc, GstPad *pad)
{
  GstCamBaseSrcClass *bclass;
  gboolean result;
  gchar *padname = gst_pad_get_name(pad);

  bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);

  GST_DEBUG_OBJECT (basesrc, "%s pad: starting negotiation", padname);

  /* call default negotiate function to fixate and set caps */
  if (G_LIKELY (bclass->negotiate))
    result = bclass->negotiate (basesrc, pad);
  else
    result = TRUE;

  if (G_LIKELY (result)) {
    GstCaps *caps = NULL;

  caps = gst_pad_get_current_caps (pad);

  result = gst_cam_base_src_prepare_allocation (basesrc, caps, pad);

    if (caps)
      gst_caps_unref (caps);
  }
  g_free (padname);
  return result;
}

static gboolean
gst_cam_base_src_start (GstCamBaseSrc * basesrc)
{
  GstCamBaseSrcClass *bclass;
  gboolean result;

  GST_LIVE_LOCK (basesrc);

  GST_OBJECT_LOCK (basesrc);
  if (GST_CAM_BASE_SRC_IS_STARTING (basesrc))
    goto was_starting;
  if (GST_CAM_BASE_SRC_IS_STARTED (basesrc))
    goto was_started;

  basesrc->priv->start_result = GST_FLOW_FLUSHING;
  GST_OBJECT_FLAG_SET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTING);
  gst_segment_init (&basesrc->segment, basesrc->segment.format);
  GST_OBJECT_UNLOCK (basesrc);

  basesrc->num_buffers_left = basesrc->num_buffers;
  basesrc->running = FALSE;
  basesrc->priv->segment_pending = FALSE;
  basesrc->priv->segment_seqnum = gst_util_seqnum_next ();
  basesrc->priv->forced_eos = FALSE;
  GST_LIVE_UNLOCK (basesrc);

  bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);
  if (bclass->start)
    result = bclass->start (basesrc);
  else
    result = TRUE;

  if (!result)
    goto could_not_start;

  if (!gst_cam_base_src_is_async (basesrc)) {
    gst_cam_base_src_start_complete (basesrc, GST_FLOW_OK);
    /* not really waiting here, we call this to get the result
     * from the start_complete call */
    result = gst_cam_base_src_start_wait (basesrc) == GST_FLOW_OK;
  }

  return result;

  /* ERROR */
was_starting:
  {
    GST_DEBUG_OBJECT (basesrc, "was starting");
    GST_OBJECT_UNLOCK (basesrc);
    GST_LIVE_UNLOCK (basesrc);
    return TRUE;
  }
was_started:
  {
    GST_DEBUG_OBJECT (basesrc, "was started");
    GST_OBJECT_UNLOCK (basesrc);
    GST_LIVE_UNLOCK (basesrc);
    return TRUE;
  }
could_not_start:
  {
    GST_DEBUG_OBJECT (basesrc, "could not start");
    /* subclass is supposed to post a message. We don't have to call _stop. */
    gst_cam_base_src_start_complete (basesrc, GST_FLOW_ERROR);
    return FALSE;
  }
}

/**
 * gst_cam_base_src_start_complete:
 * @basesrc: base source instance
 * @ret: a #GstFlowReturn
 *
 * Complete an asynchronous start operation. When the subclass overrides the
 * start method, it should call gst_cam_base_src_start_complete() when the start
 * operation completes either from the same thread or from an asynchronous
 * helper thread.
 */
void
gst_cam_base_src_start_complete (GstCamBaseSrc * basesrc, GstFlowReturn ret)
{
  gboolean have_size;
  guint64 size;
  gboolean seekable;
  GstFormat format;
  GstPadMode mode;
  GstEvent *event;

  if (ret != GST_FLOW_OK)
    goto error;

  GST_DEBUG_OBJECT (basesrc, "starting source");
  format = basesrc->segment.format;

  /* figure out the size */
  have_size = FALSE;
  size = -1;
  if (format == GST_FORMAT_BYTES) {
    GstCamBaseSrcClass *bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);

    if (bclass->get_size) {
      if (!(have_size = bclass->get_size (basesrc, &size)))
        size = -1;
    }
    GST_DEBUG_OBJECT (basesrc, "setting size %" G_GUINT64_FORMAT, size);
    /* only update the size when operating in bytes, subclass is supposed
     * to set duration in the start method for other formats */
    GST_OBJECT_LOCK (basesrc);
    basesrc->segment.duration = size;
    GST_OBJECT_UNLOCK (basesrc);
  }

  GST_DEBUG_OBJECT (basesrc,
      "format: %s, have size: %d, size: %" G_GUINT64_FORMAT ", duration: %"
      G_GINT64_FORMAT, gst_format_get_name (format), have_size, size,
      basesrc->segment.duration);

  seekable = gst_cam_base_src_seekable (basesrc);
  GST_DEBUG_OBJECT (basesrc, "is seekable: %d", seekable);

  /* update for random access flag */
  basesrc->random_access = seekable && format == GST_FORMAT_BYTES;

  GST_DEBUG_OBJECT (basesrc, "is random_access: %d", basesrc->random_access);

  /* stop flushing now but for live sources, still block in the LIVE lock when
   * we are not yet PLAYING */
  gst_cam_base_src_set_flushing (basesrc, FALSE, FALSE, NULL);

  gst_pad_mark_reconfigure (GST_CAM_BASE_SRC_PAD (basesrc));

  GST_OBJECT_LOCK (basesrc->srcpad);
  mode = GST_PAD_MODE (basesrc->srcpad);
  GST_OBJECT_UNLOCK (basesrc->srcpad);

  /* take the stream lock here, we only want to let the task run when we have
   * set the STARTED flag */
  GST_PAD_STREAM_LOCK (basesrc->srcpad);

  switch (mode) {
    case GST_PAD_MODE_PUSH:
      /* do initial seek, which will start the task */
      GST_OBJECT_LOCK (basesrc);
      event = basesrc->pending_seek;
      basesrc->pending_seek = NULL;
      GST_OBJECT_UNLOCK (basesrc);

      /* The perform seek code will start the task when finished. We don't have to
       * unlock the streaming thread because it is not running yet */
      if (G_UNLIKELY (!gst_cam_base_src_perform_seek (basesrc, event, FALSE)))
        goto seek_failed;

      if (event)
        gst_event_unref (event);
      break;
    case GST_PAD_MODE_PULL:
      /* if not random_access, we cannot operate in pull mode for now */
      if (G_UNLIKELY (!basesrc->random_access))
        goto no_get_range;
      break;
    default:
      goto not_activated_yet;
      break;
  }

  GST_OBJECT_LOCK (basesrc);
  GST_OBJECT_FLAG_SET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTED);
  GST_OBJECT_FLAG_UNSET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTING);
  basesrc->priv->start_result = ret;
  GST_ASYNC_SIGNAL (basesrc);
  GST_OBJECT_UNLOCK (basesrc);

  GST_PAD_STREAM_UNLOCK (basesrc->srcpad);

  return;

seek_failed:
  {
    GST_PAD_STREAM_UNLOCK (basesrc->srcpad);
    GST_ERROR_OBJECT (basesrc, "Failed to perform initial seek");
    gst_cam_base_src_stop (basesrc);
    if (event)
      gst_event_unref (event);
    ret = GST_FLOW_ERROR;
    goto error;
  }
no_get_range:
  {
    GST_PAD_STREAM_UNLOCK (basesrc->srcpad);
    gst_cam_base_src_stop (basesrc);
    GST_ERROR_OBJECT (basesrc, "Cannot operate in pull mode, stopping");
    ret = GST_FLOW_ERROR;
    goto error;
  }
not_activated_yet:
  {
    GST_PAD_STREAM_UNLOCK (basesrc->srcpad);
    gst_cam_base_src_stop (basesrc);
    GST_WARNING_OBJECT (basesrc, "pad not activated yet");
    ret = GST_FLOW_ERROR;
    goto error;
  }
error:
  {
    GST_OBJECT_LOCK (basesrc);
    basesrc->priv->start_result = ret;
    GST_OBJECT_FLAG_UNSET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTING);
    GST_ASYNC_SIGNAL (basesrc);
    GST_OBJECT_UNLOCK (basesrc);
    return;
  }
}

/**
 * gst_cam_base_src_start_wait:
 * @basesrc: base source instance
 *
 * Wait until the start operation completes.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
gst_cam_base_src_start_wait (GstCamBaseSrc * basesrc)
{
  GstFlowReturn result;

  GST_OBJECT_LOCK (basesrc);
  while (GST_CAM_BASE_SRC_IS_STARTING (basesrc)) {
    GST_ASYNC_WAIT (basesrc);
  }
  result = basesrc->priv->start_result;
  GST_OBJECT_UNLOCK (basesrc);

  GST_DEBUG_OBJECT (basesrc, "got %s", gst_flow_get_name (result));

  return result;
}

static gboolean
gst_cam_base_src_stop (GstCamBaseSrc * basesrc)
{
  GstCamBaseSrcClass *bclass;
  gboolean result = TRUE;

  GST_DEBUG_OBJECT (basesrc, "stopping source");

  /* flush all */
  gst_cam_base_src_set_flushing (basesrc, TRUE, FALSE, NULL);
  /* stop the task */
  gst_pad_stop_task (basesrc->srcpad);

  GST_OBJECT_LOCK (basesrc);
  if (!GST_CAM_BASE_SRC_IS_STARTED (basesrc) && !GST_CAM_BASE_SRC_IS_STARTING (basesrc))
    goto was_stopped;

  GST_OBJECT_FLAG_UNSET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTING);
  GST_OBJECT_FLAG_UNSET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTED);
  basesrc->priv->start_result = GST_FLOW_FLUSHING;
  GST_ASYNC_SIGNAL (basesrc);
  GST_OBJECT_UNLOCK (basesrc);

  /* for multi stream, don't call stop until all buffer pool are deactivated */
  bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);
  if (bclass->stop)
    result = bclass->stop (basesrc);

  basesrc->live_running = FALSE;

  gst_cam_base_src_set_allocation (basesrc, basesrc->srcpad, NULL, NULL, NULL);

  for (guint i = 0; i < basesrc->priv->request_stream_map.size(); i++) {
      if (basesrc->mux[i].videopad) {
        gst_pad_stop_task(basesrc->mux[i].videopad);
        gst_cam_base_src_set_allocation (basesrc, basesrc->mux[i].videopad, NULL, NULL, NULL);
        g_mutex_clear (&basesrc->mux[i].vid_live_lock);
      }
  }

  return result;

was_stopped:
  {
    GST_DEBUG_OBJECT (basesrc, "was stopped");
    GST_OBJECT_UNLOCK (basesrc);
    return TRUE;
  }
}

/* start or stop flushing dataprocessing
 */
static gboolean
gst_cam_base_src_set_flushing (GstCamBaseSrc * basesrc,
    gboolean flushing, gboolean live_play, gboolean * playing)
{
  GstCamBaseSrcClass *bclass;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);

  GST_DEBUG_OBJECT (basesrc, "flushing %d, live_play %d", flushing, live_play);

  if (flushing) {
    gst_cam_base_src_activate_pool (basesrc, FALSE);
    gst_cam_base_src_activate_video_pool(basesrc, FALSE);
    /* unlock any subclasses, we need to do this before grabbing the
     * LIVE_LOCK since we hold this lock before going into ::create. We pass an
     * unlock to the params because of backwards compat (see seek handler)*/
    if (bclass->unlock)
      bclass->unlock (basesrc);
  }

  /* the live lock is released when we are blocked, waiting for playing or
   * when we sync to the clock. */
  GST_LIVE_LOCK (basesrc);
  if (playing)
    *playing = basesrc->live_running;
  basesrc->priv->flushing = flushing;
  if (flushing) {
    /* if we are locked in the live lock, signal it to make it flush */
    basesrc->live_running = TRUE;

    /* clear pending EOS if any */
    if (g_atomic_int_get (&basesrc->priv->has_pending_eos)) {
      GST_OBJECT_LOCK (basesrc);
      CLEAR_PENDING_EOS (basesrc);
      basesrc->priv->forced_eos = FALSE;
      GST_OBJECT_UNLOCK (basesrc);
    }

    /* step 1, now that we have the LIVE lock, clear our unlock request */
    if (bclass->unlock_stop)
      bclass->unlock_stop (basesrc);

    /* step 2, unblock clock sync (if any) or any other blocking thing */
    if (basesrc->clock_id)
      gst_clock_id_unschedule (basesrc->clock_id);
    } else {
    /* signal the live source that it can start playing */
    basesrc->live_running = live_play;

    gst_cam_base_src_activate_pool (basesrc, TRUE);
    gst_cam_base_src_activate_video_pool(basesrc, TRUE);

    /* Drop all delayed events */
    GST_OBJECT_LOCK (basesrc);
    if (basesrc->priv->pending_events) {
      g_list_foreach (basesrc->priv->pending_events, (GFunc) gst_event_unref,
          NULL);
      g_list_free (basesrc->priv->pending_events);
      basesrc->priv->pending_events = NULL;
      g_atomic_int_set (&basesrc->priv->have_events, FALSE);
    }
    GST_OBJECT_UNLOCK (basesrc);
  }
  GST_LIVE_SIGNAL (basesrc);
  GST_LIVE_UNLOCK (basesrc);

  return TRUE;
}

/* the purpose of this function is to make sure that a live source blocks in the
 * LIVE lock or leaves the LIVE lock and continues playing. */
static gboolean
gst_cam_base_src_set_playing (GstCamBaseSrc * basesrc, gboolean live_play)
{
  GstCamBaseSrcClass *bclass;

  bclass = GST_CAM_BASE_SRC_GET_CLASS (basesrc);

  /* unlock subclasses locked in ::create, we only do this when we stop playing. */
  if (!live_play) {
    GST_DEBUG_OBJECT (basesrc, "unlock");
    if (bclass->unlock)
      bclass->unlock (basesrc);
  }

  /* we are now able to grab the LIVE lock, when we get it, we can be
   * waiting for PLAYING while blocked in the LIVE cond or we can be waiting
   * for the clock. */
  GST_LIVE_LOCK (basesrc);
  GST_DEBUG_OBJECT (basesrc, "unschedule clock");

  /* unblock clock sync (if any) */
  if (basesrc->clock_id)
    gst_clock_id_unschedule (basesrc->clock_id);

  /* configure what to do when we get to the LIVE lock. */
  GST_DEBUG_OBJECT (basesrc, "live running %d", live_play);
  basesrc->live_running = live_play;

  if (live_play) {
    gboolean start;

    /* clear our unlock request when going to PLAYING */
    GST_DEBUG_OBJECT (basesrc, "unlock stop");
    if (bclass->unlock_stop)
      bclass->unlock_stop (basesrc);

    /* for live sources we restart the timestamp correction */
    basesrc->priv->latency = -1;
    /* have to restart the task in case it stopped because of the unlock when
     * we went to PAUSED. Only do this if we operating in push mode. */
    GST_OBJECT_LOCK (basesrc->srcpad);
    start = (GST_PAD_MODE (basesrc->srcpad) == GST_PAD_MODE_PUSH);
    GST_OBJECT_UNLOCK (basesrc->srcpad);
    if (start)
      gst_pad_start_task (basesrc->srcpad, (GstTaskFunction) gst_cam_base_src_loop,
          basesrc->srcpad, NULL);
    GST_DEBUG_OBJECT (basesrc, "signal");
    GST_LIVE_SIGNAL (basesrc);
  }
  GST_LIVE_UNLOCK (basesrc);

  return TRUE;
}

static gboolean gst_cam_base_src_activate_push (GstPad * pad, GstObject * parent, gboolean active)
{
  GstCamBaseSrc *basesrc;

  basesrc = GST_CAM_BASE_SRC (parent);

  /* prepare subclass first */
  if (active) {
    GST_DEBUG_OBJECT (basesrc, "Activating in push mode");

    if (G_UNLIKELY (!basesrc->can_activate_push))
      goto no_push_activation;

    if (G_UNLIKELY (!gst_cam_base_src_start (basesrc)))
      goto error_start;
  } else {
    GST_DEBUG_OBJECT (basesrc, "Deactivating in push mode");
    /* now we can stop the source */
    if (G_UNLIKELY (!gst_cam_base_src_stop (basesrc)))
      goto error_stop;
  }
  return TRUE;

  /* ERRORS */
no_push_activation:
  {
    GST_WARNING_OBJECT (basesrc, "Subclass disabled push-mode activation");
    return FALSE;
  }
error_start:
  {
    GST_WARNING_OBJECT (basesrc, "Failed to start in push mode");
    return FALSE;
  }
error_stop:
  {
    GST_DEBUG_OBJECT (basesrc, "Failed to stop in push mode");
    return FALSE;
  }
}

static gboolean gst_cam_base_src_video_activate_push (GstPad * pad, GstObject * parent, guint stream_id, gboolean active)
{
  GstCamBaseSrc *basesrc = GST_CAM_BASE_SRC(parent);
  GstFlowReturn ret = GST_FLOW_OK;
  gboolean res = TRUE;

  if (active) {
    basesrc->mux[stream_id].num_video_buffers_left = basesrc->num_buffers;
    GST_DEBUG_OBJECT(basesrc, "Activating video pad in push mode.");
    GST_OBJECT_LOCK(basesrc);
    GstPadMode mode = GST_PAD_MODE(basesrc->mux[stream_id].videopad);
    GST_OBJECT_UNLOCK(basesrc);

    GST_PAD_STREAM_LOCK(basesrc->mux[stream_id].videopad);
    switch (mode) {
      case GST_PAD_MODE_PUSH:
        res = gst_pad_start_task(basesrc->mux[stream_id].videopad, (GstTaskFunction)gst_cam_base_src_video_loop,
            basesrc->mux[stream_id].videopad, NULL);
        break;
      default:
        goto not_activated_yet;
        break;
    }
    GST_PAD_STREAM_UNLOCK(basesrc->mux[stream_id].videopad);
  } else {
    res = gst_pad_stop_task(basesrc->mux[stream_id].videopad);
  }

  return res;

not_activated_yet:
  {
    GST_PAD_STREAM_UNLOCK (basesrc->mux[stream_id].videopad);
    gst_cam_base_src_stop (basesrc);
    GST_WARNING_OBJECT (basesrc, "video pad not activated yet");
    ret = GST_FLOW_ERROR;
    res = FALSE;
    goto error;
  }
error:
  {
    GST_OBJECT_LOCK (basesrc);
    basesrc->priv->start_result = ret;
    GST_OBJECT_FLAG_UNSET (basesrc, GST_CAM_BASE_SRC_FLAG_STARTING);
    GST_ASYNC_SIGNAL (basesrc);
    GST_OBJECT_UNLOCK (basesrc);
    return res;
  }
}

static gboolean
gst_cam_base_src_activate_pull (GstPad * pad, GstObject * parent, gboolean active)
{
  GstCamBaseSrc *basesrc;

  basesrc = GST_CAM_BASE_SRC (parent);

  /* prepare subclass first */
  if (active) {
    GST_DEBUG_OBJECT (basesrc, "Activating in pull mode");
    if (G_UNLIKELY (!gst_cam_base_src_start (basesrc)))
      goto error_start;
  } else {
    GST_DEBUG_OBJECT (basesrc, "Deactivating in pull mode");
    if (G_UNLIKELY (!gst_cam_base_src_stop (basesrc)))
      goto error_stop;
  }
  return TRUE;

  /* ERRORS */
error_start:
  {
    GST_ERROR_OBJECT (basesrc, "Failed to start in pull mode");
    return FALSE;
  }
error_stop:
  {
    GST_ERROR_OBJECT (basesrc, "Failed to stop in pull mode");
    return FALSE;
  }
}

const gchar *
gst_pad_mode_get_name (GstPadMode mode)
{
  switch (mode) {
    case GST_PAD_MODE_NONE:
      return "none";
    case GST_PAD_MODE_PUSH:
      return "push";
    case GST_PAD_MODE_PULL:
      return "pull";
    default:
      break;
  }
  return "unknown";
}

static gboolean
gst_cam_base_src_activate_mode (GstPad * pad, GstObject * parent,
    GstPadMode mode, gboolean active)
{
    gboolean res = TRUE;
    GstCamBaseSrc *src = GST_CAM_BASE_SRC (parent);
    gchar *padname = gst_pad_get_name(pad);

    GST_DEBUG_OBJECT (pad, "activating %s pad in mode %s",
       padname, gst_pad_mode_get_name(mode));

    switch (mode) {
      case GST_PAD_MODE_PULL:
        res = gst_cam_base_src_activate_pull(pad, parent, active);
        break;
      case GST_PAD_MODE_PUSH:
        if (pad == src->srcpad) {
          src->priv->stream_start_pending = active;
          res = gst_cam_base_src_activate_push (pad, parent, active);
        } else {
          int stream_id = CameraSrcUtils::get_stream_id_by_pad(src->priv->request_stream_map, pad);
          src->priv->muxPriv[stream_id].vid_stream_start_pending = active;
          res = gst_cam_base_src_video_activate_push (pad, parent, stream_id, active);
        }
        break;
      default:
        GST_WARNING ("unknown activation mode %s", gst_pad_mode_get_name(mode));
        res = FALSE;
        break;
    }
    g_free (padname);
    return res;
}

static GstStateChangeReturn
gst_cam_base_src_change_state (GstElement * element, GstStateChange transition)
{
  GstCamBaseSrc *basesrc;
  GstStateChangeReturn result;
  gboolean no_preroll = FALSE;

  basesrc = GST_CAM_BASE_SRC (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      GST_DEBUG("current state: NULL -> READY\n");
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      GST_DEBUG("current state: READY -> PAUSED\n");
      no_preroll = gst_cam_base_src_is_live (basesrc);
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      GST_DEBUG("current state: PAUSED -> PLAYING\n");
      if (gst_cam_base_src_is_live (basesrc)) {
        /* now we can start playback */
        gst_cam_base_src_set_playing (basesrc, TRUE);
      }
      break;
    default:
      break;
  }

  if ((result =
          GST_ELEMENT_CLASS (parent_class)->change_state (element,
              transition)) == GST_STATE_CHANGE_FAILURE)
    goto failure;

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      GST_DEBUG("current state: PLAYING -> PAUSED\n");
      if (gst_cam_base_src_is_live (basesrc)) {
        /* make sure we block in the live lock in PAUSED */
        gst_cam_base_src_set_playing (basesrc, FALSE);
        no_preroll = TRUE;
      }
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
    {
      GST_DEBUG("current state: PAUSED -> READY\n");
      /* we don't need to unblock anything here, the pad deactivation code
       * already did this */
      if (g_atomic_int_get (&basesrc->priv->has_pending_eos)) {
        GST_OBJECT_LOCK (basesrc);
        CLEAR_PENDING_EOS (basesrc);
        GST_OBJECT_UNLOCK (basesrc);
      }
      gst_event_replace (&basesrc->pending_seek, NULL);
      break;
    }
    case GST_STATE_CHANGE_READY_TO_NULL:
      GST_DEBUG("current state: READY -> NULL\n");
      break;
    default:
      break;
  }

  if (no_preroll && result == GST_STATE_CHANGE_SUCCESS)
    result = GST_STATE_CHANGE_NO_PREROLL;

  return result;

  /* ERRORS */
failure:
  {
    GST_DEBUG_OBJECT (basesrc, "parent failed state change");
    return result;
  }
}

/**
 * gst_cam_base_src_get_buffer_pool:
 * @src: a #GstCamBaseSrc
 *
 * Returns: (transfer full): the instance of the #GstBufferPool used
 * by the src; unref it after usage.
 */
GstBufferPool *
gst_cam_base_src_get_buffer_pool (GstCamBaseSrc * src)
{
  g_return_val_if_fail (GST_IS_CAM_BASE_SRC (src), NULL);

  if (src->priv->pool)
    return (GstBufferPool *)gst_object_ref (src->priv->pool);

  return NULL;
}

/**
 * gst_cam_base_src_get_allocator:
 * @src: a #GstCamBaseSrc
 * @allocator: (out) (allow-none) (transfer full): the #GstAllocator
 * used
 * @params: (out) (allow-none) (transfer full): the
 * #GstAllocationParams of @allocator
 *
 * Lets #GstCamBaseSrc sub-classes to know the memory @allocator
 * used by the base class and its @params.
 *
 * Unref the @allocator after usage.
 */
void
gst_cam_base_src_get_allocator (GstCamBaseSrc * src,
    GstAllocator ** allocator, GstAllocationParams * params)
{
  g_return_if_fail (GST_IS_CAM_BASE_SRC (src));

  if (allocator)
    *allocator = src->priv->allocator ?
        (GstAllocator *)gst_object_ref (src->priv->allocator) : NULL;

  if (params)
    *params = src->priv->params;
}
