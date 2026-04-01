#include "snapshot_reader.h"

/*
 * Initialize reader for a partition log.
 * Arguments:
 *     snapshot_reader_t *reader - Initialize this reader.
 *     l1_partition_t *plog - Initialize the reader on this partition log.
 */
void init_snapshot_reader(snapshot_reader_t *reader, l1_partition_t *plog) {
    reader->plog = plog;
    int nthreads = atomic_load(&plog->num_threads);
    for(int t = 0; t < nthreads; t++) {
        reader->last_chunk[t] = plog->thread_logs[t]->head;
        reader->last_chunk_idx[t] = 0;
    }
}

/*
 * Process all new events since last read.
 * Arguments:
 *     snapshot_reader_t *reader - Process events from this snapshot reader.
 *     void (*process_event)(event_t *) - Function pointer to do to each event.
 */
void merge_logs(snapshot_reader_t *reader, void (*process_event)(event_t *)) {
    l1_partition_t *plog = reader->plog;
    int nthreads = atomic_load(&plog->num_threads);

    for(int t = 0; t < nthreads; t++) {
        // Load thread log
        thread_log_t *tlog = plog->thread_logs[t];
        if(!tlog) continue;

        // Get the last read chunk and index
        struct chunk *chunk = reader->last_chunk[t];
        unsigned int start_idx = reader->last_chunk_idx[t];
        unsigned int max_idx = start_idx;

        while(chunk) {
            max_idx = atomic_load_explicit(&chunk->write_idx, memory_order_acquire); // Snapshot of committed events
            
            // Process unread events 
            for(unsigned int i = start_idx; i < max_idx; i++) {
                process_event(&chunk->events[i]);
            }

            // Move to next chunk
            chunk = chunk->next;
            start_idx = 0; // After first chunk, restart index back at 0
        }
        
        // Save cached reader metadata
        reader->last_chunk[t] = chunk;
        reader->last_chunk_idx[t] = max_idx;
    }
}