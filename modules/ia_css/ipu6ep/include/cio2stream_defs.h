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
* delivery of the Materials, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights
* must be express and approved by Intel in writing.
*/
#ifndef _cio2stream_defs_h
#define _cio2stream_defs_h

#define _CIO2STR_MAX_SLAVES             2
#define _CIO2STR_MAX_CIO_IN_WIDTH      128
#define _CIO2STR_MIN_CIO_IN_WIDTH      64
#define _CIO2STR_MAX_STR_OUT_WIDTH     64
#define _CIO2STR_MIN_STR_OUT_WIDTH     32

/* Parematers and User_Parameters for HSS */
#define _CIO2STR_PIXEL_BITS             Pixel_bits
#define _CIO2STR_PPC                    Ppc
#define _CIO2STR_USEBAYER               Use_bayer
#define _CIO2STR_USEYUV422              Use_yuv422
#define _CIO2STR_USEYUV420              Use_yuv420
#define _CIO2STR_FIFODEPTH              Fifo_depth
#define _CIO2STR_NOTUSED_SRMD           NotTranslateSRMD
#define _CIO2STR_NOTUSED_RACCEPT        NotTranslateRAccept
#define _CIO2STR_NOTUSED_2DBLOCK        NotTranslate2DBlock

#define _CIO2STR_DBG                    Dbg_device_not_included

/* Constants */
#define _BAYER_COLOR_COMPONENTS    4
#define _YUV_COLOR_COMPONENTS      3

#define _CIO2STR_BAYER_NR_VECTORS      4
#define _CIO2STR_YUV422_NR_VECTORS     8
#define _CIO2STR_YUV420_NR_VECTORS     6

#ifndef VTS_NWAY
#define _CIO2STR_NWAY                 32
#endif

#define _CIO2STR_NR_ROWS               _CIO2STR_YUV422_NR_VECTORS

#define _CIO2STR_SLVEC_PORTREG_IDX     0
/* Config/Status registers and counters */
#define _CIO2STR_ADRRESS_ALIGN_REG     4
#define _CIO2STR_SLVEC_PORTREG_ADDR    _CIO2STR_SLVEC_PORTREG_IDX

/* Common Register IDx */
#define _CIO2STR_SLV_STATREG_IDX       0

/* Slave Register IDx */
#define _CIO2STR_SLV_ADDR_MASKREG_IDX  0
#define _CIO2STR_SLV_ADDR_CMPREG_IDX   1
#define _CIO2STR_SLV_CNTREG_IDX        2
#define _CIO2STR_SLV_VALIDREG_IDX      3
#define _CIO2STR_STR_CNTREG_IDX        4
#define _CIO2STR_STR_STATREG_IDX       5

#define _CIO2STR_VEC_STATUS_REG        _CIO2STR_SLV_STATREG_IDX
#define _CIO2STR_VEC_COUNTER_REG       _CIO2STR_SLV_CNTREG_IDX
#define _CIO2STR_STR_STATUS_REG        _CIO2STR_STR_STATREG_IDX
#define _CIO2STR_STR_COUNTER_REG       _CIO2STR_STR_CNTREG_IDX

#define _CIO2STR_MAX_REGS_PER_SLAVE    (_CIO2STR_STR_STATREG_IDX + 1)
#define _CIO2STR_NR_RW_REGS            0

#define _CIO2STR_INVALID_FLAG          0xDEADBEEF
#define _CIO2STR_CAFE_FLAG             0xCAFEBABE

#define _CIO2STR_YUVSHRINK_EN_REG_WIDTH    1
#define _CIO2STR_VEC_STAT_REG_WIDTH       25
#define _CIO2STR_STR_STAT_REG_WIDTH       32
#define _CIO2STR_VEC_CNT_REG_WIDTH         4
#define _CIO2STR_PXL_REG_CNT_WIDTH         8 // Not used, calculated inside CHDL

#define _CIO2STR_YUV422_VALID            0x0
#define _CIO2STR_YUV420_VALID            0x1

