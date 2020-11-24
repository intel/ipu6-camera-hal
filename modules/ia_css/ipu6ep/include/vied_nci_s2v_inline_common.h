/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __VIED_NCI_S2V_INLINE_COMMON_H
#define __VIED_NCI_S2V_INLINE_COMMON_H

#include "vied_nci_s2v.h"
#include "vied_nci_s2v_types.h"             /* for nci_s2v_ack_cfg, nci_s2v_ack_cfg::address,    */
#include "vied_nci_s2v_private_types.h"     /* for nci_s2v_private_cfg, nci_s2v_private_ack_cfg  */
#include "vied_nci_s2v_storage_class.h"     /* for NCI_S2V_STORAGE_CLASS_C                       */
#include "vied_nci_s2v_trace.h"
#include "str_to_vec_v2_3_defs.h"
#include "assert_support.h"                 /* for assert                                        */
#include "type_support.h"                   /* for NULL, uint32_t                                */
#include "ipu_device_eq_properties.h"       /* for ipu_device_eq_num_devices, ipu_device_eq_msg_size, ipu_device_eq_... */
#include "misc_support.h"

/* Prototypes */
NCI_S2V_STORAGE_CLASS_C
int nci_s2v_fill_section0(
	const struct nci_s2v_cfg *public_cfg,
	void *buffer);

NCI_S2V_STORAGE_CLASS_C
int nci_s2v_fill_section1(
	const struct nci_s2v_ack_cfg *public_cfg,
	void *buffer);

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_print_section0(void *buffer);

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_print_section1(void *buffer);

NCI_S2V_STORAGE_CLASS_C
uint32_t nci_s2v_pack_ack_cmd_info(uint32_t sid, uint32_t pid, uint32_t msg);

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_fill_section0_dep(
	struct nci_s2v_private_cfg *private_cfg,
	const struct nci_s2v_cfg *public_cfg);

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_print_section0_dep(struct nci_s2v_private_cfg *private_cfg);

/* Implementation */
NCI_S2V_STORAGE_CLASS_C
int nci_s2v_get_numberof_sections(void)
{
	return NCI_S2V_NUM_SECTIONS;
}

NCI_S2V_STORAGE_CLASS_C
int nci_s2v_get_sizeof_section(enum nci_s2v_section_id id)
{
	int size = -1;

	if (id == NCI_S2V_SECTION0) {
		size = sizeof(struct nci_s2v_private_cfg);
	} else if (id == NCI_S2V_SECTION1) {
		size = sizeof(struct nci_s2v_private_ack_cfg);
	} else {
		IA_CSS_TRACE_0(VIED_NCI_S2V, ERROR, "nci_s2v_get_sizeof_section: Invalid section id\n");
		assert(0);
	}

	return size;
}

NCI_S2V_STORAGE_CLASS_C
int nci_s2v_get_sizeof_sections(void)
{
	return nci_s2v_get_sizeof_section(NCI_S2V_SECTION0)
		+ nci_s2v_get_sizeof_section(NCI_S2V_SECTION1);
}

NCI_S2V_STORAGE_CLASS_C
int nci_s2v_get_offsetof_section(enum nci_s2v_section_id id)
{
	int offset = -1;

	if (id == NCI_S2V_SECTION0) {
		offset = _STR_TO_VEC_V2_3_REG_ALIGN * _STR_TO_VEC_V2_3_ACK_K_VEC_REG;
	} else if (id == NCI_S2V_SECTION1) {
		offset = 0;
	} else {
		IA_CSS_TRACE_0(VIED_NCI_S2V, ERROR, "nci_s2v_get_offsetof_section: Invalid section id\n");
		assert(0);
	}

	return offset;
}

