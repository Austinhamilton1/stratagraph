#ifndef SNAPSHOT_READER_H
#define SNAPSHOT_READER_H

#include "l1.h"

typedef struct {
    l1_partition_t *plog;                          // Partition log to read from
    unsigned int    last_chunk_idx[MAX_THREADS];    // Last read event index per thread
    struct chunk    *last_chunk[MAX_THREADS];       // Last read chunk per thread
} snapshot_reader_t;

/*
 * Initialize reader for a partition log.
 * Arguments:
 *     snapshot_reader_t *reader - Initialize this reader.
 *     l1_partition_t *plog - Initialize the reader on this partition log.
 */
void init_snapshot_reader(snapshot_reader_t *reader, l1_partition_t *plog);

/*
 * Process all new events since last read.
 * Arguments:
 *     snapshot_reader_t *reader - Process events from this snapshot reader.
 *     void (*process_event)(event_t *) - Function pointer to do to each event.
 */
void merge_logs(snapshot_reader_t *reader, void (*process_event)(event_t *));

#endif