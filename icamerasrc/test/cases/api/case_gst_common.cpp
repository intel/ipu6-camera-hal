/*
 * GStreamer
 * Copyright (C) 2016-2018 Intel Corporation
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

#include "case_gst_common.h"
#include <gst/video/video.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

struct Plugin g_plugin[MAX_PLUGIN_SIZE];
static GstElement *g_elements[MAX_PLUGIN_SIZE];
static char g_old_pro_attrs[1024];
static char g_new_pro_attrs[1024];

void clear_plugins(void) {

    memset(g_plugin, 0, (MAX_PLUGIN_SIZE * sizeof(struct Plugin)));
    return;
}

int add_plugins(const char *plugin_name, const char *pro_attrs, const char *caps_filter)
{
    int i;
    for (i = 0; i < MAX_PLUGIN_SIZE; i++) {
        if (!g_plugin[i].plugin_name) {
            g_plugin[i].plugin_name = plugin_name;
            g_plugin[i].pro_attrs = pro_attrs;
            g_plugin[i].caps_filter = caps_filter;
            break;
        }
    }
    if (i == MAX_PLUGIN_SIZE) {
        g_print("the plugin queue is full, add failed\n");
        return -1;
    }

    return 0;
}

char *get_input_device_env(void)
{
    const char *Input_device = NULL;
    Input_device = getenv("cameraInput");
    if (Input_device == NULL) {
      g_print("ERR: cameraInput is not set,unable to specify property:'device-name'!\n");
      Input_device="";
    }
    return (char *)Input_device;
}

char *get_sink_plugin_name(void)
{
    const char *sink_name = NULL;
    sink_name = getenv("sinkPluginName");
    if (sink_name == NULL) {
      g_print("sinkPluginName is not set, use the default plugin: vaapi!\n");
      sink_name = "vaapi";
    }
    return (char *)sink_name;
}

int get_dump_flag(void)
{
  int flag = 0;
  char *dumpflag = getenv("camerasrcDump");
  if (dumpflag)
    flag = std::stoi(dumpflag);

  g_print("Current dump flag is: %d\n", flag);
  return flag;
}

void copy_field(CheckField *check, CustomData &data)
{
  data.cam3a_field.update_img_enhancement = check->field_3a.update_img_enhancement;
  data.cam3a_field.update_exp_time = check->field_3a.update_exp_time;
}

int get_enum_value_from_field(GParamSpec * prop, const char *nick, PropertyRange *range)
{
    int match_idx = -1, num_of_values = 0;
    GEnumValue *values;

    values = G_ENUM_CLASS (g_type_class_ref (prop->value_type))->values;
    while (values[num_of_values].value_nick) {
        if (strcmp(values[num_of_values].value_nick, nick) == 0) {
          match_idx = num_of_values;
        }
        num_of_values++;
    }

    if (match_idx < 0) {
      g_print("doesn't find the enum nick: %s", nick);
      return -1;
    }

    if (range)
      range->enum_str = values[(match_idx+1) % num_of_values].value_nick;

    return values[match_idx].value;
}

static void get_value_range_from_property(char *property, PropertyRange *range)
{
    /* range.min and range.max are inited 0 by default*/
    if (strcmp(property, "sharpness") == 0 || //properties with range 0 - 100
          strcmp(property, "brightness") == 0 ||
          strcmp(property, "contrast") == 0 ||
          strcmp(property, "hue") == 0 ||
          strcmp(property, "saturation") == 0 ||
          strcmp(property, "iris-level") == 0 ||
          strcmp(property, "exposure-time") == 0 ||
          strcmp(property, "nr-filter-level") == 0) {
      range->min = 0;
      range->max = 100;
    } else if (strcmp(property, "gain") == 0) { //property with range 0 - 60
      range->min = 0;
      range->max= 60;
    } else if (strcmp(property, "wdr-level") == 0) { //property with range 0 - 15
      range->min = 0;
      range->max = 15;
    } else if (strcmp(property, "R-gain") == 0 || //properties with range 0 - 255
               strcmp(property, "G-gain") == 0 ||
               strcmp(property, "B-gain") == 0) {
       range->min = 0;
       range->max = 255;
    }

}

void set_elements_properties(GstElement *element, const char * properties)
{
    char *token;
    char pro_str[1024];
    GObjectClass *oclass;
    GParamSpec *prop;

    strncpy(pro_str, properties, 1024);
    token = strtok( (char*)pro_str, ", ");
    while (token != NULL) {
        char *ptr = strstr(token, "=");
        char *str_value = ptr + 1;
        char *property = token;
        *ptr = '\0';

        oclass = G_OBJECT_GET_CLASS (element);
        prop = g_object_class_find_property (oclass, property);
        if (!prop) {
            g_print("there isn't the property: %s in element\n", property);
            token = strtok(NULL, ", ");
            continue;
        }
        switch (prop->value_type) {
            gint value_int, value_enum;
            gboolean value_bool;
            case G_TYPE_INT:
                value_int = atoi(str_value);
                g_object_set(G_OBJECT(element), property, value_int, NULL);
                break;
            case G_TYPE_BOOLEAN:
                value_bool = ((strcmp(str_value, "true") == 0) ? true : false);
                g_object_set(G_OBJECT(element), property, value_bool, NULL);
                break;
            case G_TYPE_STRING:
                g_object_set(G_OBJECT(element), property, str_value, NULL);
                break;
            default:
                if (G_TYPE_IS_ENUM(prop->value_type)) {
                    value_enum = get_enum_value_from_field(prop, str_value, NULL);
                    if (value_enum == -1) {
                      break;
                    }
                    g_object_set(G_OBJECT(element), property, value_enum, NULL);
                } else
                    g_print("ERR, this type isn't supported\n");
                break;
        }

        token = strtok(NULL, ", ");
    }
}

