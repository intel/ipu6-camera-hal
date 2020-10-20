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

#ifndef __IA_CSS_PSYS_PROGRAM_MANIFEST_H
#define __IA_CSS_PSYS_PROGRAM_MANIFEST_H

/*! \file */

/** @file ia_css_psys_program_manifest.h
 *
 * Define the methods on the program manifest object that are not part of a
 * single interface.
 *
 * @note The "set" functions here are for testing simulation only.
 * Real manifests are generated in advance and must be considered read-only from
 * the production user's perspective.
 */

#include "ia_css_psys_static_storage_class.h"

#include <ia_css_psys_manifest_types.h>

#include <type_support.h>			/* uint8_t */

#include <ia_css_psys_program_manifest.sim.h>

#include <ia_css_psys_program_manifest.hsys.user.h>

#include <ia_css_kernel_bitmap.h>		/* ia_css_kernel_bitmap_t */

/*
 * Resources needs
 */
#include <ia_css_psys_program_manifest.hsys.kernel.h>

#define IA_CSS_PROGRAM_INVALID_DEPENDENCY	((uint8_t)(-1))
#define IA_CSS_PROGRAM_INVALID_DEC_CONFIG	((uint8_t)(-1))

/** Retrieve extended manifest information generally not visible
 *  to the SW stack
 *
 *  @see ia_css_program_manifest_ext_t
 *  @note Function only to be used in FW
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_manifest_ext_t *ia_css_program_manifest_get_extension(const ia_css_program_manifest_t *program);

/*! Check if the program manifest object specifies a fixed cell allocation

 @param	manifest[in]			program manifest object

 @return has_fixed_cell, false on invalid argument
 */
extern bool ia_css_has_program_manifest_fixed_cell(
	const ia_css_program_manifest_t			*manifest);

/*! Get the stored size of the program manifest object

 @param	manifest[in]			program manifest object

 @return size, 0 on invalid argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
size_t ia_css_program_manifest_get_size(
	const ia_css_program_manifest_t			*manifest);

/*! Get the program ID of the program manifest object

 @param	manifest[in]			program manifest object

 @return program ID, IA_CSS_PROGRAM_INVALID_ID on invalid argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_ID_t ia_css_program_manifest_get_program_ID(
	const ia_css_program_manifest_t			*manifest);

/*! Set the program ID of the program manifest object

 @param	manifest[in]			program manifest object

 @param program ID

 @return 0 on success, -1 on invalid manifest argument
 */
extern int ia_css_program_manifest_set_program_ID(
	ia_css_program_manifest_t			*manifest,
	ia_css_program_ID_t id);

/*! Get the (pointer to) the program *group* manifest which contains
 * this program manifest

 @param	manifest[in]			program manifest object

 @return the pointer to the parent, NULL on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_group_manifest_t *ia_css_program_manifest_get_parent(
	const ia_css_program_manifest_t			*manifest);

/*! Set the offset to the beginning of the program *group* manifest
 * which contains this program manifest

 @param	manifest[in]			program manifest object
 @param	program_offset[in]		this program's offset from
					program_group_manifest's base address.

 @return < 0 on invalid manifest argument
 */
extern int ia_css_program_manifest_set_parent_offset(
	ia_css_program_manifest_t			*manifest,
	int32_t program_offset);

/*! Get the type of the program manifest object

 @param	manifest[in]			program manifest object

 @return program type, limit value (IA_CSS_N_PROGRAM_TYPES) on invalid manifest
	argument
*/
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_type_t ia_css_program_manifest_get_type(
	const ia_css_program_manifest_t			*manifest);

/*! Set the type of the program manifest object

 @param	manifest[in]			program manifest object
 @param	program_type[in]		program type

 @return < 0 on invalid manifest argument
 */
extern int ia_css_program_manifest_set_type(
	ia_css_program_manifest_t			*manifest,
	const ia_css_program_type_t			program_type);

/*! Set the cell id of the program manifest object

 @param	manifest[in]			program manifest object
 @param	program_cell_id[in]		program cell id

 @return < 0 on invalid manifest argument
  */
extern int ia_css_program_manifest_set_cell_ID(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_cell_ID_t			cell_id);

/*! Set the cell type of the program manifest object

 @param	manifest[in]			program manifest object
 @param	program_cell_type[in]		program cell type

 @return < 0 on invalid manifest argument
 */
