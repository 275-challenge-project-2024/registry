#include <iostream>
#include <mpi.h>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <algorithm>
#include <cstring>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Node {
    int capacity;
    int current_capacity;
    char id[256];
    bool active;
};

class NodeRegistry {
private:
    Node* nodes;
    int max_nodes;
    MPI_Win shmwin;
    mutex mtx;

    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> workerQueue;
    unordered_map<string, json> clientMap;

public:
    NodeRegistry(Node* nodes, int max_nodes, MPI_Win& shmwin) :
        nodes(nodes), max_nodes(max_nodes), shmwin(shmwin) {}

    void processMessage(const string& messageJson) {
        lock_guard<mutex> lock(mtx);
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, shmwin);

        auto message = json::parse(messageJson);
        string id = message["id"].get<string>();
        int type = message["type"].get<int>();

        if (type == 0) { 
            // Worker node
            int capacity = message["capacity"].get<int>();
            int current_capacity = message["current-capacity"].get<int>();
            bool found = false;

            for (int i = 0; i < max_nodes; ++i) {
                if (nodes[i].active && strcmp(nodes[i].id, id.c_str()) == 0) {
                    nodes[i].capacity = capacity;
                    nodes[i].current_capacity = current_capacity;
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (int i = 0; i < max_nodes; ++i) {
                    if (!nodes[i].active) {
                        strcpy(nodes[i].id, id.c_str());
                        nodes[i].capacity = capacity;
                        nodes[i].current_capacity = current_capacity;
                        nodes[i].active = true;
                        break;
                    }
                }
            }
            workerQueue.push({current_capacity, id});
        } else if (type == 1) { 
            // Client node
            clientMap[id] = message;
        }

        MPI_Win_unlock(0, shmwin);
    }

    void removeNode(const string& nodeId) {
        lock_guard<mutex> lock(mtx);
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, shmwin);

        for (int i = 0; i < max_nodes; ++i) {
            if (nodes[i].active && strcmp(nodes[i].id, nodeId.c_str()) == 0) {
                nodes[i].active = false;
                break;
            }
        }

        MPI_Win_unlock(0, shmwin);

        priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> tempQueue;
        while (!workerQueue.empty()) {
            auto top = workerQueue.top();
            workerQueue.pop();
            if (top.second != nodeId) {
                tempQueue.push(top);
            }
        }
        swap(workerQueue, tempQueue);
    }

    void printQueue() {
        priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> tempQueue(workerQueue);
        cout << "Current Priority Queue:" << endl;
        if(workerQueue.empty()) {
            cout << "Empty Queue" << endl;
        }
        while (!tempQueue.empty()) {
            auto top = tempQueue.top();
            tempQueue.pop();
            cout << "ID: " << top.second << ", Capacity: " << top.first << endl;
        }
    }
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    const int max_nodes = 100;
    Node* nodes;
    MPI_Win shmwin;

    MPI_Win_allocate_shared(world_rank == 0 ? sizeof(Node) * max_nodes : 0,
                            sizeof(Node),
                            MPI_INFO_NULL,
                            MPI_COMM_WORLD,
                            &nodes,
                            &shmwin);

    if (world_rank == 0) {
        memset(nodes, 0, sizeof(Node) * max_nodes);
    }

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, shmwin);
    MPI_Win_sync(shmwin);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Win_unlock(0, shmwin);

    NodeRegistry registry(nodes, max_nodes, shmwin);

    if (world_rank == 0) {
        
        // registry.printQueue();

        string incomingMessage1 = R"({"id":"worker1", "type":0, "capacity":100, "current-capacity":90})";
        string incomingMessage2 = R"({"id":"worker2", "type":0, "capacity":120, "current-capacity":110})";
        string incomingMessage3 = R"({"id":"worker3", "type":0, "capacity":90, "current-capacity":85})";
        string incomingMessage4 = R"({"id":"worker4", "type":0, "capacity":10, "current-capacity":5})";
        string incomingMessage5 = R"({"id":"client1", "type":1, "other-data":"example data"})";

        registry.processMessage(incomingMessage1);
        registry.processMessage(incomingMessage2);
        registry.processMessage(incomingMessage3);
        registry.processMessage(incomingMessage4);
        registry.processMessage(incomingMessage5);

        // registry.printQueue();

        // Removing a node
        registry.removeNode("worker2");
        
        // registry.printQueue();
    }

    MPI_Win_free(&shmwin);
    MPI_Finalize();
    return 0;
}
