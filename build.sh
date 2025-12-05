#!/bin/bash


set -e  # Exit on error

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}FCG Project - Build Script${NC}"
echo "========================================"

# Check if cmake is available
if command -v cmake &> /dev/null; then
    echo -e "${GREEN}Using CMake build system${NC}"

    if [ ! -d "build" ]; then
        mkdir build
    fi

    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    cd ..

else
    echo -e "${YELLOW}CMake not found, using Makefile${NC}"
    make -j$(nproc)
fi

echo -e "${GREEN}Build complete!${NC}"
