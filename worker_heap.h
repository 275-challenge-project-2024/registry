#ifndef WORKER_HEAP_H
#define WORKER_HEAP_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX_WORKERS 10000

// Structure to store each worker element in the heap
struct WorkerHeapElement {
    char workerId[32];
    char timestamp[32];
    int32_t priority;
    int32_t capacity_difference;
};

// Structure to store the worker heap data
struct WorkerHeapData {
    struct WorkerHeapElement data[MAX_WORKERS];
    size_t size;
    pthread_mutex_t mutex; // Add the mutex here
};

// Function prototypes for worker heap
struct WorkerHeapData* initialize_worker_heap(void* shm_addr);
void worker_swap(struct WorkerHeapElement* a, struct WorkerHeapElement* b);
void worker_heapify_up(struct WorkerHeapData* heap, size_t index);
void worker_heapify_down(struct WorkerHeapData* heap, size_t index);
void worker_heap_push(struct WorkerHeapData* heap, const char* workerId, const char* timestamp, int32_t priority, int32_t capacity_difference);
struct WorkerHeapElement worker_heap_pop(struct WorkerHeapData* heap);
ssize_t worker_find_index_by_id(struct WorkerHeapData* heap, const char* workerId);
bool worker_remove_node_by_id(struct WorkerHeapData* heap, const char* workerId);

#endif // WORKER_HEAP_H
