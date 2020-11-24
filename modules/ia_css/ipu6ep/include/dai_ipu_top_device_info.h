/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2017 Intel Corporation.
* All Rights Reserved.
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Intel Corporation
* or licensors. Title to the Material remains with Intel
* Corporation or its licensors. The Material contains trade
* secrets and proprietary and confidential information of Intel or its
* licensors. The Material is protected by worldwide copyright
* and trade secret laws and treaty provisions. No part of the Material may
* be used, copied, reproduced, modified, published, uploaded, posted,
* transmitted, distributed, or disclosed in any way without Intel's prior
* express written permission.
*
* No License under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or
* delivery of the Materials, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights
* must be express and approved by Intel in writing.
*/
#ifndef _dai_ipu_top_device_info_h_
#define _dai_ipu_top_device_info_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_info_types.h>
#include <dai/dai_subsystem_property_types.h>
#include "bb_output_formatter_properties_types.h"
#include "bbgdc4_properties_types.h"
#include "bbpolyphase_scaler_properties_types.h"
#include "bbtnr_properties_types.h"
#include "cell_properties_types.h"
#include "cell_regmem.h"
#include "dai_ipu_top_device_properties.h"
#include "dai_ipu_top_devices.h"
#include "device_property_types.h"
#include "device_types.h"
#include "device_types_master_ports.h"
#include "device_types_slave_ports.h"
#include "dfm_properties_types.h"
#include "dvs_controller_properties_types.h"
#include "mbr_properties_types.h"
#include "vec_to_str_v3_properties_types.h"
#include "wpt_properties_types.h"
/* all property values for property ack_fifo_depth for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_ack_fifo_depth_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x4
};

/* all property values for property cmd_fifo_depth for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_cmd_fifo_depth_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x6
};

/* all property values for property implementation for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_implementation_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property max_nr_hist_y_lines for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_max_nr_hist_y_lines_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x1F
};

/* all property values for property max_nr_inp_buffers for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_max_nr_inp_buffers_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x8
};

/* all property values for property max_nr_out_buf_y_lines for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_max_nr_out_buf_y_lines_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x40
};

/* all property values for property max_nr_outp_buffers for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_max_nr_outp_buffers_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x4
};

/* all property values for property max_out_buf_level for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_max_out_buf_level_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x2
};

/* all property values for property native_cio2_dblock for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_native_cio2_dblock_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x0
};

/* all property values for property native_cioraccept for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_native_cioraccept_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x1
};

/* all property values for property native_ciosrmd for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_native_ciosrmd_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x0
};

/* all property values for property nr_pins for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_nr_pins_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x3
};

/* all property values for property output_formats for devices of type bb_output_formatter */
static const dai_property_value8_t * const dai_ipu_top_bb_output_formatter_output_formats_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property pin_parameters for devices of type bb_output_formatter */
static const dai_property_value8_t * const dai_ipu_top_bb_output_formatter_pin_parameters_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property pixel_bits for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_pixel_bits_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0xC
};

/* all property values for property range_pcpc for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_range_pcpc_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x10
};

/* all property values for property tiling_support for devices of type bb_output_formatter */
static const dai_property_value8_t * const dai_ipu_top_bb_output_formatter_tiling_support_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property upsample_pcpc for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_upsample_pcpc_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x10
};

/* all property values for property use_sidpid for devices of type bb_output_formatter */
static const dai_property_value8_t dai_ipu_top_bb_output_formatter_use_sidpid_list[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	0x1
};

/* struct with references to all property lists for device type 'CustomDeviceType bb_output_formatter (0 instances, 1 derived types)' */
static const dai_dev_bb_output_formatter_property_t dai_ipu_top_all_bb_output_formatter_properties = {
	dai_ipu_top_bb_output_formatter_ack_fifo_depth_list/* ack_fifo_depth */,
	dai_ipu_top_bb_output_formatter_cmd_fifo_depth_list/* cmd_fifo_depth */,
	dai_ipu_top_bb_output_formatter_implementation_list/* implementation */,
	dai_ipu_top_bb_output_formatter_max_nr_hist_y_lines_list/* max_nr_hist_y_lines */,
	dai_ipu_top_bb_output_formatter_max_nr_inp_buffers_list/* max_nr_inp_buffers */,
	dai_ipu_top_bb_output_formatter_max_nr_out_buf_y_lines_list/* max_nr_out_buf_y_lines */,
	dai_ipu_top_bb_output_formatter_max_nr_outp_buffers_list/* max_nr_outp_buffers */,
	dai_ipu_top_bb_output_formatter_max_out_buf_level_list/* max_out_buf_level */,
	dai_ipu_top_bb_output_formatter_native_cio2_dblock_list/* native_cio2_dblock */,
	dai_ipu_top_bb_output_formatter_native_cioraccept_list/* native_cioraccept */,
	dai_ipu_top_bb_output_formatter_native_ciosrmd_list/* native_ciosrmd */,
	dai_ipu_top_bb_output_formatter_nr_pins_list/* nr_pins */,
	dai_ipu_top_bb_output_formatter_output_formats_list/* output_formats */,
	dai_ipu_top_bb_output_formatter_pin_parameters_list/* pin_parameters */,
	dai_ipu_top_bb_output_formatter_pixel_bits_list/* pixel_bits */,
	dai_ipu_top_bb_output_formatter_range_pcpc_list/* range_pcpc */,
	dai_ipu_top_bb_output_formatter_tiling_support_list/* tiling_support */,
	dai_ipu_top_bb_output_formatter_upsample_pcpc_list/* upsample_pcpc */,
	dai_ipu_top_bb_output_formatter_use_sidpid_list/* use_sidpid */
};

