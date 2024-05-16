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

#define SHM_KEY 12345

void monitor_worker_heartbeats() {
    int shm_id = shmget(SHM_KEY, sizeof(HeapData), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        return;
    }

    void* shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("shmat failed");
        return;
    }

    HeapData* heap = initialize_heap(shm_addr);

    // Example usage of heap functions
    heap_push(heap, "worker_4", "2024-05-16T02:00:00Z", 2, 10);

    // Check and print the worker with max capacity_difference
    if (heap->size > 0) {
        HeapElement top_worker = heap_pop(heap);
        printf("Worker with max capacity difference: %s\n", top_worker.workerId);
    }

    if (shmdt(shm_addr) < 0) {
        perror("shmdt failed");
    }
}

int main() {
    monitor_worker_heartbeats();
    return 0;
}
