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

#ifndef _DFM_NCI_H_
#define _DFM_NCI_H_

#include "ipu_nci_config.h"
#include "dfm_address_map.h"
//#include "dfm_full_ccbcr.h"
#include "dfm_properties.h"

#define DEV_NAME dfm
#include <stdint.h>
#ifdef CFG_VIED_NCI_INLINE_IMPL
#include <nci_dev_common.h>
#endif
#include "ipu_nci_dfm_struct.h"
#include <dai/dai_device_access.h>

#define DFM_CHAN_DIFF (DFM_DP_1_BUFF_CTRL_DYNAMIC_VAL_ADDR - DFM_DP_0_BUFF_CTRL_DYNAMIC_VAL_ADDR)

#ifndef CFG_VIED_NCI_INLINE_IMPL
#if __IPU_CHECK_NO_INLINE
#define _IPU_NCI_COMMON_INLINE
#else
#ifndef _IPU_NCI_INLINE
#define _IPU_NCI_COMMON_INLINE static inline
#else
#define _IPU_NCI_COMMON_INLINE _IPU_NCI_INLINE
#endif
#endif
#undef __IPU_CHECK_NO_INLINE
#endif

/*********************************//**
 *        DFM NCI Controls
 ************************************/

/** @brief Retrieves the IRQ info and data from the DFM.  
 * @param[in]   dev_instance_id         The device ID.  
 * @param[out]  *irq                    general configuration of this device 
 * @return      None  
 *  
 * This function returns the IRQ information from DFM. 
 * The register with the irq addr, type and valid should be 
 * accessed first. 
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_irq(dai_device_id_t dev_instance_id,
                         ipu_nci_dfm_irq_t *irq); 

/** @brief sets the event_cmd register in the DFM.  
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      None  
 *  
 * This function sets address for the buffer full/release 
 * command. The command indicate on event, the DATA of the 
 * command is ignored. 
 */  
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_event_cmd(dai_device_id_t dev_instance_id, 
                               uint32_t dp_index);

/** @brief sets the suspend_cmd register in the DFM.  
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      None  
 *  
 * Address for the suspend command. The command will reset the 
 * ratio_conv iter_cnt and will cause the pipe to drain. Finally 
 * an done_ack command will be sent. Indicate on event, the DATA 
 * of the command is ignored.
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_suspend_cmd(dai_device_id_t dev_instance_id, 
                                 uint32_t dp_index);

/** @brief sets the enable_bit in the DFM for a buffer
 *         controler.  
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      None  
 *  
 * Enabling the device for a specific device port
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_buffer_ctrl_enable(dai_device_id_t dev_instance_id, 
                                    uint32_t dp_index);

/** @brief resets the enable_bit in the DFM for a buffer
 *         controler.
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      None  
 *  
 * Disabling the device for a specific device port
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_buffer_ctrl_disable(dai_device_id_t dev_instance_id, 
                                     uint32_t dp_index);

/** @brief sets the enable_bit in the DFM for a ratio converter.
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      None  
 *  
 * Enabling the device for a specific device port
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_ratio_conv_enable(dai_device_id_t dev_instance_id,
                                   uint32_t dp_index);

/** @brief resets the enable_bit in the DFM for a ratio
 *         converter
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      None  
 *  
 * Disabling the device for a specific device port
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_ratio_conv_disable(dai_device_id_t dev_instance_id,
                                    uint32_t dp_index);

/** @brief returns the buffer control enable_bit in the DFM.  
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index 
 * @return      0-disabled;1-enabled  
 *  
 * 0 - disabled
 * 1 - enabled
 */
_IPU_NCI_COMMON_INLINE
int32_t ipu_nci_dfm_buffer_ctrl_is_enabled(dai_device_id_t dev_instance_id,
                                           uint32_t dp_index);

/** @brief returns the ratio converter enable_bit in the DFM.  
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index 
 * @return      0-disabled;1-enabled  
 *  
 * 0 - disabled
 * 1 - enabled
 */
_IPU_NCI_COMMON_INLINE
int32_t ipu_nci_dfm_ratio_conv_is_enabled(dai_device_id_t dev_instance_id,
                                          uint32_t dp_index);

