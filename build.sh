#!/bin/bash

# Remove the build directory if it exists
if [ -d "b" ]; then
    rm -rf b
fi

# Create a new build directory
mkdir b
cd b

# Run CMake to configure the build system
cmake ..

# Build the project
make


