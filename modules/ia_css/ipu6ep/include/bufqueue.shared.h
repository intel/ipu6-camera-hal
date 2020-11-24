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
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 */

#ifndef __BUFQUEUE_SHARED_H
#define __BUFQUEUE_SHARED_H

/* System and cell agnostic interface of kernel */

#include <storage_class.h>

#include "buf_blk.h"
#include "ctrl_blk.h"

#define CIRCULAR_QUEUE_MIRROR 2

/** Defines the queue type data structure
 */
typedef __register struct {
	ctrl_blk_t ctrl;                 /**< control block*/
	buf_blk_t  buffer;               /**< buffer0 block*/
	buf_blk_t  buffer1;              /**< buffer1 block*/
	buf_blk_t  buffer2;              /**< buffer2 block*/
	buf_blk_t  buffer3;              /**< buffer3 block*/
	buf_blk_t  buffer4;              /**< buffer4 block*/
} bufqueue_t;

typedef struct {
	ctrl_blk_mem_t ctrl;             /**< control block*/
	buf_blk_mem_t  buffer;           /**< buffer0 block*/
	buf_blk_mem_t  buffer1;          /**< buffer1 block*/
	buf_blk_mem_t  buffer2;          /**< buffer2 block*/
	buf_blk_mem_t  buffer3;          /**< buffer3 block*/
	buf_blk_mem_t  buffer4;          /**< buffer4 block*/
} bufqueue_mem_t;

/* this is a subset of bufqueue_t */
typedef __register struct {
	mem_ptr_t ht_mem_ptr;
	unsigned wrapcount;
} bufqueue_ptr_t;

typedef struct {
	uint32_t offset;                 /**< mem_ptr_t::buf_offset */
	uint16_t mem;                    /**< mem_ptr_t::buf_mem */
	uint16_t wrapcount;              /**< ctrl_blk_t::num_blocks */
} bufqueue_ptr_mem_t;

typedef __register struct isp_dmem_queues_ptrs {
	unsigned *deq_head;
	unsigned *rel_head;
	unsigned *acq_tail;
	unsigned *enq_tail;
} isp_dmem_head_tail_ptrs_t;

typedef __register struct block_dimensions_s {
	uint32_t block_width;
	uint32_t block_height;
} block_dimensions_t;

/****************************************************************************/
/* Public interface */
/****************************************************************************/

/* Cell independent interface */

STORAGE_CLASS_BUFQUEUE_H bufqueue_t
queue_create_1buff(
	ctrl_blk_t  ctrl,
	buf_blk_t   buffer);

STORAGE_CLASS_BUFQUEUE_H bufqueue_t
queue_create_2buff(
	ctrl_blk_t  ctrl,
	buf_blk_t   buffer0,
	buf_blk_t   buffer1);

STORAGE_CLASS_BUFQUEUE_H bufqueue_t
queue_create_3buff(
	ctrl_blk_t  ctrl,
	buf_blk_t   buffer0,
	buf_blk_t   buffer1,
	buf_blk_t   buffer2);

STORAGE_CLASS_BUFQUEUE_H bufqueue_t
queue_create_4buff(
	ctrl_blk_t  ctrl,
	buf_blk_t   buffer0,
	buf_blk_t   buffer1,
	buf_blk_t   buffer2,
	buf_blk_t   buffer3);

STORAGE_CLASS_BUFQUEUE_H bufqueue_t
queue_create_5buff(
	ctrl_blk_t  ctrl,
	buf_blk_t   buffer0,
	buf_blk_t   buffer1,
	buf_blk_t   buffer2,
	buf_blk_t   buffer3,
	buf_blk_t   buffer4);

STORAGE_CLASS_BUFQUEUE_C bufqueue_t
queue_explicit_create_2buff(buf_mem_t mem_type,
		uint32_t start_offset,
		uint32_t num_blocks,
		uint32_t size_buf0,
		uint32_t width_buf0,
		uint32_t height_buf0,
		uint32_t size_buf1,
		uint32_t width_buf1,
		uint32_t height_buf1);

STORAGE_CLASS_BUFQUEUE_H bufqueue_t
queue_explicit_create_3buff(buf_mem_t mem_type,
		uint32_t start_offset,
		uint32_t num_blocks,
		uint32_t size_buf0,
		uint32_t width_buf0,
		uint32_t height_buf0,
		uint32_t size_buf1,
		uint32_t width_buf1,
		uint32_t height_buf1,
		uint32_t size_buf2,
		uint32_t width_buf2,
		uint32_t height_buf2);

STORAGE_CLASS_BUFQUEUE_H bufqueue_t
bufqueue_explicit_create_1buff_with_stride(buf_mem_t mem_type,
		unsigned start_offset,
		unsigned num_blocks,
		unsigned size_buf0,
		unsigned width_buf0,
		unsigned stride_buf0,
		unsigned height_buf0);

STORAGE_CLASS_BUFQUEUE_H block_dimensions_t calc_block_dimensions(
			uint32_t buffer_width,
			uint32_t buffer_height,
			uint32_t fragment_width,
			uint32_t fragment_height);

