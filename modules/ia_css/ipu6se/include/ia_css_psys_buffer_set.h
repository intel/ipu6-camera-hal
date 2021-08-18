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

#ifndef __IA_CSS_PSYS_BUFFER_SET_H
#define __IA_CSS_PSYS_BUFFER_SET_H

#include "ia_css_base_types.h"
#include "ia_css_psys_dynamic_storage_class.h"
#include "ia_css_psys_process_types.h"
#include "ia_css_terminal_types.h"
#include "ia_css_kernel_bitmap.h"
#include "ia_css_rbm.h"

#define N_UINT64_IN_BUFFER_SET_STRUCT		1
#define N_UINT16_IN_BUFFER_SET_STRUCT		1
#define N_UINT8_IN_BUFFER_SET_STRUCT		1
#define N_PADDING_UINT8_IN_BUFFER_SET_STRUCT	1
#define SIZE_OF_BUFFER_SET \
	(N_UINT64_IN_BUFFER_SET_STRUCT * IA_CSS_UINT64_T_BITS \
	+ IA_CSS_KERNEL_BITMAP_BITS \
	+ IA_CSS_KERNEL_BITMAP_BITS \
	+ IA_CSS_KERNEL_BITMAP_BITS \
	+ IA_CSS_RBM_BITS \
	+ VIED_VADDRESS_BITS \
	+ VIED_VADDRESS_BITS \
	+ N_UINT16_IN_BUFFER_SET_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_UINT8_IN_BUFFER_SET_STRUCT * IA_CSS_UINT8_T_BITS \
	+ N_PADDING_UINT8_IN_BUFFER_SET_STRUCT * IA_CSS_UINT8_T_BITS)

typedef struct ia_css_buffer_set_s ia_css_buffer_set_t;

struct ia_css_buffer_set_s {
	/** Token for user context reference */
	uint64_t token;
	/** Enable bits for each individual kernel
	 * For integration, all 0's should be treated as uninitialized and
	 * assume all processes are enabled */
	ia_css_kernel_bitmap_t kernel_enable_bitmap;
	/** Enable bits for each individual terminals per call */
	ia_css_kernel_bitmap_t terminal_enable_bitmap;
	/** Enable bits for each individual routing per call */
	ia_css_kernel_bitmap_t routing_enable_bitmap;
	/** Enable bits for routing per call */
	ia_css_rbm_t rbm;
	/** IPU virtual address of this buffer set */
	vied_vaddress_t ipu_virtual_address;
	/** IPU virtual address of the process group corresponding to this buffer set */
	vied_vaddress_t process_group_handle;
	/** Number of terminal buffer addresses in this structure */
	uint16_t terminal_count;
	/** Frame id to associate with this buffer set */
	uint8_t frame_counter;
	/** Padding for 64bit alignment */
	uint8_t padding[N_PADDING_UINT8_IN_BUFFER_SET_STRUCT];
};

#define N_PADDING_UINT8_IN_DYNAMIC_TERMINAL_DESCRIPTOR_STRUCT	3
#define SIZE_OF_DYNAMIC_TERMINAL_DESCRIPTOR \
	VIED_VADDRESS_BITS \
	+ ( 1 * IA_CSS_UINT8_T_BITS) \
	+ ( N_PADDING_UINT8_IN_DYNAMIC_TERMINAL_DESCRIPTOR_STRUCT * IA_CSS_UINT8_T_BITS)

typedef struct ia_css_dynamic_terminal_descriptor_s ia_css_dynamic_terminal_descriptor_t;
struct ia_css_dynamic_terminal_descriptor_s {
	/** Terminal buffer virtual address */
	vied_vaddress_t virtual_buffer_address;
	/** Per-frame compression control, boolean semantics */
	uint8_t enable_compression;
	/** Padding for 64bit alignment */
	uint8_t padding[N_PADDING_UINT8_IN_DYNAMIC_TERMINAL_DESCRIPTOR_STRUCT];
};

/*! Construct a buffer set object at specified location

 @param	buffer_set_mem[in]	memory location to create buffer set object
 @param	process_group[in]	process group corresponding to this buffer set
 @param	frame_counter[in]	frame number for this buffer set object

 @return pointer to buffer set object on success, NULL on error
 */
ia_css_buffer_set_t *ia_css_buffer_set_create(
	void *buffer_set_mem,
	const ia_css_process_group_t *process_group,
	const unsigned int frame_counter);

/*! Compute size (in bytes) required for full buffer set object

 @param	process_group[in]	process group corresponding to this buffer set

 @return size in bytes of buffer set object on success, 0 on error
 */
size_t ia_css_sizeof_buffer_set(
	const ia_css_process_group_t *process_group);

