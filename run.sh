#!/bin/bash


set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}FCG Project - Build and Run Script${NC}"
echo "========================================"

# Check if cmake is available
if command -v cmake &> /dev/null; then
    echo -e "${GREEN}Using CMake build system${NC}"

    # Create build directory if it doesn't exist
    if [ ! -d "build" ]; then
        echo "Creating build directory..."
        mkdir build
    fi

    cd build

    # Configure with CMake
    echo "Configuring project..."
    cmake .. -DCMAKE_BUILD_TYPE=Release

    # Build the project
    echo "Building project..."
    make -j$(nproc)

    cd ..

    # Check if executable exists
    if [ -f "bin/Linux/main" ]; then
        echo -e "${GREEN}Build successful!${NC}"
        echo "Running executable..."
        echo "========================================"
        ./bin/Linux/main
    else
        echo -e "${RED}Error: Executable not found at bin/Linux/main${NC}"
        exit 1
    fi

else
    echo -e "${YELLOW}CMake not found, using Makefile${NC}"

    # Build using Makefile
    echo "Building project..."
    make -j$(nproc)

    # Check if executable exists
    if [ -f "bin/Linux/main" ]; then
        echo -e "${GREEN}Build successful!${NC}"
        echo "Running executable..."
        echo "========================================"
        ./bin/Linux/main
    else
        echo -e "${RED}Error: Executable not found at bin/Linux/main${NC}"
        exit 1
    fi
fi
