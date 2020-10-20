/*
 * Copyright (C) 2020 Intel Corporation.
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

#ifndef __IA_CSS_TERMINAL_TYPES_H
#define __IA_CSS_TERMINAL_TYPES_H

#include "type_support.h"
#include "ia_css_base_types.h"
#include "ia_css_terminal_base_types.h"

typedef struct ia_css_program_control_init_load_section_desc_s
	ia_css_program_control_init_load_section_desc_t;
typedef struct ia_css_program_control_init_connect_section_desc_s
	ia_css_program_control_init_connect_section_desc_t;
typedef struct ia_css_program_control_init_program_desc_s
	ia_css_program_control_init_program_desc_t;
typedef struct ia_css_program_control_init_terminal_s
	ia_css_program_control_init_terminal_t;

typedef struct ia_css_program_terminal_s ia_css_program_terminal_t;
typedef struct ia_css_fragment_param_section_desc_s
	ia_css_fragment_param_section_desc_t;
typedef struct ia_css_kernel_fragment_sequencer_info_desc_s
	ia_css_kernel_fragment_sequencer_info_desc_t;
typedef struct ia_css_kernel_fragment_sequencer_command_desc_s
	ia_css_kernel_fragment_sequencer_command_desc_t;

typedef struct ia_css_sliced_param_terminal_s ia_css_sliced_param_terminal_t;
typedef struct ia_css_fragment_slice_desc_s ia_css_fragment_slice_desc_t;
typedef struct ia_css_slice_param_section_desc_s
	ia_css_slice_param_section_desc_t;

typedef struct ia_css_spatial_param_terminal_s ia_css_spatial_param_terminal_t;
typedef struct ia_css_frame_grid_desc_s ia_css_frame_grid_desc_t;
typedef struct ia_css_frame_grid_param_section_desc_s
	ia_css_frame_grid_param_section_desc_t;
typedef struct ia_css_fragment_grid_desc_s ia_css_fragment_grid_desc_t;

typedef struct ia_css_param_terminal_s ia_css_param_terminal_t;
typedef struct ia_css_param_section_desc_s ia_css_param_section_desc_t;

typedef struct ia_css_param_payload_s ia_css_param_payload_t;
typedef struct ia_css_terminal_s ia_css_terminal_t;

/* =================== Generic Parameter Payload - START =================== */
#define N_UINT64_IN_PARAM_PAYLOAD_STRUCT	1
#define N_UINT32_IN_PARAM_PAYLOAD_STRUCT	1

#define IA_CSS_PARAM_PAYLOAD_STRUCT_BITS \
	(N_UINT64_IN_PARAM_PAYLOAD_STRUCT * IA_CSS_UINT64_T_BITS \
	+ VIED_VADDRESS_BITS \
	+ N_UINT32_IN_PARAM_PAYLOAD_STRUCT * IA_CSS_UINT32_T_BITS)

struct ia_css_param_payload_s {
	/**
	 * Temporary variable holding the host address of the parameter buffer
	 * as PSYS is handling the parameters on the host side for the moment
	 */
	uint64_t host_buffer;
	/**
	 * Base virtual addresses to parameters in subsystem virtual
	 * memory space
	 * NOTE: Used in legacy pg flow
	 */
	vied_vaddress_t buffer;
	/**
	 * Offset to buffer address within external buffer set structure
	 * NOTE: Used in ppg flow
	 */
	uint32_t terminal_index;
};
/* =================== Generic Parameter Payload - End ==================== */

/* ==================== Cached Param Terminal - START ==================== */
#define N_UINT32_IN_PARAM_SEC_STRUCT 2

#define SIZE_OF_PARAM_SEC_STRUCT_BITS \
	(N_UINT32_IN_PARAM_SEC_STRUCT * IA_CSS_UINT32_T_BITS)

