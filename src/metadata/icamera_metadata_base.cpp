/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (C) 2015-2018 Intel Corporation
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

#define LOG_TAG "camera_metadata"

#include <inttypes.h>
#include "icamera_metadata_base.h"
#include "iutils/Utils.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

// TODO need to find out the correct print function
#include <stdio.h>
#define ALOGE printf

#define OK         0
#define ERROR      1
#define NOT_FOUND (-ENOENT)

/**
 * A single metadata entry, storing an array of values of a given type. If the
 * array is no larger than 4 bytes in size, it is stored in the data.value[]
 * array; otherwise, it can found in the parent's data array at index
 * data.offset.
 */
#define ENTRY_ALIGNMENT ((size_t) 4)
typedef struct camera_metadata_buffer_entry {
    uint32_t tag;
    uint32_t count;
    union {
        uint32_t offset;
        uint8_t  value[4];
    } data;
    uint8_t  type;
    uint8_t  reserved[3];
} camera_metadata_buffer_entry_t;

typedef uint32_t metadata_uptrdiff_t;
typedef uint32_t metadata_size_t;

/**
 * A packet of metadata. This is a list of entries, each of which may point to
 * its values stored at an offset in data.
 *
 * It is assumed by the utility functions that the memory layout of the packet
 * is as follows:
 *
 *   |-----------------------------------------------|
 *   | icamera_metadata_t                             |
 *   |                                               |
 *   |-----------------------------------------------|
 *   | reserved for future expansion                 |
 *   |-----------------------------------------------|
 *   | camera_metadata_buffer_entry_t #0             |
 *   |-----------------------------------------------|
 *   | ....                                          |
 *   |-----------------------------------------------|
 *   | camera_metadata_buffer_entry_t #entry_count-1 |
 *   |-----------------------------------------------|
 *   | free space for                                |
 *   | (entry_capacity-entry_count) entries          |
 *   |-----------------------------------------------|
 *   | start of camera_metadata.data                 |
 *   |                                               |
 *   |-----------------------------------------------|
 *   | free space for                                |
 *   | (data_capacity-data_count) bytes              |
 *   |-----------------------------------------------|
 *
 * With the total length of the whole packet being camera_metadata.size bytes.
 *
 * In short, the entries and data are contiguous in memory after the metadata
 * header.
 */
#define METADATA_ALIGNMENT ((size_t) 4)
struct icamera_metadata {
    metadata_size_t          size;
    uint32_t                 version;
    uint32_t                 flags;
    metadata_size_t          entry_count;
    metadata_size_t          entry_capacity;
    metadata_uptrdiff_t      entries_start; // Offset from camera_metadata
    metadata_size_t          data_count;
    metadata_size_t          data_capacity;
    metadata_uptrdiff_t      data_start; // Offset from camera_metadata
    uint8_t                  reserved[];
};

/**
 * A datum of metadata. This corresponds to icamera_metadata_entry_t::data
 * with the difference that each element is not a pointer. We need to have a
 * non-pointer type description in order to figure out the largest alignment
 * requirement for data (DATA_ALIGNMENT).
 */
#define DATA_ALIGNMENT ((size_t) 8)
typedef union camera_metadata_data {
    uint8_t u8;
    int32_t i32;
    float   f;
    int64_t i64;
    double  d;
    icamera_metadata_rational_t r;
} camera_metadata_data_t;

/**
 * The preferred alignment of a packet of camera metadata. In general,
 * this is the lowest common multiple of the constituents of a metadata
 * package, i.e, of DATA_ALIGNMENT and ENTRY_ALIGNMENT.
 */
#define MAX_ALIGNMENT(A, B) (((A) > (B)) ? (A) : (B))
#define METADATA_PACKET_ALIGNMENT \
    MAX_ALIGNMENT(MAX_ALIGNMENT(DATA_ALIGNMENT, METADATA_ALIGNMENT), ENTRY_ALIGNMENT);

/** Versioning information */
#define CURRENT_METADATA_VERSION 1

/** Flag definitions */
#define FLAG_SORTED 0x00000001

/** Tag information */

typedef struct tag_info {
    const char *tag_name;
    uint8_t     tag_type;
} tag_info_t;

#include "icamera_metadata_tag_info.c"

