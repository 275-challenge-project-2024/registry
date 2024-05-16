# Node Registry

This project demonstrates a distributed system for managing nodes (workers and clients) using MPI (Message Passing Interface). The system allows for adding, removing, and querying nodes based on their capacities and statuses.

## Compilation

To compile the project, use the following command:

```bash
# Compile both files
g++ -c heap.cpp
g++ -c heartbeat.cpp

# Link the object files into a single executable
g++ -o heartbeat heap.o heartbeat.o

# Run the executable
./heartbeat
```
