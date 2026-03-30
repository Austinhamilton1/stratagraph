#include <stdlib.h>
#include <stdatomic.h>

#include "event_log.h"

/*
 * Allocate a new chunk on the heap.
 * Returns:
 *     struct chunk * - Newly allocated chunk.
 */
struct chunk *alloc_chunk() {
    struct chunk *chunk = malloc(sizeof(struct chunk));
    return chunk;
}

/*
 * Free an allocated chunk.
 * Arguments:
 *     struct chunk *chunk - Chunk to free.
 */
void free_chunk(struct chunk *chunk) {
    if(chunk) free(chunk);
}

/*
 * Insert a new event into the partition log. (This algorithm assumes Single Writer!!!)
 * Arguments:
 *     struct partition_log *log - Insert into this partition log.
 *     event_t event - Insert this event.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int insert_event(struct partition_log *log, event_t event) {
    // Grab the tail and write index
    struct chunk *tail = log->tail;
    unsigned int idx = tail->write_idx;

    if(idx < CHUNK_SIZE) {
        // Fast path, values left in chunk
        tail->events[idx] = event;
        atomic_store_explicit(&tail->write_idx, idx + 1, memory_order_release);
    } else {
        // Slow path, new chunk must be created
        struct chunk *new_chunk = alloc_chunk();
        new_chunk->events[0] = event;
        new_chunk->write_idx = 1;
        tail = new_chunk;
    }
}