extern int ia_css_program_manifest_set_cell_type_ID(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_cell_type_ID_t			cell_type_id);

/*!
 * Get input terminal id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 *
 * @return registered terminal_id for idx
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_input_terminal_id(
	ia_css_program_manifest_t   *manifest,
	unsigned char                   idx);

/*!
 * Get output terminal id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 *
 * @return registered terminal_id for idx
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_output_terminal_id(
	ia_css_program_manifest_t   *manifest,
	unsigned char                    idx);

/*!
 * Set output terminal id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 * @param	val[in]			value
 *
 */
extern void ia_css_program_manifest_set_output_terminal_id(
	ia_css_program_manifest_t   *manifest,
	uint8_t                    idx,
	uint8_t                    val);

/*!
 * Set input terminal id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 * @param	val[in]			value
 *
 */
extern void ia_css_program_manifest_set_input_terminal_id(
	ia_css_program_manifest_t   *manifest,
	uint8_t                    idx,
	uint8_t                    val);

/*!
 * Get compression input MMU stream id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 *
 * @return registered stream_id for idx
 */

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_input_stream_id(
	ia_css_program_manifest_t   *manifest,
	unsigned char                   idx);

/*!
 * Get compression output MMU stream id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 *
 * @return registered stream_id for idx
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_output_stream_id(
	ia_css_program_manifest_t   *manifest,
	unsigned char                    idx);

/*!
 * Set compression output MMU stream id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 * @param	val[in]			value
 *
 */
extern void ia_css_program_manifest_set_output_stream_id(
	ia_css_program_manifest_t   *manifest,
	uint8_t                    idx,
	uint8_t                    val);

/*!
 * Set *compression* input MMU stream id [val] at [idx]
 *
 * @param	manifest[in]		program manifest object
 * @param	idx[in]			index
 * @param	val[in]			value
 *
 */
extern void ia_css_program_manifest_set_input_stream_id(
	ia_css_program_manifest_t   *manifest,
	uint8_t                    idx,
	uint8_t                    val);

/*! Set cells bitmap for the program

 @param	manifest[in]			program manifest object
 @param	bitmap[in]				bitmap

 @return 0 when not applicable and/or invalid arguments
 */
extern int ia_css_program_manifest_set_cells_bitmap(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_resource_bitmap_t	bitmap);

/*! Get cells bitmap for the program

 @param	manifest[in]			program manifest object

 @return 0 when not applicable and/or invalid arguments
 */
extern vied_nci_resource_bitmap_t ia_css_program_manifest_get_cells_bitmap(
	const ia_css_program_manifest_t			*manifest);

/*! Set DFM port bitmap for the program

 @param	manifest[in]			program manifest object
 @param	dfm_type_id[in]			DFM resource type ID
 @param	bitmap[in]				bitmap

 @return 0 when not applicable and/or invalid arguments
 */
extern int ia_css_program_manifest_set_dfm_port_bitmap(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_dfm_id_t			dfm_type_id,
	const vied_nci_resource_bitmap_t	bitmap);
#if HAS_DFM
/*! Get bitmap of DFM ports requested for the program

 @param	manifest[in]			program manifest object
 @param	dfm_type_id[in]			DFM resource type ID

 @return DFM port bitmap
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_bitmap_t ia_css_program_manifest_get_dfm_port_bitmap(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_dfm_id_t			dfm_type_id);
#endif

/*! Set active DFM port specification bitmap for the program

 @param	manifest[in]			program manifest object
 @param	dfm_type_id[in]			DFM resource type ID
 @param	bitmap[in]				bitmap

 @return 0 when not applicable and/or invalid arguments
 */
extern int ia_css_program_manifest_set_dfm_active_port_bitmap(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_dfm_id_t			dfm_type_id,
	const vied_nci_resource_bitmap_t	bitmap);

#if HAS_DFM
/*! Get active DFM port specification bitmap for the program

 @param	manifest[in]			program manifest object
 @param	dfm_type_id[in]			DFM resource type ID

 @return 0 when not applicable and/or invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_bitmap_t ia_css_program_manifest_get_dfm_active_port_bitmap(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_dfm_id_t			dfm_type_id);
#endif

/*! Set DFM device relocatability specification for the program

 @param	manifest[in]			program manifest object
 @param	dfm_type_id[in]			DFM resource type ID
 @param is_relocatable[in]		1 if dfm device ports are relocatable, 0 otherwise

 @return 0 when not applicable and/or invalid arguments
 */