int update_elements_properties(GstElement *element, const char * properties)
{
    GObjectClass *oclass;
    GParamSpec *prop;

    int ret = 0;
    char *token;
    char pro_str[1024];
    char *str_value_get = NULL;
    gint value_int, value_int_get;
    gint value_enum, value_enum_get;
    gboolean value_bool, value_bool_get;

    PropertyRange range;
    memset(&range, 0, sizeof(PropertyRange));
    range.min = 0;
    range.max = 0;
    range.enum_str = "";

    strncpy(pro_str, properties, 1024);
    token = strtok( (char*)pro_str, ", ");
    memset(g_new_pro_attrs, 0, sizeof(g_new_pro_attrs));
    memset(g_old_pro_attrs, 0, sizeof(g_old_pro_attrs));
    while (token != NULL) {
        char *ptr = strstr(token, "=");
        char *str_value = ptr + 1;
        char *property = token;
        *ptr = '\0';
        char new_token[1024] = "";

         //we don't update below properties because they are fixed.
        if (strcmp(property, "device-name") == 0 ||
            strcmp(property, "num-buffers") == 0) {
            token = strtok(NULL, ", ");
            continue;
        }

        oclass = G_OBJECT_GET_CLASS (element);
        prop = g_object_class_find_property (oclass, property);
        if (!prop) {
            g_print("there isn't the property: %s in element\n", property);
            token = strtok(NULL, ", ");
            continue;
        }

        switch (prop->value_type) {
            case G_TYPE_INT:
              value_int = atoi(str_value);
              g_object_get(G_OBJECT(element), property, &value_int_get, NULL);
              if (value_int != value_int_get) {
                g_print("Incorrect value: %d of property: %s, should be:%d\n",value_int_get, property, value_int);
                return -1;
              } else {
                g_print("Current value: %d of property: %s match with set value: %d successfully\n",value_int_get, property, value_int);
                get_value_range_from_property(property, &range);
                if (value_int >= range.min&& value_int < range.max)
                  value_int++;
                else
                  value_int = 0;
                sprintf(new_token, "%s=%d ", property, value_int);
              }
              break;
            default:
              if (G_TYPE_IS_ENUM(prop->value_type)) {
                value_enum = get_enum_value_from_field(prop, str_value, NULL);
                if (value_enum == -1) {
                  return -1;
                }
                g_object_get(G_OBJECT(element), property, &value_enum_get, NULL);
                if (value_enum != value_enum_get) {
                  g_print("Incorrect value: %s of property: %s\n",str_value, property);
                  return -1;
                } else {
                  g_print("Current enum value: %s of property: %s match with set value successfully\n",str_value, property);
                  ret = get_enum_value_from_field(prop, str_value, &range);
                  if (ret == -1) {
                    break;
                  }
                  sprintf(new_token, "%s=%s ", property, range.enum_str);
                }
              } else
                g_print("ERR, this type isn't supported\n");
                break;
         }
         token = strtok(NULL, ", ");
    }
    strncpy(g_old_pro_attrs, g_new_pro_attrs, 1024);

  return ret;
}

//calculate current fps
static GstPadProbeReturn callback_have_data(GstPad *padsrc, GstPadProbeInfo *info, gpointer user_data)
{
    double duration;
    float current_fps = 1.0;
    struct timeval current_time;
    CustomData * data = (CustomData *)user_data;

    data->buffer_num_fps++;
    printf("----Buffer num for fps test=%d----\n",data->buffer_num_fps);
    gettimeofday(&current_time, NULL);
    duration = (double)((current_time.tv_sec - data->last_time.tv_sec) * 1000000 +
            (current_time.tv_usec - data->last_time.tv_usec));
    current_fps = (float)(1000000 / duration);

    if (data->buffer_num_fps == FPS_COUNT_START)
        gettimeofday(&(data->start_time), NULL);
    gettimeofday(&(data->last_time), NULL);

    return GST_PAD_PROBE_PASS;
}

//check and change current property values
static GstPadProbeReturn callback_check_property(GstPad *padsrc, GstPadProbeInfo *info, gpointer user_data)
{
  int ret = 0;
  CustomData * data = (CustomData *)user_data;
  data->buffer_num_prop++;
  printf("----Buffer num for property test=%d----\n",data->buffer_num_prop);

  ret = update_elements_properties(g_elements[0], g_old_pro_attrs);
  if (ret == -1) {
      data->update_error = true;
      return GST_PAD_PROBE_REMOVE;
  }
  set_elements_properties(g_elements[0], g_new_pro_attrs);

  return GST_PAD_PROBE_PASS;
}

