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

#pragma once

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALIGN_TO(val, alignment) \
    (((uintptr_t)(val) + ((alignment) - 1)) & ~((alignment) - 1))

/**
 * Tag hierarchy and enum definitions for camera_metadata_entry
 * =============================================================================
 */

/**
 * Main enum definitions are in a separate file to make it easy to
 * maintain
 */
#include "icamera_metadata_tags.h"

/**
 * Enum range for each top-level category
 */

extern unsigned int icamera_metadata_section_bounds[CAMERA_SECTION_COUNT][2];

extern const char *icamera_metadata_section_names[CAMERA_SECTION_COUNT];

/**
 * Type definitions for camera_metadata_entry
 * =============================================================================
 */
enum {
    // Unsigned 8-bit integer (uint8_t)
    ICAMERA_TYPE_BYTE = 0,
    // Signed 32-bit integer (int32_t)
    ICAMERA_TYPE_INT32 = 1,
    // 32-bit float (float)
    ICAMERA_TYPE_FLOAT = 2,
    // Signed 64-bit integer (int64_t)
    ICAMERA_TYPE_INT64 = 3,
    // 64-bit float (double)
    ICAMERA_TYPE_DOUBLE = 4,
    // A 64-bit fraction (camera_metadata_rational_t)
    ICAMERA_TYPE_RATIONAL = 5,
    // Number of type fields
    ICAMERA_NUM_TYPES
};

typedef struct icamera_metadata_rational {
    int32_t numerator;
    int32_t denominator;
} icamera_metadata_rational_t;

/**
 * A reference to a metadata entry in a buffer.
 *
 * The data union pointers point to the real data in the buffer, and can be
 * modified in-place if the count does not need to change. The count is the
 * number of entries in data of the entry's type, not a count of bytes.
 */
typedef struct icamera_metadata_entry {
    size_t   index;
    uint32_t tag;
    uint8_t  type;
    size_t   count;
    union {
        uint8_t *u8;
        int32_t *i32;
        float   *f;
        int64_t *i64;
        double  *d;
        icamera_metadata_rational_t *r;
    } data;
} icamera_metadata_entry_t;

/**
 * A read-only reference to a metadata entry in a buffer. Identical to
 * camera_metadata_entry in layout
 */
typedef struct icamera_metadata_ro_entry {
    size_t   index;
    uint32_t tag;
    uint8_t  type;
    size_t   count;
    union {
        const uint8_t *u8;
        const int32_t *i32;
        const float   *f;
        const int64_t *i64;
        const double  *d;
        const icamera_metadata_rational_t *r;
    } data;
} icamera_metadata_ro_entry_t;

/**
 * Size in bytes of each entry type
 */
extern const size_t icamera_metadata_type_size[ICAMERA_NUM_TYPES];

/**
 * Human-readable name of each entry type
 */
extern const char* icamera_metadata_type_names[ICAMERA_NUM_TYPES];

/**
 * Main definitions for the metadata entry and array structures
 * =============================================================================
 */

/**
 * A packet of metadata. This is a list of metadata entries, each of which has
 * an integer tag to identify its meaning, 'type' and 'count' field, and the
 * data, which contains a 'count' number of entries of type 'type'. The packet
 * has a fixed capacity for entries and for extra data.  A new entry uses up one
 * entry slot, and possibly some amount of data capacity; the function
 * calculate_camera_metadata_entry_data_size() provides the amount of data
 * capacity that would be used up by an entry.
 *
 * Entries are not sorted by default, and are not forced to be unique - multiple
 * entries with the same tag are allowed. The packet will not dynamically resize
 * when full.
 *
 * The packet is contiguous in memory, with size in bytes given by
 * get_camera_metadata_size(). Therefore, it can be copied safely with memcpy()
 * to a buffer of sufficient size. The copy_camera_metadata() function is
 * intended for eliminating unused capacity in the destination packet.
 */
struct icamera_metadata;
typedef struct icamera_metadata icamera_metadata_t;

/**
 * Functions for manipulating camera metadata
 * =============================================================================
 *
 * NOTE: Unless otherwise specified, functions that return type "int"
 * return 0 on success, and non-0 value on error.
 */

/**
 * Allocate a new camera_metadata structure, with some initial space for entries
 * and extra data. The entry_capacity is measured in entry counts, and
 * data_capacity in bytes. The resulting structure is all contiguous in memory,
 * and can be freed with free_camera_metadata().
 */
icamera_metadata_t *allocate_icamera_metadata(size_t entry_capacity,
        size_t data_capacity);

/**
 * Get the required alignment of a packet of camera metadata, which is the
 * maximal alignment of the embedded camera_metadata, camera_metadata_buffer_entry,
 * and camera_metadata_data.
 */
size_t get_icamera_metadata_alignment();

