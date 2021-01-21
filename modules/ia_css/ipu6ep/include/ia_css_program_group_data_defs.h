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

#ifndef __IA_CSS_PROGRAM_GROUP_DATA_DEFS_H
#define __IA_CSS_PROGRAM_GROUP_DATA_DEFS_H

#include "frame_support.h"

/**
 * Pre-defined frame format
 *
 * Those formats have inbuild support of traffic
 * and access functions
 *
 * Note that the formats are for terminals, so there
 * is no distinction between input and output formats
 *    - Custom formats with ot without descriptor
 *    - 4CC formats such as YUV variants
 *    - MIPI (line) formats as produced by CSI receivers
 *    - MIPI (sensor) formats such as Bayer or RGBC
 *    - CSS internal formats (private types)
 *  - CSS parameters (type 1 - 6)
 */
#define IA_CSS_FRAME_FORMAT_TYPE_BITS 32
typedef enum ia_css_frame_format_type {
    IA_CSS_DATA_CUSTOM_NO_DESCRIPTOR = 0,
    IA_CSS_DATA_CUSTOM,

    /** 12 bit YUV 411, Y, UV 2-plane  (8 bit per element) */
    IA_CSS_DATA_FORMAT_NV11,
    /** bpp bit YUV 420, Y, U, V 3-plane (bpp/1.5 bpe) */
    IA_CSS_DATA_FORMAT_YUV420,
    /** 12 bit YUV 420, Y, V, U 3-plane (8 bit per element) */
    IA_CSS_DATA_FORMAT_YV12,
    /** 12 bit YUV 420, Y, UV 2-plane (8 bit per element) */
    IA_CSS_DATA_FORMAT_NV12,
    /** 16 bit YUV 420, Y, UV 2-plane (8 bit per element) */
    IA_CSS_DATA_FORMAT_NV12_16,
    /** 12 bit YUV 420, Intel proprietary tiled format, TileY */
    IA_CSS_DATA_FORMAT_NV12_TILEY,
    /** 12 bit YUV 420, Y, VU 2-plane  (8 bit per element) */
    IA_CSS_DATA_FORMAT_NV21,
    /** bpp bit YUV 422, Y, U, V 3-plane (bpp/2 bpe) */
    IA_CSS_DATA_FORMAT_YUV422,
    /** 16 bit YUV 422, Y, V, U 3-plane  (8 bit per element) */
    IA_CSS_DATA_FORMAT_YV16,
    /** 16 bit YUV 422, Y, UV 2-plane  (8 bit per element) */
    IA_CSS_DATA_FORMAT_NV16,
    /** 16 bit YUV 422, Y, VU 2-plane  (8 bit per element) */
    IA_CSS_DATA_FORMAT_NV61,
    /** 16 bit YUV 422, UYVY 1-plane interleaved  (8 bit per element) */
    IA_CSS_DATA_FORMAT_UYVY,
    /** 16 bit YUV 422, YUYV 1-plane interleaved  (8 bit per element) */
    IA_CSS_DATA_FORMAT_YUYV,
    /** bpp bit YUV 444, Y, U, V 3-plane (bpp/3 bpe) */
    IA_CSS_DATA_FORMAT_YUV444,
    /** 8 bit monochrome plane */
    IA_CSS_DATA_FORMAT_Y800,

    /** 5-6-5 bit packed (1-plane) RGB (16bpp, ~5 bpe) */
    IA_CSS_DATA_FORMAT_RGB565,
    /** 24 bit RGB, 3 planes  (8 bit per element) */
    IA_CSS_DATA_FORMAT_RGB888,
    /** 32 bit RGB-Alpha, 1 plane  (8 bit per element) */
    IA_CSS_DATA_FORMAT_RGBA888,

    /** bpp bit raw, [[Gr, R];[B, Gb]] 1-plane (bpp == bpe) */
    IA_CSS_DATA_FORMAT_BAYER_GRBG,
    /** bpp bit raw, [[R, Gr];[Gb, B]] 1-plane (bpp == bpe) */
    IA_CSS_DATA_FORMAT_BAYER_RGGB,
    /** bpp bit raw, [[B, Gb];[Gr, R]] 1-plane (bpp == bpe) */
    IA_CSS_DATA_FORMAT_BAYER_BGGR,
    /** bpp bit raw, [[Gb, B];[R, Gr]] 1-plane (bpp == bpe) */
    IA_CSS_DATA_FORMAT_BAYER_GBRG,

    /** bpp bit (NV12) YUV 420, Y, UV 2-plane derived 3-line,
     * 2-Y, 1-UV (bpp/1.5 bpe): M420 format
     */
    IA_CSS_DATA_FORMAT_YUV420_LINE,
    /** Deprecated RAW, 1 plane */
    IA_CSS_DATA_FORMAT_RAW,
    /** Deprecated RAW, 1 plane, packed */
    IA_CSS_DATA_FORMAT_RAW_PACKED,
    /** Internal, for advanced ISP */
    IA_CSS_DATA_FORMAT_QPLANE6,
    /** 1D byte stream, used for jpeg 1-plane */
    IA_CSS_DATA_FORMAT_BINARY_8,
    /** Deprecated MIPI frame, 1D byte stream 1 plane */
    IA_CSS_DATA_FORMAT_MIPI,
    /** 12 bit [[YY];[UYVY]] 1-plane interleaved 2-line
     * (8 bit per element)
     */
    IA_CSS_DATA_FORMAT_MIPI_YUV420_8,
    /** 15 bit [[YY];[UYVY]] 1-plane interleaved 2-line
     * (10 bit per element)
     */
    IA_CSS_DATA_FORMAT_MIPI_YUV420_10,
    /** 12 bit [[UY];[VY]] 1-plane interleaved 2-line (8 bit per element) */
    IA_CSS_DATA_FORMAT_MIPI_LEGACY_YUV420_8,

    /** Type 1-5 parameter, not fragmentable */
    IA_CSS_DATA_GENERIC_PARAMETER,
    /** Video stabilisation Type 6 parameter, fragmentable */
    IA_CSS_DATA_DVS_PARAMETER,
    /** Video stabilisation Type 6 parameter, coordinates */
    IA_CSS_DATA_DVS_COORDINATES,
    /** Dead Pixel correction Type 6 parameter, fragmentable */
    IA_CSS_DATA_DPC_PARAMETER,
    /** Lens Shading Correction Type 6 parameter, fragmentable */
    IA_CSS_DATA_LSC_PARAMETER,
    /** 3A statistics output HI. */
    IA_CSS_DATA_S3A_STATISTICS_HI,
    /** 3A statistics output LO. */
    IA_CSS_DATA_S3A_STATISTICS_LO,
    /** histogram output */
    IA_CSS_DATA_S3A_HISTOGRAM,
    /** GammaStar grid */
    IA_CSS_DATA_GAMMASTAR_GRID,

    /** Gr R B Gb Gr R B Gb  in PIXELS  (also called isys interleaved) */
    IA_CSS_DATA_FORMAT_BAYER_LINE_INTERLEAVED,
    /** Gr R B Gb Gr R B Gb  in VECTORS (VCC IMAGE, ISP NWAY depentdent) */
    IA_CSS_DATA_FORMAT_BAYER_VECTORIZED,
    /** Gr R B Gb 4-planes */
    IA_CSS_DATA_FORMAT_BAYER_PLANAR,
    /** Gr R Gr R ... | B Gb B Gb ..  in VECTORS (ISP NWAY depentdent) */
    IA_CSS_DATA_FORMAT_BAYER_GRBG_VECTORIZED,

    /** 16 bit YUV 420, Y even plane, Y uneven plane,
     * UV plane vector interleaved
     */
    IA_CSS_DATA_FORMAT_YUV420_VECTORIZED,
    /** 16 bit YUV 420, YYUVYY vector interleaved */
    IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED,

    /** 12 bit YUV 420, Intel proprietary tiled format, TileYf */
    IA_CSS_DATA_FORMAT_NV12_TILEYF,

    /** Y samples appear first in the memory. All Y samples are array of WORDs;
    * even number of lines ;
    * Surface stride can be larger than the width of Y plane.
    * This array is followed immediately by chroma array.
    * Chroma array is an array of WORDs, with interleaved U/V samples.
    * If the interleaved U/V plane is addresses as an * array of DWORDs,
    * the least significant word contains U sample. The stride of the
    * interleaved U/V plane is equal to Y plane. 10 bit data.
    */
    IA_CSS_DATA_FORMAT_P010,

    /** MSB aligned version of P010*/
    IA_CSS_DATA_FORMAT_P010_MSB,

    /** P016/P012 Y samples appear first in the memory.
    * All Y samples are array of WORDs;
    * even number of lines ;
    * Surface stride can be larger than the width of Y plane.
    * This array is followed immediately by chroma array.
    * Chroma array is an array of WORDs, with interleaved U/V samples.
    * If the interleaved U/V plane is addresses as an * array of DWORDs,
    * the least significant word contains U sample. The stride of the
    * interleaved U/V plane is equal to Y plane. 12 bit data.
    */
    IA_CSS_DATA_FORMAT_P016,

    /** MSB aligned version of P016*/
    IA_CSS_DATA_FORMAT_P016_MSB,

    /** TILEY representation of P010 LSB-aligned format */
    IA_CSS_DATA_FORMAT_P010_TILEY,

    /** TILEY representation of P010 MSB-aligned format */
    IA_CSS_DATA_FORMAT_P010_MSB_TILEY,

    /** TILEY representation of P016 LSB-aligned format */
    IA_CSS_DATA_FORMAT_P016_TILEY,

    /** TILEY representation of P016 MSB-aligned format*/
    IA_CSS_DATA_FORMAT_P016_MSB_TILEY,

    /** TILEYf representation of P010*/
    IA_CSS_DATA_FORMAT_P010_TILEYF,

    /** TILEYf representation of P010 MSB aligned*/
    IA_CSS_DATA_FORMAT_P010_MSB_TILEYF,

    /** TILEYf representation of P016*/
    IA_CSS_DATA_FORMAT_P016_TILEYF,

    /** TILEYf representation of P016 MSB aligned*/
    IA_CSS_DATA_FORMAT_P016_MSB_TILEYF,

    /** consists of L and R PDAF pixel line pairs.
     * If line n is L pixel line, n+1 will be R pixel line.
     * Depending on mode - First line can be L or R pixel line and
     * the line pair sequence can also alternate. 1-plane (bpp == bpe)
     */
    IA_CSS_DATA_FORMAT_PAF_NON_INTERLEAVED,

    /** consists of L and R PDAF pixel pairs.
         * Depending on mode - Pixel pairs can be LRLR.., RLRL.. and
         * alternating between the above two as well. 1-plane (bpp == bpe) */
    IA_CSS_DATA_FORMAT_PAF_INTERLEAVED,

    /** bpp bit raw, INFRA-RED (IR) 1-plane (bpp == bpe) */
    IA_CSS_DATA_FORMAT_IR,

    /** MSB aligned version of P012*/
    IA_CSS_DATA_FORMAT_P012_MSB,

    /* Each tile is stored in consecutive addresses in DDR.
     * The scanning walking order within a tile is raster order
     * (line-by-line).
     */
    /* NV12 is a semi-planar (two planes) format: A plane of 8 bit Y samples
     * followed by an interleaved 8 bit U/V
     * samples plane.
     */
    IA_CSS_DATA_FORMAT_NV12_TILE,
    /*P010 is similar to NV12, but with 10 bit values,
     * each stored in a 16b container (msb aligned, 6 lsb are zero).
     */
    IA_CSS_DATA_FORMAT_P010_TILE,
    /* P012 is similar to P010, but with 12 bit values,
     * each stored in a 16b containers (msb aligned, 4 lsb are zero).
     */
    IA_CSS_DATA_FORMAT_P012_TILE,

    IA_CSS_N_FRAME_FORMAT_TYPES
} ia_css_frame_format_type_t;

#endif /* __IA_CSS_PROGRAM_GROUP_DATA_DEFS_H */
