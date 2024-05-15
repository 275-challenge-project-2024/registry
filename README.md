# Node Registry

This project demonstrates a distributed system for managing nodes (workers and clients) using MPI (Message Passing Interface). The system allows for adding, removing, and querying nodes based on their capacities and statuses.

## Prerequisites

- Ensure you have MPI installed on your system.
- A C++ compiler that supports C++14.

## Compilation

To compile the project, use the following command:

```bash
mpic++ -std=c++14 -I include -o node_registry node_registry.cpp
```

```bash
mpirun -np 1 ./node_registry
```
