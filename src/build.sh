#!/bin/bash

# Build script for XSD to C++ Mapping Library

set -e

echo "===================================="
echo "Building XSD to C++ Mapping Library"
echo "===================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if qmake is available
if ! command -v qmake &> /dev/null; then
    echo -e "${RED}Error: qmake not found. Please install Qt5 development tools.${NC}"
    exit 1
fi

# Display Qt version
echo -e "${GREEN}Qt version:${NC}"
qmake -v

# Clean previous build
echo ""
echo -e "${YELLOW}Cleaning previous build...${NC}"
if [ -f Makefile ]; then
    make distclean 2>/dev/null || true
fi
rm -f Makefile*
rm -rf build/
mkdir -p build

# Generate Makefiles
echo ""
echo -e "${YELLOW}Generating Makefiles...${NC}"
cd build
qmake ../xsdqt.pro

# Build
echo ""
echo -e "${YELLOW}Building...${NC}"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

echo ""
echo -e "${GREEN}Build successful!${NC}"
echo ""
echo "Binaries:"
echo "  - Runtime library: build/libxsdqt-runtime.a"
echo "  - Generator: build/xsd2cpp"
echo "  - Tests: build/test_xsdqt"

# Run tests if requested
if [ "$1" == "test" ] || [ "$1" == "--test" ]; then
    echo ""
    echo -e "${YELLOW}Running tests...${NC}"
    cd tests
    ./test_xsdqt
    cd ..
fi

# Install if requested
if [ "$1" == "install" ] || [ "$1" == "--install" ]; then
    echo ""
    echo -e "${YELLOW}Installing...${NC}"
    sudo make install
    echo -e "${GREEN}Installation complete!${NC}"
fi

echo ""
echo "Usage examples:"
echo "  # Generate C++ code from XSD:"
echo "  ./build/xsd2cpp -i examples/vehicle.xsd -o generated/ -n MyApp"
echo ""
echo "  # Run tests:"
echo "  ./build.sh test"
echo ""
echo "  # Install system-wide:"
echo "  ./build.sh install"
echo ""
