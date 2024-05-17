#include "worker_heap.h"
#include <algorithm>

// Constructor
WorkerHeap::WorkerHeap() {}

// Private helper functions for heapify operations
void WorkerHeap::heapify_up(int index) {
    while (index > 0) {
        int parent_index = (index - 1) / 2;
        if (heap[parent_index].capacity >= heap[index].capacity) break;
        std::swap(heap[parent_index], heap[index]);
        node_index_map[heap[parent_index].id] = parent_index;
        node_index_map[heap[index].id] = index;
        index = parent_index;
    }
}

void WorkerHeap::heapify_down(int index) {
    int size = heap.size();
    while (index < size) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        if (left < size && heap[left].capacity > heap[largest].capacity) {
            largest = left;
        }
        if (right < size && heap[right].capacity > heap[largest].capacity) {
            largest = right;
        }
        if (largest == index) break;
        std::swap(heap[index], heap[largest]);
        node_index_map[heap[index].id] = index;
        node_index_map[heap[largest].id] = largest;
        index = largest;
    }
}

// Public methods
void WorkerHeap::add_worker(int id, int capacity) {
    WorkerNode node(id, capacity);
    heap.push_back(node);
    int index = heap.size() - 1;
    node_index_map[id] = index;
    heapify_up(index);
}

void WorkerHeap::remove_worker(int id) {
    int index = node_index_map[id];
    std::swap(heap[index], heap.back());
    heap.pop_back();
    node_index_map.erase(id);
    if (index < heap.size()) {
        node_index_map[heap[index].id] = index;
        heapify_up(index);
        heapify_down(index);
    }
}

void WorkerHeap::update_worker_capacity(int id, int new_capacity) {
    int index = node_index_map[id];
    heap[index].capacity = new_capacity;
    heapify_up(index);
    heapify_down(index);
}

void WorkerHeap::update_worker_heartbeat(int id) {
    int index = node_index_map[id];
    heap[index].last_heartbeat = std::time(nullptr);
}

WorkerNode WorkerHeap::get_top_worker() {
    return heap.front();
}

bool WorkerHeap::is_empty() {
    return heap.empty();
}

std::vector<WorkerNode> WorkerHeap::get_all_workers() {
    return heap;
}

std::vector<int> WorkerHeap::get_dead_workers(time_t current_time, time_t threshold) {
    std::vector<int> dead_workers;
    for (const auto& node : heap) {
        if (current_time - node.last_heartbeat > threshold) {
            dead_workers.push_back(node.id);
        }
    }
    return dead_workers;
}
