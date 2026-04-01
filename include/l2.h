#ifndef L2_H
#define L2_H

#include <stddef.h>

#include "l1.h"
#include "snapshot_reader.h"

#define MAX_RUN 4096

/* A run represents a sequence of events that are sorted by time. */
struct run {
    event_t *events;    // Events dedicated to a run
    size_t  size;       // Number of events within a run
};

/* Partitions have a sequence of runs. */
typedef struct {
    struct run          *runs;      // Distinct runs stored in the partition
    size_t              num_runs;   // Number of runs in the partition
} l2_partition_t;

/*
 * Initialize a partition.
 * Arguments:
 *     l2_partition_t *prun - Inititalize this partition.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
void init_l2_partition(l2_partition_t *prun);

/*
 * Insert a batch of events into the partition.
 * Arguments:
 *     l2_partition_t *prun - Insert into this partition.
 *     event_t *events - Insert these events.
 *     size_t num_events - Number of events in batch.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int insert_batch(l2_partition_t *prun, event_t *events, size_t num_events);

#endif