/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2017 Intel Corporation.
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
 * delivery of the Materials, either expressly , by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 */

#ifndef __BUF_BLK_SHARED_H
#define __BUF_BLK_SHARED_H

#include "mem_ptr.h"

/* Cell independent interface */

/*---------------------------  Common types and structures   --------------------------*/

/** Defines the buffer type data structure
 */
typedef __register struct buffer_s {
	mem_ptr_t ptr;		 /**< buffer pointer*/
	unsigned  block_size;	 /**< Size of individual queue block */
	unsigned  block_width;	 /**< Width of individual queue block (optional) */
	unsigned  block_height;  /**< Width of individual queue block (optional) */
	unsigned  row_stride;    /**< Stride to the next row within the block */
	unsigned  buf_stride;    /**< Stride to the next buffer within the queue */
} buf_blk_t;

typedef struct {
	mem_ptr_t_mem ptr;	 /**< buffer pointer*/
	unsigned  block_size;	 /**< Size of individual queue block */
	unsigned  block_width;	 /**< Width of individual queue block (optional) */
	unsigned  block_height;	 /**< Width of individual queue block (optional) */
	unsigned  row_stride;    /**< Stride to the next row within the block */
	unsigned  buf_stride;    /**< Stride to the next buffer within the queue */
} buf_blk_mem_t;

/*---------------------------  Common types and structures   --------------------------*/

/* -------------------------------- Functions --------------------------------------------*/

STORAGE_CLASS_BUF_BLK_H void
buffer_print(buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H buf_blk_t
buffer_create(
	mem_ptr_t    ptr,
	unsigned     block_size);

STORAGE_CLASS_BUF_BLK_H buf_blk_t
buffer_create_2d(
	mem_ptr_t    ptr,
	unsigned     block_size,
	unsigned     block_width,
	unsigned     block_height);

STORAGE_CLASS_BUF_BLK_H buf_blk_t
buffer_create_2d_with_stride(
	mem_ptr_t    ptr,
	unsigned     block_size,
	unsigned     block_width,
	unsigned     block_height,
	unsigned     row_stride);

STORAGE_CLASS_BUF_BLK_H buf_blk_t
buffer_create_2d_hor(
	mem_ptr_t    ptr,
	unsigned     block_size,
	unsigned     block_width,
	unsigned     block_height,
	unsigned     row_stride,
	unsigned     buf_stride);

STORAGE_CLASS_BUF_BLK_H mem_ptr_t
buffer_get_block_base_ptr(
	buf_blk_t buffer,
	unsigned idx);

STORAGE_CLASS_BUF_BLK_H mem_ptr_t
buffer_get_address_to_idx_buffer(
	buf_blk_t buffer,
	unsigned idx);

STORAGE_CLASS_BUF_BLK_H mem_ptr_t
buffer_get_address_to_idx_buffer_per_plane(
	buf_blk_t buffer,
	unsigned num_planes,
	unsigned num_buf,
	unsigned idx);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_block_width(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_row_stride(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_row_stride_in_bytes(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_row_stride_in_elements(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_buffer_height(
	buf_blk_t buffer,
	unsigned num_buf);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_block_width_in_bytes(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_block_width_in_elements(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_block_height(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H unsigned
buffer_get_block_size(
	buf_blk_t buffer);

STORAGE_CLASS_BUF_BLK_H bool
buffer_is_valid(
	buf_blk_t buffer);

#if defined(__INLINE_BUF_BLK__) && !defined(PIPE_GENERATION)
#include "buf_blk.shared.c"
#endif

#endif /* __BUF_BLK_SHARED_H */
