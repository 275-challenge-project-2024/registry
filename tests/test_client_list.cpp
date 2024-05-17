#include <cassert>
#include <iostream>
#include "client_list.h"

void test_add_client() {
    std::cout << "Test: Add Client" << std::endl;
    ClientList client_list;
    Client client1 = {1, 100};
    client_list.addClient(client1);
    const auto& clients = client_list.getClients();
    assert(clients.size() == 1);
    assert(clients[0].id == 1);
    assert(clients[0].capacity == 100);
    std::cout << "Client added successfully.\n" << std::endl;
}

void test_add_multiple_clients() {
    std::cout << "Test: Add Multiple Clients" << std::endl;
    ClientList client_list;
    Client client1 = {1, 100};
    Client client2 = {2, 200};
    client_list.addClient(client1);
    client_list.addClient(client2);
    const auto& clients = client_list.getClients();
    assert(clients.size() == 2);
    assert(clients[0].id == 1);
    assert(clients[0].capacity == 100);
    assert(clients[1].id == 2);
    assert(clients[1].capacity == 200);
    std::cout << "Multiple clients added successfully.\n" << std::endl;
}

void test_remove_client() {
    std::cout << "Test: Remove Client" << std::endl;
    ClientList client_list;
    Client client1 = {1, 100};
    client_list.addClient(client1);
    client_list.removeClient(1);
    const auto& clients = client_list.getClients();
    assert(clients.empty());
    std::cout << "Client removed successfully.\n" << std::endl;
}

void test_remove_multiple_clients() {
    std::cout << "Test: Remove Multiple Clients" << std::endl;
    ClientList client_list;
    Client client1 = {1, 100};
    Client client2 = {2, 200};
    client_list.addClient(client1);
    client_list.addClient(client2);
    client_list.removeClient(1);
    const auto& clients = client_list.getClients();
    assert(clients.size() == 1);
    assert(clients[0].id == 2);
    client_list.removeClient(2);
    assert(clients.empty());
    std::cout << "Multiple clients removed successfully.\n" << std::endl;
}

void test_shared_memory() {
    std::cout << "Test: Shared Memory" << std::endl;
    const char* shm_name = "/test_client_list_shm";
    size_t shm_size = sizeof(ClientList);

    // Create shared memory
    ClientList* client_list = ClientList::createSharedMemory(shm_name, shm_size);
    Client client1 = {1, 100};
    client_list->addClient(client1);

    // Access shared memory
    ClientList* client_list_shared = ClientList::accessSharedMemory(shm_name, shm_size);
    const auto& clients = client_list_shared->getClients();
    assert(clients.size() == 1);
    assert(clients[0].id == 1);
    assert(clients[0].capacity == 100);

    // Destroy shared memory
    ClientList::destroySharedMemory(shm_name, shm_size);
    std::cout << "Shared memory operations completed successfully.\n" << std::endl;
}

int main() {
    test_add_client();
    test_add_multiple_clients();
    test_remove_client();
    test_remove_multiple_clients();
    test_shared_memory();
    std::cout << "===========================" << std::endl;
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}
