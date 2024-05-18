#include <cassert>
#include <thread>
#include <iostream>
#include "worker_heap.h"

void test_add_worker() {
    std::cout << "Test: Add Worker" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    assert(!worker_heap.is_empty());
    std::cout << "Worker added successfully.\n" << std::endl;
}

void test_add_multiple_workers() {
    std::cout << "Test: Add Multiple Workers" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    worker_heap.add_worker(2, 200);
    worker_heap.add_worker(3, 50);
    assert(worker_heap.get_top_worker().id == 2);
    std::cout << "Multiple workers added successfully.\n" << std::endl;
}

void test_remove_worker() {
    std::cout << "Test: Remove Worker" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    worker_heap.remove_worker(1);
    assert(worker_heap.is_empty());
    std::cout << "Worker removed successfully.\n" << std::endl;
}

void test_remove_multiple_workers() {
    std::cout << "Test: Remove Multiple Workers" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    worker_heap.add_worker(2, 200);
    worker_heap.remove_worker(1);
    assert(worker_heap.get_top_worker().id == 2);
    worker_heap.remove_worker(2);
    assert(worker_heap.is_empty());
    std::cout << "Multiple workers removed successfully.\n" << std::endl;
}

void test_update_worker_capacity() {
    std::cout << "Test: Update Worker Capacity" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    worker_heap.update_worker_capacity(1, 200);
    assert(worker_heap.get_top_worker().capacity == 200);
    std::cout << "Worker capacity updated successfully.\n" << std::endl;
}

void test_update_worker_heartbeat() {
    std::cout << "Test: Update Worker Heartbeat" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    
    std::time_t before_update = std::time(nullptr);
    std::cout << "Before heartbeat update: " << before_update << std::endl;
    
    worker_heap.update_worker_heartbeat(1);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::time_t after_update = std::time(nullptr);
    std::cout << "After heartbeat update: " << after_update << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(4));

    std::time_t current_time = std::time(nullptr);
    assert(worker_heap.get_dead_workers(current_time, 5).empty());
    std::cout << "Worker heartbeat updated successfully.\n" << std::endl;
}


void test_get_dead_workers() {
    std::cout << "Test: Get Dead Workers" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    assert(!worker_heap.get_dead_workers(std::time(nullptr), 2).empty());
    std::cout << "Dead workers retrieved successfully.\n" << std::endl;
}

void test_adding_duplicate_worker() {
    std::cout << "Test: Adding Duplicate Worker" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    try {
        worker_heap.add_worker(1, 200);
    } catch (const std::exception& e) {
        std::cout << "Caught exception for duplicate worker: " << e.what() << std::endl;
    }
    std::cout << "Duplicate worker handled successfully.\n" << std::endl;
}

void test_removing_non_existent_worker() {
    std::cout << "Test: Removing Non-Existent Worker" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    try {
        worker_heap.remove_worker(2);
    } catch (const std::exception& e) {
        std::cout << "Caught exception for non-existent worker: " << e.what() << std::endl;
    }
    std::cout << "Non-existent worker removal handled successfully.\n" << std::endl;
}

void test_get_dead_workers_with_various_thresholds() {
    std::cout << "Test: Get Dead Workers with Various Thresholds" << std::endl;
    WorkerHeap worker_heap;
    worker_heap.add_worker(1, 100);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    assert(worker_heap.get_dead_workers(std::time(nullptr), 1).size() == 1);
    assert(worker_heap.get_dead_workers(std::time(nullptr), 10).empty());
    std::cout << "Dead workers retrieved successfully with various thresholds.\n" << std::endl;
}

int main() {
    test_add_worker();
    test_add_multiple_workers();
    test_remove_worker();
    test_remove_multiple_workers();
    test_update_worker_capacity();
    test_update_worker_heartbeat();
    test_get_dead_workers();
    test_adding_duplicate_worker();
    test_removing_non_existent_worker();
    test_get_dead_workers_with_various_thresholds();
    std::cout << "===========================" << std::endl;
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}