const size_t icamera_metadata_type_size[ICAMERA_NUM_TYPES] = {
    sizeof(uint8_t),                    // ICAMERA_TYPE_BYTE
    sizeof(int32_t),                    // ICAMERA_TYPE_INT32
    sizeof(float),                      // ICAMERA_TYPE_FLOAT
    sizeof(int64_t),                    // ICAMERA_TYPE_INT64
    sizeof(double),                     // ICAMERA_TYPE_DOUBLE
    sizeof(icamera_metadata_rational_t) // ICAMERA_TYPE_RATIONAL
};

const char *icamera_metadata_type_names[ICAMERA_NUM_TYPES] = {
    "byte",                             // ICAMERA_TYPE_BYTE
    "int32",                            // ICAMERA_TYPE_INT32
    "float",                            // ICAMERA_TYPE_FLOAT
    "int64",                            // ICAMERA_TYPE_INT64
    "double",                           // ICAMERA_TYPE_DOUBLE
    "rational"                          // ICAMERA_TYPE_RATIONAL
};

static camera_metadata_buffer_entry_t *get_entries(
        const icamera_metadata_t *metadata) {
    return (camera_metadata_buffer_entry_t*)
            ((uint8_t*)metadata + metadata->entries_start);
}

static uint8_t *get_data(const icamera_metadata_t *metadata) {
    return (uint8_t*)metadata + metadata->data_start;
}

size_t get_icamera_metadata_alignment() {
    return METADATA_PACKET_ALIGNMENT;
}

icamera_metadata_t *allocate_copy_icamera_metadata_checked(
        const icamera_metadata_t *src,
        size_t src_size) {

    if (src == NULL) {
        return NULL;
    }

    void *buffer = malloc(src_size);
    MEMCPY_S(buffer, src_size, src, src_size);

    icamera_metadata_t *metadata = (icamera_metadata_t*) buffer;
    if (validate_icamera_metadata_structure(metadata, &src_size) != OK) {
        free(buffer);
        return NULL;
    }

    return metadata;
}

icamera_metadata_t *allocate_icamera_metadata(size_t entry_capacity,
                                              size_t data_capacity) {

    size_t memory_needed = calculate_icamera_metadata_size(entry_capacity,
                                                          data_capacity);
    void *buffer = malloc(memory_needed);
    return place_icamera_metadata(buffer, memory_needed,
                                 entry_capacity,
                                 data_capacity);
}

icamera_metadata_t *place_icamera_metadata(void *dst,
                                           size_t dst_size,
                                           size_t entry_capacity,
                                           size_t data_capacity) {
    if (dst == NULL) return NULL;

    size_t memory_needed = calculate_icamera_metadata_size(entry_capacity,
                                                          data_capacity);
    if (memory_needed > dst_size) return NULL;

    icamera_metadata_t *metadata = (icamera_metadata_t*)dst;
    metadata->version = CURRENT_METADATA_VERSION;
    metadata->flags = 0;
    metadata->entry_count = 0;
    metadata->entry_capacity = entry_capacity;
    metadata->entries_start =
            ALIGN_TO(sizeof(icamera_metadata_t), ENTRY_ALIGNMENT);
    metadata->data_count = 0;
    metadata->data_capacity = data_capacity;
    metadata->size = memory_needed;
    size_t data_unaligned = (uint8_t*)(get_entries(metadata) +
            metadata->entry_capacity) - (uint8_t*)metadata;
    metadata->data_start = ALIGN_TO(data_unaligned, DATA_ALIGNMENT);

    assert(validate_icamera_metadata_structure(metadata, NULL) == OK);
    return metadata;
}
void free_icamera_metadata(icamera_metadata_t *metadata) {
    free(metadata);
}

size_t calculate_icamera_metadata_size(size_t entry_count,
                                       size_t data_count) {
    size_t memory_needed = sizeof(icamera_metadata_t);
    // Start entry list at aligned boundary
    memory_needed = ALIGN_TO(memory_needed, ENTRY_ALIGNMENT);
    memory_needed += sizeof(camera_metadata_buffer_entry_t[entry_count]);
    // Start buffer list at aligned boundary
    memory_needed = ALIGN_TO(memory_needed, DATA_ALIGNMENT);
    memory_needed += sizeof(uint8_t[data_count]);
    return memory_needed;
}

size_t get_icamera_metadata_size(const icamera_metadata_t *metadata) {
    if (metadata == NULL) return ERROR;

    return metadata->size;
}

