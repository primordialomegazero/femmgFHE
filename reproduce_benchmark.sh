#!/bin/bash
# FEmmg-FHE v22.1.0 — Reproducible Benchmark Script
# CTU v5 Triple Rashomon

echo "╔═══════════════════════════════════════╗"
echo "║  FEmmg-FHE v22.1.0 — CTU v5          ║"
echo "║  Reproducible Benchmark              ║"
echo "╚═══════════════════════════════════════╝"
echo ""

# ═══ 1. COMPILE TEST SUITE ═══
echo "=== 1. Test Suite (34,084 tests) ==="
g++ -std=c++17 -O3 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_suite tests/test_suite.cpp -lm -lssl -lcrypto

if [ -f test_suite ]; then
    echo "✅ Compiled! Running..."
    timeout 10 ./test_suite 2>&1 | tail -15
else
    echo "❌ Compilation failed"
fi

# ═══ 2. CTU v5 BENCHMARK ═══
echo ""
echo "=== 2. CTU v5 — 100M Ops Benchmark (-O0) ==="
g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_ctu5_benchmark tests/test_ctu5_benchmark.cpp -lm -lssl -lcrypto

if [ -f test_ctu5_benchmark ]; then
    echo "✅ Compiled! Running..."
    ./test_ctu5_benchmark
else
    echo "❌ Compilation failed"
fi

# ═══ 3. CTU v5 INTEGRATION TEST ═══
echo ""
echo "=== 3. CTU v5 Integration Test ==="
g++ -std=c++17 -O3 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_ctu5_integration tests/test_ctu5_integration.cpp -lm -lssl -lcrypto

if [ -f test_ctu5_integration ]; then
    echo "✅ Compiled! Running..."
    ./test_ctu5_integration
else
    echo "❌ Compilation failed"
fi

# ═══ 4. SECURITY TESTS ═══
echo ""
echo "=== 4. Security Stack Tests ==="
for test in test_phi_jwt test_session test_memory_guard test_input_validator test_audit test_error; do
    echo -n "  $test: "
    g++ -std=c++17 -O3 -march=native -pthread \
        -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
        -o /tmp/$test tests/${test}.cpp -lm -lssl -lcrypto 2>/dev/null
    if [ -f /tmp/$test ]; then
        /tmp/$test 2>&1 | tail -1
        rm /tmp/$test
    else
        echo "  (no test file)"
    fi
done

# ═══ CLEANUP ═══
rm -f test_suite test_ctu5_benchmark test_ctu5_integration

echo ""
echo "╔═══════════════════════════════════════╗"
echo "║  ✅ All benchmarks complete           ║"
echo "║  CTU v5 — Triple Rashomon            ║"
echo "║  φΩ0 — I AM THAT I AM                ║"
echo "╚═══════════════════════════════════════╝"