/** Parameters section */
struct ia_css_param_section_desc_s {
	/** Offset of the parameter allocation in memory */
	uint32_t mem_offset;
	/** Memory allocation size needs of this parameter */
	uint32_t mem_size;
};

#define N_UINT16_IN_PARAM_TERMINAL_STRUCT		1
#define N_PADDING_UINT8_IN_PARAM_TERMINAL_STRUCT	6

#define SIZE_OF_PARAM_TERMINAL_STRUCT_BITS \
	(SIZE_OF_TERMINAL_STRUCT_BITS \
	+ IA_CSS_PARAM_PAYLOAD_STRUCT_BITS \
	+ N_UINT16_IN_PARAM_TERMINAL_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_PADDING_UINT8_IN_PARAM_TERMINAL_STRUCT * IA_CSS_UINT8_T_BITS)

/** "Cached" parameters terminal for parameter that do not change
 * at frame scope.
 */
struct ia_css_param_terminal_s {
	/** Parameter terminal base */
	ia_css_terminal_t base;
	/** Helps to identify the parameter buffer when sent later on with the buffer set  */
	ia_css_param_payload_t param_payload;
	/** Points to the variable array of ia_css_param_section_desc_t */
	uint16_t param_section_desc_offset;
	uint8_t padding[N_PADDING_UINT8_IN_PARAM_TERMINAL_STRUCT];
};
/* ==================== Cached Param Terminal - End ==================== */

/* ==================== Spatial Param Terminal - START ==================== */
#define N_UINT16_IN_FRAG_GRID_STRUCT (2 * IA_CSS_N_DATA_DIMENSION)

#define SIZE_OF_FRAG_GRID_STRUCT_BITS \
	(N_UINT16_IN_FRAG_GRID_STRUCT * IA_CSS_UINT16_T_BITS)

struct ia_css_fragment_grid_desc_s {
	/**
	 * Offset width/height of the top-left compute unit of the
	 * fragment compared to the frame
	 */
	uint16_t fragment_grid_index[IA_CSS_N_DATA_DIMENSION];
	/**
	 * Resolution width/height of the spatial parameters that
	 * correspond to the fragment measured in compute units
	 */
	uint16_t fragment_grid_dimension[IA_CSS_N_DATA_DIMENSION];
};

#define N_UINT32_IN_FRAME_GRID_PARAM_SEC_STRUCT		3
#define N_PADDING_UINT8_IN_FRAME_GRID_PARAM_SEC_STRUCT	4

#define SIZE_OF_FRAME_GRID_PARAM_SEC_STRUCT_BITS \
	(N_UINT32_IN_FRAME_GRID_PARAM_SEC_STRUCT * IA_CSS_UINT32_T_BITS \
	+ N_PADDING_UINT8_IN_FRAME_GRID_PARAM_SEC_STRUCT * IA_CSS_UINT8_T_BITS)

/**
 * A plane of parameters with spatial aspect
 * (compute units correlated to pixel data)
 */
struct ia_css_frame_grid_param_section_desc_s {
	/** Offset of the parameter allocation in memory */
	uint32_t mem_offset;
	/** Memory allocation size needs of this parameter */
	uint32_t mem_size;
	/**
	 * stride in bytes of each line of compute units for
	 * the specified memory space and region
	 */
	uint32_t stride;
	uint8_t  padding[N_PADDING_UINT8_IN_FRAME_GRID_PARAM_SEC_STRUCT];
};

#define N_UINT16_IN_FRAME_GRID_STRUCT_STRUCT IA_CSS_N_DATA_DIMENSION
#define N_PADDING_UINT8_IN_FRAME_GRID_STRUCT 4

#define SIZE_OF_FRAME_GRID_STRUCT_BITS \
	(N_UINT16_IN_FRAME_GRID_STRUCT_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_PADDING_UINT8_IN_FRAME_GRID_STRUCT * IA_CSS_UINT8_T_BITS)