size_t get_icamera_metadata_compact_size(const icamera_metadata_t *metadata) {
    if (metadata == NULL) return ERROR;

    return calculate_icamera_metadata_size(metadata->entry_count,
                                          metadata->data_count);
}

size_t get_icamera_metadata_entry_count(const icamera_metadata_t *metadata) {
    return metadata->entry_count;
}

size_t get_icamera_metadata_entry_capacity(const icamera_metadata_t *metadata) {
    return metadata->entry_capacity;
}

size_t get_icamera_metadata_data_count(const icamera_metadata_t *metadata) {
    return metadata->data_count;
}

size_t get_icamera_metadata_data_capacity(const icamera_metadata_t *metadata) {
    return metadata->data_capacity;
}

icamera_metadata_t* copy_icamera_metadata(void *dst, size_t dst_size,
        const icamera_metadata_t *src) {
    size_t memory_needed = get_icamera_metadata_compact_size(src);

    if (dst == NULL) return NULL;
    if (dst_size < memory_needed) return NULL;

    icamera_metadata_t *metadata =
        place_icamera_metadata(dst, dst_size, src->entry_count, src->data_count);

    if (metadata == NULL) {
        ALOGE("%s: metadata is null!", __func__);
        return NULL;
    }
    metadata->flags = src->flags;
    metadata->entry_count = src->entry_count;
    metadata->data_count = src->data_count;

    MEMCPY_S(get_entries(metadata),sizeof(camera_metadata_buffer_entry_t[metadata->entry_count]),
             get_entries(src), sizeof(camera_metadata_buffer_entry_t[metadata->entry_count]));
    MEMCPY_S(get_data(metadata), sizeof(uint8_t[metadata->data_count]),
             get_data(src), sizeof(uint8_t[metadata->data_count]));

    assert(validate_icamera_metadata_structure(metadata, NULL) == OK);
    return metadata;
}