/** @brief gets the ratio converter counter status
 * @param[in]   dev_instance_id        The device ID.
 * @param[in]   dp_index               Device port index
 * @param[out]  *draining              How many in events should
 *       be drained. Set to the value of the priming
 *       when enabled
 * @param[out]  *out_event             How many out events still
 *       to be produced
 * @param[out]  *unit_cnt              Reflects the units counter
 *       for the SDF check/progress reporting
  * @return      None
 *
 * This function gets the ratio converter counter status
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_ratio_conv_status_0(dai_device_id_t dev_instance_id,
									     uint32_t dp_index,
									     uint8_t *draining,
									     uint8_t *out_event,
                                         uint16_t *unit_cnt);

/** @brief gets the ratio converter counter status
 * @param[in]   dev_instance_id        The device ID.
 * @param[in]   dp_index               Device port index
 * @param[out]  *in_event              How many in_events received
 *       since the last time threshold was reached
 * @param[out]  *iter_cnt             How many out_events the ratio_conv
 *       still need to produce before sending done_ack
 * @return      None
 *
 * This function gets the ratio converter counter status
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_ratio_conv_status_1(dai_device_id_t dev_instance_id,
									     uint32_t dp_index,
									     uint8_t *in_event,
									     uint32_t *iter_cnt);

/** @brief gets the iterator counters current values
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[out]  *seq_iter_cnt           Begin/middle/end
 *       iterations counter current value
 * @param[out]  *seq_iter_part          Indicates if we are in
 *       the begin/middle/end part
 * @return      None
 *
 * This function gets the iterator counters current values
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_buff_cntl_status_0(dai_device_id_t dev_instance_id,
									    uint32_t dp_index,
									    uint16_t *seq_iter_cnt,
									    uint8_t *seq_iter_part);

/** @brief sets the ratio converter configuration
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[in]   threshold               How many incoming events
 *       should be gathered before sending an out event
 * @param[in]   out_ratio               How many out events
 *       should be sent
 * @param[in]   ignore_first_event    ignore the first command
 *       recieved when on
 * @return      None
 *
 * This function sets the ratio converter configuration
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_ratio_conv_cfg_0(dai_device_id_t dev_instance_id,
									  uint32_t dp_index,
									  uint8_t threshold,
									  uint8_t out_ratio,
                                      uint8_t ignore_first_event);

/** @brief gets the ratio converter configuration
 * @param[in]   dev_instance_id        The device ID.
 * @param[in]   dp_index               Device port index
 * @param[out]  *threshold             How many incoming events
 *       should be gathered before sending an out event
 * @param[out]  *out_ratio             How many out events
 *       should be sent
 * @param[out]  *ignore_first_event    ignore the first command
 *       recieved when on
 * @return      None
 *
 * This function gets the ratio converter configuration
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_ratio_conv_cfg_0(dai_device_id_t dev_instance_id,
                                     uint32_t dp_index,
                                     uint8_t *threshold,
                                     uint8_t *out_ratio,
                                     uint8_t *ignore_first_event);

/** @brief sets the ratio converter counters
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[in]   iter                    How many iterations
 *       should the sequencer do in active mode before sending
 *       ack done command. Also determines if this ratio
 *       converter acting as passive or active device. Full
 *       device port will always be passive. When the number of
 *       iterations will be zero the ratio_conv will be passive
 *       and active otherwise.
  * @param[in]   priming_val             How many events should be
  *      generated during priming stage. This value has
 *       to be larger than zero when the iteration counter is
 *       larger than zero.
 * @return      None
 *
 * This function sets the ratio converter counters
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_ratio_conv_cfg_1(dai_device_id_t dev_instance_id,
                                      uint32_t dp_index,
                                      uint32_t iter,
                                      uint8_t  priming_val);

/** @brief gets the ratio converter counters
 * @param[in]   dev_instance_id        The device ID.
 * @param[in]   dp_index               Device port index
 * @param[out]  *iter                  How many iterations
 *       should the sequencer do in active mode before sending
 *       ack done command. Also determines if this ratio
 *       converter acting as passive or active device. Full
 *       device port will always be passive. When the number of
 *       iterations will be zero the ratio_conv will be passive
 *       and active otherwise.
  * @param[out]  *priming_val          How many events should be
  *      generated during priming stage. This value has
 *       to be larger than zero when the iteration counter is
 *       larger than zero.
 * @return      None
 *
 * This function gets the ratio converter counters
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_ratio_conv_cfg_1(dai_device_id_t dev_instance_id,
                                      uint32_t dp_index,
                                      uint32_t *iter,
                                      uint8_t  *priming_val);

/** @brief sets buffer chasing in the DFM for a ratio converter
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[in]   en                      1 - enable, 0 - disable
 * @param[in]   sdf_reg_alloc           allocating a BCPR
 *       register to sync with out of all BCPR registers.
 *       Maximum 16 BCPR registers. 0-15
 * @param[in]   iter2unit               Defines the ratio
 *       between the DDR unit consumed and the events the
 *       ratio_conv is producing #Unit = #iterations / iter2unit
 * @return      None
 *
 * Disabling the device for a specific device port
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_ratio_conv_sdf(dai_device_id_t dev_instance_id,
                                    uint32_t dp_index,
									uint8_t en,
									uint8_t frame_indx,
									uint8_t sdf_reg_alloc,
									uint8_t iter2unit);

/** @brief gets buffer chasing register in the DFM for a ratio
 *         converter
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index

 * @param[out]   *en                      1 - enable, 0 -
 *       disable
 * @param[out]   *sdf_reg_alloc           allocating a BCPR
 *       register to sync with out of all BCPR registers.
 *       Maximum 16 BCPR registers. 0-15
 * @param[out]   *iter2unit               Defines the ratio
 *       between the DDR unit consumed and the events the
 *       ratio_conv is producing #Unit = #iterations / iter2unit
 * @return      None
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_ratio_conv_sdf(dai_device_id_t dev_instance_id,
                                    uint32_t dp_index,
									uint8_t *en,
									uint8_t *frame_indx,
									uint8_t *sdf_reg_alloc,
									uint8_t *iter2unit);

/** @brief sets the mask for gather_mult_event in the DFM.  
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @param[in]   val                     mask value
 * @return      None  
 *  
 * This mask is used for producing the out event of the 
 * gather_mult_event. 
 * Every bit indicates if the corresponding input event is a
 * condition for this output event or not. The reset value will
 * support the relations of 1:1 between incoming and outgoing
 * events. No gathering and no multicasting. When equals zero 
 * the buff_ctrl will be disabled and will never receive events.
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_gather_mult_event_mask(dai_device_id_t dev_instance_id,
                                            uint32_t dp_index,
                                            uint32_t val);
 

/** @brief gets the mask for gather_mult_event in the DFM.  
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      mask value  
 *  
 * This mask is used for producing the out event of the 
 * gather_mult_event. 
 * Every bit indicates if the corresponding input event is a
 * condition for this output event or not. The reset value will
 * support the relations of 1:1 between incoming and outgoing
 * events. No gathering and no multicasting. When equals zero 
 * the buff_ctrl will be disabled and will never receive events.
 */
