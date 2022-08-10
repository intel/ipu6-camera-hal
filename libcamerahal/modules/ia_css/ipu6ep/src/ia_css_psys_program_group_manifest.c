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

#include "ia_css_psys_static_storage_class.h"
#include "ia_css_psys_program_group_manifest.h"
#include "ia_css_rbm_manifest.h"

#ifndef IPU7_ERROR_MISSING_DISABLE_SECTION
#include "vied_nci_acb_route_type.h" /* only used for printing. */
#endif /* IPU7_ERROR_MISSING_DISABLE_SECTION */

/*
 * Functions to possibly inline
 */

#ifndef __IA_CSS_PSYS_STATIC_INLINE__
#include "ia_css_psys_program_group_manifest_impl.h"
#endif /* __IA_CSS_PSYS_STATIC_INLINE__ */

/*
 * Functions not to inline
 */

/*
 * We need to refactor those files in order to
 * build in the firmware only what is needed,
 * switches are put current to workaround compilation problems
 * in the firmware (for example lack of uint64_t support)
 * supported in the firmware
 */
#if !defined(__HIVECC)
size_t ia_css_sizeof_program_group_manifest(
    const uint8_t program_count,
    const uint8_t terminal_count,
    const uint8_t *program_needs_extension,
    const uint8_t *program_dependency_count,
    const uint8_t *terminal_dependency_count,
    const ia_css_terminal_type_t *terminal_type,
    const uint16_t *cached_in_param_section_count,
    const uint16_t *cached_out_param_section_count,
    const uint16_t *spatial_param_section_count,
    const uint16_t *fragment_param_section_count,
    const uint16_t *sliced_param_section_count,
    const uint16_t *sliced_out_param_section_count,
    const uint16_t *kernel_fragment_seq_count,
    const uint16_t *progctrlinit_load_section_counts,
    const uint16_t *progctrlinit_connect_section_counts)
{
    size_t size = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    int m = 0;
    int n = 0;
    int o = 0;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_sizeof_program_group_manifest(): enter:\n");

    verifexit(program_count != 0);
    verifexit(program_dependency_count != NULL);
    verifexit(terminal_dependency_count != NULL);

    size += sizeof(ia_css_program_group_manifest_t);

    /* Private payload in the program group manifest */
    size += ceil_mul(sizeof(struct ia_css_psys_private_pg_data),
                sizeof(uint64_t));
    /* RBM manifest in the program group manifest */
    size += ceil_mul(sizeof(ia_css_rbm_manifest_t),
                sizeof(uint64_t));

    for (i = 0; i < (int)program_count; i++) {
        size += ia_css_sizeof_program_manifest(
                program_needs_extension[i],
                program_dependency_count[i],
                terminal_dependency_count[i]);
    }

    for (i = 0; i < (int)terminal_count; i++) {
        switch (terminal_type[i]) {
        case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
            size += ia_css_param_terminal_manifest_get_size(
                    cached_in_param_section_count[j]);
            j++;
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
            size += ia_css_param_terminal_manifest_get_size(
                    cached_out_param_section_count[k]);
            k++;
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
        case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT:
            size += ia_css_spatial_param_terminal_manifest_get_size(
                    spatial_param_section_count[l]);
            l++;
            break;
        case IA_CSS_TERMINAL_TYPE_PROGRAM:
            size += ia_css_program_terminal_manifest_get_size(
                    fragment_param_section_count[m],
                    kernel_fragment_seq_count[m]);
            m++;
            break;
        case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
            size += ia_css_program_control_init_terminal_manifest_get_size(
                program_count,
                progctrlinit_load_section_counts,
                progctrlinit_connect_section_counts);
            break;
        case IA_CSS_TERMINAL_TYPE_DATA_IN:
        case IA_CSS_TERMINAL_TYPE_DATA_OUT:
            size += sizeof(ia_css_data_terminal_manifest_t);
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN:
            size += ia_css_sliced_param_terminal_manifest_get_size(
                    sliced_param_section_count[n]);
            n++;
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT:
            size += ia_css_sliced_param_terminal_manifest_get_size(
                sliced_out_param_section_count[o]);
            o++;
            break;
        default:
            IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                "ia_css_sizeof_program_group_manifest invalid argument\n");
        }
    }

EXIT:
    if (0 == program_count || 0 == terminal_count ||
        NULL == program_dependency_count ||
        NULL == terminal_dependency_count) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_sizeof_program_group_manifest invalid argument\n");
    }
    return size;
}

/*
 * Currently, the design of XNR kernel inside the *_pregdc program group,
 * does not fit the exact model as is being asserted on in
 * ia_css_is_program_group_manifest_valid. We therefore disable some checks.
 * Further investigation is needed to determine whether *_pregdc program group
 * can be canged or that the model must be changed.
 * #define USE_SIMPLIFIED_GRAPH_MODEL 1 allows multiple programs to be
 * connected to the same terminal, and it allows a kernel be mapped over
 * multiple programs.
 */