#ifndef CHROME_SLIM_CAMHAL
void print_tnr5_21_t(struct camera_control_isp_tnr5_21_t *data)
{

  g_print("bypass %d\n",data->bypass);
  g_print("nm_yy_xcu_b:");
  for(int j = 0;j < 64; j++) {
    g_print("%d\t",data->nm_yy_xcu_b[j]);
  }
  g_print("\n nm_yc_xcu_b:");
  for(int j = 0;j < 64; j++) {
    g_print("%d\t",data->nm_yc_xcu_b[j]);
  }
  g_print("\n nm_cy_xcu_b:");
  for(int j = 0;j < 64; j++) {
    g_print("%d\t",data->nm_cy_xcu_b[j]);
  }
  g_print("\n nm_cc_xcu_b:");
  for(int j = 0;j < 64; j++) {
    g_print("%d\t",data->nm_cc_xcu_b[j]);
  }
  g_print("\n");
  g_print("nm_y_log_est_min_b: %d\n",    data->nm_y_log_est_min_b);
  g_print("nm_y_log_est_max_b: %d\n",    data->nm_y_log_est_max_b);
  g_print("nm_c_log_est_min_b: %d\n",    data->nm_c_log_est_min_b);
  g_print("nm_c_log_est_max_b: %d\n",    data->nm_c_log_est_max_b);
  g_print("nm_Y_alpha_b      %d\n",    data->nm_Y_alpha_b      );
  g_print("nm_C_alpha_b      %d\n",    data->nm_C_alpha_b      );
  g_print("Tnr_Strength_0    %d\n",    data->Tnr_Strength_0    );
  g_print("Tnr_Strength_1    %d\n",    data->Tnr_Strength_1    );
  g_print("SpNR_Static       %d\n",    data->SpNR_Static       );
  g_print("SpNR_Dynamic      %d\n",    data->SpNR_Dynamic      );
  g_print("Radial_Gain       %d\n",    data->Radial_Gain       );
  g_print("SAD_Gain          %d\n",    data->SAD_Gain          );
  g_print("Pre_Sim_Gain      %d\n",    data->Pre_Sim_Gain      );
  g_print("Weight_In         %d\n",    data->Weight_In         );
  g_print("g_mv_x            %d\n",    data->g_mv_x            );
  g_print("g_mv_y            %d\n",    data->g_mv_y            );
  g_print("tbd_sim_gain      %d\n",    data->tbd_sim_gain      );
  g_print("NS_Gain           %d\n",    data->NS_Gain           );
  g_print("nsw_gain          %d\n",    data->nsw_gain          );
  g_print("nsw_sigma         %d\n",    data->nsw_sigma         );
  g_print("nsw_bias          %d\n",    data->nsw_bias          );
  g_print("ns_clw_bias0      %d\n",    data->ns_clw_bias0      );
  g_print("ns_clw_bias1      %d\n",    data->ns_clw_bias1      );
  g_print("ns_clw_sigma      %d\n",    data->ns_clw_sigma      );
  g_print("ns_clw_center     %d\n",    data->ns_clw_center     );
  g_print("ns_norm_bias      %d\n",    data->ns_norm_bias      );
  g_print("ns_norm_coef      %d\n",    data->ns_norm_coef      );
  g_print("bypass_g_mv       %d\n",    data->bypass_g_mv       );
  g_print("bypass_NS         %d\n",    data->bypass_NS         );

}

void print_bnlm_t(struct camera_control_isp_bnlm_t *data)
{

  g_print("bypass            %d\n",data->bypass          );
  g_print("detailix_radgain  %d\n",data->detailix_radgain);
  for(int j = 0; j < 2; j++) {
    g_print("data->detailix_x_range[%d]%d\n",j,data->detailix_x_range[j]);
    g_print("data->sad_mu_x_range[%d]  %d\n",j,data->sad_mu_x_range[j]  );
  }
  g_print("sad_radgain  %d\n",data->sad_radgain);
  for(int j = 0; j < 3; j++) {
    g_print("detailix_coeffs[%d]              %d\n",j,data->detailix_coeffs[j]);
    g_print("sad_mu_coeffs[%d]                %d\n",j,data->sad_mu_coeffs[j]  );
    g_print("detailth[%d]                     %d\n",j,data->detailth[j]       );
  }
  for(int j = 0; j < 4; j++) {
    g_print("data->sad_spatialrad[%d]       %d\n",j,data->sad_spatialrad[j]   );
    g_print("data->sad_detailixlutx[%d]     %d\n",j,data->sad_detailixlutx[j] );
    g_print("data->sad_detailixluty[%d]     %d\n",j,data->sad_detailixluty[j] );
    g_print("data->numcandforavg[%d]        %d\n",j,data->numcandforavg[j]    );
    g_print("data->blend_power[%d]          %d\n",j,data->blend_power[j]      );
    g_print("data->blend_th[%d]             %d\n",j,data->blend_th[j]         );
    g_print("data->blend_texturegain[%d]    %d\n",j,data->blend_texturegain[j]);
    g_print("data->matchqualitycands[%d]    %d\n",j,data->matchqualitycands[j]);
  }
  g_print("data->blend_radgain   %d\n",data->blend_radgain );
  g_print("data->wsumminth       %d\n",data->wsumminth     );
  g_print("data->wmaxminth       %d\n",data->wmaxminth     );
  g_print("data->rad_enable      %d\n",data->rad_enable    );
}
#endif //CHROME_SLIM_CAMHAL

