/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2016 Intel Corporation.
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

#ifndef __CTRL_BLK_SHARED_H
#define __CTRL_BLK_SHARED_H

#include "mem_ptr.h"

/* Cell independent interface */

/*---------------------------  Common types and structures   --------------------------*/

/** Defines the control block type data structure
 */
typedef __register struct ctrl_blk_s {
	mem_ptr_t acquire;       /**< Acquire control pointer*/
	mem_ptr_t enqueue;       /**< Enqueue control pointer*/
	mem_ptr_t dequeue;       /**< Enqueue control pointer*/
	mem_ptr_t release;       /**< Release control pointer*/
	unsigned  num_blocks;    /**< Number of queue blocks*/
	unsigned  num_subq_prod; /**< Number of subqueues for producer*/
	unsigned  num_subq_cons; /**< Number of subqueues for consumer*/
	unsigned  q_id;          /**< queue id: index in the array of queueadmin*/
} ctrl_blk_t;

/* Must be identical to the register struct ctrl_blk_t */
typedef struct {
	mem_ptr_t_mem acquire;   /**< Acquire control pointer*/
	mem_ptr_t_mem enqueue;   /**< Enqueue control pointer*/
	mem_ptr_t_mem dequeue;   /**< Enqueue control pointer*/
	mem_ptr_t_mem release;   /**< Release control pointer*/
	unsigned  num_blocks;    /**< Number of queue blocks*/
	unsigned  num_subq_prod; /**< Number of subqueues for producer*/
	unsigned  num_subq_cons; /**< Number of subqueues for consumer*/
	unsigned  q_id;          /**< queue id: index in the array of queueadmin*/
} ctrl_blk_mem_t;

/* Cell independent interface */
STORAGE_CLASS_CTRL_BLK_H unsigned ctrl_acquire_value(ctrl_blk_t ctrl, unsigned subq_idx);
STORAGE_CLASS_CTRL_BLK_H unsigned ctrl_enqueue_value(ctrl_blk_t ctrl, unsigned subq_idx);
STORAGE_CLASS_CTRL_BLK_H unsigned ctrl_dequeue_value(ctrl_blk_t ctrl, unsigned subq_idx);
STORAGE_CLASS_CTRL_BLK_H unsigned ctrl_release_value(ctrl_blk_t ctrl, unsigned subq_idx);

#if defined(__INLINE_CTRL_BLK__) && !defined(PIPE_GENERATION)
#include "ctrl_blk.shared.c"
#endif

#endif /* __CTRL_BLK_SHARED_H */
