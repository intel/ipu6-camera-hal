/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2016 Intel Corporation.
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
/*! \file
 * @brief dfm Native Command Interfaces to access and control the
 * configuration registers of the device.  
 *  
 *\section dfm operation 
 *  The dfm is an infrastructure device design to offload the SP in the system.
 *  The control of the IPU dataflow is managed by Active/Client agents by:
 *  1. Instructing passive agent(s) to perform a data transfer task or process
 *     data task. 
 *  2. Updating other active agent buffer status only if the Active/Client is
 *     actual accessor (producer/consumer)
 */ 

#ifndef _DFM_NCI_STRUCT_H_
#define _DFM_NCI_STRUCT_H_

#include <stdint.h>

#define DFM_CHAN_DIFF (DFM_DP_1_BUFF_CTRL_DYNAMIC_VAL_ADDR - DFM_DP_0_BUFF_CTRL_DYNAMIC_VAL_ADDR)
#define DFM_BCPR_CHAN_DIFF (BCPR_REG_1_ADDR - BCPR_REG_0_ADDR)

typedef enum {
    DFM_SEQ_TYPE_GENERIC_CMD = 0,
    DFM_SEQ_TYPE_DMA_SEQ = 1,
    DFM_SEQ_TYPE_OFS_SEQ = 2,
    DFM_SEQ_TYPE_BUFF_IDX_UPDATE = 3,
    DFM_SEQ_TYPE_GENERIC_CMD2 = 4,
    DFM_SEQ_TYPE_DMA_POLLING = 5,
    DFM_SEQ_TYPE_FF_ACC = 6
}ipu_device_dfm_seq_type;

typedef enum {
    DFM_SEQ_TYPE_IDX_BEGIN = 0,
    DFM_SEQ_TYPE_IDX_MIDDLE = 1,
    DFM_SEQ_TYPE_IDX_END = 2
}ipu_device_dfm_seq_type_idx; 

/*! ipu_nci_dfm_irq_t */
typedef struct {
    //! 16 lsbits of incoming request address field
    uint16_t dfm_irq_addr;
    //! irq type - see MAS for details
    uint16_t dfm_irq_type;
    //! automatically set when an IRQ is triggered
    uint8_t dfm_irq_valid;     
    uint32_t dfm_irq_data;
} ipu_nci_dfm_irq_t;

typedef enum {
    DFM_SEQ_INIT_DISABLE = 0,
    DFM_SEQ_INIT_TYPE_S2V = 1,
    DFM_SEQ_INIT_TYPE_S2MMIO = 2
} ipu_device_dfm_init_type_t; 

typedef enum {
	DFM_SDF_DISABLED = 0,
	DFM_SDF_SDF_CHECK = 1,
	DFM_SDF_PROGRESS_REPORT = 2
} ipu_nci_dfm_sdf_t ;

/*! ipu_nci_dfm_seq_type_t */
typedef struct {
    //! sequence init type 
    ipu_device_dfm_init_type_t  dfm_seq_type_init_type;
    //! sequence type for begin sequences
    ipu_device_dfm_seq_type dfm_seq_type_begin_type;
    //! sequence type for middle sequences
    ipu_device_dfm_seq_type dfm_seq_type_middle_type;
    //! sequence type for end sequences
    ipu_device_dfm_seq_type dfm_seq_type_end_type;
} ipu_nci_dfm_seq_type_t; 

#endif /* _DFM_NCI_STRUCT_H_ */
