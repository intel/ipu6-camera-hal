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

#include <ia_css_psys_terminal_manifest.h>

/* Data object types on the terminals */
#include <ia_css_program_group_data.h>
/* for ia_css_kernel_bitmap_t, ia_css_kernel_bitmap_clear, ia_css_... */
#include <ia_css_kernel_bitmap.h>

#include "ia_css_psys_program_group_private.h"
#include "ia_css_terminal_manifest.h"
#include "ia_css_terminal_manifest_types.h"

#include <error_support.h>
#include <print_support.h>
#include <misc_support.h>
#include "ia_css_psys_static_trace.h"

#ifndef __IA_CSS_PSYS_STATIC_INLINE__
#include "ia_css_psys_terminal_manifest_impl.h"
#endif
#include "ia_css_terminal_manifest.h"

/* We need to refactor those files in order to build in the firmware only
   what is needed, switches are put current to workaround compilation problems
   in the firmware (for example lack of uint64_t support)
   supported in the firmware
  */
#if !defined(__HIVECC)
static const char *terminal_type_strings[IA_CSS_N_TERMINAL_TYPES + 1] = {
    "IA_CSS_TERMINAL_TYPE_DATA_IN",
    "IA_CSS_TERMINAL_TYPE_DATA_OUT",
    "IA_CSS_TERMINAL_TYPE_PARAM_STREAM",
    /**< Type 1-5 parameter input */
    "IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN",
    /**< Type 1-5 parameter output */
    "IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT",
    /**< Represent the new type of terminal for
     * the "spatial dependent parameters", when params go in
     */
    "IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN",
    /**< Represent the new type of terminal for
     * the "spatial dependent parameters", when params go out
     */
    "IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT",
    /**< Represent the new type of terminal for
     * the explicit slicing, when params go in
     */
    "IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN",
    /**< Represent the new type of terminal for
     * the explicit slicing, when params go out
     */
    "IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT",
    /**< State (private data) input */
    "IA_CSS_TERMINAL_TYPE_STATE_IN",
    /**< State (private data) output */
    "IA_CSS_TERMINAL_TYPE_STATE_OUT",
    "IA_CSS_TERMINAL_TYPE_PROGRAM",
    "IA_CSS_TERMINAL_TYPR_PROGRAM_CONTROL_INIT",
    "UNDEFINED_TERMINAL_TYPE"};

int ia_css_data_terminal_manifest_set_kernel_bitmap_unique(
    ia_css_data_terminal_manifest_t *manifest,
    const unsigned int index)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_kernel_bitmap_unique(): enter:\n");

    if (manifest != NULL) {
        ia_css_kernel_bitmap_t kernel_bitmap =
                    ia_css_kernel_bitmap_clear();

        kernel_bitmap = ia_css_kernel_bitmap_set(kernel_bitmap, index);
        verifexit(!ia_css_is_kernel_bitmap_empty(kernel_bitmap));
        verifexit(ia_css_data_terminal_manifest_set_kernel_bitmap(
                manifest, kernel_bitmap) == 0);
        retval = 0;
    }

EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_kernel_bitmap_unique failed (%i)\n",
            retval);
    }
    return retval;
}

int ia_css_data_terminal_manifest_set_min_size(
    ia_css_data_terminal_manifest_t    *manifest,
    const uint16_t min_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_min_size(): enter:\n");

    verifexit(manifest != NULL);

    manifest->min_size[IA_CSS_COL_DIMENSION] =
        min_size[IA_CSS_COL_DIMENSION];
    manifest->min_size[IA_CSS_ROW_DIMENSION] =
        min_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_min_size: invalid argument\n");
    }
    return retval;
}

int ia_css_data_terminal_manifest_set_max_size(
    ia_css_data_terminal_manifest_t    *manifest,
    const uint16_t max_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_max_size(): enter:\n");

    verifexit(manifest != NULL);

    manifest->max_size[IA_CSS_COL_DIMENSION] =
        max_size[IA_CSS_COL_DIMENSION];
    manifest->max_size[IA_CSS_ROW_DIMENSION] =
        max_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_max_size: invalid argument\n");
    }
    return retval;
}
#endif

int ia_css_data_terminal_manifest_get_min_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t min_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_min_size(): enter:\n");

    verifexit(manifest != NULL);

    min_size[IA_CSS_COL_DIMENSION] =
        manifest->min_size[IA_CSS_COL_DIMENSION];
    min_size[IA_CSS_ROW_DIMENSION] =
        manifest->min_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_get_min_size: invalid argument\n");
    }
    return retval;
}

