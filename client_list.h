#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

#include <vector>
#include <algorithm>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>

struct Client {
    int id;
    int capacity;
};

class ClientList {
private:
    std::vector<Client> clients;
    pthread_mutex_t mutex;

public:
    ClientList() {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mutex, &attr);
    }

    ~ClientList() {
        pthread_mutex_destroy(&mutex);
    }

    void addClient(const Client& client) {
        pthread_mutex_lock(&mutex);
        clients.push_back(client);
        pthread_mutex_unlock(&mutex);
    }

    void removeClient(int id) {
        pthread_mutex_lock(&mutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [id](const Client& client) { return client.id == id; }),
                      clients.end());
        pthread_mutex_unlock(&mutex);
    }

    // Method to get the list of clients
    const std::vector<Client>& getClients() const {
        return clients;
    }

    // Method to initialize shared memory
    static ClientList* createSharedMemory(const char* name, size_t size) {
        int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(fd, size);
        void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        return new(ptr) ClientList();
    }

    // Method to access existing shared memory
    static ClientList* accessSharedMemory(const char* name, size_t size) {
        int fd = shm_open(name, O_RDWR, 0666);
        void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        return reinterpret_cast<ClientList*>(ptr);
    }

    // Method to destroy shared memory
    static void destroySharedMemory(const char* name, size_t size) {
        shm_unlink(name);
    }
};

#endif // CLIENT_LIST_H
