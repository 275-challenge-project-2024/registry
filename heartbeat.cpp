#include <iostream>
#include <sstream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <vector>
#include <mutex>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;

#define MAX_WORKERS 256
#define SHM_KEY 12345
#define MAX_TIMEOUT 300000LL

std::mutex mutex_heap;

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
    heap->data[heap->size].workerId[sizeof(heap->data[heap->size].workerId) - 1] = '\0';

    strncpy(heap->data[heap->size].timestamp, timestamp, sizeof(heap->data[heap->size].timestamp) - 1);
    heap->data[heap->size].timestamp[sizeof(heap->data[heap->size].timestamp) - 1] = '\0';

    heap->data[heap->size].curr_capacity = curr_capacity;
    heap->data[heap->size].total_capacity = total_capacity;

    heap->size++;
    heapify_up(heap, heap->size - 1);
}

HeapElement heap_pop(HeapData* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        return HeapElement();
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

HeapData *attach_heap()
{
    mutex_heap.lock();

    int shm_id = shmget(SHM_KEY, sizeof(HeapData), IPC_CREAT | 0666);
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

    mutex_heap.unlock();
    
    return (HeapData *)shm_addr;
}

void print_heap(HeapData *heap)
{
    printf("Heap size: %zu\n", heap->size);
    printf("Heap items:\n");
    for (size_t i = 0; i < heap->size; ++i)
    {
        printf("Worker ID: %s, Timestamp: %s, Current Capacity: %d, Total Capacity: %d\n",
               heap->data[i].workerId, heap->data[i].timestamp,
               heap->data[i].curr_capacity, heap->data[i].total_capacity);
    }
}

std::string getCurrentTimeInISO8601() {
    
    auto now = std::chrono::system_clock::now();
    
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm* now_tm = std::gmtime(&now_time_t);
    
    std::ostringstream oss;
    oss << std::put_time(now_tm, "%Y-%m-%dT%H:%M:%SZ");
    
    return oss.str();
}

std::time_t parseTime(const std::string& time_str) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return std::mktime(&tm);
}

long long get_milliseconds_difference(const char* timestamp) {

    std::string str(timestamp);

    std::time_t t1 = parseTime(timestamp);

    std::string curr_time = getCurrentTimeInISO8601();

    std::time_t t2 = parseTime(curr_time);

    std::chrono::seconds difference_seconds = std::chrono::seconds(t2 - t1);

    long long difference_milliseconds = difference_seconds.count() * 1000;

    return difference_milliseconds;
}

void check_heartbeat(HeapData *heap)
{
    print_heap(heap);

    for (size_t i = 0; i < heap->size; ++i)
    {
        long long milliseconds_difference = get_milliseconds_difference(heap->data[i].timestamp);
        if(milliseconds_difference > MAX_TIMEOUT) {
            std::cout << "Removing Worker Node: " << heap->data[i].workerId << std::endl;
            std::cout << "Difference in Last ping (milliseconds): " << milliseconds_difference << std::endl;
            remove_node_by_id(heap, heap->data[i].workerId);
        }
    }
    std::cout << "" << std::endl;
}

struct Client {
    int id;
    int capacity;
};

class ClientList {
public:
    std::vector<Client> clients;

    static ClientList* createSharedMemory(const char* shm_name, size_t shm_size) {
        int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open failed");
            exit(1);
        }

        if (ftruncate(shm_fd, shm_size) == -1) {
            perror("ftruncate failed");
            exit(1);
        }

        void* addr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (addr == MAP_FAILED) {
            perror("mmap failed");
            exit(1);
        }

        return static_cast<ClientList*>(addr);
    }

    static void destroySharedMemory(const char* shm_name, size_t shm_size) {
        if (shm_unlink(shm_name) == -1) {
            perror("shm_unlink failed");
            exit(1);
        }
    }

    void addClient(const Client& client) {
        clients.push_back(client);
    }

    void removeClient(int id) {
        auto it = std::remove_if(clients.begin(), clients.end(), [id](const Client& client) {
            return client.id == id;
        });
        if (it != clients.end()) {
            clients.erase(it, clients.end());
        }
    }

    const std::vector<Client>& getClients() const {
        return clients;
    }
};

int main() {
    const char* shm_name = "/client_list_shm";
    size_t shm_size = sizeof(ClientList);

    ClientList* clientList = ClientList::createSharedMemory(shm_name, shm_size);

    Client client1 = {1, 100};
    Client client2 = {2, 200};
    clientList->addClient(client1);
    clientList->addClient(client2);

    while (true) {
        HeapData *heap = attach_heap();

        check_heartbeat(heap);

        if (shmdt((void *)heap) < 0) {
            perror("shmdt failed");
            exit(1);
        }

        const std::vector<Client>& clients = clientList->getClients();
        for (const auto& client : clients) {
            std::cout << "Client ID: " << client.id << ", Capacity: " << client.capacity << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ClientList::destroySharedMemory(shm_name, shm_size);

    return 0;
}
