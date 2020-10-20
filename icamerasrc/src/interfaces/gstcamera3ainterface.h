/*
 * GStreamer
 * Copyright (C) 2016-2017 Intel Corporation
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

#ifndef GST_CAMERASRC_3A_INTERFACE_H
#define GST_CAMERASRC_3A_INTERFACE_H

#include <gst/gst.h>
#include <ICamera.h>
#include <Parameters.h>

using namespace icamera;

G_BEGIN_DECLS
GST_DEBUG_CATEGORY_EXTERN(gst_camerasrc_debug);
#define GST_CAT_DEFAULT gst_camerasrc_debug

/* Get icamerasrc 3A interface type */
#define GST_TYPE_CAMERASRC_3A_IF (gst_camerasrc_3a_interface_get_type())

/* Get icamerasrc 3A handle */
#define GST_CAMERASRC_3A(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_CAMERASRC_3A_IF, GstCamerasrc3A))

/* Get icamerasrc 3A interface */
#define GST_CAMERASRC_3A_GET_INTERFACE(inst) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GST_TYPE_CAMERASRC_3A_IF, GstCamerasrc3AInterface))

/* Usage:
   * - GstCamerasrc3A *cam3a = GST_CAMERASRC_3A(camsrc);
   * - GstCamerasrc3AInterface *iface = GST_CAMERASRC_3A_GET_INTERFACE(cam3a);
   * - iface->set_exposure_time(cam3a, 100);
   * Note: camsrc could be instance of Gstcamerasrc or GstElement type.
   */
typedef struct _GstCamerasrc3A GstCamerasrc3A;
typedef struct _GstCamerasrc3AInterface GstCamerasrc3AInterface;

struct _GstCamerasrc3AInterface {
  /* Inherent from GTypeInterface */
  GTypeInterface base;

  /* Get customized effects
  * param[in]                 cam3a    Camera Source handle
  * param[in, out]       img_enhancement    image enhancement(sharpness, brightness, contrast, hue, saturation)
  * return            camera_image_enhancement_t
  */
  camera_image_enhancement_t      (*get_image_enhancement)         (GstCamerasrc3A *cam3a, camera_image_enhancement_t img_enhancement);

  /* Set customized effects
  * param[in]        cam3a    Camera Source handle
  * param[in]        img_enhancement    image enhancement(sharpness, brightness, contrast, hue, saturation)
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_image_enhancement)         (GstCamerasrc3A *cam3a, camera_image_enhancement_t img_enhancement);

  /* Set exposure time whose unit is microsecond
  * param[in]        cam3a    Camera Source handle
  * param[in]        exp_time    exposure time
  * return 0 if exposure time was set, non-0 means no exposure time was set
  */
  gboolean      (*set_exposure_time)         (GstCamerasrc3A *cam3a, guint exp_time);

  /* Set iris mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        irisMode        IRIS_MODE_AUTO(default),
  *                                  IRIS_MODE_MANUAL,
  *                                  IRIS_MODE_CUSTOMIZED,
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_iris_mode)         (GstCamerasrc3A *cam3a, camera_iris_mode_t irisMode);

  /* Set iris level
  * param[in]        cam3a    Camera Source handle
  * param[in]        irisLevel    iris level
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_iris_level)         (GstCamerasrc3A *cam3a, int irisLevel);

  /* Set sensor gain (unit: db)
  * The sensor gain only take effect when ae mode set to manual
  * param[in]        cam3a    Camera Source handle
  * param[in]        gain    gain
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_gain)      (GstCamerasrc3A *cam3a, float gain);

  /* Set BLC Area mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        blcAreaMode        BLC_AREA_MODE_OFF(default),
  *                                     BLC_AREA_MODE_ON,
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_blc_area_mode)      (GstCamerasrc3A *cam3a, camera_blc_area_mode_t blcAreaMode);

  /* Set WDR level
  * param[in]        cam3a    Camera Source handle
  * param[in]        level    wdr level
  * return 0 if set successfully, otherwise non-0 value is returned.
  */
  gboolean      (*set_wdr_level)      (GstCamerasrc3A *cam3a, uint8_t level);

