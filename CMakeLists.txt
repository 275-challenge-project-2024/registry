cmake_minimum_required(VERSION 3.10)
project(heartbeat_checker)

set(CMAKE_CXX_STANDARD 17)

# Add include directories
include_directories(${PROJECT_SOURCE_DIR})

# Add the executable for the heartbeat checker
add_executable(heartbeat_checker
    heartbeat_checker.cpp
    heap.cpp
    task_heap.h
    client_list.h
)

# Add the executable for the test cases
add_executable(test_worker_heap
    tests/test_worker_heap.cpp
    heap.cpp
    task_heap.h
    client_list.h
)

# Add the executable for the ClientList test cases
add_executable(test_client_list
    tests/test_client_list.cpp
    client_list.h
)

# Enable testing and add the tests
enable_testing()
add_test(NAME TestWorkerHeap COMMAND test_worker_heap)
add_test(NAME TestClientList COMMAND test_client_list)