int get_random_value(int min,int max)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  srand((unsigned)tv.tv_usec);
  return (rand()%(max+1 - min)) + min;
}

#ifndef CHROME_SLIM_CAMHAL
void print_sc_iefd_t(struct camera_control_isp_sc_iefd_t *data)
{
  for(int j = 0; j < 2; j++) {
    g_print("sharpening_power[%d]     %d\t" ,j,data->sharpening_power[j]);
  }
  g_print("\n");
  for(int j = 0; j < 2; j++) {
    g_print("cu_ed2_metric_th[%d]     %d\t" ,j,data->cu_ed2_metric_th[j]);
  }

  g_print("\n");
  for(int j = 0; j < 2; j++) {
    g_print("cu_dir_metric_th[%d]     %d\t" ,j,data->cu_dir_metric_th[j]);
  }

  g_print("\n");
  for(int j = 0; j < 2; j++) {
    g_print("cu_nr_power[%d]          %d\t" ,j,data->cu_nr_power[j]);
  }
  g_print("\n");
  for(int j = 0; j < 2; j++) {
    g_print("dir_far_sharp_weight[%d] %d\t" ,j,data->dir_far_sharp_weight[j]);
  }
  g_print("\n");
  g_print("sharp_power_edge     %d\n",data->sharp_power_edge     );
  g_print("sharp_power_detail   %d\n",data->sharp_power_detail   );
  g_print("unsharp_weight_edge  %d\n",data->unsharp_weight_edge  );
  g_print("unsharp_weight_detail%d\n",data->unsharp_weight_detail);
  g_print("denoise_power        %d\n",data->denoise_power        );
  g_print("radial_denoise_power %d\n",data->radial_denoise_power );
  g_print("shrpn_nega_lmt_txt   %d\n",data->shrpn_nega_lmt_txt   );
  g_print("shrpn_posi_lmt_txt   %d\n",data->shrpn_posi_lmt_txt   );
  g_print("shrpn_nega_lmt_dir   %d\n",data->shrpn_nega_lmt_dir   );
  g_print("shrpn_posi_lmt_dir   %d\n",data->shrpn_posi_lmt_dir   );
  for(int j = 0; j < 4; j++) {
    g_print("cu_var_metric_th[j] %d\t",data->cu_var_metric_th[j]);
  }
  g_print("\n");
  g_print("data->rad_enable%d\n", data->rad_enable);
  g_print("unsharp_filter[15]:");
  for(int j = 0; j < 15; j++) {
    g_print("%d\t",data->unsharp_filter[j]);
  }
  g_print("\n configunited_x:");
  for(int j = 0; j < 6; j++) {
    g_print("%d\t",data->configunited_x[j]);
  }
  g_print("\n denoise_filter:");
  for(int j = 0; j < 6; j++) {
    g_print("%d\t",data->denoise_filter[j]);
  }
  g_print("\n configunitradial_y:");
  for(int j = 0; j < 6; j++) {
    g_print("%d\t"  ,data->configunitradial_y[j]);
  }
  g_print("\n configunitradial_x:");
  for(int j = 0; j < 6; j++) {
    g_print("%d\t"  ,data->configunitradial_x[j]);
  }
  g_print("\n configunited_y:");
  for(int j = 0; j < 6; j++) {
    g_print("%d\t",data->configunited_y[j]);
  }
  g_print("\n");
  g_print("vssnlm_x0%d\n"  ,data->vssnlm_x0);
  g_print("vssnlm_x1%d\n"  ,data->vssnlm_x1);
  g_print("vssnlm_x2%d\n"  ,data->vssnlm_x2);
  g_print("vssnlm_y1%d\n"  ,data->vssnlm_y1);
  g_print("vssnlm_y2%d\n"  ,data->vssnlm_y2);
  g_print("vssnlm_y3%d\n"  ,data->vssnlm_y3);
}

