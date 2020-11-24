/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2020 Intel Corporation.
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

#ifndef __DEV_API_DFM_TYPES_H
#define __DEV_API_DFM_TYPES_H

#include "type_support.h"
#include "dai_ipu_top_devices.h"
#include "ipu_device_dfm_devices.h"
#include "nci_dfm_types.h"
#include <assert_support.h> /* COMPILATION_ERROR_IF */
#include <storage_class.h> /* STORAGE_CLASS_INLINE */

enum dev_api_stream_mode {
	DEV_API_STREAM_PASSIVE = 0,    /* port is passive when there is no sequencer and don't sends ack for fragment done*/
	DEV_API_STREAM_ACTIVE = 1,     /* port is active when there is a sequencer and sends ack done for fragment done*/
};

enum dev_api_dfm_mode {
	DEV_API_DFM_PASSIVE = 0,    /* port is passive when there is no sequencer and don't sends ack for fragment done*/
	DEV_API_DFM_ACTIVE = 1,     /* port is active when there is a sequencer and sends ack done for fragment done*/
};

#define DEV_API_DFM_MAX_PORT_NUM            (32) /* Max number of ports empty or full, an instance of DFm can have */
#define DEV_API_DFM_SEQ_MAX_NUM_CMDS        (4) /* Number of commands per sequence can be 1 or 3 as per NCI by SDK */
#define DEV_API_DFM_MAX_NUM_AGENTS_PER_PORT (NCI_DFM_MAX_NUM_SEQ) /* At max each seq can have different agent */
#define DEV_API_DFM_OTF_PASSIVE_ACK_DATA_SIGNATURE 0xa5a5a500	/* Signature that identifies OTF ACK DATA */
#define DEV_API_DFM_OTF_PASSIVE_ACK_DATA_SIGNATURE_MASK 0xffffff00
#define DEV_API_DFM_OTF_PASSIVE_ACK_DATA_OFFSET_MASK 0xff
#define DEV_API_DFM_OTF_PASSIVE_ACK_DATA_ENCODE(offset) (DEV_API_DFM_OTF_PASSIVE_ACK_DATA_SIGNATURE | (DEV_API_DFM_OTF_PASSIVE_ACK_DATA_OFFSET_MASK & offset))
#define DEV_API_DFM_OTF_PASSIVE_ACK_DATA_GET_OFFSET(token) (DEV_API_DFM_OTF_PASSIVE_ACK_DATA_OFFSET_MASK & token)
#define IS_DEV_API_DFM_OTF_PASSIVE_ACK_DATA_SIGNATURE(token) ((DEV_API_DFM_OTF_PASSIVE_ACK_DATA_SIGNATURE_MASK & token) == DEV_API_DFM_OTF_PASSIVE_ACK_DATA_SIGNATURE)

enum dev_api_dfm_agent {
	DEV_API_DFM_AGENT_SPC_EQ,
	DEV_API_DFM_AGENT_SPP0_EQ,
	DEV_API_DFM_AGENT_SPP1_EQ,
	DEV_API_DFM_AGENT_VP_EQ,
	DEV_API_DFM_AGENT_SPC_DMEM,
	DEV_API_DFM_AGENT_SPP0_DMEM,
	DEV_API_DFM_AGENT_SPP1_DMEM,
	DEV_API_DFM_AGENT_VP_DMEM,
	DEV_API_DFM_AGENT_DMA_LB,
	DEV_API_DFM_AGENT_DMA_HBTX,
	DEV_API_DFM_AGENT_DMA_HBFRX,
	DEV_API_DFM_AGENT_DMA_ISL,
	DEV_API_DFM_AGENT_DVS_STATS,
	DEV_API_DFM_AGENT_GAMMASTAR,
	DEV_API_DFM_AGENT_3A_AF,
	DEV_API_DFM_AGENT_3A_AWB,
	DEV_API_DFM_AGENT_LSC,
	DEV_API_DFM_AGENT_PDAF,
	DEV_API_DFM_AGENT_V2S,
	DEV_API_DFM_AGENT_S2V_ISL_BAYER,
	DEV_API_DFM_AGENT_S2V_ISL_YUV,
	DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED0,
#if HAS_FF_R2I_DS_B
	DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED1,
#endif
	DEV_API_DFM_AGENT_STR2MMIO_ISL_PS_SIS,
	DEV_API_DFM_AGENT_S2V_PSA_BAYER,
	DEV_API_DFM_AGENT_S2V_PSA_YUV,
	DEV_API_DFM_AGENT_STR2MMIO_PS_0,
	DEV_API_DFM_AGENT_STR2MMIO_PS_1,
	DEV_API_DFM_AGENT_STR2MMIO_IS_0,
	DEV_API_DFM_AGENT_STR2MMIO_IS_1,
	DEV_API_DFM_AGENT_OFS,
	DEV_API_DFM_AGENT_V2S_PSA_BAYER,
	DEV_API_DFM_AGENT_DFM_PORT, /* Used for forwarding the event to another port */
	DEV_API_DFM_AGENT_GENERIC, /* For any device which recieve static token and 1 command */
	DEV_API_DFM_AGENT_GENERIC_CMD2,  /* use this when seq is generic double command */
	DEV_API_DFM_AGENT_IBUF,
	DEV_API_DFM_AGENT_MBR,
	DEV_API_DFM_AGENT_STR2MMIO_PS_IR_DEPTH,
};

enum dev_api_seq_type {
	DEV_API_DFM_BEGIN_SEQ = DFM_SEQ_TYPE_IDX_BEGIN,
	DEV_API_DFM_MIDDLE_SEQ = DFM_SEQ_TYPE_IDX_MIDDLE,
	DEV_API_DFM_END_SEQ = DFM_SEQ_TYPE_IDX_END,
	DEV_API_DFM_NUM_SEQ,
};

typedef struct dev_api_dfm_stream {
	enum ipu_device_dfm_id dev_id;  /* device instance */
	enum dev_api_stream_mode mode;
	unsigned num_prod;
	unsigned num_cons;
	unsigned seq_iter;
	uint32_t ack_addr;
	uint32_t ack_data;
} dev_api_dfm_stream_s;

struct dev_api_dfm_agent_property {
	enum dev_api_dfm_agent type;
	uint32_t offset;	/* The address offset of the agent where commands from DFM
				 * needs to be sent.
				 */
	uint32_t seq_num_cmds;
	uint32_t cmd[DEV_API_DFM_SEQ_MAX_NUM_CMDS];
};

typedef struct dev_api_dfm_port {
	unsigned 				port_num;
	struct dev_api_dfm_agent_property	agent[DEV_API_DFM_MAX_NUM_AGENTS_PER_PORT];
	enum dev_api_dfm_mode             	mode;
	unsigned priming;
	struct nci_dfm_port_config port_config;
	/* Buffer allocated by the host to write the configuration */
	/* For the ISP flow, this is ignored. */
	void *config_buffer;
} dev_api_dfm_port_s;

#endif /* __DEV_API_DFM_TYPES_H */