/**
 * Allocate a new camera_metadata structure of size src_size. Copy the data,
 * ignoring alignment, and then attempt validation. If validation
 * fails, free the memory and return NULL. Otherwise return the pointer.
 *
 * The resulting pointer can be freed with free_camera_metadata().
 */
icamera_metadata_t *allocate_copy_icamera_metadata_checked(
        const icamera_metadata_t *src,
        size_t src_size);

/**
 * Place a camera metadata structure into an existing buffer. Returns NULL if
 * the buffer is too small for the requested number of reserved entries and
 * bytes of data. The entry_capacity is measured in entry counts, and
 * data_capacity in bytes. If the buffer is larger than the required space,
 * unused space will be left at the end. If successful, returns a pointer to the
 * metadata header placed at the start of the buffer. It is the caller's
 * responsibility to free the original buffer; do not call
 * free_camera_metadata() with the returned pointer.
 */
icamera_metadata_t *place_icamera_metadata(void *dst, size_t dst_size,
        size_t entry_capacity,
        size_t data_capacity);

/**
 * Free a camera_metadata structure. Should only be used with structures
 * allocated with allocate_camera_metadata().
 */
void free_icamera_metadata(icamera_metadata_t *metadata);

/**
 * Calculate the buffer size needed for a metadata structure of entry_count
 * metadata entries, needing a total of data_count bytes of extra data storage.
 */
size_t calculate_icamera_metadata_size(size_t entry_count,
        size_t data_count);

/**
 * Get current size of entire metadata structure in bytes, including reserved
 * but unused space.
 */
size_t get_icamera_metadata_size(const icamera_metadata_t *metadata);

/**
 * Get size of entire metadata buffer in bytes, not including reserved but
 * unused space. This is the amount of space needed by copy_camera_metadata for
 * its dst buffer.
 */
size_t get_icamera_metadata_compact_size(const icamera_metadata_t *metadata);

/**
 * Get the current number of entries in the metadata packet.
 *
 * metadata packet must be valid, which can be checked before the call with
 * validate_camera_metadata_structure().
 */
size_t get_icamera_metadata_entry_count(const icamera_metadata_t *metadata);

/**
 * Get the maximum number of entries that could fit in the metadata packet.
 */
size_t get_icamera_metadata_entry_capacity(const icamera_metadata_t *metadata);

/**
 * Get the current count of bytes used for value storage in the metadata packet.
 */
size_t get_icamera_metadata_data_count(const icamera_metadata_t *metadata);

/**
 * Get the maximum count of bytes that could be used for value storage in the
 * metadata packet.
 */
size_t get_icamera_metadata_data_capacity(const icamera_metadata_t *metadata);

/**
 * Copy a metadata structure to a memory buffer, compacting it along the
 * way. That is, in the copied structure, entry_count == entry_capacity, and
 * data_count == data_capacity.
 *
 * If dst_size > get_camera_metadata_compact_size(), the unused bytes are at the
 * end of the buffer. If dst_size < get_camera_metadata_compact_size(), returns
 * NULL. Otherwise returns a pointer to the metadata structure header placed at
 * the start of dst.
 *
 * Since the buffer was not allocated by allocate_camera_metadata, the caller is
 * responsible for freeing the underlying buffer when needed; do not call
 * free_camera_metadata.
 */
icamera_metadata_t *copy_icamera_metadata(void *dst, size_t dst_size,
        const icamera_metadata_t *src);

/**
 * Validate that a metadata is structurally sane. That is, its internal
 * state is such that we won't get buffer overflows or run into other
 * 'impossible' issues when calling the other API functions.
 *
 * This is useful in particular after copying the binary metadata blob
 * from an untrusted source, since passing this check means the data is at least
 * consistent.
 *
 * The expected_size argument is optional.
 *
 * Returns 0 on success. A non-0 value is returned on error.
 */
int validate_icamera_metadata_structure(const icamera_metadata_t *metadata,
                                        const size_t *expected_size);

/**
 * Append camera metadata in src to an existing metadata structure in dst.  This
 * does not resize the destination structure, so if it is too small, a non-zero
 * value is returned. On success, 0 is returned. Appending onto a sorted
 * structure results in a non-sorted combined structure.
 */
int append_icamera_metadata(icamera_metadata_t *dst, const icamera_metadata_t *src);

/**
 * Clone an existing metadata buffer, compacting along the way. This is
 * equivalent to allocating a new buffer of the minimum needed size, then
 * appending the buffer to be cloned into the new buffer. The resulting buffer
 * can be freed with free_camera_metadata(). Returns NULL if cloning failed.
 */

icamera_metadata_t *clone_icamera_metadata(const icamera_metadata_t *src);
/**
 * Calculate the number of bytes of extra data a given metadata entry will take
 * up. That is, if entry of 'type' with a payload of 'data_count' values is
 * added, how much will the value returned by get_camera_metadata_data_count()
 * be increased? This value may be zero, if no extra data storage is needed.
 */