int ia_css_data_terminal_manifest_get_max_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t max_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_max_size(): enter:\n");

    verifexit(manifest != NULL);

    max_size[IA_CSS_COL_DIMENSION] =
        manifest->max_size[IA_CSS_COL_DIMENSION];
    max_size[IA_CSS_ROW_DIMENSION] =
        manifest->max_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_get_max_size: invalid argument\n");
    }
    return retval;
}

#if !defined(__HIVECC)
int ia_css_data_terminal_manifest_set_min_fragment_size(
    ia_css_data_terminal_manifest_t    *manifest,
    const uint16_t min_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_min_fragment_size(): enter:\n");

    verifexit(manifest != NULL);

    manifest->min_fragment_size[IA_CSS_COL_DIMENSION] =
        min_size[IA_CSS_COL_DIMENSION];
    manifest->min_fragment_size[IA_CSS_ROW_DIMENSION] =
        min_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_min_fragment_size invalid argument\n");
    }
    return retval;
}

int ia_css_data_terminal_manifest_set_max_fragment_size(
    ia_css_data_terminal_manifest_t    *manifest,
    const uint16_t max_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_max_fragment_size(): enter:\n");

    verifexit(manifest != NULL);

    manifest->max_fragment_size[IA_CSS_COL_DIMENSION] =
        max_size[IA_CSS_COL_DIMENSION];
    manifest->max_fragment_size[IA_CSS_ROW_DIMENSION] =
        max_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_max_fragment_size invalid argument\n");
    }
    return retval;
}
#endif

int ia_css_data_terminal_manifest_get_min_fragment_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t min_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_min_fragment_size(): enter:\n");

    verifexit(manifest != NULL);

    min_size[IA_CSS_COL_DIMENSION] =
        manifest->min_fragment_size[IA_CSS_COL_DIMENSION];
    min_size[IA_CSS_ROW_DIMENSION] =
        manifest->min_fragment_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_get_min_fragment_size invalid argument\n");
    }
    return retval;
}

int ia_css_data_terminal_manifest_get_max_fragment_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t max_size[IA_CSS_N_DATA_DIMENSION])
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_max_fragment_size(): enter:\n");

    verifexit(manifest != NULL);

    max_size[IA_CSS_COL_DIMENSION] =
        manifest->max_fragment_size[IA_CSS_COL_DIMENSION];
    max_size[IA_CSS_ROW_DIMENSION] =
        manifest->max_fragment_size[IA_CSS_ROW_DIMENSION];
    retval = 0;

EXIT:
    if (NULL == manifest) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_get_max_fragment_size invalid argument\n");
    }
    return retval;
}

/* We need to refactor those files in order to build in the firmware only
   what is needed, switches are put current to workaround compilation problems
   in the firmware (for example lack of uint64_t support)
   supported in the firmware
  */
#if !defined(__HIVECC)

#define PRINT_DIMENSION(name, var) IA_CSS_TRACE_3(PSYSAPI_STATIC, \
    INFO, "%s:\t%d %d\n", \
    (name), \
    (var)[IA_CSS_COL_DIMENSION], \
    (var)[IA_CSS_ROW_DIMENSION])

int ia_css_terminal_manifest_print(
    const ia_css_terminal_manifest_t *manifest,
    void *fid)
{
    int retval = -1;
    ia_css_terminal_type_t terminal_type =
        ia_css_terminal_manifest_get_type(manifest);

    IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO,
        "ia_css_terminal_manifest_print(): enter:\n");

    verifexit(manifest != NULL);
    NOT_USED(fid);
    NOT_USED(terminal_type_strings);

    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "sizeof(manifest) = %d\n",
        (int)ia_css_terminal_manifest_get_size(manifest));
#ifndef __XTENSA_FW__
    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "typeof(manifest) = %s\n",
            terminal_type_strings[terminal_type]);