  /* Set AWB mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        awbMode        AWB_MODE_AUTO(default),
  *                                 AWB_MODE_INCANDESCENT,
  *                                 AWB_MODE_FLUORESCENT,
  *                                 AWB_MODE_DAYLIGHT,
  *                                 AWB_MODE_FULL_OVERCAST,
  *                                 AWB_MODE_PARTLY_OVERCAST,
  *                                 AWB_MODE_SUNSET,
  *                                 AWB_MODE_VIDEO_CONFERENCE,
  *                                 AWB_MODE_MANUAL_CCT_RANGE,
  *                                 AWB_MODE_MANUAL_WHITE_POINT,
  *                                 AWB_MODE_MANUAL_GAIN,
  *                                 AWB_MODE_MANUAL_COLOR_TRANSFORM,
  * return 0 if set successfully, otherwise non-0 value is returned.
  */
  gboolean      (*set_awb_mode)      (GstCamerasrc3A *cam3a, camera_awb_mode_t awbMode);

  /* Get customized awb gains currently used
  * param[in]        cam3a    Camera Source handle
  * param[in, out]        awbGains    awb gains(r_gain, g_gain, b_gain)
  * return 0 if awb gain was set, non-0 means no awb gain was set.
  */
  camera_awb_gains_t      (*get_awb_gain)      (GstCamerasrc3A *cam3a, camera_awb_gains_t& awbGains);

  /* Set AWB gain
  * param[in]        cam3a    Camera Source handle
  * param[in]        awbGains    awb gains(r_gain, g_gain, b_gain)
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_awb_gain)      (GstCamerasrc3A *cam3a, camera_awb_gains_t awbGains);

  /* Set Scene mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        sceneMode        SCENE_MODE_AUTO(default),
  *                                   SCENE_MODE_HDR,
  *                                   SCENE_MODE_ULL,
  *                                   SCENE_MODE_HLC,
  *                                   SCENE_MODE_NORMAL,
  *                                   SCENE_MODE_CUSTOM_AIC,
  *                                   SCENE_MODE_VIDEO_LL,
  *                                   SCENE_MODE_MAX
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_scene_mode)       (GstCamerasrc3A *cam3a, camera_scene_mode_t sceneMode);

  /* Set AE mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        aeMode        AE_MODE_AUTO,
  *                                AE_MODE_MANUAL
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_ae_mode)      (GstCamerasrc3A *cam3a, camera_ae_mode_t aeMode);

  /* Set weight grid mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        weightGridMode        WEIGHT_GRID_AUTO(default),
  *                                        CUSTOM_WEIGHT_GRID_1,
  *                                        CUSTOM_WEIGHT_GRID_2,
  *                                        CUSTOM_WEIGHT_GRID_3,
  *                                        CUSTOM_WEIGHT_GRID_4,
  *                                        CUSTOM_WEIGHT_GRID_5,
  *                                        CUSTOM_WEIGHT_GRID_6,
  *                                        CUSTOM_WEIGHT_GRID_7,
  *                                        CUSTOM_WEIGHT_GRID_8,
  *                                        CUSTOM_WEIGHT_GRID_9,
  *                                        CUSTOM_WEIGHT_GRID_10,
  *                                        CUSTOM_WEIGHT_GRID_MAX
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_weight_grid_mode)      (GstCamerasrc3A *cam3a, camera_weight_grid_mode_t weightGridMode);

  /* Set AE converge speed
  * param[in]        cam3a    Camera Source handle
  * param[in]        speed        CONVERGE_NORMAL(default),
  *                               CONVERGE_MID,
  *                               CONVERGE_LOW,
  *                               CONVERGE_MAX
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_ae_converge_speed)      (GstCamerasrc3A *cam3a, camera_converge_speed_t speed);

  /* Set AWB converge speed
  * param[in]        cam3a    Camera Source handle
  * param[in]        speed        CONVERGE_NORMAL(default),
  *                               CONVERGE_MID,
  *                               CONVERGE_LOW,
  *                               CONVERGE_MAX
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_awb_converge_speed)      (GstCamerasrc3A *cam3a, camera_converge_speed_t speed);

  /* Set AE converge speed mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        mode        CONVERGE_SPEED_MODE_AIQ(default),
  *                              CONVERGE_SPEED_MODE_HAL
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_ae_converge_speed_mode)      (GstCamerasrc3A *cam3a, camera_converge_speed_mode_t mode);

  /* Set AWB converge speed mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        mode        CONVERGE_SPEED_MODE_AIQ(default),
  *                              CONVERGE_SPEED_MODE_HAL
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_awb_converge_speed_mode)      (GstCamerasrc3A *cam3a, camera_converge_speed_mode_t mode);

  /* Set exposure ev
  * param[in]        cam3a    Camera Source handle
  * param[in]        ev    exposure EV
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_exposure_ev)      (GstCamerasrc3A *cam3a, int ev);

  /* Set exposure priority
  * param[in]        cam3a    Camera Source handle
  * param[in]        priority        DISTRIBUTION_AUTO(default),
  *                                  DISTRIBUTION_SHUTTER,
  *                                  DISTRIBUTION_ISO,
  *                                  DISTRIBUTION_APERTURE
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_exposure_priority)      (GstCamerasrc3A *cam3a, camera_ae_distribution_priority_t priority);

  /* Get AWB cct range
  * Customized cct range only take effect when awb mode is set to AWB_MODE_MANUAL_CCT_RANGE
  * param[in]        cam3a    Camera Source handle
  * param[in, out]        cct    cct range(min, max)
  * return            camera_range_t
  */
  camera_range_t      (*get_awb_cct_range)      (GstCamerasrc3A *cam3a, camera_range_t& cct);

