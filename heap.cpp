#include "worker_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Initialize the worker heap structure
struct WorkerHeapData* initialize_worker_heap(void* shm_addr) {
    struct WorkerHeapData* heap = (struct WorkerHeapData*)shm_addr;
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    heap->size = 0;
    if (pthread_mutex_init(&heap->mutex, &attr) != 0) {
        printf("Mutex initialization failed\n");
        return NULL;
    }

    pthread_mutexattr_destroy(&attr); // Clean up attribute
    // printf("Heap initialized with size: %zu\n", heap->size);

    return heap;
}

// Swap two heap elements
void worker_swap(struct WorkerHeapElement* a, struct WorkerHeapElement* b) {
    struct WorkerHeapElement temp = *a;
    *a = *b;
    *b = temp;
}

// Reorder the heap from the index upwards
void worker_heapify_up(struct WorkerHeapData* heap, size_t index) {
    while (index != 0 && heap->data[(index - 1) / 2].priority < heap->data[index].priority) {
        worker_swap(&heap->data[index], &heap->data[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

// Reorder the heap from the index downwards
void worker_heapify_down(struct WorkerHeapData* heap, size_t index) {
    size_t largest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;

    if (left < heap->size && heap->data[left].priority > heap->data[largest].priority) {
        largest = left;
    }
    if (right < heap->size && heap->data[right].priority > heap->data[largest].priority) {
        largest = right;
    }
    if (largest != index) {
        worker_swap(&heap->data[index], &heap->data[largest]);
        worker_heapify_down(heap, largest);
    }
}

// Add an element to the worker heap
void worker_heap_push(struct WorkerHeapData* heap, const char* workerId, const char* timestamp, int32_t priority, int32_t capacity_difference) {
    if (heap->size == MAX_WORKERS) {
        fprintf(stderr, "Heap is full\n");
        return;
    }

    strncpy(heap->data[heap->size].workerId, workerId, sizeof(heap->data[heap->size].workerId) - 1);
    heap->data[heap->size].workerId[sizeof(heap->data[heap->size].workerId) - 1] = '\0'; // Ensure null termination

    strncpy(heap->data[heap->size].timestamp, timestamp, sizeof(heap->data[heap->size].timestamp) - 1);
    heap->data[heap->size].timestamp[sizeof(heap->data[heap->size].timestamp) - 1] = '\0'; // Ensure null termination

    heap->data[heap->size].priority = priority;
    heap->data[heap->size].capacity_difference = capacity_difference;

    heap->size++;
    worker_heapify_up(heap, heap->size - 1);
    printf("Worker %s pushed to heap with priority %d\n", workerId, priority);
}

// Remove the highest priority element from the worker heap
struct WorkerHeapElement worker_heap_pop(struct WorkerHeapData* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        return (struct WorkerHeapElement){}; // Default initialization
    }

    struct WorkerHeapElement root = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    worker_heapify_down(heap, 0);

    return root;
}

// Find the index of a worker element by worker ID
ssize_t worker_find_index_by_id(struct WorkerHeapData* heap, const char* workerId) {
    for (size_t i = 0; i < heap->size; ++i) {
        if (strncmp(heap->data[i].workerId, workerId, sizeof(heap->data[i].workerId)) == 0) {
            return i;
        }
    }
    return -1;
}

// Remove a specific worker element from the heap
bool worker_remove_node_by_id(struct WorkerHeapData* heap, const char* workerId) {
    ssize_t index = worker_find_index_by_id(heap, workerId);
    if (index == -1) {
        fprintf(stderr, "Worker ID not found\n");
        return false;
    }

    heap->data[index] = heap->data[heap->size - 1];
    heap->size--;

    if (index < heap->size) {
        worker_heapify_down(heap, index);
        worker_heapify_up(heap, index);
    }

    return true;
}
