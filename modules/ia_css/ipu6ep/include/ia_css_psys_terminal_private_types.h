/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2019 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_TERMINAL_PRIVATE_TYPES_H
#define __IA_CSS_PSYS_TERMINAL_PRIVATE_TYPES_H

#include "ia_css_terminal_types.h"
#include "ia_css_program_group_data.h"
#include "ia_css_psys_manifest_types.h"

#define	N_UINT16_IN_DATA_TERMINAL_STRUCT	2
#define	N_UINT8_IN_DATA_TERMINAL_STRUCT		3
#define	N_PADDING_UINT8_IN_DATA_TERMINAL_STRUCT	1

/* ========================= Data terminal - START ========================= */

#define SIZE_OF_DATA_TERMINAL_STRUCT_BITS \
	(SIZE_OF_TERMINAL_STRUCT_BITS \
	+ IA_CSS_FRAME_DESCRIPTOR_STRUCT_BITS \
	+ IA_CSS_FRAME_STRUCT_BITS \
	+ IA_CSS_STREAM_STRUCT_BITS \
	+ IA_CSS_UINT32_T_BITS \
	+ IA_CSS_CONNECTION_TYPE_BITS \
	+ (N_UINT16_IN_DATA_TERMINAL_STRUCT * 16) \
	+ (N_UINT8_IN_DATA_TERMINAL_STRUCT * 8)	\
	+ (N_PADDING_UINT8_IN_DATA_TERMINAL_STRUCT * 8))

/**
 * The (data) terminal can be attached to a buffer or a stream.
 * The stream interface is not necessarily limited to strict in-order access.
 * For a stream the restriction is that contrary to a buffer it cannot be
 * addressed directly, i.e. it behaves as a port,
 * but it may support stream_pos() and/or seek() operations
 */
struct ia_css_data_terminal_s {
	/** Data terminal base class */
	ia_css_terminal_t base;
	/** Properties of the image data attached to the terminal */
	ia_css_frame_descriptor_t frame_descriptor;
	/** Data buffer handle attached to the terminal */
	ia_css_frame_t frame;
	/** (exclusive) Data stream handle attached to the terminal
	 * if the data is sourced over a device port
	 *
	 * @note No known use of the field.
	 */
	ia_css_stream_t stream;
	/** Reserved
	* @note No known intent for this reservation field. */
	uint32_t reserved;
	/** Tranfer method for send/receving data to/from this terminal. */
	ia_css_connection_type_t connection_type;
	/** Offset in bytes from the start of this structure to a Array[fragment_count] of
	 *  type "ia_css_fragment_descriptor_t", aligned to a 64 bit boundary.
	 *  (fragment_count being equal for all terminals in a subgraph) of fragment descriptors
	 */
	uint16_t fragment_descriptor_offset;
	/** Size of streaming to GEN buffer in lines. This field will be used only
	 *  at streaming use-case (connection type != IA_CSS_CONNECTION_MEMORY)
	 */
	uint16_t stream2gen_buffer_size;
	/** Kernel id that this terminal is associated with.  If that kernel
	 *  is disabled, this terminal is considered disabled as well.
	 *  Must be set the single kernel defined in the manifest for this terminal's
	 *  ID (ia_css_terminal_s::ID). */
	uint8_t kernel_id;
	/** Indicate to which subgraph this terminal belongs
	 * for common constraints
	 *
	 * @note No known use.
	 */
	uint8_t subgraph_id;
	/** Link ID of the data terminal.  Two terminals can optionally be
	 *  marked as linked to each other by setting the same link ID.
	 *  Hardware topology or logical or algorithmic constraints may restrict the
	 *  terminal that can be so linked.
	 *  In current practice, link ID's have special meaning related to
	 *  the connection type (see "connection_type" field in this structure) or
	 *  synchronization of program groups.
	 *  @see ia_css_isys_link_id_t
	 *  @see ia_css_data_barrier_link_id_t
	 *  @see ia_css_stream2gen_link_id_t */
	uint8_t link_id;
	/** Padding for 64bit alignment */
	uint8_t padding[N_PADDING_UINT8_IN_DATA_TERMINAL_STRUCT];
};
/* ========================== Data terminal - END ========================== */

#endif /* __IA_CSS_PSYS_TERMINAL_PRIVATE_TYPES_H */