NCI_S2V_STORAGE_CLASS_C
uint32_t nci_s2v_pack_ack_cmd_info(uint32_t sid, uint32_t pid, uint32_t msg)
{
	/* EQ FW implementation assumes that all EQs have the same event token formatting */
	unsigned int eq_id = 0;

	assert(sid < (1u << ipu_device_eq_sid_size(eq_id)));
	assert(pid < (1u << ipu_device_eq_pid_size(eq_id)));
	assert(msg < (1u << ipu_device_eq_msg_size(eq_id)));

	return (((sid << ipu_device_eq_pid_size(eq_id)) + pid) << ipu_device_eq_msg_size(eq_id)) + msg;
}

NCI_S2V_STORAGE_CLASS_C
int nci_s2v_fill_section0(
	const struct nci_s2v_cfg *public_cfg,
	void *buffer)
{
	unsigned int i = 0;
	struct nci_s2v_private_cfg *private_cfg = (struct nci_s2v_private_cfg *) buffer;

	assert(public_cfg != NULL);
	assert(buffer != NULL);

	private_cfg->ack_k_vec		= public_cfg->ack_vec_nr;
	private_cfg->pxlcmp_line	= public_cfg->pxlcmp_per_line;
	private_cfg->lines_frame	= public_cfg->lines_per_frame;
	private_cfg->yuv420_en		= public_cfg->yuv420_enable;
	private_cfg->interleave_en	= public_cfg->interleave_enable;
	private_cfg->dev_null_enable	= public_cfg->dev_null_enable;
	private_cfg->unused_0		= 0;

	assert(public_cfg->number_of_buffers <= NCI_N_S2V_BUFFERS);
	assert(public_cfg->buffers != NULL);

	for (i = 0; i < NCI_N_S2V_BUFFERS; i++) {
		if (i < public_cfg->number_of_buffers) {
			private_cfg->buff_cfg[i].start_address	= public_cfg->buffers[i].start_address;
			private_cfg->buff_cfg[i].end_address	= public_cfg->buffers[i].end_address;
			private_cfg->buff_cfg[i].offset		= public_cfg->buffers[i].offset;
			private_cfg->buff_cfg[i].offset1	= public_cfg->buffers[i].offset1;
			private_cfg->buff_cfg[i].stride		= public_cfg->buffers[i].stride;
		} else {
			private_cfg->buff_cfg[i].start_address	= 0xCAFEBABE; /* for debugging */
			private_cfg->buff_cfg[i].end_address	= 0xCAFEBABE;
			private_cfg->buff_cfg[i].offset		= 0xCAFEBABE;
			private_cfg->buff_cfg[i].offset1	= 0xCAFEBABE;
			private_cfg->buff_cfg[i].stride		= 0xCAFEBABE;
		}
		private_cfg->unused_1[i] = 0;
	}

	private_cfg->unused_2 = 0;
	private_cfg->unused_3 = 0;
	private_cfg->unused_4 = 0;
	private_cfg->unused_5 = 0;
	private_cfg->unused_6 = 0;
	private_cfg->unused_7 = 0;
	private_cfg->unused_8 = 0;

	private_cfg->ack_rpwc_en		= public_cfg->rpwc_enable;
	private_cfg->buf_addr_byte_aligned	= public_cfg->buf_addr_byte_aligned;
	private_cfg->stride_on_eof_cmd_en	= public_cfg->stride_on_eof_cmd_enable;
	private_cfg->bayer_mode_en		= public_cfg->bayer_mode_enable;

	nci_s2v_fill_section0_dep(private_cfg, public_cfg);

	return sizeof(struct nci_s2v_private_cfg);
}

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_print_section0(void *buffer)
{
	int i = 0;
	struct nci_s2v_private_cfg *private_cfg = (struct nci_s2v_private_cfg *) buffer;

	assert(buffer != NULL);

	IA_CSS_TRACE_0(VIED_NCI_S2V, DEBUG, "nci_s2v_print_section0:\n");
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tack_k_vec = 0x%08x\n", private_cfg->ack_k_vec);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tpxlcmp_line = 0x%08x\n", private_cfg->pxlcmp_line);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tlines_frame = 0x%08x\n", private_cfg->lines_frame);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tyuv420_en = 0x%08x\n", private_cfg->yuv420_en);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tinterleave_en = 0x%08x\n", private_cfg->interleave_en);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tdev_null_enable = 0x%08x\n", private_cfg->dev_null_enable);

	for (i = 0; i < NCI_N_S2V_BUFFERS; i++) {
		IA_CSS_TRACE_2(VIED_NCI_S2V, DEBUG, "\tbuff_cfg[%d].start_address = 0x%08x\n", i, private_cfg->buff_cfg[i].start_address);
		IA_CSS_TRACE_2(VIED_NCI_S2V, DEBUG, "\tbuff_cfg[%d].end_address = 0x%08x\n", i, private_cfg->buff_cfg[i].end_address);
		IA_CSS_TRACE_2(VIED_NCI_S2V, DEBUG, "\tbuff_cfg[%d].offset = 0x%08x\n", i, private_cfg->buff_cfg[i].offset);
		IA_CSS_TRACE_2(VIED_NCI_S2V, DEBUG, "\tbuff_cfg[%d].offset1 = 0x%08x\n", i, private_cfg->buff_cfg[i].offset1);
		IA_CSS_TRACE_2(VIED_NCI_S2V, DEBUG, "\tbuff_cfg[%d].stride = 0x%08x\n", i, private_cfg->buff_cfg[i].stride);
	}

	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tack_rpwc_en = 0x%08x\n", private_cfg->ack_rpwc_en);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tbuf_addr_byte_aligned = 0x%08x\n", private_cfg->buf_addr_byte_aligned);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tstride_on_eof_cmd_en = 0x%08x\n", private_cfg->stride_on_eof_cmd_en);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tbayer_mode_en = 0x%08x\n", private_cfg->bayer_mode_en);

	nci_s2v_print_section0_dep(private_cfg);

	NOT_USED(private_cfg);
}