/* all property values for property ack_address_bits for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_ack_address_bits_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x20
};

/* all property values for property ack_fifo_lat for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_ack_fifo_lat_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x0
};

/* all property values for property ack_fifo_size for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_ack_fifo_size_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x4
};

/* all property values for property ack_pid_bits for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_ack_pid_bits_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x6
};

/* all property values for property ack_pid_idx for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_ack_pid_idx_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x14
};

/* all property values for property ack_sid_bits for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_ack_sid_bits_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x6
};

/* all property values for property ack_sid_idx for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_ack_sid_idx_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x1A
};

/* all property values for property acknowledge_port_id for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_acknowledge_port_id_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property bamem_address_bits for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_bamem_address_bits_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x20
};

/* all property values for property bamem_stride_bits for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_bamem_stride_bits_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x11
};

/* all property values for property cmd_fifo_lat for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_cmd_fifo_lat_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x0
};

/* all property values for property cmd_fifo_size for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_cmd_fifo_size_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x3C
};

/* all property values for property data_port_id for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_data_port_id_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property data_slave_id for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_data_slave_id_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property elements_per_vector for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_elements_per_vector_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x20
};

/* all property values for property gmem_addr_offset for devices of type bbgdc4 */
static const dai_property_value16_t dai_ipu_top_bbgdc4_gmem_addr_offset_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0xC000
};

/* all property values for property gmem_capacity for devices of type bbgdc4 */
static const dai_property_value16_t dai_ipu_top_bbgdc4_gmem_capacity_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x800
};

/* all property values for property implementation for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_implementation_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property max_bits_per_element for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_max_bits_per_element_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x10
};

/* all property values for property memory_capacity for devices of type bbgdc4 */
static const dai_property_value16_t dai_ipu_top_bbgdc4_memory_capacity_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x2000
};

/* all property values for property native_cioraccept for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_native_cioraccept_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x1
};

/* all property values for property num_reg_channels for devices of type bbgdc4 */
static const dai_property_value8_t dai_ipu_top_bbgdc4_num_reg_channels_list[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	0x4
};

/* struct with references to all property lists for device type 'CustomDeviceType bbgdc4 (0 instances, 1 derived types)' */
static const dai_dev_bbgdc4_property_t dai_ipu_top_all_bbgdc4_properties = {
	dai_ipu_top_bbgdc4_ack_address_bits_list/* ack_address_bits */,
	dai_ipu_top_bbgdc4_ack_fifo_lat_list/* ack_fifo_lat */,
	dai_ipu_top_bbgdc4_ack_fifo_size_list/* ack_fifo_size */,
	dai_ipu_top_bbgdc4_ack_pid_bits_list/* ack_pid_bits */,
	dai_ipu_top_bbgdc4_ack_pid_idx_list/* ack_pid_idx */,
	dai_ipu_top_bbgdc4_ack_sid_bits_list/* ack_sid_bits */,
	dai_ipu_top_bbgdc4_ack_sid_idx_list/* ack_sid_idx */,
	dai_ipu_top_bbgdc4_acknowledge_port_id_list/* acknowledge_port_id */,
	dai_ipu_top_bbgdc4_bamem_address_bits_list/* bamem_address_bits */,
	dai_ipu_top_bbgdc4_bamem_stride_bits_list/* bamem_stride_bits */,
	dai_ipu_top_bbgdc4_cmd_fifo_lat_list/* cmd_fifo_lat */,
	dai_ipu_top_bbgdc4_cmd_fifo_size_list/* cmd_fifo_size */,
	dai_ipu_top_bbgdc4_data_port_id_list/* data_port_id */,
	dai_ipu_top_bbgdc4_data_slave_id_list/* data_slave_id */,
	dai_ipu_top_bbgdc4_elements_per_vector_list/* elements_per_vector */,
	dai_ipu_top_bbgdc4_gmem_addr_offset_list/* gmem_addr_offset */,
	dai_ipu_top_bbgdc4_gmem_capacity_list/* gmem_capacity */,
	dai_ipu_top_bbgdc4_implementation_list/* implementation */,
	dai_ipu_top_bbgdc4_max_bits_per_element_list/* max_bits_per_element */,
	dai_ipu_top_bbgdc4_memory_capacity_list/* memory_capacity */,
	dai_ipu_top_bbgdc4_native_cioraccept_list/* native_cioraccept */,
	dai_ipu_top_bbgdc4_num_reg_channels_list/* num_reg_channels */
};

/* all property values for property cmd_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x20,
	0x20
};

/* all property values for property cmd_buffer_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_buffer_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x4,
	0x4
};

/* all property values for property cmd_buffer_idx for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_buffer_idx_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property cmd_cmd_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_cmd_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x2,
	0x2
};

/* all property values for property cmd_cmd_idx for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_cmd_idx_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0,
	0x0
};

/* all property values for property cmd_fifo_lat for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_fifo_lat_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0,
	0x0
};

/* all property values for property cmd_fifo_size for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_fifo_size_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x4,
	0x4
};

/* all property values for property cmd_height_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_height_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x7,
	0x7
};

/* all property values for property cmd_height_idx for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_height_idx_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x9,
	0x9
};

/* all property values for property cmd_pid_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_pid_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x6,
	0x6
};

/* all property values for property cmd_pid_idx for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_pid_idx_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x14,
	0x14
};

/* all property values for property cmd_pin_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_pin_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0,
	0x0
};

/* all property values for property cmd_pin_idx for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_pin_idx_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x14,
	0x14
};

/* all property values for property cmd_sid_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_sid_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x6,
	0x6
};

/* all property values for property cmd_sid_idx for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_sid_idx_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x1A,
	0x1A
};

/* all property values for property cmd_width_bits for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_width_bits_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x7,
	0x7
};

/* all property values for property cmd_width_idx for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_cmd_width_idx_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x2,
	0x2
};

/* all property values for property context_port_id for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_context_port_id_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property control_port_id for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_control_port_id_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property elements_per_vector for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_elements_per_vector_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property implementation for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_implementation_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property input_data_bpp for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_input_data_bpp_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x8,
	0x8
};

/* all property values for property input_port_id for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_input_port_id_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property inter_data_bpp for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_inter_data_bpp_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0xA,
	0xA
};

/* all property values for property inter_port_id for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_inter_port_id_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property max_bits_per_coef for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_max_bits_per_coef_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x8,
	0x8
};

/* all property values for property num_phases for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_num_phases_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x20,
	0x20
};

/* all property values for property num_pins for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_num_pins_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x1,
	0x1
};

/* all property values for property num_polyphase_taps for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_num_polyphase_taps_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x4,
	0x4
};

/* all property values for property out_fifo_lat for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_out_fifo_lat_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0,
	0x0
};

/* all property values for property output_data_bpp for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_output_data_bpp_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x8,
	0x8
};

/* all property values for property output_port_id for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_output_port_id_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property pin_params for devices of type bbpolyphase_scaler */
static const dai_property_value8_t * const dai_ipu_top_bbpolyphase_scaler_pin_params_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property pixels_per_cycle for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_pixels_per_cycle_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x8,
	0x8
};