void print_xnr_dss_t(struct camera_control_isp_xnr_dss_t *data)
{
  g_print("rad_enable                  %d\n",data->rad_enable               );
  g_print("bypass                      %d\n",data->bypass                   );
  g_print("bypass_mf_y                 %d\n",data->bypass_mf_y              );
  g_print("bypass_mf_c                 %d\n",data->bypass_mf_c              );
  g_print("spatial_sigma_mf_y          %d\n",data->spatial_sigma_mf_y       );
  g_print("spatial_sigma_mf_c          %d\n",data->spatial_sigma_mf_c       );
  g_print("noise_white_mf_y_y          %d\n",data->noise_white_mf_y_y       );
  g_print("noise_white_mf_y_s          %d\n",data->noise_white_mf_y_s       );
  g_print("noise_white_mf_c_y          %d\n",data->noise_white_mf_c_y       );
  g_print("noise_white_mf_c_u          %d\n",data->noise_white_mf_c_u       );
  g_print("noise_white_mf_c_v          %d\n",data->noise_white_mf_c_v       );
  g_print("rad_noise_power_mf_luma     %d\n",data->rad_noise_power_mf_luma  );
  g_print("rad_noise_power_mf_chroma   %d\n",data->rad_noise_power_mf_chroma);
    g_print("range_weight_lut:");
    for(int j = 0; j < 7; j++)
    {
        g_print("%d\t",data->range_weight_lut[j]);
    }
    g_print("\n one_div_64_lut:");
    for(int j = 0; j < 64; j++)
    {
        g_print("%d\t",data->one_div_64_lut[j]);
    }
    g_print("\n");
    g_print("rad_noise_compensation_mf_chroma   %d\n",data->rad_noise_compensation_mf_chroma);
    g_print("rad_noise_compensation_mf_luma     %d\n",data->rad_noise_compensation_mf_luma  );
    g_print("mf_luma_power                      %d\n",data->mf_luma_power                   );
    g_print("rad_mf_luma_power                  %d\n",data->rad_mf_luma_power               );
    g_print("mf_chroma_power                    %d\n",data->mf_chroma_power                 );
    g_print("rad_mf_chroma_power                %d\n",data->rad_mf_chroma_power             );
    g_print("noise_black_mf_y_y                 %d\n",data->noise_black_mf_y_y              );
    g_print("noise_black_mf_c_y                 %d\n",data->noise_black_mf_c_y              );
    g_print("noise_black_mf_c_u                 %d\n",data->noise_black_mf_c_u              );
    g_print("noise_black_mf_c_v                 %d\n",data->noise_black_mf_c_v              );
    g_print("xcu_lcs_x:");
    for(int j = 0; j < 33; j++)
    {
        g_print("%d\t",data->xcu_lcs_x[j]);
    }
    g_print("\n");
    g_print("xcu_lcs_exp         %d\n",data->xcu_lcs_exp      );
    g_print("xcu_lcs_slp_a_res   %d\n",data->xcu_lcs_slp_a_res);
    g_print("xcu_lcs_offset:");
    for(int j = 0; j < 32; j++)
    {
        g_print("%d\t",data->xcu_lcs_offset[j]);
    }
    g_print("\n xcu_lcs_slope:");
    for(int j = 0; j < 32; j++)
    {
        g_print("%d\t",data->xcu_lcs_slope[j] );
    }
    g_print("\n");
    g_print("lcs_th_for_black       %d\n",data->lcs_th_for_black    );
    g_print("lcs_th_for_white       %d\n",data->lcs_th_for_white    );
    g_print("rad_lcs_th_for_black   %d\n",data->rad_lcs_th_for_black);
    g_print("rad_lcs_th_for_white   %d\n",data->rad_lcs_th_for_white);
    g_print("blnd_hf_power_y        %d\n",data->blnd_hf_power_y     );
    g_print("blnd_hf_power_c        %d\n",data->blnd_hf_power_c     );
}
#endif //CHROME_SLIM_CAMHAL

static GstPadProbeReturn callback_check_dewarping_interface_dewarping_mode_switch(GstPad *padsrc, GstPadProbeInfo *info, gpointer user_data)
{
  CustomData *data = (CustomData *)user_data;
  //switch setting every 50 frames
  if((data->buffer_num_frame++ % 50) != 0)
    return GST_PAD_PROBE_PASS;

    static camera_fisheye_dewarping_mode_t mode = FISHEYE_DEWARPING_REARVIEW;
    if (mode == FISHEYE_DEWARPING_REARVIEW)
        mode = FISHEYE_DEWARPING_HITCHVIEW;
    else if(mode == FISHEYE_DEWARPING_HITCHVIEW)
        mode = FISHEYE_DEWARPING_REARVIEW;

    data->camdewarping_iface->set_dewarping_mode(data->camdewarping, mode);

  return GST_PAD_PROBE_PASS;
}