int validate_icamera_metadata_structure(const icamera_metadata_t *metadata,
                                        const size_t *expected_size) {

    if (metadata == NULL) {
        ALOGE("%s: metadata is null!", __func__);
        return ERROR;
    }

    // Check that the metadata pointer is well-aligned first.
    {
        static const struct {
            const char *name;
            size_t alignment;
        } alignments[] = {
            {
                .name = "icamera_metadata",
                .alignment = METADATA_ALIGNMENT
            },
            {
                .name = "camera_metadata_buffer_entry",
                .alignment = ENTRY_ALIGNMENT
            },
            {
                .name = "camera_metadata_data",
                .alignment = DATA_ALIGNMENT
            },
        };

        size_t i = 0;
        for (i = 0; i < sizeof(alignments)/sizeof(alignments[0]); ++i) {
            uintptr_t aligned_ptr = ALIGN_TO(metadata, alignments[i].alignment);

            if ((uintptr_t)metadata != aligned_ptr) {
                ALOGE("%s: Metadata pointer is not aligned (actual %p, "
                      "expected %p) to type %s",
                      __func__, metadata,
                      (void*)aligned_ptr, alignments[i].name);
                return ERROR;
            }
        }
    }

    /**
     * Check that the metadata contents are correct
     */

    if (expected_size != NULL && metadata->size > *expected_size) {
        ALOGE("%s: Metadata size (%" PRIu32 ") should be <= expected size (%zu)",
              __func__, metadata->size, *expected_size);
        return ERROR;
    }

    if (metadata->entry_count > metadata->entry_capacity) {
        ALOGE("%s: Entry count (%" PRIu32 ") should be <= entry capacity "
              "(%" PRIu32 ")",
              __func__, metadata->entry_count, metadata->entry_capacity);
        return ERROR;
    }

    const metadata_uptrdiff_t entries_end =
        metadata->entries_start + metadata->entry_capacity;
    if (entries_end < metadata->entries_start || // overflow check
        entries_end > metadata->data_start) {

        ALOGE("%s: Entry start + capacity (%" PRIu32 ") should be <= data start "
              "(%" PRIu32 ")",
               __func__,
              (metadata->entries_start + metadata->entry_capacity),
              metadata->data_start);
        return ERROR;
    }

    const metadata_uptrdiff_t data_end =
        metadata->data_start + metadata->data_capacity;
    if (data_end < metadata->data_start || // overflow check
        data_end > metadata->size) {

        ALOGE("%s: Data start + capacity (%" PRIu32 ") should be <= total size "
              "(%" PRIu32 ")",
               __func__,
              (metadata->data_start + metadata->data_capacity),
              metadata->size);
        return ERROR;
    }

    // Validate each entry
    const metadata_size_t entry_count = metadata->entry_count;
    camera_metadata_buffer_entry_t *entries = get_entries(metadata);

    size_t i = 0;
    for (i = 0; i < entry_count; ++i) {

        if ((uintptr_t)&entries[i] != ALIGN_TO(&entries[i], ENTRY_ALIGNMENT)) {
            ALOGE("%s: Entry index %zu had bad alignment (address %p),"
                  " expected alignment %zu",
                  __func__, i, &entries[i], ENTRY_ALIGNMENT);
            return ERROR;
        }

        camera_metadata_buffer_entry_t entry = entries[i];

        if (entry.type >= ICAMERA_NUM_TYPES) {
            ALOGE("%s: Entry index %zu had a bad type %d",
                  __func__, i, entry.type);
            return ERROR;
        }

        int tag_type = get_icamera_metadata_tag_type(entry.tag);
        if (tag_type != (int)entry.type) {
            ALOGE("%s: Entry index %zu had tag type %d, but the type was %d",
                  __func__, i, tag_type, entry.type);
            return ERROR;
        }

        size_t data_size =
                calculate_icamera_metadata_entry_data_size(entry.type,
                                                          entry.count);

        if (data_size != 0) {
            camera_metadata_data_t *data =
                    (camera_metadata_data_t*) (get_data(metadata) +
                                               entry.data.offset);

            if ((uintptr_t)data != ALIGN_TO(data, DATA_ALIGNMENT)) {
                ALOGE("%s: Entry index %zu had bad data alignment (address %p),"
                      " expected align %zu, (tag name %s, data size %zu)",
                      __func__, i, data, DATA_ALIGNMENT,
                      get_icamera_metadata_tag_name(entry.tag) ?: "unknown",
                      data_size);
                return ERROR;
            }

            size_t data_entry_end = entry.data.offset + data_size;
            if (data_entry_end < entry.data.offset || // overflow check
                data_entry_end > metadata->data_capacity) {

                ALOGE("%s: Entry index %zu data ends (%zu) beyond the capacity "
                      "%" PRIu32, __func__, i, data_entry_end,
                      metadata->data_capacity);
                return ERROR;
            }

        } else if (entry.count == 0) {
            if (entry.data.offset != 0) {
                ALOGE("%s: Entry index %zu had 0 items, but offset was non-0 "
                     "(%" PRIu32 "), tag name: %s", __func__, i, entry.data.offset,
                        get_icamera_metadata_tag_name(entry.tag) ?: "unknown");
                return ERROR;
            }
        } // else data stored inline, so we look at value which can be anything.
    }

    return OK;
}

int append_icamera_metadata(icamera_metadata_t *dst,
        const icamera_metadata_t *src) {
    if (dst == NULL || src == NULL ) return ERROR;

    if (dst->entry_capacity < src->entry_count + dst->entry_count) return ERROR;
    if (dst->data_capacity < src->data_count + dst->data_count) return ERROR;

    if (dst->entry_capacity - dst->entry_count < src->entry_count) {
        ALOGE("%s: Dst available buffer size for entry is smaller than src needed.", __func__);
    }
    MEMCPY_S(get_entries(dst) + dst->entry_count,
             sizeof(camera_metadata_buffer_entry_t) * (dst->entry_capacity - dst->entry_count),
             get_entries(src), sizeof(camera_metadata_buffer_entry_t[src->entry_count]));

    if (dst->data_capacity - dst->data_count < src->data_count) {
        ALOGE("%s: Dst available buffer size for data is smaller than src needed.", __func__);
    }
    MEMCPY_S(get_data(dst) + dst->data_count, sizeof(uint8_t[dst->data_capacity - dst->data_count]),
             get_data(src), sizeof(uint8_t[src->data_count]));
    if (dst->data_count != 0) {
        camera_metadata_buffer_entry_t *entry = get_entries(dst) + dst->entry_count;
        size_t i = 0;
        for (i = 0; i < src->entry_count; i++, entry++) {
            if ( calculate_icamera_metadata_entry_data_size(entry->type,
                            entry->count) > 0 ) {
                entry->data.offset += dst->data_count;
            }
        }
    }
    if (dst->entry_count == 0) {
        // Appending onto empty buffer, keep sorted state
        dst->flags |= src->flags & FLAG_SORTED;
    } else if (src->entry_count != 0) {
        // Both src, dst are nonempty, cannot assume sort remains
        dst->flags &= ~FLAG_SORTED;
    } else {
        // Src is empty, keep dst sorted state
    }
    dst->entry_count += src->entry_count;
    dst->data_count += src->data_count;

    assert(validate_icamera_metadata_structure(dst, NULL) == OK);
    return OK;
}