#endif
    if (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN ||
        terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT) {
        ia_css_param_terminal_manifest_t *pterminal_manifest =
            (ia_css_param_terminal_manifest_t *)manifest;
        uint16_t section_count =
            pterminal_manifest->param_manifest_section_desc_count;
        int    i;

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "sections(manifest) = %d\n", (int)section_count);
        for (i = 0; i < section_count; i++) {
            const ia_css_param_manifest_section_desc_t *manifest =
        ia_css_param_terminal_manifest_get_prm_sct_desc(
            pterminal_manifest, i);
            verifjmpexit(manifest != NULL);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "kernel_id = %d\n", (int)ia_css_param_manifest_section_desc_get_kernel_id(manifest));
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "mem_type_id = %d\n",
                (int)ia_css_param_manifest_section_desc_get_mem_type_id(manifest));
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "max_mem_size = %d\n",
                (int)manifest->max_mem_size);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "region_id = %d\n",
                (int)ia_css_param_manifest_section_desc_get_region_id(manifest));
        }
    } else if (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN ||
        terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT) {
        ia_css_sliced_param_terminal_manifest_t
        *sliced_terminal_manifest =
            (ia_css_sliced_param_terminal_manifest_t *)manifest;
        uint32_t kernel_id;
        uint16_t section_count;
        uint16_t section_idx;

        kernel_id = sliced_terminal_manifest->kernel_id;
        section_count =
            sliced_terminal_manifest->sliced_param_section_count;

        NOT_USED(kernel_id);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "kernel_id = %d\n", (int)kernel_id);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "section_count = %d\n", (int)section_count);

        for (section_idx = 0; section_idx < section_count;
            section_idx++) {
            ia_css_sliced_param_manifest_section_desc_t
                *sliced_param_manifest_section_desc;

            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "section %d\n", (int)section_idx);
            sliced_param_manifest_section_desc =
        ia_css_sliced_param_terminal_manifest_get_sliced_prm_sct_desc(
                sliced_terminal_manifest, section_idx);
            verifjmpexit(sliced_param_manifest_section_desc !=
                NULL);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "mem_type_id = %d\n",
            (int)sliced_param_manifest_section_desc->mem_type_id);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "region_id = %d\n",
            (int)sliced_param_manifest_section_desc->region_id);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "max_mem_size = %d\n",
            (int)sliced_param_manifest_section_desc->max_mem_size);
        }
    } else if (terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM) {
        ia_css_program_terminal_manifest_t *program_terminal_manifest =
            (ia_css_program_terminal_manifest_t *)manifest;
        uint32_t sequencer_info_kernel_id;
        uint16_t max_kernel_fragment_sequencer_command_desc;
        uint16_t kernel_fragment_sequencer_info_manifest_info_count;
        uint16_t seq_info_idx;
        uint16_t section_count =
            program_terminal_manifest->fragment_param_manifest_section_desc_count;
        uint16_t sec_idx;

        sequencer_info_kernel_id =
            program_terminal_manifest->sequencer_info_kernel_id;
        max_kernel_fragment_sequencer_command_desc =
            program_terminal_manifest->
            max_kernel_fragment_sequencer_command_desc;
        kernel_fragment_sequencer_info_manifest_info_count =
            program_terminal_manifest->
            kernel_fragment_sequencer_info_manifest_info_count;

        NOT_USED(sequencer_info_kernel_id);
        NOT_USED(max_kernel_fragment_sequencer_command_desc);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "sequencer_info_kernel_id = %d\n",
            (int)sequencer_info_kernel_id);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "max_kernel_fragment_sequencer_command_desc = %d\n",
            (int)max_kernel_fragment_sequencer_command_desc);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "kernel_fragment_sequencer_info_manifest_info_count = %d\n",
            (int)
            kernel_fragment_sequencer_info_manifest_info_count);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "fragment_param_manifest_section_desc_count = %d\n", (int)section_count);
        for (sec_idx = 0; sec_idx < section_count; sec_idx++) {
            const ia_css_fragment_param_manifest_section_desc_t *ptmanifest =
        ia_css_program_terminal_manifest_get_frgmnt_prm_sct_desc(
            program_terminal_manifest, sec_idx);
            NOT_USED(ptmanifest);
            verifjmpexit(manifest != NULL);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "kernel_id = %d\n", (int)ia_css_fragment_param_manifest_section_desc_get_kernel_id(ptmanifest));
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "mem_type_id = %d\n",
                (int)ia_css_fragment_param_manifest_section_desc_get_mem_type_id(ptmanifest));
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "max_mem_size = %d\n",
                (int)ptmanifest->max_mem_size);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "region_id = %d\n",
                (int)ia_css_fragment_param_manifest_section_desc_get_region_id(ptmanifest));
        }
        for (seq_info_idx = 0; seq_info_idx <
            kernel_fragment_sequencer_info_manifest_info_count;
                seq_info_idx++) {
            ia_css_kernel_fragment_sequencer_info_manifest_desc_t
                *sequencer_info_manifest_desc;

            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "sequencer info %d\n", (int)seq_info_idx);
            sequencer_info_manifest_desc =
        ia_css_program_terminal_manifest_get_kernel_frgmnt_seq_info_desc
                (program_terminal_manifest, seq_info_idx);
            verifjmpexit(sequencer_info_manifest_desc != NULL);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "min_fragment_grid_slice_dimension[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_slice_dimension[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_slice_dimension[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "max_fragment_grid_slice_dimension[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_slice_dimension[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_slice_dimension[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "min_fragment_grid_slice_count[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_slice_count[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_slice_count[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "max_fragment_grid_slice_count[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_slice_count[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_slice_count[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "min_fragment_grid_point_decimation_factor[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_point_decimation_factor[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_point_decimation_factor[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "max_fragment_grid_point_decimation_factor[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_point_decimation_factor[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_point_decimation_factor[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "min_fragment_grid_overlay_on_pixel_topleft_index[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
            min_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
            min_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "max_fragment_grid_overlay_on_pixel_topleft_index[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
            max_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
            max_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "min_fragment_grid_overlay_on_pixel_dimension[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                min_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION]);
            IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
                "max_fragment_grid_overlay_on_pixel_dimension[] = {%d, %d}\n",
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION],
                (int)sequencer_info_manifest_desc->
                max_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION]);
        }
    } else if (terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT) {
        ia_css_program_control_init_terminal_manifest_t *progctrlinit_man =
            (ia_css_program_control_init_terminal_manifest_t *)manifest;
        ia_css_program_control_init_terminal_manifest_print(progctrlinit_man);
    } else if (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN ||
        terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT) {

        ia_css_data_terminal_manifest_t *dterminal_manifest =
            (ia_css_data_terminal_manifest_t *)manifest;
        int i;

        NOT_USED(dterminal_manifest);

        verifexit(ia_css_kernel_bitmap_print(
            ia_css_data_terminal_manifest_get_kernel_bitmap(
                dterminal_manifest), fid) == 0);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "formats(manifest) = %04x\n",
        (int)ia_css_data_terminal_manifest_get_frame_format_bitmap(
            dterminal_manifest));
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "connection(manifest) = %04x\n",
        (int)ia_css_data_terminal_manifest_get_connection_bitmap(
            dterminal_manifest));
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "dependent(manifest) = %d\n",
            (int)dterminal_manifest->terminal_dependency);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\tmin_size[%d]   = {\n",
            IA_CSS_N_DATA_DIMENSION);
        for (i = 0; i < (int)IA_CSS_N_DATA_DIMENSION - 1; i++) {
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "\t\t%4d,\n", dterminal_manifest->min_size[i]);
        }
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\t\t%4d }\n", dterminal_manifest->min_size[i]);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\tmax_size[%d]   = {\n", IA_CSS_N_DATA_DIMENSION);
        for (i = 0; i < (int)IA_CSS_N_DATA_DIMENSION - 1; i++) {
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\t\t%4d,\n", dterminal_manifest->max_size[i]);
        }
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\t\t%4d }\n", dterminal_manifest->max_size[i]);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\tmin_fragment_size[%d]   = {\n",
            IA_CSS_N_DATA_DIMENSION);
        for (i = 0; i < (int)IA_CSS_N_DATA_DIMENSION - 1; i++) {
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "\t\t%4d,\n",
                dterminal_manifest->min_fragment_size[i]);
        }
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\t\t%4d }\n",
            dterminal_manifest->min_fragment_size[i]);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\tmax_fragment_size[%d]   = {\n",
            IA_CSS_N_DATA_DIMENSION);
        for (i = 0; i < (int)IA_CSS_N_DATA_DIMENSION - 1; i++) {
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "\t\t%4d,\n",
                dterminal_manifest->max_fragment_size[i]);
        }
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\t\t%4d }\n",
            dterminal_manifest->max_fragment_size[i]);

    } else if (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN ||
        terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT) {

        ia_css_spatial_param_terminal_manifest_t *stm =
            (ia_css_spatial_param_terminal_manifest_t *)manifest;
        ia_css_frame_grid_param_manifest_section_desc_t *sec;
        int sec_count =
            stm->frame_grid_param_manifest_section_desc_count;
        int sec_index;

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "kernel_id:\t\t%d\n",
            stm->kernel_id);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "compute_units_p_elem:\t%d\n",
            stm->compute_units_p_elem);