/* all property values for property protocol_data_width for devices of type bbpolyphase_scaler */
static const dai_property_value8_t dai_ipu_top_bbpolyphase_scaler_protocol_data_width_list[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	0x20,
	0x20
};

/* struct with references to all property lists for device type 'CustomDeviceType bbpolyphase_scaler (0 instances, 1 derived types)' */
static const dai_dev_bbpolyphase_scaler_property_t dai_ipu_top_all_bbpolyphase_scaler_properties = {
	dai_ipu_top_bbpolyphase_scaler_cmd_bits_list/* cmd_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_buffer_bits_list/* cmd_buffer_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_buffer_idx_list/* cmd_buffer_idx */,
	dai_ipu_top_bbpolyphase_scaler_cmd_cmd_bits_list/* cmd_cmd_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_cmd_idx_list/* cmd_cmd_idx */,
	dai_ipu_top_bbpolyphase_scaler_cmd_fifo_lat_list/* cmd_fifo_lat */,
	dai_ipu_top_bbpolyphase_scaler_cmd_fifo_size_list/* cmd_fifo_size */,
	dai_ipu_top_bbpolyphase_scaler_cmd_height_bits_list/* cmd_height_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_height_idx_list/* cmd_height_idx */,
	dai_ipu_top_bbpolyphase_scaler_cmd_pid_bits_list/* cmd_pid_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_pid_idx_list/* cmd_pid_idx */,
	dai_ipu_top_bbpolyphase_scaler_cmd_pin_bits_list/* cmd_pin_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_pin_idx_list/* cmd_pin_idx */,
	dai_ipu_top_bbpolyphase_scaler_cmd_sid_bits_list/* cmd_sid_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_sid_idx_list/* cmd_sid_idx */,
	dai_ipu_top_bbpolyphase_scaler_cmd_width_bits_list/* cmd_width_bits */,
	dai_ipu_top_bbpolyphase_scaler_cmd_width_idx_list/* cmd_width_idx */,
	dai_ipu_top_bbpolyphase_scaler_context_port_id_list/* context_port_id */,
	dai_ipu_top_bbpolyphase_scaler_control_port_id_list/* control_port_id */,
	dai_ipu_top_bbpolyphase_scaler_elements_per_vector_list/* elements_per_vector */,
	dai_ipu_top_bbpolyphase_scaler_implementation_list/* implementation */,
	dai_ipu_top_bbpolyphase_scaler_input_data_bpp_list/* input_data_bpp */,
	dai_ipu_top_bbpolyphase_scaler_input_port_id_list/* input_port_id */,
	dai_ipu_top_bbpolyphase_scaler_inter_data_bpp_list/* inter_data_bpp */,
	dai_ipu_top_bbpolyphase_scaler_inter_port_id_list/* inter_port_id */,
	dai_ipu_top_bbpolyphase_scaler_max_bits_per_coef_list/* max_bits_per_coef */,
	dai_ipu_top_bbpolyphase_scaler_num_phases_list/* num_phases */,
	dai_ipu_top_bbpolyphase_scaler_num_pins_list/* num_pins */,
	dai_ipu_top_bbpolyphase_scaler_num_polyphase_taps_list/* num_polyphase_taps */,
	dai_ipu_top_bbpolyphase_scaler_out_fifo_lat_list/* out_fifo_lat */,
	dai_ipu_top_bbpolyphase_scaler_output_data_bpp_list/* output_data_bpp */,
	dai_ipu_top_bbpolyphase_scaler_output_port_id_list/* output_port_id */,
	dai_ipu_top_bbpolyphase_scaler_pin_params_list/* pin_params */,
	dai_ipu_top_bbpolyphase_scaler_pixels_per_cycle_list/* pixels_per_cycle */,
	dai_ipu_top_bbpolyphase_scaler_protocol_data_width_list/* protocol_data_width */
};

/* all property values for property implementation for devices of type bbtnr */
static const dai_property_value8_t dai_ipu_top_bbtnr_implementation_list[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* all property values for property native_cio2_dblock for devices of type bbtnr */
static const dai_property_value8_t dai_ipu_top_bbtnr_native_cio2_dblock_list[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	0x0
};

/* all property values for property native_cioraccept for devices of type bbtnr */
static const dai_property_value8_t dai_ipu_top_bbtnr_native_cioraccept_list[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	0x1
};

/* all property values for property native_ciosrmd for devices of type bbtnr */
static const dai_property_value8_t dai_ipu_top_bbtnr_native_ciosrmd_list[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	0x0
};

/* struct with references to all property lists for device type 'CustomDeviceType bbtnr (0 instances, 1 derived types)' */
static const dai_dev_bbtnr_property_t dai_ipu_top_all_bbtnr_properties = {
	dai_ipu_top_bbtnr_implementation_list/* implementation */,
	dai_ipu_top_bbtnr_native_cio2_dblock_list/* native_cio2_dblock */,
	dai_ipu_top_bbtnr_native_cioraccept_list/* native_cioraccept */,
	dai_ipu_top_bbtnr_native_ciosrmd_list/* native_ciosrmd */
};

/* all property values for property control_register for devices of type cell */
static const dai_property_value32_t dai_ipu_top_cell_control_register_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	DAI_CELL_REG_CONFIG_ICACHE_STAT_CONTROL_CONFIG_ICACHE_STAT_CONTROL_REG_0
};