_IPU_NCI_COMMON_INLINE
uint32_t ipu_nci_dfm_get_gather_mult_event_mask(dai_device_id_t dev_instance_id,
                                                uint32_t dp_index);

/** @brief sets the control for the dynamic part of the command
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[in]   wrap                    The value that the
 *       dynamic value wraps around typically the buffer queue
 *       depth.
 * @param[in]   inc                     The value to add each
 *       iteration to the dynamic value
 * @param[in]   init                    Initial value for the
 *       dynamic part of the command if exist. init value should
 *       never be bigger than the wrap around value.
 * @return      None
 *
 * This function sets the control for the dynamic part of the
 * command
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_buff_ctrl_dynamic_val(dai_device_id_t dev_instance_id,
                                           uint32_t dp_index,
                                           uint8_t wrap,
                                           uint8_t inc,
                                           uint8_t init);
/** @brief gets the control for the dynamic part of the command
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[out]   *wrap                  The value that the
 *       dynamic value wraps around typically the buffer queue
 *       depth.
 * @param[out]   *inc                   The value to add each
 *       iteration to the dynamic value
 * @param[out]   *init                  Initial value for the
 *       dynamic part of the command if exist. init value should
 *       never be bigger than the wrap around value.
 * @return      None
 *
 * This function gets the control for the dynamic part of the
 * command
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_buff_ctrl_dynamic_val(dai_device_id_t dev_instance_id,
                                           uint32_t dp_index,
                                           uint8_t *wrap,
                                           uint8_t *inc,
                                           uint8_t *init);

/** @brief sets the number of iterations in begin sequense.
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @param[in]   iter                    Number of times the sequence will be executed
 * @param[in]   seq_type                Type of begin sequence
 * @param[in]   init_type               Determines init command preceeding the first begin-sequence
 * @return      None  
 *  
 * Sets number of iterations and type of begin-sequence of a buffer-controller and
 * type of init command that preceeds the first begin-sequence
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_buff_ctrl_begin_seq(dai_device_id_t dev_instance_id,
                                      	 uint32_t dp_index,
										 uint16_t iter,
										 uint8_t seq_type,
										 uint8_t init_type);

/** @brief gets the attributes of begin-sequense.
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @param[out]  *iter                   Number of times the sequence will be executed
 * @param[out]  *seq_type               Type of begin sequence
 * @param[out]  *init_type              Determines init command preceeding the first begin-sequence
 * @return      None
 *  
 * Gets the number of iterations and type of begin-sequence of a buffer-controller and
 * type of init command that preceeds the first begin-sequence
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_buff_ctrl_begin_seq(dai_device_id_t dev_instance_id,
                                      	 uint32_t dp_index,
										 uint16_t *iter,
										 uint8_t *seq_type,
										 uint8_t *init_type);

/** @brief sets the number of iterations in begin sequense.
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[in]   iter                    Number of times the sequence will be executed
 * @param[in]   seq_type                Type of begin sequence
 * @return      None
 *
 * Sets the number of iterations and type of begin-sequence of a buffer-controller;
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_buff_ctrl_middle_seq(dai_device_id_t dev_instance_id,
                                      	  uint32_t dp_index,
										  uint16_t iter,
										  uint8_t seq_type);

/** @brief gets the attributes of middle-sequense.
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[out]  *iter                   Number of times the sequence will be executed
 * @param[out]  *seq_type               Type of begin sequence
 * @return      None
 *
 * Gets the number of iterations and type of middle-sequence of a buffer-controller;
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_buff_ctrl_middle_seq(dai_device_id_t dev_instance_id,
                                      	  uint32_t dp_index,
										  uint16_t *iter,
										  uint8_t *seq_type);

/** @brief sets the number of iterations in begin sequense.
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[in]   iter                    Number of times the sequence will be executed
 * @param[in]   seq_type                Type of begin sequence
 * @return      None
 *
 * Sets the number of iterations and type of end-sequence of a buffer-controller
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_buff_ctrl_end_seq(dai_device_id_t dev_instance_id,
                                       uint32_t dp_index,
									   uint16_t iter,
									   uint8_t seq_type);

/** @brief gets the attributes of middle-sequense.
 * @param[in]   dev_instance_id         The device ID.
 * @param[in]   dp_index                Device port index
 * @param[out]  *iter                   Number of times the sequence will be executed
 * @param[out]  *seq_type               Type of begin sequence
 * @return      None
 *
 * Gets the number of iterations and type of end-sequence of a buffer-controller;
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_get_buff_ctrl_end_seq(dai_device_id_t dev_instance_id,
                                   	   uint32_t dp_index,
									   uint16_t *iter,
									   uint8_t *seq_type);

///////////////////////////////////////////////////////////////////////////////

/** @brief sets a bcpr register
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @param[in]   unit_index              which unit is being 
 *       written to DDR buffer
 * @param[in]   frame_indx              which frame is being written to DDR buffer
 * @return      None  
 *  
 * This function sets a bcpr register
 */
