#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include "worker_heap.h"
#include "task_heap.h"

#define SHM_KEY 12345 // Use a unique integer key for shared memory

// Function definitions for worker heap

HeapData* initialize_heap(void* shm_addr) {
    HeapData* heap = (HeapData*)shm_addr;
    heap->size = 0;
    return heap;
}

void swap(HeapElement& a, HeapElement& b) {
    HeapElement temp = a;
    a = b;
    b = temp;
}

void heapify_up(HeapData* heap, size_t index) {
    while (index != 0 && heap->data[(index - 1) / 2].capacity_difference() < heap->data[index].capacity_difference()) {
        swap(heap->data[index], heap->data[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

void heapify_down(HeapData* heap, size_t index) {
    size_t largest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;

    if (left < heap->size && heap->data[left].capacity_difference() > heap->data[largest].capacity_difference()) {
        largest = left;
    }
    if (right < heap->size && heap->data[right].capacity_difference() > heap->data[largest].capacity_difference()) {
        largest = right;
    }
    if (largest != index) {
        swap(heap->data[index], heap->data[largest]);
        heapify_down(heap, largest);
    }
}

void heap_push(HeapData* heap, const char* workerId, const char* timestamp, int32_t curr_capacity, int32_t total_capacity) {
    if (heap->size == MAX_WORKERS) {
        fprintf(stderr, "Heap is full\n");
        return;
    }

    strncpy(heap->data[heap->size].workerId, workerId, sizeof(heap->data[heap->size].workerId) - 1);
    heap->data[heap->size].workerId[sizeof(heap->data[heap->size].workerId) - 1] = '\0';
    strncpy(heap->data[heap->size].timestamp, timestamp, sizeof(heap->data[heap->size].timestamp) - 1);
    heap->data[heap->size].timestamp[sizeof(heap->data[heap->size].timestamp) - 1] = '\0';
    heap->data[heap->size].curr_capacity = curr_capacity;
    heap->data[heap->size].total_capacity = total_capacity;
    heapify_up(heap, heap->size);
    heap->size++;
}

HeapElement heap_pop(HeapData* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        exit(1);
    }

    HeapElement root = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    heapify_down(heap, 0);

    return root;
}

ssize_t find_index_by_id(HeapData* heap, const char* workerId) {
    for (size_t i = 0; i < heap->size; ++i) {
        if (strncmp(heap->data[i].workerId, workerId, sizeof(heap->data[i].workerId)) == 0) {
            return i;
        }
    }
    return -1;
}

bool remove_node_by_id(HeapData* heap, const char* workerId) {
    ssize_t index = find_index_by_id(heap, workerId);
    if (index == -1) {
        fprintf(stderr, "Worker ID not found\n");
        return false;
    }

    heap->data[index] = heap->data[heap->size - 1];
    heap->size--;

    if (index < heap->size) {
        heapify_down(heap, index);
        heapify_up(heap, index);
    }

    return true;
}

// Remove the main function from here
