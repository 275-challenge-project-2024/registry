#ifndef WORKER_HEAP_H
#define WORKER_HEAP_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORKERS 256

struct HeapElement {
    char workerId[32];
    char timestamp[32];
    int32_t curr_capacity;
    int32_t total_capacity;

    int32_t capacity_difference() const {
        return total_capacity - curr_capacity;
    }
};

struct HeapData {
    HeapElement data[MAX_WORKERS];
    size_t size;
};

// Function declarations
HeapData* initialize_heap(void* shm_addr);
void swap(HeapElement& a, HeapElement& b);
void heapify_up(HeapData* heap, size_t index);
void heapify_down(HeapData* heap, size_t index);
void heap_push(HeapData* heap, const char* workerId, const char* timestamp, int32_t curr_capacity, int32_t total_capacity);
HeapElement heap_pop(HeapData* heap);
ssize_t find_index_by_id(HeapData* heap, const char* workerId);
bool remove_node_by_id(HeapData* heap, const char* workerId);

#endif // WORKER_HEAP_H
