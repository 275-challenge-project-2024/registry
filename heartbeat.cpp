#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include "worker_heap.h"

#define SHM_KEY 12345

void monitor_worker_heartbeats() {
    // printf("Attempting to get shared memory segment...\n");
    int shm_id = shmget(SHM_KEY, sizeof(struct WorkerHeapData), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        return;
    }

    // printf("Attempting to attach shared memory segment...\n");
    void* shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("shmat failed");
        return;
    }

    // printf("Attempting to initialize heap...\n");
    struct WorkerHeapData* heap = initialize_worker_heap(shm_addr);
    if (heap == NULL) {
        printf("Failed to initialize heap\n");
        return;
    }

    // Example usage of heap functions with mutex locking
    // printf("Attempting to push worker_4 to heap...\n");
    pthread_mutex_lock(&heap->mutex); // Lock the mutex before modifying the heap
    worker_heap_push(heap, "worker_4", "2024-05-16T02:00:00Z", 2, 10);
    pthread_mutex_unlock(&heap->mutex); // Unlock the mutex after modifying the heap
    // printf("Pushed worker_4 to heap\n");

    // Check and print the worker with max capacity_difference
    // printf("Attempting to pop worker from heap...\n");
    pthread_mutex_lock(&heap->mutex); // Lock the mutex before reading the heap
    if (heap->size > 0) {
        struct WorkerHeapElement top_worker = worker_heap_pop(heap);
        pthread_mutex_unlock(&heap->mutex); // Unlock the mutex after reading the heap
        // printf("Worker with max capacity difference: %s\n", top_worker.workerId);
    } else {
        pthread_mutex_unlock(&heap->mutex); // Unlock the mutex if heap is empty
        printf("Heap is empty\n");
    }

    if (shmdt(shm_addr) < 0) {
        perror("shmdt failed");
    }
}

int main() {
    // printf("Starting monitor_worker_heartbeats...\n");
    monitor_worker_heartbeats();
    return 0;
}
