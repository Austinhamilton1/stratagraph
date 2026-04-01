#ifndef L1_H
#define L1_H

#include <stdint.h>
#include <stdatomic.h>
#include <stddef.h>

#define CHUNK_SIZE 1024
#define MAX_THREADS 128

/* Events can be one of several types. */
enum event_type {
    NODE_ADD,       // Add a new node
    NODE_DELETE,    // Delete an existing node
    NODE_UPDATE,    // Update the attributes of an existing node
    EDGE_LINK,      // Link two nodes with an edge
    EDGE_UNLINK,    // Unlink two nodes linked by an edge
    EDGE_UPDATE,    // Update the weight of an existing edge
};

/* Event in an event log of event_type type. */
typedef struct {
    enum event_type     type;       // Type of event
    uint64_t            time;       // Time stamp of the event
    void                *payload;   // Update payload (only for updates)
    size_t              size;       // Size of update payload
    union {
        uint32_t        node;       // Node involved in event
        struct {
            uint32_t    src;        // Source node
            uint32_t    dest;       // Destination node
        } edge;
    };
    
} event_t;

/* Logs are backed by chunked arrays. */
struct chunk {
    event_t         events[CHUNK_SIZE];     // Each chunk contains a maximum of 1024 events
    atomic_uint     write_idx;              // Each chunk maintains a write index for atomic writes
    struct chunk    *next;                  // Linked list of chunks to accompany arbitrary events
};

/* Each thread maintains a list of events. */
typedef struct {
    struct chunk    *head;  // New event chunks are added here
    struct chunk    *tail;  // Old event chunks are read from here
} thread_log_t;

/* Partition log with global registry. */
typedef struct {
    thread_log_t    *thread_logs[MAX_THREADS];  // Logs associated with a thread
    atomic_int      num_threads;                // Number of active threads
} l1_partition_t;

/*
 * Allocate a new chunk on the heap.
 * Returns:
 *     struct chunk * - Newly allocated chunk.
 */
struct chunk *alloc_chunk();

/*
 * Free an allocated chunk.
 * Arguments:
 *     struct chunk *chunk - Chunk to free.
 */
void free_chunk(struct chunk *chunk);

/*
 * Initialize a partition log.
 * Arguments:
 *     l1_partition_t *log - Initialize this log.
 */
void init_l1_partition(l1_partition_t *log);

/*
 * Free data associated with a partition log.
 * Arguments:
 *     l1_partition_t *log - Free this log.
 */
void free_l1_partition(l1_partition_t *log);

/*
 * Returns a pointer to the per-thread log for this thread.
 * Arguments:
 *     l1_partition_t *log - Log to register with.
 * Returns:
 *     thread_log_t * - New thread log.
 */
thread_log_t *register_thread_log(l1_partition_t *log);

/*
 * Insert a new event into a thread log.
 * Arguments:
 *     thread_log_t *log - Insert into this partition log.
 *     event_t event - Insert this event.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int insert_event(thread_log_t *tlog, event_t event);

#endif