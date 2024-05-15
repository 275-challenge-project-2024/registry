#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_WORKERS 256
#define SHM_KEY 12345 // Use a unique integer key for shared memory

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
    heap->data[heap->size].workerId[sizeof(heap->data[heap->size].workerId) - 1] = '\0'; // Ensure null termination

    strncpy(heap->data[heap->size].timestamp, timestamp, sizeof(heap->data[heap->size].timestamp) - 1);
    heap->data[heap->size].timestamp[sizeof(heap->data[heap->size].timestamp) - 1] = '\0'; // Ensure null termination

    heap->data[heap->size].curr_capacity = curr_capacity;
    heap->data[heap->size].total_capacity = total_capacity;

    heap->size++;
    heapify_up(heap, heap->size - 1);
}

HeapElement heap_pop(HeapData* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        return HeapElement(); // Default initialization
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

// Example code for initializing and using the heap
int main() {
    int shm_id = shmget(SHM_KEY, sizeof(HeapData), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        exit(1);
    }

    void* shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    HeapData* heap = initialize_heap(shm_addr);
    // Push into heap (workerId, timestamp, curr_capacity, total_capacity)
    heap_push(heap, "worker_1", "2024-05-15T12:00:00Z", 5, 10);
    heap_push(heap, "worker_2", "2024-05-15T12:01:00Z", 3, 15);
    heap_push(heap, "worker_3", "2024-05-15T12:02:00Z", 10, 20);

    // Remove worker by ID
    remove_node_by_id(heap, "worker_2");

    // Pop worker with max capacity_difference
    HeapElement maxItem = heap_pop(heap);
    printf("Popped: %s, Timestamp: %s, Current Capacity: %d, Total Capacity: %d\n", maxItem.workerId, maxItem.timestamp, maxItem.curr_capacity, maxItem.total_capacity);

    if (shmdt(shm_addr) < 0) {
        perror("shmdt failed");
        exit(1);
    }

    // shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