static const dai_property_value32_t _dai_ipu_top_sp_control_tile_ps_sp_first_base_address_register_property_array[] = {
	DAI_CELL_REG_CONFIG_ICACHE_STAT_CONTROL_BASE_ADDR_SEG_0_MI_CONFIG_ICACHE_MASTER /* value for index imt_op */,
	DAI_CELL_REG_CONFIG_ICACHE_STAT_CONTROL_BASE_ADDR_SEG_0_MI_QMEM_MASTER_INT /* value for index qmt_op */,
	DAI_CELL_REG_CONFIG_ICACHE_STAT_CONTROL_BASE_ADDR_SEG_0_MI_CMEM_MASTER_INT /* value for index cmt_op */,
	DAI_CELL_REG_CONFIG_ICACHE_STAT_CONTROL_BASE_ADDR_SEG_0_MI_XMEM_MASTER_INT /* value for index xmt_op */
};

/* all property values for property first_base_address_register for devices of type cell */
static const dai_property_value32_t * const dai_ipu_top_cell_first_base_address_register_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	_dai_ipu_top_sp_control_tile_ps_sp_first_base_address_register_property_array
};

/* all property values for property icache_master_port for devices of type cell */
static const dai_property_value32_t dai_ipu_top_cell_icache_master_port_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	DAI_CELL_IMT_OP
};

/* all property values for property icache_segment_size for devices of type cell */
static const dai_property_value32_t dai_ipu_top_cell_icache_segment_size_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	262144
};

static const dai_property_value32_t _dai_ipu_top_sp_control_tile_ps_sp_memory_slave_port_property_array[] = {
	DAI_CELL_SL_STAT_IP /* value for index config_icache */,
	DAI_CELL_SL_DMEM_IP /* value for index dmem */,
	INVALID_PORT /* value for index qmem */,
	INVALID_PORT /* value for index cmem */,
	INVALID_PORT /* value for index xmem */
};

/* all property values for property memory_slave_port for devices of type cell */
static const dai_property_value32_t * const dai_ipu_top_cell_memory_slave_port_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	_dai_ipu_top_sp_control_tile_ps_sp_memory_slave_port_property_array
};

static const dai_property_value8_t _dai_ipu_top_sp_control_tile_ps_sp_num_base_address_registers_property_array[] = {
	1 /* value for index imt_op */,
	1 /* value for index qmt_op */,
	4 /* value for index cmt_op */,
	4 /* value for index xmt_op */
};

/* all property values for property num_base_address_registers for devices of type cell */
static const dai_property_value8_t * const dai_ipu_top_cell_num_base_address_registers_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	_dai_ipu_top_sp_control_tile_ps_sp_num_base_address_registers_property_array
};

static const dai_property_value32_t _dai_ipu_top_sp_control_tile_ps_sp_preferred_master_ports_property_array[] = {
	INVALID_PORT,
	DAI_CELL_CMT_OP,
	DAI_CELL_XMT_OP
};

/* all property values for property preferred_master_ports for devices of type cell */
static const dai_property_value32_t * const dai_ipu_top_cell_preferred_master_ports_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	_dai_ipu_top_sp_control_tile_ps_sp_preferred_master_ports_property_array
};

/* all property values for property program_memory_width for devices of type cell */
static const dai_property_value8_t dai_ipu_top_cell_program_memory_width_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	4
};

/* all property values for property start_register for devices of type cell */
static const dai_property_value32_t dai_ipu_top_cell_start_register_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	DAI_CELL_REG_CONFIG_ICACHE_STAT_CONTROL_START_ADDRESS
};

/* all property values for property status_control_slave_port for devices of type cell */
static const dai_property_value32_t dai_ipu_top_cell_status_control_slave_port_list[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	DAI_CELL_SL_STAT_IP
};

/* struct with references to all property lists for device type 'CustomDeviceType cell (0 instances, 3 derived types)' */
static const dai_dev_cell_property_t dai_ipu_top_all_cell_properties = {
	dai_ipu_top_cell_control_register_list/* control_register */,
	dai_ipu_top_cell_first_base_address_register_list/* first_base_address_register */,
	dai_ipu_top_cell_icache_master_port_list/* icache_master_port */,
	dai_ipu_top_cell_icache_segment_size_list/* icache_segment_size */,
	dai_ipu_top_cell_memory_slave_port_list/* memory_slave_port */,
	dai_ipu_top_cell_num_base_address_registers_list/* num_base_address_registers */,
	dai_ipu_top_cell_preferred_master_ports_list/* preferred_master_ports */,
	dai_ipu_top_cell_program_memory_width_list/* program_memory_width */,
	dai_ipu_top_cell_start_register_list/* start_register */,
	dai_ipu_top_cell_status_control_slave_port_list/* status_control_slave_port */
};

/* all property values for property implementation for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_implementation_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property mst_cio_cs_run for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_mst_cio_cs_run_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property native_cio2_dblock for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_native_cio2_dblock_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property native_cioraccept for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_native_cioraccept_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property native_ciosrmd for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_native_ciosrmd_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property not_translate2_dblock for devices of type dfm */
static const dai_property_value8_t * const dai_ipu_top_dfm_not_translate2_dblock_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property not_translate_srmd for devices of type dfm */
static const dai_property_value8_t * const dai_ipu_top_dfm_not_translate_srmd_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property p_cmd_bank_entr_begin_15_8 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_15_8_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x2,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_begin_23_16 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_23_16_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x2AA,
	0x0
};

