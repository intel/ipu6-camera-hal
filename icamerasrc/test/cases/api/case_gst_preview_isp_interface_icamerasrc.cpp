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

#include <string.h>
#include "case_gst_common.h"
#ifndef CHROME_SLIM_CAMHAL
/* Example to send struct to interface */
TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_wb_gains)
{
  int ret = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_wb_gains;
  struct camera_control_isp_wb_gains_t isp_ctl[ISP_CONTROL_TEST_NUM] = {3.0, 2.0, 3.0, 4.0,
                                                                        2.0, 3.0, 4.0, 2.0,
                                                                        4.0, 2.0, 2.0, 3.0};
  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}


TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_color_correction)
{
  int ret = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_color_correction_matrix;
  struct camera_control_isp_color_correction_matrix_t isp_ctl[ISP_CONTROL_TEST_NUM] =
                                                            {0, 0, 1, 0, 1, 0, 1, 0, 0,
                                                             1, 0, 0, 0, 1, 0, 0, 0, 1,
                                                             0, 0, 1, 0, 1, 0, 1, 0, 0};
  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_bxt_demosaic)
{
  int ret = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_bxt_demosaic;
  struct camera_control_isp_bxt_demosaic_t isp_ctl[ISP_CONTROL_TEST_NUM] =
                                                    {1, 1, 1, 17, 12, 0, 56,
                                                     1, 0, 0, 10, 5, 0, 40,
                                                     0, 0, 0, 25, 24, 0, 80};
  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_see)
{
  int ret = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_see;
  struct camera_control_isp_see_t isp_ctl[ISP_CONTROL_TEST_NUM] =
                                        {0, 14, 2, 200, 78, 21, 47,
                                         0, 4, 0, 100, 20, 5, 10,
                                         1, 20, 4, 250, 90, 30, 55};
  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_gamma_tone_map)
{
  int i = 0;
  int ret = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_gamma_tone_map;

  struct camera_control_isp_gamma_tone_map_t isp_ctl[ISP_CONTROL_TEST_NUM];

  for (i = 0; i < ISP_CONTROL_TEST_NUM; i++) {
    isp_ctl[i].gamma_lut_size = 1024;
    isp_ctl[i].tone_map_lut_size = 1024;
    for (int j = 0; j < 1024; j++)
      isp_ctl[i].tone_map[j] = 1;
  }

  for (i = 0; i < 1024; i++)
    isp_ctl[0].gamma[i] = (float)((float)1 / 1024 * i);

  for (i = 0; i < 512; i++)
    isp_ctl[1].gamma[i] = (float)(1 - ((float)1 / 1024 * i));
  for (i = 512; i < 1024; i++)
    isp_ctl[1].gamma[i] = (float)((float)1 / 1024 * i);

  for (i = 0; i < 512; i++)
    isp_ctl[2].gamma[i] = (float)((float)1 / 1024 * i);
  for (i = 512; i < 1024; i++)
    isp_ctl[2].gamma[i] = (float)(1 - ((float)1 / 1024 * i));

  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_tnr5_21)
{
  int ret = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_tnr5_21;
  struct camera_control_isp_tnr5_21_t isp_ctl[ISP_CONTROL_TEST_NUM];

  for(int i = 0; i < ISP_CONTROL_TEST_NUM; i++)
  {
    struct camera_control_isp_tnr5_21_t *data = (struct camera_control_isp_tnr5_21_t*)&isp_ctl[i];
    data->bypass = get_random_value(0,1);
    for(int j = 0;j < 64; j++) {
      data->nm_yy_xcu_b[j] = get_random_value(-32768,32767);
      data->nm_yc_xcu_b[j] = get_random_value(-32768,32767);
      data->nm_cy_xcu_b[j] = get_random_value(-32768,32767);
      data->nm_cc_xcu_b[j] = get_random_value(-32768,32767);
    }
    data->nm_y_log_est_min_b = get_random_value(-32768,32767);
    data->nm_y_log_est_max_b = get_random_value(-32768,32767);
    data->nm_c_log_est_min_b = get_random_value(-32768,32767);
    data->nm_c_log_est_max_b = get_random_value(-32768,32767);
    data->nm_Y_alpha_b       = get_random_value(0,32767);
    data->nm_C_alpha_b       = get_random_value(0,32767);
    data->Tnr_Strength_0     = get_random_value(0,100);
    data->Tnr_Strength_1     = get_random_value(0,100);
    data->SpNR_Static        = get_random_value(0,100);
    data->SpNR_Dynamic       = get_random_value(0,100);
    data->Radial_Gain        = get_random_value(0,100);
    data->SAD_Gain           = get_random_value(0,100);
    data->Pre_Sim_Gain       = get_random_value(0,100);
    data->Weight_In          = get_random_value(0,100);
    data->g_mv_x             = get_random_value(-128,127);
    data->g_mv_y             = get_random_value(-128,127);
    data->tbd_sim_gain       = get_random_value(5,128);
    data->NS_Gain            = get_random_value(0,100);
    data->nsw_gain           = get_random_value(0,100);
    data->nsw_sigma          = get_random_value(0,100);
    data->nsw_bias           = get_random_value(0,100);
    data->ns_clw_bias0       = get_random_value(0,100);
    data->ns_clw_bias1       = get_random_value(0,100);
    data->ns_clw_sigma       = get_random_value(0,100);
    data->ns_clw_center      = get_random_value(0,100);
    data->ns_norm_bias       = get_random_value(0,32767);
    data->ns_norm_coef       = get_random_value(-32768,32767);
    data->bypass_g_mv        = get_random_value(0,1);
    data->bypass_NS          = get_random_value(0,1);
  }
  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_bnlm)
{
  int ret = 0, i = 0, j = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 scene-mode=ull device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_bnlm;
  struct camera_control_isp_bnlm_t isp_ctl[ISP_CONTROL_TEST_NUM];

  for (j = 0; j < ISP_CONTROL_TEST_NUM; j++) {
    isp_ctl[j].bypass = 0;
    isp_ctl[j].detailix_x_range[0] = 200;
    isp_ctl[j].detailix_x_range[1] = 6553;
    isp_ctl[j].detailix_radgain = 65536;
    isp_ctl[j].detailix_coeffs[0] = 1;
    isp_ctl[j].detailix_coeffs[1] = 44;
    isp_ctl[j].detailix_coeffs[2] = 55830;
    isp_ctl[j].sad_mu_x_range[0] = 200;
    isp_ctl[j].sad_mu_x_range[1] = 6553;
    isp_ctl[j].sad_radgain = 65536;
    isp_ctl[j].sad_mu_coeffs[0] = 1;
    isp_ctl[j].sad_mu_coeffs[1] = 44;
    isp_ctl[j].sad_mu_coeffs[2] = 55830;
    isp_ctl[j].detailth[0] = 100;
    isp_ctl[j].detailth[1] = 467;
    isp_ctl[j].detailth[2] = 838;
    isp_ctl[j].sad_detailixlutx[0] = 100;
    isp_ctl[j].sad_detailixlutx[1] = 200;
    isp_ctl[j].sad_detailixlutx[2] = 734;
    isp_ctl[j].sad_detailixlutx[3] = 838;
    isp_ctl[j].sad_detailixluty[0] = 200;
    isp_ctl[j].sad_detailixluty[1] = 200;
    isp_ctl[j].sad_detailixluty[2] = 200;
    isp_ctl[j].sad_detailixluty[3] = 199;
    isp_ctl[j].numcandforavg[0] = 16;
    isp_ctl[j].numcandforavg[1] = 16;
    isp_ctl[j].numcandforavg[2] = 12;
    isp_ctl[j].numcandforavg[3] = 6;

    for (i = 0; i < 4; i++) {
      isp_ctl[j].blend_power[i] = 65535;
      isp_ctl[j].blend_th[i] = 1023;
      isp_ctl[j].blend_texturegain[i] = 204;
      isp_ctl[j].matchqualitycands[i] = 1170;
      isp_ctl[j].sad_spatialrad[i] = 4;
    }
    isp_ctl[j].blend_radgain = 0;
    isp_ctl[j].wsumminth = 256;
    isp_ctl[j].wmaxminth = 256;
    isp_ctl[j].rad_enable = 1;
  }

  isp_ctl[1].sad_detailixluty[0] = 8191;
  isp_ctl[1].sad_detailixluty[1] = 8203;
  isp_ctl[1].sad_detailixluty[2] = 8203;
  isp_ctl[1].sad_detailixluty[3] = 8178;

  isp_ctl[1].blend_power[0] = 65535;
  isp_ctl[1].blend_power[1] = 0;
  isp_ctl[1].blend_power[2] = 0;
  isp_ctl[1].blend_power[3] = 0;

  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_iefd)
{
  int ret = 0, i = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 scene-mode=ull device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_sc_iefd;
  struct camera_control_isp_sc_iefd_t isp_ctl[ISP_CONTROL_TEST_NUM];

  for (i = 0; i < ISP_CONTROL_TEST_NUM; i++) {
    memset(&(isp_ctl[i]), 0, sizeof(struct camera_control_isp_sc_iefd_t));
    isp_ctl[i].sharpening_power[0] = 38;
    isp_ctl[i].sharpening_power[1] = 11;
    isp_ctl[i].sharp_power_edge = 38;
    isp_ctl[i].sharp_power_detail = 51;
    isp_ctl[i].unsharp_weight_edge = 64;
    isp_ctl[i].unsharp_weight_detail = 64;
    isp_ctl[i].denoise_power = 0;
    isp_ctl[i].radial_denoise_power = 0;
    isp_ctl[i].shrpn_nega_lmt_txt = 410;
    isp_ctl[i].shrpn_posi_lmt_txt = 410;
    isp_ctl[i].shrpn_nega_lmt_dir = 0;
    isp_ctl[i].shrpn_posi_lmt_dir = 0;
    isp_ctl[i].cu_var_metric_th[0] = 0;
    isp_ctl[i].cu_var_metric_th[1] = 1;
    isp_ctl[i].cu_var_metric_th[2] = 100;
    isp_ctl[i].cu_var_metric_th[3] = 150;
    isp_ctl[i].cu_ed2_metric_th[0] = 0;
    isp_ctl[i].cu_ed2_metric_th[1] = 350;
    isp_ctl[i].cu_dir_metric_th[0] = 0;
    isp_ctl[i].cu_dir_metric_th[1] = 350;
    isp_ctl[i].rad_enable = 1;
    isp_ctl[i].cu_nr_power[0] = 19;
    isp_ctl[i].cu_nr_power[1] = 0;

    isp_ctl[i].unsharp_filter[0] = 132;
    isp_ctl[i].unsharp_filter[1] = -4;
    isp_ctl[i].unsharp_filter[2] = 12;
    isp_ctl[i].unsharp_filter[3] = 11;
    isp_ctl[i].unsharp_filter[4] = 4;
    isp_ctl[i].unsharp_filter[5] = 2;
    isp_ctl[i].unsharp_filter[6] = -1;
    isp_ctl[i].unsharp_filter[7] = 1;
    isp_ctl[i].unsharp_filter[8] = 0;
    isp_ctl[i].unsharp_filter[9] = 0;
    isp_ctl[i].unsharp_filter[10] = 1;
    isp_ctl[i].unsharp_filter[11] = 0;
    isp_ctl[i].unsharp_filter[12] = 0;
    isp_ctl[i].unsharp_filter[13] = 0;
    isp_ctl[i].unsharp_filter[14] = 0;

    isp_ctl[i].dir_far_sharp_weight[0] = 64;
    isp_ctl[i].dir_far_sharp_weight[1] = 64;
    isp_ctl[i].configunited_x[0] = 111;
    isp_ctl[i].configunited_x[1] = 163;
    isp_ctl[i].configunited_x[2] = 194;
    isp_ctl[i].configunited_x[3] = 226;
    isp_ctl[i].configunited_x[4] = 298;
    isp_ctl[i].configunited_x[5] = 479;

    isp_ctl[i].configunited_y[0] = 504;
    isp_ctl[i].configunited_y[1] = 355;
    isp_ctl[i].configunited_y[2] = 301;
    isp_ctl[i].configunited_y[3] = 272;
    isp_ctl[i].configunited_y[4] = 256;
    isp_ctl[i].configunited_y[5] = 234;
    isp_ctl[i].configunitradial_x[0] = 50;
    isp_ctl[i].configunitradial_x[1] = 86;
    isp_ctl[i].configunitradial_x[2] = 142;
    isp_ctl[i].configunitradial_x[3] = 189;
    isp_ctl[i].configunitradial_x[4] = 224;
    isp_ctl[i].configunitradial_x[5] = 255;
    isp_ctl[i].configunitradial_y[0] = 1;
    isp_ctl[i].configunitradial_y[1] = 101;
    isp_ctl[i].configunitradial_y[2] = 162;
    isp_ctl[i].configunitradial_y[3] = 216;
    isp_ctl[i].configunitradial_y[4] = 255;
    isp_ctl[i].configunitradial_y[5] = 255;
    isp_ctl[i].denoise_filter[0] = 64;
    isp_ctl[i].denoise_filter[1] = 32;
    isp_ctl[i].denoise_filter[2] = 16;
    isp_ctl[i].denoise_filter[3] = 0;
    isp_ctl[i].denoise_filter[4] = 0;
    isp_ctl[i].denoise_filter[5] = 0;
    isp_ctl[i].vssnlm_x0 = 48;
    isp_ctl[i].vssnlm_x1 = 89;
    isp_ctl[i].vssnlm_x2 = 130;
    isp_ctl[i].vssnlm_y1 = 1;
    isp_ctl[i].vssnlm_y2 = 2;
    isp_ctl[i].vssnlm_y3 = 8;
  }
  isp_ctl[1].sharpening_power[0] = 127;
  isp_ctl[1].sharpening_power[1] = 38;
  isp_ctl[1].shrpn_nega_lmt_dir = 8192;
  isp_ctl[1].shrpn_posi_lmt_dir = 8192;

  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_isp_control_xnr)
{
  int ret = 0, i = 0, j = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 scene-mode=ull device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = camera_control_isp_ctrl_id_xnr_dss;
  struct camera_control_isp_xnr_dss_t isp_ctl[ISP_CONTROL_TEST_NUM];

  for (i = 0; i < ISP_CONTROL_TEST_NUM; i++) {
    isp_ctl[i].rad_enable = 0;
    isp_ctl[i].bypass = 0;
    isp_ctl[i].bypass_mf_y = 0;
    isp_ctl[i].bypass_mf_c = 0;
    isp_ctl[i].spatial_sigma_mf_y = 2048;
    isp_ctl[i].spatial_sigma_mf_c = 2048;
    isp_ctl[i].noise_white_mf_y_y = 120667;
    isp_ctl[i].noise_white_mf_y_s = 463825;
    isp_ctl[i].noise_white_mf_c_y = 120667;
    isp_ctl[i].noise_white_mf_c_u = 120667;
    isp_ctl[i].noise_white_mf_c_v = 120667;
    isp_ctl[i].rad_noise_power_mf_luma = 0;
    isp_ctl[i].rad_noise_power_mf_chroma = 0;
    isp_ctl[i].range_weight_lut[0] = 895;
    isp_ctl[i].range_weight_lut[1] = 2940;
    isp_ctl[i].range_weight_lut[2] = 4209;
    isp_ctl[i].range_weight_lut[3] = 5729;
    isp_ctl[i].range_weight_lut[4] = 15691;
    isp_ctl[i].range_weight_lut[5] = 3342;
    isp_ctl[i].range_weight_lut[6] = 257;

    isp_ctl[i].one_div_64_lut[0] = 255;
    isp_ctl[i].one_div_64_lut[1] = 255;
    isp_ctl[i].one_div_64_lut[2] = 181;
    isp_ctl[i].one_div_64_lut[3] = 148;
    isp_ctl[i].one_div_64_lut[4] = 128;
    isp_ctl[i].one_div_64_lut[5] = 114;
    isp_ctl[i].one_div_64_lut[6] = 105;
    isp_ctl[i].one_div_64_lut[7] = 97;
    isp_ctl[i].one_div_64_lut[8] = 91;
    isp_ctl[i].one_div_64_lut[9] = 85;
    isp_ctl[i].one_div_64_lut[10] = 81;
    isp_ctl[i].one_div_64_lut[11] = 77;
    isp_ctl[i].one_div_64_lut[12] = 74;
    isp_ctl[i].one_div_64_lut[13] = 71;
    isp_ctl[i].one_div_64_lut[14] = 68;
    isp_ctl[i].one_div_64_lut[15] = 66;
    isp_ctl[i].one_div_64_lut[16] = 64;
    isp_ctl[i].one_div_64_lut[17] = 62;
    isp_ctl[i].one_div_64_lut[18] = 60;
    isp_ctl[i].one_div_64_lut[19] = 59;
    isp_ctl[i].one_div_64_lut[20] = 57;
    isp_ctl[i].one_div_64_lut[21] = 56;
    isp_ctl[i].one_div_64_lut[22] = 55;
    isp_ctl[i].one_div_64_lut[23] = 53;
    isp_ctl[i].one_div_64_lut[24] = 52;
    isp_ctl[i].one_div_64_lut[25] = 51;
    isp_ctl[i].one_div_64_lut[26] = 50;
    isp_ctl[i].one_div_64_lut[27] = 49;
    isp_ctl[i].one_div_64_lut[28] = 48;
    isp_ctl[i].one_div_64_lut[29] = 48;
    isp_ctl[i].one_div_64_lut[30] = 47;
    isp_ctl[i].one_div_64_lut[31] = 46;
    isp_ctl[i].one_div_64_lut[32] = 45;
    isp_ctl[i].one_div_64_lut[33] = 45;
    isp_ctl[i].one_div_64_lut[34] = 44;
    isp_ctl[i].one_div_64_lut[35] = 43;
    isp_ctl[i].one_div_64_lut[36] = 43;
    isp_ctl[i].one_div_64_lut[37] = 42;
    isp_ctl[i].one_div_64_lut[38] = 42;
    isp_ctl[i].one_div_64_lut[39] = 41;
    isp_ctl[i].one_div_64_lut[40] = 40;
    isp_ctl[i].one_div_64_lut[41] = 40;
    isp_ctl[i].one_div_64_lut[42] = 40;
    isp_ctl[i].one_div_64_lut[43] = 39;
    isp_ctl[i].one_div_64_lut[44] = 39;
    isp_ctl[i].one_div_64_lut[45] = 38;
    isp_ctl[i].one_div_64_lut[46] = 38;
    isp_ctl[i].one_div_64_lut[47] = 37;
    isp_ctl[i].one_div_64_lut[48] = 37;
    isp_ctl[i].one_div_64_lut[49] = 37;
    isp_ctl[i].one_div_64_lut[50] = 36;
    isp_ctl[i].one_div_64_lut[51] = 36;
    isp_ctl[i].one_div_64_lut[52] = 36;
    isp_ctl[i].one_div_64_lut[53] = 35;
    isp_ctl[i].one_div_64_lut[54] = 35;
    isp_ctl[i].one_div_64_lut[55] = 35;
    isp_ctl[i].one_div_64_lut[56] = 34;
    isp_ctl[i].one_div_64_lut[57] = 34;
    isp_ctl[i].one_div_64_lut[58] = 34;
    isp_ctl[i].one_div_64_lut[59] = 33;
    isp_ctl[i].one_div_64_lut[60] = 33;
    isp_ctl[i].one_div_64_lut[61] = 33;
    isp_ctl[i].one_div_64_lut[62] = 33;
    isp_ctl[i].one_div_64_lut[63] = 32;

    isp_ctl[i].rad_noise_compensation_mf_chroma = 6553;
    isp_ctl[i].rad_noise_compensation_mf_luma = 6553;
    isp_ctl[i].mf_luma_power = 13107;
    isp_ctl[i].rad_mf_luma_power = 8192;
    isp_ctl[i].mf_chroma_power = 32768;
    isp_ctl[i].rad_mf_chroma_power = 8192;
    isp_ctl[i].noise_black_mf_y_y = 120667;
    isp_ctl[i].noise_black_mf_c_y = 120667;
    isp_ctl[i].noise_black_mf_c_u = 120667;
    isp_ctl[i].noise_black_mf_c_v = 120667;

    isp_ctl[i].xcu_lcs_x[0] = 0;
    isp_ctl[i].xcu_lcs_x[1] = 0;
    isp_ctl[i].xcu_lcs_x[2] = 0;
    isp_ctl[i].xcu_lcs_x[3] = 562;
    isp_ctl[i].xcu_lcs_x[4] = 1586;
    isp_ctl[i].xcu_lcs_x[5] = 2610;
    isp_ctl[i].xcu_lcs_x[6] = 3634;
    isp_ctl[i].xcu_lcs_x[7] = 4658;
    isp_ctl[i].xcu_lcs_x[8] = 5682;
    isp_ctl[i].xcu_lcs_x[9] = 6706;
    isp_ctl[i].xcu_lcs_x[10] = 7730;
    isp_ctl[i].xcu_lcs_x[11] = 8754;
    isp_ctl[i].xcu_lcs_x[12] = 9778;
    isp_ctl[i].xcu_lcs_x[13] = 10802;
    isp_ctl[i].xcu_lcs_x[14] = 11826;
    isp_ctl[i].xcu_lcs_x[15] = 12850;
    isp_ctl[i].xcu_lcs_x[16] = 13874;
    isp_ctl[i].xcu_lcs_x[17] = 14898;
    isp_ctl[i].xcu_lcs_x[18] = 15922;
    isp_ctl[i].xcu_lcs_x[19] = 16946;
    isp_ctl[i].xcu_lcs_x[20] = 17970;
    isp_ctl[i].xcu_lcs_x[21] = 18994;
    isp_ctl[i].xcu_lcs_x[22] = 20018;
    isp_ctl[i].xcu_lcs_x[23] = 21042;
    isp_ctl[i].xcu_lcs_x[24] = 22066;
    isp_ctl[i].xcu_lcs_x[25] = 23090;
    isp_ctl[i].xcu_lcs_x[26] = 24114;
    isp_ctl[i].xcu_lcs_x[22] = 25138;
    isp_ctl[i].xcu_lcs_x[28] = 26162;
    isp_ctl[i].xcu_lcs_x[29] = 27186;
    isp_ctl[i].xcu_lcs_x[30] = 28210;
    isp_ctl[i].xcu_lcs_x[31] = 29234;
    isp_ctl[i].xcu_lcs_x[32] = 30257;

    isp_ctl[i].xcu_lcs_exp = 10;
    isp_ctl[i].xcu_lcs_slp_a_res = 14;

    isp_ctl[i].xcu_lcs_offset[0] = 8867;
    isp_ctl[i].xcu_lcs_offset[1] = 9097;
    isp_ctl[i].xcu_lcs_offset[2] = 9382;
    isp_ctl[i].xcu_lcs_offset[3] = 9777;
    isp_ctl[i].xcu_lcs_offset[4] = 10278;
    isp_ctl[i].xcu_lcs_offset[5] = 10881;
    isp_ctl[i].xcu_lcs_offset[6] = 11579;
    isp_ctl[i].xcu_lcs_offset[7] = 12367;
    isp_ctl[i].xcu_lcs_offset[8] = 13236;
    isp_ctl[i].xcu_lcs_offset[9] = 14178;
    isp_ctl[i].xcu_lcs_offset[10] = 15184;
    isp_ctl[i].xcu_lcs_offset[11] = 16244;
    isp_ctl[i].xcu_lcs_offset[12] = 17348;
    isp_ctl[i].xcu_lcs_offset[13] = 18486;
    isp_ctl[i].xcu_lcs_offset[14] = 19646;
    isp_ctl[i].xcu_lcs_offset[15] = 20817;
    isp_ctl[i].xcu_lcs_offset[16] = 21988;
    isp_ctl[i].xcu_lcs_offset[17] = 23148;
    isp_ctl[i].xcu_lcs_offset[18] = 24286;
    isp_ctl[i].xcu_lcs_offset[19] = 25390;
    isp_ctl[i].xcu_lcs_offset[20] = 26450;
    isp_ctl[i].xcu_lcs_offset[21] = 27456;
    isp_ctl[i].xcu_lcs_offset[22] = 28398;
    isp_ctl[i].xcu_lcs_offset[23] = 29267;
    isp_ctl[i].xcu_lcs_offset[24] = 30055;
    isp_ctl[i].xcu_lcs_offset[25] = 30753;
    isp_ctl[i].xcu_lcs_offset[26] = 31356;
    isp_ctl[i].xcu_lcs_offset[27] = 31857;
    isp_ctl[i].xcu_lcs_offset[28] = 32252;
    isp_ctl[i].xcu_lcs_offset[29] = 32537;
    isp_ctl[i].xcu_lcs_offset[30] = 32709;
    isp_ctl[i].xcu_lcs_offset[31] = 32767;

    isp_ctl[i].xcu_lcs_slope[0] = 0;
    isp_ctl[i].xcu_lcs_slope[1] = 0;
    isp_ctl[i].xcu_lcs_slope[2] = 11515;
    isp_ctl[i].xcu_lcs_slope[3] = 8016;
    isp_ctl[i].xcu_lcs_slope[4] = 9648;
    isp_ctl[i].xcu_lcs_slope[5] = 11168;
    isp_ctl[i].xcu_lcs_slope[6] = 12608;
    isp_ctl[i].xcu_lcs_slope[7] = 13904;
    isp_ctl[i].xcu_lcs_slope[8] = 15072;
    isp_ctl[i].xcu_lcs_slope[9] = 16096;
    isp_ctl[i].xcu_lcs_slope[10] = 16960;
    isp_ctl[i].xcu_lcs_slope[11] = 17664;
    isp_ctl[i].xcu_lcs_slope[12] = 18208;
    isp_ctl[i].xcu_lcs_slope[13] = 18560;
    isp_ctl[i].xcu_lcs_slope[14] = 18736;
    isp_ctl[i].xcu_lcs_slope[15] = 18736;
    isp_ctl[i].xcu_lcs_slope[16] = 18560;
    isp_ctl[i].xcu_lcs_slope[17] = 18208;
    isp_ctl[i].xcu_lcs_slope[18] = 17664;
    isp_ctl[i].xcu_lcs_slope[19] = 16960;
    isp_ctl[i].xcu_lcs_slope[20] = 16096;
    isp_ctl[i].xcu_lcs_slope[21] = 15072;
    isp_ctl[i].xcu_lcs_slope[22] = 13904;
    isp_ctl[i].xcu_lcs_slope[23] = 12608;
    isp_ctl[i].xcu_lcs_slope[24] = 11168;
    isp_ctl[i].xcu_lcs_slope[25] = 9648;
    isp_ctl[i].xcu_lcs_slope[26] = 8016;
    isp_ctl[i].xcu_lcs_slope[27] = 6320;
    isp_ctl[i].xcu_lcs_slope[28] = 4560;
    isp_ctl[i].xcu_lcs_slope[29] = 2752;
    isp_ctl[i].xcu_lcs_slope[30] = 928;
    isp_ctl[i].xcu_lcs_slope[31] = 0;

    isp_ctl[i].lcs_th_for_black = 300;
    isp_ctl[i].lcs_th_for_white = 500;
    isp_ctl[i].rad_lcs_th_for_black = 0;
    isp_ctl[i].rad_lcs_th_for_white = 0;
    isp_ctl[i].blnd_hf_power_y = 32767;
    isp_ctl[i].blnd_hf_power_c = 32767;
  }
  isp_ctl[1].mf_chroma_power = 0;

  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_ltm_tuning_data)
{
  int ret = 0;
  CheckField check;
  clear_plugins();
  char property[256] = "num-buffers=300 scene-mode=hdr device-name=";
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  strncat(property, device_name, strlen(device_name));
  char *sink_name = get_sink_plugin_name();
  const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

  if (strcmp(sink_name, "mfx") == 0) {
      sink = "mfxsink";
      convert = "mfxvpp";
  } else {
      sink = "vaapisink";
      convert = "vaapipostproc";
  }

  memset(&check, 0, sizeof(CheckField));
  check.check_isp_control = true;
  check.field_isp.tag = LTM_TUNING_DATA_TAG;
  ltm_tuning_data isp_ctl[ISP_CONTROL_TEST_NUM];

  for (int i = 0; i < ISP_CONTROL_TEST_NUM; i++) {
      memset(&isp_ctl[i], 0, sizeof(ltm_tuning_data));
      isp_ctl[i].algo_mode = ltm_algo_optibright_gain_map;
      isp_ctl[i].optibright_tuning.GTM_Str = 120;
      isp_ctl[i].optibright_tuning.GF_epspar = 2621;
      isp_ctl[i].optibright_tuning.alpham1 = 32767;
      isp_ctl[i].optibright_tuning.alpham = 21299;
      isp_ctl[i].optibright_tuning.maskmin = 0;
      isp_ctl[i].optibright_tuning.maskmax = 4915;
      isp_ctl[i].optibright_tuning.num_iteration = 16;
      isp_ctl[i].optibright_tuning.maskmid = 4915;
      isp_ctl[i].optibright_tuning.hlc_mode = 0;
      isp_ctl[i].optibright_tuning.max_isp_gain = 32;
      isp_ctl[i].optibright_tuning.convergence_speed = 1229;
      isp_ctl[i].optibright_tuning.lm_treatment = 22938;
      isp_ctl[i].optibright_tuning.GTM_mode = 1;
      isp_ctl[i].optibright_tuning.pre_gamma = 60;
      isp_ctl[i].optibright_tuning.lav2p_scale = 5;
      isp_ctl[i].optibright_tuning.p_max = 9830;
      isp_ctl[i].optibright_tuning.p_mode = 0;
      isp_ctl[i].optibright_tuning.p_value = 9830;
      isp_ctl[i].optibright_tuning.filter_size = 0;
      isp_ctl[i].optibright_tuning.max_percentile = 32604;
      isp_ctl[i].optibright_tuning.ldr_brightness = 10650;
      isp_ctl[i].optibright_tuning.dr_mid = 7022;
      isp_ctl[i].optibright_tuning.dr_norm_max = 7168;
      isp_ctl[i].optibright_tuning.dr_norm_min = 0;
      isp_ctl[i].optibright_tuning.convergence_speed_slow = 8192;
      isp_ctl[i].optibright_tuning.convergence_sigma = 4915;
      isp_ctl[i].optibright_tuning.gainext_mode = 1;
      isp_ctl[i].optibright_tuning.wdr_scale_max = 12288;
      isp_ctl[i].optibright_tuning.wdr_scale_min = 1024;
      isp_ctl[i].optibright_tuning.wdr_gain_max = 16384;
      isp_ctl[i].optibright_tuning.frame_delay_compensation = 1;
      isp_ctl[i].mpgc_tuning.lm_stability = 3277;
      isp_ctl[i].mpgc_tuning.lm_sensitivity = 16;
      isp_ctl[i].mpgc_tuning.blur_size = 1;
      isp_ctl[i].mpgc_tuning.tf_str = 6553;
      isp_ctl[i].drcsw_tuning.blus_sim_sigma = 8192;
  }

  isp_ctl[1].optibright_tuning.wdr_scale_max = 2000;
  isp_ctl[1].optibright_tuning.wdr_scale_min = 1024;
  isp_ctl[1].optibright_tuning.wdr_gain_max = 16;

  check.field_isp.data = (void *) &isp_ctl;

  ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
  ASSERT_EQ(ret, 0);
  ret = add_plugins(convert, NULL, sink_filter);
  ASSERT_EQ(ret, 0);
  ret = add_plugins(sink, NULL, NULL);
  ASSERT_EQ(ret, 0);

  ret = do_pipline(&check);
  ASSERT_EQ(ret, 0);
}
#endif //CHROME_SLIM_CAMHAL
