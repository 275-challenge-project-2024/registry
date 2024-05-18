#ifndef TASK_HEAP_H
#define TASK_HEAP_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX_TASKS 10000

struct TaskHeapElement {
    char taskId[32];
    char timestamp[32];
    int32_t priority;
};

struct TaskHeapData {
    struct TaskHeapElement data[MAX_TASKS];
    size_t size;
    pthread_mutex_t mutex;
};

struct TaskHeapData* initialize_task_heap(void* shm_addr);
void task_swap(struct TaskHeapElement* a, struct TaskHeapElement* b);
void task_heapify_up(struct TaskHeapData* heap, size_t index);
void task_heapify_down(struct TaskHeapData* heap, size_t index);
void task_heap_push(struct TaskHeapData* heap, const char* taskId, const char* timestamp, int32_t priority);
struct TaskHeapElement task_heap_pop(struct TaskHeapData* heap);
ssize_t task_find_index_by_id(struct TaskHeapData* heap, const char* taskId);
bool task_remove_node_by_id(struct TaskHeapData* heap, const char* taskId);

#endif
