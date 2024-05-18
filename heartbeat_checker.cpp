#include <iostream>
#include <thread>
#include <chrono>
#include "worker_heap.h"

void heartbeat_checker(WorkerHeap& worker_heap, time_t heartbeat_interval, time_t threshold) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(heartbeat_interval));
        time_t current_time = std::time(nullptr);
        std::vector<int> dead_workers = worker_heap.get_dead_workers(current_time, threshold);

        for (int id : dead_workers) {
            std::cout << "Worker " << id << " is dead. Removing from registry." << std::endl;
            worker_heap.remove_worker(id);
        }

        std::cout << "Heartbeat check completed at " << current_time << std::endl;
    }
}

int main() {
    WorkerHeap worker_heap;
    time_t heartbeat_interval = 10;
    time_t threshold = 30;

    std::thread checker(heartbeat_checker, std::ref(worker_heap), heartbeat_interval, threshold);

    std::cout << "Adding worker 1 with capacity 100" << std::endl;
    worker_heap.add_worker(1, 100);

    std::this_thread::sleep_for(std::chrono::seconds(15));
    std::cout << "Updating heartbeat for worker 1" << std::endl;
    worker_heap.update_worker_heartbeat(1);

    std::this_thread::sleep_for(std::chrono::seconds(20));

    checker.join();
    return 0;
}
