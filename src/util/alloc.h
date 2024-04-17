#pragma once

#include <stddef.h>

/**
 * Allocate memory from the allocator, will be at least 16 byte aligned and
 * will align the size to 16 bytes
 */
void* mem_alloc(size_t size);

/**
 * Free the memory allocated, must be the same size as the mem_alloc
 */
void mem_free(void* ptr, size_t size);

/**
 * Add a range to the allocator
 */
void mem_add_range(void* ptr, size_t size);

/**
 * Dump the freelist of the allocator
 */
void mem_dump(void);