#ifndef __XTENSA_FW__
        PRINT_DIMENSION("min_fragment_grid_dimension",
            stm->common_fragment_grid_desc.min_fragment_grid_dimension);
        PRINT_DIMENSION("max_fragment_grid_dimension",
            stm->common_fragment_grid_desc.max_fragment_grid_dimension);
        PRINT_DIMENSION("min_frame_grid_dimension",
            stm->frame_grid_desc.min_frame_grid_dimension);
        PRINT_DIMENSION("max_frame_grid_dimension",
            stm->frame_grid_desc.max_frame_grid_dimension);
#endif
        for (sec_index = 0; sec_index < sec_count; sec_index++) {
            sec = ia_css_spatial_param_terminal_manifest_get_frm_grid_prm_sct_desc(
                stm, sec_index);
            verifjmpexit(sec != NULL);

            IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO, "--------------------------\n");
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "\tmem_type_id:\t%d\n",
                sec->mem_type_id);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "\tregion_id:\t%d\n",
                sec->region_id);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "\telem_size:\t%d\n",
                sec->elem_size);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "\tmax_mem_size:\t%d\n",
                sec->max_mem_size);
        }
    } else if (terminal_type < IA_CSS_N_TERMINAL_TYPES) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "terminal type can not be pretty printed, not supported\n");
    }

    retval = 0;
EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_print failed (%i)\n",
            retval);
    }
    return retval;
}

/* Program control init Terminal */
unsigned int ia_css_program_control_init_terminal_manifest_get_connect_section_count(
    const ia_css_program_control_init_manifest_program_desc_t *prog)
{
    assert(prog);
    return prog->connect_section_count;
}

unsigned int ia_css_program_control_init_terminal_manifest_get_load_section_count(
    const ia_css_program_control_init_manifest_program_desc_t *prog)
{
    assert(prog);
    return prog->load_section_count;
}

unsigned int ia_css_program_control_init_terminal_manifest_get_size(
    const uint16_t nof_programs,
    const uint16_t *nof_load_sections,
    const uint16_t *nof_connect_sections)
{
    (void)nof_load_sections; /* might be needed in future */
    (void)nof_connect_sections; /* might be needed in future */

    return sizeof(ia_css_program_control_init_terminal_manifest_t) +
        nof_programs *
        sizeof(ia_css_program_control_init_manifest_program_desc_t);
}

ia_css_program_control_init_manifest_program_desc_t *
ia_css_program_control_init_terminal_manifest_get_program_desc(
    const ia_css_program_control_init_terminal_manifest_t *terminal,
    unsigned int program)
{
    ia_css_program_control_init_manifest_program_desc_t *progs;

    assert(terminal != NULL);
    assert(program < terminal->program_count);

    progs = (ia_css_program_control_init_manifest_program_desc_t *)
        ((const char *)terminal + terminal->program_desc_offset);

    return &progs[program];
}

int ia_css_program_control_init_terminal_manifest_init(
    ia_css_program_control_init_terminal_manifest_t *terminal,
    const uint16_t nof_programs,
    const uint16_t *nof_load_sections,
    const uint16_t *nof_connect_sections)
{
    unsigned int i;
    ia_css_program_control_init_manifest_program_desc_t *progs;

    if (terminal == NULL) {
        return -EFAULT;
    }

    terminal->program_count = nof_programs;
    terminal->program_desc_offset =
        sizeof(ia_css_program_control_init_terminal_manifest_t);

    progs = ia_css_program_control_init_terminal_manifest_get_program_desc(
        terminal, 0);

    for (i = 0; i < nof_programs; i++) {
        progs[i].load_section_count = nof_load_sections[i];
        progs[i].connect_section_count = nof_connect_sections[i];
    }
    return 0;
}

void ia_css_program_control_init_terminal_manifest_print(
    ia_css_program_control_init_terminal_manifest_t *terminal)
{
    unsigned int i;

    ia_css_program_control_init_manifest_program_desc_t *progs;

    progs = ia_css_program_control_init_terminal_manifest_get_program_desc(
        terminal, 0);

    assert(progs);
    (void)progs;

    for (i = 0; i < terminal->program_count; i++) {
        IA_CSS_TRACE_3(PSYSAPI_STATIC, INFO,
            "program index: %d, load sec: %d, connect sec: %d\n",
            i,
            progs[i].load_section_count,
            progs[i].connect_section_count);
    }
}

#endif