/* all property values for property p_cmd_bank_entr_begin_31_24 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_31_24_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_cmd_bank_entr_begin_39_32 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_39_32_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0xAAAA,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_begin_47_40 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_47_40_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x2,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_begin_55_48 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_55_48_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x2AA,
	0x0
};

/* all property values for property p_cmd_bank_entr_begin_63_56 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_63_56_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_cmd_bank_entr_begin_7_0 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_begin_7_0_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0xAAAA,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_end_15_8 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_end_15_8_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x2,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_end_23_16 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_end_23_16_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x2AA,
	0x0
};

/* all property values for property p_cmd_bank_entr_end_31_24 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_cmd_bank_entr_end_31_24_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_cmd_bank_entr_end_39_32 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_end_39_32_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0xAAAA,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_end_47_40 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_end_47_40_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x2,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_end_55_48 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_end_55_48_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x2AA,
	0x0
};

/* all property values for property p_cmd_bank_entr_end_63_56 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_cmd_bank_entr_end_63_56_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_cmd_bank_entr_end_7_0 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_end_7_0_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0xAAAA,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_middle_15_8 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_15_8_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x2,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_middle_23_16 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_23_16_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x2AA,
	0x0
};

/* all property values for property p_cmd_bank_entr_middle_31_24 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_31_24_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_cmd_bank_entr_middle_39_32 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_39_32_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0xAAAA,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_middle_47_40 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_47_40_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x2,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_cmd_bank_entr_middle_55_48 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_55_48_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x2AA,
	0x0
};

/* all property values for property p_cmd_bank_entr_middle_63_56 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_63_56_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_cmd_bank_entr_middle_7_0 for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_cmd_bank_entr_middle_7_0_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0xAAAA,
	0xAAAA,
	0xAAAA
};

/* all property values for property p_dev_port_pairs_num for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dev_port_pairs_num_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x9,
	0x15,
	0x10
};

/* all property values for property p_dp_en_0 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_0_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_1 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_1_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_10 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_10_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x1
};

/* all property values for property p_dp_en_11 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_11_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x1
};

/* all property values for property p_dp_en_12 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_12_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x1
};

/* all property values for property p_dp_en_13 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_13_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x1
};

/* all property values for property p_dp_en_14 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_14_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x1
};

/* all property values for property p_dp_en_15 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_15_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x1
};

/* all property values for property p_dp_en_16 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_16_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x0
};

/* all property values for property p_dp_en_17 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_17_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x0
};

/* all property values for property p_dp_en_18 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_18_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x0
};

/* all property values for property p_dp_en_19 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_19_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x0
};

/* all property values for property p_dp_en_2 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_2_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_20 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_20_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x0
};

/* all property values for property p_dp_en_21 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_21_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_22 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_22_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_23 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_23_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_24 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_24_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_25 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_25_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_26 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_26_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_27 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_27_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_28 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_28_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_29 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_29_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_3 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_3_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_30 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_30_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_31 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_31_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_dp_en_4 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_4_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_5 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_5_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_6 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_6_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_7 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_7_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_8 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_8_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_dp_en_9 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_dp_en_9_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x1,
	0x1
};

/* all property values for property p_rf_depth for devices of type dfm */
static const dai_property_value16_t dai_ipu_top_dfm_p_rf_depth_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x80,
	0x16C,
	0x16C
};

/* all property values for property p_sdf_reg_en_0 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_0_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_1 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_1_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_10 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_10_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_reg_en_11 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_11_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_reg_en_12 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_12_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_reg_en_13 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_13_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_reg_en_14 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_14_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_reg_en_15 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_15_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_reg_en_2 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_2_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_3 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_3_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_4 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_4_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_5 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_5_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_6 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_6_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_7 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_7_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x1,
	0x1,
	0x1
};

/* all property values for property p_sdf_reg_en_8 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_8_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_reg_en_9 for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_reg_en_9_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property p_sdf_regs_num for devices of type dfm */
static const dai_property_value8_t dai_ipu_top_dfm_p_sdf_regs_num_list[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	0x8,
	0x8,
	0x8
};