_IPU_NCI_COMMON_INLINE
   void ipu_nci_dfm_set_bcpr_reg(dai_device_id_t dev_instance_id,
                                 uint32_t dp_index,
                                 uint16_t unit_indx,
                                 uint8_t frame_indx);

/** @brief gets a bcpr register contents
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @param[out]   *unit_index              which unit is being 
 *       written to DDR buffer
 * @param[out]   *frame_indx              which frame is being 
 *       written to DDR buffer
 * @return      None  
 *  
 * This function gets a bcpr register contents
 */
_IPU_NCI_COMMON_INLINE
   void ipu_nci_dfm_get_bcpr_reg(dai_device_id_t dev_instance_id,
                                 uint32_t dp_index,
                                 uint16_t *unit_indx,
                                 uint8_t *frame_indx);

/** @brief deasserts a bcpr valid register
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index
 * @return      None  
 *  
 * This function deasserts a bcpr valid register 
 */
_IPU_NCI_COMMON_INLINE
   void ipu_nci_dfm_deassert_bcpr_reg_valid(dai_device_id_t dev_instance_id, uint32_t dp_index);

/** @brief gets a bcpr valid register
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   dp_index                Device port index 
 * @param[out]  *valid                  bcpr registe valid
 * @return      None  
 *  
 * This function gets a bcpr valid register 
 */
