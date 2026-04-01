#include <stdlib.h>

#include "l2.h"

#define ENTITY_NODE 0
#define ENTITY_EDGE 1

/*
 * Initialize a partition.
 * Arguments:
 *     l2_partition_t *prun - Inititalize this partition.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
void init_l2_partition(l2_partition_t *prun) {
    // Initialize L2 data
    prun->runs = NULL;
    prun->num_runs = 0;
}

/*
 * Compare two events in temporal order.
 * Arguments:
 *     void *e1 - Event 1.
 *     void *e2 - Event 2.
 * Returns:
 *     int - 0 if equal, -1 if e1 happens-before e2, 1 otherwise.
 */
static int cmp_event(void *e1, void *e2) {
    event_t *a = (event_t *)e1;
    event_t *b = (event_t *)e2;

    // 1. Entity type (node vs edge)
    int a_type = (a->type <= NODE_UPDATE) ? ENTITY_NODE : ENTITY_EDGE;
    int b_type = (b->type <= NODE_UPDATE) ? ENTITY_NODE : ENTITY_EDGE;

    if(a_type != b_type)
        return a_type < b_type ? -1 : 1;

    // 2. Entity identity
    if(a_type == ENTITY_NODE) {
        if(a->node != b->node)
            return a->node < b->node ? -1 : 1;
    } else {
        if(a->edge.src != b->edge.src)
            return a->edge.src < b->edge.src ? -1 : 1;

        if(a->edge.dest != b->edge.dest)
            return a->edge.dest < b->edge.dest ? -1 : 1;
    }

    // 3. Timestamp
    if(a->time != b->time)
        return a->time < b->time ? -1 : 1;

    // 4. Tie-breaker for deterministic ordering
    return a->type - b->type;
}

/*
 * Determines if two events occur over the same entity.
 * Arguments:
 *     event_t *a - First event.
 *     event_t *b - Second event.
 * Returns:
 *     int - 1 if the events are related, 0 if they aren't.
 */
static int same_entity(event_t *a, event_t *b) {
    int a_type = (a->type <= NODE_UPDATE) ? ENTITY_NODE : ENTITY_EDGE;
    int b_type = (b->type <= NODE_UPDATE) ? ENTITY_NODE : ENTITY_EDGE;

    if(a_type != b_type)
        return 0;

    if(a_type == ENTITY_NODE)
        return a->node == b->node;

    return (a->edge.src == b->edge.src && a->edge.dest == b->edge.dest);
}

/*
 * Semantically reduce a series of events that occur over the same entity.
 * Arguments:
 *     event_t *start - Start in a group of events to reduce.
 *     size_t len - Number of events to reduce.
 *     event_t *out - Reduce to this event.
 * Returns:
 *     int - 1 if the edge was reduced, 0 otherwise.
 */
static int reduced_events(event_t *start, size_t len, event_t *out) {

}

/*
 * Insert a batch of events into the partition.
 * Arguments:
 *     l2_partition_t *prun - Insert into this partition.
 *     event_t *events - Insert these events.
 *     size_t num_events - Number of events in batch.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int insert_batch(l2_partition_t *prun, event_t *events, size_t num_events) {
    qsort(events, num_events, sizeof(event_t), cmp_event);

    size_t write_idx = 0;

    // Semantic deduplication of events
    for(size_t i = 0; i < num_events; ) {
        size_t j = i + 1;

        // Group all events with same key
        while(j < num_events && same_entity(&events[i], &events[j])) {
            j++;
        }

        // Now [i, j) is a group for one entity
        event_t reduced;
        if(reduced_events(&events[i], j - i, &reduced)) {
            events[write_idx++] = reduced;
        }

        i = j;
    }
}