icamera_metadata_t *clone_icamera_metadata(const icamera_metadata_t *src) {
    if (src == NULL) return NULL;
    icamera_metadata_t *clone = allocate_icamera_metadata(
        get_icamera_metadata_entry_count(src),
        get_icamera_metadata_data_count(src));
    if (clone != NULL) {
        int res = append_icamera_metadata(clone, src);
        if (res != OK) {
            free_icamera_metadata(clone);
            clone = NULL;
        }
    }
    assert(validate_icamera_metadata_structure(clone, NULL) == OK);
    return clone;
}

size_t calculate_icamera_metadata_entry_data_size(uint8_t type,
        size_t data_count) {
    if (type >= ICAMERA_NUM_TYPES) return 0;
    size_t data_bytes = data_count *
            icamera_metadata_type_size[type];
    return data_bytes <= 4 ? 0 : ALIGN_TO(data_bytes, DATA_ALIGNMENT);
}

static int add_camera_metadata_entry_raw(icamera_metadata_t *dst,
        uint32_t tag,
        uint8_t  type,
        const void *data,
        size_t data_count) {

    if (dst == NULL) return ERROR;
    if (dst->entry_count == dst->entry_capacity) return ERROR;
    if (data == NULL) return ERROR;

    size_t data_bytes =
            calculate_icamera_metadata_entry_data_size(type, data_count);
    if (data_bytes + dst->data_count > dst->data_capacity) return ERROR;

    if (type >= ICAMERA_NUM_TYPES) {
        ALOGE("%s: Bad type %d", __func__, type);
        return ERROR;
    }
    size_t data_payload_bytes =
            data_count * icamera_metadata_type_size[type];
    camera_metadata_buffer_entry_t *entry = get_entries(dst) + dst->entry_count;
    memset(entry, 0, sizeof(camera_metadata_buffer_entry_t));
    entry->tag = tag;
    entry->type = type;
    entry->count = data_count;

    if (data_bytes == 0) {
        MEMCPY_S(entry->data.value, data_payload_bytes, data, data_payload_bytes);
    } else {
        entry->data.offset = dst->data_count;
        MEMCPY_S(get_data(dst) + entry->data.offset, data_payload_bytes, data, data_payload_bytes);
        dst->data_count += data_bytes;
    }
    dst->entry_count++;
    dst->flags &= ~FLAG_SORTED;
    assert(validate_icamera_metadata_structure(dst, NULL) == OK);
    return OK;
}

int add_icamera_metadata_entry(icamera_metadata_t *dst,
        uint32_t tag,
        const void *data,
        size_t data_count) {

    int type = get_icamera_metadata_tag_type(tag);
    if (type == -1) {
        ALOGE("%s: Unknown tag %04x.", __func__, tag);
        return ERROR;
    }

    return add_camera_metadata_entry_raw(dst,
            tag,
            type,
            data,
            data_count);
}

static int compare_entry_tags(const void *p1, const void *p2) {
    uint32_t tag1 = ((camera_metadata_buffer_entry_t*)p1)->tag;
    uint32_t tag2 = ((camera_metadata_buffer_entry_t*)p2)->tag;
    return  tag1 < tag2 ? -1 :
            tag1 == tag2 ? 0 :
            1;
}

int sort_icamera_metadata(icamera_metadata_t *dst) {
    if (dst == NULL) return ERROR;
    if (dst->flags & FLAG_SORTED) return OK;

    qsort(get_entries(dst), dst->entry_count,
            sizeof(camera_metadata_buffer_entry_t),
            compare_entry_tags);
    dst->flags |= FLAG_SORTED;

    assert(validate_icamera_metadata_structure(dst, NULL) == OK);
    return OK;
}