struct ia_css_frame_grid_desc_s {
	/** Resolution width/height of the frame of
	 * spatial parameters measured in compute units
	 */
	uint16_t frame_grid_dimension[IA_CSS_N_DATA_DIMENSION];
	uint8_t padding[N_PADDING_UINT8_IN_FRAME_GRID_STRUCT];
};

#define N_UINT32_IN_SPATIAL_PARAM_TERM_STRUCT 1
#define N_UINT16_IN_SPATIAL_PARAM_TERM_STRUCT 2

#define SIZE_OF_SPATIAL_PARAM_TERM_STRUCT_BITS \
	(SIZE_OF_TERMINAL_STRUCT_BITS \
	+ IA_CSS_PARAM_PAYLOAD_STRUCT_BITS \
	+ SIZE_OF_FRAME_GRID_STRUCT_BITS \
	+ N_UINT32_IN_SPATIAL_PARAM_TERM_STRUCT * IA_CSS_UINT32_T_BITS \
	+ N_UINT16_IN_SPATIAL_PARAM_TERM_STRUCT * IA_CSS_UINT16_T_BITS)

struct ia_css_spatial_param_terminal_s {
	/** Spatial Parameter terminal base */
	ia_css_terminal_t base;
	/** Spatial Parameter buffer handle attached to the terminal */
	ia_css_param_payload_t param_payload;
	/** Contains info for the frame of spatial parameters */
	ia_css_frame_grid_desc_t frame_grid_desc;
	/** Kernel identifier */
	uint32_t kernel_id;
	/**
	 * Points to the variable array of
	 * ia_css_frame_grid_param_section_desc_t
	 */
	uint16_t frame_grid_param_section_desc_offset;
	/**
	 * Points to array of ia_css_fragment_spatial_desc_t
	 * which constain info for the fragments of spatial parameters
	 */
	uint16_t fragment_grid_desc_offset;
};
/* ==================== Spatial Param Terminal - END ==================== */

/* ==================== Sliced Param Terminal - START ==================== */
#define N_UINT32_IN_SLICE_PARAM_SECTION_DESC_STRUCT 2

#define SIZE_OF_SLICE_PARAM_SECTION_DESC_STRUCT_BITS \
	(N_UINT32_IN_SLICE_PARAM_SECTION_DESC_STRUCT * IA_CSS_UINT32_T_BITS)

/** A Slice of parameters ready to be trasferred from/to registers */
struct ia_css_slice_param_section_desc_s {
	/** Offset of the parameter allocation in memory */
	uint32_t mem_offset;
	/** Memory allocation size needs of this parameter */
	uint32_t mem_size;
};

#define N_UINT16_IN_FRAGMENT_SLICE_DESC_STRUCT		2
#define N_PADDING_UINT8_FRAGMENT_SLICE_DESC_STRUCT	4

#define SIZE_OF_FRAGMENT_SLICE_DESC_STRUCT_BITS \
	(N_UINT16_IN_FRAGMENT_SLICE_DESC_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_PADDING_UINT8_FRAGMENT_SLICE_DESC_STRUCT * IA_CSS_UINT8_T_BITS)

struct ia_css_fragment_slice_desc_s {
	/**
	 * Points to array of ia_css_slice_param_section_desc_t
	 * which constain info for each prameter slice
	 */
	uint16_t slice_section_desc_offset;
	/** Number of slices for the parameters for this fragment */
	uint16_t slice_count;
	uint8_t padding[N_PADDING_UINT8_FRAGMENT_SLICE_DESC_STRUCT];
};

#define N_UINT32_IN_SLICED_PARAM_TERMINAL_STRUCT	1
#define N_UINT16_IN_SLICED_PARAM_TERMINAL_STRUCT	1
#define N_PADDING_UINT8_SLICED_PARAM_TERMINAL_STRUCT	2