#ifndef CHROME_SLIM_CAMHAL
// Call isp control interface
static GstPadProbeReturn callback_check_isp_interface(GstPad *padsrc, GstPadProbeInfo *info, gpointer user_data)
{
  CustomData *data = (CustomData *)user_data;
  //switch setting every ten frames
  if((data->buffer_num_isp++ % 150) != 0)
    return GST_PAD_PROBE_PASS;

  data->isp_control_index++;
  data->isp_control_index = data->isp_control_index % ISP_CONTROL_TEST_NUM;

  //check camera_control_isp_ctrl_id_wb_gains
  if (data->camisp_control.tag == camera_control_isp_ctrl_id_wb_gains) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_wb_gains_t);
    struct camera_control_isp_wb_gains_t *gainPtr =
        (struct camera_control_isp_wb_gains_t *)((char *)data->camisp_control.data + offset);
    g_message("%s, set the wb gain data: (%f, %f, %f, %f)",
        __func__, gainPtr->gr, gainPtr->r, gainPtr->b, gainPtr->gb);

    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)gainPtr);
    data->camisp_iface->apply_isp_control(data->camisp);

    struct camera_control_isp_wb_gains_t wbGain;
    memset(&wbGain, 0, sizeof(struct camera_control_isp_wb_gains_t));
    data->camisp_iface->get_isp_control(data->camisp, camera_control_isp_ctrl_id_wb_gains, &wbGain);
    if (wbGain.gr != gainPtr->gr ||
        wbGain.r != gainPtr->r ||
        wbGain.b != gainPtr->b ||
        wbGain.gb != gainPtr->gb) {
      g_printerr("ERROR: Test isp control failed for wg gain apis");
      return GST_PAD_PROBE_REMOVE;
    }

  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_color_correction_matrix) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_color_correction_matrix_t);
    struct camera_control_isp_color_correction_matrix_t *testPtr =
        (struct camera_control_isp_color_correction_matrix_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_wb_gains, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);

    struct camera_control_isp_color_correction_matrix_t ccm;
    memset(&ccm, 0, sizeof(struct camera_control_isp_color_correction_matrix_t));
    data->camisp_iface->get_isp_control(data->camisp, camera_control_isp_ctrl_id_color_correction_matrix, &ccm);
    if (ccm.ccm_gains[2] != testPtr->ccm_gains[2] ||
        ccm.ccm_gains[4] != testPtr->ccm_gains[4] ||
        ccm.ccm_gains[6] != testPtr->ccm_gains[6]) {
      g_printerr("ERROR: Test isp control failed for ccm apis");
      return GST_PAD_PROBE_REMOVE;
    }

  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_bxt_demosaic) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_bxt_demosaic_t);
    struct camera_control_isp_bxt_demosaic_t *testPtr =
        (struct camera_control_isp_bxt_demosaic_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_color_correction_matrix, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);
  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_see) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_see_t);
    struct camera_control_isp_see_t *testPtr =
        (struct camera_control_isp_see_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_bxt_demosaic, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);
  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_gamma_tone_map) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_gamma_tone_map_t);
    struct camera_control_isp_gamma_tone_map_t *testPtr =
        (struct camera_control_isp_gamma_tone_map_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_bxt_demosaic, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);
  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_tnr5_21) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_tnr5_21_t);
    struct camera_control_isp_tnr5_21_t *testPtr =
        (struct camera_control_isp_tnr5_21_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_gamma_tone_map, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);
    print_tnr5_21_t(testPtr);
  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_bnlm) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_bnlm_t);
    struct camera_control_isp_bnlm_t *testPtr =
        (struct camera_control_isp_bnlm_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_tnr5_21, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);
    print_bnlm_t(testPtr);
  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_sc_iefd) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_sc_iefd_t);
    struct camera_control_isp_sc_iefd_t *testPtr =
        (struct camera_control_isp_sc_iefd_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_bnlm, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);
    print_sc_iefd_t(testPtr);
  } else if (data->camisp_control.tag == camera_control_isp_ctrl_id_xnr_dss) {
    gint offset = data->isp_control_index * sizeof(struct camera_control_isp_xnr_dss_t);
    struct camera_control_isp_xnr_dss_t *testPtr =
        (struct camera_control_isp_xnr_dss_t *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_isp_control(data->camisp, camera_control_isp_ctrl_id_bnlm, NULL);
    data->camisp_iface->set_isp_control(data->camisp, data->camisp_control.tag, (void *)testPtr);
    data->camisp_iface->apply_isp_control(data->camisp);
    print_xnr_dss_t(testPtr);
  } else if (data->camisp_control.tag == LTM_TUNING_DATA_TAG) {
    gint offset = data->isp_control_index * sizeof(ltm_tuning_data);
    ltm_tuning_data *testPtr = (ltm_tuning_data *)((char *)data->camisp_control.data + offset);

    data->camisp_iface->set_ltm_tuning_data(data->camisp, (void *)testPtr);
  }

  return GST_PAD_PROBE_PASS;
}
#endif //CHROME_SLIM_CAMHAL

//call 3a interface
static GstPadProbeReturn callback_check_3a_interface(GstPad *padsrc, GstPadProbeInfo *info, gpointer user_data)
{
  int ret = 0;
  CustomData *data = (CustomData *)user_data;
  data->buffer_num_3a++;

  /* call image enhancement interface */
  if (data->cam3a_field.update_img_enhancement) {
    camera_image_enhancement_t img_enhancement;
    memset(&img_enhancement, 0, sizeof(camera_image_enhancement_t));
    img_enhancement = data->cam3a_iface->get_image_enhancement(data->cam3a, img_enhancement);

    /* keep enhancement values in range */
    if ((img_enhancement.sharpness += 20) > 255)
      img_enhancement.sharpness = 0;
    if ((img_enhancement.brightness += 20) > 255)
      img_enhancement.brightness = 0;
    data->cam3a_iface->set_image_enhancement(data->cam3a, img_enhancement);
  }

  /* call exposure time interface */
  if (data->cam3a_field.update_exp_time) {
    if ((data->cam3a_data.exp_time += 500) > 33333)
      data->cam3a_data.exp_time = 0;
    data->cam3a_iface->set_exposure_time(data->cam3a, data->cam3a_data.exp_time);
  }

  /* call iris mode interface */
  if (data->cam3a_field.update_iris_mode) {
    switch(data->buffer_num_3a % 3) {
      case 0:
        data->cam3a_data.irismode = IRIS_MODE_AUTO;
        break;
      case 1:
        data->cam3a_data.irismode = IRIS_MODE_MANUAL;
        break;
      case 2:
        data->cam3a_data.irismode = IRIS_MODE_CUSTOMIZED;
        break;
    }
    data->cam3a_iface->set_iris_mode(data->cam3a, data->cam3a_data.irismode);
  }

  return GST_PAD_PROBE_PASS;
}