/**
 * @brief  obtain the queue index
 *
 * @param  q        queue
 *
 * @return q_id
 */
STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_id(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H bufqueue_t queue_ident(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H bool queue_num_acquired_is_zero(bufqueue_t q);
STORAGE_CLASS_BUFQUEUE_H bool queue_num_acquired_is_zero_subq(bufqueue_t q, unsigned idx);

STORAGE_CLASS_BUFQUEUE_H bool queue_num_dequeued_is_zero(bufqueue_t q);
STORAGE_CLASS_BUFQUEUE_H bool queue_num_dequeued_is_zero_subq(bufqueue_t q, unsigned idx);

STORAGE_CLASS_BUFQUEUE_H bool is_queue_empty(bufqueue_t q);
STORAGE_CLASS_BUFQUEUE_H bool is_queue_full(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H bool queue_can_acquire_n(bufqueue_t q, unsigned n);
STORAGE_CLASS_BUFQUEUE_H bool queue_can_dequeue_n(bufqueue_t q, unsigned n);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_acquire(bool cond, bufqueue_t q);
STORAGE_CLASS_BUFQUEUE_H void     queue_enqueue(bool cond, bufqueue_t q);
STORAGE_CLASS_BUFQUEUE_H unsigned queue_dequeue(bool cond, bufqueue_t q);
STORAGE_CLASS_BUFQUEUE_H void     queue_release(bool cond, bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_acquire_n(bufqueue_t q, unsigned n);
STORAGE_CLASS_BUFQUEUE_H void     queue_enqueue_n(bufqueue_t q, unsigned n);
STORAGE_CLASS_BUFQUEUE_H unsigned queue_dequeue_n(bufqueue_t q, unsigned n);
STORAGE_CLASS_BUFQUEUE_H void     queue_release_n(bufqueue_t q, unsigned n);

STORAGE_CLASS_BUFQUEUE_H bool is_queue_empty_subq(bufqueue_t q, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H bool is_queue_full_subq(bufqueue_t q, unsigned idx);

STORAGE_CLASS_BUFQUEUE_H bool queue_can_acquire_n_subq(bufqueue_t q, unsigned n, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H bool queue_can_dequeue_n_subq(bufqueue_t q, unsigned n, unsigned idx);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_acquire_subq(bool cond, bufqueue_t q, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H void     queue_enqueue_subq(bool cond, bufqueue_t q, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H unsigned queue_dequeue_subq(bool cond, bufqueue_t q, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H void     queue_release_subq(bool cond, bufqueue_t q, unsigned idx);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_acquire_n_subq(bufqueue_t q, unsigned n, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H void     queue_enqueue_n_subq(bufqueue_t q, unsigned n, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H unsigned queue_dequeue_n_subq(bufqueue_t q, unsigned n, unsigned idx);
STORAGE_CLASS_BUFQUEUE_H void     queue_release_n_subq(bufqueue_t q, unsigned n, unsigned idx);

STORAGE_CLASS_BUFQUEUE_H buf_blk_t queue_get_buf0(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H buf_blk_t queue_get_buf1(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H buf_blk_t queue_get_buf2(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H mem_ptr_t queue_get_block_base_ptr(bufqueue_t q, unsigned buf_idx);

STORAGE_CLASS_BUFQUEUE_H mem_ptr_t queue_get_mem_ptr(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_block_size(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_numblocks(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_wrapcount(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_blockwidth(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_blockheight(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_acquired_buf_idx(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_dequeued_buf_idx(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H unsigned queue_get_acquired_buf_idx_subq(bufqueue_t q, unsigned idx);

STORAGE_CLASS_BUFQUEUE_H bufqueue_ptr_t queue_get_qptr_producer(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H bufqueue_ptr_t queue_get_qptr_consumer(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H mem_ptr_t queue_get_enqueue_ptr(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H mem_ptr_t queue_get_release_ptr(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H mem_ptr_t queue_get_dequeue_ptr(bufqueue_t q);

STORAGE_CLASS_BUFQUEUE_H bufqueue_ptr_t load_bufqueue_ptr(const bufqueue_ptr_mem_t *from);

STORAGE_CLASS_BUFQUEUE_H void store_bufqueue_ptr(bufqueue_ptr_mem_t *to, bufqueue_ptr_t from);

STORAGE_CLASS_BUFQUEUE_H void store_bufqueue(bufqueue_mem_t *to, bufqueue_t from);

STORAGE_CLASS_BUFQUEUE_H void queue_check_acquired_buf_idx(bufqueue_t q, unsigned buf_idx, bool cond);

STORAGE_CLASS_BUFQUEUE_H void queue_check_acquired_buf_idx_subq(bufqueue_t q, unsigned subq_idx,
								unsigned buf_idx, bool cond);

/* Implementation for isp and host are different*/
STORAGE_CLASS_BUFQUEUE_H unsigned increment_bufqueue_ctrl_ptr(
		mem_ptr_t ptr, unsigned idx, unsigned inc, unsigned wrapcount);

#if defined(__INLINE_BUFQUEUE__) && !defined(PIPE_GENERATION)
#include "bufqueue.shared.c"
#endif

#endif /* __BUFQUEUE_SHARED_H */