int get_icamera_metadata_entry(icamera_metadata_t *src,
        size_t index,
        icamera_metadata_entry_t *entry) {
    if (src == NULL || entry == NULL) return ERROR;
    if (index >= src->entry_count) return ERROR;

    camera_metadata_buffer_entry_t *buffer_entry = get_entries(src) + index;

    entry->index = index;
    entry->tag = buffer_entry->tag;
    entry->type = buffer_entry->type;
    entry->count = buffer_entry->count;
    if (buffer_entry->count *
            icamera_metadata_type_size[buffer_entry->type] > 4) {
        entry->data.u8 = get_data(src) + buffer_entry->data.offset;
    } else {
        entry->data.u8 = buffer_entry->data.value;
    }
    return OK;
}

int get_icamera_metadata_ro_entry(const icamera_metadata_t *src,
        size_t index,
        icamera_metadata_ro_entry_t *entry) {
    return get_icamera_metadata_entry((icamera_metadata_t*)src, index,
            (icamera_metadata_entry_t*)entry);
}

int find_icamera_metadata_entry(icamera_metadata_t *src,
        uint32_t tag,
        icamera_metadata_entry_t *entry) {
    if (src == NULL) return ERROR;

    uint32_t index;
    if (src->flags & FLAG_SORTED) {
        // Sorted entries, do a binary search
        camera_metadata_buffer_entry_t *search_entry = NULL;
        camera_metadata_buffer_entry_t key;
        key.tag = tag;
        search_entry = (camera_metadata_buffer_entry_t *)bsearch(&key,
                get_entries(src),
                src->entry_count,
                sizeof(camera_metadata_buffer_entry_t),
                compare_entry_tags);
        if (search_entry == NULL) return NOT_FOUND;
        index = search_entry - get_entries(src);
    } else {
        // Not sorted, linear search
        camera_metadata_buffer_entry_t *search_entry = get_entries(src);
        for (index = 0; index < src->entry_count; index++, search_entry++) {
            if (search_entry->tag == tag) {
                break;
            }
        }
        if (index == src->entry_count) return NOT_FOUND;
    }

    return get_icamera_metadata_entry(src, index,
            entry);
}

int find_icamera_metadata_ro_entry(const icamera_metadata_t *src,
        uint32_t tag,
        icamera_metadata_ro_entry_t *entry) {
    return find_icamera_metadata_entry((icamera_metadata_t*)src, tag,
            (icamera_metadata_entry_t*)entry);
}

int delete_icamera_metadata_entry(icamera_metadata_t *dst,
        size_t index) {
    if (dst == NULL) return ERROR;
    if (index >= dst->entry_count) return ERROR;

    camera_metadata_buffer_entry_t *entry = get_entries(dst) + index;
    size_t data_bytes = calculate_icamera_metadata_entry_data_size(entry->type,
            entry->count);

    if (data_bytes > 0) {
        // Shift data buffer to overwrite deleted data
        uint8_t *start = get_data(dst) + entry->data.offset;
        uint8_t *end = start + data_bytes;
        size_t length = dst->data_count - entry->data.offset - data_bytes;
        memmove(start, end, length);

        // Update all entry indices to account for shift
        camera_metadata_buffer_entry_t *e = get_entries(dst);
        size_t i;
        for (i = 0; i < dst->entry_count; i++) {
            if (calculate_icamera_metadata_entry_data_size(
                    e->type, e->count) > 0 &&
                    e->data.offset > entry->data.offset) {
                e->data.offset -= data_bytes;
            }
            ++e;
        }
        dst->data_count -= data_bytes;
    }
    // Shift entry array
    memmove(entry, entry + 1,
            sizeof(camera_metadata_buffer_entry_t) *
            (dst->entry_count - index - 1) );
    dst->entry_count -= 1;

    assert(validate_icamera_metadata_structure(dst, NULL) == OK);
    return OK;
}