static gboolean bus_call(GstBus *bus,GstMessage *msg, gpointer user_data)
{
    CustomData *data = (CustomData *)user_data;

    switch (GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            g_print("End of stream\n");
            g_main_loop_quit(data->main_loop);
            if (data->test_fps && (data->buffer_num_fps > FPS_COUNT_START)) {
                //calculate average fps
                double div = (double)((data->last_time.tv_sec - data->start_time.tv_sec) * 1000000 +
                                                    (data->last_time.tv_usec - data->start_time.tv_usec));
                data->fps = (float) ((data->buffer_num_fps - FPS_COUNT_START)/(div/1000000));
            }
            break;
        case GST_MESSAGE_ERROR:
            {
                gchar *debug;
                GError *error;
                gst_message_parse_error(msg,&error,&debug);
                g_free(debug);
                g_printerr("ERROR:%s\n",error->message);
                g_error_free(error);
                g_main_loop_quit(data->main_loop);
                data->bus_error = true;
                break;
            }
        default:
            break;
    }
    return TRUE;

}

void init_custom_data(CustomData &data)
{
    data.fps = 1.0;
    data.update_error = false;
    data.buffer_num_fps = 0;
    data.buffer_num_prop = 0;
    data.buffer_num_3a = 0;
    data.buffer_num_isp = 0;
    data.buffer_num_frame = 0;
    data.isp_control_index = -1;
    data.test_fps = false;
    data.test_property = false;
    data.test_3a_iface = false;
    data.bus_error = false;
    data.test_isp_control = false;
    gettimeofday(&(data.start_time), NULL);
    gettimeofday(&(data.last_time), NULL);
}

int do_pipline(CheckField *check)
{
    int i, j;
    GstBus *bus;
    CustomData data;
    int plugin_num = 0;
    GstElement *pipeline;
    GstCaps *caps[MAX_PLUGIN_SIZE];

    //clear data.
    for (i = 0; i < MAX_PLUGIN_SIZE; i++) {
        caps[i] = NULL;
        g_elements[i] = NULL;
    }

    //set environment
    if (putenv((char *)("DISPLAY=:0")) != 0) {
        g_print("Failed to set display env\n");
        return -1;
    }
    //gst init
    gst_init(NULL, NULL);

    init_custom_data(data);
    //initialize the CustomData.
    data.main_loop = g_main_loop_new(NULL,FALSE);
    if (!data.main_loop) {
        g_print("Failed to get new main loop\n");
        return -1;
    }

    if (check && check->check_fps)
        data.test_fps = true;
    if (check && check->check_property)
        data.test_property = true;
    if (check && check->check_3a_iface) {
      data.test_3a_iface = true;
      copy_field(check, data);
    }
    if (check && check->check_isp_control) {
        data.test_isp_control = true;
        data.camisp_control = check->field_isp;
    }

    if (check && check->check_dewarping_mode_switch) {
        data.test_dewarping_mode_switch = true;
    }

#ifndef CHROME_SLIM_CAMHAL
    if (check && check->check_wfov) {
        data.test_wfov = true;
        data.set_projection = check->set_projection;
        data.set_rotation = check->set_rotation;
        data.set_fine_adjustments = check->set_fine_adjustments;
        data.rotation = check->rotation;
        data.projection = check->projection;
        data.fine_adjustments = check->fine_adjustments;
    }
#endif //CHROME_SLIM_CAMHAL

    //create pipeline, elements, and set the property
    pipeline = gst_pipeline_new ("pipeline");
    if (!pipeline) {
        g_print("Failed to ctreat pipeline\n");
        return -1;
    }
    for (i = 0; g_plugin[i].plugin_name != NULL; i++) {
        char element_name[20];
        snprintf(element_name, 20, "element-%d", i);
        g_elements[i] = gst_element_factory_make(g_plugin[i].plugin_name, element_name);
        if (!g_elements[i]) {
            gst_object_unref(GST_OBJECT(pipeline));
            g_print("Failed to create source element\n");
            return -1;
        }
        /* Init interface handle for icamerasrc plugin */
        if (strcmp(g_plugin[i].plugin_name, "icamerasrc") == 0) {
            if (check->check_3a_iface) {
                data.cam3a = GST_CAMERASRC_3A(g_elements[i]);
                data.cam3a_iface = GST_CAMERASRC_3A_GET_INTERFACE(data.cam3a);
            }
            if (check->check_isp_control) {
                data.camisp = GST_CAMERASRC_ISP(g_elements[i]);
                data.camisp_iface = GST_CAMERASRC_ISP_GET_INTERFACE(data.camisp);
            }
            if (check->check_dewarping_mode_switch) {
                data.camdewarping = GST_CAMERASRC_DEWARPING(g_elements[i]);
                data.camdewarping_iface = GST_CAMERASRC_DEWARPING_GET_INTERFACE(data.camdewarping);
            }
            
#ifndef CHROME_SLIM_CAMHAL
            if (check->check_wfov) {
                data.camwfov = GST_CAMERASRC_WFOV(g_elements[i]);
                data.camwfov_iface = GST_CAMERASRC_WFOV_GET_INTERFACE(data.camwfov);
            }
#endif //CHROME_SLIM_CAMHAL
        }

        if (g_plugin[i].pro_attrs != NULL)
            set_elements_properties(g_elements[i], g_plugin[i].pro_attrs);
        if (g_plugin[i].caps_filter != NULL)
            caps[i] = gst_caps_from_string(g_plugin[i].caps_filter);

        gst_bin_add_many(GST_BIN(pipeline), g_elements[i], NULL);
        if (i > 0 && g_elements[i - 1]) {
            if (caps[i - 1] != NULL) {
                if(!gst_element_link_filtered(g_elements[i - 1], g_elements[i], caps[i - 1])) {
                    gst_object_unref(GST_OBJECT(pipeline));
                    g_print("Failed to link element with caps_filter:%s\n", g_plugin[i - 1].caps_filter);
                    return -1;
                }
                gst_caps_unref(caps[i - 1]);
            } else {
                if (!gst_element_link_many(g_elements[i - 1], g_elements[i], NULL)) {
                    gst_object_unref(GST_OBJECT(pipeline));
                    g_print("Failed to link many elements\n");
                    return -1;
                }
            }
        }
        plugin_num++;
    }
    g_print("plugin_num: %d\n", plugin_num);

    if (check) {
        GstPad * pad;
        pad = gst_element_get_static_pad(g_elements[plugin_num - 1], "sink");
        if (!pad) {
            gst_object_unref(GST_OBJECT(pipeline));
            g_print("Failed to get the pad\n");
            return -1;
        }
        //add a probe to test fps
        if (data.test_fps) {
          gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                  callback_have_data, &data, NULL);
        }
        //add a probe to update & check property
        if (data.test_property) {
          strncpy(g_old_pro_attrs, g_plugin[0].pro_attrs, 1024);
          gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                   callback_check_property, &data, NULL);
        }

        //add a probe to update and call 3a interface
        if(data.test_3a_iface) {
          gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                   callback_check_3a_interface, &data, NULL);
        }