#define USE_SIMPLIFIED_GRAPH_MODEL 1

/*
 * Model and/or check refinements
 * - Parallel programs do not yet have mutual exclusive alternatives
 * - The pgram dependencies do not need to be acyclic
 * - Parallel programs need to have an equal kernel requirement
 */
bool ia_css_is_program_group_manifest_valid(
    const ia_css_program_group_manifest_t *manifest)
{
    int i;
    bool is_valid = false;
    uint8_t terminal_count;
    uint8_t program_count;
    ia_css_kernel_bitmap_t total_bitmap;
    ia_css_kernel_bitmap_t check_bitmap;
    ia_css_kernel_bitmap_t terminal_bitmap;
    /*
     * Use a standard bitmap type for the minimum logic to check the DAG,
     * generic functions can be used for the kernel enable bitmaps; Later
     */
    vied_nci_resource_bitmap_t resource_bitmap;
    int terminal_bitmap_weight;
    int num_parameter_terminal_in = 0;
    int num_parameter_terminal_out = 0;
    int num_program_terminal = 0;
    int num_program_terminal_sequencer_info = 0;
    bool has_program_control_init_terminal = false;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_program_group_manifest_valid(): enter:\n");

    verifexit(manifest != NULL);
    verifexit(ia_css_program_group_manifest_get_size(manifest) != 0);
    verifexit(ia_css_program_group_manifest_get_alignment(manifest) != 0);
    verifexit(ia_css_program_group_manifest_get_program_group_ID(manifest) != 0);

    terminal_count =
        ia_css_program_group_manifest_get_terminal_count(manifest);
    program_count =
        ia_css_program_group_manifest_get_program_count(manifest);
    total_bitmap =
        ia_css_program_group_manifest_get_kernel_bitmap(manifest);
    check_bitmap = ia_css_kernel_bitmap_clear();
    resource_bitmap = vied_nci_bit_mask(VIED_NCI_RESOURCE_BITMAP_BITS);
    terminal_bitmap = ia_css_kernel_bitmap_clear();

    verifexit(program_count != 0);
    verifexit(terminal_count != 0);
    verifexit(!ia_css_is_kernel_bitmap_empty(total_bitmap));
    verifexit(vied_nci_is_bitmap_empty(resource_bitmap));

    /* Check the kernel bitmaps for terminals */
    for (i = 0; i < (int)terminal_count; i++) {
        ia_css_terminal_manifest_t *terminal_manifest_i =
            ia_css_program_group_manifest_get_term_mnfst(
                manifest, i);
        bool is_parameter_in =
            (IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN ==
            ia_css_terminal_manifest_get_type(
                terminal_manifest_i));
        bool is_parameter_out =
            (IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT ==
            ia_css_terminal_manifest_get_type(
                terminal_manifest_i));
        bool is_data =
            ia_css_is_terminal_manifest_data_terminal(
                terminal_manifest_i);
        bool is_program =
            ia_css_is_terminal_manifest_program_terminal(
                terminal_manifest_i);
        bool is_spatial_param =
            ia_css_is_terminal_manifest_spatial_parameter_terminal(
                terminal_manifest_i);
        bool is_program_control_init =
            ia_css_is_terminal_manifest_program_control_init_terminal(
                terminal_manifest_i);

        if (is_parameter_in) {
            num_parameter_terminal_in = is_parameter_in;
        } else if (is_parameter_out) {
            num_parameter_terminal_out = is_parameter_out;
        } else if (is_data) {
            ia_css_data_terminal_manifest_t *dterminal_manifest_i =
                (ia_css_data_terminal_manifest_t *)
                terminal_manifest_i;
            ia_css_kernel_bitmap_t terminal_bitmap_i =
                ia_css_data_terminal_manifest_get_kernel_bitmap(
                    dterminal_manifest_i);
            /*
             * A terminal must depend on kernels that are a subset
             * of the total, correction, it can only depend on one
             * kernel
             */
            verifexit(!ia_css_is_kernel_bitmap_empty(
                    terminal_bitmap_i));
            verifexit(ia_css_is_kernel_bitmap_subset(
                    total_bitmap, terminal_bitmap_i));
            verifexit(ia_css_is_kernel_bitmap_onehot(
                    terminal_bitmap_i));
        } else if (is_program) {
            verifexit(terminal_manifest_i);
            num_program_terminal += is_program;
            num_program_terminal_sequencer_info +=
                ((ia_css_program_terminal_manifest_t *)
                terminal_manifest_i)->
            kernel_fragment_sequencer_info_manifest_info_count;
        } else if (is_program_control_init) {
            has_program_control_init_terminal = is_program_control_init;
        } else {
            const ia_css_spatial_param_terminal_manifest_t
                *spatial_param_man =
            (const ia_css_spatial_param_terminal_manifest_t *)
                terminal_manifest_i;
            verifexit(spatial_param_man);
            verifexit(is_spatial_param);

            terminal_bitmap =
                ia_css_kernel_bitmap_set(terminal_bitmap,
                spatial_param_man->kernel_id);
            verifexit(!ia_css_is_kernel_bitmap_empty(terminal_bitmap));
            verifexit(ia_css_is_kernel_bitmap_subset(
                    total_bitmap, terminal_bitmap));
        }
    }

    /* Check the kernel bitmaps for programs */
    for (i = 0; i < (int)program_count; i++) {
        int j;
        ia_css_program_manifest_t *program_manifest_i =
            ia_css_program_group_manifest_get_prgrm_mnfst(
                manifest, i);
        ia_css_program_type_t program_type_i =
            ia_css_program_manifest_get_type(program_manifest_i);
        ia_css_kernel_bitmap_t program_bitmap_i =
            ia_css_program_manifest_get_kernel_bitmap(
                program_manifest_i);
        uint8_t program_dependency_count_i =
            ia_css_program_manifest_get_program_dependency_count(
                program_manifest_i);
        uint8_t terminal_dependency_count_i =
            ia_css_program_manifest_get_terminal_dependency_count(
                program_manifest_i);
        uint8_t program_dependency_i0 =
            ia_css_program_manifest_get_program_dependency(
                program_manifest_i, 0);
        bool is_sub_i =
            ia_css_is_program_manifest_subnode_program_type(
                program_manifest_i);
        bool is_exclusive_sub_i =
            (program_type_i == IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB);
        bool is_virtual_sub_i =
            (program_type_i == IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB);
        bool is_super_i =
            ia_css_is_program_manifest_supernode_program_type(
                program_manifest_i);

        /*
         * A program must have kernels that
         * are a subset of the total
         */
        verifexit(!ia_css_is_kernel_bitmap_empty(
                program_bitmap_i));
        verifexit(ia_css_is_kernel_bitmap_subset(
                total_bitmap, program_bitmap_i));
        verifexit((program_type_i != IA_CSS_N_PROGRAM_TYPES));
        verifexit((program_dependency_count_i + terminal_dependency_count_i) != 0);
        /*
         * Checks for subnodes
         * - Parallel subnodes cannot depend on terminals
         * - Exclusive subnodes must depend on
         *   fewer terminals than the supernode
         * - Subnodes only depend on a supernode of the same type
         * - Must have a subset of the supernode's kernels
         *   (but not equal)
         * - This tests only positive cases
         * Checks for singular or supernodes
         * - Cannot depend on exclusive subnodes
         * - No intersection between kernels
         *   (too strict for multiple instances ?)
         */
        if (is_sub_i) {
            /* Subnode */
            ia_css_program_manifest_t *program_manifest_k =
            ia_css_program_group_manifest_get_prgrm_mnfst(
                manifest, program_dependency_i0);
            ia_css_program_type_t program_type_k =
                ia_css_program_manifest_get_type(
                    program_manifest_k);
            ia_css_kernel_bitmap_t program_bitmap_k =
                ia_css_program_manifest_get_kernel_bitmap(
                    program_manifest_k);

            verifexit(program_dependency_count_i == 1);
            if (is_exclusive_sub_i || is_virtual_sub_i) {
                verifexit(terminal_dependency_count_i <=
                    ia_css_program_manifest_get_terminal_dependency_count(
                        program_manifest_k));
            } else{
                verifexit(terminal_dependency_count_i == 0);
            }
            verifexit(program_type_k ==
                (is_exclusive_sub_i ?
                    IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUPER :
                is_virtual_sub_i ?
                    IA_CSS_PROGRAM_TYPE_VIRTUAL_SUPER :
                    IA_CSS_PROGRAM_TYPE_PARALLEL_SUPER));
            verifexit(!ia_css_is_kernel_bitmap_equal(
                    program_bitmap_k, program_bitmap_i));
            verifexit(ia_css_is_kernel_bitmap_subset(
                    program_bitmap_k, program_bitmap_i));
        } else {
            /* Singular or Supernode */
            int k;
            ia_css_kernel_bitmap_t program_bitmap_k;

            for (k = 0; k < program_dependency_count_i; k++) {
                uint8_t program_dependency_k =
                ia_css_program_manifest_get_program_dependency(
                    program_manifest_i, k);
                ia_css_program_manifest_t *program_manifest_k =
                ia_css_program_group_manifest_get_prgrm_mnfst(
                manifest, (int)program_dependency_k);
                ia_css_program_type_t program_type_k =
                ia_css_program_manifest_get_type(
                    program_manifest_k);
                program_bitmap_k =
                ia_css_program_manifest_get_kernel_bitmap(
                    program_manifest_k);

                verifexit(program_dependency_k <
                    program_count);
                verifexit((program_type_k !=
                    IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB) &&
                    (program_type_k !=
                    IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB));
                verifexit(USE_SIMPLIFIED_GRAPH_MODEL ||
                ia_css_is_kernel_bitmap_intersection_empty(
                    program_bitmap_i, program_bitmap_k));
            }
        }

        /* Check for relations */
        for (j = 0; j < (int)program_count; j++) {
            int k;
            ia_css_program_manifest_t *program_manifest_j =
            ia_css_program_group_manifest_get_prgrm_mnfst(
                manifest, j);
            ia_css_program_type_t program_type_j =
            ia_css_program_manifest_get_type(program_manifest_j);
            ia_css_kernel_bitmap_t program_bitmap_j =
            ia_css_program_manifest_get_kernel_bitmap(
                program_manifest_j);
            uint8_t program_dependency_count_j =
            ia_css_program_manifest_get_program_dependency_count(
                program_manifest_j);
            uint8_t program_dependency_j0 =
            ia_css_program_manifest_get_program_dependency(
                program_manifest_j, 0);
            bool is_sub_j =
            ia_css_is_program_manifest_subnode_program_type(
                program_manifest_j);
            bool is_super_j =
            ia_css_is_program_manifest_supernode_program_type(
                program_manifest_j);
            bool is_virtual_sub_j =
            (program_type_j == IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB);
            bool is_j_subset_i =
            ia_css_is_kernel_bitmap_subset(
                program_bitmap_i, program_bitmap_j);
            bool is_i_subset_j =
            ia_css_is_kernel_bitmap_subset(
                program_bitmap_j, program_bitmap_i);

            /* Test below would fail for i==j */
            if (i == j)
                continue;

            /* Empty sets are always subsets, but meaningless */
            verifexit(!ia_css_is_kernel_bitmap_empty(
                    program_bitmap_j));

            /*
             * Checks for mutual subnodes
             * - Parallel subnodes must have an equal
             *   set of kernels
             * - Exclusive and virtual subnodes must
             *   have an unequal set of kernels
             * Checks for subnodes
             * - Subnodes must have a subset of kernels
             */
            if (((program_type_i ==
                IA_CSS_PROGRAM_TYPE_PARALLEL_SUB) &&
                (program_type_j ==
                IA_CSS_PROGRAM_TYPE_PARALLEL_SUB)) ||
                ((program_type_i ==
                IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB) &&
                (program_type_j ==
                IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB)) ||
                ((program_type_i ==
                IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB) &&
                (program_type_j ==
                IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB))) {

                verifexit(program_dependency_count_j == 1);
                verifexit(program_dependency_i0 != i);
                verifexit(program_dependency_j0 != i);

                if (program_dependency_i0 ==
                    program_dependency_j0) {
                    verifexit(is_sub_i);
                    /*
                     * Subnodes are subsets,
                     * not for virtual nodes
                     */
                    if (!is_virtual_sub_i)
                        verifexit(
                            ((is_j_subset_i ||
                            is_i_subset_j)));
                    /*
                     * That must be equal for
                     * parallel subnodes,
                     * must be unequal for
                     * exlusive and virtual subnodes
                     */
                    verifexit(
                    ((is_j_subset_i && is_i_subset_j) ^
                    (is_exclusive_sub_i |
                    is_virtual_sub_i)));

                }
                if (is_j_subset_i || is_i_subset_j) {
                    /* One being subset of the other does not necessarily mean that they are part
                     * of the same "cluster" (i.e. having same super dependency).
                     */
                    /* verifexit(program_dependency_i0 ==
                     *    program_dependency_j0);
                     */
                }
                /* If subnodes are the same, they need different program dependency. */
                if (ia_css_is_kernel_bitmap_equal(program_bitmap_i, program_bitmap_j)) {
                    verifexit(program_dependency_i0 != program_dependency_j0);
                }
            }

            if (((program_type_i ==
                IA_CSS_PROGRAM_TYPE_PARALLEL_SUPER) &&
                (program_type_j ==
                IA_CSS_PROGRAM_TYPE_PARALLEL_SUB)) ||
                ((program_type_i ==
                IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUPER) &&
                (program_type_j ==
                IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB)) ||
                ((program_type_i ==
                IA_CSS_PROGRAM_TYPE_VIRTUAL_SUPER) &&
                (program_type_j ==
                IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB))) {

                verifexit(program_dependency_count_j == 1);
                verifexit(!is_i_subset_j);

                if (program_dependency_j0 == i) {
                    verifexit(program_dependency_i0 !=
                        program_dependency_j0);
                    verifexit(is_super_i);
                    verifexit(is_j_subset_i);

                }
                if (is_j_subset_i) {
                    /* verifexit(program_dependency_j0 == i); */
                    /* A sub that is subset of a super, is not necessarily dependent to it. */
                }
            }

            /*
             * Checks for dependent nodes
             * - Cannot depend on exclusive subnodes
             * - No intersection between kernels
             *   (too strict for multiple instances ?)
             *   unless a subnode
             */
            for (k = 0; k < (int)program_dependency_count_j; k++) {
                uint8_t program_dependency_k =
                ia_css_program_manifest_get_program_dependency(
                    program_manifest_j, k);

                verifexit((program_dependency_k <
                    program_count));
                if (program_dependency_k == i) {
                    /* program[j] depends on program[i] */
                    verifexit((i != j));
                    verifexit((program_type_i !=
                    IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB) &&
                    (program_type_i !=
                    IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB));
                    verifexit(USE_SIMPLIFIED_GRAPH_MODEL ||
                (ia_css_is_kernel_bitmap_intersection_empty(
                program_bitmap_i, program_bitmap_j) ^ is_sub_j));
                }
            }

            /*
             * Checks for supernodes and subnodes
             * - Detect nodes that kernel-wise are subsets,
             *   but not connected to the correct supernode
             * - We do not (yet) detect if programs properly
             *   depend on all parallel nodes
             */
            if (!ia_css_is_kernel_bitmap_intersection_empty(
                program_bitmap_i, program_bitmap_j)) {
                /*
                 * This test will pass if
                 * the program manifest is NULL,
                 * but that's no concern here
                 */
                verifexit(USE_SIMPLIFIED_GRAPH_MODEL ||
            !ia_css_is_program_manifest_singular_program_type(
                program_manifest_i));
                verifexit(USE_SIMPLIFIED_GRAPH_MODEL ||
            !ia_css_is_program_manifest_singular_program_type(
                program_manifest_j));
                if (!is_virtual_sub_j)
                    verifexit(USE_SIMPLIFIED_GRAPH_MODEL ||
                    (is_j_subset_i || is_i_subset_j));
                if (is_super_i) {
                    /* verifexit(is_sub_j); */
                    /* verifexit(program_dependency_j0 == i); */
                }
                if (is_super_j) {
                    /* verifexit(is_sub_i); */
                    /* verifexit(program_dependency_i0 == j); */
                }
                if (is_super_i && is_super_j) {
                    /* Allow two supernodes to intersect as long as they are different */
                    verifexit(!ia_css_is_kernel_bitmap_equal(program_bitmap_i, program_bitmap_j));
                }
                /* A bitmap intersect does not necessarily need to mean super and sub. */
                /* A sub that intersects with a super, is not necessarily dependent to it. */
            }
        }
        check_bitmap = ia_css_kernel_bitmap_union(
                    check_bitmap, program_bitmap_i);
        /*
         * A terminal can be bound to only a single
         * (of multiple concurrent) program(s),
         * i.e. the one that holds the iterator to control it
         * Only singular and super nodes can depend on a terminal.
         * This loop accumulates all terminal
         * dependencies over all programs
         */
        for (j = 0; j < (int)terminal_dependency_count_i; j++) {
            uint8_t terminal_dependency =
            ia_css_program_manifest_get_terminal_dependency(
                    program_manifest_i, j);

            verifexit(terminal_dependency < terminal_count);
            if ((program_type_i !=
                IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB) &&
                (program_type_i !=
                IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB)) {
                /* If the subnode always came after the */
                /* supernode we could check for presence */
                resource_bitmap =
                    vied_nci_bit_mask_set_unique(
                        resource_bitmap,
                        terminal_dependency);
                verifexit(USE_SIMPLIFIED_GRAPH_MODEL ||
                    !vied_nci_is_bitmap_empty(
                        resource_bitmap));
            }
        }
    }
    verifexit(ia_css_is_kernel_bitmap_equal(
            total_bitmap, check_bitmap));

    terminal_bitmap_weight =
        vied_nci_bitmap_compute_weight(resource_bitmap);
    verifexit(terminal_bitmap_weight >= 0);
    if (num_parameter_terminal_in ||
        num_parameter_terminal_out ||
        num_program_terminal ||
        has_program_control_init_terminal) {
        int skip_terminal_count = 0;

        skip_terminal_count += num_parameter_terminal_in;
        skip_terminal_count += num_parameter_terminal_out;
        skip_terminal_count += num_program_terminal;
        skip_terminal_count -= num_program_terminal_sequencer_info;
        if (has_program_control_init_terminal) {
            skip_terminal_count++;
        }
        verifexit(USE_SIMPLIFIED_GRAPH_MODEL ||
            (terminal_bitmap_weight ==
            (terminal_count - skip_terminal_count)));
    } else
        verifexit((terminal_bitmap_weight == terminal_count));

    is_valid = true;
EXIT:
    if (is_valid == false) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_is_program_group_manifest_valid: failed\n");
    }
    return is_valid;
}

