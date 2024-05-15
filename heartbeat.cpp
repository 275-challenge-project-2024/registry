#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

const char *sharedMemoryName = "/my_shared_memory";
const size_t sharedMemorySize = 1024 * 1024; // Must match heap.cpp

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

int main() {
    // Open the shared memory object
    int shm_fd = shm_open(sharedMemoryName, O_RDONLY, 0666);
    if (shm_fd == -1) {
        std::cerr << "Failed to open shared memory object: " << strerror(errno) << "\n";
        return 1;
    }

    // Map the shared memory object into the address space
    void *ptr = mmap(0, sharedMemorySize, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "Failed to map shared memory object: " << strerror(errno) << "\n";
        return 1;
    }

    // Read the heap data
    HeapData *heapData = static_cast<HeapData*>(ptr);

    // Print the heap data
    std::cout << "Heap data size: " << heapData->size << "\n";
    for (size_t i = 0; i < heapData->size; ++i) {
        std::cout << "Data[" << i << "]: workerId=" << heapData->data[i].workerId
                  << ", timestamp=" << heapData->data[i].timestamp
                  << ", curr_capacity=" << heapData->data[i].curr_capacity
                  << ", total_capacity=" << heapData->data[i].total_capacity
                  << ", capacity_difference=" << heapData->data[i].capacity_difference()
                  << "\n";
    }

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
