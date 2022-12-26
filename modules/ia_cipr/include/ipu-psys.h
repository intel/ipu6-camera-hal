/****************************************************************************
 * Copyright (C) 2019 Intel Corporation.
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef _UAPI_IPU_PSYS_H
#define _UAPI_IPU_PSYS_H
#include <linux/types.h>
struct ipu_psys_capability {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint32_t version;
  uint8_t driver[20];
  uint32_t pg_count;
  uint8_t dev_model[32];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint32_t reserved[17];
} __attribute__((packed));
struct ipu_psys_event {
  uint32_t type;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint64_t user_token;
  uint64_t issue_id;
  uint32_t buffer_idx;
  uint32_t error;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  int32_t reserved[2];
} __attribute__((packed));
#define IPU_PSYS_EVENT_TYPE_CMD_COMPLETE 1
#define IPU_PSYS_EVENT_TYPE_BUFFER_COMPLETE 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ipu_psys_buffer {
  uint64_t len;
  union {
    int fd;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    void* userptr;
    uint64_t reserved;
  } base;
  uint32_t data_offset;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint32_t bytes_used;
  uint32_t flags;
  uint32_t reserved[2];
} __attribute__((packed));
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPU_BUFFER_FLAG_INPUT (1 << 0)
#define IPU_BUFFER_FLAG_OUTPUT (1 << 1)
#define IPU_BUFFER_FLAG_MAPPED (1 << 2)
#define IPU_BUFFER_FLAG_NO_FLUSH (1 << 3)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPU_BUFFER_FLAG_DMA_HANDLE (1 << 4)
#define IPU_BUFFER_FLAG_USERPTR (1 << 5)
#define IPU_PSYS_CMD_PRIORITY_HIGH 0
#define IPU_PSYS_CMD_PRIORITY_MED 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPU_PSYS_CMD_PRIORITY_LOW 2
#define IPU_PSYS_CMD_PRIORITY_NUM 3
struct ipu_psys_command {
  uint64_t issue_id;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint64_t user_token;
  uint32_t priority;
  void* pg_manifest;
  struct ipu_psys_buffer* buffers;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  int pg;
  uint32_t pg_manifest_size;
  uint32_t bufcount;
  uint32_t min_psys_freq;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint32_t frame_counter;
  uint32_t kernel_enable_bitmap[4];
  uint32_t terminal_enable_bitmap[4];
  uint32_t routing_enable_bitmap[4];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint32_t rbm[5];
  uint32_t reserved[2];
} __attribute__((packed));
struct ipu_psys_manifest {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  uint32_t index;
  uint32_t size;
  void* manifest;
  uint32_t reserved[5];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} __attribute__((packed));
#define IPU_IOC_QUERYCAP _IOR('A', 1, struct ipu_psys_capability)
#define IPU_IOC_MAPBUF _IOWR('A', 2, int)
#define IPU_IOC_UNMAPBUF _IOWR('A', 3, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPU_IOC_GETBUF _IOWR('A', 4, struct ipu_psys_buffer)
#define IPU_IOC_PUTBUF _IOWR('A', 5, struct ipu_psys_buffer)
#define IPU_IOC_QCMD _IOWR('A', 6, struct ipu_psys_command)
#define IPU_IOC_DQEVENT _IOWR('A', 7, struct ipu_psys_event)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPU_IOC_CMD_CANCEL _IOWR('A', 8, struct ipu_psys_command)
#define IPU_IOC_GET_MANIFEST _IOWR('A', 9, struct ipu_psys_manifest)
#endif