extern int ia_css_program_manifest_set_is_dfm_relocatable(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_dfm_id_t			dfm_type_id,
	const uint8_t						is_relocatable);

#if HAS_DFM
/*! Get DFM device relocatability specification for the program

 @param	manifest[in]			program manifest object
 @param	dfm_type_id[in]			DFM resource type ID

 @return 1 if dfm device ports are relocatable, 0 otherwise
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_is_dfm_relocatable(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_dfm_id_t			dfm_type_id);
#endif

/*! Get the memory resource (size) specification for a memory
 that belongs to the cell where the program will be mapped

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type ID

 @return 0 when not applicable and/or invalid arguments

 @deprecated Implementation is trival (returns 0) and
 no known users or known intention for future use.
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_int_mem_size(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id);

/*! Set the memory resource (size) specification for a memory
 that belongs to the cell where the program will be mapped

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type id
 @param	int_mem_size[in]		internal memory size

 @return < 0 on invalid arguments
 @deprecated see ia_css_program_manifest_get_int_mem_size()
 */
extern int ia_css_program_manifest_set_int_mem_size(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id,
	const vied_nci_resource_size_t			int_mem_size);

/*! Get the memory resource (size) specification for a memory
 that does not belong to the cell where the program will be mapped

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type ID

 @return 0 when not applicable and/or invalid arguments

 @see ia_css_program_manifest_get_ext_mem_offset()
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_ext_mem_size(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id);

/*! Set the memory resource (size) specification for a memory
 that does not belong to the cell where the program will be mapped

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type id
 @param	ext_mem_size[in]		external memory size

 @return < 0 on invalid arguments

 @see ia_css_program_manifest_get_ext_mem_offset()
 */
extern int ia_css_program_manifest_set_ext_mem_size(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id,
	const vied_nci_resource_size_t			ext_mem_size);

#if HAS_DFM
/*! Get a device channel resource (size) specification

 @param	manifest[in]			program manifest object
 @param	dev_chn_id[in]			device channel ID

 @return 0 when not applicable and/or invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_dev_chn_size(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_chn_ID_t			dev_chn_id);
#endif

/*! Set a device channel resource (size) specification

 @param	manifest[in]			program manifest object
 @param	dev_chn_id[in]			device channel ID
 @param	dev_chn_size[in]		device channel size

 @return < 0 on invalid arguments
 */
extern int ia_css_program_manifest_set_dev_chn_size(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_chn_ID_t			dev_chn_id,
	const vied_nci_resource_size_t			dev_chn_size);

/*! Set a device channel resource (offset) specification

 @param	manifest[in]			program manifest object
 @param	dev_chn_id[in]			device channel ID
 @param	dev_chn_offset[in]		device channel offset

 @see ia_css_program_manifest_get_dev_chn_offset()

 @return < 0 on invalid arguments
 */
extern int ia_css_program_manifest_set_dev_chn_offset(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_chn_ID_t			dev_chn_id,
	const vied_nci_resource_size_t			dev_chn_offset);

/*! Set the memory resource (offset) specification for a memory
 that does not belong to the cell where the program will be mapped

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type id
 @param	ext_mem_offset[in]		external memory offset

 @see ia_css_program_manifest_get_ext_mem_offset()

 @return < 0 on invalid arguments
 */
extern int ia_css_program_manifest_set_ext_mem_offset(
	ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id,
	const vied_nci_resource_size_t			ext_mem_offset);