size_t calculate_icamera_metadata_entry_data_size(uint8_t type,
        size_t data_count);

/**
 * Add a metadata entry to a metadata structure. Returns 0 if the addition
 * succeeded. Returns a non-zero value if there is insufficient reserved space
 * left to add the entry, or if the tag is unknown.  data_count is the number of
 * entries in the data array of the tag's type, not a count of
 * bytes. Entries are always added to the end of the structure (highest index),
 * so after addition, a previously-sorted array will be marked as unsorted.
 *
 * Returns 0 on success. A non-0 value is returned on error.
 */
int add_icamera_metadata_entry(icamera_metadata_t *dst,
        uint32_t tag,
        const void *data,
        size_t data_count);

/**
 * Sort the metadata buffer for fast searching. If already marked as sorted,
 * does nothing. Adding or appending entries to the buffer will place the buffer
 * back into an unsorted state.
 *
 * Returns 0 on success. A non-0 value is returned on error.
 */
int sort_icamera_metadata(icamera_metadata_t *dst);

/**
 * Get metadata entry at position index in the metadata buffer.
 * Index must be less than entry count, which is returned by
 * get_icamera_metadata_entry_count().
 *
 * src and index are inputs; the passed-in entry is updated with the details of
 * the entry. The data pointer points to the real data in the buffer, and can be
 * updated as long as the data count does not change.
 *
 * Returns 0 on success. A non-0 value is returned on error.
 */
int get_icamera_metadata_entry(icamera_metadata_t *src,
        size_t index,
        icamera_metadata_entry_t *entry);

/**
 * Get metadata entry at position index, but disallow editing the data.
 */
int get_icamera_metadata_ro_entry(const icamera_metadata_t *src,
        size_t index,
        icamera_metadata_ro_entry_t *entry);

/**
 * Find an entry with given tag value. If not found, returns -ENOENT. Otherwise,
 * returns entry contents like get_camera_metadata_entry.
 *
 * If multiple entries with the same tag exist, does not have any guarantees on
 * which is returned. To speed up searching for tags, sort the metadata
 * structure first by calling sort_camera_metadata().
 */
int find_icamera_metadata_entry(icamera_metadata_t *src,
        uint32_t tag,
        icamera_metadata_entry_t *entry);

/**
 * Find an entry with given tag value, but disallow editing the data
 */
int find_icamera_metadata_ro_entry(const icamera_metadata_t *src,
        uint32_t tag,
        icamera_metadata_ro_entry_t *entry);

/**
 * Delete an entry at given index. This is an expensive operation, since it
 * requires repacking entries and possibly entry data. This also invalidates any
 * existing camera_metadata_entry.data pointers to this buffer. Sorting is
 * maintained.
 */
int delete_icamera_metadata_entry(icamera_metadata_t *dst,
        size_t index);

/**
 * Updates a metadata entry with new data. If the data size is changing, may
 * need to adjust the data array, making this an O(N) operation. If the data
 * size is the same or still fits in the entry space, this is O(1). Maintains
 * sorting, but invalidates camera_metadata_entry instances that point to the
 * updated entry. If a non-NULL value is passed in to entry, the entry structure
 * is updated to match the new buffer state.  Returns a non-zero value if there
 * is no room for the new data in the buffer.
 */
int update_icamera_metadata_entry(icamera_metadata_t *dst,
        size_t index,
        const void *data,
        size_t data_count,
        icamera_metadata_entry_t *updated_entry);

/**
 * Retrieve human-readable name of section the tag is in. Returns NULL if
 * no such tag is defined.
 */
const char *get_icamera_metadata_section_name(uint32_t tag);

/**
 * Retrieve human-readable name of tag (not including section). Returns NULL if
 * no such tag is defined.
 */
const char *get_icamera_metadata_tag_name(uint32_t tag);

/**
 * Retrieve the type of a tag. Returns -1 if no such tag is defined.
 */
int get_icamera_metadata_tag_type(uint32_t tag);

/**
 * Print fields in the metadata to the log.
 * verbosity = 0: Only tag entry information
 * verbosity = 1: Tag entry information plus at most 16 data values
 * verbosity = 2: All information
 */
void dump_icamera_metadata(const icamera_metadata_t *metadata,
        int fd,
        int verbosity);

/**
 * Print fields in the metadata to the log; adds indentation parameter, which
 * specifies the number of spaces to insert before each line of the dump
 */
void dump_indented_icamera_metadata(const icamera_metadata_t *metadata,
        int fd,
        int verbosity,
        int indentation);

/**
 * Prints the specified tag value as a string. Only works for enum tags.
 * Returns 0 on success, -1 on failure.
 */
int icamera_metadata_enum_snprint(uint32_t tag,
                                  int32_t value,
                                  char *dst,
                                  size_t size);

#ifdef __cplusplus
}
#endif
