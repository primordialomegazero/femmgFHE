#!/bin/bash
# FEmmg-FHE v22.1.0 — Reproducible Benchmark
# CTU v5.0 Triple Rashomon
echo "╔═══════════════════════════════════════╗"
echo "║  FEmmg-FHE v22.1.0 — CTU v5.0        ║"
echo "║  Reproducible Benchmark              ║"
echo "╚═══════════════════════════════════════╝"
echo ""
echo "=== 1. Test Suite ==="
g++ -std=c++17 -O3 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_suite tests/test_suite.cpp -lm -lssl -lcrypto 2>&1 | head -3
[ -f test_suite ] && timeout 10 ./test_suite 2>&1 | tail -10 || echo "❌ Failed"
echo ""
echo "=== 2. CTU v5 Benchmark (1M ops, -O0) ==="
g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_ctu5_benchmark tests/test_ctu5_benchmark.cpp -lm -lssl -lcrypto 2>&1 | head -3
[ -f test_ctu5_benchmark ] && timeout 60 ./test_ctu5_benchmark || echo "❌ Failed"
rm -f test_suite test_ctu5_benchmark
echo ""
echo "╔═══════════════════════════════════════╗"
echo "║  ✅ Benchmark Complete                ║"
echo "║  CTU v5.0 — Triple Rashomon          ║"
echo "║  φΩ0 — I AM THAT I AM                ║"
echo "╚═══════════════════════════════════════╝"
