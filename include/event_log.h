#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdint.h>
#include <stdatomic.h>

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
    union {
        struct {
            uint64_t    u;          // Node being affected
            void        *payload;   // Node update payload
        } node;
        struct {
            uint64_t    src;        // Source node
            uint64_t    dest;       // Destination node
            void        *payload;   // Edge update payload
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
} partition_log_t;

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
 *     partition_log_t *log - Initialize this log.
 */
void init_partition_log(partition_log_t *log);

/*
 * Free data associated with a partition log.
 * Arguments:
 *     partition_log_t *log - Free this log.
 */
void free_partition_log(partition_log_t *log);

/*
 * Returns a pointer to the per-thread log for this thread.
 * Arguments:
 *     partition_log_t *log - Log to register with.
 * Returns:
 *     thread_log_t * - New thread log.
 */
thread_log_t *register_thread_log(partition_log_t *log);

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