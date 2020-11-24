/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __NCI_PIFCONV_CTRL_REG_PACK_UNPACK_H
#define __NCI_PIFCONV_CTRL_REG_PACK_UNPACK_H

/*
 * This file is copied from SDK nci/include.
 * The copy was needed as there are issues with
 * assert support if we try to use the one in SDK.
 */

#ifndef NCI_SIGNED
#define NCI_SIGNED(param) 1,param
#endif

#ifndef NCI_UNSIGNED
#define NCI_UNSIGNED(param) 0,param
#endif

#define PIFCONVERTER_PIF_CONVERTER_CTRL_PIF_HDR_MASK_SIZE		4
#define PIFCONVERTER_PIF_CONVERTER_CTRL_CROP_EN_SIZE			1
#define PIFCONVERTER_PIF_CONVERTER_INPUT_DIM_LINE_WIDTH_SIZE		16
#define PIFCONVERTER_PIF_CONVERTER_INPUT_DIM_NOF_LINES_SIZE		16
#define PIFCONVERTER_PIF_CONVERTER_CROP_COL_CROP_COL_START_SIZE		16
#define PIFCONVERTER_PIF_CONVERTER_CROP_COL_CROP_COL_END_SIZE		16
#define PIFCONVERTER_PIF_CONVERTER_CROP_ROW_CROP_ROW_START_SIZE		16
#define PIFCONVERTER_PIF_CONVERTER_CROP_ROW_CROP_ROW_END_SIZE		16

STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_ctrl (uint8_t is_signed_pif_hdr_mask, uint8_t pif_hdr_mask, uint8_t is_signed_crop_en, uint8_t crop_en);
STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_ctrl (uint8_t * pif_hdr_mask, uint8_t * crop_en, uint32_t in_data);
STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_input_dim (uint8_t is_signed_line_width, uint16_t line_width, uint8_t is_signed_nof_lines, uint16_t nof_lines);
STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_input_dim (uint16_t * line_width, uint16_t * nof_lines, uint32_t in_data);
STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_crop_col (uint8_t is_signed_crop_col_start, uint16_t crop_col_start, uint8_t is_signed_crop_col_end, uint16_t crop_col_end);
STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_crop_col (uint16_t * crop_col_start, uint16_t * crop_col_end, uint32_t in_data);
STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_crop_row (uint8_t is_signed_crop_row_start, uint16_t crop_row_start, uint8_t is_signed_crop_row_end, uint16_t crop_row_end);
STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_crop_row (uint16_t * crop_row_start, uint16_t * crop_row_end, uint32_t in_data);

STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_ctrl (uint8_t is_signed_pif_hdr_mask, uint8_t pif_hdr_mask, uint8_t is_signed_crop_en, uint8_t crop_en)
{
#ifdef NCI_CHECK_FIELD_WIDTHS
    assert ((!(pif_hdr_mask & ~0xF) || is_signed_pif_hdr_mask) && "pif_hdr_mask value exceeds 4 bits");
    assert ((!(crop_en & ~0x1) || is_signed_crop_en) && "crop_en value exceeds 1 bits");
#endif
    ((void) (is_signed_pif_hdr_mask));
    ((void) (is_signed_crop_en));
    return ((pif_hdr_mask & 0xF) | ((crop_en & 0x1) << 4));
}

STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_ctrl (uint8_t * pif_hdr_mask, uint8_t * crop_en, uint32_t in_data)
{
    *pif_hdr_mask = in_data & 0xF;
    *crop_en = (in_data >> 4) & 0x1;
}

STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_input_dim (uint8_t is_signed_line_width, uint16_t line_width, uint8_t is_signed_nof_lines, uint16_t nof_lines)
{
#ifdef NCI_CHECK_FIELD_WIDTHS
    assert ((!(line_width & ~0xFFFF) || is_signed_line_width) && "line_width value exceeds 16 bits");
    assert ((!(nof_lines & ~0xFFFF) || is_signed_nof_lines) && "nof_lines value exceeds 16 bits");
#endif
    ((void) (is_signed_line_width));
    ((void) (is_signed_nof_lines));
    return ((line_width & 0xFFFF) | ((nof_lines & 0xFFFF) << 16));
}

STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_input_dim (uint16_t * line_width, uint16_t * nof_lines, uint32_t in_data)
{
    *line_width = in_data & 0xFFFF;
    *nof_lines = (in_data >> 16) & 0xFFFF;
}

STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_crop_col (uint8_t is_signed_crop_col_start, uint16_t crop_col_start, uint8_t is_signed_crop_col_end, uint16_t crop_col_end)
{
#ifdef NCI_CHECK_FIELD_WIDTHS
    assert ((!(crop_col_start & ~0xFFFF) || is_signed_crop_col_start) && "crop_col_start value exceeds 16 bits");
    assert ((!(crop_col_end & ~0xFFFF) || is_signed_crop_col_end) && "crop_col_end value exceeds 16 bits");
#endif
    ((void) (is_signed_crop_col_start));
    ((void) (is_signed_crop_col_end));
    return ((crop_col_start & 0xFFFF) | ((crop_col_end & 0xFFFF) << 16));
}

STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_crop_col (uint16_t * crop_col_start, uint16_t * crop_col_end, uint32_t in_data)
{
    *crop_col_start = in_data & 0xFFFF;
    *crop_col_end = (in_data >> 16) & 0xFFFF;
}

STORAGE_CLASS_INLINE uint32_t
pack_reg_PifConverter_pif_converter_crop_row (uint8_t is_signed_crop_row_start, uint16_t crop_row_start, uint8_t is_signed_crop_row_end, uint16_t crop_row_end)
{
#ifdef NCI_CHECK_FIELD_WIDTHS
    assert ((!(crop_row_start & ~0xFFFF) || is_signed_crop_row_start) && "crop_row_start value exceeds 16 bits");
    assert ((!(crop_row_end & ~0xFFFF) || is_signed_crop_row_end) && "crop_row_end value exceeds 16 bits");
#endif
    ((void) (is_signed_crop_row_start));
    ((void) (is_signed_crop_row_end));
    return ((crop_row_start & 0xFFFF) | ((crop_row_end & 0xFFFF) << 16));
}

STORAGE_CLASS_INLINE void
unpack_reg_PifConverter_pif_converter_crop_row (uint16_t * crop_row_start, uint16_t * crop_row_end, uint32_t in_data)
{
    *crop_row_start = in_data & 0xFFFF;
    *crop_row_end = (in_data >> 16) & 0xFFFF;
}

#endif /* __NCI_PIFCONV_CTRL_REG_PACK_UNPACK_H */