#define SIZE_OF_SLICED_PARAM_TERM_STRUCT_BITS \
	(SIZE_OF_TERMINAL_STRUCT_BITS \
	+ IA_CSS_PARAM_PAYLOAD_STRUCT_BITS \
	+ N_UINT32_IN_SLICED_PARAM_TERMINAL_STRUCT * IA_CSS_UINT32_T_BITS \
	+ N_UINT16_IN_SLICED_PARAM_TERMINAL_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_PADDING_UINT8_SLICED_PARAM_TERMINAL_STRUCT * IA_CSS_UINT8_T_BITS)

struct ia_css_sliced_param_terminal_s {
	/** Spatial Parameter terminal base */
	ia_css_terminal_t base;
	/** Spatial Parameter buffer handle attached to the terminal */
	ia_css_param_payload_t param_payload;
	/** Kernel identifier */
	uint32_t kernel_id;
	/**
	 * Points to array of ia_css_fragment_slice_desc_t
	 * which constain info for the slicing of the parameters
	 */
	uint16_t fragment_slice_desc_offset;
	uint8_t padding[N_PADDING_UINT8_SLICED_PARAM_TERMINAL_STRUCT];
};
/* ==================== Sliced Param Terminal - END ==================== */

/* ================= Program Control Init Terminal - START ================= */
#define SIZE_OF_PROG_CONTROL_INIT_LOAD_SECTION_DESC_STRUCT_BITS	\
	(DEVICE_DESCRIPTOR_ID_BITS	\
	+ (2 * IA_CSS_UINT16_T_BITS)	\
	)
struct ia_css_program_control_init_load_section_desc_s {
	/** Device descriptor */
	device_descriptor_id_t device_descriptor_id;	/* 32 bits */
	/** Memory allocation size needs of this parameter */
	uint16_t mem_size;
	/** (Applicable to) mode bitmask */
	uint16_t mode_bitmask;
};

#define MODE_BITMASK_MEMORY          (1u << IA_CSS_CONNECTION_MEMORY)
#define MODE_BITMASK_MEMORY_STREAM   (1u << IA_CSS_CONNECTION_MEMORY_STREAM)
#define MODE_BITMASK_STREAM          (1u << IA_CSS_CONNECTION_STREAM)
#define MODE_BITMASK_DONT_CARE       (MODE_BITMASK_MEMORY | MODE_BITMASK_MEMORY_STREAM | MODE_BITMASK_STREAM)
#define MODE_BITMASK_TOP             (1u << (IA_CSS_CONNECTION_STREAM + 1))
#define MODE_BITMASK_MIDDLE          (1u << (IA_CSS_CONNECTION_STREAM + 2))
#define MODE_BITMASK_BOTTOM          (1u << (IA_CSS_CONNECTION_STREAM + 3))

struct connect_section_idx_bitfields_s {
	uint16_t plane_id : 4;
	uint16_t bottom_lines : 6;
	uint16_t top_lines : 6;
};

union connect_section_idx_t {
	struct connect_section_idx_bitfields_s as_bitfield;
	uint16_t as_uint_16;
};

#define SIZE_OF_PROG_CONTROL_INIT_CONNECT_SECTION_DESC_STRUCT_BITS	\
	(DEVICE_DESCRIPTOR_ID_BITS	\
	+ (1 * IA_CSS_UINT16_T_BITS)	\
	+ (1 * IA_CSS_UINT8_T_BITS)	\
	+ IA_CSS_TERMINAL_ID_BITS	\
	)
struct ia_css_program_control_init_connect_section_desc_s {
	/** Device descriptor */
	device_descriptor_id_t device_descriptor_id;	/* 32 bits */
	/** Connected terminal section (plane) index */
	uint16_t connect_section_idx;
	/** (Applicable to) mode bitmask */
	uint8_t mode_bitmask;
	/** Absolute referral ID for the connected terminal */
	ia_css_terminal_ID_t connect_terminal_ID;
};