#if HAS_DFM
/*! Get a device channel resource (offset) specification

 Used to define a resource dependency on devices that support
 parallel users via channel multiplexing within the device.

 Channel resources can be "relocatable". That is, some channel
 is required, but the exact channel is not important and
 can be assigned at runtime.

 Other channel resources are static "fixed" in advance.  Either
 each channel is customized for a particular need such that
 the exact channel needs to be specified.  Alternatively, there
 are enough channels resources available for the worst case
 system requirement such that the channels can be statically
 assigned in advance "fixed" channels.

 Both kinds of resource dependency specification are supported here.

 @param	manifest[in]			program manifest object
 @param	dev_chn_id[in]			device channel ID

 @return Valid fixed offset (if value is greater or equal to 0) or
  IA_CSS_PROGRAM_MANIFEST_RESOURCE_OFFSET_IS_RELOCATABLE if offset
  is relocatable
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_dev_chn_offset(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_chn_ID_t				dev_chn_id);
#endif

/*! Get the memory resource (offset) specification for a memory
 that does not belong to the cell where the program will be mapped.

 Memory specified here is typically SRAM within IPU used for
 intermediate storage during data transfer between processing
 stages/main memory or as computation scratch space.

 The offset specified essentially defines the location of the memory
 within the memory device.  It can be fixed in advance (static allocation)
 or left to the system to determine at runtime.

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type ID

 @return Valid fixed offset (if value is greater or equal to 0) or
  IA_CSS_PROGRAM_MANIFEST_RESOURCE_OFFSET_IS_RELOCATABLE if offset
  is relocatable
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_ext_mem_offset(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id);

/*! Get the kernel composition of the program manifest object

  The collection of kernels described in the bitmap is defined
  at the program group level. This function returns the subset
  of kernels that are part of this program.
 @param	manifest[in]			program manifest object

 @return bitmap, 0 on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_program_manifest_get_kernel_bitmap(
	const ia_css_program_manifest_t			*manifest);

/*! Set the kernel dependency of the program manifest object

 @param	manifest[in]			program manifest object
 @param	kernel_bitmap[in]		kernel composition bitmap

 @return < 0 on invalid arguments
 */
extern int ia_css_program_manifest_set_kernel_bitmap(
	ia_css_program_manifest_t			*manifest,
	const ia_css_kernel_bitmap_t			kernel_bitmap);

/*! Get the number of programs this programs depends on from the program group
 * manifest object

 @param	manifest[in]			program manifest object

 @return program dependency count
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_program_dependency_count(
	const ia_css_program_manifest_t			*manifest);

/*! Get the index of the program which the programs at this index depends on
    from the program manifest object

 @param	manifest[in]			program manifest object

 @return program dependency,
	IA_CSS_PROGRAM_INVALID_DEPENDENCY on invalid arguments
	*/
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_program_dependency(
	const ia_css_program_manifest_t			*manifest,
	const unsigned int				index);

/*! Set the index of the program which the programs at this index depends on
    in the program manifest object

 @param	manifest[in]			program manifest object

 @return program dependency
 */
extern int ia_css_program_manifest_set_program_dependency(
	ia_css_program_manifest_t			*manifest,
	const uint8_t					program_dependency,
	const unsigned int				index);

/*! Get the number of terminals this programs depends on from the program group
 * manifest object

 @param	manifest[in]			program manifest object

 @return program dependency count
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_terminal_dependency_count(
	const ia_css_program_manifest_t			*manifest);

/*! Get the index of the terminal which the programs at this index depends on
    from the program manifest object

 @param	manifest[in]			program manifest object

 @return terminal dependency, IA_CSS_PROGRAM_INVALID_DEPENDENCY on error
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_manifest_get_terminal_dependency(
	const ia_css_program_manifest_t			*manifest,
	const unsigned int				index);

/*! Set the index of the terminal which the programs at this index depends on
    in the program manifest object

 @param	manifest[in]			program manifest object

 @return < 0 on invalid arguments
 */
extern int ia_css_program_manifest_set_terminal_dependency(
	ia_css_program_manifest_t			*manifest,
	const uint8_t					terminal_dependency,
	const unsigned int				index);

/*! Check if the program manifest object specifies a subnode program

 @param	manifest[in]			program manifest object

 @return is_subnode, false on invalid argument
 @deprecated for IPU7
 */
extern bool ia_css_is_program_manifest_subnode_program_type(
	const ia_css_program_manifest_t			*manifest);

/*! Check if the program manifest object specifies a supernode program

 @param	manifest[in]			program manifest object

 @return is_supernode, false on invalid argument
 @deprecated for IPU7
 */
extern bool ia_css_is_program_manifest_supernode_program_type(
	const ia_css_program_manifest_t			*manifest);
/*! Check if the program manifest object specifies a singular program

 @param	manifest[in]			program manifest object

 @return is_singular, false on invalid argument
 @deprecated for IPU7
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_is_program_manifest_singular_program_type(
	const ia_css_program_manifest_t			*manifest);

#ifdef __IA_CSS_PSYS_STATIC_INLINE__
#include "ia_css_psys_program_manifest_impl.h"
#endif /* __IA_CSS_PSYS_STATIC_INLINE__ */

#endif /* __IA_CSS_PSYS_PROGRAM_MANIFEST_H */
