#ifndef WORKER_HEAP_H
#define WORKER_HEAP_H

#include <vector>
#include <unordered_map>
#include <ctime>
#include <iostream>

// Structure to represent a worker node
struct WorkerNode {
    int id;
    int capacity;
    time_t last_heartbeat;
    WorkerNode(int id, int capacity) : id(id), capacity(capacity) {
        last_heartbeat = std::time(nullptr);
    }
};

// Comparator for the priority queue
struct WorkerNodeComparator {
    bool operator()(const WorkerNode& a, const WorkerNode& b) {
        return a.capacity < b.capacity;
    }
};

class WorkerHeap {
private:
    std::vector<WorkerNode> heap;
    std::unordered_map<int, int> node_index_map; // Map to store node id and their index in heap

    void heapify_up(int index);
    void heapify_down(int index);

public:
    WorkerHeap();
    void add_worker(int id, int capacity);
    void remove_worker(int id);
    void update_worker_capacity(int id, int new_capacity);
    void update_worker_heartbeat(int id);
    WorkerNode get_top_worker();
    bool is_empty();
    std::vector<WorkerNode> get_all_workers();
    std::vector<int> get_dead_workers(time_t current_time, time_t threshold);
};

#endif // WORKER_HEAP_H
