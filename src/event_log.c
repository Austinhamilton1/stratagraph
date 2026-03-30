#include <stdlib.h>

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
 * Insert a new event into the partition log.
 * Arguments:
 *     struct partition_log *log - Insert into this partition log.
 *     event_t event - Insert this event.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int insert_event(struct partition_log *log, event_t event) {
    struct chunk *tail = log->tail;
}