int update_icamera_metadata_entry(icamera_metadata_t *dst,
        size_t index,
        const void *data,
        size_t data_count,
        icamera_metadata_entry_t *updated_entry) {
    if (dst == NULL) return ERROR;
    if (index >= dst->entry_count) return ERROR;

    camera_metadata_buffer_entry_t *entry = get_entries(dst) + index;
    if (entry->type >= ICAMERA_NUM_TYPES) return ERROR;

    size_t data_bytes =
            calculate_icamera_metadata_entry_data_size(entry->type,
                    data_count);
    size_t data_payload_bytes =
            data_count * icamera_metadata_type_size[entry->type];

    size_t entry_bytes =
            calculate_icamera_metadata_entry_data_size(entry->type,
                    entry->count);
    if (data_bytes != entry_bytes) {
        // May need to shift/add to data array
        if (dst->data_capacity < dst->data_count + data_bytes - entry_bytes) {
            // No room
            return ERROR;
        }
        if (entry_bytes != 0) {
            // Remove old data
            uint8_t *start = get_data(dst) + entry->data.offset;
            uint8_t *end = start + entry_bytes;
            size_t length = dst->data_count - entry->data.offset - entry_bytes;
            memmove(start, end, length);
            dst->data_count -= entry_bytes;

            // Update all entry indices to account for shift
            camera_metadata_buffer_entry_t *e = get_entries(dst);
            size_t i;
            for (i = 0; i < dst->entry_count; i++) {
                if (calculate_icamera_metadata_entry_data_size(
                        e->type, e->count) > 0 &&
                        e->data.offset > entry->data.offset) {
                    e->data.offset -= entry_bytes;
                }
                ++e;
            }
        }

        if (data_bytes != 0) {
            // Append new data
            entry->data.offset = dst->data_count;

            MEMCPY_S(get_data(dst) + entry->data.offset, data_payload_bytes, data, data_payload_bytes);
            dst->data_count += data_bytes;
        }
    } else if (data_bytes != 0) {
        // data size unchanged, reuse same data location
        MEMCPY_S(get_data(dst) + entry->data.offset, data_payload_bytes, data, data_payload_bytes);
    }

    if (data_bytes == 0) {
        // Data fits into entry
        MEMCPY_S(entry->data.value, data_payload_bytes, data, data_payload_bytes);
    }

    entry->count = data_count;

    if (updated_entry != NULL) {
        get_icamera_metadata_entry(dst,
                index,
                updated_entry);
    }

    assert(validate_icamera_metadata_structure(dst, NULL) == OK);
    return OK;
}

const char *get_icamera_metadata_section_name(uint32_t tag) {
    uint32_t tag_section = tag >> 16;
    if (tag_section >= CAMERA_SECTION_COUNT) {
        return NULL;
    }
    return icamera_metadata_section_names[tag_section];
}

const char *get_icamera_metadata_tag_name(uint32_t tag) {
    uint32_t tag_section = tag >> 16;
    if (tag_section >= CAMERA_SECTION_COUNT ||
        tag >= icamera_metadata_section_bounds[tag_section][1] ) {
        return NULL;
    }
    uint32_t tag_index = tag & 0xFFFF;
    return icamera_tag_info[tag_section][tag_index].tag_name;
}

int get_icamera_metadata_tag_type(uint32_t tag) {
    uint32_t tag_section = tag >> 16;
    if (tag_section >= CAMERA_SECTION_COUNT ||
            tag >= icamera_metadata_section_bounds[tag_section][1] ) {
        return -1;
    }
    uint32_t tag_index = tag & 0xFFFF;
    return icamera_tag_info[tag_section][tag_index].tag_type;
}

static void print_data(int fd, const uint8_t *data_ptr, uint32_t tag, int type,
        int count,
        int indentation);

void dump_icamera_metadata(const icamera_metadata_t *metadata,
        int fd,
        int verbosity) {
    dump_indented_icamera_metadata(metadata, fd, verbosity, 0);
}