#if !defined(__HIVECC)
int ia_css_program_group_manifest_set_kernel_bitmap(
    ia_css_program_group_manifest_t *manifest,
    const ia_css_kernel_bitmap_t bitmap)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_group_manifest_set_kernel_bitmap(): enter:\n");

    if (manifest != NULL) {
        manifest->kernel_bitmap = bitmap;
        retval = 0;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_program_group_manifest_set_kernel_bitmap invalid argument\n");
    }
    return retval;
}
#endif

ia_css_kernel_bitmap_t ia_css_program_group_manifest_get_kernel_bitmap(
    const ia_css_program_group_manifest_t *manifest)
{
    ia_css_kernel_bitmap_t bitmap = ia_css_kernel_bitmap_clear();

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_group_manifest_get_kernel_bitmap(): enter:\n");

    if (manifest != NULL) {
        bitmap = manifest->kernel_bitmap;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_program_group_manifest_get_kernel_bitmap invalid argument\n");
    }
    return bitmap;
}

#if !defined(__HIVECC)
void ia_css_program_group_manifest_init(
    ia_css_program_group_manifest_t *blob,
    const uint8_t program_count,
    const uint8_t terminal_count,
    const uint8_t *program_needs_extension,
    const uint8_t *program_dependencies,
    const uint8_t *terminal_dependencies,
    const ia_css_terminal_type_t *terminal_type,
    const uint16_t *cached_in_param_section_count,
    const uint16_t *cached_out_param_section_count,
    const uint16_t *spatial_param_section_count,
    const uint16_t *fragment_param_section_count,
    const uint16_t *sliced_in_param_section_count,
    const uint16_t *sliced_out_param_section_count,
    const uint16_t *kernel_fragment_seq_count,
    const uint16_t *progctrlinit_load_section_counts,
    const uint16_t *progctrlinit_connect_section_counts)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    int m = 0;
    int n = 0;
    int o = 0;
    int result;
    uint32_t offset = 0;
    char *prg_manifest_base, *terminal_manifest_base;
    size_t program_size = 0;

    /*
     * assert(blob != NULL);
     */
    COMPILATION_ERROR_IF(
        SIZE_OF_DATA_TERMINAL_MANIFEST_STRUCT_IN_BITS !=
            (CHAR_BIT * sizeof(ia_css_data_terminal_manifest_t)));
    COMPILATION_ERROR_IF(
        SIZE_OF_PROGRAM_GROUP_MANIFEST_STRUCT_IN_BITS !=
            (CHAR_BIT * sizeof(ia_css_program_group_manifest_t)));
    COMPILATION_ERROR_IF(
        SIZE_OF_PROGRAM_MANIFEST_STRUCT_IN_BITS !=
            (CHAR_BIT * sizeof(ia_css_program_manifest_t)));
    COMPILATION_ERROR_IF(
        SIZE_OF_PROGRAM_MANIFEST_EXT_STRUCT_IN_BYTES !=
            (CHAR_BIT * sizeof(ia_css_program_manifest_ext_t)));

    COMPILATION_ERROR_IF(0 != sizeof(ia_css_program_manifest_t)%sizeof(uint32_t));
    COMPILATION_ERROR_IF(0 != sizeof(ia_css_program_manifest_ext_t)%sizeof(uint32_t));

    IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO,
        "ia_css_program_group_manifest_init(): enter:\n");

    for (i = 0; i < (int)program_count; i++) {
        program_size +=
            ia_css_sizeof_program_manifest(
                program_needs_extension[i],
                program_dependencies[i],
                terminal_dependencies[i]);
    }

    /* A program group ID cannot be zero */
    blob->ID = 1;
    blob->program_count = program_count;
    blob->terminal_count = terminal_count;
    blob->program_manifest_offset = sizeof(ia_css_program_group_manifest_t);
    blob->terminal_manifest_offset =
        (uint32_t)blob->program_manifest_offset + program_size;

    prg_manifest_base = (char *)
        (((char *)blob) + blob->program_manifest_offset);
    offset = blob->program_manifest_offset;
    for (i = 0; i < (int)program_count; i++) {
        ia_css_program_manifest_init(
            (ia_css_program_manifest_t *)prg_manifest_base,
            program_needs_extension[i],
            program_dependencies[i], terminal_dependencies[i]);
        ia_css_program_manifest_set_parent_offset(
            (ia_css_program_manifest_t *)prg_manifest_base, offset);
        program_size =
            ia_css_sizeof_program_manifest(
                program_needs_extension[i],
                program_dependencies[i],
                terminal_dependencies[i]);
        prg_manifest_base += program_size;
        offset += (uint32_t)program_size;
    }

    offset = blob->terminal_manifest_offset;
    terminal_manifest_base = (char *) (((char *)blob) + offset);
    for (i = 0; i < (int)terminal_count; i++) {
        size_t terminal_size = 0;
        ia_css_terminal_manifest_t *term_manifest =
            (ia_css_terminal_manifest_t *)terminal_manifest_base;

        ia_css_terminal_manifest_set_parent_offset(
                (ia_css_terminal_manifest_t *)
                terminal_manifest_base,
                offset);
        switch (terminal_type[i]) {
        case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
            result = ia_css_param_terminal_manifest_init(
                (ia_css_param_terminal_manifest_t *)
                term_manifest,
                cached_in_param_section_count[j]);
            if (0 == result) {
                terminal_size =
                ia_css_param_terminal_manifest_get_size(
                    cached_in_param_section_count[j]);
                j++;
            } else {
                IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
                    "ia_css_param_terminal_manifest_init failed in cached in terminal\n");
            }
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
            result = ia_css_param_terminal_manifest_init(
                (ia_css_param_terminal_manifest_t *)
                term_manifest,
                cached_out_param_section_count[k]);
            if (0 == result) {
                terminal_size =
                ia_css_param_terminal_manifest_get_size(
                    cached_out_param_section_count[k]);
                k++;
            } else {
                IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
                    "ia_css_param_terminal_manifest_init failed\n");
            }
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
        case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT:
            result = ia_css_spatial_param_terminal_manifest_init(
                (ia_css_spatial_param_terminal_manifest_t *)
                term_manifest,
                spatial_param_section_count[l]);
            if (0 == result) {
                terminal_size =
            ia_css_spatial_param_terminal_manifest_get_size(
                spatial_param_section_count[l]);
                l++;
            } else {
                IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
                    "ia_css_spatial_param_terminal_manifest_init failed in spatial terminal\n");
            }
            break;
        case IA_CSS_TERMINAL_TYPE_PROGRAM:
            result = ia_css_program_terminal_manifest_init(
                (ia_css_program_terminal_manifest_t *)
                term_manifest,
                fragment_param_section_count[m],
                kernel_fragment_seq_count[m]);
            if (0 == result) {
                terminal_size =
                ia_css_program_terminal_manifest_get_size(
                    fragment_param_section_count[m],
                    kernel_fragment_seq_count[m]);
                m++;
            } else {
                IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
                    "ia_css_program_terminal_manifest_init failed in program terminal\n");
            }
            break;
        case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
            result = ia_css_program_control_init_terminal_manifest_init(
                (ia_css_program_control_init_terminal_manifest_t *)
                term_manifest,
                program_count,
                progctrlinit_load_section_counts,
                progctrlinit_connect_section_counts);
            if (0 == result) {
                terminal_size =
                ia_css_program_control_init_terminal_manifest_get_size(
                    program_count,
                    NULL,
                    NULL);
            } else {
                IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
                    "ia_css_program_control_init_terminal_manifest_init failed\n");
            }
            break;
        case IA_CSS_TERMINAL_TYPE_DATA_IN:
        case IA_CSS_TERMINAL_TYPE_DATA_OUT:
            terminal_size = sizeof(ia_css_data_terminal_manifest_t);
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN:
            result = ia_css_sliced_param_terminal_manifest_init(
                (ia_css_sliced_param_terminal_manifest_t *)
                term_manifest,
                sliced_in_param_section_count[n]);
            if (0 == result) {
                terminal_size =
            ia_css_sliced_param_terminal_manifest_get_size(
                sliced_in_param_section_count[n]);
                n++;
            } else {
                IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
                    "ia_css_param_terminal_manifest_init in sliced terminal failed\n");
            }
            break;
        case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT:
            result = ia_css_sliced_param_terminal_manifest_init(
                (ia_css_sliced_param_terminal_manifest_t *)
                term_manifest,
                sliced_out_param_section_count[o]);
            if (0 == result) {
                terminal_size =
                ia_css_sliced_param_terminal_manifest_get_size(
                    sliced_out_param_section_count[o]);
                n++;
            } else {
                IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
                    "ia_css_param_terminal_manifest_init in sliced out terminal failed\n");
            }
            break;
        default:
            IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                "ia_css_program_group_manifest_init invalid argument\n");
        }
        term_manifest->size = (uint16_t)terminal_size;
        /* index: i equal to terminal ID due to sorted of the terminals array */
        term_manifest->ID = i;
        term_manifest->terminal_type = terminal_type[i];
        terminal_manifest_base += terminal_size;
        offset += (uint32_t)terminal_size;
    }

    /* Set the private program group manifest blob offset */
    blob->private_data_offset = offset;
    offset += ceil_mul(sizeof(struct ia_css_psys_private_pg_data),
                sizeof(uint64_t));

    /* Set the RBM manifest blob offset */
    blob->rbm_manifest_offset = offset;
    offset += ceil_mul(sizeof(ia_css_rbm_manifest_t),
                sizeof(uint64_t));

    assert(offset <= UINT16_MAX);
    blob->size = (uint16_t)offset;
}
#endif