/* struct with references to all property lists for device type 'CustomDeviceType dfm (0 instances, 3 derived types)' */
static const dai_dev_dfm_property_t dai_ipu_top_all_dfm_properties = {
	dai_ipu_top_dfm_implementation_list/* implementation */,
	dai_ipu_top_dfm_mst_cio_cs_run_list/* mst_cio_cs_run */,
	dai_ipu_top_dfm_native_cio2_dblock_list/* native_cio2_dblock */,
	dai_ipu_top_dfm_native_cioraccept_list/* native_cioraccept */,
	dai_ipu_top_dfm_native_ciosrmd_list/* native_ciosrmd */,
	dai_ipu_top_dfm_not_translate2_dblock_list/* not_translate2_dblock */,
	dai_ipu_top_dfm_not_translate_srmd_list/* not_translate_srmd */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_15_8_list/* p_cmd_bank_entr_begin_15_8 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_23_16_list/* p_cmd_bank_entr_begin_23_16 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_31_24_list/* p_cmd_bank_entr_begin_31_24 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_39_32_list/* p_cmd_bank_entr_begin_39_32 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_47_40_list/* p_cmd_bank_entr_begin_47_40 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_55_48_list/* p_cmd_bank_entr_begin_55_48 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_63_56_list/* p_cmd_bank_entr_begin_63_56 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_begin_7_0_list/* p_cmd_bank_entr_begin_7_0 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_15_8_list/* p_cmd_bank_entr_end_15_8 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_23_16_list/* p_cmd_bank_entr_end_23_16 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_31_24_list/* p_cmd_bank_entr_end_31_24 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_39_32_list/* p_cmd_bank_entr_end_39_32 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_47_40_list/* p_cmd_bank_entr_end_47_40 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_55_48_list/* p_cmd_bank_entr_end_55_48 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_63_56_list/* p_cmd_bank_entr_end_63_56 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_end_7_0_list/* p_cmd_bank_entr_end_7_0 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_15_8_list/* p_cmd_bank_entr_middle_15_8 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_23_16_list/* p_cmd_bank_entr_middle_23_16 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_31_24_list/* p_cmd_bank_entr_middle_31_24 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_39_32_list/* p_cmd_bank_entr_middle_39_32 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_47_40_list/* p_cmd_bank_entr_middle_47_40 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_55_48_list/* p_cmd_bank_entr_middle_55_48 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_63_56_list/* p_cmd_bank_entr_middle_63_56 */,
	dai_ipu_top_dfm_p_cmd_bank_entr_middle_7_0_list/* p_cmd_bank_entr_middle_7_0 */,
	dai_ipu_top_dfm_p_dev_port_pairs_num_list/* p_dev_port_pairs_num */,
	dai_ipu_top_dfm_p_dp_en_0_list/* p_dp_en_0 */,
	dai_ipu_top_dfm_p_dp_en_1_list/* p_dp_en_1 */,
	dai_ipu_top_dfm_p_dp_en_10_list/* p_dp_en_10 */,
	dai_ipu_top_dfm_p_dp_en_11_list/* p_dp_en_11 */,
	dai_ipu_top_dfm_p_dp_en_12_list/* p_dp_en_12 */,
	dai_ipu_top_dfm_p_dp_en_13_list/* p_dp_en_13 */,
	dai_ipu_top_dfm_p_dp_en_14_list/* p_dp_en_14 */,
	dai_ipu_top_dfm_p_dp_en_15_list/* p_dp_en_15 */,
	dai_ipu_top_dfm_p_dp_en_16_list/* p_dp_en_16 */,
	dai_ipu_top_dfm_p_dp_en_17_list/* p_dp_en_17 */,
	dai_ipu_top_dfm_p_dp_en_18_list/* p_dp_en_18 */,
	dai_ipu_top_dfm_p_dp_en_19_list/* p_dp_en_19 */,
	dai_ipu_top_dfm_p_dp_en_2_list/* p_dp_en_2 */,
	dai_ipu_top_dfm_p_dp_en_20_list/* p_dp_en_20 */,
	dai_ipu_top_dfm_p_dp_en_21_list/* p_dp_en_21 */,
	dai_ipu_top_dfm_p_dp_en_22_list/* p_dp_en_22 */,
	dai_ipu_top_dfm_p_dp_en_23_list/* p_dp_en_23 */,
	dai_ipu_top_dfm_p_dp_en_24_list/* p_dp_en_24 */,
	dai_ipu_top_dfm_p_dp_en_25_list/* p_dp_en_25 */,
	dai_ipu_top_dfm_p_dp_en_26_list/* p_dp_en_26 */,
	dai_ipu_top_dfm_p_dp_en_27_list/* p_dp_en_27 */,
	dai_ipu_top_dfm_p_dp_en_28_list/* p_dp_en_28 */,
	dai_ipu_top_dfm_p_dp_en_29_list/* p_dp_en_29 */,
	dai_ipu_top_dfm_p_dp_en_3_list/* p_dp_en_3 */,
	dai_ipu_top_dfm_p_dp_en_30_list/* p_dp_en_30 */,
	dai_ipu_top_dfm_p_dp_en_31_list/* p_dp_en_31 */,
	dai_ipu_top_dfm_p_dp_en_4_list/* p_dp_en_4 */,
	dai_ipu_top_dfm_p_dp_en_5_list/* p_dp_en_5 */,
	dai_ipu_top_dfm_p_dp_en_6_list/* p_dp_en_6 */,
	dai_ipu_top_dfm_p_dp_en_7_list/* p_dp_en_7 */,
	dai_ipu_top_dfm_p_dp_en_8_list/* p_dp_en_8 */,
	dai_ipu_top_dfm_p_dp_en_9_list/* p_dp_en_9 */,
	dai_ipu_top_dfm_p_rf_depth_list/* p_rf_depth */,
	dai_ipu_top_dfm_p_sdf_reg_en_0_list/* p_sdf_reg_en_0 */,
	dai_ipu_top_dfm_p_sdf_reg_en_1_list/* p_sdf_reg_en_1 */,
	dai_ipu_top_dfm_p_sdf_reg_en_10_list/* p_sdf_reg_en_10 */,
	dai_ipu_top_dfm_p_sdf_reg_en_11_list/* p_sdf_reg_en_11 */,
	dai_ipu_top_dfm_p_sdf_reg_en_12_list/* p_sdf_reg_en_12 */,
	dai_ipu_top_dfm_p_sdf_reg_en_13_list/* p_sdf_reg_en_13 */,
	dai_ipu_top_dfm_p_sdf_reg_en_14_list/* p_sdf_reg_en_14 */,
	dai_ipu_top_dfm_p_sdf_reg_en_15_list/* p_sdf_reg_en_15 */,
	dai_ipu_top_dfm_p_sdf_reg_en_2_list/* p_sdf_reg_en_2 */,
	dai_ipu_top_dfm_p_sdf_reg_en_3_list/* p_sdf_reg_en_3 */,
	dai_ipu_top_dfm_p_sdf_reg_en_4_list/* p_sdf_reg_en_4 */,
	dai_ipu_top_dfm_p_sdf_reg_en_5_list/* p_sdf_reg_en_5 */,
	dai_ipu_top_dfm_p_sdf_reg_en_6_list/* p_sdf_reg_en_6 */,
	dai_ipu_top_dfm_p_sdf_reg_en_7_list/* p_sdf_reg_en_7 */,
	dai_ipu_top_dfm_p_sdf_reg_en_8_list/* p_sdf_reg_en_8 */,
	dai_ipu_top_dfm_p_sdf_reg_en_9_list/* p_sdf_reg_en_9 */,
	dai_ipu_top_dfm_p_sdf_regs_num_list/* p_sdf_regs_num */
};

/* all property values for property _hwp_level_num for devices of type dvs_controller */
static const dai_property_value8_t dai_ipu_top_dvs_controller__hwp_level_num_list[NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES] = {
	0x3
};

/* all property values for property implementation for devices of type dvs_controller */
static const dai_property_value8_t dai_ipu_top_dvs_controller_implementation_list[NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES] = {
	0x0 /* invalid value in HSD */
};

/* struct with references to all property lists for device type 'CustomDeviceType dvs_controller (0 instances, 1 derived types)' */
static const dai_dev_dvs_controller_property_t dai_ipu_top_all_dvs_controller_properties = {
	dai_ipu_top_dvs_controller__hwp_level_num_list/* _hwp_level_num */,
	dai_ipu_top_dvs_controller_implementation_list/* implementation */
};