void dump_indented_icamera_metadata(const icamera_metadata_t *metadata,
        int fd,
        int verbosity,
        int indentation) {
    if (metadata == NULL) {
        dprintf(fd, "%*sDumping camera metadata array: Not allocated\n",
                indentation, "");
        return;
    }
    unsigned int i;
    dprintf(fd,
            "%*sDumping camera metadata array: %" PRIu32 " / %" PRIu32 " entries, "
            "%" PRIu32 " / %" PRIu32 " bytes of extra data.\n", indentation, "",
            metadata->entry_count, metadata->entry_capacity,
            metadata->data_count, metadata->data_capacity);
    dprintf(fd, "%*sVersion: %d, Flags: %08x\n",
            indentation + 2, "",
            metadata->version, metadata->flags);
    camera_metadata_buffer_entry_t *entry = get_entries(metadata);
    for (i=0; i < metadata->entry_count; i++, entry++) {

        const char *tag_name, *tag_section;
        tag_section = get_icamera_metadata_section_name(entry->tag);
        if (tag_section == NULL) {
            tag_section = "unknownSection";
        }
        tag_name = get_icamera_metadata_tag_name(entry->tag);
        if (tag_name == NULL) {
            tag_name = "unknownTag";
        }
        const char *type_name;
        if (entry->type >= ICAMERA_NUM_TYPES) {
            type_name = "unknown";
        } else {
            type_name = icamera_metadata_type_names[entry->type];
        }
        dprintf(fd, "%*s%s.%s (%05x): %s[%" PRIu32 "]\n",
             indentation + 2, "",
             tag_section,
             tag_name,
             entry->tag,
             type_name,
             entry->count);

        if (verbosity < 1) continue;

        if (entry->type >= ICAMERA_NUM_TYPES) continue;

        size_t type_size = icamera_metadata_type_size[entry->type];
        uint8_t *data_ptr;
        if ( type_size * entry->count > 4 ) {
            if (entry->data.offset >= metadata->data_count) {
                ALOGE("%s: Malformed entry data offset: %" PRIu32 " (max %" PRIu32 ")",
                        __func__,
                        entry->data.offset,
                        metadata->data_count);
                continue;
            }
            data_ptr = get_data(metadata) + entry->data.offset;
        } else {
            data_ptr = entry->data.value;
        }
        int count = entry->count;
        if (verbosity < 2 && count > 16) count = 16;

        print_data(fd, data_ptr, entry->tag, entry->type, count, indentation);
    }
}

static void print_data(int fd, const uint8_t *data_ptr, uint32_t tag,
        int type, int count, int indentation) {
    static int values_per_line[ICAMERA_NUM_TYPES] = {
        16,                   // ICAMERA_TYPE_BYTE
        4,                    // ICAMERA_TYPE_INT32
        8,                    // ICAMERA_TYPE_FLOAT
        2,                    // ICAMERA_TYPE_INT64
        4,                    // ICAMERA_TYPE_DOUBLE
        2,                    // ICAMERA_TYPE_RATIONAL
    };
    size_t type_size = icamera_metadata_type_size[type];
    char value_string_tmp[ICAMERA_METADATA_ENUM_STRING_MAX_SIZE];
    int32_t value;

    int lines = count / values_per_line[type];
    if (count % values_per_line[type] != 0) lines++;

    int index = 0;
    int j, k;
    for (j = 0; j < lines; j++) {
        dprintf(fd, "%*s[", indentation + 4, "");
        for (k = 0;
             k < values_per_line[type] && count > 0;
             k++, count--, index += type_size) {

            switch (type) {
                case ICAMERA_TYPE_BYTE:
                    value = *(data_ptr + index);
                    if (icamera_metadata_enum_snprint(tag,
                                                      value,
                                                      value_string_tmp,
                                                      sizeof(value_string_tmp))
                        == OK) {
                        dprintf(fd, "%s ", value_string_tmp);
                    } else {
                        dprintf(fd, "%hhu ",
                                *(data_ptr + index));
                    }
                    break;
                case ICAMERA_TYPE_INT32:
                    value =
                            *(int32_t*)(data_ptr + index);
                    if (icamera_metadata_enum_snprint(tag,
                                                      value,
                                                      value_string_tmp,
                                                      sizeof(value_string_tmp))
                        == OK) {
                        dprintf(fd, "%s ", value_string_tmp);
                    } else {
                        dprintf(fd, "%" PRId32 " ",
                                *(int32_t*)(data_ptr + index));
                    }
                    break;
                case ICAMERA_TYPE_FLOAT:
                    dprintf(fd, "%0.8f ",
                            *(float*)(data_ptr + index));
                    break;
                case ICAMERA_TYPE_INT64:
                    dprintf(fd, "%" PRId64 " ",
                            *(int64_t*)(data_ptr + index));
                    break;
                case ICAMERA_TYPE_DOUBLE:
                    dprintf(fd, "%0.8f ",
                            *(double*)(data_ptr + index));
                    break;
                case ICAMERA_TYPE_RATIONAL: {
                    int32_t numerator = *(int32_t*)(data_ptr + index);
                    int32_t denominator = *(int32_t*)(data_ptr + index + 4);
                    dprintf(fd, "(%d / %d) ",
                            numerator, denominator);
                    break;
                }
                default:
                    dprintf(fd, "??? ");
            }
        }
        dprintf(fd, "]\n");
    }
}