NCI_S2V_STORAGE_CLASS_C
int nci_s2v_fill_section1(
	const struct nci_s2v_ack_cfg *public_cfg,
	void *buffer)
{
	struct nci_s2v_private_ack_cfg *private_cfg = (struct nci_s2v_private_ack_cfg *) buffer;

	assert(public_cfg != NULL);
	assert(buffer != NULL);

	private_cfg->address		= public_cfg->address;
	private_cfg->ack_cmd_info	= nci_s2v_pack_ack_cmd_info(public_cfg->ack_sid,
								public_cfg->ack_pid,
								public_cfg->ack_msg);
	return sizeof(struct nci_s2v_private_ack_cfg);
}

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_print_section1(void *buffer)
{
	struct nci_s2v_private_ack_cfg *private_cfg = (struct nci_s2v_private_ack_cfg *) buffer;

	assert(buffer != NULL);

	IA_CSS_TRACE_0(VIED_NCI_S2V, DEBUG, "nci_s2v_print_section1:\n");
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\taddress = 0x%08x\n", private_cfg->address);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tack_cmd_info = 0x%08x\n", private_cfg->ack_cmd_info);

	NOT_USED(private_cfg);
}

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_print_sections(void *buffer)
{
	int size_sec0 = nci_s2v_get_sizeof_section(NCI_S2V_SECTION0);

	assert(buffer != NULL);

	nci_s2v_print_section0(buffer);
	buffer = (uint8_t *) buffer + size_sec0;
	nci_s2v_print_section1(buffer);
}

NCI_S2V_STORAGE_CLASS_C
unsigned int nci_s2v_get_command_register_offset(void)
{
	return (_STR_TO_VEC_V2_3_REG_ALIGN * _STR_TO_VEC_V2_3_CMD);
}

NCI_S2V_STORAGE_CLASS_C
unsigned int nci_s2v_get_process_N_vectors_command(unsigned int nof_vectors)
{
	return (_STR_TO_VEC_V2_3_CMD_PROC_N_VEC +
			(nof_vectors << NCI_S2V_CMD_REG_BIT_SHIFT));
}

#endif /* __VIED_NCI_S2V_INLINE_COMMON_H */