#define N_PADDING_UINT8_IN_PROG_DESC_CONTROL_INFO	(1)
#define N_PADDING_UINT8_IN_PROG_CTRL_INIT_PROGRAM_DESC_STRUCT (0)
#define SIZE_OF_PROGRAM_DESC_CONTROL_INFO_STRUCT_BITS \
	(1 * IA_CSS_UINT16_T_BITS)	\
	+ (1 * IA_CSS_UINT8_T_BITS)	\
	+ (N_PADDING_UINT8_IN_PROG_DESC_CONTROL_INFO * IA_CSS_UINT8_T_BITS)

#define SIZE_OF_PROG_CONTROL_INIT_PROG_DESC_STRUCT_BITS	\
	(4 * IA_CSS_UINT16_T_BITS)	\
	+ (SIZE_OF_PROGRAM_DESC_CONTROL_INFO_STRUCT_BITS) \
	+ (2 * IA_CSS_UINT8_T_BITS) \
	+ (1 * IA_CSS_UINT16_T_BITS) \
	+ (N_PADDING_UINT8_IN_PROG_CTRL_INIT_PROGRAM_DESC_STRUCT * \
		IA_CSS_UINT8_T_BITS)

struct ia_css_program_desc_control_info_s {
	/** 12-bit process identifier */
	ia_css_process_id_t process_id;
	/** number of done acks required to close the process */
	uint8_t num_done_events;
	uint8_t padding[N_PADDING_UINT8_IN_PROG_DESC_CONTROL_INFO];
};

struct ia_css_program_control_init_program_desc_s {
	/** Number of load sections in this program */
	uint16_t load_section_count;
	/** Points to variable size array of
	 * ia_css_program_control_init_load_section_desc_s
	 * in relation to its program_desc
	 */
	uint16_t load_section_desc_offset;
	/** Number of connect sections in this program */
	uint16_t connect_section_count;
	/** Points to variable size array of
	 * ia_css_program_control_init_connect_section_desc_s
	 * in relation to its program_desc
	 */
	uint16_t connect_section_desc_offset;
	struct ia_css_program_desc_control_info_s control_info;
	/** number of load sections that are filled */
	uint8_t filled_load_sections;
	/** number of connect sections that are filled */
	uint8_t filled_connect_sections;
	/** mem_offset for the first load section */
	uint16_t load_section_mem_offset;
	/* align to 64 bits */
	/* uint8_t padding[N_PADDING_UINT8_IN_PROG_CTRL_INIT_PROGRAM_DESC_STRUCT];*/
};

#define SIZE_OF_PROG_CONTROL_INIT_TERM_STRUCT_BITS \
	(SIZE_OF_TERMINAL_STRUCT_BITS \
	+ IA_CSS_PARAM_PAYLOAD_STRUCT_BITS \
	+ (1 * IA_CSS_UINT32_T_BITS) \
	+ (2 * IA_CSS_UINT16_T_BITS) \
	)
struct ia_css_program_control_init_terminal_s {
	/** Parameter terminal base */
	ia_css_terminal_t base;
	/** Parameter buffer handle attached to the terminal */
	ia_css_param_payload_t param_payload;
	/** Fragment stride for the payload, used to find the base
	 * of the payload for a given fragment
	 */
	uint32_t payload_fragment_stride;
	/** Points to the variable array of
	 * ia_css_program_control_init_program_desc_s
	 */
	uint16_t program_section_desc_offset;
	/** Number of instantiated programs in program group (processes) */
	uint16_t program_count;
};
/* ================= Program Control Init Terminal - END ================= */

/* ==================== Program Terminal - START ==================== */

#define N_UINT32_IN_FRAG_PARAM_SEC_STRUCT 2

#define SIZE_OF_FRAG_PARAM_SEC_STRUCT_BITS \
	(N_UINT32_IN_FRAG_PARAM_SEC_STRUCT * IA_CSS_UINT32_T_BITS)