  /* Set AWB cct range
  * param[in]        cam3a    Camera Source handle
  * param[in]        cct    cct range(min, max)
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_awb_cct_range)      (GstCamerasrc3A *cam3a, camera_range_t cct);

  /* Get white point
  * param[in]        cam3a    Camera Source handle
  * param[in, out]        whitePoint    white point coordinate(x, y)
  * return            camera_coordinate_t
  */
  camera_coordinate_t      (*get_white_point)      (GstCamerasrc3A *cam3a, camera_coordinate_t &whitePoint);

  /* Set white point
  * Only take effect when awb mode is set to AWB_MODE_MANUAL_WHITE_POINT.
  * The coordinate system is based on frame which is currently displayed.
  * param[in]        cam3a    Camera Source handle
  * param[in]        whitePoint    white point coordinate(x, y)
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_white_point)      (GstCamerasrc3A *cam3a, camera_coordinate_t whitePoint);

  /* Get AWB gain shift
  * param[in]        cam3a    Camera Source handle
  * param[in, out]        awbGainShift    gain shift(r_gain, g_gain, b_gain)
  * return camera_awb_gains_t
  */
  camera_awb_gains_t      (*get_awb_gain_shift)      (GstCamerasrc3A *cam3a, camera_awb_gains_t& awbGainShift);

  /* Set AWB gain shift
  * param[in]        cam3a    Camera Source handle
  * param[in]        awbGainShift    gain shift(r_gain, g_gain, b_gain)
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_awb_gain_shift)       (GstCamerasrc3A *cam3a, camera_awb_gains_t awbGainShift);

  /* Set AE region
  * param[in]        cam3a    Camera Source handle
  * param[in]        aeRegions    regions(left, top, right, bottom, weight)
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_ae_region)      (GstCamerasrc3A *cam3a, camera_window_list_t aeRegions);

  /* Set color transform
  * param[in]        cam3a    Camera Source handle
  * param[in]        colorTransform    float array
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_color_transform)      (GstCamerasrc3A *cam3a, camera_color_transform_t colorTransform);

  /* Set custom Aic param
  * param[in]        cam3a    Camera Source handle
  * param[in]        data    the pointer of destination buffer
  * param[in]        length    but buffer size
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_custom_aic_param)      (GstCamerasrc3A *cam3a, const void* data, unsigned int length);

  /* Set antibanding mode
  * param[in]        cam3a    Camera Source handle
  * param[in]        bandingMode        ANTIBANDING_MODE_AUTO,
  *                                     ANTIBANDING_MODE_50HZ,
  *                                     ANTIBANDING_MODE_60HZ,
  *                                     ANTIBANDING_MODE_OFF,
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_antibanding_mode)      (GstCamerasrc3A *cam3a, camera_antibanding_mode_t bandingMode);

  /* Set color range mode
  * param[in]        cam3a        Camera Source handle
  * param[in]        colorRangeMode     CAMERA_FULL_MODE_YUV_COLOR_RANGE,
  *                                     CAMERA_REDUCED_MODE_YUV_COLOR_RANGE,
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_color_range_mode)      (GstCamerasrc3A *cam3a, camera_yuv_color_range_mode_t colorRangeMode);

  /* Set exposure time range
  * param[in]        cam3a        Camera Source handle
  * param[in]        exposureTimeRange        the exposure time range to be set
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_exposure_time_range)      (GstCamerasrc3A *cam3a, camera_range_t exposureTimeRange);

  /* set sensitivity gain range
  * param[in]        cam3a        Camera Source handle
  * param[in]        sensitivityGainRange        the sensitivity gain range to be set
  * return 0 if set successfully, otherwise non-0 value is returned
  */
  gboolean      (*set_sensitivity_gain_range)      (GstCamerasrc3A *cam3a, camera_range_t sensitivityGainRange);
};

GType gst_camerasrc_3a_interface_get_type(void);

G_END_DECLS

#endif /* __GST_CAMERASRC_3A_INTERFACE_H__ */