/*! Set a buffer address in a buffer set object

 @param	buffer_set[in]		buffer set object to set buffer in
 @param	terminal_index[in]	terminal index to use as a reference between
				buffer and terminal
 @param	buffer[in]		buffer address to store

 @return 0 on success, -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_buffer(
	ia_css_buffer_set_t *buffer_set,
	const unsigned int terminal_index,
	const vied_vaddress_t buffer);

/*! Get virtual buffer address from a buffer set object and terminal object by
   resolving the index used

 @param	buffer_set[in]		buffer set object to get buffer from
 @param	terminal[in]		terminal object to get buffer of

 @return virtual buffer address on success, VIED_NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
vied_vaddress_t ia_css_buffer_set_get_buffer(
	const ia_css_buffer_set_t *buffer_set,
	const ia_css_terminal_t *terminal);

/*! Set ipu virtual address of a buffer set object within the buffer set object

 @param	buffer_set[in]		buffer set object to set ipu address in
 @param	ipu_vaddress[in]	ipu virtual address of the buffer set object

 @return 0 on success, -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_ipu_address(
	ia_css_buffer_set_t *buffer_set,
	const vied_vaddress_t ipu_vaddress);

/*! Get ipu virtual address from a buffer set object

 @param	buffer_set[in]		buffer set object to get ipu address from

 @return virtual buffer set address on success, VIED_NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
vied_vaddress_t ia_css_buffer_set_get_ipu_address(
	const ia_css_buffer_set_t *buffer_set);

/*! Set kernel enable bitmap of a buffer set object within the buffer set object

 @param	buffer_set[in]			buffer set object to set kernel enable bitmap in
 @param	kernel_enable_bitmap[in]	kernel enable bitmap of the buffer set object

 @return 0 on success, -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_kernel_enable_bitmap(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_kernel_bitmap_t kernel_enable_bitmap);

/*! Get kernel enable bitmap from a buffer set object

 @param	buffer_set[in]		buffer set object to get kernel enable bitmap from

 @return buffer set kernel enable bitmap on success, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_buffer_set_get_kernel_enable_bitmap(
	const ia_css_buffer_set_t *buffer_set);

/*! Set terminal enable bitmap of a buffer set object within the buffer set object

 @param	buffer_set[in]			buffer set object to set kernel enable bitmap in
 @param	kernel_enable_bitmap[in]	kernel enable bitmap of the buffer set object

 @return 0 on success, -1 on error

 @todo New for POC
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_terminal_enable_bitmap(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_kernel_bitmap_t terminal_enable_bitmap);

/*! Get terminal enable bitmap from a buffer set object

 @param	buffer_set[in]		buffer set object to get kernel enable bitmap from

 @return buffer set terminal enable bitmap on success, 0 on error

 @todo New for POC
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_buffer_set_get_terminal_enable_bitmap(
	const ia_css_buffer_set_t *buffer_set);

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_routing_enable_bitmap(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_kernel_bitmap_t terminal_enable_bitmap);

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_buffer_set_get_routing_enable_bitmap(
	const ia_css_buffer_set_t *buffer_set);

/*! Set Routing BitMap of a buffer set object within the buffer set object

 @param	buffer_set[in]			buffer set object to set kernel enable bitmap in
 @param	kernel_enable_bitmap[in]	kernel enable bitmap of the buffer set object

 @return 0 on success, -1 on error

 @todo New for POC
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_rbm(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_rbm_t rbm);

/*! Get Routing BitMap from a buffer set object

 @param	buffer_set[in]		buffer set object to get kernel enable bitmap from

 @return buffer set terminal enable bitmap on success, 0 on error

 @todo New for POC
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_rbm_t ia_css_buffer_set_get_rbm(
	const ia_css_buffer_set_t *buffer_set);

/*! Set process group handle in a buffer set object

 @param	buffer_set[in]			buffer set object to set handle in
 @param	process_group_handle[in]	process group handle of the buffer set
					object

 @return 0 on success, -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_process_group_handle(
	ia_css_buffer_set_t *buffer_set,
	const vied_vaddress_t process_group_handle);

/*! Get process group handle from a buffer set object

 @param	buffer_set[in]		buffer set object to get handle from

 @return virtual process group address on success, VIED_NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
vied_vaddress_t ia_css_buffer_set_get_process_group_handle(
	const ia_css_buffer_set_t *buffer_set);

/*! Set token of a buffer set object within the buffer set object

 @param	buffer_set[in]		buffer set object to set ipu address in
 @param	token[in]		token of the buffer set object

 @return 0 on success, -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_token(
	ia_css_buffer_set_t *buffer_set,
	const uint64_t token);

/*! Get token from a buffer set object

 @param	buffer_set[in]		buffer set object to get token from

 @return token on success, NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint64_t ia_css_buffer_set_get_token(
	const ia_css_buffer_set_t *buffer_set);

#ifdef __IA_CSS_PSYS_DYNAMIC_INLINE__
#include "ia_css_psys_buffer_set_impl.h"
#endif /* __IA_CSS_PSYS_DYNAMIC_INLINE__ */

#endif /* __IA_CSS_PSYS_BUFFER_SET_H */
