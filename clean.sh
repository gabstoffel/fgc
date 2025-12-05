#!/bin/bash


GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}Cleaning project...${NC}"

# Clean Makefile artifacts
if [ -f "Makefile" ]; then
    make clean 2>/dev/null || true
fi

# Remove build directory
if [ -d "build" ]; then
    rm -rf build
    echo "Removed build directory"
fi

# Remove object files
find src -name "*.o" -delete 2>/dev/null || true

echo -e "${GREEN}Clean complete!${NC}"
