#!/bin/bash
# FEmmg-FHE v20.0 — Reproducible Benchmark Script
# Hardware: AMD Ryzen 5 2600 (12 cores), 16GB RAM, Ubuntu 22.04

echo "=== FEmmg-FHE v20.0 Reproducible Benchmark ==="
echo "Cloning repository..."
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE

echo ""
echo "=== Compiling test suite (34,084 tests) ==="
g++ -std=c++17 -O3 -march=native -pthread -o test_suite src/test_suite.cpp -lm
echo "Running test suite..."
./test_suite

echo ""
echo "=== Compiling 10B ops deep circuit test ==="
g++ -std=c++17 -O3 -march=native -o test_10b_ops test_10b_ops.cpp -lm
echo "Running 10 billion operation test (takes ~8 minutes)..."
echo "For quick verification, edit test_10b_ops.cpp and reduce TOTAL."
./test_10b_ops

echo ""
echo "=== Compiling negative values test ==="
g++ -std=c++17 -O3 -march=native -o test_negative_deep test_negative_deep.cpp -lm
./test_negative_deep

echo ""
echo "=== All benchmarks complete ==="
echo "Results should match paper/10b_ops.log"
