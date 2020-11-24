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

#ifndef __NCI_DEC400_H
#define __NCI_DEC400_H

#include "nci_dec400_struct.h"
#include "nci_dec400_vpless_defs.h"
#include "nci_dec400_storage_class.h"
#include "ipu_device_dec400_devices.h"
#include <type_support.h>

/* Description : Declarations of DEC400 NCI functions
*/

/** Get DEC400 chip information
*
*
* @param[in]    dev_id                nci_dec400_device_id, device ID
* @param[out]    dec_info                nci_dec400_chip_info_t, pointer to information read from chip
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_get_dec_info(
	nci_dec400_device_id dev_id,
	struct nci_dec400_chip_info_t *dec_info);

/** Compression configuration required once per stream (i.e. doesn't
*   change from frame to frame in the same stream)
*
*   Needs to be called for each plane in an image
*
* @param[in] dev_id Device ID
* @param[in] channel The channel (i.e. compression stream) to
* configure
* @param[in] cfg the configuration to write
* @param[in] buffer Fill buffer for VP-less host-side config prep.
* Ignored in firmware (and should be set to NULL there).
*
* @return Size of VP-less configuration blob
*/
NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_channel_config(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

/** Compression configuration required once per buffer (i.e.
*   changes from frame to frame in the same stream)
*
* Needs to be called for each plane in an image
*
* @param[in] dev_id Device ID
* @param[in] channel The channel (i.e. compression stream) to
* configure
* @param[in] cfg The configuraiton to write
* @param[in] buffer Fill buffer for VP-less host-side config prep.
* Ignored in firmware (and should be set to NULL there).
*
* @return Size of VP-less configuration blob
*/
NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_buffer_config(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

/** Frame configuration, including channel configuration
*
*   Needs to be called for each plane in an image
*
* @param[in] dev_id Device ID
* @param[in] channel The channel (i.e. compression stream) to configure
* @param[in] cfg The configuration to write
* @param[in] buffer Fill buffer for VP-less host-side config prep.
* Ignored in firmware (and should be set to NULL there).
*
* @return Size of VP-less configuration blob
*/
NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_frame_config(
		nci_dec400_device_id dev_id,
		uint8_t channel,
		const struct nci_dec400_stream_cfg_t *cfg,
		void *buffer);

/**
 * Fill only channel (i.e. compression stream) level sections,
 * and not per-buffer sections.
 *
 * Each channel corresponds to a single plane of the image format.
 * So this needs to be called once per plane.
 *
 * @param[in] dev_id Device ID
 * @param[in] cfg The configuration to write
 * @param[in] buffer Fill buffer to populate.
 */
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_channel_config_fill_sections(
	nci_dec400_device_id dev_id,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

/**
 * Fill buffer level registers, and not channel (aka compression stream)
 * sections.
 *
 * Each channel corresponds to a single plane of the image.
 * So this needs to be called once per plane.
 *
 * @param[in] dev_id Device ID
 * @param[in] channel The channel (i.e. compression stream) to configure
 * @param[in] cfg The configuration to write
 * @param[in] buffer Fill buffer to populate.
 */
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_buffer_config_fill_sections(
	nci_dec400_device_id dev_id,
	unsigned int channel,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

/**
 * Fill BOTH channel and buffer level sections.
 *
 * Each channel corresponds to a single plane of the image.
 * So this needs to be called once per plane.
 *
 * @param[in] dev_id Device ID
 * @param[in] cfg The configuration to write
 * @param[in] buffer Fill buffer to populate.
 *
 * @see nci_dec400_channel_config_fill_sections()
 * @see nci_dec400_channel_config_fill_sections()
 */
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_frame_config_fill_sections(
	nci_dec400_device_id dev_id,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer);

/**
* Enable/disable interrupt per channel
*
* This is a read-modify-write operation on gcregAHBDECIntrEnbl
* register
*
* @param[in] dev_id Device ID
* @param[in] channel The channel (i.e. compression stream) to enable
* @param[in] enable  Enable/Disable interrupt
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_config_interrupt_enable(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	bool enable);

NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_get_sizeof_blob_per_stream(void);

NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_get_sizeof_channel_blob(void);

NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_get_sizeof_buffer_blob(void);

NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_get_reg_address_by_section(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	enum nci_dec400_fill_sections section);

/**
* Enable/disable interrupt for all channels
*
* This is a read-modify-write operation on gcregAHBDECIntrEnbl
* register
*
* @param[in] dev_id Device ID
* @param[in] enable Enable/disable all interrupts
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_config_interrupt_enable_all_channels(
	nci_dec400_device_id dev_id,
	bool enable);

/** clears one pending interrupt
*
*   This function will read one time the interrupt-ACK register
*   (gcregAHBDECIntrAcknowledge) and return the channel number
*   of the interrupt.
*   It also clears the interrupts in the hardware
*
* @param [in]    dev_id                nci_dec400_device_id, device ID
* @param [out]    error_irq_mask       Output bit mask of error interrupts.
*
* @return     Bit mask of the raised interrupts.
*/
NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_clear_interrupt_status(
	nci_dec400_device_id dev_id,
	uint32_t *error_irq_mask);

/** global "control" configuration of the DEC400
*
*   writes to gcregAHBDECControl and gcregAHBDECControlEx
*
* @param[in]    dev_id                nci_dec400_device_id, device ID
*   	configure
* @param[in]    cfg            struct
*   	dec_per_frame_cfg_t*, configuraiton to write
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_control_config(
	nci_dec400_device_id dev_id,
	struct nci_dec400_control_cfg_t *cfg);

/** soft reset to the DEC400
*
*   writes to gcregAHBDECControl
*   This is a read-modify-write operation
*
* @param[in]    dev_id                nci_dec400_device_id, device ID
*   	configure
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_soft_reset(
	nci_dec400_device_id dev_id);

/** software flush of a tile-status buffer of a channel
*
*   writes to gcregAHBDECControl
*   This is a read-modify-write operation
*
* @param[in]    dev_id                nci_dec400_device_id, device ID
* @param[in]    channel                uint8_t, the channel to
*   	configure
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_flush(
	nci_dec400_device_id dev_id,
	uint8_t channel);

/** configure stream debug selection register
*
*
* @param[in]    dev_id      DEC400 device ID
* @param[in]    stream_id   the id of the stream to debug [0-31]
* @param[in]    counter_id  the id of the counter to expose
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_config_stream_debug(
	nci_dec400_device_id dev_id,
	uint8_t stream_id,
	uint8_t counter_id
	);

/** read stream and additional debug registers
*
*
* @param[in]    dev_id      DEC400 device ID
* @param[out]   info struct the debug values read from registers
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_read_extended_debug_info(
	nci_dec400_device_id dev_id,
	struct nci_dec400_extended_debug_info_t *info);

/** read debug registers
*
*
* @param[in]    dev_id                nci_dec400_device_id, device ID
* @param[out]    info struct
*   	nci_dec400_debug_info_t*, the
*   	debug values read from registers
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_read_debug_info(
	nci_dec400_device_id dev_id,
	struct nci_dec400_debug_info_t *info);

/** set GP regs
*
*
* @param[in]    dev_id                nci_dec400_device_id, device ID
* @param[in]    axi_id 		uint8_t, the AXI ID of the dec
*   	device
* @param[in]    path_en		uint8_t, should data pass through
*   	DEC
*
*/
NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_set_gp_wrapper_regs(
	nci_dec400_device_id dev_id,
	uint8_t axi_id,
	bool path_en);

/** set axi user field for dec400 control register.
*   This is essentially the global CIO info used for TS and is
*   different across dec enc/dec and different IPUs.
*
* @param[in]    dev_id      DEC400 device ID
*
* @return       axuser field as 16bit value (essentially 12bit)
*/
NCI_DEC400_STORAGE_CLASS_H
uint16_t nci_dec400_get_control_config_axuser(
	nci_dec400_device_id dev_id);

NCI_DEC400_STORAGE_CLASS_H
uint32_t nci_dec400_irq_wrapper_regs_status(void);

NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_irq_wrapper_regs_clear(uint32_t irq_mask);

NCI_DEC400_STORAGE_CLASS_H
void nci_dec400_irq_wrapper_regs_setup(uint32_t enable_mask,
									   uint32_t edge_mask,
									   uint32_t level_n_pulse_mask);

#define ENABLE_SIG2CIO_EVENT_DEC400_FLUSH_DONE	(1)

#ifdef __INLINE_NCI_DEC400__
#include "nci_dec400_inline.h"
#endif

#endif /* __NCI_DEC400_H */