/* all property values for property do_not_flatten for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_do_not_flatten_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x1,
	0x1
};

/* all property values for property has_ch_alg_type for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_ch_alg_type_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x3,
	0x3
};

/* all property values for property has_ch_bpp_bmem for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_ch_bpp_bmem_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x3,
	0x3
};

/* all property values for property has_ch_bpp_ddr for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_ch_bpp_ddr_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x3,
	0x3
};

/* all property values for property has_ch_en for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_ch_en_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x1,
	0x1
};

/* all property values for property has_ch_filter_pad for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_ch_filter_pad_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x3,
	0x3
};

/* all property values for property has_ch_frame_ba for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_ch_frame_ba_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x20,
	0x20
};

/* all property values for property has_ch_stride for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_ch_stride_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_cord_val_w for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_cord_val_w_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_dma_ext_unit_height_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_dma_ext_unit_height_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x7,
	0x7
};

/* all property values for property has_dma_ext_unit_width_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_dma_ext_unit_width_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0xD,
	0xD
};

/* all property values for property has_dma_ext_x_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_dma_ext_x_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_dma_ext_y_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_dma_ext_y_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_dma_int_x_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_dma_int_x_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_dma_int_y_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_dma_int_y_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_enable_for_tnr_or_gdc for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_enable_for_tnr_or_gdc_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x0,
	0x1
};

/* all property values for property has_lower_id_w for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_lower_id_w_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x7,
	0x7
};

/* all property values for property has_m_addr_w for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_m_addr_w_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x11,
	0x11
};

/* all property values for property has_max_block_number for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_max_block_number_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x8,
	0x8
};

/* all property values for property has_max_number_dma_requests for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_max_number_dma_requests_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0xF,
	0xF
};

/* all property values for property has_number_channels for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_number_channels_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x4,
	0x4
};

/* all property values for property has_span_bank_size for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_span_bank_size_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_span_ids for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_span_ids_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x80,
	0x80
};

/* all property values for property has_unit_bank_size for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_unit_bank_size_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x4,
	0x4
};

/* all property values for property has_unit_ids for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_unit_ids_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x40,
	0x40
};

/* all property values for property has_x_cl_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_x_cl_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property has_y_cl_width for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_has_y_cl_width_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x10,
	0x10
};

/* all property values for property implementation for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_implementation_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property native_cio2_dblock for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_native_cio2_dblock_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x1,
	0x1
};

/* all property values for property native_cioraccept for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_native_cioraccept_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x1,
	0x1
};

/* all property values for property native_ciosrmd for devices of type mbr */
static const dai_property_value8_t dai_ipu_top_mbr_native_ciosrmd_list[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	0x1,
	0x1
};

/* struct with references to all property lists for device type 'CustomDeviceType mbr (0 instances, 2 derived types)' */
static const dai_dev_mbr_property_t dai_ipu_top_all_mbr_properties = {
	dai_ipu_top_mbr_do_not_flatten_list/* do_not_flatten */,
	dai_ipu_top_mbr_has_ch_alg_type_list/* has_ch_alg_type */,
	dai_ipu_top_mbr_has_ch_bpp_bmem_list/* has_ch_bpp_bmem */,
	dai_ipu_top_mbr_has_ch_bpp_ddr_list/* has_ch_bpp_ddr */,
	dai_ipu_top_mbr_has_ch_en_list/* has_ch_en */,
	dai_ipu_top_mbr_has_ch_filter_pad_list/* has_ch_filter_pad */,
	dai_ipu_top_mbr_has_ch_frame_ba_list/* has_ch_frame_ba */,
	dai_ipu_top_mbr_has_ch_stride_list/* has_ch_stride */,
	dai_ipu_top_mbr_has_cord_val_w_list/* has_cord_val_w */,
	dai_ipu_top_mbr_has_dma_ext_unit_height_width_list/* has_dma_ext_unit_height_width */,
	dai_ipu_top_mbr_has_dma_ext_unit_width_width_list/* has_dma_ext_unit_width_width */,
	dai_ipu_top_mbr_has_dma_ext_x_width_list/* has_dma_ext_x_width */,
	dai_ipu_top_mbr_has_dma_ext_y_width_list/* has_dma_ext_y_width */,
	dai_ipu_top_mbr_has_dma_int_x_width_list/* has_dma_int_x_width */,
	dai_ipu_top_mbr_has_dma_int_y_width_list/* has_dma_int_y_width */,
	dai_ipu_top_mbr_has_enable_for_tnr_or_gdc_list/* has_enable_for_tnr_or_gdc */,
	dai_ipu_top_mbr_has_lower_id_w_list/* has_lower_id_w */,
	dai_ipu_top_mbr_has_m_addr_w_list/* has_m_addr_w */,
	dai_ipu_top_mbr_has_max_block_number_list/* has_max_block_number */,
	dai_ipu_top_mbr_has_max_number_dma_requests_list/* has_max_number_dma_requests */,
	dai_ipu_top_mbr_has_number_channels_list/* has_number_channels */,
	dai_ipu_top_mbr_has_span_bank_size_list/* has_span_bank_size */,
	dai_ipu_top_mbr_has_span_ids_list/* has_span_ids */,
	dai_ipu_top_mbr_has_unit_bank_size_list/* has_unit_bank_size */,
	dai_ipu_top_mbr_has_unit_ids_list/* has_unit_ids */,
	dai_ipu_top_mbr_has_x_cl_width_list/* has_x_cl_width */,
	dai_ipu_top_mbr_has_y_cl_width_list/* has_y_cl_width */,
	dai_ipu_top_mbr_implementation_list/* implementation */,
	dai_ipu_top_mbr_native_cio2_dblock_list/* native_cio2_dblock */,
	dai_ipu_top_mbr_native_cioraccept_list/* native_cioraccept */,
	dai_ipu_top_mbr_native_ciosrmd_list/* native_ciosrmd */
};