/* Storage */
#ifndef _CIO2STR_FIFO_DEPTH
#define _CIO2STR_FIFO_DEPTH            4
#endif

#ifndef NO_MASK
#define NO_MASK                    0xFFFFFFFF
#endif
/* Resgister settings */
/*
typedef struct RegisterBank{
    char*   name;
    uint    id;
    uint    rst_value;
    int     address;
    uint    value;
    char*   access;
    uint    width;
}RegisterBank_t;
  
extern struct RegisterBank cfg_regs[_CIO2STR_NR_RW_RO_REGS];

  cfg_regs[_CIO2STR_SLVEC_STATREG_IDX].name="sl_vec_port_status";
  cfg_regs[_CIO2STR_SLVEC_STATREG_IDX].id=_CIO2STR_SLVEC_STATREG_IDX;
  cfg_regs[_CIO2STR_SLVEC_STATREG_IDX].rst_value=_CIO2STR_INVALID_FLAG;
  cfg_regs[_CIO2STR_SLVEC_STATREG_IDX].address=_CIO2STR_VEC_STATUS_REG;
  cfg_regs[_CIO2STR_SLVEC_STATREG_IDX].value=_CIO2STR_INVALID_FLAG;
  cfg_regs[_CIO2STR_SLVEC_STATREG_IDX].access="RO";
  cfg_regs[_CIO2STR_SLVEC_STATREG_IDX].width=_CIO2STR_VEC_STAT_REG_WIDTH;
  cfg_regs[_CIO2STR_SLCFG_STATREG_IDX].name="str_out_port_status";
  cfg_regs[_CIO2STR_SLCFG_STATREG_IDX].id=_CIO2STR_SLCFG_STATREG_IDX;
  cfg_regs[_CIO2STR_SLCFG_STATREG_IDX].rst_value=_CIO2STR_INVALID_FLAG;
  cfg_regs[_CIO2STR_SLCFG_STATREG_IDX].address=_CIO2STR_STR_STATUS_REG;
  cfg_regs[_CIO2STR_SLCFG_STATREG_IDX].value=_CIO2STR_INVALID_FLAG;
  cfg_regs[_CIO2STR_SLCFG_STATREG_IDX].access="RO";
  cfg_regs[_CIO2STR_SLCFG_STATREG_IDX].width=_CIO2STR_STR_STAT_REG_WIDTH;
  cfg_regs[_CIO2STR_SLVEC_CNTREG_IDX].name="sl_vec_counter";
  cfg_regs[_CIO2STR_SLVEC_CNTREG_IDX].id=_CIO2STR_SLVEC_CNTREG_IDX;
  cfg_regs[_CIO2STR_SLVEC_CNTREG_IDX].rst_value=0x0;
  cfg_regs[_CIO2STR_SLVEC_CNTREG_IDX].address=_CIO2STR_VEC_COUNTER_REG;
  cfg_regs[_CIO2STR_SLVEC_CNTREG_IDX].value=0x0;
  cfg_regs[_CIO2STR_SLVEC_CNTREG_IDX].access="RO";
  cfg_regs[_CIO2STR_SLVEC_CNTREG_IDX].width=_CIO2STR_VEC_CNT_REG_WIDTH;
  cfg_regs[_CIO2STR_STR_TRA_CNTREG_IDX].name="pxl_out_counter";
  cfg_regs[_CIO2STR_STR_TRA_CNTREG_IDX].id=_CIO2STR_STR_TRA_CNTREG_IDX;
  cfg_regs[_CIO2STR_STR_TRA_CNTREG_IDX].rst_value=0x0;
  cfg_regs[_CIO2STR_STR_TRA_CNTREG_IDX].address=_CIO2STR_STR_COUNTER_REG;
  cfg_regs[_CIO2STR_STR_TRA_CNTREG_IDX].value=0x0;
  cfg_regs[_CIO2STR_STR_TRA_CNTREG_IDX].access="RO";
  cfg_regs[_CIO2STR_STR_TRA_CNTREG_IDX].width=_CIO2STR_PXL_REG_CNT_WIDTH;
*/
#endif /* _cio2stream_defs_h */
