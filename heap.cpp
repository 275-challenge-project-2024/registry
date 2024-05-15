#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <vector>

const char *sharedMemoryName = "/my_shared_memory";
const size_t sharedMemorySize = 1024 * 1024; // Increase the size for complex structures

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
    HeapElement data[256];
    size_t size;
};

// Function to heapify the element at index i in a max heap
void heapify(HeapData* heap, size_t i) {
    size_t largest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;

    if (left < heap->size && heap->data[left].capacity_difference() > heap->data[largest].capacity_difference()) {
        largest = left;
    }

    if (right < heap->size && heap->data[right].capacity_difference() > heap->data[largest].capacity_difference()) {
        largest = right;
    }

    if (largest != i) {
        std::swap(heap->data[i], heap->data[largest]);
        heapify(heap, largest);
    }
}

void push(HeapData* heap, const HeapElement& value) {
    if (heap->size >= 256) {
        std::cerr << "Heap overflow\n";
        return;
    }

    heap->data[heap->size] = value;
    size_t i = heap->size;
    heap->size++;

    while (i != 0 && heap->data[(i - 1) / 2].capacity_difference() < heap->data[i].capacity_difference()) {
        std::swap(heap->data[i], heap->data[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

HeapElement pop(HeapData* heap) {
    if (heap->size <= 0) {
        std::cerr << "Heap underflow\n";
        return {};
    }

    HeapElement root = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    heapify(heap, 0);

    return root;
}

int main() {
    // Clean up any previous shared memory object
    shm_unlink(sharedMemoryName);

    // Create shared memory object
    int shm_fd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Failed to create shared memory object: " << strerror(errno) << "\n";
        return 1;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, sharedMemorySize) == -1) {
        std::cerr << "Failed to set size of shared memory object: " << strerror(errno) << "\n";
        return 1;
    }

    // Map the shared memory object into the address space
    void *ptr = mmap(0, sharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "Failed to map shared memory object: " << strerror(errno) << "\n";
        return 1;
    }

    // Initialize the heap data
    HeapData *heapData = static_cast<HeapData*>(ptr);
    heapData->size = 0;

    // Example elements to push to the heap
    HeapElement element1 = {"worker1", "2023-05-15T08:00:00Z", 10, 50};
    HeapElement element2 = {"worker2", "2023-05-15T08:01:00Z", 20, 60};
    HeapElement element3 = {"worker3", "2023-05-15T08:02:00Z", 30, 70};

    // Push elements to the heap
    push(heapData, element1);
    push(heapData, element2);
    push(heapData, element3);

    std::cout << "Data pushed to heap\n";

    // Unmap the shared memory object
    if (munmap(ptr, sharedMemorySize) == -1) {
        std::cerr << "Failed to unmap shared memory object: " << strerror(errno) << "\n";
        return 1;
    }

    // Close the shared memory object
    if (close(shm_fd) == -1) {
        std::cerr << "Failed to close shared memory object: " << strerror(errno) << "\n";
        return 1;
    }

    return 0;
}
