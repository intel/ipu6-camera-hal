/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2019 Intel Corporation.
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

#ifndef __NCI_DEC400_INLINE_H
#define __NCI_DEC400_INLINE_H

#include <assert_support.h>
#include <type_support.h>

#include "nci_dec400.h"
#include "nci_dec400_struct.h"
#include "nci_dec400_vpless_defs.h"
#include "nci_dec400_regs_defs.h"
#include "nci_dec400_storage_class.h"
#include "nci_dec400_reg_access.h"
#include "nci_dec400_inline_dep.h"

#ifdef __INLINE_NCI_DEC400__
extern uint32_t shadow_cfg_val[NCI_DEC400_NUM_DEVICES];
#endif

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section0_config(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section1_exconfig(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section2_buffer_base(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section3_buffer_end(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section4_buffer_cache(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section5_stride(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section6_commit(
	unsigned int channel,
	void *buffer);

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_generic_section(
	void *buffer,
	uint32_t reg_val);

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_get_sizeof_channel_blob(void)
{
	return (NCI_DEC400_FILL_SECTIONS_CHANNEL) * sizeof(uint32_t);
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_get_sizeof_buffer_blob(void)
{
	return (NCI_DEC400_FILL_SECTIONS_BUFFER) * sizeof(uint32_t);
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_get_sizeof_blob_per_stream(void)
{
	assert(NCI_DEC400_FILL_SECTIONS_NOF ==
		   (NCI_DEC400_FILL_SECTIONS_CHANNEL + NCI_DEC400_FILL_SECTIONS_BUFFER));
	return NCI_DEC400_FILL_SECTIONS_NOF * sizeof(uint32_t);
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_get_sizeof_connected_blob_per_stream(void)
{
	return NCI_DEC400_FILL_LOAD_SECTION_NOF * sizeof(uint32_t);
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_get_reg_address_by_section(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	enum nci_dec400_fill_sections section)
{
	uint32_t addr = 0;

	switch (section) {
	case NCI_DEC400_FILL_SECTIONS_CONFIG:
		addr = ipu_device_dec400_get_reg_config_addr(dev_id, channel);
		break;
	case NCI_DEC400_FILL_SECTIONS_EX_CONFIG:
		addr = ipu_device_dec400_get_reg_exConfig_addr(dev_id, channel);
		break;
	case NCI_DEC400_FILL_SECTIONS_BUFFER_BASE:
		addr = ipu_device_dec400_get_reg_buffer_base_addr(dev_id, channel);
		break;
	case NCI_DEC400_FILL_SECTIONS_BUFFER_END:
		addr = ipu_device_dec400_get_reg_buffer_end_addr(dev_id, channel);
		break;
	case NCI_DEC400_FILL_SECTIONS_BUFFER_CACHE:
		addr = ipu_device_dec400_get_reg_buffer_cache_addr(dev_id, channel);
		break;
	case NCI_DEC400_FILL_SECTIONS_STRIDE:
		addr = ipu_device_dec400_get_reg_stride_addr(dev_id, channel);
		break;
	case NCI_DEC400_FILL_SECTIONS_COMMIT:
		addr = ipu_device_dec400_get_reg_commit_addr(dev_id);
		break;
	case NCI_DEC400_FILL_SECTIONS_NOF:
		break;
	}

	return addr;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_get_dec_info(
	nci_dec400_device_id dev_id,
	struct nci_dec400_chip_info_t *dec_info)
{
	uint32_t addr;
	union nci_dec400_register_chip_info chip_info;
	union nci_dec400_register_product_id product_id;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(dec_info != NULL);

	/* DEC400_REG_GCCHIP_REV */
	addr = (DEC400_REG_GCCHIP_REV << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT);
	addr += ipu_device_dec400_get_base_addr(dev_id);
	dec_info->chip_rev = nci_dec400_reg_load(addr);

	/* DEC400_REG_GCCHIP_DATE */
	addr += (1 << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT);
	dec_info->chip_date = nci_dec400_reg_load(addr);

	/* DEC400_REG_GCREG_HICHIP_PATCH_REV */
	addr += (1 << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT);
	dec_info->chip_patch_revision = nci_dec400_reg_load(addr);

	/* DEC400_REG_GC_CHIP_INFO */
	addr += (1 << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT);
	chip_info.data = nci_dec400_reg_load(addr);
	dec_info->engine_dec = chip_info.data_bits.engine_dec;
	dec_info->one_set_bus = chip_info.data_bits.one_set_bus;
	dec_info->two_set_bus = chip_info.data_bits.two_set_bus;
	dec_info->interface_ahb = chip_info.data_bits.interface_ahb;
	dec_info->one_gpu = chip_info.data_bits.one_gpu;
	dec_info->bus64_bits = chip_info.data_bits.bus64_bits;
	dec_info->bus128_bits = chip_info.data_bits.bus128_bits;
	dec_info->dec = chip_info.data_bits.dec;
	dec_info->dec_d = chip_info.data_bits.dec_d;
	dec_info->dec_e = chip_info.data_bits.dec_e;

	/* DEC400_REG_GC_PRODUCT_ID */
	addr += (1 << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT);
	product_id.data = nci_dec400_reg_load(addr);
	dec_info->grade_level = product_id.data_bits.grade_level;
	dec_info->product_type = product_id.data_bits.product_type;
	dec_info->product_num = product_id.data_bits.product_num;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section0_config(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	union nci_dec400_register_config config_reg;
	uint32_t *fill_buffer = (uint32_t *)buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	config_reg.data = 0;
	config_reg.data_bits.compression_enable = cfg->compression_enable;
	config_reg.data_bits.compression_format = cfg->compression_format;
	config_reg.data_bits.compression_align_mode = cfg->compression_align_mode;
	config_reg.data_bits.tile_align_mode = cfg->tile_align_mode;
	config_reg.data_bits.tile_mode = cfg->tile_mode;

	IA_CSS_TRACE_2(NCI_DEC400, DEBUG,
	"nci_dec400_fill_section0_config || {tile_mode: %u, config.data: 0x%08x}\n",
	config_reg.data_bits.tile_mode, config_reg.data);

	*fill_buffer = config_reg.data;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section1_exconfig(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	union nci_dec400_register_ex_config ex_config_reg;
	uint32_t *fill_buffer = (uint32_t *)buffer;

	assert(cfg != NULL);
	assert(buffer);

	ex_config_reg.data = 0;
	ex_config_reg.data_bits.cbsr_width = cfg->cbsr_width;
	ex_config_reg.data_bits.bit_depth = cfg->bit_depth;
	ex_config_reg.data_bits.tile_y = cfg->tile_y;
	ex_config_reg.data_bits.stream_mode = cfg->stream_mode.data;
	ex_config_reg.data_bits.ts_cache_mode = cfg->ts_cache_mode;
	ex_config_reg.data_bits.pixel_cache_replacement = cfg->pixel_cache_replacement;
	ex_config_reg.data_bits.intel_p010 = cfg->intel_p010;
	ex_config_reg.data_bits.ts_cache_replacement = cfg->ts_cache_replacement;

	IA_CSS_TRACE_2(NCI_DEC400, DEBUG,
				   "nci_dec400_fill_section1_exconfig || {stream_mode: %u, ex_config.data: 0x%08x}\n",
				   ex_config_reg.data_bits.stream_mode, ex_config_reg.data);

	*fill_buffer = ex_config_reg.data;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section2_buffer_base(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	IA_CSS_TRACE_1(NCI_DEC400, DEBUG,
				   "nci_dec400_fill_section2_buffer_base || {buffer_base.data: 0x%08x}\n",
				   cfg->buffer_base);

	nci_dec400_fill_generic_section(buffer, cfg->buffer_base);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section3_buffer_end(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	IA_CSS_TRACE_1(NCI_DEC400, DEBUG,
				   "nci_dec400_fill_section3_buffer_end || {buffer_end.data: 0x%08x}\n",
				   cfg->buffer_end);

	nci_dec400_fill_generic_section(buffer, cfg->buffer_end);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section4_buffer_cache(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	IA_CSS_TRACE_1(NCI_DEC400, DEBUG,
				   "nci_dec400_fill_section4_buffer_cache || {cache_base.data: 0x%08x}\n",
				   cfg->cache_base);

	nci_dec400_fill_generic_section(buffer, cfg->cache_base);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section5_stride(
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	IA_CSS_TRACE_1(NCI_DEC400, DEBUG,
				   "nci_dec400_fill_section5_stride || {buffer_stride.data: 0x%08x}\n",
				   cfg->buffer_stride);

	nci_dec400_fill_generic_section(buffer, cfg->buffer_stride);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_section6_commit(
	unsigned int channel,
	void *buffer)
{
	nci_dec400_fill_generic_section(buffer, (1 << channel));
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_channel_config_fill_sections(
	nci_dec400_device_id dev_id,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	uint32_t *fill_buffer = (uint32_t *)buffer;

	assert(cfg != NULL);
	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	nci_dec400_fill_section1_exconfig(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_EX_CONFIG]);
	nci_dec400_fill_section5_stride(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_STRIDE]);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_buffer_config_fill_sections(
	nci_dec400_device_id dev_id,
	unsigned int channel,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	uint32_t *fill_buffer = (uint32_t *)buffer;

	assert(cfg != NULL);
	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	nci_dec400_fill_section0_config(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_CONFIG]);
	nci_dec400_fill_section2_buffer_base(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_BUFFER_BASE]);
	nci_dec400_fill_section3_buffer_end(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_BUFFER_END]);
	nci_dec400_fill_section4_buffer_cache(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_BUFFER_CACHE]);
	nci_dec400_fill_section6_commit(channel, &fill_buffer[NCI_DEC400_FILL_SECTIONS_COMMIT]);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_fill_generic_section(
	void *buffer,
	uint32_t reg_val)
{
	uint32_t *fill_buffer = (uint32_t *)buffer;

	assert(buffer != NULL);

	*fill_buffer = reg_val;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_config_interrupt_enable(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	bool enable)
{
	uint32_t intEn_address;
	uint32_t val;
	uint32_t val_mask;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(channel < IPU_DEVICE_DEC400_NUM_OF_CHANNELS);

	if (channel <= IPU_DEVICE_DEC400_INTR_EN_MAX_CHANNEL) {
		val_mask = (1 << channel);

		intEn_address = DEC400_REG_GCREG_AHBDECINTR_ENBL;
		intEn_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
		intEn_address += ipu_device_dec400_get_base_addr(dev_id);
		val = nci_dec400_reg_load(intEn_address);
		/* TODO: Need to deal with error interrupts */

		if (enable) {
			val |= val_mask;
		} else {
			val &= (~val_mask);
		}
		nci_dec400_reg_store(intEn_address, val);
	} else {
		nci_dec400_config_interrupt_ex_enable(dev_id, channel, enable);
	}
}

NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_config_interrupt_enable_all_channels(
	nci_dec400_device_id dev_id,
	bool enable)
{
	uint32_t intEn_address;
	uint32_t val_mask = (enable ? 0x7FFFFFFF : 0);

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	intEn_address = DEC400_REG_GCREG_AHBDECINTR_ENBL;
	intEn_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	intEn_address += ipu_device_dec400_get_base_addr(dev_id);

	nci_dec400_reg_store(intEn_address, val_mask);
}

#define NCI_DEC400_IRQ_FIRST_ERROR_BIT NCI_DEC400_IRQ_SHADOW_REGS_BIT

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_clear_interrupt_status(
	nci_dec400_device_id dev_id,
	uint32_t *error_irq_mask)
{
	uint32_t intAck_address;
	uint32_t val;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(NULL != error_irq_mask);

	intAck_address = DEC400_REG_GCREG_AHBDECINTR_ACKNOWLEDGE;
	intAck_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	intAck_address += ipu_device_dec400_get_base_addr(dev_id);
	val = nci_dec400_reg_load(intAck_address);
	*error_irq_mask = val & NCI_DEC400_IRQ_ERRORS_MASK;
	val &= (~NCI_DEC400_IRQ_ERRORS_MASK);

	/* TODO: Need to deal with new extended interrupt register.  Not important now since
	   we don't use compression stream ID 30 and 31. */

	return val;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_control_config(
	nci_dec400_device_id dev_id,
	struct nci_dec400_control_cfg_t *cfg)
{
	uint32_t control_address;
	uint32_t controlEx_address;
	uint32_t base_address;
	union nci_dec400_register_control control_cfg;
	union nci_dec400_register_control_ex control_ex_cfg;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(cfg != NULL);

	base_address = ipu_device_dec400_get_base_addr(dev_id);

	control_address = DEC400_REG_GCREG_AHBDECCONTROL;
	control_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	control_address += base_address;

	controlEx_address = DEC400_REG_GCREG_AHBDECCONTROL_EX;
	controlEx_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	controlEx_address += base_address;

	control_cfg.data = 0;
	control_cfg.data_bits.disable_compression = cfg->disable_compression;
	control_cfg.data_bits.disable_ram_clock_gating = cfg->disable_ram_clock_gating;
	control_cfg.data_bits.disable_debug_registers = cfg->disable_debug_registers;
	control_cfg.data_bits.tile_status_read_id = cfg->tile_status_read_id;
	control_cfg.data_bits.tile_status_write_id = cfg->tile_status_write_id;
	control_cfg.data_bits.disable_hw_flush = cfg->disable_hw_flush;
	control_cfg.data_bits.clk_dis = cfg->clk_dis;
	control_cfg.data_bits.disable_module_clock_gating = cfg->disable_module_clock_gating;

	control_ex_cfg.data = 0;
	control_ex_cfg.data_bits.axuser = cfg->axuser;
	control_ex_cfg.data_bits.axqos = cfg->axqos;
	control_ex_cfg.data_bits.enable_burst_split = cfg->enable_burst_split;
	control_ex_cfg.data_bits.enable_end_address_range_check = cfg->enable_end_address_range_check;
	shadow_cfg_val[dev_id] = control_cfg.data;

	nci_dec400_reg_store(control_address, control_cfg.data);
	nci_dec400_reg_store(controlEx_address, control_ex_cfg.data);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_soft_reset(
	nci_dec400_device_id dev_id)
{
	uint32_t control_address;
	uint32_t base_address;
	union nci_dec400_register_control control_cfg;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	base_address = ipu_device_dec400_get_base_addr(dev_id);

	control_address = DEC400_REG_GCREG_AHBDECCONTROL;
	control_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	control_address += base_address;

	control_cfg.data = shadow_cfg_val[dev_id];

	control_cfg.data_bits.soft_reset = 1;
	nci_dec400_reg_store(control_address, control_cfg.data);

	control_cfg.data_bits.soft_reset = 0;
	nci_dec400_reg_store(control_address, control_cfg.data);
	shadow_cfg_val[dev_id] = control_cfg.data;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_flush(
	nci_dec400_device_id dev_id,
	uint8_t channel)
{
	uint32_t control_address;
	uint32_t base_address;
	union nci_dec400_register_control control_cfg;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(channel < IPU_DEVICE_DEC400_NUM_OF_CHANNELS);

	base_address = ipu_device_dec400_get_base_addr(dev_id);

	control_address = DEC400_REG_GCREG_AHBDECCONTROL;
	control_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	control_address += base_address;

	control_cfg.data = shadow_cfg_val[dev_id];
	control_cfg.data_bits.sw_flush_id = channel;
	control_cfg.data_bits.flush = 1;

	nci_dec400_reg_store(control_address, control_cfg.data);

	control_cfg.data_bits.flush = 0;
	control_cfg.data_bits.sw_flush_id = 0;
	shadow_cfg_val[dev_id] = control_cfg.data;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_read_debug_info(
	nci_dec400_device_id dev_id,
	struct nci_dec400_debug_info_t *info)
{
	uint32_t debug_address;
	uint32_t base_address;
	uint32_t inc_addr;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(info != NULL);

	base_address = ipu_device_dec400_get_base_addr(dev_id);
	inc_addr = 1 << IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;

	debug_address = DEC400_REG_GCREG_AHBDECTILE_STATUS_DEBUG;
	debug_address <<= IPU_DEVICE_DEC400_REG_OFFSET_TO_BYTES_SHIFT;
	debug_address += base_address;
	info->tile_status_debug = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->encoder_debug = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->decoder_debug = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_reads_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_writes_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_read_bursts_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_write_bursts_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_reads_req_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_writes_req_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_read_lasts_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_write_lasts_in = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_reads_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_writes_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_read_bursts_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_write_bursts_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_reads_req_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_writes_req_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_read_lasts_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->total_write_lasts_out = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->debug0 = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->debug1 = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->debug2 = nci_dec400_reg_load(debug_address);

	debug_address += inc_addr;
	info->debug3 = nci_dec400_reg_load(debug_address);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_set_gp_wrapper_regs(
	nci_dec400_device_id dev_id,
	uint8_t axi_id,
	bool path_en)
{
	uint32_t base_address;

	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	base_address = ipu_device_dec400_get_gp_reg_axi_id_addr(dev_id);
	nci_dec400_reg_store(base_address, axi_id);

	base_address = ipu_device_dec400_get_gp_reg_path_en_addr(dev_id);
	nci_dec400_reg_store(base_address, path_en);
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_irq_wrapper_regs_setup(uint32_t enable_mask,
									   uint32_t edge_mask,
									   uint32_t level_n_pulse_mask)
{
	uint32_t address;

	IA_CSS_TRACE_3(NCI_DEC400, INFO,
				   "nci_dec400_irq_wrapper_regs_setup || {enable_mask: %d, edge_mask: %d, level_n_pulse_mask: %d}\n",
				   enable_mask, edge_mask, level_n_pulse_mask);

	/* to trigger interrupts with input rising edges */
	address = ipu_device_dec400_get_irq_reg_addr(DEC400_IRQ_REG_EDGE);
	nci_dec400_reg_store(address, edge_mask);

	/* to generate a constant level ?1? to signal an interrupt */
	address = ipu_device_dec400_get_irq_reg_addr(DEC400_IRQ_REG_LEVEL_NOT_PULSE);
	nci_dec400_reg_store(address, level_n_pulse_mask);

	/* allow interrupts to affect the status register */
	address = ipu_device_dec400_get_irq_reg_addr(DEC400_IRQ_REG_MASK);
	nci_dec400_reg_store(address, enable_mask);

	/* allow the interrupt to reach the IRQ output of IC (Interrupt Controller) */
	address = ipu_device_dec400_get_irq_reg_addr(DEC400_IRQ_REG_ENABLE);
	nci_dec400_reg_store(address, enable_mask);
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_irq_wrapper_regs_status(void)
{
	uint32_t address = ipu_device_dec400_get_irq_reg_addr(DEC400_IRQ_REG_STATUS);
	uint32_t irq_mask = nci_dec400_reg_load(address);

	return irq_mask;
}

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_irq_wrapper_regs_clear(uint32_t irq_mask)
{
	uint32_t address = ipu_device_dec400_get_irq_reg_addr(DEC400_IRQ_REG_CLEAR);

	nci_dec400_reg_store(address, irq_mask);
}

#include "nci_dec400_inline_local.h"

#endif /* __NCI_DEC400_INLINE_H */