/* all property values for property hwp_ack_cmd_range for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_ack_cmd_range_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x12,
	0x12,
	0x12
};

/* all property values for property hwp_ack_fifo_depth for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_ack_fifo_depth_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x4,
	0x4,
	0x4
};

/* all property values for property hwp_cmd_fifo_depth for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_cmd_fifo_depth_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x4,
	0x4,
	0x4
};

/* all property values for property hwp_nof_bufs for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_nof_bufs_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x4,
	0x6,
	0x4
};

/* all property values for property hwp_nof_strm_comps for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_nof_strm_comps_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x6,
	0xC,
	0x6
};

/* all property values for property hwp_pif_output_support for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_pif_output_support_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x0,
	0x0,
	0x0
};

/* all property values for property hwp_strm_comp_w for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_strm_comp_w_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x10,
	0x10,
	0x10
};

/* all property values for property hwp_strm_fifo_depth for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_strm_fifo_depth_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0xB,
	0xE,
	0xB
};

/* all property values for property hwp_vec_comp_w for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_hwp_vec_comp_w_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x10,
	0x10,
	0x10
};

/* all property values for property implementation for devices of type vec_to_str_v3 */
static const dai_property_value8_t dai_ipu_top_vec_to_str_v3_implementation_list[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* struct with references to all property lists for device type 'CustomDeviceType vec_to_str_v3 (0 instances, 1 derived types)' */
static const dai_dev_vec_to_str_v3_property_t dai_ipu_top_all_vec_to_str_v3_properties = {
	dai_ipu_top_vec_to_str_v3_hwp_ack_cmd_range_list/* hwp_ack_cmd_range */,
	dai_ipu_top_vec_to_str_v3_hwp_ack_fifo_depth_list/* hwp_ack_fifo_depth */,
	dai_ipu_top_vec_to_str_v3_hwp_cmd_fifo_depth_list/* hwp_cmd_fifo_depth */,
	dai_ipu_top_vec_to_str_v3_hwp_nof_bufs_list/* hwp_nof_bufs */,
	dai_ipu_top_vec_to_str_v3_hwp_nof_strm_comps_list/* hwp_nof_strm_comps */,
	dai_ipu_top_vec_to_str_v3_hwp_pif_output_support_list/* hwp_pif_output_support */,
	dai_ipu_top_vec_to_str_v3_hwp_strm_comp_w_list/* hwp_strm_comp_w */,
	dai_ipu_top_vec_to_str_v3_hwp_strm_fifo_depth_list/* hwp_strm_fifo_depth */,
	dai_ipu_top_vec_to_str_v3_hwp_vec_comp_w_list/* hwp_vec_comp_w */,
	dai_ipu_top_vec_to_str_v3_implementation_list/* implementation */
};

/* all property values for property implementation for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_implementation_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */,
	0x0 /* invalid value in HSD */
};

/* all property values for property native_cio2_dblock for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_native_cio2_dblock_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

/* all property values for property native_cioraccept for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_native_cioraccept_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0x1,
	0x1,
	0x1,
	0x1,
	0x1,
	0x1,
	0x1,
	0x1,
	0x1
};

/* all property values for property native_ciosrmd for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_native_ciosrmd_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

/* all property values for property num_watch_points for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_num_watch_points_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0x8,
	0x8,
	0x4,
	0x10,
	0x8,
	0x8,
	0x8,
	0x8,
	0x8
};

/* all property values for property trace_ddren_bit for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_trace_ddren_bit_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0xF,
	0xF,
	0xF,
	0xF,
	0xF,
	0xF,
	0xF,
	0xF,
	0xF
};

/* all property values for property trace_depth for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_trace_depth_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0x10,
	0x10,
	0x10,
	0x10,
	0x10,
	0x10,
	0x10,
	0x10,
	0x10
};

/* all property values for property trace_entry_depth for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_trace_entry_depth_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0x2,
	0x2,
	0x2,
	0x2,
	0x2,
	0x2,
	0x2,
	0x2,
	0x2
};

/* all property values for property trace_npken_bit for devices of type wpt */
static const dai_property_value8_t dai_ipu_top_wpt_trace_npken_bit_list[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	0xE,
	0xE,
	0xE,
	0xE,
	0xE,
	0xE,
	0xE,
	0xE,
	0xE
};

/* struct with references to all property lists for device type 'CustomDeviceType wpt (0 instances, 5 derived types)' */
static const dai_dev_wpt_property_t dai_ipu_top_all_wpt_properties = {
	dai_ipu_top_wpt_implementation_list/* implementation */,
	dai_ipu_top_wpt_native_cio2_dblock_list/* native_cio2_dblock */,
	dai_ipu_top_wpt_native_cioraccept_list/* native_cioraccept */,
	dai_ipu_top_wpt_native_ciosrmd_list/* native_ciosrmd */,
	dai_ipu_top_wpt_num_watch_points_list/* num_watch_points */,
	dai_ipu_top_wpt_trace_ddren_bit_list/* trace_ddren_bit */,
	dai_ipu_top_wpt_trace_depth_list/* trace_depth */,
	dai_ipu_top_wpt_trace_entry_depth_list/* trace_entry_depth */,
	dai_ipu_top_wpt_trace_npken_bit_list/* trace_npken_bit */
};

/* contains all user properties of the system */
static const dai_device_user_properties_t dai_ipu_top_user_properties = {
	&dai_ipu_top_all_bb_output_formatter_properties/* all_bb_output_formatter_properties */,
	&dai_ipu_top_all_bbgdc4_properties/* all_bbgdc4_properties */,
	&dai_ipu_top_all_bbpolyphase_scaler_properties/* all_bbpolyphase_scaler_properties */,
	&dai_ipu_top_all_bbtnr_properties/* all_bbtnr_properties */,
	&dai_ipu_top_all_cell_properties/* all_cell_properties */,
	&dai_ipu_top_all_dfm_properties/* all_dfm_properties */,
	&dai_ipu_top_all_dvs_controller_properties/* all_dvs_controller_properties */,
	&dai_ipu_top_all_mbr_properties/* all_mbr_properties */,
	&dai_ipu_top_all_vec_to_str_v3_properties/* all_vec_to_str_v3_properties */,
	&dai_ipu_top_all_wpt_properties/* all_wpt_properties */
};

/* contains all properties of the system */
static const dai_subsystem_properties_t dai_ipu_top_properties = {
	NUM_DAI_DEVICE_TYPES/* device_num_types */,
	dai_ipu_top_num_instances/* device_num_instances */,
	dai_ipu_top_num_slave_ports/* device_num_slave_ports */,
	dai_ipu_top_num_master_ports/* device_num_master_ports */,
	dai_ipu_top_num_memories_ports/* device_num_memories */,
	dai_ipu_top_num_registersbanks/* device_num_registerbanks */,
	dai_ipu_top_num_registers/* device_num_registers */,
	&dai_ipu_top_user_properties/* device_user_properties */
};

#endif /* _dai_ipu_top_device_info_h_ */