_IPU_NCI_COMMON_INLINE
   void ipu_nci_dfm_get_bcpr_valid(dai_device_id_t dev_instance_id,
                                   uint32_t dp_index,
                                   uint8_t *valid);

/*********************************//**
 *        DFM NCI Commands          
 ************************************/

/** @brief sets a command in the DFM memory banks. Option 2
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   offset                  Command offset in 
 *       command memory space
 * @param[in]   address                 Command address
 * @param[in]   seq_size                How much data will 
 *       follow in dwords (should be either 1 or 3)
 * @param[in]   *data                   Data received in array
 * @return      None  
 *  
 * This function sets a command in the DFM memory banks. 
 * The NCI in this option will not know the mapping from device 
 * port index to base address, so the user must provide that. 
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_set_command(dai_device_id_t dev_instance_id, 
                             uint32_t offset,
                             uint32_t address,
                             uint32_t seq_size,
                             const uint32_t *data); 

/** @brief sets an ack_done command in the DFM memory banks.
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   offset                  Command offset in 
 *       command memory space
 * @param[in]   address                 Command address
 * @param[in]   data                   The ack_done command
 * @return      None  
 *  
 * This function sets the ack_done command in the DFM memory 
 * banks. The NCI in this option will not know the mapping from 
 * device port index to base address, so the user must provide 
 * that. 
 */
_IPU_NCI_COMMON_INLINE
   void ipu_nci_dfm_set_ack_done(dai_device_id_t dev_instance_id,
                                 uint32_t offset,
                                 uint32_t address,
                                 const uint32_t data);

/*********************************//**
 *        DFM NCI Generic access          
 ************************************/

/** @brief stores 32-bits to the device 
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   offset                  Address within the 
 *       device address space
 * @param[in]   data                    32-bit to be written to 
 *       that offset
 * @return      None  
 *  
 * This function stores 32 bit data to the device at a given 
 * offset in reference to the device base address 
 */
_IPU_NCI_COMMON_INLINE
void ipu_nci_dfm_store(dai_device_id_t dev_instance_id, uint32_t offset,
                       uint32_t data);

/** @brief loads 32 bits from the device 
 * @param[in]   dev_instance_id         The device ID.   
 * @param[in]   offset                  Address within the 
 *       device address space
 * @return      32-bit data  
 *  
 * This function loads 32 bit data from the device at a given 
 * offset in reference to the device base address 
 */
_IPU_NCI_COMMON_INLINE
uint32_t ipu_nci_dfm_load(dai_device_id_t dev_instance_id, uint32_t offset);

#ifdef CFG_VIED_NCI_INLINE_IMPL
#include "ipu_nci_dfm_impl.h"
#endif

#endif /* _DFM_NCI_H_ */
