#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>

#include "event_log.h"

/*
 * Allocate a new chunk on the heap.
 * Returns:
 *     struct chunk * - Newly allocated chunk.
 */
struct chunk *alloc_chunk() {
    struct chunk *chunk = malloc(sizeof(struct chunk));
    if(chunk)
        memset(chunk, 0, sizeof(*chunk));
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
 * Initialize a partition log.
 * Arguments:
 *     partition_log_t *log - Initialize this log.
 */
void init_partition_log(partition_log_t *log) {
    memset(log, 0, sizeof(*log));
    atomic_store(&log->num_threads, 0);
}

/*
 * Free data associated with a partition log.
 * Arguments:
 *     partition_log_t *log - Free this log.
 */
void free_partition_log(partition_log_t *log) {
    int n = atomic_load(&log->num_threads);
    for(int i = 0; i < n; i++) {
        thread_log_t *tlog = log->thread_logs[i];
        if(!tlog) continue;
        struct chunk *curr = tlog->head;
        while(curr) {
            struct chunk *next = curr->next;
            free_chunk(curr);
            curr = next;
        }
        free(tlog);
    }
}

/* Thread Registration */
__thread thread_log_t *tls_log = NULL;

/*
 * Returns a pointer to the per-thread log for this thread.
 * Arguments:
 *     partition_log_t *log - Log to register with.
 * Returns:
 *     thread_log_t * - New thread log.
 */
thread_log_t *register_thread_log(partition_log_t *log) {
    if(tls_log) return tls_log; // Already registered

    // Allocate a new thread log
    thread_log_t *tlog = malloc(sizeof(thread_log_t));
    if(!tlog) return NULL;

    // Initialize the thread log
    tlog->head = alloc_chunk();
    tlog->tail = tlog->head;

    // Reserve a thread log spot
    int idx = atomic_fetch_add(&log->num_threads, 1);
    if(idx >= MAX_THREADS) {
        free_chunk(tlog->head);
        free(tlog);
        return NULL;
    }

    // Register thread log
    log->thread_logs[idx] = tlog;
    tls_log = tlog;
    return tlog;
}

/*
 * Insert a new event into the partition log.
 * Arguments:
 *     thread_log_t *log - Insert into this partition log.
 *     event_t event - Insert this event.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int insert_event(thread_log_t *tlog, event_t event) {
    // Grab the tail and the write index
    struct chunk *tail = tlog->tail;
    unsigned int idx = tail->write_idx;

    if(idx < CHUNK_SIZE) {
        // Fast path, values left in chunk
        tail->events[idx] = event;
        atomic_store_explicit(&tail->write_idx, idx + 1, memory_order_release);
    } else {
        // Slow path, new chunk must be created
        struct chunk *new_chunk = alloc_chunk();
        if(!new_chunk) return -1;
        new_chunk->events[0] = event;
        new_chunk->write_idx = 1;
        new_chunk->next = NULL;
        tail->next = new_chunk;
        tlog->tail = new_chunk;
    }

    return 0;
}