int ia_css_program_group_manifest_print(
    const ia_css_program_group_manifest_t *manifest,
    void *fid)
{
    int retval = -1;
    int i;
    uint8_t program_count, terminal_count;
    ia_css_kernel_bitmap_t bitmap;
    struct ia_css_psys_private_pg_data *priv_data;
    ia_css_rbm_manifest_t *rbm_manifest;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO,
        "ia_css_program_group_manifest_print(): enter:\n");

    NOT_USED(fid);

    verifexit(manifest != NULL);

    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
        "sizeof(manifest) = %d\n",
        (int)ia_css_program_group_manifest_get_size(manifest));
    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
        "alignment(manifest) = %d\n",
        (int)ia_css_program_group_manifest_get_alignment(manifest));

    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
        "program group ID = %d\n",
        (int)ia_css_program_group_manifest_get_program_group_ID(
            manifest));

    program_count =
        ia_css_program_group_manifest_get_program_count(manifest);
    terminal_count =
        ia_css_program_group_manifest_get_terminal_count(manifest);

    bitmap = ia_css_program_group_manifest_get_kernel_bitmap(manifest);
    verifexit(ia_css_kernel_bitmap_print(bitmap, fid) == 0);

    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
        "%d program manifests\n", (int)program_count);
    for (i = 0; i < (int)program_count; i++) {
        ia_css_program_manifest_t *program_manifest =
            ia_css_program_group_manifest_get_prgrm_mnfst(
                manifest, i);

        retval = ia_css_program_manifest_print(program_manifest, fid);
        verifjmpexit(retval == 0);
    }
    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
        "%d terminal manifests\n", (int)terminal_count);
    for (i = 0; i < (int)terminal_count; i++) {
        ia_css_terminal_manifest_t *terminal_manifest =
            ia_css_program_group_manifest_get_term_mnfst(
                manifest, i);

        retval = ia_css_terminal_manifest_print(
                terminal_manifest, fid);
        verifjmpexit(retval == 0);
    }

    priv_data =
        (struct ia_css_psys_private_pg_data *)
        ia_css_program_group_manifest_get_private_data(manifest);
    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
        "private_data_offset %d\n", manifest->private_data_offset);

    for (i = 0; i < IPU_DEVICE_GP_PSA_MUX_NUM_MUX; i++) {
        IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
            "PSA MUX id %d mux val %d\n", i,
            priv_data->psa_mux_conf[i]);

    }

    for (i = 0; i < IPU_DEVICE_GP_ISA_STATIC_MUX_NUM_MUX; i++) {
        IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
            "ISA MUX id %d mux val %d\n", i,
            priv_data->isa_mux_conf[i]);

    }

