/*
 * Copyright (C) 2017-2018 Intel Corporation.
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

/**
 * Warning this header is automatically generated.
 * Any manual change here will be lost.
 */

#pragma once

#include <stdint.h>

namespace icamera {

typedef enum
{
    camera_control_isp_ctrl_id_wb_gains = 34710,
    camera_control_isp_ctrl_id_color_correction_matrix = 30009,
    camera_control_isp_ctrl_id_advanced_color_correction_matrix = 51448,
    camera_control_isp_ctrl_id_bxt_csc = 5291,
    camera_control_isp_ctrl_id_bxt_demosaic = 55032,
    camera_control_isp_ctrl_id_sc_iefd = 5151,
    camera_control_isp_ctrl_id_see = 10947,
    camera_control_isp_ctrl_id_bnlm = 40283,
    camera_control_isp_ctrl_id_tnr5_21 = 20429,
    camera_control_isp_ctrl_id_xnr_dss = 22233,
    camera_control_isp_ctrl_id_gamma_tone_map = 5809,
    camera_control_isp_ctrl_id_tnr5_22 = 15132,
    camera_control_isp_ctrl_id_tnr5_25 = 41091,
} camera_control_isp_ctrl_id;

/**
 * \struct camera_control_isp_wb_gains_t
 */
struct camera_control_isp_wb_gains_t
{
    /*!< gr Gr gain.*/
    float gr;
    /*!< r R gain.*/
    float r;
    /*!< b B gain.*/
    float b;
    /*!< gb Gb gain.*/
    float gb;

};

/**
 * \struct camera_control_isp_color_correction_matrix_t
 */
struct camera_control_isp_color_correction_matrix_t
{
    /*!< ccm_gains[9] Color correction matrix from sensor RGB to sRGB/target color space. Row-major order.*/
    float ccm_gains[9];
    /*!< ccm_gains_media_format colorspace the ccm matrix was calibrated to*/
    int32_t ccm_gains_media_format;

};

/**
 * \struct camera_control_isp_advanced_color_correction_matrix_t
 */
struct camera_control_isp_advanced_color_correction_matrix_t
{
    /*!< bypass bypass*/
    int32_t bypass;
    /*!< number_of_sectors number of sectors (CCM matrices)*/
    int32_t number_of_sectors;
    /*!< ccm_matrices[216] ccm matrices*/
    float ccm_matrices[216];
    /*!< hue_of_sectors[24] hue of input calibration vectors*/
    float hue_of_sectors[24];
    /*!< ccm_matrices_media_format colorspace the ccm matrices where calibrated to*/
    int32_t ccm_matrices_media_format;

};

/**
 * \struct camera_control_isp_bxt_csc_t
 */
struct camera_control_isp_bxt_csc_t
{
    /*!< rgb2yuv_coef[9] Matrix for RGB to YUV conversion*/
    int32_t rgb2yuv_coef[9];

};

/**
 * \struct camera_control_isp_bxt_demosaic_t
 */
struct camera_control_isp_bxt_demosaic_t
{
    /*!< high_frequency_denoise_enable High frequency denoise enbale flag*/
    int32_t high_frequency_denoise_enable;
    /*!< false_color_correction_enable False color correction enable flag*/
    int32_t false_color_correction_enable;
    /*!< chroma_artifact_removal_enable Chroma artifact removal enable flag*/
    int32_t chroma_artifact_removal_enable;
    /*!< sharpening_strength Sharpening strength*/
    int32_t sharpening_strength;
    /*!< false_color_correction_gain False color correction gain*/
    int32_t false_color_correction_gain;
    /*!< high_frequency_denoise_power High frequency denoise power*/
    int32_t high_frequency_denoise_power;
    /*!< checkers_removal_w Checkers removal homogeneity weight*/
    int32_t checkers_removal_w;

};

/**
 * \struct camera_control_isp_sc_iefd_t
 */
struct camera_control_isp_sc_iefd_t
{
    /*!< sharpening_power[2] base power of sharpening*/
    int32_t sharpening_power[2];
    /*!< sharp_power_edge shapening power for direct edge*/
    int32_t sharp_power_edge;
    /*!< sharp_power_detail shapening power for non-direct edge*/
    int32_t sharp_power_detail;
    /*!< unsharp_weight_edge unsharp weight for direct edge*/
    int32_t unsharp_weight_edge;
    /*!< unsharp_weight_detail unsharp weight for non-direct edge*/
    int32_t unsharp_weight_detail;
    /*!< denoise_power base power of denoise*/
    int32_t denoise_power;
    /*!< radial_denoise_power base power of radial denoise*/
    int32_t radial_denoise_power;
    /*!< shrpn_nega_lmt_txt Undershoot limit for texture*/
    int32_t shrpn_nega_lmt_txt;
    /*!< shrpn_posi_lmt_txt Overshoot limit for texture*/
    int32_t shrpn_posi_lmt_txt;
    /*!< shrpn_nega_lmt_dir Undershoot limit for edge*/
    int32_t shrpn_nega_lmt_dir;
    /*!< shrpn_posi_lmt_dir Overshoot limit for edge*/
    int32_t shrpn_posi_lmt_dir;
    /*!< cu_var_metric_th[4] */
    int32_t cu_var_metric_th[4];
    /*!< cu_ed2_metric_th[2] */
    int32_t cu_ed2_metric_th[2];
    /*!< cu_dir_metric_th[2] */
    int32_t cu_dir_metric_th[2];
    /*!< rad_enable Enable radial dependency flag*/
    int32_t rad_enable;
    /*!< cu_nr_power[2] */
    int32_t cu_nr_power[2];
    /*!< unsharp_filter[15] Unsharp-mask filter coefficients*/
    int32_t unsharp_filter[15];
    /*!< dir_far_sharp_weight[2] Weight of wide kernel direct sharpening*/
    int32_t dir_far_sharp_weight[2];
    /*!< configunited_x[6] CU_ED x points*/
    int32_t configunited_x[6];
    /*!< configunited_y[6] CU_ED y points*/
    int32_t configunited_y[6];
    /*!< configunitradial_x[6] CU_RAD x points*/
    int32_t configunitradial_x[6];
    /*!< configunitradial_y[6] CU_RAD y points*/
    int32_t configunitradial_y[6];
    /*!< denoise_filter[6] Denoising filter coefficients*/
    int32_t denoise_filter[6];
    /*!< vssnlm_x0 Edge denoising LUT x0*/
    int32_t vssnlm_x0;
    /*!< vssnlm_x1 Edge denoising LUT x1*/
    int32_t vssnlm_x1;
    /*!< vssnlm_x2 Edge denoising LUT x2*/
    int32_t vssnlm_x2;
    /*!< vssnlm_y1 Edge denoising LUT y0*/
    int32_t vssnlm_y1;
    /*!< vssnlm_y2 Edge denoising LUT y1*/
    int32_t vssnlm_y2;
    /*!< vssnlm_y3 Edge denoising LUT y2*/
    int32_t vssnlm_y3;

};

/**
 * \struct camera_control_isp_see_t
 */
struct camera_control_isp_see_t
{
    /*!< bypass bypass*/
    int32_t bypass;
    /*!< edge_max max edge value for clamping*/
    int32_t edge_max;
    /*!< edge_thres threshold for coring*/
    int32_t edge_thres;
    /*!< alpha overall edge enhance strength*/
    int32_t alpha;
    /*!< alpha_shift */
    int32_t alpha_shift;
    /*!< alpha_width */
    int32_t alpha_width;
    /*!< alpha_bias */
    int32_t alpha_bias;

};

/**
 * \struct camera_control_isp_bnlm_t
 */
struct camera_control_isp_bnlm_t
{
    /*!< nm_offset NR level for noise model adaptation*/
    int32_t nm_offset;
    /*!< nm_th Threshold for noise model adaptation*/
    int32_t nm_th;
    /*!< bypass bypass for the filter*/
    int32_t bypass;
    /*!< detailix_x_range[2] detailIx noise model - noise input range - tunned automatically from data - set QNN fit range -  [Q14.0]  [0..16383]*/
    int32_t detailix_x_range[2];
    /*!< detailix_radgain detailIx Lens shading radial compensation power -  [Q8.16], [0..256%]*/
    int32_t detailix_radgain;
    /*!< detailix_coeffs[3] detailIx SAD noise model - NoiseModel=SQRT(C1*mu^2+C2*mu+C3)*/
    int32_t detailix_coeffs[3];
    /*!< sad_spatialrad[4] Neighbors spatial (radial) weight - filter radial bias - allowing reduction of effective filter size [Q3.5]   [1:0.25:4]*/
    int32_t sad_spatialrad[4];
    /*!< sad_mu_x_range[2] detailIx noise model - noise input range - tunned automatically from data - set QNN fit range  [Q14.0]  [0..16383]*/
    int32_t sad_mu_x_range[2];
    /*!< sad_radgain SAD Lens shading radial compensation power  - [Q8.16], [0..256%]*/
    int32_t sad_radgain;
    /*!< sad_mu_coeffs[3] SAD noise model - NoiseModel=SQRT(C1*mu^2+C2*mu+C3)*/
    int32_t sad_mu_coeffs[3];
    /*!< detailth[3] detailTH - 3 THs classifying content(detail) type to {flat/weak texture/texture/edge} based on detailIx   -  [14.0]  [0..16383]*/
    int32_t detailth[3];
    /*!< sad_detailixlutx[4] Continuos LUT aligning SAD from different detailIx levels to a common scale before appling exponent scaling - texture to flat & edge discrimination, X axis is currently fixed 0:8:1023  -  [14.0]  [0..16383]*/
    int32_t sad_detailixlutx[4];
    /*!< sad_detailixluty[4] Continuos LUT aligning SAD from different detailIx levels to a common scale before appling exponent scaling - texture to flat & edge discrimination - Yaxis is continuos description of (2^10-1)/QNN_o  -  [14.0]  [0..16383]*/
    int32_t sad_detailixluty[4];
    /*!< numcandforavg[4] max num neighbors to average for smoothing per detail type  - [4.0]  [1..16]*/
    int32_t numcandforavg[4];
    /*!< blend_power[4] blend alpha(alpha0)  - Increase central pixel effect to enhance detail preservation vs smoothing  -  [8.24], [0..256%]*/
    int32_t blend_power[4];
    /*!< blend_th[4] blend alpha(alpha0)  - preserve details based on texture classification of the form alpha0>TH - [0:2^10-1]*/
    int32_t blend_th[4];
    /*!< blend_texturegain[4] blend alpha(alpha0)  - define steepness of blent_th effect for preserve details - alphaRes=2^10-1;   gainScale=255;  outScale=0.01;    curSlope = outScale*tan(pi/2*((fixGain/(gainScale-1))^2))*alphaRes/(alphaRes-curTH) -   [0..256]*/
    int32_t blend_texturegain[4];
    /*!< blend_radgain blend  radial weigh -  [8.16], [0..256%]*/
    int32_t blend_radgain;
    /*!< matchqualitycands[4] set max rank limit for alpha0 selected weights   -   [8.8] [0..14]*/
    int32_t matchqualitycands[4];
    /*!< wsumminth final regret factor on Sum(weight)<TH  -  [16.0]   [0..65532]*/
    int32_t wsumminth;
    /*!< wmaxminth final regret factor on Max(weight)<TH  -  [14.0]   [0..16383]*/
    int32_t wmaxminth;
    /*!< rad_enable Radial LSC correction*/
    int32_t rad_enable;

};

/**
 * \struct camera_control_isp_tnr5_21_t
 */
struct camera_control_isp_tnr5_21_t
{
    /*!< bypass bypass filter*/
    int32_t bypass;
    /*!< nm_yy_xcu_b[64] base y noise model - y dependency*/
    int32_t nm_yy_xcu_b[64];
    /*!< nm_yc_xcu_b[64] base y noise model - c dependency*/
    int32_t nm_yc_xcu_b[64];
    /*!< nm_cy_xcu_b[64] base c noise model - y dependency*/
    int32_t nm_cy_xcu_b[64];
    /*!< nm_cc_xcu_b[64] base c noise model - c dependency*/
    int32_t nm_cc_xcu_b[64];
    /*!< nm_y_log_est_min_b base y noise model - log space min*/
    int32_t nm_y_log_est_min_b;
    /*!< nm_y_log_est_max_b base y noise model - log space max*/
    int32_t nm_y_log_est_max_b;
    /*!< nm_c_log_est_min_b base c noise model - log space min*/
    int32_t nm_c_log_est_min_b;
    /*!< nm_c_log_est_max_b base c noise model - log space max*/
    int32_t nm_c_log_est_max_b;
    /*!< nm_Y_alpha_b base y noise model - scale factor*/
    int32_t nm_Y_alpha_b;
    /*!< nm_C_alpha_b base c noise model - scale factor*/
    int32_t nm_C_alpha_b;
    /*!< Tnr_Strength_0 TNR Strength at dark region*/
    int32_t Tnr_Strength_0;
    /*!< Tnr_Strength_1 TNR Strength at bright region*/
    int32_t Tnr_Strength_1;
    /*!< SpNR_Static Spatial Noise Reduction Strength at static region*/
    int32_t SpNR_Static;
    /*!< SpNR_Dynamic Spatial Noise Reduction Strength at dynamic region*/
    int32_t SpNR_Dynamic;
    /*!< Radial_Gain Radial Gain*/
    int32_t Radial_Gain;
    /*!< SAD_Gain SAD Gain*/
    int32_t SAD_Gain;
    /*!< Pre_Sim_Gain Previous Similarity Gain*/
    int32_t Pre_Sim_Gain;
    /*!< Weight_In Input Weight*/
    int32_t Weight_In;
    /*!< g_mv_x Global Motion Vector - x coordinate*/
    int32_t g_mv_x;
    /*!< g_mv_y Global Motion Vector - y coordinate*/
    int32_t g_mv_y;
    /*!< tbd_sim_gain Transition Gain for Similarity LUT*/
    int32_t tbd_sim_gain;
    /*!< NS_Gain Noise Stream Gain*/
    int32_t NS_Gain;
    /*!< nsw_gain */
    int32_t nsw_gain;
    /*!< nsw_sigma */
    int32_t nsw_sigma;
    /*!< nsw_bias */
    int32_t nsw_bias;
    /*!< ns_clw_bias0 */
    int32_t ns_clw_bias0;
    /*!< ns_clw_bias1 */
    int32_t ns_clw_bias1;
    /*!< ns_clw_sigma */
    int32_t ns_clw_sigma;
    /*!< ns_clw_center */
    int32_t ns_clw_center;
    /*!< ns_norm_bias */
    int32_t ns_norm_bias;
    /*!< ns_norm_coef */
    int32_t ns_norm_coef;
    /*!< bypass_g_mv Bypass Global Motion Vector*/
    int32_t bypass_g_mv;
    /*!< bypass_NS Bypass Noise Stream*/
    int32_t bypass_NS;

};

/**
 * \struct camera_control_isp_xnr_dss_t
 */
struct camera_control_isp_xnr_dss_t
{
    /*!< rad_enable */
    int32_t rad_enable;
    /*!< bypass Bypass all XNR4*/
    int32_t bypass;
    /*!< bypass_mf_y Bypass MF Luma*/
    int32_t bypass_mf_y;
    /*!< bypass_mf_c Bypass MF Chroma*/
    int32_t bypass_mf_c;
    /*!< spatial_sigma_mf_y MF Luma spatial filter - gaussian sigma (Q10)*/
    int32_t spatial_sigma_mf_y;
    /*!< spatial_sigma_mf_c MF Chroma spatial filter - gaussian sigma (Q10)*/
    int32_t spatial_sigma_mf_c;
    /*!< noise_white_mf_y_y MF Luma Noise power per Luma at high Intensity*/
    int32_t noise_white_mf_y_y;
    /*!< noise_white_mf_y_s MF Luma Noise power per Saturation at high Intensity*/
    int32_t noise_white_mf_y_s;
    /*!< noise_white_mf_c_y MF Chroma Noise power per Luma at high Intensity*/
    int32_t noise_white_mf_c_y;
    /*!< noise_white_mf_c_u MF Chroma  Noise power per U at high Intensity*/
    int32_t noise_white_mf_c_u;
    /*!< noise_white_mf_c_v MF Chroma  Noise power per V at high Intensity*/
    int32_t noise_white_mf_c_v;
    /*!< rad_noise_power_mf_luma MF Radial power of Luma Noise power*/
    int32_t rad_noise_power_mf_luma;
    /*!< rad_noise_power_mf_chroma MF Radial power of Chroma Noise power*/
    int32_t rad_noise_power_mf_chroma;
    /*!< range_weight_lut[7] */
    int32_t range_weight_lut[7];
    /*!< one_div_64_lut[64] */
    int32_t one_div_64_lut[64];
    /*!< rad_noise_compensation_mf_chroma */
    int32_t rad_noise_compensation_mf_chroma;
    /*!< rad_noise_compensation_mf_luma */
    int32_t rad_noise_compensation_mf_luma;
    /*!< mf_luma_power */
    int32_t mf_luma_power;
    /*!< rad_mf_luma_power */
    int32_t rad_mf_luma_power;
    /*!< mf_chroma_power */
    int32_t mf_chroma_power;
    /*!< rad_mf_chroma_power */
    int32_t rad_mf_chroma_power;
    /*!< noise_black_mf_y_y */
    int32_t noise_black_mf_y_y;
    /*!< noise_black_mf_c_y */
    int32_t noise_black_mf_c_y;
    /*!< noise_black_mf_c_u */
    int32_t noise_black_mf_c_u;
    /*!< noise_black_mf_c_v */
    int32_t noise_black_mf_c_v;
    /*!< xcu_lcs_x[33] */
    int32_t xcu_lcs_x[33];
    /*!< xcu_lcs_exp XCU ed exponent*/
    int32_t xcu_lcs_exp;
    /*!< xcu_lcs_slp_a_res XCU ed slope precision*/
    int32_t xcu_lcs_slp_a_res;
    /*!< xcu_lcs_offset[32] XCU ed offset(y) vector*/
    int32_t xcu_lcs_offset[32];
    /*!< xcu_lcs_slope[32] XCU ed slope vector*/
    int32_t xcu_lcs_slope[32];
    /*!< lcs_th_for_black */
    int32_t lcs_th_for_black;
    /*!< lcs_th_for_white */
    int32_t lcs_th_for_white;
    /*!< rad_lcs_th_for_black */
    int32_t rad_lcs_th_for_black;
    /*!< rad_lcs_th_for_white */
    int32_t rad_lcs_th_for_white;
    /*!< blnd_hf_power_y New in DSS_XNR*/
    int32_t blnd_hf_power_y;
    /*!< blnd_hf_power_c New in DSS_XNR*/
    int32_t blnd_hf_power_c;

};

/**
 * \struct camera_control_isp_gamma_tone_map_t
 */
struct camera_control_isp_gamma_tone_map_t
{
    /*!< gamma[2048] Gamma table for all channels*/
    float gamma[2048];
    /*!< gamma_lut_size Gamma LUT size*/
    uint32_t gamma_lut_size;
    /*!< tone_map[2048] Tone map for all channels*/
    float tone_map[2048];
    /*!< tone_map_lut_size Tone map LUT size*/
    uint32_t tone_map_lut_size;

};

/**
 * \struct camera_control_isp_tnr5_22_t
 */
struct camera_control_isp_tnr5_22_t
{
    /*!< bypass bypass filter*/
    int32_t bypass;
    /*!< nm_yy_xcu_b[64] base y noise model - y dependency*/
    int32_t nm_yy_xcu_b[64];
    /*!< nm_yc_xcu_b[64] base y noise model - c dependency*/
    int32_t nm_yc_xcu_b[64];
    /*!< nm_cy_xcu_b[64] base c noise model - y dependency*/
    int32_t nm_cy_xcu_b[64];
    /*!< nm_cc_xcu_b[64] base c noise model - c dependency*/
    int32_t nm_cc_xcu_b[64];
    /*!< nm_y_log_est_min_b base y noise model - log space min*/
    int32_t nm_y_log_est_min_b;
    /*!< nm_y_log_est_max_b base y noise model - log space max*/
    int32_t nm_y_log_est_max_b;
    /*!< nm_c_log_est_min_b base c noise model - log space min*/
    int32_t nm_c_log_est_min_b;
    /*!< nm_c_log_est_max_b base c noise model - log space max*/
    int32_t nm_c_log_est_max_b;
    /*!< nm_Y_alpha_b base y noise model - scale factor*/
    int32_t nm_Y_alpha_b;
    /*!< nm_C_alpha_b base c noise model - scale factor*/
    int32_t nm_C_alpha_b;
    /*!< Tnr_Strength_0 TNR Strength at dark region*/
    int32_t Tnr_Strength_0;
    /*!< Tnr_Strength_1 TNR Strength at bright region*/
    int32_t Tnr_Strength_1;
    /*!< SpNR_Static Spatial Noise Reduction Strength at static region*/
    int32_t SpNR_Static;
    /*!< SpNR_Dynamic Spatial Noise Reduction Strength at dynamic region*/
    int32_t SpNR_Dynamic;
    /*!< Radial_Gain Radial Gain*/
    int32_t Radial_Gain;
    /*!< SAD_Gain SAD Gain*/
    int32_t SAD_Gain;
    /*!< Pre_Sim_Gain Previous Similarity Gain*/
    int32_t Pre_Sim_Gain;
    /*!< Weight_In Input Weight*/
    int32_t Weight_In;
    /*!< g_mv_x Global Motion Vector - x coordinate*/
    int32_t g_mv_x;
    /*!< g_mv_y Global Motion Vector - y coordinate*/
    int32_t g_mv_y;
    /*!< tbd_sim_gain Transition Gain for Similarity LUT*/
    int32_t tbd_sim_gain;
    /*!< NS_Gain Noise Stream Gain*/
    int32_t NS_Gain;
    /*!< nsw_gain */
    int32_t nsw_gain;
    /*!< nsw_sigma */
    int32_t nsw_sigma;
    /*!< nsw_bias */
    int32_t nsw_bias;
    /*!< ns_clw_bias0 */
    int32_t ns_clw_bias0;
    /*!< ns_clw_bias1 */
    int32_t ns_clw_bias1;
    /*!< ns_clw_sigma */
    int32_t ns_clw_sigma;
    /*!< ns_clw_center */
    int32_t ns_clw_center;
    /*!< ns_norm_bias */
    int32_t ns_norm_bias;
    /*!< ns_norm_coef */
    int32_t ns_norm_coef;
    /*!< bypass_g_mv Bypass Global Motion Vector*/
    int32_t bypass_g_mv;
    /*!< bypass_NS Bypass Noise Stream*/
    int32_t bypass_NS;

};

/**
 * \struct camera_control_isp_tnr5_25_t
 */
struct camera_control_isp_tnr5_25_t
{
    /*!< bypass bypass filter*/
    int32_t bypass;
    /*!< nm_yy_xcu_b[64] base y noise model - y dependency*/
    int32_t nm_yy_xcu_b[64];
    /*!< nm_yc_xcu_b[64] base y noise model - c dependency*/
    int32_t nm_yc_xcu_b[64];
    /*!< nm_cy_xcu_b[64] base c noise model - y dependency*/
    int32_t nm_cy_xcu_b[64];
    /*!< nm_cc_xcu_b[64] base c noise model - c dependency*/
    int32_t nm_cc_xcu_b[64];
    /*!< nm_y_log_est_min_b base y noise model - log space min*/
    int32_t nm_y_log_est_min_b;
    /*!< nm_y_log_est_max_b base y noise model - log space max*/
    int32_t nm_y_log_est_max_b;
    /*!< nm_c_log_est_min_b base c noise model - log space min*/
    int32_t nm_c_log_est_min_b;
    /*!< nm_c_log_est_max_b base c noise model - log space max*/
    int32_t nm_c_log_est_max_b;
    /*!< nm_Y_alpha_b base y noise model - scale factor*/
    int32_t nm_Y_alpha_b;
    /*!< nm_C_alpha_b base c noise model - scale factor*/
    int32_t nm_C_alpha_b;
    /*!< Tnr_Strength_0 TNR Strength at dark region*/
    int32_t Tnr_Strength_0;
    /*!< Tnr_Strength_1 TNR Strength at bright region*/
    int32_t Tnr_Strength_1;
    /*!< Radial_Gain Radial Gain*/
    int32_t Radial_Gain;
    /*!< SAD_Gain SAD Gain*/
    int32_t SAD_Gain;
    /*!< Pre_Sim_Gain Previous Similarity Gain*/
    int32_t Pre_Sim_Gain;
    /*!< Weight_In Input Weight*/
    int32_t Weight_In;
    /*!< g_mv_x Global Motion Vector - x coordinate*/
    int32_t g_mv_x;
    /*!< g_mv_y Global Motion Vector - y coordinate*/
    int32_t g_mv_y;
    /*!< tbd_sim_gain Transition Gain for Similarity LUT*/
    int32_t tbd_sim_gain;
    /*!< bypass_g_mv Bypass Global Motion Vector*/
    int32_t bypass_g_mv;

};

} // end of icamera