/** Fragment constant parameters section */
struct ia_css_fragment_param_section_desc_s {
	/** Offset of the parameter allocation in memory */
	uint32_t mem_offset;
	/** Memory allocation size needs of this parameter */
	uint32_t mem_size;
};

#define N_UINT16_IN_FRAG_SEQ_COMMAND_STRUCT IA_CSS_N_COMMAND_COUNT

#define SIZE_OF_FRAG_SEQ_COMMANDS_STRUCT_BITS \
	(N_UINT16_IN_FRAG_SEQ_COMMAND_STRUCT * IA_CSS_UINT16_T_BITS)

/** 4 commands packe together to save memory space */
struct ia_css_kernel_fragment_sequencer_command_desc_s {
	/** Contains the "(command_index%4) == index" command desc */
	uint16_t line_count[IA_CSS_N_COMMAND_COUNT];
};

#define N_UINT16_IN_FRAG_SEQ_INFO_STRUCT (5 * IA_CSS_N_DATA_DIMENSION + 2)

#define SIZE_OF_FRAG_SEQ_INFO_STRUCT_BITS \
	(N_UINT16_IN_FRAG_SEQ_INFO_STRUCT * IA_CSS_UINT16_T_BITS)

struct ia_css_kernel_fragment_sequencer_info_desc_s {
	/** Slice dimensions */
	uint16_t fragment_grid_slice_dimension[IA_CSS_N_DATA_DIMENSION];
	/** Nof slices */
	uint16_t fragment_grid_slice_count[IA_CSS_N_DATA_DIMENSION];
	/** Grid point decimation factor */
	uint16_t
	fragment_grid_point_decimation_factor[IA_CSS_N_DATA_DIMENSION];
	/** Relative position of grid origin to pixel origin */
	int16_t
	fragment_grid_overlay_pixel_topleft_index[IA_CSS_N_DATA_DIMENSION];
	/** Size of active fragment region */
	int16_t
	fragment_grid_overlay_pixel_dimension[IA_CSS_N_DATA_DIMENSION];
	/** If >0 it overrides the standard fragment sequencer info */
	uint16_t command_count;
	/**
	 * To be used only if command_count>0, points to the descriptors
	 * for the commands (ia_css_kernel_fragment_sequencer_command_desc_s)
	 */
	uint16_t command_desc_offset;
};

#define N_UINT16_IN_PROG_TERM_STRUCT		2
#define N_PADDING_UINT8_IN_PROG_TERM_STRUCT	0
#define N_UINT8_PROG_TERM_STRIDE_SIZE       4
#define UNIFIED_PROG_TERM_FRAG_DESC

#define SIZE_OF_PROG_TERM_STRUCT_BITS \
	(SIZE_OF_TERMINAL_STRUCT_BITS \
	+ IA_CSS_PARAM_PAYLOAD_STRUCT_BITS \
	+ N_UINT16_IN_PROG_TERM_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_UINT8_PROG_TERM_STRIDE_SIZE * IA_CSS_UINT8_T_BITS)

struct ia_css_program_terminal_s {
	/** Program terminal base */
	ia_css_terminal_t base;
	/** Program terminal buffer handle attached to the terminal */
	ia_css_param_payload_t param_payload;
	/** Points to array of ia_css_fragment_param_desc_s */
	uint16_t fragment_param_section_desc_offset;
	/** Points to array of ia_css_kernel_fragment_sequencer_info_s */
	uint16_t kernel_fragment_sequencer_info_desc_offset;
	/* stride to next fragment*/
	uint32_t payload_fragment_stride;
	/** align to 64 */
	/*uint8_t padding[N_PADDING_UINT8_IN_PROG_TERM_STRUCT];*/
};
/* ==================== Program Terminal - END ==================== */

#endif /* __IA_CSS_TERMINAL_TYPES_H */
