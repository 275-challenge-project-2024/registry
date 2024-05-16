#ifndef TASK_HEAP_H
#define TASK_HEAP_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 256

struct TaskElement {
    char taskId[32];
    char timestamp[32];
    int32_t priority;
    int32_t execution_time;

    int32_t priority_difference() const {
        return priority;
    }
};

struct TaskHeapData {
    TaskElement data[MAX_TASKS];
    size_t size;
};

// Function declarations
TaskHeapData* initialize_task_heap(void* shm_addr);
void swap(TaskElement& a, TaskElement& b);
void task_heapify_up(TaskHeapData* heap, size_t index);
void task_heapify_down(TaskHeapData* heap, size_t index);
void task_heap_push(TaskHeapData* heap, const char* taskId, const char* timestamp, int32_t priority, int32_t execution_time);
TaskElement task_heap_pop(TaskHeapData* heap);
ssize_t find_task_index_by_id(TaskHeapData* heap, const char* taskId);
bool remove_task_by_id(TaskHeapData* heap, const char* taskId);

#endif // TASK_HEAP_H
