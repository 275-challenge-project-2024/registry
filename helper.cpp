#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_STRING_LENGTH 256
#define INITIAL_CAPACITY 100
key_t worker_heap_key = 'shm_unique_key';

typedef struct
{
    char id[MAX_STRING_LENGTH];
    int value;
} HeapItem;

typedef struct
{
    int size;
    int capacity;
    HeapItem data[];
} Heap;

Heap *attach_heap()
{
    int shm_id = shmget(worker_heap_key, sizeof(Heap) + INITIAL_CAPACITY * sizeof(HeapItem), 0666);
    if (shm_id < 0)
    {
        perror("shmget failed");
        exit(1);
    }

    void *shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1)
    {
        perror("shmat failed");
        exit(1);
    }
    return (Heap *)shm_addr;
}

void print_heap(Heap *heap)
{
    printf("Heap size: %d\n", heap->size);
    printf("Heap capacity: %d\n", heap->capacity);
    printf("Heap items:\n");
    for (int i = 0; i < heap->size; ++i)
    {
        printf("ID: %s, Value: %d\n", heap->data[i].id, heap->data[i].value);
    }
}

int main()
{
    Heap *heap = attach_heap();

    if (shmdt((void *)heap) < 0)
    {
        perror("shmdt failed");
        exit(1);
    }

    return 0;
}
