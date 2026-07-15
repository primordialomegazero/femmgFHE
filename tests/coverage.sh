#!/bin/bash
# FEmmg-FHE Test Coverage Report Generator
echo "=== FEmmg-FHE Test Coverage ==="
echo "Requires: gcov, lcov (sudo apt install lcov)"
echo ""

# Check tools
if ! command -v gcov &> /dev/null; then
    echo "gcov not found. Install: sudo apt install gcov"
    exit 1
fi

# Build with coverage flags
echo "Building with coverage flags..."
g++ -std=c++17 -O0 --coverage -fprofile-arcs -ftest-coverage \
    -I/usr/local/include/openfhe -I/usr/local/include/openfhe/core \
    -I/usr/local/include/openfhe/pke -I/usr/local/include/openfhe/binfhe \
    -o /tmp/test_coverage \
    tests/test_unified_zans.cpp \
    -L/usr/local/lib -lOPENFHEcore -lOPENFHEpke -lOPENFHEbinfhe -lssl -lcrypto \
    -Wl,-rpath,/usr/local/lib 2>/dev/null && {
    /tmp/test_coverage > /dev/null 2>&1
    echo "Coverage data generated."
} || echo "Build failed — run individual binaries with coverage manually."

echo ""
echo "For full coverage report:"
echo "  1. Rebuild with: make CXXFLAGS='--coverage'"
echo "  2. Run tests: ./tests/full_blown_test.sh"
echo "  3. Generate report: lcov --capture --directory . --output-file coverage.info"
echo "  4. HTML: genhtml coverage.info --output-directory docs/coverage"
