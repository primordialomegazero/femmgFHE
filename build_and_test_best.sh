#!/bin/bash
set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  BUILDING BEST SATOSHI CANDIDATES                         ║"
echo "╚══════════════════════════════════════════════════════════════╝"

# Find all include dirs
ALL_INCLUDES=$(find . -type d -not -path './.git/*' -not -path './build/*' -not -path './openfhe-development/*' -not -path './secp256k1/*' | sed 's/^/-I.\//' | tr '\n' ' ')
SECP256K1_INCLUDES="-I./secp256k1/include -I./secp256k1/src"

echo ""
echo "═══ 1. ULTIMATE SATOSHI (FGG Cache + BCP + φ-grouping) ═══"
echo ""

g++ -std=c++17 -O2 -o build/test_satoshi_ultimate \
    tests/breakthrough/test_satoshi_ultimate.cpp \
    $ALL_INCLUDES \
    -lsecp256k1 -lgmp -lm 2>&1 | head -20

if [ -x build/test_satoshi_ultimate ]; then
    echo "  ✅ Build success!"
    echo "  Running (timeout 60s)..."
    timeout 60 build/test_satoshi_ultimate 2>&1 || echo "  (timeout or error)"
else
    echo "  ❌ Build failed — trying without libsecp256k1..."
    g++ -std=c++17 -O2 -o build/test_satoshi_ultimate \
        tests/breakthrough/test_satoshi_ultimate.cpp \
        $ALL_INCLUDES \
        -lgmp -lm 2>&1 | head -20
    if [ -x build/test_satoshi_ultimate ]; then
        echo "  ✅ Build success (no libsecp256k1)!"
        timeout 60 build/test_satoshi_ultimate 2>&1 || echo "  (timeout or error)"
    fi
fi

echo ""
echo "═══ 2. φ-DPLL ON ECDLP (EC Constraint Propagation) ═══"
echo ""

g++ -std=c++17 -O2 -o build/test_satoshi_phi_dpll_ecdlp \
    tests/breakthrough/test_satoshi_phi_dpll_ecdlp.cpp \
    $ALL_INCLUDES \
    -lssl -lcrypto -lm 2>&1 | head -20

if [ -x build/test_satoshi_phi_dpll_ecdlp ]; then
    echo "  ✅ Build success!"
    echo "  Running (timeout 60s)..."
    timeout 60 build/test_satoshi_phi_dpll_ecdlp 2>&1 || echo "  (timeout or error)"
else
    echo "  ❌ Build failed"
fi

echo ""
echo "═══ 3. GOD LEVEL SOLVER (Python) ═══"
echo ""
timeout 30 python3 -c "
from god_level_solver import GodLevelECDLP
print('Testing GodLevelECDLP...')
solver = GodLevelECDLP('tiny17')
print(f'Curve: {solver.curve_display}')
print(f'G = {solver.G}')
print('Ready.')
" 2>&1 || echo "  (error or timeout)"

echo ""
echo "═══ DONE ═══"

