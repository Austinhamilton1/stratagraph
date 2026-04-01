#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "l1.h"
#include "snapshot_reader.h"

#define NS_PER_SEC 1000000000.0

struct event_arg {
    event_t         *start;
    event_t         *end;
    l1_partition_t  *plog;
};

void *add_event(void *ptr) {
    struct event_arg *arg = (struct event_arg *)ptr;
    thread_log_t *tlog = register_thread_log(arg->plog);
    for(event_t *e = arg->start; e < arg->end; e++) {
        insert_event(tlog, *e);
    }
    return NULL;
}

static int event_count = 0;

void process_event(event_t *event) {
    event_count++;
}

int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <num_events> <num_threads>\n", argv[0]);
        return -1;
    }

    struct timespec start, end;
    double elapsed_time;

    l1_partition_t plog;
    init_l1_partition(&plog);

    int num_events = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    event_t *events = malloc(num_events * sizeof(event_t));
    pthread_t threads[num_threads];
    struct event_arg args[num_threads];

    for(int i = 0; i < num_events; i++) {
        events[i] = (event_t){.type=NODE_ADD, .time=i, .node=i};
    }

    for(int i = 0; i < num_threads; i++) {
        int base = num_events / num_threads;
        int rem = num_events % num_threads;

        int start_idx = i * base + (i < rem ? i : rem);
        int size = base + (i < rem ? 1 : 0);
        int end_idx = start_idx + size;

        args[i] = (struct event_arg){.start=events+start_idx, .end=events+end_idx, .plog=&plog};
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, add_event, (void *)&args[i]);
    }

    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec - start.tv_sec) +
                    (end.tv_nsec - start.tv_nsec) / NS_PER_SEC;

    snapshot_reader_t reader;
    init_snapshot_reader(&reader, &plog);

    merge_logs(&reader, process_event);

    free_l1_partition(&plog);

    printf("Event Log Benchmark Results:\n");
    printf("----------------------------\n");
    printf("%d items added in %f seconds\n", num_events, elapsed_time);
    printf("Results:\n");
    if(event_count == num_events) {
        printf("%d events merged.\n", num_events);
    } else {
        printf("%d missing events.\n", num_events - event_count);
    }
    printf("----------------------------\n");
}
