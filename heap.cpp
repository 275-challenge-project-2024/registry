#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_STRING_LENGTH 256
#define INITIAL_CAPACITY 100 // Initial capacity of the heap
key_t worker_heap = 'worker_heap';

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

Heap *initialize_heap(int capacity, void *shm_addr)
{
  Heap *heap = (Heap *)shm_addr;
  heap->size = 0;
  heap->capacity = capacity;
  return heap;
}

void heap_push(Heap *heap, const char *id, int value)
{
  if (heap->size == heap->capacity)
  {
    //edge case where we have more than 100 workers
    fprintf(stderr, "Heap is full\n");
    return;
  }

  int i = heap->size;
  strncpy(heap->data[i].id, id, MAX_STRING_LENGTH - 1);
  heap->data[i].id[MAX_STRING_LENGTH - 1] = '\0'; // Ensure null termination
  heap->data[i].value = value;
  heap->size++;

  //heapify after pushing element to bottom
  while (i != 0 && heap->data[(i - 1) / 2].value < heap->data[i].value)
  {
    HeapItem temp = heap->data[i];
    heap->data[i] = heap->data[(i - 1) / 2];
    heap->data[(i - 1) / 2] = temp;
    i = (i - 1) / 2;
  }
}

HeapItem heap_pop(Heap *heap)
{
  if (heap->size == 0)
  {
    fprintf(stderr, "Heap is empty\n");
    return (HeapItem){.id = "", .value = 0};
  }

  HeapItem root = heap->data[0];
  heap->data[0] = heap->data[heap->size - 1];
  heap->size--;

  //heapify after popping
  int i = 0;
  while (2 * i + 1 < heap->size)
  {
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    int largest = i;

    if (left < heap->size && heap->data[left].value > heap->data[largest].value)
      largest = left;
    if (right < heap->size && heap->data[right].value > heap->data[largest].value)
      largest = right;

    if (largest == i)
      break;

    HeapItem temp = heap->data[i];
    heap->data[i] = heap->data[largest];
    heap->data[largest] = temp;
    i = largest;
  }

  return root;
}

// example code for initializing and using the heap, instead of this plug resgistry maintenance
int main()
{
  int shm_id = shmget(worker_heap, sizeof(Heap) + INITIAL_CAPACITY * sizeof(HeapItem), IPC_CREAT | 0666);
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

  Heap *heap = initialize_heap(INITIAL_CAPACITY, shm_addr);
  //push into heap (worker_address, current_capacity)
  heap_push(heap, "worker_address", 5);
  

  //pop worker with max current_capacity
  HeapItem maxItem = heap_pop(heap);
  printf("Popped: %s, %d\n", maxItem.id, maxItem.value);

  if (shmdt(shm_addr) < 0)
  {
    perror("shmdt failed");
    exit(1);
  }

  shmctl(shm_id, IPC_RMID, NULL);

  return 0;
}