#ifndef CHROME_SLIM_CAMHAL
        //add a probe to update and call isp interface
        if(data.test_isp_control) {
          gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                   callback_check_isp_interface, &data, NULL);
        }
#endif  //CHROME_SLIM_CAMHAL
        if(data.test_dewarping_mode_switch) {
          gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                   callback_check_dewarping_interface_dewarping_mode_switch, &data, NULL);
        }
#ifndef CHROME_SLIM_CAMHAL
        if(data.test_wfov) {
	        gboolean ret;
          ret = data.camwfov_iface->get_wfov_mode(data.camwfov, data.wfov_mode);
          if (ret == FALSE)
             g_print("get wfov mode error\n");
          g_print("wfov mode %d\n", data.wfov_mode);

          ret = data.camwfov_iface->get_sensor_mount_type(data.camwfov, data.mount_type);
          if (ret == FALSE)
             g_print("get sensor mount type error\n");
          g_print("mount type %d\n", data.mount_type);

          if (data.set_projection) {
              ret = data.camwfov_iface->set_view_projection(data.camwfov, data.projection);
              if (ret == FALSE)
                 g_print("set projection error\n");
              g_print("set projection %d, %f\n", data.projection.type, data.projection.cone_angle);
          }

          if (data.set_rotation) {
              ret = data.camwfov_iface->set_view_rotation(data.camwfov, data.rotation);
              if (ret == FALSE)
                 g_print("set rotation error\n");
              g_print("set rotation %f, %f, %f\n", data.rotation.pitch, data.rotation.yaw, data.rotation.roll);
          }

          if (data.set_fine_adjustments) {
              ret = data.camwfov_iface->set_view_fine_adjustments(data.camwfov, data.fine_adjustments);
              if (ret == FALSE)
                 g_print("set fine adjustments error\n");
              g_print("set fine adjustments %f, %f, %f\n", data.fine_adjustments.horizontal_shift, data.fine_adjustments.vertical_shift, data.fine_adjustments.window_rotation);
          }

          if (data.set_camera_rotation) {
              ret = data.camwfov_iface->set_camera_rotation(data.camwfov, data.camera_rotation);
              if (ret == FALSE)
                 g_print("set camera rotation error\n");
              g_print("set camera rotation %f, %f, %f\n", data.camera_rotation.pitch, data.camera_rotation.yaw, data.camera_rotation.roll);
          }
      }
      gst_object_unref(pad);
#endif  //CHROME_SLIM_CAMHAL
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus,bus_call, &data);
    gst_object_unref(bus);

    g_print("starting sender pipeline\n");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run(data.main_loop);

    g_print("Returned, stopping the loop\n");
    gst_element_set_state(pipeline,GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));
    g_main_loop_unref (data.main_loop);

    if (data.bus_error == true) {
        g_print("gstreamer bus error\n");
        return -1;
    }

    if (data.test_fps)
        check->info.fps = data.fps;
    if (data.test_property)
        check->property.update_error = data.update_error;

    return 0;
}
