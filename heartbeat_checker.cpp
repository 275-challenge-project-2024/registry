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
    time_t heartbeat_interval = 10; // Check every 10 seconds
    time_t threshold = 30; // Worker considered dead if no heartbeat for 30 seconds

    std::thread checker(heartbeat_checker, std::ref(worker_heap), heartbeat_interval, threshold);

    // Simulate adding and updating workers
    std::cout << "Adding worker 1 with capacity 100" << std::endl;
    worker_heap.add_worker(1, 100);

    std::this_thread::sleep_for(std::chrono::seconds(15));
    std::cout << "Updating heartbeat for worker 1" << std::endl;
    worker_heap.update_worker_heartbeat(1);

    std::this_thread::sleep_for(std::chrono::seconds(20));

    // Join the thread to allow the program to exit gracefully
    checker.join();
    return 0;
}
