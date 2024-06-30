#include "alloc.h"

#include "list.h"
#include "defs.h"
#include "except.h"

#include <stdint.h>

typedef struct free_chunk {
    list_entry_t link;
    size_t num_of_chunks;
    size_t _padding;
} free_chunk_t;

// choose the parameters based onthe arch
#if UINTPTR_MAX == 0xffffffff
    #define CHUNK_SHIFT     4
#elif UINTPTR_MAX == 0xffffffffffffffff
    #define CHUNK_SHIFT     5
#endif

// validate the chunk size we have
#define CHUNK_SIZE      (1 << CHUNK_SHIFT)
#define CHUNK_MASK      (CHUNK_SIZE - 1)
STATIC_ASSERT(sizeof(free_chunk_t) == CHUNK_SIZE);

/**
 * Convert a size to a chunk count (align down)
 */
#define TRUNCATE_TO_CHUNKS(a) ((a) >> CHUNK_SHIFT)

/**
 * Convert a size to chunk count (align up)
 */
#define SIZE_TO_CHUNKS(size)  \
    (((size) >> CHUNK_SHIFT) + (((size) & CHUNK_MASK) ? 1 : 0))

/**
 * Convert the chunk count to a size
 */
#define CHUNKS_TO_SIZE(chunks)  \
    ((chunks) << CHUNK_SHIFT)


/**
 * The chunk freelist
 */
static list_t m_alloc_freelist = INIT_LIST(m_alloc_freelist);

/**
 * Allocate the amount of pages from the given node (assuming it has enough space
 * to allocate). Removes the node if its out of pages. Returns the pointer of the
 * allocated memory.
 */
static size_t alloc_chunks_on_node(free_chunk_t* chunk, size_t number_of_chunks) {
    size_t chunks_left = chunk->num_of_chunks - number_of_chunks;
    if (chunks_left == 0) {
        list_remove(&chunk->link);
    } else {
        chunk->num_of_chunks = chunks_left;
    }
    return (uintptr_t)chunk + CHUNKS_TO_SIZE(chunks_left);
}

void* mem_alloc(size_t size) {
    // zero sized allocations are invalid
    if (size == 0) {
        return NULL;
    }

    // get the aligned size
    size_t num_of_pages = SIZE_TO_CHUNKS(size);

    // iterate and find a good place to put this
    for (list_entry_t* node = m_alloc_freelist.prev; node != &m_alloc_freelist; node = node->prev) {
        free_chunk_t* chunk = CR(node, free_chunk_t, link);
        if (chunk->num_of_chunks >= num_of_pages) {
            return (void*)alloc_chunks_on_node(chunk, num_of_pages);
        }
    }

    return NULL;
}

/**
 * Merge the given node and the previous next node
 * if they are one after another
 */
static free_chunk_t* alloc_merge_nodes(free_chunk_t* first) {
    free_chunk_t* next = CR(first->link.next, free_chunk_t, link);
    ASSERT(TRUNCATE_TO_CHUNKS((uintptr_t)next - (uintptr_t)first) >= first->num_of_chunks);

    if (TRUNCATE_TO_CHUNKS((uintptr_t)next - (uintptr_t)first) == first->num_of_chunks) {
        first->num_of_chunks += next->num_of_chunks;
        list_remove(&next->link);
        next = first;
    }

    return next;
}

void mem_free(void* ptr, size_t size) {
    // allow null, ignore
    if (ptr == NULL) {
        return;
    }

    // make sure the pointer is aligned
    ASSERT(((uintptr_t)ptr & CHUNK_MASK) == 0);
    ASSERT(size != 0);

    // find the location in the list to add this
    free_chunk_t* chunk = NULL;
    list_entry_t* node = m_alloc_freelist.next;
    while (node != &m_alloc_freelist) {
        chunk = CR(node, free_chunk_t, link);
        if (ptr < (void*)chunk) {
            break;
        }
        node = node->next;
    }

    // get one chunk back if possible
    if (node->prev != &m_alloc_freelist) {
        chunk = CR(node->prev, free_chunk_t, link);
    }

    // initialize the chunk struct
    chunk = (free_chunk_t*)ptr;
    chunk->num_of_chunks = SIZE_TO_CHUNKS(size);
    list_insert_tail(node, &chunk->link);

    // merge with the previous entry if possible
    if (chunk->link.prev != &m_alloc_freelist) {
        chunk = alloc_merge_nodes(CR(chunk->link.prev, free_chunk_t, link));
    }

    // merge with the next one if possible
    if (node != &m_alloc_freelist) {
        alloc_merge_nodes(chunk);
    }
}

void mem_add_range(void* ptr, size_t size) {
    mem_free(ptr, size);
}

void mem_dump(void) {
    LOG_INFO("Allocator:");

    size_t total_free = 0;
    for (list_entry_t* node = m_alloc_freelist.prev; node != &m_alloc_freelist; node = node->prev) {
        free_chunk_t* chunk = CR(node, free_chunk_t, link);
        total_free += chunk->num_of_chunks;
    }
    LOG_INFO("\ttotal free space: %d bytes", CHUNKS_TO_SIZE(total_free));
    LOG_INFO("\tfreelist:");
    for (list_entry_t* node = m_alloc_freelist.prev; node != &m_alloc_freelist; node = node->prev) {
        free_chunk_t* chunk = CR(node, free_chunk_t, link);
        LOG_INFO("\t\t%p-%p (%d bytes)",
                 chunk, (uintptr_t)chunk + CHUNKS_TO_SIZE(chunk->num_of_chunks),
                 CHUNKS_TO_SIZE(chunk->num_of_chunks));
    }
}