#ifndef IPU7_ERROR_MISSING_DISABLE_SECTION
    for (i = 0; i < IPU_DEVICE_ACB_NUM_ACB; i++) {

        if (priv_data->acb_route[i].in_select !=
            NCI_ACB_PORT_INVALID) {

            assert(priv_data->acb_route[i].in_select !=
                NCI_ACB_PORT_INVALID &&
                priv_data->acb_route[i].out_select !=
                NCI_ACB_PORT_INVALID);

            IA_CSS_TRACE_3(PSYSAPI_STATIC, INFO,
                "Route Cell id %d In %d Out %d\n", i,
                priv_data->acb_route[i].in_select,
                priv_data->acb_route[i].out_select);
        }

    }
#endif /* IPU7_ERROR_MISSING_DISABLE_SECTION */

    for (i = 0; i < MAX_INPUT_BUFFER; i++) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "Input Buffer: link_id = %d\n",
                priv_data->input_buffer_info[i].link_id);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "Input Buffer: buffer_base_addr 0x%x\n",
                priv_data->input_buffer_info[i].buffer_base_addr);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "Input Buffer: bpe = %d\n",
                priv_data->input_buffer_info[i].bpe);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "Input Buffer: buffer_width = %d\n",
                priv_data->input_buffer_info[i].buffer_width);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "Input Buffer: buffer_height = %d\n",
                priv_data->input_buffer_info[i].buffer_height);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "Input Buffer: num_of_buffers = %d\n",
                priv_data->input_buffer_info[i].num_of_buffers);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "Input Buffer: dfm_port_addr = 0x%x\n",
                priv_data->input_buffer_info[i].dfm_port_addr);
        IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO, "======== Input Buffer: end ===========\n");
    }

    rbm_manifest = ia_css_program_group_manifest_get_rbm_manifest((ia_css_program_group_manifest_t *)manifest);
    ia_css_rbm_manifest_print(rbm_manifest);

    retval = 0;
EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_program_group_manifest_print failed (%i)\n",
            retval);
    }
    return retval;
}
#endif /* !defined(__HIVECC) */
